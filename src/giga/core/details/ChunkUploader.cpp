/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ChunkUploader.h"
#include "../../Application.h"
#include "../../api/GigaApi.h"
#include "../../api/data/Node.h"
#include "../../rest/HttpErrors.h"
#include "../../utils/Utils.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <curl_easy.h>
#include <pplx/pplxtasks.h>
#include <iosfwd>
#include <mutex>
#include <string>

using boost::filesystem::file_size;
using boost::filesystem::path;
using curl::curl_easy;
using curl::curl_ios;
using giga::data::Node;
using web::http::oauth2::experimental::oauth2_config;
using web::uri;
using web::uri_builder;
using utility::string_t;
using giga::utils::to_string;

namespace
{
class ReadCallbackData
{
public:
    explicit ReadCallbackData(const path filename);
    ~ReadCallbackData();
    ReadCallbackData(const ReadCallbackData&)            = delete;
    ReadCallbackData(ReadCallbackData&&)                 = delete;
    ReadCallbackData& operator=(ReadCallbackData&&)      = delete;
    ReadCallbackData& operator=(const ReadCallbackData&) = delete;

    void
    setChunck(uint64_t start, uint64_t end);

    size_t
    callback(char* buffer, size_t size, size_t nitems) noexcept;

private:
    std::ifstream  _file;
    uint64_t       _start;
    uint64_t       _end;
};

ReadCallbackData::ReadCallbackData (const path filename) :
        _file{}, _start{0}, _end{0}
{
    _file.open(filename.c_str(), std::fstream::binary);
}

ReadCallbackData::~ReadCallbackData ()
{
    if (_file.is_open())
    {
        _file.close();
    }
}

void
ReadCallbackData::setChunck (uint64_t start, uint64_t end)
{
    _start = start;
    _end = end;
    _file.seekg(start);
}

size_t
ReadCallbackData::callback (char* buffer, size_t size, size_t nitems) noexcept
{
    try
    {
        auto sizeToRead = std::min(size * nitems, static_cast<size_t>(_end - _start));
        _file.read(buffer, sizeToRead);
        _start += sizeToRead;
        return sizeToRead;
    }
    catch (...)
    {
        return CURL_READFUNC_ABORT;
    }
}

size_t
readCallback(void* buffer, size_t size, size_t nitems, void* obj)
{
    auto clb = static_cast<ReadCallbackData*>(obj);
    return clb->callback(static_cast<char*>(buffer), size, nitems);
}

int
curlProgressCallback (void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    auto progress = static_cast<giga::details::CurlProgress*>(clientp);
    return progress->onCallback(dltotal, dlnow, ultotal, ulnow);
}
}

namespace giga
{
namespace details
{

ChunkUploader::ChunkUploader (web::uri_builder& uploadUrl, const string_t& nodeName, const std::string& sha1, const path& filename,
                              const string_t& mime, details::CurlProgress* progress, const Application& app) :
               _uploadUrl{uri_builder{uploadUrl}.append_query(U("access_token"), app.api().getOAuthConfig()->token().access_token()).to_uri()},
               _nodeName{nodeName},
               _sha1{sha1},
               _filename{filename},
               _mime{mime},
               _fileSize{file_size(filename)},
               _progress{progress},
               _app(&app)
{
}

std::shared_ptr<Node>
ChunkUploader::upload ()
{
    auto position = 0ul;
    do {
        ReadCallbackData callbackData{_filename};

        std::ostringstream str;
        curl_ios<std::ostringstream> writer(str);
        curl_easy curl(writer);
        _progress->setCurl(curl);

        auto response = sendChunk(position, callbackData, curl, str);
        auto regex    = boost::regex{"^([0-9]+)-([0-9]+)/([0-9]+)$"};
        auto what     = boost::cmatch{};
        auto resp     = utils::wstr2str(response);
        if(boost::regex_match(resp.c_str(), what, regex))
        {
            auto start = std::stoul(string_t{what[1].first, what[1].second});
            auto end = std::stoul(string_t{what[2].first, what[2].second});
            if (start > 0 && end > start)
            {
                position = 0;
            }
            else
            {
                position = end;
            }
        }
        else
        {
            auto nodes = JSonUnserializer::fromString<std::vector<std::shared_ptr<Node>>>(response);
            if (nodes.size() != 1)
            {
                BOOST_THROW_EXCEPTION(ErrorException{U("Wrong number of nodes")});
            }
            return nodes[0];
        }
    } while (position < _fileSize);

    BOOST_THROW_EXCEPTION(ErrorException{U("Upload error")});
}

string_t
ChunkUploader::sendChunk (uint64_t position, ReadCallbackData& data, curl_easy& curl, std::ostringstream& str)
{
    if (position >= _fileSize)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Invalid position/fileSize")});
    }
    _progress->setUploadPosition(position);

    auto chunkSize = std::min(position == 0 ? 1024 : CHUNK_SIZE, _fileSize - position);
    data.setChunck(position, position + chunkSize);

    {
        auto upUri = utils::wstr2str(_uploadUrl.to_uri().to_string());
        GIGA_DEBUG_LOG(trace, _uploadUrl.to_uri().to_string());
        curl.add<CURLOPT_URL>(upUri.c_str());
    }
//#ifdef USE_DEV_GG
    curl.add<CURLOPT_SSL_VERIFYPEER>(0L);
//#endif
    curl.add<CURLOPT_FOLLOWLOCATION>(1L);
    curl.add<CURLOPT_XFERINFOFUNCTION>(curlProgressCallback);
    curl.add<CURLOPT_PROGRESSDATA>(_progress);
    curl.add<CURLOPT_NOPROGRESS>(0L);

    curl.add<CURLOPT_POST>(1L);
    curl.add<CURLOPT_POSTFIELDSIZE_LARGE>(chunkSize);
    curl.add<CURLOPT_POSTFIELDS>(nullptr);
    curl.add<CURLOPT_READFUNCTION>(&readCallback);
    curl.add<CURLOPT_READDATA>(&data);

    auto userId = _app->currentUser().id();
    curl_slist* list = nullptr;
    try {
        {
            auto hcontentDisposition = "Content-Disposition: attachment, filename=\"" + utils::wstr2str(web::uri::encode_data_string(_nodeName)) + "\"";
            auto hSession            = "Session-Id: " + std::to_string(userId) + "-" + _sha1;
            auto hcontentRange       = "Content-Range: bytes " + std::to_string(position) + "-" + std::to_string(chunkSize - 1 + position) + "/" + std::to_string(_fileSize);
            auto hcontentType        = "Content-Type: application/octet-stream";
            list = curl_slist_append(list, hcontentDisposition.c_str());
            list = curl_slist_append(list, hSession.c_str());
            list = curl_slist_append(list, hcontentRange.c_str());
            list = curl_slist_append(list, hcontentType);
            list = curl_slist_append(list, "Expect: ");
            curl.add<CURLOPT_HTTPHEADER>(list);
        }

    #ifdef DEBUG
        curl.add<CURLOPT_SSL_VERIFYPEER>(0L);
    #endif

        curl.perform();
        curl_slist_free_all(list);
    }
    catch (...)
    {
        if (list != nullptr)
        {
            curl_slist_free_all(list);
        }
        throw;
    }

    uint64_t httpCode;
    curl_easy_getinfo (curl.get_curl(), CURLINFO_RESPONSE_CODE, &httpCode);

    if (httpCode >= 300)
    {
        auto shttpCode = static_cast<unsigned short>(httpCode);
        GIGA_THROW_HTTPERROR(shttpCode, utils::str2wstr(str.str()), U(""));
    }
    return utils::str2wstr(str.str());
}

} /* namespace details */
} /* namespace giga */

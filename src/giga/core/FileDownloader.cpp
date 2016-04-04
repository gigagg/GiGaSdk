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

#include "FileDownloader.h"
#include "Node.h"
#include "FileNode.h"
#include "details/CurlWriter.h"
#include "details/CurlProgress.h"
#include "../Application.h"
#include "../api/GigaApi.h"
#include "../rest/HttpErrors.h"
#include "../utils/Utils.h"

#include <boost/filesystem.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <mutex>
#include <curl_easy.h>
#include <curl_exception.h>
#include <algorithm>
#include <thread>

using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::path;
using boost::filesystem::last_write_time;
using curl::curl_easy;
using curl::curl_easy_exception;
using curl::curl_ios;
using pplx::create_task;
using web::uri;
using web::uri_builder;
using utility::string_t;
using giga::utils::to_string;

namespace
{
int
curlProgressCallback (void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    auto progress = static_cast<giga::details::CurlProgress*>(clientp);
    return progress->onCallback(dltotal, dlnow, ultotal, ulnow);
}

size_t
curlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    auto realsize = size * nmemb;
    auto writer = static_cast<giga::details::CurlWriter*>(userp);
    return writer->write(static_cast<const char *>(contents), realsize);
}
}

namespace giga
{
namespace core
{

FileDownloader::FileDownloader (const boost::filesystem::path& folder, const Node& node, const Application& app, pplx::cancellation_token_source cts, Policy policy) :
        FileTransferer{cts}, _task{}, _tempFile{}, _destFile{}, _fileUri{},
        _fileSize{node.size()}, _startAt{0}, _lastUpdateDate{node.lastUpdateDate()}, _policy{policy},
        _app(&app)
{
    if (!is_directory(folder))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("FolderDest should be a directory")});
    }
    if (node.name() == U("") || node.name() == U(".") || node.name() == U(".."))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Invalid node name")});
    }

    auto name = utils::cleanUpFilename(node.name());
    auto pos = name.find_last_of(U("."));
    auto firstPart = name;
    auto lastPart = string_t{};
    if (pos != string_t::npos)
    {
        firstPart = name.substr(0, pos);
        lastPart = name.substr(pos, name.length() - pos + 1);
    }
    auto count = 0;
    while (policy == Policy::rename && exists(folder / name))
    {
        name = firstPart + U("-") + to_string(++count) + lastPart;
    }

    auto fid = node.fileData().fid();
    std::replace(fid.begin(), fid.end(), '/', '_');

    _tempFile = folder /  (U(".") + utils::str2wstr(fid) + U(".part"));
    _destFile = folder / name;

    _fileUri = node.fileData().fileUrl();

    if (exists(_tempFile))
    {
        _startAt = boost::filesystem::file_size(_tempFile);
    }
    // TODO Make sure there is enough space on device.
}

FileDownloader::~FileDownloader ()
{
}

FileDownloader::FileDownloader (FileDownloader&& other) :
                FileTransferer{std::move(other)},
                _task{std::move(other._task)},
                _tempFile{std::move(other._tempFile)},
                _destFile{std::move(other._destFile)},
                _fileUri{std::move(other._fileUri)},
                _fileSize{other._fileSize},
                _startAt{other._startAt},
                _lastUpdateDate{other._lastUpdateDate},
                _policy{other._policy},
                _app{_app}
{
}

struct AutoClosingFilestream
{
    explicit
    AutoClosingFilestream () : file{}{}
    ~AutoClosingFilestream()
    {
        if (file.is_open())
        {
            file.close();
        }
    }
    std::ofstream file;
};

void
FileDownloader::doStart()
{
    auto tempFile = _tempFile;
    auto destFile = _destFile;
    auto fileSize = _fileSize;
    auto policy   = _policy;
    auto progress = _progress.get();

    uri_builder b{_fileUri};
    b.append_query(U("access_token"), _app->api().getOAuthConfig()->token().access_token());
    auto fileUri = b.to_uri();

    auto ignore = false;
    if (_policy == Policy::overrideNewerSize && exists(_destFile))
    {
        auto lastWriteTime = std::chrono::system_clock::from_time_t(last_write_time(_destFile));
        ignore = fileSize == boost::filesystem::file_size(_destFile) && lastWriteTime >= _lastUpdateDate;
    }

    if (ignore || (_policy == Policy::ignore && exists(_destFile)))
    {
        _task = pplx::create_task([destFile]() {
            return destFile;
        }, _cts.get_token());
    }
    else
    {
        _task = _app->api().refreshToken().then([tempFile, fileUri, progress, fileSize]() {
            try {
                details::CurlWriter writer{tempFile};

                unsigned short httpCode = 0;
                for (auto i = 0; i < 5 && httpCode != 200; ++i)
                {
                    if (i != 0)
                    {
                        // Wait 3 seconds on retry
                        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                    }
                    auto pos = writer.file().tellp();
                    if (pos == static_cast<int64_t>(fileSize))
                    {
                        return; // the file is already completed.
                    }

                    curl_ios<details::CurlWriter> easyWriter(&writer, &curlWriteCallback);
                    curl_easy curl(easyWriter);
                    progress->setCurl(curl);
                    writer.setCurl(curl);

                    GIGA_DEBUG_LOG(U("downloading: ") << fileUri.to_string());

                    auto filUriStr = utils::wstr2str(fileUri.to_string());
                    curl.add<CURLOPT_URL>(filUriStr.c_str());
                    curl.add<CURLOPT_FOLLOWLOCATION>(1L);
                    curl.add<CURLOPT_XFERINFOFUNCTION>(curlProgressCallback);
                    curl.add<CURLOPT_XFERINFODATA>(progress);
                    curl.add<CURLOPT_NOPROGRESS>(0L);

                    if (pos > 0)
                    {
                        auto posStr = std::to_string(pos) + "-";
                        curl.add<CURLOPT_RANGE>(posStr.c_str());
                    }

#ifdef DEBUG
                    curl.add<CURLOPT_SSL_VERIFYPEER>(0L);
#endif
                    curl.perform();
                    curl_easy_getinfo (curl.get_curl(), CURLINFO_RESPONSE_CODE, &httpCode);
                    if (httpCode >= 300)
                    {
                        GIGA_DEBUG_LOG(U("downloading error (retrying): ") << writer.getErrorData());
                    }
                }
                if (httpCode != 200)
                {
                    if (httpCode != CURLE_ABORTED_BY_CALLBACK)
                    {
                        boost::filesystem::remove(tempFile);
                    }
                    BOOST_THROW_EXCEPTION(HttpErrorGeneric::create(httpCode));
                }
            } catch (const curl_easy_exception& error) {
                auto track = error.get_traceback();
                std::ostringstream ss;
                for(const auto& obj : track)
                {
                    ss << obj.first << ": " << obj.second << "\n";
                    if (obj.first == "Operation was aborted by an application callback")
                    {
                        throw pplx::task_canceled{"Download canceled"};
                    }
                }
                BOOST_THROW_EXCEPTION(ErrorException{utils::str2wstr(ss.str())});
            }
        }, _cts.get_token()).then([destFile, tempFile, policy]() {
            if (policy != Policy::override && policy != Policy::overrideNewerSize && exists(destFile))
            {
                // TODO: remove tempFile ?
                BOOST_THROW_EXCEPTION(ErrorException{U("Destination file already exists")});
            }

            boost::filesystem::rename(tempFile, destFile);
            return destFile;
        }, _cts.get_token());
    }
}

const pplx::task<boost::filesystem::path>&
FileDownloader::task () const
{
    return _task;
}

path
FileDownloader::downloadingFile () const
{
    return _tempFile;
}

path
FileDownloader::destinationFile () const
{
    return _destFile;
}

FileTransferer::Progress
FileDownloader::progress () const
{
    auto p = _progress->data();
    if (_state != State::pending && _task.is_done())
    {
        return Progress{_fileSize, _fileSize};
    }
    return Progress{p.dlnow + _startAt, _fileSize};
}

} /* namespace core */
} /* namespace giga */

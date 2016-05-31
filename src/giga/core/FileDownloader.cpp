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
        FileTransferer{cts}, _task{}, _tempFile{}, _destFile{}, _action{Action::fileDownloaded}, _fileUri{},
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
        _action = Action::fileRenamed;
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
    auto fileUri  = _fileUri;
    auto action   = _action;

    auto ignore = false;
    if (_policy == Policy::overrideNewerSize && exists(_destFile))
    {
        auto lastWriteTime = std::chrono::system_clock::from_time_t(last_write_time(_destFile));
        ignore = fileSize == boost::filesystem::file_size(_destFile) && lastWriteTime >= _lastUpdateDate;
    }

    if (ignore || (_policy == Policy::ignore && exists(_destFile)))
    {
        _task = pplx::create_task([destFile]() {
            return Result{destFile, Action::fileIgnored};
        }, _cts.get_token());
    }
    else
    {
        auto cts  = _cts;
        auto& api = _app->api();
        auto ua   = _app->userAgent().c_str();
        _task = api.refreshToken().then([tempFile, fileUri, progress, fileSize, cts, &api, action, ua]() {

            uint64_t httpCode = 0;
            const auto maxTry = 5;
            for (auto i = 0; i <= maxTry && httpCode != 200; ++i)
            {
                uri_builder b{fileUri};
                b.append_query(U("access_token"), api.accessToken());
                auto tokenedFileUri = b.to_uri().to_string();

                try {
                    details::CurlWriter writer{tempFile};
                    auto pos = writer.file().tellp();
                    if (pos == static_cast<int64_t>(fileSize))
                    {
                        return; // the file is already completed.
                    }

                    curl_ios<details::CurlWriter> easyWriter(&writer, &curlWriteCallback);
                    curl_easy curl(easyWriter);
                    progress->setCurl(curl);
                    writer.setCurl(curl);

                    GIGA_DEBUG_LOG(trace, U("downloading: ") << tokenedFileUri);

                    auto filUriStr = utils::wstr2str(tokenedFileUri);
                    curl.add<CURLOPT_URL>(filUriStr.c_str());
                    curl.add<CURLOPT_FOLLOWLOCATION>(1L);
                    curl.add<CURLOPT_XFERINFOFUNCTION>(curlProgressCallback);
                    curl.add<CURLOPT_XFERINFODATA>(progress);
                    curl.add<CURLOPT_NOPROGRESS>(0L);
                    curl.add<CURLOPT_USERAGENT>(ua);

                    if (pos > 0)
                    {
                        auto posStr = std::to_string(pos) + "-";
                        curl.add<CURLOPT_RANGE>(posStr.c_str());
                    }

//#ifdef USE_DEV_GG
                    curl.add<CURLOPT_SSL_VERIFYPEER>(0L);
//#endif
                    curl.perform();
                    curl_easy_getinfo (curl.get_curl(), CURLINFO_RESPONSE_CODE, &httpCode);
                    if (httpCode >= 300)
                    {
                        GIGA_DEBUG_LOG(trace, U("downloading error (retrying): ") << writer.getErrorData());
                    }
                    if (httpCode != 200)
                    {
                        if (httpCode != CURLE_ABORTED_BY_CALLBACK && httpCode != 206)
                        {
                            boost::filesystem::remove(tempFile);
                        }
                        auto shttpCode = static_cast<unsigned short>(httpCode);
                        GIGA_THROW_HTTPERROR(shttpCode, U(""), U(""));
                    }

                    // httpcode == 200 => download is complete.
                    return;
                }
                catch (ErrorUnauthorized const&)
                {
                    api.refreshToken().wait();

                    uri_builder bu{fileUri};
                    bu.append_query(U("access_token"), api.accessToken());
                    tokenedFileUri = bu.to_uri().to_string();

                    if (i == maxTry)
                    {
                        throw;
                    }
                }
                catch (...)
                {
                    if (i == maxTry || cts.get_token().is_canceled())
                    {
                        throw;
                    }
                    GIGA_DEBUG_LOG(debug, utils::exceptionInfos());
                    std::this_thread::sleep_for(std::chrono::milliseconds(250 * i));
                }
            }
        }, _cts.get_token()).then([destFile, tempFile, policy, action]() {
            auto destfileExists = exists(destFile);
            if (policy != Policy::override && policy != Policy::overrideNewerSize && destfileExists)
            {
                // TODO: remove tempFile ?
                BOOST_THROW_EXCEPTION(ErrorException{U("Destination file already exists")});
            }

            boost::filesystem::rename(tempFile, destFile);

            if (action == Action::fileRenamed)
            {
                return Result{destFile, Action::fileRenamed};
            }
            if (action == Action::fileDownloaded && destfileExists)
            {
                return Result{destFile, Action::fileOverriden};
            }
            return Result{destFile, Action::fileDownloaded};

        }, _cts.get_token());
    }
}

const pplx::task<FileDownloader::Result>&
FileDownloader::task () const
{
    return _task;
}

const path&
FileDownloader::downloadingFile () const
{
    return _tempFile;
}

const path&
FileDownloader::destinationFile () const
{
    return _destFile;
}

const path&
FileDownloader::filename () const
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

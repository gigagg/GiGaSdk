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

#include "FileUploader.h"
#include "Node.h"
#include "details/ChunkUploader.h"
#include "details/CurlWriter.h"
#include "../Application.h"
#include "../api/data/Node.h"
#include "../api/data/DataNode.h"
#include "../api/data/User.h"
#include "../utils/Crypto.h"
#include "../utils/Utils.h"

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <boost/filesystem.hpp>
#include <utility>
#include <curl_easy.h>
#include <chrono>
#include <mutex>
#include <thread>

using boost::filesystem::path;
using Concurrency::streams::basic_istream;
using Concurrency::streams::file_stream;
using giga::details::ChunkUploader;
using giga::data::Node;
using pplx::cancellation_token_source;
using pplx::create_task;
using pplx::task;
using web::http::client::http_client;
using web::http::client::http_client_config;
using web::http::http_request;
using web::http::http_response;
using web::http::methods;
using web::http::status_codes;
using web::uri;
using web::uri_builder;
using utility::string_t;

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

FileUploader::FileUploader (const path& filename, const string_t& nodeName, const std::string& parentId,
                            const std::string& sha1, const std::string& fid, const std::string& fkey,
                            const Application& app, pplx::cancellation_token_source cts) :
        FileTransferer{cts},
        _task{},
        _filename{filename},
        _nodeName{nodeName},
        _parentId{parentId},
        _sha1{sha1},
        _fid{fid},
        _fkey{fkey},
        _fileSize{boost::filesystem::file_size(filename)},
        _app(&app)
{
}

FileUploader::~FileUploader()
{
    try
    {
        _task.wait();
    }
    catch (...)
    {
        GIGA_DEBUG_LOG(warning, utils::exceptionInfos());
    }
}

void
FileUploader::doStart ()
{
    auto filename   = _filename;
    auto nodeName   = _nodeName;
    auto parentId   = _parentId;
    auto sha1       = _sha1;
    auto fid        = _fid;
    auto fkey       = _fkey;
    auto nodeKeyCl  = _app->currentUser().personalData().nodeKeyClear();
    auto cts        = _cts;
    auto progress   = _progress.get();
    auto app        = _app;


    _task = _app->api().refreshToken().then([=] {
        try {

            //
            // Test if the file is on giga (and add it if possible)
            //

            auto res = app->api().nodes.addNode(nodeName, U("file"), parentId, fkey, fid).get();
            return std::shared_ptr<data::Node>{std::move(res->data)};
        } catch (const ErrorNotFound& e) {

            //
            // The file is not yet on giga
            //

            if (e.getJson().has_field(U("uploadUrl"))) {
                auto uploadUrl = e.getJson().at(U("uploadUrl")).as_string();
                auto uriBuilder = uri_builder(uri{U("https:") + uploadUrl + web::uri::encode_data_string(utils::str2wstr(nodeKeyCl))});
                const auto maxTry = 3;
                for (auto i = 1; i <= maxTry; ++i) {
                    try {
                        ChunkUploader ch{uriBuilder, nodeName, sha1, filename, U("application/octet-stream"), progress, *app};
                        return ch.upload();
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
            }
            throw;
        } catch (const ErrorLocked& e) {

            //
            // The file is already on giga (same fid/name).
            //

            if (e.getJson().has_field(U("data"))) {
                auto s = JSonUnserializer{e.getJson().at(U("data"))};
                return s.unserialize<std::shared_ptr<data::Node>>();
            }
            throw;
        }
    }, _cts.get_token()).then([=] (std::shared_ptr<data::Node> n) {
        return std::shared_ptr<Node>(Node::create(n, *app).release());
    }, _cts.get_token());
}

const pplx::task<std::shared_ptr<Node>>&
FileUploader::task () const
{
    return _task;
}

FileTransferer::Progress
FileUploader::progress () const
{
    auto p = _progress->data();
    if (_state != State::pending && _task.is_done())
    {
        return Progress{_fileSize, _fileSize};
    }
    return Progress{p.ulnow, _fileSize};
}

const string_t&
FileUploader::nodeName() const
{
    return _nodeName;
}

const boost::filesystem::path&
FileUploader::filename() const
{
    return _filename;
}

uint64_t
FileUploader::fileSize() const
{
    return _fileSize;
}

} /* namespace api */
} /* namespace giga */

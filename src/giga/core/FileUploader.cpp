/*
 * FileUploader.cpp
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#include "FileUploader.h"
#include "Node.h"
#include "details/ChunkUploader.h"
#include "details/CurlWriter.h"
#include "../Application.h"
#include "../api/data/Node.h"
#include "../api/data/DataNode.h"
#include "../api/data/User.h"
#include "../api/NodesApi.h"
#include "../utils/Crypto.h"

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <exception>
#include <utility>
#include <boost/filesystem.hpp>
#include <curl_easy.h>
#include <mutex>

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

FileUploader::FileUploader (const std::string& filename, const std::string& nodeName, const std::string& parentId, const std::string& sha1,
                            const std::string& fid, const std::string& fkey) :
        FileTransferer{}, _task{}, _filename{filename}, _nodeName{nodeName},
        _parentId{parentId}, _sha1{sha1}, _fid{fid}, _fkey{fkey}, _fileSize{boost::filesystem::file_size(filename)}
{
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
    auto nodeKeyCl  = Application::get().currentUser().privateData().nodeKeyClear;
    auto cts        = _cts;
    auto progress   = &_progress;

    _task = create_task([=] {
        try {

            //
            // Test if the file is on giga (and add it if possible)
            //

            auto res = NodesApi::addNode(nodeName, "file", parentId, fkey, fid).get();
            return std::shared_ptr<data::Node>{std::move(res->data)};
        } catch (const ErrorNotFound& e) {

            //
            // The file is not yet on giga
            //

            if (e.getJson().has_field("uploadUrl")) {
                auto uploadUrl = e.getJson().at("uploadUrl").as_string();
                auto uriBuilder = uri_builder(uri{"https:" + uploadUrl + web::uri::encode_data_string(nodeKeyCl)});
                ChunkUploader ch{uriBuilder, nodeName, sha1, filename, "application/octet-stream", progress};
                return ch.upload();
            }
            throw;
        } catch (const ErrorLocked& e) {

            //
            // The file is already on giga (same fid/name).
            //

            if (e.getJson().has_field("data")) {
                auto s = JSonUnserializer{e.getJson().at("data")};
                return s.unserialize<std::shared_ptr<data::Node>>();
            }
            throw;
        }
    }).then([=] (std::shared_ptr<data::Node> n) {
        return std::shared_ptr<Node>(Node::create(n).release());
    });
}

const pplx::task<std::shared_ptr<Node>>&
FileUploader::task () const
{
    return _task;
}

double
FileUploader::progress () const
{
    auto p = _progress.data();
    if (p.ulnow <= 0)
    {
        return 0.;
    }
    return ((double) p.ulnow) / (double) (_fileSize);
}

} /* namespace api */
} /* namespace giga */

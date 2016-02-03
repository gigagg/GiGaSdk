/*
 * FileUploader.cpp
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#include "FileUploader.h"
#include "ChunkUploader.h"
#include "data/Node.h"
#include "data/DataNode.h"
#include "data/User.h"
#include "NodesApi.h"
#include "../utils/Crypto.h"

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <exception>
#include <utility>
#include <boost/filesystem.hpp>
#include <mutex>

using Concurrency::streams::basic_istream;
using Concurrency::streams::file_stream;
using giga::api::ChunkUploader;
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

namespace giga
{
namespace api
{

struct UploadState
{
    UploadState (const std::string& filename, const std::string& nodeName, const std::string& parentId, const std::string& nodeKeyClear) :
            filename{filename},
            nodeName{nodeName},
            parentId{parentId},
            nodeKeyClear{nodeKeyClear},
            sha1{}, fid{}, fkey{}, _isDone{false},
            uploader{std::unique_ptr<ChunkUploader>{}},
            mut{}
    {
    }

    void
    setIdCalculated (std::string&& sha1, std::string&& fid, std::string&& fkey)
    {
        std::lock_guard<std::mutex> l(mut);
        this->sha1 = sha1;
        this->fid  = fid;
        this->fkey = fkey;
    }

    bool
    isIdCalculated() const
    {
        std::lock_guard<std::mutex> l(mut);
        return fid != "" && sha1 != "" && fkey != "";
    }

    const std::string filename;
    const std::string nodeName;
    const std::string parentId;
    const std::string nodeKeyClear;

    std::string sha1;
    std::string fid;
    std::string fkey;


    bool
    isDone() const
    {
        std::lock_guard<std::mutex> l(mut);
        return _isDone;
    }

    void
    setDone()
    {
        std::lock_guard<std::mutex> l(mut);
        _isDone = true;
    }

    std::unique_ptr<ChunkUploader>&
    getUploader()
    {
        std::lock_guard<std::mutex> l(mut);
        return uploader;
    }

    void
    setUploader(std::unique_ptr<ChunkUploader>&& up)
    {
        std::lock_guard<std::mutex> l(mut);
        uploader = std::move(up);
    }

private:
    bool _isDone;
    std::unique_ptr<ChunkUploader> uploader;
    mutable std::mutex mut;
};

FileUploader::FileUploader (const std::string& filename, const std::string& nodeName, const std::string& parentId,
                            const std::string& nodeKeyClear) :
        _state{FileUploader::State::pending},
        _result{},
        _uploadState{std::unique_ptr<UploadState>{new UploadState{filename, nodeName, parentId, nodeKeyClear}}},
        _cts{}
{
}

FileUploader::~FileUploader()
{
}

struct SetDoneRaii
{
    SetDoneRaii(UploadState& uploadState):
        uploadState(uploadState)
    {}
    ~SetDoneRaii()
    {
        uploadState.setDone();
    }

    UploadState& uploadState;
};

void
FileUploader::start ()
{
    if (_state != State::pending) {
        THROW(ErrorException{"Start is valid only in 'pending' state"});
    }

    auto cts          = _cts;
    auto& uploadState = _uploadState;

    _task = create_task([&uploadState]() -> void {
        if (uploadState->isIdCalculated())
        {
            return;
        }

        auto sha1 = Crypto::sha1File(uploadState->filename);
        auto fkey = Crypto::calculateFkey(sha1);
        auto decodedNodeKey = Crypto::base64decode(uploadState->nodeKeyClear);
        auto fkeyEnc = Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey);
        uploadState->setIdCalculated(std::move(sha1), Crypto::calculateFid(sha1), Crypto::base64encode(fkeyEnc));

    }).then([&uploadState, cts] () {

        SetDoneRaii done(*uploadState);
        try {

            //
            // Test if the file is on giga (and add it if possible)
            //

            auto res = NodesApi::addNode(uploadState->nodeName, "file", uploadState->parentId, uploadState->fkey, uploadState->fid).get();
            return std::shared_ptr<Node>{std::move(res->data)};
        } catch (const ErrorNotFound& e) {

            //
            // The file is not yet on giga
            //

            if (e.getJson().has_field("uploadUrl")) {
                auto uploadUrl = e.getJson().at("uploadUrl").as_string();
                auto uriBuilder = uri_builder(uri{"https:" + uploadUrl + web::uri::encode_data_string(uploadState->nodeKeyClear)});
                uploadState->setUploader(std::unique_ptr<ChunkUploader>{
                    new ChunkUploader{uriBuilder, uploadState->nodeName, uploadState->sha1, uploadState->filename, "application/octet-stream", cts}
                });
                return uploadState->getUploader()->upload();
            }
            throw;
        } catch (const ErrorLocked& e) {

            //
            // The file is already on giga (same fid/name).
            //

            if (e.getJson().has_field("data")) {
                auto s = JSonUnserializer{e.getJson().at("data")};
                return s.unserialize<std::shared_ptr<Node>>();
            }
            throw;
        }
    });

    _state = State::started;
}

void
FileUploader::pause ()
{
    if (_state != State::started) {
        THROW(ErrorException{"Pause is valid only in 'started' state"});
    }
    _state = State::paused;
    _cts.cancel();
    _task.wait();
}

void
FileUploader::resume ()
{
    if (_state != State::paused) {
        THROW(ErrorException{"Resume is valid only in 'paused' state"});
    }
    _cts = cancellation_token_source{};
    _state = State::pending;
    start();
    _state = State::started;
}

void
FileUploader::cancel ()
{
    if (_state != State::paused && _state != State::started) {
        THROW(ErrorException{"Resume is valid only in 'paused' and 'started' state"});
    }
    _state = State::canceled;
    _cts.cancel();
    _task.wait();
}

FileUploader::Progress
FileUploader::progress () const
{
    if (_uploadState->isDone())
    {
        return Progress{1., 1.};
    }
    if (_uploadState->getUploader() != nullptr)
    {
        return Progress{1., _uploadState->getUploader()->progress()};
    }
    if (_uploadState->isIdCalculated())
    {
        return Progress{1., 0.};
    }
    return Progress{0., 0.};
}

std::shared_ptr<data::Node>
FileUploader::result () const
{
    if (_task.wait() != pplx::task_status::canceled)
    {
        return _task.get();
    }
    return std::shared_ptr<data::Node>{};
}

} /* namespace api */
} /* namespace giga */

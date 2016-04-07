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

#include "Uploader.h"

#include "../rest/HttpErrors.h"
#include "../utils/make_unique.h"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "../Application.h"
#include "../utils/Crypto.h"
using boost::filesystem::path;
using moodycamel::BlockingReaderWriterQueue;
using utility::details::make_unique;

namespace giga
{
namespace core
{

struct ScannedNode
{
    explicit ScannedNode(FolderNode parent, boost::filesystem::path path, Uploader::FileTree entry):
            parent{std::move(parent)}, path{std::move(path)}, entry{std::move(entry)}
    {}

    FolderNode              parent;
    boost::filesystem::path path;
    Uploader::FileTree      entry;
};

struct PreparedFile
{
    explicit
    PreparedFile (std::string parentId, boost::filesystem::path path, std::string sha1, std::string fkey, std::string fid, std::string fkeyEnc) :
            parentId(parentId), path(path), sha1(sha1), fkey(fkey), fid(fid), fkeyEnc(fkeyEnc)
    {
    }
    std::string             parentId;
    boost::filesystem::path path;
    std::string             sha1;
    std::string             fkey;
    std::string             fid;
    std::string             fkeyEnc;
};

struct UploadRequest
{
    explicit UploadRequest(FolderNode parent, std::vector<boost::filesystem::path> pathes):
                    parent{std::move(parent)}, pathes{std::move(pathes)}
    {}
    FolderNode                           parent;
    std::vector<boost::filesystem::path> pathes;
};

Uploader::Uploader(const Application& app):
    _upRequest{},
    _toPrepare{},
    _toUpload{},
    _scanningFile{nullptr},
    _preparingFile{nullptr},
    _uploadingFile{nullptr},
    _scanCts{},
    _clearCts{},
    _cts{},
    _mainTask{},
    _isStarted{false},
    _mut{},
    _upBytes{0},
    _upCount{0},
    _sha1Bytes{0},
    _sha1Count{0},
    _totalCount{0},
    _isFinished{false},
    _progressUp{[](FileTransferer&, uint64_t, uint64_t){}},
    _progressPrep{[](FileTransferer&, uint64_t, uint64_t){}},
    _onNewNode{[](FileTransferer&, std::shared_ptr<Node>){}},
    _app(&app),
    _rate{0}
{
}

Uploader::~Uploader()
{
    if (_isStarted)
    {
        try
        {
            join();
        }
        catch (...)
        {
            GIGA_DEBUG_LOG(boost::current_exception_diagnostic_information());
        }
    }
}

void
Uploader::setUploadProgressFct (ProgressFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _progressUp = fct;
}

void
Uploader::setPreparationProgressFct (ProgressFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _progressPrep = fct;
}

void
Uploader::setOnNewNodeFct (NewNodeFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _onNewNode = fct;
}

void
Uploader::addUpload (FolderNode parent, boost::filesystem::path&& path)
{
    std::vector<boost::filesystem::path> vect;
    vect.emplace_back(std::move(path));
    addUploads(std::move(parent), std::move(vect));
}

void
Uploader::addUploads(FolderNode parent, std::vector<boost::filesystem::path>&& pathes)
{
    _upRequest.enqueue(giga::make_unique<UploadRequest>(std::move(parent), std::move(pathes)));
}

void
Uploader::start()
{
    auto scanTask = pplx::create_task([this]() {
        std::unique_ptr<UploadRequest> element = nullptr;
        for(_upRequest.wait_dequeue(element); element != nullptr; _upRequest.wait_dequeue(element))
        {
            {
                std::lock_guard<std::mutex> l(_mut);
                _scanningFile = std::move(element);
            }
            for (const auto& path : _scanningFile->pathes)
            {
                    FileTree e{U(""), is_directory(path) ? FileTree::Type::directory : FileTree::Type::file};
                    scanFiles(e, path);
                    _toPrepare.enqueue(giga::make_unique<ScannedNode>(_scanningFile->parent, path, std::move(e)));
            }
        }
    });

    auto prepareTask = pplx::create_task([this]() {
        std::unique_ptr<ScannedNode> element = nullptr;
        for(_toPrepare.wait_dequeue(element); element != nullptr; _toPrepare.wait_dequeue(element))
        {
            try
            {
                prepare(element->parent, element->path, element->entry);
            }
            catch (...)
            {
                if (_clearCts.get_token().is_canceled())
                {
                    _clearCts = {};
                }
                _errors.enqueue(std::make_pair(element->path, boost::current_exception_diagnostic_information()));
            }
        }
    });

    auto uploadTask = pplx::create_task([this]() {
        std::unique_ptr<PreparedFile> element = nullptr;
        for(_toUpload.wait_dequeue(element); element != nullptr; _toUpload.wait_dequeue(element))
        {
            try
            {
                uploadFile(element);
            }
            catch (...)
            {
                _errors.enqueue(std::make_pair(element->path, boost::current_exception_diagnostic_information()));
            }
        }
        _isFinished = true;
    });

    auto progressTask = pplx::create_task([this]() {
        while(!_isFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::lock_guard<std::mutex> l(_mut);
            if (_uploadingFile != nullptr)
            {
                _progressUp(*_uploadingFile, _totalCount - _upCount, _upBytes + _uploadingFile->progress().transfered);
            }
            if (_preparingFile != nullptr)
            {
                _progressPrep(*_preparingFile, _totalCount - _sha1Count, _sha1Bytes + _preparingFile->progress().transfered);
            }
        }
    });

    auto tasks = {scanTask, prepareTask, uploadTask, progressTask};
    _mainTask =  pplx::when_all(tasks.begin(), tasks.end());
    _isStarted = true;
}

void
Uploader::join()
{
    if (_isStarted)
    {
        _upRequest.enqueue(nullptr);
        _toPrepare.enqueue(nullptr);
        _toUpload.enqueue(nullptr);
        _mainTask.wait();
        _isStarted = false;
    }
}

void
Uploader::kill()
{
    if (_isStarted)
    {
        clearQueues();
        _upRequest.enqueue(nullptr);
        _toPrepare.enqueue(nullptr);
        _toUpload.enqueue(nullptr);
        _cts.cancel();
        join();
    }
}

void
Uploader::limitRate(uint64_t rate)
{
    _rate = rate;
    std::lock_guard<std::mutex> l(_mut);
    _uploadingFile->limitRate(rate);
}

void
Uploader::pause()
{
    std::lock_guard<std::mutex> l(_mut);
    _uploadingFile->pause();
}

void
Uploader::resume()
{
    std::lock_guard<std::mutex> l(_mut);
    _uploadingFile->resume();
}

void
Uploader::clearQueues ()
{
    std::unique_ptr<UploadRequest> u = nullptr;
    while (_upRequest.try_dequeue(u))
    {}
    std::unique_ptr<ScannedNode> s = nullptr;
    while (_toPrepare.try_dequeue(s))
    {}
    std::unique_ptr<PreparedFile> p = nullptr;
    while (_toUpload.try_dequeue(p))
    {}
    Error e;
    while (_errors.try_dequeue(e))
    {}
}

void
Uploader::clear()
{
    if (_isStarted)
    {
        clearQueues();
        _clearCts.cancel();
        {
            std::lock_guard<std::mutex> l(_mut);
            _upBytes = _uploadingFile != nullptr ? _uploadingFile->progress().transfered : 0ul;
            _upCount = 0;
            _sha1Bytes = _preparingFile != nullptr ? _preparingFile->progress().transfered : 0ul;
            _sha1Count = 0;
            _totalCount = 0;
        }
        clearQueues();
    }
}

FileTransferer::State
Uploader::state()
{
    std::lock_guard<std::mutex> l(_mut);
    return _uploadingFile->state();
}

bool
Uploader::isStarted() const
{
    std::lock_guard<std::mutex> l(_mut);
    return _isStarted;
}

std::unique_ptr<Uploader::Error>
Uploader::consumeError ()
{
    Error e{};
    if (!_errors.try_dequeue(e))
    {
        return nullptr;
    }
    return giga::make_unique<Uploader::Error>(e);

}


void
Uploader::scanFiles(FileTree& parent, const boost::filesystem::path& ppath)
{
    if (_clearCts.get_token().is_canceled())
    {
        return;
    }
    if (parent.type == FileTree::Type::file)
    {
        std::lock_guard<std::mutex> l{_mut};
        ++_totalCount;
        return;
    }

    using namespace boost::filesystem;
    auto rng = boost::make_iterator_range(directory_iterator(ppath), directory_iterator());
    for (directory_entry& entry : rng)
    {
        if (_cts.get_token().is_canceled())
        {
            return;
        }

        const auto& path = entry.path();
        auto name        = path.filename().native();
        auto isDirectory = is_directory(path);
        if (!exists (path) || (!isDirectory && !is_regular_file(path)))
        {
            continue;
        }
        if (name == U("") || name == U(".") || name == U(".."))
        {
            continue;
        }

        auto it = parent.children.emplace(parent.children.end(), std::move(name), isDirectory ? FileTree::Type::directory : FileTree::Type::file);
        auto& children = *it;
        scanFiles(children, entry.path());
    }
    std::sort(parent.children.begin(), parent.children.end(), [](const FileTree& lhs, const FileTree& rhs) {
       return lhs.filename <  rhs.filename;
    });
}


void
Uploader::prepare (FolderNode& parent, const boost::filesystem::path& aPath, const FileTree& entry)
{
    using namespace boost::filesystem;
    auto currentPath = aPath;
    if (entry.filename != U("") && entry.filename != U("."))
    {
        currentPath /= entry.filename;
    }

    if (_cts.get_token().is_canceled())
    {
        return;
    }
    if (!exists (currentPath))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("File or directory not found")});
    }

    if (entry.type == FileTree::Type::file)
    {

        auto parentId = parent.id();
        auto nodeName = currentPath.filename().native();
        auto decodedNodeKey = Crypto::base64decode(_app->currentUser().personalData().nodeKeyClear());

        // WARNING: uploader gets moved into _preparingFile
        auto calculator = std::unique_ptr<Sha1Calculator>{new Sha1Calculator(currentPath, _clearCts)};
        calculator->start();

        auto task = calculator->task().then([=](std::string sha1) {
            auto fkey = Crypto::calculateFkey(sha1);
            auto prepared = std::make_shared<PreparedFile>(
                parent.id(),
                currentPath,
                sha1,
                fkey,
                Crypto::calculateFid(sha1),
                Crypto::base64encode(Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey))
            );

            return prepared;
        });

        {
            std::lock_guard<std::mutex> l{_mut};
            _preparingFile = std::move(calculator);
            _progressPrep(*_preparingFile, _totalCount - _sha1Count, _sha1Bytes);
        }
        std::shared_ptr<PreparedFile> prepared = task.get();
        _toUpload.enqueue(giga::make_unique<PreparedFile>(std::move(*prepared)));
        {
            std::lock_guard<std::mutex> l{_mut};
            _sha1Count += 1;
            _sha1Bytes += _preparingFile->progress().transfered;
        }
    }
    else
    {
        auto name = currentPath.filename().native();
        const auto& children = parent.getChildren();
        auto it = std::find_if(children.begin(), children.end(), [&name](const std::unique_ptr<Node>& e){
            return e->type() == Node::Type::folder && e->name() == name;
        });

        FolderNode node;
        if (it != children.end())
        {
            node = *static_cast<FolderNode*>(it->get());
        }
        else
        {
            node = parent.addChildFolder(name);
        }

        for (const auto& child: entry.children)
        {
            prepare(node, currentPath, child);
        }
    }
}

void
Uploader::uploadFile (const std::unique_ptr<PreparedFile>& element)
{
    // WARNING: uploader gets moved into _uploadingFile
    auto uploader = giga::make_unique<FileUploader>(element->path,
                                                    element->path.filename().native(),
                                                    element->parentId,
                                                    element->sha1,
                                                    element->fid,
                                                    element->fkeyEnc,
                                                    *_app,
                                                    _cts);
    {
        std::lock_guard<std::mutex> l(_mut);
        _uploadingFile = std::move(uploader);
        _uploadingFile->limitRate(_rate);
        _progressUp(*_uploadingFile, _totalCount - _upCount, _upBytes);
    }
    auto node = _uploadingFile->task().get();
    {
        std::lock_guard<std::mutex> l(_mut);
        _upCount += 1;
        _upBytes += _uploadingFile->progress().transfered;
        _onNewNode(*_uploadingFile, node);
    }
}

}/* namespace core */
} /* namespace giga */


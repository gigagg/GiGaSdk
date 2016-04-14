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

struct FileTree final
{
public:
    enum class Type {
        file, directory
    };

public:
    explicit FileTree(utility::string_t filename, Type type):
        filename{std::move(filename)}, type{type}, children{} {}

    ~FileTree()                          = default;
    FileTree(FileTree&&)                 = default;
    FileTree(const FileTree&)            = default;
    FileTree& operator=(const FileTree&) = default;
    FileTree& operator=(FileTree&&)      = default;

public:
    utility::string_t filename;
    Type              type;
    std::vector<FileTree>   children;
};

struct ScannedNode
{
    explicit ScannedNode(FolderNode parent, boost::filesystem::path path, FileTree entry):
            parent{std::move(parent)}, path{std::move(path)}, entry{std::move(entry)}
    {}

    FolderNode              parent;
    boost::filesystem::path path;
    FileTree                entry;
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
    _cts{},
    _mainTask{},
    _isStarted{false},
    _mut{},
    _upProgressFct{[](FileTransferer&, TransferProgress){}},
    _sha1ProgressFct{[](FileTransferer&, TransferProgress){}},
    _onNewNode{[](FileTransferer&, std::shared_ptr<Node>){}},
    _isFinished{false},
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
    _upProgressFct = fct;
}

void
Uploader::setPreparationProgressFct (ProgressFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _sha1ProgressFct = fct;
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
    if (_cts.get_token().is_canceled())
    {
        _cts = pplx::cancellation_token_source{};
    }
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
            prepare(element->parent, element->path, element->entry);
        }
    });

    auto uploadTask = pplx::create_task([this]() {
        std::unique_ptr<PreparedFile> element = nullptr;
        for(_toUpload.wait_dequeue(element); element != nullptr; _toUpload.wait_dequeue(element))
        {
            uploadFile(element);
        }
        _isFinished = true;
    });

    auto progressTask = pplx::create_task([this]() {

        FileUploader* upSaved = nullptr;
        FileTransferer::Progress upProgress{0, 0};

        Sha1Calculator* sha1Saved = nullptr;
        FileTransferer::Progress sha1Progress{0, 0};

        while(!_isFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::lock_guard<std::mutex> l(_mut);
            if (_uploadingFile != nullptr && (upSaved != _uploadingFile.get() || upProgress != _uploadingFile->progress() || upSaved->state() != _uploadingFile->state()))
            {
                upProgress = _uploadingFile->progress();
                upSaved    = _uploadingFile.get();
                _upProgressFct(*_uploadingFile, _upProgress.getProgressAddByte(upProgress.transfered));
            }
            if (_preparingFile != nullptr && (sha1Saved != _preparingFile.get() || sha1Progress != _preparingFile->progress() || sha1Saved->state() != _preparingFile->state()))
            {
                sha1Progress = _preparingFile->progress();
                sha1Saved    = _preparingFile.get();
                _sha1ProgressFct(*_preparingFile, _sha1Progress.getProgressAddByte(sha1Progress.transfered));
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
        if (_preparingFile != nullptr)
        {
            _uploadingFile->cancel();
        }
        if (_uploadingFile != nullptr)
        {
            _uploadingFile->cancel();
        }
        join();
    }
}

void
Uploader::limitRate(uint64_t rate)
{
    std::lock_guard<std::mutex> l(_mut);
    _rate = rate;
    if (_uploadingFile != nullptr)
    {
        _uploadingFile->limitRate(rate);
    }
}

void
Uploader::pause()
{
    std::lock_guard<std::mutex> l(_mut);
    if (_uploadingFile != nullptr)
    {
        _uploadingFile->pause();
    }
}

void
Uploader::resume()
{
    std::lock_guard<std::mutex> l(_mut);
    if (_uploadingFile != nullptr)
    {
        _uploadingFile->resume();
    }
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
        if (_preparingFile != nullptr)
        {
            _preparingFile->cancel();
        }

        bool isUploading = false;
        if (_uploadingFile != nullptr &&
            _uploadingFile->state() != FileTransferer::State::canceled &&
            _uploadingFile->state() != FileTransferer::State::finished &&
            _uploadingFile->state() != FileTransferer::State::error)
        {
            isUploading = true;
        }

        bool isPreparing = false;
        if (_preparingFile != nullptr &&
            _preparingFile->state() != FileTransferer::State::canceled &&
            _preparingFile->state() != FileTransferer::State::finished &&
            _preparingFile->state() != FileTransferer::State::error)
        {
            isPreparing = true;
        }

        {
            std::lock_guard<std::mutex> l(_mut);
            _upProgress.bytesTransfered = isUploading ? _uploadingFile->progress().transfered : 0ul;
            _upProgress.bytesTotal      = isUploading ? _uploadingFile->progress().size : 0ul;
            _upProgress.fileDone        = 0ul;
            _upProgress.fileCount       = isUploading ? 1ul: 0ul;

            _sha1Progress.bytesTransfered = isPreparing ? _preparingFile->progress().transfered : 0ul;
            _sha1Progress.bytesTotal      = isPreparing ? _preparingFile->progress().size : 0ul;
            _sha1Progress.fileDone        = 0ul;
            _sha1Progress.fileCount       = isPreparing ? 1ul: 0ul;
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
    return _isStarted;
}

FileUploader*
Uploader::uploadingFile()
{
    return _uploadingFile.get();
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
Uploader::callProgressFct () const
{
    if (_uploadingFile != nullptr)
    {
        _upProgressFct(*_uploadingFile, _upProgress.getProgressAddByte(_uploadingFile->progress().transfered));
    }
    if (_preparingFile != nullptr)
    {
        _sha1ProgressFct(*_preparingFile, _sha1Progress.getProgressAddByte(_preparingFile->progress().transfered));
    }
}

void
Uploader::scanFiles(FileTree& parent, const boost::filesystem::path& ppath)
{
    if (parent.type == FileTree::Type::file)
    {
        uint64_t size = boost::filesystem::file_size(ppath);
        std::lock_guard<std::mutex> l{_mut};
        _upProgress.fileCount  += 1;
        _upProgress.bytesTotal += size;
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

    try
    {
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
            auto calculator = std::unique_ptr<Sha1Calculator>{new Sha1Calculator(currentPath)};
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
                _sha1ProgressFct(*_preparingFile, _sha1Progress);
            }
            std::shared_ptr<PreparedFile> prepared = task.get();
            _toUpload.enqueue(giga::make_unique<PreparedFile>(std::move(*prepared)));
            {
                std::lock_guard<std::mutex> l{_mut};
                _sha1Progress.bytesTransfered += _preparingFile->progress().transfered;
                _sha1Progress.fileDone += 1;
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
    catch (...)
    {
        if (_preparingFile == nullptr || _preparingFile->state() != FileTransferer::State::canceled)
        {
            GIGA_DEBUG_LOG(boost::current_exception_diagnostic_information());
            _errors.enqueue(std::make_pair(currentPath, boost::current_exception_diagnostic_information()));
        }
        if (_preparingFile != nullptr && entry.type == FileTree::Type::file)
        {
            _sha1Progress.bytesTransfered += _preparingFile->progress().size;
            _sha1Progress.fileDone += 1;
        }
    }
}

void
Uploader::uploadFile (const std::unique_ptr<PreparedFile>& element)
{
    try
    {
        // WARNING: uploader gets moved into _uploadingFile
        auto uploader = giga::make_unique<FileUploader>(element->path,
                                                        element->path.filename().native(),
                                                        element->parentId,
                                                        element->sha1,
                                                        element->fid,
                                                        element->fkeyEnc,
                                                        *_app);
        {
            std::lock_guard<std::mutex> l(_mut);
            _uploadingFile = std::move(uploader);
            _uploadingFile->limitRate(_rate);
            _uploadingFile->start();
            _upProgressFct(*_uploadingFile, _upProgress);
        }
        auto node = _uploadingFile->task().get();
        {
            std::lock_guard<std::mutex> l(_mut);
            _onNewNode(*_uploadingFile, node);  // I must do the onNewNode() first, in case it throw an exception
            _upProgress.bytesTransfered += _uploadingFile->progress().transfered;
            _upProgress.fileDone += 1;
        }
    }
    catch (...)
    {
        if (_uploadingFile->state() != FileTransferer::State::canceled)
        {
            GIGA_DEBUG_LOG(boost::current_exception_diagnostic_information());
            _errors.enqueue(std::make_pair(element->path, boost::current_exception_diagnostic_information()));
        }
        std::lock_guard<std::mutex> l(_mut);
        _upProgress.bytesTransfered += _uploadingFile->progress().size;
        _upProgress.fileDone += 1;
    }

}

}/* namespace core */
} /* namespace giga */

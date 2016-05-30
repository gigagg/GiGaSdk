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
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "../Application.h"
#include "../utils/Crypto.h"
#include "../utils/Utils.h"
using boost::filesystem::path;
using moodycamel::BlockingReaderWriterQueue;
using utility::details::make_unique;

namespace giga
{
namespace core
{

struct UploadRequest
{
    explicit UploadRequest(FolderNode dest, std::vector<boost::filesystem::path> pathes):
                    dest{std::move(dest)}, pathes{std::move(pathes)}
    {}
    FolderNode                           dest;
    std::vector<boost::filesystem::path> pathes;
};

Uploader::Uploader(const Application& app):
    _requests{},
    _scanned{},
    _prepared{},
    _clearRequestQueue{0},
    _clearScannedQueue{0},
    _clearPreparedQueue{0},
    _scanningFile{nullptr},
    _preparingFile{nullptr},
    _uploadingFile{nullptr},
    _cts{},
    _mainTask{},
    _isStarted{false},
    _mut{},
    _mutQueue{},
    _upProgressFct{[](FileTransferer&, TransferProgress){}},
    _sha1ProgressFct{[](FileTransferer&, TransferProgress){}},
    _onScannedFct{[](const ScannedFile&){}},
    _onPreparedFct{[](const PreparedFile&){}},
    _onUploadedFct{[](const boost::filesystem::path&, std::shared_ptr<Node>){}},
    _onErrorFct{[](const boost::filesystem::path&, std::string&&, Step){}},
    _isFinished{false},
    _app(&app),
    _rate{0},
    _isPaused{false}
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
            GIGA_DEBUG_LOG(warning, utils::exceptionInfos());
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
Uploader::setOnScannedFct(OnScannedFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _onScannedFct = fct;
}

void
Uploader::setOnPreparedFct(OnPreparedFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _onPreparedFct = fct;
}

void
Uploader::setOnUploadedFct(OnUploadedFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _onUploadedFct = fct;
}

void
Uploader::setOnErrorFct(OnErrorFct fct)
{
    std::lock_guard<std::mutex> l(_mut);
    _onErrorFct = fct;
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
    _requests.enqueue(giga::make_unique<UploadRequest>(std::move(parent), std::move(pathes)));
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
        for(_requests.wait_dequeue(element); element != nullptr || _clearRequestQueue > 0; _requests.wait_dequeue(element))
        {
            if (_clearRequestQueue > 0)
            {
                if (element == nullptr)
                {
                    _clearRequestQueue -= 1;
                    enqueue<ScannedFile>(_scanned, nullptr);
                }
                continue;
            }
            {
                std::lock_guard<std::mutex> l(_mut);
                _scanningFile = std::move(element);
            }
            for (const auto& path : _scanningFile->pathes)
            {
                try
                {
                    scanFiles(_scanningFile->dest, path);
                }
                catch (...)
                {
                    auto info = utils::exceptionInfos();
                    GIGA_DEBUG_LOG(debug, info);

                    std::lock_guard<std::mutex> l(_mut);
                    _onErrorFct(path, std::move(info), Step::scanning);
                }

                // notify that the scan of path is complete
                ScannedFile sf{_scanningFile->dest.id(), path, 0ul};
                std::lock_guard<std::mutex> l{_mut};
                _onScannedFct(sf);
            }
        }
        enqueue<ScannedFile>(_scanned, nullptr);
    });

    auto prepareTask = pplx::create_task([this]() {
        std::unique_ptr<ScannedFile> element = nullptr;
        for(_scanned.wait_dequeue(element); element != nullptr || _clearScannedQueue > 0; _scanned.wait_dequeue(element))
        {
            if (_clearScannedQueue > 0)
            {
                if (element == nullptr)
                {
                    _clearScannedQueue -= 1;
                    enqueue<PreparedFile>(_prepared, nullptr);
                }
                continue;
            }

            prepare(*element);
        }
        enqueue<PreparedFile>(_prepared, nullptr);
    });

    auto uploadTask = pplx::create_task([this]() {
        std::unique_ptr<PreparedFile> element = nullptr;
        for(_prepared.wait_dequeue(element); element != nullptr || _clearPreparedQueue > 0; _prepared.wait_dequeue(element))
        {
            if (_clearPreparedQueue > 0)
            {
                if (element == nullptr)
                {
                    _clearPreparedQueue -= 1;
                }
                continue;
            }

            uploadFile(*element);
        }
        _isFinished = true;
    });

    auto progressTask = pplx::create_task([this]() {

//        FileUploader* upSaved = nullptr;
        FileTransferer::Progress upProgress{0, 0};

        Sha1Calculator* sha1Saved = nullptr;
        FileTransferer::Progress sha1Progress{0, 0};

        while(!_isFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            {
                std::lock_guard<std::mutex> l(_mut);
                if (_uploadingFile != nullptr) // && (upSaved != _uploadingFile.get() || upProgress != _uploadingFile->progress() || upSaved->state() != _uploadingFile->state()))
                {
                    upProgress = _uploadingFile->progress();
//                    upSaved    = _uploadingFile.get();
                    _upProgressFct(*_uploadingFile, _upProgress.getProgressAddByte(upProgress.transfered));
                }
                if (_preparingFile != nullptr && (sha1Saved != _preparingFile.get() || sha1Progress != _preparingFile->progress() || sha1Saved->state() != _preparingFile->state()))
                {
                    sha1Progress = _preparingFile->progress();
                    sha1Saved    = _preparingFile.get();
                    _sha1ProgressFct(*_preparingFile, _sha1Progress.getProgressAddByte(sha1Progress.transfered));
                }
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
        _requests.enqueue(nullptr);
        _mainTask.wait();
        _isStarted = false;
    }
}

void
Uploader::kill()
{
    if (_isStarted)
    {
        _cts.cancel();
        if (_preparingFile != nullptr)
        {
            _preparingFile->cancel();
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
    _isPaused = true;
}

void
Uploader::resume()
{
    std::lock_guard<std::mutex> l(_mut);
    if (_uploadingFile != nullptr)
    {
        _uploadingFile->resume();
    }
    _isPaused = false;
}

void
Uploader::clear()
{
    if (_isStarted)
    {
        _clearPreparedQueue += 1;
        _clearScannedQueue  += 1;
        _clearRequestQueue  += 1;

        {
            std::lock_guard<std::mutex> l(_mut);
            if (_preparingFile != nullptr)
            {
                _preparingFile->cancel();
            }
            _requests.enqueue(nullptr);

            auto ustate = _uploadingFile != nullptr ? _uploadingFile->state() : FileTransferer::State::canceled;
            auto isUploading = ustate == FileTransferer::State::started || ustate == FileTransferer::State::paused;

            auto pstate = _preparingFile != nullptr ? _preparingFile->state() : FileTransferer::State::canceled;
            auto isPreparing = pstate == FileTransferer::State::started || pstate == FileTransferer::State::paused;

            _upProgress.bytesTransfered = isUploading ? _uploadingFile->progress().transfered : 0ul;
            _upProgress.bytesTotal      = isUploading ? _uploadingFile->progress().size : 0ul;
            _upProgress.fileDone        = 0ul;
            _upProgress.fileCount       = isUploading ? 1ul: 0ul;

            _sha1Progress.bytesTransfered = isPreparing ? _preparingFile->progress().transfered : 0ul;
            _sha1Progress.bytesTotal      = isPreparing ? _preparingFile->progress().size : 0ul;
            _sha1Progress.fileDone        = 0ul;
            _sha1Progress.fileCount       = isPreparing ? 1ul: 0ul;

            if (_preparingFile != nullptr)
            {
                _sha1ProgressFct(*_preparingFile, _sha1Progress);
            }
            if (_uploadingFile != nullptr)
            {
                _upProgressFct(*_uploadingFile, _upProgress);
            }
        }
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

void
Uploader::callProgressFct () const
{
    std::lock_guard<std::mutex> l{_mut};
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
Uploader::addScannedFile (const std::string parentId, const boost::filesystem::path& path)
{
    auto size = boost::filesystem::file_size(path);
    {
        std::lock_guard<std::mutex> l{_mut};
        _sha1Progress.fileCount  += 1;
        _sha1Progress.bytesTotal += size;
    }
    enqueue<ScannedFile>(_scanned, giga::make_unique<ScannedFile>(parentId, path, size));
}

void
Uploader::addPreparedFile (const std::string parentId, const boost::filesystem::path& path, const std::string& sha1)
{
    auto decodedNodeKey = Crypto::base64decode(_app->currentUser().personalData().nodeKeyClear());
    auto fkey = Crypto::calculateFkey(sha1);
    auto prepared = giga::make_unique<PreparedFile>(
        parentId,
        path,
        sha1,
        fkey,
        Crypto::calculateFid(sha1),
        Crypto::base64encode(Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey))
    );

    auto size = boost::filesystem::file_size(path);
    {
        std::lock_guard<std::mutex> l{_mut};
        _upProgress.fileCount    += 1;
        _upProgress.bytesTotal   += size;
    }
    enqueue<PreparedFile>(_prepared, std::move(prepared));
}

void
Uploader::scanFiles(FolderNode& dest, const boost::filesystem::path& ppath)
{
    if (dest.type() == Node::Type::file)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("dest should be FolderNode")});
    }
    if (_clearRequestQueue > 0)
    {
        return;
    }

    auto isDirectory = is_directory(ppath);
    auto name        = utils::replaceInvalidUtf8(ppath.filename().native());
    if (!exists (ppath) || (!isDirectory && !is_regular_file(ppath)))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("dest should be a regular file")});
    }
    if (name == U("") || name[0] == U('.'))
    {
        return;
    }

    if (!isDirectory)
    {
        uint64_t size  = boost::filesystem::file_size(ppath);
        if (size == 0ul)
        {
            return;
        }
        auto scannedFile = giga::make_unique<ScannedFile>(dest.id(), ppath, size);
        {
            std::lock_guard<std::mutex> l{_mut};
            _sha1Progress.fileCount  += 1;
            _sha1Progress.bytesTotal += size;
            _onScannedFct(*scannedFile);
        }
        enqueue(_scanned, std::move(scannedFile));
        return;
    }
    else
    {
        const auto& children = dest.getChildren();
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
            node = dest.addChildFolder(name);
            std::lock_guard<std::mutex> l(_mut);
            _onUploadedFct(ppath, std::shared_ptr<Node>(new FolderNode(node)));
        }

        using namespace boost::filesystem;
        auto rng = boost::make_iterator_range(directory_iterator(ppath), directory_iterator());
        std::vector<directory_entry> entries;
        std::copy(directory_iterator(ppath), directory_iterator(), std::back_inserter(entries));
        std::sort(entries.begin(), entries.end());

        for (auto& entry : entries)
        {
            if (_cts.get_token().is_canceled())
            {
                return;
            }
            scanFiles(node, entry.path());
        }
    }
}

void
Uploader::prepare (const ScannedFile& scanned)
{
    const auto& path     = scanned.path;
    const auto& parentId = scanned.parentId;

    using namespace boost::filesystem;
    try
    {
        if (_cts.get_token().is_canceled())
        {
            return;
        }
        if (!exists (path) || !is_regular_file(path))
        {
            BOOST_THROW_EXCEPTION(ErrorException{U("Not a file")});
        }

        auto nodeName = path.filename().native();
        auto decodedNodeKey = Crypto::base64decode(_app->currentUser().personalData().nodeKeyClear());

        // WARNING: uploader gets moved into _preparingFile
        auto calculator = std::unique_ptr<Sha1Calculator>{new Sha1Calculator(path)};
        calculator->start();

        auto task = calculator->task().then([=](std::string sha1) {
            auto fkey = Crypto::calculateFkey(sha1);
            auto prepared = std::make_shared<PreparedFile>(
                parentId,
                path,
                sha1,
                fkey,
                Crypto::calculateFid(sha1),
                Crypto::base64encode(Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey))
            );

            return prepared;
        });

        {
            std::lock_guard<std::mutex> l{_mut};
            _upProgress.fileCount    += 1;
            _upProgress.bytesTotal   += scanned.size;
            _preparingFile = std::move(calculator);
            _sha1ProgressFct(*_preparingFile, _sha1Progress);
        }
        std::shared_ptr<PreparedFile> prepared = task.get();
        _onPreparedFct(*prepared);
        enqueue(_prepared, giga::make_unique<PreparedFile>(std::move(*prepared)));
        {
            std::lock_guard<std::mutex> l{_mut};
            _sha1Progress.bytesTransfered += _preparingFile->progress().transfered;
            _sha1Progress.fileDone += 1;
        }
    }
    catch (...)
    {
        auto info =  utils::exceptionInfos();
        if (_uploadingFile != nullptr && _uploadingFile->state() == FileTransferer::State::canceled)
        {
            info = "canceled";
        }
        GIGA_DEBUG_LOG(debug, info);

        std::lock_guard<std::mutex> l{_mut};
        _onErrorFct(path, std::move(info), Step::preparing);
        if (_preparingFile != nullptr)
        {
            _sha1Progress.bytesTransfered += _preparingFile->progress().size;
            _sha1Progress.fileDone += 1;
        }
    }
}

bool
Uploader::isPaused () const
{
    std::lock_guard<std::mutex> l(_mut);
    return _isPaused;
}


void
Uploader::uploadFile (const PreparedFile& prepared)
{
    try
    {
        // WARNING: uploader gets moved into _uploadingFile
        auto uploader = giga::make_unique<FileUploader>(prepared.path,
                                                        utils::replaceInvalidUtf8(prepared.path.filename().native()),
                                                        prepared.parentId,
                                                        prepared.sha1,
                                                        prepared.fid,
                                                        prepared.fkeyEnc,
                                                        *_app);
        {
            std::lock_guard<std::mutex> l(_mut);
            _uploadingFile = std::move(uploader);
            _uploadingFile->limitRate(_rate);
            _uploadingFile->start();
            if (_isPaused)
            {
                _uploadingFile->pause();
            }
            _upProgressFct(*_uploadingFile, _upProgress);
        }
        while (isPaused())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        auto node = _uploadingFile->task().get();
        {
            std::lock_guard<std::mutex> l(_mut);
            _onUploadedFct(prepared.path, node);  // I must do the _onUploadedFct() first, in case it throw an exception
            _upProgress.bytesTransfered += _uploadingFile->progress().transfered;
            _upProgress.fileDone += 1;
        }
    }
    catch (...)
    {
        auto info = utils::exceptionInfos();
        if (_uploadingFile != nullptr && _uploadingFile->state() == FileTransferer::State::canceled)
        {
            info = "canceled";
        }
        GIGA_DEBUG_LOG(debug, info);

        std::lock_guard<std::mutex> l(_mut);
        _onErrorFct(prepared.path, std::move(info), Step::uploading);
        _upProgress.bytesTransfered += _uploadingFile->progress().size;
        _upProgress.fileDone += 1;
    }

}

}/* namespace core */
} /* namespace giga */

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

#include "Downloader.h"
#include "Node.h"
#include "FolderNode.h"
#include "../Application.h"
#include "../utils/Utils.h"
#include "../rest/HttpErrors.h"
#include "../utils/make_unique.h"

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <string>
#include <memory>
#include <mutex>
#include <thread>

using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::create_directory;

namespace giga
{
namespace core
{

Downloader::Downloader (const Application& app) :
        _queue{},
        _mainTask{},
        _isStarted{false},
        _isFinished{false},
        _mut{},
        _downloading{},
        _progress{},
        _progressCallback{[](giga::core::FileTransferer&, TransferProgress){}},
        _onDownloadedFct{[](const Node&, const boost::filesystem::path&){}},
        _onFileDownloadedFct{[](const Node&, const boost::filesystem::path&, FileDownloader::Action){}},
        _onErrorFct{[](const std::string& /*id*/, const utility::string_t& /*name*/, std::string&&){}},
        _rate{0},
        _isPaused{false},
        _clearing{0},
        _app{&app},
        _cts{}
{
}

Downloader::~Downloader ()
{
    if (_isStarted)
    {
        try
        {
            join();
        }
        catch (...)
        {
            GIGA_DEBUG_LOG_U(warning, _app->currentUser().id(), utils::exceptionInfos());
        }
    }
}

void
Downloader::setDownloadProgressFct(ProgressFct fct)
{
    std::lock_guard<std::mutex> l{_mut};
    _progressCallback = fct;
}

void
Downloader::setOnDownloadedFct(OnDownloadedFct fct)
{
    std::lock_guard<std::mutex> l{_mut};
    _onDownloadedFct = fct;
}

void
Downloader::setOnFileDownloadedFct(OnFileDownloadedFct fct)
{
    std::lock_guard<std::mutex> l{_mut};
    _onFileDownloadedFct = fct;
}

void
Downloader::setOnErrorFct(OnErrorFct fct)
{
    std::lock_guard<std::mutex> l{_mut};
    _onErrorFct = fct;
}

std::shared_ptr<FileDownloader>
Downloader::downloadingFile ()
{
    std::lock_guard<std::mutex> l{_mut};
    return _downloading;
}

void
Downloader::start ()
{
    _isStarted = true;
    auto dlTask = pplx::create_task([this]() {
        std::unique_ptr<QueueElement> element = nullptr;
        for(_queue.wait_dequeue(element); element != nullptr || _clearing > 0; _queue.wait_dequeue(element))
        {
            try
            {
                if (_clearing > 0)
                {
                    if (element == nullptr)
                    {
                        _clearing -= 1;
                    }
                    continue;
                }


                downloadNode(*element->first, element->second);
                std::lock_guard<std::mutex> l{_mut};
                _onDownloadedFct(*element->first, element->second);
            }
            catch (...)
            {

                auto error = utils::exceptionInfos();
                GIGA_DEBUG_LOG(debug, error);
                try
                {
                    std::lock_guard<std::mutex> l{_mut};
                    _onErrorFct(element->first->id(), element->first->name(), std::move(error));
                }
                catch (...)
                {
                    GIGA_DEBUG_LOG(warning, utils::exceptionInfos());
                }
            }
        }
        _isFinished = true;
    }, _cts.get_token());

    auto progressTask = pplx::create_task([this]() {
        while(!_isFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::lock_guard<std::mutex> l(_mut);
            if (_downloading != nullptr)
            {
                try
                {
                    _progressCallback(*_downloading, _progress.getProgressAddByte(_downloading->progress().transfered));
                }
                catch (...)
                {
                    auto error = utils::exceptionInfos();
                    GIGA_DEBUG_LOG(warning, error);
                }
            }
        }
    }, _cts.get_token());

    auto tasks = {dlTask, progressTask};
    _mainTask =  pplx::when_all(tasks.begin(), tasks.end());
}

void
Downloader::join()
{
    if (_isStarted)
    {
        _queue.enqueue(nullptr);
        _mainTask.wait();
        _isStarted = false;
    }
}

void
Downloader::kill()
{
    if (_isStarted)
    {
        // clear
        std::unique_ptr<QueueElement> element = nullptr;
        while(_queue.try_dequeue(element)){}

        // cancel
        _queue.enqueue(nullptr);
        _cts.cancel();

        // wait for it
        join();
    }
}

void
Downloader::limitRate(uint64_t rate)
{
    _rate = rate;
    if (_downloading != nullptr)
    {
        _downloading->limitRate(rate);
    }
}

void
Downloader::pause()
{
    std::lock_guard<std::mutex> l(_mut);
    if (_downloading != nullptr)
    {
        _downloading->pause();
    }
    _isPaused = true;
}

void
Downloader::resume()
{
    std::lock_guard<std::mutex> l(_mut);
    if (_downloading != nullptr)
    {
        _downloading->resume();
    }
    _isPaused = false;
}

void
Downloader::clear ()
{
    if (_isStarted)
    {
        _clearing += 1;

        {
            std::lock_guard<std::mutex> l(_mut);
            _queue.enqueue(nullptr);

            auto dstate = _downloading != nullptr ? _downloading->state() : FileTransferer::State::canceled;
            auto isUploading = dstate == FileTransferer::State::started || dstate == FileTransferer::State::paused;

            _progress.bytesTransfered = isUploading ? _downloading->progress().transfered : 0ul;
            _progress.bytesTotal      = isUploading ? _downloading->progress().size : 0ul;
            _progress.fileDone        = 0ul;
            _progress.fileCount       = isUploading ? 1ul: 0ul;

            if (_downloading != nullptr)
            {
                _progressCallback(*_downloading, _progress);
            }
        }
    }
}

void
Downloader::callProgressFct () const
{
    if (_downloading != nullptr)
    {
        _progressCallback(*_downloading, _progress.getProgressAddByte(_downloading->progress().transfered));
    }
}

FileTransferer::State
Downloader::state()
{
    if (_downloading != nullptr)
    {
        return _downloading->state();
    }
    return FileTransferer::State::pending;
}

bool
Downloader::isStarted() const
{
    return _isStarted;
}

void
Downloader::addDownload (std::unique_ptr<Node>&& node, const boost::filesystem::path& path)
{
    auto size = node->size();
    auto count = node->type() == Node::Type::file ? 1ul : node->nbFiles();
    _queue.enqueue(giga::make_unique<QueueElement>(std::make_pair(std::move(node), path)));
    {
        std::lock_guard<std::mutex> l{_mut};
        _progress.fileCount  += count;
        _progress.bytesTotal += size;
    }
}


void
Downloader::downloadNode (Node& node, const boost::filesystem::path& path)
{
    TransferProgress saved{};
    {
        std::lock_guard<std::mutex> l{_mut};
        saved = _progress;
    }

    try
    {
        doDownloadNode(node, path);
    }
    catch (const std::exception&)
    {
        if (_downloading != nullptr && _downloading->state() == FileTransferer::State::canceled)
        {
            // Ignore cancel errors
            // TODO: this should be reported.
            std::lock_guard<std::mutex> l{_mut};
            _downloading = nullptr;
            return;
        }

        auto error = utils::exceptionInfos();
        GIGA_DEBUG_LOG(debug, error);
        try
        {
            saved.fileDone += node.type() == Node::Type::file ? 1 : node.nbFiles();
            saved.bytesTransfered += node.size();

            std::lock_guard<std::mutex> l{_mut};
            _progress = saved;
            _onErrorFct(node.id(), node.name(), std::move(error));
            _downloading = nullptr;
        }
        catch (...)
        {
            GIGA_DEBUG_LOG(warning, utils::exceptionInfos());
        }
    }
}

void
Downloader::doDownloadNode (Node& node, const boost::filesystem::path& path)
{
    if (!is_directory(path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("path should be a directory")});
    }
    if (_cts.get_token().is_canceled())
    {
        return;
    }
    if (_clearing > 0)
    {
        return;
    }

    auto name = utils::cleanUpFilename(node.name());
    auto npath = path / name;
    auto npathExists = exists(npath);
    if (npathExists)
    {
        auto isDir = is_directory(npath);
        if (node.type() == Node::Type::file)
        {
            if (isDir)
            {
                BOOST_THROW_EXCEPTION(ErrorException{U("Cannot download file: a directory with the same name already exists.")});
            }
        }
        if (node.type() != Node::Type::file && !isDir)
        {
            BOOST_THROW_EXCEPTION(ErrorException{U("Cannot create directory: a file with the same name already exists.")});
        }
    }

    if (node.type() == Node::Type::file)
    {
        auto fdownloader = std::make_shared<FileDownloader>(path.native(), node, *_app, pplx::cancellation_token_source{}, FileDownloader::Policy::overrideNewerSize);
        {
            std::lock_guard<std::mutex> l{_mut};
            fdownloader->limitRate(_rate);
            fdownloader->start();
            if (_isPaused)
            {
                fdownloader->pause();
            }
        }
        auto task = fdownloader->task();
        {
            std::lock_guard<std::mutex> l{_mut};
            _downloading = std::move(fdownloader);
            _progressCallback(*_downloading, _progress);
        }
        auto result = task.get();
        {
            std::lock_guard<std::mutex> l{_mut};
            _progress.fileDone += 1;
            _progress.bytesTransfered += node.size();
            _onFileDownloadedFct(node, result.path, result.action);
        }
    }
    else if (!npathExists)
    {
        create_directory(npath);
    }

    // recursion
    for(auto& child : node.getChildren())
    {
        downloadNode(*child, npath);
    }
}

} /* namespace core */
} /* namespace giga */

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
#include <boost/exception/diagnostic_information.hpp>
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
        _onErrorFct{[](const std::string& /*id*/, std::string&&){}},
        _rate{0},
        _isPaused{false},
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
            GIGA_DEBUG_LOG(boost::current_exception_diagnostic_information());
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
    auto dlTask = pplx::create_task([this]() {
        std::unique_ptr<QueueElement> element = nullptr;
        for(_queue.wait_dequeue(element); element != nullptr; _queue.wait_dequeue(element))
        {
            try
            {
                downloadFile(*element->first, element->second);
                std::lock_guard<std::mutex> l{_mut};
                _onDownloadedFct(*element->first, element->second);
            }
            catch (...)
            {
                auto error = boost::current_exception_diagnostic_information();
                GIGA_DEBUG_LOG(error);
                {
                    std::lock_guard<std::mutex> l{_mut};
                    _onErrorFct(element->first->id(), std::move(error));
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
                _progressCallback(*_downloading, _progress.getProgressAddByte(_downloading->progress().transfered));
            }
        }
    }, _cts.get_token());

    auto tasks = {dlTask, progressTask};
    _mainTask =  pplx::when_all(tasks.begin(), tasks.end());
    _isStarted = true;
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
        std::unique_ptr<QueueElement> element = nullptr;
        while(_queue.try_dequeue(element)){}
        _queue.enqueue(nullptr);
        _cts.cancel();
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
        std::unique_ptr<QueueElement> element = nullptr;
        while(_queue.try_dequeue(element)){}
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
Downloader::downloadFile (Node& node, const boost::filesystem::path& path)
{
    if (!is_directory(path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("path should be a directory")});
    }
    if (_cts.get_token().is_canceled())
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
        auto fdownloader = std::make_shared<FileDownloader>(path.native(), node, *_app, _cts, FileDownloader::Policy::overrideNewerSize);
        {
            std::lock_guard<std::mutex> l{_mut};
            fdownloader->limitRate(_rate);
            fdownloader->start();
            if (_isPaused)
            {
                fdownloader->pause();
            }
        }
        try {
            auto task = fdownloader->task();
            {
                std::lock_guard<std::mutex> l{_mut};
                _downloading = std::move(fdownloader);
                _progressCallback(*_downloading, _progress);
            }
            task.wait();
            {
                std::lock_guard<std::mutex> l{_mut};
                _progress.fileDone += 1;
                _progress.bytesTransfered += node.size();
            }
        }
        catch (...)
        {
            auto error = boost::current_exception_diagnostic_information();
            GIGA_DEBUG_LOG(error);
            {
                std::lock_guard<std::mutex> l{_mut};
                fdownloader->setError(error);
                _onErrorFct(node.id(), std::move(error));
            }
        }
    }
    else if (!npathExists)
    {
        create_directory(npath);
    }

    // recursion
    for(auto& child : node.getChildren())
    {
        downloadFile(*child, npath);
    }
}

} /* namespace core */
} /* namespace giga */

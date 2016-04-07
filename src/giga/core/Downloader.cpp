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
        _isStarted{false},
        _downloading{},
        _nbFiles{0},
        _dlCount{0},
        _dlBytes{0},
        _mainTask{},
        _isFinished{false},
        _mut{},
        _progressCallback{[](giga::core::FileTransferer&, uint64_t, uint64_t){}},
        _app{&app},
        _rate{0}
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
            downloadFile(*element->first, element->second);
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
                _progressCallback(*_downloading, _nbFiles - _dlCount, _dlBytes + _downloading->progress().transfered);
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
    if (_downloading != nullptr)
    {
        _downloading->pause();
    }
}

void
Downloader::resume()
{
    if (_downloading != nullptr)
    {
        _downloading->resume();
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
    std::lock_guard<std::mutex> l(_mut);
    return _isStarted;
}

void
Downloader::addDownload (std::unique_ptr<Node>&& node, const boost::filesystem::path& path)
{
    _queue.enqueue(giga::make_unique<QueueElement>(std::make_pair(std::move(node), path)));
    {
        std::lock_guard<std::mutex> l{_mut};
        _nbFiles += node->nbFiles();
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
        fdownloader->start();
        fdownloader->limitRate(_rate);
        try {
            auto task = fdownloader->task();
            {
                std::lock_guard<std::mutex> l{_mut};
                _downloading = std::move(fdownloader);
                ++_dlCount;
                _progressCallback(*_downloading, _nbFiles - _dlCount, _dlBytes);
            }
            task.wait();
            {
                std::lock_guard<std::mutex> l{_mut};
                _dlBytes += node.size();
            }
        }
        catch (...)
        {
            fdownloader->setError(boost::current_exception_diagnostic_information());
            _progressCallback(*_downloading, _nbFiles - _dlCount, _dlBytes);
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

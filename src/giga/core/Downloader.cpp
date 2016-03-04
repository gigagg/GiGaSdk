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
#include "../utils/Utils.h"
#include "../rest/HttpErrors.h"

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

Downloader::Downloader (std::unique_ptr<Node>&& node, const boost::filesystem::path& path, Downloader::ProgressCallback clb) :
        _node{std::move(node)},
        _path{path},
        _downloading{},
        _dlCount{0},
        _dlBytes{0},
        _mainTask{},
        _isFinished{false},
        _mut{},
        _progressCallback{clb}
{
}

Downloader::~Downloader ()
{
}

std::shared_ptr<FileDownloader>
Downloader::downloadingFile ()
{
    std::lock_guard<std::mutex> l{_mut};
    return _downloading;
}

uint64_t
Downloader::downloadingFileNumber ()
{
    std::lock_guard<std::mutex> l{_mut};
    return _dlCount;
}

pplx::task<void>
Downloader::start ()
{
    auto dlTask = pplx::create_task([this]() {
        downloadFile(*_node, _path);
        _isFinished = true;
    });

    auto progressTask = pplx::create_task([this]() {
        while(!_isFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::lock_guard<std::mutex> l(_mut);
            if (_downloading != nullptr)
            {
                _progressCallback(*_downloading, _dlCount, _dlBytes + _downloading->progress().transfered);
            }
        }
    });

    auto tasks = {dlTask, progressTask};
    _mainTask =  pplx::when_all(tasks.begin(), tasks.end());
    return _mainTask;
}

void
Downloader::downloadFile (Node& node, const boost::filesystem::path& path)
{
    if (!is_directory(path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("path should be a directory")});
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
        auto fdownloader = std::make_shared<FileDownloader>(path.native(), node, FileDownloader::Policy::overrideNewerSize);
        fdownloader->start();
        auto task = fdownloader->task();
        {
            std::lock_guard<std::mutex> l{_mut};
            _downloading = std::move(fdownloader);
            ++_dlCount;
            _progressCallback(*_downloading, _dlCount, _dlBytes);
        }
        task.wait();
        {
            std::lock_guard<std::mutex> l{_mut};
            _dlBytes += node.size();
        }
    }
    else if (!npathExists)
    {
        create_directory(npath);
    }

    // recursion
    if (node.shouldLoadChildren())
    {
        node.loadChildren();
    }
    for(auto& child : node.children())
    {
        downloadFile(*child, npath);
    }
}

} /* namespace core */
} /* namespace giga */

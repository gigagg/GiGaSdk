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

using boost::filesystem::path;
using moodycamel::BlockingReaderWriterQueue;
using utility::details::make_unique;

namespace giga
{
namespace core
{

Uploader::Uploader(const Application& app, ProgressUpload clbUp, ProgressPreparation clbPrep):
    _preparing{nullptr},
    _uploading{pplx::create_task([]() -> std::shared_ptr<Node> {return nullptr;})},
    _cts{},
    _mainTask{},
    _isStarted{false},
    _mut{},
    _readyList{},
    _isPreparationFinished{false},
    _upBytes{0},
    _upCount{0},
    _isFinished{false},
    _progressUp{clbUp},
    _progressPrep{clbPrep},
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
Uploader::addUpload (FolderNode parent, boost::filesystem::path&& path)
{
    std::vector<boost::filesystem::path> vect;
    vect.emplace_back(std::move(path));
    addUploads(std::move(parent), std::move(vect));
}

void
Uploader::addUploads(FolderNode parent, std::vector<boost::filesystem::path>&& pathes)
{
    _queue.enqueue(giga::make_unique<QueueElement>(std::make_pair(std::move(parent), std::move(pathes))));
}

bool
Uploader::isPreparationFinished() const
{
    std::lock_guard<std::mutex> l{_mut};
    return _isPreparationFinished;
}

std::vector<std::shared_ptr<FileUploader>>
Uploader::uploadingFiles()
{
    std::lock_guard<std::mutex> l{_mut};
    return _readyList;
}

std::vector<std::shared_ptr<FileUploader>>::size_type
Uploader::uploadingFilesCount()
{
    std::lock_guard<std::mutex> l{_mut};
    return _readyList.size();
}

void
Uploader::start()
{
    auto upTask = pplx::create_task([this]() {
        std::unique_ptr<QueueElement> element = nullptr;

        for(_queue.wait_dequeue(element); element != nullptr; _queue.wait_dequeue(element))
        {
            for (auto& path : element->second)
            {
                scanFilesAddUploads(element->first, path);
            }

            if (_preparing != nullptr)
            {
                auto fileUploader = _preparing->second.get();
                _uploading = _uploading.then(startNextUpload(fileUploader));
                {
                    std::lock_guard<std::mutex> l{_mut};
                    _readyList.emplace_back(fileUploader);
                }
                _preparing = nullptr;
            }
        }
        return _uploading;
    }, _cts.get_token()).then([this](pplx::task<std::shared_ptr<Node>> _uploading) {
        {
            std::lock_guard<std::mutex> l{_mut};
            _isPreparationFinished = true;
        }
        _uploading.get();
        _isFinished = true;
    }, _cts.get_token());

    auto progressTask = pplx::create_task([this]() {
        while(!_isFinished)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::lock_guard<std::mutex> l(_mut);
            auto result = std::find_if(_readyList.begin(), _readyList.end(), [](ReadyEntry entry){
                auto p = entry->progress();
                return p.size > p.transfered;
            });
            if (result != _readyList.end())
            {
                _progressUp(**result, _upCount, _upBytes + result->get()->progress().transfered);
            }
            if (_preparing != nullptr)
            {
                _progressPrep(*_preparing->first);
            }
        }
    });

    auto tasks = {upTask, progressTask};
    _mainTask =  pplx::when_all(tasks.begin(), tasks.end());
    _isStarted = true;
}

void
Uploader::join()
{
    if (_isStarted)
    {
        _queue.enqueue(nullptr);
        _mainTask.wait();
        _isStarted = false;
    }
}

void
Uploader::kill()
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
Uploader::limitRate(uint64_t rate)
{
    for(auto fu : _readyList)
    {
        fu->limitRate(rate);
    }
    _rate = rate;
}

void
Uploader::pause()
{
    for(auto fu : _readyList)
    {
        if (fu->state() == FileTransferer::State::started)
        {
            fu->pause();
        }
    }
}

void
Uploader::resume()
{
    for(auto fu : _readyList)
    {
        if (fu->state() == FileTransferer::State::paused)
        {
            fu->resume();
        }
    }
}

FileTransferer::State
Uploader::state()
{
    for(auto fu : _readyList)
    {
        if (fu->state() == FileTransferer::State::paused)
        {
            return FileTransferer::State::paused;
        }
        if (fu->state() == FileTransferer::State::started)
        {
            return FileTransferer::State::started;
        }
    }
    if (_readyList.empty())
    {
        return FileTransferer::State::pending;
    }
    return _readyList.back()->state();
}

void
Uploader::scanFilesAddUploads (FolderNode& parent, const boost::filesystem::path& path)
{
    using namespace boost::filesystem;
    if (!exists (path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("File or directory not found")});
    }
    if (_cts.get_token().is_canceled())
    {
        return;
    }

    if (is_regular_file(path))
    {
        bool preparing = false;
        {
            std::lock_guard<std::mutex> l{_mut};
            preparing = _preparing != nullptr;
        }
        if (!preparing)
        {
            auto preparing = parent.uploadFile(path.native(), _cts);
            {
                std::lock_guard<std::mutex> l{_mut};
                _preparing = std::move(preparing);
                _progressPrep(*_preparing->first);
            }
        }
        else
        {
            auto fileUploader = _preparing->second.get();
            _uploading = _uploading.then(startNextUpload(fileUploader), _cts.get_token());
            auto preparing = parent.uploadFile(path.native());

            {
                std::lock_guard<std::mutex> l{_mut};
                _readyList.emplace_back(std::move(fileUploader));
                _preparing = std::move(preparing);
                _progressPrep(*_preparing->first);
            }
        }
    }
    else if (is_directory(path))
    {
        auto name = path.filename().native();

        // ignore these folders...
        if (name == U("") || name == U(".") || name == U(".."))
        {
            return;
        }

        const auto& children = parent.getChildren();
        auto it = std::find_if(children.begin(), children.end(), [name](const std::unique_ptr<Node>& e){
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

        auto rng = boost::make_iterator_range(directory_iterator(path), directory_iterator());
        for (directory_entry& entry : rng)
        {
            scanFilesAddUploads(node, entry.path());
        }
    }
    else
    {
        // TODO report neither file neither dir
    }
}

void
callCallback (std::mutex& mut, uint64_t& upByte, uint64_t& upCount,
              std::shared_ptr<Node> n, Uploader::ProgressUpload clb,
              std::shared_ptr<giga::core::FileUploader> next)
{
    std::lock_guard<std::mutex> l{mut};
    if (n != nullptr) {
        upByte += n->size();
        clb(*next, ++upCount, upByte);
    }
    else
    {
        clb(*next, ++upCount, 0);
    }
}

std::function<std::shared_ptr<Node> (std::shared_ptr<Node> n)>
Uploader::startNextUpload(std::shared_ptr<giga::core::FileUploader> next)
{
    auto clb      = _progressUp;
    auto& upByte  = _upBytes;
    auto& upCount = _upCount;
    auto& mut     = _mut;
    return [next, clb, &upByte, &mut, &upCount] (std::shared_ptr<Node> n) -> std::shared_ptr<giga::core::Node> {
        try {
            next->start();
            callCallback(mut, upByte, upCount, n, clb, next);
            return next->task().get();
        }
        catch (...)
        {
            next->setError(boost::current_exception_diagnostic_information());
            callCallback(mut, upByte, upCount, n, clb, next);
            return nullptr;
        }
    };
}


}/* namespace core */
} /* namespace giga */


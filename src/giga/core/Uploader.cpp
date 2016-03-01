/*
 * FolderUploader.cpp
 *
 *  Created on: 19 févr. 2016
 *      Author: thomas
 */

#include "Uploader.h"

#include "../rest/HttpErrors.h"

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

using boost::filesystem::path;

namespace giga
{
namespace core
{

Uploader::Uploader(FolderNode parent, const path& path, ProgressCallback clb):
    _parent{std::move(parent)},
    _path{path},
    _preparingList{},
    _uploading{pplx::create_task([]() -> std::shared_ptr<Node> {return nullptr;})},
    _mainTask{},
    _isStarted{false},
    _mut{},
    _readyList{},
    _isPreparationFinished{false},
    _upBytes{0},
    _upCount{0},
    _isFinished{false},
    _progressCallback{clb}
{
}

Uploader::~Uploader()
{
    if (_isStarted)
    {
        _mainTask.wait();
    }
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

pplx::task<void>
Uploader::start()
{
    auto upTask = pplx::create_task([this]() {
        scanFilesAddUploads(_parent, _path);

        while (_preparingList.size() > 0)
        {
            auto ready = pplx::when_any(_preparingList.begin(), _preparingList.end()).get();
            _uploading = _uploading.then(startNextUpload(ready.first));
            {
                std::lock_guard<std::mutex> l{_mut};
                _readyList.emplace_back(std::move(ready.first));
            }
            _preparingList.erase(_preparingList.begin() + ready.second);
        }
        return _uploading;
    }).then([this](pplx::task<std::shared_ptr<Node>> _uploading) {
        {
            std::lock_guard<std::mutex> l{_mut};
            _isPreparationFinished = true;
        }
        _uploading.get();
        _isFinished = true;
    });

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
                _progressCallback(**result, _upCount, _upBytes + result->get()->progress().transfered);
            }
        }
    });

    auto tasks = {upTask, progressTask};
    _mainTask =  pplx::when_all(tasks.begin(), tasks.end());

    _isStarted = true;
    return _mainTask;
}

void
Uploader::scanFilesAddUploads (FolderNode& parent, const boost::filesystem::path& path)
{
    using namespace boost::filesystem;
    if (!exists (path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{"File or directory not found"});
    }
    if (parent.shouldLoadChildren())
    {
        parent.loadChildren();
    }

    if (is_regular_file(path))
    {
        if (_preparingList.size() < 4)
        {
            _preparingList.emplace_back(parent.uploadFile(path.string()));
        }
        else
        {
            auto clb = _progressCallback;
            auto ready = pplx::when_any(_preparingList.begin(), _preparingList.end()).get();
            _uploading = _uploading.then(startNextUpload(ready.first));
            _preparingList[ready.second] = parent.uploadFile(path.string());

            {
                std::lock_guard<std::mutex> l{_mut};
                _readyList.emplace_back(ready.first);
            }
        }
    }
    else if (is_directory(path))
    {
        auto name = path.filename().string();

        // ignore these folders...
        if (name == "" || name == "." || name == "..")
        {
            return;
        }

        const auto& children = parent.children();
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

std::function<std::shared_ptr<Node> (std::shared_ptr<Node> n)>
Uploader::startNextUpload(std::shared_ptr<giga::core::FileUploader> next)
{
    auto clb      = _progressCallback;
    auto& upByte  = _upBytes;
    auto& upCount = _upCount;
    auto& mut     = _mut;
    return [next, clb, &upByte, &mut, &upCount] (std::shared_ptr<Node> n) {
        next->start();
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
        return next->task().get();
    };
}


}/* namespace core */
} /* namespace giga */

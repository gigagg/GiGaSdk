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
#include <utility>
#include <vector>

using boost::filesystem::path;

namespace giga
{
namespace core
{

Uploader::Uploader(FolderNode parent, const std::string& path):
    _parent{std::move(parent)},
    _path{path},
    _preparingList{},
    _uploading{pplx::create_task([]() -> std::shared_ptr<Node> {return nullptr;})},
    _mainTask{},
    _isStarted{false},
    _mut{},
    _readyList{},
    _isPreparationFinished{false}
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
    _mainTask = pplx::create_task([this](){
        scanFilesAddUploads(_parent, _path);

        while (_preparingList.size() > 0)
        {
            auto ready = pplx::when_any(_preparingList.begin(), _preparingList.end()).get();
            auto next = ready.first;
            _uploading = _uploading.then([next] (std::shared_ptr<Node>) {
                next->start();
                return next->task();
            });
            {
                std::lock_guard<std::mutex> l{_mut};
                _readyList.emplace_back(std::move(ready.first));
            }
            _preparingList.erase(_preparingList.begin() + ready.second);;
        }
        return _uploading;
    }).then([this](pplx::task<std::shared_ptr<Node>> _uploading) {
        {
            std::lock_guard<std::mutex> l{_mut};
            _isPreparationFinished = true;
        }
        _uploading.get();
    });
    _isStarted = true;

    return _mainTask;
}

void
Uploader::scanFilesAddUploads (FolderNode parent, boost::filesystem::path path)
{
    using namespace boost::filesystem;
    if (!exists (path))
    {
        THROW(ErrorException{"File or directory not found"});
    }
    parent.loadChildren();

    if (is_regular_file(path))
    {
        if (_preparingList.size() < 4)
        {
            _preparingList.emplace_back(parent.uploadFile(path.string()));
        }
        else
        {
            std::vector<PreparingEntry>::iterator it;
            do {
                auto ready = pplx::when_any(_preparingList.begin(), _preparingList.end()).get();
                auto next = ready.first;
                _uploading = _uploading.then([next] (std::shared_ptr<Node>) {
                    next->start();
                    return next->task().get();
                });
                _preparingList[ready.second] = parent.uploadFile(path.string());

                {
                    std::lock_guard<std::mutex> l{_mut};
                    _readyList.emplace_back(ready.first);
                }

            } while (it == _preparingList.end());
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



}/* namespace core */
} /* namespace giga */

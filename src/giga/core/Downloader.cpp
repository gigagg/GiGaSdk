/*
 * Downloader.cpp
 *
 *  Created on: 23 févr. 2016
 *      Author: thomas
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

Downloader::Downloader (std::shared_ptr<Node> node, const std::string& path) :
        _node{node},
        _path{path},
        _downloading{},
        _dlCount{0},
        _mainTask{},
        _isStarted{false},
        _mut{}
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
    _isStarted = true;

    _mainTask = pplx::create_task([this]() {
        auto g = _node;
        downloadFile(*_node, path{_path});
    });

    return _mainTask;
}

void
Downloader::downloadFile (Node& node, boost::filesystem::path path)
{
    if (!is_directory(path))
    {
        THROW(ErrorException{"path should be a directory"});
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
                THROW(ErrorException{"Cannot download file: a directory with the same name already exists."});
            }
        }
        if (node.type() != Node::Type::file && !isDir)
        {
            THROW(ErrorException{"Cannot create directory: a file with the same name already exists."});
        }
    }

    if (node.type() == Node::Type::file)
    {
        auto fdownloader = node.download(path.string(), FileDownloader::Policy::overrideNewerSize);
        fdownloader.start();
        auto task = fdownloader.task();
        {
            std::lock_guard<std::mutex> l{_mut};
            _downloading = std::make_shared<FileDownloader>(std::move(fdownloader));
            ++_dlCount;
        }
        task.wait();
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

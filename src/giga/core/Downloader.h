/*
 * Downloader.h
 *
 *  Created on: 23 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_DOWNLOADER_H_
#define GIGA_CORE_DOWNLOADER_H_

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>

namespace giga
{
namespace core
{

class FileDownloader;
class Node;

class Downloader
{
public:
    explicit Downloader(std::shared_ptr<Node> node, const std::string& path);
    ~Downloader();

    std::shared_ptr<FileDownloader>
    downloadingFile();

    uint64_t
    downloadingFileNumber();

    pplx::task<void>
    start();

private:
    void
    downloadFile (Node& node, boost::filesystem::path path);

private:
    std::shared_ptr<Node>             _node;
    std::string                       _path;
    std::shared_ptr<FileDownloader>   _downloading;
    uint64_t                          _dlCount;

    pplx::task<void>                  _mainTask;
    bool                              _isStarted;

    mutable std::mutex                _mut;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_DOWNLOADER_H_ */

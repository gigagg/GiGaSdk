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

class Downloader final
{
public:
    typedef std::function<void(FileDownloader&, uint64_t count, uint64_t bytes)> ProgressCallback;

public:
    explicit Downloader(std::shared_ptr<Node> node, const std::string& path, ProgressCallback clb = [](FileDownloader&, uint64_t, uint64_t){});
    ~Downloader();

    Downloader(Downloader&&)                 = delete;
    Downloader& operator=(Downloader&&)      = delete;
    Downloader(const Downloader&)            = delete;
    Downloader& operator=(const Downloader&) = delete;

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
    std::shared_ptr<Node>           _node;
    std::string                     _path;
    std::shared_ptr<FileDownloader> _downloading;
    uint64_t                        _dlCount;
    uint64_t                        _dlBytes;
    pplx::task<void>                _mainTask;
    std::atomic<bool>               _isFinished;
    mutable std::mutex              _mut;
    ProgressCallback                _progressCallback;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_DOWNLOADER_H_ */

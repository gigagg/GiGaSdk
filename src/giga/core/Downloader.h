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
    explicit Downloader(std::unique_ptr<Node>&& node, const boost::filesystem::path& path, ProgressCallback clb = [](FileDownloader&, uint64_t, uint64_t){});
    ~Downloader();

    Downloader(Downloader&&)                 = delete;
    Downloader(const Downloader&)            = delete;
    Downloader& operator=(const Downloader&) = delete;
    Downloader& operator=(Downloader&&)      = delete;

    std::shared_ptr<FileDownloader>
    downloadingFile();

    uint64_t
    downloadingFileNumber();

    pplx::task<void>
    start();

private:
    void
    downloadFile (Node& node, const boost::filesystem::path& path);

private:
    std::unique_ptr<Node>           _node;
    boost::filesystem::path         _path;
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

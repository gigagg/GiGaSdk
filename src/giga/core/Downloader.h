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

#include "FileTransferer.h"
#include "../utils/readerwriterqueue.h"

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>

namespace giga
{
class Application;
namespace core
{

class FileDownloader;
class Node;

/**
 * Download folders and files.
 */
class Downloader final
{
public:
    typedef std::function<void(FileDownloader&, uint64_t count, uint64_t bytes)> ProgressCallback;

public:
    /**
     * @brief construct a Downloader
     * @param clb a callback function that will be executed periodically to let you know of the download progress.
     *
     * ```clb``` will be called at least once for each file being downloaded.
     */
    explicit Downloader(const Application& app, ProgressCallback clb = [](FileDownloader&, uint64_t, uint64_t){});
    ~Downloader();

    Downloader(Downloader&&)                 = delete;
    Downloader(const Downloader&)            = delete;
    Downloader& operator=(const Downloader&) = delete;
    Downloader& operator=(Downloader&&)      = delete;

public:
    /**
     * @brief add a file or folder to the list of download
     *
     * @param node move here the node you want to download.
     *             It can either be a ```FileNode``` or a ```FolderNode```.
     * @param path the path to a folder where you want ```node``` to be downloaded.
     */
    void
    addDownload(std::unique_ptr<Node>&& node, const boost::filesystem::path& path);

    /**
     * @brief Gets the current FileDownloader.
     *
     * For each file downloaded by the ```Downloader``` a ```FileDownloader``` is created.
     * This methods return the ```FileDownloader``` for the file being currently download.
     * It may return ```nullptr``` if there is no file downloading yet.
     * @return The current FileDownloader or nullptr.
     */
    std::shared_ptr<FileDownloader>
    downloadingFile();

    /**
     * @return The number of files downloaded + 1 if there is a file currently being downloaded.
     */
    uint64_t
    downloadingFileNumber();

    /**
     * @brief Start the uploading process
     */
    void
    start();

    /**
     * @brief Wit for the uploading process to finish
     */
    void
    join();

    /**
     * @brief Cancel the current download; Clear the queue; stop the process.
     */
    void
    kill();

    /**
     * @brief Limit the current download rate
     * @param rate the download rate in Octet/s. Uses 0 for no limit.
     */
    void
    limitRate(uint64_t rate);

    /**
     * @brief Pause the current download. Uses ```resume()``` to restart.
     */
    void
    pause();

    /**
     * @brief Resume a previously ```pause()```d download.
     */
    void
    resume();

    /**
     * @brief Gets the current download state
     */
    FileTransferer::State
    state();

private:
    void
    downloadFile (Node& node, const boost::filesystem::path& path);

private:
    typedef std::pair<std::unique_ptr<Node>, const boost::filesystem::path> QueueElement;
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<QueueElement>> Queue;

    Queue                           _queue;
    bool                            _isStarted;
    std::unique_ptr<Node>           _node;
    boost::filesystem::path         _path;
    std::shared_ptr<FileDownloader> _downloading;
    uint64_t                        _dlCount;
    uint64_t                        _dlBytes;
    pplx::task<void>                _mainTask;
    std::atomic<bool>               _isFinished;
    mutable std::mutex              _mut;
    ProgressCallback                _progressCallback;
    const Application*              _app;
    pplx::cancellation_token_source _cts;
    uint64_t                        _rate;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_DOWNLOADER_H_ */

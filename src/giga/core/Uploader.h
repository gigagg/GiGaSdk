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

#ifndef GIGA_CORE_UPLOADER_H_
#define GIGA_CORE_UPLOADER_H_

#include "FolderNode.h"
#include "../utils/readerwriterqueue.h"

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>
#include <utility>
#include <vector>

namespace giga
{
namespace core
{

/**
 * Upload files and folders.
 *
 * Start the upload process with the ```Uploader::start()``` method.
 * Uses ```Uploader::addUpload()``` to add file/folder to upload.
 * Terminate the uploading process using the ```Uploader::join()``` method.
 */
class Uploader
{
public:
    typedef std::function<void(FileUploader&, uint64_t count, uint64_t bytes)> ProgressUpload;
    typedef std::function<void(Sha1Calculator&)> ProgressPreparation;

public:
    /**
     * @brief construct an Uploader
     *
     * @param clbUp a callback function that will be called periodically to let you know of the upload progress
     * @param clbPrep a callback function that will be called periodically to let you know of the preparation progress (sha1)
     *
     * ```clb``` will be called at least once for each file being uploaded.
     */
    explicit Uploader(ProgressUpload clbUp = [](FileUploader&, uint64_t, uint64_t){},
                      ProgressPreparation clbPrep = [](Sha1Calculator&){});
    ~Uploader();

    Uploader(Uploader&&)                 = delete;
    Uploader(const Uploader&)            = delete;
    Uploader& operator=(const Uploader&) = delete;
    Uploader& operator=(Uploader&&)      = delete;

public:

    /**
     * @brief add a file or folder to the list of uploads
     *
     * @param parent the folder in which we want to upload data
     * @param path the path to the file or folder we want to upload
     */
    void
    addUpload(FolderNode parent, const boost::filesystem::path& path);

    /**
     * @brief Tells if the preparation phase is finished
     *
     * Before uploading a file, we need to calculate its sha1.
     * This phase is called the preparation phase.
     */
    bool
    isPreparationFinished() const;

    /**
     * @brief Gets the uploading files.
     *
     * During the uploading process, the local files are scanned.
     * The scanned files are prepared (see ```isPreparationFinished()```).
     * When the preparation phase is finished a FileUploader object is created.
     * This method return the list of FileUploader object already created.
     *
     * In this list of FileUploader, at most one is currently being uploaded,
     * some might be finished, other might be waiting.
     */
    std::vector<std::shared_ptr<FileUploader>>
    uploadingFiles();

    /**
     * @brief return ```uploadingFiles().size()```
     */
    std::vector<std::shared_ptr<FileUploader>>::size_type
    uploadingFilesCount();

    /**
     * @brief start the uploading process
     */
    void
    start();

    /**
     * @brief wait for the uploading process to finish
     */
    void
    join();

private:
    void
    scanFilesAddUploads (FolderNode& parent, const boost::filesystem::path& path);

    std::function<std::shared_ptr<Node> (std::shared_ptr<Node> n)>
    startNextUpload(std::shared_ptr<giga::core::FileUploader> next);


private:
    typedef std::shared_ptr<FileUploader> ReadyEntry;
    typedef std::pair<FolderNode, boost::filesystem::path> QueueElement;
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<QueueElement>> Queue;

    Queue                             _queue;
    Node::UploadingFile               _preparing;
    pplx::task<std::shared_ptr<Node>> _uploading;
    pplx::task<void>                  _mainTask;
    bool                              _isStarted;

    mutable std::mutex                _mut;
    std::vector<ReadyEntry>           _readyList;
    bool                              _isPreparationFinished;

    uint64_t                          _upBytes;
    uint64_t                          _upCount;
    std::atomic<bool>                 _isFinished;
    ProgressUpload                    _progressUp;
    ProgressPreparation               _progressPrep;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_UPLOADER_H_ */

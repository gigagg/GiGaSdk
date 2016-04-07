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
class Application;

namespace core
{

struct ScannedNode;
struct PreparedFile;
struct UploadRequest;

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
    typedef std::function<void(FileTransferer&, uint64_t waitingCount, uint64_t bytesTransfered)> ProgressFct;
    typedef std::function<void(FileTransferer&, std::shared_ptr<Node>)> NewNodeFct;
    typedef std::pair<boost::filesystem::path, std::string>  Error;

public:
    /**
     * @brief Construct an Uploader
     *
     * @see setUploadProgressFct()
     * @see setPreparationProgressFct()
     * @see setOnNewNodeFct()
     */
    explicit Uploader(const Application& app);
    ~Uploader();

    Uploader(Uploader&&)                 = delete;
    Uploader(const Uploader&)            = delete;
    Uploader& operator=(const Uploader&) = delete;
    Uploader& operator=(Uploader&&)      = delete;

public:

    /**
     * @brief Set a callback to follow the upload progress
     * @param fct a callback function that will be called periodically to let you know of the upload progress
     *
     * ```fct``` will be called :
     *
     * - At the beginning of the upload for each file
     * - For long upload it is called periodically.
     *
     * You should not do any long lasting computation in this function.
     */
    void
    setUploadProgressFct(ProgressFct fct);

    /**
     * @brief Set a callback to follow the preparation process
     * @param fct a callback function that will be called periodically to let you know of the progress of the preparation process (mostly sha1 calculation).
     *
     * ```fct``` will be called at least once for each file being prepared.
     * You should not do any long lasting computation in this function.
     */
    void
    setPreparationProgressFct(ProgressFct fct);

    /**
     * @brief The ```fct``` will be called for each new node
     */
    void
    setOnNewNodeFct(NewNodeFct fct);

    /**
     * @brief add a file or folder to the list of uploads
     *
     * @param parent the folder in which we want to upload data
     * @param path the path to the file or folder we want to upload
     */
    void
    addUpload(FolderNode parent, boost::filesystem::path&& path);

    void
    addUploads(FolderNode parent, std::vector<boost::filesystem::path>&& pathes);

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

    /**
     * @brief cancel the current upload; Clear the queue; stop the process.
     */
    void
    kill();

    /**
     * @brief Limit the current upload rate
     * @param rate the upload rate in Octet/s. Uses 0 for no limit.
     */
    void
    limitRate(uint64_t rate);

    /**
     * @brief Pause the current upload. Uses ```resume()``` to restart.
     */
    void
    pause();

    /**
     * @brief Resume a previously ```pause()```d upload.
     */
    void
    resume();

    /**
     * @brief Remove all the files waiting to be process. Only the current uploading file remains.
     * The Error queue is cleared (@see ```consumeError()```)
     */
    void
    clear();

    /**
     * @brief Gets the current upload state
     */
    FileTransferer::State
    state();

    bool
    isStarted() const;

    /**
     * @brief Gets an error from the error queue
     * @return The error, or nullptr if there is no error left
     */
    std::unique_ptr<Error>
    consumeError();

    struct FileTree final
    {
    public:
        enum class Type {
            file, directory
        };

    public:
        explicit FileTree(utility::string_t filename, Type type):
            filename{std::move(filename)}, type{type}, children{} {}

        ~FileTree()                          = default;
        FileTree(FileTree&&)                 = default;
        FileTree(const FileTree&)            = default;
        FileTree& operator=(const FileTree&) = default;
        FileTree& operator=(FileTree&&)      = default;

    public:
        utility::string_t filename;
        Type              type;
        std::vector<FileTree>   children;
    };

private:
    void
    scanFiles(FileTree& parent, const boost::filesystem::path& path);

    void
    prepare (FolderNode& parent, const boost::filesystem::path& aPath, const FileTree& entry);

    void
    uploadFile (const std::unique_ptr<PreparedFile>& element);

    void
    clearQueues ();

private:
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<UploadRequest>> RequestQueue;
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<ScannedNode>>   ScannedQueue;
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<PreparedFile>>  PreparedQueue;

    typedef moodycamel::BlockingReaderWriterQueue<Error>  ErrorQueue;

    RequestQueue                    _upRequest;
    ScannedQueue                    _toPrepare;
    PreparedQueue                   _toUpload;
    ErrorQueue                      _errors;

    std::unique_ptr<UploadRequest>  _scanningFile;
    std::unique_ptr<Sha1Calculator> _preparingFile;
    std::unique_ptr<FileUploader>   _uploadingFile;

    pplx::cancellation_token_source _scanCts;
    pplx::cancellation_token_source _clearCts;
    pplx::cancellation_token_source _cts;
    pplx::task<void>                _mainTask;
    bool                            _isStarted;

    mutable std::mutex              _mut;

    uint64_t                        _upBytes;
    uint64_t                        _upCount;
    uint64_t                        _sha1Bytes;
    uint64_t                        _sha1Count;
    uint64_t                        _totalCount;
    std::atomic<bool>               _isFinished;
    ProgressFct                     _progressUp;
    ProgressFct                     _progressPrep;
    NewNodeFct                      _onNewNode;
    const Application*              _app;
    uint64_t                        _rate;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_UPLOADER_H_ */

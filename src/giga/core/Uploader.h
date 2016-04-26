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
#include "TransferProgress.h"
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

struct ScannedFile;
struct PreparedFile;
struct UploadRequest;
struct FileTree;

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
    typedef std::function<void(FileTransferer&, TransferProgress)> ProgressFct;

    typedef std::function<void(const ScannedFile&)>   OnScannedFct;
    typedef std::function<void(const PreparedFile&)>  OnPreparedFct;
    typedef std::function<void(const boost::filesystem::path&, std::shared_ptr<Node>)> OnUploadedFct;
    typedef std::function<void(const boost::filesystem::path&, std::string&&)>         OnErrorFct;

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

    void
    setOnScannedFct(OnScannedFct fct);

    void
    setOnPreparedFct(OnPreparedFct fct);

    void
    setOnUploadedFct(OnUploadedFct fct);

    void
    setOnErrorFct(OnErrorFct fct);

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

    FileUploader*
    uploadingFile();

    void
    callProgressFct() const;

    void
    addScannedFile(const std::string parentId, const boost::filesystem::path& path);

    void
    addPreparedFile(const std::string parentId, const boost::filesystem::path& path, const std::string& sha1);

private:
    void
    scanFiles(FolderNode& dest, const boost::filesystem::path& ppath);

    void
    prepare (const ScannedFile& scanned);

    void
    uploadFile (const PreparedFile& element);

    bool
    isPaused () const;

    template <typename T> void
    enqueue(moodycamel::BlockingReaderWriterQueue<std::unique_ptr<T>>& queue, std::unique_ptr<T>&& element);

private:
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<UploadRequest>> RequestQueue;
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<ScannedFile>>   ScannedQueue;
    typedef moodycamel::BlockingReaderWriterQueue<std::unique_ptr<PreparedFile>>  PreparedQueue;

    RequestQueue                    _requests;
    ScannedQueue                    _scanned;
    PreparedQueue                   _prepared;

    std::atomic<int>                _clearRequestQueue;
    std::atomic<int>                _clearScannedQueue;
    std::atomic<int>                _clearPreparedQueue;

    std::unique_ptr<UploadRequest>  _scanningFile;
    std::unique_ptr<Sha1Calculator> _preparingFile;
    std::unique_ptr<FileUploader>   _uploadingFile;

    pplx::cancellation_token_source _cts;
    pplx::task<void>                _mainTask;
    bool                            _isStarted;

    mutable std::mutex              _mut;
    std::mutex                      _mutQueue;

    TransferProgress                _upProgress;
    TransferProgress                _sha1Progress;
    ProgressFct                     _upProgressFct;
    ProgressFct                     _sha1ProgressFct;
    OnScannedFct                    _onScannedFct;
    OnPreparedFct                   _onPreparedFct;
    OnUploadedFct                   _onUploadedFct;
    OnErrorFct                      _onErrorFct;
    std::atomic<bool>               _isFinished;
    const Application*              _app;
    uint64_t                        _rate;

    bool                            _isPaused;
};


struct ScannedFile
{
    explicit
    ScannedFile(std::string parentId, boost::filesystem::path path, uint64_t size):
        parentId{std::move(parentId)}, path{std::move(path)}, size{size}
    {}

    ScannedFile(ScannedFile&&)                 = default;
    ScannedFile(const ScannedFile&)            = default;
    ScannedFile& operator=(const ScannedFile&) = default;
    ScannedFile& operator=(ScannedFile&&)      = default;

    const std::string             parentId;
    const boost::filesystem::path path;
    const uint64_t                size;
};

template <typename T> void
Uploader::enqueue(moodycamel::BlockingReaderWriterQueue<std::unique_ptr<T>>& queue, std::unique_ptr<T>&& element)
{
    std::lock_guard<std::mutex> l{_mutQueue};
    queue.enqueue(std::move(element));
}

struct PreparedFile
{
    explicit
    PreparedFile (std::string parentId, boost::filesystem::path path, std::string sha1, std::string fkey, std::string fid, std::string fkeyEnc) :
            parentId(parentId), path(path), sha1(sha1), fkey(fkey), fid(fid), fkeyEnc(fkeyEnc)
    {
    }

    const std::string             parentId;
    const boost::filesystem::path path;
    const std::string             sha1;
    const std::string             fkey;
    const std::string             fid;
    const std::string             fkeyEnc;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_UPLOADER_H_ */

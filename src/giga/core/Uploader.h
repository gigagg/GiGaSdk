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
#include <boost/variant.hpp>
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

struct UploadRequestedFile;
struct ScannedFile;
struct PreparedFile;
struct UploadedFile;

struct UploadRequest;

/**
 * Upload files and folders.
 *
 * Start the upload process with the ```Uploader::start()``` method.
 * Uses ```Uploader::addUpload()``` to add file/folder to upload.
 * Terminate the uploading process using the ```Uploader::join()``` method.
 *
 * There are 3 phases in the upload process :
 *  - First the folder to upload is scanned, and every file found will be sent to the preparation process
 *  - Every file to upload needs to get prepared (calculate its sha1 etc...)
 *  - The prepared files gets finally uploaded.
 */
class Uploader
{
public:
    enum class Step {
        scanning, preparing, uploading
    };

    typedef std::function<void(FileTransferer&, TransferProgress)> ProgressFct;

    typedef std::function<void(const ScannedFile&)>  OnScannedFct;
    typedef std::function<void(const PreparedFile&)> OnPreparedFct;
    typedef std::function<void(UploadedFile&&)> OnUploadedFct;

    typedef boost::variant<UploadRequestedFile, ScannedFile, PreparedFile> UploadErrorData;
    typedef std::function<void(UploadErrorData&&, std::string&&, Step)>    OnErrorFct;


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
     * @param fct will be called every file found in the folders to upload
     *
     * For each file/folder passed to ```addUpload()```, the ```fct``` gets called
     * one extra time, to notify that all files for this ```addUpload()``` have been scanned.
     * In this special case, the ```ScannedFile::size``` is set to zero.
     *
     */
    void
    setOnScannedFct(OnScannedFct fct);

    /**
     * @param fct will be called every time a preparation is finish..
     */
    void
    setOnPreparedFct(OnPreparedFct fct);

    /**
     * @param fct will be called for every node created (file or folder) during the upload process.
     */
    void
    setOnUploadedFct(OnUploadedFct fct);

    /**
     * @param fct will be called for every error/canceled uploads.
     */
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
    addScannedFile(UploadRequestedFile request, boost::filesystem::path nodePath);

    void
    addPreparedFile(ScannedFile scanned, const std::string& sha1);

private:
    void
    scanFiles(const FolderNode& dest,  boost::filesystem::path relativeNodePath, const boost::filesystem::path& realPath);

    void
    prepare (const ScannedFile& scanned);

    void
    uploadFile (const PreparedFile& element, int retryCount = 0);

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

    std::unique_ptr<Node>           _cacheNode;
};

template <typename T> void
Uploader::enqueue(moodycamel::BlockingReaderWriterQueue<std::unique_ptr<T>>& queue, std::unique_ptr<T>&& element)
{
    std::lock_guard<std::mutex> l{_mutQueue};
    queue.enqueue(std::move(element));
}

struct UploadRequestedFile
{
    explicit
    UploadRequestedFile(std::string parentId, boost::filesystem::path path) :
            parentId{std::move(parentId)}, path{std::move(path)}
    {}

    UploadRequestedFile(UploadRequestedFile&&)                 = default;
    UploadRequestedFile(const UploadRequestedFile&)            = default;
    UploadRequestedFile& operator=(const UploadRequestedFile&) = default;
    UploadRequestedFile& operator=(UploadRequestedFile&&)      = default;

    const std::string             parentId;
    const boost::filesystem::path path;
};

struct ScannedFile
{
    explicit
    ScannedFile(UploadRequestedFile request, boost::filesystem::path nodePath, uint64_t size) :
        request{std::move(request)}, nodePath{std::move(nodePath)}, size{size}
    {}

    ScannedFile(ScannedFile&&)                 = default;
    ScannedFile(const ScannedFile&)            = default;
    ScannedFile& operator=(const ScannedFile&) = default;
    ScannedFile& operator=(ScannedFile&&)      = default;

    const UploadRequestedFile     request;
    const boost::filesystem::path nodePath;
    const uint64_t                size;
};

struct PreparedFile
{
    explicit
    PreparedFile (ScannedFile scanned, std::string sha1, std::string fkey, std::string fid, std::string fkeyEnc) :
        scanned{std::move(scanned)},
        sha1(std::move(sha1)), fkey(std::move(fkey)), fid(std::move(fid)), fkeyEnc(std::move(fkeyEnc))
    {}

    PreparedFile(PreparedFile&&)                 = default;
    PreparedFile(const PreparedFile&)            = default;
    PreparedFile& operator=(const PreparedFile&) = default;
    PreparedFile& operator=(PreparedFile&&)      = default;

    const ScannedFile scanned;
    const std::string sha1;
    const std::string fkey;
    const std::string fid;
    const std::string fkeyEnc;
};

struct UploadedFile
{
    explicit
    UploadedFile (PreparedFile prepared, std::shared_ptr<Node> node) :
        prepared{std::move(prepared)}, node(std::move(node))
    {}

    UploadedFile(UploadedFile&&)                 = default;
    UploadedFile(const UploadedFile&)            = default;
    UploadedFile& operator=(const UploadedFile&) = default;
    UploadedFile& operator=(UploadedFile&&)      = default;

    const PreparedFile prepared;
    std::shared_ptr<Node> node;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_UPLOADER_H_ */

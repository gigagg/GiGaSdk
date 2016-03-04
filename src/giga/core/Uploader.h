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

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#include "FolderNode.h"

namespace giga
{
namespace core
{

class Uploader
{
public:
    typedef std::function<void(FileUploader&, uint64_t count, uint64_t bytes)> ProgressCallback;

public:
    explicit Uploader(FolderNode parent, const boost::filesystem::path& path, ProgressCallback clb = [](FileUploader&, uint64_t, uint64_t){});
    ~Uploader();

    bool
    isPreparationFinished() const;

    std::vector<std::shared_ptr<FileUploader>>
    uploadingFiles();

    std::vector<std::shared_ptr<FileUploader>>::size_type
    uploadingFilesCount();

    pplx::task<void>
    start();

private:
    void
    scanFilesAddUploads (FolderNode& parent, const boost::filesystem::path& path);

    std::function<std::shared_ptr<Node> (std::shared_ptr<Node> n)>
    startNextUpload(std::shared_ptr<giga::core::FileUploader> next);


private:
    typedef pplx::task<std::shared_ptr<FileUploader>> PreparingEntry;
    typedef std::shared_ptr<FileUploader> ReadyEntry;

    FolderNode                        _parent;
    boost::filesystem::path           _path;
    std::vector<PreparingEntry>       _preparingList;
    pplx::task<std::shared_ptr<Node>> _uploading;
    pplx::task<void>                  _mainTask;
    bool                              _isStarted;

    mutable std::mutex                _mut;
    std::vector<ReadyEntry>           _readyList;
    bool                              _isPreparationFinished;

    uint64_t                          _upBytes;
    uint64_t                          _upCount;
    std::atomic<bool>                 _isFinished;
    ProgressCallback                  _progressCallback;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_UPLOADER_H_ */

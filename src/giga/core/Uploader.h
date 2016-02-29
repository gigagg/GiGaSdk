/*
 * FolderUploader.h
 *
 *  Created on: 19 févr. 2016
 *      Author: thomas
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
    explicit Uploader(FolderNode parent, const std::string& path, ProgressCallback clb = [](FileUploader&, uint64_t, uint64_t){});
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
    scanFilesAddUploads (FolderNode parent, boost::filesystem::path path);

    std::function<std::shared_ptr<Node> (std::shared_ptr<Node> n)>
    startNextUpload(std::shared_ptr<giga::core::FileUploader> next);


private:
    typedef pplx::task<std::shared_ptr<FileUploader>> PreparingEntry;
    typedef std::shared_ptr<FileUploader> ReadyEntry;

    FolderNode                        _parent;
    std::string                       _path;
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

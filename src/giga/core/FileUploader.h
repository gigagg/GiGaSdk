/*
 * FileUploader.h
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_API_FILEUPLOADER_H_
#define GIGA_API_FILEUPLOADER_H_

#include "FileTransferer.h"

#include <pplx/pplxtasks.h>

namespace giga
{
namespace core
{
class Node;
class UploadState;

class FileUploader final : public FileTransferer
{
public:
    explicit
    FileUploader (const std::string& filename, const std::string& nodeName, const std::string& parentId, const std::string& sha1,
                  const std::string& fid, const std::string& fkey);
    ~FileUploader ()                             = default;

    FileUploader ()                              = delete;
    FileUploader (FileUploader&&)                = delete;
    FileUploader& operator=(const FileUploader&) = delete;
    FileUploader (const FileUploader&)           = delete;

public:
    void
    doStart () override;

    const pplx::task<std::shared_ptr<Node>>&
    task () const;

    double
    progress () const;

private:
    pplx::task<std::shared_ptr<Node>> _task;

    std::string _filename;
    std::string _nodeName;
    std::string _parentId;
    std::string _sha1;
    std::string _fid;
    std::string _fkey;

    uint64_t _fileSize;
};

} /* namespace api */
} /* namespace giga */

#endif /* GIGA_API_FILEUPLOADER_H_ */

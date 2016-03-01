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
#include <cpprest/details/basic_types.h>

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
    FileUploader (const utility::string_t& filename, const utility::string_t& nodeName, const utility::string_t& parentId,
                  const utility::string_t& sha1, const utility::string_t& fid, const utility::string_t& fkey);
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

    FileTransferer::Progress
    progress () const;

    const utility::string_t&
    nodeName() const;

    const utility::string_t&
    fileName() const;

    uint64_t
    fileSize() const;

private:
    pplx::task<std::shared_ptr<Node>> _task;

    utility::string_t _filename;
    utility::string_t _nodeName;
    utility::string_t _parentId;
    utility::string_t _sha1;
    utility::string_t _fid;
    utility::string_t _fkey;

    uint64_t _fileSize;
};

} /* namespace api */
} /* namespace giga */

#endif /* GIGA_API_FILEUPLOADER_H_ */

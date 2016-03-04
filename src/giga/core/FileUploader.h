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
    FileUploader (const utility::string_t& filename, const utility::string_t& nodeName, const std::string& parentId,
                  const std::string& sha1, const std::string& fid, const std::string& fkey);
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
    std::string       _parentId;
    std::string          _sha1;
    std::string       _fid;
    std::string       _fkey;

    uint64_t _fileSize;
};

} /* namespace api */
} /* namespace giga */

#endif /* GIGA_API_FILEUPLOADER_H_ */

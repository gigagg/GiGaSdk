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

#ifndef GIGA_CORE_FILENODE_H_
#define GIGA_CORE_FILENODE_H_

#include <cpprest/http_client.h>

#include "FileDownloader.h"
#include "Node.h"

namespace giga
{
namespace core
{

class FolderNode;

class FileNodeData final
{
    friend class FileNode;

private:
    FileNodeData()                                = default;
    explicit FileNodeData(std::shared_ptr<data::Node> n);

public:
    ~FileNodeData()                               = default;
    FileNodeData(FileNodeData&&)                  = default;
    FileNodeData& operator=(FileNodeData&&)       = default;
    FileNodeData(const FileNodeData&)             = default;
    FileNodeData& operator=(const FileNodeData&)  = default;

public:
    const utility::string_t&
    mimeType() const;

    const std::string&
    fid() const;

    int64_t
    previewState() const;

    web::uri
    iconUrl() const;

    web::uri
    squareUrl() const;

    web::uri
    originalUrl() const;

    web::uri
    posterUrl() const;

    web::uri
    fileUrl() const;

private:
    std::shared_ptr<data::Node> n;
};

class FileNode final : public Node
{
public:
    FileNode()                        = default;
    virtual ~FileNode()               = default;
    FileNode(FileNode&&)              = default;
    FileNode& operator=(FileNode&&)   = default;

    explicit FileNode(std::shared_ptr<data::Node> n);
    FileNode(const FileNode&);
    FileNode& operator=(const FileNode&);

public:
    virtual const std::vector<std::unique_ptr<Node>>&
    children() const override;

    virtual void
    loadChildren() override {}

    virtual FolderNode&
    addChildFolder(const utility::string_t& name) override;

    virtual pplx::task<std::shared_ptr<FileUploader>>
    uploadFile(const utility::string_t& filepath) override;

    virtual FileDownloader
    download(const utility::string_t& destinationPath, FileDownloader::Policy policy = FileDownloader::Policy::ignore) override;

    virtual const FileNodeData&
    fileData() const override;

private:
    FileNodeData _fileData;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILENODE_H_ */

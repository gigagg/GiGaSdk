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

/**
 * Store the data specific to FileNode (ie: not found in FolderNode)
 */
class FileNodeData final
{
    friend class FileNode;

public:
    enum class PreviewState
    {
        noPreview = 0, previewNoIcon = 4, preview = 7, computing = 8, error = 16
    };

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

    PreviewState
    previewState() const;

    /**
     * @brief An url to a 48x48 image.
     *
     * If this icon is not set (see ```previewState()```)
     * then we fall back to a generic icon (48x48)
     */
    web::uri
    iconUrl() const;

    /**
     * @brief An url to a 185x185 image
     *
     * If this icon is not set (see ```previewState()```)
     * then we fall back to a generic icon (255x255)
     */
    web::uri
    squareUrl() const;

    /**
     * @brief An url to a small none cropped image (max size is 185x278)
     *
     * If this icon is not set (see ```previewState()```)
     * then we fall back to a generic icon (255x255)
     */
    web::uri
    originalUrl() const;

    /**
     * @brief An url to a 185x278 image
     *
     * If this icon is not set (see ```previewState()```)
     * then we fall back to a generic icon (255x255)
     */
    web::uri
    posterUrl() const;

    /**
     * @return The download url for this file.
     */
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
    getChildren() const override;

    virtual FolderNode&
    addChildFolder(const utility::string_t& name) override;

    virtual FolderNode
    createChildFolder(const utility::string_t& name) const override;

    virtual UploadingFile
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

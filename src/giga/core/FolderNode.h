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

#ifndef GIGA_CORE_FOLDERNODE_H_
#define GIGA_CORE_FOLDERNODE_H_

#include "Node.h"

namespace giga
{
namespace core
{

class FolderNode final : public Node
{
public:
    FolderNode()                        = default;
    virtual ~FolderNode()               = default;
    FolderNode(FolderNode&&)            = default;
    FolderNode& operator=(FolderNode&&) = default;

    FolderNode& operator=(const FolderNode&);
    FolderNode(const FolderNode& other);
    explicit FolderNode(std::shared_ptr<data::Node> n, const Application& app);

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
    mutable std::vector<std::unique_ptr<Node>> _children; // cache
    // TODO mutex _children

};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FOLDERNODE_H_ */

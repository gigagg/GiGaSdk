/*
 * FolderNode.h
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
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
    explicit FolderNode(std::shared_ptr<data::Node> n);

public:
    virtual const std::vector<std::unique_ptr<Node>>&
    children() const override;

    virtual void
    loadChildren() override;

    virtual FolderNode&
    addChildFolder(const utility::string_t& name) override;

    virtual pplx::task<std::shared_ptr<FileUploader>>
    uploadFile(const utility::string_t& filepath) override;

    virtual FileDownloader
    download(const utility::string_t& destinationPath, FileDownloader::Policy policy = FileDownloader::Policy::ignore) override;

    virtual const FileNodeData&
    fileData() const override;

private:
    std::vector<std::unique_ptr<Node>> _children;

};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FOLDERNODE_H_ */

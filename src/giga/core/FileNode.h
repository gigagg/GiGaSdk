/*
 * FileNode.h
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_FILENODE_H_
#define GIGA_CORE_FILENODE_H_

#include <cpprest/http_client.h>
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
    FileNodeData(const FileNodeData&)             = default;
    FileNodeData& operator=(const FileNodeData&)  = default;
    FileNodeData& operator=(FileNodeData&&)       = default;

public:
    const std::string&
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
    FileNode()                        = default; // TODO: private + friend to correct class (pplx).
    virtual ~FileNode()               = default;
    FileNode(FileNode&&)                  = default;
    FileNode(const FileNode&)             = default;
    FileNode& operator=(const FileNode&)  = default;
    FileNode& operator=(FileNode&&)       = default;
    explicit FileNode(std::shared_ptr<data::Node> n);

public:
    virtual const std::vector<std::unique_ptr<Node>>&
    children() const override;

    virtual void
    loadChildren() override {}

    virtual FolderNode&
    addChildFolder(const std::string& name) override;

    virtual pplx::task<std::shared_ptr<FileUploader>>
    uploadFile(const std::string& filepath) override;

    virtual FileDownloader
    download(const std::string& destinationPath, bool doContinue = false) override;

    virtual const FileNodeData&
    fileData() const override;

private:
    FileNodeData _data;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILENODE_H_ */

/*
 * Node.h
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_NODE_H_
#define GIGA_CORE_NODE_H_

#include "FileUploader.h"
#include "../utils/EnumConvertor.h"

#include <chrono>
#include <iosfwd>
#include <memory>
#include <vector>

#include "FileDownloader.h"
namespace giga
{

namespace data
{
class Node;
}

namespace core
{
class FolderNode;
class FileNodeData;

class Node
{
public:
    enum class Type
    {
        root = 0, folder, file
    };
    static const utils::EnumConvertor<core::Node::Type, 3> typeCvrt;

    enum class MediaType
    {
        audio = 0, document, video, image, unknown, folder //, user
    };
    static const utils::EnumConvertor<MediaType, 6> mediaTypeCvrt;

public:
    Node()                        = default; // TODO: protected + friend to correct class (pplx).
    virtual ~Node()               = default;
    Node(Node&&)                  = default;
    Node(const Node&)             = default;
    Node& operator=(const Node&)  = default;
    Node& operator=(Node&&)       = default;

    static std::unique_ptr<Node>
    create(std::shared_ptr<data::Node> n);

protected:
    explicit Node(std::shared_ptr<data::Node> n);

public:
    const std::string&
    id() const;

    Type
    type() const;

    const std::string&
    name() const;

    const std::string&
    parentId() const;

    const std::vector<std::string>&
    ancestors() const;

    int64_t
    ownerId() const;

    std::chrono::system_clock::time_point
    creationDate () const;

    std::chrono::system_clock::time_point
    lastUpdateDate () const;

    uint64_t
    nbChildren () const;

    uint64_t
    nbFiles () const;

    uint64_t
    size () const;

    virtual const std::vector<std::unique_ptr<Node>>&
    children() const = 0;

    virtual FolderNode&
    addChildFolder(const std::string& name) = 0;

    virtual pplx::task<std::shared_ptr<FileUploader>>
    uploadFile(const std::string& filepath) = 0;

    virtual void
    loadChildren() = 0;

    virtual FileDownloader
    download(const std::string& destinationPath, bool doContinue = false) = 0;

    virtual const FileNodeData&
    fileData() const = 0;

    bool
    shouldLoadChildren() const;

    void
    remove();

    void
    rename(const std::string& name);


protected:
    std::shared_ptr<data::Node> n;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_NODE_H_ */

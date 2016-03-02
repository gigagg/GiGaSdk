/*
 * Node.h
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_NODE_H_
#define GIGA_CORE_NODE_H_

#include "FileUploader.h"
#include "FileDownloader.h"
#include "../utils/EnumConvertor.h"

#include <cpprest/details/basic_types.h>
#include <chrono>
#include <memory>
#include <vector>

namespace giga
{

namespace data
{
struct Node;
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
    Node()                        = default;
    virtual ~Node()               = default;
    Node(Node&&)                  = default;
    Node& operator=(Node&&)       = default;

    Node(const Node& rhs);
    Node& operator=(const Node& rhs);

    static std::unique_ptr<Node>
    create(std::shared_ptr<data::Node> n);

protected:
    explicit Node(std::shared_ptr<data::Node> n);

public:
    const utility::string_t&
    id() const;

    Type
    type() const;

    const utility::string_t&
    name() const;

    const utility::string_t&
    parentId() const;

    const std::vector<utility::string_t>&
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
    addChildFolder(const utility::string_t& name) = 0;

    virtual pplx::task<std::shared_ptr<FileUploader>>
    uploadFile(const utility::string_t& filepath) = 0;

    virtual void
    loadChildren() = 0;

    virtual FileDownloader
    download(const utility::string_t& destinationPath, FileDownloader::Policy policy = FileDownloader::Policy::ignore) = 0;

    virtual const FileNodeData&
    fileData() const = 0;

    bool
    shouldLoadChildren() const;

    void
    remove();

    void
    rename(const utility::string_t& name);


protected:
    std::shared_ptr<data::Node> _data;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_NODE_H_ */

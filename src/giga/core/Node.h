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

#ifndef GIGA_CORE_NODE_H_
#define GIGA_CORE_NODE_H_

#include "FileUploader.h"
#include "FileDownloader.h"
#include "../utils/EnumConvertor.h"
#include "Sha1Calculator.h"

#include <cpprest/details/basic_types.h>
#include <chrono>
#include <memory>
#include <utility>
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

/**
 * The class Node represents a file or a folder.
 * It is the base class of a composite pattern, with the
 * ```FileNode``` and ```FolderNode``` classes.
 */
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

    static std::unique_ptr<Node>
    create(const Node& n);

protected:
    explicit Node(std::shared_ptr<data::Node> n);

public:
    const std::string&
    id() const;

    Type
    type() const;

    const utility::string_t&
    name() const;

    const std::string&
    parentId() const;

    const std::vector<std::string>&
    ancestors() const;

    uint64_t
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

    /**
     * @brief Load the children node if needed then return them.
     * The children nodes are stored in a local cache variable.
     *
     * @return The list of children nodes
     * @throw HttpError
     */
    virtual const std::vector<std::unique_ptr<Node>>&
    getChildren() const = 0;

    /**
     * @brief Create a new FolderNode and add it to the children list.
     * @return The new FolderNode
     * @throw HttpError
     */
    virtual FolderNode&
    addChildFolder(const utility::string_t& name) = 0;

    /**
     * @brief Create a new FolderNode (Do not add it to the local children list)
     * @return The new FolderNode
     * @throw HttpError
     */
    virtual FolderNode
    createChildFolder(const utility::string_t& name) const = 0;

    typedef std::unique_ptr<
                std::pair<
                    std::unique_ptr<Sha1Calculator>,
                    pplx::task<std::shared_ptr<FileUploader>>
                >
             > UploadingFile;

    /**
     * @brief Upload a file into this FolderNode.
     *
     * @param filepath the path to the file to upload
     * @return A FileUploader object to control the upload.
     * @throw ErrorException if this node is a FileNode.
     * @see Uploader to upload folders.
     */
    virtual UploadingFile
    uploadFile(const utility::string_t& filepath) = 0;

    /**
     * @brief Download a FileNode
     *
     * @param destinationPath a path to a folder where the FileNode will be downloaded
     * @param policy what to do if the file already exists at ```destinationPath```
     * @return A FileDownloader to control the download.
     * @throw ErrorException if this node is a FolderNode
     * @see Downloader to download folders.
     */
    virtual FileDownloader
    download(const utility::string_t& destinationPath, FileDownloader::Policy policy = FileDownloader::Policy::ignore) = 0;

    /**
     * @brief Get the data associated with a FileNode
     * @throw ErrorException if this node is a FolderNode
     */
    virtual const FileNodeData&
    fileData() const = 0;

    /**
     * @brief Remove this node. Set its id to ```""```
     * @throw HttpError
     */
    void
    remove();

    /**
     * @brief Rename this node.
     *
     * The new name may be different from the name parameter (some characters may be changed to have a valid filename).
     * @param name must not exceed 255 chars
     * @return The new name.
     * @throw HttpError
     * @throw ErrorException if the name is not valid.
     */
    const utility::string_t&
    rename(const utility::string_t& name);


protected:
    std::shared_ptr<data::Node> _data;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_NODE_H_ */

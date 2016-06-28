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
class Application;

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

    /**
     * The MergePolicy is used when inserting / uploading / copying / moving files and folders.
     * It dictate what to do when there is a name conflict between 2 files / folders.
     *
     * The possible values are :
     *
     *   - ```override```      : Override the destination file with the file being copied/moved/...
     *   - ```ignore```        : Ignore the copy/move/... and keep the destination file
     *   - ```renameSource``` : Rename the file being copied/moved/... before inserting it.
     *   - ```renameDest```    : Rename the conflicting file in the destination folder before inserting the new file.
     *
     * The way the mergePolicy is applied is different whether it's applied on files or folders:
     *
     * |                 |          Destination: File            |          Destination: Folder          |
     * |:---------------:|:-------------------------------------:|:-------------------------------------:|
     * |   Source: File  |  Use policy (Ignore on same files)₁   | Use policy (renameSource on override)₂|
     * | Source:  Folder | Use policy (renameSource on override)₂|     Always merge the 2 folders.₃      |
     *
     * - 1) If the 2 files are the same, the specified policy is ignored, and ```ignore``` policy is used instead.
     *    If the 2 files are different, the policy is used as specified.
     * - 2) When there is a folder/file or file/folder conflict, even if the ```override``` policy is specified, no file/folder gets overridden.
     *    In this case, the ```renameSource``` policy is used instead of the ```override``` policy
     * - 3) When there is a folder/folder conflict, the policy is ignored, and the 2 folder are merged together.
     */
    enum class MergePolicy
    {
        override = 0, ignore, renameSource, renameDest
    };
    static const utils::EnumConvertor<MergePolicy, 4> mergePolicyCvrt;

public:
    Node()                        = default;
    virtual ~Node()               = default;
    Node(Node&&)                  = default;
    Node& operator=(Node&&)       = default;

    Node(const Node& rhs);
    Node& operator=(const Node& rhs);

    static std::unique_ptr<Node>
    create(std::shared_ptr<data::Node> n, const Application& app);

    static std::unique_ptr<Node>
    create(const Node& node);

protected:
    explicit Node(std::shared_ptr<data::Node> n, const Application& app);

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
    uploadFile(const boost::filesystem::path& filepath, pplx::cancellation_token_source cts = pplx::cancellation_token_source{}) = 0;

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

    /**
     * @brief Shortcut for ```copyOrMoveTo(const FolderNode&, false, MergePolicy)```
     */
    std::unique_ptr<core::Node>
    copyTo(const FolderNode& node, MergePolicy policy = MergePolicy::renameSource)  const;

    /**
     * @brief Shortcut for ```copyOrMoveTo(const FolderNode&, bool, MergePolicy)```
     */
    std::unique_ptr<core::Node>
    moveTo(const FolderNode& node, MergePolicy policy = MergePolicy::renameSource)  const;

    /**
     * @brief Copy or move a node
     *
     * - If ```this``` is a file and ```node``` contains a file with the same name,
     * the ```policy``` is used
     * - If ```this``` is a folder and ```node``` contains a folder with the same name,
     * the two folders will be merged..
     * - For more information on merge policy see ```giga::core::Node::MergePolicy```
     *
     * The copy/move operation is a long lasting operation (more than 1s).
     * The operation will timeout after 465 seconds
     *
     * @param node is the destination folder. You may want to reload ```node``` (uses ```Application::getNodeById()```)
     * @param isMove true for a move, false for a copy
     * @param policy the merge policy is used when there are name conflicts.
     * @return the copied or moved node (```this``` node should not be used after being moved).
     */
    std::unique_ptr<core::Node>
    copyOrMoveTo(const FolderNode& node, bool isMove, MergePolicy policy = MergePolicy::renameSource) const;

    const std::shared_ptr<data::Node>
    handle() const;

protected:
    std::shared_ptr<data::Node> _data;
    const Application*          _app;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_NODE_H_ */

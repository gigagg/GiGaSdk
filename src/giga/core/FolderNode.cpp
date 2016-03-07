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

#include "FolderNode.h"
#include "../Application.h"
#include "../api/data/Node.h"
#include "../api/data/DataNode.h"
#include "../api/NodesApi.h"

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <algorithm>

using pplx::create_task;
using utility::string_t;

namespace {

void
regenerateChildren(std::vector<std::unique_ptr<giga::core::Node>>& chld, std::shared_ptr<giga::data::Node> n)
{
    chld.resize(n->nodes.size());
    std::transform (n->nodes.begin(), n->nodes.end(), chld.begin(), [](const std::shared_ptr<giga::data::Node>& data) {
        return giga::core::Node::create(data);
    });
}

}

namespace giga
{
namespace core
{

FolderNode::FolderNode (std::shared_ptr<data::Node> n) :
        Node(n), _children{}
{
    regenerateChildren(_children, n);
}

FolderNode::FolderNode(const FolderNode& rhs) :
        Node(rhs), _children{}
{
    regenerateChildren(_children, rhs._data);
}

FolderNode&
FolderNode::operator=(const FolderNode& rhs)
{
    Node::operator=(rhs);
    regenerateChildren(_children, rhs._data);
    return *this;
}

const std::vector<std::unique_ptr<Node>>&
FolderNode::getChildren() const
{
    if (nbChildren() > 0 && _children.size() < nbChildren())
    {
        auto results = NodesApi::getChildrenNode(id()).get();

        _children.resize(results->size());
        std::transform (results->begin(), results->end(), _children.begin(), [](const data::Node& data) {
            return Node::create(std::make_shared<data::Node>(data));
        });
    }
    return _children;
}

FolderNode&
FolderNode::addChildFolder(const string_t& name)
{
    auto result = NodesApi::addFolderNode(name, id()).get();
    _children.push_back(Node::create(std::make_shared<data::Node>(*result->data.release())));
    _data->nbChildren += 1;
    return static_cast<FolderNode&>(*_children.back());
}

namespace fs = boost::filesystem;

pplx::task<std::shared_ptr<FileUploader>>
FolderNode::uploadFile(const string_t& filepath)
{
    auto path = fs::path{filepath};
    if (!fs::exists(path) || !fs::is_regular_file(path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("This is not a regular file")});
    }

    auto parentId = this->id();
    auto nodeName = path.filename().native();
    auto nodeKeyClear = Application::get().currentUser().personalData().nodeKeyClear();

    return create_task([=]() {
        auto sha1 = Crypto::sha1File(filepath);
        auto fkey = Crypto::calculateFkey(sha1);
        auto decodedNodeKey = Crypto::base64decode(nodeKeyClear);
        auto fkeyEnc = Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey);

        return std::make_shared<FileUploader>(filepath, nodeName, parentId, std::move(sha1), Crypto::calculateFid(sha1), Crypto::base64encode(fkeyEnc));
    });
}

FileDownloader
FolderNode::download(const string_t&, FileDownloader::Policy)
{
    BOOST_THROW_EXCEPTION(ErrorException{U("Not implemented")});
}

const FileNodeData&
FolderNode::fileData() const
{
    BOOST_THROW_EXCEPTION(ErrorException{U("No file data in a folder")});
}

}/* namespace core */
} /* namespace giga */

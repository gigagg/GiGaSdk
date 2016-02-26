/*
 * FolderNode.cpp
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
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
    regenerateChildren(_children, rhs.n);
}

FolderNode&
FolderNode::operator=(const FolderNode& rhs)
{
    Node::operator=(rhs);
    regenerateChildren(_children, rhs.n);
    return *this;
}

const std::vector<std::unique_ptr<Node>>&
FolderNode::children() const
{
    return _children;
}


void
FolderNode::loadChildren()
{
    if (nbChildren() > 0)
    {
        auto results = NodesApi::getChildrenNode(id()).get();

        _children.resize(results->size());
        std::transform (results->begin(), results->end(), _children.begin(), [](const data::Node& data) {
            return Node::create(std::make_shared<data::Node>(data));
        });
    }
}

FolderNode&
FolderNode::addChildFolder(const std::string& name)
{
    auto result = NodesApi::addFolderNode(name, id()).get();
    _children.push_back(Node::create(std::make_shared<data::Node>(*result->data.release())));
    n->nbChildren += 1;
    return static_cast<FolderNode&>(*_children.back());
}

namespace fs = boost::filesystem;

pplx::task<std::shared_ptr<FileUploader>>
FolderNode::uploadFile(const std::string& filepath)
{
    auto path = fs::path{filepath};
    if (!fs::exists(path) || !fs::is_regular_file(path))
    {
        THROW(ErrorException{"This is not a regular file"});
    }

    auto parentId = this->id();
    auto nodeName = path.filename().string();
    auto nodeKeyClear = Application::get().currentUser().privateData().nodeKeyClear;

    return create_task([=]() {
        auto sha1 = Crypto::sha1File(filepath);
        auto fkey = Crypto::calculateFkey(sha1);
        auto decodedNodeKey = Crypto::base64decode(nodeKeyClear);
        auto fkeyEnc = Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey);

        return std::make_shared<FileUploader>(filepath, nodeName, parentId, std::move(sha1), Crypto::calculateFid(sha1), Crypto::base64encode(fkeyEnc));
    });
}

FileDownloader
FolderNode::download(const std::string&, FileDownloader::Policy)
{
    THROW(ErrorException{"Not implemented"});
}

const FileNodeData&
FolderNode::fileData() const
{
    THROW(ErrorException{"No file data in a folder"});
}

}/* namespace core */
} /* namespace giga */

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

#include <boost/filesystem.hpp>
#include <pplx/pplxtasks.h>
#include <algorithm>

#include "Sha1Calculator.h"
using pplx::create_task;
using utility::string_t;

namespace {

void
regenerateChildren(std::vector<std::unique_ptr<giga::core::Node>>& chld, std::shared_ptr<giga::data::Node> n, const giga::Application* app)
{
    chld.resize(n->nodes.size());
    std::transform (n->nodes.begin(), n->nodes.end(), chld.begin(), [app](const std::shared_ptr<giga::data::Node>& data) {
        return giga::core::Node::create(data, *app);
    });
}

}

namespace giga
{
namespace core
{

FolderNode::FolderNode (std::shared_ptr<data::Node> n, const Application& app) :
        Node(n, app), _children{}
{
    regenerateChildren(_children, n, _app);
}

FolderNode::FolderNode(const FolderNode& rhs) :
        Node(rhs), _children{}
{
    regenerateChildren(_children, rhs._data, _app);
}

FolderNode&
FolderNode::operator=(const FolderNode& rhs)
{
    Node::operator=(rhs);
    regenerateChildren(_children, rhs._data, _app);
    return *this;
}

const std::vector<std::unique_ptr<Node>>&
FolderNode::getChildren() const
{
    if (nbChildren() > 0 && _children.size() < nbChildren())
    {
        if (_app == nullptr)
        {
            BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
        }

        auto results = _app->api().nodes.getChildrenNode(id()).get();
        auto app = _app;
        _children.resize(results->size());
        std::transform (results->begin(), results->end(), _children.begin(), [app](const data::Node& data) {
            return Node::create(std::make_shared<data::Node>(data), *app);
        });
    }
    return _children;
}

FolderNode&
FolderNode::addChildFolder(const string_t& name)
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }

    auto result = _app->api().nodes.addFolderNode(name, id()).get();
    _children.push_back(Node::create(std::make_shared<data::Node>(*result->data.release()), *_app));
    _data->nbChildren += 1;
    return static_cast<FolderNode&>(*_children.back());
}

FolderNode
FolderNode::createChildFolder(const utility::string_t& name) const
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }

    auto result = _app->api().nodes.addFolderNode(name, id()).get();
    return FolderNode{std::make_shared<data::Node>(*result->data.release()), *_app};
}

namespace fs = boost::filesystem;

Node::UploadingFile
FolderNode::uploadFile(const fs::path& path, pplx::cancellation_token_source cts)
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }

    if (!fs::exists(path) || !fs::is_regular_file(path))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("This is not a regular file")});
    }

    auto parentId = this->id();
    auto nodeName = utils::replaceInvalidUtf8(path.filename().native());
    auto nodeKeyClear = _app->currentUser().personalData().nodeKeyClear();

    auto calculator = std::unique_ptr<Sha1Calculator>{new Sha1Calculator(path, cts)};
    calculator->start();

    auto app = _app;
    auto task = calculator->task().then([=](std::string sha1) {
        auto fkey = Crypto::calculateFkey(sha1);
        auto fid = Crypto::calculateFid(sha1);
        auto decodedNodeKey = Crypto::base64decode(nodeKeyClear);
        auto fkeyEnc = Crypto::aesEncrypt(decodedNodeKey.substr(0, 16), decodedNodeKey.substr(16, 16), fkey);

        return std::make_shared<FileUploader>(path, nodeName, parentId, std::move(sha1), fid, Crypto::base64encode(fkeyEnc), *app, cts);
    });

    return UploadingFile{new std::pair<std::unique_ptr<Sha1Calculator>, pplx::task<std::shared_ptr<FileUploader>>>{std::move(calculator), std::move(task)}};
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

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

#include "Node.h"
#include "FileNode.h"
#include "FolderNode.h"
#include "../Application.h"
#include "../api/data/Node.h"
#include "../rest/HttpErrors.h"
#include "../utils/EnumConvertor.h"
#include "../utils/Utils.h"

#include <boost/filesystem.hpp>
#include <giga/Application.h>
#include <chrono>
#include <string>
#include <thread>

using std::chrono::system_clock;
using utility::string_t;

#define _THROW_IF_NO_NODE_ if (_data == nullptr) { BOOST_THROW_EXCEPTION(ErrorException{U("Node is not set")}); } do {} while(0)

namespace giga
{

const utils::EnumConvertor<core::Node::Type, 3> core::Node::typeCvrt =
    {U("root"), U("folder"), U("file")};

const utils::EnumConvertor<core::Node::MediaType, 6> core::Node::mediaTypeCvrt =
    {U("audio"), U("document"), U("video"), U("image"), U("unknown"), U("folder")};

const utils::EnumConvertor<core::Node::MergePolicy, 4> core::Node::mergePolicyCvrt =
    {U("override"), U("ignore"), U("renameSource"), U("renameDest")};

namespace core
{

std::unique_ptr<Node>
Node::create (std::shared_ptr<data::Node> n, const Application& app)
{
    switch (typeCvrt.fromStr(n->type)) {
        case Type::file:
            return std::unique_ptr<Node>{new FileNode{n, app}};
        case Type::folder:
        case Type::root:
            return std::unique_ptr<Node>{new FolderNode{n, app}};
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("unreachable")});
}

std::unique_ptr<Node>
Node::create (const Node& node)
{
    if (node._app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    switch (typeCvrt.fromStr(node._data->type)) {
        case Type::file:
            return std::unique_ptr<Node>{new FileNode{node._data, *node._app}};
        case Type::folder:
        case Type::root:
            return std::unique_ptr<Node>{new FolderNode{node._data, *node._app}};
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("unreachable")});
}

Node::Node (std::shared_ptr<data::Node> n, const Application& app)  :
        _data{n}, _app(&app)
{
    _THROW_IF_NO_NODE_;
}

Node::Node(const Node& rhs) :
        _data(nullptr), _app(rhs._app)
{
    if (rhs._data != nullptr)
    {
        _data = std::make_shared<data::Node>(*rhs._data);
    }
    _app = rhs._app;
}

Node&
Node::operator=(const Node& rhs)
{
    if (rhs._data != nullptr)
    {
        _data = std::make_shared<data::Node>(*rhs._data);
    }
    _app = rhs._app;
    return *this;
}

const std::string&
Node::id () const
{
    _THROW_IF_NO_NODE_;
    return _data->id;
}

Node::Type
Node::type () const
{
    _THROW_IF_NO_NODE_;
    return typeCvrt.fromStr(_data->type);
}

const string_t&
Node::name () const
{
    _THROW_IF_NO_NODE_;
    return _data->name;
}

const std::string&
Node::parentId () const
{
    _THROW_IF_NO_NODE_;
    return _data->parentId.get_value_or("");
}

const std::vector<std::string>&
Node::ancestors () const
{
    _THROW_IF_NO_NODE_;
    return _data->ancestors;
}

uint64_t
Node::ownerId () const
{
    _THROW_IF_NO_NODE_;
    return _data->ownerId;
}

std::chrono::system_clock::time_point
Node::creationDate () const
{
    _THROW_IF_NO_NODE_;
    return system_clock::time_point(std::chrono::seconds(_data->creationDate));
}

std::chrono::system_clock::time_point
Node::lastUpdateDate () const
{
    return system_clock::time_point(std::chrono::seconds(_data->lastUpdateDate));
}

uint64_t
Node::nbChildren () const
{
    _THROW_IF_NO_NODE_;
    return _data->nbChildren;
}

uint64_t
Node::nbFiles () const
{
    _THROW_IF_NO_NODE_;
    return _data->nbFiles;
}

uint64_t
Node::size () const
{
    _THROW_IF_NO_NODE_;
    return _data->size;
}

void
Node::remove()
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    _app->api().nodes.deleteNode(id()).get();
    _data->id = "";
}

const utility::string_t&
Node::rename(const string_t& name)
{
    if (name == U("") || name == U(".") || name == U(".."))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Name is not valid")});
    }
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    auto node = _app->api().nodes.renameNode(id(), name).get();
    _data->name = node->name;
    return node->name;
}

std::unique_ptr<core::Node>
Node::moveTo(const FolderNode& node, MergePolicy policy) const
{
    return copyOrMoveTo(node, true, policy);
}

std::unique_ptr<core::Node>
Node::copyTo(const FolderNode& node, MergePolicy policy) const
{
    return copyOrMoveTo(node, false, policy);
}

std::unique_ptr<core::Node>
Node::copyOrMoveTo(const FolderNode& node, bool isMove, MergePolicy policy) const
{
    if (isMove && ownerId() != _app->currentUser().id())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("It is forbidden to move files you do not own")});
    }
    if (node.ownerId() != _app->currentUser().id())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("You must own the destination node")});
    }

    std::string otherNodeKey;
    std::string myNodeKey;
    if (ownerId() != _app->currentUser().id())
    {
        otherNodeKey = _app->getNodeKeyClear(node.ownerId());
        myNodeKey =_app->currentUser().personalData().nodeKeyClear();
    }

    // Do the copy / move
    auto idc = _app->api().nodes.copyNode(id(), node.id(), isMove, mergePolicyCvrt.toStr(policy), myNodeKey, otherNodeKey).get();

    auto i = 0;
    do {
        i++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * i));
        try
        {
            auto copyLog = _app->api().nodes.getCopyLog(id(), node.id()).get();

            return _app->getNodeById(copyLog->mergedWith.get_value_or(copyLog->newId.get_value_or(copyLog->from)));
        }
        catch (const ErrorNotFound& e)
        {
            // ErrorNotFound means the copy is not done yet ; there are expected
            GIGA_DEBUG_LOG(trace, utils::exceptionInfos());
        }
    } while (i <= 30);
    BOOST_THROW_EXCEPTION(ErrorException{U("Timeout while waiting for the node to be copied/moved")});
}

const std::shared_ptr<data::Node>
Node::handle() const
{
    return _data;
}

} /* namespace core */
} /* namespace giga */

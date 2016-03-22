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
#include "../api/data/Node.h"
#include "../api/NodesApi.h"
#include "../rest/HttpErrors.h"
#include "../utils/EnumConvertor.h"
#include "../utils/Utils.h"

#include <boost/filesystem.hpp>
#include <chrono>
#include <string>

using std::chrono::system_clock;
using utility::string_t;

#define _THROW_IF_NO_NODE_ if (_data == nullptr) { BOOST_THROW_EXCEPTION(ErrorException{U("Node is not set")}); } do {} while(0)

namespace giga
{

const utils::EnumConvertor<core::Node::Type, 3> core::Node::typeCvrt =
    {U("root"), U("folder"), U("file")};

const utils::EnumConvertor<core::Node::MediaType, 6> core::Node::mediaTypeCvrt =
    {U("audio"), U("document"), U("video"), U("image"), U("unknown"), U("folder")};

namespace core
{

std::unique_ptr<Node>
Node::create (std::shared_ptr<data::Node> n)
{
    switch (typeCvrt.fromStr(n->type)) {
        case Type::file:
            return std::unique_ptr<Node>{new FileNode{n}};
        case Type::folder:
        case Type::root:
            return std::unique_ptr<Node>{new FolderNode{n}};
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("unreachable")});
}

std::unique_ptr<Node>
Node::create (const Node& node)
{
    switch (typeCvrt.fromStr(node._data->type)) {
        case Type::file:
            return std::unique_ptr<Node>{new FileNode{node._data}};
        case Type::folder:
        case Type::root:
            return std::unique_ptr<Node>{new FolderNode{node._data}};
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("unreachable")});
}

Node::Node (std::shared_ptr<data::Node> n)  :
_data(n)
{
    _THROW_IF_NO_NODE_;
}

Node::Node(const Node& rhs) :
        _data(nullptr)
{
    if (rhs._data != nullptr)
    {
        _data = std::make_shared<data::Node>(*rhs._data);
    }
}

Node&
Node::operator=(const Node& rhs)
{
    if (rhs._data != nullptr)
    {
        _data = std::make_shared<data::Node>(*rhs._data);
    }
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
    std::cout << _data->name << std::endl;
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
    NodesApi::deleteNode(id()).get();
    _data->id = "";
}

const utility::string_t&
Node::rename(const string_t& name)
{
    if (name == U("") || name == U(".") || name == U(".."))
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Name is not valid")});
    }
    auto node = NodesApi::renameNode(id(), name).get();
    _data->name = node->name;
    return node->name;
}

} /* namespace core */
} /* namespace giga */

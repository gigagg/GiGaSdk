/*
 * Node.cpp
 *
 *  Created on: 4 févr. 2016
 *      Author: thomas
 */

#include "Node.h"
#include "FileNode.h"
#include "FolderNode.h"
#include "../api/data/Node.h"
#include "../api/NodesApi.h"
#include "../rest/HttpErrors.h"
#include "../utils/EnumConvertor.h"

#include <boost/filesystem.hpp>
#include <chrono>
#include <string>

using std::chrono::_V2::system_clock;

#define _THROW_IF_NO_NODE_ if (n == nullptr) { THROW(ErrorException{"Node is not set"}); } do {} while(0)

namespace giga
{

const utils::EnumConvertor<core::Node::Type, 3> core::Node::typeCvrt =
    {"root", "folder", "file"};

const utils::EnumConvertor<core::Node::MediaType, 6> core::Node::mediaTypeCvrt =
        {"audio", "document", "video", "image", "unknown", "folder"};

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
    THROW(ErrorException{"unreachable"});
}

Node::Node (std::shared_ptr<data::Node> n)  :
n(n)
{
    _THROW_IF_NO_NODE_;
}

const std::string&
Node::id () const
{
    _THROW_IF_NO_NODE_;
    return n->id;
}

Node::Type
Node::type () const
{
    _THROW_IF_NO_NODE_;
    return typeCvrt.fromStr(n->type);
}

const std::string&
Node::name () const
{
    _THROW_IF_NO_NODE_;
    return n->name;
}

const std::string&
Node::parentId () const
{
    _THROW_IF_NO_NODE_;
    return n->parentId.get_value_or("");
}

const std::vector<std::string>&
Node::ancestors () const
{
    _THROW_IF_NO_NODE_;
    return n->ancestors;
}

int64_t
Node::ownerId () const
{
    _THROW_IF_NO_NODE_;
    return n->ownerId;
}

std::chrono::system_clock::time_point
Node::creationDate () const
{
    _THROW_IF_NO_NODE_;
    return system_clock::time_point(std::chrono::seconds(n->creationDate));
}

std::chrono::system_clock::time_point
Node::lastUpdateDate () const
{
    return system_clock::time_point(std::chrono::seconds(n->lastUpdateDate));
}

uint64_t
Node::nbChildren () const
{
    _THROW_IF_NO_NODE_;
    return n->nbChildren;
}

uint64_t
Node::nbFiles () const
{
    _THROW_IF_NO_NODE_;
    return n->nbFiles;
}

uint64_t
Node::size () const
{
    _THROW_IF_NO_NODE_;
    return n->size;
}

bool
Node::shouldLoadChildren() const
{
    return nbChildren() > 0 && children().size() < nbChildren();
}

void
Node::remove()
{
    NodesApi::deleteNode(id()).get();
    n->id = "";
}

void
Node::rename(const std::string& name)
{
    if (!boost::filesystem::portable_name(name))
    {
        THROW(ErrorException{"Name is not valid"});
    }
    NodesApi::renameNode(id(), name).get();
    n->name = name;
}

} /* namespace core */
} /* namespace giga */

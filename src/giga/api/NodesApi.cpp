/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "NodesApi.h"
#include "data/Node.h"
#include "data/NodeList.h"
#include "data/Preview.h"
#include "data/Timeline.h"
#include "data/DataNode.h"
#include "data/IdContainer.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<NodeList>>
NodesApi::searchNode (const string_t& search, const string_t& mine, const string_t& inFolder, int64_t ownerId)
{
    auto uri = client().uri (U("nodes"));
    uri.append_query (U("search"), search);
    uri.append_query (U("mine"), mine);
    uri.append_query (U("inFolder"), inFolder);
    uri.append_query (U("ownerId"), ownerId);
    return client().request<NodeList> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<data::Node>>>
NodesApi::searchNodeByType (const string_t& search, const string_t& type, uint16_t max, uint32_t offset)
{
    auto uri = client().uri (U("search"), type);
    uri.append_query (U("q"), search);
    uri.append_query (U("max"), max);
    uri.append_query (U("offset"), offset);
    return client().request<std::vector<data::Node>> (methods::GET, uri);}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::addNode (const string_t& name, const string_t& type, const string_t& parentId, const string_t& fkey,
                   const string_t& fid)
{
    auto uri = client().uri (U("nodes"));
    auto body = JsonObj{};
    body.add (U("name"), name);
    body.add (U("type"), type);
    body.add (U("parentId"), parentId);
    body.add (U("fkey"), fkey);
    body.add (U("fid"), fid);
    return client().request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::addFolderNode (const string_t& name, const string_t& parentId)
{
    auto uri = client().uri (U("nodes"));
    auto body = JsonObj{};
    body.add (U("name"), name);
    body.add (U("type"), string_t(U("folder")));
    body.add (U("parentId"), parentId);
    return client().request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::copyNode (const string_t& fromNodeId, const string_t& toNodeId, const string_t& copy, const string_t& cut,
                    const string_t& myNodeKey, const string_t& otherNodeKey)
{
    auto uri = client().uri (U("nodes"), fromNodeId, U("nodes"), toNodeId);
    uri.append_query (U("copy"), copy);
    uri.append_query (U("cut"), cut);
    auto body = JsonObj{};
    body.add (U("myNodeKey"), myNodeKey);
    body.add (U("otherNodeKey"), otherNodeKey);
    return client().request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Node>>
NodesApi::getNodeById (const string_t& nodeId)
{
    auto uri = client().uri (U("nodes"), nodeId);
    return client().request<Node> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
NodesApi::renameNode (const string_t& nodeId, const string_t& name)
{
    auto uri = client().uri (U("nodes"), nodeId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return client().request<Node> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<IdContainer>>
NodesApi::deleteNode (const string_t& nodeId)
{
    auto uri = client().uri (U("nodes"), nodeId);
    return client().request<IdContainer> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<std::vector<Node>>>
NodesApi::getChildrenNode (const string_t& nodeId)
{
    auto uri = client().uri (U("nodes"), nodeId, U("nodes"));
    return client().request<std::vector<Node>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Preview>>
NodesApi::getPreviewsData (const string_t& nodeId)
{
    auto uri = client().uri (U("nodes"), nodeId, U("previews"));
    return client().request<Preview> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Timeline>>
NodesApi::getTimeline (const string_t& head, int64_t from, int64_t owner)
{
    auto uri = client().uri (U("timelines"));
    uri.append_query (U("head"), head);
    uri.append_query (U("from"), from);
    uri.append_query (U("owner"), owner);
    return client().request<Timeline> (methods::GET, uri);
}

} // namespace giga

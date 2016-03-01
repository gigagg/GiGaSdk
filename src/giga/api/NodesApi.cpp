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
    auto uri = client().uri ("nodes");
    uri.append_query ("search", search);
    uri.append_query ("mine", mine);
    uri.append_query ("inFolder", inFolder);
    uri.append_query ("ownerId", ownerId);
    return client().request<NodeList> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<data::Node>>>
NodesApi::searchNodeByType (const string_t& search, const string_t& type, uint16_t max, uint32_t offset)
{
    auto uri = client().uri ("search", type);
    uri.append_query ("q", search);
    uri.append_query ("max", max);
    uri.append_query ("offset", offset);
    return client().request<std::vector<data::Node>> (methods::GET, uri);}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::addNode (const string_t& name, const string_t& type, const string_t& parentId, const string_t& fkey,
                   const string_t& fid)
{
    auto uri = client().uri ("nodes");
    auto body = JsonObj{};
    body.add ("name", name);
    body.add ("type", type);
    body.add ("parentId", parentId);
    body.add ("fkey", fkey);
    body.add ("fid", fid);
    return client().request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::addFolderNode (const string_t& name, const string_t& parentId)
{
    auto uri = client().uri ("nodes");
    auto body = JsonObj{};
    body.add ("name", name);
    body.add ("type", string_t("folder"));
    body.add ("parentId", parentId);
    return client().request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::copyNode (const string_t& fromNodeId, const string_t& toNodeId, const string_t& copy, const string_t& cut,
                    const string_t& myNodeKey, const string_t& otherNodeKey)
{
    auto uri = client().uri ("nodes", fromNodeId, "nodes", toNodeId);
    uri.append_query ("copy", copy);
    uri.append_query ("cut", cut);
    auto body = JsonObj{};
    body.add ("myNodeKey", myNodeKey);
    body.add ("otherNodeKey", otherNodeKey);
    return client().request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Node>>
NodesApi::getNodeById (const string_t& nodeId)
{
    auto uri = client().uri ("nodes", nodeId);
    return client().request<Node> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
NodesApi::renameNode (const string_t& nodeId, const string_t& name)
{
    auto uri = client().uri ("nodes", nodeId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client().request<Node> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<IdContainer>>
NodesApi::deleteNode (const string_t& nodeId)
{
    auto uri = client().uri ("nodes", nodeId);
    return client().request<IdContainer> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<std::vector<Node>>>
NodesApi::getChildrenNode (const string_t& nodeId)
{
    auto uri = client().uri ("nodes", nodeId, "nodes");
    return client().request<std::vector<Node>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Preview>>
NodesApi::getPreviewsData (const string_t& nodeId)
{
    auto uri = client().uri ("nodes", nodeId, "previews");
    return client().request<Preview> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Timeline>>
NodesApi::getTimeline (const string_t& head, int64_t from, int64_t owner)
{
    auto uri = client().uri ("timelines");
    uri.append_query ("head", head);
    uri.append_query ("from", from);
    uri.append_query ("owner", owner);
    return client().request<Timeline> (methods::GET, uri);
}

} // namespace giga

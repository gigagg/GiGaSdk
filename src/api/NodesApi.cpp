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

#include <string>

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<NodeList>>
NodesApi::searchNode (const std::string& search, const std::string& mine, const std::string& inFolder, int64_t ownerId)
{
    auto uri = client.uri ("nodes");
    uri.append_query ("search", search);
    uri.append_query ("mine", mine);
    uri.append_query ("inFolder", inFolder);
    uri.append_query ("ownerId", ownerId);
    return client.request<NodeList> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
NodesApi::addNode (const std::string& name, const std::string& type, const std::string& parentId, const std::string& fkey,
                   const std::string& fid)
{
    auto uri = client.uri ("nodes");
    auto body = JsonObj{};
    body.add ("name", name);
    body.add ("type", type);
    body.add ("parentId", parentId);
    body.add ("fkey", fkey);
    body.add ("fid", fid);
    return client.request<Node> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<DataNode>>
NodesApi::copyNode (const std::string& fromNodeId, const std::string& toNodeId, const std::string& copy, const std::string& cut,
                    const std::string& myNodeKey, const std::string& otherNodeKey)
{
    auto uri = client.uri ("nodes", fromNodeId, "nodes", toNodeId);
    uri.append_query ("copy", copy);
    uri.append_query ("cut", cut);
    auto body = JsonObj{};
    body.add ("myNodeKey", myNodeKey);
    body.add ("otherNodeKey", otherNodeKey);
    return client.request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Node>>
NodesApi::getNodeById (const std::string& nodeId)
{
    auto uri = client.uri ("nodes", nodeId);
    return client.request<Node> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
NodesApi::renameNode (const std::string& nodeId, const std::string& name)
{
    auto uri = client.uri ("nodes", nodeId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client.request<Node> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<IdContainer>>
NodesApi::deleteNode (const std::string& nodeId)
{
    auto uri = client.uri ("nodes", nodeId);
    return client.request<IdContainer> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Node>>
NodesApi::getChildrenNode (const std::string& nodeId, const std::string& search)
{
    auto uri = client.uri ("nodes", nodeId, "nodes");
    uri.append_query ("search", search);
    return client.request<Node> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Preview>>
NodesApi::getPreviewsData (const std::string& nodeId)
{
    auto uri = client.uri ("nodes", nodeId, "previews");
    return client.request<Preview> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Timeline>>
NodesApi::getTimeline (const std::string& head, int64_t from, int64_t owner)
{
    auto uri = client.uri ("timelines");
    uri.append_query ("head", head);
    uri.append_query ("from", from);
    uri.append_query ("owner", owner);
    return client.request<Timeline> (methods::GET, uri);
}

} // namespace giga

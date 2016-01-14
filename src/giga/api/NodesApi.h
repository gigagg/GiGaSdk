/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef CLIENT_NODESAPI_API_H_
#define CLIENT_NODESAPI_API_H_

#include <iosfwd>
#include "GigaApi.h"

namespace giga
{

namespace data {
struct NodeList;
struct Node;
struct DataNode;
struct Preview;
struct Timeline;
struct IdContainer;
}

class NodesApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::NodeList>>
    searchNode (const std::string& search, const std::string& mine, const std::string& inFolder, int64_t ownerId);

    static pplx::task<std::shared_ptr<data::Node>>
    addNode (const std::string& name, const std::string& type, const std::string& parentId, const std::string& fkey,
             const std::string& fid);

    static pplx::task<std::shared_ptr<data::DataNode>>
    copyNode (const std::string& fromNodeId, const std::string& toNodeId, const std::string& copy, const std::string& cut,
              const std::string& myNodeKey, const std::string& otherNodeKey);

    static pplx::task<std::shared_ptr<data::Node>>
    getNodeById (const std::string& nodeId);

    static pplx::task<std::shared_ptr<data::Node>>
    renameNode (const std::string& nodeId, const std::string& name);

    static pplx::task<std::shared_ptr<data::IdContainer>>
    deleteNode (const std::string& nodeId);

    static pplx::task<std::shared_ptr<data::Node>>
    getChildrenNode (const std::string& nodeId, const std::string& search);

    static pplx::task<std::shared_ptr<data::Preview>>
    getPreviewsData (const std::string& nodeId);

    static pplx::task<std::shared_ptr<data::Timeline>>
    getTimeline (const std::string& head, int64_t from, int64_t owner);
};
} // namespace giga

#endif

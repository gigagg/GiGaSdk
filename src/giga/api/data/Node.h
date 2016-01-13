/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_NODE_MODEL_H_
#define GIGA_DATA_NODE_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

/**
 * Either a file or a folder
 */
struct Node {
    std::string id = {};
    std::string name = {};
    boost::optional<std::string> parentId = boost::none;
    std::vector<std::string> ancestors = {};
    std::vector<std::string> servers = {};
    int64_t ownerId = 0;
    int64_t size = 0;
    int64_t creationDate = 0;
    int64_t lastUpdateDate = 0;
    int64_t nbChildren = 0;
    int64_t nbFiles = 0;
    std::vector<std::unique_ptr<Node>> nodes = {};
    std::string type = {};
    boost::optional<std::string> mimeType = boost::none;
    boost::optional<std::string> fid = boost::none;
    boost::optional<std::string> fkey = boost::none;
    boost::optional<int64_t> previewState = boost::none;
    /** DownloadUrl. Concatenate the url encoded nodeKey. */
    boost::optional<std::string> url = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<std::string> icon = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<std::string> square = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<std::string> original = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<std::string> poster = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, name);
        GIGA_MANAGE(m, parentId);
        GIGA_MANAGE(m, ancestors);
        GIGA_MANAGE(m, servers);
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, size);
        GIGA_MANAGE(m, creationDate);
        GIGA_MANAGE(m, lastUpdateDate);
        GIGA_MANAGE(m, nbChildren);
        GIGA_MANAGE(m, nbFiles);
        GIGA_MANAGE(m, nodes);
        GIGA_MANAGE(m, type);
        GIGA_MANAGE(m, mimeType);
        GIGA_MANAGE(m, fid);
        GIGA_MANAGE(m, fkey);
        GIGA_MANAGE(m, previewState);
        GIGA_MANAGE(m, url);
        GIGA_MANAGE(m, icon);
        GIGA_MANAGE(m, square);
        GIGA_MANAGE(m, original);
        GIGA_MANAGE(m, poster);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_NODE_MODEL_H_ */

/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_TIMELINEENTRY_MODEL_H_
#define GIGA_DATA_TIMELINEENTRY_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

/**
 * A list of nodes, grouped by *entries*.
 */
struct TimelineEntry {
    /** Max 6 Files from this entry node list */
    std::vector<std::unique_ptr<Node>> nodes = {};
    /** The id of the user that added these nodes */
    int64_t ownerId = 0;
    int64_t start = 0;
    int64_t end = 0;
    int64_t nbFiles = 0;
    int64_t nbFolders = 0;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, nodes);
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, start);
        GIGA_MANAGE(m, end);
        GIGA_MANAGE(m, nbFiles);
        GIGA_MANAGE(m, nbFolders);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_TIMELINEENTRY_MODEL_H_ */

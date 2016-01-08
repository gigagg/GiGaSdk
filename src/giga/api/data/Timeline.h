/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_TIMELINE_MODEL_H_
#define GIGA_DATA_TIMELINE_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "TimelineEntry.h"

namespace giga {
namespace data {

/**
 * A list of nodes, grouped by *entries*.
 */
struct Timeline {
    std::vector<std::unique_ptr<TimelineEntry>> entries = {};
    int64_t from = 0;
    int64_t to = 0;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, entries);
        GIGA_MANAGE(m, from);
        GIGA_MANAGE(m, to);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_TIMELINE_MODEL_H_ */

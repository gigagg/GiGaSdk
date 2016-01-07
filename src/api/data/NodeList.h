/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_NODELIST_MODEL_H_
#define GIGA_DATA_NODELIST_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

struct NodeList {
    /** index of the first result */
    boost::optional<int64_t> from = boost::none;
    /** nb results */
    boost::optional<int64_t> count = boost::none;
    std::vector<std::unique_ptr<Node>> results = {};
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, from);
        GIGA_MANAGE(m, count);
        GIGA_MANAGE(m, results);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_NODELIST_MODEL_H_ */

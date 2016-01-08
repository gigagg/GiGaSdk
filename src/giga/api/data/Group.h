/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_GROUP_MODEL_H_
#define GIGA_DATA_GROUP_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

struct Group {
    int64_t ownerId = 0;
    std::string name = {};
    std::vector<int64_t>  users = std::vector<int64_t>();
    std::vector<std::unique_ptr<Node>> nodes = {};
    std::string id = {};
    int64_t creationDate = 0;
    boost::optional<bool> isAllMyContacts = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, name);
        GIGA_MANAGE(m, users);
        GIGA_MANAGE(m, nodes);
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, creationDate);
        GIGA_MANAGE(m, isAllMyContacts);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_GROUP_MODEL_H_ */

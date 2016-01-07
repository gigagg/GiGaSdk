/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_DATANODE_MODEL_H_
#define GIGA_DATA_DATANODE_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

struct DataNode {
    std::unique_ptr<Node> data = nullptr;
    
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, data);
        
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_DATANODE_MODEL_H_ */

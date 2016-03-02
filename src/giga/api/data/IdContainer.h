/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_IDCONTAINER_MODEL_H_
#define GIGA_DATA_IDCONTAINER_MODEL_H_

#include <cpprest/details/basic_types.h>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

struct IdContainer {
    utility::string_t id = {};
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_IDCONTAINER_MODEL_H_ */

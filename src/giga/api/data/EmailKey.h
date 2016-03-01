/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_EMAILKEY_MODEL_H_
#define GIGA_DATA_EMAILKEY_MODEL_H_

#include <cpprest/details/basic_types.h>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

struct EmailKey {
    boost::optional<utility::string_t> email = boost::none;
    /** The *fromUserId* nodeKey */
    boost::optional<utility::string_t> key   = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, email);
        GIGA_MANAGE(m, key);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_EMAILKEY_MODEL_H_ */

/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_USEREXISTS_MODEL_H_
#define GIGA_DATA_USEREXISTS_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

namespace giga {
namespace data {

struct UserExists {
    bool exists = false;
    /** The login with the correct case */
    boost::optional<std::string> login = boost::none;
    /** The user id */
    boost::optional<int64_t> id = boost::none;
    /** the big avatar url */
    boost::optional<std::string> avatar = boost::none;
    /** the small avatar url */
    boost::optional<std::string> icon = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, exists);
        GIGA_MANAGE(m, login);
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, avatar);
        GIGA_MANAGE(m, icon);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_USEREXISTS_MODEL_H_ */

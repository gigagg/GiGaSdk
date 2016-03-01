/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_SENTEMAIL_MODEL_H_
#define GIGA_DATA_SENTEMAIL_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "User.h"

namespace giga {
namespace data {

struct SentEmail {
    std::unique_ptr<User> fromUser = nullptr;
    boost::optional<utility::string_t> toEmail = boost::none;
    boost::optional<utility::string_t> key = boost::none;
    boost::optional<int64_t> id = boost::none;
    boost::optional<bool> isAccepted = boost::none;
    boost::optional<int64_t> creationDate = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, fromUser);
        GIGA_MANAGE(m, toEmail);
        GIGA_MANAGE(m, key);
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, isAccepted);
        GIGA_MANAGE(m, creationDate);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_SENTEMAIL_MODEL_H_ */

/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GIGA_DATA_UsersRelation_MODEL_H_
#define GIGA_DATA_UsersRelation_MODEL_H_

#include <cpprest/details/basic_types.h>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "User.h"

namespace giga {
namespace data {

/**
 * An edge in the User graph
 */
struct UsersRelation {
    std::shared_ptr<User> user = nullptr;
    utility::string_t type = {};
    uint64_t creationDate = 0;
    /** type=CONTACT */
    boost::optional<bool> favorite = boost::none;
    /** type=CONTACT */
    boost::optional<bool> isNew = boost::none;
    /** type=CONTACT */
    boost::optional<utility::string_t> key = boost::none;
    /** type=CONTACT, OTHER|ME */
    boost::optional<utility::string_t> initiator = boost::none;
    /** type=CONTACT|INVITE, list of userId */
    boost::optional<std::vector<uint64_t> > suggestedBy = boost::none;
    /** type=SHOULD_INVITE, list of userId */
    boost::optional<std::vector<uint64_t> > byUserId = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, user);
        GIGA_MANAGE(m, type);
        GIGA_MANAGE(m, creationDate);
        GIGA_MANAGE(m, favorite);
        GIGA_MANAGE(m, isNew);
        GIGA_MANAGE(m, key);
        GIGA_MANAGE(m, initiator);
        GIGA_MANAGE(m, suggestedBy);
        GIGA_MANAGE(m, byUserId);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_UsersRelation_MODEL_H_ */

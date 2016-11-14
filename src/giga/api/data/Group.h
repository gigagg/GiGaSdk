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

#ifndef GIGA_DATA_GROUP_MODEL_H_
#define GIGA_DATA_GROUP_MODEL_H_

#include <cpprest/details/basic_types.h>
#include "../../rest/prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

struct Group {
    uint64_t              ownerId = 0;
    utility::string_t     name = {};
    std::vector<uint64_t> users = std::vector<uint64_t>();
    std::vector<Node>     nodes = {};
    utility::string_t     id = {};
    uint64_t              creationDate = 0;
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

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

#ifndef GIGA_DATA_USEREXISTS_MODEL_H_
#define GIGA_DATA_USEREXISTS_MODEL_H_

#include <cpprest/details/basic_types.h>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

struct UserExists {
    bool exists = false;
    /** The login with the correct case */
    boost::optional<utility::string_t> login = boost::none;
    /** The user id */
    boost::optional<uint64_t> id = boost::none;
    /** the big avatar url */
    boost::optional<utility::string_t> avatar = boost::none;
    /** the small avatar url */
    boost::optional<utility::string_t> icon = boost::none;
    
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

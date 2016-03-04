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
    boost::optional<uint64_t> id = boost::none;
    boost::optional<bool> isAccepted = boost::none;
    boost::optional<uint64_t> creationDate = boost::none;
    
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

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

#ifndef GIGA_DATA_EXTERNALACCOUNT_MODEL_H_
#define GIGA_DATA_EXTERNALACCOUNT_MODEL_H_

#include <boost/optional.hpp>
#include <cpprest/details/basic_types.h>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

/**
 * A user account on an other service (ex: Facebook)
 */
struct ExternalAccount {
    utility::string_t externalId = {};
    utility::string_t providerName = {};
    uint64_t userId = 0;
    boost::optional<utility::string_t> email = boost::none;
    boost::optional<utility::string_t> fullname = boost::none;
    boost::optional<utility::string_t> profileUrl = boost::none;
    uint64_t creationDate = 0;
    uint64_t updateDate = 0;
    boost::optional<utility::string_t> avatarUrl = boost::none;
    boost::optional<utility::string_t> displayName = boost::none;
    
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, externalId);
        GIGA_MANAGE(m, providerName);
        GIGA_MANAGE(m, userId);
        GIGA_MANAGE(m, email);
        GIGA_MANAGE(m, fullname);
        GIGA_MANAGE(m, profileUrl);
        GIGA_MANAGE(m, creationDate);
        GIGA_MANAGE(m, updateDate);
        GIGA_MANAGE(m, avatarUrl);
        GIGA_MANAGE(m, displayName);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_EXTERNALACCOUNT_MODEL_H_ */

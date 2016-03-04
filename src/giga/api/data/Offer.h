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

#ifndef GIGA_DATA_OFFER_MODEL_H_
#define GIGA_DATA_OFFER_MODEL_H_

#include <boost/optional.hpp>
#include <cpprest/details/basic_types.h>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

/**
 * Something you can buy
 */
struct Offer {
    utility::string_t id          = {};
    utility::string_t state       = {};
    utility::string_t duration    = {};
    uint64_t storage              = 0;
    uint64_t download             = 0;
    utility::string_t resetDownload = {};
    utility::string_t billing     = {};
    uint64_t costEUR              = 0;
    uint64_t costUSD              = 0;
    boost::optional<uint64_t> costStar = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, state);
        GIGA_MANAGE(m, duration);
        GIGA_MANAGE(m, storage);
        GIGA_MANAGE(m, download);
        GIGA_MANAGE(m, resetDownload);
        GIGA_MANAGE(m, billing);
        GIGA_MANAGE(m, costEUR);
        GIGA_MANAGE(m, costUSD);
        GIGA_MANAGE(m, costStar);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_OFFER_MODEL_H_ */

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

#ifndef GIGA_DATA_COPYLOG_MODEL_H_
#define GIGA_DATA_COPYLOG_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

struct CopyLog {

    std::string from    = {};
    std::string to      = {};
    uint64_t ownerId    = 0ul;
    uint64_t nbFiles    = 0ul;
    uint64_t time       = 0ul;
    bool isCopy         = false;
    boost::optional<std::string> mergedWith = {};
    boost::optional<std::string> newId      = {};
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, from);
        GIGA_MANAGE(m, to);
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, nbFiles);
        GIGA_MANAGE(m, isCopy);
        GIGA_MANAGE(m, mergedWith);
        GIGA_MANAGE(m, newId);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_COPYLOG_MODEL_H_ */

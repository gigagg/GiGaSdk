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

#ifndef GIGA_DATA_SMALL_NODE_MODEL_H_
#define GIGA_DATA_SMALL_NODE_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

struct SmallNode {

    std::string       id = {};
    utility::string_t name = {};
    uint64_t          size = {};
    utility::string_t mimeType = {};
    std::string       fid = {};
    std::string       url = {};
    uint64_t          ownerId = {};
    uint64_t          previewState = {};
//    utility::string_t icon = {};
//    utility::string_t square = {};
//    utility::string_t original = {};
//    utility::string_t poster = {};
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, name);
        GIGA_MANAGE(m, size);
        GIGA_MANAGE(m, mimeType);
        GIGA_MANAGE(m, fid);
        GIGA_MANAGE(m, url);
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, previewState);
//        GIGA_MANAGE(m, icon);
//        GIGA_MANAGE(m, square);
//        GIGA_MANAGE(m, original);
//        GIGA_MANAGE(m, poster);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_SMALL_NODE_MODEL_H_ */

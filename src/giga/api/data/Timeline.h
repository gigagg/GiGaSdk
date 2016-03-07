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

#ifndef GIGA_DATA_TIMELINE_MODEL_H_
#define GIGA_DATA_TIMELINE_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "TimelineEntry.h"

namespace giga {
namespace data {

/**
 * A list of nodes, grouped by *entries*.
 */
struct Timeline {
    std::vector<std::unique_ptr<TimelineEntry>> entries = {};
    uint64_t from = 0;
    uint64_t to = 0;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, entries);
        GIGA_MANAGE(m, from);
        GIGA_MANAGE(m, to);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_TIMELINE_MODEL_H_ */

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

#ifndef GIGA_DATA_TIMELINEENTRY_MODEL_H_
#define GIGA_DATA_TIMELINEENTRY_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "Node.h"

namespace giga {
namespace data {

/**
 * A list of nodes, grouped by *entries*.
 */
struct TimelineEntry {
    /** Max 6 Files from this entry node list */
    std::vector<std::unique_ptr<Node>> nodes = {};
    /** The id of the user that added these nodes */
    uint64_t ownerId = 0;
    uint64_t start = 0;
    uint64_t end = 0;
    uint64_t nbFiles = 0;
    uint64_t nbFolders = 0;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, nodes);
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, start);
        GIGA_MANAGE(m, end);
        GIGA_MANAGE(m, nbFiles);
        GIGA_MANAGE(m, nbFolders);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_TIMELINEENTRY_MODEL_H_ */

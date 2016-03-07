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

#ifndef GIGA_DATA_NODE_MODEL_H_
#define GIGA_DATA_NODE_MODEL_H_

#include <boost/optional.hpp>
#include <cpprest/details/basic_types.h>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

/**
 * Either a file or a folder
 */
struct Node {
    std::string id = {};
    utility::string_t name = {};
    boost::optional<std::string> parentId = boost::none;
    std::vector<std::string> ancestors = {};
    std::vector<std::string> servers = {};
    uint64_t ownerId = 0;
    uint64_t size = 0;
    uint64_t creationDate = 0;
    uint64_t lastUpdateDate = 0;
    uint64_t nbChildren = 0;
    uint64_t nbFiles = 0;
    std::vector<std::shared_ptr<Node>> nodes = {};
    utility::string_t type = {};
    boost::optional<utility::string_t> mimeType = boost::none;
    boost::optional<std::string> fid = boost::none;
    boost::optional<std::string> fkey = boost::none;
    boost::optional<int64_t> previewState = boost::none;
    /** DownloadUrl. Concatenate the url encoded nodeKey. */
    boost::optional<utility::string_t> url = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<utility::string_t> icon = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<utility::string_t> square = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<utility::string_t> original = boost::none;
    /** Concatenate the url encoded nodeKey. */
    boost::optional<utility::string_t> poster = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, name);
        GIGA_MANAGE(m, parentId);
        GIGA_MANAGE(m, ancestors);
        GIGA_MANAGE(m, servers);
        GIGA_MANAGE(m, ownerId);
        GIGA_MANAGE(m, size);
        GIGA_MANAGE(m, creationDate);
        GIGA_MANAGE(m, lastUpdateDate);
        GIGA_MANAGE(m, nbChildren);
        GIGA_MANAGE(m, nbFiles);
        GIGA_MANAGE(m, nodes);
        GIGA_MANAGE(m, type);
        GIGA_MANAGE(m, mimeType);
        GIGA_MANAGE(m, fid);
        GIGA_MANAGE(m, fkey);
        GIGA_MANAGE(m, previewState);
        GIGA_MANAGE(m, url);
        GIGA_MANAGE(m, icon);
        GIGA_MANAGE(m, square);
        GIGA_MANAGE(m, original);
        GIGA_MANAGE(m, poster);
    }

//    ~Node() {
//            std::cout << U("DELETE NODE ")<< id << std::endl;
//        };
//    Node () = default;
//    Node (const Node&) = default;
//    Node (Node&&) = default;
//    Node& operator= (Node&&) = default;
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_NODE_MODEL_H_ */

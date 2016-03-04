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

#ifndef CLIENT_GROUPSAPI_API_H_
#define CLIENT_GROUPSAPI_API_H_

#include <cpprest/details/basic_types.h>
#include "GigaApi.h"

namespace giga
{
struct Empty;

namespace data {
struct Group;
}

class GroupsApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::Group>>
    getAllGroups ();

    static pplx::task<std::shared_ptr<data::Group>>
    createGroup (const utility::string_t& name);

    static pplx::task<std::shared_ptr<data::Group>>
    getGroupById (int64_t groupId);

    static pplx::task<std::shared_ptr<data::Group>>
    updateGroupName (int64_t groupId, const utility::string_t& name);

    static pplx::task<std::shared_ptr<Empty>>
    deleteGroup (int64_t groupId);

    static pplx::task<std::shared_ptr<data::Group>>
    shareNodeWithGroup (int64_t groupId, const utility::string_t& nodeId);

    static pplx::task<std::shared_ptr<data::Group>>
    unshareNodeFromGroup (int64_t groupId, const utility::string_t& nodeId);

    static pplx::task<std::shared_ptr<data::Group>>
    addUserToGroup (int64_t groupId, uint64_t userId);

    static pplx::task<std::shared_ptr<data::Group>>
    removeUserFromGroup (int64_t groupId, uint64_t userId);
};
} // namespace giga

#endif

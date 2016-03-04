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

#ifndef CLIENT_NETWORKAPI_API_H_
#define CLIENT_NETWORKAPI_API_H_

#include <cpprest/details/basic_types.h>
#include "GigaApi.h"

namespace giga
{

namespace data {
struct UsersRelation;
struct Success;
}

class NetworkApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::UsersRelation>>
    updateUserRelationData (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type, bool favorite, bool isNew);

    static pplx::task<std::shared_ptr<data::UsersRelation>>
    createUserRelation (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type, const utility::string_t& medium,
                            const std::string& key);

    static pplx::task<std::shared_ptr<data::Success>>
    deleteUserRelation (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type);

    static pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::UsersRelation>>>>
    getUserRelation (uint64_t userId, const utility::string_t& type, const utility::string_t& way);

    static pplx::task<std::shared_ptr<data::UsersRelation>>
    getUserRelation (uint64_t fromId, const utility::string_t& type, uint64_t toId, const utility::string_t& way);
};
} // namespace giga

#endif

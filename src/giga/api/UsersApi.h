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

#ifndef CLIENT_USERSAPI_API_H_
#define CLIENT_USERSAPI_API_H_

#include <cpprest/details/basic_types.h>
#include "GigaApi.h"

namespace giga
{

namespace data {
struct User;
struct UserExists;
}

class UsersApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::User>>
    getCurrentUser ();

    static pplx::task<std::shared_ptr<std::vector<utility::string_t>>>
    searchTag (const utility::string_t& name);

    static pplx::task<std::shared_ptr<data::UserExists>>
    userExists (const utility::string_t& login, const utility::string_t& email = U(""));

    static pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
    searchUsers (const utility::string_t& search, const utility::string_t& activity, const utility::string_t& isSeeder);

    static pplx::task<std::shared_ptr<data::User>>
    getUserById (uint64_t userId);

    static pplx::task<std::shared_ptr<data::User>>
    getUserByLogin (const utility::string_t& login);

    static pplx::task<std::shared_ptr<data::User>>
    updateUser (uint64_t userId, const utility::string_t& email, bool isValidation, const utility::string_t& gender, const utility::string_t& name,
                const utility::string_t& description, const utility::string_t& birthdate, const utility::string_t& avatar, const utility::string_t& currentPassword,
                const utility::string_t& password, const utility::string_t& clue, const utility::string_t& privateKey, const utility::string_t& iv,
                const utility::string_t& salt);

    static pplx::task<std::shared_ptr<data::User>>
    updateUserAddTag (uint64_t userId, const utility::string_t& name);

    static pplx::task<std::shared_ptr<data::User>>
    updateUserRemoveTag (uint64_t userId, const utility::string_t& name);
};
} // namespace giga

#endif

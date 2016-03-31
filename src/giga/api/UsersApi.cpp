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

#include "GigaApi.h"
#include "data/User.h"
#include "data/UserExists.h"
#include "../utils/Crypto.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<User>>
GigaApi::UsersApi::getCurrentUser () const
{
    auto uri = api.client.uri (U("me"));
    return api.client.request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<string_t>>>
GigaApi::UsersApi::searchTag (const string_t& name) const
{
    auto uri = api.client.uri (U("tags"));
    uri.append_query (U("name"), name);
    return api.client.request<std::vector<string_t>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<UserExists>>
GigaApi::UsersApi::userExists (const string_t& login, const string_t& email /* = U("")*/) const
{
    auto uri = api.client.uri (U("userexists"));
    if (!login.empty()) {
        uri.append_query (U("login"), login);
    }
    if (!email.empty()) {
        uri.append_query (U("email"), email);
    }
    return api.client.request<UserExists> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
GigaApi::UsersApi::searchUsers (const string_t& search, const string_t& activity, const string_t& isSeeder) const
{
    auto uri = api.client.uri (U("users"));
    uri.append_query (U("search"), search);
    if (activity != U(""))
    {
        uri.append_query (U("activity"), activity);
    }
    if (isSeeder != U(""))
    {
        uri.append_query (U("isSeeder"), isSeeder);
    }
    return api.client.request<std::vector<std::shared_ptr<data::User>>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
GigaApi::UsersApi::getUserById (uint64_t userId) const
{
    auto uri = api.client.uri (U("users"), userId);
    return api.client.request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
GigaApi::UsersApi::getUserByLogin (const string_t& login) const
{
    auto uri = api.client.uri (U("users"));
    uri.append_query(U("login"), login);
    return api.client.request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
GigaApi::UsersApi::updateUser (uint64_t userId, const string_t& email, bool isValidation, const string_t& gender, const string_t& name,
                      const string_t& description, const string_t& birthdate, const string_t& avatar,
                      const string_t& currentPassword, const string_t& password, const string_t& clue,
                      const string_t& privateKey, const string_t& iv, const string_t& salt) const
{
    auto uri = api.client.uri (U("users"), userId);
    auto body = JsonObj{};
    body.add (U("email"), email);
    body.add (U("isValidation"), isValidation);
    body.add (U("gender"), gender);
    body.add (U("name"), name);
    body.add (U("description"), description);
    body.add (U("birthdate"), birthdate);
    body.add (U("avatar"), avatar);
    body.add (U("currentPassword"), currentPassword);
    body.add (U("password"), password);
    body.add (U("clue"), clue);
    body.add (U("privateKey"), privateKey);
    body.add (U("iv"), iv);
    body.add (U("salt"), salt);
    return api.client.request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
GigaApi::UsersApi::updateUserAddTag (uint64_t userId, const string_t& name) const
{
    auto uri = api.client.uri (U("users"), userId, U("tags"), userId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return api.client.request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
GigaApi::UsersApi::updateUserRemoveTag (uint64_t userId, const string_t& name) const
{
    auto uri = api.client.uri (U("users"), userId, U("tags"), userId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return api.client.request<User> (methods::DEL, uri, std::move(body));
}

} // namespace giga

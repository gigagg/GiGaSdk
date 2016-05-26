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
#include "data/UsersRelation.h"
#include "data/Success.h"
#include "../utils/Utils.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<UsersRelation>>
GigaApi::NetworkApi::updateUserRelationData (uint64_t fromUserId, uint64_t toUserId, const string_t& type, bool favorite, bool isNew) const
{
    auto uri = api._client.uri (U("users"), fromUserId, U("users"), toUserId);
    auto body = JsonObj{};
    body.add (U("type"), type);
    body.add (U("favorite"), favorite);
    body.add (U("isNew"), isNew);
    return api._client.request<UsersRelation> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<UsersRelation>>
GigaApi::NetworkApi::createUserRelation (uint64_t fromUserId, uint64_t toUserId, const string_t& type, const string_t& medium,
                                    const std::string& key) const
{
    auto uri = api._client.uri (U("users"), fromUserId, U("users"), toUserId);
    auto body = JsonObj{};
    body.add (U("type"), type);
    body.add (U("medium"), medium);
    body.add (U("key"), utils::str2wstr(key));
    return api._client.request<UsersRelation> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Success>>
GigaApi::NetworkApi::deleteUserRelation (uint64_t fromUserId, uint64_t toUserId, const string_t& type) const
{
    auto uri = api._client.uri (U("users"), fromUserId, U("users"), toUserId);
    uri.append_query (U("type"), type);
    return api._client.request<Success> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<UsersRelation>>>>
GigaApi::NetworkApi::getUserRelation (uint64_t userId, const string_t& type, const string_t& way) const
{
    auto uri = api._client.uri (U("users"), userId, U("users"));
    uri.append_query (U("type"), type);
    uri.append_query (U("way"), way);
    return api._client.request<std::vector<std::shared_ptr<UsersRelation>>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<data::UsersRelation>>
GigaApi::NetworkApi::getUserRelation (uint64_t fromId, const utility::string_t& type, uint64_t toId, const utility::string_t& way) const
{
    auto uri = api._client.uri (U("users"), fromId, U("users"), toId);
    if (type != U(""))
    {
        uri.append_query (U("type"), type);
    }
    uri.append_query (U("way"), way);
    return api._client.request<UsersRelation> (methods::GET, uri);
}

} // namespace giga

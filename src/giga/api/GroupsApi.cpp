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
#include "data/Group.h"
#include "../rest/Empty.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::getAllGroups () const
{
    auto uri = api.client.uri (U("groups"));
    return api.client.request<Group> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::createGroup (const string_t& name) const
{
    auto uri = api.client.uri (U("groups"));
    auto body = JsonObj{};
    body.add (U("name"), name);
    return api.client.request<Group> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::getGroupById (int64_t groupId) const
{
    auto uri = api.client.uri (U("groups"), groupId);
    return api.client.request<Group> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::updateGroupName (int64_t groupId, const string_t& name) const
{
    auto uri = api.client.uri (U("groups"), groupId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return api.client.request<Group> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<Empty>>
GigaApi::GroupsApi::deleteGroup (int64_t groupId) const
{
    auto uri = api.client.uri (U("groups"), groupId);
    return api.client.request<Empty> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::shareNodeWithGroup (int64_t groupId, const string_t& nodeId) const
{
    auto uri = api.client.uri (U("groups"), groupId, U("nodes"), nodeId);
    return api.client.request<Group> (methods::PUT, uri);
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::unshareNodeFromGroup (int64_t groupId, const string_t& nodeId) const
{
    auto uri = api.client.uri (U("groups"), groupId, U("nodes"), nodeId);
    return api.client.request<Group> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::addUserToGroup (int64_t groupId, uint64_t userId) const
{
    auto uri = api.client.uri (U("groups"), groupId, U("users"), userId);
    return api.client.request<Group> (methods::PUT, uri);
}

pplx::task<std::shared_ptr<Group>>
GigaApi::GroupsApi::removeUserFromGroup (int64_t groupId, uint64_t userId) const
{
    auto uri = api.client.uri (U("groups"), groupId, U("users"), userId);
    return api.client.request<Group> (methods::DEL, uri);
}

} // namespace giga

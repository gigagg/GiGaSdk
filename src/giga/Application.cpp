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

#include "Application.h"
#include "core/User.h"
#include "api/UsersApi.h"
#include "api/NetworkApi.h"
#include "api/NodesApi.h"
#include "api/data/User.h"
#include "api/data/UsersRelation.h"

#include <cpprest/http_client.h>
#include <algorithm>

using pplx::create_task;
using utility::string_t;

namespace {
std::vector<giga::core::User>
toUserVect(std::shared_ptr<std::vector<std::shared_ptr<giga::data::UsersRelation>>> rels)
{
    auto container = std::vector<giga::core::User>{};
    container.reserve(rels->size());
    for(auto relation : *rels)
    {
        container.emplace_back(relation->user, relation);
    }
    return std::move(container);
}
}

namespace giga
{

Application&
Application::init(string_t&& appRedirectUri, string_t&& appId, string_t&& appKey, string_t&& appScope)
{
    auto& i = instance();
    i._config._appRedirectUri = appRedirectUri;
    i._config._appId          = appId;
    i._config._appKey         = appKey;
    i._config._appScope       = appScope;
    i._isInitialized = true;
    return i;
}

Application&
Application::get()
{
    if (!instance().isInitialized())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("App must be initialized first")});
    }
    return instance();
}

Config&
Application::config ()
{
    return get()._config;
}

bool
Application::isInitialized() const
{
    return _isInitialized;
}

core::User&
Application::authenticate (const string_t& login, const string_t& password)
{
    auto duser = GigaApi::authenticate(login, password).get();
    _currentUser = core::User{duser};
    _currentUser.initializePersonalData(password);
    return _currentUser;
}

core::User&
Application::currentUser()
{
    return _currentUser;
}

//
// users
//

core::User
Application::getUserById (uint64_t id) const
{
    auto duser =  UsersApi::getUserById(id).get();
    return core::User{duser};
}

core::User
Application::getUserByLogin (const string_t& login) const
{
    auto duser =  UsersApi::getUserByLogin(login).get();
    return core::User{duser};
}

core::User
Application::getContact (uint64_t id) const
{
    bool shouldLoadContact = true;
    if (_contacts.size() == 0)
    {
        shouldLoadContact = false;
        getContacts(); // update the _contacts cache variable
    }
    auto it = _contacts.find(id);
    if (it != _contacts.end())
    {
        return it->second;
    }

    if (shouldLoadContact)
    {
        auto relation = NetworkApi::getUserRelation(_currentUser.id(), U("CONTACT"), id, U("OUT")).get();
        if (relation->type == U("CONTACT"))
        {
            _contacts.emplace(id, core::User{relation->user, relation});
            return core::User{relation->user, relation};
        }
    }
    BOOST_THROW_EXCEPTION(ErrorNotFound{U("Not Found")});
}

std::vector<core::User>
Application::getContacts () const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), U("CONTACT"), U("OUT")).get();
    auto contacts = toUserVect(rels);

    _contacts.clear();
    std::transform(contacts.begin(), contacts.end(), std::inserter(_contacts, _contacts.end()), [](const core::User& user) {
        return std::make_pair(user.id(), user);
    });
    return contacts;
}

std::vector<core::User>
Application::getInvitedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), U("INVITE"), U("OUT")).get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getInvitingUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), U("INVITE"), U("IN")).get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getSuggestedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), U("SHOULD_INVITE"), U("OUT")).get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getBlockedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), U("BLOCK"), U("OUT")).get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::searchUser (const string_t& search) const
{
    auto results = UsersApi::searchUsers(search, U("") , U("")).get();

    std::vector<core::User> users{};
    users.resize(results->size());
    std::transform (results->begin(), results->end(), users.begin(), [](const std::shared_ptr<data::User>& data) {
        return core::User{data};
    });

    return users;
}

//
// Nodes
//

std::unique_ptr<core::Node>
Application::getNodeById (const std::string& id) const
{
    auto result = NodesApi::getNodeById(id).get();
    return core::Node::create(result);
}

std::vector<std::unique_ptr<core::Node>>
Application::searchNode (const string_t& search, core::Node::MediaType type) const
{
    auto results = NodesApi::searchNodeByType(search, core::Node::mediaTypeCvrt.toStr(type)).get();
    std::vector<std::unique_ptr<core::Node>> nodes{};
    nodes.resize(results->size());
    std::transform (results->begin(), results->end(), nodes.begin(), [](const data::Node& data) {
        return core::Node::create(std::make_shared<data::Node>(data));
    });

    return nodes;
}


//
// Crypto. Be carful with these ...
//

utility::string_t
Application::getNodeKeyClear(uint64_t userId) const
{
    if (userId == _currentUser.id())
    {
        return _currentUser.personalData().nodeKeyClear();
    }
    return getContact(userId).relation().nodeKeyClear();
}

} /* namespace giga */

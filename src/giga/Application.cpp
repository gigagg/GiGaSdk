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
#include "version.h"
#include "core/User.h"
#include "api/data/User.h"
#include "api/data/UsersRelation.h"

#include <cpprest/http_client.h>
#include <algorithm>

using pplx::create_task;
using utility::string_t;

namespace {
std::vector<giga::core::User>
toUserVect(std::shared_ptr<std::vector<std::shared_ptr<giga::data::UsersRelation>>> rels, const giga::Application& app)
{
    auto container = std::vector<giga::core::User>{};
    container.reserve(rels->size());
    for(auto relation : *rels)
    {
        container.emplace_back(relation->user, app, relation);
    }
    return container;
}
}

namespace giga
{

Application::Application() :
        _api{}, _currentUser{nullptr}, _userAgent{GIGA_UA}, _contacts{}
{
}

core::User&
Application::authenticate (const string_t& login, const string_t& password)
{
    auto duser = _api.authenticate(login, password).get();
    _currentUser = std::unique_ptr<core::User>(new core::User{duser, *this});
    _currentUser->initializePersonalData(password);
    return *_currentUser;
}

bool
Application::isAuthenticated() const
{
    return _currentUser != nullptr;
}

const core::User&
Application::currentUser() const
{
    if (_currentUser == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("User is null")});
    }
    return *_currentUser;
}

core::User&
Application::mutableCurrentUser()
{
    return *_currentUser;
}

//
// users
//

core::User
Application::getUserById (uint64_t id) const
{
    auto duser =  _api.users.getUserById(id).get();
    return core::User{duser, *this};
}

core::User
Application::getUserByLogin (const string_t& login) const
{
    auto duser =  _api.users.getUserByLogin(login).get();
    return core::User{duser, *this};
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
        auto relation = _api.network.getUserRelation(currentUser().id(), U("CONTACT"), id, U("OUT")).get();
        if (relation->type == U("CONTACT"))
        {
            _contacts.emplace(id, core::User{relation->user, *this, relation});
            return core::User{relation->user, *this, relation};
        }
    }
    BOOST_THROW_EXCEPTION(ErrorNotFound{U("Not Found")});
}

std::vector<core::User>
Application::getContacts () const
{
    auto rels = _api.network.getUserRelation(currentUser().id(), U("CONTACT"), U("OUT")).get();
    auto contacts = toUserVect(rels, *this);

    _contacts.clear();
    std::transform(contacts.begin(), contacts.end(), std::inserter(_contacts, _contacts.end()), [](const core::User& user) {
        return std::make_pair(user.id(), user);
    });
    return contacts;
}

std::vector<core::User>
Application::getInvitedUsers() const
{
    auto rels = _api.network.getUserRelation(currentUser().id(), U("INVITE"), U("OUT")).get();
    return toUserVect(rels, *this);
}

std::vector<core::User>
Application::getInvitingUsers() const
{
    auto rels = _api.network.getUserRelation(currentUser().id(), U("INVITE"), U("IN")).get();
    return toUserVect(rels, *this);
}

std::vector<core::User>
Application::getSuggestedUsers() const
{
    auto rels = _api.network.getUserRelation(currentUser().id(), U("SHOULD_INVITE"), U("OUT")).get();
    return toUserVect(rels, *this);
}

std::vector<core::User>
Application::getBlockedUsers() const
{
    auto rels = _api.network.getUserRelation(currentUser().id(), U("BLOCK"), U("OUT")).get();
    return toUserVect(rels, *this);
}

std::vector<core::User>
Application::searchUser (const string_t& search) const
{
    auto results = _api.users.searchUsers(search, U("") , U("")).get();

    std::vector<core::User> users{};
    users.resize(results->size());
    auto& app = *this;
    std::transform (results->begin(), results->end(), users.begin(), [&app](const std::shared_ptr<data::User>& data) {
        return core::User{data, app};
    });

    return users;
}

//
// Nodes
//

std::unique_ptr<core::Node>
Application::getNodeById (const std::string& id) const
{
    auto result = _api.nodes.getNodeById(id).get();
    return core::Node::create(result, *this);
}

std::vector<std::unique_ptr<core::Node>>
Application::searchNode (const string_t& search, core::Node::MediaType type) const
{
    auto results = _api.nodes.searchNodeByType(search, core::Node::mediaTypeCvrt.toStr(type)).get();
    std::vector<std::unique_ptr<core::Node>> nodes{};
    nodes.resize(results->size());
    std::transform (results->begin(), results->end(), nodes.begin(), [this](const data::Node& data) {
        return core::Node::create(std::make_shared<data::Node>(data), *this);
    });

    return nodes;
}

//
// Misc
//

const std::string&
Application::userAgent() const
{
    return _userAgent;
}

void
Application::setUserAgent(std::string&& userAgent)
{
    _userAgent = userAgent;
    _api.setUserAgent(utils::str2wstr(userAgent));
}

const GigaApi&
Application::api () const
{
    return _api;
}

GigaApi&
Application::mutableApi ()
{
    return _api;
}

//
// Crypto. Be carful with these ...
//

std::string
Application::getNodeKeyClear(uint64_t userId) const
{
    if (userId == currentUser().id())
    {
        return currentUser().personalData().nodeKeyClear();
    }
    return getContact(userId).relation().nodeKeyClear(currentUser());
}

} /* namespace giga */



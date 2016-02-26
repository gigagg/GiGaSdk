/*
 * Application.cpp
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
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

namespace giga
{

Application&
Application::init(std::string&& appRedirectUri, std::string&& appId, std::string&& appKey, std::string&& appScope)
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
        BOOST_THROW_EXCEPTION(ErrorException{"App must be initialized first"});
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
Application::authenticate (const std::string& login, const std::string& password)
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
Application::getUserById (int64_t id) const
{
    auto duser =  UsersApi::getUserById(id).get();
    return core::User{duser};
}

core::User
Application::getUserByLogin (const std::string& login) const
{
    auto duser =  UsersApi::getUserByLogin(login).get();
    return core::User{duser};
}

std::vector<core::User>
toUserVect(std::shared_ptr<std::vector<std::shared_ptr<data::UsersRelation>>> rels)
{
    auto container = std::vector<core::User>{};
    container.reserve(rels->size());
    for(auto relation : *rels)
    {
        container.emplace_back(relation->user, relation);
    }
    return std::move(container);
}

std::vector<core::User>
Application::getContacts () const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "CONTACT", "OUT").get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getInvitedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "INVITE", "OUT").get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getInvitingUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "INVITE", "IN").get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getSuggestedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "SHOULD_INVITE", "OUT").get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::getBlockedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "BLOCK", "OUT").get();
    return toUserVect(rels);
}

std::vector<core::User>
Application::searchUser (const std::string& search) const
{
    auto results = UsersApi::searchUsers(search, "" , "").get();

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
Application::searchNode (const std::string& search, core::Node::MediaType type) const
{
    auto results = NodesApi::searchNodeByType(search, core::Node::mediaTypeCvrt.toStr(type)).get();
    std::vector<std::unique_ptr<core::Node>> nodes{};
    nodes.resize(results->size());
    std::transform (results->begin(), results->end(), nodes.begin(), [](const data::Node& data) {
        return core::Node::create(std::make_shared<data::Node>(data));
    });

    return nodes;
}


} /* namespace giga */

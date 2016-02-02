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
#include "api/data/User.h"
#include "api/data/UsersRelation.h"

#include <cpprest/http_client.h>

using giga::data::UsersRelation;
using pplx::create_task;
using giga::core::User;

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
        THROW(ErrorException{"App must be initialized first"});
    }
    return instance();
}

Config&
Application::config ()
{
    return _config;
}

bool
Application::isInitialized() const
{
    return _isInitialized;
}


User&
Application::authenticate (const std::string& login, const std::string& password)
{
    auto duser = GigaApi::authenticate(login, password).get();
    _currentUser = User{duser};
    _currentUser.initializePrivateData(password);
    return _currentUser;
}

User
Application::getUserById (int64_t id) const
{
    auto duser =  UsersApi::getUserById(id).get();
    return User{duser};
}

User
Application::getUserByLogin (const std::string& login) const
{
    auto duser =  UsersApi::getUserByLogin(login).get();
    return User{duser};
}

std::vector<User>
toUserVect(std::shared_ptr<std::vector<std::shared_ptr<UsersRelation>>> rels)
{
    auto container = std::vector<User>{};
    container.reserve(rels->size());
    for(auto relation : *rels)
    {
        container.emplace_back(relation->user, relation);
    }
    return std::move(container);
}

std::vector<User>
Application::getContacts () const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "CONTACT", "OUT").get();
    return toUserVect(rels);
}

std::vector<User>
Application::getInvitedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "INVITE", "OUT").get();
    return toUserVect(rels);
}

std::vector<User>
Application::getInvitingUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "INVITE", "IN").get();
    return toUserVect(rels);
}

std::vector<User>
Application::getSuggestedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "SHOULD_INVITE", "OUT").get();
    return toUserVect(rels);
}

std::vector<User>
Application::getBlockedUsers() const
{
    auto rels = NetworkApi::getUserRelation(_currentUser.id(), "BLOCK", "OUT").get();
    return toUserVect(rels);
}

User&
Application::currentUser()
{
    return _currentUser;
}


} /* namespace giga */

/*
 * Application.cpp
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
 */

#include "Application.h"
#include "User.h"
#include "../api/UsersApi.h"
#include "../api/NetworkApi.h"
#include "../api/data/User.h"

#include <cpprest/http_client.h>

#include "../api/data/UsersRelation.h"
using giga::data::UsersRelation;
using pplx::create_task;

namespace giga
{
namespace core
{

User addUser(std::unordered_map<int64_t, User>& users, std::shared_ptr<data::User> u, std::shared_ptr<data::UsersRelation> r = nullptr) {
    auto it = users.find(u->id);
    if (it == users.end()) {
        auto result = users.emplace(u->id, User{u, r});
        return result.first->second;
    }
    return it->second;
}

pplx::task<User>
Application::authenticate (const std::string& login, const std::string& password)
{
    return GigaApi::authenticate(login, password).then([this](std::shared_ptr<data::User> u) {
        std::lock_guard<std::mutex> lock(mut_users);
        _currentUser = addUser(users, u);
        return _currentUser;
    });
}

pplx::task<User>
Application::getUserById (int64_t id) const
{
    std::lock_guard<std::mutex> lock(mut_users);
    auto it = users.find(id);
    if (it != users.end())
    {
        return create_task([it]
        {
            return it->second;
        });
    }
    else
    {
        return UsersApi::getUserById(id).then([this](std::shared_ptr<data::User> u) {
            std::lock_guard<std::mutex> lock(mut_users);
            return addUser(users, u);
        });
    }
}

pplx::task<User>
Application::getUserByLogin (const std::string& login) const
{
    std::lock_guard<std::mutex> lock(mut_users);
    auto iter = std::find_if(users.begin(), users.end(), [&login](const std::pair<const int64_t, User> & pair) {
        return pair.second.login() == login;
    });
    if (iter != users.end()) {
        return create_task([iter]
        {
            return iter->second;
        });
    }
    return UsersApi::getUserByLogin(login).then([this](std::shared_ptr<data::User> u) {
        std::lock_guard<std::mutex> lock(mut_users);
        return addUser(users, u);
    });
}

pplx::task<void>
Application::loadRelations ()
{
    auto getCallback = [this](std::vector<User>& container) {
        return [this, &container](std::shared_ptr<std::vector<std::shared_ptr<UsersRelation>>> rel) -> void
        {
            std::lock_guard<std::mutex> lock(mut_users);
            container.clear();
            container.reserve(rel->size());
            for(auto relation : *rel)
            {
                auto user = addUser(users, relation->user, relation);
                container.emplace_back(std::move(user));
            }
        };
    };

    auto tasks = std::array<pplx::task<void>, 5> {
        NetworkApi::getUserRelation(_currentUser.id(), "CONTACT", "OUT").then(getCallback(_contacts)),
        NetworkApi::getUserRelation(_currentUser.id(), "INVITE", "IN").then(getCallback(_invitationsReceived)),
        NetworkApi::getUserRelation(_currentUser.id(), "INVITE", "OUT").then(getCallback(_invitationsSent)),
        NetworkApi::getUserRelation(_currentUser.id(), "SHOULD_INVITE", "OUT").then(getCallback(_suggestions)),
        NetworkApi::getUserRelation(_currentUser.id(), "BLOCK", "OUT").then(getCallback(_blocked))
    };

    return pplx::when_all(tasks.begin(), tasks.end());
}

const std::vector<User>&
Application::contacts () const
{
    return _contacts;
}

const std::vector<User>&
Application::invitationsReceived () const
{
    return _invitationsReceived;
}

const std::vector<User>&
Application::invitationsSent () const
{
    return _invitationsSent;
}

const std::vector<User>&
Application::suggestions () const
{
    return _suggestions;
}

const std::vector<User>&
Application::blocked () const
{
    return _blocked;
}

User&
Application::currentUser()
{
    return _currentUser;
}

void
addRelation(std::shared_ptr<data::UsersRelation> r, std::vector<User>& container)
{
    auto iter = std::find_if(container.begin(), container.end(), [&r](const User& user){
        return user.id() == r->user->id;
    });
    if (iter != container.end()) {
        container.emplace_back(r->user, r);
    }
}

void
Application::addContact (std::shared_ptr<data::UsersRelation> r)
{
    std::lock_guard<std::mutex> lock(mut_users);
    addRelation(r, _contacts);
}

void
Application::addInvitationsReceived (std::shared_ptr<data::UsersRelation> r)
{
    std::lock_guard<std::mutex> lock(mut_users);
    addRelation(r, _invitationsReceived);
}

void
Application::addInvitationsSent (std::shared_ptr<data::UsersRelation> r)
{
    std::lock_guard<std::mutex> lock(mut_users);
    addRelation(r, _invitationsSent);
}

void
Application::addSuggestions (std::shared_ptr<data::UsersRelation> r)
{
    std::lock_guard<std::mutex> lock(mut_users);
    addRelation(r, _suggestions);
}

void
Application::addBlocked (std::shared_ptr<data::UsersRelation> r)
{
    std::lock_guard<std::mutex> lock(mut_users);
    addRelation(r, _blocked);
}

} /* namespace core */
} /* namespace giga */

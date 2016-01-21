/*
 * Application.h
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_APPLICATION_H_
#define GIGA_CORE_APPLICATION_H_

#include "User.h"

#include <cpprest/http_client.h>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace giga
{

namespace data {
struct UsersRelation;
}

namespace core
{

class Application
{
public:
    static Application&
    instance ()
    {
        static Application _instance;
        return _instance;
    }

private:
    explicit Application()                  = default;
    Application(Application const&)         = delete;
    void operator=(Application const&)      = delete;

public:
    pplx::task<User>
    authenticate (const std::string& login, const std::string& password);

    pplx::task<User>
    getUserById (int64_t id) const;

    pplx::task<User>
    getUserByLogin (const std::string& login) const;

    pplx::task<void>
    loadRelations ();

    //
    // Getters
    //

    const std::vector<User>&
    contacts () const;

    const std::vector<User>&
    invitationsReceived () const;

    const std::vector<User>&
    invitationsSent () const;

    const std::vector<User>&
    suggestions () const;

    const std::vector<User>&
    blocked () const;

    User&
    currentUser();

private:
    friend User;

    void
    addContact(std::shared_ptr<data::UsersRelation> r);

    void
    addInvitationsReceived(std::shared_ptr<data::UsersRelation> r);

    void
    addInvitationsSent(std::shared_ptr<data::UsersRelation> r);

    void
    addSuggestions(std::shared_ptr<data::UsersRelation> r);

    void
    addBlocked(std::shared_ptr<data::UsersRelation> r);

private:
    using UserDict = std::unordered_map<int64_t, User>;

private:
    mutable std::mutex mut_users;

    // this is a cache variable
    mutable UserDict users;

    bool isRelationLoaded = false;
    std::vector<User> _contacts;
    std::vector<User> _invitationsReceived;
    std::vector<User> _invitationsSent;
    std::vector<User> _blocked;
    std::vector<User> _suggestions;
    User _currentUser;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_APPLICATION_H_ */

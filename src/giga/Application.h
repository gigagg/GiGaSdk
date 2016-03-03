/*
 * Application.h
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_APPLICATION_H_
#define GIGA_CORE_APPLICATION_H_

#include "core/User.h"
#include "core/Node.h"
#include "Config.h"

#include <cpprest/http_client.h>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace giga
{

namespace data {
struct UsersRelation;
}

class Application
{
public:

    static Application&
    init(utility::string_t&& appRedirectUri, utility::string_t&& appId, utility::string_t&& appKey, utility::string_t&& appScope =
            U("basic network groups files basic:write network:write groups:write files:write"));

    static Application&
    get();

    static Config&
    config ();

private:
    explicit Application()                  = default;
    Application(Application const&)         = delete;
    void operator=(Application const&)      = delete;

    static Application&
    instance ()
    {
        static Application _instance {};
        return _instance;
    }

public:

    bool
    isInitialized() const;

    core::User&
    authenticate (const utility::string_t& login, const utility::string_t& password);

    core::User&
    currentUser();

    //
    // Users
    //

    core::User
    getUserById (int64_t id) const;

    core::User
    getUserByLogin (const utility::string_t& login) const;

    std::vector<core::User>
    getContacts () const;

    std::vector<core::User>
    getInvitingUsers () const;

    std::vector<core::User>
    getInvitedUsers () const;

    std::vector<core::User>
    getSuggestedUsers () const;

    std::vector<core::User>
    getBlockedUsers () const;

    std::vector<core::User>
    searchUser (const utility::string_t& search) const;

    //
    // Nodes
    //

    std::unique_ptr<core::Node>
    getNodeById (const std::string& id) const;

    std::vector<std::unique_ptr<core::Node>>
    searchNode (const utility::string_t& search, core::Node::MediaType type) const;

private:
    core::User  _currentUser;
    Config      _config;
    bool        _isInitialized = false;
};

} /* namespace giga */

#endif /* GIGA_CORE_APPLICATION_H_ */

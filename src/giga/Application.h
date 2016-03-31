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

#ifndef GIGA_CORE_APPLICATION_H_
#define GIGA_CORE_APPLICATION_H_

#include <boost/optional.hpp>
#include "core/User.h"
#include "core/Node.h"
#include "core/FileNode.h"
#include "Config.h"
#include "api/GigaApi.h"

#include <cpprest/http_client.h>
#include <cpprest/details/basic_types.h>
#include <mutex>
#include <memory>
#include <vector>
#include <unordered_map>

namespace giga
{

namespace data {
struct UsersRelation;
}

/**
 * The Application class is the entry point to access the GiGa API.
 * The Application must be authenticated (once and only once). @see authenticate (const utility::string_t& login, const utility::string_t& password)
 */
class Application
{
public:
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;
    Application(Application &&)                = delete;
    Application& operator=(Application &&)     = delete;

    explicit Application();

public:
    /**
     * @brief Authenticate a user by it's login/password.
     * @return The current user.
     * @see currentUser()
     */
    core::User&
    authenticate (const utility::string_t& login, const utility::string_t& password);

    /**
     * @return The currently logged user (when you called ```authenticate(const utility::string_t&, const utility::string_t&)```).
     */
    const core::User&
    currentUser() const;

    /**
     * @return The currently logged user (when you called ```authenticate(const utility::string_t&, const utility::string_t&)```).
     */
    core::User&
    mutableCurrentUser();

    //
    // Users
    //

    /**
     * @brief Call the GiGa API to get a users' informations
     *
     * WARINIG: the ```core::User::relation()``` is never set when getting
     * a user with this method. The ```core::User::contactData()``` is not set either.
     * To have the relation and associated data of a user you need to use
     * getContacts() / getInvitingUsers() / getInvitedUsers() / getSuggestedUsers() / getBlockedUsers().
     *
     * @return the user
     * @throw HttpError
     */
    core::User
    getUserById (uint64_t id) const;

    /**
     * @brief Call the GiGa API to get a users' informations
     * @see getUserById(uint64_t) const
     */
    core::User
    getUserByLogin (const utility::string_t& login) const;

    /**
     * @brief Call the GiGa API to get a contact of the currentUser,
     * including ```core::User::contactData()``` and ```core::User::relation()```
     *
     * This method may look for the data in the cache (which should be ok most of the time).
     * If you really need to make sure the cache is up to date, you can call Application::getContacts() const.
     *
     * If the user with this id is not one of the currentUser contacts, then an ErrorNotFound exception is thrown.
     * @return the contact
     * @throw HttpError
     * @see getContacts() const
     */
    core::User
    getContact (uint64_t id) const;

    /**
     * @brief Call the GiGa API to get all the currentUser contacts
     *
     * - To get the contact specific information, use ```core::User::relation() const```.
     * - To get the informations restricted to its contacts, use ```core::User::contactData()```.
     *   This will include the root ```core::Node``` shared with the currentUser (see ```core::User::ContactData::node() const```).
     *
     * To get a new contact you need to accept other user invitation, or send invitations and then get accepted. <br>
     * You can remove a contact by using: ```core::User::removeRelation()```
     *
     * This method will refresh the local contact cache.
     * @return the contact list
     * @throw HttpError
     * @see core::User::invite()
     */
    std::vector<core::User>
    getContacts () const;

    /**
     * @brief Call the GiGa API to get all the currentUser received invitations
     *
     * Use ```core::User::relation() const``` to get the received invitation specific information.
     *
     * - You can accept an invitation to get a new contact: ```core::User::acceptInvitation()```
     * - Or you can refuse an invitation: ```core::User::removeRelation()```
     *
     * @return the list of received invitation
     * @throw HttpError
     */
    std::vector<core::User>
    getInvitingUsers () const;

    /**
     * @brief Call the GiGa API to get all the currentUser sent invitations
     *
     * Use ```core::User::relation() const``` to get the sent invitation specific information.
     *
     * To cancel a sent invitation use ```core::User::removeRelation()```
     *
     * @return the list of sent invitation
     * @throw HttpError
     * @see core::User::invite()
     */
    std::vector<core::User>
    getInvitedUsers () const;

    /**
     * @brief Call the GiGa API to get the suggested users
     *
     * Use ```core::User::relation() const``` to get the suggested user specific information.
     * Use ```core::User::removeRelation()``` to remove a suggestion.
     *
     * @return the list of suggested users
     * @throw HttpError
     */
    std::vector<core::User>
    getSuggestedUsers () const;

    /**
     * @brief Call the GiGa API to get the blocked users
     *
     * Use ```core::User::removeRelation()``` to unblock a user.
     *
     * @return the list of blocked users
     * @throw HttpError
     */
    std::vector<core::User>
    getBlockedUsers () const;

    /**
     * @brief Search users by login and tag
     *
     * WARINIG: the ```core::User::relation()``` is never set when getting
     * a user with this method. The ```core::User::contactData()``` is not set either.
     * To have the relation and associated data of a user you need to use
     * getContacts() / getInvitingUsers() / getInvitedUsers() / getSuggestedUsers() / getBlockedUsers().
     *
     * @return the list of matching users
     * @throw HttpError
     */
    std::vector<core::User>
    searchUser (const utility::string_t& search) const;

    //
    // Nodes
    //

    /**
     * @brief look for a node by its id
     */
    std::unique_ptr<core::Node>
    getNodeById (const std::string& id) const;

    /**
     * @brief search for a node by its name and type
     */
    std::vector<std::unique_ptr<core::Node>>
    searchNode (const utility::string_t& search, core::Node::MediaType type) const;


public:
    const GigaApi&
    api() const;

    GigaApi&
    mutableApi();

    //
    // Crypto. Be careful with these ...
    //

private:
    friend web::uri core::FileNodeData::fileUrl () const;

    std::string
    getNodeKeyClear(uint64_t userId) const;

private:
    GigaApi                      _api;
    std::unique_ptr<core::User>  _currentUser;

    // this is a cache variable
    // TODO protect by mutex.
    mutable std::unordered_map<uint64_t, core::User> _contacts;
};

} /* namespace giga */

#endif /* GIGA_CORE_APPLICATION_H_ */

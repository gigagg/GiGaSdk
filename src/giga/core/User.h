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

#ifndef GIGA_CORE_USER_H_
#define GIGA_CORE_USER_H_

#include "UserRelation.h"
#include "FolderNode.h"
#include "../utils/Crypto.h"

#include <boost/optional.hpp>
#include <cpprest/http_client.h>
#include <chrono>
#include <mutex>

namespace giga
{
class Application;

namespace data
{
struct User;
struct UsersRelation;
}

namespace core
{

/**
 * Represents a country code like "FR" or "US"
 */
class Country final {
public:
    explicit
    Country(const utility::string_t& code) : code(code) {}

    Country(Country&&)                 = default;
    Country(const Country&)            = default;
    ~Country()                         = default;
    Country& operator=(const Country&) = default;
    Country& operator=(Country&&)      = default;

    const utility::string_t code;
};

/**
 * Represents a language code like "fr" or "en"
 */
struct Language final {
    explicit
    Language(const utility::string_t& code) : code(code) {}

    Language(Language&&)                = default;
    Language(const Language&)           = default;
    ~Language()                         = default;
    Language& operator=(const Language&)= default;
    Language& operator=(Language&&)     = default;

    const utility::string_t code;
};

/**
 * @brief A user with an account on GiGa.GG.
 * Cf: https://giga.gg/users/profile/1704781
 */
class User final
{
public:
    enum class Activity
    {
        very_active = 0, active, inactive
    };
    enum class SeederStatus
    {
        no = 0, yes, to_some_people
    };
    enum class ReportedState {
        pending = 0, probation, checkup, suspended, free,
        no_report
    };
    enum class UserGender {
        male = 0, female, unknown
    };

public:
    User()                        = default;
    ~User()                       = default;
    User(User&&)                  = default;
    User(const User&)             = default;
    User& operator=(const User&)  = default;
    User& operator=(User&&)       = default;

    explicit User(std::shared_ptr<data::User> u, std::shared_ptr<data::UsersRelation> r = nullptr);

public:
    uint64_t
    id () const;

    const utility::string_t&
    login () const;

    web::uri
    avatarUri () const;

    web::uri
    bigAvatarUri () const;

    std::chrono::system_clock::time_point
    creationDate () const;

    std::chrono::system_clock::time_point
    lastConnectionDate () const;

    Activity
    activity () const;

    bool
    isUnlimited () const;

    /**
     * The seederStatus defines if a user share some nodes to other users.
     */
    SeederStatus
    isSeeder () const;

    uint64_t
    contactCount () const;

    /**
     * A user may add a list of Tags to describe is account.
     * The tags do not include the leading '#'.
     * You should always display them with a '#' first.
     *
     */
    const std::vector<utility::string_t>&
    tags () const;

    Country
    country () const;

    Language
    language () const;

    const utility::string_t&
    description () const;

public:
    /**
     * The user data restricted to its contacts.
     * Use ```Application::getContacts()``` then
     * ```User::contactData()```to get some ContactData
     */
    class ContactData final {
        friend User;
    public:
        ~ContactData()                             = default;
        ContactData(ContactData&&)                 = default;
        ContactData(const ContactData&)            = default;
        ContactData& operator=(const ContactData&) = default;
        ContactData& operator=(ContactData&&)      = default;

    private:
        explicit ContactData(std::shared_ptr<data::User> u);

    public:
        UserGender
        gender() const;

        const boost::optional<utility::string_t>&
        name() const;

        const boost::optional<std::chrono::system_clock::time_point>
        birthDate() const;

        uint64_t
        maxContact() const;

        /**
         * The root Node, containing all the Nodes shared with the currentUser.
         */
        FolderNode
        node() const;

    private:
        std::shared_ptr<data::User> _data;
    };

    /**
     * The currentUser personal data.
     * Use ```Application::currentUser()``` then
     * ```User::personalData() const```to get the currentUser PersonalData
     */
    class PersonalData {
        friend User;
        friend UserRelation;
    public:
        ~PersonalData()                              = default;
        PersonalData(PersonalData&&)                 = default;
        PersonalData(const PersonalData&)            = default;
        PersonalData& operator=(const PersonalData&) = default;
        PersonalData& operator=(PersonalData&&)      = default;

    private:
        explicit PersonalData(std::shared_ptr<data::User> u, const utility::string_t& password);

    public:
        ReportedState
        reportedState() const;

        bool
        isEmailValidated() const;

        const utility::string_t&
        email() const;

        const utility::string_t&
        nextEmail() const;

        const utility::string_t&
        nodeKeyClear() const;

        uint64_t
        maxStorage() const;

        uint64_t
        standatdMaxStorage() const;

        uint64_t
        downloaded() const;

        uint64_t
        dlAvailable() const;

//        ExternalAccount
//        externalAccount() const;
//        Offer
//        offer() const;

    private:
        std::shared_ptr<data::User> _data;
        utility::string_t           _nodeKeyClear;
        Rsa                         _rsaKeys;
    };

    /**
     * @return ```true``` if the user has contact data.
     */
    bool
    hasContactData() const;

    /**
     * @brief Get the data shared by this user.
     *
     * Make sure this user ```hasContactData()```. <br>
     * To get a user with contact data, you can user ```Application::getContacts()``` or ```Application::getContact()```.
     *
     * @return The data shared by this user.
     * @throw ErrorException if there is no contactData.
     */
    ContactData&
    contactData();

    /**
     * @return ```true``` if the user has personal data (this is true only for the currentUser)
     */
    bool
    hasPersonalData() const;

    /**
     * @brief Get the personal data of the currentUser.
     *
     * Make sure this user ```hasPersonalData()```. <br>
     * To get the currentUser uses ```Application::currentUser()```.
     *
     * @throw ErrorException if there is no personalData.
     */
    const PersonalData&
    personalData() const;

    /**
     * @return ```true``` if this user has a UserRelation set.
     */
    bool
    hasRelation() const;

    /**
     * @brief Get the UserRelation data between this user and the currentUser
     *
     * Make sure this user ```hasRelation() const```.<br>
     * To get users with relational data with the currentUser, you may use one of these functions:
     * - ```Application::getContacts()```
     * - ```Application::getContact()```
     * - ```Application::getInvitingUsers()```
     * - ```Application::getInvitedUsers()```
     * - ```Application::getSuggestedUsers()```
     * - ```Application::getBlockedUsers()```
     *
     * @throw ErrorException if there is no relation.
     */
    UserRelation
    relation() const;

    //
    // Setters
    //

    /**
     * @brief Send an invitation from the currentUser to this user
     * @return A new User with the ```relation()``` set to an invitation.
     * @throw HttpError
     */
    User
    invite();

    /**
     * @brief The currentUser blocks this user
     * @return A new User with the ```relation()``` set to a block UserRelation.
     * @throw HttpError
     */
    User
    block();

    /**
     * @brief Accept an invitation received from this user to the currentUser
     * @return A new User with the ```relation()``` set to a contact UserRelation (and ```contactData()``` set).
     * @throw HttpError
     */
    User
    acceptInvitation();

    /**
     * @brief Suggest ```contact``` to this user.
     * @throw HttpError
     * @see UserRelation
     */
    void
    suggest(const User& contact);

    /**
     * @brief Remove a relation. Make sure this user ```hasRelation() const```
     * @throw HttpError
     * @see UserRelation
     */
    void
    removeRelation();

private:
    friend class giga::Application; // core::User& Application::authenticate (const utility::string_t&, const utility::string_t&);

    PersonalData&
    initializePersonalData (const utility::string_t& password);

private:
    std::shared_ptr<data::User>          _data;
    std::shared_ptr<data::UsersRelation> _relation;
    boost::optional<PersonalData>        _private;
    boost::optional<ContactData>         _protected;
    boost::optional<giga::Rsa>           _publicKey;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_USER_H_ */

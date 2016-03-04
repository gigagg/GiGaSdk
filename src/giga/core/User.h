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

namespace data
{
struct User;
struct UsersRelation;
}

namespace core
{

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

    SeederStatus
    isSeeder () const;

    uint64_t
    contactCount () const;

    const std::vector<utility::string_t>&
    tags () const;

    Country
    country () const;

    Language
    language () const;

    const utility::string_t&
    description () const;

public:
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

        FolderNode
        node() const;

    private:
        std::shared_ptr<data::User> _data;
    };

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

    bool
    hasContactData() const;

    ContactData&
    contactData();

    bool
    hasPersonalData() const;

    PersonalData&
    initializePersonalData (const utility::string_t& password);

    const PersonalData&
    personalData() const;

    bool
    hasRelation() const;

    UserRelation
    relation() const;

    //
    // Setters
    //

    User invite();
    User block();
    User acceptInvitation();

    void suggest(const User& contact);

    void removeRelation();

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

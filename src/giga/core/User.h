/*
 * User.h
 *
 *  Created on: 19 janv. 2016
 *      Author: thomas
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
class User;
class UsersRelation;
}

namespace core
{

class Country final {
public:
    explicit
    Country(const std::string& code) : code(code) {}

    Country(Country&&)                 = default;
    Country(const Country&)            = default;
    ~Country()                         = default;
    Country& operator=(const Country&) = default;
    Country& operator=(Country&&)      = default;

    const std::string code;
};

struct Language final {
    explicit
    Language(const std::string& code) : code(code) {}

    Language(Language&&)                = default;
    Language(const Language&)           = default;
    ~Language()                         = default;
    Language& operator=(const Language&)= default;
    Language& operator=(Language&&)     = default;

    const std::string code;
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
    int64_t
    id () const;

    const std::string&
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

    int64_t
    contactCount () const;

    const std::vector<std::string>&
    tags () const;

    Country
    country () const;

    Language
    language () const;

    const std::string&
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

        const boost::optional<std::string>&
        name() const;

        const boost::optional<std::chrono::system_clock::time_point>
        birthDate() const;

        int64_t
        maxContact() const;

        FolderNode
        node() const;


    private:
        std::shared_ptr<data::User> _data;
    };

    class PersonalData {
        friend User;
    public:
        ~PersonalData()                              = default;
        PersonalData(PersonalData&&)                 = default;
        PersonalData(const PersonalData&)            = default;
        PersonalData& operator=(const PersonalData&) = default;
        PersonalData& operator=(PersonalData&&)      = default;

    private:
        explicit PersonalData(std::shared_ptr<data::User> u, const std::string& password);

    public:
        ReportedState
        reportedState() const;

        bool
        isEmailValidated() const;

        const std::string&
        email() const;

        const std::string&
        nextEmail() const;

        const std::string&
        nodeKeyClear() const;

        int64_t
        maxStorage() const;

        int64_t
        standatdMaxStorage() const;

        int64_t
        downloaded() const;

        int64_t
        dlAvailable() const;


//        ExternalAccount
//        externalAccount() const;
//        Offer
//        offer() const;

    private:
        std::shared_ptr<data::User> _data;
        std::string                 _nodeKeyClear;
    };

    bool
    hasContactData() const;

    ContactData&
    contactData();

    bool
    hasPersonalData() const;

    PersonalData&
    initializePersonalData (const std::string& password);

    PersonalData&
    personalData();

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

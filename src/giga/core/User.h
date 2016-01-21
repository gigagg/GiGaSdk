/*
 * User.h
 *
 *  Created on: 19 janv. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_USER_H_
#define GIGA_CORE_USER_H_

#include "UserRelation.h"
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
    explicit Country(const std::string& code) : code(code) {}
    Country(Country&&) = default;
    Country(const Country&) = default;
    ~Country() = default;
    Country& operator=(const Country&) = default;
    Country& operator=(Country&&) = default;

    const std::string code;
};

struct Language final {
    explicit Language(const std::string& code) : code(code) {}
    Language(Language&&) = default;
    Language(const Language&) = default;
    ~Language() = default;
    Language& operator=(const Language&) = default;
    Language& operator=(Language&&) = default;

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
    User()                       = default; // TODO: private + friend to correct class (pplx).
    ~User()                      = default;

    explicit User(std::shared_ptr<data::User> u, std::shared_ptr<data::UsersRelation> r = nullptr);
    User(User&&);
    User(const User&);
    User& operator=(const User&);
    User& operator=(User&&);

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
    class ProtectedData final {
        friend User;
    public:
        ~ProtectedData()                               = default;
        ProtectedData(ProtectedData&&)                 = default;
        ProtectedData(const ProtectedData&)            = default;
        ProtectedData& operator=(const ProtectedData&) = default;
        ProtectedData& operator=(ProtectedData&&)      = default;

    private:
        explicit ProtectedData(std::shared_ptr<data::User> u);

    public:
        UserGender
        gender() const;

        const boost::optional<std::string>&
        name() const;

        const boost::optional<std::chrono::system_clock::time_point>
        birthDate() const;

        int64_t
        maxContact() const;

// TODO class Node.
//        Node&
//        node() const;


    private:
        std::shared_ptr<data::User> u;
    };

    class PrivateData {
        friend User;
    public:
        ~PrivateData()                             = default;
        PrivateData(PrivateData&&)                 = default;
        PrivateData(const PrivateData&)            = default;
        PrivateData& operator=(const PrivateData&) = default;
        PrivateData& operator=(PrivateData&&)      = default;

    private:
        explicit PrivateData(std::shared_ptr<data::User> u);

    public:
        ReportedState
        reportedState() const;

        bool
        isEmailValidated() const;

        const std::string&
        email() const;

        const std::string&
        nextEmail() const;

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
        std::shared_ptr<data::User> u;
        std::string nodeKeyClear;
    };

    bool
    hasProtectedData() const;

    ProtectedData&
    protectedData();

    bool
    hasPrivateData() const;

    PrivateData&
    privateData();

    bool
    hasRelation() const;

    UserRelation
    relation() const;

    //
    // Setters
    //

    pplx::task<void> invite();
    pplx::task<void> block();
    pplx::task<void> suggest();
    pplx::task<void> acceptInvitation();

private:
    void setRelation(std::shared_ptr<data::UsersRelation> r);

private:
    std::mutex mut;
    std::shared_ptr<data::User> u;
    std::shared_ptr<data::UsersRelation> r;
    boost::optional<PrivateData> _private;
    boost::optional<ProtectedData> _protected;
    boost::optional<giga::Rsa> publicKey;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_USER_H_ */

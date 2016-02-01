/*
 * User.cpp
 *
 *  Created on: 19 janv. 2016
 *      Author: thomas
 */

#include "User.h"
#include "../Application.h"
#include "../api/data/User.h"
#include "../api/NetworkApi.h"
#include "../api/data/UsersRelation.h"
#include "../utils/Crypto.h"
#include "../utils/EnumConvertor.h"

#include <chrono>

using std::chrono::system_clock;

namespace giga
{

static const utils::EnumConvertor<core::User::SeederStatus, 3> seederStatusCvrt =
    {"NO", "YES", "TO_SOME_PEOPLE"};

static const utils::EnumConvertor<core::User::Activity, 3> activityCvrt =
    {"VERY_ACTIVE", "ACTIVE", "INACTIVE"};

static const utils::EnumConvertor<core::User::ReportedState, 6> reportedStateCvrt =
    {"PENDING", "PROBATION", "CHECKUP", "SUSPENDED", "FREE", "NO_REPORT"};

static const utils::EnumConvertor<core::User::UserGender, 3> userGenderCvrt =
    {"M", "F", "UNKNOWN"};

namespace core
{

#define _THROW_IF_NO_USER_ if (u == nullptr) { THROW(ErrorException{"User is not set"}); } do {} while(0)

User::User (std::shared_ptr<data::User> u, std::shared_ptr<data::UsersRelation> r) :
        u{u}, r{r},_private{}, _protected{}, publicKey{}
{
    _THROW_IF_NO_USER_;
    if (u != nullptr && u->publicKey.is_initialized()) {
        publicKey = boost::make_optional(Rsa(u->publicKey.get()));
    }
}

int64_t
User::id () const
{
    _THROW_IF_NO_USER_;
    return u->id;
}

const std::string&
User::login () const
{
    _THROW_IF_NO_USER_;
    return u->login;
}

web::uri
User::avatarUri () const
{
    _THROW_IF_NO_USER_;
    return web::uri(u->avatarUrl);
}

web::uri
User::bigAvatarUri () const
{
    _THROW_IF_NO_USER_;
    return web::uri(u->bigAvatarUrl);
}

system_clock::time_point
User::creationDate () const
{
    _THROW_IF_NO_USER_;
    return system_clock::time_point(std::chrono::seconds(u->creationDate));
}

system_clock::time_point
User::lastConnectionDate () const
{
    _THROW_IF_NO_USER_;
    return system_clock::time_point(std::chrono::seconds(u->lastConnectionDate.get_value_or(u->creationDate)));
}

User::Activity
User::activity () const
{
    _THROW_IF_NO_USER_;
    return activityCvrt.fromStr(u->activity);
}

bool
User::isUnlimited () const
{
    _THROW_IF_NO_USER_;
    return u->isUnlimited;
}

User::SeederStatus
User::isSeeder () const
{
    _THROW_IF_NO_USER_;
    return seederStatusCvrt.fromStr(u->isSeeder);
}

int64_t
User::contactCount () const
{
    _THROW_IF_NO_USER_;
    return u->contactCount;
}

const std::vector<std::string>&
User::tags () const
{
    _THROW_IF_NO_USER_;
    return u->tags;
}

Country
User::country () const
{
    _THROW_IF_NO_USER_;
    return Country{u->country.get_value_or("XX")};
}

Language
User::language () const
{
    _THROW_IF_NO_USER_;
    return Language{u->language.get_value_or("en")};
}

User::ProtectedData::ProtectedData (std::shared_ptr<data::User> u) : u{u}
{
}

User::UserGender
User::ProtectedData::gender () const
{
    _THROW_IF_NO_USER_;
    if (u->gender.is_initialized()) {
        return userGenderCvrt.fromStr(u->gender.get());
    }
    return User::UserGender::unknown;
}

const boost::optional<std::string>&
User::ProtectedData::name () const
{
    _THROW_IF_NO_USER_;
    return u->name;
}

const boost::optional<system_clock::time_point>
User::ProtectedData::birthDate () const
{
    _THROW_IF_NO_USER_;
    if (u->birthDate.is_initialized()) {
        std::tm tm = {0,0,0,0,0,0,0,0,0,0,0};
        strptime(u->birthDate.get().c_str(), "%Y-%m-%d", &tm);
        return boost::make_optional(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
    }
    return boost::optional<system_clock::time_point>{};
}

int64_t
User::ProtectedData::maxContact () const
{
    _THROW_IF_NO_USER_;
    return u->maxContact.get_value_or(200);
}

User::PrivateData::PrivateData (std::shared_ptr<data::User> u) : u{u}
{
    auto masterKey = Crypto::calculateMasterPassword(u->salt.get(), "gigatribe");
    auto privateKey = Crypto::aesDecrypt(masterKey,
                                         Crypto::base64decode(u->rsaKeys->aesSalt),
                                         Crypto::base64decode(u->rsaKeys->aesIv),
                                         Crypto::base64decode(u->rsaKeys->privateKey));
    auto rsa = Rsa{u->rsaKeys->publicKey, privateKey};
    auto tmp = rsa.decrypt(Crypto::base64decode(u->nodeKey.get()));
    // TODO make sure the base64decode is necessary
    nodeKeyClear = Crypto::base64decode(tmp);
}

User::ReportedState
User::PrivateData::reportedState () const
{
    if (u->reportedState.is_initialized()) {
        return reportedStateCvrt.fromStr(u->reportedState.get());
    }
    return User::ReportedState::no_report;
}

const std::string&
User::PrivateData::email () const
{
    if(u->email.is_initialized()) {
        return u->email.get();
    }
    if(u->nextEmail.is_initialized()) {
        return u->nextEmail.get();
    }
    THROW(ErrorException{"Email/nextEmail are not correctly initialized"});
}

const std::string&
User::PrivateData::nextEmail () const
{
    if(u->nextEmail.is_initialized()) {
        return u->nextEmail.get();
    }
    if(u->email.is_initialized()) {
        return u->email.get();
    }
    THROW(ErrorException{"Email/nextEmail are not correctly initialized"});
}

bool
User::PrivateData::isEmailValidated() const
{
    return u->email.is_initialized();
}

int64_t
User::PrivateData::maxStorage () const
{
    return u->maxStorage.get_value_or(100);
}

int64_t
User::PrivateData::standatdMaxStorage () const
{
    return u->standatdMaxStorage.get_value_or(100);
}

int64_t
User::PrivateData::downloaded () const
{
    return u->downloaded.get_value_or(0);
}

int64_t
User::PrivateData::dlAvailable () const
{
    return u->dlAvailable.get_value_or(0);
}

bool
User::hasProtectedData () const
{
    return u != nullptr && u->node != nullptr;
}

User::ProtectedData&
User::protectedData ()
{
    if(hasProtectedData()) {
        if (!_protected.is_initialized()) {
            _protected = boost::make_optional(ProtectedData{u});
        }
        return _protected.get();
    }
    THROW(ErrorException{"No protected data"});
}

bool
User::hasPrivateData () const
{
    return u != nullptr && u->salt.is_initialized();
}

User::PrivateData&
User::privateData ()
{
    if(hasPrivateData()) {
        if (!_private.is_initialized()) {
            _private = boost::make_optional(PrivateData{u});
        }
        return _private.get();
    }
    THROW(ErrorException{"No private data"});
}

bool
User::hasRelation () const
{
    return r != nullptr;
}

UserRelation
User::relation () const
{
    if (hasRelation()) {
        return UserRelation{r};
    }
    THROW(ErrorException{"No relation"});
}


User
User::invite ()
{
    // TODO: test states + make sure optionals are here
    // TODO: groupIds ...
    auto& app = Application::get();
    auto& nodeKeyClear = app.currentUser().privateData().nodeKeyClear;
    auto key = Crypto::base64encode(publicKey.get().encrypt(nodeKeyClear));
    auto r = NetworkApi::createUserRelation(app.currentUser().id(), id(), "INVITE", "UNKNOWN", key).get();
    return User{r->user, r};
}

User
User::block ()
{
    // TODO: test states + make sure optionals are here
    auto& app = Application::get();
    auto r =  NetworkApi::createUserRelation(app.currentUser().id(), id(), "BLOCK", "", "").get();
    return User{r->user, r};
}

User
User::suggest ()
{
    // TODO: test states + make sure optionals are here
    auto& app = Application::get();
    auto r = NetworkApi::createUserRelation(app.currentUser().id(), id(), "SHOULD_INVITE", "", "").get();
    return User{r->user, r};
}

User
User::acceptInvitation ()
{
    // TODO: test states + make sure optionals are here
    auto& app = Application::get();
    auto& nodeKeyClear = app.currentUser()._private.get().nodeKeyClear;
    auto key = Crypto::base64encode(publicKey.get().encrypt(nodeKeyClear));
    auto r = NetworkApi::createUserRelation(app.currentUser().id(), id(), "CONTACT", "", key).get();
    return User{r->user, r};
}

} /* namespace core */
} /* namespace giga */

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

#include "User.h"
#include "../Application.h"
#include "../api/data/User.h"
#include "../api/data/UsersRelation.h"
#include "../utils/Crypto.h"
#include "../utils/EnumConvertor.h"
#include "../utils/Utils.h"

#include <chrono>
#include <ctime>

using std::chrono::system_clock;
using utility::string_t;

namespace giga
{

static const utils::EnumConvertor<core::User::SeederStatus, 3> seederStatusCvrt =
    {U("NO"), U("YES"), U("TO_SOME_PEOPLE")};

static const utils::EnumConvertor<core::User::Activity, 3> activityCvrt =
    {U("VERY_ACTIVE"), U("ACTIVE"), U("INACTIVE")};

static const utils::EnumConvertor<core::User::ReportedState, 6> reportedStateCvrt =
    {U("PENDING"), U("PROBATION"), U("CHECKUP"), U("SUSPENDED"), U("FREE"), U("NO_REPORT")};

static const utils::EnumConvertor<core::User::UserGender, 3> userGenderCvrt =
    {U("M"), U("F"), U("UNKNOWN")};

namespace core
{

#define _THROW_IF_NO_USER_ if (_data == nullptr) { BOOST_THROW_EXCEPTION(ErrorException{U("User is not set")}); } do {} while(0)

User::User (std::shared_ptr<data::User> u, const Application& app, std::shared_ptr<data::UsersRelation> r) :
        _data{u}, _relation{r},_private{}, _protected{}, _publicKey{}, _app(&app)
{
    _THROW_IF_NO_USER_;
    if (u != nullptr && u->publicKey.is_initialized()) {
        _publicKey = boost::make_optional(Rsa(u->publicKey.get()));
    }
}

User&
User::operator=(const User& rhs)
{
    this->_data     = rhs._data;
    this->_relation = rhs._relation;
    this->_app      = rhs._app;
    return *this;
}

User&
User::operator=(User&& rhs) noexcept
{
    this->_data     = std::move(rhs._data);
    this->_relation = std::move(rhs._relation);
    this->_app      = rhs._app;
    return *this;
}

uint64_t
User::id () const
{
    _THROW_IF_NO_USER_;
    return _data->id;
}

const string_t&
User::login () const
{
    _THROW_IF_NO_USER_;
    return _data->login;
}

web::uri
User::avatarUri () const
{
    _THROW_IF_NO_USER_;
    return web::uri(utils::httpsPrefix(_data->avatarUrl));
}

web::uri
User::bigAvatarUri () const
{
    _THROW_IF_NO_USER_;
    return web::uri(utils::httpsPrefix(_data->bigAvatarUrl));
}

system_clock::time_point
User::creationDate () const
{
    _THROW_IF_NO_USER_;
    return system_clock::time_point(std::chrono::seconds(_data->creationDate));
}

system_clock::time_point
User::lastConnectionDate () const
{
    _THROW_IF_NO_USER_;
    return system_clock::time_point(std::chrono::seconds(_data->lastConnectionDate.get_value_or(_data->creationDate)));
}

User::Activity
User::activity () const
{
    _THROW_IF_NO_USER_;
    return activityCvrt.fromStr(_data->activity);
}

bool
User::isUnlimited () const
{
    _THROW_IF_NO_USER_;
    return _data->isUnlimited;
}

User::SeederStatus
User::isSeeder () const
{
    _THROW_IF_NO_USER_;
    return seederStatusCvrt.fromStr(_data->isSeeder);
}

uint64_t
User::contactCount () const
{
    _THROW_IF_NO_USER_;
    return _data->contactCount;
}

const std::vector<string_t>&
User::tags () const
{
    _THROW_IF_NO_USER_;
    return _data->tags;
}

Country
User::country () const
{
    _THROW_IF_NO_USER_;
    return Country{_data->country.get_value_or(U("XX"))};
}

Language
User::language () const
{
    _THROW_IF_NO_USER_;
    return Language{_data->language.get_value_or(U("en"))};
}

User::ContactData::ContactData (std::shared_ptr<data::User> u, const Application &app) :
        _data{u}, _root{_data->node, app}
{
}

User::UserGender
User::ContactData::gender () const
{
    _THROW_IF_NO_USER_;
    if (_data->gender.is_initialized()) {
        return userGenderCvrt.fromStr(_data->gender.get());
    }
    return User::UserGender::unknown;
}

const boost::optional<string_t>&
User::ContactData::name () const
{
    _THROW_IF_NO_USER_;
    return _data->name;
}

const boost::optional<system_clock::time_point>
User::ContactData::birthDate () const
{
    _THROW_IF_NO_USER_;
    if (_data->birthDate.is_initialized()) {
#ifdef _MSC_VER
        std::tm tm = {}; // 0,0,0,0,0,0,0,0,0,0,0
        int year, month, day;
        auto bdStr = utils::wstr2str(_data->birthDate.get());
        sscanf_s(bdStr.c_str(), "%d-%d-%d", &year, &month, &day);
#else
        std::tm tm = {0,0,0,0,0,0,0,0,0,0,0};
        int year, month, day;
        auto bdStr = utils::wstr2str(_data->birthDate.get());
        sscanf(bdStr.c_str(), "%d-%d-%d", &year, &month, &day);
#endif
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        return boost::make_optional(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
    }
    return boost::optional<system_clock::time_point>{};
}

uint64_t
User::ContactData::maxContact () const
{
    _THROW_IF_NO_USER_;
    return _data->maxContact.get_value_or(200);
}

const FolderNode&
User::ContactData::node () const
{
    _THROW_IF_NO_USER_;
    return _root;
}

User::PersonalData::PersonalData (std::shared_ptr<data::User> u, const string_t& password) : _data{u}
{
    if (!u->salt.is_initialized() || !u->nodeKey.is_initialized())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Salt and NodeKey are required to build PrivateData")});
    }
    auto masterKey = Crypto::calculateMasterPassword(u->salt.get(), password);
    auto privateKey = Crypto::aesDecrypt(utils::str2wstr(masterKey),
                                         Crypto::base64decode(u->rsaKeys->aesSalt),
                                         Crypto::base64decode(u->rsaKeys->aesIv),
                                         Crypto::base64decode(u->rsaKeys->privateKey));
    _rsaKeys = Rsa{u->rsaKeys->publicKey, privateKey};
    _nodeKeyClear = _rsaKeys.decryptNodeKey(u->nodeKey.get());
}

User::ReportedState
User::PersonalData::reportedState () const
{
    if (_data->reportedState.is_initialized()) {
        return reportedStateCvrt.fromStr(_data->reportedState.get());
    }
    return User::ReportedState::no_report;
}

const string_t&
User::PersonalData::email () const
{
    if(_data->email.is_initialized()) {
        return _data->email.get();
    }
    if(_data->nextEmail.is_initialized()) {
        return _data->nextEmail.get();
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("Email/nextEmail are not correctly initialized")});
}

const string_t&
User::PersonalData::nextEmail () const
{
    if(_data->nextEmail.is_initialized()) {
        return _data->nextEmail.get();
    }
    if(_data->email.is_initialized()) {
        return _data->email.get();
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("Email/nextEmail are not correctly initialized")});
}

const std::string&
User::PersonalData::nodeKeyClear () const
{
    return _nodeKeyClear;
}

Node::MergePolicy
User::PersonalData::mergePolicy() const
{
    return Node::mergePolicyCvrt.fromStr(_data->mergePolicy.get_value_or(U("renameSource")));
}

bool
User::PersonalData::isEmailValidated() const
{
    return _data->email.is_initialized();
}

uint64_t
User::PersonalData::maxStorage () const
{
    return _data->maxStorage.get_value_or(100);
}

uint64_t
User::PersonalData::standatdMaxStorage () const
{
    return _data->standatdMaxStorage.get_value_or(100);
}

uint64_t
User::PersonalData::downloaded () const
{
    return _data->downloaded.get_value_or(0);
}

uint64_t
User::PersonalData::dlAvailable () const
{
    return _data->dlAvailable.get_value_or(0);
}

bool
User::hasContactData () const
{
    return _data != nullptr && _data->node != nullptr;
}

User::ContactData&
User::contactData ()
{
    if(hasContactData())
    {
        if (!_protected.is_initialized())
        {
            if (_app == nullptr)
            {
                BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
            }
            _protected = boost::make_optional(ContactData{_data, *_app});
        }
        return _protected.get();
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("No protected data")});
}

bool
User::hasPersonalData () const
{
    return _data != nullptr && _data->salt.is_initialized();
}

User::PersonalData&
User::initializePersonalData (const string_t& password)
{
    if(hasPersonalData()) {
        if (!_private.is_initialized()) {
            _private = boost::make_optional(PersonalData{_data, password});
        }
        return _private.get();
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("No private data")});
}

const User::PersonalData&
User::personalData () const
{
    if(hasPersonalData()) {
        if (!_private.is_initialized()) {
            BOOST_THROW_EXCEPTION(ErrorException{U("You must initialize private data first")});
        }
        return _private.get();
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("No private data")});
}

bool
User::hasRelation () const
{
    return _relation != nullptr;
}

UserRelation
User::relation () const
{
    if (hasRelation()) {
        return UserRelation{_relation};
    }
    BOOST_THROW_EXCEPTION(ErrorException{U("No relation")});
}


User
User::invite ()
{
    if (!_publicKey.is_initialized())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("PublicKey is needed")});
    }
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    // TODO: groupIds ...
    auto& user = _app->currentUser();
    auto key = Crypto::base64encode(_publicKey.get().encrypt(user.personalData().nodeKeyClear()));
    auto r = _app->api().network.createUserRelation(user.id(), id(), U("INVITE"), U("UNKNOWN"), key).get();
    return User{r->user, *_app, r};
}

User
User::block ()
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    auto r =  _app->api().network.createUserRelation(_app->currentUser().id(), id(), U("BLOCK"), U(""), "").get();
    return User{r->user, *_app, r};
}

void
User::suggest (const User& contact)
{
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    auto r = _app->api().network.createUserRelation(id(), contact.id(), U("SHOULD_INVITE"), U(""), "").get();
}

User
User::acceptInvitation ()
{
    if (!_publicKey.is_initialized())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("PublicKey is needed")});
    }
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    auto& user = _app->currentUser();
    auto key = Crypto::base64encode(_publicKey.get().encrypt(user._private.get().nodeKeyClear()));
    auto r = _app->api().network.createUserRelation(user.id(), id(), U("CONTACT"), U(""), key).get();
    return User{r->user, *_app, r};
}

void
User::removeRelation ()
{
    if (!hasRelation())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("No relation to remove")});
    }
    if (_app == nullptr)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("Application is null")});
    }
    auto& user = _app->currentUser();
    _app->api().network.deleteUserRelation(user.id(), id(), _relation->type).get();

    // remove relation and cache.
    this->_relation  = nullptr;
    this->_protected = boost::none;
    this->_private   = boost::none;
}

} /* namespace core */
} /* namespace giga */

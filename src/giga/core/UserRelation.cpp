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

#include "UserRelation.h"
#include "User.h"
#include "../utils/EnumConvertor.h"
#include "../utils/Utils.h"
#include "../api/data/UsersRelation.h"

#include <chrono>

using std::chrono::system_clock;

namespace giga
{
namespace core
{

static const utils::EnumConvertor<core::UserRelation::Type, 5> typeCvrt =
    {U("CONTACT"), U("INVITE"), U("BLOCK"), U("SHOULD_INVITE"), U("ENTICE_INTO_GIGA")};

static const utils::EnumConvertor<core::UserRelation::Initiator, 3> initiatorCvrt =
    {U("OTHER"), U("ME"), U("NA")};

static const std::vector<uint64_t> emptyVector = {};

UserRelation::UserRelation (std::shared_ptr<data::UsersRelation> r) :
    _data(r), _nodeKeyClear()
{
}

UserRelation::Type
UserRelation::type () const
{
    return typeCvrt.fromStr(_data->type);
}

std::chrono::system_clock::time_point
UserRelation::creationDate () const
{
    return system_clock::time_point(std::chrono::seconds(_data->creationDate));
}

const std::vector<uint64_t>&
UserRelation::userList () const
{
    if (_data->suggestedBy.is_initialized()) {
        return _data->suggestedBy.get();
    }
    if (_data->byUserId.is_initialized()) {
        return _data->byUserId.get();
    }
    return emptyVector;
}

UserRelation::Initiator
UserRelation::initiator () const
{
    if (_data->initiator.is_initialized()) {
        return initiatorCvrt.fromStr(_data->initiator.get());
    }
    return Initiator::na;
}

const std::string&
UserRelation::nodeKeyClear(const User& currentUser) const
{
    if (type() != Type::contact)
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("NodeKeyClear is only valid for Contacts")});
    }
    if (!_data->key.is_initialized())
    {
        BOOST_THROW_EXCEPTION(ErrorException{U("key should be initialized")});
    }
    if (_nodeKeyClear.empty())
    {
        _nodeKeyClear = currentUser.personalData()._rsaKeys.decryptNodeKey(utils::wstr2str(_data->key.get()));
    }
    return _nodeKeyClear;
}

} /* namespace core */
} /* namespace giga */

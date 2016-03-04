/*
 * UserRelation.cpp
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
 */

#include "UserRelation.h"
#include "../utils/EnumConvertor.h"
#include "../api/data/UsersRelation.h"
#include "../Application.h"

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

UserRelation::UserRelation (std::shared_ptr<data::UsersRelation> r) : _data(r)
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
UserRelation::nodeKeyClear() const
{
    if (type() != Type::contact)
    {
        BOOST_THROW_EXCEPTION(ErrorException{"NodeKeyClear is only valid for Contacts"});
    }
    if (!_data->key.is_initialized())
    {
        BOOST_THROW_EXCEPTION(ErrorException{"key should be initialized"});
    }
    if (_nodeKeyClear.empty())
    {
        _nodeKeyClear = Application::get().currentUser().personalData()._rsaKeys.decryptNodeKey(_data->key.get());
    }
    return _nodeKeyClear;
}

} /* namespace core */
} /* namespace giga */

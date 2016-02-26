/*
 * UserRelation.cpp
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
 */

#include "UserRelation.h"
#include "../utils/EnumConvertor.h"
#include "../api/data/UsersRelation.h"

using std::chrono::system_clock;

#include <chrono>
namespace giga
{
namespace core
{

static const utils::EnumConvertor<core::UserRelation::Type, 5> typeCvrt =
    {"CONTACT", "INVITE", "BLOCK", "SHOULD_INVITE", "ENTICE_INTO_GIGA"};

static const utils::EnumConvertor<core::UserRelation::Initiator, 3> initiatorCvrt =
    {"OTHER", "ME", "NA"};

static const std::vector<int64_t> emptyVector = {};

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

const std::vector<int64_t>&
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

} /* namespace core */
} /* namespace giga */

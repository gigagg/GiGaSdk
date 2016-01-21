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

UserRelation::UserRelation (std::shared_ptr<data::UsersRelation> r) : r(r)
{
}

UserRelation::Type
UserRelation::type () const
{
    return typeCvrt.fromStr(r->type);
}

std::chrono::system_clock::time_point
UserRelation::creationDate () const
{
    return system_clock::time_point(std::chrono::seconds(r->creationDate));
}

const std::vector<int64_t>&
UserRelation::userList () const
{
    if (r->suggestedBy.is_initialized()) {
        return r->suggestedBy.get();
    }
    if (r->byUserId.is_initialized()) {
        return r->byUserId.get();
    }
    return emptyVector;
}

UserRelation::Initiator
UserRelation::initiator () const
{
    if (r->initiator.is_initialized()) {
        return initiatorCvrt.fromStr(r->initiator.get());
    }
    return Initiator::na;
}

} /* namespace core */
} /* namespace giga */
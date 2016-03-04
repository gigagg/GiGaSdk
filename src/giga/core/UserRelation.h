/*
 * UserRelation.h
 *
 *  Created on: 20 janv. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_USERRELATION_H_
#define GIGA_CORE_USERRELATION_H_

#include <chrono>
#include <memory>
#include <vector>

namespace giga
{

namespace data
{
struct UsersRelation;
}

namespace core
{

class UserRelation
{
public:
    enum class Type
    {
        contact = 0, invite, block, should_invite, entice_into_giga
    };

    enum class Initiator
    {
        other = 0, me, na
    };

public:
    explicit UserRelation (std::shared_ptr<data::UsersRelation> r);
    UserRelation (UserRelation&&)                   = default;
    UserRelation (const UserRelation&)              = default;
    ~UserRelation ()                                = default;
    UserRelation& operator= (const UserRelation&)   = default;
    UserRelation& operator= (UserRelation&&)        = default;

    //
    // Getters
    //

    Type
    type () const;

    std::chrono::system_clock::time_point
    creationDate () const;

    const std::vector<uint64_t>&
    userList () const;

    Initiator
    initiator() const;

    const std::string&
    nodeKeyClear() const;

private:
    std::shared_ptr<data::UsersRelation> _data;
    mutable std::string                  _nodeKeyClear;

};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_USERRELATION_H_ */

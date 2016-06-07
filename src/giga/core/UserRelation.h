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

#ifndef GIGA_CORE_USERRELATION_H_
#define GIGA_CORE_USERRELATION_H_

#include <chrono>
#include <memory>
#include <vector>
#include <string>

namespace giga
{
class Application;

namespace data
{
struct UsersRelation;
}

namespace core
{
class User;

/**
 * A UserRelation represents the relation between the currentUser and an other user.
 * The ```UserRelation::Type``` enum tells what type of relation it is. Depending on
 * this type, the UserRelation may have different fields set.
 *
 * For most usage, you will not need much more than the Type of relation.
 */
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

    /**
     * @brief Get a list of users ids.
     *
     * @return
     * - for a ```Type::should_invite```: The list of suggested users
     * - for a ```Type::invite```: The list of users that has suggested this invitation
     * - for a ```Type::contact```: The list of users that has suggested this contact
     */
    const std::vector<uint64_t>&
    userList () const;

    /**
     * @brief Tells if a contact was initiated by the currentUser or the contact.
     */
    Initiator
    initiator() const;

private:
    friend class giga::Application; // utility::string_t Application::getNodeKeyClear(uint64_t) const;

    const std::string&
    nodeKeyClear(const User& currentUser) const;

private:
    std::shared_ptr<data::UsersRelation> _data;
    mutable std::string                  _nodeKeyClear; // cache data
    // TODO mutex the _nodeKeyClear.

};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_USERRELATION_H_ */

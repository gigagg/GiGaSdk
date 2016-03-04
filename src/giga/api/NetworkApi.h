/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef CLIENT_NETWORKAPI_API_H_
#define CLIENT_NETWORKAPI_API_H_

#include <cpprest/details/basic_types.h>
#include "GigaApi.h"

namespace giga
{

namespace data {
struct UsersRelation;
struct Success;
}

class NetworkApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::UsersRelation>>
    updateUserRelationData (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type, bool favorite, bool isNew);

    static pplx::task<std::shared_ptr<data::UsersRelation>>
    createUserRelation (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type, const utility::string_t& medium,
                            const std::string& key);

    static pplx::task<std::shared_ptr<data::Success>>
    deleteUserRelation (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type);

    static pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::UsersRelation>>>>
    getUserRelation (uint64_t userId, const utility::string_t& type, const utility::string_t& way);

    static pplx::task<std::shared_ptr<data::UsersRelation>>
    getUserRelation (uint64_t fromId, const utility::string_t& type, uint64_t toId, const utility::string_t& way);
};
} // namespace giga

#endif

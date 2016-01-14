/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef CLIENT_NETWORKAPI_API_H_
#define CLIENT_NETWORKAPI_API_H_

#include <iosfwd>
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
    updateUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type, bool favorite, bool isNew);

    static pplx::task<std::shared_ptr<data::UsersRelation>>
    createUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type, const std::string& medium,
                            const std::string& key);

    static pplx::task<std::shared_ptr<data::Success>>
    deleteUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type);

    static pplx::task<std::shared_ptr<data::UsersRelation>>
    getUserRelationData (int64_t userId, const std::string& type, const std::string& way);
};
} // namespace giga

#endif

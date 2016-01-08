/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "NetworkApi.h"
#include "data/UsersRelation.h"
#include "data/Success.h"
#include <string>

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::updateUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type, bool favorite, bool isNew)
{
    auto uri = client.uri ("users", fromUserId, "users", toUserId);
    auto body = JsonObj{};
    body.add ("type", type);
    body.add ("favorite", favorite);
    body.add ("isNew", isNew);
    return client.request<UsersRelation> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::createUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type, const std::string& medium,
                                    const std::string& key)
{
    auto uri = client.uri ("users", fromUserId, "users", toUserId);
    auto body = JsonObj{};
    body.add ("type", type);
    body.add ("medium", medium);
    body.add ("key", key);
    return client.request<UsersRelation> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Success>>
NetworkApi::deleteUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type)
{
    auto uri = client.uri ("users", fromUserId, "users", toUserId);
    uri.append_query ("type", type);
    return client.request<Success> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::getUserRelationData (int64_t userId, const std::string& type, const std::string& way)
{
    auto uri = client.uri ("users", userId, "users", userId);
    uri.append_query ("type", type);
    uri.append_query ("way", way);
    return client.request<UsersRelation> (methods::GET, uri);
}
} // namespace giga

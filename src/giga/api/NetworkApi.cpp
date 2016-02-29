/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "NetworkApi.h"
#include "data/UsersRelation.h"
#include "data/Success.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::updateUserRelationData (int64_t fromUserId, int64_t toUserId, const std::string& type, bool favorite, bool isNew)
{
    auto uri = client().uri ("users", fromUserId, "users", toUserId);
    auto body = JsonObj{};
    body.add ("type", type);
    body.add ("favorite", favorite);
    body.add ("isNew", isNew);
    return client().request<UsersRelation> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::createUserRelation (int64_t fromUserId, int64_t toUserId, const std::string& type, const std::string& medium,
                                    const std::string& key)
{
    auto uri = client().uri ("users", fromUserId, "users", toUserId);
    auto body = JsonObj{};
    body.add ("type", type);
    body.add ("medium", medium);
    body.add ("key", key);
    return client().request<UsersRelation> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Success>>
NetworkApi::deleteUserRelation (int64_t fromUserId, int64_t toUserId, const std::string& type)
{
    auto uri = client().uri ("users", fromUserId, "users", toUserId);
    uri.append_query ("type", type);
    return client().request<Success> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<UsersRelation>>>>
NetworkApi::getUserRelation (int64_t userId, const std::string& type, const std::string& way)
{
    auto uri = client().uri ("users", userId, "users");
    uri.append_query ("type", type);
    uri.append_query ("way", way);
    return client().request<std::vector<std::shared_ptr<UsersRelation>>> (methods::GET, uri);
}
} // namespace giga

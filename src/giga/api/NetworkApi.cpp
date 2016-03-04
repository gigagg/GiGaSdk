/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "NetworkApi.h"
#include "data/UsersRelation.h"
#include "data/Success.h"
#include "../utils/Utils.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::updateUserRelationData (uint64_t fromUserId, uint64_t toUserId, const string_t& type, bool favorite, bool isNew)
{
    auto uri = client().uri (U("users"), fromUserId, U("users"), toUserId);
    auto body = JsonObj{};
    body.add (U("type"), type);
    body.add (U("favorite"), favorite);
    body.add (U("isNew"), isNew);
    return client().request<UsersRelation> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<UsersRelation>>
NetworkApi::createUserRelation (uint64_t fromUserId, uint64_t toUserId, const string_t& type, const string_t& medium,
                                    const std::string& key)
{
    auto uri = client().uri (U("users"), fromUserId, U("users"), toUserId);
    auto body = JsonObj{};
    body.add (U("type"), type);
    body.add (U("medium"), medium);
    body.add (U("key"), utils::str2wstr(key));
    return client().request<UsersRelation> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Success>>
NetworkApi::deleteUserRelation (uint64_t fromUserId, uint64_t toUserId, const string_t& type)
{
    auto uri = client().uri (U("users"), fromUserId, U("users"), toUserId);
    uri.append_query (U("type"), type);
    return client().request<Success> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<UsersRelation>>>>
NetworkApi::getUserRelation (uint64_t userId, const string_t& type, const string_t& way)
{
    auto uri = client().uri (U("users"), userId, U("users"));
    uri.append_query (U("type"), type);
    uri.append_query (U("way"), way);
    return client().request<std::vector<std::shared_ptr<UsersRelation>>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<data::UsersRelation>>
NetworkApi::getUserRelation (uint64_t fromId, const utility::string_t& type, uint64_t toId, const utility::string_t& way)
{
    auto uri = client().uri (U("users"), fromId, U("users"), toId);
    if (type != U(""))
    {
        uri.append_query (U("type"), type);
    }
    uri.append_query (U("way"), way);
    return client().request<UsersRelation> (methods::GET, uri);
}

} // namespace giga

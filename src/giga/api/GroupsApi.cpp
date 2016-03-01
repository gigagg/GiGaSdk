/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "GroupsApi.h"
#include "data/Group.h"
#include "../rest/Empty.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<Group>>
GroupsApi::getAllGroups ()
{
    auto uri = client().uri (U("groups"));
    return client().request<Group> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::createGroup (const string_t& name)
{
    auto uri = client().uri (U("groups"));
    auto body = JsonObj{};
    body.add (U("name"), name);
    return client().request<Group> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::getGroupById (int64_t groupId)
{
    auto uri = client().uri (U("groups"), groupId);
    return client().request<Group> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::updateGroupName (int64_t groupId, const string_t& name)
{
    auto uri = client().uri (U("groups"), groupId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return client().request<Group> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<Empty>>
GroupsApi::deleteGroup (int64_t groupId)
{
    auto uri = client().uri (U("groups"), groupId);
    return client().request<Empty> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::shareNodeWithGroup (int64_t groupId, const string_t& nodeId)
{
    auto uri = client().uri (U("groups"), groupId, U("nodes"), nodeId);
    return client().request<Group> (methods::PUT, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::unshareNodeFromGroup (int64_t groupId, const string_t& nodeId)
{
    auto uri = client().uri (U("groups"), groupId, U("nodes"), nodeId);
    return client().request<Group> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::addUserToGroup (int64_t groupId, int64_t userId)
{
    auto uri = client().uri (U("groups"), groupId, U("users"), userId);
    return client().request<Group> (methods::PUT, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::removeUserFromGroup (int64_t groupId, int64_t userId)
{
    auto uri = client().uri (U("groups"), groupId, U("users"), userId);
    return client().request<Group> (methods::DEL, uri);
}

} // namespace giga

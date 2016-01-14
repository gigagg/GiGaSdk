/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "GroupsApi.h"
#include "data/Group.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<Group>>
GroupsApi::getAllGroups ()
{
    auto uri = client.uri ("groups");
    return client.request<Group> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::createGroup (const std::string& name)
{
    auto uri = client.uri ("groups");
    auto body = JsonObj{};
    body.add ("name", name);
    return client.request<Group> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::getGroupById (int64_t groupId)
{
    auto uri = client.uri ("groups", groupId);
    return client.request<Group> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::updateGroupName (int64_t groupId, const std::string& name)
{
    auto uri = client.uri ("groups", groupId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client.request<Group> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<std::string>>
GroupsApi::deleteGroup (int64_t groupId)
{
    auto uri = client.uri ("groups", groupId);
    return client.request<std::string> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::shareNodeWithGroup (int64_t groupId, const std::string& nodeId)
{
    auto uri = client.uri ("groups", groupId, "nodes", nodeId);
    return client.request<Group> (methods::PUT, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::unshareNodeFromGroup (int64_t groupId, const std::string& nodeId)
{
    auto uri = client.uri ("groups", groupId, "nodes", nodeId);
    return client.request<Group> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::addUserToGroup (int64_t groupId, int64_t userId)
{
    auto uri = client.uri ("groups", groupId, "users", userId);
    return client.request<Group> (methods::PUT, uri);
}

pplx::task<std::shared_ptr<Group>>
GroupsApi::removeUserFromGroup (int64_t groupId, int64_t userId)
{
    auto uri = client.uri ("groups", groupId, "users", userId);
    return client.request<Group> (methods::DEL, uri);
}

} // namespace giga

/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef CLIENT_GROUPSAPI_API_H_
#define CLIENT_GROUPSAPI_API_H_

#include <cpprest/details/basic_types.h>
#include "GigaApi.h"

namespace giga
{
struct Empty;

namespace data {
struct Group;
}

class GroupsApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::Group>>
    getAllGroups ();

    static pplx::task<std::shared_ptr<data::Group>>
    createGroup (const utility::string_t& name);

    static pplx::task<std::shared_ptr<data::Group>>
    getGroupById (int64_t groupId);

    static pplx::task<std::shared_ptr<data::Group>>
    updateGroupName (int64_t groupId, const utility::string_t& name);

    static pplx::task<std::shared_ptr<Empty>>
    deleteGroup (int64_t groupId);

    static pplx::task<std::shared_ptr<data::Group>>
    shareNodeWithGroup (int64_t groupId, const utility::string_t& nodeId);

    static pplx::task<std::shared_ptr<data::Group>>
    unshareNodeFromGroup (int64_t groupId, const utility::string_t& nodeId);

    static pplx::task<std::shared_ptr<data::Group>>
    addUserToGroup (int64_t groupId, uint64_t userId);

    static pplx::task<std::shared_ptr<data::Group>>
    removeUserFromGroup (int64_t groupId, uint64_t userId);
};
} // namespace giga

#endif

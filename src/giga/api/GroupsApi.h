/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef CLIENT_GROUPSAPI_API_H_
#define CLIENT_GROUPSAPI_API_H_

#include <iosfwd>
#include "GigaApi.h"

namespace giga
{

namespace data {
struct Group;
}

class GroupsApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::Group>>
    getAllGroups ();

    static pplx::task<std::shared_ptr<data::Group>>
    createGroup (const std::string& name);

    static pplx::task<std::shared_ptr<data::Group>>
    getGroupById (int64_t groupId);

    static pplx::task<std::shared_ptr<data::Group>>
    updateGroupName (int64_t groupId, const std::string& name);

    static pplx::task<std::shared_ptr<std::string>>
    deleteGroup (int64_t groupId);

    static pplx::task<std::shared_ptr<data::Group>>
    shareNodeWithGroup (int64_t groupId, const std::string& nodeId);

    static pplx::task<std::shared_ptr<data::Group>>
    unshareNodeFromGroup (int64_t groupId, const std::string& nodeId);

    static pplx::task<std::shared_ptr<data::Group>>
    addUserToGroup (int64_t groupId, int64_t userId);

    static pplx::task<std::shared_ptr<data::Group>>
    removeUserFromGroup (int64_t groupId, int64_t userId);
};
} // namespace giga

#endif

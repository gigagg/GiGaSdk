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

#ifndef CLIENT_GIGAAPI_H_
#define CLIENT_GIGAAPI_H_

#include "data/Locale.h"
#include "data/Group.h"
#include "data/IdContainer.h"
#include "data/NodeList.h"
#include "data/Node.h"
#include "data/Preview.h"
#include "data/Success.h"
#include "data/Timeline.h"
#include "data/DataNode.h"
#include "data/UsersRelation.h"
#include "data/UserExists.h"

#include "../rest/HttpClient.h"
#include "../rest/JsonObj.h"
#include "../rest/Empty.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <memory>
#include <mutex>

namespace giga {

namespace data {
struct User;
}

class GigaApi final
{
public:
    explicit GigaApi();
    ~GigaApi()                    = default;
    GigaApi(GigaApi&&)            = default;
    GigaApi& operator=(GigaApi&&) = default;
    GigaApi(const GigaApi&)            = delete;
    GigaApi& operator=(const GigaApi&) = delete;

public:
    pplx::task<std::shared_ptr<data::User>>
    authenticate(const utility::string_t& login, const utility::string_t& password);

    data::User&
    getCurrentUser();

    const std::shared_ptr<web::http::oauth2::experimental::oauth2_config>
    getOAuthConfig() const;

    pplx::task<void>
    refreshToken() const;

public:
    class GroupsApi final
    {
    private:
        friend class GigaApi;
        explicit GroupsApi(GigaApi& api):api(api){}
    public:
        pplx::task<std::shared_ptr<data::Group>>
        getAllGroups () const;

        pplx::task<std::shared_ptr<data::Group>>
        createGroup (const utility::string_t& name) const;

        pplx::task<std::shared_ptr<data::Group>>
        getGroupById (int64_t groupId) const;

        pplx::task<std::shared_ptr<data::Group>>
        updateGroupName (int64_t groupId, const utility::string_t& name) const;

        pplx::task<std::shared_ptr<Empty>>
        deleteGroup (int64_t groupId) const;

        pplx::task<std::shared_ptr<data::Group>>
        shareNodeWithGroup (int64_t groupId, const utility::string_t& nodeId) const;

        pplx::task<std::shared_ptr<data::Group>>
        unshareNodeFromGroup (int64_t groupId, const utility::string_t& nodeId) const;

        pplx::task<std::shared_ptr<data::Group>>
        addUserToGroup (int64_t groupId, uint64_t userId) const;

        pplx::task<std::shared_ptr<data::Group>>
        removeUserFromGroup (int64_t groupId, uint64_t userId) const;
    private:
        GigaApi& api;
    };

    class NetworkApi final
    {
    private:
        friend class GigaApi;
        explicit NetworkApi(GigaApi& api):api(api){}
    public:
        pplx::task<std::shared_ptr<data::UsersRelation>>
        updateUserRelationData (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type, bool favorite, bool isNew) const;

        pplx::task<std::shared_ptr<data::UsersRelation>>
        createUserRelation (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type, const utility::string_t& medium,
                                const std::string& key) const;

        pplx::task<std::shared_ptr<data::Success>>
        deleteUserRelation (uint64_t fromUserId, uint64_t toUserId, const utility::string_t& type) const;

        pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::UsersRelation>>>>
        getUserRelation (uint64_t userId, const utility::string_t& type, const utility::string_t& way) const;

        pplx::task<std::shared_ptr<data::UsersRelation>>
        getUserRelation (uint64_t fromId, const utility::string_t& type, uint64_t toId, const utility::string_t& way) const;
    private:
        GigaApi& api;
    };

    class NodesApi final
    {
    private:
        friend class GigaApi;
        explicit NodesApi(GigaApi& api):api(api){}
    public:
        pplx::task<std::shared_ptr<data::NodeList>>
        searchNode (const utility::string_t& search, const utility::string_t& mine, const std::string& inFolder, uint64_t ownerId) const;

        pplx::task<std::shared_ptr<std::vector<data::Node>>>
        searchNodeByType (const utility::string_t& search, const utility::string_t& type, uint16_t max = 50, uint32_t offset = 0) const;

        pplx::task<std::shared_ptr<data::DataNode>>
        addNode (const utility::string_t& name, const utility::string_t& type, const std::string& parentId, const std::string& fkey,
                 const std::string& fid) const;

        pplx::task<std::shared_ptr<data::DataNode>>
        addFolderNode (const utility::string_t& name, const std::string& parentId) const;

        pplx::task<std::shared_ptr<data::DataNode>>
        copyNode (const std::string& fromNodeId, const std::string& toNodeId, const utility::string_t& copy, const utility::string_t& cut,
                  const std::string& myNodeKey, const std::string& otherNodeKey) const;

        pplx::task<std::shared_ptr<data::Node>>
        getNodeById (const std::string& nodeId) const;

        pplx::task<std::shared_ptr<data::Node>>
        renameNode (const std::string& nodeId, const utility::string_t& name) const;

        pplx::task<std::shared_ptr<data::IdContainer>>
        deleteNode (const std::string& nodeId) const;

        pplx::task<std::shared_ptr<std::vector<data::Node>>>
        getChildrenNode (const std::string& nodeId) const;

        pplx::task<std::shared_ptr<data::Preview>>
        getPreviewsData (const std::string& nodeId) const;

        pplx::task<std::shared_ptr<data::Timeline>>
        getTimeline (const utility::string_t& head, uint64_t from, uint64_t owner) const;
    private:
        GigaApi& api;
    };

    class UsersApi final
    {
    private:
        friend class GigaApi;
        explicit UsersApi(GigaApi& api):api(api){}
    public:
        pplx::task<std::shared_ptr<data::User>>
        getCurrentUser () const;

        pplx::task<std::shared_ptr<std::vector<utility::string_t>>>
        searchTag (const utility::string_t& name) const;

        pplx::task<std::shared_ptr<data::UserExists>>
        userExists (const utility::string_t& login, const utility::string_t& email = U("")) const;

        pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
        searchUsers (const utility::string_t& search, const utility::string_t& activity, const utility::string_t& isSeeder) const;

        pplx::task<std::shared_ptr<data::User>>
        getUserById (uint64_t userId) const;

        pplx::task<std::shared_ptr<data::User>>
        getUserByLogin (const utility::string_t& login) const;

        pplx::task<std::shared_ptr<data::User>>
        updateUser (uint64_t userId, const utility::string_t& email, bool isValidation, const utility::string_t& gender, const utility::string_t& name,
                    const utility::string_t& description, const utility::string_t& birthdate, const utility::string_t& avatar, const utility::string_t& currentPassword,
                    const utility::string_t& password, const utility::string_t& clue, const utility::string_t& privateKey, const utility::string_t& iv,
                    const utility::string_t& salt) const;

        pplx::task<std::shared_ptr<data::User>>
        updateUserAddTag (uint64_t userId, const utility::string_t& name) const;

        pplx::task<std::shared_ptr<data::User>>
        updateUserRemoveTag (uint64_t userId, const utility::string_t& name) const;
    private:
        GigaApi& api;
    };

public:
    GroupsApi   groups;
    NetworkApi  network;
    NodesApi    nodes;
    UsersApi    users;

private:
    // todo protecte mutable with a mutex ...
    mutable HttpClient          client;
    std::shared_ptr<data::User> currentUser;
};

} // namespace giga
#endif /* CLIENT_GIGAAPI_H_ */

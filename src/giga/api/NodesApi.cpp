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

#include "GigaApi.h"
#include "data/Node.h"
#include "data/NodeList.h"
#include "data/Preview.h"
#include "data/Timeline.h"
#include "data/DataNode.h"
#include "data/IdContainer.h"
#include "../utils/Utils.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;
using giga::utils::str2wstr;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<NodeList>>
GigaApi::NodesApi::searchNode (const string_t& search, const string_t& mine, const std::string& inFolder, uint64_t ownerId) const
{
    auto uri = api._client.uri (U("nodes"));
    uri.append_query (U("search"), search);
    uri.append_query (U("mine"), mine);
    uri.append_query (U("inFolder"), str2wstr(inFolder));
    uri.append_query (U("ownerId"), ownerId);
    return api._client.request<NodeList> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<data::Node>>>
GigaApi::NodesApi::searchNodeByType (const string_t& search, const string_t& type, uint16_t max, uint32_t offset) const
{
    auto uri = api._client.uri (U("search"), type);
    uri.append_query (U("q"), search);
    uri.append_query (U("max"), max);
    uri.append_query (U("offset"), offset);
    return api._client.request<std::vector<data::Node>> (methods::GET, uri);}

pplx::task<std::shared_ptr<DataNode>>
GigaApi::NodesApi::addNode (const string_t& name, const string_t& type, const std::string& parentId, const std::string& fkey,
                   const std::string& fid, uint64_t mdate) const
{
    auto uri = api._client.uri (U("nodes"));
    auto body = JsonObj{};
    body.add (U("name"), name);
    body.add (U("type"), type);
    body.add (U("parentId"), str2wstr(parentId));
    body.add (U("fkey"), str2wstr(fkey));
    body.add (U("fid"), str2wstr(fid));
    body.add (U("mdate"), static_cast<int64_t>(mdate));
    return api._client.request<DataNode> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<DataNode>>
GigaApi::NodesApi::addFolderNode (const string_t& name, const std::string& parentId) const
{
    auto uri = api._client.uri (U("nodes"));
    auto body = JsonObj{};
    body.add (U("name"), name);
    body.add (U("type"), string_t(U("folder")));
    body.add (U("parentId"), str2wstr(parentId));
    try
    {
        return api._client.request<DataNode> (methods::POST, uri, std::move(body));
    }
    catch (const ErrorLocked& e)
    {
        return pplx::create_task([e](){
            JSonUnserializer j(e.getJson().at(U("data")));
            return j.unserialize<std::shared_ptr<DataNode>>();
        });
    }
}

pplx::task<std::shared_ptr<IdContainer>>
GigaApi::NodesApi::copyNode (const std::string& fromNodeId, const std::string& toNodeId, bool isCut,
                            const utility::string_t& mergePolicy,
                            const std::string& myNodeKey, const std::string& otherNodeKey) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(toNodeId), U("nodes"), str2wstr(fromNodeId));
    if (!mergePolicy.empty())
    {
        uri.append_query (U("mergePolicy"), mergePolicy);
    }
    if (isCut)
    {
        uri.append_query (U("cut"), U("true"));
    }
    else
    {
        uri.append_query (U("copy"), U("true"));
    }
    auto body = JsonObj{};
    if (myNodeKey != "") {
        body.add (U("myNodeKey"), myNodeKey);
    }
    if (otherNodeKey != "") {
        body.add (U("otherNodeKey"), otherNodeKey);
    }
    return api._client.request<IdContainer> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<data::CopyLog>>
GigaApi::NodesApi::getCopyLog (const std::string& fromNodeId, const std::string& toNodeId) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(toNodeId), U("nodes"), str2wstr(fromNodeId));
    return api._client.request<CopyLog> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
GigaApi::NodesApi::getNodeById (const std::string& nodeId) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(nodeId));
    return api._client.request<Node> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
GigaApi::NodesApi::renameNode (const std::string& nodeId, const string_t& name) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(nodeId));
    auto body = JsonObj{};
    body.add (U("name"), name);
    return api._client.request<Node> (methods::PUT, uri, std::move(body));
}

pplx::task<std::shared_ptr<IdContainer>>
GigaApi::NodesApi::deleteNode (const std::string& nodeId) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(nodeId));
    return api._client.request<IdContainer> (methods::DEL, uri);
}

pplx::task<std::shared_ptr<std::vector<Node>>>
GigaApi::NodesApi::getChildrenNode (const std::string& nodeId) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(nodeId), U("nodes"));
    return api._client.request<std::vector<Node>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Node>>
GigaApi::NodesApi::getChildrenNodeByName (const std::string& nodeId, const utility::string_t& name) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(nodeId), U("nodes"));
    uri.append_query(U("name"), name);
    return api._client.request<Node> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Preview>>
GigaApi::NodesApi::getPreviewsData (const std::string& nodeId) const
{
    auto uri = api._client.uri (U("nodes"), str2wstr(nodeId), U("previews"));
    return api._client.request<Preview> (methods::GET, uri);
}

pplx::task<std::shared_ptr<Timeline>>
GigaApi::NodesApi::getTimeline (const string_t& head, uint64_t from, uint64_t owner) const
{
    auto uri = api._client.uri (U("timelines"));
    uri.append_query (U("head"), head);
    uri.append_query (U("from"), from);
    uri.append_query (U("owner"), owner);
    return api._client.request<Timeline> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<data::SmallNode>>>
GigaApi::NodesApi::getAllFiles(const std::string& nodeId, const string_t& type) const
{
    auto uri = api._client.uri (U("nodes"), nodeId, U("files"), type);
    auto response = api._client.rawRequest(methods::GET, uri);
    return response.then([=](web::http::http_response response) {
        return response.extract_string();
    }).then([=](utility::string_t body) {
        utility::stringstream_t ss;
        ss.str(body);
        utility::string_t line;
        auto snodes = std::make_shared<std::vector<data::SmallNode>>();
        while (std::getline(ss, line, U('\n')))
        {
            try
            {
                auto snode = JSonUnserializer::fromString<data::SmallNode>(line);
                snodes->emplace_back(snode);
            }
            catch (const web::json::json_exception& ex)
            {
                GIGA_DEBUG_LOG(error, "error unserializing: " + utils::wstr2str(line));
            }
        }
        return snodes;
    });
}

pplx::task<uint64_t>
GigaApi::NodesApi::forEachFiles(const std::string& nodeId, const string_t& type, const std::function <void (web::json::value&&)>& fct) const
{
    auto uri = api._client.uri (U("nodes"), nodeId, U("files"), type);
    auto response = api._client.rawRequest(methods::GET, uri);
    return response.then([=](web::http::http_response response) {
        return response.extract_string();
    }).then([=](utility::string_t body) -> uint64_t {
        utility::stringstream_t ss;
        ss.str(body);
        utility::string_t line;
        while (std::getline(ss, line, U('\n')))
        {
            try
            {
                fct(web::json::value::parse(giga::utils::str2wstr(line)));
            }
            catch (const web::json::json_exception& ex)
            {
                GIGA_DEBUG_LOG(error, "error unserializing: " + utils::wstr2str(line));
            }
        }
        return 0;
    });
}

} // namespace giga

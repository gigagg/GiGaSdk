/*
 * FileApi.cpp
 *
 *  Created on: 14 janv. 2016
 *      Author: thomas
 */

#include "FileApi.h"
#include "data/Node.h"
#include "data/DataNode.h"
#include "data/User.h"
#include "NodesApi.h"
#include "../utils/Crypto.h"

#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <exception>
#include <utility>
#include <boost/filesystem.hpp>

using Concurrency::streams::basic_istream;
using Concurrency::streams::file_stream;
using giga::data::Node;
using pplx::create_task;
using pplx::task;
using web::http::client::http_client;
using web::http::client::http_client_config;
using web::http::http_request;
using web::http::http_response;
using web::http::methods;
using web::http::oauth2::experimental::oauth2_config;
using web::http::status_codes;
using web::uri;
using web::uri_builder;

using namespace boost::filesystem;


namespace giga
{

task<std::shared_ptr<data::Node>>
FileApi::uploadFile (const std::string& filename, const std::string& nodeName, const std::string& parentId)
{
    std::cout << filename << " " << nodeName << " " << parentId << std::endl;
    return create_task([=]() {
        auto sha1 = Crypto::sha1File(filename);
        auto fkey = Crypto::calculateFkey(sha1);

        auto& currUser = getCurrentUser();
        auto masterKey = Crypto::calculateMasterPassword(currUser.salt.get(), "gigatribe");
        auto privateKey = Crypto::aesDecrypt(masterKey,
                                             Crypto::base64decode(currUser.rsaKeys->aesSalt),
                                             Crypto::base64decode(currUser.rsaKeys->aesIv),
                                             Crypto::base64decode(currUser.rsaKeys->privateKey));
        auto rsa = Rsa{currUser.rsaKeys->publicKey, privateKey};
        auto nodeKeyClear = Crypto::base64decode(rsa.decrypt(Crypto::base64decode(currUser.nodeKey.get())));
        auto fkeyEnc = Crypto::aesEncrypt(nodeKeyClear.substr(0, 16), nodeKeyClear.substr(16, 16), fkey);

        return std::make_tuple(
                sha1,
                Crypto::calculateFid(sha1),
                Crypto::base64encode(fkeyEnc),
                Crypto::base64encode(nodeKeyClear));
    }).then([=] (std::tuple<std::string, std::string, std::string, std::string> result) {
        auto sha1 = std::get<0>(result);
        auto fid  = std::get<1>(result);
        auto fkey = std::get<2>(result);
        auto nk   = std::get<3>(result);

        try {
            // auto res = NodesApi::addNode(nodeName, "file", parentId, fkey, fid).get();
            auto res = NodesApi::addNode("randfile", "file", parentId, fkey, fid).get();
            return create_task([=]{
                return std::shared_ptr<Node>{std::move(res->data)};
            });
        } catch (const ErrorNotFound& e) {
            if (e.getJson().has_field("uploadUrl")) {
                auto uploadUrl = e.getJson().at("uploadUrl").as_string();
                // Do the actual upload here !

                return file_stream<unsigned char>::open_istream(filename).then([=](basic_istream<unsigned char> fileStream)
                {
                    auto size = file_size(filename);
                    auto uriBuilder = uri_builder(uri{"https:" + uploadUrl + web::uri::encode_data_string(nk)});
                    std::cout << uriBuilder.to_uri().to_string() << std::endl;

                    auto r = http_request{methods::POST};
                    r.set_request_uri(uriBuilder.to_uri());

                    r.set_body(fileStream, "application/octet-stream");
                    // r.headers().add("Content-Type", "application/octet-stream");
                    r.headers().add("Content-Disposition", "attachment, filename=\"" + web::uri::encode_data_string(nodeName) + "\"");
                    r.headers().add("Session-Id", "1559032-" + sha1);
                    r.headers().add("Content-Range", "bytes 0-" + std::to_string(size - 1) + "/" + std::to_string(size));
                    r.headers().add("Content-Length", std::to_string(size));

                    auto config = http_client_config{};
                    auto oauth = oauth2_config{*client.getClient().client_config().oauth2()};
                    config.set_oauth2(oauth);
                    config.set_validate_certificates(false);
                    auto c = http_client{uri{"https://" + uriBuilder.host() + "/"}, config};
//                    auto c = http_client{uri{"http://localhost:8088/"}, config};
                    return c.request(r).then([=](http_response response)
                    {
                        fileStream.close();
                        return client.onRequest<std::vector<std::shared_ptr<Node>>>(response);
                    }).then([](std::vector<std::shared_ptr<Node>> nodes)
                    {
                        if (nodes.size() == 1) {
                            return nodes[0];
                        }
                        THROW(ErrorException{"Incorrect number of nodes returned"});
                    });
                });
            }
            throw e;
        } catch (const ErrorLocked& e) {
            if (e.getJson().has_field("data")) {
                auto s = JSonUnserializer{e.getJson().at("data")};
                auto node = s.unserialize<std::shared_ptr<Node>>();
                return create_task([node]() {
                    return node;
                });
            }
            throw e;
        }
    });
}

pplx::task<std::shared_ptr<Node>>
FileApi::uploadChunk (const std::string& filename, const std::string& nodeName, const std::string& uploadUrl, const std::string& sha1)
{
    return file_stream<unsigned char>::open_istream(filename).then([=](basic_istream<unsigned char> fileStream)
    {
        auto r = http_request{methods::POST};
        r.set_request_uri(uploadUrl);
        r.set_body(fileStream, "application/octet-stream");
        r.headers().add("Content-Type", "application/octet-stream");
        r.headers().add("Content-Disposition", "attachment, filename=\"" + web::uri::encode_uri(nodeName) + "\"");
        r.headers().add("Session-Id", "1559032-" + sha1);
        return client.getClient().request(r).then([fileStream](http_response response)
        {
            fileStream.close();
            return client.onRequestPtr<Node>(response);
        });
    });
}

//$upload.http({
//      url: uploadUrl,
//      method: 'POST',
//      headers: {
//        "Content-Type": this.file.type || 'application/octet-stream',
//        'Content-Range' :'bytes ' + this.firstByte + '-' + this.lastByte + '/' + this.file.size,
//        'Content-Disposition': 'attachment, filename="' + encodeURIComponent(this.file.name || 'name') + '"',
//        'Session-Id' : this.sessionId
//      },
//      withCredentials: true,
//
//      data: this.view,
//      transformRequest: function(data) {
//        return data;
//      },
//      ignoreAutoError: true,
//      timeout: this.canceler.promise,
//      transformResponse : function(data, headersGetter) {
//        var headers = headersGetter();
//        if (headers['range'] === data ||
//            headers['Range'] === data ||
//            (data.length < 255 && data.length > 4 && data.match(/^[0-9/-]+$/))) {
//          return new Range(data);
//        }
//        return $http.defaults.transformResponse[0](data, headersGetter);
//      }

} /* namespace giga */

/*
 * GigaApi.h
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#ifndef CLIENT_GIGAAPI_H_
#define CLIENT_GIGAAPI_H_

#include <cpprest/http_client.h>

#include "HttpClient.h"
#include "../model/Locale.h"

using web::http::methods;

namespace giga {

class GigaApi
{
public:
    GigaApi() = default;
    GigaApi(const GigaApi&) = default;
    GigaApi(GigaApi&&) = default;
    virtual ~GigaApi() = default;

protected:
    static HttpClient client;
};

class TestApi final : GigaApi {
public:
    static pplx::task<std::shared_ptr<Test>> getTest() {
        auto uri = client.uri("test");
//        uri.append_query("param", param);
//        uri.append_query("param", param);
        return client.request<Test>(methods::GET, uri);
    }
};

} // namespace giga

#endif /* CLIENT_GIGAAPI_H_ */

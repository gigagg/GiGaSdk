/*
 * GigaApi.h
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#ifndef CLIENT_GIGAAPI_H_
#define CLIENT_GIGAAPI_H_

#include "../rest/HttpClient.h"
#include "../rest/JsonObj.h"
#include "data/Locale.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <memory>

namespace giga {

namespace data {
class User;
}

class GigaApi
{
public:
    GigaApi() = default;
    GigaApi(const GigaApi&) = default;
    GigaApi(GigaApi&&) = default;
    virtual ~GigaApi() = default;

public:
    static pplx::task<std::shared_ptr<data::User>> authenticate(const std::string& login, const std::string& password);

    static data::User& getCurrentUser();
    static std::shared_ptr<web::http::oauth2::experimental::oauth2_config> getOAuthConfig();

protected:
    static HttpClient& client();
    static std::shared_ptr<data::User> currentUser;
};

} // namespace giga
#endif /* CLIENT_GIGAAPI_H_ */

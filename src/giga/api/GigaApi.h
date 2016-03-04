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

#include "../rest/HttpClient.h"
#include "../rest/JsonObj.h"
#include "data/Locale.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <memory>

namespace giga {

namespace data {
struct User;
}

class GigaApi
{
public:
    GigaApi() = default;
    GigaApi(const GigaApi&) = default;
    GigaApi(GigaApi&&) = default;
    virtual ~GigaApi() = default;

public:
    static pplx::task<std::shared_ptr<data::User>> authenticate(const utility::string_t& login, const utility::string_t& password);

    static data::User& getCurrentUser();
    static std::shared_ptr<web::http::oauth2::experimental::oauth2_config> getOAuthConfig();

protected:
    static HttpClient& client();
    static std::shared_ptr<data::User> currentUser;
};

} // namespace giga
#endif /* CLIENT_GIGAAPI_H_ */

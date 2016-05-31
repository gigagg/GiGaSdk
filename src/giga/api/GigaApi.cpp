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
#include "data/User.h"
#include "data/UserExists.h"
#include "../utils/Crypto.h"

#include <pplx/pplxtasks.h>

using giga::data::User;
using pplx::create_task;
using pplx::task;
using utility::string_t;

namespace giga
{

GigaApi::GigaApi():
        groups{*this}, network{*this}, nodes{*this}, users{*this}, misc{*this},
        _client{}, _currentUser{nullptr}
{}

task<std::shared_ptr<User>>
GigaApi::authenticate (const string_t& login, const string_t& password)
{
    return create_task([=]
    {
        auto exists = users.userExists(login).get();
        if (exists->login.is_initialized())
        {
            auto realLogin = exists->login.get();
            _client.authenticate(realLogin, utils::str2wstr(Crypto::calculateLoginPassword(realLogin, password)));
            _currentUser = users.getCurrentUser().get();
            return _currentUser;
        }
        else
        {
            BOOST_THROW_EXCEPTION(ErrorNotFound{U("Login not found")});
        }
    });
}

data::User&
GigaApi::getCurrentUser()
{
    if (_currentUser) {
        return *_currentUser;
    }
    BOOST_THROW_EXCEPTION(ErrorException(U("You must authenticate before using getCurrentUser")));
}

const std::shared_ptr<web::http::oauth2::experimental::oauth2_config>
GigaApi::getOAuthConfig() const
{
    return _client.http().client_config().oauth2();
}

pplx::task<void>
GigaApi::refreshToken() const
{
    return _client.refreshToken();
}

utility::string_t
GigaApi::accessToken() const
{
    return _client.accessToken();
}

void
GigaApi::setUserAgent(utility::string_t userAgent)
{
    _client.setUserAgent(std::move(userAgent));
}

} // namespace giga

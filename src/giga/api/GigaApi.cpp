/*
 * GigaApi.cpp
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#include "GigaApi.h"
#include "UsersApi.h"
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

std::shared_ptr<data::User> GigaApi::currentUser{};

HttpClient&
GigaApi::client()
{
    static HttpClient cl{};
    return cl;
}

task<std::shared_ptr<User>>
GigaApi::authenticate (const string_t& login, const string_t& password)
{
    return create_task([=]
    {
        auto exists = UsersApi::userExists(login).get();
        if (exists->login.is_initialized())
        {
            auto realLogin = exists->login.get();
            client().authenticate(exists->login.get(), Crypto::calculateLoginPassword(exists->login.get(), password));
            currentUser = UsersApi::getCurrentUser().get();
            return currentUser;
        }
        else
        {
            BOOST_THROW_EXCEPTION(ErrorNotFound{"Login not found"});
        }
    });
}

data::User&
GigaApi::getCurrentUser()
{
    if (currentUser) {
        return *currentUser;
    }
    BOOST_THROW_EXCEPTION(ErrorException("You must authenticate before using getCurrentUser"));
}

std::shared_ptr<web::http::oauth2::experimental::oauth2_config>
GigaApi::getOAuthConfig()
{
    return client().http().client_config().oauth2();
}

} // namespace giga

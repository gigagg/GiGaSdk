/*
 * GigaApi.cpp
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#include "GigaApi.h"

#include "../utils/Crypto.h"
#include "UsersApi.h"
#include "data/UserExists.h"
#include "pplx/pplxtasks.h"

#include "data/User.h"
using giga::data::User;
using pplx::create_task;
using pplx::task;

namespace giga
{

HttpClient GigaApi::client{};
std::shared_ptr<data::User> GigaApi::currentUser{};

task<std::shared_ptr<User>>
GigaApi::authenticate (const std::string& login, const std::string& password)
{
    return create_task([=]
    {
        auto exists = UsersApi::userExists(login).get();
        if (exists->login.is_initialized())
        {
            auto realLogin = exists->login.get();
            client.authenticate(exists->login.get(), Crypto::calculateLoginPassword(exists->login.get(), password));
            currentUser = UsersApi::getCurrentUser().get();
            return currentUser;
        }
        else
        {
            THROW(ErrorNotFound{"Login not found"});
        }
    });
}

data::User&
GigaApi::getCurrentUser()
{
    if (currentUser) {
        return *currentUser;
    }
    THROW(ErrorException("You must authenticate before using getCurrentUser"));
}

} // namespace giga

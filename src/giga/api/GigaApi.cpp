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

using pplx::create_task;
using pplx::task;

namespace giga
{

HttpClient giga::GigaApi::client{};

task<std::string>
GigaApi::authenticate (const std::string& login, const std::string& password)
{
    return create_task([=]
    {
        auto exists = UsersApi::userExists(login).get();
        if (exists->login.is_initialized())
        {
            client.authenticate(exists->login.get(), Crypto::calculateLoginPassword(exists->login.get(), password));
            return exists->login.get();
        }
        else
        {
            throw ErrorNotFound{"Login not found"};
        }
    });
}

} // namespace giga

/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#include "UsersApi.h"
#include "data/User.h"
#include "data/UserExists.h"
#include "../utils/Crypto.h"

#include <cpprest/http_client.h>
#include <string>

using web::http::methods;
using utility::string_t;

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<User>>
UsersApi::getCurrentUser ()
{
    auto uri = client().uri (U("me"));
    return client().request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<string_t>>>
UsersApi::searchTag (const string_t& name)
{
    auto uri = client().uri (U("tags"));
    uri.append_query (U("name"), name);
    return client().request<std::vector<string_t>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<UserExists>>
UsersApi::userExists (const string_t& login, const string_t& email /* = U("")*/)
{
    auto uri = client().uri (U("userexists"));
    if (!login.empty()) {
        uri.append_query (U("login"), login);
    }
    if (!email.empty()) {
        uri.append_query (U("email"), email);
    }
    return client().request<UserExists> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
UsersApi::searchUsers (const string_t& search, const string_t& activity, const string_t& isSeeder)
{
    auto uri = client().uri (U("users"));
    uri.append_query (U("search"), search);
    if (activity != U(""))
    {
        uri.append_query (U("activity"), activity);
    }
    if (isSeeder != U(""))
    {
        uri.append_query (U("isSeeder"), isSeeder);
    }
    return client().request<std::vector<std::shared_ptr<data::User>>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::getUserById (uint64_t userId)
{
    auto uri = client().uri (U("users"), userId);
    return client().request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::getUserByLogin (const string_t& login)
{
    auto uri = client().uri (U("users"));
    uri.append_query(U("login"), login);
    return client().request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUser (uint64_t userId, const string_t& email, bool isValidation, const string_t& gender, const string_t& name,
                      const string_t& description, const string_t& birthdate, const string_t& avatar,
                      const string_t& currentPassword, const string_t& password, const string_t& clue,
                      const string_t& privateKey, const string_t& iv, const string_t& salt)
{
    auto uri = client().uri (U("users"), userId);
    auto body = JsonObj{};
    body.add (U("email"), email);
    body.add (U("isValidation"), isValidation);
    body.add (U("gender"), gender);
    body.add (U("name"), name);
    body.add (U("description"), description);
    body.add (U("birthdate"), birthdate);
    body.add (U("avatar"), avatar);
    body.add (U("currentPassword"), currentPassword);
    body.add (U("password"), password);
    body.add (U("clue"), clue);
    body.add (U("privateKey"), privateKey);
    body.add (U("iv"), iv);
    body.add (U("salt"), salt);
    return client().request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUserAddTag (uint64_t userId, const string_t& name)
{
    auto uri = client().uri (U("users"), userId, U("tags"), userId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return client().request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUserRemoveTag (uint64_t userId, const string_t& name)
{
    auto uri = client().uri (U("users"), userId, U("tags"), userId);
    auto body = JsonObj{};
    body.add (U("name"), name);
    return client().request<User> (methods::DEL, uri, std::move(body));
}

} // namespace giga

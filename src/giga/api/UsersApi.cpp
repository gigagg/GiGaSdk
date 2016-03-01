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
    auto uri = client().uri ("me");
    return client().request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<string_t>>>
UsersApi::searchTag (const string_t& name)
{
    auto uri = client().uri ("tags");
    uri.append_query ("name", name);
    return client().request<std::vector<string_t>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<UserExists>>
UsersApi::userExists (const string_t& login, const string_t& email /* = ""*/)
{
    auto uri = client().uri ("userexists");
    if (!login.empty()) {
        uri.append_query ("login", login);
    }
    if (!email.empty()) {
        uri.append_query ("email", email);
    }
    return client().request<UserExists> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
UsersApi::searchUsers (const string_t& search, const string_t& activity, const string_t& isSeeder)
{
    auto uri = client().uri ("users");
    uri.append_query ("search", search);
    if (activity != "")
    {
        uri.append_query ("activity", activity);
    }
    if (isSeeder != "")
    {
        uri.append_query ("isSeeder", isSeeder);
    }
    return client().request<std::vector<std::shared_ptr<data::User>>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::getUserById (int64_t userId)
{
    auto uri = client().uri ("users", userId);
    return client().request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::getUserByLogin (const string_t& login)
{
    auto uri = client().uri ("users");
    uri.append_query("login", login);
    return client().request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUser (int64_t userId, const string_t& email, bool isValidation, const string_t& gender, const string_t& name,
                      const string_t& description, const string_t& birthdate, const string_t& avatar,
                      const string_t& currentPassword, const string_t& password, const string_t& clue,
                      const string_t& privateKey, const string_t& iv, const string_t& salt)
{
    auto uri = client().uri ("users", userId);
    auto body = JsonObj{};
    body.add ("email", email);
    body.add ("isValidation", isValidation);
    body.add ("gender", gender);
    body.add ("name", name);
    body.add ("description", description);
    body.add ("birthdate", birthdate);
    body.add ("avatar", avatar);
    body.add ("currentPassword", currentPassword);
    body.add ("password", password);
    body.add ("clue", clue);
    body.add ("privateKey", privateKey);
    body.add ("iv", iv);
    body.add ("salt", salt);
    return client().request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUserAddTag (int64_t userId, const string_t& name)
{
    auto uri = client().uri ("users", userId, "tags", userId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client().request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUserRemoveTag (int64_t userId, const string_t& name)
{
    auto uri = client().uri ("users", userId, "tags", userId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client().request<User> (methods::DEL, uri, std::move(body));
}

} // namespace giga

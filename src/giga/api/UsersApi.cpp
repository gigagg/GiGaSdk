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

namespace giga
{
using namespace data;

pplx::task<std::shared_ptr<User>>
UsersApi::getCurrentUser ()
{
    auto uri = client.uri ("me");
    return client.request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<std::string>>>
UsersApi::searchTag (const std::string& name)
{
    auto uri = client.uri ("tags");
    uri.append_query ("name", name);
    return client.request<std::vector<std::string>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<UserExists>>
UsersApi::userExists (const std::string& login, const std::string& email /* = ""*/)
{
    auto uri = client.uri ("userexists");
    if (!login.empty()) {
        uri.append_query ("login", login);
    }
    if (!email.empty()) {
        uri.append_query ("email", email);
    }
    return client.request<UserExists> (methods::GET, uri);
}

pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
UsersApi::searchUsers (const std::string& search, const std::string& activity, const std::string& isSeeder)
{
    auto uri = client.uri ("users");
    uri.append_query ("search", search);
    if (activity != "")
    {
        uri.append_query ("activity", activity);
    }
    if (isSeeder != "")
    {
        uri.append_query ("isSeeder", isSeeder);
    }
    return client.request<std::vector<std::shared_ptr<data::User>>> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::getUserById (int64_t userId)
{
    auto uri = client.uri ("users", userId);
    return client.request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::getUserByLogin (const std::string& login)
{
    auto uri = client.uri ("users");
    uri.append_query("login", login);
    return client.request<User> (methods::GET, uri);
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUser (int64_t userId, const std::string& email, bool isValidation, const std::string& gender, const std::string& name,
                      const std::string& description, const std::string& birthdate, const std::string& avatar,
                      const std::string& currentPassword, const std::string& password, const std::string& clue,
                      const std::string& privateKey, const std::string& iv, const std::string& salt)
{
    auto uri = client.uri ("users", userId);
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
    return client.request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUserAddTag (int64_t userId, const std::string& name)
{
    auto uri = client.uri ("users", userId, "tags", userId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client.request<User> (methods::POST, uri, std::move(body));
}

pplx::task<std::shared_ptr<User>>
UsersApi::updateUserRemoveTag (int64_t userId, const std::string& name)
{
    auto uri = client.uri ("users", userId, "tags", userId);
    auto body = JsonObj{};
    body.add ("name", name);
    return client.request<User> (methods::DEL, uri, std::move(body));
}

} // namespace giga

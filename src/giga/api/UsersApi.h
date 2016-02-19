/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef CLIENT_USERSAPI_API_H_
#define CLIENT_USERSAPI_API_H_

#include <iosfwd>
#include "GigaApi.h"

namespace giga
{

namespace data {
struct User;
struct UserExists;
}

class UsersApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::User>>
    getCurrentUser ();

    static pplx::task<std::shared_ptr<std::vector<std::string>>>
    searchTag (const std::string& name);

    static pplx::task<std::shared_ptr<data::UserExists>>
    userExists (const std::string& login, const std::string& email = "");

    static pplx::task<std::shared_ptr<std::vector<std::shared_ptr<data::User>>>>
    searchUsers (const std::string& search, const std::string& activity, const std::string& isSeeder);

    static pplx::task<std::shared_ptr<data::User>>
    getUserById (int64_t userId);

    static pplx::task<std::shared_ptr<data::User>>
    getUserByLogin (const std::string& login);

    static pplx::task<std::shared_ptr<data::User>>
    updateUser (int64_t userId, const std::string& email, bool isValidation, const std::string& gender, const std::string& name,
                const std::string& description, const std::string& birthdate, const std::string& avatar, const std::string& currentPassword,
                const std::string& password, const std::string& clue, const std::string& privateKey, const std::string& iv,
                const std::string& salt);

    static pplx::task<std::shared_ptr<data::User>>
    updateUserAddTag (int64_t userId, const std::string& name);

    static pplx::task<std::shared_ptr<data::User>>
    updateUserRemoveTag (int64_t userId, const std::string& name);
};
} // namespace giga

#endif

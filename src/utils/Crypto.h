#ifndef UTILS_CRYPTO_H_
#define UTILS_CRYPTO_H_

#include <string>

namespace giga
{

class Crypto final
{
public:
    static std::string pbkdf2_sha256(const std::string& password, const std::string& salt, std::size_t length, std::size_t iteration = 1024);
    static std::string pbkdf2_sha512(const std::string& password, const std::string& salt, std::size_t length, std::size_t iteration = 1024);

    static std::string base64encode(const std::string& data);
    static std::string base64decode(const std::string& data);

    static std::string calculateFid(const std::string& hashHexEncoded);
    static std::string calculateFkey(const std::string& hashHexEncoded);

    static std::string calculateLoginPassword(const std::string& login, const std::string& password);
    static std::string calculateMasterPassword(const std::string& salt, const std::string& password);
};

} /* namespace giga */

#endif /* UTILS_CRYPTO_H_ */

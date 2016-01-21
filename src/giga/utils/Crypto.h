#ifndef UTILS_CRYPTO_H_
#define UTILS_CRYPTO_H_

#include <string>
#include <tuple>
#include <utility>

#ifdef CRYPTOPP
#include "cryptopp/rsa.h"
#else
#include "crypto++/rsa.h"
#endif


namespace giga
{

class Rsa final {
public:
    explicit Rsa(const std::string& pubStr, const std::string& privStr = "");

    std::string encrypt(const std::string& data) const;
    std::string decrypt(const std::string& data) const;

private:
    CryptoPP::RSA::PublicKey pub;
    CryptoPP::RSA::PrivateKey priv;
    bool hasPrivateKey;
};

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

    static std::string sha1File(const std::string& filename);

    // AES
    enum struct aesTuple : int {
        data = 0, iv, salt
    };
    static std::tuple<std::string, std::string, std::string>
    aesEncrypt(const std::string& password, const std::string& data);
    static std::string
    aesDecrypt(const std::string& password, const std::string& saltStr, const std::string& ivStr, const std::string& data);
    static std::string
    aesEncrypt (const std::string& key, const std::string& iv, const std::string& data);
};

template<Crypto::aesTuple n, class... Types >
typename std::tuple_element<static_cast< std::size_t >( n ), std::tuple<Types...> >::type&
    get( std::tuple<Types...>& t )
{
    return std::get< static_cast< std::size_t >( n ), Types... >( t );
}

} /* namespace giga */

#endif /* UTILS_CRYPTO_H_ */

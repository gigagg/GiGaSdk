#ifndef UTILS_CRYPTO_H_
#define UTILS_CRYPTO_H_

#include <string>
#include <tuple>
#include <utility>
#include <cpprest/details/basic_types.h>

#ifdef CRYPTOPP
#include "cryptopp/rsa.h"
#else
#include "crypto++/rsa.h"
#endif

namespace giga
{

class Rsa final
{
public:
    explicit
    Rsa (const utility::string_t& pubStr, const utility::string_t& privStr = "");

    utility::string_t
    encrypt (const utility::string_t& data) const;

    utility::string_t
    decrypt (const utility::string_t& data) const;

private:
    CryptoPP::RSA::PublicKey    _pub;
    CryptoPP::RSA::PrivateKey   _priv;
    bool                        _hasPrivateKey;
};

class Crypto final
{
public:
    static utility::string_t
    pbkdf2_sha256 (const utility::string_t& password, const utility::string_t& salt, std::size_t length, std::size_t iteration = 1024);

    static utility::string_t
    pbkdf2_sha512 (const utility::string_t& password, const utility::string_t& salt, std::size_t length, std::size_t iteration = 1024);

public:
    static utility::string_t
    base64encode (const utility::string_t& data);

    static utility::string_t
    base64decode (const utility::string_t& data);

public:
    static utility::string_t
    calculateFid (const utility::string_t& hashHexEncoded);

    static utility::string_t
    calculateFkey (const utility::string_t& hashHexEncoded);

public:
    static utility::string_t
    calculateLoginPassword (const utility::string_t& login, const utility::string_t& password);

    static utility::string_t
    calculateMasterPassword (const utility::string_t& salt, const utility::string_t& password);

    static utility::string_t
    sha1File (const utility::string_t& filename);

public:
    enum struct AesTuple : int
    {
        data = 0, iv, salt
    };

    static std::tuple<utility::string_t, utility::string_t, utility::string_t>
    aesEncrypt (const utility::string_t& password, const utility::string_t& data);

    static utility::string_t
    aesDecrypt (const utility::string_t& password, const utility::string_t& saltStr, const utility::string_t& ivStr, const utility::string_t& data);

    static utility::string_t
    aesEncrypt (const utility::string_t& key, const utility::string_t& iv, const utility::string_t& data);
};

template<Crypto::AesTuple n, class ... Types>
typename std::tuple_element<static_cast<std::size_t>(n), std::tuple<Types...> >::type&
get (std::tuple<Types...>& t)
{
    return std::get<static_cast<std::size_t>(n), Types...>(t);
}

} /* namespace giga */

#endif /* UTILS_CRYPTO_H_ */

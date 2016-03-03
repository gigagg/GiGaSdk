#ifndef UTILS_CRYPTO_H_
#define UTILS_CRYPTO_H_

#include <string>
#include <tuple>
#include <utility>
#include <cpprest/details/basic_types.h>

namespace giga
{

namespace
{
class RsaKeys;
}
class Rsa final
{
public:
    explicit
    Rsa (const std::string& pubStr, const std::string& privStr = "");
	~Rsa();
	Rsa& operator=(const Rsa& rhs);
	Rsa(const Rsa& rhs);
	Rsa& operator=(Rsa&& rhs);
	Rsa(Rsa&& rhs);

	std::string
    encrypt (const std::string& data) const;

	std::string
    decrypt (const std::string& data) const;

private:
	std::unique_ptr<RsaKeys>    _keys;
    bool                        _hasPrivateKey;
};

class Crypto final
{
public:
    static std::string
    pbkdf2_sha256 (const utility::string_t& password, const std::string& salt, std::size_t length, std::size_t iteration = 1024);

    static std::string
    pbkdf2_sha512 (const utility::string_t& password, const std::string& salt, std::size_t length, std::size_t iteration = 1024);

public:
    static std::string
    base64encode (const std::string& data);

    static std::string
    base64decode (const std::string& data);

public:
    static std::string
    calculateFid (const std::string& hashHexEncoded);

    static std::string
    calculateFkey (const std::string& hashHexEncoded);

public:
    static std::string
    calculateLoginPassword (const utility::string_t& login, const utility::string_t& password);

    static std::string
    calculateMasterPassword (const std::string& salt, const utility::string_t& password);

    static std::string
    sha1File (const utility::string_t& filename);

public:
    enum struct AesTuple : int
    {
        data = 0, iv, salt
    };

    static std::tuple<std::string, std::string, std::string>
    aesEncrypt (const utility::string_t& password, const std::string& data);

    static std::string
    aesDecrypt (const utility::string_t& password, const std::string& saltStr, const std::string& ivStr, const std::string& data);

    static std::string
    aesEncrypt (const std::string& key, const std::string& iv, const std::string& data);
};

template<Crypto::AesTuple n, class ... Types>
typename std::tuple_element<static_cast<std::size_t>(n), std::tuple<Types...> >::type&
get (std::tuple<Types...>& t)
{
    return std::get<static_cast<std::size_t>(n), Types...>(t);
}

} /* namespace giga */

#endif /* UTILS_CRYPTO_H_ */

/*
 * Crypto.cpp
 *
 *  Created on: 7 janv. 2016
 *      Author: thomas
 */

#include "Crypto.h"

#ifdef CRYPTOPP
#include "cryptopp/integer.h"
#include "cryptopp/osrng.h"
#include "cryptopp/cryptlib.h"
#include "cryptopp/hex.h"
#include "cryptopp/filters.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/pwdbased.h"
#include "cryptopp/files.h"
#include "cryptopp/base64.h"
#else
#include "crypto++/integer.h"
#include "crypto++/osrng.h"
#include "crypto++/cryptlib.h"
#include "crypto++/hex.h"
#include "crypto++/filters.h"
#include "crypto++/files.h"
#include "crypto++/aes.h"
#include "crypto++/modes.h"
#include "crypto++/pwdbased.h"
#include "crypto++/base64.h"
#endif

using CryptoPP::Exception;
using CryptoPP::StringSink;
using CryptoPP::ArraySink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;
using CryptoPP::FileSource;
using CryptoPP::AES;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::CTR_Mode;
using CryptoPP::FileSink;
using CryptoPP::Base64Decoder;
using CryptoPP::Base64Encoder;
using CryptoPP::PKCS5_PBKDF2_HMAC;

namespace giga
{

template<typename T>
std::string
pbkdf2 (const std::string& password, const std::string& salt, std::size_t length, std::size_t iteration)
{
    auto key = std::vector<char>(length);
    PKCS5_PBKDF2_HMAC<T> passToKey;
    passToKey.DeriveKey(reinterpret_cast<byte*>(key.data()),
                        key.size(),
                        '\0',
                        reinterpret_cast<const byte*>(password.c_str()),
                        password.size(),
                        reinterpret_cast<const byte*>(salt.c_str()),
                        salt.size(),
                        iteration);

    return {key.begin(), key.end()};
}

std::string
Crypto::pbkdf2_sha256 (const std::string& password, const std::string& salt, std::size_t length, std::size_t iteration)
{
    return pbkdf2<CryptoPP::SHA256> (password, salt, length, iteration);
}
std::string
Crypto::pbkdf2_sha512 (const std::string& password, const std::string& salt, std::size_t length, std::size_t iteration)
{
    return pbkdf2<CryptoPP::SHA512> (password, salt, length, iteration);
}

std::string
Crypto::base64encode (const std::string& data)
{
    std::string encoded;
    StringSource ss(reinterpret_cast<const byte*> (data.data()), data.size(), true,
        new Base64Encoder(
            new StringSink(encoded)
        ) // Base64Encoder
    ); // StringSource

    // remove the line feed char at the end (why is it here ?)
    if (encoded[encoded.size() - 1] == 10)
    {
        encoded.pop_back();
    }
    return encoded;
}

std::string
Crypto::base64decode (const std::string& data)
{
    std::string decoded;
    StringSource ss(reinterpret_cast<const byte*> (data.data()), data.size(), true,
        new Base64Decoder(
            new StringSink(decoded)
        ) // Base64Encoder
    ); // StringSource
    return decoded;
}

std::string
Crypto::calculateFid(const std::string& hashHexEncoded) {
    auto salt = "5%;[yw\"XG2&Om#i*T$v.B2'Ae/VST4t#u$@pxsauO,H){`hUd7Xu@4q4WCc<>'ie";
    return base64encode(pbkdf2_sha256(hashHexEncoded, salt, 18, 32));
}

std::string
Crypto::calculateFkey(const std::string& hashHexEncoded) {
    auto salt = "={w|>6L:{Xn;HAKf^w=,fgSX}sfw)`hxopaqk.6Hg';w23\"sd+b07`LSOGqz#-)[";
    return base64encode(pbkdf2_sha256(hashHexEncoded, salt, 18, 32));
}

std::string
Crypto::calculateLoginPassword(const std::string& login, const std::string& password) {
    auto salt = login + "\"D<?4'V%Fh(U,9SjdO4v)|1mJV31]#;W";
    auto hashed = pbkdf2_sha256(password, salt, 16);
    return base64encode(hashed);
}

std::string
Crypto::calculateMasterPassword(const std::string& salt, const std::string& password) {
    return base64encode(pbkdf2_sha256(password, salt, 16));
}

} /* namespace giga */

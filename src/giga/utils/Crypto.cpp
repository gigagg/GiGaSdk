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
#include "cryptopp/rsa.h"
#else
#include "crypto++/integer.h"
#include "crypto++/osrng.h"
#include "crypto++/cryptlib.h"
#include "crypto++/hex.h"
#include "crypto++/filters.h"
#include "crypto++/files.h"
#include "crypto++/aes.h"
#include "crypto++/rsa.h"
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
using CryptoPP::RSA;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::CTR_Mode;
using CryptoPP::FileSink;
using CryptoPP::Base64Decoder;
using CryptoPP::Base64Encoder;
using CryptoPP::ByteQueue;
using CryptoPP::CBC_Mode;
using CryptoPP::Integer;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::PK_DecryptorFilter;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PKCS5_PBKDF2_HMAC;
using CryptoPP::Redirector;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_PKCS1v15_Decryptor;
using CryptoPP::RSAES_PKCS1v15_Encryptor;
using CryptoPP::SecByteBlock;

namespace giga
{

void fillQueue(ByteQueue& queue, const std::string& b64encoded) {
    Base64Decoder decoder;
    decoder.Attach(new Redirector(queue));
    decoder.Put(reinterpret_cast<const byte*>(b64encoded.data()), b64encoded.size());
    decoder.MessageEnd();
}

Rsa::Rsa (const std::string& pubStr, const std::string& privStr)
{
    try {
        ByteQueue queue;
        fillQueue(queue, pubStr);
        pub.BERDecodePublicKey(queue, false, queue.MaxRetrievable());
    } catch (const std::exception&) {
        ByteQueue queue;
        fillQueue(queue, pubStr);
        pub.BERDecode(queue);
    }

    try {
        ByteQueue queue;
        fillQueue(queue, privStr);
        priv.BERDecodePrivateKey(queue, false, queue.MaxRetrievable());
    } catch (const std::exception&) {
        ByteQueue queue;
        fillQueue(queue, privStr);
        priv.BERDecode(queue);
    }

    AutoSeededRandomPool rnd;
    if(!priv.Validate(rnd, 3))
        throw std::runtime_error("Rsa private key validation failed");

    if(!priv.Validate(rnd, 3))
        throw std::runtime_error("Dsa private key validation failed");
}

std::string
Rsa::encrypt (const std::string& data) const
{
    AutoSeededRandomPool rng;
    RSAES_PKCS1v15_Encryptor encryptor( pub );
    std::string encrypted;
    StringSource ss(data, true,
        new PK_EncryptorFilter(rng, encryptor,
            new StringSink(encrypted)
        )
    );
    return encrypted;
}

std::string
Rsa::decrypt (const std::string& data) const
{
    AutoSeededRandomPool rng;
    RSAES_PKCS1v15_Decryptor decryptor( priv );
    std::string decrypted;
    StringSource ss(data, true,
        new PK_DecryptorFilter(rng, decryptor,
            new StringSink(decrypted)
        )
    );
    return decrypted;
}


template<typename T>
std::string
pbkdf2 (const std::string& password, const std::string& salt, std::size_t length, std::size_t iteration)
{
    auto key = std::vector<char>(length);
    PKCS5_PBKDF2_HMAC<T> passToKey;
    passToKey.DeriveKey(reinterpret_cast<byte*>(key.data()),
                        key.size(),
                        '\0',
                        reinterpret_cast<const byte*>(password.data()),
                        password.size(),
                        reinterpret_cast<const byte*>(salt.data()),
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

std::tuple<std::string, std::string, std::string>
Crypto::aesEncrypt (const std::string& password, const std::string& data)
{

    SecByteBlock iv(16);
    SecByteBlock salt(8);

    OS_GenerateRandomBlock(true, salt, salt.size());
    OS_GenerateRandomBlock(false, iv, iv.size());

    auto saltStr = std::string(salt.begin(), salt.end());
    auto ivStr = std::string(iv.begin(), iv.end());
    auto key = pbkdf2_sha256(password, saltStr, 16);

    CBC_Mode<AES>::Encryption e;
    e.SetKeyWithIV(reinterpret_cast<const byte*>(key.data()), key.size(), iv, ivStr.size());

    std::string encrypted;
    StringSource ss(reinterpret_cast<const byte*> (data.data()), data.size(), true,
        new StreamTransformationFilter(e,
            new StringSink(encrypted)
        )
    );

    return std::make_tuple(
            encrypted,
            ivStr,
            saltStr
    );
}
std::string
Crypto::aesDecrypt (const std::string& password, const std::string& saltStr, const std::string& ivStr, const std::string& data)
{
    auto key = pbkdf2_sha256(password, saltStr, 16);

    CBC_Mode<AES>::Decryption e;
    e.SetKeyWithIV(reinterpret_cast<const byte*>(key.data()),
                   key.size(),
                   reinterpret_cast<const byte*>(ivStr.data()),
                   ivStr.size());

    std::string decrypted;
    StringSource ss(reinterpret_cast<const byte*> (data.data()), data.size(), true,
        new StreamTransformationFilter(e,
            new StringSink(decrypted)
        )
    );

    return decrypted;
}

} /* namespace giga */

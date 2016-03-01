/*
 * Crypto.cpp
 *
 *  Created on: 7 janv. 2016
 *      Author: thomas
 */

#include "Crypto.h"
#include "../rest/HttpErrors.h"

#include <cpprest/details/basic_types.h>
#include <algorithm>

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
using utility::string_t;

namespace {

const byte*
toByteCst (const string_t& str)
{
    return reinterpret_cast<const byte*>(str.data());
}
byte*
toByte (std::vector<char>& str)
{
    return reinterpret_cast<byte*>(str.data());
}

void fillQueue(ByteQueue& queue, const string_t& b64encoded) {
    Base64Decoder decoder;
    decoder.Attach(new Redirector(queue));
    decoder.Put(toByteCst(b64encoded), b64encoded.size());
    decoder.MessageEnd();
}

}

namespace giga
{

Rsa::Rsa (const string_t& pubStr, const string_t& privStr)
{
    try
    {
        ByteQueue queue;
        fillQueue(queue, pubStr);
        _pub.BERDecodePublicKey(queue, false, queue.MaxRetrievable());
    }
    catch (const std::exception&)
    {
        ByteQueue queue;
        fillQueue(queue, pubStr);
        _pub.BERDecode(queue);
    }
    _hasPrivateKey = false;
    if (privStr != U(""))
    {
        try
        {
            ByteQueue queue;
            fillQueue(queue, privStr);
            _priv.BERDecodePrivateKey(queue, false, queue.MaxRetrievable());
            _hasPrivateKey = true;
        }
        catch (const std::exception&)
        {
            ByteQueue queue;
            fillQueue(queue, privStr);
            _priv.BERDecode(queue);
            _hasPrivateKey = true;
        }

        AutoSeededRandomPool rnd;
        if (!_priv.Validate(rnd, 3))
        {
            BOOST_THROW_EXCEPTION(ErrorException(U("Rsa private key validation failed")));
        }

        if (!_priv.Validate(rnd, 3))
        {
            BOOST_THROW_EXCEPTION(ErrorException(U("Dsa private key validation failed")));
        }
    }
}

string_t
Rsa::encrypt (const string_t& data) const
{
    AutoSeededRandomPool rng;
    RSAES_PKCS1v15_Encryptor encryptor( _pub );
    string_t encrypted;
    StringSource ss(data, true,
        new PK_EncryptorFilter(rng, encryptor,
            new StringSink(encrypted)
        )
    );
    return encrypted;
}

string_t
Rsa::decrypt (const string_t& data) const
{
    if (!_hasPrivateKey) {
        BOOST_THROW_EXCEPTION(ErrorException(U("PrivateKey has not been set")));
    }
    AutoSeededRandomPool rng;
    RSAES_PKCS1v15_Decryptor decryptor(_priv);
    string_t decrypted;
    StringSource ss(data, true,
        new PK_DecryptorFilter(rng, decryptor,
            new StringSink(decrypted)
        )
    );
    return decrypted;
}


template<typename T>
string_t
pbkdf2 (const string_t& password, const string_t& salt, std::size_t length, std::size_t iteration)
{
    auto key = std::vector<char>(length);
    PKCS5_PBKDF2_HMAC<T> passToKey;
    passToKey.DeriveKey(toByte(key),
                        key.size(),
                        '\0',
                        toByteCst(password),
                        password.size(),
                        toByteCst(salt),
                        salt.size(),
                        iteration);

    return {key.begin(), key.end()};
}

string_t
Crypto::pbkdf2_sha256 (const string_t& password, const string_t& salt, std::size_t length, std::size_t iteration)
{
    return pbkdf2<CryptoPP::SHA256> (password, salt, length, iteration);
}
string_t
Crypto::pbkdf2_sha512 (const string_t& password, const string_t& salt, std::size_t length, std::size_t iteration)
{
    return pbkdf2<CryptoPP::SHA512> (password, salt, length, iteration);
}

string_t
Crypto::base64encode (const string_t& data)
{
    string_t encoded;
    StringSource ss(toByteCst(data), data.size(), true,
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

string_t
Crypto::base64decode (const string_t& data)
{
    string_t decoded;
    StringSource ss(toByteCst(data), data.size(), true,
        new Base64Decoder(
            new StringSink(decoded)
        ) // Base64Encoder
    ); // StringSource
    return decoded;
}

string_t
Crypto::calculateFid(const string_t& hashHexEncoded) {
    auto salt = U("5%;[yw\"XG2&Om#i*T$v.B2'Ae/VST4t#u$@pxsauO,H){`hUd7Xu@4q4WCc<>'ie");
    return base64encode(pbkdf2_sha256(hashHexEncoded, salt, 18, 32));
}

string_t
Crypto::calculateFkey(const string_t& hashHexEncoded) {
    auto salt = U("={w|>6L:{Xn;HAKf^w=,fgSX}sfw)`hxopaqk.6Hg';w23\"sd+b07`LSOGqz#-)[");
    return base64encode(pbkdf2_sha256(hashHexEncoded, salt, 18, 32));
}

string_t
Crypto::calculateLoginPassword(const string_t& login, const string_t& password) {
    auto salt = login + U("\"D<?4'V%Fh(U,9SjdO4v)|1mJV31]#;W");
    auto hashed = pbkdf2_sha256(password, salt, 16);
    return base64encode(hashed);
}

string_t
Crypto::calculateMasterPassword(const string_t& salt, const string_t& password) {
    return base64encode(pbkdf2_sha256(password, salt, 16));
}

string_t
Crypto::sha1File (const string_t& filename)
{
    CryptoPP::SHA1 sha1;
    auto hash = string_t{};

    CryptoPP::FileSource(filename.c_str(), true,
        new CryptoPP::HashFilter(sha1,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(hash)
            )
        )
    );
    std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
    return hash;
}

std::tuple<string_t, string_t, string_t>
Crypto::aesEncrypt (const string_t& password, const string_t& data)
{

    SecByteBlock iv(16);
    SecByteBlock salt(8);

    OS_GenerateRandomBlock(true, salt, salt.size());
    OS_GenerateRandomBlock(false, iv, iv.size());

    auto saltStr = string_t(salt.begin(), salt.end());
    auto ivStr = string_t(iv.begin(), iv.end());
    auto key = pbkdf2_sha256(password, saltStr, 16);
    auto encrypted = aesEncrypt(key, ivStr, data);

    return std::make_tuple(
            encrypted,
            ivStr,
            saltStr
    );
}

string_t
Crypto::aesEncrypt (const string_t& key, const string_t& iv, const string_t& data)
{
    CBC_Mode<AES>::Encryption e;
    e.SetKeyWithIV(toByteCst(key), key.size(), toByteCst(iv), iv.size());

    string_t encrypted;
    StringSource ss(toByteCst (data), data.size(), true,
        new StreamTransformationFilter(e,
            new StringSink(encrypted)
        )
    );

    return encrypted;
}
string_t
Crypto::aesDecrypt (const string_t& password, const string_t& saltStr, const string_t& ivStr, const string_t& data)
{
    auto key = pbkdf2_sha256(password, saltStr, 16);

    CBC_Mode<AES>::Decryption e;
    e.SetKeyWithIV(toByteCst(key),
                   key.size(),
                   toByteCst(ivStr),
                   ivStr.size());

    string_t decrypted;
    StringSource ss(toByteCst (data), data.size(), true,
        new StreamTransformationFilter(e,
            new StringSink(decrypted)
        )
    );

    return decrypted;
}

} /* namespace giga */

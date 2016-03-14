/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <boost/asio/ssl.hpp>
#include <boost/chrono.hpp>

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
#include "cryptopp/hex.h"
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
#include "crypto++/hex.h"
#endif

#include "Crypto.h"
#include "../rest/HttpErrors.h"
#include "../utils/Utils.h"
#include "../utils/Utils.h"

#include <cpprest/details/basic_types.h>
#include <algorithm>
#include <openssl/sha.h>

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
using CryptoPP::Redirector;
using CryptoPP::HexEncoder;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_PKCS1v15_Decryptor;
using CryptoPP::RSAES_PKCS1v15_Encryptor;
using CryptoPP::SecByteBlock;
using giga::utils::wstr2str;
using giga::utils::str2wstr;
using utility::string_t;

namespace {

const byte*
toByteCst (const std::string& str)
{
    return reinterpret_cast<const byte*>(str.data());
}
byte*
toByte (std::vector<char>& str)
{
    return reinterpret_cast<byte*>(str.data());
}

void fillQueue(ByteQueue& queue, const std::string& b64encoded) {
    Base64Decoder decoder;
    decoder.Attach(new Redirector(queue));
    decoder.Put(toByteCst(b64encoded), b64encoded.size());
    decoder.MessageEnd();
}

}

namespace giga
{

class RsaKeys final
{
public:
    RsaKeys()               = default;
    RsaKeys(const RsaKeys&) = default;

    CryptoPP::RSA::PublicKey    pub;
    CryptoPP::RSA::PrivateKey   priv;
};
    
Rsa::Rsa (const std::string& pubStr, const std::string& privStr) :
    _keys{ new RsaKeys{} }
{
    try
    {
        ByteQueue queue;
        fillQueue(queue, pubStr);
        _keys->pub.BERDecodePublicKey(queue, false, static_cast<size_t>(queue.MaxRetrievable()));
    }
    catch (const std::exception&)
    {
        ByteQueue queue;
        fillQueue(queue, pubStr);
        _keys->pub.BERDecode(queue);
    }
    _hasPrivateKey = false;
    if (privStr != "")
    {
        try
        {
            ByteQueue queue;
            fillQueue(queue, privStr);
            _keys->priv.BERDecodePrivateKey(queue, false, static_cast<size_t>(queue.MaxRetrievable()));
            _hasPrivateKey = true;
        }
        catch (const std::exception&)
        {
            ByteQueue queue;
            fillQueue(queue, privStr);
            _keys->priv.BERDecode(queue);
            _hasPrivateKey = true;
        }

        AutoSeededRandomPool rnd;
        if (!_keys->priv.Validate(rnd, 3))
        {
            BOOST_THROW_EXCEPTION(ErrorException(U("Rsa private key validation failed")));
        }

        if (!_keys->priv.Validate(rnd, 3))
        {
            BOOST_THROW_EXCEPTION(ErrorException(U("Dsa private key validation failed")));
        }
    }
}

Rsa::Rsa () :
        _keys{nullptr}, _hasPrivateKey{false}
{
}

Rsa::~Rsa() = default;

Rsa& 
Rsa::operator=(const Rsa& rhs)
{
    this->_keys = std::unique_ptr<RsaKeys>(new RsaKeys{ rhs._keys->pub, rhs._keys->priv });
    this->_hasPrivateKey = rhs._hasPrivateKey;
    return *this;
}

Rsa::Rsa(const Rsa& rhs)
{
    this->_keys = std::unique_ptr<RsaKeys>(new RsaKeys{ rhs._keys->pub, rhs._keys->priv });
    this->_hasPrivateKey = rhs._hasPrivateKey;
}

Rsa::Rsa(Rsa&&)             = default;
Rsa& Rsa::operator=(Rsa&& ) = default;

std::string
Rsa::encrypt (const std::string& data) const
{
    if (_keys == nullptr) {
        BOOST_THROW_EXCEPTION(ErrorException(U("You cannot use a default constructed Rsa obj")));
    }
    AutoSeededRandomPool rng;
    RSAES_PKCS1v15_Encryptor encryptor(_keys->pub);
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
    if (_keys == nullptr) {
        BOOST_THROW_EXCEPTION(ErrorException(U("You cannot use a default constructed Rsa obj")));
    }
    if (!_hasPrivateKey) {
        BOOST_THROW_EXCEPTION(ErrorException(U("PrivateKey has not been set")));
    }
    AutoSeededRandomPool rng;
    RSAES_PKCS1v15_Decryptor decryptor(_keys->priv);
    std::string decrypted;
    StringSource ss(data, true,
        new PK_DecryptorFilter(rng, decryptor,
            new StringSink(decrypted)
        )
    );
    return decrypted;
}

std::string
Rsa::decryptNodeKey (const std::string& data) const
{
    auto tmp = decrypt(Crypto::base64decode(data));
    if (tmp.size() == 32)
    {
        return Crypto::base64encode(tmp);
    }
    else if (tmp.size() > 44)
    {
        return Crypto::base64decode(tmp);
    }
    else
    {
        return tmp;
    }
}


template<typename T>
std::string
pbkdf2 (const string_t& spassword, const std::string& salt, std::size_t length, uint64_t iteration)
{
    auto password = wstr2str(spassword);
    auto key = std::vector<char>(length);
    CryptoPP::PKCS5_PBKDF2_HMAC<T> passToKey;
    passToKey.DeriveKey(toByte(key),
                        key.size(),
                        '\0',
                        toByteCst(password),
                        password.size(),
                        toByteCst(salt),
                        salt.size(),
                        static_cast<unsigned int>(iteration));

    return std::string{key.begin(), key.end()};
}

std::string
Crypto::pbkdf2_sha256 (const string_t& password, const std::string& salt, std::size_t length, uint64_t iteration)
{
    return pbkdf2<CryptoPP::SHA256> (password, salt, length, iteration);
}
std::string
Crypto::pbkdf2_sha512 (const string_t& password, const std::string& salt, std::size_t length, uint64_t iteration)
{
    return pbkdf2<CryptoPP::SHA512> (password, salt, length, iteration);
}

std::string
Crypto::base64encode (const std::string& data)
{
    std::string encoded;
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

std::string
Crypto::base64decode (const std::string& data)
{
    std::string decoded;
    StringSource ss(toByteCst(data), data.size(), true,
        new Base64Decoder(
            new StringSink(decoded)
        ) // Base64Encoder
    ); // StringSource
    return decoded;
}

std::string
Crypto::calculateFid(const std::string& hashHexEncoded) {
    auto salt = "5%;[yw\"XG2&Om#i*T$v.B2'Ae/VST4t#u$@pxsauO,H){`hUd7Xu@4q4WCc<>'ie";
    return base64encode(pbkdf2_sha256(str2wstr(hashHexEncoded), salt, 18, 32ul));
}

std::string
Crypto::calculateFkey(const std::string& hashHexEncoded) {
    auto salt = "={w|>6L:{Xn;HAKf^w=,fgSX}sfw)`hxopaqk.6Hg';w23\"sd+b07`LSOGqz#-)[";
    return base64encode(pbkdf2_sha256(str2wstr(hashHexEncoded), salt, 18, 32ul));
}

std::string
Crypto::calculateLoginPassword(const string_t& login, const string_t& password) {
    auto salt = wstr2str(login) + "\"D<?4'V%Fh(U,9SjdO4v)|1mJV31]#;W";
    auto hashed = pbkdf2_sha256(password, salt, 16);
    return base64encode(hashed);
}

std::string
Crypto::calculateMasterPassword(const std::string& salt, const string_t& password) {
    return base64encode(pbkdf2_sha256(password, salt, 16));
}

static constexpr uint32_t BUF_SIZE = 8192;

std::string
Crypto::sha1File (const string_t& sfilename)
{
	std::ifstream is (sfilename.c_str(), std::ifstream::binary);
	if (!is) {
		BOOST_THROW_EXCEPTION(ErrorException{"Cannot open file"});
	}
	auto buffer = std::unique_ptr<char[]>(new char[BUF_SIZE]);

	SHA_CTX ctx;
	SHA1_Init(&ctx);
	do {
		is.read (buffer.get(), BUF_SIZE);
		auto read = is.gcount();
		if (read > 0)
		{
			SHA1_Update(&ctx, buffer.get(), read);
		}

	} while ((is.rdstate() & std::ifstream::eofbit) == 0);
	is.close();

	unsigned char hashBuf[SHA_DIGEST_LENGTH];
	SHA1_Final(hashBuf, &ctx);

    std::string hash;
    StringSource ss(hashBuf, SHA_DIGEST_LENGTH, true,
        new HexEncoder(
            new StringSink(hash)
        ) // HexEncoder
    ); // StringSource

	std::locale l{"C"};
	std::transform(hash.begin(), hash.end(), hash.begin(), [&l](char c) {
		return std::tolower(c, l);
	});

    return hash;
}

std::tuple<std::string, std::string, std::string>
Crypto::aesEncrypt (const string_t& password, const std::string& data)
{
    SecByteBlock iv(16);
    SecByteBlock salt(8);

    OS_GenerateRandomBlock(true, salt, salt.size());
    OS_GenerateRandomBlock(false, iv, iv.size());

    auto saltStr = std::string(salt.begin(), salt.end());
    auto ivStr = std::string(iv.begin(), iv.end());
    auto key = pbkdf2_sha256(password, saltStr, 16);
    auto encrypted = aesEncrypt(key, ivStr, data);

    return std::make_tuple(
            encrypted,
            ivStr,
            saltStr
    );
}

std::string
Crypto::aesEncrypt (const std::string& key, const std::string& iv, const std::string& data)
{
    CBC_Mode<AES>::Encryption e;
    e.SetKeyWithIV(toByteCst(key), key.size(), toByteCst(iv), iv.size());

    std::string encrypted;
    StringSource ss(toByteCst (data), data.size(), true,
        new StreamTransformationFilter(e,
            new StringSink(encrypted)
        )
    );

    return encrypted;
}

std::string
Crypto::aesDecrypt (const string_t& spassword, const std::string& saltStr, const std::string& ivStr, const std::string& data)
{
    auto key   = pbkdf2_sha256(spassword, saltStr, 16);

    CBC_Mode<AES>::Decryption e;
    e.SetKeyWithIV(toByteCst(key),
                   key.size(),
                   toByteCst(ivStr),
                   ivStr.size());

    std::string decrypted;
    StringSource ss(toByteCst (data), data.size(), true,
        new StreamTransformationFilter(e,
            new StringSink(decrypted)
        )
    );

    return decrypted;
}

} /* namespace giga */

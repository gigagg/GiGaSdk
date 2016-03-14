/*
 * Sha1Calculator.cpp
 *
 *  Created on: 14 mars 2016
 *      Author: thomas
 */

#include "Sha1Calculator.h"

#include "details/CurlProgress.h"
#include "../rest/HttpErrors.h"
#include "../utils/Utils.h"

#include "crypto++/filters.h"
#include "crypto++/hex.h"

#include <boost/filesystem.hpp>
#include <boost/throw_exception.hpp>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <chrono>
#include <memory>
#include <thread>

using CryptoPP::HexEncoder;
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using std::chrono::milliseconds;

namespace giga
{
namespace core
{

static constexpr uint32_t BUF_SIZE = 8192;

Sha1Calculator::Sha1Calculator(const utility::string_t& filename):
        FileTransferer{},
        _filename{utils::wstr2str(filename)},
        _task{},
        _is{filename.c_str(), std::ifstream::binary},
        _buf{std::unique_ptr<char[]>(new char[BUF_SIZE])},
        _fileSize{boost::filesystem::file_size(filename)}
{
    if (!_is)
    {
        BOOST_THROW_EXCEPTION(ErrorException{"Cannot open file"});
    }
}

Sha1Calculator::~Sha1Calculator()
{
    if (_is)
    {
        _is.close();
    }
}

const pplx::task<std::string>&
Sha1Calculator::task () const
{
    return _task;
}

FileTransferer::Progress
Sha1Calculator::progress () const
{
    auto p = _progress->data();
    if (_state != State::pending && _task.is_done())
    {
        return Progress{_fileSize, _fileSize};
    }
    return Progress{p.ulnow, _fileSize};
}

const utility::string_t&
Sha1Calculator::fileName() const
{
    return _filename;
}

void
Sha1Calculator::doStart ()
{
    _task = pplx::create_task([this] {
        SHA_CTX ctx;
        SHA1_Init(&ctx);
        uint64_t processed = 0;
        uint64_t count = 0;
        do {
            _is.read (_buf.get(), BUF_SIZE);
            if ((_is.rdstate() & std::ifstream::eofbit) == 0 &&
                (_is.rdstate() & std::ifstream::failbit) != 0)
            {
                BOOST_THROW_EXCEPTION(ErrorException{"Error reading file"});
            }

            auto read = _is.gcount();
            if (read > 0)
            {
                SHA1_Update(&ctx, _buf.get(), read);
                processed += read;
                count += 1;
            }

            if ((count % 8192) == 0)
            {
                auto result = _progress->onCallback(0, 0, _fileSize, processed);
                if (result == CURLE_ABORTED_BY_CALLBACK)
                {
                    BOOST_THROW_EXCEPTION(ErrorException{"Calculation canceled"});
                }
                while (_progress->isPaused())
                {
                    std::this_thread::sleep_for(milliseconds(500));
                }
            }

        } while ((_is.rdstate() & std::ifstream::eofbit) == 0);


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
    });
}

} /* namespace core */
} /* namespace giga */

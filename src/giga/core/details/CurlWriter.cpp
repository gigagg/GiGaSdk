/*
 * CurlWriter.cpp
 *
 *  Created on: 18 févr. 2016
 *      Author: thomas
 */

#include "CurlWriter.h"

#include <curl_easy.h>
#include <ios>

namespace giga
{
namespace details
{

CurlWriter::CurlWriter (const boost::filesystem::path& path) :
        _file{}, _stream{}, _curl{nullptr}, _httpCode{0}
{
    _file.open(path.string(), std::ios::binary | std::ios::app);
}

CurlWriter::~CurlWriter ()
{
    if (_file.is_open())
    {
        _file.close();
    }
}

int
CurlWriter::write (const char * contents, size_t size) noexcept
{
    try {
        if (_curl == nullptr)
        {
            throw "error";
        }
        if (_httpCode == 0)
        {
            auto code = curl_easy_getinfo (_curl->get_curl(), CURLINFO_RESPONSE_CODE, &_httpCode);
            if (code != CURLE_OK)
            {
                throw code;
            }
        }
        if (_httpCode >= 300)
        {
            _stream.write(contents, size);
        } else {
            _file.write(contents, size);
        }
        return size;
    } catch (...) {
        return -1;
    }
}

std::string
CurlWriter::getErrorData () const
{
    return _stream.str();
}

void
CurlWriter::setCurl (curl::curl_easy& curl)
{
    _curl = &curl;
}
std::ofstream&
CurlWriter::file()
{
    return _file;
}


} /* namespace details */
} /* namespace giga */

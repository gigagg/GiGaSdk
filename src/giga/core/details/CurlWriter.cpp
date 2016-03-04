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

#include "CurlWriter.h"
#include "../../utils/Utils.h"

#include <curl_easy.h>
#include <ios>

using utility::string_t;
using giga::utils::str2wstr;

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

size_t
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
        }
        else
        {
            _file.write(contents, size);
        }
        return size;
    }
    catch (...)
    {
        return -1;
    }
}

string_t
CurlWriter::getErrorData () const
{
    return str2wstr(_stream.str());
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

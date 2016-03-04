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

#ifndef GIGA_CORE_DETAILS_CURLWRITER_H_
#define GIGA_CORE_DETAILS_CURLWRITER_H_

#include <boost/filesystem.hpp>
#include <cpprest/details/basic_types.h>
#include <fstream>
#include <sstream>

namespace curl
{
class curl_easy;
}

namespace giga
{
namespace details
{

class CurlWriter
{
public:
    explicit CurlWriter(const boost::filesystem::path& path);
    ~CurlWriter();
    CurlWriter()                             = delete;
    CurlWriter(const CurlWriter&)            = delete;
    CurlWriter(CurlWriter&&)                 = delete;
    CurlWriter& operator=(const CurlWriter&) = delete;
    CurlWriter& operator=(CurlWriter&&)      = delete;

    size_t
    write (const char* contents, size_t size) noexcept;

    utility::string_t
    getErrorData () const;

    void
    setCurl (curl::curl_easy& curl);

    std::ofstream&
    file();

private:
    std::ofstream       _file;
    std::ostringstream  _stream;
    curl::curl_easy*    _curl;
    long                _httpCode;

};

} /* namespace details */
} /* namespace giga */

#endif /* GIGA_CORE_DETAILS_CURLWRITER_H_ */

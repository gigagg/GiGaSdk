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

#include "Utils.h"
#include "utfcpp/utf8/checked.h"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem.hpp>
#include <cpprest/details/basic_types.h>
#include <cpprest/asyncrt_utils.h>

using utility::string_t;

namespace
{
#ifdef _UTF16_STRINGS
const wchar_t invalidChars[] =
            L"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
            "<>:\"/\\|";
#else
const char invalidChars[] =
            "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
            "<>:\"/\\|";
#endif
}

namespace giga
{
namespace utils
{

string_t
httpsPrefix(const string_t& url)
{
    if (url.length() > 2 && url[0] == '/' && url[1] == '/')
    {
        return U("https:") + url;
    }
    return url;
}

string_t
cleanUpFilename(string_t name)
{
    auto pos = name.find_first_of(invalidChars);
    while (pos != string_t::npos)
    {
        name.replace(pos, 1, U("_"));
        pos = name.find_first_of(invalidChars, pos);
    }
    return name;
}

std::string
wstr2str(const utility::string_t& wstr)
{
    return utility::conversions::to_utf8string(wstr);
}

utility::string_t
str2wstr(const std::string& str)
{
#ifdef _UTF16_STRINGS
    try
    {
        return utility::conversions::to_utf16string(str);
    }
    catch (...)
    {
        try
        {
            return utility::conversions::latin1_to_utf16(str);
        }
        catch (...)
        {
            return utility::conversions::to_utf16string(replaceInvalidUtf8(str));
        }
    }
#else
    return std::string{str};
#endif
}

std::string
replaceInvalidUtf8(const std::string& str)
{
    std::string tmp{};
    tmp.reserve(str.size());
    utf8::replace_invalid(str.begin(), str.end(), std::back_inserter(tmp));
    return tmp;
}

std::wstring
replaceInvalidUtf8(const std::wstring& str)
{
    return str;
}

std::string
exceptionInfos() noexcept
{
    auto info = boost::current_exception_diagnostic_information(true);
    std::replace(info.begin(), info.end(), '\n', '\t');
    return info;
}

utility::string_t
exceptionPrettyInfos() noexcept
{
    std::string error = "";
    try
    {
        throw;
    }
    catch (const boost::filesystem::filesystem_error& e)
    {
        switch (e.code().value()) {
        case boost::system::errc::device_or_resource_busy:
            error = "device or resource busy";
            break;
        case boost::system::errc::file_exists:
            error = "file exists";
            break;
        case boost::system::errc::file_too_large:
            error = "file too large";
            break;
        case boost::system::errc::filename_too_long:
            error = "filename too long";
            break;
        case boost::system::errc::no_space_on_device:
            error = "no space on device";
            break;
        case boost::system::errc::no_such_device_or_address:
            error = "no such device or address";
            break;
        case boost::system::errc::no_such_file_or_directory:
            error = "no such file or directory";
            break;
        case boost::system::errc::not_enough_memory:
            error = "not enough memory";
            break;
        case boost::system::errc::permission_denied:
            error = "permission denied";
            break;
        case boost::system::errc::read_only_file_system:
            error = "read only file system";
            break;
        case boost::system::errc::io_error:
            error = "input/output error";
            break;
        default:
            error = "unknown file system error";
            break;
        }
    }
    catch (const std::exception& e)
    {
        error = e.what();
    }
    catch (...)
    {
        error = giga::utils::exceptionInfos();
    }

    try
    {
        return giga::utils::str2wstr(error);
    }
    catch (...)
    {
        return U("Unknown error");
    }
}

} /* namespace utils */
} /* namespace giga */

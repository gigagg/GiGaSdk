/*
 * Utils.cpp
 *
 *  Created on: 11 févr. 2016
 *      Author: thomas
 */

#include "Utils.h"
#include <cpprest/details/basic_types.h>

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
#ifdef _UTF16_STRINGS
    typedef std::codecvt_utf8_utf16<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
#else
    return wstr;
#endif
}

utility::string_t
str2wstr(const std::string& str)
{
#ifdef _UTF16_STRINGS
    typedef std::codecvt_utf8_utf16<wchar_t> convert_typeX;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(str);
#else
    return str;
#endif
}

utility::string_t
str2wstr(const std::wstring& wstr)
{
#ifdef _UTF16_STRINGS
	return wstr;
#else
	typedef std::codecvt_utf8<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	return converterX.to_bytes(wstr);
#endif
}

} /* namespace utils */
} /* namespace giga */

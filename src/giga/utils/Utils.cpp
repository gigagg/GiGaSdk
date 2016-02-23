/*
 * Utils.cpp
 *
 *  Created on: 11 févr. 2016
 *      Author: thomas
 */

#include "Utils.h"

namespace
{
const char invalidChars[] =
            "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F"
            "<>:\"/\\|";
}

namespace giga
{
namespace utils
{

std::string
httpsPrefix(const std::string& url)
{
    if (url.length() > 2 && url[0] == '/' && url[1] == '/')
    {
        return "https:" + url;
    }
    return url;
}

std::string
cleanUpFilename(std::string name)
{
    auto pos = name.find_first_of(invalidChars);
    while (pos != std::string::npos)
    {
        name.replace(pos, 1, "_");
        pos = name.find_first_of(invalidChars, pos);
    }
    return name;
}


} /* namespace utils */
} /* namespace giga */

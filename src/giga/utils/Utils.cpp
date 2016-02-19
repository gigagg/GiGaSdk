/*
 * Utils.cpp
 *
 *  Created on: 11 févr. 2016
 *      Author: thomas
 */

#include "Utils.h"

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


} /* namespace utils */
} /* namespace giga */

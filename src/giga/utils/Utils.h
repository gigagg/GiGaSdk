/*
 * Utils.h
 *
 *  Created on: 11 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_UTILS_UTILS_H_
#define GIGA_UTILS_UTILS_H_

#include <pplx/pplxtasks.h>
#include <cpprest/details/basic_types.h>

namespace giga
{
namespace utils
{

utility::string_t
httpsPrefix(const utility::string_t& url);

utility::string_t
cleanUpFilename(utility::string_t name);

template<typename T>
utility::string_t
to_string(T&& str)
{
#ifdef _UTF16_STRINGS
    return std::to_wstring(str);
#else
    return std::to_string(str);
#endif
}

} /* namespace utils */
} /* namespace giga */

#endif /* GIGA_UTILS_UTILS_H_ */

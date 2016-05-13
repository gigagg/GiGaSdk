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

#ifndef GIGA_UTILS_UTILS_H_
#define GIGA_UTILS_UTILS_H_

#include <pplx/pplxtasks.h>
#include <cpprest/details/basic_types.h>
#ifdef _UTF16_STRINGS
#include <codecvt>
#endif

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
    return std::to_wstring(std::forward<T>(str));
#else
    return std::to_string(std::forward<T>(str));
#endif
}

std::string
wstr2str(const utility::string_t& wstr);

utility::string_t
str2wstr(const std::string& str);

std::string
replaceInvalidUtf8(const std::string& str);

std::wstring
replaceInvalidUtf8(const std::wstring& str);

std::string
exceptionInfos() noexcept;

utility::string_t
exceptionPrettyInfos() noexcept;


} /* namespace utils */
} /* namespace giga */

#endif /* GIGA_UTILS_UTILS_H_ */

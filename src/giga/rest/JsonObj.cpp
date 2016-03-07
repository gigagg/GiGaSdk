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

#include "JsonObj.h"
#include "../utils/Utils.h"

using utility::string_t;

namespace giga
{

JsonObj& JsonObj::add(const string_t& name, int64_t value) {
    intData.push_back(std::make_pair(name, value));
    return *this;
}
#ifdef _UTF16_STRINGS
JsonObj& JsonObj::add(const string_t& name, const std::wstring& value) {
    strData.push_back(std::make_pair(name, value));
    return *this;
}
#endif
JsonObj& JsonObj::add(const string_t& name, const std::string& value) {
    strData.push_back(std::make_pair(name, utils::str2wstr(value)));
    return *this;
}
JsonObj& JsonObj::add(const string_t& name, bool value) {
    boolData.push_back(std::make_pair(name, value));
    return *this;
}
JsonObj& JsonObj::add(const string_t& name, double value) {
    doubleData.push_back(std::make_pair(name, value));
    return *this;
}

} /* namespace giga */

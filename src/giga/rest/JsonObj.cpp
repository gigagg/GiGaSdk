/*
 * JsonObj.cpp
 *
 *  Created on: 5 janv. 2016
 *      Author: thomas
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
    strData.push_back(std::make_pair(name, utils::str2wstr(value)));
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

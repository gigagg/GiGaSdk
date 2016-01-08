/*
 * JsonObj.cpp
 *
 *  Created on: 5 janv. 2016
 *      Author: thomas
 */

#include "JsonObj.h"

namespace giga
{

JsonObj& JsonObj::add(const std::string& name, int64_t value) {
    intData.push_back(std::make_pair(name, value));
    return *this;
}
JsonObj& JsonObj::add(const std::string& name, const std::string& value) {
    strData.push_back(std::make_pair(name, value));
    return *this;
}
JsonObj& JsonObj::add(const std::string& name, bool value) {
    boolData.push_back(std::make_pair(name, value));
    return *this;
}
JsonObj& JsonObj::add(const std::string& name, double value) {
    doubleData.push_back(std::make_pair(name, value));
    return *this;
}

} /* namespace giga */

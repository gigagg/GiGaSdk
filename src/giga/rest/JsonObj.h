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

#ifndef MODEL_JSONOBJ_H_
#define MODEL_JSONOBJ_H_

#include <cpprest/details/basic_types.h>
#include <utility>
#include <vector>

namespace giga
{

class JsonObj
{
public:
    JsonObj& add(const utility::string_t& name, int64_t value);
    JsonObj& add(const utility::string_t& name, const std::string& value);
#ifdef _UTF16_STRINGS
    JsonObj& add(const utility::string_t& name, const std::wstring& value);
#endif
    JsonObj& add(const utility::string_t& name, bool value);
    JsonObj& add(const utility::string_t& name, double value);
    JsonObj& add(const utility::string_t& name, const char* value) = delete;

    template <class Unserializer>
        void visit(const Unserializer& s){
            for(auto p : intData) {
                s.manage(p.second, p.first);
            }
            for(auto p : strData) {
                s.manage(p.second, p.first);
            }
            for(auto p : boolData) {
                s.manage(p.second, p.first);
            }
            for(auto p : doubleData) {
                s.manage(p.second, p.first);
            }
        }

private:
    std::vector<std::pair<utility::string_t, int64_t>> intData;
    std::vector<std::pair<utility::string_t, utility::string_t>> strData;
    std::vector<std::pair<utility::string_t, bool>> boolData;
    std::vector<std::pair<utility::string_t, double>> doubleData;
};

} /* namespace giga */

#endif /* MODEL_JSONOBJ_H_ */

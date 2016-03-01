/*
 * JsonObj.h
 *
 *  Created on: 5 janv. 2016
 *      Author: thomas
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
    JsonObj& add(const utility::string_t& name, const utility::string_t& value);
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

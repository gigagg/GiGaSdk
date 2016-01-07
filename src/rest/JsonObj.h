/*
 * JsonObj.h
 *
 *  Created on: 5 janv. 2016
 *      Author: thomas
 */

#ifndef MODEL_JSONOBJ_H_
#define MODEL_JSONOBJ_H_

#include <string>
#include <utility>
#include <vector>

namespace giga
{

class JsonObj
{
public:
    JsonObj& add(const std::string& name, int64_t value);
    JsonObj& add(const std::string& name, const std::string& value);
    JsonObj& add(const std::string& name, bool value);
    JsonObj& add(const std::string& name, double value);
    JsonObj& add(const std::string& name, const char* value) = delete;

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
    std::vector<std::pair<std::string, int64_t>> intData;
    std::vector<std::pair<std::string, std::string>> strData;
    std::vector<std::pair<std::string, bool>> boolData;
    std::vector<std::pair<std::string, double>> doubleData;
};

} /* namespace giga */

#endif /* MODEL_JSONOBJ_H_ */

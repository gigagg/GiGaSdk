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

#ifndef JSONSERIALIZER_H_
#define JSONSERIALIZER_H_

#include <cpprest/json.h>
#include <boost/optional.hpp>
#include <map>
#include "../utils/Utils.h"

namespace giga {

class JSonSerializer;

namespace details {
    inline web::json::value serialize(int& value) {
        return web::json::value::number(value);
    }
    inline web::json::value serialize(int64_t& value) {
        return web::json::value::number(value);
    }
    inline web::json::value serialize(bool& value) {
        return web::json::value::boolean(value);
    }
    inline web::json::value serialize(double& value) {
        return web::json::value::number(value);
    }
    inline web::json::value serialize(std::string& value) {
        return  web::json::value::string(utils::str2wstr(value));
    }
#ifdef _UTF16_STRINGS
    inline web::json::value serialize(std::wstring& value) {
        return  web::json::value::string(value);
    }
    inline web::json::value serialize(const wchar_t*& value) {
        return web::json::value::string(value);
    }
#endif

    template <typename T> web::json::value serialize(T& value) {
        auto subJson = web::json::value::object();
        value.visit(JSonSerializer{subJson});
        return subJson;
    }
    template <typename T> web::json::value serialize(std::unique_ptr<T>& value) {
        if (value) {
            return serialize(*value);
        } else {
            return web::json::value::null();
        }
    }
    template <typename T> web::json::value serialize(std::shared_ptr<T>& value) {
        if (value) {
            return serialize(*value);
        } else {
            return web::json::value::null();
        }
    }
    template <typename T> web::json::value serialize(std::vector<T>& values) {
        auto subJson = web::json::value::array(values.size());
        int i = 0;
        for(auto& value : values) {
            subJson[i++] = serialize(value);
        }
        return subJson;
    }
    template <typename T> web::json::value serialize(boost::optional<T>& value) {
        if (value) {
            return serialize(value.get());
        } else {
            return web::json::value::null();
        }
    }
} // namespace details

class JSonSerializer final
{
public:

    JSonSerializer (web::json::value& val) :
            val(val)
    {
    }

    template <typename T> void serialize(T visitable) const {
        visitable.visit(*this);
    }
    template <typename T> void manageOpt(T& current, const utility::string_t& name, T) const {
        manage(current, name);
    }
    template <typename T> void manage(T& current, utility::string_t name) const {
        val[name] = details::serialize(current);
    }

    template <typename T>
    static utility::string_t toString(T&& visitable) {
        return toJson(std::forward<T>(visitable)).serialize();
    }

    template <typename T>
    static web::json::value toJson(T&& visitable) {
        auto json = web::json::value::object();
        visit(visitable, json);
        return json;
    }

private:
    template <typename T>
    static void visit(std::shared_ptr<T> visitable, web::json::value& obj) {
        visitable->visit(giga::JSonSerializer{obj});
    }
    template <typename T>
    static void visit(T&& visitable, web::json::value& obj) {
        visitable.visit(giga::JSonSerializer{obj});
    }
    template <typename T>
    static void visit(T* visitable, web::json::value& obj) {
        visitable->visit(giga::JSonSerializer{obj});
    }

private:
    web::json::value& val;
};

} // namespace giga

#endif /* JSONSERIALIZER_H_ */

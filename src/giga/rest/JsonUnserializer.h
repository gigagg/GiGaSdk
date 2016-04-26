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

#ifndef JSONUNSERIALIZER_H_
#define JSONUNSERIALIZER_H_

#include <cpprest/json.h>
#include <boost/optional.hpp>
#include "../utils/Utils.h"

namespace giga {

class JSonUnserializer;

namespace details {
    inline void getValue(const web::json::value& value, int& ret) {
        ret = value.as_integer();
    }
    inline void getValue(const web::json::value& value, int64_t& ret) {
        ret = value.as_number().to_int64();
    }
    inline void getValue(const web::json::value& value, uint64_t& ret) {
        ret = value.as_number().to_uint64();
    }
    inline void getValue(const web::json::value& value, bool& ret) {
        ret = value.as_bool();
    }
    inline void getValue(const web::json::value& value, double& ret) {
        ret = value.as_double();
    }
    inline void getValue(const web::json::value& value, std::string& ret) {
        ret = utils::wstr2str(value.as_string());
    }
#ifdef _UTF16_STRINGS
    inline void getValue(const web::json::value& value, std::wstring& ret) {
        ret = value.as_string();
    }
#endif

    template <typename T>
    void getValue(const web::json::value& value, T& ret) {
        ret.visit(JSonUnserializer{value});
    }

    template <typename T>
    void getValue(const web::json::value& value, std::unique_ptr<T>& ret) {
        if (value.is_null()) {
            ret = nullptr;
        } else if (ret == nullptr) {
            ret = std::unique_ptr<T>(new T{});
            getValue(value, *ret);
        } else {
            getValue(value, *ret);
        }
    }

    template <typename T>
    void getValue(const web::json::value& value, std::shared_ptr<T>& ret) {
        if (value.is_null()) {
            ret = nullptr;
        } else if (ret == nullptr) {
            ret = std::make_shared<T>();
            getValue(value, *ret);
        } else {
            getValue(value, *ret);
        }
    }

    template <typename T>
    void getValue(const web::json::value& value, std::vector<T>& ret) {
        ret.clear();
        if (!value.is_null()) {
            auto values = value.as_array();
            ret.reserve(values.size());
            for(auto v : values) {
                auto t = T{};
                getValue(v, t);
                ret.push_back(std::move(t));
            }
        }
    }

    template <typename T>
    void getValue(const web::json::value& value, boost::optional<T>& ret) {
        if (value.is_null()) {
            ret = boost::none;
        } else if (!ret.is_initialized()) {
            ret = boost::make_optional(T{});
            getValue(value, ret.get());
        } else {
            getValue(value, ret.get());
        }
    }
} // namespace details

class JSonUnserializer final
{
public:

    JSonUnserializer (const web::json::value& val) :
            val (val)
    {
    }

    template <typename T>
    static T fromString(const utility::string_t& json) {
        auto val = web::json::value::parse(json);
        auto unserializer = JSonUnserializer{val};
        return unserializer.unserialize<T>();
    }

    template <typename T> T unserialize() const {
        auto t = T{};
        unserialize(t);
        return std::move(t);
    }

    template <typename T> T& unserialize(T& val) const {
        doUnserialize(val);
        return val;
    }

private:
    //
    // doUnserialize
    //

//    inline void doUnserialize(utility::string_t& data) const {
//        data = val.as_string();
//    }
    template <typename T> void doUnserialize(T& data) const {
        data.visit(*this);
    }
    template <typename T> void doUnserialize(std::unique_ptr<T>& data) const {
        if (!val.is_null()) {
            data = std::unique_ptr<T>(new T{});
            doUnserialize(*data);
        }
    }
    template <typename T> void doUnserialize(std::shared_ptr<T>& data) const {
        if (!val.is_null()) {
            data = std::make_shared<T>();
            doUnserialize(*data);
        }
    }
    template <typename T> void doUnserialize(boost::optional<T>& data) const {
        if (!val.is_null()) {
            data = boost::make_optional(T{});
            doUnserialize(data.get());
        }
    }
    template <typename T> void doUnserialize(std::vector<T>& data) const {
        if (val.is_array()) {
            auto& values = val.as_array();
            data.reserve(values.size());
            for(auto value : values) {
                auto t = T{};
                details::getValue(value, t),
                data.push_back(std::move(t));
            }
        }
    }

public:
    //
    // MANAGE
    //

    template <typename T> void manageOpt(T& current, const utility::string_t& name, T defaultValue) const {
        if (val.has_field(name)) {
            manage(current, name);
        } else {
            current = defaultValue;
        }
    }
    void manage(bool& current, utility::string_t name) const {
        if (!val.has_field(name)) {
            current = false;
        } else {
            details::getValue(val.at(name), current);
        }
    }
    template <typename T> void manage(T& current, utility::string_t name) const {
        details::getValue(val.at(name), current);
    }
    template <typename T> void manage(T* current, const utility::string_t& name) const {
        if (!val.has_field(name)) {
            current = nullptr;
        } else {
            details::getValue(val.at(name), current);
        }
    }
    template <typename T> void manage(std::unique_ptr<T>& current, const utility::string_t& name) const {
        if (!val.has_field(name)) {
            current = nullptr;
        } else {
            details::getValue(val.at(name), current);
        }
    }
    template <typename T> void manage(std::shared_ptr<T>& current, const utility::string_t& name) const {
        if (!val.has_field(name)) {
            current = nullptr;
        } else {
            details::getValue(val.at(name), current);
        }
    }
    template <typename T> void manage(boost::optional<T>& current, const utility::string_t& name) const {
        if (!val.has_field(name)) {
            current = boost::none;
        } else {
            details::getValue(val.at(name), current);
        }
    }
    template <typename T> void manage(std::vector<T>& current, const utility::string_t& name) const {
        if (!val.has_field(name)) {
            current = std::vector<T>{};
        } else {
            details::getValue(val.at(name), current);
        }
    }

private:
    const web::json::value& val;

};

} // namespace giga
#endif /* JSONUNSERIALIZER_H_ */

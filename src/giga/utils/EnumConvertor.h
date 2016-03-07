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

#ifndef GIGA_UTILS_ENUM_H_
#define GIGA_UTILS_ENUM_H_

#include "../rest/HttpErrors.h"
#include <array>

namespace giga
{
namespace utils
{

template<typename enumT, size_t enumSize>
class EnumConvertor
{
public:
    template<typename ...E>
    EnumConvertor(E&&... e) : _arr{{std::forward<E>(e)...}} {
    }

    enumT
    fromStr (const utility::string_t& value) const
    {
        auto it = std::find(_arr.begin(), _arr.end(), value);
        if (it != _arr.end())
        {
            size_t index = std::distance(_arr.begin(), it);
            return static_cast<enumT>(index);
        }
        BOOST_THROW_EXCEPTION(ErrorException{U("Value not found")});
    }

    const utility::string_t&
    toStr (enumT value) const
    {
        auto index = static_cast<size_t>(value);
        if (index < _arr.size())
        {
            return _arr[index];
        }
        BOOST_THROW_EXCEPTION(ErrorException{U("Str not found")});
    }

private:
    const std::array<utility::string_t, enumSize> _arr;
};

}
}

#endif /* GIGA_UTILS_ENUM_H_ */

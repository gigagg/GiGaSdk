/*
 * Enum.h
 *
 *  Created on: 19 janv. 2016
 *      Author: thomas
 */

#ifndef GIGA_UTILS_ENUM_H_
#define GIGA_UTILS_ENUM_H_

#include "../rest/HttpErrors.h"

namespace giga
{
namespace utils
{

template<typename enumT, size_t enumSize>
class EnumConvertor
{
public:
    template<typename ...E>
    EnumConvertor(E&&... e) : arr{{std::forward<E>(e)...}} {
    }

    enumT
    fromStr (const std::string& value) const
    {
        auto it = std::find(arr.begin(), arr.end(), value);
        if (it != arr.end())
        {
            size_t index = std::distance(arr.begin(), it);
            return static_cast<enumT>(index);
        }
        THROW(ErrorException{"Value not found"});
    }

    const std::string&
    toStr (enumT value) const
    {
        auto index = static_cast<size_t>(value);
        if (index < arr.size())
        {
            return arr[index];
        }
        THROW(ErrorException{"Str not found"});
    }

private:
    const std::array<std::string, enumSize> arr;
};

}
}

#endif /* GIGA_UTILS_ENUM_H_ */

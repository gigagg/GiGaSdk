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
        BOOST_THROW_EXCEPTION(ErrorException{"Value not found"});
    }

    const utility::string_t&
    toStr (enumT value) const
    {
        auto index = static_cast<size_t>(value);
        if (index < _arr.size())
        {
            return _arr[index];
        }
        BOOST_THROW_EXCEPTION(ErrorException{"Str not found"});
    }

private:
    const std::array<utility::string_t, enumSize> _arr;
};

}
}

#endif /* GIGA_UTILS_ENUM_H_ */

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
#include "HttpErrors.h"
#include "../utils/Utils.h"

#include <cpprest/details/basic_types.h>

using utility::string_t;

namespace giga
{

ErrorException::ErrorException () :
        std::exception()
{
}

ErrorException::ErrorException (const string_t& what) :
        std::exception(), whatStr(utils::wstr2str(what))
{
}

const char*
ErrorException::what () const noexcept
{
    return whatStr.c_str();
}

HttpErrorGeneric::HttpErrorGeneric (unsigned short status, const string_t& errorStr, const string_t& scope) :
        ErrorException(errorStr), status(status), scope(scope)
{
}

const char*
HttpErrorGeneric::what () const noexcept
{
    whatData = "status: " + std::to_string(status) + " err: " + whatStr;
    return whatData.c_str();
}

HttpErrorGeneric&
HttpErrorGeneric::setJson (const web::json::value& v)
{
    this->json = v;
    return *this;
}
HttpErrorGeneric&
HttpErrorGeneric::setJson (web::json::value&& v)
{
    this->json = v;
    return *this;
}

const web::json::value&
HttpErrorGeneric::getJson () const
{
    return this->json;
}

}


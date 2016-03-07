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
using giga::utils::to_string;

namespace giga
{

ErrorException::ErrorException () :
        std::exception()
{
}

ErrorException::ErrorException (const string_t& what) :
        std::exception(), whatStr(what)
{
}

const char*
ErrorException::what () const noexcept
{
    return utils::wstr2str(whatStr).c_str();
}

HttpErrorGeneric::HttpErrorGeneric (unsigned short status, const string_t& errorStr, const string_t& scope) :
        ErrorException(errorStr), status(status), scope(scope)
{
}

const char*
HttpErrorGeneric::what () const noexcept
{
    whatData = U("status: ") + to_string(status) + U(" err: ") + whatStr;
    return utils::wstr2str(whatData).c_str();
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

HttpErrorGeneric
HttpErrorGeneric::create(unsigned short status, const string_t& errorStr, const string_t& scope)
{
    switch (status)
    {
        case 401:
            return ErrorUnauthorized{errorStr, scope};
        case 403:
            return ErrorForbidden{errorStr, scope};
        case 400:
            return ErrorBadRequest{errorStr, scope};
        case 422:
            return ErrorUnprocessableEntity{errorStr, scope};
        case 423:
            return ErrorLocked{errorStr, scope};
        case 404:
            return ErrorNotFound{errorStr, scope};
        case 500:
            return ErrorInternalServerError{errorStr, scope};
        case 501:
            return ErrorNotImplemented{errorStr, scope};
        default:
            return HttpErrorGeneric{status, errorStr, scope};
    }
}

}


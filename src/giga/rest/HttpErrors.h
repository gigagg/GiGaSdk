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

#ifndef HTTPERRORS_H_
#define HTTPERRORS_H_

#include <boost/log/attributes/attribute.hpp>
#include <boost/throw_exception.hpp>
#include <cpprest/json.h>
#include <iostream>
#include <string>
#include "prepoc_manage.h"

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif

#ifdef DEBUG_LOG
#   define GIGA_DEBUG_LOG(lvl, data) std::cerr << U(#lvl) << ": " << data << std::endl
#   define GIGA_DEBUG_LOG_U(lvl, user, data) std::cerr << U(#lvl) << "(" << (user) << "): " << data << std::endl
#else
#   ifdef USE_BOOST_LOG
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/attributes/mutable_constant.hpp>
#include <boost/log/attributes/attribute.hpp>

#define GIGA_DEBUG_LOG(lvl, data) \
   BOOST_LOG_STREAM_WITH_PARAMS( \
      (::boost::log::trivial::logger::get()), \
         (giga::details::set_get_attrib("File", giga::details::path_to_filename(__FILE__))) \
         (giga::details::set_get_attrib("Line", __LINE__)) \
         (::boost::log::keywords::severity = (::boost::log::trivial::lvl)) \
   ) << data
#define GIGA_DEBUG_LOG_U(lvl, user, data) \
   BOOST_LOG_STREAM_WITH_PARAMS( \
      (::boost::log::trivial::logger::get()), \
         (giga::details::set_get_attrib("File", giga::details::path_to_filename(__FILE__))) \
         (giga::details::set_get_attrib("Line", __LINE__)) \
         (giga::details::set_get_attrib("User", (user))) \
         (::boost::log::keywords::severity = (::boost::log::trivial::lvl)) \
   ) << data

namespace giga {
namespace details {
// Set attribute and return the new value
template<typename ValueType>
ValueType set_get_attrib(const char* name, ValueType value) {
    auto attributes = boost::log::core::get()->get_thread_attributes();
    const auto it = attributes.find(name);
    if (it != attributes.end()) {
        auto attr = boost::log::attribute_cast<boost::log::attributes::mutable_constant<ValueType>>(it->second);
        attr.set(value);
        return attr.get();
    } else {
        auto attr = boost::log::attributes::mutable_constant<ValueType>{value};
        boost::log::core::get()->add_thread_attribute(name, attr);
        return attr.get();
    }
}

std::string
path_to_filename(std::string path);

}
}
#   else
#       define GIGA_DEBUG_LOG(lvl, data) do {} while(0)
#       define GIGA_DEBUG_LOG_U(lvl, user, data) do {} while(0)
#   endif
#endif


namespace giga {

class ErrorException : public std::exception
{
public:
    explicit ErrorException ();
    explicit ErrorException (const utility::string_t& what);

    virtual ~ErrorException ()              = default;
    ErrorException (const ErrorException&)  = default;
    ErrorException (ErrorException&&)       = default;

    virtual const char*
    what () const noexcept;

protected:
	mutable std::string whatStr = "";
};

class HttpErrorGeneric : public ErrorException
{
public:
    explicit HttpErrorGeneric (unsigned short status, const utility::string_t& errorStr = U(""), const utility::string_t& scope = U(""));

    virtual ~HttpErrorGeneric ()                = default;
    HttpErrorGeneric (const HttpErrorGeneric&)  = default;
    HttpErrorGeneric (HttpErrorGeneric&&)       = default;

    virtual const char*
    what () const noexcept;

    HttpErrorGeneric&
    setJson (const web::json::value& json);

    HttpErrorGeneric&
    setJson (web::json::value&& json);

    const web::json::value&
    getJson () const;

    template <class Manager>
    void
    visit(const Manager& us)
    {
        us.manageOpt(whatStr, U("errorStr"), std::string{});
        GIGA_MANAGE_OPT(us, scope, utility::string_t{});
    }

public:
    const unsigned short status = 500;
    utility::string_t scope     = U("");

private:
    web::json::value json = {};
    mutable std::string whatData;
};

template <unsigned short TStatus>
class HttpError : public HttpErrorGeneric
{
public:
    explicit HttpError (const utility::string_t& errorStr = U(""), const utility::string_t& scope = U("")) :
        HttpErrorGeneric (TStatus, errorStr, scope)
    {
    }
    virtual ~HttpError ()        = default;
    HttpError (const HttpError&) = default;
    HttpError (HttpError&&)      = default;
};

typedef HttpError<401> ErrorUnauthorized;
typedef HttpError<403> ErrorForbidden;
typedef HttpError<400> ErrorBadRequest;
typedef HttpError<422> ErrorUnprocessableEntity;
typedef HttpError<423> ErrorLocked;
typedef HttpError<404> ErrorNotFound;
typedef HttpError<500> ErrorInternalServerError;
typedef HttpError<501> ErrorNotImplemented;

//
// I use a macro to make sure the BOOST_THROW_EXCEPTION has the correct data in
// LINE/FILE/function
//
#define GIGA_THROW_HTTPERROR(status, errorStr, scope)                           \
switch (status)                                                                 \
    {                                                                           \
        case 401:                                                               \
            BOOST_THROW_EXCEPTION((ErrorUnauthorized{errorStr, scope}));        \
            break;                                                              \
        case 403:                                                               \
            BOOST_THROW_EXCEPTION((ErrorForbidden{errorStr, scope}));           \
            break;                                                              \
        case 400:                                                               \
            BOOST_THROW_EXCEPTION((ErrorBadRequest{errorStr, scope}));          \
            break;                                                              \
        case 422:                                                               \
            BOOST_THROW_EXCEPTION((ErrorUnprocessableEntity{errorStr, scope})); \
            break;                                                              \
        case 423:                                                               \
            BOOST_THROW_EXCEPTION((ErrorLocked{errorStr, scope}));              \
            break;                                                              \
        case 404:                                                               \
            BOOST_THROW_EXCEPTION((ErrorNotFound{errorStr, scope}));            \
            break;                                                              \
        case 500:                                                               \
            BOOST_THROW_EXCEPTION((ErrorInternalServerError{errorStr, scope})); \
            break;                                                              \
        case 501:                                                               \
            BOOST_THROW_EXCEPTION((ErrorNotImplemented{errorStr, scope}));      \
            break;                                                              \
        default:                                                                \
            BOOST_THROW_EXCEPTION((HttpErrorGeneric{status, errorStr, scope})); \
    }

}  // namespace giga

#endif /* HTTPERRORS_H_ */

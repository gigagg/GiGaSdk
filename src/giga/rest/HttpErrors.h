/*
 * HttpErrors.h
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#ifndef HTTPERRORS_H_
#define HTTPERRORS_H_

#include <boost/throw_exception.hpp>
#include <cpprest/json.h>
#include <iostream>
#include <string>
#include "prepoc_manage.h"

#ifdef DEBUG_LOG
#define GIGA_DEBUG_LOG(data) std::cerr << data << std::endl
#else
#define GIGA_DEBUG_LOG(data) do {} while(0)
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
    utility::string_t whatStr  = U("");
};

class HttpErrorGeneric : public ErrorException
{
public:
    static HttpErrorGeneric
    create(unsigned short status, const utility::string_t& errorStr = U(""), const utility::string_t& scope = U(""));

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
        us.manageOpt(whatStr, U("errorStr"), utility::string_t{});
        GIGA_MANAGE_OPT(us, scope, utility::string_t{});
    }

public:
    const unsigned short status = 500;
    utility::string_t scope           = U("");

private:
    web::json::value json = {};
    mutable utility::string_t whatData;
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

}  // namespace giga

#endif /* HTTPERRORS_H_ */

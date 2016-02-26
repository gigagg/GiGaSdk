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

#ifdef DEBUG
#define GIGA_DEBUG_LOG(data) std::cerr << data << std::endl
#else
#define GIGA_DEBUG_LOG(data) do {} while(0)
#endif

namespace giga {

class ErrorException : public std::exception
{
public:
    explicit ErrorException ();
    explicit ErrorException (const std::string& what);

    virtual ~ErrorException ()              = default;
    ErrorException (const ErrorException&)  = default;
    ErrorException (ErrorException&&)       = default;

    virtual const char*
    what () const noexcept;

protected:
    std::string whatStr  = "";
};

class HttpErrorGeneric : public ErrorException
{
public:
    static HttpErrorGeneric
    create(unsigned short status, const std::string& errorStr = "", const std::string& scope = "");

public:
    explicit HttpErrorGeneric (unsigned short status, const std::string& errorStr = "", const std::string& scope = "");

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
        us.manageOpt(whatStr, "errorStr", std::string{});
        GIGA_MANAGE_OPT(us, scope, std::string{});
    }

public:
    const unsigned short status = 500;
    std::string scope           = "";

private:
    web::json::value json = {};
    mutable std::string whatData;
};

template <unsigned short TStatus>
class HttpError : public HttpErrorGeneric
{
public:
    explicit HttpError (const std::string& errorStr = "", const std::string& scope = "") :
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

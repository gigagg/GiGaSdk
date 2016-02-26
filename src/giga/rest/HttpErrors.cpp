#include "HttpErrors.h"

namespace giga
{

ErrorException::ErrorException () :
        std::exception()
{
}

ErrorException::ErrorException (const std::string& what) :
        std::exception(), whatStr(what)
{
}

const char*
ErrorException::what () const noexcept
{
    return whatStr.c_str();
}

HttpErrorGeneric::HttpErrorGeneric (unsigned short status, const std::string& errorStr, const std::string& scope) :
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
HttpErrorGeneric::setJson (const web::json::value& json)
{
    this->json = json;
    return *this;
}
HttpErrorGeneric&
HttpErrorGeneric::setJson (web::json::value&& json)
{
    this->json = json;
    return *this;
}

const web::json::value&
HttpErrorGeneric::getJson () const
{
    return this->json;
}

HttpErrorGeneric
HttpErrorGeneric::create(unsigned short status, const std::string& errorStr, const std::string& scope)
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


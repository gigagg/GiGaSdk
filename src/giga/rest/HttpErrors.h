/*
 * HttpErrors.h
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#ifndef HTTPERRORS_H_
#define HTTPERRORS_H_

#include <string>
#include "prepoc_manage.h"

namespace giga {

    class HttpErrorGeneric : public std::exception {
    public:
        explicit HttpErrorGeneric (unsigned short status, const std::string& errorStr = "", const std::string& scope = "") :
                status (status), errorStr(errorStr), scope(scope)
        {
        }
        virtual ~HttpErrorGeneric () = default;
        HttpErrorGeneric (const HttpErrorGeneric&) = default;
        HttpErrorGeneric (HttpErrorGeneric&&) = default;

        virtual const char* what() const noexcept {
            return errorStr.c_str();
        }

        template <class Manager>
        void visit(const Manager& us){
          GIGA_MANAGE_OPT(us, errorStr, std::string{});
          GIGA_MANAGE_OPT(us, scope, std::string{});
        }

        const unsigned short status = 500;
        std::string errorStr = "";
        std::string scope = "";
    };

    template <unsigned short TStatus>
    class HttpError : public HttpErrorGeneric {
    public:
        explicit HttpError (const std::string& errorStr = "", const std::string& scope = "") :
            HttpErrorGeneric (TStatus, errorStr, scope)
        {
        }
        virtual ~HttpError () = default;
        HttpError (const HttpError&) = default;
        HttpError (HttpError&&) = default;
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

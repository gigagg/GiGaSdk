/*
 * HttpErrors.h
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#ifndef HTTPERRORS_H_
#define HTTPERRORS_H_

#include <cpprest/json.h>
#include <iostream>
#include <string>
#include "prepoc_manage.h"

#ifdef DEBUG
#define THROW(ex)               \
{                               \
    auto _m_exGG_ = (ex);       \
    _m_exGG_.file = __FILE__;   \
    _m_exGG_.line = std::to_string(__LINE__);   \
    _m_exGG_.name = #ex;        \
    throw (_m_exGG_);           \
} do {} while(0)
#else
#define THROW(ex) throw (ex)
#endif

#ifdef DEBUG
#define GIGA_DEBUG_LOG(data) std::cerr << data << std::endl
#else
#define GIGA_DEBUG_LOG(data) do {} while(0)
#endif

namespace giga {

    class ErrorException : public std::exception {
    public:
        explicit ErrorException () : std::exception(){}
        explicit ErrorException (const std::string& what) : std::exception(), whatStr(what)
        {}
        virtual ~ErrorException () = default;
        ErrorException (const ErrorException&) = default;
        ErrorException (ErrorException&&) = default;

        virtual const char* what() const noexcept {
            return whatStr.c_str();
        }

#ifdef DEBUG
        std::string file;
        std::string line;
        std::string name;

        void print(std::ostream &stream = std::cerr) const {
            stream << "### ErrorException: " << name << " ###"
                   << "\nFile : " << file
                   << "\nLine : " << line
                   << "\n" << what() << std::endl;
        }
#else
        void print(std::ostream &stream = std::cerr) const {
            stream << what() << std::endl;
        }
#endif
    protected:
        std::string whatStr  = "";
    };

    class HttpErrorGeneric : public ErrorException {
    public:
        explicit HttpErrorGeneric (unsigned short status, const std::string& errorStr = "", const std::string& scope = "") :
            ErrorException(errorStr), status (status), scope(scope)
        {
        }
        virtual ~HttpErrorGeneric () = default;
        HttpErrorGeneric (const HttpErrorGeneric&) = default;
        HttpErrorGeneric (HttpErrorGeneric&&) = default;

        virtual const char* what() const noexcept {
            whatData = "status: " + std::to_string(status) + " err: " + whatStr;
            return whatData.c_str();
        }

        template <class Manager>
        void visit(const Manager& us){
            us.manageOpt(whatStr, "errorStr", std::string{});
            GIGA_MANAGE_OPT(us, scope, std::string{});
        }

        HttpErrorGeneric& setJson(const web::json::value& json) {
            this->json = json;
            return *this;
        }
        HttpErrorGeneric& setJson(web::json::value&& json) {
            this->json = json;
            return *this;
        }

        const web::json::value& getJson() const {
            return this->json;
        }

    public:
        const unsigned short status = 500;
        std::string scope     = "";

    private:
        web::json::value json = {};
        mutable std::string whatData;
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

    inline HttpErrorGeneric
    BuildHttpError(unsigned short status, const std::string& errorStr = "", const std::string& scope = "")
    {
        switch (status) {
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
}  // namespace giga

#endif /* HTTPERRORS_H_ */

/**
 * @file ./HttpClient.h
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include "JsonUnserializer.h"
#include "JsonSerializer.h"
#include "HttpErrors.h"

#include <cpprest/http_client.h>

namespace giga {

class HttpClient final {

public:

    static constexpr auto API = "/api/1.0/";
//    static constexpr auto HOST = "http://localhost:8088";
    static constexpr auto HOST = "https://dev.gg";

    HttpClient();
    ~HttpClient() = default;
    HttpClient(const HttpClient&) = default;
    HttpClient(HttpClient&&) = default;

    void authenticate(const std::string& login, const std::string& password);

    web::uri_builder uri(const std::string& resource);

    template <typename T>
    web::uri_builder uri(const std::string& resource, const T& id, const std::string& subResource = "") {
        utility::ostringstream_t ss;
        ss.imbue(std::locale::classic());
        ss << API << resource << "/" << id;
        if (subResource.size() > 0) {
            ss << "/" << subResource;
        }
        return web::uri_builder{ss.str()};
    }
    template <typename T, typename U>
    web::uri_builder uri(const std::string& resource, const T& id, const std::string& subResource, const U& subId) {
        utility::ostringstream_t ss;
        ss.imbue(std::locale::classic());
        ss << API << resource << "/" << id << "/" << subResource << subId;
        return web::uri_builder{ss.str()};
    }

    template <typename T>
    pplx::task<std::shared_ptr<T>> request(const web::http::method &mtd, web::uri_builder uri) {
        return client.request(mtd, uri.to_string()).then([=](web::http::http_response response) {
            return onRequestPtr<T>(response);
        });
    }
    template <typename T, class U>
    pplx::task<std::shared_ptr<T>> request(const web::http::method &mtd, web::uri_builder uri, U&& bodyData) {
        auto json = web::json::value::object();
        bodyData.visit(JSonSerializer{json});
        return client.request(mtd, uri.to_string(), json.serialize(), "application/json;charset=utf-8").then([=](web::http::http_response response) {
            return onRequestPtr<T>(response);
        });
    }

    template <typename T>
    std::shared_ptr<T> onRequestPtr(web::http::http_response response) {
        return onRequest<std::shared_ptr<T>>(response);
    }

    template <typename T>
    T onRequest(web::http::http_response response) {
        auto headers = response.headers();
        auto ctype = headers.find("Content-Type");
        auto jsonType = std::string{"application/json"};
        if (ctype != headers.end() && ctype->second.compare(0, jsonType.size(), jsonType) == 0) {
            auto json = response.extract_json().get();
            auto s = JSonUnserializer{json};
            switch (response.status_code()) {
                case 200:
                    return s.unserialize<T>();
                case 401:
                    throw s.unserialize<ErrorUnauthorized>();
                case 403:
                    throw s.unserialize<ErrorForbidden>();
                case 400:
                    throw s.unserialize<ErrorBadRequest>();
                case 422:
                    throw s.unserialize<ErrorUnprocessableEntity>();
                case 423:
                    throw s.unserialize<ErrorLocked>();
                case 404:
                    throw s.unserialize<ErrorNotFound>();
                case 500:
                    throw s.unserialize<ErrorInternalServerError>();
                case 501:
                    throw s.unserialize<ErrorNotImplemented>();
                default:
                    HttpErrorGeneric data{response.status_code()};
                    data.visit(s);
                    throw data;
            }
        }
        HttpErrorGeneric data{response.status_code()};
        data.errorStr = response.extract_string().get();
        throw data;
    }

private:
    web::http::client::http_client_config getConfig() {
        auto config = web::http::client::http_client_config{};
#ifdef DEBUG
        config.set_validate_certificates(false);
#endif
        return config;
    }

private:
    web::http::client::http_client client;
};

}  // namespace giga

#endif /* HTTPCLIENT_H_ */

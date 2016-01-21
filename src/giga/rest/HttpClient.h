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

class HttpClient final
{
public:
    static constexpr auto API = "/api/1.0/";
    static constexpr auto HOST = "https://dev.gg";
    static constexpr auto JSON_CONTENT_TYPE = "application/json;charset=utf-8";

public:
    HttpClient();
    ~HttpClient() = default;
    HttpClient(const HttpClient&) = default;
    HttpClient(HttpClient&&) = default;

private:
    template <typename T>
    T getError(const JSonUnserializer& s, web::json::value&& json) const {
        auto e = s.unserialize<T>();
        e.setJson(std::move(json));
        return e;
    }

public:
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
        ss << API << resource << "/" << id << "/" << subResource << "/" << subId;
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
        auto data = json.serialize();
        return client.request(mtd, uri.to_string(), data, JSON_CONTENT_TYPE).then([=](web::http::http_response response) {
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
        auto jsonType = std::string("application/json");
        if (ctype != headers.end() && ctype->second.compare(0, jsonType.size(), jsonType) == 0) {
            auto json = response.extract_json(true).get();
            auto s = JSonUnserializer{json};
            switch (response.status_code()) {
                case 200:
                    try
                    {
                        return s.unserialize<T>();
                    }
                    catch (const std::exception& e)
                    {
                        std::cout << json.serialize() << std::endl;
                        throw e;
                    }
                    break;
                case 401:
                    THROW((getError<ErrorUnauthorized>(s, std::move(json))));
                case 403:
                    THROW((getError<ErrorForbidden>(s, std::move(json))));
                case 400:
                    THROW((getError<ErrorBadRequest>(s, std::move(json))));
                case 422:
                    THROW((getError<ErrorUnprocessableEntity>(s, std::move(json))));
                case 423:
                    THROW((getError<ErrorLocked>(s, std::move(json))));
                case 404:
                    THROW((getError<ErrorNotFound>(s, std::move(json))));
                case 500:
                    THROW((getError<ErrorInternalServerError>(s, std::move(json))));
                case 501:
                    THROW((getError<ErrorNotImplemented>(s, std::move(json))));
                default:
                    HttpErrorGeneric data{response.status_code()};
                    data.visit(s);
                    THROW(data.setJson(std::move(json)));
            }
        }
        THROW((HttpErrorGeneric{response.status_code(), response.extract_string().get()}));
    }

    inline web::http::client::http_client& getClient() { return client;};

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

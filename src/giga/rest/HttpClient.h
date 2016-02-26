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

namespace giga
{

class HttpClient final
{
public:
    static constexpr auto API = "/api/1.0/";
    static constexpr auto JSON_CONTENT_TYPE = "application/json;charset=utf-8";

public:
    HttpClient ();
    ~HttpClient () = default;
    HttpClient (const HttpClient&) = default;
    HttpClient (HttpClient&&) = default;

private:
    template<int unsigned short TStatus>
    HttpError<TStatus>
    getError (const JSonUnserializer& s, web::json::value&& json) const
    {
        auto e = s.unserialize<HttpError<TStatus>>();
        e.setJson(std::move(json));
        return e;
    }

public:
    void
    authenticate (const std::string& login, const std::string& password);

    web::uri_builder
    uri (const std::string& resource);

    template<typename T>
    web::uri_builder
    uri (const std::string& resource, const T& id, const std::string& subResource = "")
    {
        utility::ostringstream_t ss;
        ss.imbue(std::locale::classic());
        ss << API << resource << "/" << id;
        if (subResource.size() > 0)
        {
            ss << "/" << subResource;
        }
        return web::uri_builder{ss.str()};
    }

    template<typename T, typename U>
    web::uri_builder
    uri (const std::string& resource, const T& id, const std::string& subResource, const U& subId)
    {
        utility::ostringstream_t ss;
        ss.imbue(std::locale::classic());
        ss << API << resource << "/" << id << "/" << subResource << "/" << subId;
        return web::uri_builder{ss.str()};
    }

    template<typename T>
    pplx::task<std::shared_ptr<T>>
    request (const web::http::method &mtd, web::uri_builder uri)
    {
        GIGA_DEBUG_LOG(mtd << "  " << uri.to_string());
        return _http.request(mtd, uri.to_string()).then([=](web::http::http_response response) {
            return onRequestPtr<T>(response);
        });
    }

    template<typename T, class U>
    pplx::task<std::shared_ptr<T>>
    request (const web::http::method &mtd, web::uri_builder uri, U&& bodyData)
    {
        GIGA_DEBUG_LOG(mtd << "  " << uri.to_string());
        auto json = web::json::value::object();
        auto data = JSonSerializer{json}.toString(std::move(bodyData));
        return _http.request(mtd, uri.to_string(), data, JSON_CONTENT_TYPE).then([=](web::http::http_response response) {
            return onRequestPtr<T>(response);
        });
    }

    template<typename T>
    std::shared_ptr<T>
    onRequestPtr (web::http::http_response response)
    {
        return onRequest<std::shared_ptr<T>>(response);
    }

    template<typename T>
    T
    onRequest (web::http::http_response response)
    {
        auto headers = response.headers();
        auto ctype = headers.find("Content-Type");
        auto jsonType = std::string("application/json");
        if (ctype != headers.end() && ctype->second.compare(0, jsonType.size(), jsonType) == 0)
        {
            auto json = response.extract_json(true).get();
            auto s = JSonUnserializer{json};
            if (response.status_code() == 200)
            {
                try
                {
                    return s.unserialize<T>();
                }
                catch (const std::exception& e)
                {
                    GIGA_DEBUG_LOG("Error unserializing: " << json.serialize());
                    throw e;
                }
            }
            else
            {
                throwHttpError(response.status_code(), std::move(json));
            }
        }
        BOOST_THROW_EXCEPTION((HttpErrorGeneric{response.status_code(), response.extract_string().get()}));
    }

    void
    throwHttpError(unsigned short status, web::json::value&& json) const;

    const web::http::client::http_client&
    http () const;

private:
    web::http::client::http_client _http;
};

}  // namespace giga

#endif /* HTTPCLIENT_H_ */

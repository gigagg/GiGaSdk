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

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include "JsonUnserializer.h"
#include "JsonSerializer.h"
#include "HttpErrors.h"

#include <cpprest/http_client.h>

namespace giga
{

class RefreshingState;

class HttpClient final
{
public:
    static constexpr auto API = U("/api/1.0/");
    static constexpr auto JSON_CONTENT_TYPE = U("application/json");

public:
    HttpClient ();
    ~HttpClient ()                              = default;
    HttpClient (const HttpClient&)              = delete;
    HttpClient (HttpClient&&)                   = delete;
    HttpClient& operator= (const HttpClient&)   = delete;
    HttpClient& operator= (HttpClient&&)        = delete;

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
    authenticate (const utility::string_t& login, const utility::string_t& password);

    web::uri_builder
    uri (const utility::string_t& resource);

    template<typename T> web::uri_builder
    uri (const utility::string_t& resource, const T& id, const utility::string_t& subResource = U("")) const;

    template<typename T, typename U> web::uri_builder
    uri (const utility::string_t& resource, const T& id, const utility::string_t& subResource, const U& subId) const;

    template<typename T> pplx::task<std::shared_ptr<T>>
    request (const web::http::method &mtd, web::uri_builder uri);

    template<typename T, class U> pplx::task<std::shared_ptr<T>>
    request (const web::http::method &mtd, web::uri_builder uri, U&& bodyData);

    template<class U>
    pplx::task<web::http::http_response>
    rawRequest(const web::http::method &mtd, web::uri_builder uri, U&& bodyData);

    inline pplx::task<web::http::http_response>
    rawRequest (const web::http::method &mtd, web::uri_builder uri);

    template<typename T> std::shared_ptr<T>
    onRequestPtr (web::http::http_response response) const;

    template<typename T> T
    onRequest (web::http::http_response response) const;

    void
    throwHttpError(unsigned short status, web::json::value&& json) const;

    pplx::task<void>
    refreshToken();

    utility::string_t
    accessToken() const;

    void
    setUserAgent(utility::string_t userAgent);

private:
    web::http::client::http_client&
    http ();

private:
    web::http::client::http_client   _http;
    std::shared_ptr<RefreshingState> _rstate;
    utility::string_t                _userAgent;
    utility::string_t                _accessToken;
};

template<typename T>
web::uri_builder
HttpClient::uri (const utility::string_t& resource, const T& id, const utility::string_t& subResource /* = U("")*/) const
{
   utility::ostringstream_t ss;
   ss.imbue(std::locale::classic());
   ss << API << resource << U("/") << id;
   if (subResource.size() > 0)
   {
       ss << U("/") << subResource;
   }
   return web::uri_builder{ss.str()};
}

template<typename T, typename U>
web::uri_builder
HttpClient::uri (const utility::string_t& resource, const T& id, const utility::string_t& subResource, const U& subId) const
{
   utility::ostringstream_t ss;
   ss.imbue(std::locale::classic());
   ss << API << resource << U("/") << id << U("/") << subResource << U("/") << subId;
   return web::uri_builder{ss.str()};
}

template<class U>
pplx::task<web::http::http_response>
HttpClient::rawRequest(const web::http::method &mtd, web::uri_builder uri, U&& bodyData)
{
   auto json      = web::json::value::object();
   auto data      = JSonSerializer{json}.toString(std::move(bodyData));
   auto uriString = uri.to_string();
   GIGA_DEBUG_LOG(trace, mtd << U("  ") << uri.to_string() << U(" ") << data);

   web::http::http_request msg(mtd);
   msg.set_request_uri(uri.to_string());
   msg.set_body(JSonSerializer{json}.toString(std::move(bodyData)), JSON_CONTENT_TYPE);
   msg.headers().add(web::http::header_names::user_agent, _userAgent);

   return refreshToken().then([=]() {
       return http().request(msg);
   });
}

inline pplx::task<web::http::http_response>
HttpClient::rawRequest (const web::http::method &mtd, web::uri_builder uri)
{
   GIGA_DEBUG_LOG(trace, mtd << U("  ") << uri.to_string());
   auto uriString = uri.to_string();

   web::http::http_request msg(mtd);
   msg.set_request_uri(uri.to_string());
   msg.headers().add(web::http::header_names::user_agent, _userAgent);

   return refreshToken().then([=]() {
       return http().request(msg);
   });
}

template<typename T>
pplx::task<std::shared_ptr<T>>
HttpClient::request (const web::http::method &mtd, web::uri_builder uri)
{
   return rawRequest(mtd, uri).then([=](web::http::http_response response) {
       return onRequestPtr<T>(response);
   });
}

template<typename T, class U>
pplx::task<std::shared_ptr<T>>
HttpClient::request (const web::http::method &mtd, web::uri_builder uri, U&& bodyData)
{
   return rawRequest(mtd, uri, std::move(bodyData)).then([=](web::http::http_response response) {
       return onRequestPtr<T>(response);
   });
}

template<typename T>
std::shared_ptr<T>
HttpClient::onRequestPtr (web::http::http_response response) const
{
   return onRequest<std::shared_ptr<T>>(response);
}

template<typename T>
T
HttpClient::onRequest (web::http::http_response response) const
{
   auto headers = response.headers();
   auto ctype = headers.find(U("Content-Type"));
   auto jsonType = utility::string_t(U("application/json"));
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
               GIGA_DEBUG_LOG(error, U("Error unserializing: ") << json.serialize());
               throw e;
           }
       }
       else
       {
           throwHttpError(response.status_code(), std::move(json));
       }
   }
   GIGA_THROW_HTTPERROR(response.status_code(), response.extract_string().get(), U(""));
}

}  // namespace giga

#endif /* HTTPCLIENT_H_ */

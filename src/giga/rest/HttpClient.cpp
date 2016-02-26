#include "HttpClient.h"
#include <cpprest/http_client.h>
#include <boost/regex.hpp>

#include "Empty.h"
#include "JsonObj.h"
#include "../Config.h"
#include "../Application.h"

using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;

namespace {

web::http::client::http_client_config getConfig() {
    auto config = web::http::client::http_client_config{};
#ifdef DEBUG
    config.set_validate_certificates(false);
#endif
    return config;
}

}

namespace giga
{

HttpClient::HttpClient () :
        _http (Application::config().apiHost(), getConfig())
{
}

web::uri_builder
HttpClient::uri (const std::string& resource)
{
    return web::uri_builder
        {API + resource};
}

struct Redirect {
    std::string redirect = "";
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, redirect);
    }
};

void
HttpClient::authenticate (const std::string& login, const std::string& password)
{
    const auto& conf = Application::get().config();
    oauth2_config m_oauth2_config(conf.appId(), conf.appKey(), conf.appOauthAuthorizationEndpoint(),
                                  conf.appOauthTokenEndpoint(), conf.appRedirectUri());

    m_oauth2_config.set_scope(conf.appScope());
    auto auth_uri = m_oauth2_config.build_authorization_uri(true);  /* Get the authorization uri */
    auto state  = std::string{};
    auto regex  = boost::regex{".*state=([a-zA-Z0-9]+).*"};
    auto what   = boost::cmatch{};
    if(boost::regex_match(auth_uri.c_str(), what, regex))
    {
        state = std::string{what[1].first, what[1].second};
    }

    // I manually do the browser work here ...

    auto body = JsonObj{};
    body.add("login", login);
    body.add("password", password);
    auto url = "/rest/login";
    auto request = _http.request(methods::POST, url, JSonSerializer::toString(body), JSON_CONTENT_TYPE).then([=](web::http::http_response response) {
        onRequest<Empty>(response);
        auto headers = response.headers();

        const auto& conf = Application::get().config();
        auto body = JsonObj{};
        body.add("oauth", std::string("true"));
        body.add("response_type", std::string("code"));
        body.add("client_id", conf.appId());
        body.add("redirect_uri", conf.appRedirectUri());
        body.add("state", state);
        body.add("scope", conf.appScope());
        body.add("authorized", true);

        auto r = http_request{methods::POST};
        r.set_request_uri("/rest/oauthvalidate");
        r.set_body(JSonSerializer::toString(body), JSON_CONTENT_TYPE);
        auto it = headers.find("Set-Cookie");
        if (it != headers.end()) {
            r.headers().add("Cookie", it->second);
        }
        auto request = _http.request(r).then([=](web::http::http_response response) -> std::string {
            auto redirect = onRequest<Redirect>(response);
            return redirect.redirect;
        });
        request.wait();
        return request.get();
    });
    request.wait();

    auto uri = uri_builder{request.get()}.to_uri();
    m_oauth2_config.token_from_redirected_uri(uri).wait();

    // regenerate client, with the oauth2 config.
    auto config = getConfig();
    config.set_oauth2(m_oauth2_config);
    _http = {Application::config().apiHost(), config};
}

void
HttpClient::throwHttpError(unsigned short status, web::json::value&& json) const
{
    auto s = JSonUnserializer{json};
    switch (status)
    {
        case 401:
            BOOST_THROW_EXCEPTION((getError<401>(s, std::move(json))));
            break;
        case 403:
            BOOST_THROW_EXCEPTION((getError<403>(s, std::move(json))));
            break;
        case 400:
            BOOST_THROW_EXCEPTION((getError<400>(s, std::move(json))));
            break;
        case 422:
            BOOST_THROW_EXCEPTION((getError<422>(s, std::move(json))));
            break;
        case 423:
            BOOST_THROW_EXCEPTION((getError<423>(s, std::move(json))));
            break;
        case 404:
            BOOST_THROW_EXCEPTION((getError<404>(s, std::move(json))));
            break;
        case 500:
            BOOST_THROW_EXCEPTION((getError<500>(s, std::move(json))));
            break;
        case 501:
            BOOST_THROW_EXCEPTION((getError<501>(s, std::move(json))));
            break;
        default:
            HttpErrorGeneric data{status};
            data.visit(s);
            data.setJson(std::move(json));
            BOOST_THROW_EXCEPTION(data);
            break;
    }
}

const web::http::client::http_client&
HttpClient::http () const
{
    return _http;
}


}


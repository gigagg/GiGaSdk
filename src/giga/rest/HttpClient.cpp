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
#include "HttpClient.h"
#include <cpprest/http_client.h>
#include <boost/regex.hpp>
#include <chrono>

#include "Empty.h"
#include "JsonObj.h"
#include "../Config.h"
#include "../Application.h"
#include "../utils/Utils.h"

using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;
using std::chrono::high_resolution_clock;
using utility::string_t;

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

class RefreshingState
{
public:
    bool                              isRefreshing  = false;
    high_resolution_clock::time_point tokenExpireAt = {};
    std::mutex                        mut           = {};
};

HttpClient::HttpClient () :
        _http (Application::config().apiHost(), getConfig()), _rstate{std::make_shared<RefreshingState>()}
{
}

web::uri_builder
HttpClient::uri (const string_t& resource)
{
    return web::uri_builder{API + resource};
}

struct Redirect {
    string_t redirect = U("");
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, redirect);
    }
};

void
HttpClient::authenticate (const string_t& login, const string_t& password)
{
    const auto& conf = Application::get().config();
    oauth2_config m_oauth2_config(conf.appId(), conf.appKey(), conf.appOauthAuthorizationEndpoint(),
                                  conf.appOauthTokenEndpoint(), conf.appRedirectUri());

    m_oauth2_config.set_scope(conf.appScope());
    auto auth_uri = m_oauth2_config.build_authorization_uri(true);  /* Get the authorization uri */
    auto state  = string_t{};
    auto regex  = boost::regex{".*state=([a-zA-Z0-9]+).*"};
    auto what   = boost::cmatch{};
    auto uriStr = utils::wstr2str(auth_uri);
    if(boost::regex_match(uriStr.c_str(), what, regex))
    {
        state = utils::str2wstr(what.str(1));
    }

    // I manually do the browser work here ...

    auto body = JsonObj{};
    body.add(U("login"), login);
    body.add(U("password"), password);
    auto url = U("/rest/login");
    auto request = _http.request(methods::POST, url, JSonSerializer::toString(body), JSON_CONTENT_TYPE).then([=](web::http::http_response response) {
        onRequest<Empty>(response);
        auto headers = response.headers();

        const auto& conf = Application::get().config();
        auto body = JsonObj{};
        body.add(U("oauth"), string_t(U("true")));
        body.add(U("response_type"), string_t(U("code")));
        body.add(U("client_id"), conf.appId());
        body.add(U("redirect_uri"), conf.appRedirectUri());
        body.add(U("state"), state);
        body.add(U("scope"), conf.appScope());
        body.add(U("authorized"), true);

        auto r = http_request{methods::POST};
        r.set_request_uri(U("/rest/oauthvalidate"));
        r.set_body(JSonSerializer::toString(body), JSON_CONTENT_TYPE);
        auto it = headers.find(U("Set-Cookie"));
        if (it != headers.end()) {
            r.headers().add(U("Cookie"), it->second);
        }
        auto request = _http.request(r).then([=](web::http::http_response response) -> string_t {
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
    _rstate->tokenExpireAt = std::chrono::high_resolution_clock::now() + std::chrono::seconds{3600};
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

pplx::task<void>
HttpClient::refreshToken()
{
    bool shouldRefresh = false;
    {
        std::lock_guard<std::mutex> l{_rstate->mut};
        shouldRefresh = !_rstate->isRefreshing
                            && _http.client_config().oauth2() != nullptr
                            && _rstate->tokenExpireAt < (std::chrono::high_resolution_clock::now() + std::chrono::seconds{600}); // gets 600s to do the refresh
        _rstate->isRefreshing = shouldRefresh || _rstate->isRefreshing;
    }

    if (shouldRefresh)
    {
        GIGA_DEBUG_LOG("-- Refreshing access token");
        try {
            auto rstate = _rstate;
            return _http.client_config().oauth2()->token_from_refresh().then([rstate](){
                rstate->tokenExpireAt = std::chrono::high_resolution_clock::now() + std::chrono::seconds{3600};
                rstate->mut.unlock();
            });
        }
        catch (...)
        {
            _rstate->mut.unlock();
            throw;
        }
    }
    return pplx::create_task([](){});
}
}


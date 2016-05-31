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
#include "../version.h"
#include "../utils/Utils.h"

using namespace web::http;
using namespace web::http::client;
using namespace web::http::oauth2::experimental;
using std::chrono::high_resolution_clock;
using utility::string_t;

namespace {

web::http::client::http_client_config getConfig() {
    auto config = web::http::client::http_client_config{};
#ifdef USE_DEV_GG
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
    mutable std::mutex                mut           = {};
};

HttpClient::HttpClient () :
        _http (Config::get().apiHost(), getConfig()), _rstate{std::make_shared<RefreshingState>()},
        _userAgent{U(GIGA_UA)},
        _accessToken{}
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
    const auto& conf = Config::get();
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

    http_request msg(methods::POST);
    msg.set_request_uri(url);
    msg.set_body(JSonSerializer::toString(body), JSON_CONTENT_TYPE);
    msg.headers().add(header_names::user_agent, _userAgent);

    auto request = _http.request(msg).then([=](web::http::http_response response) {
        onRequest<Empty>(response);
        auto headers = response.headers();

        const auto& conf = Config::get();
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
    _http = {Config::get().apiHost(), config};
    _rstate->tokenExpireAt = std::chrono::high_resolution_clock::now() + std::chrono::seconds{3600};
    _accessToken = config.oauth2()->token().access_token();
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
                            && _rstate->tokenExpireAt < (std::chrono::high_resolution_clock::now() + std::chrono::seconds{300}); // gets 300s to do the refresh
        _rstate->isRefreshing = shouldRefresh || _rstate->isRefreshing;
    }

    if (shouldRefresh)
    {
        auto rstate       = _rstate;
        auto& http        = _http;
        auto& accessToken = _accessToken;

        //
        // All this is an ugly hack to make sure the refresh works
        // cf: const_cast etc ...
        //

        auto oaut2copy = std::make_shared<oauth2::experimental::oauth2_config>(*_http.client_config().oauth2());
        return pplx::create_task([rstate, &http, &accessToken, oaut2copy]() {
            try
            {
                GIGA_DEBUG_LOG(trace, "Refreshing access token");
                oaut2copy->token_from_refresh().get();

                std::lock_guard<std::mutex> l{ rstate->mut };
                if (oaut2copy->token().refresh_token().empty() &&
                    !http.client_config().oauth2()->token().refresh_token().empty())
                {
                    const_cast<oauth2_token&>(oaut2copy->token()).set_refresh_token(http.client_config().oauth2()->token().refresh_token());
                }
                rstate->tokenExpireAt = std::chrono::high_resolution_clock::now() + std::chrono::seconds{ 3600 };
                const_cast<http_client_config*>(&http.client_config())->set_oauth2(*oaut2copy);
                accessToken = http.client_config().oauth2()->token().access_token();
                rstate->isRefreshing = false;
            }
            catch (std::exception e)
            {
                // Ignore errors here (no throw)
                // because the token may be still valid
                GIGA_DEBUG_LOG(error, utils::exceptionInfos());
                std::lock_guard<std::mutex> l{ rstate->mut };
                rstate->isRefreshing = false;
            }
        });

    }
    return pplx::create_task([](){});
}

utility::string_t
HttpClient::accessToken() const
{
    std::lock_guard<std::mutex> l{ _rstate->mut };
    return _accessToken;
}

void
HttpClient::setUserAgent(utility::string_t userAgent)
{
    _userAgent = userAgent;
}

}


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

#ifndef GIGA_CONFIG_H_
#define GIGA_CONFIG_H_

#include <string>

namespace giga
{
class Application;

class Config final
{
public:
    Config(Config const&)         = delete;
    void operator=(Config const&) = delete;

private:
    friend class Application;

    explicit
    Config() = default;

public:
    inline const utility::string_t& appRedirectUri() const {return _appRedirectUri;}
    inline const utility::string_t& appId() const {return _appId;}
    inline const utility::string_t& appKey() const {return _appKey;}
    inline const utility::string_t& appScope() const {return _appScope;}

    inline const utility::string_t& appOauthAuthorizationEndpoint() const {return _oauthAuthorizationEndpoint;}
    inline const utility::string_t& appOauthTokenEndpoint() const {return _oauthTokenEndpoint;}

    inline const utility::string_t& apiHost() const {return _apiHost;}

private:
    utility::string_t _appRedirectUri;
    utility::string_t _appId;
    utility::string_t _appKey;
    utility::string_t _appScope;

    const utility::string_t _oauthAuthorizationEndpoint = U("https://giga.gg/oauth/authorize");
    const utility::string_t _oauthTokenEndpoint         = U("https://giga.gg/oauth/token");
    const utility::string_t _apiHost                    = U("https://giga.gg");
};

} /* namespace giga */

#endif /* GIGA_CONFIG_H_ */

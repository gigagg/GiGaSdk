/*
 * Config.h
 *
 *  Created on: 13 janv. 2016
 *      Author: thomas
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

    const utility::string_t _oauthAuthorizationEndpoint = "https://dev.gg/oauth/authorize";
    const utility::string_t _oauthTokenEndpoint         = "https://dev.gg/oauth/token";
    const utility::string_t _apiHost                    = "https://dev.gg";
};

} /* namespace giga */

#endif /* GIGA_CONFIG_H_ */

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
    inline const std::string& appRedirectUri() const {return _appRedirectUri;}
    inline const std::string& appId() const {return _appId;}
    inline const std::string& appKey() const {return _appKey;}
    inline const std::string& appScope() const {return _appScope;}

    inline const std::string& appOauthAuthorizationEndpoint() const {return _oauthAuthorizationEndpoint;}
    inline const std::string& appOauthTokenEndpoint() const {return _oauthTokenEndpoint;}

private:
    std::string _appRedirectUri;
    std::string _appId;
    std::string _appKey;
    std::string _appScope;

    const std::string _oauthAuthorizationEndpoint = "https://dev.gg/oauth/authorize";
    const std::string _oauthTokenEndpoint         = "https://dev.gg/oauth/token";
};

} /* namespace giga */

#endif /* GIGA_CONFIG_H_ */

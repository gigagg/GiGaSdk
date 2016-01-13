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

class Config final
{
public:
    static Config&
    getInstance ()
    {
        static Config instance;
        return instance;
    }

    static Config& init(std::string&& appRedirectUri, std::string&& appId, std::string&& appKey, std::string&& appScope = "basic network groups files basic:write network:write groups:write files:write")
    {
        auto& instance = getInstance();
        instance.doInit(std::move(appRedirectUri), std::move(appId), std::move(appKey), std::move(appScope));
        return instance;
    }

    explicit Config() {}
    Config(Config const&)         = delete;
    void operator=(Config const&) = delete;

private:
    inline void doInit(std::string&& appRedirectUri, std::string&& appId, std::string&& appKey, std::string&& appScope)
    {
        this->appRedirectUri = appRedirectUri;
        this->appId = appId;
        this->appKey = appKey;
        this->appScope = appScope;
    }

public:
    inline const std::string& getAppRedirectUri() const {return appRedirectUri;}
    inline const std::string& getAppId() const {return appId;}
    inline const std::string& getAppKey() const {return appKey;}
    inline const std::string& getAppScope() const {return appScope;}

    inline const std::string& getAppOauthAuthorizationEndpoint() const {return oauthAuthorizationEndpoint;}
    inline const std::string& getAppOauthTokenEndpoint() const {return oauthTokenEndpoint;}

private:
    std::string appRedirectUri;
    std::string appId;
    std::string appKey;
    std::string appScope;

    const std::string oauthAuthorizationEndpoint = "https://dev.gg/oauth/authorize";
    const std::string oauthTokenEndpoint         = "https://dev.gg/oauth/token";


};

} /* namespace giga */

#endif /* GIGA_CONFIG_H_ */

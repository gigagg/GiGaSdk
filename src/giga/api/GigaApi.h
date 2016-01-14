/*
 * GigaApi.h
 *
 *  Created on: 2 oct. 2015
 *      Author: thomas
 */

#ifndef CLIENT_GIGAAPI_H_
#define CLIENT_GIGAAPI_H_

#include "../rest/HttpClient.h"
#include "../rest/JsonObj.h"
#include "data/Locale.h"

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

namespace giga {

class GigaApi
{
public:
    GigaApi() = default;
    GigaApi(const GigaApi&) = default;
    GigaApi(GigaApi&&) = default;
    virtual ~GigaApi() = default;

public:
    static pplx::task<std::string> authenticate(const std::string& login, const std::string& password);

protected:
    static HttpClient client;
};

} // namespace giga
#endif /* CLIENT_GIGAAPI_H_ */

#include "HttpClient.h"
#include <cpprest/http_client.h>

namespace giga {
    const std::string HttpClient::API = "/rest/";
    const std::string HttpClient::HOST = "https://dev.gg";

    HttpClient::HttpClient() : client(HOST, getConfig()) {
    }

    web::uri_builder HttpClient::uri(const std::string& resource) {
        return web::uri_builder{API + resource};
    }
}

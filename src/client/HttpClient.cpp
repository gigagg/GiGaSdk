#include "HttpClient.h"
#include <cpprest/http_client.h>

namespace giga {

    HttpClient::HttpClient() : client(HttpClient::HOST, getConfig()) {
    }

    web::uri_builder HttpClient::uri(const std::string& resource) {
        return web::uri_builder{API + resource};
    }
}

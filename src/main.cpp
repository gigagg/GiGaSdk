/**
 * @file main.cpp
 * @brief the main
 * @author Thomas Guyard
 */


#include <iostream>
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

#include <cpprest/http_listener.h>              // HTTP server
#include <cpprest/json.h>                       // JSON library
#include <cpprest/uri.h>                        // URI library
#include <cpprest/ws_client.h>                  // WebSocket client
#include <cpprest/containerstream.h>            // Async streams backed by STL containers
#include <cpprest/interopstream.h>              // Bridges for integrating Async streams with STL and WinRT streams
#include <cpprest/rawptrstream.h>               // Async streams backed by raw pointer to memory
#include <cpprest/producerconsumerstream.h>     // Async streams for producer consumer scenarios

#include "client/GigaApi.h"
#include "model/JsonSerializer.h"

using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams


int main ( int /*argc*/, char** /*argv*/ )
{
//    auto client = giga::HttpClient{};
//    auto requestTask = client.request<Test>(methods::GET, client.uri("test"));
    auto requestTask = giga::TestApi::getTest();

    // Wait for all the outstanding I/O to complete and handle any exceptions
    try
    {
        requestTask.wait();
        auto result = requestTask.get();

        auto json = web::json::value::object();
        result->visit(giga::JSonSerializer{json});
        std::cout << json.serialize() << std::endl;
    }
    catch (const std::exception &e)
    {
        printf("Error exception:%s\n", e.what());
    }

	return 0;
}	// ----------  end of function main  ----------

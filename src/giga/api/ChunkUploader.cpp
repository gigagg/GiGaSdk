/*
 * ChunkUploader.cpp
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#include "ChunkUploader.h"
#include "data/Node.h"
#include "data/User.h"
#include "../rest/HttpErrors.h"

#include <boost/filesystem.hpp>
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <string>

using boost::filesystem::file_size;
using Concurrency::streams::basic_istream;
using Concurrency::streams::file_stream;
using giga::data::Node;
using pplx::cancellation_token;
using pplx::cancellation_token_source;
using web::http::client::http_client;
using web::http::client::http_client_config;
using web::http::http_request;
using web::http::http_response;
using web::http::methods;
using web::http::oauth2::experimental::oauth2_config;
using web::uri;
using web::uri_builder;


namespace giga
{
namespace api
{

http_client_config createConfig(const oauth2_config& oauth)
{
    auto config = http_client_config{};
    config.set_oauth2(oauth2_config{oauth});
#ifdef DEBUG
    config.set_validate_certificates(false);
#endif
    return config;
}

ChunkUploader::ChunkUploader (uri_builder& uploadUrl, const std::string& nodeName, const std::string& sha1, const std::string& filename,
                              const std::string& mime, cancellation_token_source cts) :
               uploadUrl{uploadUrl},
               nodeName{nodeName},
               sha1{sha1},
               filename{filename},
               mime{mime},
               fileSize{file_size(filename)},
               http{uri{"https://" + uploadUrl.host() + "/"}, createConfig(*client.getClient().client_config().oauth2())},
               cts{cts}
{
}

struct AutoClosingFileStream
{
    explicit AutoClosingFileStream(Concurrency::streams::basic_istream<unsigned char> filestream) :
        filestream{filestream}
    {
    }

    ~AutoClosingFileStream()
    {
        filestream.close();
    }

    Concurrency::streams::basic_istream<unsigned char> filestream;
};

std::shared_ptr<Node>
ChunkUploader::upload ()
{
    try {
        AutoClosingFileStream cfs{file_stream<unsigned char>::open_istream(filename).get()};
        filestream = cfs.filestream;

        return sendChunk(0ul, true).then([](std::vector<std::shared_ptr<Node>> nodes)
        {
            if (nodes.size() == 1)
            {
                return nodes[0];
            }
            if (nodes.size() == 0)
            {
                return std::shared_ptr<data::Node>{};
            }
            THROW(ErrorException{"Incorrect number of nodes returned"});
        }).get();

    } catch (const pplx::task_canceled& e) {
        return std::shared_ptr<Node>{};
    }
    catch (const web::http::http_exception& e)
    {
        if (std::string{"Operation canceled"}.compare(e.what()) == 0)
        {
            return std::shared_ptr<Node>{};
        }
        throw;
    }
}

pplx::task<std::vector<std::shared_ptr<Node>>>
ChunkUploader::sendChunk (uint64_t position, bool isFirstChunk)
{
    if (position >= fileSize)
    {
        THROW(ErrorException{"Invalid position/fileSize"});
    }
    if (!filestream.is_open() || !filestream.is_valid())
    {
        THROW(ErrorException{"File is not open/valid"});
    }

    // For some reason number smaller to 512 seemed to make it bug. (segmentation fault with 256).
    // I should investigate this further ...
    auto chunkSize = std::min(isFirstChunk ? 1024 : CHUNK_SIZE, fileSize - position);

    auto r = http_request{methods::POST};
    r.set_request_uri(uploadUrl.to_uri());

    r.headers().add("Content-Disposition", "attachment, filename=\"" + web::uri::encode_data_string(nodeName) + "\"");
    r.headers().add("Session-Id", getCurrentUser().id + "-" + sha1);
    r.headers().add("Content-Range", "bytes " + std::to_string(position) + "-" + std::to_string(chunkSize - 1 + position) + "/" + std::to_string(fileSize));

    filestream.seek(position);

    r.set_body(filestream, chunkSize, mime);

    return http.request(r, cts.get_token()).then([this](http_response response)
    {
        auto headers = response.headers();
        auto ctype = headers.find("Content-Type");

        if (ctype != headers.end() && ctype->second == std::string("text/plain"))
        {
            auto content = response.extract_string(true).get();
            std::stringstream ss(content);
            std::string startStr;
            std::string endStr;
            std::getline(ss, startStr, '-');
            std::getline(ss, endStr, '/');

            auto start = std::stoul(startStr.c_str());
            auto end = std::stoul(endStr.c_str());

            if (start > 0 && end > start)
            {
                end = 0;
            }

            auto t = sendChunk(end + 1);
            if (t.wait() != pplx::task_status::canceled)
            {
                return t.get();
            }
            return std::vector<std::shared_ptr<Node>>{};
        }
        return client.onRequest<std::vector<std::shared_ptr<Node>>>(response);
    }, cts.get_token());
}

double_t ChunkUploader::progress() const
{
    if (!filestream.is_open() || !filestream.is_valid())
    {
        return 0.;
    }
    if (filestream.is_eof())
    {
        return 1.;
    }
    return ((double)filestream.tell()) / (double)fileSize;
}

} /* namespace api */
} /* namespace giga */

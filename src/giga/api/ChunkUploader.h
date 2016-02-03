/*
 * ChunkUploader.h
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_API_CHUNKUPLOADER_H_
#define GIGA_API_CHUNKUPLOADER_H_

#include "GigaApi.h"
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <iosfwd>

namespace web
{
class uri_builder;
}

namespace Concurrency
{
namespace streams
{
template<typename CharType>
    class basic_istream;
}
}

namespace giga
{
namespace data
{
class Node;
}

namespace api
{


class ChunkUploader : GigaApi
{
public:
    static constexpr uint64_t CHUNK_SIZE = 1024ul * 1024ul;

    explicit
    ChunkUploader (web::uri_builder& uploadUrl, const std::string& nodeName, const std::string& sha1, const std::string& filename,
                   const std::string& mime, pplx::cancellation_token_source cts);

    std::shared_ptr<data::Node>
    upload ();

    double_t
    progress () const;

private:
    pplx::task<std::vector<std::shared_ptr<data::Node>>>
    sendChunk (uint64_t position, bool isFirstChunk = false);

private:
    web::uri_builder uploadUrl;
    std::string nodeName;
    std::string sha1;
    std::string filename;
    std::string mime;

    uint64_t fileSize;

    web::http::client::http_client http;
    Concurrency::streams::basic_istream<unsigned char> filestream;
    pplx::cancellation_token_source cts;
};

} /* namespace api */
} /* namespace giga */

#endif /* GIGA_API_CHUNKUPLOADER_H_ */

/*
 * ChunkUploader.h
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_API_CHUNKUPLOADER_H_
#define GIGA_API_CHUNKUPLOADER_H_

#include "CurlProgress.h"

#include <cpprest/http_client.h>
#include <iosfwd>
#include <cpprest/details/basic_types.h>

namespace
{
class ReadCallbackData;
}

namespace curl
{
class curl_easy;
}

namespace giga
{
namespace data
{
class Node;
}

namespace details
{

class ChunkUploader
{
public:
    static constexpr uint64_t CHUNK_SIZE = 1024ul * 1024ul;

    explicit
    ChunkUploader (web::uri_builder& uploadUrl, const utility::string_t& nodeName, const utility::string_t& sha1,
                   const utility::string_t& filename, const utility::string_t& mime, CurlProgress* progress);

    std::shared_ptr<data::Node>
    upload ();

private:
    utility::string_t
    sendChunk (uint64_t position, ReadCallbackData& data, curl::curl_easy& curl, std::ostringstream& str);

private:
    web::uri_builder  _uploadUrl;
    const utility::string_t _nodeName;
    const utility::string_t _sha1;
    const utility::string_t _filename;
    const utility::string_t _mime;

    uint64_t          _fileSize;

    CurlProgress* _progress;
};

} /* namespace details */
} /* namespace giga */

#endif /* GIGA_API_CHUNKUPLOADER_H_ */

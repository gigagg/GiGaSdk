/*
 * Copyright 2016 Gigatribe
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
struct Node;
}

namespace details
{

class ChunkUploader
{
public:
    static constexpr uint64_t CHUNK_SIZE = 1024ul * 1024ul;

    explicit
    ChunkUploader (web::uri_builder& uploadUrl, const utility::string_t& nodeName, const std::string& sha1,
                   const utility::string_t& filename, const utility::string_t& mime, CurlProgress* progress);

    ChunkUploader()                                = delete;
    ChunkUploader(const ChunkUploader&)            = delete;
    ChunkUploader(ChunkUploader&&)                 = delete;
    ChunkUploader& operator=(ChunkUploader&&)      = delete;
    ChunkUploader& operator=(const ChunkUploader&) = delete;


    std::shared_ptr<data::Node>
    upload ();

private:
    utility::string_t
    sendChunk (uint64_t position, ReadCallbackData& data, curl::curl_easy& curl, std::ostringstream& str);

private:
    web::uri_builder        _uploadUrl;
    const utility::string_t _nodeName;
    const std::string       _sha1;
    const utility::string_t _filename;
    const utility::string_t _mime;
    uint64_t                _fileSize;
    CurlProgress*           _progress;
};

} /* namespace details */
} /* namespace giga */

#endif /* GIGA_API_CHUNKUPLOADER_H_ */

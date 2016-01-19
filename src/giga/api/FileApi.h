/*
 * FileApi.h
 *
 *  Created on: 14 janv. 2016
 *      Author: thomas
 */

#ifndef GIGA_API_FILEAPI_H_
#define GIGA_API_FILEAPI_H_

#include <iosfwd>
#include "GigaApi.h"

namespace giga
{

namespace data {
class Node;
}

class FileApi final : GigaApi
{
public:
    static pplx::task<std::shared_ptr<data::Node>>
    uploadFile (const std::string& filename, const std::string& nodeName, const std::string& parentId);

    pplx::task<std::shared_ptr<data::Node>>
    uploadChunk (const std::string& filename, const std::string& nodeName, const std::string& uploadUrl, const std::string& sha1);
};

} /* namespace giga */

#endif /* GIGA_API_FILEAPI_H_ */

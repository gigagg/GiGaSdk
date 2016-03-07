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

#ifndef GIGA_API_DATA_MIMEICONASSOCIATION_H_
#define GIGA_API_DATA_MIMEICONASSOCIATION_H_

#include <boost/filesystem.hpp>
#include <cpprest/http_client.h>
#include <iosfwd>
#include <unordered_map>

namespace giga
{
namespace data
{

class MimeIconAssociation
{
public:
    static web::uri
    icon(const std::string& mimeType, const boost::filesystem::path& filename);

    static web::uri
    bigIcon(const std::string& mimeType, const boost::filesystem::path& filename);

private:
    static std::string
    name(const std::string& mimeType, const boost::filesystem::path& filename);

private:
    static std::unordered_map<std::string, std::string> _data;
};

} /* namespace data */
} /* namespace giga */

#endif /* GIGA_API_DATA_MIMEICONASSOCIATION_H_ */

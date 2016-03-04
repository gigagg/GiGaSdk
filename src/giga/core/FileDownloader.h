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

#ifndef GIGA_CORE_FILEDOWNLOADER_H_
#define GIGA_CORE_FILEDOWNLOADER_H_

#include "FileTransferer.h"

#include <boost/filesystem.hpp>
#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>
#include <chrono>

namespace giga
{
namespace core
{
class Node;


class FileDownloader : public FileTransferer
{
public:
    enum class Policy
    {
        override, overrideNewerSize, ignore, rename
    };

public:
    explicit
    FileDownloader (const boost::filesystem::path& folderDest, const Node& node, Policy policy = Policy::ignore);
    virtual ~FileDownloader();
    FileDownloader (FileDownloader&& other);

    FileDownloader(const FileDownloader&)            = delete;
    FileDownloader& operator=(const FileDownloader&) = delete;
    FileDownloader& operator=(FileDownloader&&)      = delete;

public:
    void doStart () override;

    const pplx::task<boost::filesystem::path>&
    task () const;

    boost::filesystem::path
    downloadingFile () const;

    boost::filesystem::path
    destinationFile () const;

    FileTransferer::Progress
    progress () const;

private:
    pplx::task<boost::filesystem::path> _task;

    boost::filesystem::path  _tempFile;
    boost::filesystem::path  _destFile;
    web::uri                 _fileUri;
    uint64_t                 _fileSize;
    uint64_t                 _startAt;
    std::chrono::system_clock::time_point _lastUpdateDate;
    Policy                  _policy;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILEDOWNLOADER_H_ */

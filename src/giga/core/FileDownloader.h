/*
 * FileDownloader.h
 *
 *  Created on: 9 févr. 2016
 *      Author: thomas
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
    uint64_t                _startAt;
    std::chrono::system_clock::time_point _lastUpdateDate;
    Policy                  _policy;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILEDOWNLOADER_H_ */

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

namespace giga
{
namespace core
{
class Node;


class FileDownloader : public FileTransferer
{
public:
    explicit
    FileDownloader (const std::string& folderDest, const Node& node, bool doContinue = false);
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

    double
    progress () const;

private:
    pplx::task<boost::filesystem::path> _task;

    boost::filesystem::path _tempFile;
    boost::filesystem::path _destFile;
    web::uri                _fileUri;
    uint64_t                _fileSize;
    uint64_t                _startAt;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILEDOWNLOADER_H_ */

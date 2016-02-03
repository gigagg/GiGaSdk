/*
 * FileUploader.h
 *
 *  Created on: 2 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_API_FILEUPLOADER_H_
#define GIGA_API_FILEUPLOADER_H_

#include <pplx/pplxtasks.h>

namespace giga
{
namespace data
{
class Node;
}

namespace api
{

class UploadState;


class FileUploader final
{
public:
    struct Progress
    {
        double preparation;
        double upload;
    };

    enum class State {
        pending, started, paused, canceled, finished
    };

public:
    explicit
    FileUploader (const std::string& filename, const std::string& nodeName, const std::string& parentId, const std::string& nodeKeyClear);
    ~FileUploader();

public:
    void
    start ();

    void
    pause ();

    void
    resume ();

    void
    cancel ();

    Progress
    progress () const;

    std::shared_ptr<data::Node>
    result () const;

private:
    State _state;
    std::shared_ptr<data::Node> _result;
    std::unique_ptr<UploadState> _uploadState;

    pplx::task<std::shared_ptr<data::Node>> _task;
    pplx::cancellation_token_source _cts;
};

} /* namespace api */
} /* namespace giga */

#endif /* GIGA_API_FILEUPLOADER_H_ */

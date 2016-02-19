/*
 * FileTransferer.h
 *
 *  Created on: 17 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_FILETRANSFERER_H_
#define GIGA_CORE_FILETRANSFERER_H_

#include "details/CurlProgress.h"

#include <pplx/pplxtasks.h>
#include <mutex>

namespace giga
{
namespace core
{

class FileTransferer
{
public:
    enum class State {
        pending, started, paused, canceled, finished
    };

public:
    explicit
    FileTransferer ();

    virtual ~FileTransferer();
    FileTransferer (FileTransferer&& other);

    FileTransferer(const FileTransferer&)             = delete;
    FileTransferer& operator=(const FileTransferer&)  = delete;
    FileTransferer& operator=(FileTransferer&&)       = delete;

public:
    void
    start ();

    void
    pause ();

    void
    resume ();

    void
    cancel ();

    State
    state () const;

    void
    limitRate (uint64_t rate);

protected:
    virtual void doStart() = 0;

protected:
    State                   _state;
    details::CurlProgress   _progress;
    mutable std::mutex      _mut;
    pplx::cancellation_token_source _cts;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILETRANSFERER_H_ */

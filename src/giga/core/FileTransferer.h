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

#ifndef GIGA_CORE_FILETRANSFERER_H_
#define GIGA_CORE_FILETRANSFERER_H_

#include <pplx/pplxtasks.h>
#include <mutex>

namespace giga
{
namespace details {
class CurlProgress;
}

namespace core
{


class FileTransferer
{
public:
    enum class State {
        pending, started, paused, canceled
    };
    struct Progress
    {
        double
        percent() const;

        const uint64_t transfered;
        const uint64_t size;
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
    State                                  _state;
    std::unique_ptr<details::CurlProgress> _progress;
    mutable std::mutex                     _mut;
    pplx::cancellation_token_source        _cts;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILETRANSFERER_H_ */

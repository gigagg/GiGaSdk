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

#include <boost/filesystem.hpp>
#include <cpprest/details/basic_types.h>
#include <pplx/pplxtasks.h>
#include <mutex>

namespace giga
{
namespace details {
class CurlProgress;
}

namespace core
{

/**
 * The FileTransferer can be a FileDownload or a FileUpload.
 */
class FileTransferer
{
public:
    enum class State {
        pending, started, paused, canceled, error
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
    FileTransferer (pplx::cancellation_token_source cts);

    virtual ~FileTransferer();
    FileTransferer (FileTransferer&& other);

    FileTransferer(const FileTransferer&)             = delete;
    FileTransferer& operator=(const FileTransferer&)  = delete;
    FileTransferer& operator=(FileTransferer&&)       = delete;

public:
    /**
     * @brief Start a transfer.
     */
    void
    start ();

    /**
     * @brief Pause the current transfer (see ```resume()```)
     */
    void
    pause ();

    /**
     * @brief Resume the current transfer (see ```pause()```)
     */
    void
    resume ();

    /**
     * @brief Cancel a started upload.
     * The task managing the transfer will most probably throw an exception.
     */
    void
    cancel ();

    void
    setError (const std::string& error);

    State
    state () const;

    /**
     * @brief Limit the rate of the transfer
     * @param rate in Byte/s
     */
    void
    limitRate (uint64_t rate);

    virtual FileTransferer::Progress
    progress () const = 0;

    virtual const boost::filesystem::path&
    filename() const = 0;

protected:
    virtual void doStart() = 0;

protected:
    State                                  _state;
    std::unique_ptr<details::CurlProgress> _progress;
    mutable std::mutex                     _mut;
    pplx::cancellation_token_source        _cts;
    utility::string_t                      _error;
};

} /* namespace core */
} /* namespace giga */

#endif /* GIGA_CORE_FILETRANSFERER_H_ */

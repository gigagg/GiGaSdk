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

#include "FileTransferer.h"
#include "../rest/HttpErrors.h"
#include "details/CurlProgress.h"

namespace giga
{
namespace core
{

double
FileTransferer::Progress::percent() const
{
    return ((double) (transfered * 100)) / (double) size;
}

FileTransferer::FileTransferer () :
        _state{State::pending}, _progress{new details::CurlProgress{}}, _mut{}, _cts{}
{
}

FileTransferer::~FileTransferer ()
{
}

FileTransferer::FileTransferer (FileTransferer&& other) :
                _state{std::move(other._state)},
                _progress{std::move(other._progress)},
                _mut{},
                _cts{std::move(other._cts)}
{
}

void
FileTransferer::start ()
{
    std::lock_guard<std::mutex> l{_mut};

    if (_state != State::pending) {
        BOOST_THROW_EXCEPTION(ErrorException{U("Start is valid only in 'pending' state")});
    }

    doStart();
    _state = State::started;
}

void
FileTransferer::pause ()
{
    std::lock_guard<std::mutex> l{_mut};
    if (_state != State::started) {
        BOOST_THROW_EXCEPTION(ErrorException{U("Pause is valid only in 'started' state")});
    }
    _progress->setPause(true);
    _state = State::paused;
}

void
FileTransferer::resume ()
{
    std::lock_guard<std::mutex> l{_mut};
    if (_state != State::paused) {
        BOOST_THROW_EXCEPTION(ErrorException{U("Resume is valid only in 'paused' state")});
    }
    _progress->setPause(false);
    _state = State::started;
}

void
FileTransferer::cancel ()
{
    std::lock_guard<std::mutex> l{_mut};
    if (_state != State::paused && _state != State::started) {
        BOOST_THROW_EXCEPTION(ErrorException{U("Resume is valid only in 'paused' and 'started' state")});
    }
    _state = State::canceled;
    _progress->cancel();
    _cts.cancel();
}

FileTransferer::State
FileTransferer::state () const
{
    std::lock_guard<std::mutex> l{_mut};
    return _state;
}

void
FileTransferer::limitRate (uint64_t rate)
{
    _progress->setLimitRate(rate);
}

} /* namespace core */
} /* namespace giga */

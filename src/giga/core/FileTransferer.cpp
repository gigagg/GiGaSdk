/*
 * FileTransferer.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: thomas
 */

#include "FileTransferer.h"
#include "../rest/HttpErrors.h"
#include "details/CurlProgress.h"

namespace giga
{
namespace core
{

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
        THROW(ErrorException{"Start is valid only in 'pending' state"});
    }

    doStart();
    _state = State::started;
}

void
FileTransferer::pause ()
{
    std::lock_guard<std::mutex> l{_mut};
    if (_state != State::started) {
        THROW(ErrorException{"Pause is valid only in 'started' state"});
    }
    _progress->setPause(true);
    _state = State::paused;
}

void
FileTransferer::resume ()
{
    std::lock_guard<std::mutex> l{_mut};
    if (_state != State::paused) {
        THROW(ErrorException{"Resume is valid only in 'paused' state"});
    }
    _progress->setPause(false);
    _state = State::started;
}

void
FileTransferer::cancel ()
{
    std::lock_guard<std::mutex> l{_mut};
    if (_state != State::paused && _state != State::started) {
        THROW(ErrorException{"Resume is valid only in 'paused' and 'started' state"});
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

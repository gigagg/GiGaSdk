/*
 * CurlProgress.cpp
 *
 *  Created on: 17 févr. 2016
 *      Author: thomas
 */

#include "CurlProgress.h"
#include "../../rest/HttpErrors.h"

#include <curl_easy.h>
#include <mutex>
#include <curl_exception.h>
#include <chrono>
#include <thread>

using curl::curl_easy_exception;
using namespace std::chrono;

namespace giga
{
namespace details
{

CurlProgress::CurlProgress () :
        _item{-1L, -1L, -1L, -1L}, _cancel{false}, _pause{false}, _isPaused{false}, _curl{nullptr},
        _limitRate{0}, _currentLimitRate{0}, _rateTime{}, _rateBytes{0}, _bucket{0}, _upPostion{0}
{
}

CurlProgress::Item
CurlProgress::data () const
{
    std::lock_guard<std::mutex> l(_mut);
    return _item;
}

void
CurlProgress::setPause (bool pause)
{
    std::lock_guard<std::mutex> l(_mut);
    _pause = pause;
}

void
CurlProgress::setUploadPosition (uint64_t pos)
{
    std::lock_guard<std::mutex> l(_mut);
    _upPostion = pos;
}

void
CurlProgress::setLimitRate (uint64_t rate)
{
    std::lock_guard<std::mutex> l(_mut);
    _limitRate = rate;
}


void
CurlProgress::cancel ()
{
    std::lock_guard<std::mutex> l(_mut);
    _cancel = true;
}

void
CurlProgress::setCurl (curl::curl_easy& curl)
{
    std::lock_guard<std::mutex> l(_mut);
    _curl = &curl;
}

int
CurlProgress::onCallback (long dltotal, long dlnow, long ultotal, long ulnow) noexcept
{
    try {
        uint64_t limitRate = 0;
        {
            std::lock_guard<std::mutex> l(_mut);
            limitRate = _limitRate;

            _item.dltotal = dltotal;
            _item.dlnow = dlnow;
            _item.ultotal = ultotal + _upPostion;
            _item.ulnow = ulnow + _upPostion;

            if (_pause != _isPaused)
            {
                _curl->pause(_pause ? CURLPAUSE_ALL : CURLPAUSE_CONT);
                _isPaused = _pause;
            }
            if (_cancel)
            {
                return CURLE_ABORTED_BY_CALLBACK;
            }
        }

        // Do the limit rate outside of the mutex locked zone
        // because there is waiting here.
        auto transfered = dlnow + ulnow;
        if (limitRate != _currentLimitRate)
        {
            _rateBytes = transfered;
            _currentLimitRate = limitRate;
            _rateTime = high_resolution_clock::now();
            _bucket = _limitRate; // give it 1 sec
        }
        if (limitRate > 0)
        {
            auto now = high_resolution_clock::now();
            do
            {
                // calculate duration
                duration<float> duration = now - _rateTime;
                auto elapsedMs = duration_cast<milliseconds>(duration).count();

                // add bytes in the bucket
                _bucket += static_cast<uint64_t>((elapsedMs * _limitRate) / 1000);

                // take bytes in the bucket
                auto take = std::min(_bucket, transfered - _rateBytes);
                _bucket    -= take;
                _rateBytes += take;

                // wait for new bytes.
                if (_bucket == 0 && (transfered - _rateBytes) > 0)
                {
                    std::this_thread::sleep_for(milliseconds(500));
                    now = high_resolution_clock::now();
                }
            } while (_bucket == 0 && (dlnow - _rateBytes) > 0);

            _rateTime = now;
        }
    } catch (...) {
        return CURLE_OBSOLETE40;
    }
    return CURLE_OK;
}

} /* namespace details */
} /* namespace giga */

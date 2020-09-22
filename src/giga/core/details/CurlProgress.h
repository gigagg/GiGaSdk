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

#ifndef GIGA_CORE_DETAILS_CURLPROGRESS_H_
#define GIGA_CORE_DETAILS_CURLPROGRESS_H_

#include <chrono>
#include <mutex>
#include <curl/system.h>
#include <pplx/pplxtasks.h>

namespace curl
{
class curl_easy;
}

namespace giga
{
namespace details
{

class CurlProgress
{
public:
    struct Item
    {
        uint64_t dltotal;
        uint64_t dlnow;
        uint64_t ultotal;
        uint64_t ulnow;
    };

public:
    explicit
    CurlProgress(pplx::cancellation_token token);
    CurlProgress(const CurlProgress& rhs);

    CurlProgress(CurlProgress&&)                 = delete;
    CurlProgress& operator=(CurlProgress&&)      = delete;
    CurlProgress& operator=(const CurlProgress&) = delete;

    int
    onCallback (curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) noexcept;

    Item
    data () const;

    void
    setPause (bool pause);

    void
    setUploadPosition (uint64_t position);

    /**
     * limit the dl/up rate.
     * @param rate in Bytes/s. O for no limit.
     */
    void
    setLimitRate (uint64_t rate);

    void
    setCurl (curl::curl_easy& curl);

    bool
    isPaused () const;

    bool
    isCanceled() const;

private:
    mutable std::mutex       _mut;
    Item                     _item;
    pplx::cancellation_token _cancelToken;
    bool                     _pause;
    bool                     _isPaused;
    curl::curl_easy*         _curl;

    typedef std::chrono::high_resolution_clock::time_point Time;
    uint64_t   _limitRate;
    uint64_t   _currentLimitRate;
    Time       _rateTime;
    uint64_t   _rateBytes;
    uint64_t   _bucket;

    uint64_t   _upPostion;

};

} /* namespace details */
} /* namespace giga */

#endif /* GIGA_CORE_DETAILS_CURLPROGRESS_H_ */

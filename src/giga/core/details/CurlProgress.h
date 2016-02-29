/*
 * CurlProgress.h
 *
 *  Created on: 17 févr. 2016
 *      Author: thomas
 */

#ifndef GIGA_CORE_DETAILS_CURLPROGRESS_H_
#define GIGA_CORE_DETAILS_CURLPROGRESS_H_

#include <chrono>
#include <mutex>

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
    CurlProgress ();
    CurlProgress (const CurlProgress& other);

    int
    onCallback (long dltotal, long dlnow, long ultotal, long ulnow) noexcept;

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
    cancel ();

    void
    setCurl (curl::curl_easy& curl);

private:
    mutable std::mutex  _mut;
    Item                _item;
    bool                _cancel;
    bool                _pause;
    bool                _isPaused;
    curl::curl_easy*    _curl;

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

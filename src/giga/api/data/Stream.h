/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_STREAM_MODEL_H_
#define GIGA_DATA_STREAM_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"


namespace giga {
namespace data {

struct Stream {
    boost::optional<utility::string_t> mimeType = boost::none;
    /** ex: '720p' */
    boost::optional<utility::string_t> size = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<utility::string_t> url = boost::none;
    
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, mimeType);
        GIGA_MANAGE(m, size);
        GIGA_MANAGE(m, url);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_STREAM_MODEL_H_ */

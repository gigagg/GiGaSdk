/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_SUBTITLE_MODEL_H_
#define GIGA_DATA_SUBTITLE_MODEL_H_

#include <cpprest/details/basic_types.h>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

struct Subtitle {
    /** ex: 'eng' */
    boost::optional<utility::string_t> lang = boost::none;
    /** 'vtt' or 'srt' */
    boost::optional<utility::string_t> type = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    utility::string_t url = {};
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, lang);
        GIGA_MANAGE(m, type);
        GIGA_MANAGE(m, url);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_SUBTITLE_MODEL_H_ */

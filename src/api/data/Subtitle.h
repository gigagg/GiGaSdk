/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_SUBTITLE_MODEL_H_
#define GIGA_DATA_SUBTITLE_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

namespace giga {
namespace data {

struct Subtitle {
    /** ex: 'eng' */
    boost::optional<std::string> lang = boost::none;
    /** 'vtt' or 'srt' */
    boost::optional<std::string> type = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    std::string url = {};
    
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

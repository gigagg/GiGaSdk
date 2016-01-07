/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_PREVIEWCOVER_MODEL_H_
#define GIGA_DATA_PREVIEWCOVER_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "prepoc_manage.h"

namespace giga {
namespace data {

/**
 * Some images
 */
struct PreviewCover {
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<std::string> poster = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<std::string> square = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<std::string> original = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, poster);
        GIGA_MANAGE(m, square);
        GIGA_MANAGE(m, original);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_PREVIEWCOVER_MODEL_H_ */

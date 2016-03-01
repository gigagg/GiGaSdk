/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_PREVIEW_MODEL_H_
#define GIGA_DATA_PREVIEW_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

#include "Subtitle.h"
#include "Stream.h"
#include "PreviewCover.h"

namespace giga {
namespace data {

/**
 * Describe which preview are available for this file
 */
struct Preview {
    /** video, pdf, image, images, or audio */
    boost::optional<utility::string_t> type = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<utility::string_t> icon = boost::none;
    std::unique_ptr<PreviewCover> cover = nullptr;
    std::vector<std::unique_ptr<Stream>> streams = {};
    std::vector<std::unique_ptr<Subtitle>> subtitles = {};
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<utility::string_t> albumart = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<utility::string_t> image = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<utility::string_t> pdf = boost::none;
    /** Concat the url encoded nodeKey at the end of this url */
    boost::optional<utility::string_t> epub = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, type);
        GIGA_MANAGE(m, icon);
        GIGA_MANAGE(m, cover);
        GIGA_MANAGE(m, streams);
        GIGA_MANAGE(m, subtitles);
        GIGA_MANAGE(m, albumart);
        GIGA_MANAGE(m, image);
        GIGA_MANAGE(m, pdf);
        GIGA_MANAGE(m, epub);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_PREVIEW_MODEL_H_ */

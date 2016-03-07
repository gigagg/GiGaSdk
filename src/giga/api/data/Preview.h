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

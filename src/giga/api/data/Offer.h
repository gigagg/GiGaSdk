/**
 * @author Thomas Guyard <t.guyard@gigatribe.com>
 */

#ifndef GIGA_DATA_OFFER_MODEL_H_
#define GIGA_DATA_OFFER_MODEL_H_

#include <string>
#include <vector>
#include <map>
#include "../../rest/prepoc_manage.h"

namespace giga {
namespace data {

/**
 * Something you can buy
 */
struct Offer {
    std::string id          = {};
    std::string state       = {};
    std::string duration    = {};
    int64_t storage         = 0;
    int64_t download        = 0;
    std::string resetDownload = {};
    std::string billing     = {};
    int64_t costEUR         = 0;
    int64_t costUSD         = 0;
    boost::optional<int64_t> costStar = boost::none;
    
    template <class Manager>
    void visit(const Manager& m) {
        GIGA_MANAGE(m, id);
        GIGA_MANAGE(m, state);
        GIGA_MANAGE(m, duration);
        GIGA_MANAGE(m, storage);
        GIGA_MANAGE(m, download);
        GIGA_MANAGE(m, resetDownload);
        GIGA_MANAGE(m, billing);
        GIGA_MANAGE(m, costEUR);
        GIGA_MANAGE(m, costUSD);
        GIGA_MANAGE(m, costStar);
    }
};

} /* namespace data */
} /* namespace giga */
#endif /* GIGA_DATA_OFFER_MODEL_H_ */

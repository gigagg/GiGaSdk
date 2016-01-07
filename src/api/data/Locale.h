/*
 * Locale.h
 *
 *  Created on: 30 sept. 2015
 *      Author: thomas
 */

#ifndef LOCALE_H_
#define LOCALE_H_

#include <boost/optional.hpp>
#include <string.h>
#include "prepoc_manage.h"

struct Locale {
    std::string contryCode  = "FR";
    std::string countryName = {};
    std::string currency    = {};
    boost::optional<std::string> ip = boost::none;

    template <class Unserializer>
    void visit(const Unserializer& us){
        us.manageOpt(contryCode, std::string("contryCode"), std::string("FR"));
        us.manageOpt(countryName, std::string("countryName"), std::string("France"));
        us.manage(currency, std::string("currency"));
        us.manage(ip, std::string("ip"));
    }
};

#endif /* LOCALE_H_ */

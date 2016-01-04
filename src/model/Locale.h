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
    std::string contryCode = "FR";
    std::string countryName= "";
    std::string currency = "";
    boost::optional<std::string> ip = boost::none;

    template <class Unserializer>
    void visit(const Unserializer& us){
        us.manageOpt(contryCode, std::string("contryCode"), std::string("FR"));
        us.manageOpt(countryName, std::string("countryName"), std::string("France"));
        us.manage(currency, std::string("currency"));
        us.manage(ip, std::string("ip"));
    }
};

struct A {
    int a = 0;
    template <class Unserializer>
    void visit(const Unserializer& us){
        us.manage(a, "a");
    }
};

struct Test {
    bool tbool                  = false;
    int tint                    = 0;
    double tdouble              = 0.;
    std::string tstring         = "";
    std::vector<int> tarray     = {};
    std::unique_ptr<A> tobject  = nullptr;
    boost::optional<A> tobjectOpt1 = boost::none;
    boost::optional<A> tobjectOpt2 = boost::none;
    boost::optional<A> unexisting  = boost::none;
    std::vector<std::unique_ptr<A>> tobjectArray = {};

    template <class Unserializer>
    void visit(const Unserializer& us){
        GIGA_MANAGE(us, tbool);
        GIGA_MANAGE(us, tint);
        GIGA_MANAGE(us, tdouble);
        GIGA_MANAGE(us, tstring);
        GIGA_MANAGE(us, tarray);
        GIGA_MANAGE(us, tobject);
        GIGA_MANAGE(us, tobjectOpt1);
        GIGA_MANAGE(us, tobjectOpt2);
        GIGA_MANAGE(us, unexisting);
        GIGA_MANAGE(us, tobjectArray);
    }
};


#endif /* LOCALE_H_ */

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

#ifndef LOCALE_H_
#define LOCALE_H_

#include <boost/optional.hpp>
#include <cpprest/details/basic_types.h>
#include "../../rest/prepoc_manage.h"

struct Locale {
    utility::string_t contryCode  = U("FR");
    utility::string_t countryName = {};
    utility::string_t currency    = {};
    boost::optional<utility::string_t> ip = boost::none;

    template <class Unserializer>
    void visit(const Unserializer& us){
        us.manageOpt(contryCode, utility::string_t(U("contryCode")), utility::string_t(U("FR")));
        us.manageOpt(countryName, utility::string_t(U("countryName")), utility::string_t(U("France")));
        us.manage(currency, utility::string_t(U("currency")));
        us.manage(ip, utility::string_t(U("ip")));
    }
};

#endif /* LOCALE_H_ */

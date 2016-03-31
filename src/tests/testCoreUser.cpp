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
#define BOOST_TEST_MODULE core

#include <boost/test/included/unit_test.hpp>
#include <giga/Application.h>
#include <giga/core/User.h>
#include <giga/api/data/User.h>
#include <giga/utils/Utils.h>
#include <giga/rest/JsonUnserializer.h>
#include <cpprest/details/basic_types.h>
#include <chrono>

using std::chrono::system_clock;
using std::chrono::time_point;
using namespace boost::unit_test;
using namespace giga;
using namespace giga::core;

BOOST_AUTO_TEST_CASE(test_core_user_simple) {
    auto json = giga::utils::str2wstr(R"({
      "activity" : "INACTIVE",
      "adultStatus" : 0,
      "avatarUrl" : "https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=81&r=pg",
      "bigAvatarUrl" : "https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=215&r=pg",
      "contactCount" : 16,
      "country" : "XX",
      "creationDate" : 1426154892,
      "dlAvailable" : 0,
      "email" : "t.guyard@gigatribe.com",
      "hasBeenEnticed" : false,
      "id" : 1,
      "isHiddingFirstSteps" : true,
      "isSeeder" : "YES",
      "language" : "en",
      "lastConnectionDate" : 1426154898,
      "login" : "t.guyard",
      "maxContact" : 200,
      "tags" : [ "test2", "plop", "adults", "plip", "plup" ]
    })");

    auto dataUser = std::make_shared<data::User>(JSonUnserializer::fromString<data::User>(json));
    Application app;
    auto user = User{dataUser, app};

    BOOST_CHECK_EQUAL(1, user.id());
    BOOST_CHECK(User::Activity::inactive == user.activity());
//    BOOST_CHECK_EQUAL(U("https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=81&r=pg"), user.avatarUri().to_string());
//    BOOST_CHECK_EQUAL(U("https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=215&r=pg"), user.bigAvatarUri().to_string());
    BOOST_CHECK_EQUAL(16, user.contactCount());

    auto creationDate = system_clock::time_point(std::chrono::seconds(1426154892));
    BOOST_CHECK(creationDate == user.creationDate());

    auto lastConnectionDate = system_clock::time_point(std::chrono::seconds(1426154898));
    BOOST_CHECK(lastConnectionDate == user.lastConnectionDate());

//    BOOST_CHECK_EQUAL(U("t.guyard"), user.login());
    BOOST_CHECK(!user.hasPersonalData());
    BOOST_CHECK(!user.hasContactData());
}

BOOST_AUTO_TEST_CASE(test_core_user_protected) {
    auto json = giga::utils::str2wstr(R"({
        "id" : 1704770,
        "login" : "bot-GiGa",
        "creationDate" : 1416502505,
        "lastConnectionDate" : 1424473200,
        "activity" : "VERY_ACTIVE",
        "country" : "FR",
        "language" : "en",
        "description" : "Bonjour, \nJe suis un robot g\u00c3\u00a9r\u00c3\u00a9 par GiGa.",
        "avatarState" : "UNDEFINED",
        "avatarUrl" : "https:\/\/www.gravatar.com\/avatar\/76c6d24b1e7b6fb88cf2dfa39e218449?d=wavatar\u0026s=81\u0026r=pg",
        "bigAvatarUrl" : "https:\/\/www.gravatar.com\/avatar\/76c6d24b1e7b6fb88cf2dfa39e218449?d=wavatar\u0026s=215\u0026r=pg",
        "contactCount" : 2420,
        "maxContact" : 200,
        "tags" : [],
        "node" : {
          "uploadUrl" : "\/upload\/?s=1704770",
          "moduleId" : [],
          "name" : "All my files",
          "ancestors" : [],
          "ownerId" : 1704770,
          "size" : 0,
          "creationDate" : 1453220703,
          "lastUpdateDate" : 0,
          "id" : "1704770",
          "nbChildren" : 0,
          "nbFiles" : 0,
          "nodes" : [],
          "type" : "root"
        },
        "isSeeder" : "NO",
        "dlAvailable" : 2147483648,
        "isUnlimited" : false,
        "isBot" : true
      })");

    auto dataUser = std::make_shared<data::User>(JSonUnserializer::fromString<data::User>(json));
    Application app;
    auto user = User{dataUser, app};

    BOOST_CHECK_EQUAL(1704770, user.id());
    BOOST_CHECK(User::Activity::very_active == user.activity());
    BOOST_CHECK_EQUAL(2420, user.contactCount());
//    BOOST_CHECK_EQUAL(U("bot-GiGa"), user.login());
    BOOST_CHECK(!user.hasPersonalData());
    BOOST_CHECK(user.hasContactData());

    BOOST_CHECK(user.contactData().gender() == User::UserGender::unknown);
    BOOST_CHECK_EQUAL(user.contactData().maxContact(), 200);
}

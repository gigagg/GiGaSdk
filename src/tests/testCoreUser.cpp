#include <chrono>


using std::chrono::_V2::system_clock;
using std::chrono::time_point;
#define BOOST_TEST_MODULE core

#include <boost/test/included/unit_test.hpp>
#include <giga/core/User.h>
#include <giga/api/data/User.h>
#include <giga/rest/JsonUnserializer.h>


using namespace boost::unit_test;
using namespace giga;
using namespace giga::core;

BOOST_AUTO_TEST_CASE(test_core_user_simple) {
    auto json = R"({
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
    })";

    auto dataUser = std::make_shared<data::User>(JSonUnserializer::fromString<data::User>(json));
    auto user = User{dataUser};

    BOOST_CHECK_EQUAL(1, user.id());
    BOOST_CHECK(User::Activity::inactive == user.activity());
    BOOST_CHECK_EQUAL("https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=81&r=pg", user.avatarUri().to_string());
    BOOST_CHECK_EQUAL("https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=215&r=pg", user.bigAvatarUri().to_string());
    BOOST_CHECK_EQUAL(16, user.contactCount());

    auto creationDate = system_clock::time_point(std::chrono::seconds(1426154892));
    BOOST_CHECK(creationDate == user.creationDate());

    auto lastConnectionDate = system_clock::time_point(std::chrono::seconds(1426154898));
    BOOST_CHECK(lastConnectionDate == user.lastConnectionDate());

    BOOST_CHECK_EQUAL("t.guyard", user.login());
    BOOST_CHECK(!user.hasPrivateData());
    BOOST_CHECK(!user.hasProtectedData());
}

BOOST_AUTO_TEST_CASE(test_core_user_protected) {
    auto json = R"({
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
      })";

    auto dataUser = std::make_shared<data::User>(JSonUnserializer::fromString<data::User>(json));
    auto user = User{dataUser};

    BOOST_CHECK_EQUAL(1704770, user.id());
    BOOST_CHECK(User::Activity::very_active == user.activity());
    BOOST_CHECK_EQUAL(2420, user.contactCount());
    BOOST_CHECK_EQUAL("bot-GiGa", user.login());
    BOOST_CHECK(!user.hasPrivateData());
    BOOST_CHECK(user.hasProtectedData());

    BOOST_CHECK(user.protectedData().gender() == User::UserGender::unknown);
    BOOST_CHECK_EQUAL(user.protectedData().maxContact(), 200);
}
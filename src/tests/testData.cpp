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
#define BOOST_TEST_MODULE data
#include <boost/test/included/unit_test.hpp>
#include <giga/api/data/Node.h>
#include <giga/api/data/User.h>
#include <giga/utils/Utils.h>
#include <giga/rest/JsonUnserializer.h>
#include <cpprest/details/basic_types.h>

using namespace boost::unit_test;
using namespace giga;
using namespace giga::data;

BOOST_AUTO_TEST_CASE(test_data_user) {
    auto json = giga::utils::str2wstr(R"({
  "achievements" : 476,
  "activity" : "INACTIVE",
  "adultStatus" : 0,
  "avatarState" : "UNDEFINED",
  "avatarUrl" : "https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=81&r=pg",
  "bigAvatarUrl" : "https://www.gravatar.com/avatar/e9c0238fd06779c6486e41ac18d542b3?d=wavatar&s=215&r=pg",
  "contactCount" : 16,
  "country" : "XX",
  "creationDate" : 1426154892,
  "dlAvailable" : 0,
  "email" : "t.guyard@gigatribe.com",
  "externalAccounts" : [ {
    "avatarUrl" : "https://cloud03.dev.gg/preview/561e575533e5dffa008b4567/image?k=2cPc94ZWhbT_25N4ECoDT6HalOsBR7f-dVL5TzOLdfU%3D",
    "creationDate" : 1446465304,
    "displayName" : "t.guyard",
    "email" : "t.guyard@gigatribe.com",
    "externalId" : "t.guyard@gigatribe.com",
    "login" : "t.guyard",
    "providerName" : "EMAIL",
    "updateDate" : 1442326082,
    "userId" : 1
  } ],
  "hasBeenEnticed" : false,
  "id" : 1,
  "isHiddingFirstSteps" : true,
  "isSeeder" : "YES",
  "language" : "en",
  "lastConnectionDate" : 1426154892,
  "login" : "t.guyard",
  "maxContact" : 200,
  "maxStorage" : 10240,
  "modules" : [],
  "nextEmail" : "t.guyard@gigatribe.com",
  "nodeKey" : "VkAEFGC4OzGoQh74QOThIGhGUN7kiaXcbEHK8EtTsdMcmktR1qb8l20d3rFF1IDROHfQwmpe/jM9fqIFfTdl2+6IYqmzZVCF8KhFcaGmathpNN3Ajs5ulwHqbdbXSBCHrz6avTnUNysaOodW5Ld/rMl7Uqdso2VSVeY+AJvmEZI=",
  "offer" : {
    "billing" : "NEVER",
    "costEUR" : 0,
    "costGBP" : 0,
    "costStar" : 0,
    "costUSD" : 0,
    "download" : 0,
    "duration" : "TWO_WEEKS",
    "expire" : 1477128263,
    "id" : "UNLIMITED_2WEEKS_FREE",
    "isApple" : false,
    "isSubscription" : false,
    "resetDownload" : "NEVER",
    "state" : "GIFT",
    "storage" : 10240
  },
  "rsaKeys" : {
    "aesIv" : "6pb4g/omiiiBZLs6IQgyMw==",
    "aesSalt" : "oMAlsjc9ejk=",
    "privateKey" : "mkhR345BrvSAo91wFQOvHYBtYET3cGf48L0l8XY6vbhXKSCq2Sl1OKMoMeFRadk70jo5gJh61OXO4gH8nOQAAUUmUVKg0GycvCH6WtbUljGwS9WfFWFcs/TJjn6MKlpgoBGyx3NJtx48Dd51xL36Xlu9zh6XOuAGH2tE7BrBdL+JBdUcFE2m6BI+mXG+pdxtnCGQRJn3xcycq8AjwJtdALfEdWm6p28rPsladSiH9oSYBqmb4otqjqBkS+61Gi7f8E50GoscMIxgHNfl6JckbW7b4/vY2Lz/QEUYQqXvein3tEUu5IjTCORbpBE1mUmj8/0h9QScQ0ek4kzgbJIrJccG4ocyXIglcmagLmZM6GLAmzh6jn2DPOw9sFVCwhgeLOLqwKzx+WMcBUAYToyBAMIfPfBJ5NjnWboFvVceYZRP902n2YlRpEG/6uXpmxk76Av8Jcl4Qr11Ay4+9JAMgvCq/vDOgGM+frzsXS2uPOQkVtlx6qzrrv1iLuHrVNg3hZ96Ir8f1uUsbe3nyZnKBrPJgi7rOp4FLjqNyQ9/kmk9KUP2HBuHMhbKSRLbwbpl7V+NZxaFS1gWdkg0eZR+NbX9sIhoLENOEotRjlNY3+asg3Eq8iKuzPHHQ79N9nUBSdE7T92EdYqznAMPOKMZDIcwls1YtLFAq0LKgvx1CypnC//e8n8cU3ARmqfZ2aTbrJIpgIN60sFrYSRJsfVlsLq1GWk4J89ECuxFXd05Kdy5MFrIOtCYuHJFcal7geeKdNt5GCnMrgSKAVkFKH0+EO9JQU/nk8fOtw/mP3iXcvGLOcmqqMFvcIGYsif3exQ0ZsRhnGUfSm10n6eClHyzx2XnPyQYrs6c4PzKiWUHGjoxXvN/0hxihhZcWn46wdEwXyeZ3ewH8+ePn+wVvLt7F2mqWwI+Wsfa3CkFZX8R580E29tJ4f/0OFnQ9k/JvYV6US6NoTQ12M7xf7/ptruySnvfLncMx2wx0sRVPzVuPvY3q1YT7oTGt9nSpq0Ue4bMj/iY1hWb22cmLYHwM/21Dlg/Yco/Qv2bxhZwwCcMLgizfqndiF4V5aIc8nTMVi91",
    "publicKey" : "MIGeMA0GCSqGSIb3DQEBAQUAA4GMADCBiAKBgHu9dAzcWviz6LqHtToLfhW45OqrJGcm9MUfQZ9rDqQdAyKCVZqkdJqjY713OVzDnPARemRLe9OZ6pB+Si4RQf7Nhm0FiwrP4lLfgZI3MCyqg3pBzfJI9G9BUp2l5H93kxjT0CFG2vpX8x8c5n81AWuLsCt+eOvqxH8THr1uWq1fAgMBAAE="
  },
  "salt" : "0JtCHg2s3LAyqm4SCUOCdAdAQj0iM/f9C0LbJCTub94=",
  "standardMaxStorage" : 475,
  "starCount" : 750,
  "tags" : [ "test2", "plop", "adults", "plip", "plup" ]
})");
    auto user = JSonUnserializer::fromString<User>(json);
}

BOOST_AUTO_TEST_CASE(test_data_node)
{
    auto json = giga::utils::str2wstr(R"({
    "uploadUrl" : "\/upload\/?s=561cc82833e5dfb5008b4567",
    "name" : "All my files",
    "ancestors" : [],
    "ownerId" : 1,
    "size" : 6248637323,
    "creationDate" : 1444726824,
    "lastUpdateDate" : 1452174320,
    "id" : "561cc82833e5dfb5008b4567",
    "nbChildren" : 2,
    "nbFiles" : 234,
    "nodes" : [ {
      "mimeType" : "video\/x-matroska",
      "media" : "video",
      "fid" : "zjIENuYyf27JN7xxil6M0cc1",
      "fkey" : "z1DwH5cOZirP3vbtj2QsD4vGd3PyN+ldYe9U02GIoN4=",
      "previewState" : 7,
      "url" : "\/\/cloud03.dev.gg\/download\/565dc08533e5dfa8008b4568\/testFile?k=",
      "icon" : "\/\/cloud03.dev.gg\/preview\/565dc08533e5dfa8008b4568\/icon?k=",
      "square" : "\/\/cloud03.dev.gg\/preview\/565dc08533e5dfa8008b4568\/square?",
      "original" : "\/\/cloud03.dev.gg\/preview\/565dc08533e5dfa8008b4568\/original?",
      "poster" : "\/\/cloud03.dev.gg\/preview\/565dc08533e5dfa8008b4568\/poster?",
      "servers" : [ "03" ],
      "name" : "testFile",
      "parentId" : "561cc82833e5dfb5008b4567",
      "shareId" : "565dc08533e5dfa8008b4568",
      "ancestors" : [ "561cc82833e5dfb5008b4567" ],
      "ownerId" : 1,
      "size" : 728464990,
      "creationDate" : 1448984709,
      "lastUpdateDate" : 1448984709,
      "id" : "565dc08533e5dfa8008b4568",
      "nbChildren" : 0,
      "nbFiles" : 0,
      "nodes" : [],
      "type" : "file"
    }, {
      "uploadUrl" : "\/upload\/?s=568e5e7233e5df3b008b4567",
      "name" : "images",
      "parentId" : "561cc82833e5dfb5008b4567",
      "ancestors" : [ "561cc82833e5dfb5008b4567" ],
      "ownerId" : 1,
      "size" : 0,
      "creationDate" : 1452170866,
      "lastUpdateDate" : 1452170867,
      "id" : "568e5e7233e5df3b008b4567",
      "nbChildren" : 12,
      "nbFiles" : 233,
      "nodes" : [],
      "type" : "folder"
    } ],
    "type" : "root"
  })");

    auto node = JSonUnserializer::fromString<Node>(json);
    BOOST_CHECK(U("root") == node.type);
    BOOST_CHECK(0 == node.ancestors.size());
    BOOST_CHECK_EQUAL(1444726824, node.creationDate);
    BOOST_CHECK(boost::none == node.fid);
    BOOST_CHECK(boost::none == node.fkey);
    BOOST_CHECK(boost::none == node.icon);
    BOOST_CHECK("561cc82833e5dfb5008b4567" == node.id);
    BOOST_CHECK_EQUAL(1452174320, node.lastUpdateDate);
    BOOST_CHECK(boost::none == node.mimeType);
    BOOST_CHECK(U("All my files") == node.name);
    BOOST_CHECK_EQUAL(2, node.nbChildren);
    BOOST_CHECK_EQUAL(234, node.nbFiles);
    BOOST_CHECK(2 == node.nodes.size());
    BOOST_CHECK(boost::none == node.original);
    BOOST_CHECK_EQUAL(1, node.ownerId);
    BOOST_CHECK(boost::none == node.parentId);
    BOOST_CHECK(boost::none == node.poster);
    BOOST_CHECK(boost::none == node.previewState);
    BOOST_CHECK_EQUAL(6248637323ul, node.size);
    BOOST_CHECK(boost::none == node.square);
    BOOST_CHECK(boost::none == node.url);

    auto& file = node.nodes[0];
    BOOST_CHECK(U("file") == file->type);
    BOOST_CHECK(1 == file->ancestors.size());
    BOOST_CHECK_EQUAL(1448984709, file->creationDate);
    BOOST_CHECK("zjIENuYyf27JN7xxil6M0cc1" == file->fid.get());
    BOOST_CHECK("z1DwH5cOZirP3vbtj2QsD4vGd3PyN+ldYe9U02GIoN4=" == file->fkey.get());
    BOOST_CHECK(U("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/icon?k=") == file->icon.get());
    BOOST_CHECK("565dc08533e5dfa8008b4568" == file->id);
    BOOST_CHECK_EQUAL(1448984709, file->lastUpdateDate);
    BOOST_CHECK(U("video/x-matroska") == file->mimeType.get());
    BOOST_CHECK(U("testFile") == file->name);
    BOOST_CHECK_EQUAL(0, file->nbChildren);
    BOOST_CHECK_EQUAL(0, file->nbFiles);
    BOOST_CHECK(0 == file->nodes.size());
    BOOST_CHECK(U("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/original?") == file->original.get());
    BOOST_CHECK_EQUAL(1, file->ownerId);
    BOOST_CHECK("561cc82833e5dfb5008b4567" == file->parentId.get());
    BOOST_CHECK(U("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/poster?") == file->poster.get());
    BOOST_CHECK_EQUAL(7, file->previewState.get());
    BOOST_CHECK_EQUAL(728464990, file->size);
    BOOST_CHECK(1 == file->servers.size());
    BOOST_CHECK("03" == file->servers[0]);
    BOOST_CHECK(U("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/square?") == file->square.get());
    BOOST_CHECK(U("//cloud03.dev.gg/download/565dc08533e5dfa8008b4568/testFile?k=") == file->url.get());

    auto& folder = node.nodes[1];
    BOOST_CHECK(U("folder") == folder->type);
    BOOST_CHECK(1 == folder->ancestors.size());
    BOOST_CHECK_EQUAL(1452170866, folder->creationDate);
    BOOST_CHECK(boost::none == folder->fid);
    BOOST_CHECK(boost::none == folder->fkey);
    BOOST_CHECK(boost::none == folder->icon);
    BOOST_CHECK("568e5e7233e5df3b008b4567" == folder->id);
    BOOST_CHECK_EQUAL(1452170867, folder->lastUpdateDate);
    BOOST_CHECK(boost::none == folder->mimeType);
    BOOST_CHECK(U("images") == folder->name);
    BOOST_CHECK_EQUAL(12, folder->nbChildren);
    BOOST_CHECK_EQUAL(233, folder->nbFiles);
    BOOST_CHECK(0 == folder->nodes.size());
    BOOST_CHECK(boost::none == folder->original);
    BOOST_CHECK_EQUAL(1, folder->ownerId);
    BOOST_CHECK("561cc82833e5dfb5008b4567" == folder->parentId.get());
    BOOST_CHECK(boost::none == folder->poster);
    BOOST_CHECK(boost::none == folder->previewState);
    BOOST_CHECK(0 == folder->size);
    BOOST_CHECK(boost::none == folder->square);
    BOOST_CHECK(boost::none == folder->url);
}

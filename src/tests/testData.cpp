#define BOOST_TEST_MODULE data
#include <boost/test/included/unit_test.hpp>

#include <giga/api/data/Node.h>
#include <giga/rest/JsonUnserializer.h>


using namespace boost::unit_test;
using namespace giga;
using namespace giga::data;

BOOST_AUTO_TEST_CASE(test_data_node)
{
    auto json = R"({
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
  })";

    auto node = JSonUnserializer::fromString<Node>(json);
    BOOST_CHECK_EQUAL("root", node.type);
    BOOST_CHECK_EQUAL(0, node.ancestors.size());
    BOOST_CHECK_EQUAL(1444726824, node.creationDate);
    BOOST_CHECK_EQUAL(boost::none, node.fid);
    BOOST_CHECK_EQUAL(boost::none, node.fkey);
    BOOST_CHECK_EQUAL(boost::none, node.icon);
    BOOST_CHECK_EQUAL("561cc82833e5dfb5008b4567", node.id);
    BOOST_CHECK_EQUAL(1452174320, node.lastUpdateDate);
    BOOST_CHECK_EQUAL(boost::none, node.mimeType);
    BOOST_CHECK_EQUAL("All my files", node.name);
    BOOST_CHECK_EQUAL(2, node.nbChildren);
    BOOST_CHECK_EQUAL(234, node.nbFiles);
    BOOST_CHECK_EQUAL(2, node.nodes.size());
    BOOST_CHECK_EQUAL(boost::none, node.original);
    BOOST_CHECK_EQUAL(1, node.ownerId);
    BOOST_CHECK_EQUAL(boost::none, node.parentId);
    BOOST_CHECK_EQUAL(boost::none, node.poster);
    BOOST_CHECK_EQUAL(boost::none, node.previewState);
    BOOST_CHECK_EQUAL(6248637323, node.size);
    BOOST_CHECK_EQUAL(boost::none, node.square);
    BOOST_CHECK_EQUAL(boost::none, node.url);

    auto& file = node.nodes[0];
    BOOST_CHECK_EQUAL("file", file->type);
    BOOST_CHECK_EQUAL(1, file->ancestors.size());
    BOOST_CHECK_EQUAL(1448984709, file->creationDate);
    BOOST_CHECK_EQUAL("zjIENuYyf27JN7xxil6M0cc1", file->fid.get());
    BOOST_CHECK_EQUAL("z1DwH5cOZirP3vbtj2QsD4vGd3PyN+ldYe9U02GIoN4=", file->fkey.get());
    BOOST_CHECK_EQUAL("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/icon?k=", file->icon.get());
    BOOST_CHECK_EQUAL("565dc08533e5dfa8008b4568", file->id);
    BOOST_CHECK_EQUAL(1448984709, file->lastUpdateDate);
    BOOST_CHECK_EQUAL("video/x-matroska", file->mimeType.get());
    BOOST_CHECK_EQUAL("testFile", file->name);
    BOOST_CHECK_EQUAL(0, file->nbChildren);
    BOOST_CHECK_EQUAL(0, file->nbFiles);
    BOOST_CHECK_EQUAL(0, file->nodes.size());
    BOOST_CHECK_EQUAL("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/original?", file->original.get());
    BOOST_CHECK_EQUAL(1, file->ownerId);
    BOOST_CHECK_EQUAL("561cc82833e5dfb5008b4567", file->parentId.get());
    BOOST_CHECK_EQUAL("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/poster?", file->poster.get());
    BOOST_CHECK_EQUAL(7, file->previewState.get());
    BOOST_CHECK_EQUAL(728464990, file->size);
    BOOST_CHECK_EQUAL(1, file->servers.size());
    BOOST_CHECK_EQUAL("03", file->servers[0]);
    BOOST_CHECK_EQUAL("//cloud03.dev.gg/preview/565dc08533e5dfa8008b4568/square?", file->square.get());
    BOOST_CHECK_EQUAL("//cloud03.dev.gg/download/565dc08533e5dfa8008b4568/testFile?k=", file->url.get());

    auto& folder = node.nodes[1];
    BOOST_CHECK_EQUAL("folder", folder->type);
    BOOST_CHECK_EQUAL(1, folder->ancestors.size());
    BOOST_CHECK_EQUAL(1452170866, folder->creationDate);
    BOOST_CHECK_EQUAL(boost::none, folder->fid);
    BOOST_CHECK_EQUAL(boost::none, folder->fkey);
    BOOST_CHECK_EQUAL(boost::none, folder->icon);
    BOOST_CHECK_EQUAL("568e5e7233e5df3b008b4567", folder->id);
    BOOST_CHECK_EQUAL(1452170867, folder->lastUpdateDate);
    BOOST_CHECK_EQUAL(boost::none, folder->mimeType);
    BOOST_CHECK_EQUAL("images", folder->name);
    BOOST_CHECK_EQUAL(12, folder->nbChildren);
    BOOST_CHECK_EQUAL(233, folder->nbFiles);
    BOOST_CHECK_EQUAL(0, folder->nodes.size());
    BOOST_CHECK_EQUAL(boost::none, folder->original);
    BOOST_CHECK_EQUAL(1, folder->ownerId);
    BOOST_CHECK_EQUAL("561cc82833e5dfb5008b4567", folder->parentId.get());
    BOOST_CHECK_EQUAL(boost::none, folder->poster);
    BOOST_CHECK_EQUAL(boost::none, folder->previewState);
    BOOST_CHECK_EQUAL(0, folder->size);
    BOOST_CHECK_EQUAL(boost::none, folder->square);
    BOOST_CHECK_EQUAL(boost::none, folder->url);
}

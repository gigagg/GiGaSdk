#include <boost/exception/diagnostic_information.hpp>
#include <giga/Application.h>
#include <giga/core/Uploader.h>
#include <cpprest/details/basic_types.h>
#include <boost/algorithm/string/predicate.hpp>

using giga::Config;
using giga::Application;
using giga::core::Uploader;
using giga::core::Node;
using giga::core::FolderNode;
using boost::filesystem::path;

using utility::string_t;

int main(int, char**)
{
    Config::init(string_t(U("http://localhost:5001")),
                    string_t(U("1142f21cf897")),
                    string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

    Application app;
    auto owner = app.authenticate(U("test_main"), U("password"));
    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

    std::vector<giga::data::SmallNode> smallNodes;
    auto contacts = app.getContacts();
    for(auto & contact : contacts)
    {
        const auto& children = contact.contactData().node().getChildren();
        for(const auto& child : children)
        {
            auto nodes = app.api().nodes.getAllFiles(child->id(), U("document")).get();
            smallNodes.reserve(smallNodes.size() + nodes->size());
            std::copy(nodes->begin(), nodes->end(), std::back_inserter(smallNodes));
        }
    }
    auto nodes = app.api().nodes.getAllFiles(owner.contactData().node().id(), U("document")).get();
    smallNodes.reserve(smallNodes.size() + nodes->size());
    std::copy(nodes->begin(), nodes->end(), std::back_inserter(smallNodes));

    std::sort(smallNodes.begin(), smallNodes.end(), [](const giga::data::SmallNode& rhs, const giga::data::SmallNode& lhs) {
       return rhs.fid < lhs.fid;
    });
    auto it = std::unique(smallNodes.begin(), smallNodes.end(), [](const giga::data::SmallNode& rhs, const giga::data::SmallNode& lhs) {
        return rhs.fid == lhs.fid;
    });
    smallNodes.resize(std::distance(smallNodes.begin(), it));

    std::cout << smallNodes.size() << " documents found\n";
    auto now = std::time(0);

    for(const auto& sn : smallNodes)
    {
        auto n = std::make_shared<giga::data::Node >();
        n->name = sn.name;
        n->id = sn.id;
        n->ownerId = sn.ownerId;
        n->size = sn.size;
        n->creationDate = now;
        n->lastUpdateDate = now;
        n->type = U("file");
        n->fid = sn.fid;
        n->mimeType = sn.mimeType;
        n->url = giga::utils::str2wstr(sn.url);
        n->previewState = sn.previewState;

        auto node = giga::core::Node::create(n, app);

        if(boost::algorithm::ends_with(sn.name, "epub") ||
           boost::algorithm::ends_with(sn.name, "pdf")  ||
           boost::algorithm::ends_with(sn.name, "mobi"))
        {
            std::cout << "downloading: " << giga::utils::wstr2str(n->name) << " " << node->size() << std::endl;
            giga::core::FileDownloader fd{".", *node, app};
            fd.start();
            try
            {
                fd.task().get();
            }
            catch (...)
            {
                std::cerr << "Unhandled exception!" << std::endl <<
                boost::current_exception_diagnostic_information();
            }
        }
    }
    return 0;
}

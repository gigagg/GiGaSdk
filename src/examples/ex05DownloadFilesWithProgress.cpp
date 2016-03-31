#include <giga/Application.h>
#include <giga/core/Downloader.h>
#include <iomanip>

using giga::Config;
using giga::Application;
using giga::core::Downloader;
using giga::core::FileDownloader;
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

    // find the 'comics' folder
    auto& nodes = owner.contactData().node().getChildren();
    auto it = std::find_if(nodes.begin(), nodes.end(), [](const std::unique_ptr<giga::core::Node>& n){
        return n->name() == U("Comics");
    });
    auto comics = giga::core::Node::create(**it);

    // The number or file we will download
    auto nbFiles   = comics->nbFiles() + (comics->type() == giga::core::Node::Type::file ? 1 : 0);
    auto totalSize = comics->size(); // The total size to download

    auto progress = [nbFiles, totalSize](FileDownloader& fileDownloader, uint64_t count, uint64_t bytes) {
        auto percent = ((double) bytes * 100) / (double) totalSize;
        (ucout << U("                                                      \r")).flush(); // erase the current line.
        ucout << count << U("/") << nbFiles << U(" ")
                  << std::setprecision(3) << percent << U("% - ")
                  << fileDownloader.destinationFile().filename().native();
    };

    Downloader downloader{app, progress};
    downloader.addDownload(std::move(comics), path{U("./")});
    downloader.start();
    downloader.join();
    std::cout << std::endl;
    return 0;
}

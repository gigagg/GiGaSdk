#include <giga/Application.h>
#include <giga/core/Downloader.h>
#include <cpprest/details/basic_types.h>

using giga::Application;
using giga::core::Downloader;
using boost::filesystem::path;

using utility::string_t;

int main(int, char**)
{
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));
    auto owner = app.authenticate(U("test_main"), U("password"));
    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

    // See the ex03ListNodes.cpp to find how you can list the user nodes.
    // This is the node named 'Ebook'.
    auto ebook = app.getNodeById("56deee4b35e5df98038b4587");

    // create the downloader
    Downloader downloader{};
    downloader.addDownload(std::move(ebook), path{U("./")});

    // start the download and wait for it to finish
    downloader.start();
    downloader.join();

    return 0;
}

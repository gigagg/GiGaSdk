/**
 * @file main.cpp
 * @brief the main
 * @author Thomas Guyard
 */

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

#include <giga/Application.h>
#include <giga/core/FileUploader.h>
#include <giga/core/Node.h>
#include <giga/api/data/Node.h>
#include <giga/rest/JsonSerializer.h>
#include <giga/rest/HttpErrors.h>
#include <giga/core/Uploader.h>
#include <giga/core/Downloader.h>
#include <giga/utils/Utils.h>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <algorithm>

using boost::exception_detail::error_info_container;
using CryptoPP::EncodedObjectFilter;

#include "crypto++/rsa.h"
using namespace giga;
namespace po = boost::program_options;

void printNodeTree(core::Node& n, std::string space = "")
{
    std::cout << space << n.name() << "\n";
    space += " ";
    if (n.shouldLoadChildren())
    {
        n.loadChildren();
    }
    for(const auto& child : n.children())
    {
        printNodeTree(*child, space);
    }
}

template <typename T>
void printUsers(const char* name, T users)
{
    std::cout << "\n" << name << std::endl;
    for(auto& user : users)
    {
        std::cout << user.login() << "\t" << user.id() << "\t" << user.hasRelation() << "\n";
    }
}

template <typename T>
void printNodes(const char* name, T& nodes)
{
    std::cout << "\n" << name << std::endl;
    for(auto& node : nodes)
    {
        std::cout << core::Node::typeCvrt.toStr(node->type()) << "\t" << node->id() << "\t" << node->name() << "\n";
    }
}

void
searchNodes (const po::variables_map& vm, giga::Application& app, const char* name, core::Node::MediaType type)
{
    if (vm.count(name))
    {
        auto nodes = app.searchNode(vm[name].as<std::string>(), type);
        printNodes(name, nodes);
    }
}

int main(int argc, const char* argv[]) {
    try {
        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")

        ("login",        po::value<std::string>()->required(), "Current user login")

        ("list-contact",         po::value<bool>()->implicit_value(true), "print contacts")
        ("list-received-invits", po::value<bool>()->implicit_value(true), "print received invitation")
        ("list-sent-invits",     po::value<bool>()->implicit_value(true), "print sent invitation")
        ("list-blocked",         po::value<bool>()->implicit_value(true), "print blocked users")

        ("search-user",     po::value<std::string>(), "search users")
        ("search-video",    po::value<std::string>(), "search video files")
        ("search-audio",    po::value<std::string>(), "search audio files")
        ("search-other",    po::value<std::string>(), "search other files")
        ("search-document", po::value<std::string>(), "search document files")
        ("search-image",    po::value<std::string>(), "search image files")
        ("search-folder",   po::value<std::string>(), "search folder files")

        ("node",    po::value<std::string>(), "select the current node by id")
        ("tree",    po::value<bool>()->implicit_value(true), "print the current node tree")
        ("ls",      po::value<bool>()->implicit_value(true), "print the current node children")
        ("mkdir",   po::value<std::string>(), "create a directory under the current node")
        ("upload",  po::value<std::string>(), "file path")
        ("download",po::value<std::string>(), "folder path")
        ("continue",po::value<bool>()->implicit_value(false), "continue the download")

        ("user",    po::value<int64_t>(), "select the current user by id")
        ("invite",  po::value<bool>()->implicit_value(true), "invite the current user")
        ("block",   po::value<bool>()->implicit_value(true), "block the current user")
        ("accept",  po::value<bool>()->implicit_value(true), "accept the current user invitation")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }
        po::notify(vm);

        auto& app = Application::init(
                        std::string("http://localhost:5001"),
                        std::string("9ab7baa696ca"),
                        std::string("b1af65bffd64aa1e44d2408b44b4c4d8"));

        auto login = vm["login"].as<std::string>();
        std::string password;
        std::cout << "login: " << login << "\npassword ?" << std::endl;
//        std::cin >> password;
        password = "gigatribe";

        auto user = app.authenticate(login, password);
        std::cout << "Logged as: " << user.login()
                << ", root: " << user.protectedData().node().id()
                << std::endl;

        //
        // NETWORK
        //

        if (vm.count("list-contact"))
        {
            auto users = app.getContacts();
            printUsers("list-contact", users);
        }
        if (vm.count("list-received-invits"))
        {
            auto users = app.getInvitingUsers();
            printUsers("list-received-invits", users);
        }
        if (vm.count("list-sent-invits"))
        {
            auto users = app.getInvitedUsers();
            printUsers("list-sent-invits", users);
        }
        if (vm.count("list-blocked"))
        {
            auto users = app.getBlockedUsers();
            printUsers("list-blocked", users);
        }

        //
        // SEARCH
        //

        if (vm.count("search-user"))
        {
            auto users = app.searchUser(vm["search-user"].as<std::string>());
            printUsers("search-user", users);
        }

        searchNodes(vm, app, "search-video", core::Node::MediaType::video);
        searchNodes(vm, app, "search-audio", core::Node::MediaType::audio);
        searchNodes(vm, app, "search-other", core::Node::MediaType::unknown);
        searchNodes(vm, app, "search-document", core::Node::MediaType::document);
        searchNodes(vm, app, "search-image", core::Node::MediaType::image);
        searchNodes(vm, app, "search-folder", core::Node::MediaType::folder);

        //
        // NODE OP
        //

        if (vm.count("node"))
        {
            auto node = app.getNodeById(vm["node"].as<std::string>());
            if (vm.count("tree"))
            {
                std::cout << "tree" << std::endl;
                printNodeTree(*node);
                std::cout << std::endl;
            }
            if (vm.count("ls"))
            {
                printNodes("ls", node->children());
            }
            if (vm.count("mkdir"))
            {
                auto dir = node->addChildFolder(vm["mkdir"].as<std::string>());
                auto arr = std::array<core::Node*, 1>{&dir};
                printNodes("mkdir", arr);
            }
            if (vm.count("upload"))
            {
                if (node->type() == core::Node::Type::file)
                {
                    THROW(ErrorException{"Node must be a folder"});
                }
                core::Uploader uploader{*static_cast<core::FolderNode*>(node.get()), vm["upload"].as<std::string>()};
                auto t = pplx::create_task([&uploader] () {
                    while(true)
                    {
                        std::cout << "prep: " << uploader.isPreparationFinished();
                        auto count = uploader.uploadingFilesCount();
                        if (count > 0)
                        {
                            std::cout << " count: " << count << " p: " << std::setprecision(3);
                            uint64_t totalP = 0;
                            for(auto& up: uploader.uploadingFiles())
                            {
                                totalP += up->progress();
                                std::cout << up->progress() << " ";
                            }
                            if ((count - totalP) < 0.01 &&  uploader.isPreparationFinished())
                            {
                                std::cout << " FINISH " << std::endl;
                                return;
                            }
                        }
                        else
                        {
                            std::cout << " count: " << count;
                        }
                        std::cout << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    }
                });
                auto task = uploader.start();

                utils::waitTasks({t, task});
                auto uploads = uploader.uploadingFiles();
                std::vector<std::shared_ptr<core::Node>> arr(uploads.size());
                std::transform(uploads.begin(), uploads.end(), arr.begin(), [](const std::shared_ptr<core::FileUploader> u){
                    std::cout << "transform" << std::endl;
                    return u->task().get();
                });
                printNodes("uploaded", arr);
            }
            if (vm.count("download"))
            {
                auto nbFiles = node->nbFiles() + (node->type() == core::Node::Type::file ? 1 : 0);
                core::Downloader dl{std::shared_ptr<core::Node>(node.release()), vm["download"].as<std::string>()};
                auto dlTask = dl.start();
                auto t = pplx::create_task([&dl, nbFiles] () {
                    auto dlding = dl.downloadingFile();
                    while(dlding == nullptr || (dlding->progress() <= 0.99 && dl.downloadingFileNumber() < nbFiles))
                    {
                        if (dlding != nullptr)
                        {
                            std::cout << dlding->destinationFile().string() << " - " << std::setprecision(3) << dlding->progress() * 100 << "%" << std::endl;

                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                        dlding = dl.downloadingFile();
                    }
                });
                utils::waitTasks({t, dlTask});
                std::cout << "File downloaded: " << dl.downloadingFileNumber() << std::endl;
            }
        }

        //
        // USER OP
        //

        if (vm.count("user"))
        {
            auto user = app.getUserById(vm["user"].as<int64_t>());
            if (vm.count("invite"))
            {
                std::cout << "invite" << std::endl;
                user.invite();
            }
            if (vm.count("block"))
            {
                std::cout << "block" << std::endl;
                user.block();
            }
            if (vm.count("accept"))
            {
                std::cout << "accept" << std::endl;
                user.acceptInvitation();
            }
        }
        std::cout << "DONE" << std::endl;
    }
    catch (const ErrorException &e)
    {
        e.print();
        return 1;
    }
    catch(boost::system::system_error& e)
    {
        std::cout<<"Error: " << e.what() << std::endl;
        std::cout<<"Info: "  << boost::diagnostic_information(e) <<std::endl;

//#include <boost/lexical_cast.hpp>
//#include <boost/asio.hpp>
//#include <openssl/err.h>
//        auto error = e.code();
//        auto err = error.message();
//        err = std::string(" (")
//                +boost::lexical_cast<std::string>(ERR_GET_LIB(error.value()))+","
//                +boost::lexical_cast<std::string>(ERR_GET_FUNC(error.value()))+","
//                +boost::lexical_cast<std::string>(ERR_GET_REASON(error.value()))+") "
//        ;
//        char buf[128];
//        ::ERR_error_string_n(error.value(), buf, sizeof(buf));
//        err += buf;
//        std::cout<< err <<std::endl;

        return 1;
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << "Unknown error" << std::endl;
        return 1;
    }
    return 0;
}

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

#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>

#include <boost/program_options.hpp>

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
                        std::string("86ebaa36c3f0"),
                        std::string("2ed5cb98abd9c1a0699679990576a97e"));


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
                auto up = node->uploadFile(vm["upload"].as<std::string>()).get();
                auto t = pplx::create_task([&up] (){
                    while( up->progress() <= 0.99)
                    {
                        auto p = up->progress();
                        std::cout << "p: " << std::setprecision(3) << p * 100 << "%" << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    }
                });
                up->start();
                t.wait();
                auto node = up->task().get();
                auto arr = std::array<std::shared_ptr<core::Node>, 1>{std::move(node)};
                printNodes("uploaded", arr);
            }
            if (vm.count("download"))
            {
                auto dl = node->download(vm["download"].as<std::string>(), vm["continue"].as<bool>());
                auto t = pplx::create_task([&dl] () {
                    while( dl.progress() <= 0.99 && dl.state() != core::FileTransferer::State::canceled)
                    {
                        auto p = dl.progress();
                        std::cout << "p: " << std::setprecision(3) << p * 100 << "% - " << std::endl;
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    }
                });

                dl.start();
                t.wait();
                std::cout << "File downloaded: " << dl.task().get() << std::endl;
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
    }
    catch (const std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown error" << std::endl;
    }
    return 0;
}

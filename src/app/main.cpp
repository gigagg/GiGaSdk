/**
 * @file main.cpp
 * @brief the main
 * @author Thomas Guyard
 */

#include <cpprest/http_client.h>
#include <pplx/pplxtasks.h>

#include <giga/Application.h>
#include <giga/api/FileUploader.h>
#include <giga/api/data/Node.h>
#include <giga/rest/JsonSerializer.h>
#include <giga/rest/HttpErrors.h>

#include <iostream>
#include <chrono>
#include <iomanip>
#include <thread>

using namespace giga;

int main ( int /*argc*/, char** /*argv*/ )
{
    try
    {
        auto& app = Application::init(
                std::string("http://localhost:5001"),
                std::string("86ebaa36c3f0"),
                std::string("2ed5cb98abd9c1a0699679990576a97e"));

        auto user = app.authenticate("t.guYard", "password");
        std::cout << user.login() << std::endl;

        std::cout << "Contacts:" << std::endl;
        auto contacts = app.getContacts();
        for(auto contact : contacts)
        {
            std::cout << contact.login() << " " << contact.id() << std::endl;
        }
        std::cout << "EndContact" << std::endl;

        auto test21 = app.getUserByLogin("test21");
        std::cout << test21.login() << " " << test21.id() << " " << test21.hasRelation() << std::endl;

        auto test02 = app.getUserByLogin("test02");
        std::cout << test02.login() << " " << test02.id() << " " << test02.hasRelation() << std::endl;

//        test02 = test02.invite();
//        std::cout << test02.login() << " " << test02.id() << " " << test02.hasRelation() << std::endl;

        api::FileUploader up{
                "/home/thomas/Téléchargements/m4v-with-ac3-and-aac-startsWithKeyframe.m4v",
                "m4v-with-ac3-and-aac-startsWithKeyframe.2.m4v",
                "56b0bff533e5df21008b4567", user.privateData().nodeKeyClear};

        auto t = pplx::create_task([&up] (){
            while( up.progress().upload <= 0.99)
            {
                auto p = up.progress();
                std::cout << "p: " << std::setprecision(3) << p.upload * 100 << "% - " << p.preparation * 100 << "%" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });

        up.start();

//        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
//
//        up.pause();
//
//        std::cout << "Pause waiting ..." << std::endl;
//        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
//
//        up.resume();

        std::cout << JSonSerializer::toString(up.result()) << std::endl;
        t.get();

//        user = UsersApi::getUserById(1704770).get();
//        std::cout << user.get()->login << std::endl;

//        auto tags = UsersApi::searchTag("plop").get();
//        std::cout << JSonSerializer::toString(*tags.get()) << std::endl;
    }
    catch (const ErrorException &e)
    {
        e.print();
    }
    catch (const std::exception &e)
    {
        printf("Error exception: %s\n", e.what());
    }

    std::cout << "DONE" << std::endl;
	return 0;
}	// ----------  end of function main  ----------

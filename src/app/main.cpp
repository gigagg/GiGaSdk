/**
 * @file main.cpp
 * @brief the main
 * @author Thomas Guyard
 */

#include <iostream>

#include <giga/Application.h>
#include <giga/api/FileApi.h>
#include <giga/rest/JsonSerializer.h>
#include <giga/Config.h>

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

        test02 = test02.invite();
        std::cout << test02.login() << " " << test02.id() << " " << test02.hasRelation() << std::endl;

//        auto node = FileApi::uploadFile("/home/thomas/tmp/randfile", "randfile", user->node->id);
//        std::cout << JSonSerializer::toString(node.get()) << std::endl;

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

	return 0;
}	// ----------  end of function main  ----------

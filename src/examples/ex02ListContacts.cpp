#include <giga/Application.h>

using giga::Application;
using utility::string_t;

int main(int, char**)
{
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

    auto owner = app.authenticate(U("test_main"), U("password"));
    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << U("\n");

    auto contacts = app.getContacts();
    ucout << U("You have ") << contacts.size() << U(" contacts: ") << U("\n");

    for(const auto& contact : contacts)
    {
        auto creationDate = contact.relation().creationDate();
        auto contactLogin = contact.login();

        auto creationTime = std::chrono::system_clock::to_time_t(creationDate);
        ucout << U("\t") << contactLogin << U(" is your contact since ");
        char mbstr[100];
        if (std::strftime(mbstr, sizeof(mbstr), "%c", std::localtime(&creationTime))) {
            std::cout << mbstr << '\n';
        }
    }
    return 0;
}

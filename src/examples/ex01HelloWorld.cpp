#include <giga/Application.h>

using giga::Config;
using giga::Application;
using utility::string_t;

int main(int, char**)
{
    // We use string_t (from the casablanca library)
    // string_t is a std::string on unix (for utf8 string)
    // and std::wstring on windows (for utf16 string)
    //
    // the U("") macro will be resolved as l"" on windows

    Config::init(string_t(U("http://localhost:5001")),
                 string_t(U("1142f21cf897")),
                 string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

    Application app;
    auto owner = app.authenticate(U("test_main"), U("password"));

    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

    return 0;
}

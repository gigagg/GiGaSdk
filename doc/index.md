
Introduction
------------

GiGaSdk is a c++ sdk that connect to the [GiGa.GG](https://giga.gg) API (REST like).
You can use it to download or upload files, search and invite people etc.
For now, not all the features from the GiGa.GG API are available.
 
The GiGaSdk also provide a small CLI application (as a demo).

Getting Started
---------------

- Follow the build instructions [here](md__home_thomas_code__gi_ga_sdk__r_e_a_d_m_e.html)
- Create an account or log-in at https://giga.gg 
- Get your applications credentials by creating a new app at https://giga.gg/app
    - chose your app name
    - use http://localhost:5001 as a redirect uri
    - get your client_id and client_secret, you will need them for initializing the GiGaSdk

Hello world
-----------

~~~{cpp}
#include <giga/Application.h>

using giga::Application;
using utility::string_t;

int main(int, char**)
{
    // We use string_t (from the casablanca library)
    // string_t is a std::string on unix (for utf8 string)
    // and std::wstring on windows (for utf16 string)
    //
    // the U("") macro will be resolved as l"" on windows

    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("1142f21cf897")),
                        string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

    auto owner = app.authenticate(U("test_main"), U("password"));

    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

    return 0;
}
~~~

Compile it with ```g++ -std=c++1y main.cpp -lgigad -lboost_system -lboost_filesystem -lcrypto++ -lcpprest -lboost_regex -lcurl -lcurlcpp```





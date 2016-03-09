
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

// string_t is a std::wstring on Windows and a std::string on Unix
// use the U() macro to initialize string_t : string_t a = U("str");
using utility::string_t;

int main(int, char**)
{
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("your client_id")),
                        string_t(U("your client_secret")));
    
    auto owner = app.authenticate(U("login"), U("password"));
    
    std::cout << "Hello " << owner.id() << std::endl;
    
    return 0;
}
~~~

Compile it with ```g++ -std=c++1y main.cpp -lgigad -lboost_system -lboost_filesystem -lcrypto++ -lcpprest -lboost_regex -lcurl -lcurlcpp```


Navigating the nodes
--------------------

In the GiGa.GG api, the files and folders are represented by the ```Node``` class.
The ```Node``` class has two inheriting classes:  ```FileNode``` and ```FolderNode```.
  


~~~{cpp}
#include <giga/Application.h>
#include <giga/core/Downloader.h>

using giga::Application;
using giga::core::Downloader;
using utility::string_t;

int main(int, char**)
{
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("your client_id")),
                        string_t(U("your client_secret")));
    auto owner = app.authenticate(U("login"), U("password"));

    // Get the node you want to download.
    auto node = app.getNodeById("put a node Id here");
    
    // create a downloader
    core::Downloader dl{std::move(node), "/path/to/dest/folder"};
    
    // Start the download and wait for it ...
    dl.start().wait();
    
    std::cout << "File downloaded: " << dl.downloadingFileNumber() << std::endl;
    return 0;
}
~~~


Downloading data
----------------

To download data from GiGa.GG, you can use the ```Downloader``` class.
For example:

~~~{cpp}
#include <giga/Application.h>
#include <giga/core/Downloader.h>

using giga::Application;
using giga::core::Downloader;
using utility::string_t;

int main(int, char**)
{
    auto& app = Application::init(
                        string_t(U("http://localhost:5001")),
                        string_t(U("your client_id")),
                        string_t(U("your client_secret")));
    auto owner = app.authenticate(U("login"), U("password"));

    // Get the node you want to download.
    auto node = app.getNodeById("put a node Id here");
    
    // create a downloader
    core::Downloader dl{std::move(node), "/path/to/dest/folder"};
    
    // Start the download and wait for it ...
    dl.start().wait();
    
    std::cout << "File downloaded: " << dl.downloadingFileNumber() << std::endl;
    return 0;
}
~~~




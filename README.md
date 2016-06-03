<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Welcome!](#welcome)
- [Getting Started](#getting-started)
- [Overview](#overview)
  - [Hello world](#hello-world)
  - [Concurrency](#concurrency)
  - [Strings](#strings)
- [Dependancies](#dependancies)
- [Build On linux (tested on ubuntu 14.04)](#build-on-linux-tested-on-ubuntu-1404)
- [Build with Visual Studio 2015](#build-with-visual-studio-2015)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->


Welcome!
--------

The GiGaSdk provide an easy access to the https://giga.gg/ api.
It includes a c++ library and a small cli executable : ``` GiGaSdk -h ```


Getting Started
---------------

Are you new to the GiGaSdk ? To get going, we recommend you start by playing around on https://giga.gg
The more you know about the web app GiGa.GG, the easier it will be to use the GiGaSdk.

Most of the documentation is :

- this file you are reading :)
- the reference documentation: http://gigagg.github.io/index.html
- some examples: [here](src/examples)
- the main file: [here](src/app/main.cpp)

Overview
--------

The GiGaSdk uses the GiGa.GG rest-like API, and authenticate via oauth2. 
You can create an 'app' at https://giga.gg/app to get your oauth2 client id and secret.

### Hello world
~~~{cpp}
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

    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

    return 0;
}
~~~

There are more examples [here](src/examples)


The Sdk entry point is the ```giga::Application``` class. You will need to init the ```Application``` with your oauth2 credentials. 
Then you have to log-in using your giga login/password. 
The SDK does not allows to create a new account. 

You should only need stuff from the ```giga::core``` namespace :
- ```giga::core::User``` represents a user. It can have ```UserRelation```s with other users (contact, invitation ...)
- ```giga::core::Node``` represents a file or a Folder. This is a virtual class. FileNode and FolderNode inherite from it.
- The ```giga::core::Uploader``` and ```giga::core::Downloader``` classes allows you to control upload and downloads

### Concurrency
Inside the GiGaSdk, the concurrency is done via the cpprestSdk and the ```pplx::task<T>``` class.

**The GiGaSdk is not thread-safe**

### Strings
The default type of string is different on Windows and Unix. 
By default Windows uses std::wstring and utf16 char, Unix uses std::string and utf8 char. 
GiGaSdk eavily uses the [Microsoft cpprestsdk (casablanca)](https://github.com/Microsoft/cpprestsdk/). 
So it uses the same solution:
- ```utility::string_t``` is a ```wstring``` on Window and a ```string``` on linux
- the macro ```U("...")``` will resolve to ```l"..."``` on Windows
- cf: ```#include <cpprest/details/basic_types.h>```

Dependancies
------------

- [cmake](https://cmake.org) >= 2.8
- [openssl](https://www.openssl.org/) >= 1.0.0
- [boost](http://www.boost.org/) >= 1.54
- [libcurl](https://github.com/curl/curl) >= 7.46
- [casablanca](https://github.com/Microsoft/cpprestsdk)
- [curlcpp](https://github.com/Giga-gg/curlcpp)
- [crypto++](http://cryptopp.com/)


Build On linux (tested on ubuntu 14.04)
---------------------------------------

~~~{.sh}
# installing dependancies
sudo apt-get install g++ git make libboost1.54-all-dev libssl-dev cmake

# getting the sources
git clone https://github.com/gigagg/GiGaSdk.git
cd GiGaSdk
git submodule update --init

# compiling casablanca
cd vendors/casablanca/Release/
mkdir build.release
cd build.release/
cmake .. -DCMAKE_BUILD_TYPE=Release
make
sudo make install
cd ../../../..

# compiling crypto++
cd vendors/cryptopp/crypto++/
make
sudo make install
cd ../../..

# compiling/installing libcurl
cd vendors/curl
mkdir build
cd build
cmake ..
make
sudo make install
cd ../../..

# compiling curlcpp
cd vendors/curlcpp/build
cmake ..
make
sudo make install
cd ../../..

# compiling/installing GiGaSdk
cd build
cmake ..
make
sudo make install

sudo ldconfig
~~~

Build with Visual Studio 2015
-----------------------------

- Make sure cmake and git are install and in the PATH
- Get the sources using git at ```http://gitlab.giga.gg/t.guyard/GiGaSdk.git```
- Get the dependencies:
    - Open visual studio project ```vendors/VS2015/get_dependencies.sln```.
    - Generating the solution should get the dependencies via the NuGet package manager, and copy them in the right folder.
- Compile curl_cpp: 
    - In a visual studio command prompt go to ```vendors/curlcpp/build```
    - Use cmake to build the project: 
    ```cmake .. -DCMAKE_LIBRARY_PATH="../VS2015/deps/Debug/lib" -DCMAKE_PREFIX_PATH="../VS2015/deps/" -DCMAKE_INSTALL_PREFIX="../../VS2015/deps/" -DCMAKE_BUILD_TYPE=Debug```
    OR for a release build: 
    ```cmake .. -DCMAKE_LIBRARY_PATH="../VS2015/deps/Release/lib" -DCMAKE_PREFIX_PATH="../VS2015/deps/" -DCMAKE_INSTALL_PREFIX="../../VS2015/deps/" -DCMAKE_BUILD_TYPE=Release```
    - Open the generated solution file generate the main project
    - Generate the INSTALL project.
- Compile cryto++
    - Go to ```vendors/cryptopp/crypto++```
    - open the .sln file
    - Generate the solution (select release for a release build)
- Compile the GiGaSdk:
    - In a visual studio command prompt go to ```build```
    - ```cmake ..``` OR for a release build ```cmake .. -DCMAKE_BUILD_TYPE=Release```
    - use the generated vs project to build GiGaSdk


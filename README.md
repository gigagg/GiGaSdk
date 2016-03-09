GiGaSdk provide an easy access to the https://giga.gg/ api.
It includes :

- a c++ library for integrating giga with other apps
- a small cli executable : ``` GiGaSdk -h ``` 


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
git clone http://gitlab.giga.gg/t.guyard/GiGaSdk.git
cd GiGaSdk
git submodule update --init

# compiling casablanca
cd vendors/casablanca/Release/
mkdir build.release
cd build.release/
cmake .. -DCMAKE_BUILD_TYPE=Release
make
cd ../../../..

# compiling crypto++
cd vendors/cryptopp/crypto++/
make
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











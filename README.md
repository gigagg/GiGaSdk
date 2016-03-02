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

``` sh
# installing dependancies
sudo apt-get install g++ git make libboost1.54-all-dev libssl-dev cmake

# getting the sources
git clone git@gitlab.giga.gg:t.guyard/GiGaSdk.git
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
```


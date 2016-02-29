from ubuntu:14.04
maintainer Thomas Guyard <t.guyard@gigatribe.com>

ENV LC_ALL C.UTF-8
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get install -y g++ git make libboost1.54-all-dev libssl-dev cmake

RUN cd /root                                          && \
    git clone git@gitlab.giga.gg:t.guyard/GiGaSdk.git && \
    cd GiGaSdk                                        && \
    git submodule update --init 

# compiling casablanca
RUN cd vendors/casablanca/Release/      && \
    mkdir build.release                 && \
    cd build.release/                   && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cd ../../..

# compiling crypto++      && \
RUN cd cryptopp/crypto++/ && \
    make                  && \
    cd ../..

# compiling/installing libcurl
RUN cd curl           && \
    mkdir build       && \
    cd build          && \
    cmake ..          && \
    make              && \
    sudo make install && \
    cd ../..

# compiling curlcpp
RUN cd curlcpp/build && \
    cmake ..         && \
    make             && \
    cd ../..

# compiling/installing GiGaSdk
RUN cd ../build       && \
    cmake ..          && \
    make              && \
    sudo make install

ENTRYPOINT ["/usr/local/bin/GiGaSdk"]

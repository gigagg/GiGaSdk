# FindCurlcpp package
#
# Tries to find the Curlcpp library
#

find_package(PkgConfig)

include(LibFindMacros)

# Include dir
find_path(CURLCPP_INCLUDE_DIR
  NAMES
    curl_config.h
  PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/curlcpp
    ${CURLCPP_PKGCONF_INCLUDE_DIRS}
    ${CURLCPP_DIR}
    $ENV{CURLCPP_DIR}
    /usr/local/include
    /usr/include
  PATH_SUFFIXES 
    include
    Release/include
)

# Library
find_library(CURLCPP_UTILS_LIBRARY
  NAMES 
    curlcpp
  PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/curlcpp
    ${CURLCPP_PKGCONF_LIBRARY_DIRS}
    ${CURLCPP_DIR}
    $ENV{CURLCPP_DIR}
    /usr/local
    /usr
  PATH_SUFFIXES
    build/src
    lib
)

set(CURLCPP_PROCESS_LIBS CURLCPP_LIBRARY CURLCPP_UTILS_LIBRARY)
set(CURLCPP_PROCESS_INCLUDES CURLCPP_INCLUDE_DIR)
libfind_process(CURLCPP)

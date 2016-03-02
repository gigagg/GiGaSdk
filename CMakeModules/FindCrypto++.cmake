# FindCrypto++ package
#
# Tries to find the Crypto++ library
#

find_package(PkgConfig)

include(LibFindMacros)

# Include dir
find_path(CRYPTO++_INCLUDE_DIR
  NAMES
    crypto++/cryptlib.h cryptopp/cryptlib.h
  PATHS 
    ${CRYPTO++_PKGCONF_INCLUDE_DIRS}
    ${CRYPTO++_DIR}
    $ENV{CRYPTO++_DIR}
    /usr/local/include
    /usr/include
  PATH_SUFFIXES 
    include
    Release/include
)

IF (NOT EXISTS "${CRYPTO++_INCLUDE_DIR}/crypto++/cryptlib.h" AND EXISTS "${CRYPTO++_INCLUDE_DIR}/cryptopp/cryptlib.h")
	SET(USE_CRYPTO_PP true)
ENDIF()

# Library
find_library(CRYPTO++_UTILS_LIBRARY
  NAMES 
    cryptopp cryptlib
  PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/vendors/cryptopp/crypto++
    ${CRYPTO++_PKGCONF_LIBRARY_DIRS}
    ${CRYPTO++_DIR}
    $ENV{CRYPTO++_DIR}
    /usr/local
    /usr
  PATH_SUFFIXES
    build
    lib
	Debug
	Release
)

set(CRYPTO++_PROCESS_LIBS CRYPTO++_LIBRARY CRYPTO++_UTILS_LIBRARY)
set(CRYPTO++_PROCESS_INCLUDES CRYPTO++_INCLUDE_DIR)
libfind_process(CRYPTO++)

# - Find XALAN library
# Find the native XALAN includes and library
# This module defines
#  XALAN_INCLUDE_DIR, where to find xercesc/dom/DOM.hpp, etc.
#  XALAN_LIBRARIES, libraries to link against to use XALAN.
#  XALAN_FOUND, If false, do not try to use XALAN.
# also defined, but not for general use are
#  XALAN_LIBRARY, where to find the XALAN library.

FIND_PATH(XALAN_INCLUDE_DIR xalanc/Include/PlatformDefinitions.hpp
  /usr/local/include
  /usr/include
  /opt/local/include
  "$ENV{XALAN_DIR}/include"
  "$ENV{XALAN_ROOT}/include"
  "$ENV{XALAN_ROOT}/src"
  "$ENV{XALAN_ROOT_DIR}/include"
)

IF(WIN32)
	IF(MSVC) # Visual / nmake
		IF(CMAKE_BUILD_TYPE MATCHES "Debug")
			SET(XALAN_NAMES ${XALAN_NAMES} Xalan-C_1D.lib)
			SET(XALAN_LIB_NAME Xalan-C_1D.lib)
		ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
			SET(XALAN_NAMES ${XALAN_NAMES} Xalan-C_1.lib)
			SET(XALAN_LIB_NAME Xalan-C_1.lib)
		ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")
	ELSE (CMAKE_GENERATOR MATCHES "(Visual.*)") # assuming it is gcc ...
		#SET(XALAN_NAMES ${XALAN_NAMES}  xerces-c.dll )
		#SET(XALAN_LIB_NAME libxerces-c.a)
	ENDIF(MSVC) # Visual / nmake
ELSE(WIN32)
	IF(APPLE)
	SET(XALAN_NAMES ${XALAN_NAMES} libxalan-c.dylib) # TODO correct this !!!
	SET(XALAN_LIB_NAME libxalan-c.dylib)
	ELSE(APPLE)
	SET(XALAN_NAMES ${XALAN_NAMES} libxalan-c.so )# TODO correct this !!!
	SET(XALAN_LIB_NAME libxalan-c.so)
	ENDIF(APPLE)
ENDIF(WIN32)


SET(SEARCH_PATH 
  /usr/local/lib 
  /opt/local/lib
  /usr/lib
  "C:/Program Files/xerces-c-vc-9.0/lib"
  "$ENV{XALAN_DIR}/lib"
  "$ENV{XALAN_ROOT}/lib"
  "$ENV{XALAN_ROOT_DIR}/lib")
  
FIND_LIBRARY(XALAN_LIBRARY
  NAMES ${XALAN_NAMES}
  PATHS ${SEARCH_PATH}
  )

FIND_PATH( XALAN_LIB_DIR ${XALAN_LIB_NAME}
  ${SEARCH_PATH}
  )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(xalan XALAN_INCLUDE_DIR XALAN_LIBRARY XALAN_LIB_DIR )

IF(XALAN_INCLUDE_DIR AND XALAN_LIBRARY)
			SET( XALAN_FOUND "YES" )
			SET( XALAN_LIBRARIES ${XALAN_LIBRARY} )
			SET(XALAN_INCLUDE_DIRS ${XALAN_INCLUDE_DIR})
ELSE()
	SET(NO_XALAN true)
ENDIF()

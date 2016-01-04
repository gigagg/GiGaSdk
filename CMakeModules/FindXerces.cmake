# - Find XERCES library
# Find the native XERCES includes and library
# This module defines
#  XERCES_INCLUDE_DIR, where to find xercesc/dom/DOM.hpp, etc.
#  XERCES_LIBRARIES, libraries to link against to use XERCES.
#  XERCES_FOUND, If false, do not try to use XERCES.
# also defined, but not for general use are
#  XERCES_LIBRARY, where to find the XERCES library.

FIND_PATH(XERCES_INCLUDE_DIR xercesc/dom/DOM.hpp
  /usr/local/include
  /usr/include
  "C:/Program Files/xerces-c-vc-9.0/include"
  "$ENV{XERCES_DIR}/include"
  "$ENV{XERCES_ROOT}/include"
  "$ENV{XERCES_ROOT_DIR}/include"
)


IF(WIN32)
	IF(MSVC) # Visual / nmake
		IF(CMAKE_BUILD_TYPE MATCHES "Debug")
			SET(XERCES_NAMES ${XERCES_NAMES} xerces-c_3D.lib)
			SET(XERCES_LIB_NAME xerces-c_3D.lib)
		ELSE(CMAKE_BUILD_TYPE MATCHES "Debug")
			SET(XERCES_NAMES ${XERCES_NAMES} xerces-c_3.lib)
			SET(XERCES_LIB_NAME xerces-c_3.lib)
		ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")
	ELSE (CMAKE_GENERATOR MATCHES "(Visual.*)") # assuming it is gcc ...
		#SET(XERCES_NAMES ${XERCES_NAMES}  xerces-c.dll )
		#SET(XERCES_LIB_NAME libxerces-c.a)
	ENDIF(MSVC) # Visual / nmake
ELSE(WIN32)
	IF(APPLE)
	SET(XERCES_NAMES ${XERCES_NAMES} libxerces-c.dylib) # TODO correct this !!!
	SET(XERCES_LIB_NAME libxerces-c.dylib)
	ELSE(APPLE)
	SET(XERCES_NAMES ${XERCES_NAMES} libxerces-c.so) # TODO correct this !!!
	SET(XERCES_LIB_NAME libxerces-c.so)
	ENDIF(APPLE)
ENDIF(WIN32)

SET(SEARCH_PATH 
  /usr/local/lib 
  /opt/local/lib
  /usr/lib
  "C:/Program Files/xerces-c-vc-9.0/lib"
  "$ENV{XERCES_DIR}/lib"
  "$ENV{XERCES_ROOT}/lib"
  "$ENV{XERCES_ROOT_DIR}/lib")
  
FIND_LIBRARY(XERCES_LIBRARY
  NAMES ${XERCES_NAMES}
  PATHS ${SEARCH_PATH}
  )

FIND_PATH( XERCES_LIB_DIR ${XERCES_LIB_NAME}
  ${SEARCH_PATH}
  )
  
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(xerces XERCES_INCLUDE_DIR XERCES_LIBRARY XERCES_LIB_DIR )

IF(XERCES_INCLUDE_DIR)
	IF(XERCES_LIBRARY)
		IF(XERCES_LIB_DIR)
			SET( XERCES_FOUND "YES" )
			SET( XERCES_LIBRARIES ${XERCES_LIBRARY} )
			SET( XERCES_INCLUDE_DIRS ${XERCES_INCLUDE_DIR} )
		ENDIF(XERCES_LIB_DIR)
	ENDIF(XERCES_LIBRARY)
ENDIF(XERCES_INCLUDE_DIR)

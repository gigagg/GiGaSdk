# - Find CURLCPP

if(CURLCPP_INCLUDE_DIR AND CURLCPP_LIBRARIES)
   set(CURLCPP_FOUND TRUE)

else(CURLCPP_INCLUDE_DIR AND CURLCPP_LIBRARIES)
  find_path(CURLCPP_INCLUDE_DIR curl_config.h
      /usr/include
      /usr/local/include
      $ENV{SystemDrive}/curlcpp/include
      )

  find_library(CURLCPP_LIBRARIES NAMES curlcpp
      PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      $ENV{SystemDrive}/CURLCPP/lib
      )

  if(CURLCPP_INCLUDE_DIR AND CURLCPP_LIBRARIES)
    set(CURLCPP_FOUND TRUE)
    message(STATUS "Found CURLCPP: ${CURLCPP_INCLUDE_DIR}, ${CURLCPP_LIBRARIES}")
  else(CURLCPP_INCLUDE_DIR AND CURLCPP_LIBRARIES)
    set(CURLCPP_FOUND FALSE)
    message(STATUS "CURLCPP not found.")
  endif(CURLCPP_INCLUDE_DIR AND CURLCPP_LIBRARIES)

  mark_as_advanced(CURLCPP_INCLUDE_DIR CURLCPP_LIBRARIES)

endif(CURLCPP_INCLUDE_DIR AND CURLCPP_LIBRARIES)

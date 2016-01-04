
INSTALL(DIRECTORY "etc/"
        DESTINATION "etc/gigasdk"
              COMPONENT etc
                FILES_MATCHING PATTERN ".svn" EXCLUDE)

INSTALL(DIRECTORY "share/"
        DESTINATION "share/gigasdk"
              COMPONENT share
                FILES_MATCHING PATTERN ".svn" EXCLUDE)


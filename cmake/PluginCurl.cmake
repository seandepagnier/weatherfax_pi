##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3+
##---------------------------------------------------------------------------

# configure curl library

INCLUDE_DIRECTORIES(src/wxcurl)
IF(WIN32)
    SET(CURL_LIBRARIES "curl.dll")
    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${CURL_LIBRARIES})
    INCLUDE_DIRECTORIES(src/wxcurl/include)
    INSTALL(FILES "buildwin/libcurl.dll" DESTINATION ".")
ENDIF(WIN32)

IF(UNIX)
    FIND_PACKAGE(CURL REQUIRED)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${CURL_LIBRARIES} )
ENDIF(UNIX)

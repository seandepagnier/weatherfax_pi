##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3
##---------------------------------------------------------------------------


# IF(WIN32)   -changed
SET(SRC_TINYXML
            src/tinyxml/tinyxml.cpp
            src/tinyxml/tinyxmlparser.cpp
            src/tinyxml/tinyxmlerror.cpp
)

# from cmake  not needed see cmake/PluginXML.cmake  Line 593
#SET (SRC_TINYXML
#            src/tinyxml/tinyxml.cpp
#			src/tinyxml/tinyxmlerror.cpp
#			src/tinyxml/tinyxmlparser.cpp
#			src/tinyxml/tinyxml.h
#)

ADD_LIBRARY(${PACKAGE_NAME}_LIB_PLUGINXML STATIC ${SRC_TINYXML})
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${PACKAGE_NAME}_LIB_PLUGINXML )
# ENDIF(WIN32)

ADD_DEFINITIONS(-DTIXML_USE_STL)

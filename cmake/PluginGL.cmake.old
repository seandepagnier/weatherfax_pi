##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3
##---------------------------------------------------------------------------

SET(SRC_PLUGINGL
          src/plugingl/pidc.cpp
          src/plugingl/pi_shaders.cpp
          src/plugingl/TexFont.cpp
          src/plugingl/qtstylesheet.cpp
          )

#  from cmake  -not needed see cmake/PluginGL.cmake  Line 594
#SET (SRC_PLUGINGL
#			src/plugingl/linmath.h
#			src/plugingl/pi_shaders.h
#			src/plugingl/pidc.h
#			src/plugingl/qtstylesheet.h
#			src/plugingl/TexFont.h
#           src/plugingl/pi_shaders.cpp
#			src/plugingl/pidc.cpp
#			src/plugingl/qtstylesheet.cpp
#			src/plugingl/TexFont.cpp
#)


if(UNIX)
    add_definitions("-fpic")
endif(UNIX)

ADD_LIBRARY(${PACKAGE_NAME}_LIB_PLUGINGL STATIC ${SRC_PLUGINGL})
TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} ${PACKAGE_NAME}_LIB_PLUGINGL )

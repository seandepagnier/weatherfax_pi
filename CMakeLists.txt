# ---------------------------------------------------------------------------
# Frontend2 Author: jongough aka. Jon Gough    License:wxWidgets License
##---------------------------------------------------------------------------
## Author:      Sean d'Epagnier
##***************************************************************************
## *   Copyright (C) 2010 by Sean d'Epagnier                  *
## *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx,                                       *
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                       *
#-----------------------------------------------------------------------------
# Frontend2 Author: Jon Gough
# Frontend2 Version: 1.0.167.0  17/05/2021  DD/MM/YYYY 
# ---------------------------------------------------------------------------
## ----- When changing this file do NOT change the order in which sections occur        ----- ##
## ----- Changes should only be made between the section blocks that identify where     ----- ##
## ----- these changes should be. The whole configuration process relies on this        ----- ##
## ----- sequence to be successful                                                      ----- ##
##  
## ----- Modify section below to include all the details for your plugin ----- ##
  

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.1.1)
if(COMMAND cmake_policy)
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif(POLICY CMP0043)
    cmake_policy(SET CMP0048 NEW)
    if(POLICY CMP0077)
        cmake_policy(SET CMP0077 NEW)
    endif(POLICY CMP0077)
endif(COMMAND cmake_policy)

# define plugin name, owner and versions
SET(VERBOSE_NAME weatherfax)
set(COMMON_NAME "WeatherFax") # Search "getcommonname"in weatherfax_pi.cpp
set(TITLE_NAME "WeatherFax")
set(PACKAGE_CONTACT "Sean d'Epagnier")
SET(PACKAGE "weatherfax_pi")
set(SHORT_DESCRIPTION "Open fax image files or decode audio faxes and overlay on chart")
set(LONG_DESCRIPTION "Open image files directly or decode audio faxes to an image, simple calibration to overlay image on top of charts. Converts images in mercator, polar, conic and uniform coordinates. Convert any image into a raster chart. Built in HF Radio Fax database for SSB and for internet retrieval from meterological sites.")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "9")
set(VERSION_PATCH "14")
set(VERSION_TWEAK "3")
set(VERSION_DATE "25-05-2021")  # DD/MM/YYYY format
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")
set(TP_COMMENT "  * Release for O5 using CI")

set(PARENT "opencpn")

#No longer needed, queried.
#set(GIT_USER "rgleason")
#set(GIT_REPOSITORY_NAME "weatherfax_pi")

# The next line allows setup of a local webserver with git for testing purposes.
# The default shown below is done in cmake/pluginconfigure.cmake now, is needed to build locally and to build flatpak.
#set(GIT_REPOSITORY_SERVER "github.com")

# Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod")   #Standard Repos
set(BETA "beta")   #Standard Repos
set(ALPHA "alpha") #Standard Repos

# Set if yourCloudsmith Base Repository name does not match your Git Repository name.
#set(CLOUDSMITH_BASE_REPOSITORY "weatherfax_pi")
# set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Rick's customization
set(CLOUDSMITH_BASE_REPOSITORY "weatherfax")  #Rick's customization

# Set the following to use OpenCPN cloudsmith repositories instead of own repositories
#set(CLOUDSMITH_USER "rick-gleason")
set(CLOUDSMITH_USER "opencpn")

# Set Plugin info-url, summary and description
set(XML_INFO_URL "https://opencpn.org/OpenCPN/plugins/weatherfax.html")
set(XML_SUMMARY  ${SHORT_DESCRIPTION}) 
set(XML_DESCRIPTION  ${LONG_DESCRIPTION})

## ----- Modify above to include all the plugin details --------- ##
## ----- Modify below for plugin special requirements------------ ##

option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")

# For Weatherfax this must be set ON or it fails to compile
option(USE_GL "Enable OpenGL support" ON)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")


## ----- The statement below is used to setup standard variables that are required by the CMAKE process - do not remove ----- ##

project(${PACKAGE})	   

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;")

##=======================================
include("PluginSetup")
##=======================================
## ----- The above is used to setup standard variables that are required by the CMAKE process - do not remove ----- ##
## ----- Modify section below if there are special requirements for the plugin ----- ##

set(CMAKE_VERBOSE_MAKEFILE "Activate verbose mode for make files" ON)

option(Plugin_CXX11 "Use c++11" OFF)

## ----- Modify section above if there are special requirements for the plugin ----- ##
## ----- do not change next section - needed to configure build process ----- ##

#======== Portaudio Section below direct from weatherfax_pi =======
IF (UNIX)
INCLUDE("cmake/FindPortaudio.cmake")
IF (PORTAUDIO_FOUND)
    MESSAGE (STATUS "${CMLOC}Portaudio Found")
    INCLUDE_DIRECTORIES(BEFORE ${PORTAUDIO_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${PORTAUDIO_LIBRARIES})
    ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
    message(STATUS "${CMLOC}In Unix:Found Portaudio")
ELSE (PORTAUDIO_FOUND)
    MESSAGE (STATUS "${CMLOC}Portaudio Not Found...")
ENDIF (PORTAUDIO_FOUND)
ELSE (UNIX)
    INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/include)
    IF(STANDALONE MATCHES "BUNDLED")
	# within OpenCPN tree
    	SET(EXTRA_LIBS ${EXTRA_LIBS} ../../buildwin/portaudio_x86.lib)
    	INSTALL(FILES "../../buildwin/portaudio_x86.dll" DESTINATION ".")
    	message(STATUS "${CMLOC}bundled: portaudio in ../../buildwin/portaudio_x86.dll")
    ELSE()
    	SET(EXTRA_LIBS ${EXTRA_LIBS} ${PROJECT_SOURCE_DIR}/buildwin/portaudio_x86.lib)
    	INSTALL(FILES "${PROJECT_SOURCE_DIR}/buildwin/portaudio_x86.dll" DESTINATION ".")
    	message(STATUS "${CMLOC}Not bundled: portaudio in ${PROJECT_SOURCE_DIR}/buildwin/portaudio_x86.dll")
    ENDIF()

    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
    message(STATUS "${CMLOC}Not in Unix:Setting USE_PORTAUDIO")
ENDIF (UNIX)

IF (MSVC)
    INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/include/msvc)
ENDIF (MSVC)

## --Section above is direct from Weatherfax located above SET(SRC_WEATHERFAX  ---##
## --Below needed for build process.
##=======================================
include("PluginConfigure")
##=======================================
## ----- do not change section above - needed to configure build process ----- 
## ----- Change below to match project requirements for source, headers, etc. ----- ##

add_definitions(-DUSE_S57)

SET(SRC_WEATHERFAX
            src/weatherfax_pi.cpp
            src/FaxDecoder.cpp
            src/DecoderOptionsDialog.cpp
            src/WeatherFax.cpp
            src/WeatherFaxWizard.cpp
            src/WeatherFaxUI.cpp
            src/WeatherFaxImage.cpp
            src/SchedulesDialog.cpp
            src/InternetRetrievalDialog.cpp
            src/AboutDialog.cpp
            src/icons.cpp
            src/wximgkap.cpp
)

# Added by Rick recently
SET (HDRS
			src/AboutDialog.h
			src/DecoderOptionsDialog.h
			src/defs.h
			src/FaxDecoder.h
			src/georef.h
			src/icons.h
			src/InternetRetrievalDialog.h
			src/SchedulesDialog.h
			src/WeatherFax.h
			src/weatherfax_pi.h
			src/WeatherFaxImage.h
			src/WeatherFaxUI.h
			src/WeatherFaxWizard.h
			src/wximgkap.h
#           api-16/ocpn_plugin.h
            libs/ocpn-api/ocpn_plugin.h
)

SET(SRC_AUDIOFILE
			src/libaudiofile/af_vfs.h
			src/libaudiofile/afinternal.h
			src/libaudiofile/AIFF.h
			src/libaudiofile/audiofile.h
			src/libaudiofile/AudioFormat.h
			src/libaudiofile/aupvinternal.h
			src/libaudiofile/aupvlist.h
			src/libaudiofile/AVR.h
			src/libaudiofile/Buffer.h
			src/libaudiofile/byteorder.h
			src/libaudiofile/CAF.h
			src/libaudiofile/Compiler.h
			src/libaudiofile/compression.h
			src/libaudiofile/config.h
			src/libaudiofile/debug.h
			src/libaudiofile/error.h
			src/libaudiofile/extended.h
			src/libaudiofile/Features.h
			src/libaudiofile/File.h
			src/libaudiofile/FileHandle.h
			src/libaudiofile/FLACFile.h
			src/libaudiofile/g711.h
			src/libaudiofile/IFF.h
			src/libaudiofile/Instrument.h
			src/libaudiofile/IRCAM.h
			src/libaudiofile/Marker.h
			src/libaudiofile/NeXT.h
			src/libaudiofile/NIST.h
			src/libaudiofile/PacketTable.h
			src/libaudiofile/pcm.h
			src/libaudiofile/Raw.h
			src/libaudiofile/SampleVision.h
			src/libaudiofile/Setup.h
			src/libaudiofile/Shared.h
			src/libaudiofile/Tag.h
			src/libaudiofile/Track.h
			src/libaudiofile/units.h
			src/libaudiofile/util.h
			src/libaudiofile/UUID.h
			src/libaudiofile/VOC.h
			src/libaudiofile/WAVE.h
			src/libaudiofile/alac/ALACAudioTypes.h
			src/libaudiofile/alac/ALACBitUtilities.h
			src/libaudiofile/alac/ALACDecoder.h
			src/libaudiofile/alac/ALACEncoder.h
			src/libaudiofile/alac/dplib.h
			src/libaudiofile/alac/EndianPortable.h
			src/libaudiofile/alac/matrixlib.h
			src/libaudiofile/modules/ALAC.h
			src/libaudiofile/modules/BlockCodec.h
			src/libaudiofile/modules/FileModule.h
			src/libaudiofile/modules/FLAC.h
			src/libaudiofile/modules/G711.h
			src/libaudiofile/modules/IMA.h
			src/libaudiofile/modules/Module.h
			src/libaudiofile/modules/ModuleState.h
			src/libaudiofile/modules/MSADPCM.h
			src/libaudiofile/modules/PCM.h
			src/libaudiofile/modules/RebufferModule.h
			src/libaudiofile/modules/SimpleModule.h

            src/libaudiofile/aupv.c
            src/libaudiofile/error.c
            src/libaudiofile/extended.c
            src/libaudiofile/g711.c

            src/libaudiofile/AIFF.cpp
            src/libaudiofile/AVR.cpp
            src/libaudiofile/CAF.cpp
            src/libaudiofile/IFF.cpp
            src/libaudiofile/IRCAM.cpp
            src/libaudiofile/NIST.cpp
            src/libaudiofile/NeXT.cpp
            src/libaudiofile/Raw.cpp
            src/libaudiofile/SampleVision.cpp
            src/libaudiofile/VOC.cpp
            src/libaudiofile/AudioFormat.cpp
            src/libaudiofile/Buffer.cpp
            src/libaudiofile/FLACFile.cpp
            src/libaudiofile/File.cpp
            src/libaudiofile/FileHandle.cpp
            src/libaudiofile/Instrument.cpp
            src/libaudiofile/Loop.cpp
            src/libaudiofile/Marker.cpp
            src/libaudiofile/Miscellaneous.cpp
            src/libaudiofile/Setup.cpp
            src/libaudiofile/Track.cpp
            src/libaudiofile/UUID.cpp
            src/libaudiofile/WAVE.cpp
            src/libaudiofile/aes.cpp
            src/libaudiofile/af_vfs.cpp
            src/libaudiofile/compression.cpp
            src/libaudiofile/data.cpp
            src/libaudiofile/debug.cpp
            src/libaudiofile/format.cpp
            src/libaudiofile/openclose.cpp
            src/libaudiofile/PacketTable.cpp
            src/libaudiofile/pcm.cpp
            src/libaudiofile/query.cpp
            src/libaudiofile/units.cpp
            src/libaudiofile/util.cpp
            src/libaudiofile/alac/ALACEncoder.cpp
            src/libaudiofile/alac/ALACDecoder.cpp
            src/libaudiofile/alac/ag_dec.c
            src/libaudiofile/alac/ag_enc.c
            src/libaudiofile/alac/ALACBitUtilities.c
            src/libaudiofile/alac/dp_dec.c
            src/libaudiofile/alac/dp_enc.c
            src/libaudiofile/alac/EndianPortable.c
            src/libaudiofile/alac/matrix_dec.c
            src/libaudiofile/alac/matrix_enc.c
            src/libaudiofile/modules/ALAC.cpp
            src/libaudiofile/modules/BlockCodec.cpp            
            src/libaudiofile/modules/FLAC.cpp
            src/libaudiofile/modules/FileModule.cpp
            src/libaudiofile/modules/G711.cpp
            src/libaudiofile/modules/IMA.cpp
            src/libaudiofile/modules/MSADPCM.cpp
            src/libaudiofile/modules/Module.cpp
            src/libaudiofile/modules/ModuleState.cpp
            src/libaudiofile/modules/PCM.cpp
            src/libaudiofile/modules/RebufferModule.cpp
            src/libaudiofile/modules/SimpleModule.cpp
)

#  not needed see cmake/PluginGL.cmake  Line 594
SET (SRC_PLUGINGL
			src/plugingl/linmath.h
			src/plugingl/pi_shaders.h
			src/plugingl/pidc.h
			src/plugingl/qtstylesheet.h
			src/plugingl/TexFont.h
            src/plugingl/pi_shaders.cpp
			src/plugingl/pidc.cpp
			src/plugingl/qtstylesheet.cpp
			src/plugingl/TexFont.cpp
)

#  not needed see cmake/PluginXML.cmake  Line 593
SET (SRC_TINYXML
            src/tinyxml/tinyxml.cpp
			src/tinyxml/tinyxmlerror.cpp
			src/tinyxml/tinyxmlparser.cpp
			src/tinyxml/tinyxml.h
)

#was commented out
 add_definitions(-DPLUGIN_USE_SVG)

set(EXTINCLUDE_DIR ${EXTINCLUDE_DIR} extinclude libs/ocpn-api/)

# ------------------------------------
#  PI_JSON SCHEMA VALIDATOR  
# ------------------------------------
if(APPLE)
    option(PI_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" OFF)
else(APPLE)
    option(PI_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" ON)
endif(APPLE)

if(PI_JSON_SCHEMA_VALIDATOR)
    message(STATUS "${CMLOC}Using JSON Schema validation - warning this changes the 'flavour' of the executable and it may not be compatible with OpenCPN")
    add_definitions(-DPI_JSON_SCHEMA_VALIDATOR)
    message(STATUS "${CMLOC}Adding JSON Schema validation")

    set(EXTSRC ${EXTSRC} extsrc/json-schema-validator/json-schema-draft7.json.cpp extsrc/json-schema-validator/json-uri.cpp extsrc/json-schema-validator/json-validator.cpp)
    set(EXTINCLUDE_DIR ${EXTINCLUDE_DIR} extinclude/nlohmann extinclude)
    #include_directories(BEFORE ${PROJECT_SOURCE_DIR}/extinclude/json-schema-validator)
else(PI_JSON_SCHEMA_VALIDATOR)
    message(STATUS "${CMLOC}Not using JSON Schema validation - warning invalid JSON 'may' cause unexpected crashes")
endif(PI_JSON_SCHEMA_VALIDATOR)

# ------------------------------------
#    End of JSON VALIDATOR
# ------------------------------------



## +++++++++Section above is added recently to Weatherfax set ++++++++++##
##++++++++++++RTLSDR Section directly from Weatherfax +++++++++##

IF(UNIX)
  find_library(RTLSDR_LIBRARY NAMES rtlsdr
    DOC "The rtlsdr library"
    )
  IF(RTLSDR_LIBRARY)
    MESSAGE (STATUS "${CMLOC}Found rtlsdr:  ${RTLSDR_LIBRARY}")
    ADD_DEFINITIONS( "-DBUILTIN_RTLAIS" )

    SET(EXTRA_LIBS ${EXTRA_LIBS} ${RTLSDR_LIBRARY})
  ELSE()
    MESSAGE (STATUS "")
    MESSAGE (STATUS "================ WARNING ===================")
    MESSAGE (STATUS "${CMLOC}rtlsdr library not found, cannot decode from rtlsdr")
    MESSAGE (STATUS "You may want to install librtlsdr-dev and run cmake again")
    MESSAGE (STATUS "============================================")
    MESSAGE (STATUS "")
  ENDIF()
ENDIF(UNIX)

      
IF(WIN32)
    IF(STANDALONE MATCHES "BUNDLED")
	# within OpenCPN tree
	INSTALL(FILES "../../buildwin/PVW32Con.exe" DESTINATION "plugins\\\\${PACKAGE_NAME}")
    ELSE()
	INSTALL(FILES "buildwin/PVW32Con.exe" DESTINATION "plugins\\\\${PACKAGE_NAME}")
    ENDIF()
ENDIF(WIN32)

##=============================================================
# Modify/configure 'Include' Directories below for the Plugin
##=============================================================
# EG  also inside cmake/Plugiin____.cmake files					


include_directories(BEFORE ${PROJECT_SOURCE_DIR}/${EXTINCLUDE_DIR})
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/ocpn-api)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wx/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wxJSON/include)

INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src/libaudiofile)
INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src/plugingl)

## Required to collect all the set( headers and SRCS  - Adjust as required
##==========================================================================
#ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE} ${SRC_PLUGINGL} ${SRC_TINYXML} )
##==========================================================================

target_sources(${PACKAGE_NAME} PUBLIC ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE} ${SRC_PLUGINGL} ${SRC_TINYXML})



TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${EXTRA_LIBS})
message(STATUS "${CMLOC}Adding PUGINS_LIBS: ${EXTRA_LIBS}")


if(NOT OCPN_FLATPAK_CONFIG)
    # Build environment not available when flatpak is being configured so following statements will not work
    message(STATUS "${CMLOC}Adding target link libraries to ${PACKAGE_NAME}")

    add_subdirectory(libs/tinyxml)
																 

    target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)
endif(NOT OCPN_FLATPAK_CONFIG)



##++++++++++Section above from Weatherfax located relative to where it was  ++++++++
##
## ----- Change below to match project requirements ----- ##
##

#if(WIN32)
#    if(MSVC)
#        target_link_libraries(${PACKAGE_NAME} ${PORTAUDIO_LIBS})
#    endif(MSVC)
#endif(WIN32)
# ADD_DEFINITIONS(-DTIXML_USE_STL)

# ---------------------------------------------
#  PI_JSON SCHEMA VALIDATOR  From Testplugin 
# ---------------------------------------------

# Every plugin that uses JSON for passing messages should use a validator. The validator  allows the system to ignore non-conformant messages. 
add_definitions(-DTIXML_USE_STL)

if(UNIX AND NOT APPLE AND NOT QT_ANDROID)
    if(PI_JSON_SCHEMA_VALIDATOR)
        # Change OS/ABI to be UNIX - System V to make it run with OCPN
        set(lib_name lib${PACKAGE_NAME}.so)
        message(STATUS "${CMLOC}lib-name: ${lib_name}")
        find_file(elfedit elfedit)
        if(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
            message(STATUS "${CMLOC}Will ensure OS/ABI is set to 0 to match OCPN")
            add_custom_command(
                TARGET ${PACKAGE_NAME}
                POST_BUILD
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                DEPENDS ${PACKAGE_NAME}
                COMMENT " Running post build action on ${lib_name}."
                COMMAND sh -c 'elfedit --input-osabi=Linux --output-osabi=none ${lib_name} || (echo 0) ')

        else(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
            message(STATUS "${CMLOC}Cannot correct OS/ABI to match OCPN if it is generated incorrectly. Package binutils needed")
        endif(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
    endif(PI_JSON_SCHEMA_VALIDATOR)
endif(UNIX AND NOT APPLE AND NOT QT_ANDROID)

# ---------------------------------------------
#    End of JSON VALIDATOR From Testplugin 
# ---------------------------------------------

## ----- Change above to match project requirements ----- ##


#IF(UNIX AND NOT APPLE)
#    INCLUDE("cmake/FindTinyXML.cmake")
#    FIND_PACKAGE(TinyXML QUIET)
#ENDIF(UNIX AND NOT APPLE)

#IF(TINYXML_FOUND)
#    message (STATUS "${CMLOC}Building with system tinyxml")
#    INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
##   ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE})
#    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_AUDIOFILE} ${HDRS} )
#    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
#ELSE(TINYXML_FOUND)
#    message (STATUS "${CMLOC}Building with embedded tinyxml")
#    SET(SRC_LTINYXML
#    src/tinyxml.h
#    src/tinystr.h
#      )
#   ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE})
#ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_PLUGINGL} ${SRC_TINYXML} ${SRC_AUDIOFILE} ${HDRS} )
#ENDIF(TINYXML_FOUND)

# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC})
#Add_Library (${PACKAGE_NAME} is used below. Cannot use same name.

#IF(NOT UNIX)
#    SET(SRC_BZIP	
#            src/bzip2/bzlib.c
#            src/bzip2/blocksort.c
#            src/bzip2/compress.c
#            src/bzip2/crctable.c
#            src/bzip2/decompress.c
#            src/bzip2/huffman.c
#            src/bzip2/randtable.c
#			src/bzip2/bzlib_private.h
#    )
#    ADD_LIBRARY(LIB_BZIP_WR STATIC ${SRC_BZIP})
#    TARGET_LINK_LIBRARIES( ${PACKAGE_NAME} LIB_BZIP_WR )
#    INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/bzip2)
#ENDIF(NOT UNIX)

#IF(NOT UNIX)
#    INCLUDE_DIRECTORIES(src/zlib-1.2.3)
#    INCLUDE_DIRECTORIES(src/bzip2)
#ENDIF(NOT UNIX)

#IF(WIN32)
#  IF(MSVC)
#    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/buildwin/include)
#    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} "${CMAKE_SOURCE_DIR}/buildwin/zlib1.lib")
#  ELSE()
#    # MINGW
##    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} "-lwxzlib-2.8")
#    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} "-lz")
#  ENDIF()
#ENDIF()

##================================================================================
#JSON Validator: Every plugin that uses JSON for passing messages should use a validator.  A JSON validator is much like an XML validator, the validator allows the system to ignore non-conformant messages.  The JSON validator changes the OSABI flavor of the Linux executable so there is code to change it back.

# IF(UNIX AND NOT APPLE)
#  IF(TP_JSON_SCHEMA_VALIDATOR)
#    # Change OS/ABI to be UNIX - System V to make it run with OCPN
#    SET(lib_name lib${PACKAGE_NAME}.so)
#    MESSAGE(STATUS "${CMLOC}lib-name: ${lib_name}")
#    FIND_FILE(elfedit elfedit)
#    IF(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
#      MESSAGE(STATUS "${CMLOC}Will ensure OS/ABI is set to 0 to match OCPN")
#      ADD_CUSTOM_COMMAND(
#        TARGET ${PACKAGE_NAME}
#        POST_BUILD
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#        DEPENDS ${PACKAGE_NAME}
#        COMMENT " Running post build action on ${lib_name}."
#        COMMAND sh -c 'elfedit --input-osabi=Linux --output-osabi=none ${lib_name} || (echo 0) '
#      )
#    ELSE(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
#      MESSAGE(STATUS "${CMLOC}Cannot correct OS/ABI to match OCPN if it is generated incorrectly. Package binutils needed")
#    ENDIF(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
#  ENDIF(TP_JSON_SCHEMA_VALIDATOR)
# ENDIF(UNIX AND NOT APPLE)
##================================================================================


# ----- Change above to match project requirements ----- ##
#--------REMOVE LN 194 to LN 296 from Testplugin_pi---------------
#------- Change below to match project requirements --------------#

##----Optional cmake files-----
##=======================================
# include("PluginNMEA") #not for wxfax
# include("PluginJSON")  #not for wxfax
include("PluginXML")
include("PluginGL")
##=======================================


## ----- do not change next section - needed to configure build process ----- ##
##=======================================
include("PluginInstall")
include("PluginLocalization")
include("PluginPackage")
##=======================================
## ----- do not change section above - needed to configure build process ----- ##

message(STATUS "${CMLOC}include directories: ")
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
    message(STATUS "${CMLOC}dir='${dir}'")
endforeach()

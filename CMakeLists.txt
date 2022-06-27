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
# Testplugin Version: 1.0.213.0 17/06/2022  DD/MM/YYYY  Flatpak work
#  buildCI:yes useODAPI:no validateJSON:no
# ---------------------------------------------------------------------------
## ----- When changing this file do NOT change the order in which sections occur        ----- ##
## ----- Changes should only be made between the section blocks that identify where     ----- ##
## ----- these changes should be. The whole configuration process relies on this        ----- ##
## ----- sequence to be successful                                                      ----- ##
##
## ----- Modify section below to include all the details for your plugin ----- ##

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.1)
if(COMMAND cmake_policy)
    if(POLICY CMP0043)
        cmake_policy(SET CMP0043 NEW)
    endif(POLICY CMP0043)
    cmake_policy(SET CMP0048 NEW)
# Need cmake 3.13 to implement this, but raspbian builds are at 3.7.2 and bionic at 3.10 and xenial builds at 3.5.1
#    cmake_policy(SET CMP0076 NEW)
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
set(SHORT_DESCRIPTION "Open fax image, decode audio fax, chart overlay")
set(LONG_DESCRIPTION "Open image files, decode audio fax to an image, calibrate chart overlay image. Convert images in mercator, polar, conic and uniform coordinates. Convert any image into a raster chart. Built in HF Radio Fax database for SSB and Internet retrieval from meterological sites. ")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "9")
set(VERSION_PATCH "40")
set(VERSION_TWEAK "0")
set(VERSION_DATE "27/06/2022")  # DD/MM/YYYY format
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")
set(TP_COMMENT "  * Release for O5 using CI")

set(PARENT "opencpn")

# The next line allows setup of a local web server with git for testing purposes.
#set(GIT_REPOSITORY_SERVER "github.com")

# Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod")   #Standard Repos
set(BETA "beta")   #Standard Repos
set(ALPHA "alpha") #Standard Repos

# Set if yourCloudsmith Base Repository name does not match your Git Repository name.
# set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Rick's customization
set(CLOUDSMITH_BASE_REPOSITORY "weatherfax")

# Set the following to use OpenCPN cloudsmith repositories instead of own repositories
set(CLOUDSMITH_USER "opencpn")
#set(CLOUDSMITH_USER "rick-gleason")

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

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

IF(PLUGIN_USE_SVG)
    MESSAGE(STATUS "${CMLOC}SVG support enabled...")
    IF(NOT QT_ANDROID)
        ADD_DEFINITIONS(-DPLUGIN_USE_SVG)
    ENDIF(NOT QT_ANDROID)
ENDIF(PLUGIN_USE_SVG)

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
            libs/ocpn-api/ocpn_plugin.h
)

# Needed otherwise - error LNK2019: unresolved external symbol
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


set(OCPNSRC
    ocpnsrc/cutil.cpp
    ocpnsrc/TexFont.cpp
)

set(OCPNHDRS
    ocpninclude/cutil.h
#    ocpninclude/ocpn_plugin.h
    ocpninclude/pluginmanager.h
    ocpninclude/shaders.h
    ocpninclude/TexFont.h
    ocpninclude/vector2D.h
    libs/ocpn-api/ocpn_plugin.h
)

set(EXTSRC
)

set(EXTINCLUDE
    extinclude/ODAPI.h
    extinclude/ODJSONSchemas.h
)

#is this needed? -Do not seem to need this for windows
set(LIBSSRC libs/wxJSON/src/jsonreader.cpp libs/wxJSON/src/jsonval.cpp libs/wxJSON/src/jsonwriter.cpp)

# Currently using cmake/PluginGL.cmake  Line 594
# See cmake/PluginXML.cmake  Line 593, Also now using cmakelists.txt  libs/tinyxml



#is this needed? windows seems to still work when commented out, still using pluginGL.cmake
set(LIBHDRS
    libs/GL/gl.h
    libs/GL/gl_private.h
    libs/GL/glext.h
    libs/GL/glu.h
    libs/wxJSON/include/json_defs.h
    libs/wxJSON/include/jsonreader.h
    libs/wxJSON/include/jsonval.h
    libs/wxJSON/include/jsonwriter.h
    )

add_definitions(-DPLUGIN_USE_SVG)

set(EXTINCLUDE_DIR ${EXTINCLUDE_DIR} extinclude libs/ocpn-api/)

## +++++++++Section above is added recently to Weatherfax set ++++++++++##
##++++++++++++RTLSDR Section directly from Weatherfax +++++++++##

#
# ----- If using JSON validation in plugin section below is needed ----- ##
#

# Not doing Json_schema_validation.

#
# ----- If using JSON validation in plugin section above is needed ----- ##
#

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

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpnsrc/glu/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/${EXTINCLUDE_DIR})
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/GL)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/ocpn-api)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wxJSON/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/libaudiofile)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/plugingl)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wxcurl/src)

#
# ----- Change below to match project requirements for android build ----- ##
#

if(QT_ANDROID)
    include_directories( ${PROJECT_SOURCE_DIR}/extinclude/GLES2)
endif(QT_ANDROID)


##
## ----- Change above to match project requirements for android build ----- ##
##

# Needed for android builds
if(QT_ANDROID)
    include_directories(BEFORE ${qt_android_include})
endif(QT_ANDROID)

## Required to collect all the set( headers and SRCS  - Adjust as required
##==========================================================================
target_sources(${PACKAGE_NAME} PUBLIC ${SRC_WEATHERFAX} ${HDRS} ${EXTSRC} ${SRC_AUDIOFILE})
##==========================================================================

TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${EXTRA_LIBS})
message(STATUS "${CMLOC}Adding PLUGINS_LIBS: ${EXTRA_LIBS}")

if(NOT OCPN_FLATPAK_CONFIG)
    # Build environment not available when flatpak is being configured so following statements will not work
    message(STATUS "${CMLOC}Adding target link libraries to ${PACKAGE_NAME}")

IF(NOT QT_ANDROID)
    add_subdirectory("libs/curl")
    target_link_libraries(${PACKAGE_NAME} libcurl::libcurl)

    add_subdirectory("libs/wxcurl")
    target_link_libraries(${PACKAGE_NAME} ocpn::wxcurl_src)
ENDIF(NOT QT_ANDROID)

    add_subdirectory(libs/tinyxml)
    target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)
endif(NOT OCPN_FLATPAK_CONFIG)

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/plugingl/include)

add_subdirectory(libs/plugingl)
target_link_libraries(${PACKAGE_NAME} ocpn::plugingl)

add_definitions(-DTIXML_USE_STL)

##++++++++++Section above from Weatherfax located relative to where it was  ++++++++
##
## ----- Change below to match project requirements ----- ##
##


#
# ----- If using JSON validation in plugin section below is needed ----- ##
#

# Not doing Json_schema_validation.

#
# ----- If using JSON validation in plugin section above is needed ----- ##
#

## ----- Change above to match project requirements ----- ##
##
## ----- do not change next section - needed to configure build process ----- ##
##



# Needed for all builds
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

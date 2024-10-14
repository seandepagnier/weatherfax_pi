##---------------------------------------------------------------------------
## Author:      Sean d'Epagnier
##***************************************************************************
## *   Copyright (C) 2010 by Sean d'Epagnier                              *
## *   License:     GPLv3+
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
# Frontend2 Author: Jon Gough Testplugin(TP)
# v1.0.309.0 14/10/2024 DD/MM/YYYY
# format buildCI:yes useODAPI:yes validateJSON:yes
# ---------------------------------------------------------------------------
## -- When changing this file do NOT change the order of the sections.    -- ##
## -- Make changes only between the section blocks identified             -- ##
## -- The whole configuration process relies on this sequence for success.-- ##
##
## -- Modify section below to include all the details for your plugin.    -- ##

set(CMLOC "CMakeLists: ")

# define minimum cmake version
cmake_minimum_required(VERSION 3.5.1)
if(COMMAND cmake_policy)
  if (POLICY CMP0043)
    cmake_policy(SET CMP0043 NEW)
  endif (POLICY CMP0043)
  cmake_policy(SET CMP0048 NEW)
  if (POLICY CMP0076)
    cmake_policy(SET CMP0076 NEW)
  endif (POLICY CMP0076)
  if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
  endif (POLICY CMP0077)
endif (COMMAND cmake_policy)

# set for vscode/vscodium
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

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
set(VERSION_PATCH "111")
set(VERSION_TWEAK "5")
set(VERSION_DATE  "14/10/24")#DD/MM/YYYY format
set(OCPN_MIN_VERSION "ov58")

set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "18")
set(TP_COMMENT "  * Release using CI")

set(PARENT "opencpn")

#Use to set up a local webserver with git installed.
#Default is github.com. Now set in pluginconfigure.cmake
#set(GIT_REPOSITORY_SERVER "github.com")

## Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod")   #Standard Repos
set(BETA "beta")   #Standard Repos
set(ALPHA "alpha") #Standard Repos

# Set if  Cloudsmith Base Repository name does not match your Git Repository name.
#set(CLOUDSMITH_BASE_REPOSITORY "${GIT_REPOSITORY_NAME}")
#set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Ricks's customization
set(CLOUDSMITH_BASE_REPOSITORY "weatherfax")   #without the _pi

# Set the following to use OpenCPN cloudsmith repositories instead of own repositories
set(CLOUDSMITH_USER "opencpn")
# set(CLOUDSMITH_USER "rick-gleason")

set(XML_INFO_URL "https://opencpn.org/OpenCPN/plugins/weatherfax.html")
set(XML_SUMMARY  ${SHORT_DESCRIPTION})
set(XML_DESCRIPTION  ${LONG_DESCRIPTION})

## ----- Modify section above to include all the details for your plugin ------- ##
## ----- Modify section below if there are special requirements for the plugin --##

option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY") # not operational
# For Weatherfax this must be set ON or it fails to compile
option(USE_GL "Enable OpenGL support" ON)
message(STATUS "${CMLOC}USE_GL: ${USE_GL}")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose the type of build" FORCE
  )
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(
  CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo"
                                  "MinSizeRel"
)
message(STATUS "${CMLOC}Build type: ${CMAKE_BUILD_TYPE}")

# ----- Statements below used to setup standard variables required by the CMAKE  -##
# ----- process -- Do not remove -##

project(${PACKAGE})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

##========================================================

include("PluginSetup")

##========================================================
## ----- Above sets up standard variables required by CMAKE - Do not remove -----##
## ----- Modify section below if there are special requirements for the plugin --##

set(CMAKE_VERBOSE_MAKEFILE "Activate verbose mode for make files" ON)

option(Plugin_CXX11 "Use c++11" OFF)

## ----- Modify section above if there are special requirements for the plugin --##
## ----- Do not change next section - needed to configure build process ---------##
##===========================================================

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

## --Below needed for build process.
##===========================================================

include("PluginConfigure")

##===========================================================
## ----- Do not change section above - needed to configure build process --------##
## ----- Change below to match project requirements for source, headers, etc. ---##

add_definitions(-DUSE_S57)

## ----- Section above - Add your project SET(SRCS and SET(HDRS  etc.-------##

SET(SRCS
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

add_definitions(-DPLUGIN_USE_SVG)

## ----- Section above - Add your project SET(SRCS and SET(HDRS  etc.-------##
## ----- If using JSON validation in plugin section below is needed --------##

# Do not need Json Schema Validation

## ----- If using JSON validation in plugin section above is needed --------##


##=============================================================
## Change as required to collect all the set( headers and SRCS
## This add_library  can be used in conditionals below.
##=============================================================

SET(SRC ${SRCS} ${HDRS}  ${SRC_AUDIOFILE})

##=============================================================

## ----- RTLSDR Section directly from Weatherfax ----- ##

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

## ----- End of RTLSDR Section directly from Weatherfax ----- ##

##=============================================================
## Modify/configure 'Include' Directories below for the Plugin
##=============================================================

include_directories(BEFORE ${PROJECT_SOURCE_DIR}/include)
include_directories(BEFORE ${PROJECT_SOURCE_DIR}/src/libaudiofile)

## Change target_sources to match your project needs.
##==============================================================

target_sources(${PACKAGE_NAME} PUBLIC ${SRC} )

##==============================================================

TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${EXTRA_LIBS})
message(STATUS "${CMLOC}Adding PLUGINS_LIBS: ${EXTRA_LIBS}")

if(NOT OCPN_FLATPAK_CONFIG)
    # Build environment not available when flatpak is being configured 
	# so following statements will not work
    message(STATUS "${CMLOC}Adding target link libraries to ${PACKAGE_NAME}")

    if (WIN32)
        add_subdirectory(opencpn-libs/WindowsHeaders)
        target_link_libraries(${PACKAGE_NAME} windows::headers)
    endif (WIN32)

    add_subdirectory(opencpn-libs/api-18)
    target_link_libraries(${PACKAGE_NAME} ocpn::api)

	IF(NOT QT_ANDROID)
    	add_subdirectory("opencpn-libs/curl")
    	target_link_libraries(${PACKAGE_NAME} ocpn::libcurl)

    	add_subdirectory("opencpn-libs/wxcurl")
    	target_link_libraries(${PACKAGE_NAME} ocpn::wxcurl)
 	ENDIF(NOT QT_ANDROID)

    add_subdirectory(opencpn-libs/tinyxml)
    target_link_libraries(${PACKAGE_NAME} ocpn::tinyxml)

    add_subdirectory(opencpn-libs/plugin_dc)
    target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

endif(NOT OCPN_FLATPAK_CONFIG)

add_definitions(-DTIXML_USE_STL)

##
## ----- If using JSON validation section below is needed ---------------##
##

if(UNIX AND NOT APPLE AND NOT QT_ANDRIOD)
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
endif(UNIX AND NOT APPLE AND NOT QT_ANDRIOD)


##------- Change below to match project requirements --------------------##

# Needed for android builds
if (QT_ANDROID)
  include_directories(BEFORE ${qt_android_include})
endif (QT_ANDROID)

## Needed for all builds Do not change - needed to build app -------------##
##===============================================================

include("PluginInstall")
include("PluginLocalization")
include("PluginPackage")

##================================================================
## ----- Do not change section above - needed to configure build process --##

message(STATUS "${CMLOC}include directories: ")
get_property(
  dirs
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY INCLUDE_DIRECTORIES
)
foreach (dir ${dirs})
  message(STATUS "${CMLOC}dir='${dir}'")
endforeach ()

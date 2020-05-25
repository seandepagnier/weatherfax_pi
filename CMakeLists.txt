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
# ---------------------------------------------------------------------------
# CMake-CI Number: 0.0.006  
#    - 2/19/2020 10 environments needed for beta testing.
#    - 3/30/2020 MacOS build fixed 12 env
#    - Reverse dates here
#    - 2/05/2020 MacOS final fix- JG#
#    - 23/05/2020 ARCH and CS Deploy and other 
# ---------------------------------------------------------------------------#
# ----- Modify section below to include all the details for your plugin ----- ##
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

SET(VERBOSE_NAME weatherfax)
# Search "getcommonname"in weather_routing_pi.cpp
set(COMMON_NAME "WeatherFax")
set(TITLE_NAME "WeatherFax")
set(PACKAGE_CONTACT "Sean d'Epagnier")
SET(PACKAGE "weatherfax_pi")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "9")
set(VERSION_PATCH "5")
set(VERSION_TWEAK "10")
set(VERSION_DATE "05/25/2020")
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")

set(PARENT "opencpn")

# The following lines are used to describe your git repository
#set(GIT_USER "seandepagnier")
#set(GIT_USER "jongough")
#set(GIT_USER "rgleason")
# Jon turned off
#set(GIT_REPOSITORY_NAME "weatherfax_pi")

# For setting up your own local webserver with git loaded, for testing purposes.
set(GIT_REPOSITORY_SERVER "github.com")

#set(CLOUDSMITH_USER "jon-gough")
#set(CLOUDSMITH_USER "sean-depagnier")
set(CLOUDSMITH_USER "rick-gleason")

# set if the cloudsmith repository name does not match your git repository name
#set(CLOUDSMITH_BASE_REPOSITORY "weatherfax_pi")  #Jon Gough's customization
#set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Sean's customization
#set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Rick's customization
set(CLOUDSMITH_BASE_REPOSITORY "weatherfax")  #Rick's customization
# Specifies Cloudsmith upload repository suffix for each catalog
set(PROD "prod") #Standard Repos
set(BETA "beta") #Standard Repos
set(ALPHA "alpha") #Standard Repos

# Set Plugin info-url, summary and description
set(XML_INFO_URL "https://opencpn.org/OpenCPN/plugins/weatherfax.html")
set(XML_SUMMARY "Open fax image files or decode audio faxes and overlay on chart")
set(XML_DESCRIPTION "Simple image overlay calibration, Converts any image image into a raster chart.  Db for HF radio fax stations via SSB and Metro URL internet retrieval.")

#set(APPVEYOR_TEST_DEPLOY_TO_CLOUDSMITH "true")

##
## ----- Modify above to include all the plugin details --------- ##
## ----- Modify below for plugin special requirements------------ ##
##

#------Settings
option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)
#------ End Settings

if(NOT ${PACKAGE} MATCHES "(.*)_pi")
  set(PACKAGE_NAME ${PACKAGE}_pi)
  set(PACKAGE_FILE_NAME "${PACKAGE}_pi")
else(NOT ${PACKAGE} MATCHES "(.*)_pi")
  set(PACKAGE_NAME ${PACKAGE})
  set(PACKAGE_FILE_NAME "${PACKAGE}")
endif(NOT ${PACKAGE} MATCHES "(.*)_pi")
string(TOUPPER "${PACKAGE}" TITLE_NAME)

project(${PACKAGE_NAME} VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK})
message(STATUS "PROJECT_VERSION: ${PROJECT_VERSION}")

set(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

message(STATUS "${VERBOSE_NAME} Version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}")
message(STATUS "OPCN_FLATPAK: ${OCPN_FLATPAK}")

#------Settings
# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")
# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)

option(USE_GL "Enable OpenGL support" OFF)
message(STATUS "USE_GL: ${USE_GL}")
#------End Settings

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(PKG_NVR ${PACKAGE_NAME}-${PROJECT_VERSION})
set(PKG_URL "https://dl.cloudsmith.io/public/--pkg_repo--/raw/names/--name--/versions/--version--/--filename--")

#===========================================================
INCLUDE("cmake/PluginSetup.cmake")
#===========================================================
if(DEFINED ENV{OCPN_TARGET})
    set(PACKAGING_NAME "${PKG_NVR}_${PKG_TARGET}-${ARCH}_${PKG_TARGET_VERSION}-$ENV{OCPN_TARGET}")
else(DEFINED ENV{OCPN_TARGET})
    set(PACKAGING_NAME "${PKG_NVR}_${PKG_TARGET}-${ARCH}_${PKG_TARGET_VERSION}")
endif(DEFINED ENV{OCPN_TARGET})
message(STATUS "PACKAGING_NAME: ${PACKAGING_NAME}")

set(PLUGIN_NAME ${PACKAGE}-plugin-${PKG_TARGET}-${ARCH}_${PKG_TARGET_VERSION})

# Settings and Options
set(CMAKE_VERBOSE_MAKEFILE ON)

option(Plugin_CXX11 "Use c++11" OFF)

# Check on compiler support
if(Plugin_CXX11)
  message(STATUS "Attempting to use c++11")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
  check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(Plugin_USE_CXX11 ON)
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    set(Plugin_USE_CXX11 ON)
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    set(Plugin_USE_CXX11 OFF)
  endif()
else()
  set(Plugin_USE_CXX11 OFF)
endif()

if(Plugin_USE_CXX11)
  message(STATUS "Allowing use of c++11")
endif()

message(STATUS "CMAKE version: ${CMAKE_VERSION}")
if(CMAKE_VERSION VERSION_GREATER 3.4)
  set(ENABLE_CLANG_TIDY
      OFF
      CACHE BOOL "Add clang-tidy automatically to builds")
  if(ENABLE_CLANG_TIDY)
    find_program(
      CLANG_TIDY_EXE
      NAMES "clang-tidy"
      PATHS /usr/local/opt/llvm/bin)
    if(CLANG_TIDY_EXE)
      message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
      # For more, see http://clang.llvm.org/extra/clang-tidy/ set(CLANG_TIDY_CHECKS "-*,modernize-*")
      set(CLANG_TIDY_CHECKS "-*,performance-*")
      set(CMAKE_CXX_CLANG_TIDY
          "${CLANG_TIDY_EXE};-checks=${CLANG_TIDY_CHECKS};-header-filter='${CMAKE_SOURCE_DIR}/*'"
          CACHE STRING "" FORCE)
    else()
      message(AUTHOR_WARNING "clang-tidy not found!")
      set(CMAKE_CXX_CLANG_TIDY
          ""
          CACHE STRING "" FORCE) # delete it
    endif()
  endif()
endif()

if(CMAKE_VERSION VERSION_GREATER 3.9)
  set(ENABLE_CPPCHECK
      OFF
      CACHE BOOL "Add cppcheck automatically to builds")
  if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE NAMES "cppcheck")
    set(CMAKE_CXX_CPPCHECK ${CPPCHECK_EXECUTABLE})
  endif()
endif()

## -------Section above is directly from Testplugin --------------#  
# ----- Modify section above for special plugin requirements----- ##
#======== Portaudio Section below direct from weatherfax_pi =======
IF (UNIX)
INCLUDE("cmake/FindPortaudio.cmake")
IF (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Found")
    INCLUDE_DIRECTORIES(BEFORE ${PORTAUDIO_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${PORTAUDIO_LIBRARIES})
    ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
    message(STATUS "In Unix:Found Portaudio")
ELSE (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Not Found...")
ENDIF (PORTAUDIO_FOUND)
ELSE (UNIX)
    INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/include)
    IF(STANDALONE MATCHES "BUNDLED")
	# within OpenCPN tree
    	SET(EXTRA_LIBS ${EXTRA_LIBS} ../../buildwin/portaudio_x86.lib)
    	INSTALL(FILES "../../buildwin/portaudio_x86.dll" DESTINATION ".")
    	message(STATUS "bundled: portaudio in ../../buildwin/portaudio_x86.dll")
    ELSE()
    	SET(EXTRA_LIBS ${EXTRA_LIBS} ${PROJECT_SOURCE_DIR}/buildwin/portaudio_x86.lib)
    	INSTALL(FILES "${PROJECT_SOURCE_DIR}/buildwin/portaudio_x86.dll" DESTINATION ".")
    	message(STATUS "Not bundled: portaudio in ${PROJECT_SOURCE_DIR}/buildwin/portaudio_x86.dll")
    ENDIF()

    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
    message(STATUS "Not in Unix:Setting USE_PORTAUDIO")
ENDIF (UNIX)

IF (MSVC)
    INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/include/msvc)
ENDIF (MSVC)

## --Section above is direct from Weatherfax located above SET(SRC_WEATHERFAX  ---##
## --Below needed for build process.
#===========================================================
INCLUDE("cmake/PluginConfigure.cmake")
#===========================================================
## --Above is needed for build process.
## ----- Change below to match your project requirements ----- ##


# ++++++++++++++Section below is direct from Weatherfax +++++++++##

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
            api-16/ocpn_plugin.h
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
			src/plugingl/linmath.h
			src/plugingl/pi_shaders.h
			src/plugingl/pidc.h
			src/plugingl/qtstylesheet.h
			src/plugingl/TexFont.h
			src/tinyxml/tinyxml.h
)			


SET(SRC_AUDIOFILE
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

## --------------------Section above is direct from Weatherfax -----------##
## --------------------Section below added recently to Weatherfax set -----##
#  not needed see PluginGL Line 594
SET (SRC_PLUGINGL
           src/plugingl/pi_shaders.cpp
			src/plugingl/pidc.cpp
			src/plugingl/qtstylesheet.cpp
			src/plugingl/TexFont.cpp
)

#  not needed see PluginXML Line 593
SET (SRC_TINYXML
           src/tinyxml/tinyxml.cpp
			src/tinyxml/tinyxmlerror.cpp
			src/tinyxml/tinyxmlparser.cpp
)

# add_definitions(-DPLUGIN_USE_SVG)

# ------------------------------------
#  TP_JSON SCHEMA VALIDATOR  From Testplugin 
# ------------------------------------
IF(APPLE)
    OPTION(TP_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" OFF)
ELSE(APPLE)
    OPTION(TP_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" ON)
ENDIF(APPLE)

IF(TP_JSON_SCHEMA_VALIDATOR)
    MESSAGE(STATUS "Using JSON Schema validation - warning this changes the 'flavour' of the executable and it may not be compatible with OpenCPN")
    ADD_DEFINITIONS(-DTP_JSON_SCHEMA_VALIDATOR)
    MESSAGE(STATUS "Adding JSON Schema validation")

    SET(EXTSRC
        ${EXTSRC}
        extsrc/json-schema-validator/json-schema-draft7.json.cpp
        extsrc/json-schema-validator/json-uri.cpp
        extsrc/json-schema-validator/json-validator.cpp
    )
    SET(EXTINCLUDE
        ${EXTINCLUDE}
        extinclude/nlohmann/json.hpp
        extinclude/json-schema-validator/json-schema.hpp
    )
    INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/extinclude/json-schema-validator)
ELSE(TP_JSON_SCHEMA_VALIDATOR)
    MESSAGE(STATUS "Not using JSON Schema validation - warning invalid JSON 'may' cause unexpected crashes")
ENDIF(TP_JSON_SCHEMA_VALIDATOR)
# ------------------------------------
#    End of JSON VALIDATOR From Testplugin 
# ------------------------------------



#INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src/libaudiofile)
#INCLUDE_DIRECTORIES (BEFORE ${PROJECT_SOURCE_DIR}/src/plugingl)

INCLUDE_DIRECTORIES (BEFORE ${PROJECT_SOURCE_DIR}/api-16)

## +++++++++Section above is added recently to Weatherfax set ++++++++++##
##++++++++++++RTLSDR Section directly from Weatherfax +++++++++##

IF(UNIX)
  find_library(RTLSDR_LIBRARY NAMES rtlsdr
    DOC "The rtlsdr library"
    )
  IF(RTLSDR_LIBRARY)
    MESSAGE (STATUS "Found rtlsdr:  ${RTLSDR_LIBRARY}")
    ADD_DEFINITIONS( "-DBUILTIN_RTLAIS" )

    SET(EXTRA_LIBS ${EXTRA_LIBS} ${RTLSDR_LIBRARY})
  ELSE()
    MESSAGE (STATUS "")
    MESSAGE (STATUS "================ WARNING ===================")
    MESSAGE (STATUS "rtlsdr library not found, cannot decode from rtlsdr")
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
INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src/libaudiofile)
INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src/plugingl)

#INCLUDE_DIRECTORIES( /src/libaudiofile )
#INCLUDE_DIRECTORIES( /src/libaudiofile/audiofile.h )
#INCLUDE_DIRECTORIES( ${PROJECT_SOURCE_DIR}/src/libaudiofile)
#INCLUDE_DIRECTORIES (src/plugingl)
#INCLUDE_DIRECTORIES(api-16)
# INCLUDE_DIRECTORIES (BEFORE ${PROJECT_SOURCE_DIR}/api-16)

##================================================================================
## Required to collect all the set( headers and SRCS  - Adjust as required
## This add_library  is used in the conditionals below.
##================================================================================
##    add_library(${PACKAGE_NAME} SHARED ${SRCS}  ${SRC_NMEA0183})
##================================================================================
# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE} )
# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE} ${SRC_PLUGINGL} ${SRC_TINYXML} )

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE})
TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${EXTRA_LIBS})
message(STATUS "Adding PUGINS_LIBS: ${EXTRA_LIBS}")

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

# ------------------------------------
#  TP_JSON SCHEMA VALIDATOR  From Testplugin 
# ------------------------------------

#JSON Validator: Every plugin that uses JSON for passing messages should use a validator.  A JSON validator is much like an XML validator, the validator allows the system to ignore non-conformant messages.  The JSON validator changes the OSABI flavor of the Linux executable so there is code to change it back.

IF(UNIX AND NOT APPLE)
  IF(TP_JSON_SCHEMA_VALIDATOR)
    # Change OS/ABI to be UNIX - System V to make it run with OCPN
    SET(lib_name lib${PACKAGE_NAME}.so)
    MESSAGE(STATUS "lib-name: ${lib_name}")
    FIND_FILE(elfedit elfedit)
    IF(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
      MESSAGE(STATUS "Will ensure OS/ABI is set to 0 to match OCPN")
      ADD_CUSTOM_COMMAND(
        TARGET ${PACKAGE_NAME}
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${PACKAGE_NAME}
        COMMENT " Running post build action on ${lib_name}."
        COMMAND sh -c 'elfedit --input-osabi=Linux --output-osabi=none ${lib_name} || (echo 0) '
      )
    ELSE(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
      MESSAGE(STATUS "Cannot correct OS/ABI to match OCPN if it is generated incorrectly. Package binutils needed")
    ENDIF(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
  ENDIF(TP_JSON_SCHEMA_VALIDATOR)
ENDIF(UNIX AND NOT APPLE)
# ------------------------------------
#    End of JSON VALIDATOR From Testplugin 
# ------------------------------------

## ----- Change above to match project requirements ----- ##



#IF(UNIX AND NOT APPLE)
#    INCLUDE("cmake/FindTinyXML.cmake")
#    FIND_PACKAGE(TinyXML QUIET)
#ENDIF(UNIX AND NOT APPLE)

#IF(TINYXML_FOUND)
#    message (STATUS "Building with system tinyxml")
#    INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
##   ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE})
#    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_AUDIOFILE} ${HDRS} )
#    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
#ELSE(TINYXML_FOUND)
#    message (STATUS "Building with embedded tinyxml")
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


# ----- Change above to match project requirements ----- ##
#--------REMOVE LN 194 to LN 296 from Testplugin_pi---------------
#------- Change below to match project requirements --------------#

#----INCLUDE CMAKE #3 & #4 -----
# INCLUDE("cmake/PluginNMEA.cmake") #not for wxfax
# INCLUDE("cmake/PluginJSON.cmake")  #not for wxfax
INCLUDE("cmake/PluginXML.cmake")
INCLUDE("cmake/PluginGL.cmake")
#
## ----- do not change next section - needed to configure build process ----- ##
#
#INCLUDE CMAKE #5,6 & 7 -----
include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")

#
## ----- do not change section above - needed to configure build process ----- ##
#

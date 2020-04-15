# ---------------------------------------------------------------------------
# Frontend2 Author: jongough aka. Jon Gough    License:wxWidgets License
# ---------------------------------------------------------------------------
##---------------------------------------------------------------------------
## Author:      Mauro Calvi
##***************************************************************************
## *   Copyright (C) 2010 by Mauro Calvi                               *
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
## *   xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx          *
## ***
##
## ----- Modify section below to include all the details for your plugin ----- ##
##

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

SET(VERBOSE_NAME Squiddio)
set(COMMON_NAME "squiddio")
set(TITLE_NAME "squiddio")
set(PACKAGE_CONTACT "Mauro Calvi")
SET(PACKAGE "squiddio")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "3")

SET(VERSION_PATCH "2")
set(VERSION_TWEAK "1")
set(VERSION_DATE "15/04/2020")
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")

set(PARENT "opencpn")
set(GIT_USER "mauroc")
#set(GIT_USER "jongough")
#set(GIT_USER "rgleason")
set(GIT_REPOSITORY_NAME "squiddio_pi")
message(STATUS "CIRCLECI: ${CIRCLECLI}, Env CIRCLECI: $ENV{CIRCLECI}")
message(STATUS "TRAVIS: ${TRAVIS}, Env TRAVIS: $ENV{TRAVIS}")

# This 'if' allows setup of a local webserver with git for testing
if($ENV{CIRCLECI})
    set(GIT_REPOSITORY_HOST "github.com")
    set(GIT_REPOSITORY_DIR "${GIT_USER}/")
    set(GIT_REPOSITORY_BRANCH "$ENV{CIRCLE_BRANCH}")
elseif($ENV{TRAVIS})
    set(GIT_REPOSITORY_HOST "github.com")
    set(GIT_REPOSITORY_DIR "${GIT_USER}/")
    set(GIT_REPOSITORY_BRANCH "$ENV{TRAVIS_BRANCH}")
else()
    set(GIT_REPOSITORY_HOST "github.com")
    set(GIT_REPOSITORY_DIR "${GIT_USER}/")
    set(GIT_REPOSITORY_BRANCH "master")
    #  set(GIT_REPOSITORY_HOST "git.eclipse.com.au")
    #  set(GIT_REPOSITORY_DIR "")
    #  set(GIT_REPOSITORY_BRANCH "cmake_flatpak_test")
endif()
message(STATUS "Git Branch: ${GIT_REPOSITORY_BRANCH}")

# Prod builds repository url
set(PROD "pi")   #Mauro Repos
#set(PROD "prod")  #Standard Repos

# Beta builds repository url
set(BETA "stable")  #Mauro Repos
#set(BETA "beta")       #Standard Repos

# Alpha builds repository url
set(ALPHA "manual") # for Mauro's  Cloudsmith 'mauro-calvi/squiddio-manual' 
#set(ALPHA "alpha") #Standard Repos

#set(CLOUDSMITH_BASE_REPOSITORY "${GIT_REPOSITORY_NAME}")
set(CLOUDSMITH_BASE_REPOSITORY "squiddio")  # Mauro's customization without the '_pi'
#set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Rick's customization

set(CLOUDSMITH_USER "mauro-calvi")
#set(CLOUDSMITH_USER "jon-gough")
#set(CLOUDSMITH_USER "rick-gleason")

set(XML_INFO_URL "https://opencpn.org/OpenCPN/plugins/squiddio.html")
set(XML_SUMMARY "Plugin to display interesting objects")
set(XML_DESCRIPTION "Plugin to display harbours, bouys, interesting locations etc. on the chart")
#set(APPVEYOR_TEST_DEPLOY_TO_CLOUDSMITH "true")

##
## ----- Modify section above to include all the details for your plugin ----- ##
##

##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

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

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")
# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)
option(USE_GL "Enable OpenGL support" OFF)
message(STATUS "USE_GL: ${USE_GL}")

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# set(PKG_NVR ${PACKAGE_NAME}-${PROJECT_VERSION}-${OCPN_MIN_VERSION}-${OCPN_API_VERSION_MAJOR}.${OCPN_API_VERSION_MINOR})
set(PKG_NVR ${PACKAGE_NAME}-${PROJECT_VERSION})
set(PKG_URL "https://dl.cloudsmith.io/public/--pkg_repo--/raw/names/--name--/versions/--version--/--filename--")

include("cmake/PluginSetup.cmake")

if(DEFINED ENV{OCPN_TARGET})
    set(PACKAGING_NAME "${PKG_NVR}_${PKG_TARGET}-${PKG_TARGET_VERSION}-$ENV{OCPN_TARGET}")
else(DEFINED ENV{OCPN_TARGET})
    set(PACKAGING_NAME "${PKG_NVR}_${PKG_TARGET}-${PKG_TARGET_VERSION}")
endif(DEFINED ENV{OCPN_TARGET})
message(STATUS "PACKAGING_NAME: ${PACKAGING_NAME}")

set(PLUGIN_NAME ${PACKAGE}-plugin-${PKG_TARGET}-${PKG_TARGET_VERSION})

set(CMAKE_VERBOSE_MAKEFILE ON)

option(Plugin_CXX11 "Use c++11" OFF)

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

##
## ----- Modify section above if there are special requirements for the plugin ----- ##
##


INCLUDE("cmake/PluginConfigure.cmake")

##
## ----- Change below to match your project requirements ----- ##
## ------These lines are only for Testplugin - do not use ------------##
##
##   REMOVED
##
## ------The lines above are only for Testplugin -do not use------------##
##
# ----Remember to Add your project Set(SRCS and set(HDRS
##----------Below From Squiddio for SET( Headers and SRCS) ------------------
SET(SRCS
    src/squiddio_pi.h
    src/squiddio_pi.cpp
    src/squiddioPrefsDialogBase.cpp
    src/squiddioPrefsDialogBase.h
    src/squiddio_pi_thread.h
    src/squiddio_pi_thread.cpp
    src/squiddioKaps.cpp
    src/sJSON.cpp
    src/sJSON.h
    src/icons.h
    src/icons.cpp
    src/logs.cpp
    src/logs.h
    src/NavObjectCollection.h
    src/NavObjectCollection.cpp
    src/Poi.h
    src/Poi.cpp
    src/PoiMan.h
    src/PoiMan.cpp
    src/Layer.h
    src/pugixml.hpp
    src/pugixml.cpp
    src/ocpn_types.h
    src/pugiconfig.hpp
    src/Hyperlink.h
    src/gpxdocument.h
    src/wxJSON/jsonval.cpp
    src/wxJSON/jsonreader.cpp
    src/wxJSON/json_defs.h
    src/wxJSON/jsonreader.h
    src/wxJSON/jsonval.h
    src/wxJSON/jsonwriter.cpp
    src/wxJSON/jsonwriter.h
  )

SET(SRC_NMEA0183
    src/nmea0183/LatLong.hpp
    src/nmea0183/latlong.cpp
    src/nmea0183/long.cpp
    src/nmea0183/nmea0183.cpp
    src/nmea0183/nmea0183.hpp
    src/nmea0183/Response.hpp
    src/nmea0183/response.cpp
    src/nmea0183/RMB.hpp
    src/nmea0183/rmb.cpp
    src/nmea0183/Sentence.hpp
    src/nmea0183/sentence.cpp
    src/nmea0183/talkerid.cpp
    src/nmea0183/RMC.HPP
    src/nmea0183/rmc.cpp
    src/nmea0183/hexvalue.cpp
    src/nmea0183/lat.cpp
    src/nmea0183/expid.cpp
    src/nmea0183/wpl.hpp
    src/nmea0183/wpl.cpp
    src/nmea0183/rte.hpp
    src/nmea0183/rte.cpp
    src/nmea0183/hdt.hpp
    src/nmea0183/hdt.cpp
    src/nmea0183/hdg.hpp
    src/nmea0183/hdg.cpp
    src/nmea0183/hdm.hpp
    src/nmea0183/hdm.cpp
    src/nmea0183/gll.hpp
    src/nmea0183/gll.cpp
    src/nmea0183/vtg.hpp
    src/nmea0183/vtg.cpp
    src/nmea0183/gga.hpp
    src/nmea0183/gga.cpp
    src/nmea0183/gsv.hpp
    src/nmea0183/gsv.cpp
    src/nmea0183/dbt.cpp
    src/nmea0183/dbt.hpp
    src/nmea0183/dpt.cpp
    src/nmea0183/dpt.hpp
    src/nmea0183/mtw.cpp
    src/nmea0183/mtw.hpp
    src/nmea0183/mwv.cpp
    src/nmea0183/mwv.hpp
    src/nmea0183/vhw.hpp
    src/nmea0183/vhw.cpp
    src/nmea0183/vwr.cpp
    src/nmea0183/vwr.hpp
    src/nmea0183/zda.cpp
    src/nmea0183/zda.hpp
    src/nmea0183/rsa.hpp
    src/nmea0183/rsa.cpp
    src/nmea0183/SatInfo.h
    src/nmea0183/mwd.cpp
    src/nmea0183/mwd.hpp
    src/nmea0183/vwt.cpp
    src/nmea0183/vwt.hpp
    src/nmea0183/mta.cpp
    src/nmea0183/mta.hpp
    src/nmea0183/vlw.cpp
    src/nmea0183/vlw.hpp
    src/nmea0183/mda.cpp
    src/nmea0183/mda.hpp
  )
INCLUDE_DIRECTORIES(nmea0183)
INCLUDE_DIRECTORIES(src/wxJSON)
INCLUDE_DIRECTORIES(api-16)


##----------Above From Squiddio ------------------

##=============REMOVE THESE LINES LEFT FROM TESTPLUGIN ================
#add_definitions(-DUSE_S57)
#set(SRCS
#    src/tpdc.cpp
#    ...
#    src/tpUtils.cpp)
#
#set(HDRS
#    include/testplugin_pi.h
#    ...
#    include/tpUtils.h)
#
#
#set(OCPNSRC ocpnsrc/TexFont.cpp)
#set(OCPNHDRS ocpninclude/ocpn_plugin.h ocpninclude/pluginmanager.h ocpninclude/TexFont.h)
#set(EXTSRC)
#set(EXTINCLUDE
#    extinclude/nlohmann/json.hpp
#    extinclude/ODAPI.h)
#set(LIBSSRC libs/wxJSON/src/jsonreader.cpp libs/wxJSON/src/jsonval.cpp libs/wxJSON/src/jsonwriter.cpp)
#set(LIBHDRS
#    libs/GL/include/gl.h
#    ...
#    libs/wx/include/jsonwriter.h)
#
# add_definitions(-DPLUGIN_USE_SVG)
#
#IF(APPLE)
#    OPTION(TP_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" OFF)
#ELSE(APPLE)
#    OPTION(TP_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" ON)
#ENDIF(APPLE)

#IF(TP_JSON_SCHEMA_VALIDATOR)
#...
#ENDIF(TP_JSON_SCHEMA_VALIDATOR)

#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude)
#...
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpnsrc/bitmaps)

#IF(UNIX AND NOT APPLE)
#  IF(TP_JSON_SCHEMA_VALIDATOR)
#  ... 
#  ENDIF(TP_JSON_SCHEMA_VALIDATOR)
#ENDIF(UNIX AND NOT APPLE)
## ========== REMOVE Lines above left over from TestPlugin ==================


##
## ----- Change below to match project requirements ----- ##
##
##================================================================================
## Required to collect all the set( headers and SRCS  - Adjust as required
## This add_library  is used in the conditionals below.
##================================================================================
##    add_library(${PACKAGE_NAME} SHARED ${SRCS}  ${SRC_NMEA0183})
##================================================================================

ADD_DEFINITIONS(-DTIXML_USE_STL)
IF(UNIX AND NOT APPLE)
    INCLUDE("cmake/FindTinyXML.cmake")
    FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX AND NOT APPLE)

IF(TINYXML_FOUND)
    message (STATUS "Building with system tinyxml")
    INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRCS} ${SRC_NMEA0183})
    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
ELSE(TINYXML_FOUND)
    message (STATUS "Building with embedded tinyxml")
    SET(SRC_LTINYXML
    src/tinyxml.h
    src/tinystr.h
      )
    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRCS} ${SRC_NMEA0183} ${SRC_LTINYXML})
ENDIF(TINYXML_FOUND)


##
## ----- Change above to match project requirements ----- ##
##

##
## ----- Do not change - needed to build app ----- ##
##

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")

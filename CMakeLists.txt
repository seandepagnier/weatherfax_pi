# ---------------------------------------------------------------------------
# Frontend2 Author: jongough aka. Jon Gough    License:wxWidgets License
## CMakeLists and Frontend
## Author: Jon Gough
## Version: 0.0.3
## Date: 04-09-2020
##------------------------------------------------------------------------##
## OpenCPN Plugin: Weatherfax_pi
## Author:Sean d'Epagnier 
## License:wxWidgets License
## License: GPLV3 
##-------------------------------------------------------------------------##
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
# ====FRONTEND 2==============
## 
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

SET(VERBOSE_NAME weatherfax)
set(COMMON_NAME "Weatherfax")
set(TITLE_NAME "WeatherFax")
set(PACKAGE_CONTACT "Sean d'Epagnier")
SET(PACKAGE "weatherfax")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "9")
set(VERSION_PATCH "4")
set(VERSION_TWEAK "2")
set(VERSION_DATE "04/24/2020")
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")

set(PARENT "opencpn")
#set(GIT_USER "mauroc")
#set(GIT_USER "jongough")
set(GIT_USER "rgleason")
set(GIT_REPOSITORY_NAME "weatherfax_pi")
message(STATUS "CIRCLECI: ${CIRCLECLI}, Env CIRCLECI: $ENV{CIRCLECI}")
message(STATUS "TRAVIS: ${TRAVIS}, Env TRAVIS: $ENV{TRAVIS}")

# Permits setup of a local webserver with git for testing
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

#set(CLOUDSMITH_USER "jon-gough")
set(CLOUDSMITH_USER "rick-gleason")

#set(CLOUDSMITH_BASE_REPOSITORY "${GIT_REPOSITORY_NAME}")
set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")  #Rick's customization

# Set Cloudsmith upload repository suffix for each catalog
set(PROD "prod") #Standard Repos
set(BETA "beta") #Standard Repos
set(ALPHA "alpha") #Standard Repos

set(XML_INFO_URL "https://opencpn.org/OpenCPN/plugins/weatherfax.html")
set(XML_SUMMARY "Open fax image files or decode audio faxes and overlay on chart")
set(XML_DESCRIPTION "Simple calibration for image overly, Converts images, and can convert any image into a raster chart.  Db for HF radio fax stations via SSB and Metro URL internet retrieval.")

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


#===========================================================
INCLUDE("cmake/PluginSetup.cmake")
#===========================================================
if(DEFINED ENV{OCPN_TARGET})
    set(PACKAGING_NAME "${PKG_NVR}_${PKG_TARGET}-${PKG_TARGET_VERSION}-$ENV{OCPN_TARGET}")
else(DEFINED ENV{OCPN_TARGET})
    set(PACKAGING_NAME "${PKG_NVR}_${PKG_TARGET}-${PKG_TARGET_VERSION}")
endif(DEFINED ENV{OCPN_TARGET})
message(STATUS "PACKAGING_NAME: ${PACKAGING_NAME}")


##=============================================================
# Modify/configure 'Include' Directories below for the Plugin
##=============================================================

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

#
# -------Section above is directly from Testplugin --------------##  
# ----- Modify section above if there are special requirements for the plugin ----- #
#


#===========================================================
INCLUDE("cmake/PluginConfigure.cmake")
#===========================================================

#
# ----- Change below to match your project requirements ----- ##
# ------These lines are only for Testplugin - do not use ------------##
#
#   REMOVED
#
# ------The lines above are only for Testplugin -do not use------------##
#++++++++Weatherfax Specific Files ++++++++++++++##


IF (UNIX)
INCLUDE("cmake/FindPortaudio.cmake")
IF (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Found")
    INCLUDE_DIRECTORIES(BEFORE ${PORTAUDIO_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${PORTAUDIO_LIBRARIES})
    ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ELSE (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Not Found...")
ENDIF (PORTAUDIO_FOUND)
ELSE (UNIX)
    INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/include)
    IF(STANDALONE MATCHES "BUNDLED")
        # within OpenCPN tree
        SET(EXTRA_LIBS ${EXTRA_LIBS} ../../buildwin/portaudio_x86)
    	INSTALL(FILES "../../buildwin/portaudio_x86.dll" DESTINATION ".")
    ELSE()
        SET(EXTRA_LIBS ${EXTRA_LIBS} ../buildwin/portaudio_x86)
    	INSTALL(FILES "buildwin/portaudio_x86.dll" DESTINATION ".")
    	message(STATUS "Not Bundled: Portaudio")
    ENDIF()

    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ENDIF (UNIX)

IF (MSVC)
    INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/include/msvc)
ENDIF (MSVC)

## --Section above is direct from Weatherfax located above SET(SRC_WEATHERFAX  ---##
##
## ++++++++++++++Section below is direct from Weatherfax +++++++++##
  
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
			src/ocpn_plugin.h
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


## +++++++++Section above is added recently to Weatherfax set ++++++++++##
##++++++++++++Section below directly from Weatherfax +++++++++##

IF(UNIX)
  find_library(RTLSDR_LIBRARY NAMES rtlsdr
    DOC "The rtlsdr library"
    )
  IF(RTLSDR_LIBRARY)
    MESSAGE (STATUS "Found rtlsdr:  ${RTLSDR_LIBRARY}")
    ADD_DEFINITIONS( "-DBUILTIN_RTLAIS" )

    SET(PLUGINS_LIBS ${PLUGINS_LIBS} ${RTLSDR_LIBRARY})
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

INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/src/libaudiofile)
MESSAGE(STATUS "PLUGINS_LIBS: ${PLUGINS_LIBS}")

# INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/src/include)   #this does not seem to work. Need to include  include/GL
# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_AUDIOFILE} ${SRC_PLUGINGL} ${SRC_TINYXML} ${HDRS} )
# ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_AUDIOFILE} ${HDRS} )
#TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${EXTRA_LIBS})

##
##++++++++++Section above from Weatherfax located relative to where it was  ++++++++
##
##
# ----Remember to Add your project Set(SRCS and set(HDRS
##----------Below From Squiddio for SET( Headers and SRCS) ------------------


# INCLUDE_DIRECTORIES(nmea0183)
# INCLUDE_DIRECTORIES(src/wxJSON)
INCLUDE_DIRECTORIES(api-16)


##----------Above From Squiddio ------------------

##=============REMOVE THESE LINES LEFT FROM TESTPLUGIN ================

# add_definitions(-DPLUGIN_USE_SVG)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpnsrc/bitmaps)
#add_library(${PACKAGE_NAME} SHARED ${SRCS} ${HDRS} ${OCPNSRC} ${EXTSRC} ${LIBSSRC})
#add_definitions(-DTIXML_USE_STL)

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

INCLUDE_DIRECTORIES (src/plugingl)

# add_library(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE} ${SRC_PLUGINGL} ${SRC_TINYSML})

ADD_DEFINITIONS(-DTIXML_USE_STL)
IF(UNIX AND NOT APPLE)
    INCLUDE("cmake/FindTinyXML.cmake")
    FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX AND NOT APPLE)

IF(TINYXML_FOUND)
    message (STATUS "Building with system tinyxml")
    INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
#   ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE})
    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_AUDIOFILE} ${HDRS} )
    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
ELSE(TINYXML_FOUND)
    message (STATUS "Building with embedded tinyxml")
    SET(SRC_LTINYXML
    src/tinyxml.h
    src/tinystr.h
      )
#   ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${HDRS} ${SRC_AUDIOFILE})
    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_PLUGINGL} ${SRC_TINYXML} ${SRC_AUDIOFILE} ${HDRS} )

ENDIF(TINYXML_FOUND)

if(WIN32)
    if(MSVC)
        target_link_libraries(${PACKAGE_NAME} ${PORTAUDIO_LIBS})
    endif(MSVC)
endif(WIN32)

# ----- Change above to match project requirements ----- ##
#
# ++++++++++ Includes from weatherfax cmake  +++++++++++++## 
#INCLUDE("cmake/PluginNMEA.cmake")   #No not for wxfax
INCLUDE("cmake/PluginJSON.cmake")#
INCLUDE("cmake/PluginXML.cmake")
INCLUDE("cmake/PluginGL.cmake")


#
# ----- Do not change - needed to build app ----- ##
#

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")

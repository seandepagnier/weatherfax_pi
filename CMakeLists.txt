##--------------------------------------------------------------------------##
## OpenCPN Plugin: Weatherfax_pi
## Author:      Sean d'Epagnier  Copyright: License:     wxWidgets License
## License: GPLV3 
##-------------------------------------------------------------------------##
## CMakeLists and Frontend
## Author: Jon Gough
## Version: 0.0.1
## Date: 03-21-2020
## -------------------------------------------------------------------------##
##
## ----- Modify section below to include all the details for your plugin -- ##
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

set(PACKAGE_NAME "Weatherfax_pi")
set(VERBOSE_NAME "WeatherFax")
set(COMMON_NAME "WeatherFax")
set(TITLE_NAME "WEATHERFAX")
set(PACKAGE_CONTACT "Sean d'Epagnier")

set(VERSION_MAJOR "1")
set(VERSION_MINOR "9")
set(VERSION_PATCH "4")
set(VERSION_TWEAK "1")
set(VERSION_DATE "01/012020")
set(OCPN_MIN_VERSION "ov50")
set(OCPN_API_VERSION_MAJOR "1")
set(OCPN_API_VERSION_MINOR "16")
set(TP_COMMENT "  * Release for O5 using CI")

set(PARENT "opencpn")
set(PACKAGE "weatherfax")
#set(GIT_USER "jongough")
set(GIT_USER "rgleason")
set(GIT_REPOSITORY_NAME "weatherfax_pi")
message(STATUS "CIRCLECI: ${CIRCLECLI}, Env CIRCLECI: $ENV{CIRCLECI}")
message(STATUS "TRAVIS: ${TRAVIS}, Env TRAVIS: $ENV{TRAVIS}")
# This if allows for setting up your own local webserver with git on it for testing purposes
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
set(CLOUDSMITH_BASE_REPOSITORY "opencpn-plugins")

set(XML_INFO_URL "https://opencpn.org/OpenCPN/plugins/weatherfax.html")
set(XML_SUMMARY "Open fax image files or decode audio faxes and overlay on chart")
set(XML_DESCRIPTION "Weatherfax Plugin has simple calibration for image overly, Converts images, and can convert any image into a raster chart.  Db for HF radio fax stations via SSB and Metro URL internet retrieval.")
set(APPVEYOR_TEST_DEPLOY_TO_CLOUDSMITH "true")
option(PLUGIN_USE_SVG "Use SVG graphics" ON)

set(CMAKE_CXX_STANDARD 11)

# Prefer libGL.so to libOpenGL.so, see CMP0072
set(OpenGL_GL_PREFERENCE "LEGACY")

##
## ----- Modify section above to include all the details for your plugin ----- ##
##

##
## ----- Modify section below if there are special requirements for the plugin ----- ##
##

set(PACKAGE_NAME ${PACKAGE}_pi)
set(PACKAGE_FILE_NAME "${PARENT}-plugin-${PACKAGE}")
string(TOUPPER "${PACKAGE}" TITLE_NAME)

project(${PACKAGE_NAME} VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK})

set(PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

message(STATUS "${VERBOSE_NAME} Version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}")
message(STATUS "OPCN_FLATPAK: ${OCPN_FLATPAK}")

# Don't use local version of GLU library
set(USE_LOCAL_GLU FALSE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif("${CMAKE_BUILD_TYPE}" STREQUAL "")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(PKG_NVR ${PACKAGE_NAME}-${PROJECT_VERSION}-${OCPN_MIN_VERSION}-${OCPN_API_VERSION_MAJOR}.${OCPN_API_VERSION_MINOR})
set(PKG_URL "https://dl.cloudsmith.io/public/--pkg_repo--/raw/names/--name--/versions/--version--/--filename--")

include("cmake/PluginSetup.cmake")

set(PLUGIN_NAME ${PACKAGE}-plugin-${PKG_TARGET}-${PKG_TARGET_VERSION})

set(CMAKE_VERBOSE_MAKEFILE ON)

option(OD_CXX11 "Use c++11" OFF)

if(OD_CXX11)
  message(STATUS "Attempting to use c++11")
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
  check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(Testplugin_USE_CXX11 ON)
  elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    set(Testplugin_USE_CXX11 ON)
  else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    set(Testplugin_USE_CXX11 OFF)
  endif()
else()
  set(Testplugin_USE_CXX11 OFF)
endif()

if(Testplugin_USE_CXX11)
  message(STATUS "Allowing use of c++11")
  add_definitions(-DTestplugin_USE_CXX11)
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


include("cmake/PluginConfigure.cmake")

##
## ----- Change below to match project requirements ----- ##
##

add_definitions(-DUSE_S57)


##-----------Weatheerfax Specific Files --------------------##

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

SET (SRC_PLUGINGL
            src/plugingl/pi_shaders.cpp
			src/plugingl/pidc.cpp
			src/plugingl/qtstylesheet.cpp
			src/plugingl/TexFont.cpp
)

SET (SRC_TINYXML
            src/tinyxml/tinyxml.cpp
			src/tinyxml/tinyxmlerror.cpp
			src/tinyxml/tinyxmlparser.cpp
)



SET (HDRS
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
#			src/wxWTranslateCatalog.h
			
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

INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/src/libaudiofile)

ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_AUDIOFILE} ${SRC_PLUGINGL} ${SRC_TINYXML} ${HDRS} )

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



TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${EXTRA_LIBS})


##-----------Testplugin Specific Files --------------------##

#set(SRCS
#    src/tpdc.cpp
#    src/tpControlDialogDef.cpp
#    src/tpControlDialogImpl.cpp
#    src/tpicons.cpp
#    src/tpJSON.cpp
#    src/testplugin_pi.cpp
#    src/tpUtils.cpp)

#set(HDRS
#    include/testplugin_pi.h
#    include/tpicons.h
#    include/tpControlDialogDef.h
#    include/tpControlDialogImpl.h
#    include/tpIconCombo.h
#    include/tpJSON.h
#    include/tpdc.h
#    include/tpUtils.h)

#set(OCPNSRC ocpnsrc/TexFont.cpp)
#set(OCPNHDRS ocpninclude/ocpn_plugin.h ocpninclude/pluginmanager.h #ocpninclude/TexFont.h)

#set(EXTSRC)

#set(EXTINCLUDE
#    extinclude/nlohmann/json.hpp
#    extinclude/ODAPI.h)

#set(LIBSSRC libs/wxJSON/src/jsonreader.cpp libs/wxJSON/src/jsonval.cpp libs/wxJSON/src/jsonwriter.cpp)

#set(LIBHDRS
#    libs/GL/include/gl.h
#    libs/GL/include/gl_private.h
#    libs/GL/include/glext.h
#    libs/GL/include/glu.h
#    libs/wx/include/json_defs.h
#    libs/wx/include/jsonreader.h
#    libs/wx/include/jsonval.h
#    libs/wx/include/jsonwriter.h)

add_definitions(-DPLUGIN_USE_SVG)

##
##---- Section needed for Testplugin operations -----------------##
##

#IF(APPLE)
#    OPTION(TP_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" OFF)
#ELSE(APPLE)
#    OPTION(TP_JSON_SCHEMA_VALIDATOR "Use JSON Schema validator" ON)
#ENDIF(APPLE)

#IF(TP_JSON_SCHEMA_VALIDATOR)
#    MESSAGE(STATUS "Using JSON Schema validation - warning this changes the 'flavour' of the executable and it may not be compatible with OpenCPN")
#    ADD_DEFINITIONS(-DTP_JSON_SCHEMA_VALIDATOR)
#    MESSAGE(STATUS "Adding JSON Schema validation")

#    SET(EXTSRC
#        ${EXTSRC}
#        extsrc/json-schema-validator/json-schema-draft7.json.cpp
#        extsrc/json-schema-validator/json-uri.cpp
#        extsrc/json-schema-validator/json-validator.cpp
#    )
#    SET(EXTINCLUDE
#        ${EXTINCLUDE}
#        extinclude/nlohmann/json.hpp
#        extinclude/json-schema-validator/json-schema.hpp
#    )
#    INCLUDE_DIRECTORIES(BEFORE ${PROJECT_SOURCE_DIR}/extinclude/json-schema-validator)
#ELSE(TP_JSON_SCHEMA_VALIDATOR)
#    MESSAGE(STATUS "Not using JSON Schema validation - warning invalid JSON 'may' cause unexpected crashes")
#ENDIF(TP_JSON_SCHEMA_VALIDATOR)

#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/extinclude)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/libs/wx/include)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude/wxcurl)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpninclude/wxcurl/include)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpnsrc/lz4)
#include_directories(BEFORE ${PROJECT_SOURCE_DIR}/ocpnsrc/bitmaps)

#add_library(${PACKAGE_NAME} SHARED ${SRCS} ${HDRS} ${OCPNSRC} ${EXTSRC} ${LIBSSRC})

add_definitions(-DTIXML_USE_STL)

#IF(UNIX AND NOT APPLE)
#  IF(TP_JSON_SCHEMA_VALIDATOR)
#    # Change OS/ABI to be UNIX - System V to make it run with OCPN
#    SET(lib_name lib${PACKAGE_NAME}.so)
#    MESSAGE(STATUS "lib-name: ${lib_name}")
#    FIND_FILE(elfedit elfedit)
#    IF(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
#      MESSAGE(STATUS "Will ensure OS/ABI is set to 0 to match OCPN")
#      ADD_CUSTOM_COMMAND(
#        TARGET ${PACKAGE_NAME}
#        POST_BUILD
#        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#        DEPENDS ${PACKAGE_NAME}
#        COMMENT " Running post build action on ${lib_name}."
#        COMMAND sh -c 'elfedit --input-osabi=Linux --output-osabi=none ${lib_name} || (echo 0) '
#      )
#    ELSE(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
#      MESSAGE(STATUS "Cannot correct OS/ABI to match OCPN if it is generated incorrectly. Package binutils needed")
#    ENDIF(NOT (${elfedit} STREQUAL "elfedit-NOTFOUND"))
#  ENDIF(TP_JSON_SCHEMA_VALIDATOR)
#ENDIF(UNIX AND NOT APPLE)

##
## ----- Change above to match project requirements ----- ##
## ------ Includes from weatherfax cmake------------------## 

INCLUDE("cmake/PluginXML.cmake")
INCLUDE("cmake/PluginGL.cmake")

##
## ----- do not change - needed to build app ----- ##
##

include("cmake/PluginInstall.cmake")
include("cmake/PluginLocalization.cmake")
include("cmake/PluginPackage.cmake")

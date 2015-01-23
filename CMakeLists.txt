##---------------------------------------------------------------------------
## Author:      Sean D'Epagnier
## Copyright:   
## License:     GPLv3
##---------------------------------------------------------------------------
 
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(weatherfax_pi)
SET(PACKAGE_NAME weatherfax_pi)
SET(VERBOSE_NAME WeatherFax)
SET(TITLE_NAME WEATHERFAX)
SET(CPACK_PACKAGE_CONTACT "Sean D'Epagnier ")

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "3")

IF (UNIX)
INCLUDE("cmake/FindPortaudio.cmake")
IF (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Found")
    INCLUDE_DIRECTORIES(${PORTAUDIO_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${EXTRA_LIBS} ${PORTAUDIO_LIBRARIES})
    ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ELSE (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Not Found...")
ENDIF (PORTAUDIO_FOUND)
INCLUDE("cmake/FindTinyXML.cmake")
FIND_PACKAGE(TinyXML QUIET)
ELSE (UNIX)
    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/include)
    SET(EXTRA_LIBS ${EXTRA_LIBS} ../buildwin/portaudio_x86)
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ENDIF (UNIX)

IF (MSVC)
    INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/include/msvc)
ENDIF (MSVC)

INCLUDE("cmake/PluginConfigure.cmake")

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

            src/wxcurl/base.cpp
            src/wxcurl/dav.cpp
            src/wxcurl/davtool.cpp
            src/wxcurl/dialog.cpp
            src/wxcurl/ftp.cpp
            src/wxcurl/ftpparse.cpp
            src/wxcurl/ftptool.cpp
            src/wxcurl/http.cpp
            src/wxcurl/panel.cpp
            src/wxcurl/telnet.cpp
            src/wxcurl/thread.cpp
            src/wxcurl/utils.cpp

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
            src/libaudiofile/pcm.cpp
            src/libaudiofile/query.cpp
            src/libaudiofile/units.cpp
            src/libaudiofile/util.cpp

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

IF(WIN32)
INSTALL(FILES "buildwin/PVW32Con.exe" DESTINATION "plugins\\\\${PACKAGE_NAME}")
ENDIF(WIN32)

ADD_DEFINITIONS(-DTIXML_USE_STL)
IF(TINYXML_FOUND)
    message (STATUS "Building with system tinyxml")
    INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX})
    TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} )
ELSE(TINYXML_FOUND)
    message (STATUS "Building with embedded tinyxml")
    INCLUDE_DIRECTORIES(src/tinyxml/)
    SET(SRC_LTINYXML
            src/tinyxml/tinyxml.cpp
            src/tinyxml/tinyxmlparser.cpp
            src/tinyxml/tinyxmlerror.cpp
    )
    ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_LTINYXML})
ENDIF(TINYXML_FOUND)

INCLUDE_DIRECTORIES(${PLUGIN_SOURCE_DIR}/src/libaudiofile)
SET(PLUGIN_PACKAGE_DEPS "curl")

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginCurl.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")


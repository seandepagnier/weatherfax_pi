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

SET(BUNDLE_DATA TRUE)

INCLUDE( VERSION.cmake )
SET(PLUGIN_VERSION "${PLUGIN_VERSION_MAJOR}.${PLUGIN_VERSION_MINOR}.${PLUGIN_VERSION_PATCH}" )

IF (UNIX)
INCLUDE("cmake/FindPortaudio.cmake")
IF (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Found")
    INCLUDE_DIRECTORIES(BEFORE ${PORTAUDIO_INCLUDE_DIRS})
    SET(PLUGINS_LIBS ${PLUGINS_LIBS} ${PORTAUDIO_LIBRARIES})
    ADD_DEFINITIONS(${PORTAUDIO_DEFINITIONS})
    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ELSE (PORTAUDIO_FOUND)
    MESSAGE (STATUS "Portaudio Not Found...")
ENDIF (PORTAUDIO_FOUND)
ELSE (UNIX)
    INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/include)
    IF(STANDALONE MATCHES "BUNDLED")
	# within OpenCPN tree
    	SET(PLUGINS_LIBS ${EXTRA_LIBS} ../../buildwin/portaudio_x86)
    	INSTALL(FILES "../../buildwin/portaudio_x86.dll" DESTINATION ".")
    ELSE()
    	SET(PLUGINS_LIBS ${EXTRA_LIBS} ../buildwin/portaudio_x86)
    	INSTALL(FILES "buildwin/portaudio_x86.dll" DESTINATION ".")
    ENDIF()

    ADD_DEFINITIONS(-DOCPN_USE_PORTAUDIO)
ENDIF (UNIX)

IF (MSVC)
    INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/include/msvc)
ENDIF (MSVC)

OPTION(WEATHERFAX_USE_SVG "Use SVG graphics" ON)

INCLUDE("cmake/PluginConfigure.cmake")

IF(WEATHERFAX_USE_SVG)
  ADD_DEFINITIONS(-DWEATHERFAX_USE_SVG)
ENDIF(WEATHERFAX_USE_SVG)

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

      
IF(UNIX)
 INCLUDE("cmake/FindTinyXML.cmake")
 FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX)

IF(TINYXML_FOUND)
 message (STATUS "Building with system tinyxml")
 INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX})
 TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES} ${PLUGINS_LIBS})
ELSE(TINYXML_FOUND)
 message (STATUS "Building with embedded tinyxml")
 INCLUDE_DIRECTORIES(src/tinyxml/)
 SET(SRC_LTINYXML
            src/tinyxml/tinyxml.cpp
            src/tinyxml/tinyxmlparser.cpp
            src/tinyxml/tinyxmlerror.cpp
 )
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC_WEATHERFAX} ${SRC_LTINYXML})
 TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${PLUGINS_LIBS})
ENDIF(TINYXML_FOUND)
ADD_DEFINITIONS(-DTIXML_USE_STL)

IF(WIN32)
    IF(STANDALONE MATCHES "BUNDLED")
	# within OpenCPN tree
	INSTALL(FILES "../../buildwin/PVW32Con.exe" DESTINATION "plugins\\\\${PACKAGE_NAME}")
    ELSE()
	INSTALL(FILES "buildwin/PVW32Con.exe" DESTINATION "plugins\\\\${PACKAGE_NAME}")
    ENDIF()

ENDIF(WIN32)

ADD_DEFINITIONS(-DTIXML_USE_STL)

INCLUDE_DIRECTORIES(BEFORE ${PLUGIN_SOURCE_DIR}/src/libaudiofile)

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")


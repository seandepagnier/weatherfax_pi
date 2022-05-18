
# OpenCPN wrapper for libcurl:
#
# Exports: libcurl::libcurl link object

cmake_minimum_required (VERSION 3.1.0)

if (TARGET libcurl::libcurl)
  return ()
endif ()

project(ocpn_curl)

set(curl_version 7.83.1)   # Actual version for downloaded tarball

set(curl_tarball "curl-${curl_version}.tar.gz")
set(curl_url https://curl.se/download/${curl_tarball})
set(curl_srcdir ${CMAKE_CURRENT_LIST_DIR}/curl-${curl_version})

# Make sure we have downloaded and unpacked the tarball
if (NOT EXISTS ${curl_srcdir})
  set(_cachedir "${CMAKE_SOURCE_DIR}/cache" CACHE STRING "Build download area")
  execute_process(COMMAND cmake -E make_directory ${_cachedir})

  # Some Debian/ubuntu versions have buggy curl and hence file(DOWNLOAD ...):
  if (NOT EXISTS ${_cachedir}/${curl_tarball})
    execute_process(COMMAND wget -O ${_cachedir}/${curl_tarball} ${curl_url})
  endif ()
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar -xzf "${_cachedir}/${curl_tarball}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
  )
endif ()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build release executable" FORCE)
endif ()
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs" FORCE)

set(BUILD_CURL_EXE OFF CACHE BOOL "Build the curl(1) executable" FORCE)
set(CURL_DISABLE_GOPHER ON CACHE BOOL "Disable Gopher support" FORCE)
set(CURL_DISABLE_HSTS ON CACHE BOOL "Disable Hsts support" FORCE)
set(CURL_DISABLE_IMAP ON CACHE BOOL "Disable IMAP support" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "Disable LDAP support" FORCE)
set(CURL_DISABLE_LDAPS ON CACHE BOOL "Disable LDAPS support" FORCE)
set(CURL_DISABLE_MIME ON CACHE BOOL "Disable MIME support" FORCE)
set(CURL_DISABLE_MQTT ON CACHE BOOL "Disable MQTT support" FORCE)
set(CURL_DISABLE_NTLM ON CACHE BOOL "Disable NTLM support" FORCE)
set(CURL_DISABLE_POP3 ON CACHE BOOL "Disable POP3 support" FORCE)
set(CURL_DISABLE_SMB ON CACHE BOOL "Disable SMB support" FORCE)
set(CURL_DISABLE_TELNET ON CACHE BOOL "Disable telnet support" FORCE)
set(ENABLE_MANUAL OFF CACHE BOOL "Don't build manual" FORCE)

if (WIN32)
  set(ENABLE_INET_PTON OFF CACHE BOOL "Disable for XP compatibility" ON)
endif ()

add_subdirectory(${curl_srcdir})

add_library(curl_if INTERFACE)
add_library(libcurl::libcurl ALIAS curl_if)
target_link_libraries(curl_if INTERFACE libcurl)
target_include_directories(curl_if INTERFACE ${curl_srcdir}/include)

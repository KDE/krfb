# Striped down version of libvncserver's original CMakeLists.txt

project(LibVNCServer)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckTypeSize)
include(TestBigEndian)
include(MacroBoolTo01)

set(FULL_PACKAGE_NAME      "Krfb LibVNCServer")
set(PACKAGE_VERSION        "0.9.8-10-g17ce0c5")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

macro_optional_find_package(ZLIB)
macro_log_feature(ZLIB_FOUND "ZLib" "The Zlib compression library" "http://www.zlib.net" FALSE "" "Used by the vncserver library.")

macro_optional_find_package(JPEG)
macro_log_feature(JPEG_FOUND "LibJPEG" "The JPEG library" "" FALSE "" "Used by the vncserver library")

macro_bool_to_01(ZLIB_FOUND LIBVNCSERVER_HAVE_LIBZ)
macro_bool_to_01(JPEG_FOUND LIBVNCSERVER_HAVE_LIBJPEG)
option(LIBVNCSERVER_ALLOW24BPP "Allow 24 bpp" ON)
# if(GNUTLS_FOUND)
#   set(LIBVNCSERVER_WITH_CLIENT_TLS 1)
# endif(GNUTLS_FOUND)
# if(LIBGCRYPT_LIBRARIES)
#   message(STATUS "Found libgcrypt: ${LIBGCRYPT_LIBRARIES}")
#   set(LIBVNCSERVER_WITH_CLIENT_GCRYPT 1)
# endif(LIBGCRYPT_LIBRARIES)


check_include_file("fcntl.h"       LIBVNCSERVER_HAVE_FCNTL_H)
check_include_file("netinet/in.h"  LIBVNCSERVER_HAVE_NETINET_IN_H)
check_include_file("sys/socket.h"  LIBVNCSERVER_HAVE_SYS_SOCKET_H)
check_include_file("sys/stat.h"    LIBVNCSERVER_HAVE_SYS_STAT_H)
check_include_file("sys/time.h"    LIBVNCSERVER_HAVE_SYS_TIME_H)
check_include_file("sys/types.h"   LIBVNCSERVER_HAVE_SYS_TYPES_H)
check_include_file("sys/wait.h"    LIBVNCSERVER_HAVE_SYS_WAIT_H)
check_include_file("unistd.h"      LIBVNCSERVER_HAVE_UNISTD_H)

# headers needed for check_type_size()
check_include_file("arpa/inet.h"   HAVE_ARPA_INET_H)
check_include_file("stdint.h"      HAVE_STDINT_H)
check_include_file("stddef.h"      HAVE_STDDEF_H)
check_include_file("sys/types.h"   HAVE_SYS_TYPES_H)

check_function_exists(gettimeofday    LIBVNCSERVER_HAVE_GETTIMEOFDAY)

macro_bool_to_01(CMAKE_USE_PTHREADS_INIT LIBVNCSERVER_HAVE_LIBPTHREAD)
if(LIBVNCSERVER_HAVE_SYS_SOCKET_H)
  # socklen_t
  list(APPEND CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h")
endif(LIBVNCSERVER_HAVE_SYS_SOCKET_H)
if(HAVE_ARPA_INET_H)
  # in_addr_t
  list(APPEND CMAKE_EXTRA_INCLUDE_FILES "arpa/inet.h")
endif(HAVE_ARPA_INET_H)

check_type_size(pid_t     LIBVNCSERVER_PID_T)
check_type_size(size_t    LIBVNCSERVER_SIZE_T)
check_type_size(socklen_t LIBVNCSERVER_SOCKLEN_T)
check_type_size(in_addr_t LIBVNCSERVER_IN_ADDR_T)
if(NOT HAVE_LIBVNCSERVER_IN_ADDR_T)
  set(LIBVNCSERVER_NEED_INADDR_T 1)
endif(NOT HAVE_LIBVNCSERVER_IN_ADDR_T)

test_big_endian(LIBVNCSERVER_WORDS_BIGENDIAN)


configure_file (${CMAKE_CURRENT_SOURCE_DIR}/libvncserver-config.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/libvncserver-config.h
)

set(LIBVNCSERVER_SOURCES
    main.c
    rfbserver.c
    rfbregion.c
    auth.c
    sockets.c
    stats.c
    corre.c
    hextile.c
    rre.c
    translate.c
    cutpaste.c
    httpd.c
    cursor.c
    font.c
    draw.c
    selbox.c
    d3des.c
    vncauth.c
    cargs.c
    minilzo.c
    ultra.c
    scale.c
)

if(ZLIB_FOUND)
  add_definitions(-DLIBVNCSERVER_HAVE_LIBZ)
  include_directories(${ZLIB_INCLUDE_DIR})
  set(LIBVNCSERVER_SOURCES
    ${LIBVNCSERVER_SOURCES}
    zlib.c
    zrle.c
    zrleoutstream.c
    zrlepalettehelper.c
  )
endif(ZLIB_FOUND)

if(JPEG_FOUND)
  add_definitions(-DLIBVNCSERVER_HAVE_LIBJPEG)
  include_directories(${JPEG_INCLUDE_DIR})
  set(LIBVNCSERVER_SOURCES
    ${LIBVNCSERVER_SOURCES}
    tight.c
  )
endif(JPEG_FOUND)


add_library(vncserver STATIC ${LIBVNCSERVER_SOURCES})

if(WIN32)
  set(ADDITIONAL_LIBS ws2_32)
endif(WIN32)

target_link_libraries(vncserver
    ${ADDITIONAL_LIBS}
    ${ZLIB_LIBRARIES}
    ${JPEG_LIBRARIES}
)

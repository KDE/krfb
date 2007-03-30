check_symbol_exists(getifaddrs     "sys/socket.h;netdb.h;ifaddrs.h"     HAVE_GETIFADDRS)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-krfb.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krfb.h )
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # for config-krfb.h

set(vncserver_SRCS
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/main.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/rfbserver.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/sraRegion.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/auth.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/sockets.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/stats.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/corre.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/hextile.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/rre.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/translate.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/cutpaste.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/zlib.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/tight.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/httpd.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/cursor.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/font.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/draw.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/selbox.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/d3des.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/vncauth.c
   ${CMAKE_SOURCE_DIR}/krfb/libvncserver/cargs.c)

set(srvloc_SRCS
   ${CMAKE_SOURCE_DIR}/krfb/srvloc/kinetinterface.cpp
   ${CMAKE_SOURCE_DIR}/krfb/srvloc/uuid.cpp
   ${CMAKE_SOURCE_DIR}/krfb/srvloc/getifaddrs.cpp
   ${CMAKE_SOURCE_DIR}/krfb/srvloc/kserviceregistry.cpp
   ${CMAKE_SOURCE_DIR}/krfb/srvloc/kinetinterfacewatcher.cpp)

set(krfbconfig_SRCS
   ${CMAKE_SOURCE_DIR}/krfb/krfb/configuration.cc
   ${CMAKE_SOURCE_DIR}/krfb/krfb/invitedialog.cc
   ${CMAKE_SOURCE_DIR}/krfb/krfb/invitation.cc
   ${CMAKE_SOURCE_DIR}/krfb/krfb/personalinvitedialog.cc)


# add_subdirectory(kinetd)
#add_subdirectory(kcm_krfb)
add_subdirectory(krfb)
#add_subdirectory(krfb_httpd)

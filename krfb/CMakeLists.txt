project(krfb)

include_directories(${CMAKE_SOURCE_DIR}/krfb/libvncserver ${CMAKE_SOURCE_DIR}/krfb/srvloc)

########### config lib ###############

set(libkrfbconfig_SRCS
configuration.cc
invitedialog.cc
invitation.cc
personalinvitedialog.cc)

kde4_add_ui3_files(libkrfbconfig_SRCS
manageinvitations.ui
personalinvitewidget.ui
invitewidget.ui)

kde4_automoc(${libkrfbconfig_SRCS})

kde4_add_library(libkrfbconfig ${libkrfbconfig_SRCS})

target_link_libraries(libkrfbconfig)

install(TARGETS libkrfbconfig DESTINATION ${LIB_INSTALL_DIR})

########### /config lib ###############

set(krfb_SRCS
   rfbcontroller.cc
   xupdatescanner.cc
   main.cpp
#    krfbifaceimpl.cc
   trayicon.cpp
   connectiondialog.cc)

kde4_automoc(${krfb_SRCS})

kde4_add_ui3_files(krfb_SRCS connectionwidget.ui)

message(STATUS "Port krfb to DBUS")
# kde4_add_dcop_skels(krfb_SRCS krfbiface.h )

kde4_add_executable(krfb ${krfb_SRCS})

target_link_libraries(krfb ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS} libkrfbconfig libvncserver libsrvloc ${JPEG_LIBRARIES})

install(TARGETS krfb  DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install(FILES krfb.desktop DESTINATION ${XDG_APPS_DIR})
install(FILES krfb.notifyrc DESTINATION ${DATA_INSTALL_DIR}/krfb)
install(FILES eyes-open24.png eyes-closed24.png connection-side-image.png DESTINATION ${DATA_INSTALL_DIR}/krfb/pics)
install(FILES kinetd_krfb.desktop DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons(${ICON_INSTALL_DIR})

project(krfb)


check_symbol_exists(rfbInitServer  "rfb/rfb.h"     HAVE_LIBVNCSERVER)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-krfb.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krfb.h )
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # for config-krfb.h


#add_subdirectory(kinetd)
#add_subdirectory(kcm_krfb)
if (HAVE_LIBVNCSERVER)

set(krfb_SRCS
   main.cpp
   trayicon.cpp
   krfbserver.cpp
   manageinvitationsdialog.cpp
   invitationmanager.cpp
   invitedialog.cpp
   invitation.cpp
   connectiondialog.cpp
   personalinvitedialog.cpp
   connectioncontroller.cpp
   events.cpp
#   rfbcontroller.cc
#   xupdatescanner.cc
)

kde4_automoc(${krfb_SRCS})

kde4_add_ui_files(krfb_SRCS connectionwidget.ui
        manageinvitations.ui
        personalinvitewidget.ui
        invitewidget.ui)

kde4_add_executable(krfb ${krfb_SRCS})

target_link_libraries(krfb ${KDE4_KIO_LIBS} ${JPEG_LIBRARIES} ${SLP_LIBRARIES} ${LIBVNCSERVER_LIBRARIES})

install(TARGETS krfb  DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install(FILES krfb.desktop DESTINATION ${XDG_APPS_DIR})
install(FILES krfb.notifyrc DESTINATION ${DATA_INSTALL_DIR}/krfb)
install(FILES eyes-open24.png eyes-closed24.png connection-side-image.png DESTINATION ${DATA_INSTALL_DIR}/krfb/pics)
# install(FILES kinetd_krfb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install(FILES org.kde.krfb.xml  DESTINATION ${DBUS_INTERFACES_DIR} )
kde4_install_icons(${ICON_INSTALL_DIR})


endif (HAVE_LIBVNCSERVER)


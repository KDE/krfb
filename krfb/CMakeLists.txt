project(krfb)

include_directories(${CMAKE_SOURCE_DIR}/krfb/libvncserver
    ${CMAKE_SOURCE_DIR}/krfb/srvloc
    ${CMAKE_CURRENT_BINARY_DIR}
)


##set(krfb_SRCS ${vncserver_SRCS} ${srvloc_SRCS} ${krfbconfig_SRCS}
set(krfb_SRCS ${vncserver_SRCS}
#   rfbcontroller.cc
#   xupdatescanner.cc
   main.cpp
   trayicon.cpp
   connectiondialog.cc
   krfbserver.cpp
   manageinvitationsdialog.cpp
   invitedialog.cc
   invitation.cc
   connectiondialog.cc
   personalinvitedialog.cc
)

## qt4_add_dbus_adaptor(krfb_SRCS org.kde.krfb.xml krfbifaceimpl.h KRfbIfaceImpl)

kde4_automoc(${krfb_SRCS})

kde4_add_ui_files(krfb_SRCS rfbconnectionwidget.ui
        rfbmanageinvitations.ui
        rfbpersonalinvitewidget.ui
        rfbinvitewidget.ui)

kde4_add_executable(krfb ${krfb_SRCS})

target_link_libraries(krfb ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${JPEG_LIBRARIES} ${SLP_LIBRARIES})

install(TARGETS krfb  DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

install(FILES krfb.desktop DESTINATION ${XDG_APPS_DIR})
install(FILES krfb.notifyrc DESTINATION ${DATA_INSTALL_DIR}/krfb)
install(FILES eyes-open24.png eyes-closed24.png connection-side-image.png DESTINATION ${DATA_INSTALL_DIR}/krfb/pics)
install(FILES kinetd_krfb.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES org.kde.krfb.xml  DESTINATION ${DBUS_INTERFACES_DIR} )
kde4_install_icons(${ICON_INSTALL_DIR})


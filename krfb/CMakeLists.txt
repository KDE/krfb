configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config-krfb.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-krfb.h
)

#####################################
# First target: libkrfbprivate - a library
# for linking plugins against.

set (krfbprivate_SRCS
     framebuffer.cpp
     framebufferplugin.cpp
)

kde4_add_library (krfbprivate
                  SHARED
                  ${krfbprivate_SRCS}
)

target_link_libraries (krfbprivate
                       ${QT_QTCORE_LIBRARY}
                       ${QT_QTGUI_LIBRARY}
                       ${X11_X11_LIB}
                       ${LIBVNCSERVER_LIBRARIES}
)

set_target_properties (krfbprivate PROPERTIES
                       VERSION ${GENERIC_LIB_VERSION}
                       SOVERSION ${GENERIC_LIB_VERSION}
)

install (TARGETS krfbprivate
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install (FILES
         krfb-framebuffer.desktop
         DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

#####################################
# Second target: krfb - the app
# itself.

set (krfb_SRCS
     connectiondialog.cpp
     events.cpp
     framebuffermanager.cpp
     invitation.cpp
     invitedialog.cpp
     invitationmanager.cpp
     main.cpp
     manageinvitationsdialog.cpp
     personalinvitedialog.cpp
     sockethelpers.cpp
     trayicon.cpp
     rfbservermanager.cpp
     rfbserver.cpp
     rfbclient.cpp
     invitationsrfbserver.cpp
)

kde4_add_kcfg_files (krfb_SRCS
                     krfbconfig.kcfgc
)

kde4_add_ui_files (krfb_SRCS
                   ui/configtcp.ui
                   ui/configsecurity.ui
                   ui/connectionwidget.ui
                   ui/invitewidget.ui
                   ui/manageinvitations.ui
                   ui/personalinvitewidget.ui
)

kde4_add_executable (krfb
                     ${krfb_SRCS}
)

target_link_libraries (krfb
                       krfbprivate
                       ${JPEG_LIBRARIES}
                       ${LIBVNCSERVER_LIBRARIES}
                       ${X11_Xext_LIB}
                       ${X11_X11_LIB}
                       ${X11_Xdamage_LIB}
                       ${QT_QTNETWORK_LIBRARY}
                       ${KDE4_KDNSSD_LIBS}
                       ${KDE4_KDEUI_LIBS}
)

if (X11_XTest_FOUND)
    target_link_libraries (krfb
                           ${X11_XTest_LIB}
    )
endif (X11_XTest_FOUND)

install (TARGETS krfb
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### install files ###############

install (PROGRAMS krfb.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install (FILES krfb.notifyrc
         DESTINATION ${DATA_INSTALL_DIR}/krfb
)
 

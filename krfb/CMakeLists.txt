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
)

set_target_properties (krfbprivate PROPERTIES
                       VERSION ${GENERIC_LIB_VERSION}
                       SOVERSION ${GENERIC_LIB_VERSION}
)

install (TARGETS krfbprivate
         ${INSTALL_TARGETS_DEFAULT_ARGS}
         LIBRARY NAMELINK_SKIP
)

install (FILES
         krfb-framebuffer.desktop
         DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

#####################################
# Second target: krfb - the app
# itself.

if(TelepathyQt4_FOUND)
    add_definitions(-DKRFB_WITH_TELEPATHY_TUBES)
    include_directories(${TELEPATHY_QT4_INCLUDE_DIR})
endif()

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
     invitationsrfbclient.cpp
)

if(TelepathyQt4_FOUND)
    set (krfb_SRCS
         ${krfb_SRCS}
         tubesrfbserver.cpp
         tubesrfbclient.cpp
    )
endif()

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

if(TelepathyQt4_FOUND)
    kde4_add_ui_files(krfb_SRCS ui/tubesconnectionwidget.ui)
endif()

kde4_add_executable (krfb
                     ${krfb_SRCS}
)

target_link_libraries (krfb
                       krfbprivate
                       vncserver
                       ${JPEG_LIBRARIES}
                       ${X11_Xext_LIB}
                       ${X11_X11_LIB}
                       ${X11_Xdamage_LIB}
                       ${QT_QTNETWORK_LIBRARY}
                       ${KDE4_KDNSSD_LIBS}
                       ${KDE4_KDEUI_LIBS}
)

if(TelepathyQt4_FOUND)
    target_link_libraries(krfb
                          ${TELEPATHY_QT4_LIBRARIES}
    )
endif()

if (X11_XTest_FOUND)
    target_link_libraries (krfb
                           ${X11_XTest_LIB}
    )
endif (X11_XTest_FOUND)

install (TARGETS krfb
         ${INSTALL_TARGETS_DEFAULT_ARGS}
)

if(TelepathyQt4_FOUND)
    configure_file(org.freedesktop.Telepathy.Client.krfb_rfb_handler.service.in
                   org.freedesktop.Telepathy.Client.krfb_rfb_handler.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.krfb_rfb_handler.service
            DESTINATION ${DBUS_SERVICES_INSTALL_DIR})

    install(FILES krfb_rfb_handler.client DESTINATION ${CMAKE_INSTALL_PREFIX}/share/telepathy/clients/)
endif()

########### install files ###############

install (PROGRAMS krfb.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install (FILES krfb.notifyrc
         DESTINATION ${DATA_INSTALL_DIR}/krfb
)
 

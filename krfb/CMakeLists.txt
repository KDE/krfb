configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config-krfb.h.cmake
                ${CMAKE_CURRENT_BINARY_DIR}/config-krfb.h
)

include(GenerateExportHeader)

#####################################
# First target: libkrfbprivate - a library
# for linking plugins against.

set (krfbprivate_SRCS
     framebuffer.cpp
     framebufferplugin.cpp
     events.cpp
     eventsplugin.cpp
)

add_library (krfbprivate
                  SHARED
                  ${krfbprivate_SRCS}
)
generate_export_header(krfbprivate BASE_NAME krfbprivate)


target_link_libraries (krfbprivate
                       Qt5::Core
                       Qt5::Widgets
                       Qt5::X11Extras
                       ${X11_X11_LIB}
                       ${LIBVNCSERVER_LIBRARIES}
)

set_target_properties (krfbprivate PROPERTIES
                       VERSION 5
                       SOVERSION 5.0
)

install (TARGETS krfbprivate
         ${INSTALL_TARGETS_DEFAULT_ARGS}
         LIBRARY NAMELINK_SKIP
)

install (FILES
         krfb-framebuffer.desktop
         krfb-events.desktop
         DESTINATION ${SERVICETYPES_INSTALL_DIR}
)

#####################################
# Second target: krfb - the app
# itself.

set (krfb_SRCS
     connectiondialog.cpp
     framebuffermanager.cpp
     events.cpp
     eventsmanager.cpp
     main.cpp
     mainwindow.cpp
     sockethelpers.cpp
     trayicon.cpp
     rfbservermanager.cpp
     rfbserver.cpp
     rfbclient.cpp
     invitationsrfbserver.cpp
     invitationsrfbclient.cpp
)

kconfig_add_kcfg_files (krfb_SRCS
                     krfbconfig.kcfgc
)

ki18n_wrap_ui (krfb_SRCS
                   ui/configtcp.ui
                   ui/configsecurity.ui
                   ui/configframebuffer.ui
                   ui/connectionwidget.ui
                   ui/mainwidget.ui
)

qt5_add_resources(krfb_SRCS
    krfb.qrc
)

add_executable (krfb
                     ${krfb_SRCS}
)

target_link_libraries (krfb
                       krfbprivate
                       ${JPEG_LIBRARIES}
                       ${X11_Xext_LIB}
                       ${X11_X11_LIB}
                       ${X11_Xdamage_LIB}
                       Qt5::Network
                       KF5::Completion
                       KF5::CoreAddons
                       KF5::DBusAddons
                       KF5::DNSSD
                       KF5::I18n
                       KF5::Notifications
                       KF5::Wallet
                       KF5::WidgetsAddons
                       KF5::XmlGui
                       ${LIBVNCSERVER_LIBRARIES}
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

install (PROGRAMS org.kde.krfb.desktop
         DESTINATION ${XDG_APPS_INSTALL_DIR}
)

install(FILES org.kde.krfb.appdata.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install (FILES krfb.notifyrc
         DESTINATION ${DATA_INSTALL_DIR}/krfb
)


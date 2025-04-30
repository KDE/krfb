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
                       Qt::Core
                       Qt::Widgets
                       Qt::GuiPrivate
                       ${X11_X11_LIB}
                       LibVNCServer::vncserver
)

set_target_properties (krfbprivate PROPERTIES
                       VERSION 5
                       SOVERSION 5.0
)

install (TARGETS krfbprivate
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
         LIBRARY NAMELINK_SKIP
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

ecm_qt_declare_logging_category(krfb_SRCS
    HEADER krfbdebug.h
    IDENTIFIER KRFB
    CATEGORY_NAME krfb.krfb
    DESCRIPTION "KRFB Application"
    EXPORT KRFB
)

kconfig_add_kcfg_files (krfb_SRCS
                     krfbconfig.kcfgc
)

ki18n_wrap_ui (krfb_UI_SRCS
                   ui/configtcp.ui
                   ui/configsecurity.ui
                   ui/configframebuffer.ui
                   ui/connectionwidget.ui
                   ui/mainwidget.ui
)

qt_add_resources(krfb_SRCS
    krfb.qrc
)

add_executable (krfb
                     ${krfb_SRCS}
                     ${krfb_UI_SRCS}
)

target_link_libraries (krfb
                       krfbprivate
                       ${JPEG_LIBRARIES}
                       ${X11_Xext_LIB}
                       ${X11_X11_LIB}
                       ${X11_Xdamage_LIB}
                       Qt::Network
                       KF6::CoreAddons
                       KF6::DBusAddons
                       KF6::DNSSD
                       KF6::I18n
                       KF6::Notifications
                       KF6::Wallet
                       KF6::WidgetsAddons
                       KF6::WindowSystem
                       KF6::XmlGui
                       KF6::StatusNotifierItem
                       KF6::Crash
                       LibVNCServer::vncserver
)

if (X11_XTest_FOUND)
    target_link_libraries (krfb
                           ${X11_XTest_LIB}
    )
endif (X11_XTest_FOUND)

install (TARGETS krfb
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

#################################
kconfig_add_kcfg_files (krfbvm_SRCS
                     krfbconfig.kcfgc
)

ecm_qt_declare_logging_category(krfbvm_SRCS
    HEADER krfbdebug.h
    IDENTIFIER KRFB
    CATEGORY_NAME krfb.krfb
    DESCRIPTION "KRFB Application"
    EXPORT KRFB
)

add_executable(krfb-virtualmonitor main-virtualmonitor.cpp ${krfbvm_SRCS} ${krfb_UI_SRCS}
                                   rfbserver.cpp rfbclient.cpp rfbservermanager.cpp eventsmanager.cpp framebuffermanager.cpp sockethelpers.cpp)
target_link_libraries(krfb-virtualmonitor
                      krfbprivate
                      Qt::Gui
                      Qt::Network
                      KF6::ConfigGui
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::WindowSystem
                      KF6::Crash
)

install (TARGETS krfb-virtualmonitor
         ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

configure_file(org.kde.krfb.virtualmonitor.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.kde.krfb.virtualmonitor.desktop @ONLY)
install (PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.krfb.virtualmonitor.desktop
         DESTINATION ${KDE_INSTALL_APPDIR}
)

########### install files ###############

install (PROGRAMS org.kde.krfb.desktop
         DESTINATION ${KDE_INSTALL_APPDIR}
)

install(FILES org.kde.krfb.appdata.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR}
)

install (FILES krfb.notifyrc
         DESTINATION ${KDE_INSTALL_DATADIR}/krfb
)

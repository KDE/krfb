project(krfb)

if(NOT INSIDE_KDENETWORK)
    message("Not building inside KDENetwork, loading KDE CMake Macros.")

    set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

    find_package(KDE4 REQUIRED)

    include(KDE4Defaults)
    include(MacroLibrary)

    include(CheckSymbolExists)
    find_package(LibVNCServer REQUIRED)

    set(CMAKE_REQUIRED_DEFINITIONS ${_KDE_PLATFORM_DEFINITIONS})

    add_definitions(${QT_DEFINITIONS} ${QT_DBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
    include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})
endif(NOT INSIDE_KDENETWORK)

check_symbol_exists(rfbInitServer  "rfb/rfb.h"     HAVE_LIBVNCSERVER)
macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE)
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-krfb.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krfb.h )
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # for config-krfb.h

if(Q_WS_X11)
  if(NOT X11_XTest_FOUND)
    macro_log_feature(X11_XTest_FOUND "libXtst" "X11 Testing Resource extension library" "http://xorg.freedesktop.org" FALSE "" "useful for automated testing of X clients.")
  endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

#add_subdirectory(kinetd)
#add_subdirectory(kcm_krfb)
if (HAVE_LIBVNCSERVER)

#####################################
# First target: libkrfb - a library
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
# Build the plugins

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory (framebuffers)

#####################################
# Second target: krfb - the app
# itself.

set(krfb_SRCS
   main.cpp
   trayicon.cpp
   manageinvitationsdialog.cpp
   invitationmanager.cpp
   invitedialog.cpp
   invitation.cpp
   connectiondialog.cpp
   personalinvitedialog.cpp
   events.cpp
   framebuffermanager.cpp
   servermanager.cpp
   sockethelpers.cpp
   abstractrfbserver.cpp
   abstractconnectioncontroller.cpp
   krfbserver.cpp
   krfbconnectioncontroller.cpp
)

kde4_add_kcfg_files(krfb_SRCS krfbconfig.kcfgc)

kde4_add_ui_files(krfb_SRCS connectionwidget.ui
        manageinvitations.ui
        personalinvitewidget.ui
        invitewidget.ui
        configtcp.ui
        configsecurity.ui
        )

kde4_add_executable(krfb ${krfb_SRCS})

target_link_libraries(krfb
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

if(X11_XTest_FOUND)
  target_link_libraries(krfb ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS krfb  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(PROGRAMS krfb.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES krfb.notifyrc DESTINATION ${DATA_INSTALL_DIR}/krfb)

endif (HAVE_LIBVNCSERVER)

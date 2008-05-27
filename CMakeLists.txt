project(krfb)

check_symbol_exists(rfbInitServer  "rfb/rfb.h"     HAVE_LIBVNCSERVER)
macro_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE)
macro_bool_to_01(X11_XShm_FOUND HAVE_XSHM)
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
   framebuffer.cpp
   qtframebuffer.cpp
   x11framebuffer.cpp
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

target_link_libraries(krfb ${JPEG_LIBRARIES} ${SLP_LIBRARIES} ${LIBVNCSERVER_LIBRARIES} ${X11_Xdamage_LIB} ${KDE4_KDNSSD_LIBS})
if(X11_XTest_FOUND)
  target_link_libraries(krfb ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

install(TARGETS krfb  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES krfb.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES krfb.notifyrc DESTINATION ${DATA_INSTALL_DIR}/krfb)

endif (HAVE_LIBVNCSERVER)

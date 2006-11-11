project(krfb)

include_directories( ${CMAKE_SOURCE_DIR}/krfb/libvncserver ${CMAKE_SOURCE_DIR}/krfb/srvloc  )


########### next target ###############


########### next target ###############

set(krfb_SRCS 
   rfbcontroller.cc 
   xupdatescanner.cc 
   main.cpp 
   krfbifaceimpl.cc 
   trayicon.cpp 
   connectiondialog.cc )

kde4_automoc(${krfb_SRCS})

kde4_add_ui_files(krfb_SRCS connectionwidget.ui )

kde4_add_dcop_skels(krfb_SRCS krfbiface.h )

kde4_add_executable(krfb ${krfb_SRCS})

target_link_libraries(krfb  ${KDE4_KIO_LIBS} krfbconfig vncserver srvloc Xtst )

install(TARGETS krfb  DESTINATION ${BIN_INSTALL_DIR} )


########### install files ###############

install( FILES krfb.desktop  DESTINATION ${XDG_APPS_DIR})
install( FILES eventsrc  DESTINATION ${DATA_INSTALL_DIR}/krfb)
install( FILES eyes-open24.png eyes-closed24.png connection-side-image.png  DESTINATION ${DATA_INSTALL_DIR}/krfb/pics)
install( FILES kinetd_krfb.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons( ${ICON_INSTALL_DIR}   )





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

kde4_automoc(krfb ${krfb_SRCS})

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




#original Makefile.am contents follow:

#KDE_CXXFLAGS = $(USE_THREADS)
#
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkrfbconfig.la
#libkrfbconfig_la_SOURCES = configuration.cc configuration.skel invitedialog.cc invitation.cc \
#                           manageinvitations.ui personalinvitewidget.ui \
#                           invitewidget.ui personalinvitedialog.cc
#libkrfbconfig_la_LIBADD = ../srvloc/libsrvloc.la $(LIB_KDEUI)
#
#bin_PROGRAMS = krfb
#krfb_SOURCES = rfbcontroller.cc xupdatescanner.cc main.cpp \
#               connectionwidget.ui krfbifaceimpl.cc krfbiface.skel \
#               trayicon.cpp connectiondialog.cc
#krfb_LDADD   = libkrfbconfig.la ../libvncserver/libvncserver.la ../srvloc/libsrvloc.la -lXtst $(LIB_KDEUI) $(LIBJPEG) -lkio 
#krfb_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#appdatadir = $(kde_datadir)/krfb/pics
#appdata_DATA = eyes-open24.png eyes-closed24.png connection-side-image.png
#
#kde_services_DATA = kinetd_krfb.desktop
#
#xdg_apps_DATA = krfb.desktop
#
#appdir = $(kde_datadir)/krfb
#app_DATA = eventsrc
#
#KDE_ICON = krfb
#
#INCLUDES= -I$(top_srcdir)/krfb/libvncserver -I$(top_srcdir)/krfb/srvloc \
#           $(all_includes)
#
#messages: rc.cpp
#	$(XGETTEXT) rc.cpp *.cpp *.cc -o $(podir)/krfb.pot

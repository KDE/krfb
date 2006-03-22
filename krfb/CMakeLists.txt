kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/krfb/libvncserver ${CMAKE_SOURCE_DIR}/krfb/srvloc ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


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

target_link_libraries(krfb  ${KDE4_KDECORE_LIBS} krfbconfig vncserver srvloc Xtst kdeui kio )

install_targets(/bin krfb )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES krfb.desktop )
install_files( ${DATA_INSTALL_DIR}/krfb FILES eventsrc )
install_files( ${DATA_INSTALL_DIR}/krfb/pics FILES eyes-open24.png eyes-closed24.png connection-side-image.png )
install_files( ${SERVICES_INSTALL_DIR} FILES kinetd_krfb.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )

kde4_footer()



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

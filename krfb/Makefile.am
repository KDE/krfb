KDE_CXXFLAGS = $(USE_THREADS)

METASOURCES = AUTO

noinst_LTLIBRARIES = libkrfbconfig.la
libkrfbconfig_la_SOURCES = configuration.cc configuration.skel invitedialog.cc invitation.cc \
                           manageinvitations.ui personalinvitewidget.ui \
                           invitewidget.ui personalinvitedialog.cc
libkrfbconfig_la_LIBADD = ../srvloc/libsrvloc.la $(LIB_KDEUI)

bin_PROGRAMS = krfb
krfb_SOURCES = rfbcontroller.cc xupdatescanner.cc main.cpp \
               connectionwidget.ui krfbifaceimpl.cc krfbiface.skel \
               trayicon.cpp connectiondialog.cc
krfb_LDADD   = libkrfbconfig.la ../libvncserver/libvncserver.la ../srvloc/libsrvloc.la -lXtst $(LIB_KDEUI) $(LIBJPEG) -lkio 
krfb_LDFLAGS = $(all_libraries) $(KDE_RPATH)

appdatadir = $(kde_datadir)/krfb/pics
appdata_DATA = eyes-open24.png eyes-closed24.png connection-side-image.png

kde_services_DATA = kinetd_krfb.desktop

xdg_apps_DATA = krfb.desktop

appdir = $(kde_datadir)/krfb
app_DATA = eventsrc

KDE_ICON = krfb

INCLUDES= -I$(top_srcdir)/krfb/libvncserver -I$(top_srcdir)/krfb/srvloc \
           $(all_includes)

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp *.cc -o $(podir)/krfb.pot

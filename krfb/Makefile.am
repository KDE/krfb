KDE_CXXFLAGS = $(USE_THREADS)

METASOURCES = AUTO

noinst_LTLIBRARIES = libkrfbconfig.la
libkrfbconfig_la_SOURCES = configuration.cc manageinvitations.ui \
                           personalinvitation.ui invite.ui invitation.cc \
                           configuration_hostname.cpp

bin_PROGRAMS = krfb
krfb_SOURCES = rfbcontroller.cc trayicon.cpp xupdatescanner.cc main.cpp \
               newconnectiondialog.ui krfbifaceimpl.cc krfbiface.skel
krfb_LDADD   = libkrfbconfig.la ../libvncserver/libvncserver.la -lXtst $(LIB_KDEUI) -ljpeg
krfb_LDFLAGS = $(all_libraries) $(KDE_RPATH)

noinst_HEADER = configuration.h invitation.h invite.h krfbiface.h \
                krfbifaceimpl.h manageinvitations.h newconnectiondialog.h \
                personalinvitation.h rfbcontroller.h trayicon.h \
                xupdatescanner.h


datadir = $(kde_datadir)/krfb/pics
data_DATA = eyes-open24.png eyes-closed24.png connection-side-image.png

kde_services_DATA = kinetd_krfb.desktop

lnkdir = $(kde_appsdir)/System
lnk_DATA = krfb.desktop

appdir = $(kde_datadir)/krfb
app_DATA = eventsrc

KDE_ICON = AUTO

INCLUDES= -I$(top_srcdir)/krfb/libvncserver $(all_includes)

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp *.cc -o $(podir)/krfb.pot

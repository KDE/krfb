METASOURCES = AUTO

bin_PROGRAMS = krfb
krfb_SOURCES = rfbcontroller.cc configuration.cc trayicon.cpp \
  xupdatescanner.cc main.cpp configurationdialog.ui newconnectiondialog.ui \
  krfbifaceimpl.cc krfbiface.skel
krfb_LDADD   = ../libvncserver/libvncserver.a -lz -lpthread -ljpeg -lXtst \
  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)  $(LIBSOCKET)

EXTRA_DIST = $(krfb_SOURCES) krfb.desktop lo32-app-krfb.png \
  lo16-app-krfb.png rfbcontroller.h eyes-closed24.png eyes-open24.png \
  xupdatescanner.h trayicon.h configuration.h krfbiface.h krfbiface.kidl \
  eventsrc

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/krfb.desktop $(kde_appsdir)/Applications/krfb.desktop
	$(mkinstalldirs) $(kde_datadir)/krfb
	$(INSTALL_DATA) $(srcdir)/eventsrc $(kde_datadir)/krfb/eventsrc
	$(mkinstalldirs) $(kde_datadir)/krfb/pics
	$(INSTALL_DATA) $(srcdir)/eyes-closed24.png $(kde_datadir)/krfb/pics/eyes-closed24.png
	$(INSTALL_DATA) $(srcdir)/eyes-open24.png $(kde_datadir)/krfb/pics/eyes-open24.png
	$(INSTALL_DATA) $(srcdir)/connection-side-image.png $(kde_datadir)/krfb/pics/connection-side-image.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/krfb.desktop
	-rm -f $(kde_datadir)/krfb/eventsrc
	-rm -f $(kde_datadir)/krfb/pics/eyes-open24.png
	-rm -f $(kde_datadir)/krfb/pics/eyes-closed24.png

KDE_ICON = krfb

# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I../libvncserver

# the library search path. 
krfb_LDFLAGS = $(all_libraries) $(KDE_RPATH)

CXXFLAGS = @CXXFLAGS@ -DQT_THREAD_SUPPORT

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE´s XML GUI builing
#rcdir = $(kde_datadir)/krfb
#rc_DATA = krfbui.rc

podir = ../po

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/krfb.pot; \
	fi


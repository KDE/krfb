METASOURCES = AUTO

# Code 
lib_LTLIBRARIES = libkded_kinetd.la

libkded_kinetd_la_SOURCES = kinetd.cpp kinetd.skel
libkded_kinetd_la_LDFLAGS  = $(all_libraries) -module -avoid-version
libkded_kinetd_la_LIBADD   = ../srvloc/libsrvloc.la $(LIB_QT) $(LIB_KDECORE) \
	$(LIB_KDEUI) -lslp

# Services
kde_servicetypes_DATA = kinetdmodule.desktop
kdeddir = $(kde_servicesdir)/kded
kded_DATA = kinetd.desktop

EXTRA_DIST = $(libkded_kinetd_la_SOURCES)\
             $(kded_DATA) \
             $(kde_servicetypes_DATA) \
	     eventsrc README.debugging

KDE_ICON = kinetd

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kinetd
	$(INSTALL_DATA) $(srcdir)/eventsrc $(DESTDIR)$(kde_datadir)/kinetd/eventsrc

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/krfb/srvloc



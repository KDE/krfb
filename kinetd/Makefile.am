METASOURCES = AUTO

# Code 
kde_module_LTLIBRARIES = kded_kinetd.la

kded_kinetd_la_SOURCES = kinetd.cpp kinetd.skel
kded_kinetd_la_LDFLAGS  = $(all_libraries) -module -avoid-version
kded_kinetd_la_LIBADD   = ../srvloc/libsrvloc.la $(LIB_KIO) $(LIB_KDNSSD)

# Services
kde_servicetypes_DATA = kinetdmodule.desktop
kdeddir = $(kde_servicesdir)/kded
kded_DATA = kinetd.desktop

EXTRA_DIST = $(kded_kinetd_la_SOURCES)\
             $(kded_DATA) \
             $(kde_servicetypes_DATA) \
	     eventsrc README.debugging

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/kinetd
	$(INSTALL_DATA) $(srcdir)/eventsrc $(DESTDIR)$(kde_datadir)/kinetd/eventsrc

# set the include path for X, qt and KDE
INCLUDES= -I$(top_srcdir)/krfb/srvloc $(all_includes)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kinetd.pot

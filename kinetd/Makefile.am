METASOURCES = AUTO

# Code 
lib_LTLIBRARIES = libkded_kinetd.la

libkded_kinetd_la_SOURCES = kinetd.cpp kinetd.h
libkded_kinetd_la_LIBADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)

# Services
kde_servicetypes_DATA = kinetdmodule.desktop
kdeddir = $(kde_servicesdir)/kded
kded_DATA = kinetd.desktop

EXTRA_DIST = $(libkded_kinetd_la_SOURCES)\
             $(kded_DATA) \
             $(kde_servicetypes_DATA)

KDE_ICON = kinetd

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)



METASOURCES = AUTO

# Code 
noinst_LIBRARIES = libkinetd.a

libkinetd_a_SOURCES = kinetd.cpp kinetd.h
libkinetd_a_LIBADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)

# Services
kdedir = $(kde_servicesdir)/kded
kde_DATA = kdeprintd.desktop


EXTRA_DIST = $(libkinetd_a_SOURCES) $(kde_DATA)


KDE_ICON = kinetd

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)


CXXFLAGS = @CXXFLAGS@


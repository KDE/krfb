METASOURCES = AUTO

# Code 
noinst_LIBRARIES = kinetd.a

kinetd_a_SOURCES = kinetd.cpp
kinetd_a_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)

# Services
kdedir = $(kde_servicesdir)/kded
kde_DATA = kdeprintd.desktop


EXTRA_DIST = $(krfb_SOURCES) $(kde_DATA) kinetd.h


KDE_ICON = kinetd

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

# the library search path. 
kinetd_a_LDFLAGS = $(all_libraries) $(KDE_RPATH)

CXXFLAGS = @CXXFLAGS@


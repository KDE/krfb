METASOURCES = AUTO

# Code 
noinst_LTLIBRARIES = libsrvloc.la

libsrvloc_la_SOURCES = kserviceregistry.cpp uuid.cpp \
        kinetinterface.cpp kinetinterfacewatcher.cpp \
        getifaddrs.cpp

libsrvloc_la_LIBADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_SLP)
libsrvloc_la_LDFLAGS  = $(all_libraries) -no-undefined 
noinst_HEADERS = kserviceregistry.h uuid.h \
                 getifaddrs.h kinetinterface.h kinetinterfacewatcher.h

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)


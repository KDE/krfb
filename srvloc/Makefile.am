METASOURCES = AUTO

# Code 
noinst_LTLIBRARIES = libsrvloc.la

libsrvloc_la_SOURCES = kinetaddr.cpp kinetaddr_ipfinder.cpp \
	kserviceregistry.cpp kuser.cpp uuid.cpp
libsrvloc_la_LIBADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_SLP)
libsrvloc_la_LDFLAGS  = $(all_libraries) -no-undefined 
noinst_HEADERS = kinetaddr.h kserviceregistry.h kuser.h uuid.h

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)


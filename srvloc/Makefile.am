METASOURCES = AUTO

# Code 
lib_LTLIBRARIES = libsrvloc.la

libsrvloc_la_SOURCES = kinetaddr.cpp kinetaddr_ipfinder.cpp \
	kserviceregistry.cpp kuser.cpp
libsrvloc_la_LIBADD   = $(LIB_QT) $(LIB_KDECORE)
noinst_HEADERS = kinetaddr.h kserviceregistry.h kuser.h

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)


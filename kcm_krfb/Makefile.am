METASOURCES = AUTO

# Code
lib_LTLIBRARIES = libkcm_krfb.la

libkcm_krfb_la_SOURCES = configurationwidget.ui kcm_krfb.cpp kcm_krfb.h 
libkcm_krfb_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_krfb_la_LIBADD   = ../krfb/libkrfbconfig.la $(LIB_KDEUI) 

# Services
kcmdir = $(kde_appsdir)/Settings/System
kcm_DATA = kcmkrfb.desktop

EXTRA_DIST = $(libkcm_krfb_la_SOURCES)\
             $(kcm_DATA)

# set the include path for X, qt and KDE
INCLUDES= -I../krfb $(all_includes)



METASOURCES = AUTO

# Code
kde_module_LTLIBRARIES = kcm_krfb.la

kcm_krfb_la_SOURCES = configurationwidget.ui kcm_krfb.cpp
kcm_krfb_la_LDFLAGS  = $(all_libraries) $(KDE_PLUGIN) -module
kcm_krfb_la_LIBADD   = ../krfb/libkrfbconfig.la $(LIB_KDEUI) -lkio

# Services
xdg_apps_DATA = kcmkrfb.desktop

EXTRA_DIST = $(kcm_krfb_la_SOURCES)\
             $(xdg_apps_DATA)

# set the include path for X, qt and KDE
INCLUDES= -I../krfb $(all_includes)

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/kcm_krfb.pot

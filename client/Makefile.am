KDE_CXXFLAGS = $(USE_THREADS)

METASOURCES = AUTO

bin_PROGRAMS = krdc
krdc_SOURCES = main.cpp krdc.cpp kvncview.cpp threads.cpp \
  colour.c cursor.c d3des.c desktop.c rfbproto.c sockets.c vncauth.c \
  newconnectiondialog.ui passworddialog.ui

EXTRA_DIST = krdc.h kvncview.cpp vncviewer.h

krdc_LDADD   = $(LIB_KDEUI) -ljpeg
krdc_LDFLAGS = $(all_libraries) $(KDE_RPATH)

lnkdir = $(kde_appsdir)/Internet
lnk_DATA = krdc.desktop

KDE_ICON = AUTO

INCLUDES= $(all_includes)

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp *.cc -o $(podir)/krdc.pot

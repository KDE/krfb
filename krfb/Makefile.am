####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = krfb
krfb_SOURCES = XUpdateScanner.cc rfbserver.cpp main.cpp 
krfb_LDADD   = lib/liblib.a  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)  $(LIBSOCKET)


EXTRA_DIST = main.cpp krfb.desktop lo32-app-krfb.png lo16-app-krfb.png rfbserver.cpp rfbserver.h eyes-closed24.png eyes-open24.png XUpdateScanner.cc XUpdateScanner.h 

install-data-local:
	$(mkinstalldirs) $(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/krfb.desktop $(kde_appsdir)/Applications/krfb.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-krfb.png $(kde_icondir)/locolor/32x32/apps/krfb.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-krfb.png $(kde_icondir)/locolor/16x16/apps/krfb.png

uninstall-local:
	-rm -f $(kde_appsdir)/Applications/krfb.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/krfb.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/krfb.png

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) ../include

METASOURCES = AUTO

# the library search path. 
krfb_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE´s XML GUI builing
#rcdir = $(kde_datadir)/krfb
#rc_DATA = krfbui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

messages: 
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/krfb.pot; \
	fi


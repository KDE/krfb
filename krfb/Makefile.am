####### kdevelop will overwrite this part!!! (begin)##########
lib_LTLIBRARIES = libkrfb.la


INCLUDES = $(all_includes)


LDFLAGS =  

libkrfb_la_METASOURCES = AUTO

libkrfb_la_SOURCES = myview.ui krfb.cpp 


EXTRA_DIST = krfb.cpp krfb.h myview.ui krfb.desktop lo32-app-krfb.png 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/kicker/applets/
	$(INSTALL_DATA) $(srcdir)/krfb.desktop $(kde_datadir)/kicker/applets/krfb.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-krfb.png $(kde_icondir)/locolor/32x32/apps/krfb.png

uninstall-local:
	-rm -f $(kde_datadir)/kicker/applets/krfb.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/krfb.png

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
METASOURCES = AUTO

# them while "make clean", use CLEANFILES
DISTCLEANFILES = $(METASOURCES)

SOMAJOR = 1
SOMINOR = 0
SOSUBMINOR = 0  

# Linker flags
libkrfb_la_LDFLAGS = $(all_libraries) -version-info $(SOMAJOR):$(SOMINOR):$(SOSUBMINOR) $(LDFLAGS) -module
# Extra libraries
libkrfb_la_LIBADD = $(LIB_KDEUI) 

 
# make messages.po. Move this one to ../po/ and "make merge" in po
# the -x is for skipping messages already translated in kdelibs
messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		 $(XGETTEXT) -C -ki18n -x $(kde_includes)/kde.pot $$LIST -o ../po/krfb.pot; \
	fi


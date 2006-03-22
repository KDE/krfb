kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_krfb_PART_SRCS kcm_krfb.cpp )

kde4_automoc(${kcm_krfb_PART_SRCS})

kde4_add_ui_files(kcm_krfb_PART_SRCS configurationwidget.ui )

kde4_add_plugin(kcm_krfb ${kcm_krfb_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_krfb )

target_link_libraries(kcm_krfb  ${KDE4_KDECORE_LIBS} krfbconfig kdeui kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_krfb )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kcmkrfb.desktop )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
## Code
#kde_module_LTLIBRARIES = kcm_krfb.la
#
#kcm_krfb_la_SOURCES = configurationwidget.ui kcm_krfb.cpp
#kcm_krfb_la_LDFLAGS  = $(all_libraries) $(KDE_PLUGIN) -module
#kcm_krfb_la_LIBADD   = ../krfb/libkrfbconfig.la $(LIB_KDEUI) -lkio
#
## Services
#xdg_apps_DATA = kcmkrfb.desktop
#
#EXTRA_DIST = $(kcm_krfb_la_SOURCES)\
#             $(xdg_apps_DATA)
#
## set the include path for X, qt and KDE
#INCLUDES= -I../krfb $(all_includes)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kcm_krfb.pot

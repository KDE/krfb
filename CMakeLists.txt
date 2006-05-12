
add_subdirectory( srvloc ) 
add_subdirectory( kinetd ) 
add_subdirectory( libvncserver ) 
add_subdirectory( krfb ) 
add_subdirectory( kcm_krfb ) 
add_subdirectory( krfb_httpd ) 




########### install files ###############





#original Makefile.am contents follow:

#SUBDIRS = srvloc kinetd libvncserver krfb kcm_krfb krfb_httpd
#
#EXTRA_DIST = AUTHORS README TODO NOTES DCOP-INTERFACE
#
## not a GNU package. You can remove this line, if 
## have all needed files, that a GNU package needs
#AUTOMAKE_OPTIONS = foreign 
#

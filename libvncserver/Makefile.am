INCLUDES = $(all_includes)

noinst_LIBRARIES = libvncserver.a

libvncserver_a_SOURCES = main.c rfbserver.c sraRegion.c auth.c sockets.c \
	stats.c corre.c hextile.c rre.c translate.c cutpaste.c \
	zlib.c tight.c httpd.c cursor.c font.c \
	draw.c selbox.c d3des.c vncauth.c cargs.c

EXTRA_DIST = $(libvncserver_a_SOURCES) CHANGES COPYING \
	README TODO

CFLAGS = @CFLAGS@ -DHAVE_PTHREADS




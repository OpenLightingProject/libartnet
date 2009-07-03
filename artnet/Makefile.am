
INCLUDES = -I$(top_builddir) -I$(top_srcdir)
AM_CFLAGS = -Wall -Werror -Wformat -W

lib_LTLIBRARIES = libartnet.la
libartnet_la_SOURCES = artnet.c private.h misc.c network.c receive.c \
                       transmit.c tod.c
libartnet_la_LDFLAGS = -version-info 1:0:0

if USING_WIN32
libartnet_la_LDFLAGS += -no-undefined -liphlpapi -lnetapi32 -lcap -lws2_32 \
                        -ldpnet
endif

EXTRA_DIST = tod.h misc.h

pkginclude_HEADERS = artnet.h packets.h common.h
pkgincludedir = $(includedir)/artnet

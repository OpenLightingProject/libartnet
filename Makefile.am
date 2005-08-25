
auxdir                  = @ac_aux_dir@
AUX_DIST                = $(auxdir)/install-sh $(auxdir)/missing \
                        $(auxdir)/mkinstalldirs

SUBDIRS = artnet 

EXTRA_DIST = reconf libartnet.pc.in Doxyfile

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libartnet.pc

ACLOCAL                 = aclocal -I $(auxdir)

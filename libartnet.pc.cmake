prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@


Name: libartnet
Version: @PROJECT_VERSION@
Description: libartnet - ArtNet DMX over IP library
Requires:
Libs: -L${libdir} -lartnet
Cflags: -I${includedir}/artnet

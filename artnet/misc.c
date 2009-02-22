/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * misc.c
 * Misc code for libartnet (almost nothing these days)
 * Copyright (C) 2004-2005 Simon Newton
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#define _GNU_SOURCE
#include "private.h"
#include <stdarg.h>
#include <stdio.h>

#if HAVE_ENDIAN_H
# include <endian.h>
#else
# ifdef HAVE_PPC_ENDIAN_H
#  include <ppc/endian.h>
# else
#  error No endian defined!
# endif
#endif

#include <assert.h>

/*
 * libartnet error function
 *
 * This writes the error string to artnet_errstr, which can be accessed
 * using artnet_strerror();
 *
 * Is vasprintf available on OSX ?
 */
void artnet_error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  free(artnet_errstr);
  if (vasprintf(&artnet_errstr, fmt , ap) < 0)
    printf("An error occured, but memory could not be allocated"
           "to display it\n");
  va_end(ap);
}


// converts 4 bytes in big endian order to a 32 bit int
int32_t artnet_misc_nbytes_to_32(uint8_t bytes[4]) {
  int32_t o = 0;

  o = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];

  return o;

}

void artnet_misc_int_to_bytes(int data, uint8_t *bytes) {
    bytes[3] = (data & 0x000000FF);
    bytes[2] = (data & 0x0000FF00) >> 8;
    bytes[1] = (data & 0x00FF0000) >> 16;
    bytes[0] = (data & 0xFF000000) >> 24;
}



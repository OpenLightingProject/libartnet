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
 * common.c
 * Constants required for both artnet.h and packets.h
 * Copyright (C) 2004-2005 Simon Newton

 */

/** 
 * The maximum ports per node built into the ArtNet protocol.
 * This is always 4. Don't change it unless you really know what your doing
 */
#define ARTNET_MAX_PORTS	4

/**
 * The length of the short name field. Always 18
 */
#define ARTNET_SHORT_NAME_LENGTH	18

/**
 * The length of the long name field. Always 64 
 */
#define ARTNET_LONG_NAME_LENGTH		64

/**
 * The length of the report field. Always 64
 */
#define ARTNET_REPORT_LENGTH		64

/**
 * The length of the DMX field. Always 512
 */
#define ARTNET_DMX_LENGTH	512


/*
 * Number of bytes in a RDM UID
 */
#define ARTNET_RDM_UID_WIDTH 6

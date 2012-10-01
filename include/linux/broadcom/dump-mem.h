/*****************************************************************************
* Copyright 2001 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL"). 
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

/****************************************************************************
*
*  dump-mem.h
*
*  PURPOSE:
*
*       This file contains functions and macros for dumping memory.
*
*****************************************************************************/

#if !defined( LINUX_BROADCOM_DUMP_MEM_H )
#define LINUX_BROADCOM_DUMP_MEM_H

/* ---- Include Files ---------------------------------------------------- */

#include <linux/types.h>

/* ---- Constants and Types ---------------------------------------------- */

typedef enum {
	DUMP_TO_NONE = 0x00,
	DUMP_TO_CONSOLE = 0x01,
	DUMP_TO_KNLLOG = 0x02,

} DUMP_DEST;

/* ---- Variable Externs ------------------------------------------------- */

#define DUMP_MEM(dumpDest, addr, mem, numBytes ) if ( dumpDest ) dump_mem( __FUNCTION__, dumpDest, addr, mem, numBytes );

/* ---- Function Prototypes ---------------------------------------------- */

void dump_mem(const char *function, DUMP_DEST dumpDest, uint32_t addr,
	      const void *mem, size_t numBytes);

void printk_dump_mem(const char *label, uint32_t addr, const void *voidMem,
		     size_t numBytes);

#endif /* LINUX_BROADCOM_DUMP_MEM_H */

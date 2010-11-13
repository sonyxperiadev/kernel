/*****************************************************************************
* Copyright 2004 - 2008 Broadcom Corporation.  All rights reserved.
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

/*
 *  GPIO boards table
 */

/* ---- Include Files ----------------------------------------------------- */
#include <asm/mach-types.h>
#include "gpiomux_boards.h"

#if 0
/* Currently unused. See gpiomux_boards.c for more on this. */
typedef struct
{
	const int machine_number;
	const gpiomux_group_t groupList[];
	const unsigned int groupListEntries;
}
gpiomux_boards_t;


/* When the boot eventually starts passing machine id's into the kernel that are unique per board,
 * we could use a simple lookup table like this instead of the current method of using externs and 
 * CONFIG settings. */

/* Note that HERARAY_RAMPAGE is not a separate machine id, probably should be. */
gpiomux_boards_t gpiomux_boards[] = 
{
		{ MACH_BCMHANA_BASE, 			gpiomux_GroupListEntries_BCMHANA_BASE, 			gpiomux_GroupList_BCMHANA_BASE},
		{ MACH_BCMHANA_FPGA, 			gpiomux_GroupListEntries_BCMHANA_FPGA, 			gpiomux_GroupList_BCMHANA_FPGA},
		{ MACH_BCMHANA_HERARAY, 		gpiomux_GroupListEntries_BCMHANA_HERARAY, 		gpiomux_GroupList_BCMHANA_HERARAY},
		{ MACH_BCMHANA_HERA_TABLET, 	gpiomux_GroupListEntries_BCMHANA_HERA_TABLET, 	gpiomux_GroupList_BCMHANA_HERA_TABLET}
};

#else

/* The externs below can eventually be removed when the table above
 * is in use and the boot is passing actual machine id's into the kernel */
extern const gpiomux_group_t gpiomux_GroupList_BCMHANA_BASE[];
extern const unsigned int gpiomux_GroupListEntries_BCMHANA_BASE;
extern const gpiomux_group_t gpiomux_GroupList_BCMHANA_FPGA[];
extern const unsigned int gpiomux_GroupListEntries_BCMHANA_FPGA;
extern const gpiomux_group_t gpiomux_GroupList_BCMHANA_HERARAY[];
extern const unsigned int gpiomux_GroupListEntries_BCMHANA_HERARAY;
extern const gpiomux_group_t gpiomux_GroupList_BCMHANA_HERARAY_RAMPAGE[];
extern const unsigned int gpiomux_GroupListEntries_BCMHANA_HERARAY_RAMPAGE;
extern const gpiomux_group_t gpiomux_GroupList_BCMHANA_HERA_TABLET[];
extern const unsigned int gpiomux_GroupListEntries_BCMHANA_HERA_TABLET;

void gpiomux_boards_init(void)
{
   /* 
    * Until the machine numbers are passed in correctly from the bootstrap, we have to
    * restrict which gpiomux tables are used. Later we can switch on machine id. Another
    * way to do this might be to register the gpiomux tables in an array in memory vs
    * machine id and find the matching table. See gpiomux_boards.c for a possible implementation.
    */

   /* For now just use the ARCH to decide what table to activate. This won't work when multiple
    * boards are present at the same time. Remove this when the bootstrap passes in the correct
    * machine id. */
#ifdef CONFIG_MACH_BCMHANA_BASE
	gpiomux_earlyinit(gpiomux_GroupListEntries_BCMHANA_BASE,             gpiomux_GroupList_BCMHANA_BASE);
   
// #elif defined(CONFIG_MACH_BCMHANA_FPGA)
#elif defined(CONFIG_MACH_BCM2850_FPGA)
	gpiomux_earlyinit(gpiomux_GroupListEntries_BCMHANA_FPGA,             gpiomux_GroupList_BCMHANA_FPGA);
   
#elif defined(CONFIG_MACH_BCMHANA_HERARAY)
	gpiomux_earlyinit(gpiomux_GroupListEntries_BCMHANA_HERARAY,          gpiomux_GroupList_BCMHANA_HERARAY);
   
#elif defined(CONFIG_MACH_BCMHANA_HERARAY_RAMPAGE)
	gpiomux_earlyinit(gpiomux_GroupListEntries_BCMHANA_HERARAY_RAMPAGE,  gpiomux_GroupList_BCMHANA_HERARAY_RAMPAGE);
   
#elif defined(CONFIG_MACH_BCMHANA_HERA_TABLET)
	gpiomux_earlyinit(gpiomux_GroupListEntries_HERA_TABLET,              gpiomux_GroupList_HERA_TABLET);
   
#else
#error Unsupported architecture. Create gpiomux table for this arch.
#endif
}

#endif

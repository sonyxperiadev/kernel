/*****************************************************************************
* Copyright 2005 - 2009 Broadcom Corporation.  All rights reserved.
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



#ifndef __ASM_ARCH_ARAM_LAYOUT_H
#define __ASM_ARCH_ARAM_LAYOUT_H

#include <mach/csp/mm_io.h>

/*
 * Base definitions
 */
#define BCMHANA_ARAM_START MM_IO_BASE_SRAM
#define BCMHANA_ARAM_SIZE  0x28000           /* 160k */
#define BCMHANA_ARAM_END   (BCMHANA_ARAM_START + BCMHANA_ARAM_SIZE)

/*
 * Secure Data Cache defined by CONFIG_BCMRING_SECURE_DATA_CACHE
 */

#if !defined( CONFIG_BCMHANA_SECURE_DATA_CACHE )
#define CONFIG_BCMHANA_SECURE_DATA_CACHE 0
#endif
#if CONFIG_BCMHANA_SECURE_DATA_CACHE
#define BCMHANA_ARAM_SDC_START  MM_IO_PHYS_TO_VIRT(CONFIG_BCMHANA_SECURE_DATA_CACHE_LOCATION)
#define BCMHANA_ARAM_SDC_SIZE   CONFIG_BCMHANA_SECURE_DATA_CACHE_SIZE
#else
#define BCMHANA_ARAM_SDC_START  BCMHANA_ARAM_START
#define BCMHANA_ARAM_SDC_SIZE   0
#endif
#define BCMHANA_ARAM_SDC_END    (BCMHANA_ARAM_SDC_START + BCMHANA_ARAM_SDC_SIZE)

#if (BCMHANA_ARAM_SDC_START < BCMHANA_ARAM_START)
#error SDC start address out of range
#endif
#if (BCMHANA_ARAM_SDC_END > BCMHANA_ARAM_END)
#error SDC size out of range
#endif

/*
 * Power Management given 32K of space immediately after SDC (assumed to be 0)
 */

#if ( BCMHANA_ARAM_SDC_SIZE != 0 )
#error Currently require BCMHANA ARAM SDC SIZE to be zero
#endif

#define BCMHANA_ARAM_PM_START   BCMHANA_ARAM_SDC_END
#define BCMHANA_ARAM_PM_SIZE    0x8000  /* 32K */
#define BCMHANA_ARAM_PM_MM_SIZE SZ_32K  /* Used for MEM_DESC */

/*
 * Videocore is given the remaining 128K 
 *  
 * Any changes made to BCMHANA_ARAM_VC_OFFSET must also be reflected in 
 * IPC_SHARED_MEM_VC_OFFSET, which can be found in 
 * vc4/src/vcinclude/bigisland_chip/ipc_shared_mem.h 
 */

#define BCMHANA_ARAM_VC_OFFSET  (BCMHANA_ARAM_PM_SIZE + BCMHANA_ARAM_SDC_SIZE)
#define BCMHANA_ARAM_VC_START   (BCMHANA_ARAM_START + BCMHANA_ARAM_VC_OFFSET)
#define BCMHANA_ARAM_VC_SIZE    0x20000 /* 128K */
#define BCMHANA_ARAM_VC_MM_SIZE SZ_128K

#endif /* __ASM_ARCH_ARAM_LAYOUT_H */


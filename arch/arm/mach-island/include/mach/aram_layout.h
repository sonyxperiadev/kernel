/*****************************************************************************
* Copyright 2005 - 2011 Broadcom Corporation.  All rights reserved.
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

#include <mach/io_map.h>

/*
 * Base definitions
 */
#define BCMHANA_ARAM_START KONA_INT_SRAM_BASE
#define BCMHANA_ARAM_SIZE  0x28000           /* 160k */
#define BCMHANA_ARAM_END   (BCMHANA_ARAM_START + BCMHANA_ARAM_SIZE)

/*
 * Power Management given 32K of space
 */

#define BCMHANA_ARAM_PM_START   BCMHANA_ARAM_START
#define BCMHANA_ARAM_PM_SIZE    0x8000  /* 32K */
#define BCMHANA_ARAM_PM_MM_SIZE SZ_32K  /* Used for MEM_DESC */

/*
 * Videocore is given the remaining 128K
 *
 * Any changes made to BCMHANA_ARAM_VC_OFFSET must also be reflected in
 * IPC_SHARED_MEM_VC_OFFSET, which can be found in the vc4-dev tree
 * in interface/vchiq_arm/vchiq_bi_ipc_shared_mem.h. The corresponding
 * location of that file in the kernel tree is
 * drivers/videocore/interface/vchiq_arm/vchiq_bi_ipc_shared_mem.h
 */

#define BCMHANA_ARAM_VC_OFFSET  (BCMHANA_ARAM_PM_SIZE)
#define BCMHANA_ARAM_VC_START   (BCMHANA_ARAM_START + BCMHANA_ARAM_VC_OFFSET)
#define BCMHANA_ARAM_VC_SIZE    0x20000 /* 128K */
#define BCMHANA_ARAM_VC_MM_SIZE SZ_128K

#endif /* __ASM_ARCH_ARAM_LAYOUT_H */


/*****************************************************************************
* Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
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

#ifndef	__ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

/*
 * Physical vs virtual RAM address space conversion.  These are
 * private definitions which should NOT be used outside memory.h
 * files.  Use virt_to_phys/phys_to_virt/__pa/__va instead.
 */

#define PHYS_OFFSET (CONFIG_BCM_RAM_BASE+CONFIG_BCM_RAM_START_RESERVED_SIZE)

#ifndef HW_IO_PHYS_TO_VIRT
#define HW_IO_PHYS_TO_VIRT MM_IO_PHYS_TO_VIRT
#endif
#define HW_IO_VIRT_TO_PHYS MM_IO_VIRT_TO_PHYS
/*
 * Maximum DMA memory allowed is 14M
 */
#define CONSISTENT_DMA_SIZE (SZ_16M - SZ_2M)

#endif /* __ASM_ARCH_MEMORY_H */

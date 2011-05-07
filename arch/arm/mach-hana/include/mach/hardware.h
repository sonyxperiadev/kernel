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

#ifndef	__ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>
#include <mach/memory.h>
#include <mach/csp/mm_io.h>

/* Hardware addresses of major areas.
 *  *_START is the physical address
 *  *_SIZE  is the size of the region
 *  *_BASE  is the virtual address
 */
#define RAM_START               PHYS_OFFSET
#define RAM_SIZE                (CONFIG_BCM_RAM_SIZE-CONFIG_BCM_RAM_START_RESERVED_SIZE)
#define RAM_BASE                PAGE_OFFSET

#define pcibios_assign_all_busses() 1

/*
 * Macros to make managing spinlocks a bit more controlled in terms of naming.
 * See reg_gpio.h, reg_irq.h, arch.c, gpio.c for example usage.
 */
#if defined( __KERNEL__ )
#define HW_DECLARE_SPINLOCK(name)  spinlock_t g##name##RegLock = SPIN_LOCK_UNLOCKED;
#define HW_EXTERN_SPINLOCK(name)   extern spinlock_t g##name##RegLock;
#define HW_IRQ_SAVE(name, val)     spin_lock_irqsave(&g##name##RegLock,(val))
#define HW_IRQ_RESTORE(name, val)  spin_unlock_irqrestore(&g##name##RegLock,(val))
#else
#define HW_DECLARE_SPINLOCK(name)
#define HW_EXTERN_SPINLOCK(name)
#define HW_IRQ_SAVE(name, val)     {(void)(name);(void)(val);}
#define HW_IRQ_RESTORE(name, val)  {(void)(name);(void)(val);}
#endif

#define IO_START MM_IO_START
#define IO_BASE  MM_IO_BASE

#endif	/* __ASM_ARCH_HARDWARE_H */

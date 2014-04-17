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
/*
 * derived from arch/arm/mach-realview/include/mach/smp.h
 */

#ifndef __ASM_ARCH_SMP_H
#define __ASM_ARCH_SMP_H __FILE__

#include <linux/irqchip/arm-gic.h>

/*
 * set_event() is used to wake up secondary core from wfe using sev. ROM
 * code puts the second core into wfe(standby).
 *
 */
#define set_event()	__asm__ __volatile__ ("sev" : : : "memory")

extern void kona_secondary_startup(void);

#define hard_smp_processor_id()			\
	({						\
		unsigned int cpunum;			\
		__asm__("mrc p15, 0, %0, c0, c0, 5"	\
			: "=r" (cpunum));		\
		cpunum &= 0x0F;				\
	})

#endif /* __ASM_ARCH_SMP_H */

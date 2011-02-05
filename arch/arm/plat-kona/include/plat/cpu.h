/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/cpu.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
 * CPU versions related functions/macros
 */

#ifndef ASM_ARCH_CPU_H
#define ASM_ARCH_CPU_H

#define cpu_is_bcm215xx()	(0)
#define cpu_is_bcm2153()	(0)
#define cpu_is_bcm2157()	(0)

#if defined(CONFIG_ARCH_BCM2153)
#undef cpu_is_bcm2153
#define cpu_is_bcm2153()	(1)
#endif

#if defined(CONFIG_ARCH_BCM2157)
#undef cpu_is_bcm2157
#define cpu_is_bcm2157()	(1)
#endif

#if defined(CONFIG_ARCH_BCM215XX)
#undef cpu_is_bcm215xx
#define cpu_is_bcm215xx()	(1)
#endif

#endif /* ASM_ARCH_CPU_H */

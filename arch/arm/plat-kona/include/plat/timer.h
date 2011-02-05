/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/timer.h
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

/**
 * @file
 * @brief Broadcom specific timer implementation
 */
#ifndef __ARCH_PLAT_TIMER_H__
#define __ARCH_PLAT_TIMER_H__

#include <linux/compiler.h>
/**
 * timer configuration identifying the timer to use.
 * @param cs_base	The base address of clock source timer. This is the address of GPT base.
 * @param ce_base	The base address of clock event timer.
 * @param cs_index	The index of GPT to use as clock source.
 * @param ce_index	The index of the clock event timer to use.
 * @param irq		The irq associated with clock event.
 */
struct timer_config {
	void __iomem *cs_base;
	void __iomem *ce_base;
	void __iomem *cp_cs_base;
	unsigned int cs_index;
	unsigned int ce_index;
	unsigned int cp_cs_index;
	unsigned int irq;
};

typedef unsigned int timer_tick_count_t;
unsigned int timer_get_tick_count(void);
unsigned long timer_get_cp_tick_count(void);

#endif

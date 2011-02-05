/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	arch/arm/plat-bcmap/include/plat/clock.h
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
 *
 *   @file   clock.h
 *   @brief  Broadcom platform clock framework interface functions.
 *
 ****************************************************************************/

#ifndef __ASM_ARCH_BCM_CLOCK_H__
#define __ASM_ARCH_BCM_CLOCK_H__

/* Clock flags */
#define BCM_CLK_ENABLE_ON_INIT		(1 << 0)	/* Enable at start of the day */
#define BCM_CLK_INVERT_ENABLE		(1 << 1)	/* 0 enables, 1 disables */
#define BCM_CLK_ALWAYS_ENABLED		(1 << 2)	/* clock can't be disabled */

/** Common clock structure. */
struct clk {
	u8 cnt;			/**< Count of drivers using this clock. */
	u16 flags;
	u8 id;			/**< ID */
	int (*enable) (struct clk *);	/**< enable function for this clock. */
	void (*disable) (struct clk *);	/**< disable */
	int (*set_rate) (struct clk *, unsigned long);	/**< Set rate. */
	unsigned long (*get_rate) (struct clk *);	/**< get clock rate. */
	long (*round_rate) (struct clk *, unsigned long);	/**< rounded freq for a desired freq. */
	int (*set_parent) (struct clk *, struct clk *);	/**< Set parent */
	struct clk *parent;
	unsigned long rate;	/**< Current clock rate (Hz) of parent or self ? Need to */
	u8 enable_bit_mask;
	void __iomem *enable_reg;	/**< Clk enable register address */

};

#endif /*__ASM_ARCH_BCM_CLOCK_H__*/

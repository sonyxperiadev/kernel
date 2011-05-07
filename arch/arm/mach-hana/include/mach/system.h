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

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

#include <asm/io.h>

#include <mach/csp/mm_io.h>

static inline void arch_idle(void)
{
	cpu_do_idle();
}

#define IROOT_RST_MGR_REG_WR_ACCESS_OFFSET			0x00000000
#define  IROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK	0x80000000
#define  IROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT		8
#define  IROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK		0x00000001
#define IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET			0x00000004
#define  IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK	0x80000000

#define GICDIST_ENABLE_S_OFFSET 0

static inline void arch_reset(char mode, const char *cmd)
{
	volatile u32 *reg;
	u32 val;

	/* Do not forward pending interrupts to CPUs */
	reg = (volatile u32 *)((u32)MM_IO_BASE_GICDIST + GICDIST_ENABLE_S_OFFSET);
	__raw_writel(0, reg);

	/* enable reset register access */
	reg = (volatile u32 *)((u32)MM_IO_BASE_ROOT_RST + IROOT_RST_MGR_REG_WR_ACCESS_OFFSET);   
	val  = __raw_readl(reg);
	val &= IROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;         /* retain access mode      */
	val |= (0xA5A5 << IROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);    /* set password            */
	val |= IROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;               /* set access enable       */
	__raw_writel(val, reg);

	/* trigger reset */
	reg = (volatile u32 *)((u32)MM_IO_BASE_ROOT_RST + IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
	val  = __raw_readl(reg);
	val &= IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK;    /* retain access mode      */
	__raw_writel(val, reg);

	while(1);
}

#endif /*__ASM_ARCH_SYSTEM_H */

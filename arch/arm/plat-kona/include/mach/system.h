/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/************************************************************************************************/

#ifndef __PLAT_KONA_SYSTEM_H
#define __PLAT_KONA_SYSTEM_H

#include <linux/io.h>
#include <mach/io_map.h>
#if defined(CONFIG_ARCH_HAWAII)
#include <mach/rdb/brcm_rdb_gicdist.h>
#endif
#if defined(CONFIG_ARCH_JAVA)
#include <mach/rdb/brcm_rdb_gic.h>
#endif
#if defined(CONFIG_ARCH_ISLAND)
#include <mach/rdb/brcm_rdb_iroot_rst_mgr_reg.h>
#else
#include <mach/rdb/brcm_rdb_root_rst_mgr_reg.h>
#include <mach/rdb/brcm_rdb_bmdm_rst_mgr_reg.h>
#endif

#if defined( CONFIG_KONA_WFI_WORKAROUND )
#include <mach/wfi_count.h>
#endif

#ifdef CONFIG_BCM_IDLE_PROFILER
#include <mach/profile_timer.h>
#endif

#ifdef CONFIG_BCM_KNLLOG_IRQ
#include <linux/broadcom/knllog.h>
#endif

#ifdef CONFIG_BCM_IDLE_PROFILER
DECLARE_PER_CPU(u32, idle_count);
#endif

static void kona_reset(char mode, const char *cmd)
{
	unsigned int val;

	/*
	 * Disable GIC interrupt distribution.
	 */
#if defined(CONFIG_ARCH_HAWAII)
	__raw_writel(0, KONA_GICDIST_VA + GICDIST_ENABLE_S_OFFSET);
#endif
#if defined(CONFIG_ARCH_JAVA)
	__raw_writel(0, KONA_GICDIST_VA + GIC_GICD_CTLR_OFFSET);
#endif

#if defined(CONFIG_ARCH_ISLAND)
	/* enable reset register access */
	val =
	    __raw_readl(KONA_ROOT_RST_VA + IROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	val &= IROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= (0xA5A5 << IROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);	/* set password                        */
	val |= IROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;	/* set access enable           */
	__raw_writel(val,
		     KONA_ROOT_RST_VA + IROOT_RST_MGR_REG_WR_ACCESS_OFFSET);

	/* trigger reset */
	val =
	    __raw_readl(KONA_ROOT_RST_VA +
			IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
	val &= IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	__raw_writel(val,
		     KONA_ROOT_RST_VA +
		     IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
#else

	/* enable reset register access */
	val = __raw_readl(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	val &= ROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= (0xA5A5 << ROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);	/* set password                        */
	val |= ROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;	/* set access enable           */
	__raw_writel(val, KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);

	/*
	 * Its safe to reset the R4 only after Chip Soft Reset.
	 * Otherwise if CP is performing some register access that is pending
	 * and if we reset the CP that could potentially lock up the bus.
	 *
	 * R4 reset is done from the Loader any way, which is after
	 * Chip Soft Reset. So we should not perform CP reset from here
	 *
	 * The problem becauase this is seen only on few Ivory Phones
	 * CSP 570028
	 */
	/*Reset the AP */
	/* trigger reset */
	val =
	    __raw_readl(KONA_ROOT_RST_VA +
			ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
	val &= ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	__raw_writel(val,
		     KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
#endif

	while (1) ;
}

#endif /*__PLAT_KONA_SYSTEM_H */

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
#include <mach/rdb/brcm_rdb_gicdist.h>
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

static void arch_idle(void)
{
#ifdef CONFIG_BCM_IDLE_PROFILER
	u32 idle_enter, idle_leave;
#endif

#ifdef CONFIG_BCM_KNLLOG_IRQ
	if (gKnllogIrqSchedEnable & KNLLOG_THREAD)
		KNLLOGCALL("schedule", "0 -> 99999");
#endif

#ifdef CONFIG_BCM_IDLE_PROFILER
	idle_enter = timer_get_tick_count();
#endif

#if defined( CONFIG_KONA_WFI_WORKAROUND )
	/*
	 * We have an issue (SW-7022) where is both cores do a WFI, then the memory controller 
	 * slows down, and in BIVCM mode, the videocore DMA's to/from the ARM memory slow down 
	 * dramatically. So as a workaround, we have the BIVCM code increment wfi_count while 
	 * it's doing a transfer. The following code will prevent the second core from doing 
	 * a WFI while the videocore is transferring from the ARM memory. 
	 */

	if (wfi_workaround_enabled) {
		if (atomic_inc_return(&wfi_count) <= 2) {
			/*
			 * This should do all the clock switching
			 * and wait for interrupt tricks
			 */
			cpu_do_idle();
		}
		atomic_dec(&wfi_count);
	} else
#endif
	{
		/*
		 * This should do all the clock switching
		 * and wait for interrupt tricks
		 */
		cpu_do_idle();
	}

#ifdef CONFIG_BCM_IDLE_PROFILER
	idle_leave = timer_get_tick_count();
	get_cpu_var(idle_count) += (idle_leave - idle_enter);
	put_cpu_var(idle_count);
#endif

#ifdef CONFIG_BCM_KNLLOG_IRQ
	if (gKnllogIrqSchedEnable & KNLLOG_THREAD)
		KNLLOGCALL("schedule", "99999 -> 0");
#endif
}

static void arch_reset(char mode, const char *cmd)
{
	unsigned int val;

	/*
	 * Disable GIC interrupt distribution.
	 */
	__raw_writel(0, KONA_GICDIST_VA + GICDIST_ENABLE_S_OFFSET);

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
	/*Reset the CP */
	/* enable reset register access */
	val = __raw_readl(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	val &= ROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= (0xA5A5 << ROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);	/* set password                        */
	val |= ROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;	/* set access enable           */
	__raw_writel(val, KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);

	val =
	    __raw_readl(KONA_ROOT_RST_VA +
			ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);
	val &= ~ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_MASK;
	__raw_writel(val,
		     KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);

	val =
	    __raw_readl(KONA_ROOT_RST_VA +
			ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);
	val |= ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_MASK;
	__raw_writel(val,
		     KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET);

	val = __raw_readl(KONA_BMDM_RST_VA + BMDM_RST_MGR_REG_WR_ACCESS_OFFSET);
	val &= BMDM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= (0xA5A5 << BMDM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);	/* set password                        */
	val |= BMDM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;	/* set access enable           */
	__raw_writel(val, KONA_BMDM_RST_VA + BMDM_RST_MGR_REG_WR_ACCESS_OFFSET);

	/* trigger reset */
	val = __raw_readl(KONA_BMDM_RST_VA + BMDM_RST_MGR_REG_CP_RSTN_OFFSET);
	val &= BMDM_RST_MGR_REG_CP_RSTN_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= BMDM_RST_MGR_REG_CP_RSTN_CP_DEBUG_RSTN_MASK;
	__raw_writel(val, KONA_BMDM_RST_VA + BMDM_RST_MGR_REG_CP_RSTN_OFFSET);

	val = __raw_readl(KONA_BMDM_RST_VA + BMDM_RST_MGR_REG_CP_RSTN_OFFSET);
	val &= BMDM_RST_MGR_REG_CP_RSTN_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |=
	    (BMDM_RST_MGR_REG_CP_RSTN_CP_RSTN_MASK |
	     BMDM_RST_MGR_REG_CP_RSTN_CP_DEBUG_RSTN_MASK);
	__raw_writel(val, KONA_BMDM_RST_VA + BMDM_RST_MGR_REG_CP_RSTN_OFFSET);

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

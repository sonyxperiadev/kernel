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
#include <mach/rdb/brcm_rdb_kpm_clk_mgr_reg.h>
#include <mach/rdb/brcm_rdb_emmcsdxc.h>

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

#ifdef CONFIG_BCM_IDLE_PROFILER
#include <mach/profile_timer.h>
#endif

#ifdef CONFIG_BCM_KNLLOG_IRQ
#include <linux/broadcom/knllog.h>
#endif

#include <plat/cpu.h>

#ifdef CONFIG_BCM_IDLE_PROFILER
DECLARE_PER_CPU(u32, idle_count);
#endif

#define RAW_WRITEL(v, a)	__raw_writel(v, (void __iomem __force *)a)
#define RAW_READL(a)		__raw_readl((void __iomem __force *)a)

#ifdef CONFIG_DISABLE_USBBOOT_NEXTBOOT
static void  __disable_usb_in_next_boot(void)
{
	int reg_val;

	/*
	 * Temporary:
	 * Disable USB Boot i.e when reboot command is issued
	 * Boot ROM would ignore looking for USB connection and will
	 * boot from eMMC - suggestion from BOOT ROM team
	 */
	reg_val = readl(KONA_CHIPREG_VA + 0x1C);
	pr_info("Address:0x%x Value:0x%x \r\n",
		KONA_CHIPREG_VA + 0x1C, reg_val);
	reg_val &= ~0x00000002;
	writel(reg_val, KONA_CHIPREG_VA + 0x1C);
	reg_val = readl(KONA_CHIPREG_VA + 0x1C);
	pr_info("Address:0x%x Value:0x%x \r\n",
		KONA_CHIPREG_VA + 0x1C, reg_val);
}
#endif

static void kona_reset(char mode, const char *cmd)
{
	unsigned int val;
	unsigned int timeout;

#ifdef CONFIG_DISABLE_USBBOOT_NEXTBOOT
	if (get_chip_id() < KONA_CHIP_ID_JAVA_A1)
		__disable_usb_in_next_boot();
#endif

	/*
	 * Disable GIC interrupt distribution.
	 */
#if defined(CONFIG_ARCH_HAWAII)
	RAW_WRITEL(0, KONA_GICDIST_VA + GICDIST_ENABLE_S_OFFSET);
#endif
#if defined(CONFIG_ARCH_JAVA)
	RAW_WRITEL(0, KONA_GICDIST_VA + GIC_GICD_CTLR_OFFSET);
#endif

	/*
	 * Some Hynix eMMC don't like abrupt clock shutdown.  When RPM
	 * autosuspend is enabled, it takes time for the clock to be cut,
	 * but during this time, the system reboot can abruptly cut it off.
	 * Avoid that by disabling the clock to the card, right before
	 * system reboot.
	 */

	/* Turn on KPM CCU */
	val = KPM_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK |
		(0xa5a5 << KPM_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);
	RAW_WRITEL(val, KONA_KPM_CLK_VA + KPM_CLK_MGR_REG_WR_ACCESS_OFFSET);

	/* Enable eMMC SDHCI clkgate */
	val = RAW_READL(KONA_KPM_CLK_VA + KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET);
	val |= KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_HW_SW_GATING_SEL_MASK |
		KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_CLK_EN_MASK |
		KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_HW_SW_GATING_SEL_MASK |
		KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_CLK_EN_MASK;
	RAW_WRITEL(val, KONA_KPM_CLK_VA + KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET);

	/* Wait for eMMC SDHCI clkgate to be enabled */
	timeout = 10000;
	do {
		val = RAW_READL(KONA_KPM_CLK_VA +
				KPM_CLK_MGR_REG_SDIO2_CLKGATE_OFFSET);
		val &= KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_STPRSTS_MASK |
			KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_STPRSTS_MASK;

		if (val == (KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_STPRSTS_MASK |
			KPM_CLK_MGR_REG_SDIO2_CLKGATE_SDIO2_AHB_STPRSTS_MASK))
			break;

		udelay(1);
		timeout--;
	} while (timeout > 0);

	/* Disable eMMC SDHCI clock if SDHCI clkgate is enabled successfully */
	if (timeout > 0) {
		val = RAW_READL(KONA_SDIO2_VA + EMMCSDXC_CTRL1_OFFSET);
		val &= ~EMMCSDXC_CTRL1_SDCLKEN_MASK;
		RAW_WRITEL(val, KONA_SDIO2_VA + EMMCSDXC_CTRL1_OFFSET);
	}

#if defined(CONFIG_ARCH_ISLAND)
	/* enable reset register access */
	val =
	    RAW_READL(KONA_ROOT_RST_VA + IROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	val &= IROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= (0xA5A5 << IROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);	/* set password                        */
	val |= IROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;	/* set access enable           */
	RAW_WRITEL(val,
		     KONA_ROOT_RST_VA + IROOT_RST_MGR_REG_WR_ACCESS_OFFSET);

	/* trigger reset */
	val =
	    RAW_READL(KONA_ROOT_RST_VA +
			IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
	val &= IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	RAW_WRITEL(val,
		     KONA_ROOT_RST_VA +
		     IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
#else

	/* enable reset register access */
	val = RAW_READL(KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);
	val &= ROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	val |= (0xA5A5 << ROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT);	/* set password                        */
	val |= ROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK;	/* set access enable           */
	RAW_WRITEL(val, KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_WR_ACCESS_OFFSET);

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
	    RAW_READL(KONA_ROOT_RST_VA +
			ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
	val &= ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK;	/* retain access mode          */
	RAW_WRITEL(val,
		     KONA_ROOT_RST_VA + ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET);
#endif

	while (1) ;
}

#endif /*__PLAT_KONA_SYSTEM_H */

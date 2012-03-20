#*****************************************************************************
# Copyright 2003 - 2008 Broadcom Corporation.  All rights reserved.
#
# Unless you and Broadcom execute a separate written software license
# agreement governing use of this software, this software is licensed to you
# under the terms of the GNU General Public License version 2, available at
# http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
#
# Notwithstanding the above, under no circumstances may you combine this
# software in any way with any other Broadcom software provided under a
# license other than the GPL, without Broadcom's express prior written
# consent.
#*****************************************************************************

obj-y := irq.o kona_timer.o timer.o profile_timer.o pinmux.o sysfs.o scu.o
obj-$(CONFIG_SMP) += headsmp.o platsmp.o
obj-$(CONFIG_LOCAL_TIMERS) += localtimer.o
obj-$(CONFIG_HOTPLUG_CPU)  += hotplug.o
obj-$(CONFIG_ARCH_REQUIRE_GPIOLIB)  += gpio.o
obj-$(CONFIG_HAVE_CLK) += clock.o
obj-$(CONFIG_MACH_RHEA_RAY) += chal/
obj-$(CONFIG_MACH_RHEA_FARADAY_EB10) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_EDN2X) += chal/
obj-$(CONFIG_MACH_RHEA_SS) += chal/
obj-$(CONFIG_MACH_RHEA_SS_AMAZING) += chal/
obj-$(CONFIG_MACH_RHEA_SS_LUCAS) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_DEMO) += chal/
obj-$(CONFIG_MACH_RHEA_BERRI) += chal/
obj-$(CONFIG_MACH_RHEA_BERRI_EDN40) += chal/
obj-$(CONFIG_MACH_RHEALC_BERRI) += chal/
obj-$(CONFIG_MACH_RHEA_RAY) += csl/
obj-$(CONFIG_MACH_RHEA_FARADAY_EB10) += csl/
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += csl/
obj-$(CONFIG_MACH_RHEA_RAY_EDN2X) += csl/
obj-$(CONFIG_MACH_RHEA_SS) += csl/
obj-$(CONFIG_MACH_RHEA_SS_AMAZING) += csl/
obj-$(CONFIG_MACH_RHEA_SS_LUCAS) += csl/
obj-$(CONFIG_MACH_RHEA_RAY_DEMO) += csl/
obj-$(CONFIG_MACH_RHEA_BERRI) += csl/
obj-$(CONFIG_MACH_RHEA_BERRI_EDN40) += csl/
obj-$(CONFIG_MACH_RHEALC_BERRI) += csl/
obj-$(CONFIG_MACH_SAMOA_RAY) += chal/
obj-$(CONFIG_MACH_ISLAND_FF) += chal/
obj-$(CONFIG_MACH_ISLAND_IS) += chal/
obj-$(CONFIG_MACH_RHEA_STONE) += csl/
obj-$(CONFIG_MACH_RHEA_STONE) += chal/
obj-$(CONFIG_MACH_RHEA_STONE_EDN2X) += csl/
obj-$(CONFIG_MACH_RHEA_STONE_EDN2X) += chal/
# SAMOA RAY bringup only has keypad at this point, so no CSL
#obj-$(CONFIG_MACH_SAMOA_RAY) += csl/
obj-$(CONFIG_DMAC_PL330) += dmux.o dma.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += kona_pm.o kona_pm_dbg.o
obj-$(CONFIG_KONA_POWER_MGR) += pwr_mgr.o
obj-$(CONFIG_KONA_PI_MGR) += pi_mgr.o
obj-$(CONFIG_KONA_CPU_FREQ_DRV) += kona_cpufreq.o
obj-$(CONFIG_KONA_AVS) += kona_avs.o
obj-$(CONFIG_KONA_ATAG_DT) += atag_dt.o
obj-$(CONFIG_USB_DWC_OTG) += bcm_hsotgctrl.o
obj-$(CONFIG_PROC_PINMUX_DUMP)	+= pindump.o
obj-$(CONFIG_KONA_DT_BCMPMU)	+= bcmpmu-dt.o

# wfi_count.o is required for SW-7022.
obj-$(CONFIG_KONA_WFI_WORKAROUND) += wfi_count.o

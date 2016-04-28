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

obj-y := irq.o kona_timer.o timer.o pinmux.o sysfs.o kona_mic_bias.o ns_ioremap.o
obj-$(CONFIG_ARCH_HAWAII) += scu.o profile_timer.o
obj-$(CONFIG_SMP) += headsmp.o platsmp.o
obj-$(CONFIG_HOTPLUG_CPU)  += hotplug.o
obj-$(CONFIG_ARCH_REQUIRE_GPIOLIB)  += gpio.o
obj-$(CONFIG_HAVE_CLK) += clock.o clock_debug.o
obj-$(CONFIG_MACH_RHEA_RAY) += chal/
obj-$(CONFIG_MACH_RHEA_FARADAY_EB10) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_EDN2X) += chal/
obj-$(CONFIG_MACH_RHEA_DALTON2_EB30) += chal/
obj-$(CONFIG_MACH_RHEA_SS) += chal/
obj-$(CONFIG_MACH_RHEA_SS_AMAZING) += chal/
obj-$(CONFIG_MACH_RHEA_SS_LUCAS) += chal/
obj-$(CONFIG_MACH_RHEA_SS_ZANIN) += chal/
obj-$(CONFIG_MACH_RHEA_SS_ZANIN_02) += chal/
obj-$(CONFIG_MACH_RHEA_SS_ZANIN_04) += chal/
obj-$(CONFIG_MACH_RHEA_SS_IVORY) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_DEMO) += chal/
obj-$(CONFIG_MACH_RHEA_BERRI) += chal/
obj-$(CONFIG_MACH_RHEA_BERRI_EDN40) += chal/
obj-$(CONFIG_MACH_RHEALC_BERRI) += chal/
obj-$(CONFIG_ARCH_HAWAII) += chal/
obj-$(CONFIG_ARCH_HAWAII) += csl/
obj-$(CONFIG_ARCH_JAVA) += chal/
obj-$(CONFIG_ARCH_JAVA) += csl/
obj-$(CONFIG_MACH_RHEA_RAY) += csl/
obj-$(CONFIG_MACH_RHEA_FARADAY_EB10) += csl/
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += csl/
obj-$(CONFIG_MACH_RHEA_RAY_EDN2X) += csl/
obj-$(CONFIG_MACH_RHEA_DALTON2_EB30) += csl/
obj-$(CONFIG_MACH_RHEA_SS) += csl/
obj-$(CONFIG_MACH_RHEA_SS_AMAZING) += csl/
obj-$(CONFIG_MACH_RHEA_SS_LUCAS) += csl/
obj-$(CONFIG_MACH_RHEA_SS_ZANIN) += csl/
obj-$(CONFIG_MACH_RHEA_SS_ZANIN_02) += csl/
obj-$(CONFIG_MACH_RHEA_SS_ZANIN_04) += csl/
obj-$(CONFIG_MACH_RHEA_SS_IVORY) += csl/
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
#obj-$(CONFIG_DMAC_PL330) += dmux.o

obj-$(CONFIG_KONA_CPU_PM_HANDLER) += kona_pm.o kona_pm_dbg.o
obj-$(CONFIG_KONA_POWER_MGR) += pwr_mgr.o
obj-$(CONFIG_KONA_PI_MGR) += pi_mgr.o
obj-$(CONFIG_KONA_CPU_FREQ_DRV) += kona_cpufreq.o
obj-$(CONFIG_KONA_ATAG_DT) += atag_dt.o
obj-$(CONFIG_KONA_USB_CONTROL) += bcm_hsotgctrl.o bcm_hsotgctrl_phy_mdio.o
obj-$(CONFIG_PROC_PINMUX_DUMP)	+= pindump.o
obj-$(CONFIG_KONA_DT_BCMPMU)	+= bcmpmu-dt.o

ifeq ($(CONFIG_1MHZ_SLV_SYSTEM_TIMER),y)
KBUILD_CFLAGS += -DTIMER_NAME="\"slave\""
else
ifeq ($(CONFIG_ARCH_HAWAII), y)
KBUILD_CFLAGS += -DTIMER_NAME="\"aon\""
else
KBUILD_CFLAGS += -DTIMER_NAME="\"core\""
endif
endif

obj-$(CONFIG_LOCAL_TIMERS)	+= localtimer.o

#kona profiler
obj-$(CONFIG_KONA_PROFILER) += profiler.o ccu_profiler.o pi_profiler.o
obj-$(CONFIG_KONA_MEMC) += kona_memc.o
obj-$(CONFIG_KONA_SECURE_MEMC) += kona_secure_memc.o
obj-$(CONFIG_KONA_AXITRACE)	+= axitrace.o
obj-$(CONFIG_BRCM_CDC)	+= cdc.o

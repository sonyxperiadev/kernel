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

obj-y := irq.o kona_timer.o timer.o profile_timer.o pinmux.o sysfs.o
obj-$(CONFIG_SMP) += headsmp.o platsmp.o
obj-$(CONFIG_LOCAL_TIMERS) += localtimer.o
obj-$(CONFIG_HOTPLUG_CPU)  += hotplug.o
obj-$(CONFIG_USB_GADGET_DWC_OTG) += lm.o 
obj-$(CONFIG_ARCH_REQUIRE_GPIOLIB)  += gpio.o 
obj-$(CONFIG_HAVE_CLK) += clock.o
obj-$(CONFIG_MACH_RHEA_RAY) += chal/
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += chal/
obj-$(CONFIG_MACH_RHEA_RAY) += csl/
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += csl/
obj-$(CONFIG_MACH_SAMOA_RAY) += chal/
obj-$(CONFIG_MACH_ISLAND_FF) += chal/
# SAMOA RAY bringup only has keypad at this point, so no CSL
#obj-$(CONFIG_MACH_SAMOA_RAY) += csl/
obj-$(CONFIG_DMAC_PL330) += dmux.o dma.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += kona_pm.o
ifneq ($(CONFIG_ARCH_SAMOA),y) 
	obj-y += pwr_mgr.o pi_mgr.o
endif


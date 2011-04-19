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

ifeq ($(CONFIG_ARCH_SAMOA),y)
obj-y := irq.o kona_timer.o timer.o
else
obj-y := irq.o kona_timer.o timer.o pinmux.o sysfs.o
endif
obj-$(CONFIG_SMP) += headsmp.o platsmp.o
obj-$(CONFIG_LOCAL_TIMERS) += localtimer.o
obj-$(CONFIG_HOTPLUG_CPU)  += hotplug.o
obj-$(CONFIG_USB_GADGET_DWC_OTG) += lm.o 
obj-$(CONFIG_ARCH_REQUIRE_GPIOLIB)  += gpio.o 
ifneq ($(CONFIG_ARCH_SAMOA),y)
obj-$(CONFIG_HAVE_CLK) += clock.o
endif
obj-$(CONFIG_MACH_RHEA_RAY) += chal/
obj-$(CONFIG_MACH_RHEA_RAY) += csl/
obj-$(CONFIG_DMAC_PL330) += dmux.o dma.o

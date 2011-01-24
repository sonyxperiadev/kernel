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

obj-y := irq.o timer.o 
obj-$(CONFIG_SMP) += headsmp.o smp.o
obj-$(CONFIG_LOCAL_TIMERS) += localtimer.o
obj-$(CONFIG_HOTPLUG_CPU)  += hotplug.o
obj-$(CONFIG_USB_GADGET_DWC_OTG) += lm.o 
obj-$(CONFIG_ARCH_REQUIRE_GPIOLIB)  += gpio.o gpio_irq.o
obj-$(CONFIG_HAVE_CLK) += clock.o
obj-$(CONFIG_CACHE_L2X0)+= l2x0.o

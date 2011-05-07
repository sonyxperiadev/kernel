#*****************************************************************************
# Copyright 2011 Broadcom Corporation.  All rights reserved.
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

obj-y	:= mm.o arch.o clock.o

obj-$(CONFIG_SMP)			+= smp.o headsmp.o

obj-$(CONFIG_MACH_HANA_BASE) 	+= board_base.o
CFLAGS_board_base.o += -Iarch/arm/mach-hana/hw_cfg/BASE

obj-$(CONFIG_MACH_HANA_SV) 		+= board_sv.o pinmux_hana.o pinmux_sv.o
CFLAGS_board_sv.o += -Iarch/arm/mach-hana/hw_cfg/HANA_SV \
-Iarch/arm/mach-hana/hw_cfg/BASE

obj-$(CONFIG_MACH_HANA_TABLET) 	+= board_tablet.o pinmux_hana.o pinmux_tablet.o
CFLAGS_board_tablet.o += -Iarch/arm/mach-hana/hw_cfg/HANA_TABLET \
-Iarch/arm/mach-hana/hw_cfg/BASE

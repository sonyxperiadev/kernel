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

obj-y := io_map.o island.o ipc_chip.o island_pinmux.o
obj-$(CONFIG_MACH_ISLAND_FPGA) += board_fpga.o
obj-$(CONFIG_MACH_ISLAND_BU) += board_bu.o
obj-$(CONFIG_MACH_ISLAND_FF) += board_ff.o island_ff_pinmux.o
obj-$(CONFIG_HAVE_CLK) += clock_island.o

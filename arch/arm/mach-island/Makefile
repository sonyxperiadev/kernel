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

obj-y := io_map.o island.o ipc_chip.o island_pinmux.o common.o

obj-$(CONFIG_HAVE_CLK) += clock_island.o

obj-$(CONFIG_NET_ISLAND) += ethHw_driver.o
ethHw_driver-objs := ethHw.o ethHw_asm.o

obj-$(CONFIG_PM) += pm.o

obj-$(CONFIG_MACH_ISLAND_FPGA) += board_fpga.o
obj-$(CONFIG_MACH_ISLAND_BU) += board_bu.o pinmux_island_bu.o
obj-$(CONFIG_MACH_ISLAND_FF) += board_ff.o pinmux_island_ff.o board-bcm59055.o

obj-$(CONFIG_MAP_SDMA) += chal/
obj-$(CONFIG_MAP_SDMA) += sdmac.o dma_mmap.o dump-mem.o dma_mmap_test.o

obj-$(CONFIG_MAP_AADMA) += chal/
obj-$(CONFIG_MAP_AADMA) += aadmac.o

obj-$(CONFIG_MAP_SSASW) += chal/
obj-$(CONFIG_MAP_SSASW) += ssasw.o

obj-$(CONFIG_MAP_LITTLE_ISLAND_MEMC_QOS) += memc_qos.o

obj-$(CONFIG_IPC) += chal/

obj-$(CONFIG_MACH_ISLAND_AUDIOH_CHAL) += chal/

obj-$(CONFIG_MACH_ISLAND_BASE) += board_base.o
CFLAGS_board_base.o += -Iarch/arm/mach-island/hw_cfg/BASE

obj-$(CONFIG_MACH_ISLAND_SV) += board_sv.o pinmux_island_sv.o
CFLAGS_board_sv.o += -Iarch/arm/mach-island/hw_cfg/ISLAND_SV \
-Iarch/arm/mach-island/hw_cfg/BASE \
-Ivc4-dev/src/interface/vceb/host \
-Ivc4-dev/src/interface/vceb/host/linux 

obj-$(CONFIG_MACH_ISLAND_TABLET) += board_tablet.o pinmux_island_tablet.o
CFLAGS_board_tablet.o += -Iarch/arm/mach-island/hw_cfg/ISLAND_TABLET \
-Iarch/arm/mach-island/hw_cfg/BASE \
-Ivc4-dev/src/interface/vceb/host \
-Ivc4-dev/src/interface/vceb/host/linux 

# Today the 28160 tablet is almost identical to the 11160 tablet from
# Linux's point-of-view so we include it's hw_cfg directory and only
# override where necessary, which today is no-where.
obj-$(CONFIG_MACH_ISLAND_TABLET_28160) += board_tablet_28160.o pinmux_island_tablet_28160.o
CFLAGS_board_tablet_28160.o += -Iarch/arm/mach-island/hw_cfg/ISLAND_TABLET_28160 \
-Iarch/arm/mach-island/hw_cfg/ISLAND_TABLET \
-Iarch/arm/mach-island/hw_cfg/BASE \
-Ivc4-dev/src/interface/vceb/host \
-Ivc4-dev/src/interface/vceb/host/linux 



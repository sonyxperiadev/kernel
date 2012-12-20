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

obj-y := io_map.o hawaii.o devices.o hawaii_pinmux.o hawaii-cache-l2x0.o
obj-$(CONFIG_ARCH_HAWAII) += clock.o
obj-$(CONFIG_MACH_HAWAII_RAY) += hawaii.o board-hawaii.o hawaii_wifi.o
obj-$(CONFIG_MACH_HAWAII_STONE) += hawaii.o board-hawaii.o hawaii_wifi.o
obj-$(CONFIG_MACH_HAWAII_GARNET) += hawaii.o board-garnet.o garnet_wifi.o
obj-$(CONFIG_MACH_HAWAII_FPGA) += hawaii_fpga.o
obj-$(CONFIG_MACH_HAWAII_FPGA_E) += hawaii_fpga.o
obj-$(CONFIG_CDEBUGGER) += crash_debugger.o
obj-$(CONFIG_KONA_POWER_MGR) += pwr_mgr.o
obj-$(CONFIG_KONA_PI_MGR) += pi_mgr.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += pm.o sleep.o pm_dbg.o
obj-$(CONFIG_KONA_SECURE_MONITOR_CALL) += sec_api.o
obj-$(CONFIG_HAWAII_PTM) += coresight.o
obj-$(CONFIG_KONA_PROFILER) += profiler.o

CFLAGS_sec_api.o :=$(call as-instr,.arch_extension sec,-DREQUIRES_SEC=1)

obj-$(CONFIG_ARCH_HAWAII) += hawaii_bmdm_wd_monitor.o
obj-$(CONFIG_A9_DORMANT_MODE) += dm.o dm_v7_c.o dm_a9.o dm_gic.o dm_a9_c.o dm_prepare.o
obj-y += sequencer_ucode.o pm_params.o helpers.o v7.o

ifneq ($(call as-instr,.arch_extension sec,+sec),)
AFLAGS_v7.o :=-Wa,-march=armv7-a+sec
endif
ifeq ($(CONFIG_MFD_BCM_PMU59056), y)
obj-y += board-bcm59056.o
endif
ifeq ($(CONFIG_MFD_BCM_PMU59xxx), y)
obj-y += board-bcm59xxx.o
endif

CFLAGS_dm.o :=$(call as-instr,.arch_extension sec,-DSMC_INSTR=1)

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

obj-y := io_map.o java.o devices.o java_pinmux.o
obj-$(CONFIG_ARCH_JAVA) += clock.o
obj-$(CONFIG_MACH_JAVA_RAY) += java.o board-java.o java_wifi.o
obj-$(CONFIG_MACH_JAVA_STONE) += java.o board-java.o java_wifi.o
obj-$(CONFIG_MACH_JAVA_GARNET) += java.o board-garnet.o java_wifi.o
obj-$(CONFIG_MACH_JAVA_SS_EVAL) += java.o board-ss_eval.o java_ss_eval_wifi.o
obj-$(CONFIG_MACH_JAVA_FPGA) += java_fpga.o
obj-$(CONFIG_MACH_JAVA_FPGA_E) += java_fpga.o
obj-$(CONFIG_CDEBUGGER) += crash_debugger.o
obj-$(CONFIG_KONA_POWER_MGR) += pwr_mgr.o
obj-$(CONFIG_KONA_PI_MGR) += pi_mgr.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += pm.o sleep.o pm_dbg.o
obj-$(CONFIG_KONA_SECURE_MONITOR_CALL) += sec_api.o
# obj-$(CONFIG_HAWAII_PTM) += coresight.o
obj-$(CONFIG_KONA_PROFILER) += profiler.o
obj-$(CONFIG_KONA_AXITRACE) += java_axitrace.o

obj-$(CONFIG_KONA_AVS) += avs.o
CFLAGS_sec_api.o :=$(call as-instr,.arch_extension sec,-DREQUIRES_SEC=1)

obj-$(CONFIG_ARCH_JAVA) += java_bmdm_wd_monitor.o
obj-$(CONFIG_DORMANT_MODE) += dm.o dm_v7_c.o dm_a15.o dm_gic.o dm_prepare.o
obj-y += sequencer_ucode.o pm_params.o helpers.o v7.o volt_tbl.o

ifneq ($(call as-instr,.arch_extension sec,+sec),)
AFLAGS_v7.o :=-Wa,-march=armv7-a+sec
endif
ifeq ($(CONFIG_MFD_BCM_PMU59056), y)
obj-y += board-bcm59056.o
endif
ifeq ($(CONFIG_MFD_BCM_PMU59xxx), y)
ifeq ($(CONFIG_MACH_JAVA_GARNET), y)
ifeq ($(CONFIG_MACH_JAVA_C_LC1), y)
obj-y += board-bcm59xxx_garnet_c_lc1.o
else ifeq ($(CONFIG_MACH_JAVA_C_LC2), y)
obj-y += board-bcm59xxx_garnet_c_lc2.o
else
obj-y += board-bcm59xxx_garnet.o
endif
else
ifeq ($(CONFIG_MACH_JAVA_SS_EVAL), y)
obj-y += board-bcm59xxx_ss_eval.o
else
obj-y += board-bcm59xxx.o
endif
endif
endif

CFLAGS_dm.o :=$(call as-instr,.arch_extension sec,-DSMC_INSTR=1)

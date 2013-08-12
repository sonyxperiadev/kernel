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

obj-y := io_map.o hawaii.o devices.o hawaii_pinmux.o
obj-$(CONFIG_ARCH_HAWAII) += clock.o
obj-$(CONFIG_MACH_HAWAII_RAY) += hawaii.o board-hawaii.o hawaii_wifi.o
obj-$(CONFIG_MACH_HAWAII_STONE) += hawaii.o board-hawaii.o hawaii_wifi.o
obj-$(CONFIG_MACH_HAWAII_GARNET) += hawaii.o board-garnet.o hawaii_wifi.o
obj-$(CONFIG_MACH_HAWAII_SS_EVAL_REV00) += hawaii.o board-hawaii_ss_eval.o hawaii_ss_eval_wifi.o board-bcm59xxx-ss-eval.o
obj-$(CONFIG_MACH_HAWAII_SS_LOGAN_REV00) += hawaii.o board-hawaii_ss_logan.o hawaii_ss_logan_wifi.o board-bcm59xxx-ss-logan.o
obj-$(CONFIG_MACH_HAWAII_SS_LOGAN_REV01) += hawaii.o board-hawaii_ss_logan.o hawaii_ss_logan_wifi.o board-bcm59xxx-ss-logan.o
obj-$(CONFIG_MACH_HAWAII_SS_LOGAN_REV02) += hawaii.o board-hawaii_ss_logan.o hawaii_ss_logan_wifi.o board-bcm59xxx-ss-logan.o
obj-$(CONFIG_MACH_HAWAII_SS_LOGANDS) += hawaii.o board-hawaii_ss_logands.o hawaii_wifi.o board-bcm59xxx-ss-logands.o
obj-$(CONFIG_MACH_HAWAII_SS_CS02_REV00) += hawaii.o board-hawaii_ss_cs02.o hawaii_ss_cs02_wifi.o board-bcm59xxx-ss-cs02.o
obj-$(CONFIG_MACH_HAWAII_SS_LOGANDS_REV00_POWER) += hawaii.o board-hawaii_ss_logands.o hawaii_ss_logands_wifi.o board-bcm59xxx-ss-logands.o
obj-$(CONFIG_MACH_HAWAII_SS_LOGANDS_COMBINED) += hawaii.o board-hawaii_ss_logands.o hawaii_ss_logands_wifi.o board-bcm59xxx-ss-logands.o
obj-$(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV01) += hawaii.o board-hawaii_ss_goldenven.o hawaii_wifi.o board-bcm59xxx-ss-goldenven.o board-tsp-mxt224s.o
obj-$(CONFIG_MACH_HAWAII_SS_GOLDENVEN_REV02) += hawaii.o board-hawaii_ss_goldenven.o hawaii_wifi.o board-bcm59xxx-ss-goldenven.o board-tsp-mxt224s.o
obj-$(CONFIG_MACH_HAWAII_FPGA) += hawaii_fpga.o
obj-$(CONFIG_MACH_HAWAII_FPGA_E) += hawaii_fpga.o
obj-$(CONFIG_CDEBUGGER) += crash_debugger.o
obj-$(CONFIG_KONA_POWER_MGR) += pwr_mgr.o
obj-$(CONFIG_KONA_PI_MGR) += pi_mgr.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += pm.o sleep.o pm_dbg.o
obj-$(CONFIG_KONA_SECURE_MONITOR_CALL) += sec_api.o
obj-$(CONFIG_HAWAII_PTM) += coresight.o
obj-$(CONFIG_KONA_PROFILER) += profiler.o
obj-$(CONFIG_KONA_AXITRACE) += hawaii_axitrace.o

obj-$(CONFIG_KONA_AVS) += avs.o

CFLAGS_sec_api.o :=$(call as-instr,.arch_extension sec,-DREQUIRES_SEC=1)

obj-$(CONFIG_ARCH_HAWAII) += hawaii_bmdm_wd_monitor.o
obj-$(CONFIG_A9_DORMANT_MODE) += dm.o dm_v7_c.o dm_a9.o dm_gic.o dm_a9_c.o dm_prepare.o
obj-y += sequencer_ucode.o pm_params.o helpers.o v7.o volt_tbl.o

ifneq ($(call as-instr,.arch_extension sec,+sec),)
AFLAGS_v7.o :=-Wa,-march=armv7-a+sec
endif
ifeq ($(CONFIG_MFD_BCM_PMU59056), y)
obj-y += board-bcm59056.o
endif
ifeq ($(CONFIG_MFD_BCM_PMU59xxx), y)
ifeq ($(CONFIG_MACH_HAWAII_SS_COMMON),)
ifeq ($(CONFIG_MACH_HAWAII_GARNET), y)
ifeq ($(CONFIG_MACH_HAWAII_GARNET_C_A18), y)
obj-y += board-bcm59xxx_garnet_c_a18.o
else ifeq ($(CONFIG_MACH_HAWAII_GARNET_C_W68), y)
obj-y += board-bcm59xxx_garnet_c_w68.o
else
obj-y += board-bcm59xxx_garnet.o
endif
else
obj-y += board-bcm59xxx.o
endif
endif
endif

CFLAGS_dm.o :=$(call as-instr,.arch_extension sec,-DSMC_INSTR=1)

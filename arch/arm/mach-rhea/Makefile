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

obj-y := io_map.o rhea.o common.o rhea_pinmux.o cache-l2x0.o
obj-$(CONFIG_ARCH_RHEA) += clock_rhea.o
obj-$(CONFIG_MACH_HERA_RAY) += hera_ray.o
obj-$(CONFIG_MACH_RHEA_RAY) += rhea_ray.o rhea_ray_pinmux.o
obj-$(CONFIG_KONA_POWER_MGR) += rhea_pwr_mgr.o rhea_mm_workarounds.o
obj-$(CONFIG_KONA_PI_MGR) += rhea_pi_mgr.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += rhea_pm.o rhea_wfi.o rhea_pm_dbg.o
obj-$(CONFIG_KONA_PROFILER) += rhea_profiler.o
obj-$(CONFIG_RHEA_DORMANT_MODE) += rhea_dormant.o dm_pwr_policy_top.o
obj-$(CONFIG_MACH_RHEA_RAY_DEMO) += rhea_ray.o rhea_ray_pinmux.o
obj-$(CONFIG_MACH_RHEA_RAY_EDN1X) += rhea_ray.o rhea_ray_pinmux.o
obj-$(CONFIG_MACH_RHEA_RAY_EDN2X) += rhea_ray.o rhea_ray_pinmux.o board-rhea-wifi.o
obj-$(CONFIG_MACH_RHEA_SS) += rhea_ss.o rhea_ss_pinmux.o
obj-$(CONFIG_MACH_RHEA_SS_AMAZING) += rhea_ss_amazing.o rhea_ss_amazing_pinmux.o board-bcm59039-amazing.o board-rhea_ss_amazing-wifi.o
obj-$(CONFIG_MACH_RHEA_SS_LUCAS) += rhea_ss_lucas.o rhea_ss_lucas_pinmux.o board-bcm59039-lucas.o board-rhea_ss_lucas-wifi.o
obj-$(CONFIG_MACH_RHEA_SS_ZANIN) += rhea_ss_zanin.o rhea_ss_zanin_pinmux.o board-bcm59039-zanin.o board-rhea_ss_zanin-wifi.o
obj-$(CONFIG_MACH_RHEA_SS_ZANIN_02) += rhea_ss_zanin.o rhea_ss_zanin_pinmux.o board-bcm59039-zanin.o board-rhea_ss_zanin-wifi.o
obj-$(CONFIG_MACH_RHEA_SS_ZANIN_04) += rhea_ss_zanin.o rhea_ss_zanin_pinmux.o board-bcm59039-zanin.o board-rhea_ss_zanin-wifi.o
obj-$(CONFIG_MACH_RHEA_SS_IVORY) += rhea_ss_ivory.o board-bcm59039-ivory.o board-rhea_ss_ivory-wifi.o
obj-$(CONFIG_MACH_RHEA_FARADAY_EB10) += rhea_ray.o rhea_faraday_eb10_pinmux.o
obj-$(CONFIG_MACH_RHEA_FARADAY_EB20) += rhea_faraday.o rhea_faraday_eb20_pinmux.o
obj-$(CONFIG_MACH_RHEA_DALTON2_EB30) += rhea_dalton2.o rhea_dalton2_eb30_pinmux.o
obj-$(CONFIG_MACH_RHEA_BERRI) += rhea_berri.o rhea_berri_pinmux.o
obj-$(CONFIG_MACH_RHEA_BERRI_EDN40) += rhea_berri.o rhea_berri_pinmux.o board-rhea-wifi.o
obj-$(CONFIG_MACH_RHEALC_BERRI) += rhea_berri.o rhea_berri_pinmux.o
obj-$(CONFIG_MACH_RHEA_STONE) += rhea_stone.o board-rhea-wifi.o
obj-$(CONFIG_MACH_RHEA_STONE_EDN2X) += rhea_stone.o board-rhea-wifi.o
obj-$(CONFIG_CDEBUGGER) += crash_debugger.o
obj-$(CONFIG_ARCH_RHEA) += rhea_bmdm_wd_monitor.o
obj-$(CONFIG_RHEA_AVS) += rhea_avs.o
obj-y += pm_params.o

obj-$(CONFIG_ROM_SEC_DISPATCHER_LIB) += sec_bridge_tz.i
obj-$(CONFIG_ROM_SEC_DISPATCHER_SRC) += rhea_smc.o

CFLAGS_rhea_smc.o :=$(call as-instr,.arch_extension sec,-DSMC_INSTR=1)
ifeq ($(CONFIG_MFD_BCM_PMU59xxx),y)
obj-y += board-bcm59xxx.o
else
obj-$(CONFIG_MFD_BCM59055) += board-bcm59055.o
ifeq ($(CONFIG_MACH_RHEA_SS_COMMON),)
obj-$(CONFIG_MFD_BCM59039) += board-bcm59039.o
endif
obj-$(CONFIG_MFD_BCM59042) += board-bcm59042.o
endif

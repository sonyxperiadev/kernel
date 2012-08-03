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

obj-y := io_map.o hawaii.o devices.o hawaii_ray_pinmux.o hawaii_pinmux.o
obj-$(CONFIG_ARCH_HAWAII) += clock_hawaii.o
obj-$(CONFIG_MACH_HAWAII_RAY) += hawaii_ray.o
obj-$(CONFIG_MACH_HAWAII_FPGA) += hawaii_fpga.o
obj-$(CONFIG_MACH_HAWAII_FPGA_E) += hawaii_fpga.o
obj-$(CONFIG_CDEBUGGER) += crash_debugger.o
obj-$(CONFIG_MFD_BCM59039) += board-bcm59039.o
obj-$(CONFIG_KONA_POWER_MGR) += rhea_pwr_mgr.o rhea_mm_workarounds.o
obj-$(CONFIG_KONA_PI_MGR) += rhea_pi_mgr.o
obj-$(CONFIG_KONA_CPU_PM_HANDLER) += rhea_pm.o rhea_wfi.o rhea_pm_dbg.o
obj-y += pm_params.o

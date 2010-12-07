/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010 Broadcom Corporation                                                        */
/*                                                                                              */
/*     Unless you and Broadcom execute a separate written software license agreement governing  */
/*     use of this software, this software is licensed to you under the terms of the GNU        */
/*     General Public License version 2 (the GPL), available at                                 */
/*                                                                                              */
/*          http://www.broadcom.com/licenses/GPLv2.php                                          */
/*                                                                                              */
/*     with the following added to such license:                                                */
/*                                                                                              */
/*     As a special exception, the copyright holders of this software give you permission to    */
/*     link this software with independent modules, and to copy and distribute the resulting    */
/*     executable under terms of your choice, provided that you also meet, for each linked      */
/*     independent module, the terms and conditions of the license of that module.              */
/*     An independent module is a module which is not derived from this software.  The special  */
/*     exception does not apply to any modifications of the software.                           */
/*                                                                                              */
/*     Notwithstanding the above, under no circumstances may you combine this software in any   */
/*     way with any other Broadcom software provided under a license other than the GPL,        */
/*     without Broadcom's express prior written consent.                                        */
/*                                                                                              */
/*     Date     :    Generated on 11/9/2010 1:16:58                                            */
/*     RDB file : //HERA/                                                                    */
/************************************************************************************************/

#ifndef __REGISTERS_HERA_H__
#define __REGISTERS_HERA_H__

#include "brcm_rdb_a9cpu.h"
#include "brcm_rdb_a9pmu.h"
#include "brcm_rdb_a9ptm.h"
#include "brcm_rdb_aci.h"
#include "brcm_rdb_apbtoatb.h"
#include "brcm_rdb_aphy_csr.h"
#include "brcm_rdb_atb2ocp.h"
#include "brcm_rdb_atb2pti.h"
#include "brcm_rdb_atbfilter.h"
#include "brcm_rdb_audioh.h"
#include "brcm_rdb_auxmic.h"
#include "brcm_rdb_axitp1.h"
#include "brcm_rdb_cam.h"
#include "brcm_rdb_chipreg.h"
#include "brcm_rdb_crc.h"
#include "brcm_rdb_csitm.h"
#include "brcm_rdb_csr.h"
#include "brcm_rdb_cstf.h"
#include "brcm_rdb_cstpiu.h"
#include "brcm_rdb_cti.h"
#include "brcm_rdb_dma.h"
#include "brcm_rdb_dmux.h"
#include "brcm_rdb_dphy_csr.h"
#include "brcm_rdb_dsi1.h"
#include "brcm_rdb_etb.h"
#include "brcm_rdb_etb2axi.h"
#include "brcm_rdb_fmon.h"
#include "brcm_rdb_fshost.h"
#include "brcm_rdb_fshost_ctrl.h"
#include "brcm_rdb_giccpu.h"
#include "brcm_rdb_gicdist.h"
#include "brcm_rdb_gictr.h"
#include "brcm_rdb_glbtmr.h"
#include "brcm_rdb_globperf.h"
#include "brcm_rdb_gpio.h"
#include "brcm_rdb_hsi_apb.h"
#include "brcm_rdb_hsmcfg.h"
#include "brcm_rdb_hsmipc_ext.h"
#include "brcm_rdb_hsmipc_int.h"
#include "brcm_rdb_hsotg.h"
#include "brcm_rdb_hsotg_ctrl.h"
#include "brcm_rdb_hubocp2atb.h"
#include "brcm_rdb_hubsw.h"
#include "brcm_rdb_keypad.h"
#include "brcm_rdb_konatzcfg.h"
#include "brcm_rdb_kona_gptimer.h"
#include "brcm_rdb_mastersw.h"
#include "brcm_rdb_missing.h"
#include "brcm_rdb_mmsw.h"
#include "brcm_rdb_mm_cfg.h"
#include "brcm_rdb_mphi.h"
#include "brcm_rdb_mpu.h"
#include "brcm_rdb_nms_modembus_addr_remap.h"
#include "brcm_rdb_nms_pmu_shim.h"
#include "brcm_rdb_non_dmac.h"
#include "brcm_rdb_otp.h"
#include "brcm_rdb_padctrlreg.h"
#include "brcm_rdb_pka.h"
#include "brcm_rdb_pl310.h"
#include "brcm_rdb_ptmr_wd.h"
#include "brcm_rdb_pwm_top.h"
#include "brcm_rdb_pwrmgr.h"
#include "brcm_rdb_rng.h"
#include "brcm_rdb_scu.h"
#include "brcm_rdb_sdt.h"
#include "brcm_rdb_sec.h"
#include "brcm_rdb_sectrap.h"
#include "brcm_rdb_secwatchdog.h"
#include "brcm_rdb_sec_dmac.h"
#include "brcm_rdb_simi.h"
#include "brcm_rdb_slimbus.h"
#include "brcm_rdb_slimwrap.h"
#include "brcm_rdb_smi.h"
#include "brcm_rdb_spi.h"
#include "brcm_rdb_spm.h"
#include "brcm_rdb_spum_apb.h"
#include "brcm_rdb_spum_axi.h"
#include "brcm_rdb_sspil.h"
#include "brcm_rdb_syssw.h"
#include "brcm_rdb_te.h"
#include "brcm_rdb_tmon.h"
#include "brcm_rdb_uartb.h"
#include "brcm_rdb_v3d.h"
#include "brcm_rdb_vce.h"
#include "brcm_rdb_wgm_ocp2atb.h"
#include "brcm_rdb_xti.h"
#include "brcm_rdb_emmcsdxc.h"
#include "brcm_rdb_i2c_mm_hs.h"
#include "brcm_rdb_khub_clk_mgr_reg.h"
#include "brcm_rdb_khub_rst_mgr_reg.h"
#include "brcm_rdb_khubaon_clk_mgr_reg.h"
#include "brcm_rdb_khubaon_rst_mgr_reg.h"
#include "brcm_rdb_kpm_clk_mgr_reg.h"
#include "brcm_rdb_kpm_rst_mgr_reg.h"
#include "brcm_rdb_kproc_clk_mgr_reg.h"
#include "brcm_rdb_kproc_rst_mgr_reg.h"
#include "brcm_rdb_kps_clk_mgr_reg.h"
#include "brcm_rdb_kps_rst_mgr_reg.h"
#include "brcm_rdb_mdm_clk_mgr_reg.h"
#include "brcm_rdb_mdm_rst_mgr_reg.h"
#include "brcm_rdb_mm_clk_mgr_reg.h"
#include "brcm_rdb_mm_rst_mgr_reg.h"
#include "brcm_rdb_root_clk_mgr_reg.h"
#include "brcm_rdb_root_rst_mgr_reg.h"

#endif /* __REGISTERS_HERA_H__ */

/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010 Broadcom Corporation                                                         */
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
/*                                                                                              */
/*     Date     : Generated on 11/09/2010 01:16:58                                   */
/*     RDB file : //HERA/                                                                    */
/************************************************************************************************/

/**** Hub ****/

/**** APB5 ****/
#define HUB_CLK_BASE_ADDR         0x34000000 /* brcm_rdb_khub_clk_mgr_reg.h */
#define HUB_RST_BASE_ADDR         0x34000F00 /* brcm_rdb_khub_rst_mgr_reg.h */
#define HUBSWITCH_BASE_ADDR       0x34001000 /* brcm_rdb_hubsw.h */
#define MPU_BASE_ADDR             0x34002000 /* brcm_rdb_mpu.h */
#define TMON_BASE_ADDR            0x34008000 /* brcm_rdb_tmon.h */
#define SECTRAP7_BASE_ADDR        0x34009000 /* brcm_rdb_sectrap.h */
#define SECTRAP4_BASE_ADDR        0x3400A000 /* brcm_rdb_sectrap.h */
#define SECTRAP5_BASE_ADDR        0x3400B000 /* brcm_rdb_sectrap.h */
#define VARSPM_BASE_ADDR          0x3400F000 /* brcm_rdb_spm.h */

/**** APB9 ****/
#define AXITRACE10_BASE_ADDR      0x34010000 /* brcm_rdb_axitp1.h */
#define AXITRACE11_BASE_ADDR      0x34011000 /* brcm_rdb_axitp1.h */
#define AXITRACE12_BASE_ADDR      0x34012000 /* brcm_rdb_axitp1.h */
#define HUBOCP2ATB_BASE_ADDR      0x34015000 /* brcm_rdb_hubocp2atb.h */
#define ATBFILTER_BASE_ADDR       0x34016000 /* brcm_rdb_atbfilter.h */
#define ATB2OCP_BASE_ADDR         0x34017000 /* brcm_rdb_atb2ocp.h */
#define HUB_CTI_BASE_ADDR         0x34018000 /* brcm_rdb_cti.h */
#define HUB_FUNNEL_BASE_ADDR      0x34019000 /* brcm_rdb_cstf.h */
#define FIN_FUNNEL_BASE_ADDR      0x3401A000 /* brcm_rdb_cstf.h */
#define STM_BASE_ADDR             0x3401B000 /* brcm_rdb_xti.h */
#define ATB2PTI_BASE_ADDR         0x3401C000 /* brcm_rdb_atb2pti.h */
#define ETB_BASE_ADDR             0x3401D000 /* brcm_rdb_etb.h */
#define ETB2AXI_BASE_ADDR         0x3401E000 /* brcm_rdb_etb2axi.h */
#define TPIU_BASE_ADDR            0x3401F000 /* brcm_rdb_cstpiu.h */

/**** APB6 ****/
#define TZCFG_BASE_ADDR           0x35000000 /* brcm_rdb_konatzcfg.h */
#define ROOT_CLK_BASE_ADDR        0x35001000 /* brcm_rdb_root_clk_mgr_reg.h */
#define ROOT_RST_BASE_ADDR        0x35001F00 /* brcm_rdb_root_rst_mgr_reg.h */
#define AON_CLK_BASE_ADDR         0x35002000 /* brcm_rdb_khubaon_clk_mgr_reg.h */
#define AON_RST_BASE_ADDR         0x35002F00 /* brcm_rdb_khubaon_rst_mgr_reg.h */
#define GPIO2_BASE_ADDR           0x35003000 /* brcm_rdb_gpio.h */
#define KEYPAD_BASE_ADDR          0x35003800 /* brcm_rdb_keypad.h */
#define CHIPREGS_BASE_ADDR        0x35004000 /* brcm_rdb_chipreg.h */
#define PAD_CTRL_BASE_ADDR        0x35004800 /* brcm_rdb_padctrlreg.h */
#define SIM_BASE_ADDR             0x35005000 /* brcm_rdb_simi.h */
#define SIM2_BASE_ADDR            0x35005800 /* brcm_rdb_simi.h */
#define HUB_TIMER_BASE_ADDR       0x35006000 /* brcm_rdb_kona_gptimer.h */
#define MEMC0_SECURE_BASE_ADDR    0x35007000 /* brcm_rdb_sec.h */
#define MEMC0_OPEN_BASE_ADDR      0x35008000 /* brcm_rdb_csr.h */
#define MEMC0_OPEN_APHY_BASE_ADDR 0x35008400 /* brcm_rdb_aphy_csr.h */
#define MEMC0_OPEN_DPHY_BASE_ADDR 0x35008800 /* brcm_rdb_dphy_csr.h */
#define FMON_BASE_ADDR            0x3500B000 /* brcm_rdb_fmon.h */
#define SECWD_BASE_ADDR           0x3500C000 /* brcm_rdb_secwatchdog.h */
#define PMU_BSC_BASE_ADDR         0x3500D000 /* brcm_rdb_i2c_mm_hs.h */
#define AUXMIC_BASE_ADDR          0x3500E000 /* brcm_rdb_auxmic.h */
#define ACI_BASE_ADDR             0x3500E000 /* brcm_rdb_aci.h */
#define SPM_BASE_ADDR             0x3500F000 /* brcm_rdb_spm.h */

/**** Power_Manager ****/
#define PWRMGR_BASE_ADDR          0x35010000 /* brcm_rdb_pwrmgr.h */

/**** APB10 ****/
#define AUDIOH_BASE_ADDR          0x35020000 /* brcm_rdb_audioh.h */
#define SDT_BASE_ADDR             0x35020800 /* brcm_rdb_sdt.h */
#define SLIMWRAP_BASE_ADDR        0x35029000 /* brcm_rdb_slimwrap.h */
#define SLIMBUS_BASE_ADDR         0x35029400 /* brcm_rdb_slimbus.h */
#define SSP3_BASE_ADDR            0x3502B000 /* brcm_rdb_sspil.h */
#define HSI_BASE_ADDR             0x3502F000 /* brcm_rdb_hsi_apb.h */

/**** Comm_Subsystem ****/
#define R4CS_CTI_BASE_ADDR        0x3A5C2000 /* brcm_rdb_cti.h */
#define R4MPSS_CTI_BASE_ADDR      0x3A5C3000 /* brcm_rdb_cti.h */
#define R4_CTI_BASE_ADDR          0x3A5C4000 /* brcm_rdb_cti.h */

/**** APB13 ****/
#define MDM_CLK_BASE_ADDR         0x3BFF0000 /* brcm_rdb_mdm_clk_mgr_reg.h */
#define MDM_RST_BASE_ADDR         0x3BFF0F00 /* brcm_rdb_mdm_rst_mgr_reg.h */
#define R4_PMU_SHIM_BASE_ADDR     0x3BFF2000 /* brcm_rdb_nms_pmu_shim.h */
#define A9_MODEMBUS_ADDR_REMAP_BASE_ADDR 0x3BFF3000 /* brcm_rdb_nms_modembus_addr_remap.h */
#define A9_STMBUS_OCP2ATB_BASE_ADDR 0x3BFFB000 /* brcm_rdb_wgm_ocp2atb.h */
#define A9_AXI_TRACE15_BASE_ADDR  0x3BFFC000 /* brcm_rdb_axitp1.h */
#define A9_AXI_TRACE19_BASE_ADDR  0x3BFFE000 /* brcm_rdb_axitp1.h */
#define A9_ATB_FUNNEL_BASE_ADDR   0x3BFFF000 /* brcm_rdb_cstf.h */

/**** MM_Subsystem ****/
#define ISP_BASE_ADDR             0x3C080000 /* brcm_rdb_isp.h */
#define VCE_BASE_ADDR             0x3C100000 /* brcm_rdb_vce.h */
#define SMI_BASE_ADDR             0x3C180000 /* brcm_rdb_smi.h */
#define DSI0_BASE_ADDR            0x3C200000 /* brcm_rdb_dsi1.h */
#define DSI1_BASE_ADDR            0x3C280000 /* brcm_rdb_dsi1.h */

/**** APB12 ****/
#define MM_CLK_BASE_ADDR          0x3C000000 /* brcm_rdb_mm_clk_mgr_reg.h */
#define MM_RST_BASE_ADDR          0x3C000F00 /* brcm_rdb_mm_rst_mgr_reg.h */
#define MM_SWITCH_BASE_ADDR       0x3C001000 /* brcm_rdb_mmsw.h */
#define MM_TECTL_BASE_ADDR        0x3C002000 /* brcm_rdb_te.h */
#define MM_SPI_BASE_ADDR          0x3C003000 /* brcm_rdb_spi.h */
#define MM_CFG_BASE_ADDR          0x3C004000 /* brcm_rdb_mm_cfg.h */
#define MM_CSI0_BASE_ADDR         0x3C008000 /* brcm_rdb_cam.h */
#define MM_DMA_BASE_ADDR          0x3C00A000 /* brcm_rdb_dma.h */
#define MM_V3D_BASE_ADDR          0x3C00B000 /* brcm_rdb_v3d.h */
#define MM_CTI_BASE_ADDR          0x3C00C000 /* brcm_rdb_cti.h */
#define AXITRACE14_BASE_ADDR      0x3C00D000 /* brcm_rdb_axitp1.h */
#define AXITRACE17_BASE_ADDR      0x3C00E000 /* brcm_rdb_axitp1.h */
#define MM_FUNNEL_BASE_ADDR       0x3C00F000 /* brcm_rdb_cstf.h */

/**** Apps_Subsystem ****/
#define SPUM_NS_BASE_ADDR         0x3E300000 /* brcm_rdb_spum_axi.h */
#define SPUM_S_BASE_ADDR          0x3E310000 /* brcm_rdb_spum_axi.h */

/**** SECURITY ****/
#define SEC_PKA_BASE_ADDR         0x3E600000 /* brcm_rdb_pka.h */
#define SEC_OTP_BASE_ADDR         0x3E601000 /* brcm_rdb_otp.h */
#define SEC_RNG_BASE_ADDR         0x3E602000 /* brcm_rdb_rng.h */
#define SEC_CFG_BASE_ADDR         0x3E605000 /* brcm_rdb_hsmcfg.h */
#define SEC_SPUM_S_APB_BASE_ADDR  0x3E606000 /* brcm_rdb_spum_apb.h */
#define SEC_SPUM_NS_APB_BASE_ADDR 0x3E607000 /* brcm_rdb_spum_apb.h */
#define SEC_IPC_BASE_ADDR         0x3E680000 /* brcm_rdb_hsmipc_ext.h */

/**** Fabric ****/
#define L2C_BASE_ADDR             0x3FF20000 /* brcm_rdb_pl310.h */

/**** APB4 ****/
#define KONA_MST_CLK_BASE_ADDR    0x3F001000 /* brcm_rdb_kpm_clk_mgr_reg.h */
#define KONA_MST_RST_BASE_ADDR    0x3F001F00 /* brcm_rdb_kpm_rst_mgr_reg.h */
#define SECDMAC_BASE_ADDR         0x3F002000 /* brcm_rdb_sec_dmac.h */
#define NONDMAC_BASE_ADDR         0x3F003000 /* brcm_rdb_non_dmac.h */
#define SYSSWITCH_BASE_ADDR       0x3F004000 /* brcm_rdb_syssw.h */
#define MASTERSW_BASE_ADDR        0x3F005000 /* brcm_rdb_mastersw.h */
#define SECTRAP2_BASE_ADDR        0x3F006000 /* brcm_rdb_sectrap.h */
#define SECTRAP3_BASE_ADDR        0x3F007000 /* brcm_rdb_sectrap.h */

/**** APB8 ****/
#define GLOBPERF_BASE_ADDR        0x3F010000 /* brcm_rdb_globperf.h */
#define AXITRACE2_BASE_ADDR       0x3F012000 /* brcm_rdb_axitp1.h */
#define AXITRACE3_BASE_ADDR       0x3F013000 /* brcm_rdb_axitp1.h */
#define AXITRACE5_BASE_ADDR       0x3F015000 /* brcm_rdb_axitp1.h */
#define AXITRACE6_BASE_ADDR       0x3F016000 /* brcm_rdb_axitp1.h */
#define AXITRACE7_BASE_ADDR       0x3F017000 /* brcm_rdb_axitp1.h */
#define AXITRACE9_BASE_ADDR       0x3F019000 /* brcm_rdb_axitp1.h */
#define ARMITM_BASE_ADDR          0x3F01B000 /* brcm_rdb_csitm.h */
#define APB2ATB_BASE_ADDR         0x3F01C000 /* brcm_rdb_apbtoatb.h */
#define FAB_FUNNEL1_BASE_ADDR     0x3F01E000 /* brcm_rdb_cstf.h */
#define FAB_CTI_BASE_ADDR         0x3F01F000 /* brcm_rdb_cti.h */

/**** AHB2 ****/
#define HSOTG_BASE_ADDR           0x3F120000 /* brcm_rdb_hsotg.h */
#define HSOTG_CTRL_BASE_ADDR      0x3F130000 /* brcm_rdb_hsotg_ctrl.h */
#define FSHOST_BASE_ADDR          0x3F160000 /* brcm_rdb_fshost.h */
#define FSHOST_CTRL_BASE_ADDR     0x3F170000 /* brcm_rdb_fshost_ctrl.h */
#define SDIO1_BASE_ADDR           0x3F180000 /* brcm_rdb_emmcsdxc.h */
#define SDIO2_BASE_ADDR           0x3F190000 /* brcm_rdb_emmcsdxc.h */
#define SDIO3_BASE_ADDR           0x3F1A0000 /* brcm_rdb_emmcsdxc.h */

/**** Processor ****/

/**** APB0 ****/
#define PROC_CLK_BASE_ADDR        0x3FE00000 /* brcm_rdb_kproc_clk_mgr_reg.h */
#define PROC_RST_BASE_ADDR        0x3FE00F00 /* brcm_rdb_kproc_rst_mgr_reg.h */
#define SECTRAP1_BASE_ADDR        0x3FE01000 /* brcm_rdb_sectrap.h */
#define SECTRAP8_BASE_ADDR        0x3FE02000 /* brcm_rdb_sectrap.h */
#define AXITRACE1_BASE_ADDR       0x3FE03000 /* brcm_rdb_axitp1.h */
#define AXITRACE4_BASE_ADDR       0x3FE04000 /* brcm_rdb_axitp1.h */
#define GICTR_BASE_ADDR           0x3FE05000 /* brcm_rdb_gictr.h */
#define FUNNEL_BASE_ADDR          0x3FE06000 /* brcm_rdb_cstf.h */
#define AXITRACE16_BASE_ADDR      0x3FE07000 /* brcm_rdb_axitp1.h */

/**** APB11 ****/
#define A9CPU0_BASE_ADDR          0x3FE10000 /* brcm_rdb_a9cpu.h */
#define A9PMU0_BASE_ADDR          0x3FE11000 /* brcm_rdb_a9pmu.h */
#define A9PTM0_BASE_ADDR          0x3FE18000 /* brcm_rdb_a9ptm.h */
#define A9CTI0_BASE_ADDR          0x3FE1C000 /* brcm_rdb_cti.h */

/**** A9_PRIVATE ****/
#define SCU_BASE_ADDR             0x3FF00000 /* brcm_rdb_scu.h */
#define GICCPU_BASE_ADDR          0x3FF00100 /* brcm_rdb_giccpu.h */
#define GTIM_BASE_ADDR            0x3FF00200 /* brcm_rdb_glbtmr.h */
#define PTIM_BASE_ADDR            0x3FF00600 /* brcm_rdb_ptmr_wd.h */
#define GICDIST_BASE_ADDR         0x3FF01000 /* brcm_rdb_gicdist.h */

/**** Slaves ****/

/**** APB1 ****/
#define UARTB_BASE_ADDR           0x3E000000 /* brcm_rdb_uartb.h */
#define UARTB2_BASE_ADDR          0x3E001000 /* brcm_rdb_uartb.h */
#define UARTB3_BASE_ADDR          0x3E002000 /* brcm_rdb_uartb.h */
#define SSP0_BASE_ADDR            0x3E004000 /* brcm_rdb_sspil.h */
#define SSP1_BASE_ADDR            0x3E005000 /* brcm_rdb_sspil.h */
#define TIMER_BASE_ADDR           0x3E00D000 /* brcm_rdb_kona_gptimer.h */
#define DMUX_BASE_ADDR            0x3E00E000 /* brcm_rdb_dmux.h */

/**** APB2 ****/
#define KONA_SLV_CLK_BASE_ADDR    0x3E011000 /* brcm_rdb_kps_clk_mgr_reg.h */
#define KONA_SLV_RST_BASE_ADDR    0x3E011F00 /* brcm_rdb_kps_rst_mgr_reg.h */
#define BSC1_BASE_ADDR            0x3E016000 /* brcm_rdb_i2c_mm_hs.h */
#define BSC2_BASE_ADDR            0x3E017000 /* brcm_rdb_i2c_mm_hs.h */
#define PWM_BASE_ADDR             0x3E01A000 /* brcm_rdb_pwm_top.h */

/**** AHB1 ****/
#define MPHI_BASE_ADDR            0x3E130000 /* brcm_rdb_mphi.h */


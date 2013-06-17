/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013 Broadcom Corporation                                                         */
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
/*     Date     : Generated on 03/04/2013 11:52:05                                   */
/*     RDB file : //JAVA/                                                                    */
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
#define AXITRACE19_BASE_ADDR      0x34010000 /* brcm_rdb_axitp2.h */
#define AXITRACE11_BASE_ADDR      0x34011000 /* brcm_rdb_axitp2.h */
#define AXITRACE12_BASE_ADDR      0x34012000 /* brcm_rdb_axitp2.h */
#define ATBFILTER_BASE_ADDR       0x34016000 /* brcm_rdb_atbfilter.h */
#define HUB_CTI_BASE_ADDR         0x34018000 /* brcm_rdb_cti.h */
#define HUB_FUNNEL_BASE_ADDR      0x34019000 /* brcm_rdb_cstf.h */
#define FIN_FUNNEL_BASE_ADDR      0x3401A000 /* brcm_rdb_cstf.h */
#define STM_BASE_ADDR             0x3401B000 /* brcm_rdb_atb_stm.h */
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
#define MEMC0_OPEN_PWRWDOG_BASE_ADDR 0x350083C0 /* brcm_rdb_pwrwdog.h */
#define MEMC0_OPEN_APHY_BASE_ADDR 0x35008400 /* brcm_rdb_aphy_csr.h */
#define MEMC0_OPEN_DPHY_BASE_ADDR 0x35008800 /* brcm_rdb_dphy_csr.h */
#define MMMMU_SECURE_BASE_ADDR    0x35009000 /* brcm_rdb_mmmmu_secure.h */
#define MMMMU_OPEN_BASE_ADDR      0x3500A000 /* brcm_rdb_mmmmu_open.h */
#define CORE_TIMER_BASE_ADDR      0x3500A800 /* brcm_rdb_kona_gptimer.h */
#define MEMC_SWITCH_BASE_ADDR     0x3500AC00 /* brcm_rdb_memcsw.h */
#define MEMC_SPARE_BASE_ADDR      0x3500AE00 /* brcm_rdb_memcspare.h */
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
#define SSP4_BASE_ADDR            0x35028000 /* brcm_rdb_sspil.h */
#define SSP3_BASE_ADDR            0x3502B000 /* brcm_rdb_sspil.h */
#define SRCMIXER_BASE_ADDR        0x3502C000 /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR           0x3502D000 /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR          0x3502E000 /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR           0x3502F000 /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR          0x3502F800 /* brcm_rdb_ahintc.h */

/**** Comm_Subsystem ****/

/**** APB13 ****/
#define GEA_BASE_ADDR             0x3A001000 /* brcm_rdb_gea.h */
#define SCLKCAL_BASE_ADDR         0x3A002000 /* brcm_rdb_sclkcal.h */
#define BMODEM_SYSCFG_BASE_ADDR   0x3A004000 /* brcm_rdb_bmodem_syscfg.h */
#define BMODEM_SWSTM_BASE_ADDR    0x3A005000 /* brcm_rdb_swstm.h */
#define BMODEM_SWSTM_TS_BASE_ADDR 0x3A006000 /* brcm_rdb_swstm.h */
#define BMODEM_GICTR_BASE_ADDR    0x3A007000 /* brcm_rdb_gictr.h */
#define R4DEBUG_BASE_ADDR         0x3A008000 /* brcm_rdb_cr4dbg.h */
#define R4ETM_BASE_ADDR           0x3A009000 /* brcm_rdb_etmr4.h */
#define BMODEM_SW_BASE_ADDR       0x3A00A000 /* brcm_rdb_modem_sw.h */
#define COMMS_SW_BASE_ADDR        0x3A00B000 /* brcm_rdb_comms_sw.h */
#define AXITP18_BASE_ADDR         0x3A00D000 /* brcm_rdb_axi_trace_with_data.h */
#define BMODEM_CTI_BASE_ADDR      0x3A00E000 /* brcm_rdb_cti.h */
#define BMODEM_FUNNEL_BASE_ADDR   0x3A00F000 /* brcm_rdb_cstf.h */

/**** AHB ****/
#define CIPHER_BASE_ADDR          0x3A010000 /* brcm_rdb_cipher.h */
#define DP_BASE_ADDR              0x3A020000 /* brcm_rdb_datapacker.h */
#define HUCM_BASE_ADDR            0x3A030000 /* brcm_rdb_hucm.h */
#define MP_BASE_ADDR              0x3A040000 /* brcm_rdb_mp.h */

/**** APB15 ****/
#define BINTC_BASE_ADDR           0x3A050000 /* brcm_rdb_bintc.h */
#define BMODEM_GPTIMERS_BASE_ADDR 0x3A052000 /* brcm_rdb_gptimers.h */
#define WTI_BASE_ADDR             0x3A053000 /* brcm_rdb_wti.h */
#define BMODEM_SLPTIMER_BASE_ADDR 0x3A054000 /* brcm_rdb_slptimer.h */
#define BMDM_CCU_BASE_ADDR        0x3A055000 /* brcm_rdb_bmdm_clk_mgr_reg.h */
#define BMDM_RST_BASE_ADDR        0x3A055F00 /* brcm_rdb_bmdm_rst_mgr_reg.h */
#define DSP_CCU_BASE_ADDR         0x3A056000 /* brcm_rdb_dsp_clk_mgr_reg.h */
#define DSP_RST_BASE_ADDR         0x3A056F00 /* brcm_rdb_dsp_rst_mgr_reg.h */
#define BMDM_PWRMGR_BASE_ADDR     0x3A057000 /* brcm_rdb_bmdm_pwrmgr.h */
#define SLEEPTIMER3G_BASE_ADDR    0x3A059000 /* brcm_rdb_sleeptimer3g.h */
#define FSCLKCAL_BASE_ADDR        0x3A05A000 /* brcm_rdb_fsclkcal.h */
#define BMODEM_AONCFG_BASE_ADDR   0x3A05B000 /* brcm_rdb_bmodem_aoncfg.h */

/**** WCDMA ****/
#define WCDMACM_BASE_ADDR         0x3A108000 /* brcm_rdb_combiner.h */
#define WCDMAMPD0_15_BASE_ADDR    0x3A108400 /* brcm_rdb_mpdxx_apb.h */
#define WCDMAMPD0_30_BASE_ADDR    0x3A108500 /* brcm_rdb_mpdxx_apb.h */
#define WCDMACM2_BASE_ADDR        0x3A108600 /* brcm_rdb_combiner2.h */
#define WCDMAE2SS_BASE_ADDR       0x3A108B80 /* brcm_rdb_e2ss.h */
#define WCDMASSYNC_BASE_ADDR      0x3A108C00 /* brcm_rdb_ssync_apb.h */
#define WCDMAPSYNC_BASE_ADDR      0x3A109000 /* brcm_rdb_psync_for_10ppm.h */
#define WCDMATX_BASE_ADDR         0x3A109100 /* brcm_rdb_tx.h */
#define WCDMAFNGMUX_BASE_ADDR     0x3A109200 /* brcm_rdb_rake_fng_top.h */
#define WCDMARXBIT_BASE_ADDR      0x3A109280 /* brcm_rdb_rxbitlevel.h */
#define WCDMARXDEINT_BASE_ADDR    0x3A109300 /* brcm_rdb_deinterleaver.h */
#define WCDMAAGC_BASE_ADDR        0x3A109380 /* brcm_rdb_rf_interface_block1_top.h */
#define WCDMATWIF_BASE_ADDR       0x3A109400 /* brcm_rdb_rf_interface_block5_top.h */
#define WCDMASTTD_BASE_ADDR       0x3A109480 /* brcm_rdb_sttd_apb.h */
#define WCDMAAFC_BASE_ADDR        0x3A109500 /* brcm_rdb_afc_top.h */
#define WCDMATNYRAKE_1_BASE_ADDR  0x3A109600 /* brcm_rdb_tiny_fng_core_top.h */
#define WCDMATNYRAKE_2_BASE_ADDR  0x3A109700 /* brcm_rdb_tiny_fng_core_top.h */
#define WCDMATNYRAKE_3_BASE_ADDR  0x3A109800 /* brcm_rdb_tiny_fng_core_top.h */
#define WCDMATNYCTRL_BASE_ADDR    0x3A109900 /* brcm_rdb_rake_tiny_fng_top.h */
#define WCDMASCRAMXY_BASE_ADDR    0x3A109980 /* brcm_rdb_sc_xy_state_derive.h */
#define WCDMABBRX_BASE_ADDR       0x3A109A00 /* brcm_rdb_bbrx.h */
#define WCDMAPRISM_BASE_ADDR      0x3A109B00 /* brcm_rdb_prism.h */
#define WCDMACPP_BASE_ADDR        0x3A109C00 /* brcm_rdb_cpp_cluster.h */
#define WCDMACPCHARQ_BASE_ADDR    0x3A109E00 /* brcm_rdb_cpc_harq.h */
#define WCDMAMC_BASE_ADDR         0x3A109F00 /* brcm_rdb_master_timer.h */
#define WCDMASPINCLK1_BASE_ADDR   0x3A109F80 /* brcm_rdb_modem_clocks_part_1.h */
#define WCDMASPINCLK2_BASE_ADDR   0x3A109FC0 /* brcm_rdb_modem_clocks_part_2.h */
#define WCDMAHTDM_BASE_ADDR       0x3A10A100 /* brcm_rdb_hsdpa_turbo_decoder_module.h */
#define WCDMASCCH_BASE_ADDR       0x3A10A200 /* brcm_rdb_hs_scch.h */
#define WCDMAHARQ_BASE_ADDR       0x3A10A300 /* brcm_rdb_harq.h */
#define WCDMAPSYCHIC_BASE_ADDR    0x3A10A700 /* brcm_rdb_psychic.h */
#define WCDMARFIC_BASE_ADDR       0x3A10A780 /* brcm_rdb_rfic_mst.h */
#define WCDMARFICBUFF_BASE_ADDR   0x3A10A800 /* brcm_rdb_rfic_reg_file.h */
#define WCDMAMPD1_15_BASE_ADDR    0x3A10AA00 /* brcm_rdb_mpdxx_apb.h */
#define WCDMAMPD1_30_BASE_ADDR    0x3A10AB00 /* brcm_rdb_mpdxx_apb.h */
#define WCDMASCHED_BASE_ADDR      0x3A10AC00 /* brcm_rdb_rfic_scheduler.h */
#define WCDMABBRFTX_BASE_ADDR     0x3A10AE00 /* brcm_rdb_bb_tx_filters_and_2091_if.h */
#define WCDMARXADC_BASE_ADDR      0x3A10B000 /* brcm_rdb_rf_interface_block2_top.h */
#define WCDMAGP_BASE_ADDR         0x3A10B080 /* brcm_rdb_rf_interface_block3_top.h */
#define WCDMAPDM_BASE_ADDR        0x3A10B100 /* brcm_rdb_pdm_top.h */
#define WCDMATXDAC_BASE_ADDR      0x3A10B180 /* brcm_rdb_rf_interface_block4_top.h */
#define WCDMASYM1_BASE_ADDR       0x3A10B800 /* brcm_rdb_sym_div.h */
#define WCDMAGPTIMER_BASE_ADDR    0x3A10C000 /* brcm_rdb_wcdma_gp_timers.h */
#define WCDMAL2INT_BASE_ADDR      0x3A10C180 /* brcm_rdb_layer_2_int_cont.h */
#define WCDMAL1IRQ_BASE_ADDR      0x3A10C280 /* brcm_rdb_layer_1_int_cont.h */
#define WCDMAL1FIQ_BASE_ADDR      0x3A10C2C0 /* brcm_rdb_layer_1_int_cont.h */
#define WCDMAASICCLK_BASE_ADDR    0x3A10C300 /* brcm_rdb_modem_cfg_and_core_clks.h */
#define WCDMAGPIO_BASE_ADDR       0x3A10C500 /* brcm_rdb_wcdma_gpio.h */
#define WCDMAUART1_BASE_ADDR      0x3A10C600 /* brcm_rdb_wcdma_uart.h */
#define WCDMARAKE_1_BASE_ADDR     0x3A10D000 /* brcm_rdb_fng_core_top.h */
#define WCDMARAKE_2_BASE_ADDR     0x3A10D200 /* brcm_rdb_fng_core_top.h */
#define WCDMARAKE_3_BASE_ADDR     0x3A10D400 /* brcm_rdb_fng_core_top.h */
#define WCDMARAKE_4_BASE_ADDR     0x3A10D600 /* brcm_rdb_fng_core_top.h */
#define WCDMARAKE_5_BASE_ADDR     0x3A10D800 /* brcm_rdb_fng_core_top.h */
#define WCDMARAKE_6_BASE_ADDR     0x3A10DA00 /* brcm_rdb_fng_core_top.h */
#define WCDMARAKE_7_BASE_ADDR     0x3A10DC00 /* brcm_rdb_edch_fng_core_top.h */
#define WCDMARAKE_8_BASE_ADDR     0x3A10DE00 /* brcm_rdb_edch_fng_core_top.h */
#define WCDMAL2INT_ASYNC_BASE_ADDR 0x3A10F000 /* brcm_rdb_layer_2_async.h */

/**** ARMDSP ****/
#define AHB_DSP_TL3R_BASE_ADDR    0x3B400000 /* brcm_rdb_dsp_tl3r.h */
#define AHB_DSP_INTC_BASE_ADDR    0x3B800000 /* brcm_rdb_dsp_intc.h */
#define AHB_DSP_CIPHER_BASE_ADDR  0x3B800000 /* brcm_rdb_dsp_cipher.h */
#define AHB_DSP_EQUALIZER_BASE_ADDR 0x3B800000 /* brcm_rdb_dsp_equalizer.h */
#define AHB_DSP_EVENT_CTRL_BASE_ADDR 0x3B800000 /* brcm_rdb_dsp_event_ctrl.h */
#define AHB_DSP_RF_GPO_BASE_ADDR  0x3B800000 /* brcm_rdb_dsp_rf_gpo.h */
#define ARM_DSP_RFSPI_BASE_ADDR   0x3B800000 /* brcm_rdb_dsp_rfspi.h */
#define AHB_DSP_3WIRE_INTF_BASE_ADDR 0x3B800000 /* brcm_rdb_dsp_3wire_intf.h */
#define AHB_DSP_SYS_TIMER_BASE_ADDR 0x3B800000 /* brcm_rdb_dsp_sys_timer.h */
#define AHB_DSP_DIGRF_BASE_ADDR   0x3B800000 /* brcm_rdb_dsp_digrf.h */
#define AHB_DSP_AC_BASE_ADDR      0x3B800000 /* brcm_rdb_dsp_ac.h */
#define AHB_DSP_RX_CONTROL_BASE_ADDR 0x3B800000 /* brcm_rdb_dsp_rx_control.h */
#define AHB_DSP_TX_CONTROL_BASE_ADDR 0x3B800000 /* brcm_rdb_dsp_tx_control.h */
#define AHB_DSP_TRACE_BASE_ADDR   0x3B800000 /* brcm_rdb_dsp_trace.h */

/**** MM_Subsystem ****/
#define ISP_BASE_ADDR             0x3C080000 /* brcm_rdb_isp.h */
#define VCE_BASE_ADDR             0x3C100000 /* brcm_rdb_vce.h */
#define SMI_BASE_ADDR             0x3C180000 /* brcm_rdb_smi.h */
#define DSI0_BASE_ADDR            0x3C200000 /* brcm_rdb_dsi1.h */
#define H264_BASE_ADDR            0x3D000000 /* brcm_rdb_h264.h */

/**** APB12 ****/
#define MM_CLK_BASE_ADDR          0x3C000000 /* brcm_rdb_mm_clk_mgr_reg.h */
#define MM_RST_BASE_ADDR          0x3C000F00 /* brcm_rdb_mm_rst_mgr_reg.h */
#define MM_SWITCH_BASE_ADDR       0x3C001000 /* brcm_rdb_mmsw.h */
#define MM_TECTL_BASE_ADDR        0x3C002000 /* brcm_rdb_te.h */
#define MM_SPI_BASE_ADDR          0x3C003000 /* brcm_rdb_spi.h */
#define MM_CFG_BASE_ADDR          0x3C004000 /* brcm_rdb_mm_cfg.h */
#define PIXELVALVE_BASE_ADDR      0x3C005000 /* brcm_rdb_pixelvalve.h */
#define AXIPV_BASE_ADDR           0x3C006000 /* brcm_rdb_axipv.h */
#define MM_CSI0_BASE_ADDR         0x3C008000 /* brcm_rdb_cam.h */
#define MM_DMA_BASE_ADDR          0x3C00A000 /* brcm_rdb_dma.h */
#define MM_V3D_BASE_ADDR          0x3C00B000 /* brcm_rdb_v3d.h */
#define MM_CTI_BASE_ADDR          0x3C00C000 /* brcm_rdb_cti.h */
#define AXITRACE17_BASE_ADDR      0x3C00E000 /* brcm_rdb_axitp2.h */
#define H264ASYNC_BASE_ADDR       0x3C00F000 /* brcm_rdb_asb.h */

/**** MM2_Subsystem ****/
#define ISP2_BASE_ADDR            0x3DA00000 /* brcm_rdb_isp2.h */

/**** APB16 ****/
#define MM2_CLK_BASE_ADDR         0x3D800000 /* brcm_rdb_mm2_clk_mgr_reg.h */
#define MM2_RST_BASE_ADDR         0x3D800F00 /* brcm_rdb_mm2_rst_mgr_reg.h */
#define MM2_SWITCH_BASE_ADDR      0x3D801000 /* brcm_rdb_mm2sw.h */
#define JP_BASE_ADDR              0x3D805000 /* brcm_rdb_jp.h */
#define MM2_CTI_BASE_ADDR         0x3D80C000 /* brcm_rdb_cti.h */
#define MM2_FUNNEL_BASE_ADDR      0x3D80D000 /* brcm_rdb_cstf.h */
#define AXITRACE20_BASE_ADDR      0x3D80E000 /* brcm_rdb_axitp2.h */

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

/**** Fabric ****/

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
#define AXITRACE2_BASE_ADDR       0x3F012000 /* brcm_rdb_axitp2.h */
#define AXITRACE3_BASE_ADDR       0x3F013000 /* brcm_rdb_axitp2.h */
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
#define SDIO4_BASE_ADDR           0x3F1B0000 /* brcm_rdb_emmcsdxc.h */

/**** Processor ****/
#define CCI400_BASE_ADDR          0x35308000 /* brcm_rdb_cci400.h */

/**** APB0 ****/
#define PROC_CLK_BASE_ADDR        0x3FE00000 /* brcm_rdb_kproc_clk_mgr_reg.h */
#define PROC_RST_BASE_ADDR        0x3FE00F00 /* brcm_rdb_kproc_rst_mgr_reg.h */
#define SECTRAP1_BASE_ADDR        0x3FE01000 /* brcm_rdb_sectrap.h */
#define SECTRAP8_BASE_ADDR        0x3FE02000 /* brcm_rdb_sectrap.h */
#define AXITRACE1_BASE_ADDR       0x3FE03000 /* brcm_rdb_axitp_one.h */
#define AXITRACE4_BASE_ADDR       0x3FE04000 /* brcm_rdb_axitp_one.h */
#define GICTR_BASE_ADDR           0x3FE05000 /* brcm_rdb_gictr4.h */
#define FUNNEL_BASE_ADDR          0x3FE06000 /* brcm_rdb_cstf.h */
#define AXITRACE16_BASE_ADDR      0x3FE07000 /* brcm_rdb_axitp_one.h */
#define AXICTI_BASE_ADDR          0x3FE08000 /* brcm_rdb_cti.h */
#define SWSTM_BASE_ADDR           0x3FE09000 /* brcm_rdb_swstm.h */
#define SWSTM_ST_BASE_ADDR        0x3FE0A000 /* brcm_rdb_swstm.h */
#define PWRWDOG_BASE_ADDR         0x3FE0B000 /* brcm_rdb_pwrwdog.h */
#define PROC_TIMER_BASE_ADDR      0x3FE0C000 /* brcm_rdb_kona_proc_timer.h */
#define CDC_BASE_ADDR             0x3FE0E000 /* brcm_rdb_cdc.h */
#define FUNNEL2_BASE_ADDR         0x3FE0F000 /* brcm_rdb_cstf.h */

/**** APB11 ****/
#define A7CPU0_BASE_ADDR          0x3FE10000 /* brcm_rdb_cortexa7_debug.h */
#define A7PMU0_BASE_ADDR          0x3FE11000 /* brcm_rdb_cortexa7_pmu.h */
#define A7CPU1_BASE_ADDR          0x3FE12000 /* brcm_rdb_cortexa7_debug.h */
#define A7PMU1_BASE_ADDR          0x3FE13000 /* brcm_rdb_cortexa7_pmu.h */
#define A7CPU2_BASE_ADDR          0x3FE14000 /* brcm_rdb_cortexa7_debug.h */
#define A7PMU2_BASE_ADDR          0x3FE15000 /* brcm_rdb_cortexa7_pmu.h */
#define A7CPU3_BASE_ADDR          0x3FE16000 /* brcm_rdb_cortexa7_debug.h */
#define A7PMU3_BASE_ADDR          0x3FE17000 /* brcm_rdb_cortexa7_pmu.h */
#define A7CTI0_BASE_ADDR          0x3FE18000 /* brcm_rdb_cortexa7_cti.h */
#define A7CTI1_BASE_ADDR          0x3FE19000 /* brcm_rdb_cortexa7_cti.h */
#define A7CTI2_BASE_ADDR          0x3FE1A000 /* brcm_rdb_cortexa7_cti.h */
#define A7CTI3_BASE_ADDR          0x3FE1B000 /* brcm_rdb_cortexa7_cti.h */
#define A7PTM0_BASE_ADDR          0x3FE1C000 /* brcm_rdb_cortexa7_etm.h */
#define A7PTM1_BASE_ADDR          0x3FE1D000 /* brcm_rdb_cortexa7_etm.h */
#define A7PTM2_BASE_ADDR          0x3FE1E000 /* brcm_rdb_cortexa7_etm.h */
#define A7PTM3_BASE_ADDR          0x3FE1F000 /* brcm_rdb_cortexa7_etm.h */

/**** GIC_400 ****/
#define GIC_AXI_BASE_ADDR         0x3FE20000 /* brcm_rdb_gic.h */
#define GIC_MEM0_BASE_ADDR        0x3FE40000 /* brcm_rdb_gic.h */
#define GIC_MEM1_BASE_ADDR        0x3FE50000 /* brcm_rdb_gic.h */
#define GIC_MEM2_BASE_ADDR        0x3FE60000 /* brcm_rdb_gic.h */
#define GIC_MEM3_BASE_ADDR        0x3FE70000 /* brcm_rdb_gic.h */

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
#define BSC1_BASE_ADDR            0x3E016000 /* brcm_rdb_i2c_mm_hs_slave.h */
#define BSC2_BASE_ADDR            0x3E017000 /* brcm_rdb_i2c_mm_hs_slave.h */
#define BSC3_BASE_ADDR            0x3E018000 /* brcm_rdb_i2c_mm_hs_slave.h */
#define PWM_BASE_ADDR             0x3E01A000 /* brcm_rdb_pwm_top.h */
#define BSC4_BASE_ADDR            0x3E01C000 /* brcm_rdb_i2c_mm_hs_slave.h */


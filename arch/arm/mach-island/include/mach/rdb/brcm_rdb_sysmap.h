/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011 Broadcom Corporation                                                         */
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
/*     Date     : Generated on 04/25/2011 11:06:08                                   */
/*     RDB file : /projects/BIGISLAND/revA0                                                                    */
/************************************************************************************************/

/**** BBL_INDIRECT ****/
#define BBL_RTC_BASE_ADDR         0x00002000 /* brcm_rdb_bbl_rtc.h */
#define BBL_WATCHDOG_BASE_ADDR    0x00004000 /* brcm_rdb_secwatchdog.h */
#define BBL_FMON_BASE_ADDR        0x00006000 /* brcm_rdb_fmon.h */
#define BBL_PWSEQ_BASE_ADDR       0x00008000 /* brcm_rdb_bbl_pwseq.h */
#define BBL_VMON_BASE_ADDR        0x0000A000 /* brcm_rdb_bbl_vmon.h */
#define BBL_CONFIG_BASE_ADDR      0x0000C000 /* brcm_rdb_bbl_config.h */

/**** HSM ****/
#define HSM_IPC_BASE_ADDR         0x00100000 /* brcm_rdb_hsmipc_int.h */
#define HSM_PKA_BASE_ADDR         0x00200000 /* brcm_rdb_pka.h */
#define HSM_OTP_BASE_ADDR         0x00201000 /* brcm_rdb_otp.h */
#define HSM_RNG_BASE_ADDR         0x00202000 /* brcm_rdb_rng.h */
#define HSM_KEK_BASE_ADDR         0x00203000 /* brcm_rdb_kek_apb.h */
#define HSM_WATCHDOG_BASE_ADDR    0x00204000 /* brcm_rdb_secwatchdog.h */
#define HSM_CFG_BASE_ADDR         0x00205000 /* brcm_rdb_hsmcfg.h */
#define HSM_SPUM_S_APB_BASE_ADDR  0x00206000 /* brcm_rdb_spum_apb.h */
#define HSM_SPUM_NS_APB_BASE_ADDR 0x00207000 /* brcm_rdb_spum_apb.h */

/**** Hub ****/

/**** APB5 ****/
#define HUB_CLK_BASE_ADDR         0x34000000 /* brcm_rdb_khub_clk_mgr_reg.h */
#define HUB_RST_BASE_ADDR         0x34000F00 /* brcm_rdb_khub_rst_mgr_reg.h */
#define HUBSWITCH_BASE_ADDR       0x34001000 /* brcm_rdb_hubsw.h */
#define MPU_BASE_ADDR             0x34002000 /* brcm_rdb_mpu.h */
#define NVSRAM_BASE_ADDR          0x34003000 /* brcm_rdb_nvsram_axi.h */
#define IPCSEC_BASE_ADDR          0x34004000 /* brcm_rdb_ipcsec.h */
#define IPC_BASE_ADDR             0x34005000 /* brcm_rdb_ipcopen.h */
#define TMON_BASE_ADDR            0x34008000 /* brcm_rdb_tmon.h */
#define SECTRAP7_BASE_ADDR        0x34009000 /* brcm_rdb_sectrap.h */
#define SECTRAP4_BASE_ADDR        0x3400A000 /* brcm_rdb_sectrap.h */
#define SECTRAP5_BASE_ADDR        0x3400B000 /* brcm_rdb_sectrap.h */
#define SECTRAP6_BASE_ADDR        0x3400C000 /* brcm_rdb_sectrap.h */
#define HSI_BASE_ADDR             0x3400E000 /* brcm_rdb_hsi_apb.h */
#define VARSPM_BASE_ADDR          0x3400F000 /* brcm_rdb_spm.h */

/**** APB9 ****/
#define AXITRACE19_BASE_ADDR      0x34010000 /* brcm_rdb_axitp1.h */
#define AXITRACE11_BASE_ADDR      0x34011000 /* brcm_rdb_axitp1.h */
#define AXITRACE12_BASE_ADDR      0x34012000 /* brcm_rdb_axitp1.h */
#define ATBFILTER_BASE_ADDR       0x34016000 /* brcm_rdb_atbfilter.h */
#define HUB_CTI_BASE_ADDR         0x34018000 /* brcm_rdb_cti.h */
#define HUB_FUNNEL_BASE_ADDR      0x34019000 /* brcm_rdb_cstf.h */
#define FIN_FUNNEL_BASE_ADDR      0x3401A000 /* brcm_rdb_cstf.h */
#define STM_BASE_ADDR             0x3401B000 /* brcm_rdb_atb_stm.h */
#define ETB_BASE_ADDR             0x3401D000 /* brcm_rdb_etb.h */
#define ETB2AXI_BASE_ADDR         0x3401E000 /* brcm_rdb_etb2axi.h */
#define TPIU_BASE_ADDR            0x3401F000 /* brcm_rdb_cstpiu.h */

/**** Internal SRAM ****/
#define INT_SRAM_BASE             0x34040000 /* Internal SRAM (160 KB) */

/**** AONHub ****/

/**** APB6 ****/
#define TZCFG_BASE_ADDR           0x35000000 /* brcm_rdb_konatzcfg.h */
#define ROOT_CLK_BASE_ADDR        0x35001000 /* brcm_rdb_iroot_clk_mgr_reg.h */
#define ROOT_RST_BASE_ADDR        0x35001F00 /* brcm_rdb_iroot_rst_mgr_reg.h */
#define AON_CLK_BASE_ADDR         0x35002000 /* brcm_rdb_khubaon_clk_mgr_reg.h */
#define AON_RST_BASE_ADDR         0x35002F00 /* brcm_rdb_khubaon_rst_mgr_reg.h */
#define GPIO2_BASE_ADDR           0x35003000 /* brcm_rdb_gpio.h */
#define KEYPAD_BASE_ADDR          0x35003800 /* brcm_rdb_keypad.h */
#define CHIPREGS_BASE_ADDR        0x35004000 /* brcm_rdb_chipreg.h */
#define SIM_BASE_ADDR             0x35005000 /* brcm_rdb_simi.h */
#define SIM2_BASE_ADDR            0x35005800 /* brcm_rdb_simi.h */
#define HUB_TIMER_BASE_ADDR       0x35006000 /* brcm_rdb_kona_gptimer.h */
#define MEMC0_SECURE_BASE_ADDR    0x35007000 /* brcm_rdb_sec.h */
#define MEMC0_OPEN_BASE_ADDR      0x35008000 /* brcm_rdb_csr.h */
#define MEMC0_OPEN_PWRWDOG_BASE_ADDR 0x350083C0 /* brcm_rdb_pwrwdog.h */
#define MEMC0_OPEN_APHY_BASE_ADDR 0x35008400 /* brcm_rdb_aphy_csr.h */
#define MEMC0_DDR3_CTL_BASE_ADDR  0x35008400 /* brcm_rdb_ddr3ctl.h */
#define MEMC0_OPEN_DPHY_BASE_ADDR 0x35008800 /* brcm_rdb_dphy_csr.h */
#define MEMC0_DDR3_PHY_ADDR_CTL_BASE_ADDR 0x35008800 /* brcm_rdb_ddr40_phy_addr_ctl.h */
#define MEMC0_DDR3_PHY_WL_0_BASE_ADDR 0x35008A00 /* brcm_rdb_ddr40_phy_word_lane.h */
#define MEMC0_DDR3_PHY_WL_1_BASE_ADDR 0x35008C00 /* brcm_rdb_ddr40_phy_word_lane.h */
#define MEMC1_SECURE_BASE_ADDR    0x35009000 /* brcm_rdb_sec.h */
#define MEMC1_OPEN_BASE_ADDR      0x3500A000 /* brcm_rdb_csr.h */
#define MEMC1_OPEN_PWRWDOG_BASE_ADDR 0x3500A3C0 /* brcm_rdb_pwrwdog.h */
#define MEMC1_OPEN_APHY_BASE_ADDR 0x3500A400 /* brcm_rdb_aphy_csr.h */
#define MEMC1_OPEN_DPHY_BASE_ADDR 0x3500A800 /* brcm_rdb_dphy_csr.h */
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
#define SLIMWRAP_BASE_ADDR        0x35029000 /* brcm_rdb_slimwrap.h */
#define SLIMBUS_BASE_ADDR         0x35029400 /* brcm_rdb_slimbus.h */
#define SSP3_BASE_ADDR            0x3502B000 /* brcm_rdb_sspil.h */
#define SRCMIXER_BASE_ADDR        0x3502C000 /* brcm_rdb_srcmixer.h */
#define CFIFO_BASE_ADDR           0x3502D000 /* brcm_rdb_cph_cfifo.h */
#define AADMAC_BASE_ADDR          0x3502E000 /* brcm_rdb_cph_aadmac.h */
#define SSASW_BASE_ADDR           0x3502F000 /* brcm_rdb_cph_ssasw.h */
#define AHINTC_BASE_ADDR          0x3502F800 /* brcm_rdb_ahintc.h */

/**** ESUB ****/
#define DWDMA_AHB_BASE_ADDR       0x38100000 /* brcm_rdb_dwdma_ahb.h */

/**** APB14 ****/
#define ESUB_CLK_BASE_ADDR        0x38000000 /* brcm_rdb_esub_clk_mgr_reg.h */
#define ESUB_RST_BASE_ADDR        0x38000F00 /* brcm_rdb_esub_rst_mgr_reg.h */
#define VINTC_APB_BASE_ADDR       0x38002000 /* brcm_rdb_vintc_apb.h */
#define PWRWDOG1_BASE_ADDR        0x38004000 /* brcm_rdb_pwrwdog.h */
#define AXITRACE21_BASE_ADDR      0x3800D000 /* brcm_rdb_axitp1.h */
#define ESW_CTI_BASE_ADDR         0x3800E000 /* brcm_rdb_cti.h */

/**** ESW ****/
#define ESW_CONTRL_BASE_ADDR      0x38200000 /* brcm_rdb_esw_contrl.h */
#define ESW_STATUS_BASE_ADDR      0x38200800 /* brcm_rdb_esw_status.h */
#define ESW_MGMN_BASE_ADDR        0x38201000 /* brcm_rdb_esw_mgmn.h */
#define ESW_ARL_ACCESS_BASE_ADDR  0x38202000 /* brcm_rdb_esw_arl_cntrl.h */
#define ESW_ARL_VTBL_BASE_ADDR    0x38202800 /* brcm_rdb_esw_arl_vtbl_acc.h */
#define ESW_MEM_ACC_BASE_ADDR     0x38204000 /* brcm_rdb_esw_mem_acc.h */
#define ESW_DIAG_BASE_ADDR        0x38204800 /* brcm_rdb_esw_diag.h */
#define ESW_FLOW_0_BASE_ADDR      0x38205000 /* brcm_rdb_esw_flow.h */
#define ESW_FLOW_IMP_BASE_ADDR    0x38206800 /* brcm_rdb_esw_flow_imp.h */
#define ESW_FLOW_1_BASE_ADDR      0x38207000 /* brcm_rdb_esw_flow.h */
#define ESW_PORT0_MIB_BASE_ADDR   0x38210000 /* brcm_rdb_esw_mib.h */
#define ESW_PORT1_MIB_BASE_ADDR   0x38210800 /* brcm_rdb_esw_mib.h */
#define ESW_IMP_PORT_MIB_BASE_ADDR 0x38214000 /* brcm_rdb_esw_mib.h */
#define ESW_QOS_REGISTERS_BASE_ADDR 0x38218000 /* brcm_rdb_qos_registers.h */
#define ESW_VLAN_REGISTERS_BASE_ADDR 0x38218800 /* brcm_rdb_esw_vlan_register.h */
#define ESW_TRUNKING_REGISTERS_BASE_ADDR 0x38219000 /* brcm_rdb_trunking_registers.h */
#define ESW_802_1Q_VLAN_REGISTERS_BASE_ADDR 0x3821A000 /* brcm_rdb_ieee_802_1q_vlan_registers.h */
#define ESW_DOS_ATTACK_PREVENT_REGISTER_BASE_ADDR 0x3821B000 /* brcm_rdb_dos_attack_prevent_register.h */
#define ESW_JUMBO_FRAME_CONTROL_REGISTER_BASE_ADDR 0x38220000 /* brcm_rdb_jumbo_frame_control_register.h */
#define ESW_BROADCAST_STORM_SUPPRESSION_REGISTER_BASE_ADDR 0x38220800 /* brcm_rdb_broadcast_storm_suppression_register.h */
#define ESW_EAP_REGISTER_BASE_ADDR 0x38221000 /* brcm_rdb_eap_register.h */
#define ESW_MSPT_REGISTER_BASE_ADDR 0x38221800 /* brcm_rdb_mspt_register.h */
#define MIB_SNAPSHOT_CONTROL_REGISTER_BASE_ADDR 0x38238000 /* brcm_rdb_mib_snapshot_register.h */
#define MIB_SNAPSHOT_REGISTER_BASE_ADDR 0x38239000 /* brcm_rdb_esw_mib.h */
#define LOOP_DETECTION_REGISTER_BASE_ADDR 0x3823A000 /* brcm_rdb_loop_detection_register.h */
#define EAV_REGISTER_BASE_ADDR    0x38248000 /* brcm_rdb_eav_register.h */
#define TRAFFIC_REMARKING_REGISTER_BASE_ADDR 0x38249000 /* brcm_rdb_traffic_remarking_register.h */
#define CFP_TCAM_REGISTER_BASE_ADDR 0x38250000 /* brcm_rdb_cfp_tcam_register.h */
#define CFP_CONFIGURATION_REGISTER_BASE_ADDR 0x38250800 /* brcm_rdb_cfp_configuration_register.h */
#define ESW_BUSIF_PTM_BASE_ADDR   0x38280000 /* brcm_rdb_esw_busif_ptm.h */
#define ESW_BUSIF_MTP_BASE_ADDR   0x38290000 /* brcm_rdb_esw_busif_mtp.h */
#define ESW_BUSIF_IMP_BASE_ADDR   0x382A0000 /* brcm_rdb_esw_busif_imp.h */

/**** Comm_Subsystem ****/

/**** APB13 ****/
#define GEA_BASE_ADDR             0x3A001000 /* brcm_rdb_gea.h */
#define SCLKCAL_BASE_ADDR         0x3A002000 /* brcm_rdb_sclkcal.h */
#define BMODEM_SYSCFG_BASE_ADDR   0x3A004000 /* brcm_rdb_bmodem_syscfg.h */
#define BMODEM_SWSTM_BASE_ADDR    0x3A005000 /* brcm_rdb_swstm.h */
#define BMODEM_SWSTM_TS_BASE_ADDR 0x3A006000 /* brcm_rdb_swstm.h */
#define BMODEM_GICTR_BASE_ADDR    0x3A007000 /* brcm_rdb_gictr.h */
#define BMODEM_SW_BASE_ADDR       0x3A00A000 /* brcm_rdb_modem_sw.h */
#define COMMS_SW_BASE_ADDR        0x3A00B000 /* brcm_rdb_comms_sw.h */
#define AXITP18_BASE_ADDR         0x3A00D000 /* brcm_rdb_axitp1.h */
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
#define BMODEM_AONCFG_BASE_ADDR   0x3A05B000 /* brcm_rdb_bmodem_aoncfg.h */

/**** WCDMA ****/
#define WCDMACM_BASE_ADDR         0x3A108000 /* brcm_rdb_combiner.h */
#define WCDMAMPD0_15_BASE_ADDR    0x3A108400 /* brcm_rdb_mpdxx_apb.h */
#define WCDMAMPD0_30_BASE_ADDR    0x3A108500 /* brcm_rdb_mpdxx_apb.h */
#define WCDMACM2_BASE_ADDR        0x3A108600 /* brcm_rdb_combiner2.h */
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
#define WCDMACPP_BASE_ADDR        0x3A109C00 /* brcm_rdb_cpp_cluster.h */
#define WCDMACPCHARQ_BASE_ADDR    0x3A109E00 /* brcm_rdb_cpc_harq.h */
#define WCDMAMC_BASE_ADDR         0x3A109F00 /* brcm_rdb_master_timer.h */
#define WCDMASPINCLK1_BASE_ADDR   0x3A109F80 /* brcm_rdb_modem_clocks_part_1.h */
#define WCDMASPINCLK2_BASE_ADDR   0x3A109FC0 /* brcm_rdb_modem_clocks_part_2.h */
#define WCDMAHTDM_BASE_ADDR       0x3A10A100 /* brcm_rdb_hsdpa_turbo_decoder_module.h */
#define WCDMASCCH_BASE_ADDR       0x3A10A200 /* brcm_rdb_hs_scch.h */
#define WCDMAHARQ_BASE_ADDR       0x3A10A300 /* brcm_rdb_harq.h */
#define WCDMATNYRAKE_4_BASE_ADDR  0x3A10A400 /* brcm_rdb_tiny_fng_core_top.h */
#define WCDMATNYRAKE_5_BASE_ADDR  0x3A10A500 /* brcm_rdb_tiny_fng_core_top.h */
#define WCDMATNYRAKE_6_BASE_ADDR  0x3A10A600 /* brcm_rdb_tiny_fng_core_top.h */
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

/**** Slaves ****/

/**** APB1 ****/
#define UARTB_BASE_ADDR           0x3E000000 /* brcm_rdb_uartb.h */
#define UARTB2_BASE_ADDR          0x3E001000 /* brcm_rdb_uartb.h */
#define UARTB3_BASE_ADDR          0x3E002000 /* brcm_rdb_uartb.h */
#define UARTB4_BASE_ADDR          0x3E003000 /* brcm_rdb_uartb.h */
#define SSP0_BASE_ADDR            0x3E004000 /* brcm_rdb_sspil.h */
#define SSP2_BASE_ADDR            0x3E006000 /* brcm_rdb_sspi.h */
#define IRDA_BASE_ADDR            0x3E009000 /* brcm_rdb_irda.h */
#define TIMER_BASE_ADDR           0x3E00D000 /* brcm_rdb_kona_gptimer.h */
#define DMUX_BASE_ADDR            0x3E00E000 /* brcm_rdb_dmux.h */

/**** APB2 ****/
#define KONA_SLV_CLK_BASE_ADDR    0x3E011000 /* brcm_rdb_ikps_clk_mgr_reg.h */
#define KONA_SLV_RST_BASE_ADDR    0x3E011F00 /* brcm_rdb_ikps_rst_mgr_reg.h */
#define D1W_BASE_ADDR             0x3E015000 /* brcm_rdb_d1w.h */
#define BSC1_BASE_ADDR            0x3E016000 /* brcm_rdb_i2c_mm_hs.h */
#define BSC2_BASE_ADDR            0x3E017000 /* brcm_rdb_i2c_mm_hs.h */
#define PWM_BASE_ADDR             0x3E01A000 /* brcm_rdb_pwm_top.h */
#define BBL_BASE_ADDR             0x3E01B000 /* brcm_rdb_bbl_apb.h */

/**** AHB1 ****/
#define MPHI_BASE_ADDR            0x3E130000 /* brcm_rdb_mphi.h */

/**** AXI ****/
#define SPUM_NS_BASE_ADDR         0x3E300000 /* brcm_rdb_spum_axi.h */
#define SPUM_S_BASE_ADDR          0x3E310000 /* brcm_rdb_spum_axi.h */

/**** SECURITY ****/
#define SEC_PKA_BASE_ADDR         0x3E600000 /* brcm_rdb_pka.h */
#define SEC_OTP_BASE_ADDR         0x3E601000 /* brcm_rdb_otp.h */
#define SEC_RNG_BASE_ADDR         0x3E602000 /* brcm_rdb_rng.h */
#define SEC_KEK_BASE_ADDR         0x3E603000 /* brcm_rdb_kek_apb.h */
#define SEC_WATCHDOG_BASE_ADDR    0x3E604000 /* brcm_rdb_secwatchdog.h */
#define SEC_CFG_BASE_ADDR         0x3E605000 /* brcm_rdb_hsmcfg.h */
#define SEC_SPUM_S_APB_BASE_ADDR  0x3E606000 /* brcm_rdb_spum_apb.h */
#define SEC_SPUM_NS_APB_BASE_ADDR 0x3E607000 /* brcm_rdb_spum_apb.h */
#define SEC_IPC_BASE_ADDR         0x3E680000 /* brcm_rdb_hsmipc_ext.h */

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
#define AXITRACE2_BASE_ADDR       0x3F012000 /* brcm_rdb_axitp1.h */
#define AXITRACE3_BASE_ADDR       0x3F013000 /* brcm_rdb_axitp1.h */
#define FAB_FUNNEL0_BASE_ADDR     0x3F01D000 /* brcm_rdb_cstf.h */
#define FAB_FUNNEL1_BASE_ADDR     0x3F01E000 /* brcm_rdb_cstf.h */
#define FAB_CTI_BASE_ADDR         0x3F01F000 /* brcm_rdb_cti.h */

/**** AHB2 ****/
#define NAND_BASE_ADDR            0x3F110000 /* brcm_rdb_nand.h */
#define HSOTG_BASE_ADDR           0x3F120000 /* brcm_rdb_hsotg.h */
#define HSOTG_CTRL_BASE_ADDR      0x3F130000 /* brcm_rdb_hsotg_ctrl.h */
#define EHCI_BASE_ADDR            0x3F140000 /* brcm_rdb_ehci.h */
#define OHCI_BASE_ADDR            0x3F150000 /* brcm_rdb_ohci.h */
#define FSHOST_BASE_ADDR          0x3F160000 /* brcm_rdb_fshost.h */
#define FSHOST_CTRL_BASE_ADDR     0x3F170000 /* brcm_rdb_fshost_ctrl.h */
#define SDIO1_BASE_ADDR           0x3F180000 /* brcm_rdb_emmcsdxc.h */
#define SDIO2_BASE_ADDR           0x3F190000 /* brcm_rdb_emmcsdxc.h */
#define SDIO3_BASE_ADDR           0x3F1A0000 /* brcm_rdb_emmcsdxc.h */
#define SDIO4_BASE_ADDR           0x3F1B0000 /* brcm_rdb_emmcsdxc.h */

/**** Processor ****/
#define L2C_BASE_ADDR             0x3FF20000 /* brcm_rdb_pl310.h */

/**** APB0 ****/
#define PROC_CLK_BASE_ADDR        0x3FE00000 /* brcm_rdb_kproc_clk_mgr_reg.h */
#define PROC_RST_BASE_ADDR        0x3FE00F00 /* brcm_rdb_kproc_rst_mgr_reg.h */
#define SECTRAP1_BASE_ADDR        0x3FE01000 /* brcm_rdb_sectrap.h */
#define SECTRAP8_BASE_ADDR        0x3FE02000 /* brcm_rdb_sectrap.h */
#define AXITRACE1_BASE_ADDR       0x3FE03000 /* brcm_rdb_axitp1.h */
#define AXITRACE4_BASE_ADDR       0x3FE04000 /* brcm_rdb_axitp1.h */
#define GICTR_BASE_ADDR           0x3FE05000 /* brcm_rdb_gictr.h */
#define ARM_FUNNEL_BASE_ADDR      0x3FE06000 /* brcm_rdb_cstf.h */
#define AXITRACE16_BASE_ADDR      0x3FE07000 /* brcm_rdb_axitp1.h */
#define ACPSW_BASE_ADDR           0x3FE08000 /* brcm_rdb_acp_sw.h */
#define SWSTM_BASE_ADDR           0x3FE09000 /* brcm_rdb_swstm.h */
#define SWSTM_ST_BASE_ADDR        0x3FE0A000 /* brcm_rdb_swstm.h */
#define PWRWDOG0_BASE_ADDR        0x3FE0B000 /* brcm_rdb_pwrwdog.h */

/**** APB11 ****/
#define A9CPU0_BASE_ADDR          0x3FE10000 /* brcm_rdb_a9cpu.h */
#define A9PMU0_BASE_ADDR          0x3FE11000 /* brcm_rdb_a9pmu.h */
#define A9CPU1_BASE_ADDR          0x3FE12000 /* brcm_rdb_a9cpu.h */
#define A9PMU1_BASE_ADDR          0x3FE13000 /* brcm_rdb_a9pmu.h */
#define A9PTM0_BASE_ADDR          0x3FE18000 /* brcm_rdb_a9ptm.h */
#define A9PTM1_BASE_ADDR          0x3FE19000 /* brcm_rdb_a9ptm.h */
#define A9CTI0_BASE_ADDR          0x3FE1C000 /* brcm_rdb_cti.h */
#define A9CTI1_BASE_ADDR          0x3FE1D000 /* brcm_rdb_cti.h */

/**** A9_PRIVATE ****/
#define SCU_BASE_ADDR             0x3FF00000 /* brcm_rdb_scu.h */
#define GICCPU_BASE_ADDR          0x3FF00100 /* brcm_rdb_giccpu.h */
#define GTIM_BASE_ADDR            0x3FF00200 /* brcm_rdb_glbtmr.h */
#define PTIM_BASE_ADDR            0x3FF00600 /* brcm_rdb_ptmr_wd.h */
#define GICDIST_BASE_ADDR         0x3FF01000 /* brcm_rdb_gicdist.h */

/**** VC EMI ****/
#if defined(CONFIG_MAP_LITTLE_ISLAND_MODE)
#define VC_EMI                    0x80000000 /* VC4 EMI */
#else
#define VC_EMI                    0x40000000 /* VC4 EMI */
#endif

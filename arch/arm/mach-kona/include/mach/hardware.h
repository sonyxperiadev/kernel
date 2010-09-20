/*
 * linux/arch/arm/mach-bcm2153/include/mach/hardware.h
 *
 * Copyright (C) 2009 Broadcom Corporation.  All rights reserved.
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed to
 * you under the terms of the GNU General Public License version 2,
 * available at http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
 *
 * Notwithstanding the above, under no circumstances may you combine this
 * software in any way with any other Broadcom software provided under a
 * license other than the GPL, without Broadcom's express prior written
 * consent.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <asm/sizes.h>
#include <mach/io.h>

#define __REG32(x)		(*((volatile u32 *)IO_ADDRESS(x)))
#define __REG16(x)		(*((volatile u16 *)IO_ADDRESS(x)))
#define __REG8(x)		(*((volatile u8 *)IO_ADDRESS(x)))


/**** BBL_INDIRECT ****/
#define BBL_RTC_BASE_ADDR         0x00002000 /* brcm_rdb_bbl_rtc.h */
#define BBL_WATCHDOG_BASE_ADDR    0x00004000 /* brcm_rdb_secwatchdog.h */
#define BBL_VFM_BASE_ADDR         0x00006000 /* brcm_rdb_bbl_vfm.h */
#define BBL_PWSEQ_BASE_ADDR       0x00008000 /* brcm_rdb_bbl_pwseq.h */
#define BBL_RAM_BASE_ADDR         0x0000a000
#define BBL_CFG_0_BASE_ADDR       0x0000C000 /* brcm_rdb_bbl_cfg.h */

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
#define SECTRAP4_BASE_ADDR        0x3400A000 /* brcm_rdb_sectrap.h */
#define SECTRAP5_BASE_ADDR        0x3400B000 /* brcm_rdb_sectrap.h */
#define SECTRAP6_BASE_ADDR        0x3400C000 /* brcm_rdb_sectrap.h */
#define SECTRAP7_BASE_ADDR        0x34009000 /* brcm_rdb_sectrap.h */

#define HSI_BASE_ADDR             0x3400E000 /* brcm_rdb_hsi_apb.h */
#define VARSPM_BASE_ADDR          0x3400F000 /* brcm_rdb_spm.h */

/**** APB9 ****/
#define AXITRACE10_BASE_ADDR      0x34010000 /* brcm_rdb_axitp1.h */
#define AXITRACE11_BASE_ADDR      0x34011000 /* brcm_rdb_axitp1.h */
#define VCITM_BASE_ADDR           0x34014000 /* brcm_rdb_vcitm.h */
#define HUB_CTI_BASE_ADDR         0x34018000 /* brcm_rdb_cti.h */
#define HUB_FUNNEL_BASE_ADDR      0x34019000 /* brcm_rdb_cstf.h */
#define FIN_FUNNEL_BASE_ADDR      0x3401A000 /* brcm_rdb_cstf.h */
#define STM_BASE_ADDR             0x3401B000 /* brcm_rdb_atb_stm.h */
#define ETB_BASE_ADDR             0x3401D000 /* brcm_rdb_etb.h */
#define ETB2AXI_BASE_ADDR         0x3401E000 /* brcm_rdb_etb2axi.h */
#define TPIU_BASE_ADDR            0x3401F000 /* brcm_rdb_cstpiu.h */

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
#define MEMC0_OPEN_APHY_BASE_ADDR 0x35008400 /* brcm_rdb_aphy_csr.h */
#define MEMC0_DDR3_CTL_BASE_ADDR  0x35008400 /* brcm_rdb_ddr3ctl.h */
#define MEMC0_OPEN_DPHY_BASE_ADDR 0x35008800 /* brcm_rdb_dphy_csr.h */
#define MEMC0_DDR3_PHY_ADDR_CTL_BASE_ADDR 0x35008800 /* brcm_rdb_ddr40_phy_addr_ctl.h */
#define MEMC0_DDR3_PHY_WL_0_BASE_ADDR 0x35008A00 /* brcm_rdb_ddr40_phy_word_lane.h */
#define MEMC0_DDR3_PHY_WL_1_BASE_ADDR 0x35008C00 /* brcm_rdb_ddr40_phy_word_lane.h */
#define MEMC1_SECURE_BASE_ADDR    0x35009000 /* brcm_rdb_sec.h */
#define MEMC1_OPEN_BASE_ADDR      0x3500A000 /* brcm_rdb_csr.h */
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
#define AXITRACE20_BASE_ADDR      0x3800C000 /* brcm_rdb_axitp1.h */
#define AXITRACE21_BASE_ADDR      0x3800D000 /* brcm_rdb_axitp1.h */
#define ESW_CTI_BASE_ADDR         0x3800E000 /* brcm_rdb_cti.h */
#define ESW_FUNNEL1_BASE_ADDR     0x3800F000 /* brcm_rdb_cstf.h */

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


#endif

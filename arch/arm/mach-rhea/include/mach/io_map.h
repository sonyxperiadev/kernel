/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/************************************************************************************************/

#ifndef __RHEA_IO_MAP_H
#define __RHEA_IO_MAP_H

#include <asm/memory.h>
#include <mach/memory.h>
#include <mach/rdb/brcm_rdb_sysmap.h>

#define KONA_PROC_CLK_VA             HW_IO_PHYS_TO_VIRT(PROC_CLK_BASE_ADDR)
#define KONA_PROC_RST_VA             HW_IO_PHYS_TO_VIRT(PROC_RST_BASE_ADDR)
#define KONA_AON_CLK_VA             HW_IO_PHYS_TO_VIRT(AON_CLK_BASE_ADDR)
#define KONA_AON_RST_VA             HW_IO_PHYS_TO_VIRT(AON_RST_BASE_ADDR)
#define KONA_ATBFILTER_VA           HW_IO_PHYS_TO_VIRT(ATBFILTER_BASE_ADDR)
/* BSC I2C 1 interface  */
#define KONA_BSC1_VA                HW_IO_PHYS_TO_VIRT(BSC1_BASE_ADDR)
/* BSC I2C 2 interface  */
#define KONA_BSC2_VA                HW_IO_PHYS_TO_VIRT(BSC2_BASE_ADDR)
/* CHIPREG Block */
#define KONA_CHIPREG_VA             HW_IO_PHYS_TO_VIRT(CHIPREGS_BASE_ADDR)
/* PAD CTRL Block */
#define KONA_PAD_CTRL_VA            HW_IO_PHYS_TO_VIRT(PAD_CTRL_BASE_ADDR)
/* Non-Secure DMA interface */
#define KONA_DMAC_NS_VA             HW_IO_PHYS_TO_VIRT(NONDMAC_BASE_ADDR)
/* Secure DMA interface */
#define KONA_DMAC_S_VA              HW_IO_PHYS_TO_VIRT(SECDMAC_BASE_ADDR)
/* DMA DMUX */
#define KONA_DMUX_VA                HW_IO_PHYS_TO_VIRT(DMUX_BASE_ADDR)
#define KONA_FIN_FUNNEL_VA          HW_IO_PHYS_TO_VIRT(FIN_FUNNEL_BASE_ADDR)
#define KONA_FUNNEL_VA              HW_IO_PHYS_TO_VIRT(FUNNEL_BASE_ADDR)
/* GIC CPU INTERFACE */
#define KONA_GICCPU_VA              HW_IO_PHYS_TO_VIRT(GICCPU_BASE_ADDR)
/* GIC DISTRIBUTOR INTERFACE */
#define KONA_GICDIST_VA             HW_IO_PHYS_TO_VIRT(GICDIST_BASE_ADDR)
/* GPIO 2 */
#define KONA_GPIO2_VA               HW_IO_PHYS_TO_VIRT(GPIO2_BASE_ADDR)
/* HSI */
#define KONA_HSI_VA                 HW_IO_PHYS_TO_VIRT(HSI_BASE_ADDR)
/* HUB switch */
#define KONA_HUB_SWITCH_VA          HW_IO_PHYS_TO_VIRT(HUBSWITCH_BASE_ADDR)
#define KONA_KEYPAD_VA              HW_IO_PHYS_TO_VIRT(KEYPAD_BASE_ADDR)
/* Kona Peripheral Master Clock Manager */
#define KONA_KPM_CLK_VA             HW_IO_PHYS_TO_VIRT(KONA_MST_CLK_BASE_ADDR)
/* Kona Peripheral Master Reset Manager */
#define KONA_KPM_RST_VA             HW_IO_PHYS_TO_VIRT(KONA_MST_RST_BASE_ADDR)
/* Kona Peripheral Master Clock Manager */
#define KONA_KPS_CLK_VA             HW_IO_PHYS_TO_VIRT(KONA_SLV_CLK_BASE_ADDR)
/* Kona Peripheral Master Reset Manager */
#define KONA_KPS_RST_VA             HW_IO_PHYS_TO_VIRT(KONA_SLV_RST_BASE_ADDR)
/* L2 Cache Controller */
#define KONA_L2C_VA                 HW_IO_PHYS_TO_VIRT(L2C_BASE_ADDR)
/* System Memory Controller APHY */
#define KONA_MEMC0_APHY_VA	HW_IO_PHYS_TO_VIRT(MEMC0_OPEN_APHY_BASE_ADDR)
/* System Memory Controller DPHY */
#define KONA_MEMC0_DPHY_VA      HW_IO_PHYS_TO_VIRT(MEMC0_OPEN_DPHY_BASE_ADDR)
/* Non-Secure System Memory Controller */
#define KONA_MEMC0_NS_VA            HW_IO_PHYS_TO_VIRT(MEMC0_OPEN_BASE_ADDR)
/* Secure System Memory Controller */
#define KONA_MEMC0_S_VA             HW_IO_PHYS_TO_VIRT(MEMC0_SECURE_BASE_ADDR)
/* Videocore Memory Controller APHY */
#define KONA_MEMC1_APHY_VA      HW_IO_PHYS_TO_VIRT(MEMC1_OPEN_APHY_BASE_ADDR)
/* Videocore Memory Controller DPHY */
#define KONA_MEMC1_DPHY_VA      HW_IO_PHYS_TO_VIRT(MEMC1_OPEN_DPHY_BASE_ADDR)
/* Non-Secure Videocore Memory Controller */
#define KONA_MEMC1_NS_VA            HW_IO_PHYS_TO_VIRT(MEMC1_OPEN_BASE_ADDR)
/* Secure Videocore Memory Controller */
#define KONA_MEMC1_S_VA             HW_IO_PHYS_TO_VIRT(MEMC1_SECURE_BASE_ADDR)
#define KONA_MPHI_VA                HW_IO_PHYS_TO_VIRT(MPHI_BASE_ADDR)
/* Memory protection unit */
#define KONA_MPU_VA                 HW_IO_PHYS_TO_VIRT(MPU_BASE_ADDR)
/* NVSRAM controller */
#define KONA_NVSRAM_VA              HW_IO_PHYS_TO_VIRT(NVSRAM_BASE_ADDR)
/* OTP */
#define KONA_OTP_VA                 HW_IO_PHYS_TO_VIRT(SEC_OTP_BASE_ADDR)
#define KONA_PKA_VA                 HW_IO_PHYS_TO_VIRT(SEC_PKA_BASE_ADDR)
/* PROFILE TIMER */
#define KONA_PROFTMR_VA             HW_IO_PHYS_TO_VIRT(GTIM_BASE_ADDR)
/* Private timer and watchdog */
#define KONA_PTIM_VA                HW_IO_PHYS_TO_VIRT(PTIM_BASE_ADDR)
/* PMU BSC Controller */
#define KONA_PMU_BSC_VA             HW_IO_PHYS_TO_VIRT(PMU_BSC_BASE_ADDR)

#define KONA_ROOT_CLK_VA            HW_IO_PHYS_TO_VIRT(ROOT_CLK_BASE_ADDR)
#define KONA_ROOT_RST_VA            HW_IO_PHYS_TO_VIRT(ROOT_RST_BASE_ADDR)
/* PWM */
#define KONA_PWM_VA                 HW_IO_PHYS_TO_VIRT(PWM_BASE_ADDR)
/* RNG */
#define KONA_RNG_VA                 HW_IO_PHYS_TO_VIRT(SEC_RNG_BASE_ADDR)
/* SCU */
#define KONA_SCU_VA                 HW_IO_PHYS_TO_VIRT(SCU_BASE_ADDR)
/* SDIO 1 */
#define KONA_SDIO1_VA               HW_IO_PHYS_TO_VIRT(SDIO1_BASE_ADDR)
/* SDIO 2 */
#define KONA_SDIO2_VA               HW_IO_PHYS_TO_VIRT(SDIO2_BASE_ADDR)
/* SDIO 3 */
#define KONA_SDIO3_VA               HW_IO_PHYS_TO_VIRT(SDIO3_BASE_ADDR)

#ifdef CONFIG_ARCH_RHEA_BX
/* SDIO 4 */
#define KONA_SDIO4_VA               HW_IO_PHYS_TO_VIRT(SDIO4_BASE_ADDR)
#endif

#define KONA_SEC_VA                 HW_IO_PHYS_TO_VIRT(SEC_CFG_BASE_ADDR)
/* Watchdog Timer in security block (not to be confused with KONA_SECWD) */
#define KONA_SEC_WATCHDOG_VA        HW_IO_PHYS_TO_VIRT(SEC_WATCHDOG_BASE_ADDR)
#define KONA_SECTRAP1_VA            HW_IO_PHYS_TO_VIRT(SECTRAP1_BASE_ADDR)
#define KONA_SECTRAP2_VA            HW_IO_PHYS_TO_VIRT(SECTRAP2_BASE_ADDR)
#define KONA_SECTRAP3_VA            HW_IO_PHYS_TO_VIRT(SECTRAP3_BASE_ADDR)
#define KONA_SECTRAP4_VA            HW_IO_PHYS_TO_VIRT(SECTRAP4_BASE_ADDR)
#define KONA_SECTRAP5_VA            HW_IO_PHYS_TO_VIRT(SECTRAP5_BASE_ADDR)
#define KONA_SECTRAP6_VA            HW_IO_PHYS_TO_VIRT(SECTRAP6_BASE_ADDR)
#define KONA_SECTRAP7_VA            HW_IO_PHYS_TO_VIRT(SECTRAP7_BASE_ADDR)
#define KONA_SECTRAP8_VA            HW_IO_PHYS_TO_VIRT(SECTRAP8_BASE_ADDR)
/* Secure WD Timer (not to be confused with KONA_SEC_WATCHDOG) */
#define KONA_SECWD_VA               HW_IO_PHYS_TO_VIRT(SECWD_BASE_ADDR)
/* Pad control */
#define KONA_PAD_CTRL               HW_IO_PHYS_TO_VIRT(PAD_CTRL_BASE_ADDR)
/* SIM interface */
#define KONA_SIMI_VA                HW_IO_PHYS_TO_VIRT(SIM_BASE_ADDR)
/* SIM interface */
#define KONA_SIMI2_VA               HW_IO_PHYS_TO_VIRT(SIM2_BASE_ADDR)
/* Kona Peripheral Slave Clock Manager */
#define KONA_SLV_CLK_VA             HW_IO_PHYS_TO_VIRT(KONA_SLV_CLK_BASE_ADDR)
/* Kona Peripheral Slave Reset Manager */
#define KONA_SLV_RST_VA             HW_IO_PHYS_TO_VIRT(KONA_SLV_RST_BASE_ADDR)
/* Non-Secure AXI interface */
#define KONA_SPUM_NS_VA             HW_IO_PHYS_TO_VIRT(SPUM_NS_BASE_ADDR)
/* Secure AXI interface */
#define KONA_SPUM_S_VA              HW_IO_PHYS_TO_VIRT(SPUM_S_BASE_ADDR)
/* Non-Secure APB interface */
#define KONA_SPUM_APB_NS_VA	HW_IO_PHYS_TO_VIRT(SEC_SPUM_NS_APB_BASE_ADDR)
/* Secure APB interface */
#define KONA_SPUM_APB_S_VA          HW_IO_PHYS_TO_VIRT(SEC_SPUM_S_APB_BASE_ADDR)
/* INTERNAL SRAM (160kB)*/
#define KONA_SRAM_VA                HW_IO_PHYS_TO_VIRT(SPUM_S_BASE_ADDR)

#define KONA_SSP0_VA			HW_IO_PHYS_TO_VIRT(SSP0_BASE_ADDR)
#define KONA_SSP3_VA			HW_IO_PHYS_TO_VIRT(SSP3_BASE_ADDR)
#define KONA_SSP4_VA			HW_IO_PHYS_TO_VIRT(SSP4_BASE_ADDR)

#define KONA_SCLKCAL_VA			HW_IO_PHYS_TO_VIRT(SCLKCAL_BASE_ADDR)
#define KONA_BMDM_CCU_VA		HW_IO_PHYS_TO_VIRT(BMDM_CCU_BASE_ADDR)
#define KONA_BMDM_RST_VA		HW_IO_PHYS_TO_VIRT(BMDM_RST_BASE_ADDR)
#define KONA_BMDM_PWRMGR_VA	HW_IO_PHYS_TO_VIRT(BMDM_PWRMGR_BASE_ADDR)

#define KONA_SLPTIMER_VA	HW_IO_PHYS_TO_VIRT(BMODEM_SLPTIMER_BASE_ADDR)

#define KONA_STM_VA                 HW_IO_PHYS_TO_VIRT(STM_BASE_ADDR)
#define KONA_SWSTM_VA               HW_IO_PHYS_TO_VIRT(SWSTM_BASE_ADDR)
#define KONA_SWSTM_ST_VA            HW_IO_PHYS_TO_VIRT(SWSTM_ST_BASE_ADDR)
/* System switch  */
#define KONA_SYSTEM_SWITCH_VA       HW_IO_PHYS_TO_VIRT(SYSSWITCH_BASE_ADDR)
/* SYSTEM TIMER */
#define KONA_SYSTMR_VA              HW_IO_PHYS_TO_VIRT(TIMER_BASE_ADDR)
/* Hub timer */
#define KONA_TMR_HUB_VA             HW_IO_PHYS_TO_VIRT(HUB_TIMER_BASE_ADDR)
/* Hub Clock */
#define KONA_HUB_CLK_VA             HW_IO_PHYS_TO_VIRT(HUB_CLK_BASE_ADDR)
/* Hub Clock */
#define KONA_HUB_RST_VA             HW_IO_PHYS_TO_VIRT(HUB_RST_BASE_ADDR)
#define KONA_TZCFG_VA               HW_IO_PHYS_TO_VIRT(TZCFG_BASE_ADDR)
/* UART 0 */
#define KONA_UART0_VA               HW_IO_PHYS_TO_VIRT(UARTB_BASE_ADDR)
/* UART 1 */
#define KONA_UART1_VA               HW_IO_PHYS_TO_VIRT(UARTB2_BASE_ADDR)
/* UART 2 */
#define KONA_UART2_VA               HW_IO_PHYS_TO_VIRT(UARTB3_BASE_ADDR)
/* USB FSHOST */
#define KONA_USB_FSHOST_VA          HW_IO_PHYS_TO_VIRT(FSHOST_BASE_ADDR)
/* USB FSHOST Control */
#define KONA_USB_FSHOST_CTRL_VA     HW_IO_PHYS_TO_VIRT(FSHOST_CTRL_BASE_ADDR)
/* USB OTG */
#define KONA_USB_HSOTG_VA           HW_IO_PHYS_TO_VIRT(HSOTG_BASE_ADDR)
/* USB OTG Control */
#define KONA_USB_HSOTG_CTRL_VA      HW_IO_PHYS_TO_VIRT(HSOTG_CTRL_BASE_ADDR)

/* BModem Int Ctrlr Base Address */
#define KONA_BINTC_BASE_ADDR        HW_IO_PHYS_TO_VIRT(BINTC_BASE_ADDR)

/* DSI0 Interface */
#define KONA_DSI0_VA		    HW_IO_PHYS_TO_VIRT(DSI0_BASE_ADDR)
/* DSI1 Interface */
#define KONA_DSI1_VA		    HW_IO_PHYS_TO_VIRT(DSI1_BASE_ADDR)

/* SMI Interface */
#define KONA_SMI_VA		    HW_IO_PHYS_TO_VIRT(SMI_BASE_ADDR)

/* MultiMedia CCU */
#define KONA_MM_CLK_VA		    HW_IO_PHYS_TO_VIRT(MM_CLK_BASE_ADDR)
/* MultiMedia CCU */
#define KONA_MM_RST_VA		    HW_IO_PHYS_TO_VIRT(MM_RST_BASE_ADDR)

/* TECTL interface in MM subsystem */
#define KONA_MM_TECTL_VA		HW_IO_PHYS_TO_VIRT(MM_TECTL_BASE_ADDR)
/* CSI Interface */
#define KONA_MM_CSI0_VA		HW_IO_PHYS_TO_VIRT(MM_CSI0_BASE_ADDR)
#define KONA_MM_DMA_VA		    HW_IO_PHYS_TO_VIRT(MM_DMA_BASE_ADDR)

#define KONA_MM_CFG_VA		    HW_IO_PHYS_TO_VIRT(MM_CFG_BASE_ADDR)

#define KONA_PWRMGR_VA		    HW_IO_PHYS_TO_VIRT(PWRMGR_BASE_ADDR)

#define KONA_ACI_VA		    HW_IO_PHYS_TO_VIRT(ACI_BASE_ADDR)
#define KONA_AUXMIC_VA		    HW_IO_PHYS_TO_VIRT(AUXMIC_BASE_ADDR)
#define KONA_AUDIOH_VA		    HW_IO_PHYS_TO_VIRT(AUDIOH_BASE_ADDR)

#define KONA_UART_LLDEBUG_VA		KONA_UART0_VA
#define KONA_UART_LLDEBUG_PA		UARTB_BASE_ADDR

/* add for CAPH*/
/* brcm_rdb_khub_clk_mgr_reg.h */
#define KONA_HUB_CLK_BASE_VA		  HW_IO_PHYS_TO_VIRT(HUB_CLK_BASE_ADDR)
/* brcm_rdb_audioh.h */
#define KONA_AUDIOH_BASE_VA          HW_IO_PHYS_TO_VIRT(AUDIOH_BASE_ADDR)
/* brcm_rdb_sdt.h */
#define KONA_SDT_BASE_VA             HW_IO_PHYS_TO_VIRT(SDT_BASE_ADDR)
/* brcm_rdb_sspil.h */
#define KONA_SSP4_BASE_VA            HW_IO_PHYS_TO_VIRT(SSP4_BASE_ADDR)
/* brcm_rdb_sspil.h */
#define KONA_SSP3_BASE_VA            HW_IO_PHYS_TO_VIRT(SSP3_BASE_ADDR)
/* brcm_rdb_srcmixer.h */
#define KONA_SRCMIXER_BASE_VA        HW_IO_PHYS_TO_VIRT(SRCMIXER_BASE_ADDR)
/* brcm_rdb_cph_cfifo.h */
#define KONA_CFIFO_BASE_VA           HW_IO_PHYS_TO_VIRT(CFIFO_BASE_ADDR)
/* brcm_rdb_cph_aadmac.h */
#define KONA_AADMAC_BASE_VA          HW_IO_PHYS_TO_VIRT(AADMAC_BASE_ADDR)
/* brcm_rdb_cph_ssasw.h */
#define KONA_SSASW_BASE_VA           HW_IO_PHYS_TO_VIRT(SSASW_BASE_ADDR)
/* brcm_rdb_ahintc.h */
#define KONA_AHINTC_BASE_VA          HW_IO_PHYS_TO_VIRT(AHINTC_BASE_ADDR)

#define KONA_HUB_FUNNEL_VA	HW_IO_PHYS_TO_VIRT(HUB_FUNNEL_BASE_ADDR)
#define KONA_FAB_FUNNEL1_VA	HW_IO_PHYS_TO_VIRT(FAB_FUNNEL1_BASE_ADDR)
#define KONA_BMODEM_FUNNEL_VA	HW_IO_PHYS_TO_VIRT(BMODEM_FUNNEL_BASE_ADDR)
#define KONA_AXITRACE1_VA	HW_IO_PHYS_TO_VIRT(AXITRACE1_BASE_ADDR)
#define KONA_AXITRACE2_VA	HW_IO_PHYS_TO_VIRT(AXITRACE2_BASE_ADDR)
#define KONA_AXITRACE3_VA	HW_IO_PHYS_TO_VIRT(AXITRACE3_BASE_ADDR)
#define KONA_AXITRACE4_VA	HW_IO_PHYS_TO_VIRT(AXITRACE4_BASE_ADDR)
#define KONA_AXITRACE11_VA	HW_IO_PHYS_TO_VIRT(AXITRACE11_BASE_ADDR)
#define KONA_AXITRACE12_VA	HW_IO_PHYS_TO_VIRT(AXITRACE12_BASE_ADDR)
#define KONA_AXITRACE16_VA	HW_IO_PHYS_TO_VIRT(AXITRACE16_BASE_ADDR)
#define KONA_AXITRACE17_VA	HW_IO_PHYS_TO_VIRT(AXITRACE17_BASE_ADDR)
#define KONA_AXITP18_VA		HW_IO_PHYS_TO_VIRT(AXITP18_BASE_ADDR)
#define KONA_AXITRACE19_VA	HW_IO_PHYS_TO_VIRT(AXITRACE19_BASE_ADDR)
#define KONA_HUB_CTI_VA		HW_IO_PHYS_TO_VIRT(HUB_CTI_BASE_ADDR)
#define KONA_MM_CTI_VA		HW_IO_PHYS_TO_VIRT(MM_CTI_BASE_ADDR)
#define KONA_FAB_CTI_VA		HW_IO_PHYS_TO_VIRT(FAB_CTI_BASE_ADDR)
#define KONA_A9CTI0_VA		HW_IO_PHYS_TO_VIRT(A9CTI0_BASE_ADDR)
#define KONA_BMODEM_CTI_VA	HW_IO_PHYS_TO_VIRT(BMODEM_CTI_BASE_ADDR)
#define KONA_ETB_VA		HW_IO_PHYS_TO_VIRT(ETB_BASE_ADDR)
#define KONA_ETB2AXI_VA		HW_IO_PHYS_TO_VIRT(ETB2AXI_BASE_ADDR)
#define KONA_GLOBPERF_VA	HW_IO_PHYS_TO_VIRT(GLOBPERF_BASE_ADDR)
#define KONA_GICTR_VA		HW_IO_PHYS_TO_VIRT(GICTR_BASE_ADDR)

#define KONA_PWRWDOG_VA		HW_IO_PHYS_TO_VIRT(PWRWDOG_BASE_ADDR)
#define KONA_A9CPU0_VA		HW_IO_PHYS_TO_VIRT(A9CPU0_BASE_ADDR)
#define KONA_A9PMU0_VA		HW_IO_PHYS_TO_VIRT(A9PMU0_BASE_ADDR)
#define KONA_A9PTM0_VA		HW_IO_PHYS_TO_VIRT(A9PTM0_BASE_ADDR)
#define KONA_DSP_CCU_VA		HW_IO_PHYS_TO_VIRT(DSP_CCU_BASE_ADDR)

#define KONA_RESERVED_ROM_VA HW_IO_PHYS_TO_VIRT(0x3404B000)

#endif /* __RHEA_IO_MAP_H */

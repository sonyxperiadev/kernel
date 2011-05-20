/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
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

#ifndef __CHIP_PINMUX_H__
#define __CHIP_PINMUX_H__

/*
This file defines ball and function name for Samoa A0
http://mpgbu.broadcom.com/mobcom/3GULC/ASIC BB/top level/samoa_pinmuxing.xlsx
Initial update for Samoa
*/

/* define ball name, generated from RDB */
enum PIN_NAME {
    PN_ANA_CLK_REQ0,
    PN_BSC1_CLK,
    PN_BSC1_DAT,
    PN_BSC2_CLK,
    PN_BSC2_DAT,
    PN_CAM_CS0,
    PN_CLK_32K,
    PN_DCLK0,
    PN_DCLK_REQ0,
    PN_GPIO00,
    PN_GPIO01,
    PN_GPIO02,
    PN_GPIO03,
    PN_GPIO04,
    PN_GPIO05,
    PN_GPIO06,
    PN_GPIO07,
    PN_GPIO08,
    PN_GPIO09,
    PN_GPIO10,
    PN_GPIO11,
    PN_GPIO12,
    PN_GPIO13,
    PN_GPIO14,
    PN_GPIO15,
    PN_GPIO16,
    PN_GPIO17,
    PN_GPIO18,
    PN_GPIO19,
    PN_GPIO20,
    PN_GPIO21,
    PN_GPIO22,
    PN_GPIO23,
    PN_GPIO24,
    PN_LCD_CD,
    PN_LCD_CS0,
    PN_LCD_CS1,
    PN_LCD_D0,
    PN_LCD_D1,
    PN_LCD_D2,
    PN_LCD_D3,
    PN_LCD_D4,
    PN_LCD_D5,
    PN_LCD_D6,
    PN_LCD_D7,
    PN_LCD_RE,
    PN_LCD_RST,
    PN_LCD_TE0,
    PN_LCD_TE1,
    PN_LCD_WE,
    PN_MDM_GPIO00,
    PN_MDM_GPIO01,
    PN_MDM_GPIO02,
    PN_MDM_GPIO03,
    PN_MDM_GPIO04,
    PN_MDM_GPIO05,
    PN_MDM_GPIO06,
    PN_MDM_GPIO07,
    PN_MDM_GPIO08,
    PN_MMC_CLK,
    PN_MMC_CMD,
    PN_MMC_DAT0,
    PN_MMC_DAT1,
    PN_MMC_DAT2,
    PN_MMC_DAT3,
    PN_MMC_DAT4,
    PN_MMC_DAT5,
    PN_MMC_DAT6,
    PN_MMC_DAT7,
    PN_MMC_RST,
    PN_PCM_CLK,
    PN_PCM_DI,
    PN_PCM_DO,
    PN_PCM_SYN,
    PN_RESET_N,
    PN_SD_CLK,
    PN_SD_CMD,
    PN_SD_DAT0,
    PN_SD_DAT1,
    PN_SD_DAT2,
    PN_SD_DAT3,
    PN_SIM1_CLK,
    PN_SIM1_DAT,
    PN_SIM1_DET,
    PN_SIM1_RST,
    PN_SIM2_CLK,
    PN_SIM2_DAT,
    PN_SIM2_DET,
    PN_SIM2_RST,
    PN_SPI_CLK,
    PN_SPI_FSS,
    PN_SPI_RX,
    PN_SPI_TX,
    PN_SWD_TCKC,
    PN_SWD_TMSC,
    PN_TRACE_CLK,
    PN_TRACE_DATA00,
    PN_TRACE_DATA01,
    PN_TRACE_DATA02,
    PN_TRACE_DATA03,
    PN_UARTA_CTS_N,
    PN_UARTA_RTS_N,
    PN_UARTA_RX,
    PN_UARTA_TX,

    PN_MAX
};

/* define function name, order is not important */
enum PIN_FUNC {
	PF_RESERVED	=	0,
	PF_GPIO,

    PF_ANA_CLK_REQ0,
    PF_ANA_CLK_REQ1,
    PF_ANA_CLK_REQ2,
	PF_BSC1_CLK,
	PF_BSC1_DAT,
	PF_BSC2_CLK,
	PF_BSC2_DAT,

	PF_DCLK0,
	PF_DCLK_REQ0,
	PF_DCLK1,
	PF_DCLK_REQ1,

	PF_SRI_C,
	PF_SRI_D,
	PF_SRI_E,

	PF_MMCX_GPO,

	PF_KEY_R0,
	PF_KEY_R1,
	PF_KEY_R2,
	PF_KEY_R3,
	PF_KEY_R4,
	PF_KEY_R5,
	PF_KEY_R6,
	PF_KEY_R7,
	PF_KEY_C0,
	PF_KEY_C1,
	PF_KEY_C2,
	PF_KEY_C3,
	PF_KEY_C4,
	PF_KEY_C5,
	PF_KEY_C6,
	PF_KEY_C7,

	PF_PWM0,
	PF_PWM1,
	PF_PWM2,
	PF_PWM3,
	PF_PWM4,
	PF_PWM5,

	PF_GPEN0,
	PF_GPEN1,
	PF_GPEN2,
	PF_GPEN3,
	PF_GPEN4,
	PF_GPEN5,
	PF_GPEN6,
	PF_GPEN7,
	PF_GPEN8,
	PF_GPEN9,
	PF_GPEN10,
	PF_GPEN11,
	PF_GPEN12,
	PF_GPEN13,
	PF_GPEN14,
	PF_GPEN15,
	PF_GPEN16,
	PF_GPEN17,
	PF_GPEN18,

	PF_DMIC0CLK,
	PF_DMIC0DQ,
	PF_DMIC1CLK,
	PF_DMIC1DQ,

	PF_LCD_SCL,
	PF_LCD_SDA,

	PF_LCD_CS1,
	PF_LCD_CS0,
	PF_LCD_CD,
	PF_LCD_WE,
	PF_LCD_RE,
	PF_LCD_D8,
	PF_LCD_D7,
	PF_LCD_D6,
	PF_LCD_D5,
	PF_LCD_D4,
	PF_LCD_D3,
	PF_LCD_D2,
	PF_LCD_D1,
	PF_LCD_D0,
	PF_LCD_TE0,
	PF_LCD_RST,
	PF_LCD_TE1,

	PF_CAMD0,
	PF_CAMD1,
	PF_CAMD2,
	PF_CAMD3,
	PF_CAMD4,
	PF_CAMD5,
	PF_CAMD6,
	PF_CAMD7,
	PF_CAMPCLK,
	PF_CAMHS,
	PF_CAMVS,

	PF_UB2TX,
	PF_UB2RX,
	PF_UB2RTSN,
	PF_UB2CTSN,

    PF_CLK_32K,
    PF_CLK32EXT,
	PF_CLK_MON,

	PF_ACDATA,
	PF_ACFLAG,
	PF_ACREADY,
	PF_ACWAKE,

	PF_CADATA,
	PF_CAFLAG,
	PF_CAREADY,
	PF_CAWAKE,

	PF_BATREMO,
	PF_BATREMI,

	PF_PTL_CLK,
	PF_PTL_DAT3,
	PF_PTL_DAT2,
	PF_PTL_DAT1,
	PF_PTL_DAT0,

	PF_SSP1_CLK,
	PF_SSP1_SYN,
	PF_SSP1_DO,
	PF_SSP1_DI,
	PF_SSP4_CLK,
	PF_SSP4_SYN,
	PF_SSP4_DO,
	PF_SSP4_DI,
	PF_SSP0_CLK,
	PF_SSP0_SYN,
	PF_SSP0_DO,
	PF_SSP0_DI,
	PF_SSP3_CLK,
	PF_SSP3_SYN,
	PF_SSP3_DO,
	PF_SSP3_DI,

	PF_PTI_CLK,
	PF_PTI_DAT0,
	PF_PTI_DAT1,
	PF_PTI_DAT2,
	PF_PTI_DAT3,
	PF_RXD,

	PF_DEBUG_PORT,
	PF_PM_DEBUG0,
	PF_PM_DEBUG1,
	PF_PM_DEBUG2,
	PF_PM_DEBUG3,

	PF_OSC1_OUT,
	PF_OSC2_OUT,

	PF_AP,
	PF_RF,
	PF_TRACE,
	PF_DSP_TRACE,

	PF_SIM1_CLK,
	PF_SIM1_DAT,
	PF_SIM1_DET,
	PF_SIM1_RST,
	PF_SIM2_CLK,
	PF_SIM2_DAT,
	PF_SIM2_DET,
	PF_SIM2_RST,

    PF_UARTA_CTS_N,
    PF_UARTA_RTS_N,
    PF_UARTA_RX,
    PF_UARTA_TX,

	PF_PC1,
	PF_PC2,
	PF_PC3,
    PF_SWD_TCKC,
    PF_SWD_TMSC,

	PF_MMC0_CLK,
	PF_MMC0_CMD,
	PF_MMC0_RST,
	PF_MMC0_DAT0,
	PF_MMC0_DAT1,
	PF_MMC0_DAT2,
	PF_MMC0_DAT3,
	PF_MMC0_DAT4,
	PF_MMC0_DAT5,
	PF_MMC0_DAT6,
	PF_MMC0_DAT7,

	PF_MMC1_CLK,
	PF_MMC1_CMD,
	PF_MMC1_RST,
	PF_MMC1_DAT0,
	PF_MMC1_DAT1,
	PF_MMC1_DAT2,
	PF_MMC1_DAT3,
	PF_MMC1_DAT4,
	PF_MMC1_DAT5,
	PF_MMC1_DAT6,
	PF_MMC1_DAT7,

	PF_DSP_RTCK,
	PF_DSP_TDI,
	PF_DSP_TCKC,
	PF_DSP_TMSC,
	PF_DSP_TRSTB,
	PF_DSP_TDO,

	PF_TDO,
	PF_TDI,
	PF_TRSTB,

	PF_RESETN,
	PF_U1TXD,
	PF_U1RXD,

	PF_RFGPIO5,

	PF_MAX
};

/* each Pin has up to 6 functions */
#define	MAX_ALT_FUNC		6

#endif /* __CHIP_PINMUX_H__ */

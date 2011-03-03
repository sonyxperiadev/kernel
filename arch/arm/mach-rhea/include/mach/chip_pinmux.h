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
This file defines ball and function name for Rhea -21654
http://mpgbu.broadcom.com/mobcom/hera/Documents/Rhea-BB/ASIC%20Specification/Top%20Level/rhea_PinMuxing.xls
*/

/* define ball name, generated from RDB */
enum PIN_NAME {
	PN_ADCSYN,
	PN_BATRM,
	PN_BSC1CLK,
	PN_BSC1DAT,
	PN_CAMCS0,
	PN_CAMCS1,
	PN_CLK32K,
	PN_CLK_CX8,
	PN_DCLK1,
	PN_DCLK4,
	PN_DCLKREQ1,
	PN_DCLKREQ4,
	PN_DMIC0CLK,
	PN_DMIC0DQ,
	PN_DSI0TE,
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
	PN_GPIO25,
	PN_GPIO26,
	PN_GPIO27,
	PN_GPIO28,
	PN_GPIO32,
	PN_GPIO33,
	PN_GPIO34,
	PN_GPS_CALREQ,
	PN_GPS_HOSTREQ,
	PN_GPS_PABLANK,
	PN_GPS_TMARK,
	PN_ICUSBDM,
	PN_ICUSBDP,
	PN_LCDCS0,
	PN_LCDRES,
	PN_LCDSCL,
	PN_LCDSDA,
	PN_LCDTE,
	PN_MDMGPIO00,
	PN_MDMGPIO01,
	PN_MDMGPIO02,
	PN_MDMGPIO03,
	PN_MDMGPIO04,
	PN_MDMGPIO05,
	PN_MDMGPIO06,
	PN_MDMGPIO07,
	PN_MDMGPIO08,
	PN_MMC0CK,
	PN_MMC0CMD,
	PN_MMC0DAT0,
	PN_MMC0DAT1,
	PN_MMC0DAT2,
	PN_MMC0DAT3,
	PN_MMC0DAT4,
	PN_MMC0DAT5,
	PN_MMC0DAT6,
	PN_MMC0DAT7,
	PN_MMC0RST,
	PN_MMC1CK,
	PN_MMC1CMD,
	PN_MMC1DAT0,
	PN_MMC1DAT1,
	PN_MMC1DAT2,
	PN_MMC1DAT3,
	PN_MMC1DAT4,
	PN_MMC1DAT5,
	PN_MMC1DAT6,
	PN_MMC1DAT7,
	PN_MMC1RST,
	PN_PC1,
	PN_PC2,
	PN_PMBSCCLK,
	PN_PMBSCDAT,
	PN_PMUINT,
	PN_RESETN,
	PN_RFST2G_MTSLOTEN3G,
	PN_RTXDATA2G_TXDATA3G1,
	PN_RTXEN2G_TXDATA3G2,
	PN_RXDATA3G0,
	PN_RXDATA3G1,
	PN_RXDATA3G2,
	PN_SDCK,
	PN_SDCMD,
	PN_SDDAT0,
	PN_SDDAT1,
	PN_SDDAT2,
	PN_SDDAT3,
	PN_SIMCLK,
	PN_SIMDAT,
	PN_SIMDET,
	PN_SIMRST,
	PN_GPIO93,
	PN_GPIO94,
	PN_SPI0CLK,
	PN_SPI0FSS,
	PN_SPI0RXD,
	PN_SPI0TXD,
	PN_SRI_C,
	PN_SRI_D,
	PN_SRI_E,
	PN_SSPCK,
	PN_SSPDI,
	PN_SSPDO,
	PN_SSPSYN,
	PN_STAT1,
	PN_STAT2,
	PN_SWCLKTCK,
	PN_SWDIOTMS,
	PN_SYSCLKEN,
	PN_TDI,
	PN_TDO,
	PN_TESTMODE,
	PN_TRACECLK,
	PN_TRACEDT00,
	PN_TRACEDT01,
	PN_TRACEDT02,
	PN_TRACEDT03,
	PN_TRACEDT04,
	PN_TRACEDT05,
	PN_TRACEDT06,
	PN_TRACEDT07,
	PN_TRSTB,
	PN_TXDATA3G0,
	PN_UBCTSN,
	PN_UBRTSN,
	PN_UBRX,
	PN_UBTX,

	PN_MAX
};

/* define function name, order is not important */
enum PIN_FUNC {
	PF_RESERVED	=	0,
	PF_GPIO,

	PF_MMC0CK,
	PF_MMC0CMD,
	PF_MMC0RST,
	PF_MMC0DAT7,
	PF_MMC0DAT6,
	PF_MMC0DAT5,
	PF_MMC0DAT4,
	PF_MMC0DAT3,
	PF_MMC0DAT2,
	PF_MMC0DAT1,
	PF_MMC0DAT0,

	PF_MMC1CK,
	PF_MMC1CMD,
	PF_MMC1RST,
	PF_MMC1DAT7,
	PF_MMC1DAT6,
	PF_MMC1DAT5,
	PF_MMC1DAT4,
	PF_MMC1DAT3,
	PF_MMC1DAT2,
	PF_MMC1DAT1,
	PF_MMC1DAT0,


	PF_SDCK,
	PF_SDCMD,
	PF_SDDAT3,
	PF_SDDAT2,
	PF_SDDAT1,
	PF_SDDAT0,

	PF_SSP0CK,
	PF_SSP0SYN,
	PF_SSP0DO,
	PF_SSP0DI,

	PF_SSP1CK,
	PF_SSP1SYN,
	PF_SSP1DO,
	PF_SSP1DI,

	PF_SSP2CK,
	PF_SSP2SYN,
	PF_SSP2DO,
	PF_SSP2DI,

	PF_PTI_CLK,
	PF_RXD,

	PF_SIMLDO_EN,

	PF_PWM0,
	PF_PWM1,
	PF_PWM2,
	PF_PWM3,
	PF_PWM4,
	PF_PWM5,

	PF_SYSCLKREQA,
	PF_SYSCLKREQB,

	PF_DCLK2,
	PF_DCLK3,

	PF_MAX
};

/* each Pin has up to 6 functions */
#define	MAX_ALT_FUNC		6

#endif /* __CHIP_PINMUX_H__ */

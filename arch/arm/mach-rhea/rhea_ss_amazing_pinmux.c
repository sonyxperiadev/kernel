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
#include <linux/kernel.h>
#include <linux/init.h>
#include "mach/pinmux.h"
#include <mach/rdb/brcm_rdb_padctrlreg.h>

static struct __init pin_config board_pin_config[] = {
	/* BSC1 */
	PIN_BSC_CFG(BSC1CLK, BSC1CLK, 0x20),
	PIN_BSC_CFG(BSC1DAT, BSC1DAT, 0x20),

	/* BSC2 */
	PIN_BSC_CFG(GPIO16, BSC2CLK, 0x20),
	PIN_BSC_CFG(GPIO17, BSC2DAT, 0x20),

	/* PMU BSC */
	PIN_BSC_CFG(PMBSCCLK, PMBSCCLK, 0x20),
	PIN_BSC_CFG(PMBSCDAT, PMBSCDAT, 0x20),

	/*
	 * Note:- For eMMC, Enable Slew-rate, Increase pin drive strength to 10mA.
	 * 	This is to fix the random eMMC timeout errors due to data crc error
	 * 	seen on few rhea edn11 hardware, where eMMC is on a daughter-card.
	 *
	 * 	We may need to revisit these settings for other platforms where the
	 * 	pin drive requirements can change.
	 *
	 */
	/* eMMC */
	PIN_CFG(MMC0CK, MMC0CK, 0, OFF, OFF, 1, 0, 10MA),
	PIN_CFG(MMC0CMD, MMC0CMD, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0RST, MMC0RST, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT7, MMC0DAT7, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT6, MMC0DAT6, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT5, MMC0DAT5, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT4, MMC0DAT4, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT3, MMC0DAT3, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT2, MMC0DAT2, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT1, MMC0DAT1, 0, OFF, ON, 1, 0, 10MA),
	PIN_CFG(MMC0DAT0, MMC0DAT0, 0, OFF, ON, 1, 0, 10MA),

    /* Micro SD - SDIO0 4 bit interface */
	PIN_CFG(SDCK, SDCK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SDCMD, SDCMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT3, SDDAT3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT2, SDDAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT1, SDDAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT0, SDDAT0, 0, OFF, ON, 0, 0, 8MA),

#if defined(CONFIG_MACH_RHEA_RAY_EDN1X) || defined(CONFIG_MACH_RHEA_RAY_EDN2X) || defined(CONFIG_MACH_RHEA_SS) 
	
	/* GPIO121 for TCA9539 IO expander */
	PIN_CFG(ICUSBDP, GPIO121, 0, OFF, OFF, 0, 0, 8MA),
#else
	/* GPIO74 for TCA9539 IO expander */
//	PIN_CFG(MMC1DAT4, GPIO74, 0, OFF, ON, 0, 0, 8MA),
#endif

	/*	Pinmux for keypad */
	PIN_CFG(GPIO00, KEY_R0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO01, KEY_R1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO02, KEY_R2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO08, KEY_C0, 0, OFF, ON, 0, 0, 8MA),

	/* SSP0 */
//	PIN_CFG(SPI0FSS, SSP0SYN, 0, OFF, OFF, 0, 0, 8MA),
//	PIN_CFG(SPI0CLK,  SSP0CK, 0, OFF, OFF, 0, 0, 8MA),
//	PIN_CFG(SPI0TXD,  SSP0DO, 0, OFF, OFF, 0, 0, 8MA),
//	PIN_CFG(SPI0RXD,  SSP0DI, 0, OFF,  ON, 0, 0, 8MA),

	/* SSP3 - PCM
	   SSP3 pinmux is set since keypad also check the same pins currently */
#ifndef CONFIG_MACH_RHEA_RAY_DEMO
	PIN_CFG(GPIO15, GPIO15, 0, OFF, OFF, 0, 0, 8MA), // SENSOR_SDA
	PIN_CFG(GPIO14, GPIO14, 0, OFF, OFF, 0, 0, 8MA), // TP
	PIN_CFG(GPIO07, GPIO7, 0, OFF, OFF, 0, 0, 8MA), //TP
	PIN_CFG(GPIO06, GPIO6, 0, OFF,  ON, 0, 0, 8MA), //NC
#endif

	/* SSP4 - I2S */
#if defined(CONFIG_MACH_RHEA_RAY) || defined (CONFIG_MACH_RHEA_RAY_EDN1X) || defined (CONFIG_MACH_RHEA_RAY_EDN2X) \
	|| defined(CONFIG_MACH_RHEA_RAY_DEMO) || defined(CONFIG_MACH_RHEA_SS) || defined(CONFIG_MACH_RHEA_SS_AMAZING)
	PIN_CFG(GPIO94, SSP1SYN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO93, SSP1CK, 0, OFF, OFF, 0, 0, 8MA),
//	PIN_CFG(DCLK4,  SSP1DO, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(DCLKREQ4, SSP1DI, 0, OFF,  ON, 0, 0, 8MA),
#endif

//	PIN_CFG(DCLK4,  GPIO95, 0, OFF, OFF, 0, 0, 8MA),
	/* LCD */
#if defined(CONFIG_MACH_RHEA_RAY)
	PIN_CFG(DSI0TE, LCDTE, 0, OFF, ON, 0, 0, 8MA),
#else
	PIN_CFG(LCDTE, LCDTE, 0, OFF, ON, 0, 0, 8MA),
#endif
	PIN_CFG(LCDRES, GPIO41, 0, OFF, ON, 0, 0, 8MA),
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
	/* conflicts with SSP4 so comment it out.  Leave the code here for possible future change */
//	PIN_CFG(DCLK4, GPIO95, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(DCLKREQ4, GPIO96, 0, OFF, ON, 0, 0, 8MA),
#endif

	/* STM trace - PTI */
	PIN_CFG(TRACECLK, PTI_CLK, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT07, RXD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT00, PTI_DAT0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT01, PTI_DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT02, PTI_DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT03, PTI_DAT3, 0, OFF, ON, 0, 0, 8MA),
		

	/* BSC1 - CAM */
	PIN_BSC_CFG(BSC1CLK, BSC1CLK, 0x20),						// CAM_SCL       
	PIN_BSC_CFG(BSC1DAT, BSC1DAT, 0x20),						// CAM_SDA
	/* Camera */
	PIN_CFG(DCLK1, DCLK1, 0, ON, OFF, 0, 0, 8MA), 				// CAM_DCLK1
	PIN_CFG(LCDTE, GPIO42, 0, ON, OFF, 0, 0, 8MA), //CAM_CORE_EN
	PIN_CFG(DCLKREQ1, GPIO111, 0, ON, OFF, 0, 0, 8MA), //3M_CAM_STNBY
	PIN_CFG(GPIO33, GPIO33, 0, ON,OFF, 0, 0, 8MA), //3M_CAM_RESET
	PIN_CFG(GPIO34, GPIO34, 0, ON,OFF, 0, 0, 8MA), //VT_CAM_STNBY
	PIN_CFG(GPIO23, GPIO23, 0, ON,OFF, 0, 0, 8MA), //VT_CAM_RESET	


	/* SMI */
//	PIN_CFG(LCDSCL, LCDCD, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(LCDSDA, LCDD0, 0, OFF, ON, 0, 0, 8MA),
#ifdef CONFIG_MACH_RHEA_RAY_DEMO
	PIN_CFG(GPIO00, LCDD15, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO01, LCDD14, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO02, LCDD13, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(GPIO03, LCDD12, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO08, LCDD11, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO09, LCDD10, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(GPIO10, LCDD9, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(GPIO11, LCDD8, 0, OFF, ON, 0, 0, 8MA),
#endif
//	PIN_CFG(GPIO18, LCDCS1, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(GPIO19, LCDWE, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(GPIO20, LCDRE, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(GPIO21, LCDD7, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO22, GPIO22, 0, OFF, ON, 0, 0, 8MA), //TOUCH_EN
	PIN_CFG(GPIO24, LCDD4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO25, GPIO25, 0, OFF, ON, 0, 0, 8MA), //@CAM_AF_EN -> NFC_EN
	PIN_CFG(GPIO26, GPIO26, 0, OFF, OFF, 0, 0, 16MA), // TSP_SCL
	PIN_CFG(GPIO27, GPIO27, 0, OFF, OFF, 0, 0, 16MA), // TSP_SDA

	/* PWM config - PWM4, PWM5*/
	PIN_CFG(DCLK4, PWM4, 0, OFF, ON, 0, 0, 8MA),
#ifdef CONFIG_MACH_RHEA_RAY_EDN1X
	/* conflicts with SSP4 so comment it out. Leave the code here for possible future change */
//	PIN_CFG(DCLK4, PWM4, 0, OFF, ON, 0, 0, 8MA),
//	PIN_CFG(DCLKREQ4, PWM5, 0, OFF, ON, 0, 0, 8MA),
#endif

	/* SIM2LDO_EN through GPIO99 (TPS728XX) */
	PIN_CFG(GPS_CALREQ, GPIO99, 0, OFF, ON, 0, 0, 8MA), /* NFC_IRQ */

	/* Bluetooth related GPIOS */
	PIN_CFG(LCDSCL, GPIO39, 0, ON, OFF, 0, 0, 8MA), /* BT_WAKE */
	PIN_CFG(UBRTSN, GPIO47, 0, OFF, OFF, 0, 0, 8MA), /* BT_HOST_WAKE */
	PIN_CFG(GPS_HOSTREQ, GPIO100, 0, ON, OFF, 0, 0, 8MA), /* BT_REG_ON */

	PIN_CFG(MMC1DAT4, SSP2SYN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC1DAT7, SSP2CK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC1DAT6, SSP2DO, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC1DAT5, SSP2DI, 0, OFF,  ON, 0, 0, 8MA),

	// for GPS
	PIN_CFG(GPIO28, GPIO28, 0, OFF, ON, 0, 0, 8MA),   /* GPS_EN */
	PIN_CFG(DMIC0DQ, GPIO124, 0, OFF, ON, 0, 0, 8MA), /* GPS_HOST_REQ */
	PIN_CFG(GPS_PABLANK, GPIO97, 0, ON, OFF, 0, 0, 8MA),   /* GPS_CAL_REQ */


	PIN_CFG(GPIO18, UB2TX, 0, OFF, OFF, 0, 0, 8MA),   /* UART B2 */
	PIN_CFG(GPIO19, UB2RX, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO20, UB2RTSN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO21, UB2CTSN, 0, OFF, OFF, 0, 0, 8MA),

	/*WLAN set SSPSYN as GPIO85 */
	/*
	 * On Rhearay EDN1x the MMC1 i.e SDIO3 is used for 
	 * WLAN connectivity
	 */
#if defined(CONFIG_MACH_RHEA_RAY_EDN1X) || defined(CONFIG_MACH_RHEA_SS) || defined(CONFIG_MACH_RHEA_SS_AMAZING)
	PIN_CFG(MMC1DAT0, MMC1DAT0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC1DAT1, MMC1DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC1DAT2, MMC1DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC1DAT3, MMC1DAT3, 0, OFF, ON, 0, 0, 8MA),


	PIN_CFG(MMC1CK, MMC1CK, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC1CMD, MMC1CMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC1RST, GPIO70, 0, OFF, ON, 0, 0, 8MA),


#endif

	/* WLAN configuration for EDN2x */
	/*
	 * On Rhearay EDN2x the MMC1 i.e SDIO3 is used for 
	 * WLAN connectivity
	 */
#if defined(CONFIG_MACH_RHEA_RAY_EDN2X)
	/* WLAN - SDIO4 - 4 bit interface */
	/* TODO - This is not complete, connectivity team to
	 * review and update if needed
	 */
//	PIN_CFG(SPI0FSS, SD1DAT3, 0, OFF, ON, 0, 0, 16MA),
//	PIN_CFG(SPI0CLK, SD1CK, 0, OFF, ON, 0, 0, 16MA),
//	PIN_CFG(SPI0TXD, SD1CMD, 0, OFF, ON, 0, 0, 16MA),
//	PIN_CFG(SPI0RXD, SD1DAT0, 0, OFF, ON, 0, 0, 16MA),
//	PIN_CFG(GPIO93, SD1DAT1, 0, OFF, ON, 0, 0, 16MA),
//	PIN_CFG(GPIO94, SD1DAT2, 0, OFF, ON, 0, 0, 16MA),
#endif


	PIN_CFG(SSPDO, SIM2DAT, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(SSPCK, SIM2CLK, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(DMIC0CLK, GPIO123, 0, OFF, OFF, 0, 0, 16MA), //SD_DECTECT
	PIN_CFG(DSI0TE, LCDTE, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(CAMCS0, GPIO43, 0, OFF, OFF, 0, 0, 16MA),
	/* Headset */
        PIN_CFG(STAT1, GPIO31, 0, OFF, ON, 0, 0, 4MA),

};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

	return 0;
}

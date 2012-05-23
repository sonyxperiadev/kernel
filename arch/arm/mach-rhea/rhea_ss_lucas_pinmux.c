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


	/* PMU BSC */
	PIN_BSC_CFG(PMBSCCLK, PMBSCCLK, 0x20),						// PMU_SCL
	PIN_BSC_CFG(PMBSCDAT, PMBSCDAT, 0x20),						// PMU_SDA
//	PIN_CFG(PMUINT, GPIO29, 0, OFF, ON, 1, 0, 8MA),			// PMU_IRQ
//	PIN_CFG(UBRX, UB1_IF_UART_RX, 0, OFF, ON, 0, 0, 8MA),		// UB1_IF_UART_RX
//	PIN_CFG(UBTX, UB1_IF_UART_TX, 0, OFF, ON, 0, 0, 8MA),		// UB1_IF_UART_TX
//	PIN_CFG(ADCSYN, GPEN09, 0, OFF, ON, 0, 0, 8MA),			// ADCSYNC_PMU

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
	PIN_CFG(MMC0CK, MMC0CK, 0, OFF, ON, 1, 0, 10MA),
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
	PIN_CFG(DMIC0CLK, GPIO123, 0, OFF, ON, 0, 0, 16MA),		//SD_DECTECT

	/*	Pinmux for keypad */
	PIN_CFG(GPIO00, KEY_R0, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO01, KEY_R1, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO02, KEY_R2, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO03, KEY_R3, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO04, KEY_R4, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO05, KEY_R5, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO06, KEY_R6, 0, ON, OFF, 0, 0, 8MA),

	PIN_CFG(GPIO08, KEY_C0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO09, KEY_C1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO10, KEY_C2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO11, KEY_C3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO12, KEY_C4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO13, KEY_C5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO14, KEY_C6, 0, OFF, ON, 0, 0, 8MA),


	/* LCD */
	PIN_CFG(GPIO24, LCDD4, 0, OFF, ON, 0, 0, 8MA),				// LCD_DET
	PIN_CFG(LCDRES, GPIO41, 0, OFF, ON, 0, 0, 8MA),				// LCD_RST
	PIN_CFG(DCLK4,  GPIO95, 0, ON, OFF, 0, 0, 8MA),				// LCD_BL_CTRL
	PIN_CFG(DSI0TE, LCDTE, 0, OFF, ON, 0, 0, 8MA),				// FLM
	PIN_CFG(GPIO22, GPIO22, 0, ON, OFF, 0, 0, 8MA), 			// TOUCH_EN

	/* STM trace - PTI */
//	PIN_CFG(TRACECLK, PTI_CLK, 0, OFF, ON, 0, 0, 8MA),		// N.C
//	PIN_CFG(TRACEDT07, RXD, 0, OFF, ON, 0, 0, 8MA),		// N.C
//	PIN_CFG(TRACEDT00, PTI_DAT0, 0, OFF, ON, 0, 0, 8MA),	// N.C
//	PIN_CFG(TRACEDT01, PTI_DAT1, 0, OFF, ON, 0, 0, 8MA),	// N.C
//	PIN_CFG(TRACEDT02, PTI_DAT2, 0, OFF, ON, 0, 0, 8MA),	// N.C
//	PIN_CFG(TRACEDT03, PTI_DAT3, 0, OFF, ON, 0, 0, 8MA),	// N.C
		

	/* BSC1 - CAM */
	PIN_BSC_CFG(BSC1CLK, BSC1CLK, 0x20),						// CAM_SCL
	PIN_BSC_CFG(BSC1DAT, BSC1DAT, 0x20),						// CAM_SDA
	/* Camera */
	PIN_CFG(DCLK1, DCLK1, 0, ON, OFF, 0, 0, 8MA), 				// CAM_DCLK1
	PIN_CFG(LCDTE, GPIO42, 0, ON, OFF, 0, 0, 8MA), 				// CAM_CORE_EN
	PIN_CFG(ICUSBDP, GPIO121, 0, ON, OFF, 0, 0, 8MA), 			// CAM_AF_EN	
	PIN_CFG(DCLKREQ1, GPIO111, 0, ON, OFF, 0, 0, 8MA), 			// 5M_CAM_STNBY
	PIN_CFG(GPIO33, GPIO33, 0, ON, OFF, 0, 0, 8MA), 			// 5M_CAM_RESET
	PIN_CFG(GPIO34, GPIO34, 0, ON, OFF, 0, 0, 8MA), 			// VT_CAM_STNBY
	PIN_CFG(GPIO23, GPIO23, 0, ON, OFF, 0, 0, 8MA), 			// VT_CAM_RESET	


	/* Bluetooth related GPIOS */
	PIN_CFG(LCDSCL, GPIO39, 0, ON, OFF, 0, 0, 8MA),      		/* BT_WAKE */
	PIN_CFG(UBRTSN, GPIO47, 0, OFF, OFF, 0, 0, 8MA), 		/* BT_HOST_WAKE */
	PIN_CFG(GPS_HOSTREQ, GPIO100, 0, ON, OFF, 0, 0, 8MA), 		/* BT_REG_ON */

	/* SSP4 - I2S */
	PIN_CFG(DCLKREQ4, SSP1DI, 0, OFF,  ON, 0, 0, 8MA),		/* BT_I2S_DO */
	PIN_CFG(GPIO94, SSP1SYN, 0, OFF, ON, 0, 0, 8MA),		/* BT_I2S_WS */
	PIN_CFG(GPIO93, SSP1CK, 0, OFF, ON, 0, 0, 8MA),			/* BT_I2S_CLK */

	PIN_CFG(MMC1DAT7, SSP2CK, 0, OFF, OFF, 0, 0, 16MA), 		/* BT_PCM_CLK */
	PIN_CFG(MMC1DAT6, SSP2DO, 0, OFF, OFF, 0, 0, 16MA), 		/* BT_PCM_IN */
	PIN_CFG(MMC1DAT5, SSP2DI, 0, OFF, ON, 0, 0, 16MA), 		/* BT_PCM_OUT */
	PIN_CFG(MMC1DAT4, SSP2SYN, 0, OFF, OFF, 0, 0, 16MA), 		/* BT_PCM_SYNC */

	PIN_CFG(GPIO18, UB2TX, 0, OFF, ON, 0, 0, 8MA),   			// UB2_BT_UART_TX
	PIN_CFG(GPIO19, UB2RX, 0, OFF, ON, 0, 0, 8MA),				// UB2_BT_UART_RX
	PIN_CFG(GPIO20, UB2RTSN, 0, OFF, ON, 0, 0, 8MA),			// UB2_BT_UART_RTS_N
	PIN_CFG(GPIO21, UB2CTSN, 0, OFF, ON, 0, 0, 8MA),			// UB2_BT_UART_CTS_N


	/* for GPS */
	PIN_CFG(GPIO28, GPIO28, 0, OFF, ON, 0, 0, 8MA), 		/* GPS_EN */
	PIN_CFG(DMIC0DQ, GPIO124, 0, ON, OFF, 0, 0, 8MA), 		/* GPS_HOST_REQ */

	/* GPS - BSC2 */
	PIN_BSC_CFG(GPIO16, BSC2CLK, 0x20), 						// GPS_SCL
	PIN_BSC_CFG(GPIO17, BSC2DAT, 0x20), 						// GPS_SDA
	PIN_CFG(GPS_PABLANK, GPIO97, 0, ON, OFF, 0, 0, 8MA), 		// GPS_CAL_REQ
	PIN_CFG(GPS_TMARK, GPEN10, 0, ON, OFF, 0, 0, 8MA),			// GPS_SYNC


	/* WLAN */
	PIN_CFG(MMC1DAT0, MMC1DAT0, 0, OFF, ON, 0, 0, 8MA),		// WLAN_SDIO_DAT0
	PIN_CFG(MMC1DAT1, MMC1DAT1, 0, OFF, ON, 0, 0, 8MA),		// WLAN_SDIO_DAT1
	PIN_CFG(MMC1DAT2, MMC1DAT2, 0, OFF, ON, 0, 0, 8MA),		// WLAN_SDIO_DAT2
	PIN_CFG(MMC1DAT3, MMC1DAT3, 0, OFF, ON, 0, 0, 8MA),		// WLAN_SDIO_DAT3
	PIN_CFG(MMC1CK, MMC1CK, 0, OFF, ON, 0, 0, 8MA),			// WLAN_SDIO_CLK
	PIN_CFG(MMC1CMD, MMC1CMD, 0, OFF, ON, 0, 0, 8MA),		// WLAN_SDIO_CMD
	PIN_CFG(MMC1RST, GPIO70, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(UBCTSN, GPIO48, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(CAMCS1, ANA_SYS_REQ2, 0, ON, OFF, 0, 0, 8MA),

	/* Touch */	
	PIN_CFG(SSPSYN, GPIO85, 0, OFF, OFF, 0, 0, 16MA), 			/* TSP_SDA*/
	PIN_CFG(SSPDO, GPIO86, 0, OFF, ON, 0, 0, 8MA),				/* TSP_INT*/
	PIN_CFG(SSPCK, GPIO87, 0, OFF, OFF, 0, 0, 16MA), 			/* TSP_SCL*/

	/* NFC */
	PIN_CFG(GPIO25, GPIO25, 0, ON, OFF, 0, 0, 8MA),				// NFC_EN
	PIN_CFG(CAMCS0, ANA_SYS_REQ1, 0, ON, OFF, 0, 0, 8MA),		// NFC_CLK_REQ
	PIN_CFG(SSPDI, GPIO88, 0, OFF, ON, 0, 0, 8MA),				// NFC_WAKE
	PIN_CFG(LCDCS0, GPIO38, 0, OFF, ON, 0, 0, 8MA), 			// NFC_SCL
	PIN_CFG(SIMDET, GPIO56, 0, OFF, ON, 0, 0, 8MA), 			// NFC_SDA 
	PIN_CFG(GPS_CALREQ, GPIO99, 0, OFF, ON, 0, 0, 8MA), 		// NFC_WAKE 

	/* Headset */
	PIN_CFG(STAT1, GPIO31, 0, OFF, ON, 0, 0, 4MA),				//	EAR_3.5_DETECT	

	/* Sensor(Acceleromter,Magnetic,Proximity) */
	PIN_CFG(GPIO15, GPIO15, 0, OFF, OFF, 0, 0, 8MA), 			/* SENSOR_SDA*/
	PIN_CFG(GPIO32, GPIO32, 0, OFF, OFF, 0, 0, 8MA), 			/* SENSOR_SCL*/	
	PIN_CFG(ICUSBDM, GPIO122, 0, OFF, ON, 0, 0, 8MA), 			/* PROXI_INT*/

	PIN_CFG(MDMGPIO01, GPIO113, 0, OFF, ON, 0, 0, 8MA), 		//	S_SENSOR_INT
	PIN_CFG(SPI0FSS, SSP0SYN, 0, ON, OFF, 0, 0, 8MA), 			//	S_SENSOR_FSS
	PIN_CFG(SPI0CLK, SSP0CK, 0, ON, OFF, 0, 0, 8MA), 			//	S_SENSOR_CLK
	PIN_CFG(SPI0TXD, SSP0DO, 0, ON, OFF, 0, 0, 8MA), 			//	S_SENSOR_TXD
	PIN_CFG(SPI0RXD, SSP0DI, 0, ON, OFF, 0, 0, 8MA), 			//	S_SENSOR_RXD

	/* SIMCARD */
	PIN_CFG(SIMRST, SIMRST, 0, OFF, ON, 0, 0, 8MA), 			//	SIM_RST
	PIN_CFG(SIMDAT, SIMDAT, 0, OFF, ON, 0, 0, 8MA), 			//	SIM_IO
	PIN_CFG(SPI0CLK, SIMCLK, 0, OFF, ON, 0, 0, 8MA), 			//	SIM_CLK
};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	void __iomem *base = g_chip_pin_desc.base;

	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

        // Work around to SIMDAT/SIM2DAT. Once we move to DTS, we don't need this
        // We cannot use PIN_CFG because bit 12 is used (previously RESERVED)
        writel(0x00001033, base + PADCTRLREG_SIMDAT_OFFSET);

	return 0;
}

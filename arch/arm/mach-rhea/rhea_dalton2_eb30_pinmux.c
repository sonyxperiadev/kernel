/*****************************************************************************/
/*                                                                           */
/*  Copyright 2011  Broadcom Corporation                                     */
/*                                                                           */
/*     Unless you and Broadcom execute a separate written software license   */
/*     agreement governing use of this software, this software is licensed   */
/*     to you under the terms of the GNU                                     */
/*     General Public License version 2 (the GPL), available at              */
/*                                                                           */
/*          http://www.broadcom.com/licenses/GPLv2.php                       */
/*                                                                           */
/*     with the following added to such license:                             */
/*                                                                           */
/*     As a special exception, the copyright holders of this software give   */
/*     you permission to link this software with independent modules, and to */
/*     copy and distribute the resulting executable under terms of your      */
/*     choice, provided that you also meet, for each linked independent      */
/*     module, the terms and conditions of the license of that module.       */
/*     An independent module is a module which is not derived from this      */
/*     software.  The special exception does not apply to any modifications  */
/*     of the software.                                                      */
/*                                                                           */
/*     Notwithstanding the above, under no circumstances may you combine     */
/*     this software in any way with any other Broadcom software provided    */
/*     under a license other than the GPL, without Broadcom's express prior  */
/*     written consent.                                                      */
/*                                                                           */
/*****************************************************************************/
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

	/* eMMC */
	PIN_CFG(MMC0CK, MMC0CK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC0CMD, MMC0CMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0RST, MMC0RST, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT7, MMC0DAT7, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT6, MMC0DAT6, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT5, MMC0DAT5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT4, MMC0DAT4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT3, MMC0DAT3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT2, MMC0DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT1, MMC0DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC0DAT0, MMC0DAT0, 0, OFF, ON, 0, 0, 8MA),

	/* Micro SD */
	PIN_CFG(SDCK, SDCK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SDCMD, SDCMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT3, SDDAT3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT2, SDDAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT1, SDDAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SDDAT0, SDDAT0, 0, OFF, ON, 0, 0, 8MA),

	PIN_CFG(ICUSBDP, GPIO121, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC1DAT4, GPIO74, 0, OFF, ON, 0, 0, 8MA),

	PIN_CFG(GPIO00, KEY_R0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO01, KEY_R1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO02, KEY_R2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO03, KEY_R3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO08, KEY_C0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO09, KEY_C1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO10, KEY_C2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO11, KEY_C3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO12, KEY_C4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO13, KEY_C5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO14, KEY_C6, 0, OFF, ON, 0, 0, 8MA),

	/* SSP0 */
	PIN_CFG(SPI0FSS, SSP0SYN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SPI0CLK,  SSP0CK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SPI0TXD,  SSP0DO, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SPI0RXD,  SSP0DI, 0, OFF,  ON, 0, 0, 8MA),


	/* BBRF_TESTPORT15 */
	PIN_CFG(GPIO15, GPIO15, 0, OFF, OFF, 0, 0, 8MA),
	
	/* WLAN */
	/* PIN_CFG(GPIO15, SSP2SYN, 0, OFF, OFF, 0, 0, 8MA), */
	PIN_CFG(MMC1DAT7, SSP2CK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC1DAT6, SSP2DO, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC1DAT5, SSP2DI, 0, OFF,  ON, 0, 0, 8MA),

	/* SSP4 - I2S */
	PIN_CFG(GPIO94, SSP1SYN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(GPIO32,  SSP1CK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(DCLK4,  SSP1DO, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(DCLKREQ4, SSP1DI, 0, OFF,  ON, 0, 0, 8MA),

	/* LCD */
	PIN_CFG(LCDTE, LCDTE, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(LCDRES, GPIO41, 0, OFF, ON, 0, 0, 8MA), /* default */

	/* STM trace - PTI */
	PIN_CFG(TRACECLK, PTI_CLK, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT07, RXD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT00, PTI_DAT0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT01, PTI_DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT02, PTI_DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(TRACEDT03, PTI_DAT3, 0, OFF, ON, 0, 0, 8MA),


		
	/* Camera */
	PIN_CFG(CAMCS0, ANA_SYS_REQ1, 0, ON, OFF, 0, 0, 8MA),

	/* SMI */
	PIN_CFG(LCDSCL, LCDCD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(LCDSDA, LCDD0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO18, LCDCS1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO19, LCDWE, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO20, LCDRE, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO21, LCDD7, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO22, LCDD6, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO23, LCDD5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO24, LCDD4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO25, LCDD3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO26, LCDD2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO27, LCDD1, 0, OFF, ON, 0, 0, 8MA),

	/* SIM2LDO_EN through GPIO99 (TPS728XX) */
	PIN_CFG(GPS_CALREQ, GPIO99, 0, OFF, ON, 0, 0, 8MA),
	
	/* Bluetooth related GPIOS */
	PIN_CFG(MMC1CK, GPIO68, 0, ON, OFF, 0, 0, 8MA),
	PIN_CFG(DCLKREQ1, GPIO111, 0, OFF, ON, 0, 1, 8MA),

};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	for (i = 0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

	return 0;
}

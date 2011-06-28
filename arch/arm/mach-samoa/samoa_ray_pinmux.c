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

#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
#include <mach/io_map.h>
#endif

static struct __init pin_config board_pin_config[] = {

#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
	/* BSC1 CLK This a hack for rhearay*/
	PIN_BSC_CFG(BSC1_DAT, BSC1_DAT, 0x20),
	/* BSC1 DAT*/
	PIN_BSC_CFG(BSC2_CLK, BSC2_CLK, 0x20),

	/* BSC2 CLK This a hack for rhearay*/
	PIN_BSC_CFG(GPIO22, LCD_SCL, 0x20),
	/* BSC2 DAT*/
	PIN_BSC_CFG(GPIO23, DMIC0CLK, 0x20),
#else
	/* BSC1 & BSC2 */
	PIN_BSC_CFG(BSC1_CLK, BSC1_CLK, 0x20),
	PIN_BSC_CFG(BSC1_DAT, BSC1_DAT, 0x20),
	PIN_BSC_CFG(BSC2_CLK, BSC2_CLK, 0x20),
	PIN_BSC_CFG(BSC2_DAT, BSC2_DAT, 0x20),

	/* UARTA */
	PIN_CFG(UARTA_CTS_N, UARTA_CTS_N, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(UARTA_RTS_N, UARTA_RTS_N, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(UARTA_RX, UARTA_RX, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(UARTA_TX, UARTA_TX, 0, OFF, ON, 0, 0, 8MA),

	/* UARTB */
	PIN_CFG(GPIO20, UB2TX, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO21, UB2RX, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO23, UB2CTSN, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO24, UB2RTSN, 0, OFF, ON, 0, 0, 8MA),

	/* eMMC */
	PIN_CFG(MMC_CLK, MMC1_CLK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(MMC_CMD, MMC1_CMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_RST, MMC1_RST, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT7, MMC1_DAT7, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT6, MMC1_DAT6, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT5, MMC1_DAT5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT4, MMC1_DAT4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT3, MMC1_DAT3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT2, MMC1_DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT1, MMC1_DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(MMC_DAT0, MMC1_DAT0, 0, OFF, ON, 0, 0, 8MA),

	/* Micro SD */
	PIN_CFG(SD_CLK, MMC0_CLK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SD_CMD, MMC0_CMD, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SD_DAT3, MMC0_DAT3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SD_DAT2, MMC0_DAT2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SD_DAT1, MMC0_DAT1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(SD_DAT0, MMC0_DAT0, 0, OFF, ON, 0, 0, 8MA),

#ifdef CONFIG_KEYBOARD_TC3589X
	/*	Pinmux for GPIOs */
	PIN_CFG(GPIO00, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO01, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO02, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO03, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO04, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO05, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO06, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO07, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO08, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO09, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO10, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO11, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO12, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO13, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO14, GPIO, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO15, GPIO, 0, OFF, ON, 0, 0, 8MA),
#else
	/*	Pinmux for keypad */
	PIN_CFG(GPIO00, KEY_R0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO01, KEY_R1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO02, KEY_R2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO03, KEY_C0, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO04, KEY_R4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO05, KEY_R5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO06, KEY_R6, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO07, KEY_R3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO08, KEY_R7, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO09, KEY_C1, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO10, KEY_C2, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO11, KEY_C3, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO12, KEY_C4, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO13, KEY_C5, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO14, KEY_C6, 0, OFF, ON, 0, 0, 8MA),
	PIN_CFG(GPIO15, KEY_C7, 0, OFF, ON, 0, 0, 8MA),
#endif

#endif /* CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY */
};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

#ifdef CONFIG_MACH_SAMOA_RAY_TEST_ON_RHEA_RAY
	{
	// Pre-bringup Samoa pinmux does not initialize GPIO0-15.
	// So hack it in here for Rhearay.
	// Note: Rhea GPIO0-15 pad ctrl registers at 0x3c-7C.
	//
	volatile unsigned int *pc;
	pc = (unsigned int *) (KONA_PAD_CTRL_VA + 0x3C); // first GPIO

	for (i=0x3C; i<0x7C; i+=4,pc++) {
		*pc = 0x123;  // keypad function selected
	}
	}
#endif

	return 0;
}

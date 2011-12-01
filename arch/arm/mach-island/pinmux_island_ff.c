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
#include <mach/pinmux.h>

static struct __init pin_config board_pin_config[] = {
	/* STM trace - PTI */
	PIN_CFG(TRACECLK, PTI_CLK, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(TRACEDT07, UARTB2_URXD, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(TRACEDT00, PTI_DAT0, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(TRACEDT01, PTI_DAT1, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(TRACEDT02, PTI_DAT2, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(TRACEDT03, PTI_DAT3, 0, OFF, OFF, 0, 0, 16MA),

	PIN_CFG(ULPI0_DATA_3, UARTB3_URTSN, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(ULPI0_DATA_4, UARTB3_UCTSN, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(ULPI0_DATA_5, UARTB3_URXD, 0, OFF, OFF, 0, 0, 16MA),
	PIN_CFG(ULPI0_DATA_6, UARTB3_UTXD, 0, OFF, OFF, 0, 0, 16MA),

	/* PMU INT */
	PIN_CFG(PMU_INT, GPIO, 0, OFF, ON, 0, 0, 8MA),

	/* Touchscreen uses this for reset pin with GPIO selected */
	PIN_CFG(NAND_CEN_1, GPIO, 0, OFF, ON, 0, 0, 8MA),

	/* SIM DATA needs a pull up */
	PIN_CFG(SIM_DATA, SIM_DATA, 0, OFF, ON, 0, 0, 12MA),
	PIN_CFG(SIM_RESETN, SIM_RESETN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SIM_CLK, SIM_CLK, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SIM_DET, SIM_DET, 0, OFF, ON, 0, 0, 8MA),

	/* HDMI Hotplug Detect */
	PIN_CFG(NORFLSH_ADDR_19, GPIO, 0, OFF, OFF, 0, 0, 8MA),
        
	/* VC HDMI I2C */
	PIN_BSC_CFG(HDMI_SCL, HDMI_SCL, 0x08),
	PIN_BSC_CFG(HDMI_SDA, HDMI_SDA, 0x08),

	/* WLAN */
	PIN_CFG(NORFLSH_WE_N,  GPIO, 0, OFF, ON,  0, 0, 8MA),      /* GPIO 179 - WLAN RESET */
	PIN_CFG(NORFLSH_CE1_N, GPIO, 0, OFF, OFF, 0, 1, 8MA),     /* GPIO 178 - WLAN HOST WAKE */
	PIN_CFG(NORFLSH_CE0_N, GPIO, 0, OFF, OFF, 0, 1, 8MA),     /* GPIO 177 - WLAN CLK REQ */
	/* WiFi SDIO */
	PIN_CFG(NORFLSH_AD_03, SDIO1_DATA_3, 0, OFF, ON, 0, 0, 2MA),
	PIN_CFG(NORFLSH_AD_02, SDIO1_CLK,    0, OFF, ON, 0, 0, 2MA),
	PIN_CFG(NORFLSH_AD_01, SDIO1_DATA_0, 0, OFF, ON, 0, 0, 2MA),
	PIN_CFG(NORFLSH_AD_00, SDIO1_CMD,    0, OFF, ON, 0, 0, 2MA),
	PIN_CFG(NORFLSH_RDY,   SDIO1_DATA_1, 0, OFF, ON, 0, 0, 2MA),
	PIN_CFG(NORFLSH_CLK_N, SDIO1_DATA_2, 0, OFF, ON, 0, 0, 2MA),

	/*Required for Bluetooth to work correctly*/
	PIN_CFG(SSP1_TXD,SSP1_TXD, 0, OFF, OFF, 0, 0,8MA),
	PIN_CFG(ULPI0_DATA_4, UARTB3_UCTSN, 0, OFF, ON, 0, 0,8MA),


};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

	return 0;
}

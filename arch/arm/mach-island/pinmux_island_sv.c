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
	/* BSC1 */
        PIN_BSC_CFG(VC_CAM1_SCL, VC_CAM1_SCL, 0x08),
        PIN_BSC_CFG(VC_CAM1_SDA, VC_CAM1_SDA, 0x08),

        /* BSC2 */
        PIN_BSC_CFG(BSC2_SCL, BSC2_SCL, 0x08),
        PIN_BSC_CFG(BSC2_SDA, BSC2_SDA, 0x08),

        /* PMU BSC */
        PIN_BSC_CFG(PMU_SCL, PMU_SCL, 0x08),
        PIN_BSC_CFG(PMU_SDA, PMU_SDA, 0x08),

	/* SD/MMC */
        PIN_CFG(NAND_AD_5,    SDIO3_DATA_2, 0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_4,    SDIO3_DATA_1, 0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_3,    SDIO3_CMD,    0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_2,    SDIO3_DATA_3, 0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_1,    SDIO3_CLK,    0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_0,    SDIO3_DATA_0, 0, OFF, OFF, 0, 0, 12MA),

	/* eMMC */
	PIN_CFG(SDIO2_DATA_3, SDIO2_DATA_3, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SDIO2_DATA_2, SDIO2_DATA_2, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SDIO2_DATA_1, SDIO2_DATA_1, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SDIO2_DATA_0, SDIO2_DATA_0, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SDIO2_CMD, SDIO2_CMD, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(SDIO2_CLK, SDIO2_CLK, 0, OFF, OFF, 0, 0, 8MA),

	PIN_CFG(SDIO3_DATA_3, SDIO2_DATA_7, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(SDIO3_DATA_2, SDIO2_DATA_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(SDIO3_DATA_1, SDIO2_DATA_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(SDIO3_DATA_0, SDIO2_DATA_4, 0, OFF, OFF, 0, 0, 8MA),
};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

	return 0;
}

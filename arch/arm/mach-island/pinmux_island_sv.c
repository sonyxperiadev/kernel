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
	/* VC camera */
	PIN_CFG(NORFLSH_CE1_N,   VC_GPIO_0, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_ADDR_19, VC_GPIO_1, 0, OFF, OFF, 0, 0, 8MA),

        /* BSC2 */
        PIN_BSC_CFG(BSC2_SCL, BSC2_SCL, 0x08),
        PIN_BSC_CFG(BSC2_SDA, BSC2_SDA, 0x08),

        /* PMU BSC */
        PIN_BSC_CFG(PMU_SCL, PMU_SCL, 0x08),
        PIN_BSC_CFG(PMU_SDA, PMU_SDA, 0x08),

        /* PWM */
        PIN_CFG(SIM_DATA,          PWM_O_2,     0, OFF, OFF, 0, 0, 8MA),

        /* Keypad */
        PIN_CFG(NORFLSH_AD_14,     KP_ROW_OP_0, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_AD_13,     KP_ROW_OP_1, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_AD_12,     KP_ROW_OP_2, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_AD_11,     KP_ROW_OP_3, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_AD_10,     KP_ROW_OP_4, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_AD_09,     KP_ROW_OP_5, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NORFLSH_AD_08,     KP_ROW_OP_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NORFLSH_ADLAT_EN,  KP_COL_IP_1, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NORFLSH_AADLAT_EN, KP_COL_IP_2, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NORFLSH_ADDR_20,   KP_COL_IP_3, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NORFLSH_ADDR_21,   KP_COL_IP_4, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NORFLSH_ADDR_22,   KP_COL_IP_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NORFLSH_ADDR_23,   KP_COL_IP_6, 0, OFF, OFF, 0, 0, 8MA),

	/* WiFi SDIO */
        PIN_CFG(NORFLSH_AD_03, SDIO1_DATA_3, 0, OFF, ON, 0, 0, 8MA),
        PIN_CFG(NORFLSH_AD_02, SDIO1_CLK,    0, OFF, ON, 0, 0, 8MA),
        PIN_CFG(NORFLSH_AD_01, SDIO1_DATA_0, 0, OFF, ON, 0, 0, 8MA),
        PIN_CFG(NORFLSH_AD_00, SDIO1_CMD,    0, OFF, ON, 0, 0, 8MA),
        PIN_CFG(NORFLSH_RDY,   SDIO1_DATA_1, 0, OFF, ON, 0, 0, 8MA),
        PIN_CFG(NORFLSH_CLK_N, SDIO1_DATA_2, 0, OFF, ON, 0, 0, 8MA),

#ifndef CONFIG_MTD_BCMNAND
	/* VC4 JTAG */
	PIN_CFG(NAND_RDY_1, VC_TRSTB, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_CLE,   VC_TCK,   0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NAND_ALE,   VC_TDI,   0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NAND_OEN,   VC_TDO,   0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(NAND_WEN,   VC_TMS,   0, OFF, OFF, 0, 0, 8MA),

	/* SD/MMC */
        PIN_CFG(NAND_AD_5,    SDIO3_DATA_2, 0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_4,    SDIO3_DATA_1, 0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_3,    SDIO3_CMD,    0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_2,    SDIO3_DATA_3, 0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_1,    SDIO3_CLK,    0, OFF, OFF, 0, 0, 12MA),
        PIN_CFG(NAND_AD_0,    SDIO3_DATA_0, 0, OFF, OFF, 0, 0, 12MA),
#else
	PIN_CFG(NAND_WP,    NAND_WP,    0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_CEN_0, NAND_CEN_0, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_CEN_1, NAND_CEN_1, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_RDY_0, NAND_RDY_0, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_RDY_1, NAND_RDY_1, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_CLE,   NAND_CLE,   0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_ALE,   NAND_ALE,   0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_OEN,   NAND_OEN,   0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_WEN,   NAND_WEN,   0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_7,  NAND_AD_7,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_6,  NAND_AD_6,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_5,  NAND_AD_5,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_4,  NAND_AD_4,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_3,  NAND_AD_3,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_2,  NAND_AD_2,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_1,  NAND_AD_1,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(NAND_AD_0,  NAND_AD_0,  0, OFF, OFF, 0, 0, 8MA),
#endif
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

	/* USB Host (ULPI) */
        PIN_CFG(ULPI0_CLOCK,  ULPI0_CLOCK,  0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_0, ULPI0_DATA_0, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_1, ULPI0_DATA_1, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_2, ULPI0_DATA_2, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_3, ULPI0_DATA_3, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_4, ULPI0_DATA_4, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_5, ULPI0_DATA_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_6, ULPI0_DATA_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DATA_7, ULPI0_DATA_7, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_DIR,    ULPI0_DIR,    0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_NXT,    ULPI0_NXT,    0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI0_STP,    ULPI0_STP,    0, OFF, OFF, 0, 1, 8MA),

        /* USB Host (ULPI) */
        PIN_CFG(ULPI1_CLOCK,  ULPI1_CLOCK,  0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_0, ULPI1_DATA_0, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_1, ULPI1_DATA_1, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_2, ULPI1_DATA_2, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_3, ULPI1_DATA_3, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_4, ULPI1_DATA_4, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_5, ULPI1_DATA_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_6, ULPI1_DATA_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DATA_7, ULPI1_DATA_7, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_DIR,    ULPI1_DIR,    0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_NXT,    ULPI1_NXT,    0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(ULPI1_STP,    ULPI1_STP,    0, OFF, OFF, 0, 1, 8MA),

	/* VC CAM1 I2C */
	PIN_BSC_CFG(VC_CAM1_SCL, VC_CAM1_SCL, 0x08),
	PIN_BSC_CFG(VC_CAM1_SDA, VC_CAM1_SDA, 0x08),

	/* VC HDMI I2C */
	PIN_BSC_CFG(HDMI_SCL, HDMI_SCL, 0x08),
	PIN_BSC_CFG(HDMI_SDA, HDMI_SDA, 0x08),

	/* LCD */
	PIN_CFG(LCD_R_7, LCD_R_7, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_6, LCD_R_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_5, LCD_R_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_4, LCD_R_4, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_3, LCD_R_3, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_2, LCD_R_2, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_1, LCD_R_1, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_R_0, LCD_R_0, 0, OFF, OFF, 0, 0, 8MA),

        PIN_CFG(LCD_G_7, LCD_G_7, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_6, LCD_G_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_5, LCD_G_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_4, LCD_G_4, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_3, LCD_G_3, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_2, LCD_G_2, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_1, LCD_G_1, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_G_0, LCD_G_0, 0, OFF, OFF, 0, 0, 8MA),

        PIN_CFG(LCD_B_7, LCD_B_7, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_6, LCD_B_6, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_5, LCD_B_5, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_4, LCD_B_4, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_3, LCD_B_3, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_2, LCD_B_2, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_1, LCD_B_1, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_B_0, LCD_B_0, 0, OFF, OFF, 0, 0, 8MA),

        PIN_CFG(LCD_HSYNC, LCD_HSYNC, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_VSYNC, LCD_VSYNC, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_OE,    LCD_OE,    0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(LCD_PCLK,  LCD_PCLK,  0, OFF, OFF, 0, 0, 8MA),

   /* Digital Mics*/
   PIN_CFG( DIGMIC1_CLK, DIGMIC1_CLK, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( DIGMIC1_DQ,  DIGMIC1_DQ,  0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( DIGMIC2_CLK, DIGMIC2_CLK, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( DIGMIC2_DQ,  DIGMIC2_DQ,  0, OFF, OFF, 0, 0, 8MA ),

	/* VC GP clock */
	PIN_CFG(GPIO_3, VC_GPCLK_0, 0, OFF, OFF, 0, 0, 8MA),
        PIN_CFG(GPIO_2, VC_GPCLK_1, 0, OFF, OFF, 0, 0, 8MA),

   /* SSP0 */
   PIN_CFG( SSP0_FS,  SSP0_FS,  0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( SSP0_CLK, SSP0_CLK, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( SSP0_RXD, SSP0_RXD, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( SSP0_TXD, SSP0_TXD, 0, OFF, OFF, 0, 0, 8MA ),

   /* UARTB3 */
   PIN_CFG( SSP1_FS,  UARTB3_URXD,  0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( SSP1_CLK, UARTB3_UTXD,  0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( SSP1_RXD, UARTB3_URTSN, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( SSP1_TXD, UARTB3_UCTSN, 0, OFF, OFF, 0, 0, 8MA ),

	/* UARTB4 */
	PIN_CFG(SSP2_FS_0,  UARTB4_URXD,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SSP2_CLK,   UARTB4_UTXD,  0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SSP2_RXD_0, UARTB4_URTSN, 0, OFF, OFF, 0, 0, 8MA),
	PIN_CFG(SSP2_TXD_0, UARTB4_UCTSN, 0, OFF, OFF, 0, 0, 8MA),

   /* AUDIOH */
   PIN_CFG( UARTB4_UTXD,   GPIO, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( UARTB4_URXD,   GPIO, 0, OFF, OFF, 0, 0, 8MA ),

   /* BT GPIO */
   PIN_CFG( NORFLSH_CE0_N, GPIO, 0, OFF, OFF, 0, 0, 8MA ),
   PIN_CFG( NORFLSH_AD_05, GPIO, 0, OFF, OFF, 0, 0, 8MA ),

};

/* board level init */
int __init pinmux_board_init(void)
{
	int i;
	for (i=0; i<ARRAY_SIZE(board_pin_config); i++)
		pinmux_set_pin_config(&board_pin_config[i]);

	return 0;
}

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
 * Define ball name, to match what's generated in RDB
 */
enum PIN_NAME {
	/* GPIO 178 */ PN_NORFLSH_CE1_N,
	/* GPIO 175 */ PN_NORFLSH_ADDR_23,
	/* GPIO 174 */ PN_NORFLSH_ADDR_22,
	/* GPIO 173 */ PN_NORFLSH_ADDR_21,
	/* GPIO 172 */ PN_NORFLSH_ADDR_20,
	/* GPIO 171 */ PN_NORFLSH_ADDR_19,
	/* GPIO 167 */ PN_NORFLSH_AADLAT_EN,
	/* GPIO 166 */ PN_NORFLSH_ADLAT_EN,
        /* GPIO 164 */ PN_NORFLSH_AD_14,
	/* GPIO 163 */ PN_NORFLSH_AD_13,
	/* GPIO 162 */ PN_NORFLSH_AD_12,
	/* GPIO 161 */ PN_NORFLSH_AD_11,
	/* GPIO 160 */ PN_NORFLSH_AD_10,
	/* GPIO 159 */ PN_NORFLSH_AD_09,
	/* GPIO 158 */ PN_NORFLSH_AD_08,
	/* GPIO 157 */ PN_NORFLSH_AD_07,
        /* GPIO 154 */ PN_NORFLSH_AD_04,
        /* GPIO 153 */ PN_NORFLSH_AD_03,
	/* GPIO 152 */ PN_NORFLSH_AD_02,
	/* GPIO 151 */ PN_NORFLSH_AD_01,
	/* GPIO 150 */ PN_NORFLSH_AD_00,
	/* GPIO 149 */ PN_NORFLSH_RDY,
	/* GPIO 148 */ PN_NORFLSH_CLK_N,

	/* GPIO 145 */ PN_SIM_DATA,

        /* GPIO 139 */ PN_ULPI0_CLOCK,
        /* GPIO 138 */ PN_ULPI0_DATA_0,
        /* GPIO 137 */ PN_ULPI0_DATA_1,
        /* GPIO 136 */ PN_ULPI0_DATA_2,
        /* GPIO 135 */ PN_ULPI0_DATA_3,
        /* GPIO 134 */ PN_ULPI0_DATA_4,
        /* GPIO 133 */ PN_ULPI0_DATA_5,
        /* GPIO 132 */ PN_ULPI0_DATA_6,
        /* GPIO 131 */ PN_ULPI0_DATA_7,
        /* GPIO 130 */ PN_ULPI0_DIR,
        /* GPIO 129 */ PN_ULPI0_NXT,
        /* GPIO 128 */ PN_ULPI0_STP,

	/* GPIO 127 */ PN_ULPI1_CLOCK,  
        /* GPIO 126 */ PN_ULPI1_DATA_0, 
        /* GPIO 125 */ PN_ULPI1_DATA_1, 
        /* GPIO 124 */ PN_ULPI1_DATA_2, 
        /* GPIO 123 */ PN_ULPI1_DATA_3, 
        /* GPIO 122 */ PN_ULPI1_DATA_4, 
        /* GPIO 121 */ PN_ULPI1_DATA_5, 
        /* GPIO 120 */ PN_ULPI1_DATA_6, 
        /* GPIO 119 */ PN_ULPI1_DATA_7, 
        /* GPIO 118 */ PN_ULPI1_DIR,    
        /* GPIO 117 */ PN_ULPI1_NXT,    
        /* GPIO 116 */ PN_ULPI1_STP,

	/* GPIO 111 */ PN_NAND_RDY_1,
	/* GPIO 110 */ PN_NAND_CLE,
	/* GPIO 109 */ PN_NAND_ALE,
	/* GPIO 108 */ PN_NAND_OEN,
	/* GPIO 107 */ PN_NAND_WEN,

	/* GPIO 104 */ PN_NAND_AD_5,
	/* GPIO 103 */ PN_NAND_AD_4,
	/* GPIO 102 */ PN_NAND_AD_3,
	/* GPIO 101 */ PN_NAND_AD_2,
	/* GPIO 100 */ PN_NAND_AD_1,
	/* GPIO 99 */  PN_NAND_AD_0,

	/* GPIO 96 */ PN_SDIO2_DATA_3,
	/* GPIO 95 */ PN_SDIO2_DATA_2,
	/* GPIO 94 */ PN_SDIO2_DATA_1,
	/* GPIO 93 */ PN_SDIO2_DATA_0,
	/* GPIO 92 */ PN_SDIO2_CMD,
        /* GPIO 91 */ PN_SDIO2_CLK,

        /* GPIO 90 */ PN_SDIO3_DATA_3,
        /* GPIO 89 */ PN_SDIO3_DATA_2,
        /* GPIO 88 */ PN_SDIO3_DATA_1,
        /* GPIO 87 */ PN_SDIO3_DATA_0,
        /* GPIO 86 */ PN_SDIO3_CMD,
        /* GPIO 85 */ PN_SDIO3_CLK,

	/* GPIO 61 */ PN_PMU_SCL,
        /* GPIO 60 */ PN_PMU_SDA,

        /* GPIO 59 */ PN_BSC2_SCL,
        /* GPIO 58 */ PN_BSC2_SDA,

        /* GPIO 57 */ PN_VC_CAM1_SCL,
        /* GPIO 56 */ PN_VC_CAM1_SDA,

	/* GPIO 55 */ PN_HDMI_SCL,
        /* GPIO 54 */ PN_HDMI_SDA,

	/* GPIO 47 */ PN_LCD_R_7,
        /* GPIO 46 */ PN_LCD_R_6,
        /* GPIO 45 */ PN_LCD_R_5,
        /* GPIO 44 */ PN_LCD_R_4,
        /* GPIO 43 */ PN_LCD_R_3,
        /* GPIO 42 */ PN_LCD_R_2,
        /* GPIO 41 */ PN_LCD_R_1,
        /* GPIO 40 */ PN_LCD_R_0,

        /* GPIO 39 */ PN_LCD_G_7,
        /* GPIO 38 */ PN_LCD_G_6,
        /* GPIO 37 */ PN_LCD_G_5,
        /* GPIO 36 */ PN_LCD_G_4,
        /* GPIO 35 */ PN_LCD_G_3,
        /* GPIO 34 */ PN_LCD_G_2,
        /* GPIO 33 */ PN_LCD_G_1,
        /* GPIO 32 */ PN_LCD_G_0,

        /* GPIO 31 */ PN_LCD_B_7,
        /* GPIO 30 */ PN_LCD_B_6,
        /* GPIO 29 */ PN_LCD_B_5,
        /* GPIO 28 */ PN_LCD_B_4,
        /* GPIO 27 */ PN_LCD_B_3,
        /* GPIO 26 */ PN_LCD_B_2,
        /* GPIO 25 */ PN_LCD_B_1,
        /* GPIO 24 */ PN_LCD_B_0,

        /* GPIO 23 */ PN_LCD_HSYNC, 
        /* GPIO 22 */ PN_LCD_VSYNC,
        /* GPIO 21 */ PN_LCD_OE,  
        /* GPIO 20 */ PN_LCD_PCLK,

	/* GPIO 3 */  PN_GPIO_3,  
        /* GPIO 2 */  PN_GPIO_2,

	PN_MAX
};

/* define function name */
enum PIN_FUNC {
	PF_RESERVED	=	0,
	PF_GPIO,

	PF_SIM_DATA,
	
	PF_PWM_O_0,
        PF_PWM_O_2,

	PF_VC_GPIO_0,
	PF_VC_GPIO_1,
        PF_VC_GPIO_7,

	PF_NORFLSH_CE1_N,
	PF_NORFLSH_ADDR_23,
        PF_NORFLSH_ADDR_22,
        PF_NORFLSH_ADDR_21,
        PF_NORFLSH_ADDR_20,
	PF_NORFLSH_ADDR_19,
	PF_NORFLSH_AADLAT_EN,
	PF_NORFLSH_ADLAT_EN,
        PF_NORFLSH_AD_14,
	PF_NORFLSH_AD_13,
	PF_NORFLSH_AD_12,
	PF_NORFLSH_AD_11,
	PF_NORFLSH_AD_10,
	PF_NORFLSH_AD_09,
	PF_NORFLSH_AD_08,
	PF_NORFLSH_AD_07,
	PF_NORFLSH_AD_04,
        PF_NORFLSH_AD_03,
        PF_NORFLSH_AD_02,
        PF_NORFLSH_AD_01,
        PF_NORFLSH_AD_00,
        PF_NORFLSH_RDY,
        PF_NORFLSH_CLK_N,

        PF_KP_ROW_OP_0,
	PF_KP_ROW_OP_1,
	PF_KP_ROW_OP_2,
	PF_KP_ROW_OP_3,
	PF_KP_ROW_OP_4,
	PF_KP_ROW_OP_5,
	PF_KP_ROW_OP_6,

        PF_KP_COL_IP_0,
        PF_KP_COL_IP_1,
        PF_KP_COL_IP_2,
        PF_KP_COL_IP_3,
        PF_KP_COL_IP_4,
        PF_KP_COL_IP_5,
        PF_KP_COL_IP_6,
	PF_KP_COL_IP_7,

	PF_ULPI0_CLOCK,
        PF_ULPI0_DATA_0,
        PF_ULPI0_DATA_1,
        PF_ULPI0_DATA_2,
        PF_ULPI0_DATA_3,
        PF_ULPI0_DATA_4,
        PF_ULPI0_DATA_5,
        PF_ULPI0_DATA_6,
        PF_ULPI0_DATA_7,
        PF_ULPI0_DIR,
        PF_ULPI0_NXT,
        PF_ULPI0_STP,

        PF_ULPI1_CLOCK,
        PF_ULPI1_DATA_0,
        PF_ULPI1_DATA_1,
        PF_ULPI1_DATA_2,
        PF_ULPI1_DATA_3,
        PF_ULPI1_DATA_4,
        PF_ULPI1_DATA_5,
        PF_ULPI1_DATA_6,
        PF_ULPI1_DATA_7,
        PF_ULPI1_DIR,
        PF_ULPI1_NXT,
        PF_ULPI1_STP,

	PF_NAND_RDY_1,
	PF_NAND_CLE,
	PF_NAND_ALE,
	PF_NAND_OEN,
	PF_NAND_WEN,

	PF_VC_TRSTB,
        PF_VC_TCK,  
        PF_VC_TDI,  
        PF_VC_TDO,  
        PF_VC_TMS,  

	PF_NAND_AD_5,
        PF_NAND_AD_4,
        PF_NAND_AD_3,
        PF_NAND_AD_2,
        PF_NAND_AD_1,
        PF_NAND_AD_0,

	PF_SDIO1_DATA_3,
        PF_SDIO1_DATA_2,
        PF_SDIO1_DATA_1,
        PF_SDIO1_DATA_0,
        PF_SDIO1_CMD,
        PF_SDIO1_CLK,
	
	PF_SDIO2_DATA_7,
        PF_SDIO2_DATA_6,
        PF_SDIO2_DATA_5,
        PF_SDIO2_DATA_4,
        PF_SDIO2_DATA_3,
        PF_SDIO2_DATA_2,
        PF_SDIO2_DATA_1,
        PF_SDIO2_DATA_0,
        PF_SDIO2_CMD,
        PF_SDIO2_CLK,

        PF_SDIO3_DATA_3,
        PF_SDIO3_DATA_2,
        PF_SDIO3_DATA_1,
        PF_SDIO3_DATA_0,
        PF_SDIO3_CMD,
        PF_SDIO3_CLK,

	PF_PMU_SCL,
        PF_PMU_SDA,

	PF_BSC1_SCL,
        PF_BSC1_SDA,

        PF_BSC2_SCL,
        PF_BSC2_SDA,

        PF_VC_CAM1_SCL,
        PF_VC_CAM1_SDA,

	PF_HDMI_SCL,
        PF_HDMI_SDA,

	PF_LCD_R_7,
        PF_LCD_R_6,
        PF_LCD_R_5,
        PF_LCD_R_4,
        PF_LCD_R_3,
        PF_LCD_R_2,
        PF_LCD_R_1,
        PF_LCD_R_0,

        PF_LCD_G_7,
        PF_LCD_G_6,
        PF_LCD_G_5,
        PF_LCD_G_4,
        PF_LCD_G_3,
        PF_LCD_G_2,
        PF_LCD_G_1,
        PF_LCD_G_0,

        PF_LCD_B_7,
        PF_LCD_B_6,
        PF_LCD_B_5,
        PF_LCD_B_4,
        PF_LCD_B_3,
        PF_LCD_B_2,
        PF_LCD_B_1,
        PF_LCD_B_0,

        PF_LCD_HSYNC,
        PF_LCD_VSYNC,
        PF_LCD_OE,
        PF_LCD_PCLK,

	PF_GPIO_3,
        PF_GPIO_2,

	PF_VC_GPCLK_0,
	PF_VC_GPCLK_1,

	PF_MAX
};

/* each Pin has up to 6 functions */
#define	MAX_ALT_FUNC		6

#endif /* __CHIP_PINMUX_H__ */

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
*/

/* define ball name, generated from RDB */
enum PIN_NAME {
        /* Keypad */
        PN_NORFLSH_AD_14,     // KP_ROW_OP_0
	PN_NORFLSH_AD_13,     // KP_ROW_OP_1
	PN_NORFLSH_AD_12,     // KP_ROW_OP_2
	PN_NORFLSH_AD_11,     // KP_ROW_OP_3
	PN_NORFLSH_AD_10,     // KP_ROW_OP_4
	PN_NORFLSH_AD_09,     // KP_ROW_OP_5
	PN_NORFLSH_AD_08,     // KP_ROW_OP_6

        PN_NORFLSH_AD_04,     // KP_ROW_OP_0
        PN_NORFLSH_ADLAT_EN,  // KP_COL_IP_1
        PN_NORFLSH_AADLAT_EN, // KP_COL_IP_2
        PN_NORFLSH_ADDR_20,   // KP_COL_IP_3
        PN_NORFLSH_ADDR_21,   // KP_COL_IP_4
        PN_NORFLSH_ADDR_22,   // KP_COL_IP_5
        PN_NORFLSH_ADDR_23,   // KP_COL_IP_6
	PN_NORFLSH_AD_07,     // KP_COL_IP_7

        /* WiFi SDIO */
        PN_NORFLSH_AD_03,
	PN_NORFLSH_AD_02,
	PN_NORFLSH_AD_01,
	PN_NORFLSH_AD_00,
	PN_NORFLSH_RDY,
	PN_NORFLSH_CLK_N,

	PN_SIM_DATA,

        PN_ULPI0_CLOCK,
        PN_ULPI0_DATA_0,
        PN_ULPI0_DATA_1,
        PN_ULPI0_DATA_2,
        PN_ULPI0_DATA_3,
        PN_ULPI0_DATA_4,
        PN_ULPI0_DATA_5,
        PN_ULPI0_DATA_6,
        PN_ULPI0_DATA_7,
        PN_ULPI0_DIR,
        PN_ULPI0_NXT,
        PN_ULPI0_STP,

	PN_ULPI1_CLOCK,  
        PN_ULPI1_DATA_0, 
        PN_ULPI1_DATA_1, 
        PN_ULPI1_DATA_2, 
        PN_ULPI1_DATA_3, 
        PN_ULPI1_DATA_4, 
        PN_ULPI1_DATA_5, 
        PN_ULPI1_DATA_6, 
        PN_ULPI1_DATA_7, 
        PN_ULPI1_DIR,    
        PN_ULPI1_NXT,    
        PN_ULPI1_STP,    

	PN_NAND_AD_5,
	PN_NAND_AD_4,
	PN_NAND_AD_3,
	PN_NAND_AD_2,
	PN_NAND_AD_1,
	PN_NAND_AD_0,

	PN_SDIO2_DATA_3,
	PN_SDIO2_DATA_2,
	PN_SDIO2_DATA_1,
	PN_SDIO2_DATA_0,
	PN_SDIO2_CMD,
        PN_SDIO2_CLK,

        PN_SDIO3_DATA_3,
        PN_SDIO3_DATA_2,
        PN_SDIO3_DATA_1,
        PN_SDIO3_DATA_0,
        PN_SDIO3_CMD,
        PN_SDIO3_CLK,

	PN_PMU_SCL,
        PN_PMU_SDA,

        PN_BSC2_SCL,
        PN_BSC2_SDA,

        PN_VC_CAM1_SCL,
        PN_VC_CAM1_SDA,

	PN_MAX
};

/* define function name, order is not important */
enum PIN_FUNC {
	PF_RESERVED	=	0,
	PF_GPIO,

	PF_SIM_DATA,
        PF_PWM_O_2,
        PF_VC_GPIO_7,

        PF_NORFLSH_AD_14,
	PF_NORFLSH_AD_13,
	PF_NORFLSH_AD_12,
	PF_NORFLSH_AD_11,
	PF_NORFLSH_AD_10,
	PF_NORFLSH_AD_09,
	PF_NORFLSH_AD_08,

        PF_NORFLSH_AD_04,
        PF_NORFLSH_ADLAT_EN,
        PF_NORFLSH_AADLAT_EN,
        PF_NORFLSH_ADDR_20,
        PF_NORFLSH_ADDR_21,
        PF_NORFLSH_ADDR_22,
        PF_NORFLSH_ADDR_23,
	PF_NORFLSH_AD_07,

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

	PF_NORFLSH_AD_03,
        PF_NORFLSH_AD_02,
        PF_NORFLSH_AD_01,
        PF_NORFLSH_AD_00,
        PF_NORFLSH_RDY,
        PF_NORFLSH_CLK_N,

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

	PF_MAX
};

/* each Pin has up to 6 functions */
#define	MAX_ALT_FUNC		6

#endif /* __CHIP_PINMUX_H__ */

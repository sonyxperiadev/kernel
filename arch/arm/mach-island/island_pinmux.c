/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
#include <mach/rdb/brcm_rdb_sysmap.h>
#include <mach/rdb/brcm_rdb_chipreg.h>

#define	PIN_DESC(ball, alt1, alt2, alt3, alt4, alt5, alt6)	 	\
	[PN_##ball] = {							\
		.name		=	PN_##ball,			\
		.reg_offset	=	CHIPREG_##ball##_OFFSET,	\
		.f_tbl		=	{				\
			PF_##alt1, PF_##alt2, PF_##alt3, 		\
			PF_##alt4, PF_##alt5, PF_##alt6, 		\
		},							\
	}

/*
 * Island chip-level pin description table
 */
static const struct pin_desc pin_desc_tbl[PN_MAX] = {
	PIN_DESC(NORFLSH_AD_03, NORFLSH_AD_03, RESERVED, SDIO1_DATA_3, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_02, NORFLSH_AD_02, RESERVED, SDIO1_CLK, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_01, NORFLSH_AD_01, RESERVED, SDIO1_DATA_0, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_00, NORFLSH_AD_00, RESERVED, SDIO1_CMD, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_RDY, NORFLSH_RDY, RESERVED, SDIO1_DATA_1, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_CLK_N, NORFLSH_CLK_N, RESERVED, SDIO1_DATA_2, GPIO, RESERVED, RESERVED),

        PIN_DESC(SIM_DATA,          SIM_DATA,          PWM_O_2,  VC_GPIO_7,   GPIO, RESERVED, RESERVED),

        PIN_DESC(NORFLSH_AD_14,     NORFLSH_AD_14,     RESERVED, KP_ROW_OP_0, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_13,     NORFLSH_AD_13,     RESERVED, KP_ROW_OP_1, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_12,     NORFLSH_AD_12,     RESERVED, KP_ROW_OP_2, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_11,     NORFLSH_AD_11,     RESERVED, KP_ROW_OP_3, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_10,     NORFLSH_AD_10,     RESERVED, KP_ROW_OP_4, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_09,     NORFLSH_AD_09,     RESERVED, KP_ROW_OP_5, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_08,     NORFLSH_AD_08,     RESERVED, KP_ROW_OP_6, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_AD_04,     NORFLSH_AD_04,     RESERVED, KP_COL_IP_0, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_ADLAT_EN,  NORFLSH_ADLAT_EN,  RESERVED, KP_COL_IP_1, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_AADLAT_EN, NORFLSH_AADLAT_EN, RESERVED, KP_COL_IP_2, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_ADDR_20,   NORFLSH_ADDR_20,   RESERVED, KP_COL_IP_3, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_ADDR_21,   NORFLSH_ADDR_21,   RESERVED, KP_COL_IP_4, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_ADDR_22,   NORFLSH_ADDR_22,   RESERVED, KP_COL_IP_5, GPIO, RESERVED, RESERVED),
        PIN_DESC(NORFLSH_ADDR_23,   NORFLSH_ADDR_23,   RESERVED, KP_COL_IP_6, GPIO, RESERVED, RESERVED),
	PIN_DESC(NORFLSH_AD_07,     NORFLSH_AD_07,     RESERVED, KP_COL_IP_7, GPIO, RESERVED, RESERVED),

	PIN_DESC(ULPI0_CLOCK,  ULPI0_CLOCK,  RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_0, ULPI0_DATA_0, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_1, ULPI0_DATA_1, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_2, ULPI0_DATA_2, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_3, ULPI0_DATA_3, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_4, ULPI0_DATA_4, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_5, ULPI0_DATA_5, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DATA_6, ULPI0_DATA_6, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI0_DATA_7, ULPI0_DATA_7, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_DIR,    ULPI0_DIR,    RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI0_NXT,    ULPI0_NXT,    RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(ULPI0_STP,    ULPI0_STP,    RESERVED, RESERVED, GPIO, RESERVED, RESERVED),

	PIN_DESC(ULPI1_CLOCK,  ULPI1_CLOCK,  RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_0, ULPI1_DATA_0, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_1, ULPI1_DATA_1, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_2, ULPI1_DATA_2, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_3, ULPI1_DATA_3, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_4, ULPI1_DATA_4, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_5, ULPI1_DATA_5, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_6, ULPI1_DATA_6, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DATA_7, ULPI1_DATA_7, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_DIR,    ULPI1_DIR,    RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_NXT,    ULPI1_NXT,    RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(ULPI1_STP,    ULPI1_STP,    RESERVED, RESERVED, GPIO, RESERVED, RESERVED),

	PIN_DESC(NAND_AD_5, NAND_AD_5, SDIO3_DATA_2, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(NAND_AD_4, NAND_AD_4, SDIO3_DATA_1, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(NAND_AD_3, NAND_AD_3, SDIO3_CMD, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(NAND_AD_2, NAND_AD_2, SDIO3_DATA_3, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(NAND_AD_1, NAND_AD_1, SDIO3_CLK, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(NAND_AD_0, NAND_AD_0, SDIO3_DATA_0, RESERVED, GPIO, RESERVED, RESERVED),
	
	PIN_DESC(SDIO2_DATA_3, SDIO2_DATA_3, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(SDIO2_DATA_2, SDIO2_DATA_2, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(SDIO2_DATA_1, SDIO2_DATA_1, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(SDIO2_DATA_0, SDIO2_DATA_0, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(SDIO2_CMD, SDIO2_CMD, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(SDIO2_CLK, SDIO2_CLK, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	
	PIN_DESC(SDIO3_DATA_3, SDIO2_DATA_7, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(SDIO3_DATA_2, SDIO2_DATA_6, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(SDIO3_DATA_1, SDIO2_DATA_5, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(SDIO3_DATA_0, SDIO2_DATA_4, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(SDIO3_CMD, RESERVED, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(SDIO3_CLK, RESERVED, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),

	PIN_DESC(PMU_SCL, PMU_SCL, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
	PIN_DESC(PMU_SDA, PMU_SDA, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),

	PIN_DESC(BSC2_SCL, BSC2_SCL, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(BSC2_SDA, BSC2_SDA, RESERVED, RESERVED, GPIO, RESERVED, RESERVED),

	PIN_DESC(VC_CAM1_SCL, VC_CAM1_SCL, BSC1_SCL, RESERVED, GPIO, RESERVED, RESERVED),
        PIN_DESC(VC_CAM1_SDA, VC_CAM1_SDA, BSC1_SDA, RESERVED, GPIO, RESERVED, RESERVED),
};

struct chip_pin_desc g_chip_pin_desc = {
	.desc_tbl	=	pin_desc_tbl,
};

int __init pinmux_chip_init (void)
{
	g_chip_pin_desc.base = ioremap(CHIPREGS_BASE_ADDR, SZ_4K);
	BUG_ON (!g_chip_pin_desc.base);

	return 0;
}

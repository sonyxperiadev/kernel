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

#define	PIN_DESC(ball, alt1, alt2, alt3, alt4, alt5, alt6)		\
	[PN_##ball] = {							\
		.name		=	PN_##ball,			\
		.reg_offset	=	CHIPREG_##ball##_OFFSET,	\
		.f_tbl		=	{				\
			PF_##alt1, PF_##alt2, PF_##alt3,		\
			PF_##alt4, PF_##alt5, PF_##alt6,		\
		},							\
	}

/*
 * Island chip-level pin description table
 */
static const struct pin_desc pin_desc_tbl[PN_MAX] = {
    /*                      Signal Name        Alt_1              Alt_2             Alt_3           Alt_4     Alt_5         Alt_6   */
//	/* DUMMY 32 */ PIN_DESC(MPHI_DATA15,       GPEN09,            MPHI_DATA_15,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 31 */ PIN_DESC(MPHI_DATA14,       GPEN12,            MPHI_DATA_14,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 30 */ PIN_DESC(MPHI_DATA13,       GPEN10,            MPHI_DATA_13,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 29 */ PIN_DESC(MPHI_DATA12,       SIM2LDO_EN,        MPHI_DATA_12,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 28 */ PIN_DESC(MPHI_DATA11,       GPEN11,            MPHI_DATA_11,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 27 */ PIN_DESC(MPHI_DATA10,       GPEN00,            MPHI_DATA_10,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 26 */ PIN_DESC(MPHI_DATA09,       GPEN01,            MPHI_DATA_09,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 25 */ PIN_DESC(MPHI_DATA08,       GPEN02,            MPHI_DATA_08,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 24 */ PIN_DESC(MPHI_DATA07,       GPEN14,            MPHI_DATA_07,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 23 */ PIN_DESC(MPHI_DATA06,       GPEN04,            MPHI_DATA_06,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 22 */ PIN_DESC(MPHI_DATA05,       GPEN05,            MPHI_DATA_05,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 21 */ PIN_DESC(MPHI_DATA04,       GPEN06,            MPHI_DATA_04,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 20 */ PIN_DESC(MPHI_DATA03,       GPEN07,            MPHI_DATA_03,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 19 */ PIN_DESC(MPHI_DATA02,       GPEN08,            MPHI_DATA_02,     RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 20 */ PIN_DESC(MPHI_HCE1_N,       GPEN13,            MPHI_HCE1_N,      RESERVED,       RESERVED, RESERVED,     RESERVED),
//	/* DUMMY 19 */ PIN_DESC(MPHI_HCE0_N,       GPEN15,            MPHI_HCE0_N,      RESERVED,       RESERVED, RESERVED,     RESERVED),

	/* GPIO 191 */ PIN_DESC(SRI_C,             SRI_C,             MPHI_DATA_1,      RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 190 */ PIN_DESC(SRI_E,             SRI_E,             MPHI_DATA_0,      RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 189 */ PIN_DESC(SRI_D,             SRI_D,             MPHI_RUN,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 188 */ PIN_DESC(RFST2G_MTSLOTEN3G, GPEN03,            MPHI_HWR_N,       RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 187 */ PIN_DESC(TXDATA3G0,         TXDATA3G0,         MPHI_HRD_N,       RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 186 */ PIN_DESC(RTXDATA2G_TXDATA3G1, RTXDATA2G_TXDATA3G1, MPHI_HA0,     RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 185 */ PIN_DESC(RTXEN2G_TXDATA3G2, RTXEN2G_TXDATA3G2, MPHI_HAT1,        RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 184 */ PIN_DESC(RXDATA3G0,         RXDATA3G0,         MPHI_HAT0,        RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 183 */ PIN_DESC(RXDATA3G1,         RXDATA3G1,         RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 182 */ PIN_DESC(RXDATA3G2,         RXDATA3G2,         RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 181 */ PIN_DESC(CLK_CX8,           CLK_CX8,           RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 180 */ PIN_DESC(SYSCLKEN,          SYSCLKEN,          RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 179 */ PIN_DESC(NORFLSH_WE_N,      NORFLSH_WE_N,      RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 178 */ PIN_DESC(NORFLSH_CE1_N,     NORFLSH_CE1_N,     VC_GPIO_0,        UARTB3_URXD,    GPIO, RESERVED,         RESERVED),
	/* GPIO 177 */ PIN_DESC(NORFLSH_CE0_N,     NORFLSH_CE0_N,     CWS_SYS_REQ1,     CLKOUT_2,       GPIO, RESERVED,         RESERVED),
	/* GPIO 176 */ PIN_DESC(NORFLSH_OE_N,      NORFLSH_OE_N,      RESERVED,         CLKOUT_3,       GPIO, RESERVED,         RESERVED),
	/* GPIO 175 */ PIN_DESC(NORFLSH_ADDR_23,   NORFLSH_ADDR_23,   SSP2_FS_0,        KP_COL_IP_6,    GPIO, VC_TMS,           RESERVED),
	/* GPIO 174 */ PIN_DESC(NORFLSH_ADDR_22,   NORFLSH_ADDR_22,   SSP2_CLK,         KP_COL_IP_5,    GPIO, VC_TCK,           RESERVED),
	/* GPIO 173 */ PIN_DESC(NORFLSH_ADDR_21,   NORFLSH_ADDR_21,   SSP2_RXD_0,       KP_COL_IP_4,    GPIO, VC_TDI,           RESERVED),
	/* GPIO 172 */ PIN_DESC(NORFLSH_ADDR_20,   NORFLSH_ADDR_20,   SSP2_TXD_0,       KP_COL_IP_3,    GPIO, IRD_OSC_0,        RESERVED),
	/* GPIO 171 */ PIN_DESC(NORFLSH_ADDR_19,   NORFLSH_ADDR_19,   VC_GPIO_1,        PWM_O_0,        GPIO, IRD_OSC_1,        RESERVED),
	/* GPIO 170 */ PIN_DESC(NORFLSH_ADDR_18,   NORFLSH_ADDR_18,   RESERVED,         PWM_O_1,        GPIO, IRD_OSC_2,        RESERVED),
	/* GPIO 169 */ PIN_DESC(NORFLSH_ADDR_17,   NORFLSH_ADDR_17,   RESERVED,         PWM_O_2,        GPIO, IRD_OSC_3,        RESERVED),
	/* GPIO 168 */ PIN_DESC(NORFLSH_ADDR_16,   NORFLSH_ADDR_16,   RESERVED,         PWM_O_5,        GPIO, IRD_OSC_4,        RESERVED),
	/* GPIO 167 */ PIN_DESC(NORFLSH_AADLAT_EN, NORFLSH_AADLAT_EN, VC_GPIO_2,        KP_COL_IP_2,    GPIO, IRD_OSC_5,        RESERVED),
	/* GPIO 166 */ PIN_DESC(NORFLSH_ADLAT_EN,  NORFLSH_ADLAT_EN,  VC_GPIO_3,        KP_COL_IP_1,    GPIO, IRD_OSC_6,        RESERVED),
	/* GPIO 165 */ PIN_DESC(NORFLSH_AD_15,     NORFLSH_AD_15,     SWDCLKTCK,        UARTB3_UTXD,    GPIO, IRD_OSC_7,        RESERVED),
	/* GPIO 164 */ PIN_DESC(NORFLSH_AD_14,     NORFLSH_AD_14,     SWDIOTMS,         KP_ROW_OP_0,    GPIO, IRD_OSC_8,        RESERVED),
	/* GPIO 163 */ PIN_DESC(NORFLSH_AD_13,     NORFLSH_AD_13,     CLKREQ_IN_2,      KP_ROW_OP_1,    GPIO, IRD_OSC_9,        RESERVED),
	/* GPIO 162 */ PIN_DESC(NORFLSH_AD_12,     NORFLSH_AD_12,     CLKREQ_IN_3,      KP_ROW_OP_2,    GPIO, IRD_OSC_10,       RESERVED),
	/* GPIO 161 */ PIN_DESC(NORFLSH_AD_11,     NORFLSH_AD_11,     CWS_SYS_REQ2,     KP_ROW_OP_3,    GPIO, RESERVED,         RESERVED),
	/* GPIO 160 */ PIN_DESC(NORFLSH_AD_10,     NORFLSH_AD_10,     CWS_SYS_REQ3,     KP_ROW_OP_4,    GPIO, RESERVED,         RESERVED),
	/* GPIO 159 */ PIN_DESC(NORFLSH_AD_09,     NORFLSH_AD_09,     VC_GPIO_4,        KP_ROW_OP_5,    GPIO, VC_TDO,           RESERVED),
	/* GPIO 158 */ PIN_DESC(NORFLSH_AD_08,     NORFLSH_AD_08,     SSP2_FS_3,        KP_ROW_OP_6,    GPIO, VC_TRSTB,         RESERVED),
	/* GPIO 157 */ PIN_DESC(NORFLSH_AD_07,     NORFLSH_AD_07,     SSP2_FS_1,        KP_COL_IP_7,    GPIO, RESERVED,         RESERVED),
	/* GPIO 156 */ PIN_DESC(NORFLSH_AD_06,     NORFLSH_AD_06,     SSP2_RXD_1,       SDIO1_DATA_2,   GPIO, RESERVED,         RESERVED),
	/* GPIO 155 */ PIN_DESC(NORFLSH_AD_05,     NORFLSH_AD_05,     SSP2_TXD_1,       SDIO1_DATA_1,   GPIO, RESERVED,         RESERVED),
	/* GPIO 154 */ PIN_DESC(NORFLSH_AD_04,     NORFLSH_AD_04,     VC_PWM_0,         KP_COL_IP_0,    GPIO, RESERVED,         RESERVED),
	/* GPIO 153 */ PIN_DESC(NORFLSH_AD_03,     NORFLSH_AD_03,     SSP3_FS,          SDIO1_DATA_3,   GPIO, RESERVED,         RESERVED),
	/* GPIO 152 */ PIN_DESC(NORFLSH_AD_02,     NORFLSH_AD_02,     SSP3_CLK,         SDIO1_CLK,      GPIO, RESERVED,         RESERVED),
	/* GPIO 151 */ PIN_DESC(NORFLSH_AD_01,     NORFLSH_AD_01,     SSP3_RXD,         SDIO1_DATA_0,   GPIO, RESERVED,         RESERVED),
	/* GPIO 150 */ PIN_DESC(NORFLSH_AD_00,     NORFLSH_AD_00,     SSP3_TXD,         SDIO1_CMD,      GPIO, RESERVED,         RESERVED),
	/* GPIO 149 */ PIN_DESC(NORFLSH_RDY,       NORFLSH_RDY,       SDIO4_DATA_7,     SDIO1_DATA_1,   GPIO, RESERVED,         RESERVED),
	/* GPIO 148 */ PIN_DESC(NORFLSH_CLK_N,     NORFLSH_CLK_N,     SDIO4_DATA_6,     SDIO1_DATA_2,   GPIO, RESERVED,         RESERVED),

	/* GPIO 147 */ PIN_DESC(SIM_RESETN,        SIM_RESETN,        RESERVED,         VC_GPIO_5,      GPIO, RESERVED,         RESERVED),
	/* GPIO 146 */ PIN_DESC(SIM_CLK,           SIM_CLK,           RESERVED,         VC_GPIO_6,      GPIO, RESERVED,         RESERVED),
	/* GPIO 145 */ PIN_DESC(SIM_DATA,          SIM_DATA,          PWM_O_2,          VC_GPIO_7,      GPIO, RESERVED,         RESERVED),
	/* GPIO 144 */ PIN_DESC(SIM_DET,           SIM_DET,           RESERVED,         VC_PWM_1,       GPIO, RESERVED,         RESERVED),

	/* GPIO 143 */ PIN_DESC(SIM2_RESETN,       SIM2_RESETN,       SSP3_FS,          VC_GPIO_8,      GPIO, RESERVED,         RESERVED),
	/* GPIO 142 */ PIN_DESC(SIM2_CLK,          SIM2_CLK,          SSP3_CLK,         VC_TE2,         GPIO, RESERVED,         RESERVED),
	/* GPIO 141 */ PIN_DESC(SIM2_DATA,         SIM2_DATA,         SSP3_RXD,         VC_TE0,         GPIO, RESERVED,         RESERVED),
	/* GPIO 140 */ PIN_DESC(SIM2_DET,          SIM2_DET,          SSP3_TXD,         VC_TE1,         GPIO, RESERVED,         RESERVED),

	/* GPIO 139 */ PIN_DESC(ULPI0_CLOCK,       ULPI0_CLOCK,       SSP0_CLK,         UARTB4_URTSN,   GPIO, AP_REF_2P4M_CLK,  RESERVED),
	/* GPIO 138 */ PIN_DESC(ULPI0_DATA_0,      ULPI0_DATA_0,      SSP0_FS,          UARTB4_UCTSN,   GPIO, ADC_AP_EN,        RESERVED),
	/* GPIO 137 */ PIN_DESC(ULPI0_DATA_1,      ULPI0_DATA_1,      SSP0_RXD,         UARTB4_URXD,    GPIO, EP_AP_EN,         RESERVED),
	/* GPIO 136 */ PIN_DESC(ULPI0_DATA_2,      ULPI0_DATA_2,      SSP0_TXD,         UARTB4_UTXD,    GPIO, HS_AP_EN,         RESERVED),
	/* GPIO 135 */ PIN_DESC(ULPI0_DATA_3,      ULPI0_DATA_3,      ACDATA,           UARTB3_URTSN,   GPIO, VIBRA_AP_EN,      RESERVED),
	/* GPIO 134 */ PIN_DESC(ULPI0_DATA_4,      ULPI0_DATA_4,      ACFLAG,           UARTB3_UCTSN,   GPIO, IHF_AP_EN,        RESERVED),
	/* GPIO 133 */ PIN_DESC(ULPI0_DATA_5,      ULPI0_DATA_5,      ACREADY,          UARTB3_URXD,    GPIO, DAC_SCL_EN,       RESERVED),
	/* GPIO 132 */ PIN_DESC(ULPI0_DATA_6,      ULPI0_DATA_6,      ACWAKE,           UARTB3_UTXD,    GPIO, AP_ADC_DATA_O,    RESERVED),
	/* GPIO 131 */ PIN_DESC(ULPI0_DATA_7,      ULPI0_DATA_7,      CADATA,           UARTB2_URTSN,   GPIO, AP_DAC_DATA_I,    RESERVED),
	/* GPIO 130 */ PIN_DESC(ULPI0_DIR,         ULPI0_DIR,         CAFLAG,           UARTB2_UCTSN,   GPIO, AP_IHF_DATA_I,    RESERVED),
	/* GPIO 129 */ PIN_DESC(ULPI0_NXT,         ULPI0_NXT,         CAREADY,          UARTB2_URXD,    GPIO, AP_LRCK_I,        RESERVED),
	/* GPIO 128 */ PIN_DESC(ULPI0_STP,         ULPI0_STP,         CAWAKE,           UARTB2_UTXD,    GPIO, AP_SCLK_I,        RESERVED),

	/* GPIO 127 */ PIN_DESC(ULPI1_CLOCK,       ULPI1_CLOCK,       SSP2_CLK,         SDIO3_CLK,      GPIO, IRD_OSC_0,        RESERVED),
	/* GPIO 126 */ PIN_DESC(ULPI1_DATA_0,      ULPI1_DATA_0,      SSP2_FS_0,        SDIO3_DATA_3,   GPIO, AP_RSTN,          RESERVED),
	/* GPIO 125 */ PIN_DESC(ULPI1_DATA_1,      ULPI1_DATA_1,      SSP2_FS_1,        SDIO3_DATA_2,   GPIO, IRD_OSC_1,        RESERVED),
	/* GPIO 124 */ PIN_DESC(ULPI1_DATA_2,      ULPI1_DATA_2,      SSP2_RXD_1,       SDIO3_DATA_1,   GPIO, IRD_OSC_2,        RESERVED),
	/* GPIO 123 */ PIN_DESC(ULPI1_DATA_3,      ULPI1_DATA_3,      SSP2_RXD_0,       SDIO3_DATA_0,   GPIO, IRD_OSC_3,        RESERVED),
	/* GPIO 122 */ PIN_DESC(ULPI1_DATA_4,      ULPI1_DATA_4,      SSP2_TXD_0,       SDIO3_CMD,      GPIO, IRD_OSC_4,        RESERVED),
	/* GPIO 121 */ PIN_DESC(ULPI1_DATA_5,      ULPI1_DATA_5,      SSP2_TXD_0,       RESERVED,       GPIO, IRD_OSC_5,        RESERVED),
	/* GPIO 120 */ PIN_DESC(ULPI1_DATA_6,      ULPI1_DATA_6,      SSP2_FS_3,        RESERVED,       GPIO, IRD_OSC_6,        RESERVED),
	/* GPIO 119 */ PIN_DESC(ULPI1_DATA_7,      ULPI1_DATA_7,      SSP2_FS_2,        PWM_O_2,        GPIO, IRD_OSC_7,        RESERVED),
	/* GPIO 118 */ PIN_DESC(ULPI1_DIR,         ULPI1_DIR,         BSC1_SCL_GP,      NORFLSH_WP,     GPIO, IRD_OSC_8,        RESERVED),
	/* GPIO 117 */ PIN_DESC(ULPI1_NXT,         ULPI1_NXT,         BSC1_SDA_GP,      VC_TE2,         GPIO, IRD_OSC_9,        RESERVED),
	/* GPIO 116 */ PIN_DESC(ULPI1_STP,         ULPI1_STP,         SDIO4_CLK,        VC_TE1,         GPIO, IRD_OSC_10,       RESERVED),

	/* GPIO 115 */ PIN_DESC(NAND_WP,           NAND_WP,           SDIO4_DATA_3,     OSC1_OUT,       GPIO, RESERVED,         RESERVED),
	/* GPIO 114 */ PIN_DESC(NAND_CEN_0,        NAND_CEN_0,        SDIO4_DATA_2,     OSC2_OUT,       GPIO, WCDMA_CLK1,       DEBUG_BUS00),
	/* GPIO 113 */ PIN_DESC(NAND_CEN_1,        NAND_CEN_1,        SDIO4_DATA_1,     RESERVED,       GPIO, WCDMA_CLK2,       DEBUG_BUS01),
	/* GPIO 112 */ PIN_DESC(NAND_RDY_0,        NAND_RDY_0,        SDIO4_DATA_0,     RESERVED,       GPIO, WCDMA_CLK3,       DEBUG_BUS02),
	/* GPIO 111 */ PIN_DESC(NAND_RDY_1,        NAND_RDY_1,        SDIO4_CMD,        VC_TRSTB,       GPIO, WCDMA_SYNC,       DEBUG_BUS03),
	/* GPIO 110 */ PIN_DESC(NAND_CLE,          NAND_CLE,          SDIO4_DATA_5,     VC_TCK,         GPIO, WCDMA_DEBUG_0,    DEBUG_BUS04),
	/* GPIO 109 */ PIN_DESC(NAND_ALE,          NAND_ALE,          SDIO4_DATA_4,     VC_TDI,         GPIO, WCDMA_DEBUG_1,    DEBUG_BUS05),
	/* GPIO 108 */ PIN_DESC(NAND_OEN,          NAND_OEN,          SDIO3_DATA_7,     VC_TDO,         GPIO, WCDMA_DEBUG_2,    DEBUG_BUS06),
	/* GPIO 107 */ PIN_DESC(NAND_WEN,          NAND_WEN,          SDIO3_DATA_6,     VC_TMS,         GPIO, WCDMA_DEBUG_3,    DEBUG_BUS07),
	/* GPIO 106 */ PIN_DESC(NAND_AD_7,         NAND_AD_7,         SDIO3_DATA_5,     UARTB_UTXD,     GPIO, WCDMA_DEBUG_4,    DEBUG_BUS08),
	/* GPIO 105 */ PIN_DESC(NAND_AD_6,         NAND_AD_6,         SDIO3_DATA_4,     UARTB_URTSN,    GPIO, WCDMA_DEBUG_5,    DEBUG_BUS09),
	/* GPIO 104 */ PIN_DESC(NAND_AD_5,         NAND_AD_5,         SDIO3_DATA_2,     UARTB_UCTSN,    GPIO, WCDMA_DEBUG_6,    DEBUG_BUS10),
	/* GPIO 103 */ PIN_DESC(NAND_AD_4,         NAND_AD_4,         SDIO3_DATA_1,     UARTB_URXD,     GPIO, WCDMA_DEBUG_7,    DEBUG_BUS11),
	/* GPIO 102 */ PIN_DESC(NAND_AD_3,         NAND_AD_3,         SDIO3_CMD,        SSP1_TXD,       GPIO, WCDMA_UART_RXD,   DEBUG_BUS12),
	/* GPIO 101 */ PIN_DESC(NAND_AD_2,         NAND_AD_2,         SDIO3_DATA_3,     SSP1_FS,        GPIO, WCDMA_UART_TXD,   DEBUG_BUS13),
	/* GPIO 100 */ PIN_DESC(NAND_AD_1,         NAND_AD_1,         SDIO3_CLK,        SSP1_CLK,       GPIO, RESERVED,         DEBUG_BUS14),
	/* GPIO 99 */  PIN_DESC(NAND_AD_0,         NAND_AD_0,         SDIO3_DATA_0,     SSP1_RXD,       GPIO, RESERVED,         DEBUG_BUS15),

	/* GPIO 98 */ PIN_DESC(UARTB4_UTXD,        UARTB4_UTXD,       VC_TE0,           VC_UTXD,        GPIO, RESERVED,         RESERVED),
	/* GPIO 97 */ PIN_DESC(UARTB4_URXD,        UARTB4_URXD,       VC_TE1,           VC_URXD,        GPIO, RESERVED,         RESERVED),

	/* GPIO 96 */ PIN_DESC(SDIO2_DATA_3,       SDIO2_DATA_3,      SWDCLKTCK,        PTI_DAT3,       GPIO, RESERVED,         RESERVED),
	/* GPIO 95 */ PIN_DESC(SDIO2_DATA_2,       SDIO2_DATA_2,      SWDIOTMS,         PTI_DAT2,       GPIO, RESERVED,         RESERVED),
	/* GPIO 94 */ PIN_DESC(SDIO2_DATA_1,       SDIO2_DATA_1,      RESERVED,         PTI_DAT1,       GPIO, RESERVED,         RESERVED),
	/* GPIO 93 */ PIN_DESC(SDIO2_DATA_0,       SDIO2_DATA_0,      RESERVED,         PTI_DAT0,       GPIO, RESERVED,         RESERVED),
	/* GPIO 92 */ PIN_DESC(SDIO2_CMD,          SDIO2_CMD,         RESERVED,         UARTB2_URXD,    GPIO, RESERVED,         RESERVED),
	/* GPIO 91 */ PIN_DESC(SDIO2_CLK,          SDIO2_CLK,         RESERVED,         PTI_CLK,        GPIO, RESERVED,         RESERVED),

	/* GPIO 90 */ PIN_DESC(SDIO3_DATA_3,       SDIO2_DATA_7,      VC_TESTDEBUG_CLK, SDIO3_DATA_3,   GPIO, AFCPDM,           RESERVED),
	/* GPIO 89 */ PIN_DESC(SDIO3_DATA_2,       SDIO2_DATA_6,      VC_TMS,           SDIO3_DATA_2,   GPIO, VC_SPI_CE1_N,     PM_DEBUG0),
	/* GPIO 88 */ PIN_DESC(SDIO3_DATA_1,       SDIO2_DATA_5,      VC_TRSTB,         SDIO3_DATA_1,   GPIO, VC_SPI_CE0_N,     PM_DEBUG1),
	/* GPIO 87 */ PIN_DESC(SDIO3_DATA_0,       SDIO2_DATA_4,      VC_TDI,           SDIO3_DATA_0,   GPIO, VC_SPI_MISO,      PM_DEBUG2),
	/* GPIO 86 */ PIN_DESC(SDIO3_CMD,          SDIO2_WP,          VC_TDO,           SDIO3_CMD,      GPIO, VC_SPI_MOSI,      PM_DEBUG3),
	/* GPIO 85 */ PIN_DESC(SDIO3_CLK,          SDIO2_CD_N,        VC_TCK,           SDIO3_CLK,      GPIO, VC_SPI_SCLK,      RESERVED),

	/* GPIO 84 */ PIN_DESC(SSP0_FS,            SSP0_FS,           KP_ROW_OP_4,      UARTB2_URXD,    GPIO, RFGPO_3,          RESERVED),
	/* GPIO 83 */ PIN_DESC(SSP0_CLK,           SSP0_CLK,          KP_ROW_OP_5,      UARTB2_UTXD,    GPIO, RFGPO_2,          RESERVED),
	/* GPIO 82 */ PIN_DESC(SSP0_RXD,           SSP0_RXD,          KP_ROW_OP_6,      UARTB2_URTSN,   GPIO, RFGPO_1,          RESERVED),
	/* GPIO 81 */ PIN_DESC(SSP0_TXD,           SSP0_TXD,          KP_ROW_OP_7,      UARTB2_UCTSN,   GPIO, RFGPO_0,          RESERVED),

	/* GPIO 80 */ PIN_DESC(SSP1_FS,            SSP1_FS,           KP_ROW_OP_0,      UARTB3_URXD,    GPIO, WCDMA_CLK1,       RESERVED),
	/* GPIO 79 */ PIN_DESC(SSP1_CLK,           SSP1_CLK,          KP_ROW_OP_1,      UARTB3_UTXD,    GPIO, WCDMA_CLK2,       RESERVED),
	/* GPIO 78 */ PIN_DESC(SSP1_RXD,           SSP1_RXD,          KP_ROW_OP_2,      UARTB3_URTSN,   GPIO, WCDMA_CLK3,       RESERVED),
	/* GPIO 77 */ PIN_DESC(SSP1_TXD,           SSP1_TXD,          KP_ROW_OP_3,      UARTB3_UCTSN,   GPIO, WCDMA_SYNC,       RESERVED),

	/* GPIO 76 */ PIN_DESC(SSP2_FS_0,          SSP2_FS_0,         UARTB4_URXD,      ACDATA,         GPIO, WCDMA_DEBUG_0,    RESERVED),
	/* GPIO 75 */ PIN_DESC(SSP2_CLK,           SSP2_CLK,          UARTB4_UTXD,      ACFLAG,         GPIO, WCDMA_DEBUG_1,    RESERVED),
	/* GPIO 74 */ PIN_DESC(SSP2_RXD_0,         SSP2_RXD_0,        UARTB4_URTSN,     ACREADY,        GPIO, WCDMA_DEBUG_2,    RESERVED),
	/* GPIO 73 */ PIN_DESC(SSP2_TXD_0,         SSP2_TXD_0,        UARTB4_UCTSN,     ACWAKE,         GPIO, WCDMA_DEBUG_3,    RESERVED),
	/* GPIO 72 */ PIN_DESC(SSP2_FS_1,          SSP2_FS_1,         IrRtsSd,          CADATA,         GPIO, WCDMA_DEBUG_4,    RESERVED),
	/* GPIO 71 */ PIN_DESC(SSP2_RXD_1,         SSP2_RXD_1,        IrRx,             CAFLAG,         GPIO, WCDMA_DEBUG_5,    RESERVED),
	/* GPIO 70 */ PIN_DESC(SSP2_TXD_1,         SSP2_TXD_1,        IrTx,             CAREADY,        GPIO, WCDMA_DEBUG_6,    RESERVED),
	/* GPIO 69 */ PIN_DESC(SSP2_FS_2,          SSP2_FS_2,         VC_PWM_0,         CAWAKE,         GPIO, WCDMA_DEBUG_7,    RESERVED),

	/* GPIO 68 */ PIN_DESC(SSP3_FS,            SSP3_FS,           VC_I2S_WSIO,      RESERVED,       GPIO, VC_SPI_CE1_N,     RESERVED),
	/* GPIO 67 */ PIN_DESC(SSP3_CLK,           SSP3_CLK,          VC_I2S_SCK,       RESERVED,       GPIO, VC_SPI_CE0_N,     RESERVED),
	/* GPIO 66 */ PIN_DESC(SSP3_RXD,           SSP3_RXD,          VC_I2S_SDI,       RESERVED,       GPIO, VC_SPI_MISO,      RESERVED),
	/* GPIO 65 */ PIN_DESC(SSP3_TXD,           SSP3_TXD,          VC_I2S_SDO,       RESERVED,       GPIO, VC_SPI_MOSI,      RESERVED),
	/* GPIO 64 */ PIN_DESC(SSP3_EXTCLK,        SSP3_EXTCLK,       VC_TESTDEBUG_CLK, RESERVED,       GPIO, VC_SPI_SCLK,      RESERVED),

	/* GPIO 63 */ PIN_DESC(ARM_SLB_CLK,        ARM_SLB_CLK,       RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 62 */ PIN_DESC(ARM_SLB_DATA,       ARM_SLB_DATA,      RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 61 */ PIN_DESC(PMU_SCL,            PMU_SCL,           RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 60 */ PIN_DESC(PMU_SDA,            PMU_SDA,           RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 59 */ PIN_DESC(BSC2_SCL,           BSC2_SCL,          VC_CAM2_SCL,      VC_GPIO_8,      GPIO, RESERVED,         RESERVED),
	/* GPIO 58 */ PIN_DESC(BSC2_SDA,           BSC2_SDA,          VC_CAM2_SDA,      VC_GPIO_9,      GPIO, RESERVED,         RESERVED),

	/* GPIO 57 */ PIN_DESC(VC_CAM1_SCL,        VC_CAM1_SCL,       BSC1_SCL,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 56 */ PIN_DESC(VC_CAM1_SDA,        VC_CAM1_SDA,       BSC1_SDA,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 55 */ PIN_DESC(HDMI_SCL,           HDMI_SCL,          RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 54 */ PIN_DESC(HDMI_SDA,           HDMI_SDA,          RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 53 */ PIN_DESC(UARTB_URXD,         UARTB_URXD,        VC_TMS,           SSP1_FS,        GPIO, RESERVED,         RESERVED),
	/* GPIO 52 */ PIN_DESC(UARTB_UTXD,         UARTB_UTXD,        VC_TCK,           SSP1_CLK,       GPIO, RESERVED,         RESERVED),
	/* GPIO 51 */ PIN_DESC(UARTB_URTSN,        UARTB_URTSN,       VC_TDI,           SSP1_RXD,       GPIO, RESERVED,         RESERVED),
	/* GPIO 50 */ PIN_DESC(UARTB_UCTSN,        UARTB_UCTSN,       VC_TDO,           SSP1_TXD,       GPIO, RESERVED,         RESERVED),

	/* GPIO 49 */ PIN_DESC(UARTB2_URXD,        UARTB2_URXD,       VC_TRSTB,         PWM_O_3,        GPIO, RESERVED,         RESERVED),
	/* GPIO 48 */ PIN_DESC(UARTB2_UTXD,        UARTB2_UTXD,       RTCK,             PWM_O_4,        GPIO, AFCPDM,           RESERVED),

	/* GPIO 47 */ PIN_DESC(LCD_R_7,            LCD_R_7,           MPHI_RUN,         SSP2_FS_0,      GPIO, CLCD_D_17,        RESERVED),
	/* GPIO 46 */ PIN_DESC(LCD_R_6,            LCD_R_6,           MPHI_HCE1_N,      SSP2_FS_2,      GPIO, CLCD_D_16,        RESERVED),
	/* GPIO 45 */ PIN_DESC(LCD_R_5,            LCD_R_5,           MPHI_HCE0_N,      SSP2_CLK,       GPIO, CLCD_D_15,        RESERVED),
	/* GPIO 44 */ PIN_DESC(LCD_R_4,            LCD_R_4,           MPHI_HWR_N,       SSP2_RXD_0,     GPIO, CLCD_D_14,        RESERVED),
	/* GPIO 43 */ PIN_DESC(LCD_R_3,            LCD_R_3,           MPHI_HRD_N,       SSP2_TXD_0,     GPIO, CLCD_D_13,        RESERVED),
	/* GPIO 42 */ PIN_DESC(LCD_R_2,            LCD_R_2,           MPHI_HA0,         PWM_O_0,        GPIO, CLCD_D_12,        RESERVED),
	/* GPIO 41 */ PIN_DESC(LCD_R_1,            LCD_R_1,           MPHI_HAT0,        KP_COL_IP_0,    GPIO, VC_PWM_0,         RESERVED),
	/* GPIO 40 */ PIN_DESC(LCD_R_0,            LCD_R_0,           MPHI_HAT1,        KP_COL_IP_1,    GPIO, CLCD_OEN,         RESERVED),

	/* GPIO 39 */ PIN_DESC(LCD_G_7,            LCD_G_7,           MPHI_DATA_15,     SSP2_FS_1,      GPIO, CLCD_D_11,        RESERVED),
	/* GPIO 38 */ PIN_DESC(LCD_G_6,            LCD_G_6,           MPHI_DATA_14,     WCDMA_UART_RXD, GPIO, CLCD_D_10,        RESERVED),
	/* GPIO 37 */ PIN_DESC(LCD_G_5,            LCD_G_5,           MPHI_DATA_13,     SSP2_RXD_1,     GPIO, CLCD_D_9,         RESERVED),
	/* GPIO 36 */ PIN_DESC(LCD_G_4,            LCD_G_4,           MPHI_DATA_12,     SSP2_TXD_1,     GPIO, CLCD_D_8,         RESERVED),
	/* GPIO 35 */ PIN_DESC(LCD_G_3,            LCD_G_3,           MPHI_DATA_11,     PWM_O_1,        GPIO, CLCD_D_7,         RESERVED),
	/* GPIO 34 */ PIN_DESC(LCD_G_2,            LCD_G_2,           MPHI_DATA_10,     PWM_O_2,        GPIO, CLCD_D_6,         RESERVED),
	/* GPIO 33 */ PIN_DESC(LCD_G_1,            LCD_G_1,           MPHI_DATA_9,      KP_COL_IP_2,    GPIO, CLCD_WEN,         RESERVED),
	/* GPIO 32 */ PIN_DESC(LCD_G_0,            LCD_G_0,           MPHI_DATA_8,      KP_COL_IP_3,    GPIO, CLCD_A_0,         RESERVED),

	/* GPIO 31 */ PIN_DESC(LCD_B_7,            LCD_B_7,           MPHI_DATA_7,      KP_COL_IP_6,    GPIO, CLCD_D_5,         RESERVED),
	/* GPIO 30 */ PIN_DESC(LCD_B_6,            LCD_B_6,           MPHI_DATA_6,      KP_COL_IP_7,    GPIO, CLCD_D_4,         RESERVED),
	/* GPIO 29 */ PIN_DESC(LCD_B_5,            LCD_B_5,           MPHI_DATA_5,      PWM_O_5,        GPIO, CLCD_D_3,         RESERVED),
	/* GPIO 28 */ PIN_DESC(LCD_B_4,            LCD_B_4,           MPHI_DATA_4,      WCDMA_UART_TXD, GPIO, CLCD_D_2,         RESERVED),
	/* GPIO 27 */ PIN_DESC(LCD_B_3,            LCD_B_3,           MPHI_DATA_3,      SDIO1_DATA_3,   GPIO, CLCD_D_1,         RESERVED),
	/* GPIO 26 */ PIN_DESC(LCD_B_2,            LCD_B_2,           MPHI_DATA_2,      SDIO1_DATA_2,   GPIO, CLCD_D_0,         RESERVED),
	/* GPIO 25 */ PIN_DESC(LCD_B_1,            LCD_B_1,           MPHI_DATA_1,      KP_COL_IP_4,    GPIO, CLCD_A_1,         RESERVED),
	/* GPIO 24 */ PIN_DESC(LCD_B_0,            LCD_B_0,           MPHI_DATA_0,      KP_COL_IP_5,    GPIO, CLCD_A_2,         RESERVED),

	/* GPIO 23 */ PIN_DESC(LCD_HSYNC,          LCD_HSYNC,         PWM_O_2,          SDIO1_DATA_1,   GPIO, CLCD_A_3,         RESERVED),
	/* GPIO 22 */ PIN_DESC(LCD_VSYNC,          LCD_VSYNC,         DW_WIRE_1,        SDIO1_DATA_0,   GPIO, CLCD_A_4,         RESERVED),
	/* GPIO 21 */ PIN_DESC(LCD_OE,             LCD_OE,            SWDCLKTCK,        SDIO1_CMD,      GPIO, CLCD_A_5,         RESERVED),
	/* GPIO 20 */ PIN_DESC(LCD_PCLK,           LCD_PCLK,          SWDIOTMS,         SDIO1_CLK,      GPIO, VC_PWM_1,         RESERVED),

	/* GPIO 19 */ PIN_DESC(CLKREQ_IN_0,        CLKREQ_IN_0,       RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 18 */ PIN_DESC(CLKREQ_IN_1,        CLKREQ_IN_1,       RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 17 */ PIN_DESC(CLKOUT_0,           CLKOUT_0,          RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 16 */ PIN_DESC(CLKOUT_1,           CLKOUT_1,          RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 15 */ PIN_DESC(DIGMIC1_CLK,        DIGMIC1_CLK,       RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
   	/* GPIO 14 */ PIN_DESC(DIGMIC1_DQ,         DIGMIC1_DQ,        RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 13 */ PIN_DESC(DIGMIC2_CLK,        DIGMIC2_CLK,       RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 12 */ PIN_DESC(DIGMIC2_DQ,         DIGMIC2_DQ,        RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 11 */ PIN_DESC(BAT_RM,             BAT_RM,            RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 10 */ PIN_DESC(PMU_INT,            PMU_INT,           RESERVED,         RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 9 */  PIN_DESC(STAT_1,             STAT_1,            SIMLDO_EN,        RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 8 */  PIN_DESC(STAT_2,             STAT_2,            CLASSABPWR,       RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 7 */  PIN_DESC(GPIO_7,             SDIO2_WP,          VC_TE2,           GENERIC_CLK,    GPIO, RESERVED,         RESERVED),
	/* GPIO 6 */  PIN_DESC(GPIO_6,             SDIO2_CD_N,        VC_PWM_1,         RESERVED,       GPIO, RESERVED,         RESERVED),

	/* GPIO 5 */  PIN_DESC(GPIO_5,             ARM_SLB_CLK,       VC_CPG0,          UARTB2_URXD,    GPIO, RESERVED,         RESERVED),
	/* GPIO 4 */  PIN_DESC(GPIO_4,             ARM_SLB_DATA,      VC_CPG1,          UARTB2_UTXD,    GPIO, RESERVED,         RESERVED),

	/* GPIO 3 */  PIN_DESC(GPIO_3,             SDIO3_WP,          VC_GPCLK_0,       RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 2 */  PIN_DESC(GPIO_2,             SDIO3_CD_N,        VC_GPCLK_1,       RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 1 */  PIN_DESC(GPIO_1,             SWDCLKTCK,         VC_TE0,           RESERVED,       GPIO, RESERVED,         RESERVED),
	/* GPIO 0 */  PIN_DESC(GPIO_0,             SWDIOTMS,          VC_TE1,           RESERVED,       GPIO, RESERVED,         RESERVED),
	
	/* STM pin description */	
	PIN_DESC(TRACECLK, TRACECLK, PTI_CLK, RESERVED, MDN_DSP_TRACE_CLK, GPIO, RESERVED),
	PIN_DESC(TRACEDT07, TRACEDT07, UARTB2_URXD, RESERVED, MDN_DSP_TRACE_DAT7, GPIO, RESERVED),
	PIN_DESC(TRACEDT03, TRACEDT03, PTI_DAT3, RESERVED, MDN_DSP_TRACE_DAT3, GPIO, RESERVED),
	PIN_DESC(TRACEDT02, TRACEDT02, PTI_DAT2, RESERVED, MDN_DSP_TRACE_DAT2, GPIO, RESERVED),
	PIN_DESC(TRACEDT01, TRACEDT01, PTI_DAT1, RESERVED, MDN_DSP_TRACE_DAT1, GPIO, RESERVED),
	PIN_DESC(TRACEDT00, TRACEDT00, PTI_DAT0, RESERVED, MDN_DSP_TRACE_DAT0, GPIO, RESERVED),
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

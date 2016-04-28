/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
*
* 	@file	include/linux/broadcom/xscript/xsapi_clk_def.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/**
*
*   @file   xsapi_clk_def.h
*
*   @brief  This file contains clock ID definitions.
*
****************************************************************************/
#ifndef XSAPI_CLK_DEF_H__
#define XSAPI_CLK_DEF_H__	/* /< Include guard. */

/*
 * Clock IDs from kernel/common/arch/arm/mach-bcm116x/include/mach/clkmgr.h
 * and kernel/common/arch/arm/mach-bcm215xx/include/mach/clkmgr.h
 * mapped to xScript clock enumeration values. The strings in clkmgr.h
 * may change, but the clock IDs should not change
 * Define xScript versions of the IDs so that the user space is immune
 * to string changes in the kernel.
 */
typedef enum {
XS_BCM_CLK_MAIN_PLL_STR_ID = 0,	/* /< BCM_CLK_MAIN_PLL_STR_ID */
XS_BCM_CLK_ARM11_STR_ID = 2,	/* /< BCM_CLK_ARM11_STR_ID */
XS_BCM_CLK_CAMERA_STR_ID = 3,	/* /< BCM_CLK_CAMERA_STR_ID */
XS_BCM_CLK_I2S_INT_STR_ID = 4,	/* /< BCM_CLK_I2S_INT_STR_ID */
XS_BCM_CLK_I2S_EXT_STR_ID = 5,	/* /< BCM_CLK_I2S_EXT_STR_ID */
XS_BCM_CLK_DAM_STR_ID = 6,	/* /< BCM_CLK_DAM_STR_ID */
XS_BCM_CLK_PDP_STR_ID = 7,	/* /< BCM_CLK_PDP_STR_ID */
XS_BCM_CLK_SDIO1_STR_ID = 8,	/* /< BCM_CLK_SDIO1_STR_ID */
XS_BCM_CLK_SDIO2_STR_ID = 9,	/* /< BCM_CLK_SDIO2_STR_ID */

/**********************************/
/* Only available for 2153 */
XS_BCM_CLK_SM_STR_ID = 10,	/* /< BCM_CLK_SM_STR_ID */
/**********************************/

XS_BCM_CLK_SPI0_STR_ID = 11,	/* /< BCM_CLK_SPI0_STR_ID */
XS_BCM_CLK_UARTA_STR_ID = 12,	/* /< BCM_CLK_UARTA_STR_ID */
XS_BCM_CLK_UARTB_STR_ID = 13,	/* /< BCM_CLK_UARTB_STR_ID */
XS_BCM_CLK_GP_STR_ID = 14,	/* /< BCM_CLK_GP_STR_ID */

/**********************************/
/* Only available for 2153, 21553 */
XS_BCM_CLK_MSPRO_STR_ID = 15,	/* /< BCM_CLK_MSPRO_STR_ID */
/**********************************/

XS_BCM_CLK_I2C1_STR_ID = 16,	/* /< BCM_CLK_I2C1_STR_ID */
XS_BCM_CLK_I2C2_STR_ID = 17,	/* /< BCM_CLK_I2C2_STR_ID */

/**********************************/
/* Only available for 2153, 2157 */
XS_BCM_CLK_GE_STR_ID = 18,	    /* /< BCM_CLK_GE_STR_ID */
XS_BCM_CLK_USB_STR_ID = 19,     /* /< BCM_CLK_USB_STR_ID */
/**********************************/

/**********************************/
/* Only available for 2157, 21553 */
XS_BCM_CLK_PWM_STR_ID = 20,	    /* /< BCM_CLK_PWM_STR_ID */
XS_BCM_CLK_VCODEC_STR_ID = 22,	/* /< BCM_CLK_VCODEC_STR_ID */
XS_BCM_CLK_DMAC_STR_ID = 23,	/* /< BCM_CLK_DMAC_STR_ID */
/**********************************/

/**********************************/
/* Only available for 2157, 21553 */
XS_BCM_CLK_RNG_STR_ID = 24,	    /* /< BCM_CLK_RNG_STR_ID */
XS_BCM_CLK_LCD_STR_ID = 25,	    /* /< BCM_CLK_LCD_STR_ID */
XS_BCM_CLK_MPHI_STR_ID = 26,	/* /< BCM_CLK_MPHI_STR_ID */
/**********************************/

/**********************************/
/* Only available for 2157 */
XS_BCM_CLK_CMI_STR_ID = 27,	    /* /< BCM_CLK_CMI_STR_ID */
/**********************************/

/**********************************/
/* Only available for 21553 */
XS_BCM_CLK_CAMERARX_STR_ID = 28,   /* /< BCM_CLK_CAMERARX_STR_ID */
XS_BCM_CLK_CAMERASYS_STR_ID = 29,   /* /< BCM_CLK_CAMERASYS_STR_ID */
XS_BCM_CLK_DPE_STR_ID = 30,     /* /< BCM_CLK_DPE_STR_ID */
XS_BCM_CLK_SDIO3_STR_ID = 31,   /* /< BCM_CLK_SDIO3_STR_ID */
XS_BCM_CLK_SDRAM_STR_ID = 32,   /* /< BCM_CLK_SDRAM_STR_ID */
XS_BCM_CLK_NVSRAM_STR_ID = 33,  /* /< BCM_CLK_NVSRAM_STR_ID */
XS_BCM_CLK_SPI1_STR_ID = 34,    /* /< BCM_CLK_SPI1_STR_ID */
XS_BCM_CLK_UARTC_STR_ID = 35,   /* /< BCM_CLK_UARTC_STR_ID */
XS_BCM_CLK_MIPIDSI_STR_ID = 36, /* /< BCM_CLK_MIPIDSI_STR_ID */
/**********************************/

XS_BCM_CLK_TOTAL
} XS_BcmClk_t;

#endif /* XSAPI_CLK_DEF_H__ */

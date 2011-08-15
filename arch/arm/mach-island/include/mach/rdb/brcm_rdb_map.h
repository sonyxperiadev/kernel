/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010 Broadcom Corporation                                                         */
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


/* RDB mnemonic mapping between Hana and BigIsland */
/* Note: Hana mnemonics are used by CSP code */

/**** MEMC ****/
#define SYS_EMI_SECURE_BASE_ADDR             MEMC0_SECURE_BASE_ADDR
#define SYS_EMI_OPEN_BASE_ADDR               MEMC0_OPEN_BASE_ADDR
#define SYS_EMI_OPEN_PWRWDOG_BASE_ADDR       MEMC0_OPEN_PWRWDOG_BASE_ADDR
#define SYS_EMI_DDR3_CTL_BASE_ADDR           MEMC0_DDR3_CTL_BASE_ADDR
#define SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR  MEMC0_DDR3_PHY_ADDR_CTL_BASE_ADDR
#define SYS_EMI_DDR3_PHY_WL_0_BASE_ADDR      MEMC0_DDR3_PHY_WL_0_BASE_ADDR
#define SYS_EMI_DDR3_PHY_WL_1_BASE_ADDR      MEMC0_DDR3_PHY_WL_1_BASE_ADDR
#define VC4_EMI_SECURE_BASE_ADDR             MEMC1_SECURE_BASE_ADDR
#define VC4_EMI_OPEN_BASE_ADDR               MEMC1_OPEN_BASE_ADDR
#define VC4_EMI_OPEN_PWRWDOG_BASE_ADDR       MEMC1_OPEN_PWRWDOG_BASE_ADDR
#define VC4_EMI_DDR3_CTL_BASE_ADDR           MEMC1_OPEN_APHY_BASE_ADDR
#define VC4_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR  MEMC1_OPEN_DPHY_BASE_ADDR


/**** SSP missing instances ****/
#define SSP1_BASE_ADDR                       SSP4_BASE_ADDR
#define SSP5_BASE_ADDR                       SSP4_BASE_ADDR
#define SSP6_BASE_ADDR                       SSP4_BASE_ADDR


/**** GPIO ****/
#define CHIPREG_GPIO_0_HYS_EN_MASK           CHIPREG_GPIO_0_HYS_EN_GPIO_0_MASK
#define CHIPREG_GPIO_0_HYS_EN_SHIFT          CHIPREG_GPIO_0_HYS_EN_GPIO_0_SHIFT
#define CHIPREG_GPIO_0_SRC_MASK              CHIPREG_GPIO_0_SRC_GPIO_0_MASK
#define CHIPREG_GPIO_0_SRC_SHIFT             CHIPREG_GPIO_0_SRC_GPIO_0_SHIFT
#define CHIPREG_GPIO_0_PUP_SHIFT             CHIPREG_GPIO_0_PUP_GPIO_0_SHIFT
#define CHIPREG_GPIO_0_PUP_MASK              CHIPREG_GPIO_0_PUP_GPIO_0_MASK
#define CHIPREG_GPIO_0_PDN_MASK              CHIPREG_GPIO_0_PDN_GPIO_0_MASK
#define CHIPREG_GPIO_0_SEL_MASK              CHIPREG_GPIO_0_SEL_2_0_MASK
#define CHIPREG_GPIO_0_SEL_SHIFT             CHIPREG_GPIO_0_SEL_2_0_SHIFT


/**** VC ****/
#define CHIPREG_VC_CAM1_SCL_PUP_SHIFT        CHIPREG_VC_CAM1_SCL_PUP_VC_CAM1_SCL_SHIFT
#define CHIPREG_VC_CAM1_SCL_PUP_MASK         CHIPREG_VC_CAM1_SCL_PUP_VC_CAM1_SCL_MASK
        
#define CHIPREG_VC_CAM1_SCL_SRC_SHIFT        CHIPREG_VC_CAM1_SCL_SRC_VC_CAM1_SCL_SHIFT
#define CHIPREG_VC_CAM1_SCL_SRC_MASK         CHIPREG_VC_CAM1_SCL_SRC_VC_CAM1_SCL_MASK
        
#define CHIPREG_VC_CAM1_SCL_SEL_SHIFT        CHIPREG_VC_CAM1_SCL_MODE_VC_CAM1_SCL_SHIFT
#define CHIPREG_VC_CAM1_SCL_SEL_MASK         CHIPREG_VC_CAM1_SCL_MODE_VC_CAM1_SCL_MASK
        
#define CHIPREG_VC_CAM1_SCL_IND_SHIFT        CHIPREG_VC_CAM1_SCL_IND_VC_CAM1_SCL_SHIFT
#define CHIPREG_VC_CAM1_SCL_IND_MASK         CHIPREG_VC_CAM1_SCL_IND_VC_CAM1_SCL_MASK


/**** SDIO ****/
#define CHIPREG_SDIO2_DATA_3_PUP_SHIFT       CHIPREG_SDIO2_DATA_3_PUP_SDIO2_DATA_3_SHIFT
#define CHIPREG_SDIO2_DATA_3_PUP_MASK        CHIPREG_SDIO2_DATA_3_PUP_SDIO2_DATA_3_MASK

/**** RTC ****/
#define RTC_SET_DIV_OFFSET                          0x00000000
#define RTC_SEC_0_OFFSET                            0x00000004
#define RTC_CTRL_OFFSET                             0x00000008
#define RTC_PER_OFFSET                              0x0000000C
#define RTC_MATCH_OFFSET                            0x00000010
#define RTC_MATCH_RTC_MATCH_MASK                    0x0000FFFF
#define RTC_CLR_INT_OFFSET                          0x00000014
#define RTC_INT_STS_OFFSET                          0x00000018
#define RTC_INT_ENABLE_OFFSET                       0x0000001C
#define RTC_RESET_ACCESS_OFFSET                     0x00000020
#define RTC_MTC_CTRL_OFFSET                         0x0000002C
#define RTC_MTC_CTRL_RTC_MT_CTR_LOCK_MASK           0x00000001
#define RTC_MTC_CTRL_RTC_MT_CTR_INCR_MASK           0x00000002
#define RTC_MTC_CTRL_RTC_MT_CTR_INCR_AMT_MASK       0x00FFFF00
#define RTC_MTC_CTRL_RTC_MT_CTR_INCR_AMT_SHIFT      8
#define RTC_MTC_MSB_OFFSET                          0x00000028
#define RTC_MTC_LSB_OFFSET                          0x00000024
#define RTC_CTRL_RTC_LOCK_MASK                      0x00000002
#define RTC_CTRL_RTC_STOP_MASK                      0x00000001

#define RTC_BASE_ADDR   BBL_RTC_BASE_ADDR

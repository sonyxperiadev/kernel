/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011 Broadcom Corporation                                                         */
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
#ifndef RDB_FIXUPS_H
#define RDB_FIXUPS_H

/* Missing GPIO_GPCTRL definitions */
#define GPIO_GPCTR0_ITR_CMD_NO_INT                                  0x00000000
#define GPIO_GPCTR0_ITR_CMD_RISING_EDGE                             0x00000001
#define GPIO_GPCTR0_ITR_CMD_FALLING_EDGE                            0x00000002
#define GPIO_GPCTR0_ITR_CMD_BOTH_EDGE                               0x00000003

/* Missing USB OTG definitions */
#define HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK  HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_MASK

/* Missing non-secure DMAC definitions */
#define NON_DMAC_INTEN_OFFSET                                       0x00000020


/* Legacy chipregs SW strap bit definitions. Capri SW straps can be used differently
 * but these legacy definitions are maintained for backward compatibility in
 * chipregHw_inline.h
 */
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_15TO11_SHIFT                     11
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_15TO11_MASK                      0x0000F800
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_10_SHIFT                         10
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_10_MASK                          0x00000400
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_9_SHIFT                          9
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_9_MASK                           0x00000200
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_8_SHIFT                          8
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_8_MASK                           0x00000100
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_SHIFT                       6
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_7TO6_MASK                        0x000000C0
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_5TO3_SHIFT                       3
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_5TO3_MASK                        0x00000038
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_2_SHIFT                          2
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_2_MASK                           0x00000004
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_1_SHIFT                          1
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_1_MASK                           0x00000002
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_0_SHIFT                          0
#define    CHIPREG_ISLAND_STRAP_STRAP_IN_0_MASK                           0x00000001
#define    CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_SHIFT                        31
#define    CHIPREG_CHIP_SW_STRAP_SW_STRAP_EN_MASK                         0x80000000

/* Mnemonic mapping between Hana and Kona */
#define SYS_EMI_SECURE_BASE_ADDR             MEMC0_SECURE_BASE_ADDR
#define SYS_EMI_OPEN_BASE_ADDR               MEMC0_OPEN_BASE_ADDR
#define SYS_EMI_OPEN_PWRWDOG_BASE_ADDR       MEMC0_OPEN_PWRWDOG_BASE_ADDR
#define SYS_EMI_DDR3_CTL_BASE_ADDR           MEMC0_DDR3_CTL_BASE_ADDR
#define SYS_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR  MEMC0_DDR3_PHY_ADDR_CTL_BASE_ADDR

#define ARM_FUNNEL_BASE_ADDR                 FUNNEL_BASE_ADDR

#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_HIGH_SHIFT             31
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_HIGH_MASK              0x80000000
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_LOW_SHIFT              30
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_SW_READY_LOW_MASK               0x40000000
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_FIFO_RST_MASK            0x04000000

#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_DONT_PAUSE_SHIFT         27
#define    CPH_AADMAC_CH1_AADMAC_CR_2_CH1_AADMAC_DONT_PAUSE_MASK          0x08000000

#define    ACI_ADC_CTRL_OFFSET                                            0x000000D4
#define    ACI_ADC_CTRL_AUDIORX_VREF_PWRUP_MASK                           0x00000080
#define    ACI_ADC_CTRL_AUDIORX_BIAS_PWRUP_MASK                           0x00000040

#define    AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_SHIFT                    8
#define    AUDIOH_AUDIORX_VMIC_AUDIORX_VMIC_CTRL_MASK                     0x00000300


/* Obsolete */
#if 0
#include <mach/rdb/brcm_rdb_padctrlreg.h>

/* RDB mnemonic mapping between Hana and BigIsland */
/* Note: BigIsland mnemonics are used by CSP code  */

#define UARTB1_BASE_ADDR                     UARTB_BASE_ADDR

/* TODO: DDR3 and LPDDR2 register space does not shadow on Capri so pretty sure this is wrong */
#define VC4_EMI_DDR3_CTL_BASE_ADDR           VC4_EMI_OPEN_APHY_BASE_ADDR
#define VC4_EMI_DDR3_PHY_ADDR_CTL_BASE_ADDR  VC4_EMI_OPEN_DPHY_BASE_ADDR

#define HSOTG_CTRL_PHY_P1CTL_PLL_SUSPEND_ENABLE_MASK  HSOTG_CTRL_PHY_P1CTL_USB11_OEB_IS_TXEB_MASK
#define HSOTG_CTRL_BC11_STATUS_OFFSET                 HSOTG_CTRL_BC_STATUS_OFFSET
#define HSOTG_CTRL_BC11_STATUS_SHP_MASK               HSOTG_CTRL_BC_STATUS_SDP_MASK
#define HSOTG_CTRL_BC11_CFG_OFFSET                    HSOTG_CTRL_BC_CFG_OFFSET
#define HSOTG_CTRL_BC11_CFG_BC11_OVWR_KEY_MASK        HSOTG_CTRL_BC_CFG_BC_OVWR_KEY_MASK
#define HSOTG_CTRL_BC11_CFG_SW_OVWR_EN_MASK           HSOTG_CTRL_BC_CFG_SW_OVWR_EN_MASK
#define HSOTG_CTRL_BC11_CFG_BC11_OVWR_SET_M0_MASK     HSOTG_CTRL_BC_CFG_BC_OVWR_SET_M0_MASK
#define HSOTG_CTRL_BC11_CFG_BC11_OVWR_SET_P0_MASK     HSOTG_CTRL_BC_CFG_BC_OVWR_SET_P0_MASK

#define    CHIPREG_VC_CAM1_SCL_PUP_SHIFT     PADCTRLREG_VC_CAM1_SCL_PUP_2_0_SHIFT
#define    CHIPREG_VC_CAM1_SCL_PUP_MASK      PADCTRLREG_VC_CAM1_SCL_PUP_2_0_MASK

/* Missing definitions */
#define NVSRAM_BASE_ADDR          0x34003000 /* brcm_rdb_nvsram_axi.h */
#define SSP1_BASE_ADDR            0x35028000 /* brcm_rdb_sspil.h */
#define D1W_BASE_ADDR             0x3E015000 /* brcm_rdb_d1w.h */
#define VINTC_APB_BASE_ADDR       0x38002000 /* brcm_rdb_vintc_apb.h */
#define AXITRACE21_BASE_ADDR      0x3800D000 /* brcm_rdb_axitp1.h */

/* BBL no longer exists in Capri, so this will need to be removed/changed */
#define BBL_WATCHDOG_BASE_ADDR    0x00004000 /* brcm_rdb_secwatchdog.h */
#define BBL_BASE_ADDR             0x3E01B000 /* brcm_rdb_bbl_apb.h */

/* Capri does not support D1W, so this can be removed once the build is cleaned-up to remove D1W from testapps */
#define    KONATZCFG_KONA_SLV_APB2_TZPROT_DALLAS_1_WIRE_TZPROT_MASK       0x00000020

#define    CHIPREG_VC_CAM1_SCL_SEL_SHIFT                                  0
#define    CHIPREG_VC_CAM1_SCL_SEL_MASK                                   0x00000007

#define EHCI_MODE_OFFSET                                                  0x00008000


#define KEYPAD_KPIOC_OFFSET                                               0x00000008
#define    KEYPAD_KPIOC_ROWOCONTRL_SHIFT                                  16

#define    CHIPREG_SDIO2_DATA_3_PUP_MASK                                  0x00000020

#define KONATZCFG_KONA_PERIPH_AHB2_TZPROT_OFFSET                          0x00000204
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_UNMG_NAND_TZPROT_MASK        0x00000002
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO4_TZPROT_MASK            0x00000800
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO3_TZPROT_MASK            0x00000400
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO2_TZPROT_MASK            0x00000200
#define    KONATZCFG_KONA_PERIPH_AHB2_TZPROT_SDIO1_TZPROT_MASK            0x00000100

#define    IKPS_CLK_MGR_REG_DIV_TRIG_SSP0_AUDIO_TRIGGER_MASK              0x00001000
#define    IKPS_CLK_MGR_REG_DIV_TRIG_SSP2_AUDIO_TRIGGER_MASK              0x00004000
#define    KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP3_AUDIO_TRIGGER_MASK        0x00010000
#define    KHUB_CLK_MGR_REG_PERIPH_SEG_TRG_SSP4_AUDIO_TRIGGER_MASK        0x00080000

#define CHIPREG_GPIO_0_OFFSET                   PADCTRLREG_GPIO00_OFFSET
#define CHIPREG_GPIO_0_TYPE                     PADCTRLREG_GPIO00_TYPE
#define CHIPREG_GPIO_0_RESERVED_MASK            PADCTRLREG_GPIO00_RESERVED_MASK
#define    CHIPREG_GPIO_0_PINSEL_2_0_SHIFT         PADCTRLREG_GPIO00_PINSEL_2_0_SHIFT
#define    CHIPREG_GPIO_0_PINSEL_2_0_MASK          PADCTRLREG_GPIO00_PINSEL_2_0_MASK
#define    CHIPREG_GPIO_0_HYS_EN_SHIFT             PADCTRLREG_GPIO00_HYS_EN_SHIFT
#define    CHIPREG_GPIO_0_HYS_EN_MASK              PADCTRLREG_GPIO00_HYS_EN_MASK
#define    CHIPREG_GPIO_0_PDN_SHIFT                PADCTRLREG_GPIO00_PDN_SHIFT
#define    CHIPREG_GPIO_0_PDN_MASK                 PADCTRLREG_GPIO00_PDN_MASK
#define    CHIPREG_GPIO_0_PUP_SHIFT                PADCTRLREG_GPIO00_PUP_SHIFT
#define    CHIPREG_GPIO_0_PUP_MASK                 PADCTRLREG_GPIO00_PUP_MASK
#define    CHIPREG_GPIO_0_SRC_SHIFT                PADCTRLREG_GPIO00_SRC_SHIFT
#define    CHIPREG_GPIO_0_SRC_MASK                 PADCTRLREG_GPIO00_SRC_MASK
#define    CHIPREG_GPIO_0_IND_SHIFT                PADCTRLREG_GPIO00_IND_SHIFT
#define    CHIPREG_GPIO_0_IND_MASK                 PADCTRLREG_GPIO00_IND_MASK
#define    CHIPREG_GPIO_0_SEL_SHIFT                PADCTRLREG_GPIO00_SEL_SHIFT
#define    CHIPREG_GPIO_0_SEL_MASK                 PADCTRLREG_GPIO00_SEL_MASK

#define    CHIPREG_VC_CAM1_SCL_SRC_SHIFT           PADCTRLREG_VC_CAM1_SCL_SRC_SHIFT
#define    CHIPREG_VC_CAM1_SCL_SRC_MASK            PADCTRLREG_VC_CAM1_SCL_SRC_MASK

#define    CHIPREG_VC_CAM1_SCL_IND_SHIFT           PADCTRLREG_VC_CAM1_SCL_IND_SHIFT
#define    CHIPREG_VC_CAM1_SCL_IND_MASK            PADCTRLREG_VC_CAM1_SCL_IND_MASK

#define CHIPREG_UARTB_UCTSN_OFFSET        CHIPREG_UARTB1_UCTS_OFFSET
#define CHIPREG_UARTB_URTSN_OFFSET        CHIPREG_UARTB1_URTS_OFFSET
#define CHIPREG_UARTB_UTXD_OFFSET         CHIPREG_UARTB1_UTXD_OFFSET
#define CHIPREG_UARTB_URXD_OFFSET         CHIPREG_UARTB1_URXD_OFFSET

/**** DMACHW ****/
#define  DMACHW_RDB_MAP_MODULE_0_NUM_OF_CHANNELS            4 /**< Number of Channels supported by Module 0 */
#define  DMACHW_RDB_MAP_MODULE_1_NUM_OF_CHANNELS            0 /**< Number of Channels supported by Module 1 */

#define  DMACHW_RDB_MAP_REG_INT_RAW_BASE_CHANNEL(module)    8 /**< Used for calculating register offset */

#endif

#endif /* RDB_FIXUPS_H */

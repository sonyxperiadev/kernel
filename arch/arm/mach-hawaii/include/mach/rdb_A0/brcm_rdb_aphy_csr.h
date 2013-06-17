/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2012  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_APHY_CSR_H__
#define __BRCM_RDB_APHY_CSR_H__

#define APHY_CSR_ADDR_REV_ID_OFFSET                                       0x00000000
#define APHY_CSR_ADDR_REV_ID_TYPE                                         UInt32
#define APHY_CSR_ADDR_REV_ID_RESERVED_MASK                                0xFFFF0000
#define    APHY_CSR_ADDR_REV_ID_MAJOR_SHIFT                               8
#define    APHY_CSR_ADDR_REV_ID_MAJOR_MASK                                0x0000FF00
#define    APHY_CSR_ADDR_REV_ID_MINOR_SHIFT                               0
#define    APHY_CSR_ADDR_REV_ID_MINOR_MASK                                0x000000FF

#define APHY_CSR_GLOBAL_ADDR_DLL_RESET_OFFSET                             0x00000004
#define APHY_CSR_GLOBAL_ADDR_DLL_RESET_TYPE                               UInt32
#define APHY_CSR_GLOBAL_ADDR_DLL_RESET_RESERVED_MASK                      0xFFFFFFFE
#define    APHY_CSR_GLOBAL_ADDR_DLL_RESET_DLL_RESET_SHIFT                 0
#define    APHY_CSR_GLOBAL_ADDR_DLL_RESET_DLL_RESET_MASK                  0x00000001

#define APHY_CSR_GLOBAL_ADDR_DLL_RECALIBRATE_OFFSET                       0x00000008
#define APHY_CSR_GLOBAL_ADDR_DLL_RECALIBRATE_TYPE                         UInt32
#define APHY_CSR_GLOBAL_ADDR_DLL_RECALIBRATE_RESERVED_MASK                0xFFFFFFFE
#define    APHY_CSR_GLOBAL_ADDR_DLL_RECALIBRATE_DLL_RECALIBRATE_SHIFT     0
#define    APHY_CSR_GLOBAL_ADDR_DLL_RECALIBRATE_DLL_RECALIBRATE_MASK      0x00000001

#define APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_OFFSET                             0x0000000C
#define APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_TYPE                               UInt32
#define APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_RESERVED_MASK                      0xFFFFFEFC
#define    APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_DLL_RECALIBRATION_ENABLE_SHIFT  8
#define    APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_DLL_RECALIBRATION_ENABLE_MASK   0x00000100
#define    APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_DLL_PHASE_SEL_SHIFT             0
#define    APHY_CSR_GLOBAL_ADDR_DLL_CNTRL_DLL_PHASE_SEL_MASK              0x00000003

#define APHY_CSR_GLOBAL_ADDR_DLL_PHASE_LOAD_VALUE_OFFSET                  0x00000010
#define APHY_CSR_GLOBAL_ADDR_DLL_PHASE_LOAD_VALUE_TYPE                    UInt32
#define APHY_CSR_GLOBAL_ADDR_DLL_PHASE_LOAD_VALUE_RESERVED_MASK           0xFFFFFFC0
#define    APHY_CSR_GLOBAL_ADDR_DLL_PHASE_LOAD_VALUE_DLL_RELOAD_PHASE_SHIFT 0
#define    APHY_CSR_GLOBAL_ADDR_DLL_PHASE_LOAD_VALUE_DLL_RELOAD_PHASE_MASK 0x0000003F

#define APHY_CSR_ADDR_MASTER_DLL_OUTPUT_OFFSET                            0x00000014
#define APHY_CSR_ADDR_MASTER_DLL_OUTPUT_TYPE                              UInt32
#define APHY_CSR_ADDR_MASTER_DLL_OUTPUT_RESERVED_MASK                     0xFFFFC0C0
#define    APHY_CSR_ADDR_MASTER_DLL_OUTPUT_ADDR_MASTER_PHASE_P_SHIFT      8
#define    APHY_CSR_ADDR_MASTER_DLL_OUTPUT_ADDR_MASTER_PHASE_P_MASK       0x00003F00
#define    APHY_CSR_ADDR_MASTER_DLL_OUTPUT_ADDR_MASTER_PHASE_N_SHIFT      0
#define    APHY_CSR_ADDR_MASTER_DLL_OUTPUT_ADDR_MASTER_PHASE_N_MASK       0x0000003F

#define APHY_CSR_ADDR_SLAVE_DLL_OFFSET_OFFSET                             0x00000018
#define APHY_CSR_ADDR_SLAVE_DLL_OFFSET_TYPE                               UInt32
#define APHY_CSR_ADDR_SLAVE_DLL_OFFSET_RESERVED_MASK                      0xFFFFFF80
#define    APHY_CSR_ADDR_SLAVE_DLL_OFFSET_ADDR_SLAVE_OFFSET_SHIFT         0
#define    APHY_CSR_ADDR_SLAVE_DLL_OFFSET_ADDR_SLAVE_OFFSET_MASK          0x0000007F

#define APHY_CSR_GLOBAL_ADDR_MASTER_DLL_BYPASS_ENABLE_OFFSET              0x0000001C
#define APHY_CSR_GLOBAL_ADDR_MASTER_DLL_BYPASS_ENABLE_TYPE                UInt32
#define APHY_CSR_GLOBAL_ADDR_MASTER_DLL_BYPASS_ENABLE_RESERVED_MASK       0xFFFFFFFE
#define    APHY_CSR_GLOBAL_ADDR_MASTER_DLL_BYPASS_ENABLE_ADDR_MASTER_DLL_BYPASS_SHIFT 0
#define    APHY_CSR_GLOBAL_ADDR_MASTER_DLL_BYPASS_ENABLE_ADDR_MASTER_DLL_BYPASS_MASK 0x00000001

#define APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_OFFSET                       0x00000020
#define APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_TYPE                         UInt32
#define APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_RESERVED_MASK                0xFFFFFFFC
#define    APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_ADDR_MASTER_DLL_LOCK_P_SHIFT 1
#define    APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_ADDR_MASTER_DLL_LOCK_P_MASK 0x00000002
#define    APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_ADDR_MASTER_DLL_LOCK_N_SHIFT 0
#define    APHY_CSR_GLOBAL_ADDR_DLL_LOCK_STATUS_ADDR_MASTER_DLL_LOCK_N_MASK 0x00000001

#define APHY_CSR_DDR_PLL_GLOBAL_RESET_OFFSET                              0x00000024
#define APHY_CSR_DDR_PLL_GLOBAL_RESET_TYPE                                UInt32
#define APHY_CSR_DDR_PLL_GLOBAL_RESET_RESERVED_MASK                       0xFFFFFFFE
#define    APHY_CSR_DDR_PLL_GLOBAL_RESET_PLL_RESETB_SHIFT                 0
#define    APHY_CSR_DDR_PLL_GLOBAL_RESET_PLL_RESETB_MASK                  0x00000001

#define APHY_CSR_DDR_PLL_POST_DIVIDER_RESET_OFFSET                        0x00000028
#define APHY_CSR_DDR_PLL_POST_DIVIDER_RESET_TYPE                          UInt32
#define APHY_CSR_DDR_PLL_POST_DIVIDER_RESET_RESERVED_MASK                 0xFFFFFFFE
#define    APHY_CSR_DDR_PLL_POST_DIVIDER_RESET_POST_RESETB_SHIFT          0
#define    APHY_CSR_DDR_PLL_POST_DIVIDER_RESET_POST_RESETB_MASK           0x00000001

#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_OFFSET                           0x0000002C
#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_TYPE                             UInt32
#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_RESERVED_MASK                    0xFFF0FC00
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_SHIFT                    16
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_PDIV_MASK                     0x000F0000
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_SHIFT                0
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL0_NDIV_INT_MASK                 0x000003FF

#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_OFFSET                           0x00000030
#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_TYPE                             UInt32
#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_RESERVED_MASK                    0xFFF00000
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_SHIFT               0
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL1_NDIV_FRAC_MASK                0x000FFFFF

#define APHY_CSR_DDR_PLL_MDIV_VALUE_OFFSET                                0x00000034
#define APHY_CSR_DDR_PLL_MDIV_VALUE_TYPE                                  UInt32
#define APHY_CSR_DDR_PLL_MDIV_VALUE_RESERVED_MASK                         0xFFFF0000
#define    APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV2_SHIFT                        8
#define    APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV2_MASK                         0x0000FF00
#define    APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_SHIFT                         0
#define    APHY_CSR_DDR_PLL_MDIV_VALUE_MDIV_MASK                          0x000000FF

#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_OFFSET                              0x00000038
#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_TYPE                                UInt32
#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_RESERVED_MASK                       0xFFE00000
#define    APHY_CSR_DDR_PLL_CONFIG_CNTRL_PLL_CFG_CNTRL_SHIFT              0
#define    APHY_CSR_DDR_PLL_CONFIG_CNTRL_PLL_CFG_CNTRL_MASK               0x001FFFFF

#define APHY_CSR_DDR_PLL_MISC_CNTRL_OFFSET                                0x0000003C
#define APHY_CSR_DDR_PLL_MISC_CNTRL_TYPE                                  UInt32
#define APHY_CSR_DDR_PLL_MISC_CNTRL_RESERVED_MASK                         0xFC000088
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_EIGHT_PHASE_EN_SHIFT               25
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_EIGHT_PHASE_EN_MASK                0x02000000
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_FB_PHASE_EN_SHIFT                  24
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_FB_PHASE_EN_MASK                   0x01000000
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_FB_PHASE_OFFSET_SHIFT              12
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_FB_PHASE_OFFSET_MASK               0x00FFF000
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_KP_SHIFT                           8
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_KP_MASK                            0x00000F00
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_KI_SHIFT                           4
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_KI_MASK                            0x00000070
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_KA_SHIFT                           0
#define    APHY_CSR_DDR_PLL_MISC_CNTRL_KA_MASK                            0x00000007

#define APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_OFFSET                       0x00000040
#define APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_TYPE                         UInt32
#define APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_RESERVED_MASK                0xFC00000E
#define    APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_SSC_LIMIT_SHIFT           4
#define    APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_SSC_LIMIT_MASK            0x03FFFFF0
#define    APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_SSC_MODE_SHIFT            0
#define    APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL0_SSC_MODE_MASK             0x00000001

#define APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL1_OFFSET                       0x00000044
#define APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL1_TYPE                         UInt32
#define APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL1_RESERVED_MASK                0xFFFF0000
#define    APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL1_SSC_STEP_SHIFT            0
#define    APHY_CSR_DDR_PLL_SPREAD_SPECT_CNTRL1_SSC_STEP_MASK             0x0000FFFF

#define APHY_CSR_DDR_PLL_LOCK_STATUS_OFFSET                               0x00000048
#define APHY_CSR_DDR_PLL_LOCK_STATUS_TYPE                                 UInt32
#define APHY_CSR_DDR_PLL_LOCK_STATUS_RESERVED_MASK                        0xFFFCF000
#define    APHY_CSR_DDR_PLL_LOCK_STATUS_LOCK_LOST_SHIFT                   17
#define    APHY_CSR_DDR_PLL_LOCK_STATUS_LOCK_LOST_MASK                    0x00020000
#define    APHY_CSR_DDR_PLL_LOCK_STATUS_LOCK_BIT_SHIFT                    16
#define    APHY_CSR_DDR_PLL_LOCK_STATUS_LOCK_BIT_MASK                     0x00010000
#define    APHY_CSR_DDR_PLL_LOCK_STATUS_STAT_OUT_SHIFT                    0
#define    APHY_CSR_DDR_PLL_LOCK_STATUS_STAT_OUT_MASK                     0x00000FFF

#define APHY_CSR_DDR_PLL_HOLD_CH_OFFSET                                   0x0000004C
#define APHY_CSR_DDR_PLL_HOLD_CH_TYPE                                     UInt32
#define APHY_CSR_DDR_PLL_HOLD_CH_RESERVED_MASK                            0xFFFFFFFE
#define    APHY_CSR_DDR_PLL_HOLD_CH_HOLD_CH_SHIFT                         0
#define    APHY_CSR_DDR_PLL_HOLD_CH_HOLD_CH_MASK                          0x00000001

#define APHY_CSR_DDR_PLL_ENABLE_CH_OFFSET                                 0x00000050
#define APHY_CSR_DDR_PLL_ENABLE_CH_TYPE                                   UInt32
#define APHY_CSR_DDR_PLL_ENABLE_CH_RESERVED_MASK                          0xFFFFFFFE
#define    APHY_CSR_DDR_PLL_ENABLE_CH_ENABLEB_CH_SHIFT                    0
#define    APHY_CSR_DDR_PLL_ENABLE_CH_ENABLEB_CH_MASK                     0x00000001

#define APHY_CSR_DDR_PLL_BYPASS_OFFSET                                    0x00000054
#define APHY_CSR_DDR_PLL_BYPASS_TYPE                                      UInt32
#define APHY_CSR_DDR_PLL_BYPASS_RESERVED_MASK                             0xFFFFFFFE
#define    APHY_CSR_DDR_PLL_BYPASS_PLL_BYPASS_SHIFT                       0
#define    APHY_CSR_DDR_PLL_BYPASS_PLL_BYPASS_MASK                        0x00000001

#define APHY_CSR_DDR_PLL_PWRDWN_OFFSET                                    0x00000058
#define APHY_CSR_DDR_PLL_PWRDWN_TYPE                                      UInt32
#define APHY_CSR_DDR_PLL_PWRDWN_RESERVED_MASK                             0xFFFFFFFE
#define    APHY_CSR_DDR_PLL_PWRDWN_PLL_PWRDWN_SHIFT                       0
#define    APHY_CSR_DDR_PLL_PWRDWN_PLL_PWRDWN_MASK                        0x00000001

#define APHY_CSR_DDR_PLL_CH0_DESKEW_CNTRL_OFFSET                          0x0000005C
#define APHY_CSR_DDR_PLL_CH0_DESKEW_CNTRL_TYPE                            UInt32
#define APHY_CSR_DDR_PLL_CH0_DESKEW_CNTRL_RESERVED_MASK                   0xFFFFFFC0
#define    APHY_CSR_DDR_PLL_CH0_DESKEW_CNTRL_DDR_PLL_CH0_MDEL_SHIFT       0
#define    APHY_CSR_DDR_PLL_CH0_DESKEW_CNTRL_DDR_PLL_CH0_MDEL_MASK        0x0000003F

#define APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_OFFSET                          0x00000060
#define APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_TYPE                            UInt32
#define APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_RESERVED_MASK                   0xFFFC0E0C
#define    APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_DDR_PLL_CH1_MDEL_SHIFT       12
#define    APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_DDR_PLL_CH1_MDEL_MASK        0x0003F000
#define    APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_PLL_CH1_FBOFFS_SHIFT         4
#define    APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_PLL_CH1_FBOFFS_MASK          0x000001F0
#define    APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_PLL_CH1_FBMODE_SHIFT         0
#define    APHY_CSR_DDR_PLL_CH1_DESKEW_CNTRL_PLL_CH1_FBMODE_MASK          0x00000003

#define APHY_CSR_DDR_PLL_DESKEW_STATUS_OFFSET                             0x00000064
#define APHY_CSR_DDR_PLL_DESKEW_STATUS_TYPE                               UInt32
#define APHY_CSR_DDR_PLL_DESKEW_STATUS_RESERVED_MASK                      0xFFFFFEC0
#define    APHY_CSR_DDR_PLL_DESKEW_STATUS_CH1_FBCLK_LOCK_SHIFT            8
#define    APHY_CSR_DDR_PLL_DESKEW_STATUS_CH1_FBCLK_LOCK_MASK             0x00000100
#define    APHY_CSR_DDR_PLL_DESKEW_STATUS_CH1_MDEL_STAT_SHIFT             0
#define    APHY_CSR_DDR_PLL_DESKEW_STATUS_CH1_MDEL_STAT_MASK              0x0000003F

#define APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_OFFSET                           0x00000068
#define APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_TYPE                             UInt32
#define APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_RESERVED_MASK                    0xFFFFF888
#define    APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_CLK_DRV_SHIFT                 8
#define    APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_CLK_DRV_MASK                  0x00000700
#define    APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_CA_DRV_SHIFT                  4
#define    APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_CA_DRV_MASK                   0x00000070
#define    APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_SLEW_SHIFT                    0
#define    APHY_CSR_ADDR_PAD_DRV_SLEW_CNTRL_SLEW_MASK                     0x00000007

#define APHY_CSR_ADDR_PAD_MISC_CNTRL_OFFSET                               0x0000006C
#define APHY_CSR_ADDR_PAD_MISC_CNTRL_TYPE                                 UInt32
#define APHY_CSR_ADDR_PAD_MISC_CNTRL_RESERVED_MASK                        0xFFFFFFC0
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_PDN_CLK_SHIFT                     5
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_PDN_CLK_MASK                      0x00000020
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_RT_EN_SHIFT                       4
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_RT_EN_MASK                        0x00000010
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_RT60_SHIFT                        3
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_RT60_MASK                         0x00000008
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_LPWR_RX_SHIFT                     2
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_LPWR_RX_MASK                      0x00000004
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_HCURR_SEL_SHIFT                   1
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_HCURR_SEL_MASK                    0x00000002
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_PDN_SHIFT                         0
#define    APHY_CSR_ADDR_PAD_MISC_CNTRL_PDN_MASK                          0x00000001

#define APHY_CSR_ADDR_PVT_COMP_CNTRL_OFFSET                               0x00000070
#define APHY_CSR_ADDR_PVT_COMP_CNTRL_TYPE                                 UInt32
#define APHY_CSR_ADDR_PVT_COMP_CNTRL_RESERVED_MASK                        0xFFFFFFCE
#define    APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_OFFSET_OP_SHIFT          5
#define    APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_OFFSET_OP_MASK           0x00000020
#define    APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_OFFSET_EN_SHIFT          4
#define    APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_OFFSET_EN_MASK           0x00000010
#define    APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_EN_SHIFT                 0
#define    APHY_CSR_ADDR_PVT_COMP_CNTRL_PVT_COMP_EN_MASK                  0x00000001

#define APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_OFFSET                      0x00000074
#define APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_TYPE                        UInt32
#define APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_RESERVED_MASK               0xFFFFF08E
#define    APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_PVT_OVERRIDE_VAL_SHIFT   8
#define    APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_PVT_OVERRIDE_VAL_MASK    0x00000F00
#define    APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_PVT_OVERRIDE_MODE_SHIFT  4
#define    APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_PVT_OVERRIDE_MODE_MASK   0x00000070
#define    APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_PVT_OVERRIDE_EN_SHIFT    0
#define    APHY_CSR_ADDR_PVT_COMP_OVERRIDE_CNTRL_PVT_OVERRIDE_EN_MASK     0x00000001

#define APHY_CSR_ADDR_PVT_COMP_STATUS_OFFSET                              0x00000078
#define APHY_CSR_ADDR_PVT_COMP_STATUS_TYPE                                UInt32
#define APHY_CSR_ADDR_PVT_COMP_STATUS_RESERVED_MASK                       0xFFFFFFFC
#define    APHY_CSR_ADDR_PVT_COMP_STATUS_PVT_COMP_DONE_SHIFT              1
#define    APHY_CSR_ADDR_PVT_COMP_STATUS_PVT_COMP_DONE_MASK               0x00000002
#define    APHY_CSR_ADDR_PVT_COMP_STATUS_PVT_COMP_ACK_SHIFT               0
#define    APHY_CSR_ADDR_PVT_COMP_STATUS_PVT_COMP_ACK_MASK                0x00000001

#define APHY_CSR_ADDR_PVT_COMP_DEBUG_OFFSET                               0x0000007C
#define APHY_CSR_ADDR_PVT_COMP_DEBUG_TYPE                                 UInt32
#define APHY_CSR_ADDR_PVT_COMP_DEBUG_RESERVED_MASK                        0xFFF088C0
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_NCOMP_ENB_SHIFT               19
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_NCOMP_ENB_MASK                0x00080000
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_PCOMP_ENB_SHIFT               18
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_PCOMP_ENB_MASK                0x00040000
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_NDONE_SHIFT                   17
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_NDONE_MASK                    0x00020000
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_PDONE_SHIFT                   16
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_PDONE_MASK                    0x00010000
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_NCOMP_CODE_SHIFT              12
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_NCOMP_CODE_MASK               0x00007000
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_PCOMP_CODE_SHIFT              8
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_PCOMP_CODE_MASK               0x00000700
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_COMP_ERR_SHIFT                0
#define    APHY_CSR_ADDR_PVT_COMP_DEBUG_PVT_COMP_ERR_MASK                 0x0000003F

#define APHY_CSR_DISABLE_SOFT_DDR_PLL_CONTROL_OFFSET                      0x00000084
#define APHY_CSR_DISABLE_SOFT_DDR_PLL_CONTROL_TYPE                        UInt32
#define APHY_CSR_DISABLE_SOFT_DDR_PLL_CONTROL_RESERVED_MASK               0xFFFFFFFE
#define    APHY_CSR_DISABLE_SOFT_DDR_PLL_CONTROL_DISABLE_SOFT_PLL_CNTRL_SHIFT 0
#define    APHY_CSR_DISABLE_SOFT_DDR_PLL_CONTROL_DISABLE_SOFT_PLL_CNTRL_MASK 0x00000001

#define APHY_CSR_PHY_BIST_CA_CRC_SPR_OFFSET                               0x00000088
#define APHY_CSR_PHY_BIST_CA_CRC_SPR_TYPE                                 UInt32
#define APHY_CSR_PHY_BIST_CA_CRC_SPR_RESERVED_MASK                        0x00000000
#define    APHY_CSR_PHY_BIST_CA_CRC_SPR_CA_RISE_CRC_SHIFT                 16
#define    APHY_CSR_PHY_BIST_CA_CRC_SPR_CA_RISE_CRC_MASK                  0xFFFF0000
#define    APHY_CSR_PHY_BIST_CA_CRC_SPR_CA_FALL_CRC_SHIFT                 0
#define    APHY_CSR_PHY_BIST_CA_CRC_SPR_CA_FALL_CRC_MASK                  0x0000FFFF

#define APHY_CSR_ADDR_SPR0_RW_OFFSET                                      0x0000008C
#define APHY_CSR_ADDR_SPR0_RW_TYPE                                        UInt32
#define APHY_CSR_ADDR_SPR0_RW_RESERVED_MASK                               0x00000000
#define    APHY_CSR_ADDR_SPR0_RW_SPR0_RW_SHIFT                            0
#define    APHY_CSR_ADDR_SPR0_RW_SPR0_RW_MASK                             0xFFFFFFFF

#define APHY_CSR_ADDR_SPR1_RW_OFFSET                                      0x00000090
#define APHY_CSR_ADDR_SPR1_RW_TYPE                                        UInt32
#define APHY_CSR_ADDR_SPR1_RW_RESERVED_MASK                               0x00000000
#define    APHY_CSR_ADDR_SPR1_RW_SPR1_RW_SHIFT                            0
#define    APHY_CSR_ADDR_SPR1_RW_SPR1_RW_MASK                             0xFFFFFFFF

#define APHY_CSR_ADDR_SPR_RO_OFFSET                                       0x00000094
#define APHY_CSR_ADDR_SPR_RO_TYPE                                         UInt32
#define APHY_CSR_ADDR_SPR_RO_RESERVED_MASK                                0x00000000
#define    APHY_CSR_ADDR_SPR_RO_SPR_RO_SHIFT                              0
#define    APHY_CSR_ADDR_SPR_RO_SPR_RO_MASK                               0xFFFFFFFF

#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_OFFSET                           0x00000098
#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_TYPE                             UInt32
#define APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_RESERVED_MASK                    0xC0000000
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_NDIV_INT_2_SHIFT              20
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_NDIV_INT_2_MASK               0x3FF00000
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_NDIV_FRAC_2_SHIFT             0
#define    APHY_CSR_DDR_PLL_VCO_FREQ_CNTRL2_NDIV_FRAC_2_MASK              0x000FFFFF

#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_0_OFFSET                       0x0000009C
#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_0_TYPE                         UInt32
#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_0_RESERVED_MASK                0x00000000
#define    APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_0_PLL_CFG_CNTRL_28NM_0_SHIFT 0
#define    APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_0_PLL_CFG_CNTRL_28NM_0_MASK 0xFFFFFFFF

#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_1_OFFSET                       0x00000100
#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_1_TYPE                         UInt32
#define APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_1_RESERVED_MASK                0x00000000
#define    APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_1_PLL_CFG_CNTRL_28NM_1_SHIFT 0
#define    APHY_CSR_DDR_PLL_CONFIG_CNTRL_28NM_1_PLL_CFG_CNTRL_28NM_1_MASK 0xFFFFFFFF

#endif /* __BRCM_RDB_APHY_CSR_H__ */



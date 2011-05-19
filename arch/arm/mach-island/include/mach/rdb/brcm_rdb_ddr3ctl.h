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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_DDR3CTL_H__
#define __BRCM_RDB_DDR3CTL_H__

#define DDR3CTL_GLUE_CONFIG_OFFSET                                        0x00000000
#define DDR3CTL_GLUE_CONFIG_TYPE                                          UInt32
#define DDR3CTL_GLUE_CONFIG_RESERVED_MASK                                 0xFFFFFFFE
#define    DDR3CTL_GLUE_CONFIG_SFTCTL_UPD_VDL_SHIFT                       0
#define    DDR3CTL_GLUE_CONFIG_SFTCTL_UPD_VDL_MASK                        0x00000001

#define DDR3CTL_PHY_CTL_OFFSET                                            0x00000004
#define DDR3CTL_PHY_CTL_TYPE                                              UInt32
#define DDR3CTL_PHY_CTL_RESERVED_MASK                                     0xFFFFFFFE
#define    DDR3CTL_PHY_CTL_PHY_UPD_VDL_SHIFT                              0
#define    DDR3CTL_PHY_CTL_PHY_UPD_VDL_MASK                               0x00000001

#define DDR3CTL_PHY_CONFIG_OFFSET                                         0x00000008
#define DDR3CTL_PHY_CONFIG_TYPE                                           UInt32
#define DDR3CTL_PHY_CONFIG_RESERVED_MASK                                  0xE0000000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_STANDBY_EXIT_L_SHIFT                 28
#define    DDR3CTL_PHY_CONFIG_MC2IOB_STANDBY_EXIT_L_MASK                  0x10000000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_USE_4X_CLOCKING_SHIFT                27
#define    DDR3CTL_PHY_CONFIG_MC2IOB_USE_4X_CLOCKING_MASK                 0x08000000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_RST_N_SFTCTL_SHIFT               26
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_RST_N_SFTCTL_MASK                0x04000000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_CK_DIS_SHIFT                     25
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_CK_DIS_MASK                      0x02000000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_USE_DYN_VDL_SHIFT                24
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_USE_DYN_VDL_MASK                 0x01000000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_IDLE_SHIFT                       23
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_IDLE_MASK                        0x00800000
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_DDR_MHZ_SHIFT                    11
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_DDR_MHZ_MASK                     0x007FF800
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_STANDBY_SHIFT                    10
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_STANDBY_MASK                     0x00000400
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_PLL_PWRDN_SHIFT                  9
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_PLL_PWRDN_MASK                   0x00000200
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_AUX_IS_ODT_SHIFT                 6
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_AUX_IS_ODT_MASK                  0x000001C0
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_AUX_IS_CS_SHIFT                  3
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_AUX_IS_CS_MASK                   0x00000038
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_AUX_IS_AD_SHIFT                  0
#define    DDR3CTL_PHY_CONFIG_MC2IOB_PHY_AUX_IS_AD_MASK                   0x00000007

#define DDR3CTL_PHY_STRAPS_CTL1_OFFSET                                    0x0000000C
#define DDR3CTL_PHY_STRAPS_CTL1_TYPE                                      UInt32
#define DDR3CTL_PHY_STRAPS_CTL1_RESERVED_MASK                             0xF0000000
#define    DDR3CTL_PHY_STRAPS_CTL1_MHZ_SHIFT                              16
#define    DDR3CTL_PHY_STRAPS_CTL1_MHZ_MASK                               0x0FFF0000
#define    DDR3CTL_PHY_STRAPS_CTL1_AD_WIDTH_SHIFT                         14
#define    DDR3CTL_PHY_STRAPS_CTL1_AD_WIDTH_MASK                          0x0000C000
#define    DDR3CTL_PHY_STRAPS_CTL1_DUAL_RANK_SHIFT                        13
#define    DDR3CTL_PHY_STRAPS_CTL1_DUAL_RANK_MASK                         0x00002000
#define    DDR3CTL_PHY_STRAPS_CTL1_BUS16_SHIFT                            12
#define    DDR3CTL_PHY_STRAPS_CTL1_BUS16_MASK                             0x00001000
#define    DDR3CTL_PHY_STRAPS_CTL1_BUS8_SHIFT                             11
#define    DDR3CTL_PHY_STRAPS_CTL1_BUS8_MASK                              0x00000800
#define    DDR3CTL_PHY_STRAPS_CTL1_CHIP_WIDTH_SHIFT                       10
#define    DDR3CTL_PHY_STRAPS_CTL1_CHIP_WIDTH_MASK                        0x00000400
#define    DDR3CTL_PHY_STRAPS_CTL1_VDDQ_SHIFT                             8
#define    DDR3CTL_PHY_STRAPS_CTL1_VDDQ_MASK                              0x00000300
#define    DDR3CTL_PHY_STRAPS_CTL1_CHIP_SIZE_SHIFT                        6
#define    DDR3CTL_PHY_STRAPS_CTL1_CHIP_SIZE_MASK                         0x000000C0
#define    DDR3CTL_PHY_STRAPS_CTL1_JEDEC_TYPE_SHIFT                       1
#define    DDR3CTL_PHY_STRAPS_CTL1_JEDEC_TYPE_MASK                        0x0000003E
#define    DDR3CTL_PHY_STRAPS_CTL1_STRAPS_VALID_SHIFT                     0
#define    DDR3CTL_PHY_STRAPS_CTL1_STRAPS_VALID_MASK                      0x00000001

#define DDR3CTL_PHY_STRAPS_CTL2_OFFSET                                    0x00000010
#define DDR3CTL_PHY_STRAPS_CTL2_TYPE                                      UInt32
#define DDR3CTL_PHY_STRAPS_CTL2_RESERVED_MASK                             0xFFF80000
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_DDR3_SHIFT                      18
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_DDR3_MASK                       0x00040000
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_WR_SHIFT                        13
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_WR_MASK                         0x0003E000
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_AL_SHIFT                        10
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_AL_MASK                         0x00001C00
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_CWL_SHIFT                       5
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_CWL_MASK                        0x000003E0
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_CL_SHIFT                        0
#define    DDR3CTL_PHY_STRAPS_CTL2_STRAPS_CL_MASK                         0x0000001F

#define DDR3CTL_PHY_STRAPS_STATUS1_OFFSET                                 0x00000014
#define DDR3CTL_PHY_STRAPS_STATUS1_TYPE                                   UInt32
#define DDR3CTL_PHY_STRAPS_STATUS1_RESERVED_MASK                          0xF0000000
#define    DDR3CTL_PHY_STRAPS_STATUS1_MHZ_SHIFT                           16
#define    DDR3CTL_PHY_STRAPS_STATUS1_MHZ_MASK                            0x0FFF0000
#define    DDR3CTL_PHY_STRAPS_STATUS1_AD_WIDTH_SHIFT                      14
#define    DDR3CTL_PHY_STRAPS_STATUS1_AD_WIDTH_MASK                       0x0000C000
#define    DDR3CTL_PHY_STRAPS_STATUS1_DUAL_RANK_SHIFT                     13
#define    DDR3CTL_PHY_STRAPS_STATUS1_DUAL_RANK_MASK                      0x00002000
#define    DDR3CTL_PHY_STRAPS_STATUS1_BUS16_SHIFT                         12
#define    DDR3CTL_PHY_STRAPS_STATUS1_BUS16_MASK                          0x00001000
#define    DDR3CTL_PHY_STRAPS_STATUS1_BUS8_SHIFT                          11
#define    DDR3CTL_PHY_STRAPS_STATUS1_BUS8_MASK                           0x00000800
#define    DDR3CTL_PHY_STRAPS_STATUS1_CHIP_WIDTH_SHIFT                    10
#define    DDR3CTL_PHY_STRAPS_STATUS1_CHIP_WIDTH_MASK                     0x00000400
#define    DDR3CTL_PHY_STRAPS_STATUS1_VDDQ_SHIFT                          8
#define    DDR3CTL_PHY_STRAPS_STATUS1_VDDQ_MASK                           0x00000300
#define    DDR3CTL_PHY_STRAPS_STATUS1_CHIP_SIZE_SHIFT                     6
#define    DDR3CTL_PHY_STRAPS_STATUS1_CHIP_SIZE_MASK                      0x000000C0
#define    DDR3CTL_PHY_STRAPS_STATUS1_JEDEC_TYPE_SHIFT                    1
#define    DDR3CTL_PHY_STRAPS_STATUS1_JEDEC_TYPE_MASK                     0x0000003E
#define    DDR3CTL_PHY_STRAPS_STATUS1_STRAPS_VALID_SHIFT                  0
#define    DDR3CTL_PHY_STRAPS_STATUS1_STRAPS_VALID_MASK                   0x00000001

#define DDR3CTL_PHY_STRAPS_STATUS2_OFFSET                                 0x00000018
#define DDR3CTL_PHY_STRAPS_STATUS2_TYPE                                   UInt32
#define DDR3CTL_PHY_STRAPS_STATUS2_RESERVED_MASK                          0xFFF80000
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_DDR3_SHIFT                   18
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_DDR3_MASK                    0x00040000
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_WR_SHIFT                     13
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_WR_MASK                      0x0003E000
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_AL_SHIFT                     10
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_AL_MASK                      0x00001C00
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_CWL_SHIFT                    5
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_CWL_MASK                     0x000003E0
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_CL_SHIFT                     0
#define    DDR3CTL_PHY_STRAPS_STATUS2_STRAPS_CL_MASK                      0x0000001F

#define DDR3CTL_PHY_STATUS_OFFSET                                         0x0000001C
#define DDR3CTL_PHY_STATUS_TYPE                                           UInt32
#define DDR3CTL_PHY_STATUS_RESERVED_MASK                                  0xFFFFFC00
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_WARMSTART_SHIFT                  9
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_WARMSTART_MASK                   0x00000200
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL3_RD_FIFO_ERROR_SHIFT              8
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL3_RD_FIFO_ERROR_MASK               0x00000100
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL2_RD_FIFO_ERROR_SHIFT              7
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL2_RD_FIFO_ERROR_MASK               0x00000080
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL1_RD_FIFO_ERROR_SHIFT              6
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL1_RD_FIFO_ERROR_MASK               0x00000040
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL0_RD_FIFO_ERROR_SHIFT              5
#define    DDR3CTL_PHY_STATUS_IOB2MC_BL0_RD_FIFO_ERROR_MASK               0x00000020
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_WIRE_DELAY_SHIFT                 2
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_WIRE_DELAY_MASK                  0x0000001C
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_READY_SHIFT                      1
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_READY_MASK                       0x00000002
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_AUTO_INIT_SHIFT                  0
#define    DDR3CTL_PHY_STATUS_IOB2MC_PHY_AUTO_INIT_MASK                   0x00000001

#define DDR3CTL_PLL_CTL1_OFFSET                                           0x00000020
#define DDR3CTL_PLL_CTL1_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL1_RESERVED_MASK                                    0xFC000000
#define    DDR3CTL_PLL_CTL1_PLL_8PHASE_EN_SHIFT                           25
#define    DDR3CTL_PLL_CTL1_PLL_8PHASE_EN_MASK                            0x02000000
#define    DDR3CTL_PLL_CTL1_PLL_BYPCLK_EN_SHIFT                           19
#define    DDR3CTL_PLL_CTL1_PLL_BYPCLK_EN_MASK                            0x01F80000
#define    DDR3CTL_PLL_CTL1_PLL_CH_ENABLEB_SHIFT                          13
#define    DDR3CTL_PLL_CTL1_PLL_CH_ENABLEB_MASK                           0x0007E000
#define    DDR3CTL_PLL_CTL1_PLL_FB_OFFSET_SHIFT                           1
#define    DDR3CTL_PLL_CTL1_PLL_FB_OFFSET_MASK                            0x00001FFE
#define    DDR3CTL_PLL_CTL1_PLL_FB_PHASE_EN_SHIFT                         0
#define    DDR3CTL_PLL_CTL1_PLL_FB_PHASE_EN_MASK                          0x00000001

#define DDR3CTL_PLL_CTL2_OFFSET                                           0x00000024
#define DDR3CTL_PLL_CTL2_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL2_RESERVED_MASK                                    0xFFC00000
#define    DDR3CTL_PLL_CTL2_PLL_CH1_MDEL_SHIFT                            19
#define    DDR3CTL_PLL_CTL2_PLL_CH1_MDEL_MASK                             0x00380000
#define    DDR3CTL_PLL_CTL2_PLL_CH1_MDIV_SHIFT                            11
#define    DDR3CTL_PLL_CTL2_PLL_CH1_MDIV_MASK                             0x0007F800
#define    DDR3CTL_PLL_CTL2_PLL_CH0_MDEL_SHIFT                            8
#define    DDR3CTL_PLL_CTL2_PLL_CH0_MDEL_MASK                             0x00000700
#define    DDR3CTL_PLL_CTL2_PLL_CH0_MDIV_SHIFT                            0
#define    DDR3CTL_PLL_CTL2_PLL_CH0_MDIV_MASK                             0x000000FF

#define DDR3CTL_PLL_CTL3_OFFSET                                           0x00000028
#define DDR3CTL_PLL_CTL3_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL3_RESERVED_MASK                                    0xFFC00000
#define    DDR3CTL_PLL_CTL3_PLL_CH3_MDEL_SHIFT                            19
#define    DDR3CTL_PLL_CTL3_PLL_CH3_MDEL_MASK                             0x00380000
#define    DDR3CTL_PLL_CTL3_PLL_CH3_MDIV_SHIFT                            11
#define    DDR3CTL_PLL_CTL3_PLL_CH3_MDIV_MASK                             0x0007F800
#define    DDR3CTL_PLL_CTL3_PLL_CH2_MDEL_SHIFT                            8
#define    DDR3CTL_PLL_CTL3_PLL_CH2_MDEL_MASK                             0x00000700
#define    DDR3CTL_PLL_CTL3_PLL_CH2_MDIV_SHIFT                            0
#define    DDR3CTL_PLL_CTL3_PLL_CH2_MDIV_MASK                             0x000000FF

#define DDR3CTL_PLL_CTL4_OFFSET                                           0x0000002C
#define DDR3CTL_PLL_CTL4_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL4_RESERVED_MASK                                    0xFFC00000
#define    DDR3CTL_PLL_CTL4_PLL_CH5_MDEL_SHIFT                            19
#define    DDR3CTL_PLL_CTL4_PLL_CH5_MDEL_MASK                             0x00380000
#define    DDR3CTL_PLL_CTL4_PLL_CH5_MDIV_SHIFT                            11
#define    DDR3CTL_PLL_CTL4_PLL_CH5_MDIV_MASK                             0x0007F800
#define    DDR3CTL_PLL_CTL4_PLL_CH4_MDEL_SHIFT                            8
#define    DDR3CTL_PLL_CTL4_PLL_CH4_MDEL_MASK                             0x00000700
#define    DDR3CTL_PLL_CTL4_PLL_CH4_MDIV_SHIFT                            0
#define    DDR3CTL_PLL_CTL4_PLL_CH4_MDIV_MASK                             0x000000FF

#define DDR3CTL_PLL_CTL5_OFFSET                                           0x00000030
#define DDR3CTL_PLL_CTL5_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL5_RESERVED_MASK                                    0xFE000000
#define    DDR3CTL_PLL_CTL5_PLL_CH_HOLD_SHIFT                             19
#define    DDR3CTL_PLL_CTL5_PLL_CH_HOLD_MASK                              0x01F80000
#define    DDR3CTL_PLL_CTL5_PLL_KA_SHIFT                                  16
#define    DDR3CTL_PLL_CTL5_PLL_KA_MASK                                   0x00070000
#define    DDR3CTL_PLL_CTL5_PLL_KI_SHIFT                                  13
#define    DDR3CTL_PLL_CTL5_PLL_KI_MASK                                   0x0000E000
#define    DDR3CTL_PLL_CTL5_PLL_KP_SHIFT                                  9
#define    DDR3CTL_PLL_CTL5_PLL_KP_MASK                                   0x00001E00
#define    DDR3CTL_PLL_CTL5_PLL_CH_LOAD_EN_SHIFT                          3
#define    DDR3CTL_PLL_CTL5_PLL_CH_LOAD_EN_MASK                           0x000001F8
#define    DDR3CTL_PLL_CTL5_PLL_POST_RESETB_SHIFT                         2
#define    DDR3CTL_PLL_CTL5_PLL_POST_RESETB_MASK                          0x00000004
#define    DDR3CTL_PLL_CTL5_PLL_PWRDN_SHIFT                               1
#define    DDR3CTL_PLL_CTL5_PLL_PWRDN_MASK                                0x00000002
#define    DDR3CTL_PLL_CTL5_PLL_RESETB_SHIFT                              0
#define    DDR3CTL_PLL_CTL5_PLL_RESETB_MASK                               0x00000001

#define DDR3CTL_PLL_CTL6_OFFSET                                           0x00000034
#define DDR3CTL_PLL_CTL6_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL6_RESERVED_MASK                                    0xC0000000
#define    DDR3CTL_PLL_CTL6_PLL_NDIV_FRAC_SHIFT                           10
#define    DDR3CTL_PLL_CTL6_PLL_NDIV_FRAC_MASK                            0x3FFFFC00
#define    DDR3CTL_PLL_CTL6_PLL_NDIV_INT_SHIFT                            0
#define    DDR3CTL_PLL_CTL6_PLL_NDIV_INT_MASK                             0x000003FF

#define DDR3CTL_PLL_CTL7_OFFSET                                           0x00000038
#define DDR3CTL_PLL_CTL7_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL7_RESERVED_MASK                                    0x80000000
#define    DDR3CTL_PLL_CTL7_PLL_PDIV_SHIFT                                28
#define    DDR3CTL_PLL_CTL7_PLL_PDIV_MASK                                 0x70000000
#define    DDR3CTL_PLL_CTL7_PLL_CFG_CTRL_SHIFT                            0
#define    DDR3CTL_PLL_CTL7_PLL_CFG_CTRL_MASK                             0x0FFFFFFF

#define DDR3CTL_PLL_CTL8_OFFSET                                           0x0000003C
#define DDR3CTL_PLL_CTL8_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL8_RESERVED_MASK                                    0xFC000000
#define    DDR3CTL_PLL_CTL8_PLL_SSC_LIMIT_SHIFT                           0
#define    DDR3CTL_PLL_CTL8_PLL_SSC_LIMIT_MASK                            0x03FFFFFF

#define DDR3CTL_PLL_CTL9_OFFSET                                           0x00000040
#define DDR3CTL_PLL_CTL9_TYPE                                             UInt32
#define DDR3CTL_PLL_CTL9_RESERVED_MASK                                    0xFFFE0000
#define    DDR3CTL_PLL_CTL9_PLL_SSC_MODE_SHIFT                            16
#define    DDR3CTL_PLL_CTL9_PLL_SSC_MODE_MASK                             0x00010000
#define    DDR3CTL_PLL_CTL9_PLL_SSC_STEP_SHIFT                            0
#define    DDR3CTL_PLL_CTL9_PLL_SSC_STEP_MASK                             0x0000FFFF

#define DDR3CTL_PLL_STATUS_OFFSET                                         0x00000044
#define DDR3CTL_PLL_STATUS_TYPE                                           UInt32
#define DDR3CTL_PLL_STATUS_RESERVED_MASK                                  0xFFFF8000
#define    DDR3CTL_PLL_STATUS_PLL_FLOCK_SHIFT                             14
#define    DDR3CTL_PLL_STATUS_PLL_FLOCK_MASK                              0x00004000
#define    DDR3CTL_PLL_STATUS_PLL_LOCK_SHIFT                              13
#define    DDR3CTL_PLL_STATUS_PLL_LOCK_MASK                               0x00002000
#define    DDR3CTL_PLL_STATUS_PLL_LOCK_LATCH_SHIFT                        12
#define    DDR3CTL_PLL_STATUS_PLL_LOCK_LATCH_MASK                         0x00001000
#define    DDR3CTL_PLL_STATUS_PLL_STAT_OUT_SHIFT                          0
#define    DDR3CTL_PLL_STATUS_PLL_STAT_OUT_MASK                           0x00000FFF

#define DDR3CTL_GLUE_USER_OFFSET                                          0x00000048
#define DDR3CTL_GLUE_USER_TYPE                                            UInt32
#define DDR3CTL_GLUE_USER_RESERVED_MASK                                   0x00000000
#define    DDR3CTL_GLUE_USER_USER_BITS_SHIFT                              0
#define    DDR3CTL_GLUE_USER_USER_BITS_MASK                               0xFFFFFFFF

#endif /* __BRCM_RDB_DDR3CTL_H__ */



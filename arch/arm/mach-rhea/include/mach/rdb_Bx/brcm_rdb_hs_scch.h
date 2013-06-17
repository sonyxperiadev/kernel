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
/*     Date     : Generated on 2/10/2012 15:44:22                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_HS_SCCH_H__
#define __BRCM_RDB_HS_SCCH_H__

#define HS_SCCH_ID_OFFSET                                                 0x00000000
#define HS_SCCH_ID_TYPE                                                   UInt32
#define HS_SCCH_ID_RESERVED_MASK                                          0xFFE00000
#define    HS_SCCH_ID_CH_SEL_SHIFT                                        16
#define    HS_SCCH_ID_CH_SEL_MASK                                         0x001F0000
#define    HS_SCCH_ID_UE_ID_HS0_SHIFT                                     0
#define    HS_SCCH_ID_UE_ID_HS0_MASK                                      0x0000FFFF

#define HS_SCCH_VIT_MIN_ZERO_01_OFFSET                                    0x00000004
#define HS_SCCH_VIT_MIN_ZERO_01_TYPE                                      UInt32
#define HS_SCCH_VIT_MIN_ZERO_01_RESERVED_MASK                             0xF000F000
#define    HS_SCCH_VIT_MIN_ZERO_01_VIT_MIN_ZERO_1_SHIFT                   16
#define    HS_SCCH_VIT_MIN_ZERO_01_VIT_MIN_ZERO_1_MASK                    0x0FFF0000
#define    HS_SCCH_VIT_MIN_ZERO_01_VIT_MIN_ZERO_0_SHIFT                   0
#define    HS_SCCH_VIT_MIN_ZERO_01_VIT_MIN_ZERO_0_MASK                    0x00000FFF

#define HS_SCCH_VIT_MIN_ZERO_23_OFFSET                                    0x00000008
#define HS_SCCH_VIT_MIN_ZERO_23_TYPE                                      UInt32
#define HS_SCCH_VIT_MIN_ZERO_23_RESERVED_MASK                             0xF000F000
#define    HS_SCCH_VIT_MIN_ZERO_23_VIT_MIN_ZERO_3_SHIFT                   16
#define    HS_SCCH_VIT_MIN_ZERO_23_VIT_MIN_ZERO_3_MASK                    0x0FFF0000
#define    HS_SCCH_VIT_MIN_ZERO_23_VIT_MIN_ZERO_2_SHIFT                   0
#define    HS_SCCH_VIT_MIN_ZERO_23_VIT_MIN_ZERO_2_MASK                    0x00000FFF

#define HS_SCCH_VIT_MAX_MIN_01_OFFSET                                     0x0000000C
#define HS_SCCH_VIT_MAX_MIN_01_TYPE                                       UInt32
#define HS_SCCH_VIT_MAX_MIN_01_RESERVED_MASK                              0xF000F000
#define    HS_SCCH_VIT_MAX_MIN_01_VIT_MAX_MIN_1_SHIFT                     16
#define    HS_SCCH_VIT_MAX_MIN_01_VIT_MAX_MIN_1_MASK                      0x0FFF0000
#define    HS_SCCH_VIT_MAX_MIN_01_VIT_MAX_MIN_0_SHIFT                     0
#define    HS_SCCH_VIT_MAX_MIN_01_VIT_MAX_MIN_0_MASK                      0x00000FFF

#define HS_SCCH_VIT_MAX_MIN_23_OFFSET                                     0x00000010
#define HS_SCCH_VIT_MAX_MIN_23_TYPE                                       UInt32
#define HS_SCCH_VIT_MAX_MIN_23_RESERVED_MASK                              0xF000F000
#define    HS_SCCH_VIT_MAX_MIN_23_VIT_MAX_MIN_3_SHIFT                     16
#define    HS_SCCH_VIT_MAX_MIN_23_VIT_MAX_MIN_3_MASK                      0x0FFF0000
#define    HS_SCCH_VIT_MAX_MIN_23_VIT_MAX_MIN_2_SHIFT                     0
#define    HS_SCCH_VIT_MAX_MIN_23_VIT_MAX_MIN_2_MASK                      0x00000FFF

#define HS_SCCH_VIT_SM_ZERO_01_OFFSET                                     0x00000014
#define HS_SCCH_VIT_SM_ZERO_01_TYPE                                       UInt32
#define HS_SCCH_VIT_SM_ZERO_01_RESERVED_MASK                              0xF000F000
#define    HS_SCCH_VIT_SM_ZERO_01_VIT_SM_ZERO_1_SHIFT                     16
#define    HS_SCCH_VIT_SM_ZERO_01_VIT_SM_ZERO_1_MASK                      0x0FFF0000
#define    HS_SCCH_VIT_SM_ZERO_01_VIT_SM_ZERO_0_SHIFT                     0
#define    HS_SCCH_VIT_SM_ZERO_01_VIT_SM_ZERO_0_MASK                      0x00000FFF

#define HS_SCCH_VIT_SM_ZERO_23_OFFSET                                     0x00000018
#define HS_SCCH_VIT_SM_ZERO_23_TYPE                                       UInt32
#define HS_SCCH_VIT_SM_ZERO_23_RESERVED_MASK                              0xF000F000
#define    HS_SCCH_VIT_SM_ZERO_23_VIT_SM_ZERO_3_SHIFT                     16
#define    HS_SCCH_VIT_SM_ZERO_23_VIT_SM_ZERO_3_MASK                      0x0FFF0000
#define    HS_SCCH_VIT_SM_ZERO_23_VIT_SM_ZERO_2_SHIFT                     0
#define    HS_SCCH_VIT_SM_ZERO_23_VIT_SM_ZERO_2_MASK                      0x00000FFF

#define HS_SCCH_X1_OFFSET                                                 0x0000001C
#define HS_SCCH_X1_TYPE                                                   UInt32
#define HS_SCCH_X1_RESERVED_MASK                                          0x00000000
#define    HS_SCCH_X1_X1_3_SHIFT                                          24
#define    HS_SCCH_X1_X1_3_MASK                                           0xFF000000
#define    HS_SCCH_X1_X1_2_SHIFT                                          16
#define    HS_SCCH_X1_X1_2_MASK                                           0x00FF0000
#define    HS_SCCH_X1_X1_1_SHIFT                                          8
#define    HS_SCCH_X1_X1_1_MASK                                           0x0000FF00
#define    HS_SCCH_X1_X1_0_SHIFT                                          0
#define    HS_SCCH_X1_X1_0_MASK                                           0x000000FF

#define HS_SCCH_X2_01_OFFSET                                              0x00000020
#define HS_SCCH_X2_01_TYPE                                                UInt32
#define HS_SCCH_X2_01_RESERVED_MASK                                       0xE000E000
#define    HS_SCCH_X2_01_X2_1_SHIFT                                       16
#define    HS_SCCH_X2_01_X2_1_MASK                                        0x1FFF0000
#define    HS_SCCH_X2_01_X2_0_SHIFT                                       0
#define    HS_SCCH_X2_01_X2_0_MASK                                        0x00001FFF

#define HS_SCCH_X2_23_OFFSET                                              0x00000024
#define HS_SCCH_X2_23_TYPE                                                UInt32
#define HS_SCCH_X2_23_RESERVED_MASK                                       0xE000E000
#define    HS_SCCH_X2_23_X2_3_SHIFT                                       16
#define    HS_SCCH_X2_23_X2_3_MASK                                        0x1FFF0000
#define    HS_SCCH_X2_23_X2_2_SHIFT                                       0
#define    HS_SCCH_X2_23_X2_2_MASK                                        0x00001FFF

#define HS_SCCH_CRC_OFFSET                                                0x00000028
#define HS_SCCH_CRC_TYPE                                                  UInt32
#define HS_SCCH_CRC_RESERVED_MASK                                         0xFFFFFFE0
#define    HS_SCCH_CRC_CRC_OUT_HS_SHIFT                                   0
#define    HS_SCCH_CRC_CRC_OUT_HS_MASK                                    0x0000001F

#define HS_SCCH_CRC_FLIP_OFFSET                                           0x0000002C
#define HS_SCCH_CRC_FLIP_TYPE                                             UInt32
#define HS_SCCH_CRC_FLIP_RESERVED_MASK                                    0xFFFFEEE8
#define    HS_SCCH_CRC_FLIP_VITERBI_METRIC_SEL_SHIFT                      12
#define    HS_SCCH_CRC_FLIP_VITERBI_METRIC_SEL_MASK                       0x00001000
#define    HS_SCCH_CRC_FLIP_RESET_AG_SHIFT                                8
#define    HS_SCCH_CRC_FLIP_RESET_AG_MASK                                 0x00000100
#define    HS_SCCH_CRC_FLIP_RESET_HS_SHIFT                                4
#define    HS_SCCH_CRC_FLIP_RESET_HS_MASK                                 0x00000010
#define    HS_SCCH_CRC_FLIP_RW_SEL_MODE_SHIFT                             2
#define    HS_SCCH_CRC_FLIP_RW_SEL_MODE_MASK                              0x00000004
#define    HS_SCCH_CRC_FLIP_TTI_STRB_MODE_SHIFT                           1
#define    HS_SCCH_CRC_FLIP_TTI_STRB_MODE_MASK                            0x00000002
#define    HS_SCCH_CRC_FLIP_CRC_FLIP_SHIFT                                0
#define    HS_SCCH_CRC_FLIP_CRC_FLIP_MASK                                 0x00000001

#define HS_SCCH_AGCH_EN_OFFSET                                            0x00000030
#define HS_SCCH_AGCH_EN_TYPE                                              UInt32
#define HS_SCCH_AGCH_EN_RESERVED_MASK                                     0xFFFFFFFE
#define    HS_SCCH_AGCH_EN_AGCH_EN_SHIFT                                  0
#define    HS_SCCH_AGCH_EN_AGCH_EN_MASK                                   0x00000001

#define HS_SCCH_AGCH_TTI_OFFSET                                           0x00000034
#define HS_SCCH_AGCH_TTI_TYPE                                             UInt32
#define HS_SCCH_AGCH_TTI_RESERVED_MASK                                    0xFFFFFFF8
#define    HS_SCCH_AGCH_TTI_AGCH_FLIP_R1_SHIFT                            2
#define    HS_SCCH_AGCH_TTI_AGCH_FLIP_R1_MASK                             0x00000004
#define    HS_SCCH_AGCH_TTI_AGCH_TTI_MODE_SHIFT                           1
#define    HS_SCCH_AGCH_TTI_AGCH_TTI_MODE_MASK                            0x00000002
#define    HS_SCCH_AGCH_TTI_AGCH_TTI_START_SHIFT                          0
#define    HS_SCCH_AGCH_TTI_AGCH_TTI_START_MASK                           0x00000001

#define HS_SCCH_UE_ID_AG_OFFSET                                           0x00000038
#define HS_SCCH_UE_ID_AG_TYPE                                             UInt32
#define HS_SCCH_UE_ID_AG_RESERVED_MASK                                    0x00000000
#define    HS_SCCH_UE_ID_AG_UE_ID_AG0_SHIFT                               16
#define    HS_SCCH_UE_ID_AG_UE_ID_AG0_MASK                                0xFFFF0000
#define    HS_SCCH_UE_ID_AG_UE_ID_AG1_SHIFT                               0
#define    HS_SCCH_UE_ID_AG_UE_ID_AG1_MASK                                0x0000FFFF

#define HS_SCCH_X_AG_OUT_OFFSET                                           0x0000003C
#define HS_SCCH_X_AG_OUT_TYPE                                             UInt32
#define HS_SCCH_X_AG_OUT_RESERVED_MASK                                    0xFFFFFCC0
#define    HS_SCCH_X_AG_OUT_CRC_OUT_AG0_SHIFT                             9
#define    HS_SCCH_X_AG_OUT_CRC_OUT_AG0_MASK                              0x00000200
#define    HS_SCCH_X_AG_OUT_CRC_OUT_AG1_SHIFT                             8
#define    HS_SCCH_X_AG_OUT_CRC_OUT_AG1_MASK                              0x00000100
#define    HS_SCCH_X_AG_OUT_X_AG_OUT_SHIFT                                0
#define    HS_SCCH_X_AG_OUT_X_AG_OUT_MASK                                 0x0000003F

#define HS_SCCH_UE_ID_HS1_OFFSET                                          0x00000040
#define HS_SCCH_UE_ID_HS1_TYPE                                            UInt32
#define HS_SCCH_UE_ID_HS1_RESERVED_MASK                                   0xFFFF0000
#define    HS_SCCH_UE_ID_HS1_UE_ID_HS1_SHIFT                              0
#define    HS_SCCH_UE_ID_HS1_UE_ID_HS1_MASK                               0x0000FFFF

#define HS_SCCH_VIT_MIN_ZERO_04_OFFSET                                    0x00000044
#define HS_SCCH_VIT_MIN_ZERO_04_TYPE                                      UInt32
#define HS_SCCH_VIT_MIN_ZERO_04_RESERVED_MASK                             0xFFFFF000
#define    HS_SCCH_VIT_MIN_ZERO_04_VIT_MIN_ZERO_4_SHIFT                   0
#define    HS_SCCH_VIT_MIN_ZERO_04_VIT_MIN_ZERO_4_MASK                    0x00000FFF

#define HS_SCCH_VIT_MAX_MIN_04_OFFSET                                     0x00000048
#define HS_SCCH_VIT_MAX_MIN_04_TYPE                                       UInt32
#define HS_SCCH_VIT_MAX_MIN_04_RESERVED_MASK                              0xFFFFF000
#define    HS_SCCH_VIT_MAX_MIN_04_VIT_MAX_MIN_4_SHIFT                     0
#define    HS_SCCH_VIT_MAX_MIN_04_VIT_MAX_MIN_4_MASK                      0x00000FFF

#define HS_SCCH_VIT_SM_ZERO_04_OFFSET                                     0x0000004C
#define HS_SCCH_VIT_SM_ZERO_04_TYPE                                       UInt32
#define HS_SCCH_VIT_SM_ZERO_04_RESERVED_MASK                              0xFFFFF000
#define    HS_SCCH_VIT_SM_ZERO_04_VIT_SM_ZERO_4_SHIFT                     0
#define    HS_SCCH_VIT_SM_ZERO_04_VIT_SM_ZERO_4_MASK                      0x00000FFF

#define HS_SCCH_X1_04_OFFSET                                              0x00000050
#define HS_SCCH_X1_04_TYPE                                                UInt32
#define HS_SCCH_X1_04_RESERVED_MASK                                       0xFFFFFF00
#define    HS_SCCH_X1_04_X1_4_SHIFT                                       0
#define    HS_SCCH_X1_04_X1_4_MASK                                        0x000000FF

#define HS_SCCH_X2_04_OFFSET                                              0x00000054
#define HS_SCCH_X2_04_TYPE                                                UInt32
#define HS_SCCH_X2_04_RESERVED_MASK                                       0xFFFFE000
#define    HS_SCCH_X2_04_X2_4_SHIFT                                       0
#define    HS_SCCH_X2_04_X2_4_MASK                                        0x00001FFF

#define HS_SCCH_TC_ENABLE_OFFSET                                          0x00000058
#define HS_SCCH_TC_ENABLE_TYPE                                            UInt32
#define HS_SCCH_TC_ENABLE_RESERVED_MASK                                   0xFFFFFFFE
#define    HS_SCCH_TC_ENABLE_TARGET_CELL_ENABLE_SHIFT                     0
#define    HS_SCCH_TC_ENABLE_TARGET_CELL_ENABLE_MASK                      0x00000001

#define HS_SCCH_TC_UE_ID_OFFSET                                           0x0000005C
#define HS_SCCH_TC_UE_ID_TYPE                                             UInt32
#define HS_SCCH_TC_UE_ID_RESERVED_MASK                                    0xFFFF0000
#define    HS_SCCH_TC_UE_ID_TARGET_CELL_UE_ID_SHIFT                       0
#define    HS_SCCH_TC_UE_ID_TARGET_CELL_UE_ID_MASK                        0x0000FFFF

#define HS_SCCH_TC_INT_ENABLE_OFFSET                                      0x00000060
#define HS_SCCH_TC_INT_ENABLE_TYPE                                        UInt32
#define HS_SCCH_TC_INT_ENABLE_RESERVED_MASK                               0xFFFFFFEE
#define    HS_SCCH_TC_INT_ENABLE_TARGET_CELL_ENABLE_SLOT23_INT_SHIFT      4
#define    HS_SCCH_TC_INT_ENABLE_TARGET_CELL_ENABLE_SLOT23_INT_MASK       0x00000010
#define    HS_SCCH_TC_INT_ENABLE_TARGET_CELL_ENABLE_SLOT1_INT_SHIFT       0
#define    HS_SCCH_TC_INT_ENABLE_TARGET_CELL_ENABLE_SLOT1_INT_MASK        0x00000001

#define HS_SCCH_TC_INT_CLEAR_OFFSET                                       0x00000064
#define HS_SCCH_TC_INT_CLEAR_TYPE                                         UInt32
#define HS_SCCH_TC_INT_CLEAR_RESERVED_MASK                                0xFFFFFFEE
#define    HS_SCCH_TC_INT_CLEAR_TARGET_CELL_CLEAR_SLOT23_INT_SHIFT        4
#define    HS_SCCH_TC_INT_CLEAR_TARGET_CELL_CLEAR_SLOT23_INT_MASK         0x00000010
#define    HS_SCCH_TC_INT_CLEAR_TARGET_CELL_CLEAR_SLOT1_INT_SHIFT         0
#define    HS_SCCH_TC_INT_CLEAR_TARGET_CELL_CLEAR_SLOT1_INT_MASK          0x00000001

#define HS_SCCH_TC_INT_STATUS_OFFSET                                      0x00000068
#define HS_SCCH_TC_INT_STATUS_TYPE                                        UInt32
#define HS_SCCH_TC_INT_STATUS_RESERVED_MASK                               0xFFFFFFEE
#define    HS_SCCH_TC_INT_STATUS_TARGET_CELL_SLOT23_INT_STATUS_SHIFT      4
#define    HS_SCCH_TC_INT_STATUS_TARGET_CELL_SLOT23_INT_STATUS_MASK       0x00000010
#define    HS_SCCH_TC_INT_STATUS_TARGET_CELL_SLOT1_INT_STATUS_SHIFT       0
#define    HS_SCCH_TC_INT_STATUS_TARGET_CELL_SLOT1_INT_STATUS_MASK        0x00000001

#define HS_SCCH_TC_DATA_CRC_OFFSET                                        0x0000006C
#define HS_SCCH_TC_DATA_CRC_TYPE                                          UInt32
#define HS_SCCH_TC_DATA_CRC_RESERVED_MASK                                 0xFEE00000
#define    HS_SCCH_TC_DATA_CRC_TARGET_CELL_CRC_RESULT_SHIFT               24
#define    HS_SCCH_TC_DATA_CRC_TARGET_CELL_CRC_RESULT_MASK                0x01000000
#define    HS_SCCH_TC_DATA_CRC_TARGET_CELL_X2_DATA_SHIFT                  8
#define    HS_SCCH_TC_DATA_CRC_TARGET_CELL_X2_DATA_MASK                   0x001FFF00
#define    HS_SCCH_TC_DATA_CRC_TARGET_CELL_X1_DATA_SHIFT                  0
#define    HS_SCCH_TC_DATA_CRC_TARGET_CELL_X1_DATA_MASK                   0x000000FF

#define HS_SCCH_TC_VIT_METRICS_OFFSET                                     0x00000070
#define HS_SCCH_TC_VIT_METRICS_TYPE                                       UInt32
#define HS_SCCH_TC_VIT_METRICS_RESERVED_MASK                              0xF000F000
#define    HS_SCCH_TC_VIT_METRICS_TARGET_CELL_VIT_MAX_MIN_SHIFT           16
#define    HS_SCCH_TC_VIT_METRICS_TARGET_CELL_VIT_MAX_MIN_MASK            0x0FFF0000
#define    HS_SCCH_TC_VIT_METRICS_TARGET_CELL_VIT_MIN_ZERO_SHIFT          0
#define    HS_SCCH_TC_VIT_METRICS_TARGET_CELL_VIT_MIN_ZERO_MASK           0x00000FFF

#define HS_SCCH_TC_VIT_SM_ZERO_OFFSET                                     0x00000074
#define HS_SCCH_TC_VIT_SM_ZERO_TYPE                                       UInt32
#define HS_SCCH_TC_VIT_SM_ZERO_RESERVED_MASK                              0xFFFFF000
#define    HS_SCCH_TC_VIT_SM_ZERO_TARGET_CELL_VIT_SM_ZERO_SHIFT           0
#define    HS_SCCH_TC_VIT_SM_ZERO_TARGET_CELL_VIT_SM_ZERO_MASK            0x00000FFF

#endif /* __BRCM_RDB_HS_SCCH_H__ */



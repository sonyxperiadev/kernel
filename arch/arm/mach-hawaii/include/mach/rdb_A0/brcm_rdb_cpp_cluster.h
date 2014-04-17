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

#ifndef __BRCM_RDB_CPP_CLUSTER_H__
#define __BRCM_RDB_CPP_CLUSTER_H__

#define CPP_CLUSTER_CPP_START_POS_A1_OFFSET                               0x00000000
#define CPP_CLUSTER_CPP_START_POS_A1_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_START_POS_A1_RESERVED_MASK                        0xFFF0C000
#define    CPP_CLUSTER_CPP_START_POS_A1_ANT1_START_SLOT_POS_SHIFT         16
#define    CPP_CLUSTER_CPP_START_POS_A1_ANT1_START_SLOT_POS_MASK          0x000F0000
#define    CPP_CLUSTER_CPP_START_POS_A1_ANT1_START_SAMPLE_POS_SHIFT       0
#define    CPP_CLUSTER_CPP_START_POS_A1_ANT1_START_SAMPLE_POS_MASK        0x00003FFF

#define CPP_CLUSTER_CPP_START_POS_A2_OFFSET                               0x00000004
#define CPP_CLUSTER_CPP_START_POS_A2_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_START_POS_A2_RESERVED_MASK                        0xFFF0C000
#define    CPP_CLUSTER_CPP_START_POS_A2_ANT2_START_SLOT_POS_SHIFT         16
#define    CPP_CLUSTER_CPP_START_POS_A2_ANT2_START_SLOT_POS_MASK          0x000F0000
#define    CPP_CLUSTER_CPP_START_POS_A2_ANT2_START_SAMPLE_POS_SHIFT       0
#define    CPP_CLUSTER_CPP_START_POS_A2_ANT2_START_SAMPLE_POS_MASK        0x00003FFF

#define CPP_CLUSTER_GC_OVSF_CFG_OFFSET                                    0x00000008
#define CPP_CLUSTER_GC_OVSF_CFG_TYPE                                      UInt32
#define CPP_CLUSTER_GC_OVSF_CFG_RESERVED_MASK                             0x0E00E000
#define    CPP_CLUSTER_GC_OVSF_CFG_GC_OFFSET_SHIFT                        28
#define    CPP_CLUSTER_GC_OVSF_CFG_GC_OFFSET_MASK                         0xF0000000
#define    CPP_CLUSTER_GC_OVSF_CFG_PRIM_GC_INDX_SHIFT                     16
#define    CPP_CLUSTER_GC_OVSF_CFG_PRIM_GC_INDX_MASK                      0x01FF0000
#define    CPP_CLUSTER_GC_OVSF_CFG_OVSF_ID_SHIFT                          4
#define    CPP_CLUSTER_GC_OVSF_CFG_OVSF_ID_MASK                           0x00001FF0
#define    CPP_CLUSTER_GC_OVSF_CFG_OVSF_SF_SHIFT                          0
#define    CPP_CLUSTER_GC_OVSF_CFG_OVSF_SF_MASK                           0x0000000F

#define CPP_CLUSTER_GC_OVSF_MASK_OFFSET                                   0x0000000C
#define CPP_CLUSTER_GC_OVSF_MASK_TYPE                                     UInt32
#define CPP_CLUSTER_GC_OVSF_MASK_RESERVED_MASK                            0xEF00EFF0
#define    CPP_CLUSTER_GC_OVSF_MASK_OVSF_MASK_GEN_SHIFT                   28
#define    CPP_CLUSTER_GC_OVSF_MASK_OVSF_MASK_GEN_MASK                    0x10000000
#define    CPP_CLUSTER_GC_OVSF_MASK_OVSF_MODULE_ID_SHIFT                  16
#define    CPP_CLUSTER_GC_OVSF_MASK_OVSF_MODULE_ID_MASK                   0x00FF0000
#define    CPP_CLUSTER_GC_OVSF_MASK_GC_INIT_SHIFT                         12
#define    CPP_CLUSTER_GC_OVSF_MASK_GC_INIT_MASK                          0x00001000
#define    CPP_CLUSTER_GC_OVSF_MASK_GC_MODULE_ID_SHIFT                    0
#define    CPP_CLUSTER_GC_OVSF_MASK_GC_MODULE_ID_MASK                     0x0000000F

#define CPP_CLUSTER_DIV_CFG_OFFSET                                        0x00000010
#define CPP_CLUSTER_DIV_CFG_TYPE                                          UInt32
#define CPP_CLUSTER_DIV_CFG_RESERVED_MASK                                 0xC000C000
#define    CPP_CLUSTER_DIV_CFG_HS_SCCH_NBT_SEL_SHIFT                      29
#define    CPP_CLUSTER_DIV_CFG_HS_SCCH_NBT_SEL_MASK                       0x20000000
#define    CPP_CLUSTER_DIV_CFG_CPC_MODE_SHIFT                             28
#define    CPP_CLUSTER_DIV_CFG_CPC_MODE_MASK                              0x10000000
#define    CPP_CLUSTER_DIV_CFG_CPC_CIR_BUFF_INDX_SHIFT                    24
#define    CPP_CLUSTER_DIV_CFG_CPC_CIR_BUFF_INDX_MASK                     0x0F000000
#define    CPP_CLUSTER_DIV_CFG_CPC_CH_START_POS_SHIFT                     20
#define    CPP_CLUSTER_DIV_CFG_CPC_CH_START_POS_MASK                      0x00F00000
#define    CPP_CLUSTER_DIV_CFG_CPC_CH_LENGTH_SHIFT                        19
#define    CPP_CLUSTER_DIV_CFG_CPC_CH_LENGTH_MASK                         0x00080000
#define    CPP_CLUSTER_DIV_CFG_HS_SCCH_NBT_SHIFT                          16
#define    CPP_CLUSTER_DIV_CFG_HS_SCCH_NBT_MASK                           0x00070000
#define    CPP_CLUSTER_DIV_CFG_CL1_PHASE_INDX_SHIFT                       12
#define    CPP_CLUSTER_DIV_CFG_CL1_PHASE_INDX_MASK                        0x00003000
#define    CPP_CLUSTER_DIV_CFG_DIV_CH_LENGTH_SHIFT                        8
#define    CPP_CLUSTER_DIV_CFG_DIV_CH_LENGTH_MASK                         0x00000F00
#define    CPP_CLUSTER_DIV_CFG_DIV_CH_START_POS_SHIFT                     4
#define    CPP_CLUSTER_DIV_CFG_DIV_CH_START_POS_MASK                      0x000000F0
#define    CPP_CLUSTER_DIV_CFG_DIV4_MODE_SHIFT                            2
#define    CPP_CLUSTER_DIV_CFG_DIV4_MODE_MASK                             0x0000000C
#define    CPP_CLUSTER_DIV_CFG_DIV_MODE_SHIFT                             0
#define    CPP_CLUSTER_DIV_CFG_DIV_MODE_MASK                              0x00000003

#define CPP_CLUSTER_CPP_CFG1_OFFSET                                       0x00000014
#define CPP_CLUSTER_CPP_CFG1_TYPE                                         UInt32
#define CPP_CLUSTER_CPP_CFG1_RESERVED_MASK                                0x1CE0CCCC
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_EN_SHIFT                         31
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_EN_MASK                          0x80000000
#define    CPP_CLUSTER_CPP_CFG1_CPP_C14_EN_SHIFT                          30
#define    CPP_CLUSTER_CPP_CFG1_CPP_C14_EN_MASK                           0x40000000
#define    CPP_CLUSTER_CPP_CFG1_CPP_DLL_CX32_EN_SHIFT                     29
#define    CPP_CLUSTER_CPP_CFG1_CPP_DLL_CX32_EN_MASK                      0x20000000
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_FREEZE_EN2_SHIFT                 25
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_FREEZE_EN2_MASK                  0x02000000
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_FREEZE_EN1_SHIFT                 24
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_FREEZE_EN1_MASK                  0x01000000
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_FREEZE_TIME_SHIFT                16
#define    CPP_CLUSTER_CPP_CFG1_CPP_IIR3_FREEZE_TIME_MASK                 0x001F0000
#define    CPP_CLUSTER_CPP_CFG1_CPP_MRC_SHIFT_SHIFT                       12
#define    CPP_CLUSTER_CPP_CFG1_CPP_MRC_SHIFT_MASK                        0x00003000
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_RSSI_SHIFT                         8
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_RSSI_MASK                          0x00000300
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_PF_SHIFT                           4
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_PF_MASK                            0x00000030
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_ITT_SHIFT                          0
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_ITT_MASK                           0x00000003

#define CPP_CLUSTER_CPP_CFG2_OFFSET                                       0x00000018
#define CPP_CLUSTER_CPP_CFG2_TYPE                                         UInt32
#define CPP_CLUSTER_CPP_CFG2_RESERVED_MASK                                0x00000000
#define    CPP_CLUSTER_CPP_CFG2_CPP_TT_LOOP_GAIN_SHIFT                    16
#define    CPP_CLUSTER_CPP_CFG2_CPP_TT_LOOP_GAIN_MASK                     0xFFFF0000
#define    CPP_CLUSTER_CPP_CFG2_CPP_LOCK_HIGH_SHIFT                       8
#define    CPP_CLUSTER_CPP_CFG2_CPP_LOCK_HIGH_MASK                        0x0000FF00
#define    CPP_CLUSTER_CPP_CFG2_CPP_LOCK_LOW_SHIFT                        0
#define    CPP_CLUSTER_CPP_CFG2_CPP_LOCK_LOW_MASK                         0x000000FF

#define CPP_CLUSTER_CPP_CFG3_OFFSET                                       0x0000001C
#define CPP_CLUSTER_CPP_CFG3_TYPE                                         UInt32
#define CPP_CLUSTER_CPP_CFG3_RESERVED_MASK                                0x08EE0E0C
#define    CPP_CLUSTER_CPP_CFG3_CPP_DELAY_MODE_SHIFT                      29
#define    CPP_CLUSTER_CPP_CFG3_CPP_DELAY_MODE_MASK                       0xE0000000
#define    CPP_CLUSTER_CPP_CFG3_CPP_CHIP_TRK_EN_ANT2_SHIFT                28
#define    CPP_CLUSTER_CPP_CFG3_CPP_CHIP_TRK_EN_ANT2_MASK                 0x10000000
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_DIRECTION_ANT2_SHIFT               25
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_DIRECTION_ANT2_MASK                0x06000000
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ENABLE_ANT2_SHIFT                  24
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ENABLE_ANT2_MASK                   0x01000000
#define    CPP_CLUSTER_CPP_CFG3_CPP_EN_ANT2_SHIFT                         20
#define    CPP_CLUSTER_CPP_CFG3_CPP_EN_ANT2_MASK                          0x00100000
#define    CPP_CLUSTER_CPP_CFG3_CPP_CHIP_TRK_EN_ANT1_SHIFT                16
#define    CPP_CLUSTER_CPP_CFG3_CPP_CHIP_TRK_EN_ANT1_MASK                 0x00010000
#define    CPP_CLUSTER_CPP_CFG3_CPP_DLL_B2B_EN_SHIFT                      15
#define    CPP_CLUSTER_CPP_CFG3_CPP_DLL_B2B_EN_MASK                       0x00008000
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_DIRECTION_ANT1_SHIFT               13
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_DIRECTION_ANT1_MASK                0x00006000
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ENABLE_ANT1_SHIFT                  12
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ENABLE_ANT1_MASK                   0x00001000
#define    CPP_CLUSTER_CPP_CFG3_CPP_EN_ANT1_SHIFT                         8
#define    CPP_CLUSTER_CPP_CFG3_CPP_EN_ANT1_MASK                          0x00000100
#define    CPP_CLUSTER_CPP_CFG3_CPP_CLUSTER_POS_SHIFT                     4
#define    CPP_CLUSTER_CPP_CFG3_CPP_CLUSTER_POS_MASK                      0x000000F0
#define    CPP_CLUSTER_CPP_CFG3_CPP_AUX_MODE_SHIFT                        0
#define    CPP_CLUSTER_CPP_CFG3_CPP_AUX_MODE_MASK                         0x00000003

#define CPP_CLUSTER_SYM_CH_EN_OFFSET                                      0x00000020
#define CPP_CLUSTER_SYM_CH_EN_TYPE                                        UInt32
#define CPP_CLUSTER_SYM_CH_EN_RESERVED_MASK                               0xFEEEEEE0
#define    CPP_CLUSTER_SYM_CH_EN_SYM_ACCUM_DEL_EN_SHIFT                   24
#define    CPP_CLUSTER_SYM_CH_EN_SYM_ACCUM_DEL_EN_MASK                    0x01000000
#define    CPP_CLUSTER_SYM_CH_EN_SYM_SF_CPICH_SHIFT                       20
#define    CPP_CLUSTER_SYM_CH_EN_SYM_SF_CPICH_MASK                        0x00100000
#define    CPP_CLUSTER_SYM_CH_EN_PDSCH_H2_SEL_SHIFT                       16
#define    CPP_CLUSTER_SYM_CH_EN_PDSCH_H2_SEL_MASK                        0x00010000
#define    CPP_CLUSTER_SYM_CH_EN_SCCH_H2_SEL_SHIFT                        12
#define    CPP_CLUSTER_SYM_CH_EN_SCCH_H2_SEL_MASK                         0x00001000
#define    CPP_CLUSTER_SYM_CH_EN_CPICH_H2_SEL_SHIFT                       8
#define    CPP_CLUSTER_SYM_CH_EN_CPICH_H2_SEL_MASK                        0x00000100
#define    CPP_CLUSTER_SYM_CH_EN_PDSCH_CH_EN_ANT2_SHIFT                   4
#define    CPP_CLUSTER_SYM_CH_EN_PDSCH_CH_EN_ANT2_MASK                    0x00000010
#define    CPP_CLUSTER_SYM_CH_EN_SCCH_CH_EN_ANT2_SHIFT                    3
#define    CPP_CLUSTER_SYM_CH_EN_SCCH_CH_EN_ANT2_MASK                     0x00000008
#define    CPP_CLUSTER_SYM_CH_EN_SYM_CH_EN_SHIFT                          0
#define    CPP_CLUSTER_SYM_CH_EN_SYM_CH_EN_MASK                           0x00000007

#define CPP_CLUSTER_CPP_START_OFFSET                                      0x00000024
#define CPP_CLUSTER_CPP_START_TYPE                                        UInt32
#define CPP_CLUSTER_CPP_START_RESERVED_MASK                               0xFFFFFFEE
#define    CPP_CLUSTER_CPP_START_CPP_START_CMD1_SHIFT                     4
#define    CPP_CLUSTER_CPP_START_CPP_START_CMD1_MASK                      0x00000010
#define    CPP_CLUSTER_CPP_START_CPP_START_CMD0_SHIFT                     0
#define    CPP_CLUSTER_CPP_START_CPP_START_CMD0_MASK                      0x00000001

#define CPP_CLUSTER_CPP_COMPRESS_OFFSET                                   0x00000028
#define CPP_CLUSTER_CPP_COMPRESS_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_COMPRESS_RESERVED_MASK                            0xFF0FFF0C
#define    CPP_CLUSTER_CPP_COMPRESS_CPP_WAIT_CNT_RH_7_SHIFT               20
#define    CPP_CLUSTER_CPP_COMPRESS_CPP_WAIT_CNT_RH_7_MASK                0x00F00000
#define    CPP_CLUSTER_CPP_COMPRESS_CPM_FRAME_CNT_SHIFT                   4
#define    CPP_CLUSTER_CPP_COMPRESS_CPM_FRAME_CNT_MASK                    0x000000F0
#define    CPP_CLUSTER_CPP_COMPRESS_CMP_NEW_SHIFT                         1
#define    CPP_CLUSTER_CPP_COMPRESS_CMP_NEW_MASK                          0x00000002
#define    CPP_CLUSTER_CPP_COMPRESS_CMP_EN_SHIFT                          0
#define    CPP_CLUSTER_CPP_COMPRESS_CMP_EN_MASK                           0x00000001

#define CPP_CLUSTER_CPP_RSCP_OFFSET                                       0x0000002C
#define CPP_CLUSTER_CPP_RSCP_TYPE                                         UInt32
#define CPP_CLUSTER_CPP_RSCP_RESERVED_MASK                                0x00000000
#define    CPP_CLUSTER_CPP_RSCP_CPP_RSSI_ANT2_SHIFT                       16
#define    CPP_CLUSTER_CPP_RSCP_CPP_RSSI_ANT2_MASK                        0xFFFF0000
#define    CPP_CLUSTER_CPP_RSCP_CPP_RSSI_ANT1_SHIFT                       0
#define    CPP_CLUSTER_CPP_RSCP_CPP_RSSI_ANT1_MASK                        0x0000FFFF

#define CPP_CLUSTER_CPP_H_DATA0_OFFSET                                    0x00000030
#define CPP_CLUSTER_CPP_H_DATA0_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA0_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HQ_ANT2_0_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HQ_ANT2_0_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HI_ANT2_0_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HI_ANT2_0_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HQ_ANT1_0_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HQ_ANT1_0_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HI_ANT1_0_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA0_CPP_HI_ANT1_0_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA1_OFFSET                                    0x00000034
#define CPP_CLUSTER_CPP_H_DATA1_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA1_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HQ_ANT2_1_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HQ_ANT2_1_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HI_ANT2_1_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HI_ANT2_1_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HQ_ANT1_1_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HQ_ANT1_1_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HI_ANT1_1_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA1_CPP_HI_ANT1_1_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA2_OFFSET                                    0x00000038
#define CPP_CLUSTER_CPP_H_DATA2_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA2_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HQ_ANT2_2_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HQ_ANT2_2_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HI_ANT2_2_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HI_ANT2_2_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HQ_ANT1_2_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HQ_ANT1_2_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HI_ANT1_2_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA2_CPP_HI_ANT1_2_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA3_OFFSET                                    0x0000003C
#define CPP_CLUSTER_CPP_H_DATA3_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA3_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HQ_ANT2_3_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HQ_ANT2_3_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HI_ANT2_3_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HI_ANT2_3_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HQ_ANT1_3_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HQ_ANT1_3_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HI_ANT1_3_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA3_CPP_HI_ANT1_3_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA4_OFFSET                                    0x00000040
#define CPP_CLUSTER_CPP_H_DATA4_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA4_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HQ_ANT2_4_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HQ_ANT2_4_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HI_ANT2_4_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HI_ANT2_4_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HQ_ANT1_4_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HQ_ANT1_4_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HI_ANT1_4_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA4_CPP_HI_ANT1_4_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA5_OFFSET                                    0x00000044
#define CPP_CLUSTER_CPP_H_DATA5_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA5_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HQ_ANT2_5_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HQ_ANT2_5_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HI_ANT2_5_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HI_ANT2_5_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HQ_ANT1_5_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HQ_ANT1_5_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HI_ANT1_5_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA5_CPP_HI_ANT1_5_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA6_OFFSET                                    0x00000048
#define CPP_CLUSTER_CPP_H_DATA6_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA6_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HQ_ANT2_6_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HQ_ANT2_6_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HI_ANT2_6_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HI_ANT2_6_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HQ_ANT1_6_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HQ_ANT1_6_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HI_ANT1_6_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA6_CPP_HI_ANT1_6_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA7_OFFSET                                    0x0000004C
#define CPP_CLUSTER_CPP_H_DATA7_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA7_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HQ_ANT2_7_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HQ_ANT2_7_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HI_ANT2_7_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HI_ANT2_7_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HQ_ANT1_7_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HQ_ANT1_7_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HI_ANT1_7_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA7_CPP_HI_ANT1_7_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA8_OFFSET                                    0x00000050
#define CPP_CLUSTER_CPP_H_DATA8_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA8_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HQ_ANT2_8_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HQ_ANT2_8_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HI_ANT2_8_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HI_ANT2_8_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HQ_ANT1_8_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HQ_ANT1_8_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HI_ANT1_8_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA8_CPP_HI_ANT1_8_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA9_OFFSET                                    0x00000054
#define CPP_CLUSTER_CPP_H_DATA9_TYPE                                      UInt32
#define CPP_CLUSTER_CPP_H_DATA9_RESERVED_MASK                             0x00000000
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HQ_ANT2_9_SHIFT                    24
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HQ_ANT2_9_MASK                     0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HI_ANT2_9_SHIFT                    16
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HI_ANT2_9_MASK                     0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HQ_ANT1_9_SHIFT                    8
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HQ_ANT1_9_MASK                     0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HI_ANT1_9_SHIFT                    0
#define    CPP_CLUSTER_CPP_H_DATA9_CPP_HI_ANT1_9_MASK                     0x000000FF

#define CPP_CLUSTER_CPP_H_DATA10_OFFSET                                   0x00000058
#define CPP_CLUSTER_CPP_H_DATA10_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_H_DATA10_RESERVED_MASK                            0x00000000
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HQ_ANT2_10_SHIFT                  24
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HQ_ANT2_10_MASK                   0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HI_ANT2_10_SHIFT                  16
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HI_ANT2_10_MASK                   0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HQ_ANT1_10_SHIFT                  8
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HQ_ANT1_10_MASK                   0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HI_ANT1_10_SHIFT                  0
#define    CPP_CLUSTER_CPP_H_DATA10_CPP_HI_ANT1_10_MASK                   0x000000FF

#define CPP_CLUSTER_CPP_H_DATA11_OFFSET                                   0x0000005C
#define CPP_CLUSTER_CPP_H_DATA11_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_H_DATA11_RESERVED_MASK                            0x00000000
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HQ_ANT2_11_SHIFT                  24
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HQ_ANT2_11_MASK                   0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HI_ANT2_11_SHIFT                  16
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HI_ANT2_11_MASK                   0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HQ_ANT1_11_SHIFT                  8
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HQ_ANT1_11_MASK                   0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HI_ANT1_11_SHIFT                  0
#define    CPP_CLUSTER_CPP_H_DATA11_CPP_HI_ANT1_11_MASK                   0x000000FF

#define CPP_CLUSTER_CPP_H_DATA12_OFFSET                                   0x00000060
#define CPP_CLUSTER_CPP_H_DATA12_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_H_DATA12_RESERVED_MASK                            0x00000000
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HQ_ANT2_12_SHIFT                  24
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HQ_ANT2_12_MASK                   0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HI_ANT2_12_SHIFT                  16
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HI_ANT2_12_MASK                   0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HQ_ANT1_12_SHIFT                  8
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HQ_ANT1_12_MASK                   0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HI_ANT1_12_SHIFT                  0
#define    CPP_CLUSTER_CPP_H_DATA12_CPP_HI_ANT1_12_MASK                   0x000000FF

#define CPP_CLUSTER_CPP_H_DATA13_OFFSET                                   0x00000064
#define CPP_CLUSTER_CPP_H_DATA13_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_H_DATA13_RESERVED_MASK                            0x00000000
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HQ_ANT2_13_SHIFT                  24
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HQ_ANT2_13_MASK                   0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HI_ANT2_13_SHIFT                  16
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HI_ANT2_13_MASK                   0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HQ_ANT1_13_SHIFT                  8
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HQ_ANT1_13_MASK                   0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HI_ANT1_13_SHIFT                  0
#define    CPP_CLUSTER_CPP_H_DATA13_CPP_HI_ANT1_13_MASK                   0x000000FF

#define CPP_CLUSTER_CPP_H_DATA14_OFFSET                                   0x00000068
#define CPP_CLUSTER_CPP_H_DATA14_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_H_DATA14_RESERVED_MASK                            0x00000000
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HQ_ANT2_14_SHIFT                  24
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HQ_ANT2_14_MASK                   0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HI_ANT2_14_SHIFT                  16
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HI_ANT2_14_MASK                   0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HQ_ANT1_14_SHIFT                  8
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HQ_ANT1_14_MASK                   0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HI_ANT1_14_SHIFT                  0
#define    CPP_CLUSTER_CPP_H_DATA14_CPP_HI_ANT1_14_MASK                   0x000000FF

#define CPP_CLUSTER_CPP_H_DATA15_OFFSET                                   0x0000006C
#define CPP_CLUSTER_CPP_H_DATA15_TYPE                                     UInt32
#define CPP_CLUSTER_CPP_H_DATA15_RESERVED_MASK                            0x00000000
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HQ_ANT2_15_SHIFT                  24
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HQ_ANT2_15_MASK                   0xFF000000
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HI_ANT2_15_SHIFT                  16
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HI_ANT2_15_MASK                   0x00FF0000
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HQ_ANT1_15_SHIFT                  8
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HQ_ANT1_15_MASK                   0x0000FF00
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HI_ANT1_15_SHIFT                  0
#define    CPP_CLUSTER_CPP_H_DATA15_CPP_HI_ANT1_15_MASK                   0x000000FF

#define CPP_CLUSTER_CPP_LOCKS_OFFSET                                      0x00000074
#define CPP_CLUSTER_CPP_LOCKS_TYPE                                        UInt32
#define CPP_CLUSTER_CPP_LOCKS_RESERVED_MASK                               0xFFFF00EE
#define    CPP_CLUSTER_CPP_LOCKS_CPP_CG_ANT2_SHIFT                        12
#define    CPP_CLUSTER_CPP_LOCKS_CPP_CG_ANT2_MASK                         0x0000F000
#define    CPP_CLUSTER_CPP_LOCKS_CPP_CG_ANT1_SHIFT                        8
#define    CPP_CLUSTER_CPP_LOCKS_CPP_CG_ANT1_MASK                         0x00000F00
#define    CPP_CLUSTER_CPP_LOCKS_CPP_LOCK_ANT2_SHIFT                      4
#define    CPP_CLUSTER_CPP_LOCKS_CPP_LOCK_ANT2_MASK                       0x00000010
#define    CPP_CLUSTER_CPP_LOCKS_CPP_LOCK_ANT1_SHIFT                      0
#define    CPP_CLUSTER_CPP_LOCKS_CPP_LOCK_ANT1_MASK                       0x00000001

#define CPP_CLUSTER_CPP_TT_ERR_A1_OFFSET                                  0x00000078
#define CPP_CLUSTER_CPP_TT_ERR_A1_TYPE                                    UInt32
#define CPP_CLUSTER_CPP_TT_ERR_A1_RESERVED_MASK                           0xFFF00000
#define    CPP_CLUSTER_CPP_TT_ERR_A1_CPP_ITT_ERR_ANT1_SHIFT               0
#define    CPP_CLUSTER_CPP_TT_ERR_A1_CPP_ITT_ERR_ANT1_MASK                0x000FFFFF

#define CPP_CLUSTER_CPP_TT_ERR_A2_OFFSET                                  0x0000007C
#define CPP_CLUSTER_CPP_TT_ERR_A2_TYPE                                    UInt32
#define CPP_CLUSTER_CPP_TT_ERR_A2_RESERVED_MASK                           0xFFF00000
#define    CPP_CLUSTER_CPP_TT_ERR_A2_CPP_ITT_ERR_ANT2_SHIFT               0
#define    CPP_CLUSTER_CPP_TT_ERR_A2_CPP_ITT_ERR_ANT2_MASK                0x000FFFFF

#define CPP_CLUSTER_CPP_RF_POSITION_OFFSET                                0x00000080
#define CPP_CLUSTER_CPP_RF_POSITION_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RF_POSITION_RESERVED_MASK                         0xFFF0C000
#define    CPP_CLUSTER_CPP_RF_POSITION_SLOT_CNT_APB_SHIFT                 16
#define    CPP_CLUSTER_CPP_RF_POSITION_SLOT_CNT_APB_MASK                  0x000F0000
#define    CPP_CLUSTER_CPP_RF_POSITION_SAMPLE_CNT_APB_SHIFT               0
#define    CPP_CLUSTER_CPP_RF_POSITION_SAMPLE_CNT_APB_MASK                0x00003FFF

#define CPP_CLUSTER_SYM_CPICHS_OFFSET                                     0x00000084
#define CPP_CLUSTER_SYM_CPICHS_TYPE                                       UInt32
#define CPP_CLUSTER_SYM_CPICHS_RESERVED_MASK                              0xF000F000
#define    CPP_CLUSTER_SYM_CPICHS_SYM_CPICH_S2_SHIFT                      16
#define    CPP_CLUSTER_SYM_CPICHS_SYM_CPICH_S2_MASK                       0x0FFF0000
#define    CPP_CLUSTER_SYM_CPICHS_SYM_CPICH_S1_SHIFT                      0
#define    CPP_CLUSTER_SYM_CPICHS_SYM_CPICH_S1_MASK                       0x00000FFF

#define CPP_CLUSTER_SYM_CPICHN_A1_OFFSET                                  0x00000088
#define CPP_CLUSTER_SYM_CPICHN_A1_TYPE                                    UInt32
#define CPP_CLUSTER_SYM_CPICHN_A1_RESERVED_MASK                           0xFFF80000
#define    CPP_CLUSTER_SYM_CPICHN_A1_SYM_CPICH_N1_SHIFT                   0
#define    CPP_CLUSTER_SYM_CPICHN_A1_SYM_CPICH_N1_MASK                    0x0007FFFF

#define CPP_CLUSTER_SYM_CPICHN_A2_OFFSET                                  0x0000008C
#define CPP_CLUSTER_SYM_CPICHN_A2_TYPE                                    UInt32
#define CPP_CLUSTER_SYM_CPICHN_A2_RESERVED_MASK                           0xFFF80000
#define    CPP_CLUSTER_SYM_CPICHN_A2_SYM_CPICH_N2_SHIFT                   0
#define    CPP_CLUSTER_SYM_CPICHN_A2_SYM_CPICH_N2_MASK                    0x0007FFFF

#define CPP_CLUSTER_CHIP_EQ_CTRL_OFFSET                                   0x00000090
#define CPP_CLUSTER_CHIP_EQ_CTRL_TYPE                                     UInt32
#define CPP_CLUSTER_CHIP_EQ_CTRL_RESERVED_MASK                            0x00000ECE
#define    CPP_CLUSTER_CHIP_EQ_CTRL_EQ_PN_SHIFT                           16
#define    CPP_CLUSTER_CHIP_EQ_CTRL_EQ_PN_MASK                            0xFFFF0000
#define    CPP_CLUSTER_CHIP_EQ_CTRL_DMI_APB_ADDR_SHIFT                    12
#define    CPP_CLUSTER_CHIP_EQ_CTRL_DMI_APB_ADDR_MASK                     0x0000F000
#define    CPP_CLUSTER_CHIP_EQ_CTRL_DMI_APB_EN_SHIFT                      8
#define    CPP_CLUSTER_CHIP_EQ_CTRL_DMI_APB_EN_MASK                       0x00000100
#define    CPP_CLUSTER_CHIP_EQ_CTRL_EQ_POWER_SEL_SHIFT                    5
#define    CPP_CLUSTER_CHIP_EQ_CTRL_EQ_POWER_SEL_MASK                     0x00000020
#define    CPP_CLUSTER_CHIP_EQ_CTRL_EQ_DL_MODE_SHIFT                      4
#define    CPP_CLUSTER_CHIP_EQ_CTRL_EQ_DL_MODE_MASK                       0x00000010
#define    CPP_CLUSTER_CHIP_EQ_CTRL_HSDPA_EQ_ENABLE_SHIFT                 0
#define    CPP_CLUSTER_CHIP_EQ_CTRL_HSDPA_EQ_ENABLE_MASK                  0x00000001

#define CPP_CLUSTER_EQ_POWER_OFFSET                                       0x00000094
#define CPP_CLUSTER_EQ_POWER_TYPE                                         UInt32
#define CPP_CLUSTER_EQ_POWER_RESERVED_MASK                                0x00000000
#define    CPP_CLUSTER_EQ_POWER_EQ_POWER_SHIFT                            0
#define    CPP_CLUSTER_EQ_POWER_EQ_POWER_MASK                             0xFFFFFFFF

#define CPP_CLUSTER_DMI_WEIGHT_I_OFFSET                                   0x00000098
#define CPP_CLUSTER_DMI_WEIGHT_I_TYPE                                     UInt32
#define CPP_CLUSTER_DMI_WEIGHT_I_RESERVED_MASK                            0xFF000000
#define    CPP_CLUSTER_DMI_WEIGHT_I_DMI_WEIGHT_I_SHIFT                    0
#define    CPP_CLUSTER_DMI_WEIGHT_I_DMI_WEIGHT_I_MASK                     0x00FFFFFF

#define CPP_CLUSTER_DMI_WEIGHT_Q_OFFSET                                   0x0000009C
#define CPP_CLUSTER_DMI_WEIGHT_Q_TYPE                                     UInt32
#define CPP_CLUSTER_DMI_WEIGHT_Q_RESERVED_MASK                            0xFF000000
#define    CPP_CLUSTER_DMI_WEIGHT_Q_DMI_WEIGHT_Q_SHIFT                    0
#define    CPP_CLUSTER_DMI_WEIGHT_Q_DMI_WEIGHT_Q_MASK                     0x00FFFFFF

#define CPP_CLUSTER_DEBUG_CTRL_OFFSET                                     0x000000A4
#define CPP_CLUSTER_DEBUG_CTRL_TYPE                                       UInt32
#define CPP_CLUSTER_DEBUG_CTRL_RESERVED_MASK                              0x00C0800E
#define    CPP_CLUSTER_DEBUG_CTRL_EQ_DEBUG_MODE_SHIFT                     24
#define    CPP_CLUSTER_DEBUG_CTRL_EQ_DEBUG_MODE_MASK                      0xFF000000
#define    CPP_CLUSTER_DEBUG_CTRL_SYM_PDSCH_SCCH_SEL_SHIFT                21
#define    CPP_CLUSTER_DEBUG_CTRL_SYM_PDSCH_SCCH_SEL_MASK                 0x00200000
#define    CPP_CLUSTER_DEBUG_CTRL_SYM_ANT_SEL_SHIFT                       20
#define    CPP_CLUSTER_DEBUG_CTRL_SYM_ANT_SEL_MASK                        0x00100000
#define    CPP_CLUSTER_DEBUG_CTRL_SYM_CH_INDX_SHIFT                       16
#define    CPP_CLUSTER_DEBUG_CTRL_SYM_CH_INDX_MASK                        0x000F0000
#define    CPP_CLUSTER_DEBUG_CTRL_HSDPA_DEBUG_MODE_SHIFT                  12
#define    CPP_CLUSTER_DEBUG_CTRL_HSDPA_DEBUG_MODE_MASK                   0x00007000
#define    CPP_CLUSTER_DEBUG_CTRL_RESP_SELCT_SHIFT                        8
#define    CPP_CLUSTER_DEBUG_CTRL_RESP_SELCT_MASK                         0x00000F00
#define    CPP_CLUSTER_DEBUG_CTRL_STIM_SEL_SHIFT                          4
#define    CPP_CLUSTER_DEBUG_CTRL_STIM_SEL_MASK                           0x000000F0
#define    CPP_CLUSTER_DEBUG_CTRL_SW_STIM_EN_SHIFT                        0
#define    CPP_CLUSTER_DEBUG_CTRL_SW_STIM_EN_MASK                         0x00000001

#define CPP_CLUSTER_CMPRS_MODE_CTRL1_OFFSET                               0x000000A8
#define CPP_CLUSTER_CMPRS_MODE_CTRL1_TYPE                                 UInt32
#define CPP_CLUSTER_CMPRS_MODE_CTRL1_RESERVED_MASK                        0x0000C000
#define    CPP_CLUSTER_CMPRS_MODE_CTRL1_CPM_START_FRAMECNT_SHIFT          20
#define    CPP_CLUSTER_CMPRS_MODE_CTRL1_CPM_START_FRAMECNT_MASK           0xFFF00000
#define    CPP_CLUSTER_CMPRS_MODE_CTRL1_CPM_START_SLOTCNT_SHIFT           16
#define    CPP_CLUSTER_CMPRS_MODE_CTRL1_CPM_START_SLOTCNT_MASK            0x000F0000
#define    CPP_CLUSTER_CMPRS_MODE_CTRL1_CPM_START_SAMPLECNT_SHIFT         0
#define    CPP_CLUSTER_CMPRS_MODE_CTRL1_CPM_START_SAMPLECNT_MASK          0x00003FFF

#define CPP_CLUSTER_CMPRS_MODE_CTRL2_OFFSET                               0x000000AC
#define CPP_CLUSTER_CMPRS_MODE_CTRL2_TYPE                                 UInt32
#define CPP_CLUSTER_CMPRS_MODE_CTRL2_RESERVED_MASK                        0x0000C000
#define    CPP_CLUSTER_CMPRS_MODE_CTRL2_CPM_STOP_FRAMECNT_SHIFT           20
#define    CPP_CLUSTER_CMPRS_MODE_CTRL2_CPM_STOP_FRAMECNT_MASK            0xFFF00000
#define    CPP_CLUSTER_CMPRS_MODE_CTRL2_CPM_STOP_SLOTCNT_SHIFT            16
#define    CPP_CLUSTER_CMPRS_MODE_CTRL2_CPM_STOP_SLOTCNT_MASK             0x000F0000
#define    CPP_CLUSTER_CMPRS_MODE_CTRL2_CPM_STOP_SAMPLECNT_SHIFT          0
#define    CPP_CLUSTER_CMPRS_MODE_CTRL2_CPM_STOP_SAMPLECNT_MASK           0x00003FFF

#define CPP_CLUSTER_CPP_RSSI_DATA0_OFFSET                                 0x000000C0
#define CPP_CLUSTER_CPP_RSSI_DATA0_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA0_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA0_CPP_RSSI_VEC_ANT2_0_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA0_CPP_RSSI_VEC_ANT2_0_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA0_CPP_RSSI_VEC_ANT1_0_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA0_CPP_RSSI_VEC_ANT1_0_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA1_OFFSET                                 0x000000C4
#define CPP_CLUSTER_CPP_RSSI_DATA1_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA1_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA1_CPP_RSSI_VEC_ANT2_1_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA1_CPP_RSSI_VEC_ANT2_1_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA1_CPP_RSSI_VEC_ANT1_1_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA1_CPP_RSSI_VEC_ANT1_1_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA2_OFFSET                                 0x000000C8
#define CPP_CLUSTER_CPP_RSSI_DATA2_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA2_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA2_CPP_RSSI_VEC_ANT2_2_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA2_CPP_RSSI_VEC_ANT2_2_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA2_CPP_RSSI_VEC_ANT1_2_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA2_CPP_RSSI_VEC_ANT1_2_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA3_OFFSET                                 0x000000CC
#define CPP_CLUSTER_CPP_RSSI_DATA3_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA3_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA3_CPP_RSSI_VEC_ANT2_3_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA3_CPP_RSSI_VEC_ANT2_3_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA3_CPP_RSSI_VEC_ANT1_3_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA3_CPP_RSSI_VEC_ANT1_3_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA4_OFFSET                                 0x000000D0
#define CPP_CLUSTER_CPP_RSSI_DATA4_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA4_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA4_CPP_RSSI_VEC_ANT2_4_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA4_CPP_RSSI_VEC_ANT2_4_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA4_CPP_RSSI_VEC_ANT1_4_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA4_CPP_RSSI_VEC_ANT1_4_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA5_OFFSET                                 0x000000D4
#define CPP_CLUSTER_CPP_RSSI_DATA5_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA5_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA5_CPP_RSSI_VEC_ANT2_5_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA5_CPP_RSSI_VEC_ANT2_5_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA5_CPP_RSSI_VEC_ANT1_5_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA5_CPP_RSSI_VEC_ANT1_5_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA6_OFFSET                                 0x000000D8
#define CPP_CLUSTER_CPP_RSSI_DATA6_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA6_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA6_CPP_RSSI_VEC_ANT2_6_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA6_CPP_RSSI_VEC_ANT2_6_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA6_CPP_RSSI_VEC_ANT1_6_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA6_CPP_RSSI_VEC_ANT1_6_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA7_OFFSET                                 0x000000DC
#define CPP_CLUSTER_CPP_RSSI_DATA7_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA7_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA7_CPP_RSSI_VEC_ANT2_7_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA7_CPP_RSSI_VEC_ANT2_7_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA7_CPP_RSSI_VEC_ANT1_7_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA7_CPP_RSSI_VEC_ANT1_7_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA8_OFFSET                                 0x000000E0
#define CPP_CLUSTER_CPP_RSSI_DATA8_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA8_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA8_CPP_RSSI_VEC_ANT2_8_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA8_CPP_RSSI_VEC_ANT2_8_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA8_CPP_RSSI_VEC_ANT1_8_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA8_CPP_RSSI_VEC_ANT1_8_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA9_OFFSET                                 0x000000E4
#define CPP_CLUSTER_CPP_RSSI_DATA9_TYPE                                   UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA9_RESERVED_MASK                          0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA9_CPP_RSSI_VEC_ANT2_9_SHIFT           16
#define    CPP_CLUSTER_CPP_RSSI_DATA9_CPP_RSSI_VEC_ANT2_9_MASK            0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA9_CPP_RSSI_VEC_ANT1_9_SHIFT           0
#define    CPP_CLUSTER_CPP_RSSI_DATA9_CPP_RSSI_VEC_ANT1_9_MASK            0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA10_OFFSET                                0x000000E8
#define CPP_CLUSTER_CPP_RSSI_DATA10_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA10_RESERVED_MASK                         0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA10_CPP_RSSI_VEC_ANT2_10_SHIFT         16
#define    CPP_CLUSTER_CPP_RSSI_DATA10_CPP_RSSI_VEC_ANT2_10_MASK          0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA10_CPP_RSSI_VEC_ANT1_10_SHIFT         0
#define    CPP_CLUSTER_CPP_RSSI_DATA10_CPP_RSSI_VEC_ANT1_10_MASK          0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA11_OFFSET                                0x000000EC
#define CPP_CLUSTER_CPP_RSSI_DATA11_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA11_RESERVED_MASK                         0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA11_CPP_RSSI_VEC_ANT2_11_SHIFT         16
#define    CPP_CLUSTER_CPP_RSSI_DATA11_CPP_RSSI_VEC_ANT2_11_MASK          0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA11_CPP_RSSI_VEC_ANT1_11_SHIFT         0
#define    CPP_CLUSTER_CPP_RSSI_DATA11_CPP_RSSI_VEC_ANT1_11_MASK          0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA12_OFFSET                                0x000000F0
#define CPP_CLUSTER_CPP_RSSI_DATA12_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA12_RESERVED_MASK                         0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA12_CPP_RSSI_VEC_ANT2_12_SHIFT         16
#define    CPP_CLUSTER_CPP_RSSI_DATA12_CPP_RSSI_VEC_ANT2_12_MASK          0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA12_CPP_RSSI_VEC_ANT1_12_SHIFT         0
#define    CPP_CLUSTER_CPP_RSSI_DATA12_CPP_RSSI_VEC_ANT1_12_MASK          0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA13_OFFSET                                0x000000F4
#define CPP_CLUSTER_CPP_RSSI_DATA13_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA13_RESERVED_MASK                         0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA13_CPP_RSSI_VEC_ANT2_13_SHIFT         16
#define    CPP_CLUSTER_CPP_RSSI_DATA13_CPP_RSSI_VEC_ANT2_13_MASK          0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA13_CPP_RSSI_VEC_ANT1_13_SHIFT         0
#define    CPP_CLUSTER_CPP_RSSI_DATA13_CPP_RSSI_VEC_ANT1_13_MASK          0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA14_OFFSET                                0x000000F8
#define CPP_CLUSTER_CPP_RSSI_DATA14_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA14_RESERVED_MASK                         0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA14_CPP_RSSI_VEC_ANT2_14_SHIFT         16
#define    CPP_CLUSTER_CPP_RSSI_DATA14_CPP_RSSI_VEC_ANT2_14_MASK          0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA14_CPP_RSSI_VEC_ANT1_14_SHIFT         0
#define    CPP_CLUSTER_CPP_RSSI_DATA14_CPP_RSSI_VEC_ANT1_14_MASK          0x0000FFFF

#define CPP_CLUSTER_CPP_RSSI_DATA15_OFFSET                                0x000000FC
#define CPP_CLUSTER_CPP_RSSI_DATA15_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_RSSI_DATA15_RESERVED_MASK                         0x00000000
#define    CPP_CLUSTER_CPP_RSSI_DATA15_CPP_RSSI_VEC_ANT2_15_SHIFT         16
#define    CPP_CLUSTER_CPP_RSSI_DATA15_CPP_RSSI_VEC_ANT2_15_MASK          0xFFFF0000
#define    CPP_CLUSTER_CPP_RSSI_DATA15_CPP_RSSI_VEC_ANT1_15_SHIFT         0
#define    CPP_CLUSTER_CPP_RSSI_DATA15_CPP_RSSI_VEC_ANT1_15_MASK          0x0000FFFF

#define CPP_CLUSTER_CPP_DLL_MAN_ADJ_OFFSET                                0x00000100
#define CPP_CLUSTER_CPP_DLL_MAN_ADJ_TYPE                                  UInt32
#define CPP_CLUSTER_CPP_DLL_MAN_ADJ_RESERVED_MASK                         0xF000F000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_ASYNC_MANUAL_ANT2_SHIFT     27
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_ASYNC_MANUAL_ANT2_MASK      0x08000000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_HOLD_START_ANT2_SHIFT       26
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_HOLD_START_ANT2_MASK        0x04000000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MANUAL_EN_ANT2_SHIFT        25
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MANUAL_EN_ANT2_MASK         0x02000000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MULTI_EN_ANT2_SHIFT         24
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MULTI_EN_ANT2_MASK          0x01000000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT2_SHIFT         16
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT2_MASK          0x00FF0000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_ASYNC_MANUAL_ANT1_SHIFT     11
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_ASYNC_MANUAL_ANT1_MASK      0x00000800
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_HOLD_START_ANT1_SHIFT       10
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_HOLD_START_ANT1_MASK        0x00000400
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MANUAL_EN_ANT1_SHIFT        9
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MANUAL_EN_ANT1_MASK         0x00000200
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MULTI_EN_ANT1_SHIFT         8
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_CPP_TT_MULTI_EN_ANT1_MASK          0x00000100
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT1_SHIFT         0
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT1_MASK          0x000000FF

#define CPP_CLUSTER_CPP_IIR3_COEF_A1_OFFSET                               0x00000104
#define CPP_CLUSTER_CPP_IIR3_COEF_A1_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_IIR3_COEF_A1_RESERVED_MASK                        0xFFF00000
#define    CPP_CLUSTER_CPP_IIR3_COEF_A1_CPP_IIR3_COEF_A1_SHIFT            0
#define    CPP_CLUSTER_CPP_IIR3_COEF_A1_CPP_IIR3_COEF_A1_MASK             0x000FFFFF

#define CPP_CLUSTER_CPP_IIR3_COEF_A2_OFFSET                               0x00000108
#define CPP_CLUSTER_CPP_IIR3_COEF_A2_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_IIR3_COEF_A2_RESERVED_MASK                        0xFFF00000
#define    CPP_CLUSTER_CPP_IIR3_COEF_A2_CPP_IIR3_COEF_A2_SHIFT            0
#define    CPP_CLUSTER_CPP_IIR3_COEF_A2_CPP_IIR3_COEF_A2_MASK             0x000FFFFF

#define CPP_CLUSTER_CPP_IIR3_COEF_A3_OFFSET                               0x0000010C
#define CPP_CLUSTER_CPP_IIR3_COEF_A3_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_IIR3_COEF_A3_RESERVED_MASK                        0xFFF00000
#define    CPP_CLUSTER_CPP_IIR3_COEF_A3_CPP_IIR3_COEF_A3_SHIFT            0
#define    CPP_CLUSTER_CPP_IIR3_COEF_A3_CPP_IIR3_COEF_A3_MASK             0x000FFFFF

#define CPP_CLUSTER_CPP_IIR3_COEF_B0_OFFSET                               0x00000110
#define CPP_CLUSTER_CPP_IIR3_COEF_B0_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_IIR3_COEF_B0_RESERVED_MASK                        0xFFF00000
#define    CPP_CLUSTER_CPP_IIR3_COEF_B0_CPP_IIR3_COEF_B0_SHIFT            0
#define    CPP_CLUSTER_CPP_IIR3_COEF_B0_CPP_IIR3_COEF_B0_MASK             0x000FFFFF

#define CPP_CLUSTER_CPP_IIR3_COEF_B1_OFFSET                               0x00000114
#define CPP_CLUSTER_CPP_IIR3_COEF_B1_TYPE                                 UInt32
#define CPP_CLUSTER_CPP_IIR3_COEF_B1_RESERVED_MASK                        0xFFF00000
#define    CPP_CLUSTER_CPP_IIR3_COEF_B1_CPP_IIR3_COEF_B1_SHIFT            0
#define    CPP_CLUSTER_CPP_IIR3_COEF_B1_CPP_IIR3_COEF_B1_MASK             0x000FFFFF

#define CPP_CLUSTER_CPP_IIR3_COEF_XYZ_OFFSET                              0x00000118
#define CPP_CLUSTER_CPP_IIR3_COEF_XYZ_TYPE                                UInt32
#define CPP_CLUSTER_CPP_IIR3_COEF_XYZ_RESERVED_MASK                       0x3FFFF888
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_LATCH_SHIFT        31
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_LATCH_MASK         0x80000000
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_RESET_SHIFT             30
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_RESET_MASK              0x40000000
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_Z_SHIFT            8
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_Z_MASK             0x00000700
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_Y_SHIFT            4
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_Y_MASK             0x00000070
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_X_SHIFT            0
#define    CPP_CLUSTER_CPP_IIR3_COEF_XYZ_CPP_IIR3_COEF_X_MASK             0x00000007

#define CPP_CLUSTER_EQ_CONST_COEFF0_OFFSET                                0x0000011C
#define CPP_CLUSTER_EQ_CONST_COEFF0_TYPE                                  UInt32
#define CPP_CLUSTER_EQ_CONST_COEFF0_RESERVED_MASK                         0xFFFF0E0E
#define    CPP_CLUSTER_EQ_CONST_COEFF0_EQ_CONST_DMI_MSB_SHIFT             12
#define    CPP_CLUSTER_EQ_CONST_COEFF0_EQ_CONST_DMI_MSB_MASK              0x0000F000
#define    CPP_CLUSTER_EQ_CONST_COEFF0_EQ_CONST_COEFF_LOC_SHIFT           4
#define    CPP_CLUSTER_EQ_CONST_COEFF0_EQ_CONST_COEFF_LOC_MASK            0x000001F0
#define    CPP_CLUSTER_EQ_CONST_COEFF0_EQ_CONST_COEFF_MODE_SHIFT          0
#define    CPP_CLUSTER_EQ_CONST_COEFF0_EQ_CONST_COEFF_MODE_MASK           0x00000001

#define CPP_CLUSTER_EQ_CONST_COEFF1_OFFSET                                0x00000120
#define CPP_CLUSTER_EQ_CONST_COEFF1_TYPE                                  UInt32
#define CPP_CLUSTER_EQ_CONST_COEFF1_RESERVED_MASK                         0xF000F000
#define    CPP_CLUSTER_EQ_CONST_COEFF1_EQ_CONST_COEFF_Q_SHIFT             16
#define    CPP_CLUSTER_EQ_CONST_COEFF1_EQ_CONST_COEFF_Q_MASK              0x0FFF0000
#define    CPP_CLUSTER_EQ_CONST_COEFF1_EQ_CONST_COEFF_I_SHIFT             0
#define    CPP_CLUSTER_EQ_CONST_COEFF1_EQ_CONST_COEFF_I_MASK              0x00000FFF

#define CPP_CLUSTER_SYM_VP_GAIN_OFFSET                                    0x00000124
#define CPP_CLUSTER_SYM_VP_GAIN_TYPE                                      UInt32
#define CPP_CLUSTER_SYM_VP_GAIN_RESERVED_MASK                             0xFFFCFCCC
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_PDSCH_VP_GAIN_SEL_SHIFT            16
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_PDSCH_VP_GAIN_SEL_MASK             0x00030000
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_SCCH_VP_GAIN_SEL_SHIFT             8
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_SCCH_VP_GAIN_SEL_MASK              0x00000300
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_N_SEL_SHIFT          4
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_N_SEL_MASK           0x00000030
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_S_SEL_SHIFT          0
#define    CPP_CLUSTER_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_S_SEL_MASK           0x00000003

#define CPP_CLUSTER_CPP_C14_LOCK_THRESH_OFFSET                            0x00000128
#define CPP_CLUSTER_CPP_C14_LOCK_THRESH_TYPE                              UInt32
#define CPP_CLUSTER_CPP_C14_LOCK_THRESH_RESERVED_MASK                     0x00000000
#define    CPP_CLUSTER_CPP_C14_LOCK_THRESH_CPP_C14_LOCK_THRESH_HI_SHIFT   16
#define    CPP_CLUSTER_CPP_C14_LOCK_THRESH_CPP_C14_LOCK_THRESH_HI_MASK    0xFFFF0000
#define    CPP_CLUSTER_CPP_C14_LOCK_THRESH_CPP_C14_LOCK_THRESH_LO_SHIFT   0
#define    CPP_CLUSTER_CPP_C14_LOCK_THRESH_CPP_C14_LOCK_THRESH_LO_MASK    0x0000FFFF

#define CPP_CLUSTER_CPP_C14_TT_LOOP_GAIN_OFFSET                           0x0000012C
#define CPP_CLUSTER_CPP_C14_TT_LOOP_GAIN_TYPE                             UInt32
#define CPP_CLUSTER_CPP_C14_TT_LOOP_GAIN_RESERVED_MASK                    0xFC000000
#define    CPP_CLUSTER_CPP_C14_TT_LOOP_GAIN_CPP_C14_TT_LOOP_GAIN_SHIFT    0
#define    CPP_CLUSTER_CPP_C14_TT_LOOP_GAIN_CPP_C14_TT_LOOP_GAIN_MASK     0x03FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_0_OFFSET                        0x00000130
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_0_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_0_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_0_CPP_C14_RSSI_VEC_ANT1_0_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_0_CPP_C14_RSSI_VEC_ANT1_0_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_1_OFFSET                        0x00000134
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_1_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_1_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_1_CPP_C14_RSSI_VEC_ANT1_1_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_1_CPP_C14_RSSI_VEC_ANT1_1_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_2_OFFSET                        0x00000138
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_2_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_2_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_2_CPP_C14_RSSI_VEC_ANT1_2_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_2_CPP_C14_RSSI_VEC_ANT1_2_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_3_OFFSET                        0x0000013C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_3_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_3_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_3_CPP_C14_RSSI_VEC_ANT1_3_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_3_CPP_C14_RSSI_VEC_ANT1_3_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_4_OFFSET                        0x00000140
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_4_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_4_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_4_CPP_C14_RSSI_VEC_ANT1_4_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_4_CPP_C14_RSSI_VEC_ANT1_4_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_5_OFFSET                        0x00000144
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_5_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_5_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_5_CPP_C14_RSSI_VEC_ANT1_5_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_5_CPP_C14_RSSI_VEC_ANT1_5_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_6_OFFSET                        0x00000148
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_6_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_6_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_6_CPP_C14_RSSI_VEC_ANT1_6_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_6_CPP_C14_RSSI_VEC_ANT1_6_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_7_OFFSET                        0x0000014C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_7_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_7_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_7_CPP_C14_RSSI_VEC_ANT1_7_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_7_CPP_C14_RSSI_VEC_ANT1_7_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_8_OFFSET                        0x00000150
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_8_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_8_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_8_CPP_C14_RSSI_VEC_ANT1_8_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_8_CPP_C14_RSSI_VEC_ANT1_8_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_9_OFFSET                        0x00000154
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_9_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_9_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_9_CPP_C14_RSSI_VEC_ANT1_9_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_9_CPP_C14_RSSI_VEC_ANT1_9_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_10_OFFSET                       0x00000158
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_10_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_10_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_10_CPP_C14_RSSI_VEC_ANT1_10_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_10_CPP_C14_RSSI_VEC_ANT1_10_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_11_OFFSET                       0x0000015C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_11_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_11_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_11_CPP_C14_RSSI_VEC_ANT1_11_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_11_CPP_C14_RSSI_VEC_ANT1_11_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_12_OFFSET                       0x00000160
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_12_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_12_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_12_CPP_C14_RSSI_VEC_ANT1_12_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_12_CPP_C14_RSSI_VEC_ANT1_12_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_13_OFFSET                       0x00000164
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_13_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_13_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_13_CPP_C14_RSSI_VEC_ANT1_13_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_13_CPP_C14_RSSI_VEC_ANT1_13_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_14_OFFSET                       0x00000168
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_14_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_14_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_14_CPP_C14_RSSI_VEC_ANT1_14_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_14_CPP_C14_RSSI_VEC_ANT1_14_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_15_OFFSET                       0x0000016C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_15_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_15_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_15_CPP_C14_RSSI_VEC_ANT1_15_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT1_15_CPP_C14_RSSI_VEC_ANT1_15_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_0_OFFSET                        0x00000170
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_0_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_0_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_0_CPP_C14_RSSI_VEC_ANT2_0_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_0_CPP_C14_RSSI_VEC_ANT2_0_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_1_OFFSET                        0x00000174
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_1_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_1_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_1_CPP_C14_RSSI_VEC_ANT2_1_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_1_CPP_C14_RSSI_VEC_ANT2_1_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_2_OFFSET                        0x00000178
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_2_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_2_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_2_CPP_C14_RSSI_VEC_ANT2_2_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_2_CPP_C14_RSSI_VEC_ANT2_2_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_3_OFFSET                        0x0000017C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_3_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_3_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_3_CPP_C14_RSSI_VEC_ANT2_3_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_3_CPP_C14_RSSI_VEC_ANT2_3_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_4_OFFSET                        0x00000180
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_4_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_4_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_4_CPP_C14_RSSI_VEC_ANT2_4_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_4_CPP_C14_RSSI_VEC_ANT2_4_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_5_OFFSET                        0x00000184
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_5_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_5_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_5_CPP_C14_RSSI_VEC_ANT2_5_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_5_CPP_C14_RSSI_VEC_ANT2_5_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_6_OFFSET                        0x00000188
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_6_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_6_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_6_CPP_C14_RSSI_VEC_ANT2_6_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_6_CPP_C14_RSSI_VEC_ANT2_6_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_7_OFFSET                        0x0000018C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_7_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_7_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_7_CPP_C14_RSSI_VEC_ANT2_7_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_7_CPP_C14_RSSI_VEC_ANT2_7_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_8_OFFSET                        0x00000190
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_8_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_8_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_8_CPP_C14_RSSI_VEC_ANT2_8_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_8_CPP_C14_RSSI_VEC_ANT2_8_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_9_OFFSET                        0x00000194
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_9_TYPE                          UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_9_RESERVED_MASK                 0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_9_CPP_C14_RSSI_VEC_ANT2_9_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_9_CPP_C14_RSSI_VEC_ANT2_9_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_10_OFFSET                       0x00000198
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_10_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_10_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_10_CPP_C14_RSSI_VEC_ANT2_10_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_10_CPP_C14_RSSI_VEC_ANT2_10_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_11_OFFSET                       0x0000019C
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_11_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_11_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_11_CPP_C14_RSSI_VEC_ANT2_11_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_11_CPP_C14_RSSI_VEC_ANT2_11_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_12_OFFSET                       0x000001A0
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_12_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_12_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_12_CPP_C14_RSSI_VEC_ANT2_12_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_12_CPP_C14_RSSI_VEC_ANT2_12_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_13_OFFSET                       0x000001A4
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_13_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_13_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_13_CPP_C14_RSSI_VEC_ANT2_13_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_13_CPP_C14_RSSI_VEC_ANT2_13_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_14_OFFSET                       0x000001A8
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_14_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_14_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_14_CPP_C14_RSSI_VEC_ANT2_14_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_14_CPP_C14_RSSI_VEC_ANT2_14_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_15_OFFSET                       0x000001AC
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_15_TYPE                         UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_15_RESERVED_MASK                0xFF000000
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_15_CPP_C14_RSSI_VEC_ANT2_15_SHIFT 0
#define    CPP_CLUSTER_CPP_C14_RSSI_VEC_ANT2_15_CPP_C14_RSSI_VEC_ANT2_15_MASK 0x00FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_ANT1_OFFSET                              0x000001B0
#define CPP_CLUSTER_CPP_C14_RSSI_ANT1_TYPE                                UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_ANT1_RESERVED_MASK                       0xF8000000
#define    CPP_CLUSTER_CPP_C14_RSSI_ANT1_CPP_C14_RSSI_ANT1_SHIFT          0
#define    CPP_CLUSTER_CPP_C14_RSSI_ANT1_CPP_C14_RSSI_ANT1_MASK           0x07FFFFFF

#define CPP_CLUSTER_CPP_C14_RSSI_ANT2_OFFSET                              0x000001B4
#define CPP_CLUSTER_CPP_C14_RSSI_ANT2_TYPE                                UInt32
#define CPP_CLUSTER_CPP_C14_RSSI_ANT2_RESERVED_MASK                       0xF8000000
#define    CPP_CLUSTER_CPP_C14_RSSI_ANT2_CPP_C14_RSSI_ANT2_SHIFT          0
#define    CPP_CLUSTER_CPP_C14_RSSI_ANT2_CPP_C14_RSSI_ANT2_MASK           0x07FFFFFF

#define CPP_CLUSTER_EQ_CAT14_OFFSET                                       0x000001B8
#define CPP_CLUSTER_EQ_CAT14_TYPE                                         UInt32
#define CPP_CLUSTER_EQ_CAT14_RESERVED_MASK                                0xFEC08000
#define    CPP_CLUSTER_EQ_CAT14_EQ_CAT14_SHIFT                            24
#define    CPP_CLUSTER_EQ_CAT14_EQ_CAT14_MASK                             0x01000000
#define    CPP_CLUSTER_EQ_CAT14_EQ_NBT_SHIFT                              20
#define    CPP_CLUSTER_EQ_CAT14_EQ_NBT_MASK                               0x00300000
#define    CPP_CLUSTER_EQ_CAT14_EQ_PN_SCALE_SHIFT                         16
#define    CPP_CLUSTER_EQ_CAT14_EQ_PN_SCALE_MASK                          0x000F0000
#define    CPP_CLUSTER_EQ_CAT14_EQ_PN_CAT14_SHIFT                         0
#define    CPP_CLUSTER_EQ_CAT14_EQ_PN_CAT14_MASK                          0x00007FFF

#define CPP_CLUSTER_CPP_C14_CHEST_INDEX_OFFSET                            0x000001BC
#define CPP_CLUSTER_CPP_C14_CHEST_INDEX_TYPE                              UInt32
#define CPP_CLUSTER_CPP_C14_CHEST_INDEX_RESERVED_MASK                     0xFFFFFFF0
#define    CPP_CLUSTER_CPP_C14_CHEST_INDEX_CPP_C14_CHEST_INDEX_SHIFT      0
#define    CPP_CLUSTER_CPP_C14_CHEST_INDEX_CPP_C14_CHEST_INDEX_MASK       0x0000000F

#define CPP_CLUSTER_CPP_C14_CHEST_ANT1_OFFSET                             0x000001C0
#define CPP_CLUSTER_CPP_C14_CHEST_ANT1_TYPE                               UInt32
#define CPP_CLUSTER_CPP_C14_CHEST_ANT1_RESERVED_MASK                      0xFF000000
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT1_CPP_C14_CHEST_Q_ANT1_SHIFT      12
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT1_CPP_C14_CHEST_Q_ANT1_MASK       0x00FFF000
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT1_CPP_C14_CHEST_I_ANT1_SHIFT      0
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT1_CPP_C14_CHEST_I_ANT1_MASK       0x00000FFF

#define CPP_CLUSTER_CPP_C14_CHEST_ANT2_OFFSET                             0x000001C4
#define CPP_CLUSTER_CPP_C14_CHEST_ANT2_TYPE                               UInt32
#define CPP_CLUSTER_CPP_C14_CHEST_ANT2_RESERVED_MASK                      0xFF000000
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT2_CPP_C14_CHEST_Q_ANT2_SHIFT      12
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT2_CPP_C14_CHEST_Q_ANT2_MASK       0x00FFF000
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT2_CPP_C14_CHEST_I_ANT2_SHIFT      0
#define    CPP_CLUSTER_CPP_C14_CHEST_ANT2_CPP_C14_CHEST_I_ANT2_MASK       0x00000FFF

#endif /* __BRCM_RDB_CPP_CLUSTER_H__ */



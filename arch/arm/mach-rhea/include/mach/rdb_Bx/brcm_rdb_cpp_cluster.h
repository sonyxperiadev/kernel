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
#define CPP_CLUSTER_CPP_CFG1_RESERVED_MASK                                0xFFFF8CCC
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_FREQ_SHIFT                         12
#define    CPP_CLUSTER_CPP_CFG1_CPP_BW_FREQ_MASK                          0x00007000
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
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ADJUST_ANT2_SHIFT                  25
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ADJUST_ANT2_MASK                   0x06000000
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ENABLE_ANT2_SHIFT                  24
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ENABLE_ANT2_MASK                   0x01000000
#define    CPP_CLUSTER_CPP_CFG3_CPP_EN_ANT2_SHIFT                         20
#define    CPP_CLUSTER_CPP_CFG3_CPP_EN_ANT2_MASK                          0x00100000
#define    CPP_CLUSTER_CPP_CFG3_CPP_CHIP_TRK_EN_ANT1_SHIFT                16
#define    CPP_CLUSTER_CPP_CFG3_CPP_CHIP_TRK_EN_ANT1_MASK                 0x00010000
#define    CPP_CLUSTER_CPP_CFG3_CPP_DLL_B2B_EN_SHIFT                      15
#define    CPP_CLUSTER_CPP_CFG3_CPP_DLL_B2B_EN_MASK                       0x00008000
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ADJUST_ANT1_SHIFT                  13
#define    CPP_CLUSTER_CPP_CFG3_CPP_TT_ADJUST_ANT1_MASK                   0x00006000
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
#define CPP_CLUSTER_SYM_CH_EN_RESERVED_MASK                               0xFFEEEEE0
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

#define CPP_CLUSTER_CPP_FREQUENCY_OFFSET                                  0x00000070
#define CPP_CLUSTER_CPP_FREQUENCY_TYPE                                    UInt32
#define CPP_CLUSTER_CPP_FREQUENCY_RESERVED_MASK                           0xF000F000
#define    CPP_CLUSTER_CPP_FREQUENCY_CPP_FREQ_ANT2_SHIFT                  16
#define    CPP_CLUSTER_CPP_FREQUENCY_CPP_FREQ_ANT2_MASK                   0x0FFF0000
#define    CPP_CLUSTER_CPP_FREQUENCY_CPP_FREQ_ANT1_SHIFT                  0
#define    CPP_CLUSTER_CPP_FREQUENCY_CPP_FREQ_ANT1_MASK                   0x00000FFF

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
#define CPP_CLUSTER_CPP_DLL_MAN_ADJ_RESERVED_MASK                         0xFE00FE00
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_TIME_BETWEEN_ADJ_ANT2_SHIFT        24
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_TIME_BETWEEN_ADJ_ANT2_MASK         0x01000000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT2_SHIFT         16
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT2_MASK          0x00FF0000
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_TIME_BETWEEN_ADJ_ANT1_SHIFT        8
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_TIME_BETWEEN_ADJ_ANT1_MASK         0x00000100
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT1_SHIFT         0
#define    CPP_CLUSTER_CPP_DLL_MAN_ADJ_NUM_ADJ_MINUS_1_ANT1_MASK          0x000000FF

#endif /* __BRCM_RDB_CPP_CLUSTER_H__ */



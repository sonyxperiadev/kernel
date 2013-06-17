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

#ifndef __BRCM_RDB_SYM_DIV_H__
#define __BRCM_RDB_SYM_DIV_H__

#define SYM_DIV_LEGACY_CPP_R1_OFFSET                                      0x00000000
#define SYM_DIV_LEGACY_CPP_R1_TYPE                                        UInt32
#define SYM_DIV_LEGACY_CPP_R1_RESERVED_MASK                               0xFFFFFFFE
#define    SYM_DIV_LEGACY_CPP_R1_LEGACY_CPP_R1_SHIFT                      0
#define    SYM_DIV_LEGACY_CPP_R1_LEGACY_CPP_R1_MASK                       0x00000001

#define SYM_DIV_GC_OVSF_CFG_OFFSET                                        0x00000004
#define SYM_DIV_GC_OVSF_CFG_TYPE                                          UInt32
#define SYM_DIV_GC_OVSF_CFG_RESERVED_MASK                                 0x0E00E000
#define    SYM_DIV_GC_OVSF_CFG_GC_OFFSET_SHIFT                            28
#define    SYM_DIV_GC_OVSF_CFG_GC_OFFSET_MASK                             0xF0000000
#define    SYM_DIV_GC_OVSF_CFG_PRIM_GC_INDX_SHIFT                         16
#define    SYM_DIV_GC_OVSF_CFG_PRIM_GC_INDX_MASK                          0x01FF0000
#define    SYM_DIV_GC_OVSF_CFG_OVSF_ID_SHIFT                              4
#define    SYM_DIV_GC_OVSF_CFG_OVSF_ID_MASK                               0x00001FF0
#define    SYM_DIV_GC_OVSF_CFG_OVSF_SF_SHIFT                              0
#define    SYM_DIV_GC_OVSF_CFG_OVSF_SF_MASK                               0x0000000F

#define SYM_DIV_GC_OVSF_MASK_OFFSET                                       0x00000008
#define SYM_DIV_GC_OVSF_MASK_TYPE                                         UInt32
#define SYM_DIV_GC_OVSF_MASK_RESERVED_MASK                                0xEF03EFF3
#define    SYM_DIV_GC_OVSF_MASK_OVSF_MASK_GEN_SHIFT                       28
#define    SYM_DIV_GC_OVSF_MASK_OVSF_MASK_GEN_MASK                        0x10000000
#define    SYM_DIV_GC_OVSF_MASK_OVSF_MODULE_ID_SHIFT                      18
#define    SYM_DIV_GC_OVSF_MASK_OVSF_MODULE_ID_MASK                       0x00FC0000
#define    SYM_DIV_GC_OVSF_MASK_GC_INIT_SHIFT                             12
#define    SYM_DIV_GC_OVSF_MASK_GC_INIT_MASK                              0x00001000
#define    SYM_DIV_GC_OVSF_MASK_GC_MODULE_ID_SHIFT                        2
#define    SYM_DIV_GC_OVSF_MASK_GC_MODULE_ID_MASK                         0x0000000C

#define SYM_DIV_DIV_CFG_OFFSET                                            0x0000000C
#define SYM_DIV_DIV_CFG_TYPE                                              UInt32
#define SYM_DIV_DIV_CFG_RESERVED_MASK                                     0xC000C000
#define    SYM_DIV_DIV_CFG_HS_SCCH_NBT_SEL_SHIFT                          29
#define    SYM_DIV_DIV_CFG_HS_SCCH_NBT_SEL_MASK                           0x20000000
#define    SYM_DIV_DIV_CFG_CPC_MODE_SHIFT                                 28
#define    SYM_DIV_DIV_CFG_CPC_MODE_MASK                                  0x10000000
#define    SYM_DIV_DIV_CFG_CPC_CIR_BUFF_INDX_SHIFT                        24
#define    SYM_DIV_DIV_CFG_CPC_CIR_BUFF_INDX_MASK                         0x0F000000
#define    SYM_DIV_DIV_CFG_CPC_CH_START_POS_SHIFT                         20
#define    SYM_DIV_DIV_CFG_CPC_CH_START_POS_MASK                          0x00F00000
#define    SYM_DIV_DIV_CFG_CPC_CH_LENGTH_SHIFT                            19
#define    SYM_DIV_DIV_CFG_CPC_CH_LENGTH_MASK                             0x00080000
#define    SYM_DIV_DIV_CFG_HS_SCCH_NBT_SHIFT                              16
#define    SYM_DIV_DIV_CFG_HS_SCCH_NBT_MASK                               0x00070000
#define    SYM_DIV_DIV_CFG_CL1_PHASE_INDX_SHIFT                           12
#define    SYM_DIV_DIV_CFG_CL1_PHASE_INDX_MASK                            0x00003000
#define    SYM_DIV_DIV_CFG_DIV_CH_LENGTH_SHIFT                            8
#define    SYM_DIV_DIV_CFG_DIV_CH_LENGTH_MASK                             0x00000F00
#define    SYM_DIV_DIV_CFG_DIV_CH_START_POS_SHIFT                         4
#define    SYM_DIV_DIV_CFG_DIV_CH_START_POS_MASK                          0x000000F0
#define    SYM_DIV_DIV_CFG_DIV4_MODE_SHIFT                                2
#define    SYM_DIV_DIV_CFG_DIV4_MODE_MASK                                 0x0000000C
#define    SYM_DIV_DIV_CFG_DIV_MODE_SHIFT                                 0
#define    SYM_DIV_DIV_CFG_DIV_MODE_MASK                                  0x00000003

#define SYM_DIV_SYM_CH_EN_OFFSET                                          0x00000010
#define SYM_DIV_SYM_CH_EN_TYPE                                            UInt32
#define SYM_DIV_SYM_CH_EN_RESERVED_MASK                                   0xFEEEEEE0
#define    SYM_DIV_SYM_CH_EN_SYM_ACCUM_DEL_EN_SHIFT                       24
#define    SYM_DIV_SYM_CH_EN_SYM_ACCUM_DEL_EN_MASK                        0x01000000
#define    SYM_DIV_SYM_CH_EN_SYM_SF_CPICH_SHIFT                           20
#define    SYM_DIV_SYM_CH_EN_SYM_SF_CPICH_MASK                            0x00100000
#define    SYM_DIV_SYM_CH_EN_PDSCH_H2_SEL_SHIFT                           16
#define    SYM_DIV_SYM_CH_EN_PDSCH_H2_SEL_MASK                            0x00010000
#define    SYM_DIV_SYM_CH_EN_SCCH_H2_SEL_SHIFT                            12
#define    SYM_DIV_SYM_CH_EN_SCCH_H2_SEL_MASK                             0x00001000
#define    SYM_DIV_SYM_CH_EN_CPICH_H2_SEL_SHIFT                           8
#define    SYM_DIV_SYM_CH_EN_CPICH_H2_SEL_MASK                            0x00000100
#define    SYM_DIV_SYM_CH_EN_PDSCH_CH_EN_ANT2_SHIFT                       4
#define    SYM_DIV_SYM_CH_EN_PDSCH_CH_EN_ANT2_MASK                        0x00000010
#define    SYM_DIV_SYM_CH_EN_SCCH_CH_EN_ANT2_SHIFT                        3
#define    SYM_DIV_SYM_CH_EN_SCCH_CH_EN_ANT2_MASK                         0x00000008
#define    SYM_DIV_SYM_CH_EN_SYM_CH_EN_SHIFT                              0
#define    SYM_DIV_SYM_CH_EN_SYM_CH_EN_MASK                               0x00000007

#define SYM_DIV_SYM_CPICHS_OFFSET                                         0x00000014
#define SYM_DIV_SYM_CPICHS_TYPE                                           UInt32
#define SYM_DIV_SYM_CPICHS_RESERVED_MASK                                  0xF000F000
#define    SYM_DIV_SYM_CPICHS_SYM_CPICH_S2_SHIFT                          16
#define    SYM_DIV_SYM_CPICHS_SYM_CPICH_S2_MASK                           0x0FFF0000
#define    SYM_DIV_SYM_CPICHS_SYM_CPICH_S1_SHIFT                          0
#define    SYM_DIV_SYM_CPICHS_SYM_CPICH_S1_MASK                           0x00000FFF

#define SYM_DIV_SYM_CPICHN_A1_OFFSET                                      0x00000018
#define SYM_DIV_SYM_CPICHN_A1_TYPE                                        UInt32
#define SYM_DIV_SYM_CPICHN_A1_RESERVED_MASK                               0xFFF80000
#define    SYM_DIV_SYM_CPICHN_A1_SYM_CPICH_N1_SHIFT                       0
#define    SYM_DIV_SYM_CPICHN_A1_SYM_CPICH_N1_MASK                        0x0007FFFF

#define SYM_DIV_SYM_CPICHN_A2_OFFSET                                      0x0000001C
#define SYM_DIV_SYM_CPICHN_A2_TYPE                                        UInt32
#define SYM_DIV_SYM_CPICHN_A2_RESERVED_MASK                               0xFFF80000
#define    SYM_DIV_SYM_CPICHN_A2_SYM_CPICH_N2_SHIFT                       0
#define    SYM_DIV_SYM_CPICHN_A2_SYM_CPICH_N2_MASK                        0x0007FFFF

#define SYM_DIV_DEBUG_CTRL_OFFSET                                         0x00000020
#define SYM_DIV_DEBUG_CTRL_TYPE                                           UInt32
#define SYM_DIV_DEBUG_CTRL_RESERVED_MASK                                  0xFFC0FFFF
#define    SYM_DIV_DEBUG_CTRL_SYM_PDSCH_SCCH_SEL_SHIFT                    21
#define    SYM_DIV_DEBUG_CTRL_SYM_PDSCH_SCCH_SEL_MASK                     0x00200000
#define    SYM_DIV_DEBUG_CTRL_SYM_ANT_SEL_SHIFT                           20
#define    SYM_DIV_DEBUG_CTRL_SYM_ANT_SEL_MASK                            0x00100000
#define    SYM_DIV_DEBUG_CTRL_SYM_CH_INDX_SHIFT                           16
#define    SYM_DIV_DEBUG_CTRL_SYM_CH_INDX_MASK                            0x000F0000

#define SYM_DIV_SYM_VP_GAIN_OFFSET                                        0x00000024
#define SYM_DIV_SYM_VP_GAIN_TYPE                                          UInt32
#define SYM_DIV_SYM_VP_GAIN_RESERVED_MASK                                 0xFFFCFCCC
#define    SYM_DIV_SYM_VP_GAIN_SYM_PDSCH_VP_GAIN_SEL_SHIFT                16
#define    SYM_DIV_SYM_VP_GAIN_SYM_PDSCH_VP_GAIN_SEL_MASK                 0x00030000
#define    SYM_DIV_SYM_VP_GAIN_SYM_SCCH_VP_GAIN_SEL_SHIFT                 8
#define    SYM_DIV_SYM_VP_GAIN_SYM_SCCH_VP_GAIN_SEL_MASK                  0x00000300
#define    SYM_DIV_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_N_SEL_SHIFT              4
#define    SYM_DIV_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_N_SEL_MASK               0x00000030
#define    SYM_DIV_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_S_SEL_SHIFT              0
#define    SYM_DIV_SYM_VP_GAIN_SYM_CPICH_VP_GAIN_S_SEL_MASK               0x00000003

#define SYM_DIV_LEGACY_CHANNEL_15_OFFSET                                  0x00000028
#define SYM_DIV_LEGACY_CHANNEL_15_TYPE                                    UInt32
#define SYM_DIV_LEGACY_CHANNEL_15_RESERVED_MASK                           0xFFFFFFFE
#define    SYM_DIV_LEGACY_CHANNEL_15_LEGACY_CHANNEL_15_SHIFT              0
#define    SYM_DIV_LEGACY_CHANNEL_15_LEGACY_CHANNEL_15_MASK               0x00000001

#endif /* __BRCM_RDB_SYM_DIV_H__ */



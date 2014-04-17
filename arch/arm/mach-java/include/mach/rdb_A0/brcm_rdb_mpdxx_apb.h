/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_MPDXX_APB_H__
#define __BRCM_RDB_MPDXX_APB_H__

#define MPDXX_APB_START0_OFFSET                                           0x00000000
#define MPDXX_APB_START0_TYPE                                             UInt32
#define MPDXX_APB_START0_RESERVED_MASK                                    0xFFF0C000
#define    MPDXX_APB_START0_SRCH_SLOT_START_0_SHIFT                       16
#define    MPDXX_APB_START0_SRCH_SLOT_START_0_MASK                        0x000F0000
#define    MPDXX_APB_START0_SRCH_SAMPLE_START_0_SHIFT                     0
#define    MPDXX_APB_START0_SRCH_SAMPLE_START_0_MASK                      0x00003FFF

#define MPDXX_APB_START1_OFFSET                                           0x00000004
#define MPDXX_APB_START1_TYPE                                             UInt32
#define MPDXX_APB_START1_RESERVED_MASK                                    0xFFF0C000
#define    MPDXX_APB_START1_SRCH_SLOT_START_1_SHIFT                       16
#define    MPDXX_APB_START1_SRCH_SLOT_START_1_MASK                        0x000F0000
#define    MPDXX_APB_START1_SRCH_SAMPLE_START_1_SHIFT                     0
#define    MPDXX_APB_START1_SRCH_SAMPLE_START_1_MASK                      0x00003FFF

#define MPDXX_APB_START2_OFFSET                                           0x00000008
#define MPDXX_APB_START2_TYPE                                             UInt32
#define MPDXX_APB_START2_RESERVED_MASK                                    0xFFF0C000
#define    MPDXX_APB_START2_SRCH_SLOT_START_2_SHIFT                       16
#define    MPDXX_APB_START2_SRCH_SLOT_START_2_MASK                        0x000F0000
#define    MPDXX_APB_START2_SRCH_SAMPLE_START_2_SHIFT                     0
#define    MPDXX_APB_START2_SRCH_SAMPLE_START_2_MASK                      0x00003FFF

#define MPDXX_APB_INTV_OFFSET                                             0x0000000C
#define MPDXX_APB_INTV_TYPE                                               UInt32
#define MPDXX_APB_INTV_RESERVED_MASK                                      0xFFC0F080
#define    MPDXX_APB_INTV_SRCH_INTV_LEN_SHIFT                             16
#define    MPDXX_APB_INTV_SRCH_INTV_LEN_MASK                              0x003F0000
#define    MPDXX_APB_INTV_SRCH_N_GC_INDX_SHIFT                            8
#define    MPDXX_APB_INTV_SRCH_N_GC_INDX_MASK                             0x00000F00
#define    MPDXX_APB_INTV_SRCH_INTV_SW_GO_SHIFT                           6
#define    MPDXX_APB_INTV_SRCH_INTV_SW_GO_MASK                            0x00000040
#define    MPDXX_APB_INTV_SRCH_INTV_MODE_SHIFT                            5
#define    MPDXX_APB_INTV_SRCH_INTV_MODE_MASK                             0x00000020
#define    MPDXX_APB_INTV_SRCH_INTV_GEN_EN_SHIFT                          4
#define    MPDXX_APB_INTV_SRCH_INTV_GEN_EN_MASK                           0x00000010
#define    MPDXX_APB_INTV_SRCH_INTV_START_SHIFT                           0
#define    MPDXX_APB_INTV_SRCH_INTV_START_MASK                            0x0000000F

#define MPDXX_APB_CTRL_OFFSET                                             0x00000010
#define MPDXX_APB_CTRL_TYPE                                               UInt32
#define MPDXX_APB_CTRL_RESERVED_MASK                                      0x000ECC80
#define    MPDXX_APB_CTRL_ANT2_EN_SHIFT                                   20
#define    MPDXX_APB_CTRL_ANT2_EN_MASK                                    0xFFF00000
#define    MPDXX_APB_CTRL_ANT1_EN_R1_SHIFT                                16
#define    MPDXX_APB_CTRL_ANT1_EN_R1_MASK                                 0x00010000
#define    MPDXX_APB_CTRL_SRCH_N_CELL_SHIFT                               12
#define    MPDXX_APB_CTRL_SRCH_N_CELL_MASK                                0x00003000
#define    MPDXX_APB_CTRL_SRCH_NNCI_SHIFT                                 8
#define    MPDXX_APB_CTRL_SRCH_NNCI_MASK                                  0x00000300
#define    MPDXX_APB_CTRL_SRCH_NCI_SHIFT                                  4
#define    MPDXX_APB_CTRL_SRCH_NCI_MASK                                   0x00000070
#define    MPDXX_APB_CTRL_MODE2_POWER_SAVE_SHIFT                          3
#define    MPDXX_APB_CTRL_MODE2_POWER_SAVE_MASK                           0x00000008
#define    MPDXX_APB_CTRL_SRCH_EN_SHIFT                                   2
#define    MPDXX_APB_CTRL_SRCH_EN_MASK                                    0x00000004
#define    MPDXX_APB_CTRL_SRCH_MODE_SHIFT                                 0
#define    MPDXX_APB_CTRL_SRCH_MODE_MASK                                  0x00000003

#define MPDXX_APB_PRIM_GC_INDX_ARRAY01_OFFSET                             0x00000014
#define MPDXX_APB_PRIM_GC_INDX_ARRAY01_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAY01_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY01_PRIM_GC_INDX_ARRAY_1_SHIFT      16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY01_PRIM_GC_INDX_ARRAY_1_MASK       0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY01_PRIM_GC_INDX_ARRAY_0_SHIFT      0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY01_PRIM_GC_INDX_ARRAY_0_MASK       0x000001FF

#define MPDXX_APB_PRIM_GC_INDX_ARRAY23_OFFSET                             0x00000018
#define MPDXX_APB_PRIM_GC_INDX_ARRAY23_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAY23_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY23_PRIM_GC_INDX_ARRAY_3_SHIFT      16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY23_PRIM_GC_INDX_ARRAY_3_MASK       0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY23_PRIM_GC_INDX_ARRAY_2_SHIFT      0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY23_PRIM_GC_INDX_ARRAY_2_MASK       0x000001FF

#define MPDXX_APB_PRIM_GC_INDX_ARRAY45_OFFSET                             0x0000001C
#define MPDXX_APB_PRIM_GC_INDX_ARRAY45_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAY45_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY45_PRIM_GC_INDX_ARRAY_5_SHIFT      16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY45_PRIM_GC_INDX_ARRAY_5_MASK       0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY45_PRIM_GC_INDX_ARRAY_4_SHIFT      0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY45_PRIM_GC_INDX_ARRAY_4_MASK       0x000001FF

#define MPDXX_APB_PRIM_GC_INDX_ARRAY67_OFFSET                             0x00000020
#define MPDXX_APB_PRIM_GC_INDX_ARRAY67_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAY67_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY67_PRIM_GC_INDX_ARRAY_7_SHIFT      16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY67_PRIM_GC_INDX_ARRAY_7_MASK       0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY67_PRIM_GC_INDX_ARRAY_6_SHIFT      0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY67_PRIM_GC_INDX_ARRAY_6_MASK       0x000001FF

#define MPDXX_APB_PRIM_GC_INDX_ARRAY89_OFFSET                             0x00000024
#define MPDXX_APB_PRIM_GC_INDX_ARRAY89_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAY89_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY89_PRIM_GC_INDX_ARRAY_9_SHIFT      16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY89_PRIM_GC_INDX_ARRAY_9_MASK       0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY89_PRIM_GC_INDX_ARRAY_8_SHIFT      0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAY89_PRIM_GC_INDX_ARRAY_8_MASK       0x000001FF

#define MPDXX_APB_PRIM_GC_INDX_ARRAYAB_OFFSET                             0x00000028
#define MPDXX_APB_PRIM_GC_INDX_ARRAYAB_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAYAB_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYAB_PRIM_GC_INDX_ARRAY_11_SHIFT     16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYAB_PRIM_GC_INDX_ARRAY_11_MASK      0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYAB_PRIM_GC_INDX_ARRAY_10_SHIFT     0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYAB_PRIM_GC_INDX_ARRAY_10_MASK      0x000001FF

#define MPDXX_APB_STATUS_OFFSET                                           0x0000002C
#define MPDXX_APB_STATUS_TYPE                                             UInt32
#define MPDXX_APB_STATUS_RESERVED_MASK                                    0xFF00000C
#define    MPDXX_APB_STATUS_MP_MODE2_MT_SLOT_SHIFT                        20
#define    MPDXX_APB_STATUS_MP_MODE2_MT_SLOT_MASK                         0x00F00000
#define    MPDXX_APB_STATUS_MP_GC_INDX_MAX_POS_SHIFT                      16
#define    MPDXX_APB_STATUS_MP_GC_INDX_MAX_POS_MASK                       0x000F0000
#define    MPDXX_APB_STATUS_REPORT_VALID_SHIFT                            8
#define    MPDXX_APB_STATUS_REPORT_VALID_MASK                             0x0000FF00
#define    MPDXX_APB_STATUS_CLEAR_VALID_SHIFT                             4
#define    MPDXX_APB_STATUS_CLEAR_VALID_MASK                              0x000000F0
#define    MPDXX_APB_STATUS_MPD_BUSY_SHIFT                                1
#define    MPDXX_APB_STATUS_MPD_BUSY_MASK                                 0x00000002
#define    MPDXX_APB_STATUS_SRCH_INTV_CNTL_BUSY_SHIFT                     0
#define    MPDXX_APB_STATUS_SRCH_INTV_CNTL_BUSY_MASK                      0x00000001

#define MPDXX_APB_MP_MODE2_MAX_OFFSET                                     0x00000030
#define MPDXX_APB_MP_MODE2_MAX_TYPE                                       UInt32
#define MPDXX_APB_MP_MODE2_MAX_RESERVED_MASK                              0xFFC00000
#define    MPDXX_APB_MP_MODE2_MAX_MP_MODE2_MAX_POS_SHIFT                  16
#define    MPDXX_APB_MP_MODE2_MAX_MP_MODE2_MAX_POS_MASK                   0x003F0000
#define    MPDXX_APB_MP_MODE2_MAX_MP_MODE2_MAX_ENG_SHIFT                  0
#define    MPDXX_APB_MP_MODE2_MAX_MP_MODE2_MAX_ENG_MASK                   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_0_OFFSET                                   0x00000034
#define MPDXX_APB_MP_ENERGY_C0_0_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_0_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_0_MP_INDX_C0_TABLE_0_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_0_MP_INDX_C0_TABLE_0_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_0_MP_ENERGY_C0_TABLE_0_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_0_MP_ENERGY_C0_TABLE_0_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_1_OFFSET                                   0x00000038
#define MPDXX_APB_MP_ENERGY_C0_1_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_1_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_1_MP_INDX_C0_TABLE_1_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_1_MP_INDX_C0_TABLE_1_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_1_MP_ENERGY_C0_TABLE_1_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_1_MP_ENERGY_C0_TABLE_1_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_2_OFFSET                                   0x0000003C
#define MPDXX_APB_MP_ENERGY_C0_2_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_2_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_2_MP_INDX_C0_TABLE_2_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_2_MP_INDX_C0_TABLE_2_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_2_MP_ENERGY_C0_TABLE_2_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_2_MP_ENERGY_C0_TABLE_2_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_3_OFFSET                                   0x00000040
#define MPDXX_APB_MP_ENERGY_C0_3_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_3_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_3_MP_INDX_C0_TABLE_3_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_3_MP_INDX_C0_TABLE_3_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_3_MP_ENERGY_C0_TABLE_3_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_3_MP_ENERGY_C0_TABLE_3_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_4_OFFSET                                   0x00000044
#define MPDXX_APB_MP_ENERGY_C0_4_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_4_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_4_MP_INDX_C0_TABLE_4_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_4_MP_INDX_C0_TABLE_4_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_4_MP_ENERGY_C0_TABLE_4_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_4_MP_ENERGY_C0_TABLE_4_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_5_OFFSET                                   0x00000048
#define MPDXX_APB_MP_ENERGY_C0_5_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_5_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_5_MP_INDX_C0_TABLE_5_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_5_MP_INDX_C0_TABLE_5_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_5_MP_ENERGY_C0_TABLE_5_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_5_MP_ENERGY_C0_TABLE_5_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_6_OFFSET                                   0x0000004C
#define MPDXX_APB_MP_ENERGY_C0_6_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_6_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_6_MP_INDX_C0_TABLE_6_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_6_MP_INDX_C0_TABLE_6_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_6_MP_ENERGY_C0_TABLE_6_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_6_MP_ENERGY_C0_TABLE_6_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C0_7_OFFSET                                   0x00000050
#define MPDXX_APB_MP_ENERGY_C0_7_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C0_7_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C0_7_MP_INDX_C0_TABLE_7_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C0_7_MP_INDX_C0_TABLE_7_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C0_7_MP_ENERGY_C0_TABLE_7_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C0_7_MP_ENERGY_C0_TABLE_7_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_0_OFFSET                                   0x00000054
#define MPDXX_APB_MP_ENERGY_C1_0_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_0_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_0_MP_INDX_C1_TABLE_0_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_0_MP_INDX_C1_TABLE_0_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_0_MP_ENERGY_C1_TABLE_0_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_0_MP_ENERGY_C1_TABLE_0_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_1_OFFSET                                   0x00000058
#define MPDXX_APB_MP_ENERGY_C1_1_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_1_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_1_MP_INDX_C1_TABLE_1_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_1_MP_INDX_C1_TABLE_1_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_1_MP_ENERGY_C1_TABLE_1_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_1_MP_ENERGY_C1_TABLE_1_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_2_OFFSET                                   0x0000005C
#define MPDXX_APB_MP_ENERGY_C1_2_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_2_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_2_MP_INDX_C1_TABLE_2_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_2_MP_INDX_C1_TABLE_2_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_2_MP_ENERGY_C1_TABLE_2_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_2_MP_ENERGY_C1_TABLE_2_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_3_OFFSET                                   0x00000060
#define MPDXX_APB_MP_ENERGY_C1_3_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_3_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_3_MP_INDX_C1_TABLE_3_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_3_MP_INDX_C1_TABLE_3_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_3_MP_ENERGY_C1_TABLE_3_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_3_MP_ENERGY_C1_TABLE_3_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_4_OFFSET                                   0x00000064
#define MPDXX_APB_MP_ENERGY_C1_4_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_4_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_4_MP_INDX_C1_TABLE_4_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_4_MP_INDX_C1_TABLE_4_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_4_MP_ENERGY_C1_TABLE_4_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_4_MP_ENERGY_C1_TABLE_4_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_5_OFFSET                                   0x00000068
#define MPDXX_APB_MP_ENERGY_C1_5_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_5_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_5_MP_INDX_C1_TABLE_5_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_5_MP_INDX_C1_TABLE_5_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_5_MP_ENERGY_C1_TABLE_5_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_5_MP_ENERGY_C1_TABLE_5_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_6_OFFSET                                   0x0000006C
#define MPDXX_APB_MP_ENERGY_C1_6_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_6_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_6_MP_INDX_C1_TABLE_6_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_6_MP_INDX_C1_TABLE_6_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_6_MP_ENERGY_C1_TABLE_6_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_6_MP_ENERGY_C1_TABLE_6_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C1_7_OFFSET                                   0x00000070
#define MPDXX_APB_MP_ENERGY_C1_7_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C1_7_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C1_7_MP_INDX_C1_TABLE_7_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C1_7_MP_INDX_C1_TABLE_7_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C1_7_MP_ENERGY_C1_TABLE_7_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C1_7_MP_ENERGY_C1_TABLE_7_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_0_OFFSET                                   0x00000074
#define MPDXX_APB_MP_ENERGY_C2_0_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_0_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_0_MP_INDX_C2_TABLE_0_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_0_MP_INDX_C2_TABLE_0_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_0_MP_ENERGY_C2_TABLE_0_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_0_MP_ENERGY_C2_TABLE_0_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_1_OFFSET                                   0x00000078
#define MPDXX_APB_MP_ENERGY_C2_1_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_1_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_1_MP_INDX_C2_TABLE_1_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_1_MP_INDX_C2_TABLE_1_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_1_MP_ENERGY_C2_TABLE_1_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_1_MP_ENERGY_C2_TABLE_1_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_2_OFFSET                                   0x0000007C
#define MPDXX_APB_MP_ENERGY_C2_2_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_2_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_2_MP_INDX_C2_TABLE_2_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_2_MP_INDX_C2_TABLE_2_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_2_MP_ENERGY_C2_TABLE_2_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_2_MP_ENERGY_C2_TABLE_2_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_3_OFFSET                                   0x00000080
#define MPDXX_APB_MP_ENERGY_C2_3_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_3_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_3_MP_INDX_C2_TABLE_3_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_3_MP_INDX_C2_TABLE_3_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_3_MP_ENERGY_C2_TABLE_3_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_3_MP_ENERGY_C2_TABLE_3_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_4_OFFSET                                   0x00000084
#define MPDXX_APB_MP_ENERGY_C2_4_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_4_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_4_MP_INDX_C2_TABLE_4_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_4_MP_INDX_C2_TABLE_4_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_4_MP_ENERGY_C2_TABLE_4_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_4_MP_ENERGY_C2_TABLE_4_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_5_OFFSET                                   0x00000088
#define MPDXX_APB_MP_ENERGY_C2_5_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_5_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_5_MP_INDX_C2_TABLE_5_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_5_MP_INDX_C2_TABLE_5_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_5_MP_ENERGY_C2_TABLE_5_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_5_MP_ENERGY_C2_TABLE_5_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_6_OFFSET                                   0x0000008C
#define MPDXX_APB_MP_ENERGY_C2_6_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_6_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_6_MP_INDX_C2_TABLE_6_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_6_MP_INDX_C2_TABLE_6_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_6_MP_ENERGY_C2_TABLE_6_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_6_MP_ENERGY_C2_TABLE_6_MASK             0x0000FFFF

#define MPDXX_APB_MP_ENERGY_C2_7_OFFSET                                   0x00000090
#define MPDXX_APB_MP_ENERGY_C2_7_TYPE                                     UInt32
#define MPDXX_APB_MP_ENERGY_C2_7_RESERVED_MASK                            0xFFC00000
#define    MPDXX_APB_MP_ENERGY_C2_7_MP_INDX_C2_TABLE_7_SHIFT              16
#define    MPDXX_APB_MP_ENERGY_C2_7_MP_INDX_C2_TABLE_7_MASK               0x003F0000
#define    MPDXX_APB_MP_ENERGY_C2_7_MP_ENERGY_C2_TABLE_7_SHIFT            0
#define    MPDXX_APB_MP_ENERGY_C2_7_MP_ENERGY_C2_TABLE_7_MASK             0x0000FFFF

#define MPDXX_APB_MP_IO_ENG_0_OFFSET                                      0x00000094
#define MPDXX_APB_MP_IO_ENG_0_TYPE                                        UInt32
#define MPDXX_APB_MP_IO_ENG_0_RESERVED_MASK                               0xFFFF0000
#define    MPDXX_APB_MP_IO_ENG_0_MP_C0_IO_ENG_SHIFT                       0
#define    MPDXX_APB_MP_IO_ENG_0_MP_C0_IO_ENG_MASK                        0x0000FFFF

#define MPDXX_APB_MP_IO_ENG_1_OFFSET                                      0x00000098
#define MPDXX_APB_MP_IO_ENG_1_TYPE                                        UInt32
#define MPDXX_APB_MP_IO_ENG_1_RESERVED_MASK                               0xFFFF0000
#define    MPDXX_APB_MP_IO_ENG_1_MP_C1_IO_ENG_SHIFT                       0
#define    MPDXX_APB_MP_IO_ENG_1_MP_C1_IO_ENG_MASK                        0x0000FFFF

#define MPDXX_APB_MP_IO_ENG_2_OFFSET                                      0x0000009C
#define MPDXX_APB_MP_IO_ENG_2_TYPE                                        UInt32
#define MPDXX_APB_MP_IO_ENG_2_RESERVED_MASK                               0xFFFF0000
#define    MPDXX_APB_MP_IO_ENG_2_MP_C2_IO_ENG_SHIFT                       0
#define    MPDXX_APB_MP_IO_ENG_2_MP_C2_IO_ENG_MASK                        0x0000FFFF

#define MPDXX_APB_MP_MODE2_IO_ENG_OFFSET                                  0x000000A0
#define MPDXX_APB_MP_MODE2_IO_ENG_TYPE                                    UInt32
#define MPDXX_APB_MP_MODE2_IO_ENG_RESERVED_MASK                           0xFFFF0000
#define    MPDXX_APB_MP_MODE2_IO_ENG_MP_MODE2_IO_ENG_SHIFT                0
#define    MPDXX_APB_MP_MODE2_IO_ENG_MP_MODE2_IO_ENG_MASK                 0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C0_0_1_OFFSET                             0x000000A4
#define MPDXX_APB_MP_ENERGY_MIN_C0_0_1_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C0_0_1_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_0_1_MP_ENERGY_MIN_C0_TABLE_1_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C0_0_1_MP_ENERGY_MIN_C0_TABLE_1_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_0_1_MP_ENERGY_MIN_C0_TABLE_0_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C0_0_1_MP_ENERGY_MIN_C0_TABLE_0_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C0_2_3_OFFSET                             0x000000A8
#define MPDXX_APB_MP_ENERGY_MIN_C0_2_3_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C0_2_3_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_2_3_MP_ENERGY_MIN_C0_TABLE_3_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C0_2_3_MP_ENERGY_MIN_C0_TABLE_3_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_2_3_MP_ENERGY_MIN_C0_TABLE_2_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C0_2_3_MP_ENERGY_MIN_C0_TABLE_2_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C0_4_5_OFFSET                             0x000000AC
#define MPDXX_APB_MP_ENERGY_MIN_C0_4_5_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C0_4_5_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_4_5_MP_ENERGY_MIN_C0_TABLE_5_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C0_4_5_MP_ENERGY_MIN_C0_TABLE_5_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_4_5_MP_ENERGY_MIN_C0_TABLE_4_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C0_4_5_MP_ENERGY_MIN_C0_TABLE_4_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C0_6_7_OFFSET                             0x000000B0
#define MPDXX_APB_MP_ENERGY_MIN_C0_6_7_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C0_6_7_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_6_7_MP_ENERGY_MIN_C0_TABLE_7_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C0_6_7_MP_ENERGY_MIN_C0_TABLE_7_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C0_6_7_MP_ENERGY_MIN_C0_TABLE_6_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C0_6_7_MP_ENERGY_MIN_C0_TABLE_6_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C1_0_1_OFFSET                             0x000000B4
#define MPDXX_APB_MP_ENERGY_MIN_C1_0_1_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C1_0_1_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_0_1_MP_ENERGY_MIN_C1_TABLE_1_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C1_0_1_MP_ENERGY_MIN_C1_TABLE_1_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_0_1_MP_ENERGY_MIN_C1_TABLE_0_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C1_0_1_MP_ENERGY_MIN_C1_TABLE_0_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C1_2_3_OFFSET                             0x000000B8
#define MPDXX_APB_MP_ENERGY_MIN_C1_2_3_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C1_2_3_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_2_3_MP_ENERGY_MIN_C1_TABLE_3_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C1_2_3_MP_ENERGY_MIN_C1_TABLE_3_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_2_3_MP_ENERGY_MIN_C1_TABLE_2_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C1_2_3_MP_ENERGY_MIN_C1_TABLE_2_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C1_4_5_OFFSET                             0x000000BC
#define MPDXX_APB_MP_ENERGY_MIN_C1_4_5_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C1_4_5_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_4_5_MP_ENERGY_MIN_C1_TABLE_5_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C1_4_5_MP_ENERGY_MIN_C1_TABLE_5_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_4_5_MP_ENERGY_MIN_C1_TABLE_4_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C1_4_5_MP_ENERGY_MIN_C1_TABLE_4_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C1_6_7_OFFSET                             0x000000C0
#define MPDXX_APB_MP_ENERGY_MIN_C1_6_7_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C1_6_7_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_6_7_MP_ENERGY_MIN_C1_TABLE_7_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C1_6_7_MP_ENERGY_MIN_C1_TABLE_7_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C1_6_7_MP_ENERGY_MIN_C1_TABLE_6_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C1_6_7_MP_ENERGY_MIN_C1_TABLE_6_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C2_0_1_OFFSET                             0x000000C4
#define MPDXX_APB_MP_ENERGY_MIN_C2_0_1_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C2_0_1_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_0_1_MP_ENERGY_MIN_C2_TABLE_1_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C2_0_1_MP_ENERGY_MIN_C2_TABLE_1_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_0_1_MP_ENERGY_MIN_C2_TABLE_0_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C2_0_1_MP_ENERGY_MIN_C2_TABLE_0_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C2_2_3_OFFSET                             0x000000C8
#define MPDXX_APB_MP_ENERGY_MIN_C2_2_3_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C2_2_3_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_2_3_MP_ENERGY_MIN_C2_TABLE_3_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C2_2_3_MP_ENERGY_MIN_C2_TABLE_3_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_2_3_MP_ENERGY_MIN_C2_TABLE_2_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C2_2_3_MP_ENERGY_MIN_C2_TABLE_2_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C2_4_5_OFFSET                             0x000000CC
#define MPDXX_APB_MP_ENERGY_MIN_C2_4_5_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C2_4_5_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_4_5_MP_ENERGY_MIN_C2_TABLE_5_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C2_4_5_MP_ENERGY_MIN_C2_TABLE_5_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_4_5_MP_ENERGY_MIN_C2_TABLE_4_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C2_4_5_MP_ENERGY_MIN_C2_TABLE_4_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_MIN_C2_6_7_OFFSET                             0x000000D0
#define MPDXX_APB_MP_ENERGY_MIN_C2_6_7_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_MIN_C2_6_7_RESERVED_MASK                      0x00000000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_6_7_MP_ENERGY_MIN_C2_TABLE_7_SHIFT  16
#define    MPDXX_APB_MP_ENERGY_MIN_C2_6_7_MP_ENERGY_MIN_C2_TABLE_7_MASK   0xFFFF0000
#define    MPDXX_APB_MP_ENERGY_MIN_C2_6_7_MP_ENERGY_MIN_C2_TABLE_6_SHIFT  0
#define    MPDXX_APB_MP_ENERGY_MIN_C2_6_7_MP_ENERGY_MIN_C2_TABLE_6_MASK   0x0000FFFF

#define MPDXX_APB_MP_ENERGY_TABLE_ADDR_OFFSET                             0x000000D4
#define MPDXX_APB_MP_ENERGY_TABLE_ADDR_TYPE                               UInt32
#define MPDXX_APB_MP_ENERGY_TABLE_ADDR_RESERVED_MASK                      0xFFFFFF80
#define    MPDXX_APB_MP_ENERGY_TABLE_ADDR_MP_ENERGY_TABLE_INDX_SHIFT      0
#define    MPDXX_APB_MP_ENERGY_TABLE_ADDR_MP_ENERGY_TABLE_INDX_MASK       0x0000007F

#define MPDXX_APB_MP_ENERGY_TABLE_OFFSET                                  0x000000D8
#define MPDXX_APB_MP_ENERGY_TABLE_TYPE                                    UInt32
#define MPDXX_APB_MP_ENERGY_TABLE_RESERVED_MASK                           0x00000000
#define    MPDXX_APB_MP_ENERGY_TABLE_MP_ENERGY_TABLE_SHIFT                0
#define    MPDXX_APB_MP_ENERGY_TABLE_MP_ENERGY_TABLE_MASK                 0xFFFFFFFF

#define MPDXX_APB_CELL_16_EN_OFFSET                                       0x000000DC
#define MPDXX_APB_CELL_16_EN_TYPE                                         UInt32
#define MPDXX_APB_CELL_16_EN_RESERVED_MASK                                0x0000FFFE
#define    MPDXX_APB_CELL_16_EN_ANT2_EN_NEW_SHIFT                         16
#define    MPDXX_APB_CELL_16_EN_ANT2_EN_NEW_MASK                          0xFFFF0000
#define    MPDXX_APB_CELL_16_EN_CELL_16_EN_SHIFT                          0
#define    MPDXX_APB_CELL_16_EN_CELL_16_EN_MASK                           0x00000001

#define MPDXX_APB_PRIM_GC_INDX_ARRAYCD_OFFSET                             0x000000E0
#define MPDXX_APB_PRIM_GC_INDX_ARRAYCD_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAYCD_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYCD_PRIM_GC_INDX_ARRAY_13_SHIFT     16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYCD_PRIM_GC_INDX_ARRAY_13_MASK      0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYCD_PRIM_GC_INDX_ARRAY_12_SHIFT     0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYCD_PRIM_GC_INDX_ARRAY_12_MASK      0x000001FF

#define MPDXX_APB_PRIM_GC_INDX_ARRAYEF_OFFSET                             0x000000E4
#define MPDXX_APB_PRIM_GC_INDX_ARRAYEF_TYPE                               UInt32
#define MPDXX_APB_PRIM_GC_INDX_ARRAYEF_RESERVED_MASK                      0xFE00FE00
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYEF_PRIM_GC_INDX_ARRAY_15_SHIFT     16
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYEF_PRIM_GC_INDX_ARRAY_15_MASK      0x01FF0000
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYEF_PRIM_GC_INDX_ARRAY_14_SHIFT     0
#define    MPDXX_APB_PRIM_GC_INDX_ARRAYEF_PRIM_GC_INDX_ARRAY_14_MASK      0x000001FF

#endif /* __BRCM_RDB_MPDXX_APB_H__ */



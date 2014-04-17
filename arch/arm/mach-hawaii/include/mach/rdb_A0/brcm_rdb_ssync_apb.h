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

#ifndef __BRCM_RDB_SSYNC_APB_H__
#define __BRCM_RDB_SSYNC_APB_H__

#define SSYNC_APB_STARTPOS_OFFSET                                         0x00000000
#define SSYNC_APB_STARTPOS_TYPE                                           UInt32
#define SSYNC_APB_STARTPOS_RESERVED_MASK                                  0xC000C000
#define    SSYNC_APB_STARTPOS_STARTPOS1_RB_11111111111111_SHIFT           16
#define    SSYNC_APB_STARTPOS_STARTPOS1_RB_11111111111111_MASK            0x3FFF0000
#define    SSYNC_APB_STARTPOS_STARTPOS0_RB_11111111111111_SHIFT           0
#define    SSYNC_APB_STARTPOS_STARTPOS0_RB_11111111111111_MASK            0x00003FFF

#define SSYNC_APB_CTRL_OFFSET                                             0x00000004
#define SSYNC_APB_CTRL_TYPE                                               UInt32
#define SSYNC_APB_CTRL_RESERVED_MASK                                      0x7CF00000
#define    SSYNC_APB_CTRL_SW_RESET_SHIFT                                  31
#define    SSYNC_APB_CTRL_SW_RESET_MASK                                   0x80000000
#define    SSYNC_APB_CTRL_VPGAIN_SHIFT                                    24
#define    SSYNC_APB_CTRL_VPGAIN_MASK                                     0x03000000
#define    SSYNC_APB_CTRL_STAMP_CLEAR_SHIFT                               16
#define    SSYNC_APB_CTRL_STAMP_CLEAR_MASK                                0x000F0000
#define    SSYNC_APB_CTRL_SLOTSTAMPB1_RH_F_SHIFT                          12
#define    SSYNC_APB_CTRL_SLOTSTAMPB1_RH_F_MASK                           0x0000F000
#define    SSYNC_APB_CTRL_SLOTSTAMPA1_RH_F_SHIFT                          8
#define    SSYNC_APB_CTRL_SLOTSTAMPA1_RH_F_MASK                           0x00000F00
#define    SSYNC_APB_CTRL_SLOTSTAMPB0_RH_F_SHIFT                          4
#define    SSYNC_APB_CTRL_SLOTSTAMPB0_RH_F_MASK                           0x000000F0
#define    SSYNC_APB_CTRL_SLOTSTAMPA0_RH_F_SHIFT                          0
#define    SSYNC_APB_CTRL_SLOTSTAMPA0_RH_F_MASK                           0x0000000F

#define SSYNC_APB_RESULTBUF_A0_01_OFFSET                                  0x00000008
#define SSYNC_APB_RESULTBUF_A0_01_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_01_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_01_RESULTBUFA0_1_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A0_01_RESULTBUFA0_1_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_01_RESULTBUFA0_0_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A0_01_RESULTBUFA0_0_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_23_OFFSET                                  0x0000000C
#define SSYNC_APB_RESULTBUF_A0_23_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_23_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_23_RESULTBUFA0_3_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A0_23_RESULTBUFA0_3_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_23_RESULTBUFA0_2_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A0_23_RESULTBUFA0_2_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_45_OFFSET                                  0x00000010
#define SSYNC_APB_RESULTBUF_A0_45_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_45_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_45_RESULTBUFA0_5_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A0_45_RESULTBUFA0_5_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_45_RESULTBUFA0_4_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A0_45_RESULTBUFA0_4_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_67_OFFSET                                  0x00000014
#define SSYNC_APB_RESULTBUF_A0_67_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_67_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_67_RESULTBUFA0_7_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A0_67_RESULTBUFA0_7_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_67_RESULTBUFA0_6_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A0_67_RESULTBUFA0_6_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_89_OFFSET                                  0x00000018
#define SSYNC_APB_RESULTBUF_A0_89_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_89_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_89_RESULTBUFA0_9_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A0_89_RESULTBUFA0_9_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_89_RESULTBUFA0_8_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A0_89_RESULTBUFA0_8_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_AB_OFFSET                                  0x0000001C
#define SSYNC_APB_RESULTBUF_A0_AB_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_AB_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_AB_RESULTBUFA0_11_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_A0_AB_RESULTBUFA0_11_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_AB_RESULTBUFA0_10_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_A0_AB_RESULTBUFA0_10_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_CD_OFFSET                                  0x00000020
#define SSYNC_APB_RESULTBUF_A0_CD_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_CD_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_CD_RESULTBUFA0_13_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_A0_CD_RESULTBUFA0_13_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_CD_RESULTBUFA0_12_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_A0_CD_RESULTBUFA0_12_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_A0_EF_OFFSET                                  0x00000024
#define SSYNC_APB_RESULTBUF_A0_EF_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A0_EF_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A0_EF_RESULTBUFA0_15_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_A0_EF_RESULTBUFA0_15_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A0_EF_RESULTBUFA0_14_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_A0_EF_RESULTBUFA0_14_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_01_OFFSET                                  0x00000028
#define SSYNC_APB_RESULTBUF_B0_01_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_01_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_01_RESULTBUFB0_1_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B0_01_RESULTBUFB0_1_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_01_RESULTBUFB0_0_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B0_01_RESULTBUFB0_0_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_23_OFFSET                                  0x0000002C
#define SSYNC_APB_RESULTBUF_B0_23_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_23_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_23_RESULTBUFB0_3_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B0_23_RESULTBUFB0_3_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_23_RESULTBUFB0_2_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B0_23_RESULTBUFB0_2_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_45_OFFSET                                  0x00000030
#define SSYNC_APB_RESULTBUF_B0_45_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_45_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_45_RESULTBUFB0_5_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B0_45_RESULTBUFB0_5_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_45_RESULTBUFB0_4_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B0_45_RESULTBUFB0_4_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_67_OFFSET                                  0x00000034
#define SSYNC_APB_RESULTBUF_B0_67_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_67_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_67_RESULTBUFB0_7_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B0_67_RESULTBUFB0_7_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_67_RESULTBUFB0_6_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B0_67_RESULTBUFB0_6_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_89_OFFSET                                  0x00000038
#define SSYNC_APB_RESULTBUF_B0_89_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_89_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_89_RESULTBUFB0_9_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B0_89_RESULTBUFB0_9_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_89_RESULTBUFB0_8_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B0_89_RESULTBUFB0_8_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_AB_OFFSET                                  0x0000003C
#define SSYNC_APB_RESULTBUF_B0_AB_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_AB_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_AB_RESULTBUFB0_11_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_B0_AB_RESULTBUFB0_11_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_AB_RESULTBUFB0_10_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_B0_AB_RESULTBUFB0_10_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_CD_OFFSET                                  0x00000040
#define SSYNC_APB_RESULTBUF_B0_CD_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_CD_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_CD_RESULTBUFB0_13_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_B0_CD_RESULTBUFB0_13_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_CD_RESULTBUFB0_12_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_B0_CD_RESULTBUFB0_12_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_B0_EF_OFFSET                                  0x00000044
#define SSYNC_APB_RESULTBUF_B0_EF_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B0_EF_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B0_EF_RESULTBUFB0_15_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_B0_EF_RESULTBUFB0_15_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B0_EF_RESULTBUFB0_14_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_B0_EF_RESULTBUFB0_14_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_01_OFFSET                                  0x00000048
#define SSYNC_APB_RESULTBUF_A1_01_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_01_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_01_RESULTBUFA1_1_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A1_01_RESULTBUFA1_1_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_01_RESULTBUFA1_0_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A1_01_RESULTBUFA1_0_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_23_OFFSET                                  0x0000004C
#define SSYNC_APB_RESULTBUF_A1_23_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_23_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_23_RESULTBUFA1_3_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A1_23_RESULTBUFA1_3_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_23_RESULTBUFA1_2_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A1_23_RESULTBUFA1_2_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_45_OFFSET                                  0x00000050
#define SSYNC_APB_RESULTBUF_A1_45_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_45_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_45_RESULTBUFA1_5_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A1_45_RESULTBUFA1_5_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_45_RESULTBUFA1_4_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A1_45_RESULTBUFA1_4_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_67_OFFSET                                  0x00000054
#define SSYNC_APB_RESULTBUF_A1_67_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_67_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_67_RESULTBUFA1_7_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A1_67_RESULTBUFA1_7_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_67_RESULTBUFA1_6_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A1_67_RESULTBUFA1_6_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_89_OFFSET                                  0x00000058
#define SSYNC_APB_RESULTBUF_A1_89_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_89_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_89_RESULTBUFA1_9_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_A1_89_RESULTBUFA1_9_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_89_RESULTBUFA1_8_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_A1_89_RESULTBUFA1_8_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_AB_OFFSET                                  0x0000005C
#define SSYNC_APB_RESULTBUF_A1_AB_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_AB_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_AB_RESULTBUFA1_11_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_A1_AB_RESULTBUFA1_11_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_AB_RESULTBUFA1_10_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_A1_AB_RESULTBUFA1_10_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_CD_OFFSET                                  0x00000060
#define SSYNC_APB_RESULTBUF_A1_CD_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_CD_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_CD_RESULTBUFA1_13_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_A1_CD_RESULTBUFA1_13_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_CD_RESULTBUFA1_12_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_A1_CD_RESULTBUFA1_12_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_A1_EF_OFFSET                                  0x00000064
#define SSYNC_APB_RESULTBUF_A1_EF_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_A1_EF_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_A1_EF_RESULTBUFA1_15_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_A1_EF_RESULTBUFA1_15_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_A1_EF_RESULTBUFA1_14_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_A1_EF_RESULTBUFA1_14_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_01_OFFSET                                  0x00000068
#define SSYNC_APB_RESULTBUF_B1_01_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_01_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_01_RESULTBUFB1_1_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B1_01_RESULTBUFB1_1_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_01_RESULTBUFB1_0_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B1_01_RESULTBUFB1_0_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_23_OFFSET                                  0x0000006C
#define SSYNC_APB_RESULTBUF_B1_23_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_23_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_23_RESULTBUFB1_3_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B1_23_RESULTBUFB1_3_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_23_RESULTBUFB1_2_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B1_23_RESULTBUFB1_2_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_45_OFFSET                                  0x00000070
#define SSYNC_APB_RESULTBUF_B1_45_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_45_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_45_RESULTBUFB1_5_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B1_45_RESULTBUFB1_5_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_45_RESULTBUFB1_4_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B1_45_RESULTBUFB1_4_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_67_OFFSET                                  0x00000074
#define SSYNC_APB_RESULTBUF_B1_67_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_67_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_67_RESULTBUFB1_7_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B1_67_RESULTBUFB1_7_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_67_RESULTBUFB1_6_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B1_67_RESULTBUFB1_6_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_89_OFFSET                                  0x00000078
#define SSYNC_APB_RESULTBUF_B1_89_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_89_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_89_RESULTBUFB1_9_SHIFT                  16
#define    SSYNC_APB_RESULTBUF_B1_89_RESULTBUFB1_9_MASK                   0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_89_RESULTBUFB1_8_SHIFT                  0
#define    SSYNC_APB_RESULTBUF_B1_89_RESULTBUFB1_8_MASK                   0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_AB_OFFSET                                  0x0000007C
#define SSYNC_APB_RESULTBUF_B1_AB_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_AB_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_AB_RESULTBUFB1_11_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_B1_AB_RESULTBUFB1_11_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_AB_RESULTBUFB1_10_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_B1_AB_RESULTBUFB1_10_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_CD_OFFSET                                  0x00000080
#define SSYNC_APB_RESULTBUF_B1_CD_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_CD_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_CD_RESULTBUFB1_13_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_B1_CD_RESULTBUFB1_13_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_CD_RESULTBUFB1_12_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_B1_CD_RESULTBUFB1_12_MASK                  0x0000FFFF

#define SSYNC_APB_RESULTBUF_B1_EF_OFFSET                                  0x00000084
#define SSYNC_APB_RESULTBUF_B1_EF_TYPE                                    UInt32
#define SSYNC_APB_RESULTBUF_B1_EF_RESERVED_MASK                           0x00000000
#define    SSYNC_APB_RESULTBUF_B1_EF_RESULTBUFB1_15_SHIFT                 16
#define    SSYNC_APB_RESULTBUF_B1_EF_RESULTBUFB1_15_MASK                  0xFFFF0000
#define    SSYNC_APB_RESULTBUF_B1_EF_RESULTBUFB1_14_SHIFT                 0
#define    SSYNC_APB_RESULTBUF_B1_EF_RESULTBUFB1_14_MASK                  0x0000FFFF

#endif /* __BRCM_RDB_SSYNC_APB_H__ */



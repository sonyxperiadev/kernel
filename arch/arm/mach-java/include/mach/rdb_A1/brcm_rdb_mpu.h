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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_MPU_H__
#define __BRCM_RDB_MPU_H__

#define MPU_SRAM_MPU_CTRL0_OFFSET                                         0x00000000
#define MPU_SRAM_MPU_CTRL0_TYPE                                           UInt32
#define MPU_SRAM_MPU_CTRL0_RESERVED_MASK                                  0x00000000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_15_SHIFT                           30
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_15_MASK                            0xC0000000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_14_SHIFT                           28
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_14_MASK                            0x30000000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_13_SHIFT                           26
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_13_MASK                            0x0C000000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_12_SHIFT                           24
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_12_MASK                            0x03000000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_11_SHIFT                           22
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_11_MASK                            0x00C00000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_10_SHIFT                           20
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_10_MASK                            0x00300000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_9_SHIFT                            18
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_9_MASK                             0x000C0000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_8_SHIFT                            16
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_8_MASK                             0x00030000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_7_SHIFT                            14
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_7_MASK                             0x0000C000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_6_SHIFT                            12
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_6_MASK                             0x00003000
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_5_SHIFT                            10
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_5_MASK                             0x00000C00
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_4_SHIFT                            8
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_4_MASK                             0x00000300
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_3_SHIFT                            6
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_3_MASK                             0x000000C0
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_2_SHIFT                            4
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_2_MASK                             0x00000030
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_1_SHIFT                            2
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_1_MASK                             0x0000000C
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_0_SHIFT                            0
#define    MPU_SRAM_MPU_CTRL0_SEC_CTRL_0_MASK                             0x00000003

#define MPU_SRAM_MPU_CTRL1_OFFSET                                         0x00000004
#define MPU_SRAM_MPU_CTRL1_TYPE                                           UInt32
#define MPU_SRAM_MPU_CTRL1_RESERVED_MASK                                  0x00000000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_31_SHIFT                           30
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_31_MASK                            0xC0000000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_30_SHIFT                           28
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_30_MASK                            0x30000000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_29_SHIFT                           26
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_29_MASK                            0x0C000000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_28_SHIFT                           24
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_28_MASK                            0x03000000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_27_SHIFT                           22
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_27_MASK                            0x00C00000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_26_SHIFT                           20
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_26_MASK                            0x00300000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_25_SHIFT                           18
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_25_MASK                            0x000C0000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_24_SHIFT                           16
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_24_MASK                            0x00030000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_23_SHIFT                           14
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_23_MASK                            0x0000C000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_22_SHIFT                           12
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_22_MASK                            0x00003000
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_21_SHIFT                           10
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_21_MASK                            0x00000C00
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_20_SHIFT                           8
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_20_MASK                            0x00000300
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_19_SHIFT                           6
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_19_MASK                            0x000000C0
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_18_SHIFT                           4
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_18_MASK                            0x00000030
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_17_SHIFT                           2
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_17_MASK                            0x0000000C
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_16_SHIFT                           0
#define    MPU_SRAM_MPU_CTRL1_SEC_CTRL_16_MASK                            0x00000003

#define MPU_SRAM_MPU_CTRL2_OFFSET                                         0x00000008
#define MPU_SRAM_MPU_CTRL2_TYPE                                           UInt32
#define MPU_SRAM_MPU_CTRL2_RESERVED_MASK                                  0x00000000
#define    MPU_SRAM_MPU_CTRL2_RESERVED_MPU_CTRL2_SHIFT                    16
#define    MPU_SRAM_MPU_CTRL2_RESERVED_MPU_CTRL2_MASK                     0xFFFF0000
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_39_SHIFT                           14
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_39_MASK                            0x0000C000
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_38_SHIFT                           12
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_38_MASK                            0x00003000
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_37_SHIFT                           10
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_37_MASK                            0x00000C00
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_36_SHIFT                           8
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_36_MASK                            0x00000300
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_35_SHIFT                           6
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_35_MASK                            0x000000C0
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_34_SHIFT                           4
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_34_MASK                            0x00000030
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_33_SHIFT                           2
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_33_MASK                            0x0000000C
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_32_SHIFT                           0
#define    MPU_SRAM_MPU_CTRL2_SEC_CTRL_32_MASK                            0x00000003

#define MPU_SRAM_MPU_LOCK_0_OFFSET                                        0x0000000C
#define MPU_SRAM_MPU_LOCK_0_TYPE                                          UInt32
#define MPU_SRAM_MPU_LOCK_0_RESERVED_MASK                                 0x00000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_31_SHIFT                          31
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_31_MASK                           0x80000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_30_SHIFT                          30
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_30_MASK                           0x40000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_29_SHIFT                          29
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_29_MASK                           0x20000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_28_SHIFT                          28
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_28_MASK                           0x10000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_27_SHIFT                          27
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_27_MASK                           0x08000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_26_SHIFT                          26
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_26_MASK                           0x04000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_25_SHIFT                          25
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_25_MASK                           0x02000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_24_SHIFT                          24
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_24_MASK                           0x01000000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_23_SHIFT                          23
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_23_MASK                           0x00800000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_22_SHIFT                          22
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_22_MASK                           0x00400000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_21_SHIFT                          21
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_21_MASK                           0x00200000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_20_SHIFT                          20
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_20_MASK                           0x00100000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_19_SHIFT                          19
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_19_MASK                           0x00080000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_18_SHIFT                          18
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_18_MASK                           0x00040000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_17_SHIFT                          17
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_17_MASK                           0x00020000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_16_SHIFT                          16
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_16_MASK                           0x00010000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_15_SHIFT                          15
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_15_MASK                           0x00008000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_14_SHIFT                          14
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_14_MASK                           0x00004000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_13_SHIFT                          13
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_13_MASK                           0x00002000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_12_SHIFT                          12
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_12_MASK                           0x00001000
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_11_SHIFT                          11
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_11_MASK                           0x00000800
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_10_SHIFT                          10
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_10_MASK                           0x00000400
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_9_SHIFT                           9
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_9_MASK                            0x00000200
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_8_SHIFT                           8
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_8_MASK                            0x00000100
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_7_SHIFT                           7
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_7_MASK                            0x00000080
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_6_SHIFT                           6
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_6_MASK                            0x00000040
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_5_SHIFT                           5
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_5_MASK                            0x00000020
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_4_SHIFT                           4
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_4_MASK                            0x00000010
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_3_SHIFT                           3
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_3_MASK                            0x00000008
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_2_SHIFT                           2
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_2_MASK                            0x00000004
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_1_SHIFT                           1
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_1_MASK                            0x00000002
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_0_SHIFT                           0
#define    MPU_SRAM_MPU_LOCK_0_LOCK_RGN_0_MASK                            0x00000001

#define MPU_SRAM_MPU_LOCK_1_OFFSET                                        0x00000010
#define MPU_SRAM_MPU_LOCK_1_TYPE                                          UInt32
#define MPU_SRAM_MPU_LOCK_1_RESERVED_MASK                                 0xFFFFFF00
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_39_SHIFT                          7
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_39_MASK                           0x00000080
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_38_SHIFT                          6
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_38_MASK                           0x00000040
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_37_SHIFT                          5
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_37_MASK                           0x00000020
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_36_SHIFT                          4
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_36_MASK                           0x00000010
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_35_SHIFT                          3
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_35_MASK                           0x00000008
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_34_SHIFT                          2
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_34_MASK                           0x00000004
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_33_SHIFT                          1
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_33_MASK                           0x00000002
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_32_SHIFT                          0
#define    MPU_SRAM_MPU_LOCK_1_LOCK_RGN_32_MASK                           0x00000001

#endif /* __BRCM_RDB_MPU_H__ */



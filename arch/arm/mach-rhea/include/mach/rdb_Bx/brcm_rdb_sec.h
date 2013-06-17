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

#ifndef __BRCM_RDB_SEC_H__
#define __BRCM_RDB_SEC_H__

#define SEC_GLB_CTL_OFFSET                                                0x00000000
#define SEC_GLB_CTL_TYPE                                                  UInt32
#define SEC_GLB_CTL_RESERVED_MASK                                         0xFFFFFFCC
#define    SEC_GLB_CTL_GLB_GID_CHECK_EN_LOCK_SHIFT                        5
#define    SEC_GLB_CTL_GLB_GID_CHECK_EN_LOCK_MASK                         0x00000020
#define    SEC_GLB_CTL_GLB_GID_CHECK_EN_SHIFT                             4
#define    SEC_GLB_CTL_GLB_GID_CHECK_EN_MASK                              0x00000010
#define    SEC_GLB_CTL_GLB_ADDR_SCR_EN_SHIFT                              1
#define    SEC_GLB_CTL_GLB_ADDR_SCR_EN_MASK                               0x00000002
#define    SEC_GLB_CTL_GLB_DATA_SCR_EN_SHIFT                              0
#define    SEC_GLB_CTL_GLB_DATA_SCR_EN_MASK                               0x00000001

#define SEC_RGN_0_CTL_OFFSET                                              0x00000010
#define SEC_RGN_0_CTL_TYPE                                                UInt32
#define SEC_RGN_0_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_0_CTL_RGN_0_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_0_CTL_RGN_0_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_0_CTL_RGN_0_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_0_CTL_RGN_0_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_0_CTL_RGN_0_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_0_CTL_RGN_0_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_0_CTL_RGN_0_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_0_CTL_RGN_0_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_0_CTL_RGN_0_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_0_CTL_RGN_0_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_0_CTL_RGN_0_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_0_CTL_RGN_0_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_0_CTL_RGN_0_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_0_CTL_RGN_0_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_0_CTL_RGN_0_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_0_CTL_RGN_0_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_0_CTL_RGN_0_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_0_CTL_RGN_0_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_0_CTL_RGN_0_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_0_CTL_RGN_0_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_0_CTL_RGN_0_EN_SHIFT                                   0
#define    SEC_RGN_0_CTL_RGN_0_EN_MASK                                    0x00000001

#define SEC_RGN_0_START_ADDR_OFFSET                                       0x00000014
#define SEC_RGN_0_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_0_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_0_START_ADDR_RGN_0_START_ADDR_SHIFT                    12
#define    SEC_RGN_0_START_ADDR_RGN_0_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_0_START_ADDR_RGN_0_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_0_START_ADDR_RGN_0_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_0_END_ADDR_OFFSET                                         0x00000018
#define SEC_RGN_0_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_0_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_0_END_ADDR_RGN_0_END_ADDR_SHIFT                        12
#define    SEC_RGN_0_END_ADDR_RGN_0_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_0_END_ADDR_RGN_0_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_0_END_ADDR_RGN_0_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_1_CTL_OFFSET                                              0x00000020
#define SEC_RGN_1_CTL_TYPE                                                UInt32
#define SEC_RGN_1_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_1_CTL_RGN_1_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_1_CTL_RGN_1_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_1_CTL_RGN_1_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_1_CTL_RGN_1_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_1_CTL_RGN_1_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_1_CTL_RGN_1_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_1_CTL_RGN_1_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_1_CTL_RGN_1_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_1_CTL_RGN_1_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_1_CTL_RGN_1_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_1_CTL_RGN_1_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_1_CTL_RGN_1_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_1_CTL_RGN_1_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_1_CTL_RGN_1_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_1_CTL_RGN_1_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_1_CTL_RGN_1_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_1_CTL_RGN_1_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_1_CTL_RGN_1_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_1_CTL_RGN_1_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_1_CTL_RGN_1_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_1_CTL_RGN_1_EN_SHIFT                                   0
#define    SEC_RGN_1_CTL_RGN_1_EN_MASK                                    0x00000001

#define SEC_RGN_1_START_ADDR_OFFSET                                       0x00000024
#define SEC_RGN_1_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_1_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_1_START_ADDR_RGN_1_START_ADDR_SHIFT                    12
#define    SEC_RGN_1_START_ADDR_RGN_1_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_1_START_ADDR_RGN_1_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_1_START_ADDR_RGN_1_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_1_END_ADDR_OFFSET                                         0x00000028
#define SEC_RGN_1_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_1_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_1_END_ADDR_RGN_1_END_ADDR_SHIFT                        12
#define    SEC_RGN_1_END_ADDR_RGN_1_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_1_END_ADDR_RGN_1_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_1_END_ADDR_RGN_1_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_2_CTL_OFFSET                                              0x00000030
#define SEC_RGN_2_CTL_TYPE                                                UInt32
#define SEC_RGN_2_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_2_CTL_RGN_2_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_2_CTL_RGN_2_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_2_CTL_RGN_2_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_2_CTL_RGN_2_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_2_CTL_RGN_2_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_2_CTL_RGN_2_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_2_CTL_RGN_2_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_2_CTL_RGN_2_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_2_CTL_RGN_2_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_2_CTL_RGN_2_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_2_CTL_RGN_2_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_2_CTL_RGN_2_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_2_CTL_RGN_2_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_2_CTL_RGN_2_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_2_CTL_RGN_2_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_2_CTL_RGN_2_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_2_CTL_RGN_2_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_2_CTL_RGN_2_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_2_CTL_RGN_2_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_2_CTL_RGN_2_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_2_CTL_RGN_2_EN_SHIFT                                   0
#define    SEC_RGN_2_CTL_RGN_2_EN_MASK                                    0x00000001

#define SEC_RGN_2_START_ADDR_OFFSET                                       0x00000034
#define SEC_RGN_2_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_2_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_2_START_ADDR_RGN_2_START_ADDR_SHIFT                    12
#define    SEC_RGN_2_START_ADDR_RGN_2_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_2_START_ADDR_RGN_2_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_2_START_ADDR_RGN_2_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_2_END_ADDR_OFFSET                                         0x00000038
#define SEC_RGN_2_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_2_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_2_END_ADDR_RGN_2_END_ADDR_SHIFT                        12
#define    SEC_RGN_2_END_ADDR_RGN_2_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_2_END_ADDR_RGN_2_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_2_END_ADDR_RGN_2_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_3_CTL_OFFSET                                              0x00000040
#define SEC_RGN_3_CTL_TYPE                                                UInt32
#define SEC_RGN_3_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_3_CTL_RGN_3_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_3_CTL_RGN_3_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_3_CTL_RGN_3_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_3_CTL_RGN_3_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_3_CTL_RGN_3_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_3_CTL_RGN_3_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_3_CTL_RGN_3_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_3_CTL_RGN_3_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_3_CTL_RGN_3_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_3_CTL_RGN_3_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_3_CTL_RGN_3_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_3_CTL_RGN_3_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_3_CTL_RGN_3_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_3_CTL_RGN_3_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_3_CTL_RGN_3_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_3_CTL_RGN_3_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_3_CTL_RGN_3_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_3_CTL_RGN_3_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_3_CTL_RGN_3_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_3_CTL_RGN_3_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_3_CTL_RGN_3_EN_SHIFT                                   0
#define    SEC_RGN_3_CTL_RGN_3_EN_MASK                                    0x00000001

#define SEC_RGN_3_START_ADDR_OFFSET                                       0x00000044
#define SEC_RGN_3_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_3_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_3_START_ADDR_RGN_3_START_ADDR_SHIFT                    12
#define    SEC_RGN_3_START_ADDR_RGN_3_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_3_START_ADDR_RGN_3_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_3_START_ADDR_RGN_3_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_3_END_ADDR_OFFSET                                         0x00000048
#define SEC_RGN_3_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_3_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_3_END_ADDR_RGN_3_END_ADDR_SHIFT                        12
#define    SEC_RGN_3_END_ADDR_RGN_3_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_3_END_ADDR_RGN_3_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_3_END_ADDR_RGN_3_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_4_CTL_OFFSET                                              0x00000050
#define SEC_RGN_4_CTL_TYPE                                                UInt32
#define SEC_RGN_4_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_4_CTL_RGN_4_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_4_CTL_RGN_4_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_4_CTL_RGN_4_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_4_CTL_RGN_4_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_4_CTL_RGN_4_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_4_CTL_RGN_4_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_4_CTL_RGN_4_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_4_CTL_RGN_4_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_4_CTL_RGN_4_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_4_CTL_RGN_4_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_4_CTL_RGN_4_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_4_CTL_RGN_4_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_4_CTL_RGN_4_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_4_CTL_RGN_4_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_4_CTL_RGN_4_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_4_CTL_RGN_4_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_4_CTL_RGN_4_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_4_CTL_RGN_4_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_4_CTL_RGN_4_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_4_CTL_RGN_4_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_4_CTL_RGN_4_EN_SHIFT                                   0
#define    SEC_RGN_4_CTL_RGN_4_EN_MASK                                    0x00000001

#define SEC_RGN_4_START_ADDR_OFFSET                                       0x00000054
#define SEC_RGN_4_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_4_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_4_START_ADDR_RGN_4_START_ADDR_SHIFT                    12
#define    SEC_RGN_4_START_ADDR_RGN_4_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_4_START_ADDR_RGN_4_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_4_START_ADDR_RGN_4_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_4_END_ADDR_OFFSET                                         0x00000058
#define SEC_RGN_4_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_4_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_4_END_ADDR_RGN_4_END_ADDR_SHIFT                        12
#define    SEC_RGN_4_END_ADDR_RGN_4_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_4_END_ADDR_RGN_4_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_4_END_ADDR_RGN_4_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_5_CTL_OFFSET                                              0x00000060
#define SEC_RGN_5_CTL_TYPE                                                UInt32
#define SEC_RGN_5_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_5_CTL_RGN_5_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_5_CTL_RGN_5_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_5_CTL_RGN_5_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_5_CTL_RGN_5_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_5_CTL_RGN_5_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_5_CTL_RGN_5_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_5_CTL_RGN_5_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_5_CTL_RGN_5_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_5_CTL_RGN_5_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_5_CTL_RGN_5_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_5_CTL_RGN_5_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_5_CTL_RGN_5_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_5_CTL_RGN_5_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_5_CTL_RGN_5_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_5_CTL_RGN_5_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_5_CTL_RGN_5_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_5_CTL_RGN_5_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_5_CTL_RGN_5_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_5_CTL_RGN_5_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_5_CTL_RGN_5_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_5_CTL_RGN_5_EN_SHIFT                                   0
#define    SEC_RGN_5_CTL_RGN_5_EN_MASK                                    0x00000001

#define SEC_RGN_5_START_ADDR_OFFSET                                       0x00000064
#define SEC_RGN_5_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_5_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_5_START_ADDR_RGN_5_START_ADDR_SHIFT                    12
#define    SEC_RGN_5_START_ADDR_RGN_5_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_5_START_ADDR_RGN_5_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_5_START_ADDR_RGN_5_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_5_END_ADDR_OFFSET                                         0x00000068
#define SEC_RGN_5_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_5_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_5_END_ADDR_RGN_5_END_ADDR_SHIFT                        12
#define    SEC_RGN_5_END_ADDR_RGN_5_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_5_END_ADDR_RGN_5_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_5_END_ADDR_RGN_5_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_6_CTL_OFFSET                                              0x00000070
#define SEC_RGN_6_CTL_TYPE                                                UInt32
#define SEC_RGN_6_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_6_CTL_RGN_6_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_6_CTL_RGN_6_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_6_CTL_RGN_6_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_6_CTL_RGN_6_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_6_CTL_RGN_6_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_6_CTL_RGN_6_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_6_CTL_RGN_6_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_6_CTL_RGN_6_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_6_CTL_RGN_6_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_6_CTL_RGN_6_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_6_CTL_RGN_6_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_6_CTL_RGN_6_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_6_CTL_RGN_6_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_6_CTL_RGN_6_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_6_CTL_RGN_6_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_6_CTL_RGN_6_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_6_CTL_RGN_6_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_6_CTL_RGN_6_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_6_CTL_RGN_6_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_6_CTL_RGN_6_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_6_CTL_RGN_6_EN_SHIFT                                   0
#define    SEC_RGN_6_CTL_RGN_6_EN_MASK                                    0x00000001

#define SEC_RGN_6_START_ADDR_OFFSET                                       0x00000074
#define SEC_RGN_6_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_6_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_6_START_ADDR_RGN_6_START_ADDR_SHIFT                    12
#define    SEC_RGN_6_START_ADDR_RGN_6_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_6_START_ADDR_RGN_6_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_6_START_ADDR_RGN_6_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_6_END_ADDR_OFFSET                                         0x00000078
#define SEC_RGN_6_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_6_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_6_END_ADDR_RGN_6_END_ADDR_SHIFT                        12
#define    SEC_RGN_6_END_ADDR_RGN_6_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_6_END_ADDR_RGN_6_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_6_END_ADDR_RGN_6_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_RGN_7_CTL_OFFSET                                              0x00000080
#define SEC_RGN_7_CTL_TYPE                                                UInt32
#define SEC_RGN_7_CTL_RESERVED_MASK                                       0xFE00F0C0
#define    SEC_RGN_7_CTL_RGN_7_GID_SANDBOX_EN_LOCK_SHIFT                  24
#define    SEC_RGN_7_CTL_RGN_7_GID_SANDBOX_EN_LOCK_MASK                   0x01000000
#define    SEC_RGN_7_CTL_RGN_7_GID_SANDBOX_EN_SHIFT                       16
#define    SEC_RGN_7_CTL_RGN_7_GID_SANDBOX_EN_MASK                        0x00FF0000
#define    SEC_RGN_7_CTL_RGN_7_BYPASS_SCR_LOCK_SHIFT                      11
#define    SEC_RGN_7_CTL_RGN_7_BYPASS_SCR_LOCK_MASK                       0x00000800
#define    SEC_RGN_7_CTL_RGN_7_BYPASS_SCR_SHIFT                           10
#define    SEC_RGN_7_CTL_RGN_7_BYPASS_SCR_MASK                            0x00000400
#define    SEC_RGN_7_CTL_RGN_7_ADDR_SCR_EN_SHIFT                          9
#define    SEC_RGN_7_CTL_RGN_7_ADDR_SCR_EN_MASK                           0x00000200
#define    SEC_RGN_7_CTL_RGN_7_DATA_SCR_EN_SHIFT                          8
#define    SEC_RGN_7_CTL_RGN_7_DATA_SCR_EN_MASK                           0x00000100
#define    SEC_RGN_7_CTL_RGN_7_END_ADDR_RGN_LOCK_SHIFT                    5
#define    SEC_RGN_7_CTL_RGN_7_END_ADDR_RGN_LOCK_MASK                     0x00000020
#define    SEC_RGN_7_CTL_RGN_7_START_ADDR_RGN_LOCK_SHIFT                  4
#define    SEC_RGN_7_CTL_RGN_7_START_ADDR_RGN_LOCK_MASK                   0x00000010
#define    SEC_RGN_7_CTL_RGN_7_MEM_TYPE_LOCK_SHIFT                        3
#define    SEC_RGN_7_CTL_RGN_7_MEM_TYPE_LOCK_MASK                         0x00000008
#define    SEC_RGN_7_CTL_RGN_7_MEM_TYPE_SHIFT                             1
#define    SEC_RGN_7_CTL_RGN_7_MEM_TYPE_MASK                              0x00000006
#define    SEC_RGN_7_CTL_RGN_7_EN_SHIFT                                   0
#define    SEC_RGN_7_CTL_RGN_7_EN_MASK                                    0x00000001

#define SEC_RGN_7_START_ADDR_OFFSET                                       0x00000084
#define SEC_RGN_7_START_ADDR_TYPE                                         UInt32
#define SEC_RGN_7_START_ADDR_RESERVED_MASK                                0x00000000
#define    SEC_RGN_7_START_ADDR_RGN_7_START_ADDR_SHIFT                    12
#define    SEC_RGN_7_START_ADDR_RGN_7_START_ADDR_MASK                     0xFFFFF000
#define    SEC_RGN_7_START_ADDR_RGN_7_START_ADDR_LSB_BITS_SHIFT           0
#define    SEC_RGN_7_START_ADDR_RGN_7_START_ADDR_LSB_BITS_MASK            0x00000FFF

#define SEC_RGN_7_END_ADDR_OFFSET                                         0x00000088
#define SEC_RGN_7_END_ADDR_TYPE                                           UInt32
#define SEC_RGN_7_END_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_RGN_7_END_ADDR_RGN_7_END_ADDR_SHIFT                        12
#define    SEC_RGN_7_END_ADDR_RGN_7_END_ADDR_MASK                         0xFFFFF000
#define    SEC_RGN_7_END_ADDR_RGN_7_END_ADDR_LSB_BITS_SHIFT               0
#define    SEC_RGN_7_END_ADDR_RGN_7_END_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_ACC_VIO_ADDR_OFFSET                                           0x00000090
#define SEC_ACC_VIO_ADDR_TYPE                                             UInt32
#define SEC_ACC_VIO_ADDR_RESERVED_MASK                                    0x00000000
#define    SEC_ACC_VIO_ADDR_ACC_VIO_ADDR_SHIFT                            0
#define    SEC_ACC_VIO_ADDR_ACC_VIO_ADDR_MASK                             0xFFFFFFFF

#define SEC_ACC_VIO_INFO_OFFSET                                           0x00000094
#define SEC_ACC_VIO_INFO_TYPE                                             UInt32
#define SEC_ACC_VIO_INFO_RESERVED_MASK                                    0x00000000
#define    SEC_ACC_VIO_INFO_ACC_VIO_INTR_OVERRUN_SHIFT                    31
#define    SEC_ACC_VIO_INFO_ACC_VIO_INTR_OVERRUN_MASK                     0x80000000
#define    SEC_ACC_VIO_INFO_ACC_VIO_INTR_SHIFT                            30
#define    SEC_ACC_VIO_INFO_ACC_VIO_INTR_MASK                             0x40000000
#define    SEC_ACC_VIO_INFO_ACC_VIO_REASON_SHIFT                          27
#define    SEC_ACC_VIO_INFO_ACC_VIO_REASON_MASK                           0x38000000
#define    SEC_ACC_VIO_INFO_ACC_VIO_PORT_ID_SHIFT                         25
#define    SEC_ACC_VIO_INFO_ACC_VIO_PORT_ID_MASK                          0x06000000
#define    SEC_ACC_VIO_INFO_ACC_VIO_REGION_SHIFT                          16
#define    SEC_ACC_VIO_INFO_ACC_VIO_REGION_MASK                           0x01FF0000
#define    SEC_ACC_VIO_INFO_ACC_VIO_CLIENT_ID_SHIFT                       1
#define    SEC_ACC_VIO_INFO_ACC_VIO_CLIENT_ID_MASK                        0x0000FFFE
#define    SEC_ACC_VIO_INFO_ACC_VIO_TYPE_SHIFT                            0
#define    SEC_ACC_VIO_INFO_ACC_VIO_TYPE_MASK                             0x00000001

#define SEC_DATA_KEY_WK_IN_OFFSET                                         0x000000A0
#define SEC_DATA_KEY_WK_IN_TYPE                                           UInt32
#define SEC_DATA_KEY_WK_IN_RESERVED_MASK                                  0x00000000
#define    SEC_DATA_KEY_WK_IN_DATA_KEY_WK_IN_SHIFT                        0
#define    SEC_DATA_KEY_WK_IN_DATA_KEY_WK_IN_MASK                         0xFFFFFFFF

#define SEC_DATA_KEY_WK_OUT_OFFSET                                        0x000000A4
#define SEC_DATA_KEY_WK_OUT_TYPE                                          UInt32
#define SEC_DATA_KEY_WK_OUT_RESERVED_MASK                                 0x00000000
#define    SEC_DATA_KEY_WK_OUT_DATA_KEY_WK_OUT_SHIFT                      0
#define    SEC_DATA_KEY_WK_OUT_DATA_KEY_WK_OUT_MASK                       0xFFFFFFFF

#define SEC_RND_0_DATA_KEY_OFFSET                                         0x000000A8
#define SEC_RND_0_DATA_KEY_TYPE                                           UInt32
#define SEC_RND_0_DATA_KEY_RESERVED_MASK                                  0x00000000
#define    SEC_RND_0_DATA_KEY_RND_0_DATA_KEY_SHIFT                        0
#define    SEC_RND_0_DATA_KEY_RND_0_DATA_KEY_MASK                         0xFFFFFFFF

#define SEC_RND_1_DATA_KEY_OFFSET                                         0x000000AC
#define SEC_RND_1_DATA_KEY_TYPE                                           UInt32
#define SEC_RND_1_DATA_KEY_RESERVED_MASK                                  0x00000000
#define    SEC_RND_1_DATA_KEY_RND_1_DATA_KEY_SHIFT                        0
#define    SEC_RND_1_DATA_KEY_RND_1_DATA_KEY_MASK                         0xFFFFFFFF

#define SEC_RND_2_DATA_KEY_OFFSET                                         0x000000B0
#define SEC_RND_2_DATA_KEY_TYPE                                           UInt32
#define SEC_RND_2_DATA_KEY_RESERVED_MASK                                  0x00000000
#define    SEC_RND_2_DATA_KEY_RND_2_DATA_KEY_SHIFT                        0
#define    SEC_RND_2_DATA_KEY_RND_2_DATA_KEY_MASK                         0xFFFFFFFF

#define SEC_ADDR_SCR_MASK_OFFSET                                          0x000000C0
#define SEC_ADDR_SCR_MASK_TYPE                                            UInt32
#define SEC_ADDR_SCR_MASK_RESERVED_MASK                                   0xFFFFFF00
#define    SEC_ADDR_SCR_MASK_ADDR_SCR_MASK_SHIFT                          0
#define    SEC_ADDR_SCR_MASK_ADDR_SCR_MASK_MASK                           0x000000FF

#define SEC_SW_ADDR_SCR_SCH_SEL_OFFSET                                    0x000000C8
#define SEC_SW_ADDR_SCR_SCH_SEL_TYPE                                      UInt32
#define SEC_SW_ADDR_SCR_SCH_SEL_RESERVED_MASK                             0xFF000000
#define    SEC_SW_ADDR_SCR_SCH_SEL_SW_ADDR_SCR_SCH_SEL_SHIFT              0
#define    SEC_SW_ADDR_SCR_SCH_SEL_SW_ADDR_SCR_SCH_SEL_MASK               0x00FFFFFF

#define SEC_ADDR_KEY_WK_IN_OFFSET                                         0x000000D0
#define SEC_ADDR_KEY_WK_IN_TYPE                                           UInt32
#define SEC_ADDR_KEY_WK_IN_RESERVED_MASK                                  0x00000000
#define    SEC_ADDR_KEY_WK_IN_ADDR_KEY_WK_IN_SHIFT                        0
#define    SEC_ADDR_KEY_WK_IN_ADDR_KEY_WK_IN_MASK                         0xFFFFFFFF

#define SEC_ADDR_KEY_WK_OUT_OFFSET                                        0x000000D4
#define SEC_ADDR_KEY_WK_OUT_TYPE                                          UInt32
#define SEC_ADDR_KEY_WK_OUT_RESERVED_MASK                                 0x00000000
#define    SEC_ADDR_KEY_WK_OUT_ADDR_KEY_WK_OUT_SHIFT                      0
#define    SEC_ADDR_KEY_WK_OUT_ADDR_KEY_WK_OUT_MASK                       0xFFFFFFFF

#define SEC_RND_0_ADDR_KEY_OFFSET                                         0x000000D8
#define SEC_RND_0_ADDR_KEY_TYPE                                           UInt32
#define SEC_RND_0_ADDR_KEY_RESERVED_MASK                                  0x00000000
#define    SEC_RND_0_ADDR_KEY_RND_0_ADDR_KEY_SHIFT                        0
#define    SEC_RND_0_ADDR_KEY_RND_0_ADDR_KEY_MASK                         0xFFFFFFFF

#define SEC_RND_1_ADDR_KEY_OFFSET                                         0x000000DC
#define SEC_RND_1_ADDR_KEY_TYPE                                           UInt32
#define SEC_RND_1_ADDR_KEY_RESERVED_MASK                                  0x00000000
#define    SEC_RND_1_ADDR_KEY_RND_1_ADDR_KEY_SHIFT                        0
#define    SEC_RND_1_ADDR_KEY_RND_1_ADDR_KEY_MASK                         0xFFFFFFFF

#define SEC_RND_2_ADDR_KEY_OFFSET                                         0x000000E0
#define SEC_RND_2_ADDR_KEY_TYPE                                           UInt32
#define SEC_RND_2_ADDR_KEY_RESERVED_MASK                                  0x00000000
#define    SEC_RND_2_ADDR_KEY_RND_2_ADDR_KEY_SHIFT                        0
#define    SEC_RND_2_ADDR_KEY_RND_2_ADDR_KEY_MASK                         0xFFFFFFFF

#define SEC_PORT_0_GID_0_CMP_OFFSET                                       0x00000100
#define SEC_PORT_0_GID_0_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_0_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_0_CMP_PORT_0_GID_0_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_0_CMP_PORT_0_GID_0_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_0_CMP_PORT_0_GID_0_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_0_CMP_PORT_0_GID_0_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_1_CMP_OFFSET                                       0x00000104
#define SEC_PORT_0_GID_1_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_1_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_1_CMP_PORT_0_GID_1_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_1_CMP_PORT_0_GID_1_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_1_CMP_PORT_0_GID_1_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_1_CMP_PORT_0_GID_1_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_2_CMP_OFFSET                                       0x00000108
#define SEC_PORT_0_GID_2_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_2_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_2_CMP_PORT_0_GID_2_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_2_CMP_PORT_0_GID_2_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_2_CMP_PORT_0_GID_2_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_2_CMP_PORT_0_GID_2_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_3_CMP_OFFSET                                       0x0000010C
#define SEC_PORT_0_GID_3_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_3_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_3_CMP_PORT_0_GID_3_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_3_CMP_PORT_0_GID_3_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_3_CMP_PORT_0_GID_3_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_3_CMP_PORT_0_GID_3_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_4_CMP_OFFSET                                       0x00000110
#define SEC_PORT_0_GID_4_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_4_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_4_CMP_PORT_0_GID_4_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_4_CMP_PORT_0_GID_4_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_4_CMP_PORT_0_GID_4_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_4_CMP_PORT_0_GID_4_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_5_CMP_OFFSET                                       0x00000114
#define SEC_PORT_0_GID_5_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_5_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_5_CMP_PORT_0_GID_5_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_5_CMP_PORT_0_GID_5_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_5_CMP_PORT_0_GID_5_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_5_CMP_PORT_0_GID_5_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_6_CMP_OFFSET                                       0x00000118
#define SEC_PORT_0_GID_6_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_6_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_6_CMP_PORT_0_GID_6_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_6_CMP_PORT_0_GID_6_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_6_CMP_PORT_0_GID_6_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_6_CMP_PORT_0_GID_6_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_0_GID_7_CMP_OFFSET                                       0x0000011C
#define SEC_PORT_0_GID_7_CMP_TYPE                                         UInt32
#define SEC_PORT_0_GID_7_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_0_GID_7_CMP_PORT_0_GID_7_CMP_BITS_SHIFT               16
#define    SEC_PORT_0_GID_7_CMP_PORT_0_GID_7_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_0_GID_7_CMP_PORT_0_GID_7_CMP_VALUE_SHIFT              0
#define    SEC_PORT_0_GID_7_CMP_PORT_0_GID_7_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_0_CMP_OFFSET                                       0x00000120
#define SEC_PORT_1_GID_0_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_0_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_0_CMP_PORT_1_GID_0_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_0_CMP_PORT_1_GID_0_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_0_CMP_PORT_1_GID_0_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_0_CMP_PORT_1_GID_0_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_1_CMP_OFFSET                                       0x00000124
#define SEC_PORT_1_GID_1_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_1_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_1_CMP_PORT_1_GID_1_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_1_CMP_PORT_1_GID_1_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_1_CMP_PORT_1_GID_1_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_1_CMP_PORT_1_GID_1_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_2_CMP_OFFSET                                       0x00000128
#define SEC_PORT_1_GID_2_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_2_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_2_CMP_PORT_1_GID_2_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_2_CMP_PORT_1_GID_2_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_2_CMP_PORT_1_GID_2_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_2_CMP_PORT_1_GID_2_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_3_CMP_OFFSET                                       0x0000012C
#define SEC_PORT_1_GID_3_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_3_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_3_CMP_PORT_1_GID_3_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_3_CMP_PORT_1_GID_3_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_3_CMP_PORT_1_GID_3_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_3_CMP_PORT_1_GID_3_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_4_CMP_OFFSET                                       0x00000130
#define SEC_PORT_1_GID_4_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_4_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_4_CMP_PORT_1_GID_4_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_4_CMP_PORT_1_GID_4_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_4_CMP_PORT_1_GID_4_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_4_CMP_PORT_1_GID_4_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_5_CMP_OFFSET                                       0x00000134
#define SEC_PORT_1_GID_5_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_5_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_5_CMP_PORT_1_GID_5_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_5_CMP_PORT_1_GID_5_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_5_CMP_PORT_1_GID_5_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_5_CMP_PORT_1_GID_5_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_6_CMP_OFFSET                                       0x00000138
#define SEC_PORT_1_GID_6_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_6_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_6_CMP_PORT_1_GID_6_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_6_CMP_PORT_1_GID_6_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_6_CMP_PORT_1_GID_6_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_6_CMP_PORT_1_GID_6_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_1_GID_7_CMP_OFFSET                                       0x0000013C
#define SEC_PORT_1_GID_7_CMP_TYPE                                         UInt32
#define SEC_PORT_1_GID_7_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_1_GID_7_CMP_PORT_1_GID_7_CMP_BITS_SHIFT               16
#define    SEC_PORT_1_GID_7_CMP_PORT_1_GID_7_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_1_GID_7_CMP_PORT_1_GID_7_CMP_VALUE_SHIFT              0
#define    SEC_PORT_1_GID_7_CMP_PORT_1_GID_7_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_0_CMP_OFFSET                                       0x00000140
#define SEC_PORT_2_GID_0_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_0_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_0_CMP_PORT_2_GID_0_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_0_CMP_PORT_2_GID_0_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_0_CMP_PORT_2_GID_0_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_0_CMP_PORT_2_GID_0_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_1_CMP_OFFSET                                       0x00000144
#define SEC_PORT_2_GID_1_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_1_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_1_CMP_PORT_2_GID_1_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_1_CMP_PORT_2_GID_1_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_1_CMP_PORT_2_GID_1_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_1_CMP_PORT_2_GID_1_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_2_CMP_OFFSET                                       0x00000148
#define SEC_PORT_2_GID_2_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_2_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_2_CMP_PORT_2_GID_2_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_2_CMP_PORT_2_GID_2_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_2_CMP_PORT_2_GID_2_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_2_CMP_PORT_2_GID_2_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_3_CMP_OFFSET                                       0x0000014C
#define SEC_PORT_2_GID_3_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_3_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_3_CMP_PORT_2_GID_3_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_3_CMP_PORT_2_GID_3_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_3_CMP_PORT_2_GID_3_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_3_CMP_PORT_2_GID_3_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_4_CMP_OFFSET                                       0x00000150
#define SEC_PORT_2_GID_4_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_4_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_4_CMP_PORT_2_GID_4_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_4_CMP_PORT_2_GID_4_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_4_CMP_PORT_2_GID_4_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_4_CMP_PORT_2_GID_4_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_5_CMP_OFFSET                                       0x00000154
#define SEC_PORT_2_GID_5_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_5_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_5_CMP_PORT_2_GID_5_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_5_CMP_PORT_2_GID_5_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_5_CMP_PORT_2_GID_5_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_5_CMP_PORT_2_GID_5_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_6_CMP_OFFSET                                       0x00000158
#define SEC_PORT_2_GID_6_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_6_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_6_CMP_PORT_2_GID_6_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_6_CMP_PORT_2_GID_6_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_6_CMP_PORT_2_GID_6_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_6_CMP_PORT_2_GID_6_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_2_GID_7_CMP_OFFSET                                       0x0000015C
#define SEC_PORT_2_GID_7_CMP_TYPE                                         UInt32
#define SEC_PORT_2_GID_7_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_2_GID_7_CMP_PORT_2_GID_7_CMP_BITS_SHIFT               16
#define    SEC_PORT_2_GID_7_CMP_PORT_2_GID_7_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_2_GID_7_CMP_PORT_2_GID_7_CMP_VALUE_SHIFT              0
#define    SEC_PORT_2_GID_7_CMP_PORT_2_GID_7_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_0_CMP_OFFSET                                       0x00000160
#define SEC_PORT_3_GID_0_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_0_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_0_CMP_PORT_3_GID_0_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_0_CMP_PORT_3_GID_0_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_0_CMP_PORT_3_GID_0_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_0_CMP_PORT_3_GID_0_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_1_CMP_OFFSET                                       0x00000164
#define SEC_PORT_3_GID_1_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_1_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_1_CMP_PORT_3_GID_1_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_1_CMP_PORT_3_GID_1_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_1_CMP_PORT_3_GID_1_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_1_CMP_PORT_3_GID_1_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_2_CMP_OFFSET                                       0x00000168
#define SEC_PORT_3_GID_2_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_2_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_2_CMP_PORT_3_GID_2_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_2_CMP_PORT_3_GID_2_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_2_CMP_PORT_3_GID_2_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_2_CMP_PORT_3_GID_2_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_3_CMP_OFFSET                                       0x0000016C
#define SEC_PORT_3_GID_3_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_3_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_3_CMP_PORT_3_GID_3_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_3_CMP_PORT_3_GID_3_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_3_CMP_PORT_3_GID_3_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_3_CMP_PORT_3_GID_3_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_4_CMP_OFFSET                                       0x00000170
#define SEC_PORT_3_GID_4_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_4_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_4_CMP_PORT_3_GID_4_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_4_CMP_PORT_3_GID_4_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_4_CMP_PORT_3_GID_4_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_4_CMP_PORT_3_GID_4_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_5_CMP_OFFSET                                       0x00000174
#define SEC_PORT_3_GID_5_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_5_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_5_CMP_PORT_3_GID_5_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_5_CMP_PORT_3_GID_5_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_5_CMP_PORT_3_GID_5_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_5_CMP_PORT_3_GID_5_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_6_CMP_OFFSET                                       0x00000178
#define SEC_PORT_3_GID_6_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_6_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_6_CMP_PORT_3_GID_6_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_6_CMP_PORT_3_GID_6_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_6_CMP_PORT_3_GID_6_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_6_CMP_PORT_3_GID_6_CMP_VALUE_MASK               0x00001FFF

#define SEC_PORT_3_GID_7_CMP_OFFSET                                       0x0000017C
#define SEC_PORT_3_GID_7_CMP_TYPE                                         UInt32
#define SEC_PORT_3_GID_7_CMP_RESERVED_MASK                                0xE000E000
#define    SEC_PORT_3_GID_7_CMP_PORT_3_GID_7_CMP_BITS_SHIFT               16
#define    SEC_PORT_3_GID_7_CMP_PORT_3_GID_7_CMP_BITS_MASK                0x1FFF0000
#define    SEC_PORT_3_GID_7_CMP_PORT_3_GID_7_CMP_VALUE_SHIFT              0
#define    SEC_PORT_3_GID_7_CMP_PORT_3_GID_7_CMP_VALUE_MASK               0x00001FFF

#define SEC_VC_PROCESSOR_GID_OFFSET                                       0x00000180
#define SEC_VC_PROCESSOR_GID_TYPE                                         UInt32
#define SEC_VC_PROCESSOR_GID_RESERVED_MASK                                0xFFFFFFF8
#define    SEC_VC_PROCESSOR_GID_VC_PROCESSOR_GID_SHIFT                    0
#define    SEC_VC_PROCESSOR_GID_VC_PROCESSOR_GID_MASK                     0x00000007

#define SEC_DRAM_CONFIG_OFFSET                                            0x00000200
#define SEC_DRAM_CONFIG_TYPE                                              UInt32
#define SEC_DRAM_CONFIG_RESERVED_MASK                                     0xFFFFEE00
#define    SEC_DRAM_CONFIG_DDR_IF_WIDTH_SHIFT                             12
#define    SEC_DRAM_CONFIG_DDR_IF_WIDTH_MASK                              0x00001000
#define    SEC_DRAM_CONFIG_PASR_SHIFT                                     8
#define    SEC_DRAM_CONFIG_PASR_MASK                                      0x00000100
#define    SEC_DRAM_CONFIG_DRAM_DEVICE_WIDTH_SHIFT                        6
#define    SEC_DRAM_CONFIG_DRAM_DEVICE_WIDTH_MASK                         0x000000C0
#define    SEC_DRAM_CONFIG_DRAM_DENSITY_SHIFT                             2
#define    SEC_DRAM_CONFIG_DRAM_DENSITY_MASK                              0x0000003C
#define    SEC_DRAM_CONFIG_MEM_TYPE_SHIFT                                 0
#define    SEC_DRAM_CONFIG_MEM_TYPE_MASK                                  0x00000003

#define SEC_DRAM_START_ADDR_OFFSET                                        0x00000204
#define SEC_DRAM_START_ADDR_TYPE                                          UInt32
#define SEC_DRAM_START_ADDR_RESERVED_MASK                                 0x00000000
#define    SEC_DRAM_START_ADDR_DRAM_START_ADDR_SHIFT                      12
#define    SEC_DRAM_START_ADDR_DRAM_START_ADDR_MASK                       0xFFFFF000
#define    SEC_DRAM_START_ADDR_DRAM_START_ADDR_LSB_BITS_SHIFT             0
#define    SEC_DRAM_START_ADDR_DRAM_START_ADDR_LSB_BITS_MASK              0x00000FFF

#define SEC_DRAM_END_ADDR_OFFSET                                          0x00000208
#define SEC_DRAM_END_ADDR_TYPE                                            UInt32
#define SEC_DRAM_END_ADDR_RESERVED_MASK                                   0x00000000
#define    SEC_DRAM_END_ADDR_DRAM_END_ADDR_SHIFT                          12
#define    SEC_DRAM_END_ADDR_DRAM_END_ADDR_MASK                           0xFFFFF000
#define    SEC_DRAM_END_ADDR_DRAM_END_ADDR_LSB_BITS_SHIFT                 0
#define    SEC_DRAM_END_ADDR_DRAM_END_ADDR_LSB_BITS_MASK                  0x00000FFF

#define SEC_NVM_START_ADDR_OFFSET                                         0x0000020C
#define SEC_NVM_START_ADDR_TYPE                                           UInt32
#define SEC_NVM_START_ADDR_RESERVED_MASK                                  0x00000000
#define    SEC_NVM_START_ADDR_NVM_START_ADDR_SHIFT                        12
#define    SEC_NVM_START_ADDR_NVM_START_ADDR_MASK                         0xFFFFF000
#define    SEC_NVM_START_ADDR_NVM_START_ADDR_LSB_BITS_SHIFT               0
#define    SEC_NVM_START_ADDR_NVM_START_ADDR_LSB_BITS_MASK                0x00000FFF

#define SEC_NVM_END_ADDR_OFFSET                                           0x00000210
#define SEC_NVM_END_ADDR_TYPE                                             UInt32
#define SEC_NVM_END_ADDR_RESERVED_MASK                                    0x00000000
#define    SEC_NVM_END_ADDR_NVM_END_ADDR_SHIFT                            12
#define    SEC_NVM_END_ADDR_NVM_END_ADDR_MASK                             0xFFFFF000
#define    SEC_NVM_END_ADDR_NVM_END_ADDR_LSB_BITS_SHIFT                   0
#define    SEC_NVM_END_ADDR_NVM_END_ADDR_LSB_BITS_MASK                    0x00000FFF

#define SEC_NVM_OWBA_START_ADDR_OFFSET                                    0x00000214
#define SEC_NVM_OWBA_START_ADDR_TYPE                                      UInt32
#define SEC_NVM_OWBA_START_ADDR_RESERVED_MASK                             0x00000000
#define    SEC_NVM_OWBA_START_ADDR_OWBA_START_ADDR_SHIFT                  13
#define    SEC_NVM_OWBA_START_ADDR_OWBA_START_ADDR_MASK                   0xFFFFE000
#define    SEC_NVM_OWBA_START_ADDR_OWBA_START_ADDR_LSB_BITS_SHIFT         0
#define    SEC_NVM_OWBA_START_ADDR_OWBA_START_ADDR_LSB_BITS_MASK          0x00001FFF

#define SEC_NVM_OWBA_END_ADDR_OFFSET                                      0x00000218
#define SEC_NVM_OWBA_END_ADDR_TYPE                                        UInt32
#define SEC_NVM_OWBA_END_ADDR_RESERVED_MASK                               0x00000000
#define    SEC_NVM_OWBA_END_ADDR_OWBA_END_ADDR_SHIFT                      13
#define    SEC_NVM_OWBA_END_ADDR_OWBA_END_ADDR_MASK                       0xFFFFE000
#define    SEC_NVM_OWBA_END_ADDR_OWBA_END_ADDR_LSB_BITS_SHIFT             0
#define    SEC_NVM_OWBA_END_ADDR_OWBA_END_ADDR_LSB_BITS_MASK              0x00001FFF

#define SEC_NVM_OW_CTRL_OFFSET                                            0x0000021C
#define SEC_NVM_OW_CTRL_TYPE                                              UInt32
#define SEC_NVM_OW_CTRL_RESERVED_MASK                                     0xFFFFFFFE
#define    SEC_NVM_OW_CTRL_OW_WR_ENABLE_SHIFT                             0
#define    SEC_NVM_OW_CTRL_OW_WR_ENABLE_MASK                              0x00000001

#define SEC_ANTI_CLONING_CTRL_OFFSET                                      0x00000220
#define SEC_ANTI_CLONING_CTRL_TYPE                                        UInt32
#define SEC_ANTI_CLONING_CTRL_RESERVED_MASK                               0xFFFFFFF8
#define    SEC_ANTI_CLONING_CTRL_NVM_OWBA_ADDR_LOCK_SHIFT                 2
#define    SEC_ANTI_CLONING_CTRL_NVM_OWBA_ADDR_LOCK_MASK                  0x00000004
#define    SEC_ANTI_CLONING_CTRL_NVM_ADDR_LOCK_SHIFT                      1
#define    SEC_ANTI_CLONING_CTRL_NVM_ADDR_LOCK_MASK                       0x00000002
#define    SEC_ANTI_CLONING_CTRL_DRAM_ADDR_LOCK_SHIFT                     0
#define    SEC_ANTI_CLONING_CTRL_DRAM_ADDR_LOCK_MASK                      0x00000001

#define SEC_MEMC_DEBUG_CTRL_OFFSET                                        0x00000300
#define SEC_MEMC_DEBUG_CTRL_TYPE                                          UInt32
#define SEC_MEMC_DEBUG_CTRL_RESERVED_MASK                                 0xFFFFFFFE
#define    SEC_MEMC_DEBUG_CTRL_TZCFG_MEMC_DBG_EN_SHIFT                    0
#define    SEC_MEMC_DEBUG_CTRL_TZCFG_MEMC_DBG_EN_MASK                     0x00000001

#endif /* __BRCM_RDB_SEC_H__ */



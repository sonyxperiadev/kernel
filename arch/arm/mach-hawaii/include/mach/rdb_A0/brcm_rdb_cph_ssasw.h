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

#ifndef __BRCM_RDB_CPH_SSASW_H__
#define __BRCM_RDB_CPH_SSASW_H__

#define CPH_SSASW_SSASW_NOC_OFFSET                                        0x00000000
#define CPH_SSASW_SSASW_NOC_TYPE                                          UInt32
#define CPH_SSASW_SSASW_NOC_RESERVED_MASK                                 0xFFFFFFFC
#define    CPH_SSASW_SSASW_NOC_SSASW_CLKGATE_BYPASS_SHIFT                 1
#define    CPH_SSASW_SSASW_NOC_SSASW_CLKGATE_BYPASS_MASK                  0x00000002
#define    CPH_SSASW_SSASW_NOC_SSASW_NOC_EN_SHIFT                         0
#define    CPH_SSASW_SSASW_NOC_SSASW_NOC_EN_MASK                          0x00000001

#define CPH_SSASW_SSASW_MN0_DIVIDER_OFFSET                                0x00000004
#define CPH_SSASW_SSASW_MN0_DIVIDER_TYPE                                  UInt32
#define CPH_SSASW_SSASW_MN0_DIVIDER_RESERVED_MASK                         0xFFFFC000
#define    CPH_SSASW_SSASW_MN0_DIVIDER_M0_RATIO_SHIFT                     11
#define    CPH_SSASW_SSASW_MN0_DIVIDER_M0_RATIO_MASK                      0x00003800
#define    CPH_SSASW_SSASW_MN0_DIVIDER_N0_RATIO_SHIFT                     0
#define    CPH_SSASW_SSASW_MN0_DIVIDER_N0_RATIO_MASK                      0x000007FF

#define CPH_SSASW_SSASW_MN1_DIVIDER_OFFSET                                0x00000008
#define CPH_SSASW_SSASW_MN1_DIVIDER_TYPE                                  UInt32
#define CPH_SSASW_SSASW_MN1_DIVIDER_RESERVED_MASK                         0xFFFE0000
#define    CPH_SSASW_SSASW_MN1_DIVIDER_M1_RATIO_SHIFT                     9
#define    CPH_SSASW_SSASW_MN1_DIVIDER_M1_RATIO_MASK                      0x0001FE00
#define    CPH_SSASW_SSASW_MN1_DIVIDER_N1_RATIO_SHIFT                     0
#define    CPH_SSASW_SSASW_MN1_DIVIDER_N1_RATIO_MASK                      0x000001FF

#define CPH_SSASW_CH01_SRC_OFFSET                                         0x00000010
#define CPH_SSASW_CH01_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH01_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH01_SRC_CH01_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH01_SRC_CH01_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH01_SRC_CH01_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH01_SRC_CH01_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH01_SRC_CH01_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH01_SRC_CH01_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH01_SRC_CH01_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH01_SRC_CH01_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH01_DST1_OFFSET                                        0x00000014
#define CPH_SSASW_CH01_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH01_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH01_DST1_CH01_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH01_DST1_CH01_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH01_DST1_CH01_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH01_DST2_OFFSET                                        0x00000018
#define CPH_SSASW_CH01_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH01_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH01_DST2_CH01_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH01_DST2_CH01_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH01_DST2_CH01_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH01_DST2_CH01_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH01_DST3_OFFSET                                        0x0000001C
#define CPH_SSASW_CH01_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH01_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH01_DST3_CH01_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH01_DST3_CH01_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH01_DST3_CH01_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH01_DST3_CH01_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH01_DST4_OFFSET                                        0x00000020
#define CPH_SSASW_CH01_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH01_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH01_DST4_CH01_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH01_DST4_CH01_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH01_DST4_CH01_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH01_DST4_CH01_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH02_SRC_OFFSET                                         0x00000030
#define CPH_SSASW_CH02_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH02_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH02_SRC_CH02_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH02_SRC_CH02_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH02_SRC_CH02_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH02_SRC_CH02_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH02_SRC_CH02_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH02_SRC_CH02_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH02_SRC_CH02_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH02_SRC_CH02_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH02_DST1_OFFSET                                        0x00000034
#define CPH_SSASW_CH02_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH02_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH02_DST1_CH02_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH02_DST1_CH02_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH02_DST1_CH02_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH02_DST1_CH02_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH02_DST2_OFFSET                                        0x00000038
#define CPH_SSASW_CH02_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH02_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH02_DST2_CH02_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH02_DST2_CH02_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH02_DST2_CH02_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH02_DST2_CH02_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH02_DST3_OFFSET                                        0x0000003C
#define CPH_SSASW_CH02_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH02_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH02_DST3_CH02_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH02_DST3_CH02_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH02_DST3_CH02_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH02_DST3_CH02_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH02_DST4_OFFSET                                        0x00000040
#define CPH_SSASW_CH02_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH02_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH02_DST4_CH02_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH02_DST4_CH02_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH02_DST4_CH02_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH02_DST4_CH02_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH03_SRC_OFFSET                                         0x00000050
#define CPH_SSASW_CH03_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH03_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH03_SRC_CH03_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH03_SRC_CH03_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH03_SRC_CH03_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH03_SRC_CH03_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH03_SRC_CH03_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH03_SRC_CH03_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH03_SRC_CH03_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH03_SRC_CH03_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH03_DST1_OFFSET                                        0x00000054
#define CPH_SSASW_CH03_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH03_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH03_DST1_CH03_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH03_DST1_CH03_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH03_DST1_CH03_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH03_DST1_CH03_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH03_DST2_OFFSET                                        0x00000058
#define CPH_SSASW_CH03_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH03_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH03_DST2_CH03_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH03_DST2_CH03_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH03_DST2_CH03_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH03_DST2_CH03_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH03_DST3_OFFSET                                        0x0000005C
#define CPH_SSASW_CH03_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH03_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH03_DST3_CH03_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH03_DST3_CH03_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH03_DST3_CH03_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH03_DST3_CH03_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH03_DST4_OFFSET                                        0x00000060
#define CPH_SSASW_CH03_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH03_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH03_DST4_CH03_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH03_DST4_CH03_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH03_DST4_CH03_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH03_DST4_CH03_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH04_SRC_OFFSET                                         0x00000070
#define CPH_SSASW_CH04_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH04_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH04_SRC_CH04_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH04_SRC_CH04_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH04_SRC_CH04_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH04_SRC_CH04_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH04_SRC_CH04_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH04_SRC_CH04_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH04_SRC_CH04_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH04_SRC_CH04_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH04_DST1_OFFSET                                        0x00000074
#define CPH_SSASW_CH04_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH04_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH04_DST1_CH04_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH04_DST1_CH04_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH04_DST1_CH04_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH04_DST1_CH04_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH04_DST2_OFFSET                                        0x00000078
#define CPH_SSASW_CH04_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH04_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH04_DST2_CH04_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH04_DST2_CH04_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH04_DST2_CH04_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH04_DST2_CH04_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH04_DST3_OFFSET                                        0x0000007C
#define CPH_SSASW_CH04_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH04_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH04_DST3_CH04_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH04_DST3_CH04_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH04_DST3_CH04_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH04_DST3_CH04_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH04_DST4_OFFSET                                        0x00000080
#define CPH_SSASW_CH04_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH04_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH04_DST4_CH04_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH04_DST4_CH04_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH04_DST4_CH04_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH04_DST4_CH04_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH05_SRC_OFFSET                                         0x00000090
#define CPH_SSASW_CH05_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH05_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH05_SRC_CH05_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH05_SRC_CH05_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH05_SRC_CH05_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH05_SRC_CH05_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH05_SRC_CH05_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH05_SRC_CH05_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH05_SRC_CH05_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH05_SRC_CH05_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH05_DST1_OFFSET                                        0x00000094
#define CPH_SSASW_CH05_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH05_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH05_DST1_CH05_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH05_DST1_CH05_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH05_DST1_CH05_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH05_DST1_CH05_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH05_DST2_OFFSET                                        0x00000098
#define CPH_SSASW_CH05_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH05_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH05_DST2_CH05_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH05_DST2_CH05_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH05_DST2_CH05_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH05_DST2_CH05_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH05_DST3_OFFSET                                        0x0000009C
#define CPH_SSASW_CH05_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH05_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH05_DST3_CH05_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH05_DST3_CH05_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH05_DST3_CH05_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH05_DST3_CH05_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH05_DST4_OFFSET                                        0x000000A0
#define CPH_SSASW_CH05_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH05_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH05_DST4_CH05_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH05_DST4_CH05_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH05_DST4_CH05_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH05_DST4_CH05_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH06_SRC_OFFSET                                         0x000000B0
#define CPH_SSASW_CH06_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH06_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH06_SRC_CH06_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH06_SRC_CH06_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH06_SRC_CH06_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH06_SRC_CH06_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH06_SRC_CH06_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH06_SRC_CH06_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH06_SRC_CH06_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH06_SRC_CH06_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH06_DST1_OFFSET                                        0x000000B4
#define CPH_SSASW_CH06_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH06_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH06_DST1_CH06_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH06_DST1_CH06_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH06_DST1_CH06_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH06_DST1_CH06_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH06_DST2_OFFSET                                        0x000000B8
#define CPH_SSASW_CH06_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH06_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH06_DST2_CH06_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH06_DST2_CH06_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH06_DST2_CH06_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH06_DST2_CH06_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH06_DST3_OFFSET                                        0x000000BC
#define CPH_SSASW_CH06_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH06_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH06_DST3_CH06_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH06_DST3_CH06_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH06_DST3_CH06_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH06_DST3_CH06_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH06_DST4_OFFSET                                        0x000000C0
#define CPH_SSASW_CH06_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH06_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH06_DST4_CH06_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH06_DST4_CH06_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH06_DST4_CH06_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH06_DST4_CH06_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH07_SRC_OFFSET                                         0x000000D0
#define CPH_SSASW_CH07_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH07_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH07_SRC_CH07_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH07_SRC_CH07_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH07_SRC_CH07_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH07_SRC_CH07_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH07_SRC_CH07_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH07_SRC_CH07_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH07_SRC_CH07_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH07_SRC_CH07_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH07_DST1_OFFSET                                        0x000000D4
#define CPH_SSASW_CH07_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH07_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH07_DST1_CH07_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH07_DST1_CH07_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH07_DST1_CH07_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH07_DST1_CH07_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH07_DST2_OFFSET                                        0x000000D8
#define CPH_SSASW_CH07_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH07_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH07_DST2_CH07_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH07_DST2_CH07_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH07_DST2_CH07_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH07_DST2_CH07_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH07_DST3_OFFSET                                        0x000000DC
#define CPH_SSASW_CH07_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH07_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH07_DST3_CH07_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH07_DST3_CH07_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH07_DST3_CH07_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH07_DST3_CH07_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH07_DST4_OFFSET                                        0x000000E0
#define CPH_SSASW_CH07_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH07_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH07_DST4_CH07_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH07_DST4_CH07_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH07_DST4_CH07_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH07_DST4_CH07_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH08_SRC_OFFSET                                         0x000000F0
#define CPH_SSASW_CH08_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH08_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH08_SRC_CH08_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH08_SRC_CH08_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH08_SRC_CH08_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH08_SRC_CH08_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH08_SRC_CH08_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH08_SRC_CH08_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH08_SRC_CH08_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH08_SRC_CH08_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH08_DST1_OFFSET                                        0x000000F4
#define CPH_SSASW_CH08_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH08_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH08_DST1_CH08_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH08_DST1_CH08_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH08_DST1_CH08_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH08_DST1_CH08_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH08_DST2_OFFSET                                        0x000000F8
#define CPH_SSASW_CH08_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH08_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH08_DST2_CH08_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH08_DST2_CH08_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH08_DST2_CH08_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH08_DST2_CH08_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH08_DST3_OFFSET                                        0x000000FC
#define CPH_SSASW_CH08_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH08_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH08_DST3_CH08_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH08_DST3_CH08_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH08_DST3_CH08_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH08_DST3_CH08_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH08_DST4_OFFSET                                        0x00000100
#define CPH_SSASW_CH08_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH08_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH08_DST4_CH08_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH08_DST4_CH08_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH08_DST4_CH08_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH08_DST4_CH08_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH09_SRC_OFFSET                                         0x00000110
#define CPH_SSASW_CH09_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH09_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH09_SRC_CH09_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH09_SRC_CH09_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH09_SRC_CH09_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH09_SRC_CH09_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH09_SRC_CH09_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH09_SRC_CH09_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH09_SRC_CH09_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH09_SRC_CH09_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH09_DST1_OFFSET                                        0x00000114
#define CPH_SSASW_CH09_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH09_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH09_DST1_CH09_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH09_DST1_CH09_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH09_DST1_CH09_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH09_DST1_CH09_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH09_DST2_OFFSET                                        0x00000118
#define CPH_SSASW_CH09_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH09_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH09_DST2_CH09_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH09_DST2_CH09_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH09_DST2_CH09_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH09_DST2_CH09_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH09_DST3_OFFSET                                        0x0000011C
#define CPH_SSASW_CH09_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH09_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH09_DST3_CH09_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH09_DST3_CH09_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH09_DST3_CH09_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH09_DST3_CH09_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH09_DST4_OFFSET                                        0x00000120
#define CPH_SSASW_CH09_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH09_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH09_DST4_CH09_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH09_DST4_CH09_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH09_DST4_CH09_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH09_DST4_CH09_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH10_SRC_OFFSET                                         0x00000130
#define CPH_SSASW_CH10_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH10_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH10_SRC_CH10_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH10_SRC_CH10_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH10_SRC_CH10_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH10_SRC_CH10_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH10_SRC_CH10_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH10_SRC_CH10_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH10_SRC_CH10_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH10_SRC_CH10_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH10_DST1_OFFSET                                        0x00000134
#define CPH_SSASW_CH10_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH10_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH10_DST1_CH10_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH10_DST1_CH10_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH10_DST1_CH10_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH10_DST1_CH10_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH10_DST2_OFFSET                                        0x00000138
#define CPH_SSASW_CH10_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH10_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH10_DST2_CH10_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH10_DST2_CH10_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH10_DST2_CH10_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH10_DST2_CH10_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH10_DST3_OFFSET                                        0x0000013C
#define CPH_SSASW_CH10_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH10_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH10_DST3_CH10_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH10_DST3_CH10_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH10_DST3_CH10_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH10_DST3_CH10_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH10_DST4_OFFSET                                        0x00000140
#define CPH_SSASW_CH10_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH10_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH10_DST4_CH10_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH10_DST4_CH10_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH10_DST4_CH10_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH10_DST4_CH10_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH11_SRC_OFFSET                                         0x00000150
#define CPH_SSASW_CH11_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH11_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH11_SRC_CH11_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH11_SRC_CH11_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH11_SRC_CH11_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH11_SRC_CH11_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH11_SRC_CH11_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH11_SRC_CH11_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH11_SRC_CH11_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH11_SRC_CH11_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH11_DST1_OFFSET                                        0x00000154
#define CPH_SSASW_CH11_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH11_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH11_DST1_CH11_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH11_DST1_CH11_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH11_DST1_CH11_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH11_DST1_CH11_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH11_DST2_OFFSET                                        0x00000158
#define CPH_SSASW_CH11_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH11_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH11_DST2_CH11_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH11_DST2_CH11_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH11_DST2_CH11_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH11_DST2_CH11_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH11_DST3_OFFSET                                        0x0000015C
#define CPH_SSASW_CH11_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH11_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH11_DST3_CH11_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH11_DST3_CH11_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH11_DST3_CH11_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH11_DST3_CH11_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH11_DST4_OFFSET                                        0x00000160
#define CPH_SSASW_CH11_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH11_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH11_DST4_CH11_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH11_DST4_CH11_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH11_DST4_CH11_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH11_DST4_CH11_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH12_SRC_OFFSET                                         0x00000170
#define CPH_SSASW_CH12_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH12_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH12_SRC_CH12_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH12_SRC_CH12_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH12_SRC_CH12_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH12_SRC_CH12_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH12_SRC_CH12_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH12_SRC_CH12_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH12_SRC_CH12_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH12_SRC_CH12_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH12_DST1_OFFSET                                        0x00000174
#define CPH_SSASW_CH12_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH12_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH12_DST1_CH12_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH12_DST1_CH12_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH12_DST1_CH12_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH12_DST1_CH12_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH12_DST2_OFFSET                                        0x00000178
#define CPH_SSASW_CH12_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH12_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH12_DST2_CH12_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH12_DST2_CH12_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH12_DST2_CH12_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH12_DST2_CH12_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH12_DST3_OFFSET                                        0x0000017C
#define CPH_SSASW_CH12_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH12_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH12_DST3_CH12_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH12_DST3_CH12_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH12_DST3_CH12_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH12_DST3_CH12_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH12_DST4_OFFSET                                        0x00000180
#define CPH_SSASW_CH12_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH12_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH12_DST4_CH12_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH12_DST4_CH12_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH12_DST4_CH12_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH12_DST4_CH12_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH13_SRC_OFFSET                                         0x00000190
#define CPH_SSASW_CH13_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH13_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH13_SRC_CH13_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH13_SRC_CH13_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH13_SRC_CH13_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH13_SRC_CH13_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH13_SRC_CH13_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH13_SRC_CH13_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH13_SRC_CH13_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH13_SRC_CH13_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH13_DST1_OFFSET                                        0x00000194
#define CPH_SSASW_CH13_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH13_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH13_DST1_CH13_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH13_DST1_CH13_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH13_DST1_CH13_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH13_DST1_CH13_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH13_DST2_OFFSET                                        0x00000198
#define CPH_SSASW_CH13_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH13_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH13_DST2_CH13_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH13_DST2_CH13_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH13_DST2_CH13_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH13_DST2_CH13_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH13_DST3_OFFSET                                        0x0000019C
#define CPH_SSASW_CH13_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH13_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH13_DST3_CH13_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH13_DST3_CH13_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH13_DST3_CH13_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH13_DST3_CH13_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH13_DST4_OFFSET                                        0x000001A0
#define CPH_SSASW_CH13_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH13_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH13_DST4_CH13_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH13_DST4_CH13_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH13_DST4_CH13_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH13_DST4_CH13_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH14_SRC_OFFSET                                         0x000001B0
#define CPH_SSASW_CH14_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH14_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH14_SRC_CH14_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH14_SRC_CH14_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH14_SRC_CH14_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH14_SRC_CH14_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH14_SRC_CH14_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH14_SRC_CH14_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH14_SRC_CH14_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH14_SRC_CH14_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH14_DST1_OFFSET                                        0x000001B4
#define CPH_SSASW_CH14_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH14_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH14_DST1_CH14_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH14_DST1_CH14_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH14_DST1_CH14_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH14_DST1_CH14_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH14_DST2_OFFSET                                        0x000001B8
#define CPH_SSASW_CH14_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH14_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH14_DST2_CH14_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH14_DST2_CH14_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH14_DST2_CH14_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH14_DST2_CH14_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH14_DST3_OFFSET                                        0x000001BC
#define CPH_SSASW_CH14_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH14_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH14_DST3_CH14_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH14_DST3_CH14_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH14_DST3_CH14_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH14_DST3_CH14_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH14_DST4_OFFSET                                        0x000001C0
#define CPH_SSASW_CH14_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH14_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH14_DST4_CH14_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH14_DST4_CH14_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH14_DST4_CH14_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH14_DST4_CH14_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH15_SRC_OFFSET                                         0x000001D0
#define CPH_SSASW_CH15_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH15_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH15_SRC_CH15_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH15_SRC_CH15_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH15_SRC_CH15_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH15_SRC_CH15_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH15_SRC_CH15_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH15_SRC_CH15_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH15_SRC_CH15_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH15_SRC_CH15_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH15_DST1_OFFSET                                        0x000001D4
#define CPH_SSASW_CH15_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH15_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH15_DST1_CH15_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH15_DST1_CH15_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH15_DST1_CH15_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH15_DST1_CH15_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH15_DST2_OFFSET                                        0x000001D8
#define CPH_SSASW_CH15_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH15_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH15_DST2_CH15_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH15_DST2_CH15_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH15_DST2_CH15_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH15_DST2_CH15_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH15_DST3_OFFSET                                        0x000001DC
#define CPH_SSASW_CH15_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH15_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH15_DST3_CH15_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH15_DST3_CH15_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH15_DST3_CH15_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH15_DST3_CH15_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH15_DST4_OFFSET                                        0x000001E0
#define CPH_SSASW_CH15_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH15_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH15_DST4_CH15_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH15_DST4_CH15_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH15_DST4_CH15_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH15_DST4_CH15_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH16_SRC_OFFSET                                         0x000001F0
#define CPH_SSASW_CH16_SRC_TYPE                                           UInt32
#define CPH_SSASW_CH16_SRC_RESERVED_MASK                                  0x3F800000
#define    CPH_SSASW_CH16_SRC_CH16_SRC_EN_SHIFT                           31
#define    CPH_SSASW_CH16_SRC_CH16_SRC_EN_MASK                            0x80000000
#define    CPH_SSASW_CH16_SRC_CH16_TWO_WORDS_SHIFT                        30
#define    CPH_SSASW_CH16_SRC_CH16_TWO_WORDS_MASK                         0x40000000
#define    CPH_SSASW_CH16_SRC_CH16_TRIGGER_SEL_SHIFT                      16
#define    CPH_SSASW_CH16_SRC_CH16_TRIGGER_SEL_MASK                       0x007F0000
#define    CPH_SSASW_CH16_SRC_CH16_SRC_ADDR_SHIFT                         0
#define    CPH_SSASW_CH16_SRC_CH16_SRC_ADDR_MASK                          0x0000FFFF

#define CPH_SSASW_CH16_DST1_OFFSET                                        0x000001F4
#define CPH_SSASW_CH16_DST1_TYPE                                          UInt32
#define CPH_SSASW_CH16_DST1_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH16_DST1_CH16_DST1_EN_SHIFT                         31
#define    CPH_SSASW_CH16_DST1_CH16_DST1_EN_MASK                          0x80000000
#define    CPH_SSASW_CH16_DST1_CH16_DST1_ADDR_SHIFT                       0
#define    CPH_SSASW_CH16_DST1_CH16_DST1_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH16_DST2_OFFSET                                        0x000001F8
#define CPH_SSASW_CH16_DST2_TYPE                                          UInt32
#define CPH_SSASW_CH16_DST2_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH16_DST2_CH16_DST2_EN_SHIFT                         31
#define    CPH_SSASW_CH16_DST2_CH16_DST2_EN_MASK                          0x80000000
#define    CPH_SSASW_CH16_DST2_CH16_DST2_ADDR_SHIFT                       0
#define    CPH_SSASW_CH16_DST2_CH16_DST2_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH16_DST3_OFFSET                                        0x000001FC
#define CPH_SSASW_CH16_DST3_TYPE                                          UInt32
#define CPH_SSASW_CH16_DST3_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH16_DST3_CH16_DST3_EN_SHIFT                         31
#define    CPH_SSASW_CH16_DST3_CH16_DST3_EN_MASK                          0x80000000
#define    CPH_SSASW_CH16_DST3_CH16_DST3_ADDR_SHIFT                       0
#define    CPH_SSASW_CH16_DST3_CH16_DST3_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_CH16_DST4_OFFSET                                        0x00000200
#define CPH_SSASW_CH16_DST4_TYPE                                          UInt32
#define CPH_SSASW_CH16_DST4_RESERVED_MASK                                 0x7FFF0000
#define    CPH_SSASW_CH16_DST4_CH16_DST4_EN_SHIFT                         31
#define    CPH_SSASW_CH16_DST4_CH16_DST4_EN_MASK                          0x80000000
#define    CPH_SSASW_CH16_DST4_CH16_DST4_ADDR_SHIFT                       0
#define    CPH_SSASW_CH16_DST4_CH16_DST4_ADDR_MASK                        0x0000FFFF

#define CPH_SSASW_PREADY_MAX_TIME_OFFSET                                  0x000007F8
#define CPH_SSASW_PREADY_MAX_TIME_TYPE                                    UInt32
#define CPH_SSASW_PREADY_MAX_TIME_RESERVED_MASK                           0xFFFFFF00
#define    CPH_SSASW_PREADY_MAX_TIME_PREADY_MAX_TIME_SHIFT                0
#define    CPH_SSASW_PREADY_MAX_TIME_PREADY_MAX_TIME_MASK                 0x000000FF

#define CPH_SSASW_SSASW_ERR_OFFSET                                        0x000007FC
#define CPH_SSASW_SSASW_ERR_TYPE                                          UInt32
#define CPH_SSASW_SSASW_ERR_RESERVED_MASK                                 0x1FFFFFFF
#define    CPH_SSASW_SSASW_ERR_APB_PREADY_ERR_SHIFT                       31
#define    CPH_SSASW_SSASW_ERR_APB_PREADY_ERR_MASK                        0x80000000
#define    CPH_SSASW_SSASW_ERR_APB_PSLVERR_ERR_SHIFT                      30
#define    CPH_SSASW_SSASW_ERR_APB_PSLVERR_ERR_MASK                       0x40000000
#define    CPH_SSASW_SSASW_ERR_CH_DST_ERR_SHIFT                           29
#define    CPH_SSASW_SSASW_ERR_CH_DST_ERR_MASK                            0x20000000

#define CPH_SSASW_DSP_SSASW_NOC_OFFSET                                    0x00000000
#define CPH_SSASW_DSP_SSASW_NOC_TYPE                                      UInt32
#define CPH_SSASW_DSP_SSASW_NOC_RESERVED_MASK                             0xFFFFFFFC
#define    CPH_SSASW_DSP_SSASW_NOC_SSASW_CLKGATE_BYPASS_SHIFT             1
#define    CPH_SSASW_DSP_SSASW_NOC_SSASW_CLKGATE_BYPASS_MASK              0x00000002
#define    CPH_SSASW_DSP_SSASW_NOC_SSASW_NOC_EN_SHIFT                     0
#define    CPH_SSASW_DSP_SSASW_NOC_SSASW_NOC_EN_MASK                      0x00000001

#define CPH_SSASW_DSP_SSASW_MN0_DIVIDER_OFFSET                            0x00000002
#define CPH_SSASW_DSP_SSASW_MN0_DIVIDER_TYPE                              UInt32
#define CPH_SSASW_DSP_SSASW_MN0_DIVIDER_RESERVED_MASK                     0xFFFFC000
#define    CPH_SSASW_DSP_SSASW_MN0_DIVIDER_M0_RATIO_SHIFT                 11
#define    CPH_SSASW_DSP_SSASW_MN0_DIVIDER_M0_RATIO_MASK                  0x00003800
#define    CPH_SSASW_DSP_SSASW_MN0_DIVIDER_N0_RATIO_SHIFT                 0
#define    CPH_SSASW_DSP_SSASW_MN0_DIVIDER_N0_RATIO_MASK                  0x000007FF

#define CPH_SSASW_DSP_SSASW_MN1_DIVIDER_OFFSET                            0x00000004
#define CPH_SSASW_DSP_SSASW_MN1_DIVIDER_TYPE                              UInt32
#define CPH_SSASW_DSP_SSASW_MN1_DIVIDER_RESERVED_MASK                     0xFFFE0000
#define    CPH_SSASW_DSP_SSASW_MN1_DIVIDER_M1_RATIO_SHIFT                 9
#define    CPH_SSASW_DSP_SSASW_MN1_DIVIDER_M1_RATIO_MASK                  0x0001FE00
#define    CPH_SSASW_DSP_SSASW_MN1_DIVIDER_N1_RATIO_SHIFT                 0
#define    CPH_SSASW_DSP_SSASW_MN1_DIVIDER_N1_RATIO_MASK                  0x000001FF

#define CPH_SSASW_DSP_CH01_SRC_OFFSET                                     0x00000008
#define CPH_SSASW_DSP_CH01_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH01_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH01_SRC_CH01_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH01_SRC_CH01_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH01_SRC_CH01_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH01_SRC_CH01_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH01_SRC_CH01_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH01_SRC_CH01_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH01_SRC_CH01_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH01_SRC_CH01_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH01_DST1_OFFSET                                    0x0000000A
#define CPH_SSASW_DSP_CH01_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH01_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH01_DST1_CH01_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH01_DST1_CH01_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH01_DST1_CH01_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH01_DST1_CH01_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH01_DST2_OFFSET                                    0x0000000C
#define CPH_SSASW_DSP_CH01_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH01_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH01_DST2_CH01_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH01_DST2_CH01_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH01_DST2_CH01_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH01_DST2_CH01_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH01_DST3_OFFSET                                    0x0000000E
#define CPH_SSASW_DSP_CH01_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH01_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH01_DST3_CH01_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH01_DST3_CH01_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH01_DST3_CH01_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH01_DST3_CH01_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH01_DST4_OFFSET                                    0x00000010
#define CPH_SSASW_DSP_CH01_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH01_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH01_DST4_CH01_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH01_DST4_CH01_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH01_DST4_CH01_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH01_DST4_CH01_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH02_SRC_OFFSET                                     0x00000018
#define CPH_SSASW_DSP_CH02_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH02_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH02_SRC_CH02_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH02_SRC_CH02_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH02_SRC_CH02_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH02_SRC_CH02_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH02_SRC_CH02_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH02_SRC_CH02_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH02_SRC_CH02_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH02_SRC_CH02_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH02_DST1_OFFSET                                    0x0000001A
#define CPH_SSASW_DSP_CH02_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH02_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH02_DST1_CH02_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH02_DST1_CH02_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH02_DST1_CH02_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH02_DST1_CH02_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH02_DST2_OFFSET                                    0x0000001C
#define CPH_SSASW_DSP_CH02_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH02_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH02_DST2_CH02_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH02_DST2_CH02_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH02_DST2_CH02_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH02_DST2_CH02_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH02_DST3_OFFSET                                    0x0000001E
#define CPH_SSASW_DSP_CH02_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH02_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH02_DST3_CH02_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH02_DST3_CH02_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH02_DST3_CH02_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH02_DST3_CH02_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH02_DST4_OFFSET                                    0x00000020
#define CPH_SSASW_DSP_CH02_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH02_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH02_DST4_CH02_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH02_DST4_CH02_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH02_DST4_CH02_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH02_DST4_CH02_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH03_SRC_OFFSET                                     0x00000028
#define CPH_SSASW_DSP_CH03_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH03_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH03_SRC_CH03_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH03_SRC_CH03_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH03_SRC_CH03_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH03_SRC_CH03_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH03_SRC_CH03_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH03_SRC_CH03_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH03_SRC_CH03_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH03_SRC_CH03_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH03_DST1_OFFSET                                    0x0000002A
#define CPH_SSASW_DSP_CH03_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH03_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH03_DST1_CH03_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH03_DST1_CH03_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH03_DST1_CH03_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH03_DST1_CH03_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH03_DST2_OFFSET                                    0x0000002C
#define CPH_SSASW_DSP_CH03_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH03_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH03_DST2_CH03_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH03_DST2_CH03_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH03_DST2_CH03_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH03_DST2_CH03_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH03_DST3_OFFSET                                    0x0000002E
#define CPH_SSASW_DSP_CH03_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH03_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH03_DST3_CH03_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH03_DST3_CH03_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH03_DST3_CH03_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH03_DST3_CH03_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH03_DST4_OFFSET                                    0x00000030
#define CPH_SSASW_DSP_CH03_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH03_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH03_DST4_CH03_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH03_DST4_CH03_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH03_DST4_CH03_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH03_DST4_CH03_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH04_SRC_OFFSET                                     0x00000038
#define CPH_SSASW_DSP_CH04_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH04_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH04_SRC_CH04_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH04_SRC_CH04_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH04_SRC_CH04_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH04_SRC_CH04_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH04_SRC_CH04_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH04_SRC_CH04_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH04_SRC_CH04_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH04_SRC_CH04_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH04_DST1_OFFSET                                    0x0000003A
#define CPH_SSASW_DSP_CH04_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH04_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH04_DST1_CH04_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH04_DST1_CH04_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH04_DST1_CH04_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH04_DST1_CH04_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH04_DST2_OFFSET                                    0x0000003C
#define CPH_SSASW_DSP_CH04_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH04_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH04_DST2_CH04_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH04_DST2_CH04_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH04_DST2_CH04_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH04_DST2_CH04_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH04_DST3_OFFSET                                    0x0000003E
#define CPH_SSASW_DSP_CH04_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH04_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH04_DST3_CH04_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH04_DST3_CH04_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH04_DST3_CH04_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH04_DST3_CH04_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH04_DST4_OFFSET                                    0x00000040
#define CPH_SSASW_DSP_CH04_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH04_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH04_DST4_CH04_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH04_DST4_CH04_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH04_DST4_CH04_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH04_DST4_CH04_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH05_SRC_OFFSET                                     0x00000048
#define CPH_SSASW_DSP_CH05_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH05_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH05_SRC_CH05_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH05_SRC_CH05_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH05_SRC_CH05_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH05_SRC_CH05_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH05_SRC_CH05_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH05_SRC_CH05_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH05_SRC_CH05_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH05_SRC_CH05_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH05_DST1_OFFSET                                    0x0000004A
#define CPH_SSASW_DSP_CH05_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH05_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH05_DST1_CH05_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH05_DST1_CH05_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH05_DST1_CH05_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH05_DST1_CH05_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH05_DST2_OFFSET                                    0x0000004C
#define CPH_SSASW_DSP_CH05_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH05_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH05_DST2_CH05_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH05_DST2_CH05_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH05_DST2_CH05_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH05_DST2_CH05_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH05_DST3_OFFSET                                    0x0000004E
#define CPH_SSASW_DSP_CH05_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH05_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH05_DST3_CH05_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH05_DST3_CH05_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH05_DST3_CH05_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH05_DST3_CH05_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH05_DST4_OFFSET                                    0x00000050
#define CPH_SSASW_DSP_CH05_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH05_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH05_DST4_CH05_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH05_DST4_CH05_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH05_DST4_CH05_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH05_DST4_CH05_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH06_SRC_OFFSET                                     0x00000058
#define CPH_SSASW_DSP_CH06_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH06_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH06_SRC_CH06_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH06_SRC_CH06_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH06_SRC_CH06_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH06_SRC_CH06_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH06_SRC_CH06_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH06_SRC_CH06_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH06_SRC_CH06_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH06_SRC_CH06_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH06_DST1_OFFSET                                    0x0000005A
#define CPH_SSASW_DSP_CH06_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH06_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH06_DST1_CH06_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH06_DST1_CH06_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH06_DST1_CH06_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH06_DST1_CH06_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH06_DST2_OFFSET                                    0x0000005C
#define CPH_SSASW_DSP_CH06_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH06_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH06_DST2_CH06_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH06_DST2_CH06_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH06_DST2_CH06_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH06_DST2_CH06_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH06_DST3_OFFSET                                    0x0000005E
#define CPH_SSASW_DSP_CH06_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH06_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH06_DST3_CH06_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH06_DST3_CH06_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH06_DST3_CH06_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH06_DST3_CH06_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH06_DST4_OFFSET                                    0x00000060
#define CPH_SSASW_DSP_CH06_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH06_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH06_DST4_CH06_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH06_DST4_CH06_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH06_DST4_CH06_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH06_DST4_CH06_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH07_SRC_OFFSET                                     0x00000068
#define CPH_SSASW_DSP_CH07_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH07_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH07_SRC_CH07_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH07_SRC_CH07_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH07_SRC_CH07_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH07_SRC_CH07_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH07_SRC_CH07_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH07_SRC_CH07_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH07_SRC_CH07_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH07_SRC_CH07_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH07_DST1_OFFSET                                    0x0000006A
#define CPH_SSASW_DSP_CH07_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH07_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH07_DST1_CH07_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH07_DST1_CH07_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH07_DST1_CH07_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH07_DST1_CH07_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH07_DST2_OFFSET                                    0x0000006C
#define CPH_SSASW_DSP_CH07_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH07_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH07_DST2_CH07_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH07_DST2_CH07_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH07_DST2_CH07_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH07_DST2_CH07_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH07_DST3_OFFSET                                    0x0000006E
#define CPH_SSASW_DSP_CH07_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH07_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH07_DST3_CH07_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH07_DST3_CH07_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH07_DST3_CH07_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH07_DST3_CH07_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH07_DST4_OFFSET                                    0x00000070
#define CPH_SSASW_DSP_CH07_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH07_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH07_DST4_CH07_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH07_DST4_CH07_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH07_DST4_CH07_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH07_DST4_CH07_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH08_SRC_OFFSET                                     0x00000078
#define CPH_SSASW_DSP_CH08_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH08_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH08_SRC_CH08_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH08_SRC_CH08_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH08_SRC_CH08_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH08_SRC_CH08_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH08_SRC_CH08_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH08_SRC_CH08_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH08_SRC_CH08_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH08_SRC_CH08_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH08_DST1_OFFSET                                    0x0000007A
#define CPH_SSASW_DSP_CH08_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH08_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH08_DST1_CH08_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH08_DST1_CH08_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH08_DST1_CH08_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH08_DST1_CH08_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH08_DST2_OFFSET                                    0x0000007C
#define CPH_SSASW_DSP_CH08_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH08_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH08_DST2_CH08_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH08_DST2_CH08_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH08_DST2_CH08_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH08_DST2_CH08_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH08_DST3_OFFSET                                    0x0000007E
#define CPH_SSASW_DSP_CH08_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH08_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH08_DST3_CH08_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH08_DST3_CH08_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH08_DST3_CH08_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH08_DST3_CH08_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH08_DST4_OFFSET                                    0x00000080
#define CPH_SSASW_DSP_CH08_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH08_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH08_DST4_CH08_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH08_DST4_CH08_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH08_DST4_CH08_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH08_DST4_CH08_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH09_SRC_OFFSET                                     0x00000088
#define CPH_SSASW_DSP_CH09_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH09_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH09_SRC_CH09_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH09_SRC_CH09_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH09_SRC_CH09_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH09_SRC_CH09_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH09_SRC_CH09_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH09_SRC_CH09_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH09_SRC_CH09_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH09_SRC_CH09_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH09_DST1_OFFSET                                    0x0000008A
#define CPH_SSASW_DSP_CH09_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH09_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH09_DST1_CH09_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH09_DST1_CH09_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH09_DST1_CH09_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH09_DST1_CH09_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH09_DST2_OFFSET                                    0x0000008C
#define CPH_SSASW_DSP_CH09_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH09_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH09_DST2_CH09_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH09_DST2_CH09_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH09_DST2_CH09_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH09_DST2_CH09_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH09_DST3_OFFSET                                    0x0000008E
#define CPH_SSASW_DSP_CH09_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH09_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH09_DST3_CH09_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH09_DST3_CH09_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH09_DST3_CH09_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH09_DST3_CH09_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH09_DST4_OFFSET                                    0x00000090
#define CPH_SSASW_DSP_CH09_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH09_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH09_DST4_CH09_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH09_DST4_CH09_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH09_DST4_CH09_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH09_DST4_CH09_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH10_SRC_OFFSET                                     0x00000098
#define CPH_SSASW_DSP_CH10_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH10_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH10_SRC_CH10_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH10_SRC_CH10_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH10_SRC_CH10_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH10_SRC_CH10_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH10_SRC_CH10_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH10_SRC_CH10_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH10_SRC_CH10_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH10_SRC_CH10_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH10_DST1_OFFSET                                    0x0000009A
#define CPH_SSASW_DSP_CH10_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH10_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH10_DST1_CH10_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH10_DST1_CH10_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH10_DST1_CH10_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH10_DST1_CH10_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH10_DST2_OFFSET                                    0x0000009C
#define CPH_SSASW_DSP_CH10_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH10_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH10_DST2_CH10_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH10_DST2_CH10_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH10_DST2_CH10_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH10_DST2_CH10_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH10_DST3_OFFSET                                    0x0000009E
#define CPH_SSASW_DSP_CH10_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH10_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH10_DST3_CH10_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH10_DST3_CH10_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH10_DST3_CH10_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH10_DST3_CH10_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH10_DST4_OFFSET                                    0x000000A0
#define CPH_SSASW_DSP_CH10_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH10_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH10_DST4_CH10_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH10_DST4_CH10_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH10_DST4_CH10_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH10_DST4_CH10_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH11_SRC_OFFSET                                     0x000000A8
#define CPH_SSASW_DSP_CH11_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH11_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH11_SRC_CH11_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH11_SRC_CH11_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH11_SRC_CH11_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH11_SRC_CH11_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH11_SRC_CH11_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH11_SRC_CH11_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH11_SRC_CH11_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH11_SRC_CH11_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH11_DST1_OFFSET                                    0x000000AA
#define CPH_SSASW_DSP_CH11_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH11_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH11_DST1_CH11_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH11_DST1_CH11_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH11_DST1_CH11_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH11_DST1_CH11_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH11_DST2_OFFSET                                    0x000000AC
#define CPH_SSASW_DSP_CH11_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH11_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH11_DST2_CH11_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH11_DST2_CH11_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH11_DST2_CH11_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH11_DST2_CH11_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH11_DST3_OFFSET                                    0x000000AE
#define CPH_SSASW_DSP_CH11_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH11_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH11_DST3_CH11_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH11_DST3_CH11_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH11_DST3_CH11_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH11_DST3_CH11_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH11_DST4_OFFSET                                    0x000000B0
#define CPH_SSASW_DSP_CH11_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH11_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH11_DST4_CH11_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH11_DST4_CH11_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH11_DST4_CH11_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH11_DST4_CH11_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH12_SRC_OFFSET                                     0x000000B8
#define CPH_SSASW_DSP_CH12_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH12_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH12_SRC_CH12_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH12_SRC_CH12_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH12_SRC_CH12_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH12_SRC_CH12_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH12_SRC_CH12_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH12_SRC_CH12_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH12_SRC_CH12_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH12_SRC_CH12_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH12_DST1_OFFSET                                    0x000000BA
#define CPH_SSASW_DSP_CH12_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH12_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH12_DST1_CH12_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH12_DST1_CH12_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH12_DST1_CH12_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH12_DST1_CH12_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH12_DST2_OFFSET                                    0x000000BC
#define CPH_SSASW_DSP_CH12_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH12_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH12_DST2_CH12_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH12_DST2_CH12_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH12_DST2_CH12_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH12_DST2_CH12_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH12_DST3_OFFSET                                    0x000000BE
#define CPH_SSASW_DSP_CH12_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH12_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH12_DST3_CH12_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH12_DST3_CH12_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH12_DST3_CH12_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH12_DST3_CH12_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH12_DST4_OFFSET                                    0x000000C0
#define CPH_SSASW_DSP_CH12_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH12_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH12_DST4_CH12_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH12_DST4_CH12_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH12_DST4_CH12_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH12_DST4_CH12_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH13_SRC_OFFSET                                     0x000000C8
#define CPH_SSASW_DSP_CH13_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH13_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH13_SRC_CH13_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH13_SRC_CH13_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH13_SRC_CH13_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH13_SRC_CH13_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH13_SRC_CH13_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH13_SRC_CH13_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH13_SRC_CH13_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH13_SRC_CH13_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH13_DST1_OFFSET                                    0x000000CA
#define CPH_SSASW_DSP_CH13_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH13_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH13_DST1_CH13_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH13_DST1_CH13_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH13_DST1_CH13_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH13_DST1_CH13_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH13_DST2_OFFSET                                    0x000000CC
#define CPH_SSASW_DSP_CH13_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH13_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH13_DST2_CH13_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH13_DST2_CH13_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH13_DST2_CH13_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH13_DST2_CH13_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH13_DST3_OFFSET                                    0x000000CE
#define CPH_SSASW_DSP_CH13_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH13_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH13_DST3_CH13_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH13_DST3_CH13_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH13_DST3_CH13_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH13_DST3_CH13_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH13_DST4_OFFSET                                    0x000000D0
#define CPH_SSASW_DSP_CH13_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH13_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH13_DST4_CH13_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH13_DST4_CH13_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH13_DST4_CH13_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH13_DST4_CH13_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH14_SRC_OFFSET                                     0x000000D8
#define CPH_SSASW_DSP_CH14_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH14_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH14_SRC_CH14_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH14_SRC_CH14_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH14_SRC_CH14_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH14_SRC_CH14_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH14_SRC_CH14_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH14_SRC_CH14_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH14_SRC_CH14_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH14_SRC_CH14_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH14_DST1_OFFSET                                    0x000000DA
#define CPH_SSASW_DSP_CH14_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH14_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH14_DST1_CH14_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH14_DST1_CH14_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH14_DST1_CH14_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH14_DST1_CH14_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH14_DST2_OFFSET                                    0x000000DC
#define CPH_SSASW_DSP_CH14_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH14_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH14_DST2_CH14_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH14_DST2_CH14_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH14_DST2_CH14_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH14_DST2_CH14_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH14_DST3_OFFSET                                    0x000000DE
#define CPH_SSASW_DSP_CH14_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH14_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH14_DST3_CH14_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH14_DST3_CH14_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH14_DST3_CH14_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH14_DST3_CH14_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH14_DST4_OFFSET                                    0x000000E0
#define CPH_SSASW_DSP_CH14_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH14_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH14_DST4_CH14_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH14_DST4_CH14_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH14_DST4_CH14_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH14_DST4_CH14_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH15_SRC_OFFSET                                     0x000000E8
#define CPH_SSASW_DSP_CH15_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH15_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH15_SRC_CH15_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH15_SRC_CH15_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH15_SRC_CH15_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH15_SRC_CH15_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH15_SRC_CH15_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH15_SRC_CH15_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH15_SRC_CH15_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH15_SRC_CH15_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH15_DST1_OFFSET                                    0x000000EA
#define CPH_SSASW_DSP_CH15_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH15_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH15_DST1_CH15_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH15_DST1_CH15_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH15_DST1_CH15_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH15_DST1_CH15_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH15_DST2_OFFSET                                    0x000000EC
#define CPH_SSASW_DSP_CH15_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH15_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH15_DST2_CH15_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH15_DST2_CH15_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH15_DST2_CH15_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH15_DST2_CH15_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH15_DST3_OFFSET                                    0x000000EE
#define CPH_SSASW_DSP_CH15_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH15_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH15_DST3_CH15_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH15_DST3_CH15_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH15_DST3_CH15_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH15_DST3_CH15_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH15_DST4_OFFSET                                    0x000000F0
#define CPH_SSASW_DSP_CH15_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH15_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH15_DST4_CH15_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH15_DST4_CH15_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH15_DST4_CH15_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH15_DST4_CH15_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH16_SRC_OFFSET                                     0x000000F8
#define CPH_SSASW_DSP_CH16_SRC_TYPE                                       UInt32
#define CPH_SSASW_DSP_CH16_SRC_RESERVED_MASK                              0x3F800000
#define    CPH_SSASW_DSP_CH16_SRC_CH16_SRC_EN_SHIFT                       31
#define    CPH_SSASW_DSP_CH16_SRC_CH16_SRC_EN_MASK                        0x80000000
#define    CPH_SSASW_DSP_CH16_SRC_CH16_TWO_WORDS_SHIFT                    30
#define    CPH_SSASW_DSP_CH16_SRC_CH16_TWO_WORDS_MASK                     0x40000000
#define    CPH_SSASW_DSP_CH16_SRC_CH16_TRIGGER_SEL_SHIFT                  16
#define    CPH_SSASW_DSP_CH16_SRC_CH16_TRIGGER_SEL_MASK                   0x007F0000
#define    CPH_SSASW_DSP_CH16_SRC_CH16_SRC_ADDR_SHIFT                     0
#define    CPH_SSASW_DSP_CH16_SRC_CH16_SRC_ADDR_MASK                      0x0000FFFF

#define CPH_SSASW_DSP_CH16_DST1_OFFSET                                    0x000000FA
#define CPH_SSASW_DSP_CH16_DST1_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH16_DST1_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH16_DST1_CH16_DST1_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH16_DST1_CH16_DST1_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH16_DST1_CH16_DST1_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH16_DST1_CH16_DST1_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH16_DST2_OFFSET                                    0x000000FC
#define CPH_SSASW_DSP_CH16_DST2_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH16_DST2_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH16_DST2_CH16_DST2_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH16_DST2_CH16_DST2_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH16_DST2_CH16_DST2_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH16_DST2_CH16_DST2_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH16_DST3_OFFSET                                    0x000000FE
#define CPH_SSASW_DSP_CH16_DST3_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH16_DST3_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH16_DST3_CH16_DST3_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH16_DST3_CH16_DST3_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH16_DST3_CH16_DST3_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH16_DST3_CH16_DST3_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_CH16_DST4_OFFSET                                    0x00000100
#define CPH_SSASW_DSP_CH16_DST4_TYPE                                      UInt32
#define CPH_SSASW_DSP_CH16_DST4_RESERVED_MASK                             0x7FFF0000
#define    CPH_SSASW_DSP_CH16_DST4_CH16_DST4_EN_SHIFT                     31
#define    CPH_SSASW_DSP_CH16_DST4_CH16_DST4_EN_MASK                      0x80000000
#define    CPH_SSASW_DSP_CH16_DST4_CH16_DST4_ADDR_SHIFT                   0
#define    CPH_SSASW_DSP_CH16_DST4_CH16_DST4_ADDR_MASK                    0x0000FFFF

#define CPH_SSASW_DSP_PREADY_MAX_TIME_OFFSET                              0x000003FC
#define CPH_SSASW_DSP_PREADY_MAX_TIME_TYPE                                UInt32
#define CPH_SSASW_DSP_PREADY_MAX_TIME_RESERVED_MASK                       0xFFFFFF00
#define    CPH_SSASW_DSP_PREADY_MAX_TIME_PREADY_MAX_TIME_SHIFT            0
#define    CPH_SSASW_DSP_PREADY_MAX_TIME_PREADY_MAX_TIME_MASK             0x000000FF

#define CPH_SSASW_DSP_SSASW_ERR_OFFSET                                    0x000003FE
#define CPH_SSASW_DSP_SSASW_ERR_TYPE                                      UInt32
#define CPH_SSASW_DSP_SSASW_ERR_RESERVED_MASK                             0x1FFFFFFF
#define    CPH_SSASW_DSP_SSASW_ERR_APB_PREADY_ERR_SHIFT                   31
#define    CPH_SSASW_DSP_SSASW_ERR_APB_PREADY_ERR_MASK                    0x80000000
#define    CPH_SSASW_DSP_SSASW_ERR_APB_PSLVERR_ERR_SHIFT                  30
#define    CPH_SSASW_DSP_SSASW_ERR_APB_PSLVERR_ERR_MASK                   0x40000000
#define    CPH_SSASW_DSP_SSASW_ERR_CH_DST_ERR_SHIFT                       29
#define    CPH_SSASW_DSP_SSASW_ERR_CH_DST_ERR_MASK                        0x20000000

#endif /* __BRCM_RDB_CPH_SSASW_H__ */



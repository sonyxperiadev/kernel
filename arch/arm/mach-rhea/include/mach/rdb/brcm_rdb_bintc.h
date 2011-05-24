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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_BINTC_H__
#define __BRCM_RDB_BINTC_H__

#define BINTC_DSP_ISR0_OFFSET                                             0x00000000
#define BINTC_DSP_ISR0_TYPE                                               UInt32
#define BINTC_DSP_ISR0_RESERVED_MASK                                      0x00000000
#define    BINTC_DSP_ISR0_ISR0N_SHIFT                                     0
#define    BINTC_DSP_ISR0_ISR0N_MASK                                      0xFFFFFFFF

#define BINTC_DSP_ICR0_OFFSET                                             0x00000002
#define BINTC_DSP_ICR0_TYPE                                               UInt32
#define BINTC_DSP_ICR0_RESERVED_MASK                                      0x00000000
#define    BINTC_DSP_ICR0_ICR0N_SHIFT                                     0
#define    BINTC_DSP_ICR0_ICR0N_MASK                                      0xFFFFFFFF

#define BINTC_DSP_ISR1_OFFSET                                             0x00000004
#define BINTC_DSP_ISR1_TYPE                                               UInt32
#define BINTC_DSP_ISR1_RESERVED_MASK                                      0xFF000000
#define    BINTC_DSP_ISR1_ISR1N_SHIFT                                     0
#define    BINTC_DSP_ISR1_ISR1N_MASK                                      0x00FFFFFF

#define BINTC_DSP_ICR1_OFFSET                                             0x00000006
#define BINTC_DSP_ICR1_TYPE                                               UInt32
#define BINTC_DSP_ICR1_RESERVED_MASK                                      0xFF000000
#define    BINTC_DSP_ICR1_ICR1N_SHIFT                                     0
#define    BINTC_DSP_ICR1_ICR1N_MASK                                      0x00FFFFFF

#define BINTC_DSP_ISWIR0_OFFSET                                           0x00000010
#define BINTC_DSP_ISWIR0_TYPE                                             UInt32
#define BINTC_DSP_ISWIR0_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_ISWIR0_ISWIR0N_SHIFT                                 0
#define    BINTC_DSP_ISWIR0_ISWIR0N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_ISWIR0_CLR_OFFSET                                       0x00000012
#define BINTC_DSP_ISWIR0_CLR_TYPE                                         UInt32
#define BINTC_DSP_ISWIR0_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_ISWIR0_CLR_ISWIR0N_SHIFT                             0
#define    BINTC_DSP_ISWIR0_CLR_ISWIR0N_MASK                              0xFFFFFFFF

#define BINTC_DSP_ISWIR1_OFFSET                                           0x00000018
#define BINTC_DSP_ISWIR1_TYPE                                             UInt32
#define BINTC_DSP_ISWIR1_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_ISWIR1_ISWIR1N_SHIFT                                 0
#define    BINTC_DSP_ISWIR1_ISWIR1N_MASK                                  0x00FFFFFF

#define BINTC_DSP_ISWIR1_CLR_OFFSET                                       0x0000001A
#define BINTC_DSP_ISWIR1_CLR_TYPE                                         UInt32
#define BINTC_DSP_ISWIR1_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_ISWIR1_CLR_ISWIR1N_SHIFT                             0
#define    BINTC_DSP_ISWIR1_CLR_ISWIR1N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_0_OFFSET                                           0x00000020
#define BINTC_DSP_IMR0_0_TYPE                                             UInt32
#define BINTC_DSP_IMR0_0_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_0_IMR0_0N_SHIFT                                 0
#define    BINTC_DSP_IMR0_0_IMR0_0N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_0_SET_OFFSET                                       0x00000022
#define BINTC_DSP_IMR0_0_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_0_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_0_SET_IMR0_0N_SHIFT                             0
#define    BINTC_DSP_IMR0_0_SET_IMR0_0N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_0_CLR_OFFSET                                       0x00000024
#define BINTC_DSP_IMR0_0_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_0_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_0_CLR_IMR0_0N_SHIFT                             0
#define    BINTC_DSP_IMR0_0_CLR_IMR0_0N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_0_OFFSET                                           0x00000026
#define BINTC_DSP_IMR1_0_TYPE                                             UInt32
#define BINTC_DSP_IMR1_0_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_0_IMR1_0N_SHIFT                                 0
#define    BINTC_DSP_IMR1_0_IMR1_0N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_0_SET_OFFSET                                       0x00000028
#define BINTC_DSP_IMR1_0_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_0_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_0_SET_IMR1_0N_SHIFT                             0
#define    BINTC_DSP_IMR1_0_SET_IMR1_0N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_0_CLR_OFFSET                                       0x0000002A
#define BINTC_DSP_IMR1_0_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_0_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_0_CLR_IMR1_0N_SHIFT                             0
#define    BINTC_DSP_IMR1_0_CLR_IMR1_0N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_0_OFFSET                                          0x0000002C
#define BINTC_DSP_IMSR0_0_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_0_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_0_IMSR0_0N_SHIFT                               0
#define    BINTC_DSP_IMSR0_0_IMSR0_0N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_0_OFFSET                                          0x0000002E
#define BINTC_DSP_IMSR1_0_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_0_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_0_IMSR1_0N_SHIFT                               0
#define    BINTC_DSP_IMSR1_0_IMSR1_0N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_1_OFFSET                                           0x00000030
#define BINTC_DSP_IMR0_1_TYPE                                             UInt32
#define BINTC_DSP_IMR0_1_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_1_IMR0_1N_SHIFT                                 0
#define    BINTC_DSP_IMR0_1_IMR0_1N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_1_SET_OFFSET                                       0x00000032
#define BINTC_DSP_IMR0_1_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_1_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_1_SET_IMR0_1N_SHIFT                             0
#define    BINTC_DSP_IMR0_1_SET_IMR0_1N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_1_CLR_OFFSET                                       0x00000034
#define BINTC_DSP_IMR0_1_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_1_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_1_CLR_IMR0_1N_SHIFT                             0
#define    BINTC_DSP_IMR0_1_CLR_IMR0_1N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_1_OFFSET                                           0x00000036
#define BINTC_DSP_IMR1_1_TYPE                                             UInt32
#define BINTC_DSP_IMR1_1_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_1_IMR1_1N_SHIFT                                 0
#define    BINTC_DSP_IMR1_1_IMR1_1N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_1_SET_OFFSET                                       0x00000038
#define BINTC_DSP_IMR1_1_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_1_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_1_SET_IMR1_1N_SHIFT                             0
#define    BINTC_DSP_IMR1_1_SET_IMR1_1N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_1_CLR_OFFSET                                       0x0000003A
#define BINTC_DSP_IMR1_1_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_1_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_1_CLR_IMR1_1N_SHIFT                             0
#define    BINTC_DSP_IMR1_1_CLR_IMR1_1N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_1_OFFSET                                          0x0000003C
#define BINTC_DSP_IMSR0_1_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_1_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_1_IMSR0_1N_SHIFT                               0
#define    BINTC_DSP_IMSR0_1_IMSR0_1N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_1_OFFSET                                          0x0000003E
#define BINTC_DSP_IMSR1_1_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_1_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_1_IMSR1_1N_SHIFT                               0
#define    BINTC_DSP_IMSR1_1_IMSR1_1N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_2_OFFSET                                           0x00000040
#define BINTC_DSP_IMR0_2_TYPE                                             UInt32
#define BINTC_DSP_IMR0_2_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_2_IMR0_2N_SHIFT                                 0
#define    BINTC_DSP_IMR0_2_IMR0_2N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_2_SET_OFFSET                                       0x00000042
#define BINTC_DSP_IMR0_2_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_2_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_2_SET_IMR0_2N_SHIFT                             0
#define    BINTC_DSP_IMR0_2_SET_IMR0_2N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_2_CLR_OFFSET                                       0x00000044
#define BINTC_DSP_IMR0_2_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_2_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_2_CLR_IMR0_2N_SHIFT                             0
#define    BINTC_DSP_IMR0_2_CLR_IMR0_2N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_2_OFFSET                                           0x00000046
#define BINTC_DSP_IMR1_2_TYPE                                             UInt32
#define BINTC_DSP_IMR1_2_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_2_IMR1_2N_SHIFT                                 0
#define    BINTC_DSP_IMR1_2_IMR1_2N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_2_SET_OFFSET                                       0x00000048
#define BINTC_DSP_IMR1_2_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_2_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_2_SET_IMR1_2N_SHIFT                             0
#define    BINTC_DSP_IMR1_2_SET_IMR1_2N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_2_CLR_OFFSET                                       0x0000004A
#define BINTC_DSP_IMR1_2_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_2_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_2_CLR_IMR1_2N_SHIFT                             0
#define    BINTC_DSP_IMR1_2_CLR_IMR1_2N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_2_OFFSET                                          0x0000004C
#define BINTC_DSP_IMSR0_2_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_2_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_2_IMSR0_2N_SHIFT                               0
#define    BINTC_DSP_IMSR0_2_IMSR0_2N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_2_OFFSET                                          0x0000004E
#define BINTC_DSP_IMSR1_2_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_2_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_2_IMSR1_2N_SHIFT                               0
#define    BINTC_DSP_IMSR1_2_IMSR1_2N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_3_OFFSET                                           0x00000050
#define BINTC_DSP_IMR0_3_TYPE                                             UInt32
#define BINTC_DSP_IMR0_3_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_3_IMR0_3N_SHIFT                                 0
#define    BINTC_DSP_IMR0_3_IMR0_3N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_3_SET_OFFSET                                       0x00000052
#define BINTC_DSP_IMR0_3_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_3_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_3_SET_IMR0_3N_SHIFT                             0
#define    BINTC_DSP_IMR0_3_SET_IMR0_3N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_3_CLR_OFFSET                                       0x00000054
#define BINTC_DSP_IMR0_3_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_3_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_3_CLR_IMR0_3N_SHIFT                             0
#define    BINTC_DSP_IMR0_3_CLR_IMR0_3N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_3_OFFSET                                           0x00000056
#define BINTC_DSP_IMR1_3_TYPE                                             UInt32
#define BINTC_DSP_IMR1_3_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_3_IMR1_3N_SHIFT                                 0
#define    BINTC_DSP_IMR1_3_IMR1_3N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_3_SET_OFFSET                                       0x00000058
#define BINTC_DSP_IMR1_3_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_3_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_3_SET_IMR1_3N_SHIFT                             0
#define    BINTC_DSP_IMR1_3_SET_IMR1_3N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_3_CLR_OFFSET                                       0x0000005A
#define BINTC_DSP_IMR1_3_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_3_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_3_CLR_IMR1_3N_SHIFT                             0
#define    BINTC_DSP_IMR1_3_CLR_IMR1_3N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_3_OFFSET                                          0x0000005C
#define BINTC_DSP_IMSR0_3_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_3_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_3_IMSR0_3N_SHIFT                               0
#define    BINTC_DSP_IMSR0_3_IMSR0_3N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_3_OFFSET                                          0x0000005E
#define BINTC_DSP_IMSR1_3_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_3_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_3_IMSR1_3N_SHIFT                               0
#define    BINTC_DSP_IMSR1_3_IMSR1_3N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_4_OFFSET                                           0x00000060
#define BINTC_DSP_IMR0_4_TYPE                                             UInt32
#define BINTC_DSP_IMR0_4_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_4_IMR0_4N_SHIFT                                 0
#define    BINTC_DSP_IMR0_4_IMR0_4N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_4_SET_OFFSET                                       0x00000062
#define BINTC_DSP_IMR0_4_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_4_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_4_SET_IMR0_4N_SHIFT                             0
#define    BINTC_DSP_IMR0_4_SET_IMR0_4N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_4_CLR_OFFSET                                       0x00000064
#define BINTC_DSP_IMR0_4_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_4_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_4_CLR_IMR0_4N_SHIFT                             0
#define    BINTC_DSP_IMR0_4_CLR_IMR0_4N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_4_OFFSET                                           0x00000066
#define BINTC_DSP_IMR1_4_TYPE                                             UInt32
#define BINTC_DSP_IMR1_4_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_4_IMR1_4N_SHIFT                                 0
#define    BINTC_DSP_IMR1_4_IMR1_4N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_4_SET_OFFSET                                       0x00000068
#define BINTC_DSP_IMR1_4_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_4_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_4_SET_IMR1_4N_SHIFT                             0
#define    BINTC_DSP_IMR1_4_SET_IMR1_4N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_4_CLR_OFFSET                                       0x0000006A
#define BINTC_DSP_IMR1_4_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_4_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_4_CLR_IMR1_4N_SHIFT                             0
#define    BINTC_DSP_IMR1_4_CLR_IMR1_4N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_4_OFFSET                                          0x0000006C
#define BINTC_DSP_IMSR0_4_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_4_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_4_IMSR0_4N_SHIFT                               0
#define    BINTC_DSP_IMSR0_4_IMSR0_4N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_4_OFFSET                                          0x0000006E
#define BINTC_DSP_IMSR1_4_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_4_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_4_IMSR1_4N_SHIFT                               0
#define    BINTC_DSP_IMSR1_4_IMSR1_4N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_5_OFFSET                                           0x00000070
#define BINTC_DSP_IMR0_5_TYPE                                             UInt32
#define BINTC_DSP_IMR0_5_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_5_IMR0_5N_SHIFT                                 0
#define    BINTC_DSP_IMR0_5_IMR0_5N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_5_SET_OFFSET                                       0x00000072
#define BINTC_DSP_IMR0_5_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_5_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_5_SET_IMR0_5N_SHIFT                             0
#define    BINTC_DSP_IMR0_5_SET_IMR0_5N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_5_CLR_OFFSET                                       0x00000074
#define BINTC_DSP_IMR0_5_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_5_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_5_CLR_IMR0_5N_SHIFT                             0
#define    BINTC_DSP_IMR0_5_CLR_IMR0_5N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_5_OFFSET                                           0x00000076
#define BINTC_DSP_IMR1_5_TYPE                                             UInt32
#define BINTC_DSP_IMR1_5_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_5_IMR1_5N_SHIFT                                 0
#define    BINTC_DSP_IMR1_5_IMR1_5N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_5_SET_OFFSET                                       0x00000078
#define BINTC_DSP_IMR1_5_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_5_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_5_SET_IMR1_5N_SHIFT                             0
#define    BINTC_DSP_IMR1_5_SET_IMR1_5N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_5_CLR_OFFSET                                       0x0000007A
#define BINTC_DSP_IMR1_5_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_5_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_5_CLR_IMR1_5N_SHIFT                             0
#define    BINTC_DSP_IMR1_5_CLR_IMR1_5N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_5_OFFSET                                          0x0000007C
#define BINTC_DSP_IMSR0_5_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_5_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_5_IMSR0_5N_SHIFT                               0
#define    BINTC_DSP_IMSR0_5_IMSR0_5N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_5_OFFSET                                          0x0000007E
#define BINTC_DSP_IMSR1_5_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_5_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_5_IMSR1_5N_SHIFT                               0
#define    BINTC_DSP_IMSR1_5_IMSR1_5N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_6_OFFSET                                           0x00000080
#define BINTC_DSP_IMR0_6_TYPE                                             UInt32
#define BINTC_DSP_IMR0_6_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_6_IMR0_6N_SHIFT                                 0
#define    BINTC_DSP_IMR0_6_IMR0_6N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_6_SET_OFFSET                                       0x00000082
#define BINTC_DSP_IMR0_6_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_6_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_6_SET_IMR0_6N_SHIFT                             0
#define    BINTC_DSP_IMR0_6_SET_IMR0_6N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_6_CLR_OFFSET                                       0x00000084
#define BINTC_DSP_IMR0_6_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_6_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_6_CLR_IMR0_6N_SHIFT                             0
#define    BINTC_DSP_IMR0_6_CLR_IMR0_6N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_6_OFFSET                                           0x00000086
#define BINTC_DSP_IMR1_6_TYPE                                             UInt32
#define BINTC_DSP_IMR1_6_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_6_IMR1_6N_SHIFT                                 0
#define    BINTC_DSP_IMR1_6_IMR1_6N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_6_SET_OFFSET                                       0x00000088
#define BINTC_DSP_IMR1_6_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_6_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_6_SET_IMR1_6N_SHIFT                             0
#define    BINTC_DSP_IMR1_6_SET_IMR1_6N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_6_CLR_OFFSET                                       0x0000008A
#define BINTC_DSP_IMR1_6_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_6_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_6_CLR_IMR1_6N_SHIFT                             0
#define    BINTC_DSP_IMR1_6_CLR_IMR1_6N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_6_OFFSET                                          0x0000008C
#define BINTC_DSP_IMSR0_6_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_6_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_6_IMSR0_6N_SHIFT                               0
#define    BINTC_DSP_IMSR0_6_IMSR0_6N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_6_OFFSET                                          0x0000008E
#define BINTC_DSP_IMSR1_6_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_6_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_6_IMSR1_6N_SHIFT                               0
#define    BINTC_DSP_IMSR1_6_IMSR1_6N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_7_OFFSET                                           0x00000090
#define BINTC_DSP_IMR0_7_TYPE                                             UInt32
#define BINTC_DSP_IMR0_7_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_7_IMR0_7N_SHIFT                                 0
#define    BINTC_DSP_IMR0_7_IMR0_7N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_7_SET_OFFSET                                       0x00000092
#define BINTC_DSP_IMR0_7_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_7_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_7_SET_IMR0_7N_SHIFT                             0
#define    BINTC_DSP_IMR0_7_SET_IMR0_7N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_7_CLR_OFFSET                                       0x00000094
#define BINTC_DSP_IMR0_7_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_7_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_7_CLR_IMR0_7N_SHIFT                             0
#define    BINTC_DSP_IMR0_7_CLR_IMR0_7N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_7_OFFSET                                           0x00000096
#define BINTC_DSP_IMR1_7_TYPE                                             UInt32
#define BINTC_DSP_IMR1_7_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_7_IMR1_7N_SHIFT                                 0
#define    BINTC_DSP_IMR1_7_IMR1_7N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_7_SET_OFFSET                                       0x00000098
#define BINTC_DSP_IMR1_7_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_7_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_7_SET_IMR1_7N_SHIFT                             0
#define    BINTC_DSP_IMR1_7_SET_IMR1_7N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_7_CLR_OFFSET                                       0x0000009A
#define BINTC_DSP_IMR1_7_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_7_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_7_CLR_IMR1_7N_SHIFT                             0
#define    BINTC_DSP_IMR1_7_CLR_IMR1_7N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_7_OFFSET                                          0x0000009C
#define BINTC_DSP_IMSR0_7_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_7_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_7_IMSR0_7N_SHIFT                               0
#define    BINTC_DSP_IMSR0_7_IMSR0_7N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_7_OFFSET                                          0x0000009E
#define BINTC_DSP_IMSR1_7_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_7_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_7_IMSR1_7N_SHIFT                               0
#define    BINTC_DSP_IMSR1_7_IMSR1_7N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_8_OFFSET                                           0x000000A0
#define BINTC_DSP_IMR0_8_TYPE                                             UInt32
#define BINTC_DSP_IMR0_8_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_8_IMR0_8N_SHIFT                                 0
#define    BINTC_DSP_IMR0_8_IMR0_8N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_8_SET_OFFSET                                       0x000000A2
#define BINTC_DSP_IMR0_8_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_8_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_8_SET_IMR0_8N_SHIFT                             0
#define    BINTC_DSP_IMR0_8_SET_IMR0_8N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_8_CLR_OFFSET                                       0x000000A4
#define BINTC_DSP_IMR0_8_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_8_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_8_CLR_IMR0_8N_SHIFT                             0
#define    BINTC_DSP_IMR0_8_CLR_IMR0_8N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_8_OFFSET                                           0x000000A6
#define BINTC_DSP_IMR1_8_TYPE                                             UInt32
#define BINTC_DSP_IMR1_8_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_8_IMR1_8N_SHIFT                                 0
#define    BINTC_DSP_IMR1_8_IMR1_8N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_8_SET_OFFSET                                       0x000000A8
#define BINTC_DSP_IMR1_8_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_8_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_8_SET_IMR1_8N_SHIFT                             0
#define    BINTC_DSP_IMR1_8_SET_IMR1_8N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_8_CLR_OFFSET                                       0x000000AA
#define BINTC_DSP_IMR1_8_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_8_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_8_CLR_IMR1_8N_SHIFT                             0
#define    BINTC_DSP_IMR1_8_CLR_IMR1_8N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_8_OFFSET                                          0x000000AC
#define BINTC_DSP_IMSR0_8_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_8_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_8_IMSR0_8N_SHIFT                               0
#define    BINTC_DSP_IMSR0_8_IMSR0_8N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_8_OFFSET                                          0x000000AE
#define BINTC_DSP_IMSR1_8_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_8_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_8_IMSR1_8N_SHIFT                               0
#define    BINTC_DSP_IMSR1_8_IMSR1_8N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_9_OFFSET                                           0x000000B0
#define BINTC_DSP_IMR0_9_TYPE                                             UInt32
#define BINTC_DSP_IMR0_9_RESERVED_MASK                                    0x00000000
#define    BINTC_DSP_IMR0_9_IMR0_9N_SHIFT                                 0
#define    BINTC_DSP_IMR0_9_IMR0_9N_MASK                                  0xFFFFFFFF

#define BINTC_DSP_IMR0_9_SET_OFFSET                                       0x000000B2
#define BINTC_DSP_IMR0_9_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR0_9_SET_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_9_SET_IMR0_9N_SHIFT                             0
#define    BINTC_DSP_IMR0_9_SET_IMR0_9N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR0_9_CLR_OFFSET                                       0x000000B4
#define BINTC_DSP_IMR0_9_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR0_9_CLR_RESERVED_MASK                                0x00000000
#define    BINTC_DSP_IMR0_9_CLR_IMR0_9N_SHIFT                             0
#define    BINTC_DSP_IMR0_9_CLR_IMR0_9N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMR1_9_OFFSET                                           0x000000B6
#define BINTC_DSP_IMR1_9_TYPE                                             UInt32
#define BINTC_DSP_IMR1_9_RESERVED_MASK                                    0xFF000000
#define    BINTC_DSP_IMR1_9_IMR1_9N_SHIFT                                 0
#define    BINTC_DSP_IMR1_9_IMR1_9N_MASK                                  0x00FFFFFF

#define BINTC_DSP_IMR1_9_SET_OFFSET                                       0x000000B8
#define BINTC_DSP_IMR1_9_SET_TYPE                                         UInt32
#define BINTC_DSP_IMR1_9_SET_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_9_SET_IMR1_9N_SHIFT                             0
#define    BINTC_DSP_IMR1_9_SET_IMR1_9N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR1_9_CLR_OFFSET                                       0x000000BA
#define BINTC_DSP_IMR1_9_CLR_TYPE                                         UInt32
#define BINTC_DSP_IMR1_9_CLR_RESERVED_MASK                                0xFF000000
#define    BINTC_DSP_IMR1_9_CLR_IMR1_9N_SHIFT                             0
#define    BINTC_DSP_IMR1_9_CLR_IMR1_9N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMSR0_9_OFFSET                                          0x000000BC
#define BINTC_DSP_IMSR0_9_TYPE                                            UInt32
#define BINTC_DSP_IMSR0_9_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMSR0_9_IMSR0_9N_SHIFT                               0
#define    BINTC_DSP_IMSR0_9_IMSR0_9N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMSR1_9_OFFSET                                          0x000000BE
#define BINTC_DSP_IMSR1_9_TYPE                                            UInt32
#define BINTC_DSP_IMSR1_9_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMSR1_9_IMSR1_9N_SHIFT                               0
#define    BINTC_DSP_IMSR1_9_IMSR1_9N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR0_10_OFFSET                                          0x000000C0
#define BINTC_DSP_IMR0_10_TYPE                                            UInt32
#define BINTC_DSP_IMR0_10_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_10_IMR0_10N_SHIFT                               0
#define    BINTC_DSP_IMR0_10_IMR0_10N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_10_SET_OFFSET                                      0x000000C2
#define BINTC_DSP_IMR0_10_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_10_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_10_SET_IMR0_10N_SHIFT                           0
#define    BINTC_DSP_IMR0_10_SET_IMR0_10N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_10_CLR_OFFSET                                      0x000000C4
#define BINTC_DSP_IMR0_10_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_10_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_10_CLR_IMR0_10N_SHIFT                           0
#define    BINTC_DSP_IMR0_10_CLR_IMR0_10N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_10_OFFSET                                          0x000000C6
#define BINTC_DSP_IMR1_10_TYPE                                            UInt32
#define BINTC_DSP_IMR1_10_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_10_IMR1_10N_SHIFT                               0
#define    BINTC_DSP_IMR1_10_IMR1_10N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_10_SET_OFFSET                                      0x000000C8
#define BINTC_DSP_IMR1_10_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_10_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_10_SET_IMR1_10N_SHIFT                           0
#define    BINTC_DSP_IMR1_10_SET_IMR1_10N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_10_CLR_OFFSET                                      0x000000CA
#define BINTC_DSP_IMR1_10_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_10_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_10_CLR_IMR1_10N_SHIFT                           0
#define    BINTC_DSP_IMR1_10_CLR_IMR1_10N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_10_OFFSET                                         0x000000CC
#define BINTC_DSP_IMSR0_10_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_10_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_10_IMSR0_10N_SHIFT                             0
#define    BINTC_DSP_IMSR0_10_IMSR0_10N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_10_OFFSET                                         0x000000CE
#define BINTC_DSP_IMSR1_10_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_10_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_10_IMSR1_10N_SHIFT                             0
#define    BINTC_DSP_IMSR1_10_IMSR1_10N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_11_OFFSET                                          0x000000D0
#define BINTC_DSP_IMR0_11_TYPE                                            UInt32
#define BINTC_DSP_IMR0_11_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_11_IMR0_11N_SHIFT                               0
#define    BINTC_DSP_IMR0_11_IMR0_11N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_11_SET_OFFSET                                      0x000000D2
#define BINTC_DSP_IMR0_11_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_11_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_11_SET_IMR0_11N_SHIFT                           0
#define    BINTC_DSP_IMR0_11_SET_IMR0_11N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_11_CLR_OFFSET                                      0x000000D4
#define BINTC_DSP_IMR0_11_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_11_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_11_CLR_IMR0_11N_SHIFT                           0
#define    BINTC_DSP_IMR0_11_CLR_IMR0_11N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_11_OFFSET                                          0x000000D6
#define BINTC_DSP_IMR1_11_TYPE                                            UInt32
#define BINTC_DSP_IMR1_11_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_11_IMR1_11N_SHIFT                               0
#define    BINTC_DSP_IMR1_11_IMR1_11N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_11_SET_OFFSET                                      0x000000D8
#define BINTC_DSP_IMR1_11_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_11_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_11_SET_IMR1_11N_SHIFT                           0
#define    BINTC_DSP_IMR1_11_SET_IMR1_11N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_11_CLR_OFFSET                                      0x000000DA
#define BINTC_DSP_IMR1_11_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_11_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_11_CLR_IMR1_11N_SHIFT                           0
#define    BINTC_DSP_IMR1_11_CLR_IMR1_11N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_11_OFFSET                                         0x000000DC
#define BINTC_DSP_IMSR0_11_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_11_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_11_IMSR0_11N_SHIFT                             0
#define    BINTC_DSP_IMSR0_11_IMSR0_11N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_11_OFFSET                                         0x000000DE
#define BINTC_DSP_IMSR1_11_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_11_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_11_IMSR1_11N_SHIFT                             0
#define    BINTC_DSP_IMSR1_11_IMSR1_11N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_12_OFFSET                                          0x000000E0
#define BINTC_DSP_IMR0_12_TYPE                                            UInt32
#define BINTC_DSP_IMR0_12_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_12_IMR0_12N_SHIFT                               0
#define    BINTC_DSP_IMR0_12_IMR0_12N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_12_SET_OFFSET                                      0x000000E2
#define BINTC_DSP_IMR0_12_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_12_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_12_SET_IMR0_12N_SHIFT                           0
#define    BINTC_DSP_IMR0_12_SET_IMR0_12N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_12_CLR_OFFSET                                      0x000000E4
#define BINTC_DSP_IMR0_12_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_12_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_12_CLR_IMR0_12N_SHIFT                           0
#define    BINTC_DSP_IMR0_12_CLR_IMR0_12N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_12_OFFSET                                          0x000000E6
#define BINTC_DSP_IMR1_12_TYPE                                            UInt32
#define BINTC_DSP_IMR1_12_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_12_IMR1_12N_SHIFT                               0
#define    BINTC_DSP_IMR1_12_IMR1_12N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_12_SET_OFFSET                                      0x000000E8
#define BINTC_DSP_IMR1_12_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_12_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_12_SET_IMR1_12N_SHIFT                           0
#define    BINTC_DSP_IMR1_12_SET_IMR1_12N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_12_CLR_OFFSET                                      0x000000EA
#define BINTC_DSP_IMR1_12_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_12_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_12_CLR_IMR1_12N_SHIFT                           0
#define    BINTC_DSP_IMR1_12_CLR_IMR1_12N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_12_OFFSET                                         0x000000EC
#define BINTC_DSP_IMSR0_12_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_12_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_12_IMSR0_12N_SHIFT                             0
#define    BINTC_DSP_IMSR0_12_IMSR0_12N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_12_OFFSET                                         0x000000EE
#define BINTC_DSP_IMSR1_12_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_12_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_12_IMSR1_12N_SHIFT                             0
#define    BINTC_DSP_IMSR1_12_IMSR1_12N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_13_OFFSET                                          0x000000F0
#define BINTC_DSP_IMR0_13_TYPE                                            UInt32
#define BINTC_DSP_IMR0_13_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_13_IMR0_13N_SHIFT                               0
#define    BINTC_DSP_IMR0_13_IMR0_13N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_13_SET_OFFSET                                      0x000000F2
#define BINTC_DSP_IMR0_13_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_13_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_13_SET_IMR0_13N_SHIFT                           0
#define    BINTC_DSP_IMR0_13_SET_IMR0_13N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_13_CLR_OFFSET                                      0x000000F4
#define BINTC_DSP_IMR0_13_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_13_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_13_CLR_IMR0_13N_SHIFT                           0
#define    BINTC_DSP_IMR0_13_CLR_IMR0_13N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_13_OFFSET                                          0x000000F6
#define BINTC_DSP_IMR1_13_TYPE                                            UInt32
#define BINTC_DSP_IMR1_13_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_13_IMR1_13N_SHIFT                               0
#define    BINTC_DSP_IMR1_13_IMR1_13N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_13_SET_OFFSET                                      0x000000F8
#define BINTC_DSP_IMR1_13_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_13_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_13_SET_IMR1_13N_SHIFT                           0
#define    BINTC_DSP_IMR1_13_SET_IMR1_13N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_13_CLR_OFFSET                                      0x000000FA
#define BINTC_DSP_IMR1_13_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_13_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_13_CLR_IMR1_13N_SHIFT                           0
#define    BINTC_DSP_IMR1_13_CLR_IMR1_13N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_13_OFFSET                                         0x000000FC
#define BINTC_DSP_IMSR0_13_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_13_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_13_IMSR0_13N_SHIFT                             0
#define    BINTC_DSP_IMSR0_13_IMSR0_13N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_13_OFFSET                                         0x000000FE
#define BINTC_DSP_IMSR1_13_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_13_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_13_IMSR1_13N_SHIFT                             0
#define    BINTC_DSP_IMSR1_13_IMSR1_13N_MASK                              0x00FFFFFF

#define BINTC_DSP_ISELR0_13_OFFSET                                        0x00000100
#define BINTC_DSP_ISELR0_13_TYPE                                          UInt32
#define BINTC_DSP_ISELR0_13_RESERVED_MASK                                 0x00000000
#define    BINTC_DSP_ISELR0_13_ISELR0_13N_SHIFT                           0
#define    BINTC_DSP_ISELR0_13_ISELR0_13N_MASK                            0xFFFFFFFF

#define BINTC_DSP_ISELR1_13_OFFSET                                        0x00000102
#define BINTC_DSP_ISELR1_13_TYPE                                          UInt32
#define BINTC_DSP_ISELR1_13_RESERVED_MASK                                 0xFF000000
#define    BINTC_DSP_ISELR1_13_ISELR1_13N_SHIFT                           0
#define    BINTC_DSP_ISELR1_13_ISELR1_13N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR0_15_OFFSET                                          0x00000110
#define BINTC_DSP_IMR0_15_TYPE                                            UInt32
#define BINTC_DSP_IMR0_15_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_15_IMR0_15N_SHIFT                               0
#define    BINTC_DSP_IMR0_15_IMR0_15N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_15_SET_OFFSET                                      0x00000112
#define BINTC_DSP_IMR0_15_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_15_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_15_SET_IMR0_15N_SHIFT                           0
#define    BINTC_DSP_IMR0_15_SET_IMR0_15N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_15_CLR_OFFSET                                      0x00000114
#define BINTC_DSP_IMR0_15_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_15_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_15_CLR_IMR0_15N_SHIFT                           0
#define    BINTC_DSP_IMR0_15_CLR_IMR0_15N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_15_OFFSET                                          0x00000116
#define BINTC_DSP_IMR1_15_TYPE                                            UInt32
#define BINTC_DSP_IMR1_15_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_15_IMR1_15N_SHIFT                               0
#define    BINTC_DSP_IMR1_15_IMR1_15N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_15_SET_OFFSET                                      0x00000118
#define BINTC_DSP_IMR1_15_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_15_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_15_SET_IMR1_15N_SHIFT                           0
#define    BINTC_DSP_IMR1_15_SET_IMR1_15N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_15_CLR_OFFSET                                      0x0000011A
#define BINTC_DSP_IMR1_15_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_15_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_15_CLR_IMR1_15N_SHIFT                           0
#define    BINTC_DSP_IMR1_15_CLR_IMR1_15N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_15_OFFSET                                         0x0000011C
#define BINTC_DSP_IMSR0_15_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_15_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_15_IMSR0_15N_SHIFT                             0
#define    BINTC_DSP_IMSR0_15_IMSR0_15N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_15_OFFSET                                         0x0000011E
#define BINTC_DSP_IMSR1_15_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_15_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_15_IMSR1_15N_SHIFT                             0
#define    BINTC_DSP_IMSR1_15_IMSR1_15N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_16_OFFSET                                          0x00000120
#define BINTC_DSP_IMR0_16_TYPE                                            UInt32
#define BINTC_DSP_IMR0_16_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_16_IMR0_16N_SHIFT                               0
#define    BINTC_DSP_IMR0_16_IMR0_16N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_16_SET_OFFSET                                      0x00000122
#define BINTC_DSP_IMR0_16_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_16_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_16_SET_IMR0_16N_SHIFT                           0
#define    BINTC_DSP_IMR0_16_SET_IMR0_16N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_16_CLR_OFFSET                                      0x00000124
#define BINTC_DSP_IMR0_16_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_16_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_16_CLR_IMR0_16N_SHIFT                           0
#define    BINTC_DSP_IMR0_16_CLR_IMR0_16N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_16_OFFSET                                          0x00000126
#define BINTC_DSP_IMR1_16_TYPE                                            UInt32
#define BINTC_DSP_IMR1_16_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_16_IMR1_16N_SHIFT                               0
#define    BINTC_DSP_IMR1_16_IMR1_16N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_16_SET_OFFSET                                      0x00000128
#define BINTC_DSP_IMR1_16_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_16_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_16_SET_IMR1_16N_SHIFT                           0
#define    BINTC_DSP_IMR1_16_SET_IMR1_16N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_16_CLR_OFFSET                                      0x0000012A
#define BINTC_DSP_IMR1_16_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_16_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_16_CLR_IMR1_16N_SHIFT                           0
#define    BINTC_DSP_IMR1_16_CLR_IMR1_16N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_16_OFFSET                                         0x0000012C
#define BINTC_DSP_IMSR0_16_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_16_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_16_IMSR0_16N_SHIFT                             0
#define    BINTC_DSP_IMSR0_16_IMSR0_16N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_16_OFFSET                                         0x0000012E
#define BINTC_DSP_IMSR1_16_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_16_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_16_IMSR1_16N_SHIFT                             0
#define    BINTC_DSP_IMSR1_16_IMSR1_16N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_17_OFFSET                                          0x00000130
#define BINTC_DSP_IMR0_17_TYPE                                            UInt32
#define BINTC_DSP_IMR0_17_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_17_IMR0_17N_SHIFT                               0
#define    BINTC_DSP_IMR0_17_IMR0_17N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_17_SET_OFFSET                                      0x00000132
#define BINTC_DSP_IMR0_17_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_17_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_17_SET_IMR0_17N_SHIFT                           0
#define    BINTC_DSP_IMR0_17_SET_IMR0_17N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_17_CLR_OFFSET                                      0x00000134
#define BINTC_DSP_IMR0_17_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_17_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_17_CLR_IMR0_17N_SHIFT                           0
#define    BINTC_DSP_IMR0_17_CLR_IMR0_17N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_17_OFFSET                                          0x00000136
#define BINTC_DSP_IMR1_17_TYPE                                            UInt32
#define BINTC_DSP_IMR1_17_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_17_IMR1_17N_SHIFT                               0
#define    BINTC_DSP_IMR1_17_IMR1_17N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_17_SET_OFFSET                                      0x00000138
#define BINTC_DSP_IMR1_17_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_17_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_17_SET_IMR1_17N_SHIFT                           0
#define    BINTC_DSP_IMR1_17_SET_IMR1_17N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_17_CLR_OFFSET                                      0x0000013A
#define BINTC_DSP_IMR1_17_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_17_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_17_CLR_IMR1_17N_SHIFT                           0
#define    BINTC_DSP_IMR1_17_CLR_IMR1_17N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_17_OFFSET                                         0x0000013C
#define BINTC_DSP_IMSR0_17_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_17_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_17_IMSR0_17N_SHIFT                             0
#define    BINTC_DSP_IMSR0_17_IMSR0_17N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_17_OFFSET                                         0x0000013E
#define BINTC_DSP_IMSR1_17_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_17_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_17_IMSR1_17N_SHIFT                             0
#define    BINTC_DSP_IMSR1_17_IMSR1_17N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_18_OFFSET                                          0x00000140
#define BINTC_DSP_IMR0_18_TYPE                                            UInt32
#define BINTC_DSP_IMR0_18_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_18_IMR0_18N_SHIFT                               0
#define    BINTC_DSP_IMR0_18_IMR0_18N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_18_SET_OFFSET                                      0x00000142
#define BINTC_DSP_IMR0_18_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_18_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_18_SET_IMR0_18N_SHIFT                           0
#define    BINTC_DSP_IMR0_18_SET_IMR0_18N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_18_CLR_OFFSET                                      0x00000144
#define BINTC_DSP_IMR0_18_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_18_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_18_CLR_IMR0_18N_SHIFT                           0
#define    BINTC_DSP_IMR0_18_CLR_IMR0_18N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_18_OFFSET                                          0x00000146
#define BINTC_DSP_IMR1_18_TYPE                                            UInt32
#define BINTC_DSP_IMR1_18_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_18_IMR1_18N_SHIFT                               0
#define    BINTC_DSP_IMR1_18_IMR1_18N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_18_SET_OFFSET                                      0x00000148
#define BINTC_DSP_IMR1_18_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_18_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_18_SET_IMR1_18N_SHIFT                           0
#define    BINTC_DSP_IMR1_18_SET_IMR1_18N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_18_CLR_OFFSET                                      0x0000014A
#define BINTC_DSP_IMR1_18_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_18_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_18_CLR_IMR1_18N_SHIFT                           0
#define    BINTC_DSP_IMR1_18_CLR_IMR1_18N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_18_OFFSET                                         0x0000014C
#define BINTC_DSP_IMSR0_18_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_18_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_18_IMSR0_18N_SHIFT                             0
#define    BINTC_DSP_IMSR0_18_IMSR0_18N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_18_OFFSET                                         0x0000014E
#define BINTC_DSP_IMSR1_18_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_18_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_18_IMSR1_18N_SHIFT                             0
#define    BINTC_DSP_IMSR1_18_IMSR1_18N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_19_OFFSET                                          0x00000150
#define BINTC_DSP_IMR0_19_TYPE                                            UInt32
#define BINTC_DSP_IMR0_19_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_19_IMR0_19N_SHIFT                               0
#define    BINTC_DSP_IMR0_19_IMR0_19N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_19_SET_OFFSET                                      0x00000152
#define BINTC_DSP_IMR0_19_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_19_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_19_SET_IMR0_19N_SHIFT                           0
#define    BINTC_DSP_IMR0_19_SET_IMR0_19N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_19_CLR_OFFSET                                      0x00000154
#define BINTC_DSP_IMR0_19_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_19_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_19_CLR_IMR0_19N_SHIFT                           0
#define    BINTC_DSP_IMR0_19_CLR_IMR0_19N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_19_OFFSET                                          0x00000156
#define BINTC_DSP_IMR1_19_TYPE                                            UInt32
#define BINTC_DSP_IMR1_19_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_19_IMR1_19N_SHIFT                               0
#define    BINTC_DSP_IMR1_19_IMR1_19N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_19_SET_OFFSET                                      0x00000158
#define BINTC_DSP_IMR1_19_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_19_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_19_SET_IMR1_19N_SHIFT                           0
#define    BINTC_DSP_IMR1_19_SET_IMR1_19N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_19_CLR_OFFSET                                      0x0000015A
#define BINTC_DSP_IMR1_19_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_19_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_19_CLR_IMR1_19N_SHIFT                           0
#define    BINTC_DSP_IMR1_19_CLR_IMR1_19N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_19_OFFSET                                         0x0000015C
#define BINTC_DSP_IMSR0_19_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_19_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_19_IMSR0_19N_SHIFT                             0
#define    BINTC_DSP_IMSR0_19_IMSR0_19N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_19_OFFSET                                         0x0000015E
#define BINTC_DSP_IMSR1_19_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_19_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_19_IMSR1_19N_SHIFT                             0
#define    BINTC_DSP_IMSR1_19_IMSR1_19N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_20_OFFSET                                          0x00000160
#define BINTC_DSP_IMR0_20_TYPE                                            UInt32
#define BINTC_DSP_IMR0_20_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_20_IMR0_20N_SHIFT                               0
#define    BINTC_DSP_IMR0_20_IMR0_20N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_20_SET_OFFSET                                      0x00000162
#define BINTC_DSP_IMR0_20_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_20_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_20_SET_IMR0_20N_SHIFT                           0
#define    BINTC_DSP_IMR0_20_SET_IMR0_20N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_20_CLR_OFFSET                                      0x00000164
#define BINTC_DSP_IMR0_20_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_20_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_20_CLR_IMR0_20N_SHIFT                           0
#define    BINTC_DSP_IMR0_20_CLR_IMR0_20N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_20_OFFSET                                          0x00000166
#define BINTC_DSP_IMR1_20_TYPE                                            UInt32
#define BINTC_DSP_IMR1_20_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_20_IMR1_20N_SHIFT                               0
#define    BINTC_DSP_IMR1_20_IMR1_20N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_20_SET_OFFSET                                      0x00000168
#define BINTC_DSP_IMR1_20_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_20_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_20_SET_IMR1_20N_SHIFT                           0
#define    BINTC_DSP_IMR1_20_SET_IMR1_20N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_20_CLR_OFFSET                                      0x0000016A
#define BINTC_DSP_IMR1_20_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_20_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_20_CLR_IMR1_20N_SHIFT                           0
#define    BINTC_DSP_IMR1_20_CLR_IMR1_20N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_20_OFFSET                                         0x0000016C
#define BINTC_DSP_IMSR0_20_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_20_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_20_IMSR0_20N_SHIFT                             0
#define    BINTC_DSP_IMSR0_20_IMSR0_20N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_20_OFFSET                                         0x0000016E
#define BINTC_DSP_IMSR1_20_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_20_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_20_IMSR1_20N_SHIFT                             0
#define    BINTC_DSP_IMSR1_20_IMSR1_20N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_21_OFFSET                                          0x00000170
#define BINTC_DSP_IMR0_21_TYPE                                            UInt32
#define BINTC_DSP_IMR0_21_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_21_IMR0_21N_SHIFT                               0
#define    BINTC_DSP_IMR0_21_IMR0_21N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_21_SET_OFFSET                                      0x00000172
#define BINTC_DSP_IMR0_21_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_21_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_21_SET_IMR0_21N_SHIFT                           0
#define    BINTC_DSP_IMR0_21_SET_IMR0_21N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_21_CLR_OFFSET                                      0x00000174
#define BINTC_DSP_IMR0_21_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_21_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_21_CLR_IMR0_21N_SHIFT                           0
#define    BINTC_DSP_IMR0_21_CLR_IMR0_21N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_21_OFFSET                                          0x00000176
#define BINTC_DSP_IMR1_21_TYPE                                            UInt32
#define BINTC_DSP_IMR1_21_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_21_IMR1_21N_SHIFT                               0
#define    BINTC_DSP_IMR1_21_IMR1_21N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_21_SET_OFFSET                                      0x00000178
#define BINTC_DSP_IMR1_21_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_21_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_21_SET_IMR1_21N_SHIFT                           0
#define    BINTC_DSP_IMR1_21_SET_IMR1_21N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_21_CLR_OFFSET                                      0x0000017A
#define BINTC_DSP_IMR1_21_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_21_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_21_CLR_IMR1_21N_SHIFT                           0
#define    BINTC_DSP_IMR1_21_CLR_IMR1_21N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_21_OFFSET                                         0x0000017C
#define BINTC_DSP_IMSR0_21_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_21_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_21_IMSR0_21N_SHIFT                             0
#define    BINTC_DSP_IMSR0_21_IMSR0_21N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_21_OFFSET                                         0x0000017E
#define BINTC_DSP_IMSR1_21_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_21_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_21_IMSR1_21N_SHIFT                             0
#define    BINTC_DSP_IMSR1_21_IMSR1_21N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_22_OFFSET                                          0x00000180
#define BINTC_DSP_IMR0_22_TYPE                                            UInt32
#define BINTC_DSP_IMR0_22_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_22_IMR0_22N_SHIFT                               0
#define    BINTC_DSP_IMR0_22_IMR0_22N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_22_SET_OFFSET                                      0x00000182
#define BINTC_DSP_IMR0_22_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_22_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_22_SET_IMR0_22N_SHIFT                           0
#define    BINTC_DSP_IMR0_22_SET_IMR0_22N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_22_CLR_OFFSET                                      0x00000184
#define BINTC_DSP_IMR0_22_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_22_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_22_CLR_IMR0_22N_SHIFT                           0
#define    BINTC_DSP_IMR0_22_CLR_IMR0_22N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_22_OFFSET                                          0x00000186
#define BINTC_DSP_IMR1_22_TYPE                                            UInt32
#define BINTC_DSP_IMR1_22_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_22_IMR1_22N_SHIFT                               0
#define    BINTC_DSP_IMR1_22_IMR1_22N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_22_SET_OFFSET                                      0x00000188
#define BINTC_DSP_IMR1_22_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_22_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_22_SET_IMR1_22N_SHIFT                           0
#define    BINTC_DSP_IMR1_22_SET_IMR1_22N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_22_CLR_OFFSET                                      0x0000018A
#define BINTC_DSP_IMR1_22_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_22_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_22_CLR_IMR1_22N_SHIFT                           0
#define    BINTC_DSP_IMR1_22_CLR_IMR1_22N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_22_OFFSET                                         0x0000018C
#define BINTC_DSP_IMSR0_22_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_22_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_22_IMSR0_22N_SHIFT                             0
#define    BINTC_DSP_IMSR0_22_IMSR0_22N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_22_OFFSET                                         0x0000018E
#define BINTC_DSP_IMSR1_22_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_22_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_22_IMSR1_22N_SHIFT                             0
#define    BINTC_DSP_IMSR1_22_IMSR1_22N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_23_OFFSET                                          0x00000190
#define BINTC_DSP_IMR0_23_TYPE                                            UInt32
#define BINTC_DSP_IMR0_23_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_23_IMR0_23N_SHIFT                               0
#define    BINTC_DSP_IMR0_23_IMR0_23N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_23_SET_OFFSET                                      0x00000192
#define BINTC_DSP_IMR0_23_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_23_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_23_SET_IMR0_23N_SHIFT                           0
#define    BINTC_DSP_IMR0_23_SET_IMR0_23N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_23_CLR_OFFSET                                      0x00000194
#define BINTC_DSP_IMR0_23_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_23_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_23_CLR_IMR0_23N_SHIFT                           0
#define    BINTC_DSP_IMR0_23_CLR_IMR0_23N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_23_OFFSET                                          0x00000196
#define BINTC_DSP_IMR1_23_TYPE                                            UInt32
#define BINTC_DSP_IMR1_23_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_23_IMR1_23N_SHIFT                               0
#define    BINTC_DSP_IMR1_23_IMR1_23N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_23_SET_OFFSET                                      0x00000198
#define BINTC_DSP_IMR1_23_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_23_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_23_SET_IMR1_23N_SHIFT                           0
#define    BINTC_DSP_IMR1_23_SET_IMR1_23N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_23_CLR_OFFSET                                      0x0000019A
#define BINTC_DSP_IMR1_23_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_23_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_23_CLR_IMR1_23N_SHIFT                           0
#define    BINTC_DSP_IMR1_23_CLR_IMR1_23N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_23_OFFSET                                         0x0000019C
#define BINTC_DSP_IMSR0_23_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_23_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_23_IMSR0_23N_SHIFT                             0
#define    BINTC_DSP_IMSR0_23_IMSR0_23N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_23_OFFSET                                         0x0000019E
#define BINTC_DSP_IMSR1_23_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_23_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_23_IMSR1_23N_SHIFT                             0
#define    BINTC_DSP_IMSR1_23_IMSR1_23N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_24_OFFSET                                          0x000001A0
#define BINTC_DSP_IMR0_24_TYPE                                            UInt32
#define BINTC_DSP_IMR0_24_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_24_IMR0_24N_SHIFT                               0
#define    BINTC_DSP_IMR0_24_IMR0_24N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_24_SET_OFFSET                                      0x000001A2
#define BINTC_DSP_IMR0_24_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_24_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_24_SET_IMR0_24N_SHIFT                           0
#define    BINTC_DSP_IMR0_24_SET_IMR0_24N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_24_CLR_OFFSET                                      0x000001A4
#define BINTC_DSP_IMR0_24_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_24_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_24_CLR_IMR0_24N_SHIFT                           0
#define    BINTC_DSP_IMR0_24_CLR_IMR0_24N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_24_OFFSET                                          0x000001A6
#define BINTC_DSP_IMR1_24_TYPE                                            UInt32
#define BINTC_DSP_IMR1_24_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_24_IMR1_24N_SHIFT                               0
#define    BINTC_DSP_IMR1_24_IMR1_24N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_24_SET_OFFSET                                      0x000001A8
#define BINTC_DSP_IMR1_24_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_24_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_24_SET_IMR1_24N_SHIFT                           0
#define    BINTC_DSP_IMR1_24_SET_IMR1_24N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_24_CLR_OFFSET                                      0x000001AA
#define BINTC_DSP_IMR1_24_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_24_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_24_CLR_IMR1_24N_SHIFT                           0
#define    BINTC_DSP_IMR1_24_CLR_IMR1_24N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_24_OFFSET                                         0x000001AC
#define BINTC_DSP_IMSR0_24_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_24_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_24_IMSR0_24N_SHIFT                             0
#define    BINTC_DSP_IMSR0_24_IMSR0_24N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_24_OFFSET                                         0x000001AE
#define BINTC_DSP_IMSR1_24_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_24_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_24_IMSR1_24N_SHIFT                             0
#define    BINTC_DSP_IMSR1_24_IMSR1_24N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_25_OFFSET                                          0x000001B0
#define BINTC_DSP_IMR0_25_TYPE                                            UInt32
#define BINTC_DSP_IMR0_25_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_25_IMR0_25N_SHIFT                               0
#define    BINTC_DSP_IMR0_25_IMR0_25N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_25_SET_OFFSET                                      0x000001B2
#define BINTC_DSP_IMR0_25_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_25_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_25_SET_IMR0_25N_SHIFT                           0
#define    BINTC_DSP_IMR0_25_SET_IMR0_25N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_25_CLR_OFFSET                                      0x000001B4
#define BINTC_DSP_IMR0_25_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_25_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_25_CLR_IMR0_25N_SHIFT                           0
#define    BINTC_DSP_IMR0_25_CLR_IMR0_25N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_25_OFFSET                                          0x000001B6
#define BINTC_DSP_IMR1_25_TYPE                                            UInt32
#define BINTC_DSP_IMR1_25_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_25_IMR1_25N_SHIFT                               0
#define    BINTC_DSP_IMR1_25_IMR1_25N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_25_SET_OFFSET                                      0x000001B8
#define BINTC_DSP_IMR1_25_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_25_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_25_SET_IMR1_25N_SHIFT                           0
#define    BINTC_DSP_IMR1_25_SET_IMR1_25N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_25_CLR_OFFSET                                      0x000001BA
#define BINTC_DSP_IMR1_25_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_25_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_25_CLR_IMR1_25N_SHIFT                           0
#define    BINTC_DSP_IMR1_25_CLR_IMR1_25N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_25_OFFSET                                         0x000001BC
#define BINTC_DSP_IMSR0_25_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_25_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_25_IMSR0_25N_SHIFT                             0
#define    BINTC_DSP_IMSR0_25_IMSR0_25N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_25_OFFSET                                         0x000001BE
#define BINTC_DSP_IMSR1_25_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_25_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_25_IMSR1_25N_SHIFT                             0
#define    BINTC_DSP_IMSR1_25_IMSR1_25N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_26_OFFSET                                          0x000001C0
#define BINTC_DSP_IMR0_26_TYPE                                            UInt32
#define BINTC_DSP_IMR0_26_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_26_IMR0_26N_SHIFT                               0
#define    BINTC_DSP_IMR0_26_IMR0_26N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_26_SET_OFFSET                                      0x000001C2
#define BINTC_DSP_IMR0_26_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_26_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_26_SET_IMR0_26N_SHIFT                           0
#define    BINTC_DSP_IMR0_26_SET_IMR0_26N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_26_CLR_OFFSET                                      0x000001C4
#define BINTC_DSP_IMR0_26_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_26_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_26_CLR_IMR0_26N_SHIFT                           0
#define    BINTC_DSP_IMR0_26_CLR_IMR0_26N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_26_OFFSET                                          0x000001C6
#define BINTC_DSP_IMR1_26_TYPE                                            UInt32
#define BINTC_DSP_IMR1_26_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_26_IMR1_26N_SHIFT                               0
#define    BINTC_DSP_IMR1_26_IMR1_26N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_26_SET_OFFSET                                      0x000001C8
#define BINTC_DSP_IMR1_26_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_26_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_26_SET_IMR1_26N_SHIFT                           0
#define    BINTC_DSP_IMR1_26_SET_IMR1_26N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_26_CLR_OFFSET                                      0x000001CA
#define BINTC_DSP_IMR1_26_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_26_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_26_CLR_IMR1_26N_SHIFT                           0
#define    BINTC_DSP_IMR1_26_CLR_IMR1_26N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_26_OFFSET                                         0x000001CC
#define BINTC_DSP_IMSR0_26_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_26_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_26_IMSR0_26N_SHIFT                             0
#define    BINTC_DSP_IMSR0_26_IMSR0_26N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_26_OFFSET                                         0x000001CE
#define BINTC_DSP_IMSR1_26_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_26_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_26_IMSR1_26N_SHIFT                             0
#define    BINTC_DSP_IMSR1_26_IMSR1_26N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_27_OFFSET                                          0x000001D0
#define BINTC_DSP_IMR0_27_TYPE                                            UInt32
#define BINTC_DSP_IMR0_27_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_27_IMR0_27N_SHIFT                               0
#define    BINTC_DSP_IMR0_27_IMR0_27N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_27_SET_OFFSET                                      0x000001D2
#define BINTC_DSP_IMR0_27_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_27_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_27_SET_IMR0_27N_SHIFT                           0
#define    BINTC_DSP_IMR0_27_SET_IMR0_27N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_27_CLR_OFFSET                                      0x000001D4
#define BINTC_DSP_IMR0_27_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_27_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_27_CLR_IMR0_27N_SHIFT                           0
#define    BINTC_DSP_IMR0_27_CLR_IMR0_27N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_27_OFFSET                                          0x000001D6
#define BINTC_DSP_IMR1_27_TYPE                                            UInt32
#define BINTC_DSP_IMR1_27_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_27_IMR1_27N_SHIFT                               0
#define    BINTC_DSP_IMR1_27_IMR1_27N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_27_SET_OFFSET                                      0x000001D8
#define BINTC_DSP_IMR1_27_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_27_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_27_SET_IMR1_27N_SHIFT                           0
#define    BINTC_DSP_IMR1_27_SET_IMR1_27N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_27_CLR_OFFSET                                      0x000001DA
#define BINTC_DSP_IMR1_27_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_27_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_27_CLR_IMR1_27N_SHIFT                           0
#define    BINTC_DSP_IMR1_27_CLR_IMR1_27N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_27_OFFSET                                         0x000001DC
#define BINTC_DSP_IMSR0_27_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_27_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_27_IMSR0_27N_SHIFT                             0
#define    BINTC_DSP_IMSR0_27_IMSR0_27N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_27_OFFSET                                         0x000001DE
#define BINTC_DSP_IMSR1_27_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_27_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_27_IMSR1_27N_SHIFT                             0
#define    BINTC_DSP_IMSR1_27_IMSR1_27N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_28_OFFSET                                          0x000001E0
#define BINTC_DSP_IMR0_28_TYPE                                            UInt32
#define BINTC_DSP_IMR0_28_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_28_IMR0_28N_SHIFT                               0
#define    BINTC_DSP_IMR0_28_IMR0_28N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_28_SET_OFFSET                                      0x000001E2
#define BINTC_DSP_IMR0_28_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_28_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_28_SET_IMR0_28N_SHIFT                           0
#define    BINTC_DSP_IMR0_28_SET_IMR0_28N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_28_CLR_OFFSET                                      0x000001E4
#define BINTC_DSP_IMR0_28_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_28_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_28_CLR_IMR0_28N_SHIFT                           0
#define    BINTC_DSP_IMR0_28_CLR_IMR0_28N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_28_OFFSET                                          0x000001E6
#define BINTC_DSP_IMR1_28_TYPE                                            UInt32
#define BINTC_DSP_IMR1_28_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_28_IMR1_28N_SHIFT                               0
#define    BINTC_DSP_IMR1_28_IMR1_28N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_28_SET_OFFSET                                      0x000001E8
#define BINTC_DSP_IMR1_28_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_28_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_28_SET_IMR1_28N_SHIFT                           0
#define    BINTC_DSP_IMR1_28_SET_IMR1_28N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_28_CLR_OFFSET                                      0x000001EA
#define BINTC_DSP_IMR1_28_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_28_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_28_CLR_IMR1_28N_SHIFT                           0
#define    BINTC_DSP_IMR1_28_CLR_IMR1_28N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_28_OFFSET                                         0x000001EC
#define BINTC_DSP_IMSR0_28_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_28_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_28_IMSR0_28N_SHIFT                             0
#define    BINTC_DSP_IMSR0_28_IMSR0_28N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_28_OFFSET                                         0x000001EE
#define BINTC_DSP_IMSR1_28_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_28_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_28_IMSR1_28N_SHIFT                             0
#define    BINTC_DSP_IMSR1_28_IMSR1_28N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_29_OFFSET                                          0x000001F0
#define BINTC_DSP_IMR0_29_TYPE                                            UInt32
#define BINTC_DSP_IMR0_29_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_29_IMR0_29N_SHIFT                               0
#define    BINTC_DSP_IMR0_29_IMR0_29N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_29_SET_OFFSET                                      0x000001F2
#define BINTC_DSP_IMR0_29_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_29_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_29_SET_IMR0_29N_SHIFT                           0
#define    BINTC_DSP_IMR0_29_SET_IMR0_29N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_29_CLR_OFFSET                                      0x000001F4
#define BINTC_DSP_IMR0_29_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_29_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_29_CLR_IMR0_29N_SHIFT                           0
#define    BINTC_DSP_IMR0_29_CLR_IMR0_29N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_29_OFFSET                                          0x000001F6
#define BINTC_DSP_IMR1_29_TYPE                                            UInt32
#define BINTC_DSP_IMR1_29_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_29_IMR1_29N_SHIFT                               0
#define    BINTC_DSP_IMR1_29_IMR1_29N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_29_SET_OFFSET                                      0x000001F8
#define BINTC_DSP_IMR1_29_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_29_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_29_SET_IMR1_29N_SHIFT                           0
#define    BINTC_DSP_IMR1_29_SET_IMR1_29N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_29_CLR_OFFSET                                      0x000001FA
#define BINTC_DSP_IMR1_29_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_29_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_29_CLR_IMR1_29N_SHIFT                           0
#define    BINTC_DSP_IMR1_29_CLR_IMR1_29N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_29_OFFSET                                         0x000001FC
#define BINTC_DSP_IMSR0_29_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_29_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_29_IMSR0_29N_SHIFT                             0
#define    BINTC_DSP_IMSR0_29_IMSR0_29N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_29_OFFSET                                         0x000001FE
#define BINTC_DSP_IMSR1_29_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_29_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_29_IMSR1_29N_SHIFT                             0
#define    BINTC_DSP_IMSR1_29_IMSR1_29N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_30_OFFSET                                          0x00000200
#define BINTC_DSP_IMR0_30_TYPE                                            UInt32
#define BINTC_DSP_IMR0_30_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_30_IMR0_30N_SHIFT                               0
#define    BINTC_DSP_IMR0_30_IMR0_30N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_30_SET_OFFSET                                      0x00000202
#define BINTC_DSP_IMR0_30_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_30_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_30_SET_IMR0_30N_SHIFT                           0
#define    BINTC_DSP_IMR0_30_SET_IMR0_30N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_30_CLR_OFFSET                                      0x00000204
#define BINTC_DSP_IMR0_30_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_30_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_30_CLR_IMR0_30N_SHIFT                           0
#define    BINTC_DSP_IMR0_30_CLR_IMR0_30N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_30_OFFSET                                          0x00000206
#define BINTC_DSP_IMR1_30_TYPE                                            UInt32
#define BINTC_DSP_IMR1_30_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_30_IMR1_30N_SHIFT                               0
#define    BINTC_DSP_IMR1_30_IMR1_30N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_30_SET_OFFSET                                      0x00000208
#define BINTC_DSP_IMR1_30_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_30_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_30_SET_IMR1_30N_SHIFT                           0
#define    BINTC_DSP_IMR1_30_SET_IMR1_30N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_30_CLR_OFFSET                                      0x0000020A
#define BINTC_DSP_IMR1_30_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_30_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_30_CLR_IMR1_30N_SHIFT                           0
#define    BINTC_DSP_IMR1_30_CLR_IMR1_30N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_30_OFFSET                                         0x0000020C
#define BINTC_DSP_IMSR0_30_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_30_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_30_IMSR0_30N_SHIFT                             0
#define    BINTC_DSP_IMSR0_30_IMSR0_30N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_30_OFFSET                                         0x0000020E
#define BINTC_DSP_IMSR1_30_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_30_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_30_IMSR1_30N_SHIFT                             0
#define    BINTC_DSP_IMSR1_30_IMSR1_30N_MASK                              0x00FFFFFF

#define BINTC_DSP_IMR0_31_OFFSET                                          0x00000210
#define BINTC_DSP_IMR0_31_TYPE                                            UInt32
#define BINTC_DSP_IMR0_31_RESERVED_MASK                                   0x00000000
#define    BINTC_DSP_IMR0_31_IMR0_31N_SHIFT                               0
#define    BINTC_DSP_IMR0_31_IMR0_31N_MASK                                0xFFFFFFFF

#define BINTC_DSP_IMR0_31_SET_OFFSET                                      0x00000212
#define BINTC_DSP_IMR0_31_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR0_31_SET_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_31_SET_IMR0_31N_SHIFT                           0
#define    BINTC_DSP_IMR0_31_SET_IMR0_31N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR0_31_CLR_OFFSET                                      0x00000214
#define BINTC_DSP_IMR0_31_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR0_31_CLR_RESERVED_MASK                               0x00000000
#define    BINTC_DSP_IMR0_31_CLR_IMR0_31N_SHIFT                           0
#define    BINTC_DSP_IMR0_31_CLR_IMR0_31N_MASK                            0xFFFFFFFF

#define BINTC_DSP_IMR1_31_OFFSET                                          0x00000216
#define BINTC_DSP_IMR1_31_TYPE                                            UInt32
#define BINTC_DSP_IMR1_31_RESERVED_MASK                                   0xFF000000
#define    BINTC_DSP_IMR1_31_IMR1_31N_SHIFT                               0
#define    BINTC_DSP_IMR1_31_IMR1_31N_MASK                                0x00FFFFFF

#define BINTC_DSP_IMR1_31_SET_OFFSET                                      0x00000218
#define BINTC_DSP_IMR1_31_SET_TYPE                                        UInt32
#define BINTC_DSP_IMR1_31_SET_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_31_SET_IMR1_31N_SHIFT                           0
#define    BINTC_DSP_IMR1_31_SET_IMR1_31N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMR1_31_CLR_OFFSET                                      0x0000021A
#define BINTC_DSP_IMR1_31_CLR_TYPE                                        UInt32
#define BINTC_DSP_IMR1_31_CLR_RESERVED_MASK                               0xFF000000
#define    BINTC_DSP_IMR1_31_CLR_IMR1_31N_SHIFT                           0
#define    BINTC_DSP_IMR1_31_CLR_IMR1_31N_MASK                            0x00FFFFFF

#define BINTC_DSP_IMSR0_31_OFFSET                                         0x0000021C
#define BINTC_DSP_IMSR0_31_TYPE                                           UInt32
#define BINTC_DSP_IMSR0_31_RESERVED_MASK                                  0x00000000
#define    BINTC_DSP_IMSR0_31_IMSR0_31N_SHIFT                             0
#define    BINTC_DSP_IMSR0_31_IMSR0_31N_MASK                              0xFFFFFFFF

#define BINTC_DSP_IMSR1_31_OFFSET                                         0x0000021E
#define BINTC_DSP_IMSR1_31_TYPE                                           UInt32
#define BINTC_DSP_IMSR1_31_RESERVED_MASK                                  0xFF000000
#define    BINTC_DSP_IMSR1_31_IMSR1_31N_SHIFT                             0
#define    BINTC_DSP_IMSR1_31_IMSR1_31N_MASK                              0x00FFFFFF

#define BINTC_ISR0_OFFSET                                                 0x00000000
#define BINTC_ISR0_TYPE                                                   UInt32
#define BINTC_ISR0_RESERVED_MASK                                          0x00000000
#define    BINTC_ISR0_ISR0N_SHIFT                                         0
#define    BINTC_ISR0_ISR0N_MASK                                          0xFFFFFFFF

#define BINTC_ICR0_OFFSET                                                 0x00000004
#define BINTC_ICR0_TYPE                                                   UInt32
#define BINTC_ICR0_RESERVED_MASK                                          0x00000000
#define    BINTC_ICR0_ICR0N_SHIFT                                         0
#define    BINTC_ICR0_ICR0N_MASK                                          0xFFFFFFFF

#define BINTC_ISR1_OFFSET                                                 0x00000008
#define BINTC_ISR1_TYPE                                                   UInt32
#define BINTC_ISR1_RESERVED_MASK                                          0xFF000000
#define    BINTC_ISR1_ISR1N_SHIFT                                         0
#define    BINTC_ISR1_ISR1N_MASK                                          0x00FFFFFF

#define BINTC_ICR1_OFFSET                                                 0x0000000C
#define BINTC_ICR1_TYPE                                                   UInt32
#define BINTC_ICR1_RESERVED_MASK                                          0xFF000000
#define    BINTC_ICR1_ICR1N_SHIFT                                         0
#define    BINTC_ICR1_ICR1N_MASK                                          0x00FFFFFF

#define BINTC_ISWIR0_OFFSET                                               0x00000020
#define BINTC_ISWIR0_TYPE                                                 UInt32
#define BINTC_ISWIR0_RESERVED_MASK                                        0x00000000
#define    BINTC_ISWIR0_ISWIR0N_SHIFT                                     0
#define    BINTC_ISWIR0_ISWIR0N_MASK                                      0xFFFFFFFF

#define BINTC_ISWIR0_CLR_OFFSET                                           0x00000024
#define BINTC_ISWIR0_CLR_TYPE                                             UInt32
#define BINTC_ISWIR0_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_ISWIR0_CLR_ISWIR0N_SHIFT                                 0
#define    BINTC_ISWIR0_CLR_ISWIR0N_MASK                                  0xFFFFFFFF

#define BINTC_ISWIR1_OFFSET                                               0x00000030
#define BINTC_ISWIR1_TYPE                                                 UInt32
#define BINTC_ISWIR1_RESERVED_MASK                                        0xFF000000
#define    BINTC_ISWIR1_ISWIR1N_SHIFT                                     0
#define    BINTC_ISWIR1_ISWIR1N_MASK                                      0x00FFFFFF

#define BINTC_ISWIR1_CLR_OFFSET                                           0x00000034
#define BINTC_ISWIR1_CLR_TYPE                                             UInt32
#define BINTC_ISWIR1_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_ISWIR1_CLR_ISWIR1N_SHIFT                                 0
#define    BINTC_ISWIR1_CLR_ISWIR1N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_0_OFFSET                                               0x00000040
#define BINTC_IMR0_0_TYPE                                                 UInt32
#define BINTC_IMR0_0_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_0_IMR0_0N_SHIFT                                     0
#define    BINTC_IMR0_0_IMR0_0N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_0_SET_OFFSET                                           0x00000044
#define BINTC_IMR0_0_SET_TYPE                                             UInt32
#define BINTC_IMR0_0_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_0_SET_IMR0_0N_SHIFT                                 0
#define    BINTC_IMR0_0_SET_IMR0_0N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_0_CLR_OFFSET                                           0x00000048
#define BINTC_IMR0_0_CLR_TYPE                                             UInt32
#define BINTC_IMR0_0_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_0_CLR_IMR0_0N_SHIFT                                 0
#define    BINTC_IMR0_0_CLR_IMR0_0N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_0_OFFSET                                               0x0000004C
#define BINTC_IMR1_0_TYPE                                                 UInt32
#define BINTC_IMR1_0_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_0_IMR1_0N_SHIFT                                     0
#define    BINTC_IMR1_0_IMR1_0N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_0_SET_OFFSET                                           0x00000050
#define BINTC_IMR1_0_SET_TYPE                                             UInt32
#define BINTC_IMR1_0_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_0_SET_IMR1_0N_SHIFT                                 0
#define    BINTC_IMR1_0_SET_IMR1_0N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_0_CLR_OFFSET                                           0x00000054
#define BINTC_IMR1_0_CLR_TYPE                                             UInt32
#define BINTC_IMR1_0_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_0_CLR_IMR1_0N_SHIFT                                 0
#define    BINTC_IMR1_0_CLR_IMR1_0N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_0_OFFSET                                              0x00000058
#define BINTC_IMSR0_0_TYPE                                                UInt32
#define BINTC_IMSR0_0_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_0_IMSR0_0N_SHIFT                                   0
#define    BINTC_IMSR0_0_IMSR0_0N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_0_OFFSET                                              0x0000005C
#define BINTC_IMSR1_0_TYPE                                                UInt32
#define BINTC_IMSR1_0_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_0_IMSR1_0N_SHIFT                                   0
#define    BINTC_IMSR1_0_IMSR1_0N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_1_OFFSET                                               0x00000060
#define BINTC_IMR0_1_TYPE                                                 UInt32
#define BINTC_IMR0_1_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_1_IMR0_1N_SHIFT                                     0
#define    BINTC_IMR0_1_IMR0_1N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_1_SET_OFFSET                                           0x00000064
#define BINTC_IMR0_1_SET_TYPE                                             UInt32
#define BINTC_IMR0_1_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_1_SET_IMR0_1N_SHIFT                                 0
#define    BINTC_IMR0_1_SET_IMR0_1N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_1_CLR_OFFSET                                           0x00000068
#define BINTC_IMR0_1_CLR_TYPE                                             UInt32
#define BINTC_IMR0_1_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_1_CLR_IMR0_1N_SHIFT                                 0
#define    BINTC_IMR0_1_CLR_IMR0_1N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_1_OFFSET                                               0x0000006C
#define BINTC_IMR1_1_TYPE                                                 UInt32
#define BINTC_IMR1_1_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_1_IMR1_1N_SHIFT                                     0
#define    BINTC_IMR1_1_IMR1_1N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_1_SET_OFFSET                                           0x00000070
#define BINTC_IMR1_1_SET_TYPE                                             UInt32
#define BINTC_IMR1_1_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_1_SET_IMR1_1N_SHIFT                                 0
#define    BINTC_IMR1_1_SET_IMR1_1N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_1_CLR_OFFSET                                           0x00000074
#define BINTC_IMR1_1_CLR_TYPE                                             UInt32
#define BINTC_IMR1_1_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_1_CLR_IMR1_1N_SHIFT                                 0
#define    BINTC_IMR1_1_CLR_IMR1_1N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_1_OFFSET                                              0x00000078
#define BINTC_IMSR0_1_TYPE                                                UInt32
#define BINTC_IMSR0_1_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_1_IMSR0_1N_SHIFT                                   0
#define    BINTC_IMSR0_1_IMSR0_1N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_1_OFFSET                                              0x0000007C
#define BINTC_IMSR1_1_TYPE                                                UInt32
#define BINTC_IMSR1_1_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_1_IMSR1_1N_SHIFT                                   0
#define    BINTC_IMSR1_1_IMSR1_1N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_2_OFFSET                                               0x00000080
#define BINTC_IMR0_2_TYPE                                                 UInt32
#define BINTC_IMR0_2_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_2_IMR0_2N_SHIFT                                     0
#define    BINTC_IMR0_2_IMR0_2N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_2_SET_OFFSET                                           0x00000084
#define BINTC_IMR0_2_SET_TYPE                                             UInt32
#define BINTC_IMR0_2_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_2_SET_IMR0_2N_SHIFT                                 0
#define    BINTC_IMR0_2_SET_IMR0_2N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_2_CLR_OFFSET                                           0x00000088
#define BINTC_IMR0_2_CLR_TYPE                                             UInt32
#define BINTC_IMR0_2_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_2_CLR_IMR0_2N_SHIFT                                 0
#define    BINTC_IMR0_2_CLR_IMR0_2N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_2_OFFSET                                               0x0000008C
#define BINTC_IMR1_2_TYPE                                                 UInt32
#define BINTC_IMR1_2_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_2_IMR1_2N_SHIFT                                     0
#define    BINTC_IMR1_2_IMR1_2N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_2_SET_OFFSET                                           0x00000090
#define BINTC_IMR1_2_SET_TYPE                                             UInt32
#define BINTC_IMR1_2_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_2_SET_IMR1_2N_SHIFT                                 0
#define    BINTC_IMR1_2_SET_IMR1_2N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_2_CLR_OFFSET                                           0x00000094
#define BINTC_IMR1_2_CLR_TYPE                                             UInt32
#define BINTC_IMR1_2_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_2_CLR_IMR1_2N_SHIFT                                 0
#define    BINTC_IMR1_2_CLR_IMR1_2N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_2_OFFSET                                              0x00000098
#define BINTC_IMSR0_2_TYPE                                                UInt32
#define BINTC_IMSR0_2_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_2_IMSR0_2N_SHIFT                                   0
#define    BINTC_IMSR0_2_IMSR0_2N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_2_OFFSET                                              0x0000009C
#define BINTC_IMSR1_2_TYPE                                                UInt32
#define BINTC_IMSR1_2_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_2_IMSR1_2N_SHIFT                                   0
#define    BINTC_IMSR1_2_IMSR1_2N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_3_OFFSET                                               0x000000A0
#define BINTC_IMR0_3_TYPE                                                 UInt32
#define BINTC_IMR0_3_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_3_IMR0_3N_SHIFT                                     0
#define    BINTC_IMR0_3_IMR0_3N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_3_SET_OFFSET                                           0x000000A4
#define BINTC_IMR0_3_SET_TYPE                                             UInt32
#define BINTC_IMR0_3_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_3_SET_IMR0_3N_SHIFT                                 0
#define    BINTC_IMR0_3_SET_IMR0_3N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_3_CLR_OFFSET                                           0x000000A8
#define BINTC_IMR0_3_CLR_TYPE                                             UInt32
#define BINTC_IMR0_3_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_3_CLR_IMR0_3N_SHIFT                                 0
#define    BINTC_IMR0_3_CLR_IMR0_3N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_3_OFFSET                                               0x000000AC
#define BINTC_IMR1_3_TYPE                                                 UInt32
#define BINTC_IMR1_3_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_3_IMR1_3N_SHIFT                                     0
#define    BINTC_IMR1_3_IMR1_3N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_3_SET_OFFSET                                           0x000000B0
#define BINTC_IMR1_3_SET_TYPE                                             UInt32
#define BINTC_IMR1_3_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_3_SET_IMR1_3N_SHIFT                                 0
#define    BINTC_IMR1_3_SET_IMR1_3N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_3_CLR_OFFSET                                           0x000000B4
#define BINTC_IMR1_3_CLR_TYPE                                             UInt32
#define BINTC_IMR1_3_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_3_CLR_IMR1_3N_SHIFT                                 0
#define    BINTC_IMR1_3_CLR_IMR1_3N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_3_OFFSET                                              0x000000B8
#define BINTC_IMSR0_3_TYPE                                                UInt32
#define BINTC_IMSR0_3_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_3_IMSR0_3N_SHIFT                                   0
#define    BINTC_IMSR0_3_IMSR0_3N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_3_OFFSET                                              0x000000BC
#define BINTC_IMSR1_3_TYPE                                                UInt32
#define BINTC_IMSR1_3_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_3_IMSR1_3N_SHIFT                                   0
#define    BINTC_IMSR1_3_IMSR1_3N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_4_OFFSET                                               0x000000C0
#define BINTC_IMR0_4_TYPE                                                 UInt32
#define BINTC_IMR0_4_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_4_IMR0_4N_SHIFT                                     0
#define    BINTC_IMR0_4_IMR0_4N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_4_SET_OFFSET                                           0x000000C4
#define BINTC_IMR0_4_SET_TYPE                                             UInt32
#define BINTC_IMR0_4_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_4_SET_IMR0_4N_SHIFT                                 0
#define    BINTC_IMR0_4_SET_IMR0_4N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_4_CLR_OFFSET                                           0x000000C8
#define BINTC_IMR0_4_CLR_TYPE                                             UInt32
#define BINTC_IMR0_4_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_4_CLR_IMR0_4N_SHIFT                                 0
#define    BINTC_IMR0_4_CLR_IMR0_4N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_4_OFFSET                                               0x000000CC
#define BINTC_IMR1_4_TYPE                                                 UInt32
#define BINTC_IMR1_4_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_4_IMR1_4N_SHIFT                                     0
#define    BINTC_IMR1_4_IMR1_4N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_4_SET_OFFSET                                           0x000000D0
#define BINTC_IMR1_4_SET_TYPE                                             UInt32
#define BINTC_IMR1_4_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_4_SET_IMR1_4N_SHIFT                                 0
#define    BINTC_IMR1_4_SET_IMR1_4N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_4_CLR_OFFSET                                           0x000000D4
#define BINTC_IMR1_4_CLR_TYPE                                             UInt32
#define BINTC_IMR1_4_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_4_CLR_IMR1_4N_SHIFT                                 0
#define    BINTC_IMR1_4_CLR_IMR1_4N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_4_OFFSET                                              0x000000D8
#define BINTC_IMSR0_4_TYPE                                                UInt32
#define BINTC_IMSR0_4_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_4_IMSR0_4N_SHIFT                                   0
#define    BINTC_IMSR0_4_IMSR0_4N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_4_OFFSET                                              0x000000DC
#define BINTC_IMSR1_4_TYPE                                                UInt32
#define BINTC_IMSR1_4_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_4_IMSR1_4N_SHIFT                                   0
#define    BINTC_IMSR1_4_IMSR1_4N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_5_OFFSET                                               0x000000E0
#define BINTC_IMR0_5_TYPE                                                 UInt32
#define BINTC_IMR0_5_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_5_IMR0_5N_SHIFT                                     0
#define    BINTC_IMR0_5_IMR0_5N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_5_SET_OFFSET                                           0x000000E4
#define BINTC_IMR0_5_SET_TYPE                                             UInt32
#define BINTC_IMR0_5_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_5_SET_IMR0_5N_SHIFT                                 0
#define    BINTC_IMR0_5_SET_IMR0_5N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_5_CLR_OFFSET                                           0x000000E8
#define BINTC_IMR0_5_CLR_TYPE                                             UInt32
#define BINTC_IMR0_5_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_5_CLR_IMR0_5N_SHIFT                                 0
#define    BINTC_IMR0_5_CLR_IMR0_5N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_5_OFFSET                                               0x000000EC
#define BINTC_IMR1_5_TYPE                                                 UInt32
#define BINTC_IMR1_5_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_5_IMR1_5N_SHIFT                                     0
#define    BINTC_IMR1_5_IMR1_5N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_5_SET_OFFSET                                           0x000000F0
#define BINTC_IMR1_5_SET_TYPE                                             UInt32
#define BINTC_IMR1_5_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_5_SET_IMR1_5N_SHIFT                                 0
#define    BINTC_IMR1_5_SET_IMR1_5N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_5_CLR_OFFSET                                           0x000000F4
#define BINTC_IMR1_5_CLR_TYPE                                             UInt32
#define BINTC_IMR1_5_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_5_CLR_IMR1_5N_SHIFT                                 0
#define    BINTC_IMR1_5_CLR_IMR1_5N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_5_OFFSET                                              0x000000F8
#define BINTC_IMSR0_5_TYPE                                                UInt32
#define BINTC_IMSR0_5_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_5_IMSR0_5N_SHIFT                                   0
#define    BINTC_IMSR0_5_IMSR0_5N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_5_OFFSET                                              0x000000FC
#define BINTC_IMSR1_5_TYPE                                                UInt32
#define BINTC_IMSR1_5_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_5_IMSR1_5N_SHIFT                                   0
#define    BINTC_IMSR1_5_IMSR1_5N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_6_OFFSET                                               0x00000100
#define BINTC_IMR0_6_TYPE                                                 UInt32
#define BINTC_IMR0_6_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_6_IMR0_6N_SHIFT                                     0
#define    BINTC_IMR0_6_IMR0_6N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_6_SET_OFFSET                                           0x00000104
#define BINTC_IMR0_6_SET_TYPE                                             UInt32
#define BINTC_IMR0_6_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_6_SET_IMR0_6N_SHIFT                                 0
#define    BINTC_IMR0_6_SET_IMR0_6N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_6_CLR_OFFSET                                           0x00000108
#define BINTC_IMR0_6_CLR_TYPE                                             UInt32
#define BINTC_IMR0_6_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_6_CLR_IMR0_6N_SHIFT                                 0
#define    BINTC_IMR0_6_CLR_IMR0_6N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_6_OFFSET                                               0x0000010C
#define BINTC_IMR1_6_TYPE                                                 UInt32
#define BINTC_IMR1_6_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_6_IMR1_6N_SHIFT                                     0
#define    BINTC_IMR1_6_IMR1_6N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_6_SET_OFFSET                                           0x00000110
#define BINTC_IMR1_6_SET_TYPE                                             UInt32
#define BINTC_IMR1_6_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_6_SET_IMR1_6N_SHIFT                                 0
#define    BINTC_IMR1_6_SET_IMR1_6N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_6_CLR_OFFSET                                           0x00000114
#define BINTC_IMR1_6_CLR_TYPE                                             UInt32
#define BINTC_IMR1_6_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_6_CLR_IMR1_6N_SHIFT                                 0
#define    BINTC_IMR1_6_CLR_IMR1_6N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_6_OFFSET                                              0x00000118
#define BINTC_IMSR0_6_TYPE                                                UInt32
#define BINTC_IMSR0_6_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_6_IMSR0_6N_SHIFT                                   0
#define    BINTC_IMSR0_6_IMSR0_6N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_6_OFFSET                                              0x0000011C
#define BINTC_IMSR1_6_TYPE                                                UInt32
#define BINTC_IMSR1_6_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_6_IMSR1_6N_SHIFT                                   0
#define    BINTC_IMSR1_6_IMSR1_6N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_7_OFFSET                                               0x00000120
#define BINTC_IMR0_7_TYPE                                                 UInt32
#define BINTC_IMR0_7_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_7_IMR0_7N_SHIFT                                     0
#define    BINTC_IMR0_7_IMR0_7N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_7_SET_OFFSET                                           0x00000124
#define BINTC_IMR0_7_SET_TYPE                                             UInt32
#define BINTC_IMR0_7_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_7_SET_IMR0_7N_SHIFT                                 0
#define    BINTC_IMR0_7_SET_IMR0_7N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_7_CLR_OFFSET                                           0x00000128
#define BINTC_IMR0_7_CLR_TYPE                                             UInt32
#define BINTC_IMR0_7_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_7_CLR_IMR0_7N_SHIFT                                 0
#define    BINTC_IMR0_7_CLR_IMR0_7N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_7_OFFSET                                               0x0000012C
#define BINTC_IMR1_7_TYPE                                                 UInt32
#define BINTC_IMR1_7_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_7_IMR1_7N_SHIFT                                     0
#define    BINTC_IMR1_7_IMR1_7N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_7_SET_OFFSET                                           0x00000130
#define BINTC_IMR1_7_SET_TYPE                                             UInt32
#define BINTC_IMR1_7_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_7_SET_IMR1_7N_SHIFT                                 0
#define    BINTC_IMR1_7_SET_IMR1_7N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_7_CLR_OFFSET                                           0x00000134
#define BINTC_IMR1_7_CLR_TYPE                                             UInt32
#define BINTC_IMR1_7_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_7_CLR_IMR1_7N_SHIFT                                 0
#define    BINTC_IMR1_7_CLR_IMR1_7N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_7_OFFSET                                              0x00000138
#define BINTC_IMSR0_7_TYPE                                                UInt32
#define BINTC_IMSR0_7_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_7_IMSR0_7N_SHIFT                                   0
#define    BINTC_IMSR0_7_IMSR0_7N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_7_OFFSET                                              0x0000013C
#define BINTC_IMSR1_7_TYPE                                                UInt32
#define BINTC_IMSR1_7_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_7_IMSR1_7N_SHIFT                                   0
#define    BINTC_IMSR1_7_IMSR1_7N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_8_OFFSET                                               0x00000140
#define BINTC_IMR0_8_TYPE                                                 UInt32
#define BINTC_IMR0_8_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_8_IMR0_8N_SHIFT                                     0
#define    BINTC_IMR0_8_IMR0_8N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_8_SET_OFFSET                                           0x00000144
#define BINTC_IMR0_8_SET_TYPE                                             UInt32
#define BINTC_IMR0_8_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_8_SET_IMR0_8N_SHIFT                                 0
#define    BINTC_IMR0_8_SET_IMR0_8N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_8_CLR_OFFSET                                           0x00000148
#define BINTC_IMR0_8_CLR_TYPE                                             UInt32
#define BINTC_IMR0_8_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_8_CLR_IMR0_8N_SHIFT                                 0
#define    BINTC_IMR0_8_CLR_IMR0_8N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_8_OFFSET                                               0x0000014C
#define BINTC_IMR1_8_TYPE                                                 UInt32
#define BINTC_IMR1_8_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_8_IMR1_8N_SHIFT                                     0
#define    BINTC_IMR1_8_IMR1_8N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_8_SET_OFFSET                                           0x00000150
#define BINTC_IMR1_8_SET_TYPE                                             UInt32
#define BINTC_IMR1_8_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_8_SET_IMR1_8N_SHIFT                                 0
#define    BINTC_IMR1_8_SET_IMR1_8N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_8_CLR_OFFSET                                           0x00000154
#define BINTC_IMR1_8_CLR_TYPE                                             UInt32
#define BINTC_IMR1_8_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_8_CLR_IMR1_8N_SHIFT                                 0
#define    BINTC_IMR1_8_CLR_IMR1_8N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_8_OFFSET                                              0x00000158
#define BINTC_IMSR0_8_TYPE                                                UInt32
#define BINTC_IMSR0_8_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_8_IMSR0_8N_SHIFT                                   0
#define    BINTC_IMSR0_8_IMSR0_8N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_8_OFFSET                                              0x0000015C
#define BINTC_IMSR1_8_TYPE                                                UInt32
#define BINTC_IMSR1_8_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_8_IMSR1_8N_SHIFT                                   0
#define    BINTC_IMSR1_8_IMSR1_8N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_9_OFFSET                                               0x00000160
#define BINTC_IMR0_9_TYPE                                                 UInt32
#define BINTC_IMR0_9_RESERVED_MASK                                        0x00000000
#define    BINTC_IMR0_9_IMR0_9N_SHIFT                                     0
#define    BINTC_IMR0_9_IMR0_9N_MASK                                      0xFFFFFFFF

#define BINTC_IMR0_9_SET_OFFSET                                           0x00000164
#define BINTC_IMR0_9_SET_TYPE                                             UInt32
#define BINTC_IMR0_9_SET_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_9_SET_IMR0_9N_SHIFT                                 0
#define    BINTC_IMR0_9_SET_IMR0_9N_MASK                                  0xFFFFFFFF

#define BINTC_IMR0_9_CLR_OFFSET                                           0x00000168
#define BINTC_IMR0_9_CLR_TYPE                                             UInt32
#define BINTC_IMR0_9_CLR_RESERVED_MASK                                    0x00000000
#define    BINTC_IMR0_9_CLR_IMR0_9N_SHIFT                                 0
#define    BINTC_IMR0_9_CLR_IMR0_9N_MASK                                  0xFFFFFFFF

#define BINTC_IMR1_9_OFFSET                                               0x0000016C
#define BINTC_IMR1_9_TYPE                                                 UInt32
#define BINTC_IMR1_9_RESERVED_MASK                                        0xFF000000
#define    BINTC_IMR1_9_IMR1_9N_SHIFT                                     0
#define    BINTC_IMR1_9_IMR1_9N_MASK                                      0x00FFFFFF

#define BINTC_IMR1_9_SET_OFFSET                                           0x00000170
#define BINTC_IMR1_9_SET_TYPE                                             UInt32
#define BINTC_IMR1_9_SET_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_9_SET_IMR1_9N_SHIFT                                 0
#define    BINTC_IMR1_9_SET_IMR1_9N_MASK                                  0x00FFFFFF

#define BINTC_IMR1_9_CLR_OFFSET                                           0x00000174
#define BINTC_IMR1_9_CLR_TYPE                                             UInt32
#define BINTC_IMR1_9_CLR_RESERVED_MASK                                    0xFF000000
#define    BINTC_IMR1_9_CLR_IMR1_9N_SHIFT                                 0
#define    BINTC_IMR1_9_CLR_IMR1_9N_MASK                                  0x00FFFFFF

#define BINTC_IMSR0_9_OFFSET                                              0x00000178
#define BINTC_IMSR0_9_TYPE                                                UInt32
#define BINTC_IMSR0_9_RESERVED_MASK                                       0x00000000
#define    BINTC_IMSR0_9_IMSR0_9N_SHIFT                                   0
#define    BINTC_IMSR0_9_IMSR0_9N_MASK                                    0xFFFFFFFF

#define BINTC_IMSR1_9_OFFSET                                              0x0000017C
#define BINTC_IMSR1_9_TYPE                                                UInt32
#define BINTC_IMSR1_9_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMSR1_9_IMSR1_9N_SHIFT                                   0
#define    BINTC_IMSR1_9_IMSR1_9N_MASK                                    0x00FFFFFF

#define BINTC_IMR0_10_OFFSET                                              0x00000180
#define BINTC_IMR0_10_TYPE                                                UInt32
#define BINTC_IMR0_10_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_10_IMR0_10N_SHIFT                                   0
#define    BINTC_IMR0_10_IMR0_10N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_10_SET_OFFSET                                          0x00000184
#define BINTC_IMR0_10_SET_TYPE                                            UInt32
#define BINTC_IMR0_10_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_10_SET_IMR0_10N_SHIFT                               0
#define    BINTC_IMR0_10_SET_IMR0_10N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_10_CLR_OFFSET                                          0x00000188
#define BINTC_IMR0_10_CLR_TYPE                                            UInt32
#define BINTC_IMR0_10_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_10_CLR_IMR0_10N_SHIFT                               0
#define    BINTC_IMR0_10_CLR_IMR0_10N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_10_OFFSET                                              0x0000018C
#define BINTC_IMR1_10_TYPE                                                UInt32
#define BINTC_IMR1_10_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_10_IMR1_10N_SHIFT                                   0
#define    BINTC_IMR1_10_IMR1_10N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_10_SET_OFFSET                                          0x00000190
#define BINTC_IMR1_10_SET_TYPE                                            UInt32
#define BINTC_IMR1_10_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_10_SET_IMR1_10N_SHIFT                               0
#define    BINTC_IMR1_10_SET_IMR1_10N_MASK                                0x00FFFFFF

#define BINTC_IMR1_10_CLR_OFFSET                                          0x00000194
#define BINTC_IMR1_10_CLR_TYPE                                            UInt32
#define BINTC_IMR1_10_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_10_CLR_IMR1_10N_SHIFT                               0
#define    BINTC_IMR1_10_CLR_IMR1_10N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_10_OFFSET                                             0x00000198
#define BINTC_IMSR0_10_TYPE                                               UInt32
#define BINTC_IMSR0_10_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_10_IMSR0_10N_SHIFT                                 0
#define    BINTC_IMSR0_10_IMSR0_10N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_10_OFFSET                                             0x0000019C
#define BINTC_IMSR1_10_TYPE                                               UInt32
#define BINTC_IMSR1_10_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_10_IMSR1_10N_SHIFT                                 0
#define    BINTC_IMSR1_10_IMSR1_10N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_11_OFFSET                                              0x000001A0
#define BINTC_IMR0_11_TYPE                                                UInt32
#define BINTC_IMR0_11_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_11_IMR0_11N_SHIFT                                   0
#define    BINTC_IMR0_11_IMR0_11N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_11_SET_OFFSET                                          0x000001A4
#define BINTC_IMR0_11_SET_TYPE                                            UInt32
#define BINTC_IMR0_11_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_11_SET_IMR0_11N_SHIFT                               0
#define    BINTC_IMR0_11_SET_IMR0_11N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_11_CLR_OFFSET                                          0x000001A8
#define BINTC_IMR0_11_CLR_TYPE                                            UInt32
#define BINTC_IMR0_11_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_11_CLR_IMR0_11N_SHIFT                               0
#define    BINTC_IMR0_11_CLR_IMR0_11N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_11_OFFSET                                              0x000001AC
#define BINTC_IMR1_11_TYPE                                                UInt32
#define BINTC_IMR1_11_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_11_IMR1_11N_SHIFT                                   0
#define    BINTC_IMR1_11_IMR1_11N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_11_SET_OFFSET                                          0x000001B0
#define BINTC_IMR1_11_SET_TYPE                                            UInt32
#define BINTC_IMR1_11_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_11_SET_IMR1_11N_SHIFT                               0
#define    BINTC_IMR1_11_SET_IMR1_11N_MASK                                0x00FFFFFF

#define BINTC_IMR1_11_CLR_OFFSET                                          0x000001B4
#define BINTC_IMR1_11_CLR_TYPE                                            UInt32
#define BINTC_IMR1_11_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_11_CLR_IMR1_11N_SHIFT                               0
#define    BINTC_IMR1_11_CLR_IMR1_11N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_11_OFFSET                                             0x000001B8
#define BINTC_IMSR0_11_TYPE                                               UInt32
#define BINTC_IMSR0_11_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_11_IMSR0_11N_SHIFT                                 0
#define    BINTC_IMSR0_11_IMSR0_11N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_11_OFFSET                                             0x000001BC
#define BINTC_IMSR1_11_TYPE                                               UInt32
#define BINTC_IMSR1_11_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_11_IMSR1_11N_SHIFT                                 0
#define    BINTC_IMSR1_11_IMSR1_11N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_12_OFFSET                                              0x000001C0
#define BINTC_IMR0_12_TYPE                                                UInt32
#define BINTC_IMR0_12_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_12_IMR0_12N_SHIFT                                   0
#define    BINTC_IMR0_12_IMR0_12N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_12_SET_OFFSET                                          0x000001C4
#define BINTC_IMR0_12_SET_TYPE                                            UInt32
#define BINTC_IMR0_12_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_12_SET_IMR0_12N_SHIFT                               0
#define    BINTC_IMR0_12_SET_IMR0_12N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_12_CLR_OFFSET                                          0x000001C8
#define BINTC_IMR0_12_CLR_TYPE                                            UInt32
#define BINTC_IMR0_12_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_12_CLR_IMR0_12N_SHIFT                               0
#define    BINTC_IMR0_12_CLR_IMR0_12N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_12_OFFSET                                              0x000001CC
#define BINTC_IMR1_12_TYPE                                                UInt32
#define BINTC_IMR1_12_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_12_IMR1_12N_SHIFT                                   0
#define    BINTC_IMR1_12_IMR1_12N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_12_SET_OFFSET                                          0x000001D0
#define BINTC_IMR1_12_SET_TYPE                                            UInt32
#define BINTC_IMR1_12_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_12_SET_IMR1_12N_SHIFT                               0
#define    BINTC_IMR1_12_SET_IMR1_12N_MASK                                0x00FFFFFF

#define BINTC_IMR1_12_CLR_OFFSET                                          0x000001D4
#define BINTC_IMR1_12_CLR_TYPE                                            UInt32
#define BINTC_IMR1_12_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_12_CLR_IMR1_12N_SHIFT                               0
#define    BINTC_IMR1_12_CLR_IMR1_12N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_12_OFFSET                                             0x000001D8
#define BINTC_IMSR0_12_TYPE                                               UInt32
#define BINTC_IMSR0_12_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_12_IMSR0_12N_SHIFT                                 0
#define    BINTC_IMSR0_12_IMSR0_12N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_12_OFFSET                                             0x000001DC
#define BINTC_IMSR1_12_TYPE                                               UInt32
#define BINTC_IMSR1_12_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_12_IMSR1_12N_SHIFT                                 0
#define    BINTC_IMSR1_12_IMSR1_12N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_13_OFFSET                                              0x000001E0
#define BINTC_IMR0_13_TYPE                                                UInt32
#define BINTC_IMR0_13_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_13_IMR0_13N_SHIFT                                   0
#define    BINTC_IMR0_13_IMR0_13N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_13_SET_OFFSET                                          0x000001E4
#define BINTC_IMR0_13_SET_TYPE                                            UInt32
#define BINTC_IMR0_13_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_13_SET_IMR0_13N_SHIFT                               0
#define    BINTC_IMR0_13_SET_IMR0_13N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_13_CLR_OFFSET                                          0x000001E8
#define BINTC_IMR0_13_CLR_TYPE                                            UInt32
#define BINTC_IMR0_13_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_13_CLR_IMR0_13N_SHIFT                               0
#define    BINTC_IMR0_13_CLR_IMR0_13N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_13_OFFSET                                              0x000001EC
#define BINTC_IMR1_13_TYPE                                                UInt32
#define BINTC_IMR1_13_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_13_IMR1_13N_SHIFT                                   0
#define    BINTC_IMR1_13_IMR1_13N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_13_SET_OFFSET                                          0x000001F0
#define BINTC_IMR1_13_SET_TYPE                                            UInt32
#define BINTC_IMR1_13_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_13_SET_IMR1_13N_SHIFT                               0
#define    BINTC_IMR1_13_SET_IMR1_13N_MASK                                0x00FFFFFF

#define BINTC_IMR1_13_CLR_OFFSET                                          0x000001F4
#define BINTC_IMR1_13_CLR_TYPE                                            UInt32
#define BINTC_IMR1_13_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_13_CLR_IMR1_13N_SHIFT                               0
#define    BINTC_IMR1_13_CLR_IMR1_13N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_13_OFFSET                                             0x000001F8
#define BINTC_IMSR0_13_TYPE                                               UInt32
#define BINTC_IMSR0_13_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_13_IMSR0_13N_SHIFT                                 0
#define    BINTC_IMSR0_13_IMSR0_13N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_13_OFFSET                                             0x000001FC
#define BINTC_IMSR1_13_TYPE                                               UInt32
#define BINTC_IMSR1_13_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_13_IMSR1_13N_SHIFT                                 0
#define    BINTC_IMSR1_13_IMSR1_13N_MASK                                  0x00FFFFFF

#define BINTC_ISELR0_13_OFFSET                                            0x00000200
#define BINTC_ISELR0_13_TYPE                                              UInt32
#define BINTC_ISELR0_13_RESERVED_MASK                                     0x00000000
#define    BINTC_ISELR0_13_ISELR0_13N_SHIFT                               0
#define    BINTC_ISELR0_13_ISELR0_13N_MASK                                0xFFFFFFFF

#define BINTC_ISELR1_13_OFFSET                                            0x00000204
#define BINTC_ISELR1_13_TYPE                                              UInt32
#define BINTC_ISELR1_13_RESERVED_MASK                                     0xFF000000
#define    BINTC_ISELR1_13_ISELR1_13N_SHIFT                               0
#define    BINTC_ISELR1_13_ISELR1_13N_MASK                                0x00FFFFFF

#define BINTC_IMR0_15_OFFSET                                              0x00000220
#define BINTC_IMR0_15_TYPE                                                UInt32
#define BINTC_IMR0_15_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_15_IMR0_15N_SHIFT                                   0
#define    BINTC_IMR0_15_IMR0_15N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_15_SET_OFFSET                                          0x00000224
#define BINTC_IMR0_15_SET_TYPE                                            UInt32
#define BINTC_IMR0_15_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_15_SET_IMR0_15N_SHIFT                               0
#define    BINTC_IMR0_15_SET_IMR0_15N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_15_CLR_OFFSET                                          0x00000228
#define BINTC_IMR0_15_CLR_TYPE                                            UInt32
#define BINTC_IMR0_15_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_15_CLR_IMR0_15N_SHIFT                               0
#define    BINTC_IMR0_15_CLR_IMR0_15N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_15_OFFSET                                              0x0000022C
#define BINTC_IMR1_15_TYPE                                                UInt32
#define BINTC_IMR1_15_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_15_IMR1_15N_SHIFT                                   0
#define    BINTC_IMR1_15_IMR1_15N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_15_SET_OFFSET                                          0x00000230
#define BINTC_IMR1_15_SET_TYPE                                            UInt32
#define BINTC_IMR1_15_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_15_SET_IMR1_15N_SHIFT                               0
#define    BINTC_IMR1_15_SET_IMR1_15N_MASK                                0x00FFFFFF

#define BINTC_IMR1_15_CLR_OFFSET                                          0x00000234
#define BINTC_IMR1_15_CLR_TYPE                                            UInt32
#define BINTC_IMR1_15_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_15_CLR_IMR1_15N_SHIFT                               0
#define    BINTC_IMR1_15_CLR_IMR1_15N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_15_OFFSET                                             0x00000238
#define BINTC_IMSR0_15_TYPE                                               UInt32
#define BINTC_IMSR0_15_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_15_IMSR0_15N_SHIFT                                 0
#define    BINTC_IMSR0_15_IMSR0_15N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_15_OFFSET                                             0x0000023C
#define BINTC_IMSR1_15_TYPE                                               UInt32
#define BINTC_IMSR1_15_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_15_IMSR1_15N_SHIFT                                 0
#define    BINTC_IMSR1_15_IMSR1_15N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_16_OFFSET                                              0x00000240
#define BINTC_IMR0_16_TYPE                                                UInt32
#define BINTC_IMR0_16_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_16_IMR0_16N_SHIFT                                   0
#define    BINTC_IMR0_16_IMR0_16N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_16_SET_OFFSET                                          0x00000244
#define BINTC_IMR0_16_SET_TYPE                                            UInt32
#define BINTC_IMR0_16_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_16_SET_IMR0_16N_SHIFT                               0
#define    BINTC_IMR0_16_SET_IMR0_16N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_16_CLR_OFFSET                                          0x00000248
#define BINTC_IMR0_16_CLR_TYPE                                            UInt32
#define BINTC_IMR0_16_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_16_CLR_IMR0_16N_SHIFT                               0
#define    BINTC_IMR0_16_CLR_IMR0_16N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_16_OFFSET                                              0x0000024C
#define BINTC_IMR1_16_TYPE                                                UInt32
#define BINTC_IMR1_16_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_16_IMR1_16N_SHIFT                                   0
#define    BINTC_IMR1_16_IMR1_16N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_16_SET_OFFSET                                          0x00000250
#define BINTC_IMR1_16_SET_TYPE                                            UInt32
#define BINTC_IMR1_16_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_16_SET_IMR1_16N_SHIFT                               0
#define    BINTC_IMR1_16_SET_IMR1_16N_MASK                                0x00FFFFFF

#define BINTC_IMR1_16_CLR_OFFSET                                          0x00000254
#define BINTC_IMR1_16_CLR_TYPE                                            UInt32
#define BINTC_IMR1_16_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_16_CLR_IMR1_16N_SHIFT                               0
#define    BINTC_IMR1_16_CLR_IMR1_16N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_16_OFFSET                                             0x00000258
#define BINTC_IMSR0_16_TYPE                                               UInt32
#define BINTC_IMSR0_16_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_16_IMSR0_16N_SHIFT                                 0
#define    BINTC_IMSR0_16_IMSR0_16N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_16_OFFSET                                             0x0000025C
#define BINTC_IMSR1_16_TYPE                                               UInt32
#define BINTC_IMSR1_16_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_16_IMSR1_16N_SHIFT                                 0
#define    BINTC_IMSR1_16_IMSR1_16N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_17_OFFSET                                              0x00000260
#define BINTC_IMR0_17_TYPE                                                UInt32
#define BINTC_IMR0_17_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_17_IMR0_17N_SHIFT                                   0
#define    BINTC_IMR0_17_IMR0_17N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_17_SET_OFFSET                                          0x00000264
#define BINTC_IMR0_17_SET_TYPE                                            UInt32
#define BINTC_IMR0_17_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_17_SET_IMR0_17N_SHIFT                               0
#define    BINTC_IMR0_17_SET_IMR0_17N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_17_CLR_OFFSET                                          0x00000268
#define BINTC_IMR0_17_CLR_TYPE                                            UInt32
#define BINTC_IMR0_17_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_17_CLR_IMR0_17N_SHIFT                               0
#define    BINTC_IMR0_17_CLR_IMR0_17N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_17_OFFSET                                              0x0000026C
#define BINTC_IMR1_17_TYPE                                                UInt32
#define BINTC_IMR1_17_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_17_IMR1_17N_SHIFT                                   0
#define    BINTC_IMR1_17_IMR1_17N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_17_SET_OFFSET                                          0x00000270
#define BINTC_IMR1_17_SET_TYPE                                            UInt32
#define BINTC_IMR1_17_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_17_SET_IMR1_17N_SHIFT                               0
#define    BINTC_IMR1_17_SET_IMR1_17N_MASK                                0x00FFFFFF

#define BINTC_IMR1_17_CLR_OFFSET                                          0x00000274
#define BINTC_IMR1_17_CLR_TYPE                                            UInt32
#define BINTC_IMR1_17_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_17_CLR_IMR1_17N_SHIFT                               0
#define    BINTC_IMR1_17_CLR_IMR1_17N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_17_OFFSET                                             0x00000278
#define BINTC_IMSR0_17_TYPE                                               UInt32
#define BINTC_IMSR0_17_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_17_IMSR0_17N_SHIFT                                 0
#define    BINTC_IMSR0_17_IMSR0_17N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_17_OFFSET                                             0x0000027C
#define BINTC_IMSR1_17_TYPE                                               UInt32
#define BINTC_IMSR1_17_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_17_IMSR1_17N_SHIFT                                 0
#define    BINTC_IMSR1_17_IMSR1_17N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_18_OFFSET                                              0x00000280
#define BINTC_IMR0_18_TYPE                                                UInt32
#define BINTC_IMR0_18_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_18_IMR0_18N_SHIFT                                   0
#define    BINTC_IMR0_18_IMR0_18N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_18_SET_OFFSET                                          0x00000284
#define BINTC_IMR0_18_SET_TYPE                                            UInt32
#define BINTC_IMR0_18_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_18_SET_IMR0_18N_SHIFT                               0
#define    BINTC_IMR0_18_SET_IMR0_18N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_18_CLR_OFFSET                                          0x00000288
#define BINTC_IMR0_18_CLR_TYPE                                            UInt32
#define BINTC_IMR0_18_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_18_CLR_IMR0_18N_SHIFT                               0
#define    BINTC_IMR0_18_CLR_IMR0_18N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_18_OFFSET                                              0x0000028C
#define BINTC_IMR1_18_TYPE                                                UInt32
#define BINTC_IMR1_18_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_18_IMR1_18N_SHIFT                                   0
#define    BINTC_IMR1_18_IMR1_18N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_18_SET_OFFSET                                          0x00000290
#define BINTC_IMR1_18_SET_TYPE                                            UInt32
#define BINTC_IMR1_18_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_18_SET_IMR1_18N_SHIFT                               0
#define    BINTC_IMR1_18_SET_IMR1_18N_MASK                                0x00FFFFFF

#define BINTC_IMR1_18_CLR_OFFSET                                          0x00000294
#define BINTC_IMR1_18_CLR_TYPE                                            UInt32
#define BINTC_IMR1_18_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_18_CLR_IMR1_18N_SHIFT                               0
#define    BINTC_IMR1_18_CLR_IMR1_18N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_18_OFFSET                                             0x00000298
#define BINTC_IMSR0_18_TYPE                                               UInt32
#define BINTC_IMSR0_18_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_18_IMSR0_18N_SHIFT                                 0
#define    BINTC_IMSR0_18_IMSR0_18N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_18_OFFSET                                             0x0000029C
#define BINTC_IMSR1_18_TYPE                                               UInt32
#define BINTC_IMSR1_18_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_18_IMSR1_18N_SHIFT                                 0
#define    BINTC_IMSR1_18_IMSR1_18N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_19_OFFSET                                              0x000002A0
#define BINTC_IMR0_19_TYPE                                                UInt32
#define BINTC_IMR0_19_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_19_IMR0_19N_SHIFT                                   0
#define    BINTC_IMR0_19_IMR0_19N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_19_SET_OFFSET                                          0x000002A4
#define BINTC_IMR0_19_SET_TYPE                                            UInt32
#define BINTC_IMR0_19_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_19_SET_IMR0_19N_SHIFT                               0
#define    BINTC_IMR0_19_SET_IMR0_19N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_19_CLR_OFFSET                                          0x000002A8
#define BINTC_IMR0_19_CLR_TYPE                                            UInt32
#define BINTC_IMR0_19_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_19_CLR_IMR0_19N_SHIFT                               0
#define    BINTC_IMR0_19_CLR_IMR0_19N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_19_OFFSET                                              0x000002AC
#define BINTC_IMR1_19_TYPE                                                UInt32
#define BINTC_IMR1_19_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_19_IMR1_19N_SHIFT                                   0
#define    BINTC_IMR1_19_IMR1_19N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_19_SET_OFFSET                                          0x000002B0
#define BINTC_IMR1_19_SET_TYPE                                            UInt32
#define BINTC_IMR1_19_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_19_SET_IMR1_19N_SHIFT                               0
#define    BINTC_IMR1_19_SET_IMR1_19N_MASK                                0x00FFFFFF

#define BINTC_IMR1_19_CLR_OFFSET                                          0x000002B4
#define BINTC_IMR1_19_CLR_TYPE                                            UInt32
#define BINTC_IMR1_19_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_19_CLR_IMR1_19N_SHIFT                               0
#define    BINTC_IMR1_19_CLR_IMR1_19N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_19_OFFSET                                             0x000002B8
#define BINTC_IMSR0_19_TYPE                                               UInt32
#define BINTC_IMSR0_19_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_19_IMSR0_19N_SHIFT                                 0
#define    BINTC_IMSR0_19_IMSR0_19N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_19_OFFSET                                             0x000002BC
#define BINTC_IMSR1_19_TYPE                                               UInt32
#define BINTC_IMSR1_19_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_19_IMSR1_19N_SHIFT                                 0
#define    BINTC_IMSR1_19_IMSR1_19N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_20_OFFSET                                              0x000002C0
#define BINTC_IMR0_20_TYPE                                                UInt32
#define BINTC_IMR0_20_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_20_IMR0_20N_SHIFT                                   0
#define    BINTC_IMR0_20_IMR0_20N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_20_SET_OFFSET                                          0x000002C4
#define BINTC_IMR0_20_SET_TYPE                                            UInt32
#define BINTC_IMR0_20_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_20_SET_IMR0_20N_SHIFT                               0
#define    BINTC_IMR0_20_SET_IMR0_20N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_20_CLR_OFFSET                                          0x000002C8
#define BINTC_IMR0_20_CLR_TYPE                                            UInt32
#define BINTC_IMR0_20_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_20_CLR_IMR0_20N_SHIFT                               0
#define    BINTC_IMR0_20_CLR_IMR0_20N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_20_OFFSET                                              0x000002CC
#define BINTC_IMR1_20_TYPE                                                UInt32
#define BINTC_IMR1_20_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_20_IMR1_20N_SHIFT                                   0
#define    BINTC_IMR1_20_IMR1_20N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_20_SET_OFFSET                                          0x000002D0
#define BINTC_IMR1_20_SET_TYPE                                            UInt32
#define BINTC_IMR1_20_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_20_SET_IMR1_20N_SHIFT                               0
#define    BINTC_IMR1_20_SET_IMR1_20N_MASK                                0x00FFFFFF

#define BINTC_IMR1_20_CLR_OFFSET                                          0x000002D4
#define BINTC_IMR1_20_CLR_TYPE                                            UInt32
#define BINTC_IMR1_20_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_20_CLR_IMR1_20N_SHIFT                               0
#define    BINTC_IMR1_20_CLR_IMR1_20N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_20_OFFSET                                             0x000002D8
#define BINTC_IMSR0_20_TYPE                                               UInt32
#define BINTC_IMSR0_20_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_20_IMSR0_20N_SHIFT                                 0
#define    BINTC_IMSR0_20_IMSR0_20N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_20_OFFSET                                             0x000002DC
#define BINTC_IMSR1_20_TYPE                                               UInt32
#define BINTC_IMSR1_20_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_20_IMSR1_20N_SHIFT                                 0
#define    BINTC_IMSR1_20_IMSR1_20N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_21_OFFSET                                              0x000002E0
#define BINTC_IMR0_21_TYPE                                                UInt32
#define BINTC_IMR0_21_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_21_IMR0_21N_SHIFT                                   0
#define    BINTC_IMR0_21_IMR0_21N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_21_SET_OFFSET                                          0x000002E4
#define BINTC_IMR0_21_SET_TYPE                                            UInt32
#define BINTC_IMR0_21_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_21_SET_IMR0_21N_SHIFT                               0
#define    BINTC_IMR0_21_SET_IMR0_21N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_21_CLR_OFFSET                                          0x000002E8
#define BINTC_IMR0_21_CLR_TYPE                                            UInt32
#define BINTC_IMR0_21_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_21_CLR_IMR0_21N_SHIFT                               0
#define    BINTC_IMR0_21_CLR_IMR0_21N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_21_OFFSET                                              0x000002EC
#define BINTC_IMR1_21_TYPE                                                UInt32
#define BINTC_IMR1_21_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_21_IMR1_21N_SHIFT                                   0
#define    BINTC_IMR1_21_IMR1_21N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_21_SET_OFFSET                                          0x000002F0
#define BINTC_IMR1_21_SET_TYPE                                            UInt32
#define BINTC_IMR1_21_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_21_SET_IMR1_21N_SHIFT                               0
#define    BINTC_IMR1_21_SET_IMR1_21N_MASK                                0x00FFFFFF

#define BINTC_IMR1_21_CLR_OFFSET                                          0x000002F4
#define BINTC_IMR1_21_CLR_TYPE                                            UInt32
#define BINTC_IMR1_21_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_21_CLR_IMR1_21N_SHIFT                               0
#define    BINTC_IMR1_21_CLR_IMR1_21N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_21_OFFSET                                             0x000002F8
#define BINTC_IMSR0_21_TYPE                                               UInt32
#define BINTC_IMSR0_21_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_21_IMSR0_21N_SHIFT                                 0
#define    BINTC_IMSR0_21_IMSR0_21N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_21_OFFSET                                             0x000002FC
#define BINTC_IMSR1_21_TYPE                                               UInt32
#define BINTC_IMSR1_21_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_21_IMSR1_21N_SHIFT                                 0
#define    BINTC_IMSR1_21_IMSR1_21N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_22_OFFSET                                              0x00000300
#define BINTC_IMR0_22_TYPE                                                UInt32
#define BINTC_IMR0_22_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_22_IMR0_22N_SHIFT                                   0
#define    BINTC_IMR0_22_IMR0_22N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_22_SET_OFFSET                                          0x00000304
#define BINTC_IMR0_22_SET_TYPE                                            UInt32
#define BINTC_IMR0_22_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_22_SET_IMR0_22N_SHIFT                               0
#define    BINTC_IMR0_22_SET_IMR0_22N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_22_CLR_OFFSET                                          0x00000308
#define BINTC_IMR0_22_CLR_TYPE                                            UInt32
#define BINTC_IMR0_22_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_22_CLR_IMR0_22N_SHIFT                               0
#define    BINTC_IMR0_22_CLR_IMR0_22N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_22_OFFSET                                              0x0000030C
#define BINTC_IMR1_22_TYPE                                                UInt32
#define BINTC_IMR1_22_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_22_IMR1_22N_SHIFT                                   0
#define    BINTC_IMR1_22_IMR1_22N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_22_SET_OFFSET                                          0x00000310
#define BINTC_IMR1_22_SET_TYPE                                            UInt32
#define BINTC_IMR1_22_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_22_SET_IMR1_22N_SHIFT                               0
#define    BINTC_IMR1_22_SET_IMR1_22N_MASK                                0x00FFFFFF

#define BINTC_IMR1_22_CLR_OFFSET                                          0x00000314
#define BINTC_IMR1_22_CLR_TYPE                                            UInt32
#define BINTC_IMR1_22_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_22_CLR_IMR1_22N_SHIFT                               0
#define    BINTC_IMR1_22_CLR_IMR1_22N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_22_OFFSET                                             0x00000318
#define BINTC_IMSR0_22_TYPE                                               UInt32
#define BINTC_IMSR0_22_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_22_IMSR0_22N_SHIFT                                 0
#define    BINTC_IMSR0_22_IMSR0_22N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_22_OFFSET                                             0x0000031C
#define BINTC_IMSR1_22_TYPE                                               UInt32
#define BINTC_IMSR1_22_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_22_IMSR1_22N_SHIFT                                 0
#define    BINTC_IMSR1_22_IMSR1_22N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_23_OFFSET                                              0x00000320
#define BINTC_IMR0_23_TYPE                                                UInt32
#define BINTC_IMR0_23_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_23_IMR0_23N_SHIFT                                   0
#define    BINTC_IMR0_23_IMR0_23N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_23_SET_OFFSET                                          0x00000324
#define BINTC_IMR0_23_SET_TYPE                                            UInt32
#define BINTC_IMR0_23_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_23_SET_IMR0_23N_SHIFT                               0
#define    BINTC_IMR0_23_SET_IMR0_23N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_23_CLR_OFFSET                                          0x00000328
#define BINTC_IMR0_23_CLR_TYPE                                            UInt32
#define BINTC_IMR0_23_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_23_CLR_IMR0_23N_SHIFT                               0
#define    BINTC_IMR0_23_CLR_IMR0_23N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_23_OFFSET                                              0x0000032C
#define BINTC_IMR1_23_TYPE                                                UInt32
#define BINTC_IMR1_23_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_23_IMR1_23N_SHIFT                                   0
#define    BINTC_IMR1_23_IMR1_23N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_23_SET_OFFSET                                          0x00000330
#define BINTC_IMR1_23_SET_TYPE                                            UInt32
#define BINTC_IMR1_23_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_23_SET_IMR1_23N_SHIFT                               0
#define    BINTC_IMR1_23_SET_IMR1_23N_MASK                                0x00FFFFFF

#define BINTC_IMR1_23_CLR_OFFSET                                          0x00000334
#define BINTC_IMR1_23_CLR_TYPE                                            UInt32
#define BINTC_IMR1_23_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_23_CLR_IMR1_23N_SHIFT                               0
#define    BINTC_IMR1_23_CLR_IMR1_23N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_23_OFFSET                                             0x00000338
#define BINTC_IMSR0_23_TYPE                                               UInt32
#define BINTC_IMSR0_23_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_23_IMSR0_23N_SHIFT                                 0
#define    BINTC_IMSR0_23_IMSR0_23N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_23_OFFSET                                             0x0000033C
#define BINTC_IMSR1_23_TYPE                                               UInt32
#define BINTC_IMSR1_23_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_23_IMSR1_23N_SHIFT                                 0
#define    BINTC_IMSR1_23_IMSR1_23N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_24_OFFSET                                              0x00000340
#define BINTC_IMR0_24_TYPE                                                UInt32
#define BINTC_IMR0_24_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_24_IMR0_24N_SHIFT                                   0
#define    BINTC_IMR0_24_IMR0_24N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_24_SET_OFFSET                                          0x00000344
#define BINTC_IMR0_24_SET_TYPE                                            UInt32
#define BINTC_IMR0_24_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_24_SET_IMR0_24N_SHIFT                               0
#define    BINTC_IMR0_24_SET_IMR0_24N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_24_CLR_OFFSET                                          0x00000348
#define BINTC_IMR0_24_CLR_TYPE                                            UInt32
#define BINTC_IMR0_24_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_24_CLR_IMR0_24N_SHIFT                               0
#define    BINTC_IMR0_24_CLR_IMR0_24N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_24_OFFSET                                              0x0000034C
#define BINTC_IMR1_24_TYPE                                                UInt32
#define BINTC_IMR1_24_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_24_IMR1_24N_SHIFT                                   0
#define    BINTC_IMR1_24_IMR1_24N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_24_SET_OFFSET                                          0x00000350
#define BINTC_IMR1_24_SET_TYPE                                            UInt32
#define BINTC_IMR1_24_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_24_SET_IMR1_24N_SHIFT                               0
#define    BINTC_IMR1_24_SET_IMR1_24N_MASK                                0x00FFFFFF

#define BINTC_IMR1_24_CLR_OFFSET                                          0x00000354
#define BINTC_IMR1_24_CLR_TYPE                                            UInt32
#define BINTC_IMR1_24_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_24_CLR_IMR1_24N_SHIFT                               0
#define    BINTC_IMR1_24_CLR_IMR1_24N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_24_OFFSET                                             0x00000358
#define BINTC_IMSR0_24_TYPE                                               UInt32
#define BINTC_IMSR0_24_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_24_IMSR0_24N_SHIFT                                 0
#define    BINTC_IMSR0_24_IMSR0_24N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_24_OFFSET                                             0x0000035C
#define BINTC_IMSR1_24_TYPE                                               UInt32
#define BINTC_IMSR1_24_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_24_IMSR1_24N_SHIFT                                 0
#define    BINTC_IMSR1_24_IMSR1_24N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_25_OFFSET                                              0x00000360
#define BINTC_IMR0_25_TYPE                                                UInt32
#define BINTC_IMR0_25_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_25_IMR0_25N_SHIFT                                   0
#define    BINTC_IMR0_25_IMR0_25N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_25_SET_OFFSET                                          0x00000364
#define BINTC_IMR0_25_SET_TYPE                                            UInt32
#define BINTC_IMR0_25_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_25_SET_IMR0_25N_SHIFT                               0
#define    BINTC_IMR0_25_SET_IMR0_25N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_25_CLR_OFFSET                                          0x00000368
#define BINTC_IMR0_25_CLR_TYPE                                            UInt32
#define BINTC_IMR0_25_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_25_CLR_IMR0_25N_SHIFT                               0
#define    BINTC_IMR0_25_CLR_IMR0_25N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_25_OFFSET                                              0x0000036C
#define BINTC_IMR1_25_TYPE                                                UInt32
#define BINTC_IMR1_25_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_25_IMR1_25N_SHIFT                                   0
#define    BINTC_IMR1_25_IMR1_25N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_25_SET_OFFSET                                          0x00000370
#define BINTC_IMR1_25_SET_TYPE                                            UInt32
#define BINTC_IMR1_25_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_25_SET_IMR1_25N_SHIFT                               0
#define    BINTC_IMR1_25_SET_IMR1_25N_MASK                                0x00FFFFFF

#define BINTC_IMR1_25_CLR_OFFSET                                          0x00000374
#define BINTC_IMR1_25_CLR_TYPE                                            UInt32
#define BINTC_IMR1_25_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_25_CLR_IMR1_25N_SHIFT                               0
#define    BINTC_IMR1_25_CLR_IMR1_25N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_25_OFFSET                                             0x00000378
#define BINTC_IMSR0_25_TYPE                                               UInt32
#define BINTC_IMSR0_25_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_25_IMSR0_25N_SHIFT                                 0
#define    BINTC_IMSR0_25_IMSR0_25N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_25_OFFSET                                             0x0000037C
#define BINTC_IMSR1_25_TYPE                                               UInt32
#define BINTC_IMSR1_25_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_25_IMSR1_25N_SHIFT                                 0
#define    BINTC_IMSR1_25_IMSR1_25N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_26_OFFSET                                              0x00000380
#define BINTC_IMR0_26_TYPE                                                UInt32
#define BINTC_IMR0_26_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_26_IMR0_26N_SHIFT                                   0
#define    BINTC_IMR0_26_IMR0_26N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_26_SET_OFFSET                                          0x00000384
#define BINTC_IMR0_26_SET_TYPE                                            UInt32
#define BINTC_IMR0_26_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_26_SET_IMR0_26N_SHIFT                               0
#define    BINTC_IMR0_26_SET_IMR0_26N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_26_CLR_OFFSET                                          0x00000388
#define BINTC_IMR0_26_CLR_TYPE                                            UInt32
#define BINTC_IMR0_26_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_26_CLR_IMR0_26N_SHIFT                               0
#define    BINTC_IMR0_26_CLR_IMR0_26N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_26_OFFSET                                              0x0000038C
#define BINTC_IMR1_26_TYPE                                                UInt32
#define BINTC_IMR1_26_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_26_IMR1_26N_SHIFT                                   0
#define    BINTC_IMR1_26_IMR1_26N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_26_SET_OFFSET                                          0x00000390
#define BINTC_IMR1_26_SET_TYPE                                            UInt32
#define BINTC_IMR1_26_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_26_SET_IMR1_26N_SHIFT                               0
#define    BINTC_IMR1_26_SET_IMR1_26N_MASK                                0x00FFFFFF

#define BINTC_IMR1_26_CLR_OFFSET                                          0x00000394
#define BINTC_IMR1_26_CLR_TYPE                                            UInt32
#define BINTC_IMR1_26_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_26_CLR_IMR1_26N_SHIFT                               0
#define    BINTC_IMR1_26_CLR_IMR1_26N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_26_OFFSET                                             0x00000398
#define BINTC_IMSR0_26_TYPE                                               UInt32
#define BINTC_IMSR0_26_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_26_IMSR0_26N_SHIFT                                 0
#define    BINTC_IMSR0_26_IMSR0_26N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_26_OFFSET                                             0x0000039C
#define BINTC_IMSR1_26_TYPE                                               UInt32
#define BINTC_IMSR1_26_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_26_IMSR1_26N_SHIFT                                 0
#define    BINTC_IMSR1_26_IMSR1_26N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_27_OFFSET                                              0x000003A0
#define BINTC_IMR0_27_TYPE                                                UInt32
#define BINTC_IMR0_27_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_27_IMR0_27N_SHIFT                                   0
#define    BINTC_IMR0_27_IMR0_27N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_27_SET_OFFSET                                          0x000003A4
#define BINTC_IMR0_27_SET_TYPE                                            UInt32
#define BINTC_IMR0_27_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_27_SET_IMR0_27N_SHIFT                               0
#define    BINTC_IMR0_27_SET_IMR0_27N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_27_CLR_OFFSET                                          0x000003A8
#define BINTC_IMR0_27_CLR_TYPE                                            UInt32
#define BINTC_IMR0_27_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_27_CLR_IMR0_27N_SHIFT                               0
#define    BINTC_IMR0_27_CLR_IMR0_27N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_27_OFFSET                                              0x000003AC
#define BINTC_IMR1_27_TYPE                                                UInt32
#define BINTC_IMR1_27_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_27_IMR1_27N_SHIFT                                   0
#define    BINTC_IMR1_27_IMR1_27N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_27_SET_OFFSET                                          0x000003B0
#define BINTC_IMR1_27_SET_TYPE                                            UInt32
#define BINTC_IMR1_27_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_27_SET_IMR1_27N_SHIFT                               0
#define    BINTC_IMR1_27_SET_IMR1_27N_MASK                                0x00FFFFFF

#define BINTC_IMR1_27_CLR_OFFSET                                          0x000003B4
#define BINTC_IMR1_27_CLR_TYPE                                            UInt32
#define BINTC_IMR1_27_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_27_CLR_IMR1_27N_SHIFT                               0
#define    BINTC_IMR1_27_CLR_IMR1_27N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_27_OFFSET                                             0x000003B8
#define BINTC_IMSR0_27_TYPE                                               UInt32
#define BINTC_IMSR0_27_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_27_IMSR0_27N_SHIFT                                 0
#define    BINTC_IMSR0_27_IMSR0_27N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_27_OFFSET                                             0x000003BC
#define BINTC_IMSR1_27_TYPE                                               UInt32
#define BINTC_IMSR1_27_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_27_IMSR1_27N_SHIFT                                 0
#define    BINTC_IMSR1_27_IMSR1_27N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_28_OFFSET                                              0x000003C0
#define BINTC_IMR0_28_TYPE                                                UInt32
#define BINTC_IMR0_28_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_28_IMR0_28N_SHIFT                                   0
#define    BINTC_IMR0_28_IMR0_28N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_28_SET_OFFSET                                          0x000003C4
#define BINTC_IMR0_28_SET_TYPE                                            UInt32
#define BINTC_IMR0_28_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_28_SET_IMR0_28N_SHIFT                               0
#define    BINTC_IMR0_28_SET_IMR0_28N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_28_CLR_OFFSET                                          0x000003C8
#define BINTC_IMR0_28_CLR_TYPE                                            UInt32
#define BINTC_IMR0_28_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_28_CLR_IMR0_28N_SHIFT                               0
#define    BINTC_IMR0_28_CLR_IMR0_28N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_28_OFFSET                                              0x000003CC
#define BINTC_IMR1_28_TYPE                                                UInt32
#define BINTC_IMR1_28_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_28_IMR1_28N_SHIFT                                   0
#define    BINTC_IMR1_28_IMR1_28N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_28_SET_OFFSET                                          0x000003D0
#define BINTC_IMR1_28_SET_TYPE                                            UInt32
#define BINTC_IMR1_28_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_28_SET_IMR1_28N_SHIFT                               0
#define    BINTC_IMR1_28_SET_IMR1_28N_MASK                                0x00FFFFFF

#define BINTC_IMR1_28_CLR_OFFSET                                          0x000003D4
#define BINTC_IMR1_28_CLR_TYPE                                            UInt32
#define BINTC_IMR1_28_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_28_CLR_IMR1_28N_SHIFT                               0
#define    BINTC_IMR1_28_CLR_IMR1_28N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_28_OFFSET                                             0x000003D8
#define BINTC_IMSR0_28_TYPE                                               UInt32
#define BINTC_IMSR0_28_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_28_IMSR0_28N_SHIFT                                 0
#define    BINTC_IMSR0_28_IMSR0_28N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_28_OFFSET                                             0x000003DC
#define BINTC_IMSR1_28_TYPE                                               UInt32
#define BINTC_IMSR1_28_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_28_IMSR1_28N_SHIFT                                 0
#define    BINTC_IMSR1_28_IMSR1_28N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_29_OFFSET                                              0x000003E0
#define BINTC_IMR0_29_TYPE                                                UInt32
#define BINTC_IMR0_29_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_29_IMR0_29N_SHIFT                                   0
#define    BINTC_IMR0_29_IMR0_29N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_29_SET_OFFSET                                          0x000003E4
#define BINTC_IMR0_29_SET_TYPE                                            UInt32
#define BINTC_IMR0_29_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_29_SET_IMR0_29N_SHIFT                               0
#define    BINTC_IMR0_29_SET_IMR0_29N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_29_CLR_OFFSET                                          0x000003E8
#define BINTC_IMR0_29_CLR_TYPE                                            UInt32
#define BINTC_IMR0_29_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_29_CLR_IMR0_29N_SHIFT                               0
#define    BINTC_IMR0_29_CLR_IMR0_29N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_29_OFFSET                                              0x000003EC
#define BINTC_IMR1_29_TYPE                                                UInt32
#define BINTC_IMR1_29_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_29_IMR1_29N_SHIFT                                   0
#define    BINTC_IMR1_29_IMR1_29N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_29_SET_OFFSET                                          0x000003F0
#define BINTC_IMR1_29_SET_TYPE                                            UInt32
#define BINTC_IMR1_29_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_29_SET_IMR1_29N_SHIFT                               0
#define    BINTC_IMR1_29_SET_IMR1_29N_MASK                                0x00FFFFFF

#define BINTC_IMR1_29_CLR_OFFSET                                          0x000003F4
#define BINTC_IMR1_29_CLR_TYPE                                            UInt32
#define BINTC_IMR1_29_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_29_CLR_IMR1_29N_SHIFT                               0
#define    BINTC_IMR1_29_CLR_IMR1_29N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_29_OFFSET                                             0x000003F8
#define BINTC_IMSR0_29_TYPE                                               UInt32
#define BINTC_IMSR0_29_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_29_IMSR0_29N_SHIFT                                 0
#define    BINTC_IMSR0_29_IMSR0_29N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_29_OFFSET                                             0x000003FC
#define BINTC_IMSR1_29_TYPE                                               UInt32
#define BINTC_IMSR1_29_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_29_IMSR1_29N_SHIFT                                 0
#define    BINTC_IMSR1_29_IMSR1_29N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_30_OFFSET                                              0x00000400
#define BINTC_IMR0_30_TYPE                                                UInt32
#define BINTC_IMR0_30_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_30_IMR0_30N_SHIFT                                   0
#define    BINTC_IMR0_30_IMR0_30N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_30_SET_OFFSET                                          0x00000404
#define BINTC_IMR0_30_SET_TYPE                                            UInt32
#define BINTC_IMR0_30_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_30_SET_IMR0_30N_SHIFT                               0
#define    BINTC_IMR0_30_SET_IMR0_30N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_30_CLR_OFFSET                                          0x00000408
#define BINTC_IMR0_30_CLR_TYPE                                            UInt32
#define BINTC_IMR0_30_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_30_CLR_IMR0_30N_SHIFT                               0
#define    BINTC_IMR0_30_CLR_IMR0_30N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_30_OFFSET                                              0x0000040C
#define BINTC_IMR1_30_TYPE                                                UInt32
#define BINTC_IMR1_30_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_30_IMR1_30N_SHIFT                                   0
#define    BINTC_IMR1_30_IMR1_30N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_30_SET_OFFSET                                          0x00000410
#define BINTC_IMR1_30_SET_TYPE                                            UInt32
#define BINTC_IMR1_30_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_30_SET_IMR1_30N_SHIFT                               0
#define    BINTC_IMR1_30_SET_IMR1_30N_MASK                                0x00FFFFFF

#define BINTC_IMR1_30_CLR_OFFSET                                          0x00000414
#define BINTC_IMR1_30_CLR_TYPE                                            UInt32
#define BINTC_IMR1_30_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_30_CLR_IMR1_30N_SHIFT                               0
#define    BINTC_IMR1_30_CLR_IMR1_30N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_30_OFFSET                                             0x00000418
#define BINTC_IMSR0_30_TYPE                                               UInt32
#define BINTC_IMSR0_30_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_30_IMSR0_30N_SHIFT                                 0
#define    BINTC_IMSR0_30_IMSR0_30N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_30_OFFSET                                             0x0000041C
#define BINTC_IMSR1_30_TYPE                                               UInt32
#define BINTC_IMSR1_30_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_30_IMSR1_30N_SHIFT                                 0
#define    BINTC_IMSR1_30_IMSR1_30N_MASK                                  0x00FFFFFF

#define BINTC_IMR0_31_OFFSET                                              0x00000420
#define BINTC_IMR0_31_TYPE                                                UInt32
#define BINTC_IMR0_31_RESERVED_MASK                                       0x00000000
#define    BINTC_IMR0_31_IMR0_31N_SHIFT                                   0
#define    BINTC_IMR0_31_IMR0_31N_MASK                                    0xFFFFFFFF

#define BINTC_IMR0_31_SET_OFFSET                                          0x00000424
#define BINTC_IMR0_31_SET_TYPE                                            UInt32
#define BINTC_IMR0_31_SET_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_31_SET_IMR0_31N_SHIFT                               0
#define    BINTC_IMR0_31_SET_IMR0_31N_MASK                                0xFFFFFFFF

#define BINTC_IMR0_31_CLR_OFFSET                                          0x00000428
#define BINTC_IMR0_31_CLR_TYPE                                            UInt32
#define BINTC_IMR0_31_CLR_RESERVED_MASK                                   0x00000000
#define    BINTC_IMR0_31_CLR_IMR0_31N_SHIFT                               0
#define    BINTC_IMR0_31_CLR_IMR0_31N_MASK                                0xFFFFFFFF

#define BINTC_IMR1_31_OFFSET                                              0x0000042C
#define BINTC_IMR1_31_TYPE                                                UInt32
#define BINTC_IMR1_31_RESERVED_MASK                                       0xFF000000
#define    BINTC_IMR1_31_IMR1_31N_SHIFT                                   0
#define    BINTC_IMR1_31_IMR1_31N_MASK                                    0x00FFFFFF

#define BINTC_IMR1_31_SET_OFFSET                                          0x00000430
#define BINTC_IMR1_31_SET_TYPE                                            UInt32
#define BINTC_IMR1_31_SET_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_31_SET_IMR1_31N_SHIFT                               0
#define    BINTC_IMR1_31_SET_IMR1_31N_MASK                                0x00FFFFFF

#define BINTC_IMR1_31_CLR_OFFSET                                          0x00000434
#define BINTC_IMR1_31_CLR_TYPE                                            UInt32
#define BINTC_IMR1_31_CLR_RESERVED_MASK                                   0xFF000000
#define    BINTC_IMR1_31_CLR_IMR1_31N_SHIFT                               0
#define    BINTC_IMR1_31_CLR_IMR1_31N_MASK                                0x00FFFFFF

#define BINTC_IMSR0_31_OFFSET                                             0x00000438
#define BINTC_IMSR0_31_TYPE                                               UInt32
#define BINTC_IMSR0_31_RESERVED_MASK                                      0x00000000
#define    BINTC_IMSR0_31_IMSR0_31N_SHIFT                                 0
#define    BINTC_IMSR0_31_IMSR0_31N_MASK                                  0xFFFFFFFF

#define BINTC_IMSR1_31_OFFSET                                             0x0000043C
#define BINTC_IMSR1_31_TYPE                                               UInt32
#define BINTC_IMSR1_31_RESERVED_MASK                                      0xFF000000
#define    BINTC_IMSR1_31_IMSR1_31N_SHIFT                                 0
#define    BINTC_IMSR1_31_IMSR1_31N_MASK                                  0x00FFFFFF

#endif /* __BRCM_RDB_BINTC_H__ */



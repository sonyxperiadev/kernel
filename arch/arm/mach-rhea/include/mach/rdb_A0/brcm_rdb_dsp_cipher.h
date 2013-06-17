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

#ifndef __BRCM_RDB_DSP_CIPHER_H__
#define __BRCM_RDB_DSP_CIPHER_H__

#define DSP_CIPHER_CRYKEY_R0_OFFSET                                       0x00000200
#define DSP_CIPHER_CRYKEY_R0_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R0_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R0_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R0_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R1_OFFSET                                       0x00000202
#define DSP_CIPHER_CRYKEY_R1_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R1_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R1_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R1_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R2_OFFSET                                       0x00000204
#define DSP_CIPHER_CRYKEY_R2_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R2_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R2_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R2_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R3_OFFSET                                       0x00000206
#define DSP_CIPHER_CRYKEY_R3_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R3_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R3_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R3_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R4_OFFSET                                       0x00000208
#define DSP_CIPHER_CRYKEY_R4_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R4_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R4_CRYKEY_4_SHIFT                            0
#define    DSP_CIPHER_CRYKEY_R4_CRYKEY_4_MASK                             0x0000FFFF

#define DSP_CIPHER_CRYKEY_R5_OFFSET                                       0x0000020A
#define DSP_CIPHER_CRYKEY_R5_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R5_RESERVED_MASK                                0x00000FC0
#define    DSP_CIPHER_CRYKEY_R5_BET_SHIFT                                 15
#define    DSP_CIPHER_CRYKEY_R5_BET_MASK                                  0x00008000
#define    DSP_CIPHER_CRYKEY_R5_A512_SHIFT                                14
#define    DSP_CIPHER_CRYKEY_R5_A512_MASK                                 0x00004000
#define    DSP_CIPHER_CRYKEY_R5_MODE_SHIFT                                13
#define    DSP_CIPHER_CRYKEY_R5_MODE_MASK                                 0x00002000
#define    DSP_CIPHER_CRYKEY_R5_A53_SHIFT                                 12
#define    DSP_CIPHER_CRYKEY_R5_A53_MASK                                  0x00001000
#define    DSP_CIPHER_CRYKEY_R5_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R5_CRYKEY_MASK                               0x0000003F

#define DSP_CIPHER_CRYKEY_R6_OFFSET                                       0x0000020C
#define DSP_CIPHER_CRYKEY_R6_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R6_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R6_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R6_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R7_OFFSET                                       0x0000020E
#define DSP_CIPHER_CRYKEY_R7_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R7_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R7_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R7_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R8_OFFSET                                       0x00000210
#define DSP_CIPHER_CRYKEY_R8_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R8_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R8_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R8_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYKEY_R9_OFFSET                                       0x00000212
#define DSP_CIPHER_CRYKEY_R9_TYPE                                         UInt16
#define DSP_CIPHER_CRYKEY_R9_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYKEY_R9_CRYKEY_SHIFT                              0
#define    DSP_CIPHER_CRYKEY_R9_CRYKEY_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R0_OFFSET                                       0x00000220
#define DSP_CIPHER_CRYDAT_R0_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R0_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R0_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R0_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R1_OFFSET                                       0x00000222
#define DSP_CIPHER_CRYDAT_R1_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R1_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R1_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R1_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R2_OFFSET                                       0x00000224
#define DSP_CIPHER_CRYDAT_R2_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R2_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R2_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R2_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R3_OFFSET                                       0x00000226
#define DSP_CIPHER_CRYDAT_R3_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R3_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R3_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R3_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R4_OFFSET                                       0x00000228
#define DSP_CIPHER_CRYDAT_R4_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R4_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R4_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R4_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R5_OFFSET                                       0x0000022A
#define DSP_CIPHER_CRYDAT_R5_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R5_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R5_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R5_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R6_OFFSET                                       0x0000022C
#define DSP_CIPHER_CRYDAT_R6_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R6_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R6_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R6_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R7_OFFSET                                       0x0000022E
#define DSP_CIPHER_CRYDAT_R7_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R7_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R7_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R7_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R8_OFFSET                                       0x00000230
#define DSP_CIPHER_CRYDAT_R8_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R8_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R8_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R8_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R9_OFFSET                                       0x00000232
#define DSP_CIPHER_CRYDAT_R9_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_R9_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_R9_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_R9_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_R10_OFFSET                                      0x00000234
#define DSP_CIPHER_CRYDAT_R10_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R10_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R10_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R10_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R11_OFFSET                                      0x00000236
#define DSP_CIPHER_CRYDAT_R11_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R11_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R11_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R11_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R12_OFFSET                                      0x00000238
#define DSP_CIPHER_CRYDAT_R12_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R12_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R12_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R12_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R13_OFFSET                                      0x0000023A
#define DSP_CIPHER_CRYDAT_R13_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R13_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R13_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R13_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R14_OFFSET                                      0x0000023C
#define DSP_CIPHER_CRYDAT_R14_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R14_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R14_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R14_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R15_OFFSET                                      0x0000023E
#define DSP_CIPHER_CRYDAT_R15_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R15_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R15_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R15_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R16_OFFSET                                      0x00000240
#define DSP_CIPHER_CRYDAT_R16_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R16_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R16_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R16_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R17_OFFSET                                      0x00000242
#define DSP_CIPHER_CRYDAT_R17_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R17_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R17_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R17_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R18_OFFSET                                      0x00000244
#define DSP_CIPHER_CRYDAT_R18_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R18_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R18_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R18_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R19_OFFSET                                      0x00000246
#define DSP_CIPHER_CRYDAT_R19_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R19_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R19_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R19_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R20_OFFSET                                      0x00000248
#define DSP_CIPHER_CRYDAT_R20_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R20_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R20_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R20_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYDAT_R21_OFFSET                                      0x0000024A
#define DSP_CIPHER_CRYDAT_R21_TYPE                                        UInt16
#define DSP_CIPHER_CRYDAT_R21_RESERVED_MASK                               0x00000000
#define    DSP_CIPHER_CRYDAT_R21_CRYDAT_SHIFT                             0
#define    DSP_CIPHER_CRYDAT_R21_CRYDAT_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYKEY_0_OFFSET                                        0x0000E100
#define DSP_CIPHER_CRYKEY_0_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_0_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_0_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_0_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_1_OFFSET                                        0x0000E101
#define DSP_CIPHER_CRYKEY_1_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_1_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_1_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_1_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_2_OFFSET                                        0x0000E102
#define DSP_CIPHER_CRYKEY_2_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_2_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_2_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_2_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_3_OFFSET                                        0x0000E103
#define DSP_CIPHER_CRYKEY_3_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_3_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_3_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_3_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_4_OFFSET                                        0x0000E104
#define DSP_CIPHER_CRYKEY_4_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_4_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_4_CRYKEY_4_SHIFT                             0
#define    DSP_CIPHER_CRYKEY_4_CRYKEY_4_MASK                              0x0000FFFF

#define DSP_CIPHER_CRYKEY_5_OFFSET                                        0x0000E105
#define DSP_CIPHER_CRYKEY_5_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_5_RESERVED_MASK                                 0x00000FC0
#define    DSP_CIPHER_CRYKEY_5_BET_SHIFT                                  15
#define    DSP_CIPHER_CRYKEY_5_BET_MASK                                   0x00008000
#define    DSP_CIPHER_CRYKEY_5_A512_SHIFT                                 14
#define    DSP_CIPHER_CRYKEY_5_A512_MASK                                  0x00004000
#define    DSP_CIPHER_CRYKEY_5_MODE_SHIFT                                 13
#define    DSP_CIPHER_CRYKEY_5_MODE_MASK                                  0x00002000
#define    DSP_CIPHER_CRYKEY_5_A53_SHIFT                                  12
#define    DSP_CIPHER_CRYKEY_5_A53_MASK                                   0x00001000
#define    DSP_CIPHER_CRYKEY_5_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_5_CRYKEY_MASK                                0x0000003F

#define DSP_CIPHER_CRYKEY_6_OFFSET                                        0x0000E106
#define DSP_CIPHER_CRYKEY_6_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_6_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_6_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_6_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_7_OFFSET                                        0x0000E107
#define DSP_CIPHER_CRYKEY_7_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_7_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_7_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_7_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_8_OFFSET                                        0x0000E108
#define DSP_CIPHER_CRYKEY_8_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_8_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_8_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_8_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYKEY_9_OFFSET                                        0x0000E109
#define DSP_CIPHER_CRYKEY_9_TYPE                                          UInt16
#define DSP_CIPHER_CRYKEY_9_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYKEY_9_CRYKEY_SHIFT                               0
#define    DSP_CIPHER_CRYKEY_9_CRYKEY_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_0_OFFSET                                        0x0000E110
#define DSP_CIPHER_CRYDAT_0_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_0_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_0_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_0_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_1_OFFSET                                        0x0000E111
#define DSP_CIPHER_CRYDAT_1_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_1_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_1_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_1_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_2_OFFSET                                        0x0000E112
#define DSP_CIPHER_CRYDAT_2_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_2_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_2_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_2_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_3_OFFSET                                        0x0000E113
#define DSP_CIPHER_CRYDAT_3_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_3_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_3_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_3_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_4_OFFSET                                        0x0000E114
#define DSP_CIPHER_CRYDAT_4_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_4_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_4_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_4_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_5_OFFSET                                        0x0000E115
#define DSP_CIPHER_CRYDAT_5_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_5_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_5_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_5_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_6_OFFSET                                        0x0000E116
#define DSP_CIPHER_CRYDAT_6_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_6_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_6_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_6_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_7_OFFSET                                        0x0000E117
#define DSP_CIPHER_CRYDAT_7_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_7_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_7_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_7_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_8_OFFSET                                        0x0000E118
#define DSP_CIPHER_CRYDAT_8_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_8_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_8_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_8_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_9_OFFSET                                        0x0000E119
#define DSP_CIPHER_CRYDAT_9_TYPE                                          UInt16
#define DSP_CIPHER_CRYDAT_9_RESERVED_MASK                                 0x00000000
#define    DSP_CIPHER_CRYDAT_9_CRYDAT_SHIFT                               0
#define    DSP_CIPHER_CRYDAT_9_CRYDAT_MASK                                0x0000FFFF

#define DSP_CIPHER_CRYDAT_10_OFFSET                                       0x0000E11A
#define DSP_CIPHER_CRYDAT_10_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_10_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_10_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_10_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_11_OFFSET                                       0x0000E11B
#define DSP_CIPHER_CRYDAT_11_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_11_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_11_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_11_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_12_OFFSET                                       0x0000E11C
#define DSP_CIPHER_CRYDAT_12_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_12_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_12_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_12_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_13_OFFSET                                       0x0000E11D
#define DSP_CIPHER_CRYDAT_13_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_13_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_13_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_13_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_14_OFFSET                                       0x0000E11E
#define DSP_CIPHER_CRYDAT_14_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_14_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_14_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_14_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_15_OFFSET                                       0x0000E11F
#define DSP_CIPHER_CRYDAT_15_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_15_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_15_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_15_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_16_OFFSET                                       0x0000E120
#define DSP_CIPHER_CRYDAT_16_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_16_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_16_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_16_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_17_OFFSET                                       0x0000E121
#define DSP_CIPHER_CRYDAT_17_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_17_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_17_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_17_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_18_OFFSET                                       0x0000E122
#define DSP_CIPHER_CRYDAT_18_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_18_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_18_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_18_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_19_OFFSET                                       0x0000E123
#define DSP_CIPHER_CRYDAT_19_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_19_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_19_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_19_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_20_OFFSET                                       0x0000E124
#define DSP_CIPHER_CRYDAT_20_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_20_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_20_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_20_CRYDAT_MASK                               0x0000FFFF

#define DSP_CIPHER_CRYDAT_21_OFFSET                                       0x0000E125
#define DSP_CIPHER_CRYDAT_21_TYPE                                         UInt16
#define DSP_CIPHER_CRYDAT_21_RESERVED_MASK                                0x00000000
#define    DSP_CIPHER_CRYDAT_21_CRYDAT_SHIFT                              0
#define    DSP_CIPHER_CRYDAT_21_CRYDAT_MASK                               0x0000FFFF

#endif /* __BRCM_RDB_DSP_CIPHER_H__ */



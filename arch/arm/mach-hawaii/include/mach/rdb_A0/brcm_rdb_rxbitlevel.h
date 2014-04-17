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

#ifndef __BRCM_RDB_RXBITLEVEL_H__
#define __BRCM_RDB_RXBITLEVEL_H__

#define RXBITLEVEL_TRCHPARAMETERS1_OFFSET                                 0x00000000
#define RXBITLEVEL_TRCHPARAMETERS1_TYPE                                   UInt32
#define RXBITLEVEL_TRCHPARAMETERS1_RESERVED_MASK                          0x00CCE8E8
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_TB_LEN_RH_64_SHIFT 24
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_TB_LEN_RH_64_MASK 0xFF000000
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_VP_GAIN_SHIFT 20
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_VP_GAIN_MASK 0x00300000
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_BS_ENABLE_SHIFT 17
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_BS_ENABLE_MASK 0x00020000
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_SM_MEASURE_ON_SHIFT 16
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_VITERBI_SM_MEASURE_ON_MASK 0x00010000
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_RMPUNCTURE_SHIFT     12
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_RMPUNCTURE_MASK      0x00001000
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_INV_REPEAT_GAIN_SHIFT 8
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_INV_REPEAT_GAIN_MASK 0x00000700
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_STRIPCRC_SHIFT       4
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_STRIPCRC_MASK        0x00000010
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_MSTREAM_EN_SHIFT     2
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_MSTREAM_EN_MASK      0x00000004
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_DECODINGTYPE_SHIFT   0
#define    RXBITLEVEL_TRCHPARAMETERS1_TRCHPARAMETERS_DECODINGTYPE_MASK    0x00000003

#define RXBITLEVEL_CRCSIZE_OFFSET                                         0x00000004
#define RXBITLEVEL_CRCSIZE_TYPE                                           UInt32
#define RXBITLEVEL_CRCSIZE_RESERVED_MASK                                  0xFFFFFFF8
#define    RXBITLEVEL_CRCSIZE_TRCHPARAMETERS_CRCSIZE_SHIFT                0
#define    RXBITLEVEL_CRCSIZE_TRCHPARAMETERS_CRCSIZE_MASK                 0x00000007

#define RXBITLEVEL_TRBLOCKLENGTH_OFFSET                                   0x00000008
#define RXBITLEVEL_TRBLOCKLENGTH_TYPE                                     UInt32
#define RXBITLEVEL_TRBLOCKLENGTH_RESERVED_MASK                            0xFFFF0000
#define    RXBITLEVEL_TRBLOCKLENGTH_TRCHPARAMETERS_TRBLOCKLENGTH_SHIFT    0
#define    RXBITLEVEL_TRBLOCKLENGTH_TRCHPARAMETERS_TRBLOCKLENGTH_MASK     0x0000FFFF

#define RXBITLEVEL_PARITY_A_EINIT_OFFSET                                  0x0000000C
#define RXBITLEVEL_PARITY_A_EINIT_TYPE                                    UInt32
#define RXBITLEVEL_PARITY_A_EINIT_RESERVED_MASK                           0xFFF80000
#define    RXBITLEVEL_PARITY_A_EINIT_TRCHPARAMETERS_PARITY_A_EINIT_SHIFT  0
#define    RXBITLEVEL_PARITY_A_EINIT_TRCHPARAMETERS_PARITY_A_EINIT_MASK   0x0007FFFF

#define RXBITLEVEL_PARITY_A_EPLUS_OFFSET                                  0x00000010
#define RXBITLEVEL_PARITY_A_EPLUS_TYPE                                    UInt32
#define RXBITLEVEL_PARITY_A_EPLUS_RESERVED_MASK                           0xFFF80000
#define    RXBITLEVEL_PARITY_A_EPLUS_TRCHPARAMETERS_PARITY_A_EPLUS_SHIFT  0
#define    RXBITLEVEL_PARITY_A_EPLUS_TRCHPARAMETERS_PARITY_A_EPLUS_MASK   0x0007FFFF

#define RXBITLEVEL_PARITY_A_EMINUS_OFFSET                                 0x00000014
#define RXBITLEVEL_PARITY_A_EMINUS_TYPE                                   UInt32
#define RXBITLEVEL_PARITY_A_EMINUS_RESERVED_MASK                          0xFFF80000
#define    RXBITLEVEL_PARITY_A_EMINUS_TRCHPARAMETERS_PARITY_A_EMINUS_SHIFT 0
#define    RXBITLEVEL_PARITY_A_EMINUS_TRCHPARAMETERS_PARITY_A_EMINUS_MASK 0x0007FFFF

#define RXBITLEVEL_PARITY_B_EINIT_OFFSET                                  0x00000018
#define RXBITLEVEL_PARITY_B_EINIT_TYPE                                    UInt32
#define RXBITLEVEL_PARITY_B_EINIT_RESERVED_MASK                           0xFFF80000
#define    RXBITLEVEL_PARITY_B_EINIT_TRCHPARAMETERS_PARITY_B_EINIT_SHIFT  0
#define    RXBITLEVEL_PARITY_B_EINIT_TRCHPARAMETERS_PARITY_B_EINIT_MASK   0x0007FFFF

#define RXBITLEVEL_PARITY_B_EPLUS_OFFSET                                  0x0000001C
#define RXBITLEVEL_PARITY_B_EPLUS_TYPE                                    UInt32
#define RXBITLEVEL_PARITY_B_EPLUS_RESERVED_MASK                           0xFFF80000
#define    RXBITLEVEL_PARITY_B_EPLUS_TRCHPARAMETERS_PARITY_B_EPLUS_SHIFT  0
#define    RXBITLEVEL_PARITY_B_EPLUS_TRCHPARAMETERS_PARITY_B_EPLUS_MASK   0x0007FFFF

#define RXBITLEVEL_PARITY_B_EMINUS_OFFSET                                 0x00000020
#define RXBITLEVEL_PARITY_B_EMINUS_TYPE                                   UInt32
#define RXBITLEVEL_PARITY_B_EMINUS_RESERVED_MASK                          0xFFF80000
#define    RXBITLEVEL_PARITY_B_EMINUS_TRCHPARAMETERS_PARITY_B_EMINUS_SHIFT 0
#define    RXBITLEVEL_PARITY_B_EMINUS_TRCHPARAMETERS_PARITY_B_EMINUS_MASK 0x0007FFFF

#define RXBITLEVEL_VITERBI_DIFF_OFFSET                                    0x00000024
#define RXBITLEVEL_VITERBI_DIFF_TYPE                                      UInt32
#define RXBITLEVEL_VITERBI_DIFF_RESERVED_MASK                             0xF000F000
#define    RXBITLEVEL_VITERBI_DIFF_VITERBI_DIFF_MAX_MIN_SHIFT             16
#define    RXBITLEVEL_VITERBI_DIFF_VITERBI_DIFF_MAX_MIN_MASK              0x0FFF0000
#define    RXBITLEVEL_VITERBI_DIFF_VITERBI_DIFF_MIN_ZERO_SHIFT            0
#define    RXBITLEVEL_VITERBI_DIFF_VITERBI_DIFF_MIN_ZERO_MASK             0x00000FFF

#define RXBITLEVEL_TURBOPARAMETERS0_OFFSET                                0x00000028
#define RXBITLEVEL_TURBOPARAMETERS0_TYPE                                  UInt32
#define RXBITLEVEL_TURBOPARAMETERS0_RESERVED_MASK                         0x8080E0E0
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_SKIP2_SHIFT        24
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_SKIP2_MASK         0x7F000000
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_SKIP1_SHIFT        16
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_SKIP1_MASK         0x007F0000
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_V_SHIFT            8
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_V_MASK             0x00001F00
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_R_SHIFT            0
#define    RXBITLEVEL_TURBOPARAMETERS0_TURBOPARAMETERS_R_MASK             0x0000001F

#define RXBITLEVEL_TURBOPARAMETERS1_OFFSET                                0x0000002C
#define RXBITLEVEL_TURBOPARAMETERS1_TYPE                                  UInt32
#define RXBITLEVEL_TURBOPARAMETERS1_RESERVED_MASK                         0x8E00FE00
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_KRC_SHIFT          30
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_KRC_MASK           0x40000000
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_TSEL_SHIFT         28
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_TSEL_MASK          0x30000000
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_PRIME_SHIFT        16
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_PRIME_MASK         0x01FF0000
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_C_SHIFT            0
#define    RXBITLEVEL_TURBOPARAMETERS1_TURBOPARAMETERS_C_MASK             0x000001FF

#define RXBITLEVEL_TURBOPARAMETERS2_OFFSET                                0x00000030
#define RXBITLEVEL_TURBOPARAMETERS2_TYPE                                  UInt32
#define RXBITLEVEL_TURBOPARAMETERS2_RESERVED_MASK                         0xE080E000
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_TERM_THRESHOLD_SHIFT 24
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_TERM_THRESHOLD_MASK 0x1F000000
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_EXTRINSIC_SCALING_SHIFT 22
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_EXTRINSIC_SCALING_MASK 0x00400000
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_NUMITERATIONS_SHIFT 16
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_NUMITERATIONS_MASK 0x003F0000
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_K_SHIFT            0
#define    RXBITLEVEL_TURBOPARAMETERS2_TURBOPARAMETERS_K_MASK             0x00001FFF

#define RXBITLEVEL_TURBOPARAMETERS3_OFFSET                                0x00000034
#define RXBITLEVEL_TURBOPARAMETERS3_TYPE                                  UInt32
#define RXBITLEVEL_TURBOPARAMETERS3_RESERVED_MASK                         0xFFC0FC00
#define    RXBITLEVEL_TURBOPARAMETERS3_TRCHPARAMETERS_NUMFILLERBITS_SHIFT 16
#define    RXBITLEVEL_TURBOPARAMETERS3_TRCHPARAMETERS_NUMFILLERBITS_MASK  0x003F0000
#define    RXBITLEVEL_TURBOPARAMETERS3_TRCHPARAMETERS_BLOCKS_SHIFT        0
#define    RXBITLEVEL_TURBOPARAMETERS3_TRCHPARAMETERS_BLOCKS_MASK         0x000003FF

#define RXBITLEVEL_BLOCKSETPTR_OFFSET                                     0x00000038
#define RXBITLEVEL_BLOCKSETPTR_TYPE                                       UInt32
#define RXBITLEVEL_BLOCKSETPTR_RESERVED_MASK                              0x00000000
#define    RXBITLEVEL_BLOCKSETPTR_TRCHPARAMETERS_BLOCKSETPTR_SHIFT        0
#define    RXBITLEVEL_BLOCKSETPTR_TRCHPARAMETERS_BLOCKSETPTR_MASK         0xFFFFFFFF

#define RXBITLEVEL_ENDIAN_OFFSET                                          0x0000003C
#define RXBITLEVEL_ENDIAN_TYPE                                            UInt32
#define RXBITLEVEL_ENDIAN_RESERVED_MASK                                   0xF000EEEE
#define    RXBITLEVEL_ENDIAN_VITERBI_SM_ZERO_SHIFT                        16
#define    RXBITLEVEL_ENDIAN_VITERBI_SM_ZERO_MASK                         0x0FFF0000
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_MSTREAM_BITENDIAN_SHIFT       12
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_MSTREAM_BITENDIAN_MASK        0x00001000
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_MSTREAM_NIBBLEENDIAN_SHIFT    8
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_MSTREAM_NIBBLEENDIAN_MASK     0x00000100
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_MSTREAM_WORDENDIAN_SHIFT      4
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_MSTREAM_WORDENDIAN_MASK       0x00000010
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_CFGBIGEND_SHIFT               0
#define    RXBITLEVEL_ENDIAN_TRCHPARAMETERS_CFGBIGEND_MASK                0x00000001

#endif /* __BRCM_RDB_RXBITLEVEL_H__ */



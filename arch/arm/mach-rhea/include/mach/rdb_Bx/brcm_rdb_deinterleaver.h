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

#ifndef __BRCM_RDB_DEINTERLEAVER_H__
#define __BRCM_RDB_DEINTERLEAVER_H__

#define DEINTERLEAVER_PTR19K_OFFSET                                       0x00000000
#define DEINTERLEAVER_PTR19K_TYPE                                         UInt32
#define DEINTERLEAVER_PTR19K_RESERVED_MASK                                0xFFFF0000
#define    DEINTERLEAVER_PTR19K_PTR19K_SHIFT                              0
#define    DEINTERLEAVER_PTR19K_PTR19K_MASK                               0x0000FFFF

#define DEINTERLEAVER_PTR23K_OFFSET                                       0x00000004
#define DEINTERLEAVER_PTR23K_TYPE                                         UInt32
#define DEINTERLEAVER_PTR23K_RESERVED_MASK                                0xFFFF0000
#define    DEINTERLEAVER_PTR23K_PTR23K_SHIFT                              0
#define    DEINTERLEAVER_PTR23K_PTR23K_MASK                               0x0000FFFF

#define DEINTERLEAVER_NOBITS_OFFSET                                       0x00000008
#define DEINTERLEAVER_NOBITS_TYPE                                         UInt32
#define DEINTERLEAVER_NOBITS_RESERVED_MASK                                0xFFFF0000
#define    DEINTERLEAVER_NOBITS_NOBITS_SHIFT                              0
#define    DEINTERLEAVER_NOBITS_NOBITS_MASK                               0x0000FFFF

#define DEINTERLEAVER_CTRL_OFFSET                                         0x0000000C
#define DEINTERLEAVER_CTRL_TYPE                                           UInt32
#define DEINTERLEAVER_CTRL_RESERVED_MASK                                  0xFFFF8EEE
#define    DEINTERLEAVER_CTRL_PATTERN_SHIFT                               12
#define    DEINTERLEAVER_CTRL_PATTERN_MASK                                0x00007000
#define    DEINTERLEAVER_CTRL_NO_ACK_SHIFT                                8
#define    DEINTERLEAVER_CTRL_NO_ACK_MASK                                 0x00000100
#define    DEINTERLEAVER_CTRL_GENTRCHSTART_SHIFT                          4
#define    DEINTERLEAVER_CTRL_GENTRCHSTART_MASK                           0x00000010
#define    DEINTERLEAVER_CTRL_RS_MODE_SHIFT                               0
#define    DEINTERLEAVER_CTRL_RS_MODE_MASK                                0x00000001

#define DEINTERLEAVER_DIL_1ST_PVALID_OFFSET                               0x00000010
#define DEINTERLEAVER_DIL_1ST_PVALID_TYPE                                 UInt32
#define DEINTERLEAVER_DIL_1ST_PVALID_RESERVED_MASK                        0xFFFFFFFE
#define    DEINTERLEAVER_DIL_1ST_PVALID_DIL_1ST_PVALID_SHIFT              0
#define    DEINTERLEAVER_DIL_1ST_PVALID_DIL_1ST_PVALID_MASK               0x00000001

#define DEINTERLEAVER_STARTPOINTER_1_OFFSET                               0x00000014
#define DEINTERLEAVER_STARTPOINTER_1_TYPE                                 UInt32
#define DEINTERLEAVER_STARTPOINTER_1_RESERVED_MASK                        0xFFFF0000
#define    DEINTERLEAVER_STARTPOINTER_1_STARTPOINTER_1_SHIFT              0
#define    DEINTERLEAVER_STARTPOINTER_1_STARTPOINTER_1_MASK               0x0000FFFF

#define DEINTERLEAVER_ENDPOINTER_1_OFFSET                                 0x00000018
#define DEINTERLEAVER_ENDPOINTER_1_TYPE                                   UInt32
#define DEINTERLEAVER_ENDPOINTER_1_RESERVED_MASK                          0xFFFF0000
#define    DEINTERLEAVER_ENDPOINTER_1_ENDPOINTER_1_SHIFT                  0
#define    DEINTERLEAVER_ENDPOINTER_1_ENDPOINTER_1_MASK                   0x0000FFFF

#define DEINTERLEAVER_STARTPOINTER_2_OFFSET                               0x0000001C
#define DEINTERLEAVER_STARTPOINTER_2_TYPE                                 UInt32
#define DEINTERLEAVER_STARTPOINTER_2_RESERVED_MASK                        0xFFFF0000
#define    DEINTERLEAVER_STARTPOINTER_2_STARTPOINTER_2_SHIFT              0
#define    DEINTERLEAVER_STARTPOINTER_2_STARTPOINTER_2_MASK               0x0000FFFF

#define DEINTERLEAVER_ENDPOINTER_2_OFFSET                                 0x00000020
#define DEINTERLEAVER_ENDPOINTER_2_TYPE                                   UInt32
#define DEINTERLEAVER_ENDPOINTER_2_RESERVED_MASK                          0xFFFF0000
#define    DEINTERLEAVER_ENDPOINTER_2_ENDPOINTER_2_SHIFT                  0
#define    DEINTERLEAVER_ENDPOINTER_2_ENDPOINTER_2_MASK                   0x0000FFFF

#define DEINTERLEAVER_STARTPOINTER_3_OFFSET                               0x00000024
#define DEINTERLEAVER_STARTPOINTER_3_TYPE                                 UInt32
#define DEINTERLEAVER_STARTPOINTER_3_RESERVED_MASK                        0xFFFF0000
#define    DEINTERLEAVER_STARTPOINTER_3_STARTPOINTER_3_SHIFT              0
#define    DEINTERLEAVER_STARTPOINTER_3_STARTPOINTER_3_MASK               0x0000FFFF

#define DEINTERLEAVER_ENDPOINTER_3_OFFSET                                 0x00000028
#define DEINTERLEAVER_ENDPOINTER_3_TYPE                                   UInt32
#define DEINTERLEAVER_ENDPOINTER_3_RESERVED_MASK                          0xFFFF0000
#define    DEINTERLEAVER_ENDPOINTER_3_ENDPOINTER_3_SHIFT                  0
#define    DEINTERLEAVER_ENDPOINTER_3_ENDPOINTER_3_MASK                   0x0000FFFF

#define DEINTERLEAVER_STARTPOINTER_4_OFFSET                               0x0000002C
#define DEINTERLEAVER_STARTPOINTER_4_TYPE                                 UInt32
#define DEINTERLEAVER_STARTPOINTER_4_RESERVED_MASK                        0xFFFF0000
#define    DEINTERLEAVER_STARTPOINTER_4_STARTPOINTER_4_SHIFT              0
#define    DEINTERLEAVER_STARTPOINTER_4_STARTPOINTER_4_MASK               0x0000FFFF

#define DEINTERLEAVER_ENDPOINTER_4_OFFSET                                 0x00000030
#define DEINTERLEAVER_ENDPOINTER_4_TYPE                                   UInt32
#define DEINTERLEAVER_ENDPOINTER_4_RESERVED_MASK                          0xFFFF0000
#define    DEINTERLEAVER_ENDPOINTER_4_ENDPOINTER_4_SHIFT                  0
#define    DEINTERLEAVER_ENDPOINTER_4_ENDPOINTER_4_MASK                   0x0000FFFF

#define DEINTERLEAVER_NUMPBITS_OFFSET                                     0x00000034
#define DEINTERLEAVER_NUMPBITS_TYPE                                       UInt32
#define DEINTERLEAVER_NUMPBITS_RESERVED_MASK                              0xFFFF0000
#define    DEINTERLEAVER_NUMPBITS_NUMPBITS_SHIFT                          0
#define    DEINTERLEAVER_NUMPBITS_NUMPBITS_MASK                           0x0000FFFF

#define DEINTERLEAVER_REMOVEPBITS_OFFSET                                  0x00000038
#define DEINTERLEAVER_REMOVEPBITS_TYPE                                    UInt32
#define DEINTERLEAVER_REMOVEPBITS_RESERVED_MASK                           0xFFFFFFFE
#define    DEINTERLEAVER_REMOVEPBITS_REMOVEPBITS_SHIFT                    0
#define    DEINTERLEAVER_REMOVEPBITS_REMOVEPBITS_MASK                     0x00000001

#endif /* __BRCM_RDB_DEINTERLEAVER_H__ */



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

#ifndef __BRCM_RDB_COMBINER2_H__
#define __BRCM_RDB_COMBINER2_H__

#define COMBINER2_CH5GO0_OFFSET                                           0x00000000
#define COMBINER2_CH5GO0_TYPE                                             UInt32
#define COMBINER2_CH5GO0_RESERVED_MASK                                    0xFFFFFFEE
#define    COMBINER2_CH5GO0_CH5GO0_AUTO_SHIFT                             4
#define    COMBINER2_CH5GO0_CH5GO0_AUTO_MASK                              0x00000010
#define    COMBINER2_CH5GO0_CH5GO0_SHIFT                                  0
#define    COMBINER2_CH5GO0_CH5GO0_MASK                                   0x00000001

#define COMBINER2_CH5FINGERMASKIN_OFFSET                                  0x00000004
#define COMBINER2_CH5FINGERMASKIN_TYPE                                    UInt32
#define COMBINER2_CH5FINGERMASKIN_RESERVED_MASK                           0xFFC0C0C0
#define    COMBINER2_CH5FINGERMASKIN_CH5FIRSTINTIMEIN_SHIFT               16
#define    COMBINER2_CH5FINGERMASKIN_CH5FIRSTINTIMEIN_MASK                0x003F0000
#define    COMBINER2_CH5FINGERMASKIN_CH5LASTINTIMEIN_SHIFT                8
#define    COMBINER2_CH5FINGERMASKIN_CH5LASTINTIMEIN_MASK                 0x00003F00
#define    COMBINER2_CH5FINGERMASKIN_CH5FINGERMASKIN_SHIFT                0
#define    COMBINER2_CH5FINGERMASKIN_CH5FINGERMASKIN_MASK                 0x0000003F

#define COMBINER2_CH5FRAMEOFFSETSIN_0_OFFSET                              0x00000008
#define COMBINER2_CH5FRAMEOFFSETSIN_0_TYPE                                UInt32
#define COMBINER2_CH5FRAMEOFFSETSIN_0_RESERVED_MASK                       0xC000C000
#define    COMBINER2_CH5FRAMEOFFSETSIN_0_CH5FRAMEOFFSETSIN_1_SHIFT        16
#define    COMBINER2_CH5FRAMEOFFSETSIN_0_CH5FRAMEOFFSETSIN_1_MASK         0x3FFF0000
#define    COMBINER2_CH5FRAMEOFFSETSIN_0_CH5FRAMEOFFSETSIN_0_SHIFT        0
#define    COMBINER2_CH5FRAMEOFFSETSIN_0_CH5FRAMEOFFSETSIN_0_MASK         0x00003FFF

#define COMBINER2_CH5FRAMEOFFSETSIN_2_OFFSET                              0x0000000C
#define COMBINER2_CH5FRAMEOFFSETSIN_2_TYPE                                UInt32
#define COMBINER2_CH5FRAMEOFFSETSIN_2_RESERVED_MASK                       0xC000C000
#define    COMBINER2_CH5FRAMEOFFSETSIN_2_CH5FRAMEOFFSETSIN_3_SHIFT        16
#define    COMBINER2_CH5FRAMEOFFSETSIN_2_CH5FRAMEOFFSETSIN_3_MASK         0x3FFF0000
#define    COMBINER2_CH5FRAMEOFFSETSIN_2_CH5FRAMEOFFSETSIN_2_SHIFT        0
#define    COMBINER2_CH5FRAMEOFFSETSIN_2_CH5FRAMEOFFSETSIN_2_MASK         0x00003FFF

#define COMBINER2_CH5FRAMEOFFSETSIN_4_OFFSET                              0x00000010
#define COMBINER2_CH5FRAMEOFFSETSIN_4_TYPE                                UInt32
#define COMBINER2_CH5FRAMEOFFSETSIN_4_RESERVED_MASK                       0xC000C000
#define    COMBINER2_CH5FRAMEOFFSETSIN_4_CH5FRAMEOFFSETSIN_5_SHIFT        16
#define    COMBINER2_CH5FRAMEOFFSETSIN_4_CH5FRAMEOFFSETSIN_5_MASK         0x3FFF0000
#define    COMBINER2_CH5FRAMEOFFSETSIN_4_CH5FRAMEOFFSETSIN_4_SHIFT        0
#define    COMBINER2_CH5FRAMEOFFSETSIN_4_CH5FRAMEOFFSETSIN_4_MASK         0x00003FFF

#define COMBINER2_CH5FRAMELENGTHIN_OFFSET                                 0x00000014
#define COMBINER2_CH5FRAMELENGTHIN_TYPE                                   UInt32
#define COMBINER2_CH5FRAMELENGTHIN_RESERVED_MASK                          0xFFFFC000
#define    COMBINER2_CH5FRAMELENGTHIN_CH5FRAMELENGTHIN_SHIFT              0
#define    COMBINER2_CH5FRAMELENGTHIN_CH5FRAMELENGTHIN_MASK               0x00003FFF

#define COMBINER2_CH5SLOTFORMATIN_0_OFFSET                                0x00000018
#define COMBINER2_CH5SLOTFORMATIN_0_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_0_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_0_CH5SLOTFORMATIN_0_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_0_CH5SLOTFORMATIN_0_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_0_CH5SLOTFORMATIN_0_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_0_CH5SLOTFORMATIN_0_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_0_CH5SLOTFORMATIN_0_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_0_CH5SLOTFORMATIN_0_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_1_OFFSET                                0x0000001C
#define COMBINER2_CH5SLOTFORMATIN_1_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_1_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_1_CH5SLOTFORMATIN_1_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_1_CH5SLOTFORMATIN_1_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_1_CH5SLOTFORMATIN_1_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_1_CH5SLOTFORMATIN_1_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_1_CH5SLOTFORMATIN_1_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_1_CH5SLOTFORMATIN_1_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_2_OFFSET                                0x00000020
#define COMBINER2_CH5SLOTFORMATIN_2_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_2_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_2_CH5SLOTFORMATIN_2_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_2_CH5SLOTFORMATIN_2_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_2_CH5SLOTFORMATIN_2_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_2_CH5SLOTFORMATIN_2_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_2_CH5SLOTFORMATIN_2_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_2_CH5SLOTFORMATIN_2_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_3_OFFSET                                0x00000024
#define COMBINER2_CH5SLOTFORMATIN_3_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_3_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_3_CH5SLOTFORMATIN_3_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_3_CH5SLOTFORMATIN_3_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_3_CH5SLOTFORMATIN_3_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_3_CH5SLOTFORMATIN_3_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_3_CH5SLOTFORMATIN_3_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_3_CH5SLOTFORMATIN_3_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_4_OFFSET                                0x00000028
#define COMBINER2_CH5SLOTFORMATIN_4_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_4_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_4_CH5SLOTFORMATIN_4_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_4_CH5SLOTFORMATIN_4_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_4_CH5SLOTFORMATIN_4_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_4_CH5SLOTFORMATIN_4_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_4_CH5SLOTFORMATIN_4_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_4_CH5SLOTFORMATIN_4_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_5_OFFSET                                0x0000002C
#define COMBINER2_CH5SLOTFORMATIN_5_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_5_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_5_CH5SLOTFORMATIN_5_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_5_CH5SLOTFORMATIN_5_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_5_CH5SLOTFORMATIN_5_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_5_CH5SLOTFORMATIN_5_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_5_CH5SLOTFORMATIN_5_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_5_CH5SLOTFORMATIN_5_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_6_OFFSET                                0x00000030
#define COMBINER2_CH5SLOTFORMATIN_6_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_6_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_6_CH5SLOTFORMATIN_6_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_6_CH5SLOTFORMATIN_6_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_6_CH5SLOTFORMATIN_6_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_6_CH5SLOTFORMATIN_6_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_6_CH5SLOTFORMATIN_6_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_6_CH5SLOTFORMATIN_6_CNT_MASK         0x000003FF

#define COMBINER2_CH5SLOTFORMATIN_7_OFFSET                                0x00000034
#define COMBINER2_CH5SLOTFORMATIN_7_TYPE                                  UInt32
#define COMBINER2_CH5SLOTFORMATIN_7_RESERVED_MASK                         0xF8F8FC00
#define    COMBINER2_CH5SLOTFORMATIN_7_CH5SLOTFORMATIN_7_NEXTINDEX_SHIFT  24
#define    COMBINER2_CH5SLOTFORMATIN_7_CH5SLOTFORMATIN_7_NEXTINDEX_MASK   0x07000000
#define    COMBINER2_CH5SLOTFORMATIN_7_CH5SLOTFORMATIN_7_SYMTYPE_SHIFT    16
#define    COMBINER2_CH5SLOTFORMATIN_7_CH5SLOTFORMATIN_7_SYMTYPE_MASK     0x00070000
#define    COMBINER2_CH5SLOTFORMATIN_7_CH5SLOTFORMATIN_7_CNT_SHIFT        0
#define    COMBINER2_CH5SLOTFORMATIN_7_CH5SLOTFORMATIN_7_CNT_MASK         0x000003FF

#define COMBINER2_CH5DTX_OFFSET                                           0x0000003C
#define COMBINER2_CH5DTX_TYPE                                             UInt32
#define COMBINER2_CH5DTX_RESERVED_MASK                                    0x4000C000
#define    COMBINER2_CH5DTX_CH5DTXMODEIN_R1_SHIFT                         31
#define    COMBINER2_CH5DTX_CH5DTXMODEIN_R1_MASK                          0x80000000
#define    COMBINER2_CH5DTX_CH5DTXENDIN_SHIFT                             16
#define    COMBINER2_CH5DTX_CH5DTXENDIN_MASK                              0x3FFF0000
#define    COMBINER2_CH5DTX_CH5DTXSTARTIN_SHIFT                           0
#define    COMBINER2_CH5DTX_CH5DTXSTARTIN_MASK                            0x00003FFF

#define COMBINER2_CH5GO1_OFFSET                                           0x00000040
#define COMBINER2_CH5GO1_TYPE                                             UInt32
#define COMBINER2_CH5GO1_RESERVED_MASK                                    0xFFFFFFEE
#define    COMBINER2_CH5GO1_CH5GO1_AUTO_SHIFT                             4
#define    COMBINER2_CH5GO1_CH5GO1_AUTO_MASK                              0x00000010
#define    COMBINER2_CH5GO1_CH5GO1_SHIFT                                  0
#define    COMBINER2_CH5GO1_CH5GO1_MASK                                   0x00000001

#define COMBINER2_CH5VPGAININ_OFFSET                                      0x00000048
#define COMBINER2_CH5VPGAININ_TYPE                                        UInt32
#define COMBINER2_CH5VPGAININ_RESERVED_MASK                               0xFFFFFFF8
#define    COMBINER2_CH5VPGAININ_CH5VPGAININ_SHIFT                        0
#define    COMBINER2_CH5VPGAININ_CH5VPGAININ_MASK                         0x00000007

#define COMBINER2_SCCH_FNG_MUX_OFFSET                                     0x00000080
#define COMBINER2_SCCH_FNG_MUX_TYPE                                       UInt32
#define COMBINER2_SCCH_FNG_MUX_RESERVED_MASK                              0xFFFFFFC0
#define    COMBINER2_SCCH_FNG_MUX_SCCH_FNG_MUX_SHIFT                      0
#define    COMBINER2_SCCH_FNG_MUX_SCCH_FNG_MUX_MASK                       0x0000003F

#define COMBINER2_AUTO_EN_HS_OFFSET                                       0x00000084
#define COMBINER2_AUTO_EN_HS_TYPE                                         UInt32
#define COMBINER2_AUTO_EN_HS_RESERVED_MASK                                0xFFFFFFFC
#define    COMBINER2_AUTO_EN_HS_AUTO_EN_HS_SHIFT                          0
#define    COMBINER2_AUTO_EN_HS_AUTO_EN_HS_MASK                           0x00000003

#endif /* __BRCM_RDB_COMBINER2_H__ */



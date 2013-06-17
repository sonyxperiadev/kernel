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

#ifndef __BRCM_RDB_PDM_TOP_H__
#define __BRCM_RDB_PDM_TOP_H__

#define PDM_TOP_CTRL1_OFFSET                                              0x00000000
#define PDM_TOP_CTRL1_TYPE                                                UInt32
#define PDM_TOP_CTRL1_RESERVED_MASK                                       0xFFFFF0EE
#define    PDM_TOP_CTRL1_PDM_1_OEN_CNT_SHIFT                              8
#define    PDM_TOP_CTRL1_PDM_1_OEN_CNT_MASK                               0x00000F00
#define    PDM_TOP_CTRL1_PDM_1_OEN_STATUS_SHIFT                           4
#define    PDM_TOP_CTRL1_PDM_1_OEN_STATUS_MASK                            0x00000010
#define    PDM_TOP_CTRL1_PDM_ENA_SHIFT                                    0
#define    PDM_TOP_CTRL1_PDM_ENA_MASK                                     0x00000001

#define PDM_TOP_CTRL2_OFFSET                                              0x00000004
#define PDM_TOP_CTRL2_TYPE                                                UInt32
#define PDM_TOP_CTRL2_RESERVED_MASK                                       0xFFFFFC8E
#define    PDM_TOP_CTRL2_PDM_1_MODE_SHIFT                                 8
#define    PDM_TOP_CTRL2_PDM_1_MODE_MASK                                  0x00000300
#define    PDM_TOP_CTRL2_DATA_K_1_SHIFT                                   4
#define    PDM_TOP_CTRL2_DATA_K_1_MASK                                    0x00000070
#define    PDM_TOP_CTRL2_INV_1_SHIFT                                      0
#define    PDM_TOP_CTRL2_INV_1_MASK                                       0x00000001

#define PDM_TOP_CTRL3_OFFSET                                              0x00000008
#define PDM_TOP_CTRL3_TYPE                                                UInt32
#define PDM_TOP_CTRL3_RESERVED_MASK                                       0xFFFFFF88
#define    PDM_TOP_CTRL3_DATA_K_2_SHIFT                                   4
#define    PDM_TOP_CTRL3_DATA_K_2_MASK                                    0x00000070
#define    PDM_TOP_CTRL3_MUX_SEL_2_SHIFT                                  2
#define    PDM_TOP_CTRL3_MUX_SEL_2_MASK                                   0x00000004
#define    PDM_TOP_CTRL3_PDM_2_OEN_R1_SHIFT                               1
#define    PDM_TOP_CTRL3_PDM_2_OEN_R1_MASK                                0x00000002
#define    PDM_TOP_CTRL3_INV_2_SHIFT                                      0
#define    PDM_TOP_CTRL3_INV_2_MASK                                       0x00000001

#define PDM_TOP_CTRL4_OFFSET                                              0x0000000C
#define PDM_TOP_CTRL4_TYPE                                                UInt32
#define PDM_TOP_CTRL4_RESERVED_MASK                                       0xFFFFFF88
#define    PDM_TOP_CTRL4_DATA_K_3_SHIFT                                   4
#define    PDM_TOP_CTRL4_DATA_K_3_MASK                                    0x00000070
#define    PDM_TOP_CTRL4_MUX_SEL_3_SHIFT                                  2
#define    PDM_TOP_CTRL4_MUX_SEL_3_MASK                                   0x00000004
#define    PDM_TOP_CTRL4_PDM_3_OEN_R1_SHIFT                               1
#define    PDM_TOP_CTRL4_PDM_3_OEN_R1_MASK                                0x00000002
#define    PDM_TOP_CTRL4_INV_3_SHIFT                                      0
#define    PDM_TOP_CTRL4_INV_3_MASK                                       0x00000001

#define PDM_TOP_DATA_IN_APB_1_OFFSET                                      0x00000010
#define PDM_TOP_DATA_IN_APB_1_TYPE                                        UInt32
#define PDM_TOP_DATA_IN_APB_1_RESERVED_MASK                               0xFFFFF000
#define    PDM_TOP_DATA_IN_APB_1_DATA_IN_APB_1_SHIFT                      0
#define    PDM_TOP_DATA_IN_APB_1_DATA_IN_APB_1_MASK                       0x00000FFF

#define PDM_TOP_DATA_IN_APB_2_OFFSET                                      0x00000014
#define PDM_TOP_DATA_IN_APB_2_TYPE                                        UInt32
#define PDM_TOP_DATA_IN_APB_2_RESERVED_MASK                               0xFFFFF000
#define    PDM_TOP_DATA_IN_APB_2_DATA_IN_APB_2_SHIFT                      0
#define    PDM_TOP_DATA_IN_APB_2_DATA_IN_APB_2_MASK                       0x00000FFF

#define PDM_TOP_DATA_IN_APB_3_OFFSET                                      0x00000018
#define PDM_TOP_DATA_IN_APB_3_TYPE                                        UInt32
#define PDM_TOP_DATA_IN_APB_3_RESERVED_MASK                               0xFFFFF000
#define    PDM_TOP_DATA_IN_APB_3_DATA_IN_APB_3_SHIFT                      0
#define    PDM_TOP_DATA_IN_APB_3_DATA_IN_APB_3_MASK                       0x00000FFF

#endif /* __BRCM_RDB_PDM_TOP_H__ */



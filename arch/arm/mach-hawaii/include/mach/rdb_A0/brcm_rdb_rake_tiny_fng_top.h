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

#ifndef __BRCM_RDB_RAKE_TINY_FNG_TOP_H__
#define __BRCM_RDB_RAKE_TINY_FNG_TOP_H__

#define RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_OFFSET                           0x00000000
#define RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_TYPE                             UInt32
#define RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_RESERVED_MASK                    0xFFFFF000
#define    RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_FNG2_CPI_VPGAIN_SHIFT         8
#define    RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_FNG2_CPI_VPGAIN_MASK          0x00000F00
#define    RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_FNG1_CPI_VPGAIN_SHIFT         4
#define    RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_FNG1_CPI_VPGAIN_MASK          0x000000F0
#define    RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_FNG0_CPI_VPGAIN_SHIFT         0
#define    RAKE_TINY_FNG_TOP_FNG_CPI_VPGAIN_FNG0_CPI_VPGAIN_MASK          0x0000000F

#define RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_OFFSET                        0x00000008
#define RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TYPE                          UInt32
#define RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_RESERVED_MASK                 0xFFFFFFEF
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_LATCH_TYPE_SHIFT           4
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_LATCH_TYPE_MASK            0x00000010

#define RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_OFFSET                          0x0000000C
#define RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_TYPE                            UInt32
#define RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_RESERVED_MASK                   0xFFFFF80C
#define    RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_TINY0_RSCP_ACCLEN_SHIFT      4
#define    RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_TINY0_RSCP_ACCLEN_MASK       0x000007F0
#define    RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_TINY0_FNG_NF_SEL_SHIFT       0
#define    RAKE_TINY_FNG_TOP_TINY_FNG_NF_SEL_TINY0_FNG_NF_SEL_MASK        0x00000003

#define RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_OFFSET                       0x00000010
#define RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_TYPE                         UInt32
#define RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_RESERVED_MASK                0x00000000
#define    RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_TINY0_NF_ACC1_RSCP_SHIFT  16
#define    RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_TINY0_NF_ACC1_RSCP_MASK   0xFFFF0000
#define    RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_TINY0_NF_ACC0_RSCP_SHIFT  0
#define    RAKE_TINY_FNG_TOP_TINY0_NF_ACC_RSCP1_TINY0_NF_ACC0_RSCP_MASK   0x0000FFFF

#define RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_OFFSET                       0x00000014
#define RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_TYPE                         UInt32
#define RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_RESERVED_MASK                0x00000000
#define    RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_TINY0_NF_ACC3_RSCP_SHIFT  16
#define    RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_TINY0_NF_ACC3_RSCP_MASK   0xFFFF0000
#define    RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_TINY0_NF_ACC2_RSCP_SHIFT  0
#define    RAKE_TINY_FNG_TOP_TING0_NF_ACC_RSCP2_TINY0_NF_ACC2_RSCP_MASK   0x0000FFFF

#define RAKE_TINY_FNG_TOP_DEBUG_SEL_OFFSET                                0x00000020
#define RAKE_TINY_FNG_TOP_DEBUG_SEL_TYPE                                  UInt32
#define RAKE_TINY_FNG_TOP_DEBUG_SEL_RESERVED_MASK                         0x7FFE8EEE
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_IIR_IMP_TYPE_SHIFT                 31
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_IIR_IMP_TYPE_MASK                  0x80000000
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_CNT_SEL_SHIFT                16
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_CNT_SEL_MASK                 0x00010000
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_FNG_SEL_SHIFT                12
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_FNG_SEL_MASK                 0x00007000
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_ANT_SEL_SHIFT                8
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_ANT_SEL_MASK                 0x00000100
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_BLK_SEL_SHIFT                4
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_BLK_SEL_MASK                 0x00000010
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_SYM_SEL_SHIFT                0
#define    RAKE_TINY_FNG_TOP_DEBUG_SEL_DEBUG_SYM_SEL_MASK                 0x00000001

#define RAKE_TINY_FNG_TOP_TOP_CTRL_OFFSET                                 0x00000024
#define RAKE_TINY_FNG_TOP_TOP_CTRL_TYPE                                   UInt32
#define RAKE_TINY_FNG_TOP_TOP_CTRL_RESERVED_MASK                          0xFFFFFEEE
#define    RAKE_TINY_FNG_TOP_TOP_CTRL_PRISM_EN_SHIFT                      8
#define    RAKE_TINY_FNG_TOP_TOP_CTRL_PRISM_EN_MASK                       0x00000100
#define    RAKE_TINY_FNG_TOP_TOP_CTRL_ANT2_DLL_LATE_MUX_SHIFT             4
#define    RAKE_TINY_FNG_TOP_TOP_CTRL_ANT2_DLL_LATE_MUX_MASK              0x00000010
#define    RAKE_TINY_FNG_TOP_TOP_CTRL_LEGACY_DLL_MODE_R1_SHIFT            0
#define    RAKE_TINY_FNG_TOP_TOP_CTRL_LEGACY_DLL_MODE_R1_MASK             0x00000001

#define RAKE_TINY_FNG_TOP_TINY_RAKE_DEBUG_OFFSET                          0x00000028
#define RAKE_TINY_FNG_TOP_TINY_RAKE_DEBUG_TYPE                            UInt32
#define RAKE_TINY_FNG_TOP_TINY_RAKE_DEBUG_RESERVED_MASK                   0xFFFFFFFE
#define    RAKE_TINY_FNG_TOP_TINY_RAKE_DEBUG_TINY_RAKE_DEBUG_SHIFT        0
#define    RAKE_TINY_FNG_TOP_TINY_RAKE_DEBUG_TINY_RAKE_DEBUG_MASK         0x00000001

#endif /* __BRCM_RDB_RAKE_TINY_FNG_TOP_H__ */



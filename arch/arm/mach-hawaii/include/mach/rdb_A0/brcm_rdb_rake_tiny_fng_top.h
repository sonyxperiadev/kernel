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
/*     Date     : Generated on 4/25/2012 23:31:7                                             */
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
#define RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_RESERVED_MASK                 0xFFF666EF
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY2_CARRIER_SELECT_SHIFT  19
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY2_CARRIER_SELECT_MASK   0x00080000
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY2_ANTENNA_SELECT_SHIFT  16
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY2_ANTENNA_SELECT_MASK   0x00010000
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY1_CARRIER_SELECT_SHIFT  15
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY1_CARRIER_SELECT_MASK   0x00008000
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY1_ANTENNA_SELECT_SHIFT  12
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY1_ANTENNA_SELECT_MASK   0x00001000
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY0_CARRIER_SELECT_SHIFT  11
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY0_CARRIER_SELECT_MASK   0x00000800
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY0_ANTENNA_SELECT_SHIFT  8
#define    RAKE_TINY_FNG_TOP_DBL_BUF_SELECTION_TNY0_ANTENNA_SELECT_MASK   0x00000100
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

#define RAKE_TINY_FNG_TOP_FOC_CTRL1_OFFSET                                0x00000018
#define RAKE_TINY_FNG_TOP_FOC_CTRL1_TYPE                                  UInt32
#define RAKE_TINY_FNG_TOP_FOC_CTRL1_RESERVED_MASK                         0x80C000C6
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_SYM_CNT_FOR_ROT_SHIFT              24
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_SYM_CNT_FOR_ROT_MASK               0x7F000000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_TRAILING_FNG_SHIFT             20
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_TRAILING_FNG_MASK              0x00300000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AFC_TNY2_VP_SHIFT              16
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AFC_TNY2_VP_MASK               0x000F0000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AFC_TNY1_VP_SHIFT              12
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AFC_TNY1_VP_MASK               0x0000F000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AFC_TNY0_VP_SHIFT              8
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AFC_TNY0_VP_MASK               0x00000F00
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AVG_LEN_SHIFT                  4
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_AVG_LEN_MASK                   0x00000030
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_WRITE_ADDR_CLR_SHIFT               3
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_WRITE_ADDR_CLR_MASK                0x00000008
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_SW_EN_SHIFT                    0
#define    RAKE_TINY_FNG_TOP_FOC_CTRL1_FOC_SW_EN_MASK                     0x00000001

#define RAKE_TINY_FNG_TOP_FOC_CTRL2_OFFSET                                0x0000001C
#define RAKE_TINY_FNG_TOP_FOC_CTRL2_TYPE                                  UInt32
#define RAKE_TINY_FNG_TOP_FOC_CTRL2_RESERVED_MASK                         0x680C0088
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_ROTATION_EN_SHIFT                  31
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_ROTATION_EN_MASK                   0x80000000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_HW_CORRECTION_EN_SHIFT             28
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_HW_CORRECTION_EN_MASK              0x10000000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_MEM_WRITE_ADDR_SHIFT               20
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_MEM_WRITE_ADDR_MASK                0x07F00000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_PRE_CORDIC_VPGAIN_SHIFT            16
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_PRE_CORDIC_VPGAIN_MASK             0x00030000
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_FILTER_DELAY_SHIFT                 11
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_FILTER_DELAY_MASK                  0x0000F800
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_TNY2_SCALE_SHIFT                   8
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_TNY2_SCALE_MASK                    0x00000700
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_TNY1_SCALE_SHIFT                   4
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_TNY1_SCALE_MASK                    0x00000070
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_TNY0_SCALE_SHIFT                   0
#define    RAKE_TINY_FNG_TOP_FOC_CTRL2_TNY0_SCALE_MASK                    0x00000007

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

#define RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_OFFSET                         0x0000002C
#define RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_TYPE                           UInt32
#define RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_RESERVED_MASK                  0x7000000D
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_DBL_BUFF_EN_SHIFT           31
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_DBL_BUFF_EN_MASK            0x80000000
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_INSTPHASEFROMHW_SHIFT       16
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_INSTPHASEFROMHW_MASK        0x0FFF0000
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_SW_INST_PHASE_SHIFT         4
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_SW_INST_PHASE_MASK          0x0000FFF0
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_SW_MEAS_OVERRIDE_SHIFT      1
#define    RAKE_TINY_FNG_TOP_SW_FOC_MEAS_MODE_SW_MEAS_OVERRIDE_MASK       0x00000002

#define RAKE_TINY_FNG_TOP_INITIALVALUE2MEM_OFFSET                         0x00000030
#define RAKE_TINY_FNG_TOP_INITIALVALUE2MEM_TYPE                           UInt32
#define RAKE_TINY_FNG_TOP_INITIALVALUE2MEM_RESERVED_MASK                  0x00000000
#define    RAKE_TINY_FNG_TOP_INITIALVALUE2MEM_MEM_INIT_VALUE_SHIFT        0
#define    RAKE_TINY_FNG_TOP_INITIALVALUE2MEM_MEM_INIT_VALUE_MASK         0xFFFFFFFF

#define RAKE_TINY_FNG_TOP_INITIALVALUECTRL_OFFSET                         0x00000034
#define RAKE_TINY_FNG_TOP_INITIALVALUECTRL_TYPE                           UInt32
#define RAKE_TINY_FNG_TOP_INITIALVALUECTRL_RESERVED_MASK                  0xFFFF300E
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_AVGLEN_DBL_BYPASS_SHIFT     15
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_AVGLEN_DBL_BYPASS_MASK      0x00008000
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_MSTFNG_DBL_BYPASS_SHIFT     14
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_MSTFNG_DBL_BYPASS_MASK      0x00004000
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_DEPTH_SHIFT                 4
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_DEPTH_MASK                  0x00000FF0
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_INIT_WRITE_EN_SHIFT         0
#define    RAKE_TINY_FNG_TOP_INITIALVALUECTRL_INIT_WRITE_EN_MASK          0x00000001

#endif /* __BRCM_RDB_RAKE_TINY_FNG_TOP_H__ */



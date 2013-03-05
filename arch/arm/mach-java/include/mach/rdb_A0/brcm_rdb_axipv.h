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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_AXIPV_H__
#define __BRCM_RDB_AXIPV_H__

#define AXIPV_NEXT_FRAME_START_ADDR_OFFSET                                0x00000000
#define AXIPV_NEXT_FRAME_START_ADDR_TYPE                                  UInt32
#define AXIPV_NEXT_FRAME_START_ADDR_RESERVED_MASK                         0x00000000
#define    AXIPV_NEXT_FRAME_START_ADDR_NEXT_FRAME_START_ADDR_SHIFT        0
#define    AXIPV_NEXT_FRAME_START_ADDR_NEXT_FRAME_START_ADDR_MASK         0xFFFFFFFF

#define AXIPV_CURRENT_FRAME_START_ADDR_OFFSET                             0x00000004
#define AXIPV_CURRENT_FRAME_START_ADDR_TYPE                               UInt32
#define AXIPV_CURRENT_FRAME_START_ADDR_RESERVED_MASK                      0x00000000
#define    AXIPV_CURRENT_FRAME_START_ADDR_CURRENT_FRAME_START_ADDR_SHIFT  0
#define    AXIPV_CURRENT_FRAME_START_ADDR_CURRENT_FRAME_START_ADDR_MASK   0xFFFFFFFF

#define AXIPV_FRAME_ADD_UNUSED_OFFSET                                     0x00000008
#define AXIPV_FRAME_ADD_UNUSED_TYPE                                       UInt32
#define AXIPV_FRAME_ADD_UNUSED_RESERVED_MASK                              0x00000000
#define    AXIPV_FRAME_ADD_UNUSED_FRAME_ADD_UNUSED_SHIFT                  0
#define    AXIPV_FRAME_ADD_UNUSED_FRAME_ADD_UNUSED_MASK                   0xFFFFFFFF

#define AXIPV_LINE_STRIDE_OFFSET                                          0x0000000C
#define AXIPV_LINE_STRIDE_TYPE                                            UInt32
#define AXIPV_LINE_STRIDE_RESERVED_MASK                                   0x00000000
#define    AXIPV_LINE_STRIDE_LINE_STRIDE_UNUSED_SHIFT                     20
#define    AXIPV_LINE_STRIDE_LINE_STRIDE_UNUSED_MASK                      0xFFF00000
#define    AXIPV_LINE_STRIDE_LINE_STRIDE_SHIFT                            0
#define    AXIPV_LINE_STRIDE_LINE_STRIDE_MASK                             0x000FFFFF

#define AXIPV_BYTES_PER_LINE_OFFSET                                       0x00000010
#define AXIPV_BYTES_PER_LINE_TYPE                                         UInt32
#define AXIPV_BYTES_PER_LINE_RESERVED_MASK                                0x00000000
#define    AXIPV_BYTES_PER_LINE_BYTES_PER_LINE_UNUSED_SHIFT               18
#define    AXIPV_BYTES_PER_LINE_BYTES_PER_LINE_UNUSED_MASK                0xFFFC0000
#define    AXIPV_BYTES_PER_LINE_BYTES_PER_LINE_SHIFT                      0
#define    AXIPV_BYTES_PER_LINE_BYTES_PER_LINE_MASK                       0x0003FFFF

#define AXIPV_LINES_PER_FRAME_OFFSET                                      0x00000014
#define AXIPV_LINES_PER_FRAME_TYPE                                        UInt32
#define AXIPV_LINES_PER_FRAME_RESERVED_MASK                               0x00000000
#define    AXIPV_LINES_PER_FRAME_LINES_PER_FRAME_UNUSED_SHIFT             16
#define    AXIPV_LINES_PER_FRAME_LINES_PER_FRAME_UNUSED_MASK              0xFFFF0000
#define    AXIPV_LINES_PER_FRAME_LINES_PER_FRAME_SHIFT                    0
#define    AXIPV_LINES_PER_FRAME_LINES_PER_FRAME_MASK                     0x0000FFFF

#define AXIPV_AXI_BURST_LEN_OFFSET                                        0x00000018
#define AXIPV_AXI_BURST_LEN_TYPE                                          UInt32
#define AXIPV_AXI_BURST_LEN_RESERVED_MASK                                 0x00000000
#define    AXIPV_AXI_BURST_LEN_AXI_BURST_LEN_UNUSED_SHIFT                 4
#define    AXIPV_AXI_BURST_LEN_AXI_BURST_LEN_UNUSED_MASK                  0xFFFFFFF0
#define    AXIPV_AXI_BURST_LEN_AXI_BURST_LEN_SHIFT                        0
#define    AXIPV_AXI_BURST_LEN_AXI_BURST_LEN_MASK                         0x0000000F

#define AXIPV_WATER_LVL_1_OFFSET                                          0x0000001C
#define AXIPV_WATER_LVL_1_TYPE                                            UInt32
#define AXIPV_WATER_LVL_1_RESERVED_MASK                                   0x00000000
#define    AXIPV_WATER_LVL_1_WATER_LVL_1_UNUSED_SHIFT                     16
#define    AXIPV_WATER_LVL_1_WATER_LVL_1_UNUSED_MASK                      0xFFFF0000
#define    AXIPV_WATER_LVL_1_WATER_LVL_1_SHIFT                            0
#define    AXIPV_WATER_LVL_1_WATER_LVL_1_MASK                             0x0000FFFF

#define AXIPV_WATER_LVL_2_OFFSET                                          0x00000020
#define AXIPV_WATER_LVL_2_TYPE                                            UInt32
#define AXIPV_WATER_LVL_2_RESERVED_MASK                                   0x00000000
#define    AXIPV_WATER_LVL_2_WATER_LVL_2_UNUSED_SHIFT                     16
#define    AXIPV_WATER_LVL_2_WATER_LVL_2_UNUSED_MASK                      0xFFFF0000
#define    AXIPV_WATER_LVL_2_WATER_LVL_2_SHIFT                            0
#define    AXIPV_WATER_LVL_2_WATER_LVL_2_MASK                             0x0000FFFF

#define AXIPV_PV_START_THRESH_OFFSET                                      0x00000024
#define AXIPV_PV_START_THRESH_TYPE                                        UInt32
#define AXIPV_PV_START_THRESH_RESERVED_MASK                               0x00000000
#define    AXIPV_PV_START_THRESH_PV_START_THRESH_UNUSED_SHIFT             16
#define    AXIPV_PV_START_THRESH_PV_START_THRESH_UNUSED_MASK              0xFFFF0000
#define    AXIPV_PV_START_THRESH_PV_START_THRESH_SHIFT                    0
#define    AXIPV_PV_START_THRESH_PV_START_THRESH_MASK                     0x0000FFFF

#define AXIPV_TE_LINE_NUM_OFFSET                                          0x00000028
#define AXIPV_TE_LINE_NUM_TYPE                                            UInt32
#define AXIPV_TE_LINE_NUM_RESERVED_MASK                                   0x00000000
#define    AXIPV_TE_LINE_NUM_TE_LINE_NUM_UNUSED_SHIFT                     16
#define    AXIPV_TE_LINE_NUM_TE_LINE_NUM_UNUSED_MASK                      0xFFFF0000
#define    AXIPV_TE_LINE_NUM_TE_LINE_NUM_SHIFT                            0
#define    AXIPV_TE_LINE_NUM_TE_LINE_NUM_MASK                             0x0000FFFF

#define AXIPV_CTRL_REG_OFFSET                                             0x0000002C
#define AXIPV_CTRL_REG_TYPE                                               UInt32
#define AXIPV_CTRL_REG_RESERVED_MASK                                      0x00000000
#define    AXIPV_CTRL_REG_RES_REG_CTRL_31_29_SHIFT                        29
#define    AXIPV_CTRL_REG_RES_REG_CTRL_31_29_MASK                         0xE0000000
#define    AXIPV_CTRL_REG_AXIPV_DBG_CLK_DIV_SHIFT                         27
#define    AXIPV_CTRL_REG_AXIPV_DBG_CLK_DIV_MASK                          0x18000000
#define    AXIPV_CTRL_REG_AXIPV_DBG_SEL_SHIFT                             21
#define    AXIPV_CTRL_REG_AXIPV_DBG_SEL_MASK                              0x07E00000
#define    AXIPV_CTRL_REG_TEST_DATA_RDY_SHIFT                             20
#define    AXIPV_CTRL_REG_TEST_DATA_RDY_MASK                              0x00100000
#define    AXIPV_CTRL_REG_AXIPV_TESTMODE_SHIFT                            19
#define    AXIPV_CTRL_REG_AXIPV_TESTMODE_MASK                             0x00080000
#define    AXIPV_CTRL_REG_ARCACHE_SHIFT                                   15
#define    AXIPV_CTRL_REG_ARCACHE_MASK                                    0x00078000
#define    AXIPV_CTRL_REG_ARPROT_SHIFT                                    12
#define    AXIPV_CTRL_REG_ARPROT_MASK                                     0x00007000
#define    AXIPV_CTRL_REG_AXI_ID_SYS_SHIFT                                11
#define    AXIPV_CTRL_REG_AXI_ID_SYS_MASK                                 0x00000800
#define    AXIPV_CTRL_REG_NUM_OUTSTDG_XFERS_SHIFT                         9
#define    AXIPV_CTRL_REG_NUM_OUTSTDG_XFERS_MASK                          0x00000600
#define    AXIPV_CTRL_REG_RES_REG_CTRL_8_SHIFT                            8
#define    AXIPV_CTRL_REG_RES_REG_CTRL_8_MASK                             0x00000100
#define    AXIPV_CTRL_REG_PIXEL_FORMAT_SHIFT                              5
#define    AXIPV_CTRL_REG_PIXEL_FORMAT_MASK                               0x000000E0
#define    AXIPV_CTRL_REG_AXIPV_ACTIVE_SHIFT                              4
#define    AXIPV_CTRL_REG_AXIPV_ACTIVE_MASK                               0x00000010
#define    AXIPV_CTRL_REG_AXIPV_MODE_SHIFT                                3
#define    AXIPV_CTRL_REG_AXIPV_MODE_MASK                                 0x00000008
#define    AXIPV_CTRL_REG_SINGLE_SHOT_FRAME_SHIFT                         2
#define    AXIPV_CTRL_REG_SINGLE_SHOT_FRAME_MASK                          0x00000004
#define    AXIPV_CTRL_REG_SFT_RSTN_SHIFT                                  1
#define    AXIPV_CTRL_REG_SFT_RSTN_MASK                                   0x00000002
#define    AXIPV_CTRL_REG_AXIPV_EN_SHIFT                                  0
#define    AXIPV_CTRL_REG_AXIPV_EN_MASK                                   0x00000001

#define AXIPV_STATUS_REG_OFFSET                                           0x00000030
#define AXIPV_STATUS_REG_TYPE                                             UInt32
#define AXIPV_STATUS_REG_RESERVED_MASK                                    0x00000000
#define    AXIPV_STATUS_REG_CUR_LINE_NUM_SHIFT                            16
#define    AXIPV_STATUS_REG_CUR_LINE_NUM_MASK                             0xFFFF0000
#define    AXIPV_STATUS_REG_AXIPV_STATUS_UNUSED_SHIFT                     0
#define    AXIPV_STATUS_REG_AXIPV_STATUS_UNUSED_MASK                      0x0000FFFF

#define AXIPV_INTR_EN_REG_OFFSET                                          0x00000034
#define AXIPV_INTR_EN_REG_TYPE                                            UInt32
#define AXIPV_INTR_EN_REG_RESERVED_MASK                                   0x00000000
#define    AXIPV_INTR_EN_REG_AXIPV_INTR_EN_UNUSED_SHIFT                   9
#define    AXIPV_INTR_EN_REG_AXIPV_INTR_EN_UNUSED_MASK                    0xFFFFFE00
#define    AXIPV_INTR_EN_REG_LB_ALMOST_EMPTY_INT_EN_SHIFT                 8
#define    AXIPV_INTR_EN_REG_LB_ALMOST_EMPTY_INT_EN_MASK                  0x00000100
#define    AXIPV_INTR_EN_REG_PV_START_THRESH_INT_EN_SHIFT                 7
#define    AXIPV_INTR_EN_REG_PV_START_THRESH_INT_EN_MASK                  0x00000080
#define    AXIPV_INTR_EN_REG_AXIPV_DISABLED_EN_SHIFT                      6
#define    AXIPV_INTR_EN_REG_AXIPV_DISABLED_EN_MASK                       0x00000040
#define    AXIPV_INTR_EN_REG_FRAME_END_EN_SHIFT                           5
#define    AXIPV_INTR_EN_REG_FRAME_END_EN_MASK                            0x00000020
#define    AXIPV_INTR_EN_REG_ERR_ON_RDATA_EN_SHIFT                        4
#define    AXIPV_INTR_EN_REG_ERR_ON_RDATA_EN_MASK                         0x00000010
#define    AXIPV_INTR_EN_REG_FRAME_UNDFL_EN_SHIFT                         3
#define    AXIPV_INTR_EN_REG_FRAME_UNDFL_EN_MASK                          0x00000008
#define    AXIPV_INTR_EN_REG_LINE_BUF_UNDFLW_EN_SHIFT                     2
#define    AXIPV_INTR_EN_REG_LINE_BUF_UNDFLW_EN_MASK                      0x00000004
#define    AXIPV_INTR_EN_REG_WATER_LVL2_INTPT_EN_SHIFT                    1
#define    AXIPV_INTR_EN_REG_WATER_LVL2_INTPT_EN_MASK                     0x00000002
#define    AXIPV_INTR_EN_REG_TE_INTPT_EN_SHIFT                            0
#define    AXIPV_INTR_EN_REG_TE_INTPT_EN_MASK                             0x00000001

#define AXIPV_INTR_STAT_REG_OFFSET                                        0x00000038
#define AXIPV_INTR_STAT_REG_TYPE                                          UInt32
#define AXIPV_INTR_STAT_REG_RESERVED_MASK                                 0x00000000
#define    AXIPV_INTR_STAT_REG_AXIPV_INTR_STAT_UNUSED_SHIFT               9
#define    AXIPV_INTR_STAT_REG_AXIPV_INTR_STAT_UNUSED_MASK                0xFFFFFE00
#define    AXIPV_INTR_STAT_REG_LB_ALMOST_EMPTY_INT_SHIFT                  8
#define    AXIPV_INTR_STAT_REG_LB_ALMOST_EMPTY_INT_MASK                   0x00000100
#define    AXIPV_INTR_STAT_REG_PV_START_THRESH_INT_SHIFT                  7
#define    AXIPV_INTR_STAT_REG_PV_START_THRESH_INT_MASK                   0x00000080
#define    AXIPV_INTR_STAT_REG_AXIPV_DISABLED_SHIFT                       6
#define    AXIPV_INTR_STAT_REG_AXIPV_DISABLED_MASK                        0x00000040
#define    AXIPV_INTR_STAT_REG_FRAME_END_SHIFT                            5
#define    AXIPV_INTR_STAT_REG_FRAME_END_MASK                             0x00000020
#define    AXIPV_INTR_STAT_REG_ERR_ON_RDATA_SHIFT                         4
#define    AXIPV_INTR_STAT_REG_ERR_ON_RDATA_MASK                          0x00000010
#define    AXIPV_INTR_STAT_REG_FRAME_UNDFL_SHIFT                          3
#define    AXIPV_INTR_STAT_REG_FRAME_UNDFL_MASK                           0x00000008
#define    AXIPV_INTR_STAT_REG_LINE_BUF_UNDFL_SHIFT                       2
#define    AXIPV_INTR_STAT_REG_LINE_BUF_UNDFL_MASK                        0x00000004
#define    AXIPV_INTR_STAT_REG_WATER_LVL2_INTPT_SHIFT                     1
#define    AXIPV_INTR_STAT_REG_WATER_LVL2_INTPT_MASK                      0x00000002
#define    AXIPV_INTR_STAT_REG_TE_INTPT_SHIFT                             0
#define    AXIPV_INTR_STAT_REG_TE_INTPT_MASK                              0x00000001

#define AXIPV_INTR_CLR_REG_OFFSET                                         0x0000003C
#define AXIPV_INTR_CLR_REG_TYPE                                           UInt32
#define AXIPV_INTR_CLR_REG_RESERVED_MASK                                  0x00000000
#define    AXIPV_INTR_CLR_REG_AXIPV_INTR_CLR_UNUSED_SHIFT                 9
#define    AXIPV_INTR_CLR_REG_AXIPV_INTR_CLR_UNUSED_MASK                  0xFFFFFE00
#define    AXIPV_INTR_CLR_REG_LB_ALMOST_EMPTY_INT_CLR_SHIFT               8
#define    AXIPV_INTR_CLR_REG_LB_ALMOST_EMPTY_INT_CLR_MASK                0x00000100
#define    AXIPV_INTR_CLR_REG_PV_START_THRESH_INT_CLR_SHIFT               7
#define    AXIPV_INTR_CLR_REG_PV_START_THRESH_INT_CLR_MASK                0x00000080
#define    AXIPV_INTR_CLR_REG_AXIPV_DISABLED_CLR_SHIFT                    6
#define    AXIPV_INTR_CLR_REG_AXIPV_DISABLED_CLR_MASK                     0x00000040
#define    AXIPV_INTR_CLR_REG_FRAME_END_CLR_SHIFT                         5
#define    AXIPV_INTR_CLR_REG_FRAME_END_CLR_MASK                          0x00000020
#define    AXIPV_INTR_CLR_REG_ERR_ON_RDATA_CLR_SHIFT                      4
#define    AXIPV_INTR_CLR_REG_ERR_ON_RDATA_CLR_MASK                       0x00000010
#define    AXIPV_INTR_CLR_REG_FRAME_UNDFL_CLR_SHIFT                       3
#define    AXIPV_INTR_CLR_REG_FRAME_UNDFL_CLR_MASK                        0x00000008
#define    AXIPV_INTR_CLR_REG_LINE_BUF_UNDFL_CLR_SHIFT                    2
#define    AXIPV_INTR_CLR_REG_LINE_BUF_UNDFL_CLR_MASK                     0x00000004
#define    AXIPV_INTR_CLR_REG_WATER_LVL2_INTPT_CLR_SHIFT                  1
#define    AXIPV_INTR_CLR_REG_WATER_LVL2_INTPT_CLR_MASK                   0x00000002
#define    AXIPV_INTR_CLR_REG_TE_INTPT_CLR_SHIFT                          0
#define    AXIPV_INTR_CLR_REG_TE_INTPT_CLR_MASK                           0x00000001

#define AXIPV_AXI_ID_CFG_1_OFFSET                                         0x00000040
#define AXIPV_AXI_ID_CFG_1_TYPE                                           UInt32
#define AXIPV_AXI_ID_CFG_1_RESERVED_MASK                                  0x00000000
#define    AXIPV_AXI_ID_CFG_1_AXI_ID_CFG_1_UNUSED_SHIFT                   8
#define    AXIPV_AXI_ID_CFG_1_AXI_ID_CFG_1_UNUSED_MASK                    0xFFFFFF00
#define    AXIPV_AXI_ID_CFG_1_AXI_ID_CFG_1_SHIFT                          0
#define    AXIPV_AXI_ID_CFG_1_AXI_ID_CFG_1_MASK                           0x000000FF

#define AXIPV_AXI_ID_CFG_2_OFFSET                                         0x00000044
#define AXIPV_AXI_ID_CFG_2_TYPE                                           UInt32
#define AXIPV_AXI_ID_CFG_2_RESERVED_MASK                                  0x00000000
#define    AXIPV_AXI_ID_CFG_2_AXI_ID_CFG_2_UNUSED_SHIFT                   8
#define    AXIPV_AXI_ID_CFG_2_AXI_ID_CFG_2_UNUSED_MASK                    0xFFFFFF00
#define    AXIPV_AXI_ID_CFG_2_AXI_ID_CFG_2_SHIFT                          0
#define    AXIPV_AXI_ID_CFG_2_AXI_ID_CFG_2_MASK                           0x000000FF

#define AXIPV_LINEBUFF_EMPTY_THRES_OFFSET                                 0x00000048
#define AXIPV_LINEBUFF_EMPTY_THRES_TYPE                                   UInt32
#define AXIPV_LINEBUFF_EMPTY_THRES_RESERVED_MASK                          0x00000000
#define    AXIPV_LINEBUFF_EMPTY_THRES_LINEBUFF_EMPTY_THRES_UNUSED_SHIFT   16
#define    AXIPV_LINEBUFF_EMPTY_THRES_LINEBUFF_EMPTY_THRES_UNUSED_MASK    0xFFFF0000
#define    AXIPV_LINEBUFF_EMPTY_THRES_LINEBUFF_EMPTY_THRES_SHIFT          0
#define    AXIPV_LINEBUFF_EMPTY_THRES_LINEBUFF_EMPTY_THRES_MASK           0x0000FFFF

#define AXIPV_LINEBUFF_FULL_THRES_UNUSED_OFFSET                           0x0000004C
#define AXIPV_LINEBUFF_FULL_THRES_UNUSED_TYPE                             UInt32
#define AXIPV_LINEBUFF_FULL_THRES_UNUSED_RESERVED_MASK                    0x00000000
#define    AXIPV_LINEBUFF_FULL_THRES_UNUSED_LINEBUFF_FULL_THRES_UNUSED_SHIFT 0
#define    AXIPV_LINEBUFF_FULL_THRES_UNUSED_LINEBUFF_FULL_THRES_UNUSED_MASK 0xFFFFFFFF

#define AXIPV_LINE_BUF_WPTR_OFFSET                                        0x00000050
#define AXIPV_LINE_BUF_WPTR_TYPE                                          UInt32
#define AXIPV_LINE_BUF_WPTR_RESERVED_MASK                                 0x00000000
#define    AXIPV_LINE_BUF_WPTR_LINE_BUF_WPTR_UNUSED_SHIFT                 17
#define    AXIPV_LINE_BUF_WPTR_LINE_BUF_WPTR_UNUSED_MASK                  0xFFFE0000
#define    AXIPV_LINE_BUF_WPTR_LINE_BUF_WPTR_SHIFT                        0
#define    AXIPV_LINE_BUF_WPTR_LINE_BUF_WPTR_MASK                         0x0001FFFF

#define AXIPV_LINE_BUF_RPTR_OFFSET                                        0x00000054
#define AXIPV_LINE_BUF_RPTR_TYPE                                          UInt32
#define AXIPV_LINE_BUF_RPTR_RESERVED_MASK                                 0x00000000
#define    AXIPV_LINE_BUF_RPTR_LINE_BUF_RPTR_UNUSED_SHIFT                 17
#define    AXIPV_LINE_BUF_RPTR_LINE_BUF_RPTR_UNUSED_MASK                  0xFFFE0000
#define    AXIPV_LINE_BUF_RPTR_LINE_BUF_RPTR_SHIFT                        0
#define    AXIPV_LINE_BUF_RPTR_LINE_BUF_RPTR_MASK                         0x0001FFFF

#define AXIPV_LINE_BUFF_DATA_H_OFFSET                                     0x00000058
#define AXIPV_LINE_BUFF_DATA_H_TYPE                                       UInt32
#define AXIPV_LINE_BUFF_DATA_H_RESERVED_MASK                              0x00000000
#define    AXIPV_LINE_BUFF_DATA_H_LINE_BUFF_DATA_H_SHIFT                  0
#define    AXIPV_LINE_BUFF_DATA_H_LINE_BUFF_DATA_H_MASK                   0xFFFFFFFF

#define AXIPV_LINE_BUFF_DATA_L_OFFSET                                     0x0000005C
#define AXIPV_LINE_BUFF_DATA_L_TYPE                                       UInt32
#define AXIPV_LINE_BUFF_DATA_L_RESERVED_MASK                              0x00000000
#define    AXIPV_LINE_BUFF_DATA_L_LINE_BUFF_DATA_L_SHIFT                  0
#define    AXIPV_LINE_BUFF_DATA_L_LINE_BUFF_DATA_L_MASK                   0xFFFFFFFF

#define AXIPV_RES_REG_1_OFFSET                                            0x00000060
#define AXIPV_RES_REG_1_TYPE                                              UInt32
#define AXIPV_RES_REG_1_RESERVED_MASK                                     0x00000000
#define    AXIPV_RES_REG_1_RES_REG_1_SHIFT                                0
#define    AXIPV_RES_REG_1_RES_REG_1_MASK                                 0xFFFFFFFF

#define AXIPV_RES_REG_2_OFFSET                                            0x00000064
#define AXIPV_RES_REG_2_TYPE                                              UInt32
#define AXIPV_RES_REG_2_RESERVED_MASK                                     0x00000000
#define    AXIPV_RES_REG_2_RES_REG_2_SHIFT                                0
#define    AXIPV_RES_REG_2_RES_REG_2_MASK                                 0xFFFFFFFF

#endif /* __BRCM_RDB_AXIPV_H__ */



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

#ifndef __BRCM_RDB_RAKE_FNG_TOP_H__
#define __BRCM_RDB_RAKE_FNG_TOP_H__

#define RAKE_FNG_TOP_FNG_CPI_VPGAIN_OFFSET                                0x00000000
#define RAKE_FNG_TOP_FNG_CPI_VPGAIN_TYPE                                  UInt32
#define RAKE_FNG_TOP_FNG_CPI_VPGAIN_RESERVED_MASK                         0x00000000
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG7_CPI_VPGAIN_SHIFT              28
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG7_CPI_VPGAIN_MASK               0xF0000000
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG6_CPI_VPGAIN_SHIFT              24
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG6_CPI_VPGAIN_MASK               0x0F000000
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG5_CPI_VPGAIN_SHIFT              20
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG5_CPI_VPGAIN_MASK               0x00F00000
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG4_CPI_VPGAIN_SHIFT              16
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG4_CPI_VPGAIN_MASK               0x000F0000
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG3_CPI_VPGAIN_SHIFT              12
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG3_CPI_VPGAIN_MASK               0x0000F000
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG2_CPI_VPGAIN_SHIFT              8
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG2_CPI_VPGAIN_MASK               0x00000F00
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG1_CPI_VPGAIN_SHIFT              4
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG1_CPI_VPGAIN_MASK               0x000000F0
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG0_CPI_VPGAIN_SHIFT              0
#define    RAKE_FNG_TOP_FNG_CPI_VPGAIN_FNG0_CPI_VPGAIN_MASK               0x0000000F

#define RAKE_FNG_TOP_MUX_SELECTION_OFFSET                                 0x00000004
#define RAKE_FNG_TOP_MUX_SELECTION_TYPE                                   UInt32
#define RAKE_FNG_TOP_MUX_SELECTION_RESERVED_MASK                          0xFFFFFEEE
#define    RAKE_FNG_TOP_MUX_SELECTION_EDCH_FNG_CHEST_SHIFT                8
#define    RAKE_FNG_TOP_MUX_SELECTION_EDCH_FNG_CHEST_MASK                 0x00000100
#define    RAKE_FNG_TOP_MUX_SELECTION_CH3_COMPUTATION_MUX_SHIFT           4
#define    RAKE_FNG_TOP_MUX_SELECTION_CH3_COMPUTATION_MUX_MASK            0x00000010
#define    RAKE_FNG_TOP_MUX_SELECTION_NEW_FNG_POS_IMP_SHIFT               0
#define    RAKE_FNG_TOP_MUX_SELECTION_NEW_FNG_POS_IMP_MASK                0x00000001

#define RAKE_FNG_TOP_DBL_BUF_SELECTION_OFFSET                             0x00000008
#define RAKE_FNG_TOP_DBL_BUF_SELECTION_TYPE                               UInt32
#define RAKE_FNG_TOP_DBL_BUF_SELECTION_RESERVED_MASK                      0xFFFFFFEE
#define    RAKE_FNG_TOP_DBL_BUF_SELECTION_LATCH_TYPE_SHIFT                4
#define    RAKE_FNG_TOP_DBL_BUF_SELECTION_LATCH_TYPE_MASK                 0x00000010
#define    RAKE_FNG_TOP_DBL_BUF_SELECTION_AGCH_FNG_LATCH_PER_SHIFT        0
#define    RAKE_FNG_TOP_DBL_BUF_SELECTION_AGCH_FNG_LATCH_PER_MASK         0x00000001

#define RAKE_FNG_TOP_FNG_DMM_START_OFFSET                                 0x0000000C
#define RAKE_FNG_TOP_FNG_DMM_START_TYPE                                   UInt32
#define RAKE_FNG_TOP_FNG_DMM_START_RESERVED_MASK                          0x770C000E
#define    RAKE_FNG_TOP_FNG_DMM_START_USE_MST_CNT_FOR_DMM_SHIFT           31
#define    RAKE_FNG_TOP_FNG_DMM_START_USE_MST_CNT_FOR_DMM_MASK            0x80000000
#define    RAKE_FNG_TOP_FNG_DMM_START_DEBUG_PRISM_EN_SHIFT                27
#define    RAKE_FNG_TOP_FNG_DMM_START_DEBUG_PRISM_EN_MASK                 0x08000000
#define    RAKE_FNG_TOP_FNG_DMM_START_MST_SLOT_DMM_START_SHIFT            20
#define    RAKE_FNG_TOP_FNG_DMM_START_MST_SLOT_DMM_START_MASK             0x00F00000
#define    RAKE_FNG_TOP_FNG_DMM_START_MST_CNT_DMM_START_SHIFT             4
#define    RAKE_FNG_TOP_FNG_DMM_START_MST_CNT_DMM_START_MASK              0x0003FFF0
#define    RAKE_FNG_TOP_FNG_DMM_START_PRISM_RAKE_DEBUG_SHIFT              0
#define    RAKE_FNG_TOP_FNG_DMM_START_PRISM_RAKE_DEBUG_MASK               0x00000001

#define RAKE_FNG_TOP_HSUPA_DL_DELAY_OFFSET                                0x00000010
#define RAKE_FNG_TOP_HSUPA_DL_DELAY_TYPE                                  UInt32
#define RAKE_FNG_TOP_HSUPA_DL_DELAY_RESERVED_MASK                         0xEF00E000
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HALF_CHIP_DELAY_HD_SHIFT           28
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HALF_CHIP_DELAY_HD_MASK            0x10000000
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HSUPA_READ_OFFSET_SHIFT            16
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HSUPA_READ_OFFSET_MASK             0x00FF0000
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HSUPA_DM_EN_SHIFT                  12
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HSUPA_DM_EN_MASK                   0x00001000
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HSUPA_DL_DELAY_SHIFT               0
#define    RAKE_FNG_TOP_HSUPA_DL_DELAY_HSUPA_DL_DELAY_MASK                0x00000FFF

#define RAKE_FNG_TOP_FNG_NF_SEL_OFFSET                                    0x00000030
#define RAKE_FNG_TOP_FNG_NF_SEL_TYPE                                      UInt32
#define RAKE_FNG_TOP_FNG_NF_SEL_RESERVED_MASK                             0xFFE00808
#define    RAKE_FNG_TOP_FNG_NF_SEL_CPICH_RSCP_ACC_MODE_SHIFT              20
#define    RAKE_FNG_TOP_FNG_NF_SEL_CPICH_RSCP_ACC_MODE_MASK               0x00100000
#define    RAKE_FNG_TOP_FNG_NF_SEL_SCH_NPE2_ACCLEN_SHIFT                  16
#define    RAKE_FNG_TOP_FNG_NF_SEL_SCH_NPE2_ACCLEN_MASK                   0x000F0000
#define    RAKE_FNG_TOP_FNG_NF_SEL_SCH_NPE1_ACCLEN_SHIFT                  12
#define    RAKE_FNG_TOP_FNG_NF_SEL_SCH_NPE1_ACCLEN_MASK                   0x0000F000
#define    RAKE_FNG_TOP_FNG_NF_SEL_RSCP_ACCLEN_SHIFT                      4
#define    RAKE_FNG_TOP_FNG_NF_SEL_RSCP_ACCLEN_MASK                       0x000007F0
#define    RAKE_FNG_TOP_FNG_NF_SEL_FNG_NF_SEL_SHIFT                       0
#define    RAKE_FNG_TOP_FNG_NF_SEL_FNG_NF_SEL_MASK                        0x00000007

#define RAKE_FNG_TOP_NF_ACC_RSCP1_OFFSET                                  0x00000034
#define RAKE_FNG_TOP_NF_ACC_RSCP1_TYPE                                    UInt32
#define RAKE_FNG_TOP_NF_ACC_RSCP1_RESERVED_MASK                           0x00000000
#define    RAKE_FNG_TOP_NF_ACC_RSCP1_NF_ACC1_RSCP_SHIFT                   16
#define    RAKE_FNG_TOP_NF_ACC_RSCP1_NF_ACC1_RSCP_MASK                    0xFFFF0000
#define    RAKE_FNG_TOP_NF_ACC_RSCP1_NF_ACC0_RSCP_SHIFT                   0
#define    RAKE_FNG_TOP_NF_ACC_RSCP1_NF_ACC0_RSCP_MASK                    0x0000FFFF

#define RAKE_FNG_TOP_NF_ACC_RSCP2_OFFSET                                  0x00000038
#define RAKE_FNG_TOP_NF_ACC_RSCP2_TYPE                                    UInt32
#define RAKE_FNG_TOP_NF_ACC_RSCP2_RESERVED_MASK                           0x00000000
#define    RAKE_FNG_TOP_NF_ACC_RSCP2_NF_ACC3_RSCP_SHIFT                   16
#define    RAKE_FNG_TOP_NF_ACC_RSCP2_NF_ACC3_RSCP_MASK                    0xFFFF0000
#define    RAKE_FNG_TOP_NF_ACC_RSCP2_NF_ACC2_RSCP_SHIFT                   0
#define    RAKE_FNG_TOP_NF_ACC_RSCP2_NF_ACC2_RSCP_MASK                    0x0000FFFF

#define RAKE_FNG_TOP_HSUPA_INT_FNG_SEL_OFFSET                             0x0000003C
#define RAKE_FNG_TOP_HSUPA_INT_FNG_SEL_TYPE                               UInt32
#define RAKE_FNG_TOP_HSUPA_INT_FNG_SEL_RESERVED_MASK                      0xFFFFFF00
#define    RAKE_FNG_TOP_HSUPA_INT_FNG_SEL_HSUPA_INT_FNG_SEL_SHIFT         0
#define    RAKE_FNG_TOP_HSUPA_INT_FNG_SEL_HSUPA_INT_FNG_SEL_MASK          0x000000FF

#define RAKE_FNG_TOP_DEBUG_SEL_OFFSET                                     0x00000040
#define RAKE_FNG_TOP_DEBUG_SEL_TYPE                                       UInt32
#define RAKE_FNG_TOP_DEBUG_SEL_RESERVED_MASK                              0xFEE88CCE
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_BUS_SEL_SHIFT                     24
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_BUS_SEL_MASK                      0x01000000
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_CNT_SEL_SHIFT                     20
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_CNT_SEL_MASK                      0x00100000
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_FNG_SEL_SHIFT                     16
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_FNG_SEL_MASK                      0x00070000
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_CHN_SEL_SHIFT                     12
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_CHN_SEL_MASK                      0x00007000
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_ANT_SEL_SHIFT                     8
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_ANT_SEL_MASK                      0x00000300
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_BLK_SEL_SHIFT                     4
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_BLK_SEL_MASK                      0x00000030
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_SYM_SEL_SHIFT                     0
#define    RAKE_FNG_TOP_DEBUG_SEL_DEBUG_SYM_SEL_MASK                      0x00000001

#define RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_OFFSET                            0x00000048
#define RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_TYPE                              UInt32
#define RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_RESERVED_MASK                     0xEE0CE000
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_HALF_CHIP_DELAY_RD_SHIFT       28
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_HALF_CHIP_DELAY_RD_MASK        0x10000000
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_DMM_DEC_SAMPLE_SHIFT           24
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_DMM_DEC_SAMPLE_MASK            0x01000000
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_PRISM_DELAY_RB_0010_SHIFT      20
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_PRISM_DELAY_RB_0010_MASK       0x00F00000
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_HALF_CHIP_BOUNDARY_SHIFT       16
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_HALF_CHIP_BOUNDARY_MASK        0x00030000
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_FIFO_DLY_EN_SHIFT              12
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_FIFO_DLY_EN_MASK               0x00001000
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_FIFO_DLYLEN_RH_97F_SHIFT       0
#define    RAKE_FNG_TOP_FIFO_DLYLEN_RH_97F_FIFO_DLYLEN_RH_97F_MASK        0x00000FFF

#define RAKE_FNG_TOP_CPM_CTRL_OFFSET                                      0x0000004C
#define RAKE_FNG_TOP_CPM_CTRL_TYPE                                        UInt32
#define RAKE_FNG_TOP_CPM_CTRL_RESERVED_MASK                               0xFFFE00EE
#define    RAKE_FNG_TOP_CPM_CTRL_CPM_NEW_SHIFT                            16
#define    RAKE_FNG_TOP_CPM_CTRL_CPM_NEW_MASK                             0x00010000
#define    RAKE_FNG_TOP_CPM_CTRL_TPC_FRAME_NUM_SHIFT                      12
#define    RAKE_FNG_TOP_CPM_CTRL_TPC_FRAME_NUM_MASK                       0x0000F000
#define    RAKE_FNG_TOP_CPM_CTRL_CPM_FRAME_NUM_SHIFT                      8
#define    RAKE_FNG_TOP_CPM_CTRL_CPM_FRAME_NUM_MASK                       0x00000F00
#define    RAKE_FNG_TOP_CPM_CTRL_SW_TPC_EN_SHIFT                          4
#define    RAKE_FNG_TOP_CPM_CTRL_SW_TPC_EN_MASK                           0x00000010
#define    RAKE_FNG_TOP_CPM_CTRL_SW_CPM_EN_SHIFT                          0
#define    RAKE_FNG_TOP_CPM_CTRL_SW_CPM_EN_MASK                           0x00000001

#define RAKE_FNG_TOP_CPM_START_OFFSET                                     0x00000050
#define RAKE_FNG_TOP_CPM_START_TYPE                                       UInt32
#define RAKE_FNG_TOP_CPM_START_RESERVED_MASK                              0x0000C000
#define    RAKE_FNG_TOP_CPM_START_CPM_START_FRAMEPOS_SHIFT                20
#define    RAKE_FNG_TOP_CPM_START_CPM_START_FRAMEPOS_MASK                 0xFFF00000
#define    RAKE_FNG_TOP_CPM_START_CPM_START_SLOTPOS_SHIFT                 16
#define    RAKE_FNG_TOP_CPM_START_CPM_START_SLOTPOS_MASK                  0x000F0000
#define    RAKE_FNG_TOP_CPM_START_CPM_START_SAMPLEPOS_SHIFT               0
#define    RAKE_FNG_TOP_CPM_START_CPM_START_SAMPLEPOS_MASK                0x00003FFF

#define RAKE_FNG_TOP_CPM_STOP_OFFSET                                      0x00000054
#define RAKE_FNG_TOP_CPM_STOP_TYPE                                        UInt32
#define RAKE_FNG_TOP_CPM_STOP_RESERVED_MASK                               0x0000C000
#define    RAKE_FNG_TOP_CPM_STOP_CPM_STOP_FRAMEPOS_SHIFT                  20
#define    RAKE_FNG_TOP_CPM_STOP_CPM_STOP_FRAMEPOS_MASK                   0xFFF00000
#define    RAKE_FNG_TOP_CPM_STOP_CPM_STOP_SLOTPOS_SHIFT                   16
#define    RAKE_FNG_TOP_CPM_STOP_CPM_STOP_SLOTPOS_MASK                    0x000F0000
#define    RAKE_FNG_TOP_CPM_STOP_CPM_STOP_SAMPLEPOS_SHIFT                 0
#define    RAKE_FNG_TOP_CPM_STOP_CPM_STOP_SAMPLEPOS_MASK                  0x00003FFF

#define RAKE_FNG_TOP_TPC_START_OFFSET                                     0x00000058
#define RAKE_FNG_TOP_TPC_START_TYPE                                       UInt32
#define RAKE_FNG_TOP_TPC_START_RESERVED_MASK                              0xFFF0C000
#define    RAKE_FNG_TOP_TPC_START_TPC_START_SLOTPOS_SHIFT                 16
#define    RAKE_FNG_TOP_TPC_START_TPC_START_SLOTPOS_MASK                  0x000F0000
#define    RAKE_FNG_TOP_TPC_START_TPC_START_SAMPLEPOS_SHIFT               0
#define    RAKE_FNG_TOP_TPC_START_TPC_START_SAMPLEPOS_MASK                0x00003FFF

#define RAKE_FNG_TOP_TPC_STOP_OFFSET                                      0x0000005C
#define RAKE_FNG_TOP_TPC_STOP_TYPE                                        UInt32
#define RAKE_FNG_TOP_TPC_STOP_RESERVED_MASK                               0xFFF0C000
#define    RAKE_FNG_TOP_TPC_STOP_TPC_STOP_SLOTPOS_SHIFT                   16
#define    RAKE_FNG_TOP_TPC_STOP_TPC_STOP_SLOTPOS_MASK                    0x000F0000
#define    RAKE_FNG_TOP_TPC_STOP_TPC_STOP_SAMPLEPOS_SHIFT                 0
#define    RAKE_FNG_TOP_TPC_STOP_TPC_STOP_SAMPLEPOS_MASK                  0x00003FFF

#define RAKE_FNG_TOP_SW_ACQ_EN_OFFSET                                     0x00000064
#define RAKE_FNG_TOP_SW_ACQ_EN_TYPE                                       UInt32
#define RAKE_FNG_TOP_SW_ACQ_EN_RESERVED_MASK                              0xFFFFEEEE
#define    RAKE_FNG_TOP_SW_ACQ_EN_CLTD_DBUF_MODE_SHIFT                    12
#define    RAKE_FNG_TOP_SW_ACQ_EN_CLTD_DBUF_MODE_MASK                     0x00001000
#define    RAKE_FNG_TOP_SW_ACQ_EN_IIR_IMP_TYPE_SHIFT                      8
#define    RAKE_FNG_TOP_SW_ACQ_EN_IIR_IMP_TYPE_MASK                       0x00000100
#define    RAKE_FNG_TOP_SW_ACQ_EN_BYPASS_LPF_SHIFT                        4
#define    RAKE_FNG_TOP_SW_ACQ_EN_BYPASS_LPF_MASK                         0x00000010
#define    RAKE_FNG_TOP_SW_ACQ_EN_SW_ACQ_EN_SHIFT                         0
#define    RAKE_FNG_TOP_SW_ACQ_EN_SW_ACQ_EN_MASK                          0x00000001

#define RAKE_FNG_TOP_DPHASE_CTRL_OFFSET                                   0x00000068
#define RAKE_FNG_TOP_DPHASE_CTRL_TYPE                                     UInt32
#define RAKE_FNG_TOP_DPHASE_CTRL_RESERVED_MASK                            0xC0C0C0EE
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_RSCP_QUAL_SHIFT                24
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_RSCP_QUAL_MASK                 0x3F000000
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_SQ_MARGIN_SHIFT                16
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_SQ_MARGIN_MASK                 0x003F0000
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_INTERVAL_SHIFT                 8
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_INTERVAL_MASK                  0x00003F00
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_CTRL_SHIFT                     4
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_CTRL_MASK                      0x00000010
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_EST_EN_SHIFT                   0
#define    RAKE_FNG_TOP_DPHASE_CTRL_DPHASE_EST_EN_MASK                    0x00000001

#define RAKE_FNG_TOP_FNG_DPHASE_REPORT1_OFFSET                            0x0000006C
#define RAKE_FNG_TOP_FNG_DPHASE_REPORT1_TYPE                              UInt32
#define RAKE_FNG_TOP_FNG_DPHASE_REPORT1_RESERVED_MASK                     0x00000000
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT1_FNG1_DPHASE_REPORT_RH_FFFF_SHIFT 16
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT1_FNG1_DPHASE_REPORT_RH_FFFF_MASK 0xFFFF0000
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT1_FNG0_DPHASE_REPORT_RH_FFFF_SHIFT 0
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT1_FNG0_DPHASE_REPORT_RH_FFFF_MASK 0x0000FFFF

#define RAKE_FNG_TOP_FNG_DPHASE_REPORT2_OFFSET                            0x00000070
#define RAKE_FNG_TOP_FNG_DPHASE_REPORT2_TYPE                              UInt32
#define RAKE_FNG_TOP_FNG_DPHASE_REPORT2_RESERVED_MASK                     0x00000000
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT2_FNG3_DPHASE_REPORT_RH_FFFF_SHIFT 16
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT2_FNG3_DPHASE_REPORT_RH_FFFF_MASK 0xFFFF0000
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT2_FNG2_DPHASE_REPORT_RH_FFFF_SHIFT 0
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT2_FNG2_DPHASE_REPORT_RH_FFFF_MASK 0x0000FFFF

#define RAKE_FNG_TOP_FNG_DPHASE_REPORT3_OFFSET                            0x00000074
#define RAKE_FNG_TOP_FNG_DPHASE_REPORT3_TYPE                              UInt32
#define RAKE_FNG_TOP_FNG_DPHASE_REPORT3_RESERVED_MASK                     0x00000000
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT3_FNG5_DPHASE_REPORT_RH_FFFF_SHIFT 16
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT3_FNG5_DPHASE_REPORT_RH_FFFF_MASK 0xFFFF0000
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT3_FNG4_DPHASE_REPORT_RH_FFFF_SHIFT 0
#define    RAKE_FNG_TOP_FNG_DPHASE_REPORT3_FNG4_DPHASE_REPORT_RH_FFFF_MASK 0x0000FFFF

#define RAKE_FNG_TOP_SCH_NP_VP_GAIN1_OFFSET                               0x00000078
#define RAKE_FNG_TOP_SCH_NP_VP_GAIN1_TYPE                                 UInt32
#define RAKE_FNG_TOP_SCH_NP_VP_GAIN1_RESERVED_MASK                        0xFF000000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG5_SCH_NPE1_VPGAIN_SHIFT        20
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG5_SCH_NPE1_VPGAIN_MASK         0x00F00000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG4_SCH_NPE1_VPGAIN_SHIFT        16
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG4_SCH_NPE1_VPGAIN_MASK         0x000F0000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG3_SCH_NPE1_VPGAIN_SHIFT        12
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG3_SCH_NPE1_VPGAIN_MASK         0x0000F000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG2_SCH_NPE1_VPGAIN_SHIFT        8
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG2_SCH_NPE1_VPGAIN_MASK         0x00000F00
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG1_SCH_NPE1_VPGAIN_SHIFT        4
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG1_SCH_NPE1_VPGAIN_MASK         0x000000F0
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG0_SCH_NPE1_VPGAIN_SHIFT        0
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN1_FNG0_SCH_NPE1_VPGAIN_MASK         0x0000000F

#define RAKE_FNG_TOP_SCH_NP_VP_GAIN2_OFFSET                               0x0000007C
#define RAKE_FNG_TOP_SCH_NP_VP_GAIN2_TYPE                                 UInt32
#define RAKE_FNG_TOP_SCH_NP_VP_GAIN2_RESERVED_MASK                        0xFF000000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG5_SCH_NPE2_VPGAIN_SHIFT        20
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG5_SCH_NPE2_VPGAIN_MASK         0x00F00000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG4_SCH_NPE2_VPGAIN_SHIFT        16
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG4_SCH_NPE2_VPGAIN_MASK         0x000F0000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG3_SCH_NPE2_VPGAIN_SHIFT        12
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG3_SCH_NPE2_VPGAIN_MASK         0x0000F000
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG2_SCH_NPE2_VPGAIN_SHIFT        8
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG2_SCH_NPE2_VPGAIN_MASK         0x00000F00
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG1_SCH_NPE2_VPGAIN_SHIFT        4
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG1_SCH_NPE2_VPGAIN_MASK         0x000000F0
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG0_SCH_NPE2_VPGAIN_SHIFT        0
#define    RAKE_FNG_TOP_SCH_NP_VP_GAIN2_FNG0_SCH_NPE2_VPGAIN_MASK         0x0000000F

#endif /* __BRCM_RDB_RAKE_FNG_TOP_H__ */



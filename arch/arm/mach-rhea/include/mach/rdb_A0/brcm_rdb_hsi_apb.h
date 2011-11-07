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

#ifndef __BRCM_RDB_HSI_APB_H__
#define __BRCM_RDB_HSI_APB_H__

#define HSI_APB_TX_CTRL_OFFSET                                            0x00000000
#define HSI_APB_TX_CTRL_TYPE                                              UInt32
#define HSI_APB_TX_CTRL_RESERVED_MASK                                     0x00000000
#define    HSI_APB_TX_CTRL_TX_CTRL_UNUSED_SHIFT                           22
#define    HSI_APB_TX_CTRL_TX_CTRL_UNUSED_MASK                            0xFFC00000
#define    HSI_APB_TX_CTRL_CH_ID_BIT_CNT_SHIFT                            19
#define    HSI_APB_TX_CTRL_CH_ID_BIT_CNT_MASK                             0x00380000
#define    HSI_APB_TX_CTRL_TX_SW_FIFO_PRI_SHIFT                           18
#define    HSI_APB_TX_CTRL_TX_SW_FIFO_PRI_MASK                            0x00040000
#define    HSI_APB_TX_CTRL_TX_SOFT_RST_SHIFT                              17
#define    HSI_APB_TX_CTRL_TX_SOFT_RST_MASK                               0x00020000
#define    HSI_APB_TX_CTRL_TRAIL_BIT_CNT_SHIFT                            12
#define    HSI_APB_TX_CTRL_TRAIL_BIT_CNT_MASK                             0x0001F000
#define    HSI_APB_TX_CTRL_EN_TRAIL_BITS_SHIFT                            11
#define    HSI_APB_TX_CTRL_EN_TRAIL_BITS_MASK                             0x00000800
#define    HSI_APB_TX_CTRL_ACWAKE_SHIFT                                   10
#define    HSI_APB_TX_CTRL_ACWAKE_MASK                                    0x00000400
#define    HSI_APB_TX_CTRL_TRANS_MODE_SHIFT                               8
#define    HSI_APB_TX_CTRL_TRANS_MODE_MASK                                0x00000300
#define    HSI_APB_TX_CTRL_TX_BR_CTRL_SHIFT                               0
#define    HSI_APB_TX_CTRL_TX_BR_CTRL_MASK                                0x000000FF

#define HSI_APB_TX0_FIFO_DATA_OFFSET                                      0x00000004
#define HSI_APB_TX0_FIFO_DATA_TYPE                                        UInt32
#define HSI_APB_TX0_FIFO_DATA_RESERVED_MASK                               0x00000000
#define    HSI_APB_TX0_FIFO_DATA_TX0_DATA_SHIFT                           0
#define    HSI_APB_TX0_FIFO_DATA_TX0_DATA_MASK                            0xFFFFFFFF

#define HSI_APB_TX1_FIFO_DATA_OFFSET                                      0x00000008
#define HSI_APB_TX1_FIFO_DATA_TYPE                                        UInt32
#define HSI_APB_TX1_FIFO_DATA_RESERVED_MASK                               0x00000000
#define    HSI_APB_TX1_FIFO_DATA_TX1_DATA_SHIFT                           0
#define    HSI_APB_TX1_FIFO_DATA_TX1_DATA_MASK                            0xFFFFFFFF

#define HSI_APB_TX_CH_ID_OFFSET                                           0x0000000C
#define HSI_APB_TX_CH_ID_TYPE                                             UInt32
#define HSI_APB_TX_CH_ID_RESERVED_MASK                                    0x00000000
#define    HSI_APB_TX_CH_ID_TX_CH_ID_UNUSED_SHIFT                         8
#define    HSI_APB_TX_CH_ID_TX_CH_ID_UNUSED_MASK                          0xFFFFFF00
#define    HSI_APB_TX_CH_ID_TX1_CH_ID_SHIFT                               4
#define    HSI_APB_TX_CH_ID_TX1_CH_ID_MASK                                0x000000F0
#define    HSI_APB_TX_CH_ID_TX0_CH_ID_SHIFT                               0
#define    HSI_APB_TX_CH_ID_TX0_CH_ID_MASK                                0x0000000F

#define HSI_APB_TX_FIFO_STS_OFFSET                                        0x00000010
#define HSI_APB_TX_FIFO_STS_TYPE                                          UInt32
#define HSI_APB_TX_FIFO_STS_RESERVED_MASK                                 0x00000000
#define    HSI_APB_TX_FIFO_STS_TX_FIFO_STS_UNUSED_SHIFT                   14
#define    HSI_APB_TX_FIFO_STS_TX_FIFO_STS_UNUSED_MASK                    0xFFFFC000
#define    HSI_APB_TX_FIFO_STS_TX1_FIFO_DATA_CNT_SHIFT                    10
#define    HSI_APB_TX_FIFO_STS_TX1_FIFO_DATA_CNT_MASK                     0x00003C00
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_DATA_CNT_UNUSED_SHIFT             8
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_DATA_CNT_UNUSED_MASK              0x00000300
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_DATA_CNT_SHIFT                    4
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_DATA_CNT_MASK                     0x000000F0
#define    HSI_APB_TX_FIFO_STS_TX1_FIFO_EMPTY_SHIFT                       3
#define    HSI_APB_TX_FIFO_STS_TX1_FIFO_EMPTY_MASK                        0x00000008
#define    HSI_APB_TX_FIFO_STS_TX1_FIFO_FULL_SHIFT                        2
#define    HSI_APB_TX_FIFO_STS_TX1_FIFO_FULL_MASK                         0x00000004
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_EMPTY_SHIFT                       1
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_EMPTY_MASK                        0x00000002
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_FULL_SHIFT                        0
#define    HSI_APB_TX_FIFO_STS_TX0_FIFO_FULL_MASK                         0x00000001

#define HSI_APB_TX_FIFO_THRSD_OFFSET                                      0x00000014
#define HSI_APB_TX_FIFO_THRSD_TYPE                                        UInt32
#define HSI_APB_TX_FIFO_THRSD_RESERVED_MASK                               0x00000000
#define    HSI_APB_TX_FIFO_THRSD_TX_FIFO_THRSD_UNUSED_SHIFT               10
#define    HSI_APB_TX_FIFO_THRSD_TX_FIFO_THRSD_UNUSED_MASK                0xFFFFFC00
#define    HSI_APB_TX_FIFO_THRSD_TX1_FIFO_THRSD_SHIFT                     5
#define    HSI_APB_TX_FIFO_THRSD_TX1_FIFO_THRSD_MASK                      0x000003E0
#define    HSI_APB_TX_FIFO_THRSD_TX0_FIFO_THRSD_SHIFT                     0
#define    HSI_APB_TX_FIFO_THRSD_TX0_FIFO_THRSD_MASK                      0x0000001F

#define HSI_APB_TX_DMA_CTRL_OFFSET                                        0x00000018
#define HSI_APB_TX_DMA_CTRL_TYPE                                          UInt32
#define HSI_APB_TX_DMA_CTRL_RESERVED_MASK                                 0x00000000
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH1_UNUSED_SHIFT                    28
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH1_UNUSED_MASK                     0xF0000000
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH1_TRF_CNT_SHIFT                   16
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH1_TRF_CNT_MASK                    0x0FFF0000
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH0_UNUSED_SHIFT                    14
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH0_UNUSED_MASK                     0x0000C000
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH0_TRF_CNT_SHIFT                   2
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH0_TRF_CNT_MASK                    0x00003FFC
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH1_EN_SHIFT                        1
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH1_EN_MASK                         0x00000002
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH0_EN_SHIFT                        0
#define    HSI_APB_TX_DMA_CTRL_TX_DMA_CH0_EN_MASK                         0x00000001

#define HSI_APB_TX_DMA_BURST_LENGTH_OFFSET                                0x0000001C
#define HSI_APB_TX_DMA_BURST_LENGTH_TYPE                                  UInt32
#define HSI_APB_TX_DMA_BURST_LENGTH_RESERVED_MASK                         0x00000000
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_BURST_CH1_UNUSED_SHIFT      21
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_BURST_CH1_UNUSED_MASK       0xFFE00000
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_CH1_BURST_LEN_SHIFT         16
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_CH1_BURST_LEN_MASK          0x001F0000
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_BURST_CH0_UNUSED_SHIFT      5
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_BURST_CH0_UNUSED_MASK       0x0000FFE0
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_CH0_BURST_LEN_SHIFT         0
#define    HSI_APB_TX_DMA_BURST_LENGTH_TX_DMA_CH0_BURST_LEN_MASK          0x0000001F

#define HSI_APB_TX_DMA_STS_OFFSET                                         0x00000020
#define HSI_APB_TX_DMA_STS_TYPE                                           UInt32
#define HSI_APB_TX_DMA_STS_RESERVED_MASK                                  0x00000000
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH1_UNUSED_SHIFT                28
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH1_UNUSED_MASK                 0xF0000000
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH1_TRF_CNT_SHIFT               16
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH1_TRF_CNT_MASK                0x0FFF0000
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH0_UNUSED_SHIFT                12
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH0_UNUSED_MASK                 0x0000F000
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH0_TRF_CNT_SHIFT               0
#define    HSI_APB_TX_DMA_STS_CURR_TX_DMA_CH0_TRF_CNT_MASK                0x00000FFF

#define HSI_APB_RX_CTRL_OFFSET                                            0x00000030
#define HSI_APB_RX_CTRL_TYPE                                              UInt32
#define HSI_APB_RX_CTRL_RESERVED_MASK                                     0x00000000
#define    HSI_APB_RX_CTRL_RX_CTRL_UNUSED_SHIFT                           7
#define    HSI_APB_RX_CTRL_RX_CTRL_UNUSED_MASK                            0xFFFFFF80
#define    HSI_APB_RX_CTRL_INV_RX_CLK_SHIFT                               6
#define    HSI_APB_RX_CTRL_INV_RX_CLK_MASK                                0x00000040
#define    HSI_APB_RX_CTRL_RX_SOFT_RST_SHIFT                              5
#define    HSI_APB_RX_CTRL_RX_SOFT_RST_MASK                               0x00000020
#define    HSI_APB_RX_CTRL_READY_OVERRIDE_SHIFT                           4
#define    HSI_APB_RX_CTRL_READY_OVERRIDE_MASK                            0x00000010
#define    HSI_APB_RX_CTRL_READY_OVERRIDE_EN_SHIFT                        3
#define    HSI_APB_RX_CTRL_READY_OVERRIDE_EN_MASK                         0x00000008
#define    HSI_APB_RX_CTRL_CAWAKE_SHIFT                                   2
#define    HSI_APB_RX_CTRL_CAWAKE_MASK                                    0x00000004
#define    HSI_APB_RX_CTRL_DATA_FLOW_SHIFT                                0
#define    HSI_APB_RX_CTRL_DATA_FLOW_MASK                                 0x00000003

#define HSI_APB_RX0_FIFO_DATA_OFFSET                                      0x00000034
#define HSI_APB_RX0_FIFO_DATA_TYPE                                        UInt32
#define HSI_APB_RX0_FIFO_DATA_RESERVED_MASK                               0x00000000
#define    HSI_APB_RX0_FIFO_DATA_RX0_DATA_SHIFT                           0
#define    HSI_APB_RX0_FIFO_DATA_RX0_DATA_MASK                            0xFFFFFFFF

#define HSI_APB_RX1_FIFO_DATA_OFFSET                                      0x00000038
#define HSI_APB_RX1_FIFO_DATA_TYPE                                        UInt32
#define HSI_APB_RX1_FIFO_DATA_RESERVED_MASK                               0x00000000
#define    HSI_APB_RX1_FIFO_DATA_RX1_DATA_SHIFT                           0
#define    HSI_APB_RX1_FIFO_DATA_RX1_DATA_MASK                            0xFFFFFFFF

#define HSI_APB_RX_CH_ID_OFFSET                                           0x0000003C
#define HSI_APB_RX_CH_ID_TYPE                                             UInt32
#define HSI_APB_RX_CH_ID_RESERVED_MASK                                    0x00000000
#define    HSI_APB_RX_CH_ID_RX_CH_ID_UNUSED_SHIFT                         8
#define    HSI_APB_RX_CH_ID_RX_CH_ID_UNUSED_MASK                          0xFFFFFF00
#define    HSI_APB_RX_CH_ID_RX1_CH_ID_SHIFT                               4
#define    HSI_APB_RX_CH_ID_RX1_CH_ID_MASK                                0x000000F0
#define    HSI_APB_RX_CH_ID_RX0_CH_ID_SHIFT                               0
#define    HSI_APB_RX_CH_ID_RX0_CH_ID_MASK                                0x0000000F

#define HSI_APB_RX_THRSD_OFFSET                                           0x00000040
#define HSI_APB_RX_THRSD_TYPE                                             UInt32
#define HSI_APB_RX_THRSD_RESERVED_MASK                                    0x00000000
#define    HSI_APB_RX_THRSD_RX_THRSD_UNUSED_SHIFT                         10
#define    HSI_APB_RX_THRSD_RX_THRSD_UNUSED_MASK                          0xFFFFFC00
#define    HSI_APB_RX_THRSD_RX1_FIFO_THRSD_SHIFT                          5
#define    HSI_APB_RX_THRSD_RX1_FIFO_THRSD_MASK                           0x000003E0
#define    HSI_APB_RX_THRSD_RX0_FIFO_THRSD_SHIFT                          0
#define    HSI_APB_RX_THRSD_RX0_FIFO_THRSD_MASK                           0x0000001F

#define HSI_APB_RX_FIFO_STS_OFFSET                                        0x00000044
#define HSI_APB_RX_FIFO_STS_TYPE                                          UInt32
#define HSI_APB_RX_FIFO_STS_RESERVED_MASK                                 0x00000000
#define    HSI_APB_RX_FIFO_STS_RX_FIFO_STS_UNUSED_SHIFT                   16
#define    HSI_APB_RX_FIFO_STS_RX_FIFO_STS_UNUSED_MASK                    0xFFFF0000
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_DATA_CNT_SHIFT                    12
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_DATA_CNT_MASK                     0x0000F000
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_DATA_UNUSED_SHIFT                 10
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_DATA_UNUSED_MASK                  0x00000C00
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_DATA_CNT_SHIFT                    6
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_DATA_CNT_MASK                     0x000003C0
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_THRSD_HIT_SHIFT                   5
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_THRSD_HIT_MASK                    0x00000020
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_EMPTY_SHIFT                       4
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_EMPTY_MASK                        0x00000010
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_FULL_SHIFT                        3
#define    HSI_APB_RX_FIFO_STS_RX1_FIFO_FULL_MASK                         0x00000008
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_THRSD_HIT_SHIFT                   2
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_THRSD_HIT_MASK                    0x00000004
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_EMPTY_SHIFT                       1
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_EMPTY_MASK                        0x00000002
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_FULL_SHIFT                        0
#define    HSI_APB_RX_FIFO_STS_RX0_FIFO_FULL_MASK                         0x00000001

#define HSI_APB_RX_DMA_CTRL_OFFSET                                        0x00000048
#define HSI_APB_RX_DMA_CTRL_TYPE                                          UInt32
#define HSI_APB_RX_DMA_CTRL_RESERVED_MASK                                 0x00000000
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH1_UNUSED_SHIFT                    28
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH1_UNUSED_MASK                     0xF0000000
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH1_TRF_CNT_SHIFT                   16
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH1_TRF_CNT_MASK                    0x0FFF0000
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH0_UNUSED_SHIFT                    14
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH0_UNUSED_MASK                     0x0000C000
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH0_TRF_CNT_SHIFT                   2
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH0_TRF_CNT_MASK                    0x00003FFC
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH1_EN_SHIFT                        1
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH1_EN_MASK                         0x00000002
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH0_EN_SHIFT                        0
#define    HSI_APB_RX_DMA_CTRL_RX_DMA_CH0_EN_MASK                         0x00000001

#define HSI_APB_RX_DMA_STS_OFFSET                                         0x0000004C
#define HSI_APB_RX_DMA_STS_TYPE                                           UInt32
#define HSI_APB_RX_DMA_STS_RESERVED_MASK                                  0x00000000
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH1_UNUSED_SHIFT                28
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH1_UNUSED_MASK                 0xF0000000
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH1_TRF_CNT_SHIFT               16
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH1_TRF_CNT_MASK                0x0FFF0000
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH0_UNUSED_SHIFT                12
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH0_UNUSED_MASK                 0x0000F000
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH0_TRF_CNT_SHIFT               0
#define    HSI_APB_RX_DMA_STS_CURR_RX_DMA_CH0_TRF_CNT_MASK                0x00000FFF

#define HSI_APB_RX_DMA_BURST_LENGTH_OFFSET                                0x00000050
#define HSI_APB_RX_DMA_BURST_LENGTH_TYPE                                  UInt32
#define HSI_APB_RX_DMA_BURST_LENGTH_RESERVED_MASK                         0x00000000
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_BURST_CH1_UNUSED_SHIFT      21
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_BURST_CH1_UNUSED_MASK       0xFFE00000
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_CH1_BURST_LEN_SHIFT         16
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_CH1_BURST_LEN_MASK          0x001F0000
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_BURST_CH0_UNUSED_SHIFT      5
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_BURST_CH0_UNUSED_MASK       0x0000FFE0
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_CH0_BURST_LEN_SHIFT         0
#define    HSI_APB_RX_DMA_BURST_LENGTH_RX_DMA_CH0_BURST_LEN_MASK          0x0000001F

#define HSI_APB_TX_SW_FIFO_DATA_OFFSET                                    0x00000060
#define HSI_APB_TX_SW_FIFO_DATA_TYPE                                      UInt32
#define HSI_APB_TX_SW_FIFO_DATA_RESERVED_MASK                             0x00000000
#define    HSI_APB_TX_SW_FIFO_DATA_TX_SW_DATA_SHIFT                       0
#define    HSI_APB_TX_SW_FIFO_DATA_TX_SW_DATA_MASK                        0xFFFFFFFF

#define HSI_APB_TX_SW_CH_ID_OFFSET                                        0x00000064
#define HSI_APB_TX_SW_CH_ID_TYPE                                          UInt32
#define HSI_APB_TX_SW_CH_ID_RESERVED_MASK                                 0x00000000
#define    HSI_APB_TX_SW_CH_ID_TX_SW_CH_ID_UNUSED_SHIFT                   9
#define    HSI_APB_TX_SW_CH_ID_TX_SW_CH_ID_UNUSED_MASK                    0xFFFFFE00
#define    HSI_APB_TX_SW_CH_ID_TX_SW_FIFO_THRSD_SHIFT                     4
#define    HSI_APB_TX_SW_CH_ID_TX_SW_FIFO_THRSD_MASK                      0x000001F0
#define    HSI_APB_TX_SW_CH_ID_TX_SW_CH_ID_SHIFT                          0
#define    HSI_APB_TX_SW_CH_ID_TX_SW_CH_ID_MASK                           0x0000000F

#define HSI_APB_TX_SW_FIFO_STS_OFFSET                                     0x00000068
#define HSI_APB_TX_SW_FIFO_STS_TYPE                                       UInt32
#define HSI_APB_TX_SW_FIFO_STS_RESERVED_MASK                              0x00000000
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_STS_UNUSED_SHIFT             7
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_STS_UNUSED_MASK              0xFFFFFF80
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_DATA_CNT_SHIFT               3
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_DATA_CNT_MASK                0x00000078
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_THRSD_HIT_SHIFT              2
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_THRSD_HIT_MASK               0x00000004
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_EMPTY_SHIFT                  1
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_EMPTY_MASK                   0x00000002
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_FULL_SHIFT                   0
#define    HSI_APB_TX_SW_FIFO_STS_TX_SW_FIFO_FULL_MASK                    0x00000001

#define HSI_APB_RX_SW_FIFO_DATA_OFFSET                                    0x0000006C
#define HSI_APB_RX_SW_FIFO_DATA_TYPE                                      UInt32
#define HSI_APB_RX_SW_FIFO_DATA_RESERVED_MASK                             0x00000000
#define    HSI_APB_RX_SW_FIFO_DATA_RX_SW_DATA_SHIFT                       0
#define    HSI_APB_RX_SW_FIFO_DATA_RX_SW_DATA_MASK                        0xFFFFFFFF

#define HSI_APB_RX_SW_CH_ID_OFFSET                                        0x00000070
#define HSI_APB_RX_SW_CH_ID_TYPE                                          UInt32
#define HSI_APB_RX_SW_CH_ID_RESERVED_MASK                                 0x00000000
#define    HSI_APB_RX_SW_CH_ID_RX_SW_ID_UNUSED_SHIFT                      4
#define    HSI_APB_RX_SW_CH_ID_RX_SW_ID_UNUSED_MASK                       0xFFFFFFF0
#define    HSI_APB_RX_SW_CH_ID_RX_SW_CH_ID_SHIFT                          0
#define    HSI_APB_RX_SW_CH_ID_RX_SW_CH_ID_MASK                           0x0000000F

#define HSI_APB_RX_SW_FIFO_THRSD_OFFSET                                   0x00000074
#define HSI_APB_RX_SW_FIFO_THRSD_TYPE                                     UInt32
#define HSI_APB_RX_SW_FIFO_THRSD_RESERVED_MASK                            0x00000000
#define    HSI_APB_RX_SW_FIFO_THRSD_RX_SW_THRSD_UNUSED_SHIFT              5
#define    HSI_APB_RX_SW_FIFO_THRSD_RX_SW_THRSD_UNUSED_MASK               0xFFFFFFE0
#define    HSI_APB_RX_SW_FIFO_THRSD_RX_SW_THRSD_SHIFT                     0
#define    HSI_APB_RX_SW_FIFO_THRSD_RX_SW_THRSD_MASK                      0x0000001F

#define HSI_APB_RX_SW_FIFO_STS_OFFSET                                     0x00000078
#define HSI_APB_RX_SW_FIFO_STS_TYPE                                       UInt32
#define HSI_APB_RX_SW_FIFO_STS_RESERVED_MASK                              0x00000000
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_STS_UNUSED_SHIFT             7
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_STS_UNUSED_MASK              0xFFFFFF80
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_DATA_CNT_SHIFT               3
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_DATA_CNT_MASK                0x00000078
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_THRSD_HIT_SHIFT              2
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_THRSD_HIT_MASK               0x00000004
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_EMPTY_SHIFT                  1
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_EMPTY_MASK                   0x00000002
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_FULL_SHIFT                   0
#define    HSI_APB_RX_SW_FIFO_STS_RX_SW_FIFO_FULL_MASK                    0x00000001

#define HSI_APB_RX_INT_STS_OFFSET                                         0x0000007C
#define HSI_APB_RX_INT_STS_TYPE                                           UInt32
#define HSI_APB_RX_INT_STS_RESERVED_MASK                                  0x00000000
#define    HSI_APB_RX_INT_STS_RX_INT_STS_UNUSED_SHIFT                     15
#define    HSI_APB_RX_INT_STS_RX_INT_STS_UNUSED_MASK                      0xFFFF8000
#define    HSI_APB_RX_INT_STS_WAKE_INT_STS_SHIFT                          14
#define    HSI_APB_RX_INT_STS_WAKE_INT_STS_MASK                           0x00004000
#define    HSI_APB_RX_INT_STS_FRAME_BURST_INT_STS_SHIFT                   13
#define    HSI_APB_RX_INT_STS_FRAME_BURST_INT_STS_MASK                    0x00002000
#define    HSI_APB_RX_INT_STS_TRAILING_BIT_INT_STS_SHIFT                  12
#define    HSI_APB_RX_INT_STS_TRAILING_BIT_INT_STS_MASK                   0x00001000
#define    HSI_APB_RX_INT_STS_FRAME_TO_INT_STS_SHIFT                      11
#define    HSI_APB_RX_INT_STS_FRAME_TO_INT_STS_MASK                       0x00000800
#define    HSI_APB_RX_INT_STS_RX_DMA_COMPLETE_STS_UNUSED_SHIFT            9
#define    HSI_APB_RX_INT_STS_RX_DMA_COMPLETE_STS_UNUSED_MASK             0x00000600
#define    HSI_APB_RX_INT_STS_RX_SW_FIFO_THRSD_REACHED_INT_STS_SHIFT      8
#define    HSI_APB_RX_INT_STS_RX_SW_FIFO_THRSD_REACHED_INT_STS_MASK       0x00000100
#define    HSI_APB_RX_INT_STS_RX1_FIFO_THRSD_REACHED_INT_STS_SHIFT        7
#define    HSI_APB_RX_INT_STS_RX1_FIFO_THRSD_REACHED_INT_STS_MASK         0x00000080
#define    HSI_APB_RX_INT_STS_RX0_FIFO_THRSD_REACHED_INT_STS_SHIFT        6
#define    HSI_APB_RX_INT_STS_RX0_FIFO_THRSD_REACHED_INT_STS_MASK         0x00000040
#define    HSI_APB_RX_INT_STS_RX_SW_FIFO_EMPTY_INT_STS_SHIFT              5
#define    HSI_APB_RX_INT_STS_RX_SW_FIFO_EMPTY_INT_STS_MASK               0x00000020
#define    HSI_APB_RX_INT_STS_RX_SW_FIFO_FULL_INT_STS_SHIFT               4
#define    HSI_APB_RX_INT_STS_RX_SW_FIFO_FULL_INT_STS_MASK                0x00000010
#define    HSI_APB_RX_INT_STS_RX1_FIFO_EMPTY_INT_STS_SHIFT                3
#define    HSI_APB_RX_INT_STS_RX1_FIFO_EMPTY_INT_STS_MASK                 0x00000008
#define    HSI_APB_RX_INT_STS_RX1_FIFO_FULL_INT_STS_SHIFT                 2
#define    HSI_APB_RX_INT_STS_RX1_FIFO_FULL_INT_STS_MASK                  0x00000004
#define    HSI_APB_RX_INT_STS_RX0_FIFO_EMPTY_INT_STS_SHIFT                1
#define    HSI_APB_RX_INT_STS_RX0_FIFO_EMPTY_INT_STS_MASK                 0x00000002
#define    HSI_APB_RX_INT_STS_RX0_FIFO_FULL_INT_STS_SHIFT                 0
#define    HSI_APB_RX_INT_STS_RX0_FIFO_FULL_INT_STS_MASK                  0x00000001

#define HSI_APB_RX_INT_CLR_OFFSET                                         0x00000080
#define HSI_APB_RX_INT_CLR_TYPE                                           UInt32
#define HSI_APB_RX_INT_CLR_RESERVED_MASK                                  0x00000000
#define    HSI_APB_RX_INT_CLR_RX_INT_CLR_UNUSED_SHIFT                     15
#define    HSI_APB_RX_INT_CLR_RX_INT_CLR_UNUSED_MASK                      0xFFFF8000
#define    HSI_APB_RX_INT_CLR_WAKE_INT_CLR_SHIFT                          14
#define    HSI_APB_RX_INT_CLR_WAKE_INT_CLR_MASK                           0x00004000
#define    HSI_APB_RX_INT_CLR_FRAME_BURST_INT_CLR_SHIFT                   13
#define    HSI_APB_RX_INT_CLR_FRAME_BURST_INT_CLR_MASK                    0x00002000
#define    HSI_APB_RX_INT_CLR_TRAILING_BIT_INT_CLR_SHIFT                  12
#define    HSI_APB_RX_INT_CLR_TRAILING_BIT_INT_CLR_MASK                   0x00001000
#define    HSI_APB_RX_INT_CLR_FRAME_TO_INT_CLR_SHIFT                      11
#define    HSI_APB_RX_INT_CLR_FRAME_TO_INT_CLR_MASK                       0x00000800
#define    HSI_APB_RX_INT_CLR_RX_DMA_COMPLETE_CLR_UNUSED_SHIFT            9
#define    HSI_APB_RX_INT_CLR_RX_DMA_COMPLETE_CLR_UNUSED_MASK             0x00000600
#define    HSI_APB_RX_INT_CLR_RX_SW_FIFO_THRSD_REACHED_CLR_SHIFT          8
#define    HSI_APB_RX_INT_CLR_RX_SW_FIFO_THRSD_REACHED_CLR_MASK           0x00000100
#define    HSI_APB_RX_INT_CLR_RX1_FIFO_THRSD_REACHED_CLR_SHIFT            7
#define    HSI_APB_RX_INT_CLR_RX1_FIFO_THRSD_REACHED_CLR_MASK             0x00000080
#define    HSI_APB_RX_INT_CLR_RX0_FIFO_THRSD_REACHED_CLR_SHIFT            6
#define    HSI_APB_RX_INT_CLR_RX0_FIFO_THRSD_REACHED_CLR_MASK             0x00000040
#define    HSI_APB_RX_INT_CLR_RX_SW_FIFO_EMPTY_CLR_SHIFT                  5
#define    HSI_APB_RX_INT_CLR_RX_SW_FIFO_EMPTY_CLR_MASK                   0x00000020
#define    HSI_APB_RX_INT_CLR_RX_SW_FIFO_FULL_CLR_SHIFT                   4
#define    HSI_APB_RX_INT_CLR_RX_SW_FIFO_FULL_CLR_MASK                    0x00000010
#define    HSI_APB_RX_INT_CLR_RX1_FIFO_EMPTY_CLR_SHIFT                    3
#define    HSI_APB_RX_INT_CLR_RX1_FIFO_EMPTY_CLR_MASK                     0x00000008
#define    HSI_APB_RX_INT_CLR_RX1_FIFO_FULL_CLR_SHIFT                     2
#define    HSI_APB_RX_INT_CLR_RX1_FIFO_FULL_CLR_MASK                      0x00000004
#define    HSI_APB_RX_INT_CLR_RX0_FIFO_EMPTY_CLR_SHIFT                    1
#define    HSI_APB_RX_INT_CLR_RX0_FIFO_EMPTY_CLR_MASK                     0x00000002
#define    HSI_APB_RX_INT_CLR_RX0_FIFO_FULL_CLR_SHIFT                     0
#define    HSI_APB_RX_INT_CLR_RX0_FIFO_FULL_CLR_MASK                      0x00000001

#define HSI_APB_RX_INT_EN_OFFSET                                          0x00000084
#define HSI_APB_RX_INT_EN_TYPE                                            UInt32
#define HSI_APB_RX_INT_EN_RESERVED_MASK                                   0x00000000
#define    HSI_APB_RX_INT_EN_RX_INT_EN_UNUSED_SHIFT                       15
#define    HSI_APB_RX_INT_EN_RX_INT_EN_UNUSED_MASK                        0xFFFF8000
#define    HSI_APB_RX_INT_EN_WAKE_INT_EN_SHIFT                            14
#define    HSI_APB_RX_INT_EN_WAKE_INT_EN_MASK                             0x00004000
#define    HSI_APB_RX_INT_EN_FRAME_BURST_INT_EN_SHIFT                     13
#define    HSI_APB_RX_INT_EN_FRAME_BURST_INT_EN_MASK                      0x00002000
#define    HSI_APB_RX_INT_EN_TRAILING_BIT_INT_EN_SHIFT                    12
#define    HSI_APB_RX_INT_EN_TRAILING_BIT_INT_EN_MASK                     0x00001000
#define    HSI_APB_RX_INT_EN_FRAME_TO_INT_EN_SHIFT                        11
#define    HSI_APB_RX_INT_EN_FRAME_TO_INT_EN_MASK                         0x00000800
#define    HSI_APB_RX_INT_EN_RX0_DMA_COMPLETE_INT_EN_UNUSED_SHIFT         9
#define    HSI_APB_RX_INT_EN_RX0_DMA_COMPLETE_INT_EN_UNUSED_MASK          0x00000600
#define    HSI_APB_RX_INT_EN_RX_SW_FIFO_THRSD_REACHED_INT_EN_SHIFT        8
#define    HSI_APB_RX_INT_EN_RX_SW_FIFO_THRSD_REACHED_INT_EN_MASK         0x00000100
#define    HSI_APB_RX_INT_EN_RX1_FIFO_THRSD_REACHED_INT_EN_SHIFT          7
#define    HSI_APB_RX_INT_EN_RX1_FIFO_THRSD_REACHED_INT_EN_MASK           0x00000080
#define    HSI_APB_RX_INT_EN_RX0_FIFO_THRSD_REACHED_INT_EN_SHIFT          6
#define    HSI_APB_RX_INT_EN_RX0_FIFO_THRSD_REACHED_INT_EN_MASK           0x00000040
#define    HSI_APB_RX_INT_EN_RX_SW_FIFO_EMPTY_INT_EN_SHIFT                5
#define    HSI_APB_RX_INT_EN_RX_SW_FIFO_EMPTY_INT_EN_MASK                 0x00000020
#define    HSI_APB_RX_INT_EN_RX_SW_FIFO_FULL_INT_EN_SHIFT                 4
#define    HSI_APB_RX_INT_EN_RX_SW_FIFO_FULL_INT_EN_MASK                  0x00000010
#define    HSI_APB_RX_INT_EN_RX1_FIFO_EMPTY_INT_EN_SHIFT                  3
#define    HSI_APB_RX_INT_EN_RX1_FIFO_EMPTY_INT_EN_MASK                   0x00000008
#define    HSI_APB_RX_INT_EN_RX1_FIFO_FULL_INT_EN_SHIFT                   2
#define    HSI_APB_RX_INT_EN_RX1_FIFO_FULL_INT_EN_MASK                    0x00000004
#define    HSI_APB_RX_INT_EN_RX0_FIFO_EMPTY_INT_EN_SHIFT                  1
#define    HSI_APB_RX_INT_EN_RX0_FIFO_EMPTY_INT_EN_MASK                   0x00000002
#define    HSI_APB_RX_INT_EN_RX0_FIFO_FULL_INT_EN_SHIFT                   0
#define    HSI_APB_RX_INT_EN_RX0_FIFO_FULL_INT_EN_MASK                    0x00000001

#define HSI_APB_TX_INT_STS_OFFSET                                         0x00000088
#define HSI_APB_TX_INT_STS_TYPE                                           UInt32
#define HSI_APB_TX_INT_STS_RESERVED_MASK                                  0x00000000
#define    HSI_APB_TX_INT_STS_TX_INT_STS_UNUSED_SHIFT                     6
#define    HSI_APB_TX_INT_STS_TX_INT_STS_UNUSED_MASK                      0xFFFFFFC0
#define    HSI_APB_TX_INT_STS_TX_SW_FIFO_EMPTY_INT_STS_SHIFT              5
#define    HSI_APB_TX_INT_STS_TX_SW_FIFO_EMPTY_INT_STS_MASK               0x00000020
#define    HSI_APB_TX_INT_STS_TX_SW_FIFO_FULL_INT_STS_SHIFT               4
#define    HSI_APB_TX_INT_STS_TX_SW_FIFO_FULL_INT_STS_MASK                0x00000010
#define    HSI_APB_TX_INT_STS_TX1_FIFO_EMPTY_INT_STS_SHIFT                3
#define    HSI_APB_TX_INT_STS_TX1_FIFO_EMPTY_INT_STS_MASK                 0x00000008
#define    HSI_APB_TX_INT_STS_TX1_FIFO_FULL_INT_STS_SHIFT                 2
#define    HSI_APB_TX_INT_STS_TX1_FIFO_FULL_INT_STS_MASK                  0x00000004
#define    HSI_APB_TX_INT_STS_TX0_FIFO_EMPTY_INT_STS_SHIFT                1
#define    HSI_APB_TX_INT_STS_TX0_FIFO_EMPTY_INT_STS_MASK                 0x00000002
#define    HSI_APB_TX_INT_STS_TX0_FIFO_FULL_INT_STS_SHIFT                 0
#define    HSI_APB_TX_INT_STS_TX0_FIFO_FULL_INT_STS_MASK                  0x00000001

#define HSI_APB_TX_INT_CLR_OFFSET                                         0x0000008C
#define HSI_APB_TX_INT_CLR_TYPE                                           UInt32
#define HSI_APB_TX_INT_CLR_RESERVED_MASK                                  0x00000000
#define    HSI_APB_TX_INT_CLR_TX_INT_CLR_UNUSED_SHIFT                     6
#define    HSI_APB_TX_INT_CLR_TX_INT_CLR_UNUSED_MASK                      0xFFFFFFC0
#define    HSI_APB_TX_INT_CLR_TX_SW_FIFO_EMPTY_CLR_SHIFT                  5
#define    HSI_APB_TX_INT_CLR_TX_SW_FIFO_EMPTY_CLR_MASK                   0x00000020
#define    HSI_APB_TX_INT_CLR_TX_SW_FIFO_FULL_CLR_SHIFT                   4
#define    HSI_APB_TX_INT_CLR_TX_SW_FIFO_FULL_CLR_MASK                    0x00000010
#define    HSI_APB_TX_INT_CLR_TX1_FIFO_EMPTY_CLR_SHIFT                    3
#define    HSI_APB_TX_INT_CLR_TX1_FIFO_EMPTY_CLR_MASK                     0x00000008
#define    HSI_APB_TX_INT_CLR_TX1_FIFO_FULL_CLR_SHIFT                     2
#define    HSI_APB_TX_INT_CLR_TX1_FIFO_FULL_CLR_MASK                      0x00000004
#define    HSI_APB_TX_INT_CLR_TX0_FIFO_EMPTY_CLR_SHIFT                    1
#define    HSI_APB_TX_INT_CLR_TX0_FIFO_EMPTY_CLR_MASK                     0x00000002
#define    HSI_APB_TX_INT_CLR_TX0_FIFO_FULL_CLR_SHIFT                     0
#define    HSI_APB_TX_INT_CLR_TX0_FIFO_FULL_CLR_MASK                      0x00000001

#define HSI_APB_TX_INT_EN_OFFSET                                          0x00000090
#define HSI_APB_TX_INT_EN_TYPE                                            UInt32
#define HSI_APB_TX_INT_EN_RESERVED_MASK                                   0x00000000
#define    HSI_APB_TX_INT_EN_TX_INT_EN_UNUSED_SHIFT                       6
#define    HSI_APB_TX_INT_EN_TX_INT_EN_UNUSED_MASK                        0xFFFFFFC0
#define    HSI_APB_TX_INT_EN_TX_SW_FIFO_EMPTY_INT_EN_SHIFT                5
#define    HSI_APB_TX_INT_EN_TX_SW_FIFO_EMPTY_INT_EN_MASK                 0x00000020
#define    HSI_APB_TX_INT_EN_TX_SW_FIFO_FULL_INT_EN_SHIFT                 4
#define    HSI_APB_TX_INT_EN_TX_SW_FIFO_FULL_INT_EN_MASK                  0x00000010
#define    HSI_APB_TX_INT_EN_TX1_FIFO_EMPTY_INT_EN_SHIFT                  3
#define    HSI_APB_TX_INT_EN_TX1_FIFO_EMPTY_INT_EN_MASK                   0x00000008
#define    HSI_APB_TX_INT_EN_TX1_FIFO_FULL_INT_EN_SHIFT                   2
#define    HSI_APB_TX_INT_EN_TX1_FIFO_FULL_INT_EN_MASK                    0x00000004
#define    HSI_APB_TX_INT_EN_TX0_FIFO_EMPTY_INT_EN_SHIFT                  1
#define    HSI_APB_TX_INT_EN_TX0_FIFO_EMPTY_INT_EN_MASK                   0x00000002
#define    HSI_APB_TX_INT_EN_TX0_FIFO_FULL_INT_EN_SHIFT                   0
#define    HSI_APB_TX_INT_EN_TX0_FIFO_FULL_INT_EN_MASK                    0x00000001

#define HSI_APB_FRAME_TO_CNT_OFFSET                                       0x000000A0
#define HSI_APB_FRAME_TO_CNT_TYPE                                         UInt32
#define HSI_APB_FRAME_TO_CNT_RESERVED_MASK                                0x00000000
#define    HSI_APB_FRAME_TO_CNT_FRAME_TO_CNT_UNUSED_SHIFT                 10
#define    HSI_APB_FRAME_TO_CNT_FRAME_TO_CNT_UNUSED_MASK                  0xFFFFFC00
#define    HSI_APB_FRAME_TO_CNT_FRAME_TO_CNT_SHIFT                        0
#define    HSI_APB_FRAME_TO_CNT_FRAME_TO_CNT_MASK                         0x000003FF

#define HSI_APB_TRAILING_BIT_CNT_OFFSET                                   0x000000A4
#define HSI_APB_TRAILING_BIT_CNT_TYPE                                     UInt32
#define HSI_APB_TRAILING_BIT_CNT_RESERVED_MASK                            0x00000000
#define    HSI_APB_TRAILING_BIT_CNT_TRAILING_BIT_CNT_UNUSED_SHIFT         8
#define    HSI_APB_TRAILING_BIT_CNT_TRAILING_BIT_CNT_UNUSED_MASK          0xFFFFFF00
#define    HSI_APB_TRAILING_BIT_CNT_TRAILING_BIT_CNT_SHIFT                0
#define    HSI_APB_TRAILING_BIT_CNT_TRAILING_BIT_CNT_MASK                 0x000000FF

#define HSI_APB_FRAME_BURST_CNT_OFFSET                                    0x000000A8
#define HSI_APB_FRAME_BURST_CNT_TYPE                                      UInt32
#define HSI_APB_FRAME_BURST_CNT_RESERVED_MASK                             0x00000000
#define    HSI_APB_FRAME_BURST_CNT_FRAME_BURST_CNT_UNUSED_SHIFT           8
#define    HSI_APB_FRAME_BURST_CNT_FRAME_BURST_CNT_UNUSED_MASK            0xFFFFFF00
#define    HSI_APB_FRAME_BURST_CNT_FRAME_BURST_CNT_SHIFT                  0
#define    HSI_APB_FRAME_BURST_CNT_FRAME_BURST_CNT_MASK                   0x000000FF

#define HSI_APB_DMA_CTRL_OFFSET                                           0x000000AC
#define HSI_APB_DMA_CTRL_TYPE                                             UInt32
#define HSI_APB_DMA_CTRL_RESERVED_MASK                                    0x00000000
#define    HSI_APB_DMA_CTRL_DMA_CTRL_UNUSED_SHIFT                         1
#define    HSI_APB_DMA_CTRL_DMA_CTRL_UNUSED_MASK                          0xFFFFFFFE
#define    HSI_APB_DMA_CTRL_DMA_MODE_SHIFT                                0
#define    HSI_APB_DMA_CTRL_DMA_MODE_MASK                                 0x00000001

#endif /* __BRCM_RDB_HSI_APB_H__ */



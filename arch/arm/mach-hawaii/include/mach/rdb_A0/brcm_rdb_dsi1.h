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

#ifndef __BRCM_RDB_DSI1_H__
#define __BRCM_RDB_DSI1_H__

#define DSI1_CTRL_OFFSET                                                  0x00000000
#define DSI1_CTRL_TYPE                                                    UInt32
#define DSI1_CTRL_RESERVED_MASK                                           0xFFFC0008
#define    DSI1_CTRL_TE_TRIGC_SHIFT                                       16
#define    DSI1_CTRL_TE_TRIGC_MASK                                        0x00030000
#define    DSI1_CTRL_HS_CLKC_SHIFT                                        14
#define    DSI1_CTRL_HS_CLKC_MASK                                         0x0000C000
#define    DSI1_CTRL_RX_LPDT_EOT_EN_SHIFT                                 13
#define    DSI1_CTRL_RX_LPDT_EOT_EN_MASK                                  0x00002000
#define    DSI1_CTRL_LPDT_EOT_EN_SHIFT                                    12
#define    DSI1_CTRL_LPDT_EOT_EN_MASK                                     0x00001000
#define    DSI1_CTRL_HSDT_EOT_EN_SHIFT                                    11
#define    DSI1_CTRL_HSDT_EOT_EN_MASK                                     0x00000800
#define    DSI1_CTRL_SOFT_RESET_CFG_SHIFT                                 10
#define    DSI1_CTRL_SOFT_RESET_CFG_MASK                                  0x00000400
#define    DSI1_CTRL_CAL_BYTE_EN_SHIFT                                    9
#define    DSI1_CTRL_CAL_BYTE_EN_MASK                                     0x00000200
#define    DSI1_CTRL_INV_BYTE_EN_SHIFT                                    8
#define    DSI1_CTRL_INV_BYTE_EN_MASK                                     0x00000100
#define    DSI1_CTRL_CLR_LDF_SHIFT                                        7
#define    DSI1_CTRL_CLR_LDF_MASK                                         0x00000080
#define    DSI1_CTRL_CLR_RXF_SHIFT                                        6
#define    DSI1_CTRL_CLR_RXF_MASK                                         0x00000040
#define    DSI1_CTRL_CLR_PDF_SHIFT                                        5
#define    DSI1_CTRL_CLR_PDF_MASK                                         0x00000020
#define    DSI1_CTRL_CLR_CDF_SHIFT                                        4
#define    DSI1_CTRL_CLR_CDF_MASK                                         0x00000010
#define    DSI1_CTRL_DIS_DISP_CRCC_SHIFT                                  2
#define    DSI1_CTRL_DIS_DISP_CRCC_MASK                                   0x00000004
#define    DSI1_CTRL_DIS_DISP_ECCC_SHIFT                                  1
#define    DSI1_CTRL_DIS_DISP_ECCC_MASK                                   0x00000002
#define    DSI1_CTRL_DSI1_EN_SHIFT                                        0
#define    DSI1_CTRL_DSI1_EN_MASK                                         0x00000001

#define DSI1_TXPKT1_C_OFFSET                                              0x00000004
#define DSI1_TXPKT1_C_TYPE                                                UInt32
#define DSI1_TXPKT1_C_RESERVED_MASK                                       0x00000000
#define    DSI1_TXPKT1_C_TRIG_CMD_SHIFT                                   24
#define    DSI1_TXPKT1_C_TRIG_CMD_MASK                                    0xFF000000
#define    DSI1_TXPKT1_C_CMD_REPEAT_SHIFT                                 10
#define    DSI1_TXPKT1_C_CMD_REPEAT_MASK                                  0x00FFFC00
#define    DSI1_TXPKT1_C_DISPLAY_NO_SHIFT                                 8
#define    DSI1_TXPKT1_C_DISPLAY_NO_MASK                                  0x00000300
#define    DSI1_TXPKT1_C_CMD_TX_TIME_SHIFT                                6
#define    DSI1_TXPKT1_C_CMD_TX_TIME_MASK                                 0x000000C0
#define    DSI1_TXPKT1_C_CMD_CTRL_SHIFT                                   4
#define    DSI1_TXPKT1_C_CMD_CTRL_MASK                                    0x00000030
#define    DSI1_TXPKT1_C_CMD_MODE_SHIFT                                   3
#define    DSI1_TXPKT1_C_CMD_MODE_MASK                                    0x00000008
#define    DSI1_TXPKT1_C_CMD_TYPE_SHIFT                                   2
#define    DSI1_TXPKT1_C_CMD_TYPE_MASK                                    0x00000004
#define    DSI1_TXPKT1_C_CMD_TE_EN_SHIFT                                  1
#define    DSI1_TXPKT1_C_CMD_TE_EN_MASK                                   0x00000002
#define    DSI1_TXPKT1_C_CMD_EN_SHIFT                                     0
#define    DSI1_TXPKT1_C_CMD_EN_MASK                                      0x00000001

#define DSI1_TXPKT1_H_OFFSET                                              0x00000008
#define DSI1_TXPKT1_H_TYPE                                                UInt32
#define DSI1_TXPKT1_H_RESERVED_MASK                                       0x00000000
#define    DSI1_TXPKT1_H_BC_CMDFIFO_SHIFT                                 24
#define    DSI1_TXPKT1_H_BC_CMDFIFO_MASK                                  0xFF000000
#define    DSI1_TXPKT1_H_BC_PARAM_SHIFT                                   8
#define    DSI1_TXPKT1_H_BC_PARAM_MASK                                    0x00FFFF00
#define    DSI1_TXPKT1_H_DT_SHIFT                                         0
#define    DSI1_TXPKT1_H_DT_MASK                                          0x000000FF

#define DSI1_TXPKT2_C_OFFSET                                              0x0000000C
#define DSI1_TXPKT2_C_TYPE                                                UInt32
#define DSI1_TXPKT2_C_RESERVED_MASK                                       0x00000000
#define    DSI1_TXPKT2_C_TRIG_CMD_SHIFT                                   24
#define    DSI1_TXPKT2_C_TRIG_CMD_MASK                                    0xFF000000
#define    DSI1_TXPKT2_C_CMD_REPEAT_SHIFT                                 10
#define    DSI1_TXPKT2_C_CMD_REPEAT_MASK                                  0x00FFFC00
#define    DSI1_TXPKT2_C_DISPLAY_NO_SHIFT                                 8
#define    DSI1_TXPKT2_C_DISPLAY_NO_MASK                                  0x00000300
#define    DSI1_TXPKT2_C_CMD_TX_TIME_SHIFT                                6
#define    DSI1_TXPKT2_C_CMD_TX_TIME_MASK                                 0x000000C0
#define    DSI1_TXPKT2_C_CMD_CTRL_SHIFT                                   4
#define    DSI1_TXPKT2_C_CMD_CTRL_MASK                                    0x00000030
#define    DSI1_TXPKT2_C_CMD_MODE_SHIFT                                   3
#define    DSI1_TXPKT2_C_CMD_MODE_MASK                                    0x00000008
#define    DSI1_TXPKT2_C_CMD_TYPE_SHIFT                                   2
#define    DSI1_TXPKT2_C_CMD_TYPE_MASK                                    0x00000004
#define    DSI1_TXPKT2_C_CMD_TE_EN_SHIFT                                  1
#define    DSI1_TXPKT2_C_CMD_TE_EN_MASK                                   0x00000002
#define    DSI1_TXPKT2_C_CMD_EN_SHIFT                                     0
#define    DSI1_TXPKT2_C_CMD_EN_MASK                                      0x00000001

#define DSI1_TXPKT2_H_OFFSET                                              0x00000010
#define DSI1_TXPKT2_H_TYPE                                                UInt32
#define DSI1_TXPKT2_H_RESERVED_MASK                                       0x00000000
#define    DSI1_TXPKT2_H_BC_CMDFIFO_SHIFT                                 24
#define    DSI1_TXPKT2_H_BC_CMDFIFO_MASK                                  0xFF000000
#define    DSI1_TXPKT2_H_BC_PARAM_SHIFT                                   8
#define    DSI1_TXPKT2_H_BC_PARAM_MASK                                    0x00FFFF00
#define    DSI1_TXPKT2_H_DT_SHIFT                                         0
#define    DSI1_TXPKT2_H_DT_MASK                                          0x000000FF

#define DSI1_RXPKT1_H_OFFSET                                              0x00000014
#define DSI1_RXPKT1_H_TYPE                                                UInt32
#define DSI1_RXPKT1_H_RESERVED_MASK                                       0x0C000000
#define    DSI1_RXPKT1_H_CRC_ERR_SHIFT                                    31
#define    DSI1_RXPKT1_H_CRC_ERR_MASK                                     0x80000000
#define    DSI1_RXPKT1_H_DET_ERR_SHIFT                                    30
#define    DSI1_RXPKT1_H_DET_ERR_MASK                                     0x40000000
#define    DSI1_RXPKT1_H_ECC_ERR_SHIFT                                    29
#define    DSI1_RXPKT1_H_ECC_ERR_MASK                                     0x20000000
#define    DSI1_RXPKT1_H_COR_ERR_SHIFT                                    28
#define    DSI1_RXPKT1_H_COR_ERR_MASK                                     0x10000000
#define    DSI1_RXPKT1_H_INCOMP_PKT_SHIFT                                 25
#define    DSI1_RXPKT1_H_INCOMP_PKT_MASK                                  0x02000000
#define    DSI1_RXPKT1_H_PKT_TYPE_SHIFT                                   24
#define    DSI1_RXPKT1_H_PKT_TYPE_MASK                                    0x01000000
#define    DSI1_RXPKT1_H_BC_PARAM_SHIFT                                   8
#define    DSI1_RXPKT1_H_BC_PARAM_MASK                                    0x00FFFF00
#define    DSI1_RXPKT1_H_DT_LP_CMD_SHIFT                                  0
#define    DSI1_RXPKT1_H_DT_LP_CMD_MASK                                   0x000000FF

#define DSI1_RXPKT2_H_OFFSET                                              0x00000018
#define DSI1_RXPKT2_H_TYPE                                                UInt32
#define DSI1_RXPKT2_H_RESERVED_MASK                                       0x8D000000
#define    DSI1_RXPKT2_H_DET_ERR_SHIFT                                    30
#define    DSI1_RXPKT2_H_DET_ERR_MASK                                     0x40000000
#define    DSI1_RXPKT2_H_ECC_ERR_SHIFT                                    29
#define    DSI1_RXPKT2_H_ECC_ERR_MASK                                     0x20000000
#define    DSI1_RXPKT2_H_COR_ERR_SHIFT                                    28
#define    DSI1_RXPKT2_H_COR_ERR_MASK                                     0x10000000
#define    DSI1_RXPKT2_H_INCOMP_PKT_SHIFT                                 25
#define    DSI1_RXPKT2_H_INCOMP_PKT_MASK                                  0x02000000
#define    DSI1_RXPKT2_H_PARAM_SHIFT                                      8
#define    DSI1_RXPKT2_H_PARAM_MASK                                       0x00FFFF00
#define    DSI1_RXPKT2_H_DT_SHIFT                                         0
#define    DSI1_RXPKT2_H_DT_MASK                                          0x000000FF

#define DSI1_TXPKT_CMD_FIFO_OFFSET                                        0x0000001C
#define DSI1_TXPKT_CMD_FIFO_TYPE                                          UInt32
#define DSI1_TXPKT_CMD_FIFO_RESERVED_MASK                                 0xFFFFFF00
#define    DSI1_TXPKT_CMD_FIFO_CMD_DATA_SHIFT                             0
#define    DSI1_TXPKT_CMD_FIFO_CMD_DATA_MASK                              0x000000FF

#define DSI1_TXPKT_PIXD_FIFO_OFFSET                                       0x00000020
#define DSI1_TXPKT_PIXD_FIFO_TYPE                                         UInt32
#define DSI1_TXPKT_PIXD_FIFO_RESERVED_MASK                                0x00000000
#define    DSI1_TXPKT_PIXD_FIFO_WORD_SHIFT                                0
#define    DSI1_TXPKT_PIXD_FIFO_WORD_MASK                                 0xFFFFFFFF

#define DSI1_RXPKT_FIFO_OFFSET                                            0x00000024
#define DSI1_RXPKT_FIFO_TYPE                                              UInt32
#define DSI1_RXPKT_FIFO_RESERVED_MASK                                     0xFFFFFF00
#define    DSI1_RXPKT_FIFO_RX_DATA_SHIFT                                  0
#define    DSI1_RXPKT_FIFO_RX_DATA_MASK                                   0x000000FF

#define DSI1_DISP0_CTRL_OFFSET                                            0x00000028
#define DSI1_DISP0_CTRL_TYPE                                              UInt32
#define DSI1_DISP0_CTRL_RESERVED_MASK                                     0xFFC00000
#define    DSI1_DISP0_CTRL_PIX_CLK_DIV_SHIFT                              13
#define    DSI1_DISP0_CTRL_PIX_CLK_DIV_MASK                               0x003FE000
#define    DSI1_DISP0_CTRL_LP_STOP_CTRL_SHIFT                             11
#define    DSI1_DISP0_CTRL_LP_STOP_CTRL_MASK                              0x00001800
#define    DSI1_DISP0_CTRL_HACTIVE_NULL_SHIFT                             10
#define    DSI1_DISP0_CTRL_HACTIVE_NULL_MASK                              0x00000400
#define    DSI1_DISP0_CTRL_VBLP_CTRL_SHIFT                                9
#define    DSI1_DISP0_CTRL_VBLP_CTRL_MASK                                 0x00000200
#define    DSI1_DISP0_CTRL_HFP_CTRL_SHIFT                                 8
#define    DSI1_DISP0_CTRL_HFP_CTRL_MASK                                  0x00000100
#define    DSI1_DISP0_CTRL_HBP_CTRL_SHIFT                                 7
#define    DSI1_DISP0_CTRL_HBP_CTRL_MASK                                  0x00000080
#define    DSI1_DISP0_CTRL_VC_SHIFT                                       5
#define    DSI1_DISP0_CTRL_VC_MASK                                        0x00000060
#define    DSI1_DISP0_CTRL_ST_END_SHIFT                                   4
#define    DSI1_DISP0_CTRL_ST_END_MASK                                    0x00000010
#define    DSI1_DISP0_CTRL_PFORMAT_SHIFT                                  2
#define    DSI1_DISP0_CTRL_PFORMAT_MASK                                   0x0000000C
#define    DSI1_DISP0_CTRL_MODE_SHIFT                                     1
#define    DSI1_DISP0_CTRL_MODE_MASK                                      0x00000002
#define    DSI1_DISP0_CTRL_EN_SHIFT                                       0
#define    DSI1_DISP0_CTRL_EN_MASK                                        0x00000001

#define DSI1_DISP1_CTRL_OFFSET                                            0x0000002C
#define DSI1_DISP1_CTRL_TYPE                                              UInt32
#define DSI1_DISP1_CTRL_RESERVED_MASK                                     0xFFFFE008
#define    DSI1_DISP1_CTRL_DMA_THRESH_SHIFT                               4
#define    DSI1_DISP1_CTRL_DMA_THRESH_MASK                                0x00001FF0
#define    DSI1_DISP1_CTRL_PFORMAT_SHIFT                                  1
#define    DSI1_DISP1_CTRL_PFORMAT_MASK                                   0x00000006
#define    DSI1_DISP1_CTRL_EN_SHIFT                                       0
#define    DSI1_DISP1_CTRL_EN_MASK                                        0x00000001

#define DSI1_INT_STAT_OFFSET                                              0x00000030
#define DSI1_INT_STAT_TYPE                                                UInt32
#define DSI1_INT_STAT_RESERVED_MASK                                       0x80000000
#define    DSI1_INT_STAT_PHY_D3_ULPS_SHIFT                                30
#define    DSI1_INT_STAT_PHY_D3_ULPS_MASK                                 0x40000000
#define    DSI1_INT_STAT_PHY_D3_STOP_SHIFT                                29
#define    DSI1_INT_STAT_PHY_D3_STOP_MASK                                 0x20000000
#define    DSI1_INT_STAT_PHY_D2_ULPS_SHIFT                                28
#define    DSI1_INT_STAT_PHY_D2_ULPS_MASK                                 0x10000000
#define    DSI1_INT_STAT_PHY_D2_STOP_SHIFT                                27
#define    DSI1_INT_STAT_PHY_D2_STOP_MASK                                 0x08000000
#define    DSI1_INT_STAT_PHY_D1_ULPS_SHIFT                                26
#define    DSI1_INT_STAT_PHY_D1_ULPS_MASK                                 0x04000000
#define    DSI1_INT_STAT_PHY_D1_STOP_SHIFT                                25
#define    DSI1_INT_STAT_PHY_D1_STOP_MASK                                 0x02000000
#define    DSI1_INT_STAT_PHY_D0_ULPS_SHIFT                                24
#define    DSI1_INT_STAT_PHY_D0_ULPS_MASK                                 0x01000000
#define    DSI1_INT_STAT_PHY_D0_STOP_SHIFT                                23
#define    DSI1_INT_STAT_PHY_D0_STOP_MASK                                 0x00800000
#define    DSI1_INT_STAT_FIFO_ERR_SHIFT                                   22
#define    DSI1_INT_STAT_FIFO_ERR_MASK                                    0x00400000
#define    DSI1_INT_STAT_PHY_DIR_RTF_SHIFT                                21
#define    DSI1_INT_STAT_PHY_DIR_RTF_MASK                                 0x00200000
#define    DSI1_INT_STAT_PHY_RXLPDT_SHIFT                                 20
#define    DSI1_INT_STAT_PHY_RXLPDT_MASK                                  0x00100000
#define    DSI1_INT_STAT_PHY_RXTRIG_SHIFT                                 19
#define    DSI1_INT_STAT_PHY_RXTRIG_MASK                                  0x00080000
#define    DSI1_INT_STAT_PHY_D0_LPDT_SHIFT                                18
#define    DSI1_INT_STAT_PHY_D0_LPDT_MASK                                 0x00040000
#define    DSI1_INT_STAT_PHY_DIR_FTR_SHIFT                                17
#define    DSI1_INT_STAT_PHY_DIR_FTR_MASK                                 0x00020000
#define    DSI1_INT_STAT_PHY_CLK_ULPS_SHIFT                               16
#define    DSI1_INT_STAT_PHY_CLK_ULPS_MASK                                0x00010000
#define    DSI1_INT_STAT_PHY_CLK_HS_SHIFT                                 15
#define    DSI1_INT_STAT_PHY_CLK_HS_MASK                                  0x00008000
#define    DSI1_INT_STAT_PHY_CLK_STOP_SHIFT                               14
#define    DSI1_INT_STAT_PHY_CLK_STOP_MASK                                0x00004000
#define    DSI1_INT_STAT_PR_TO_SHIFT                                      13
#define    DSI1_INT_STAT_PR_TO_MASK                                       0x00002000
#define    DSI1_INT_STAT_TA_TO_SHIFT                                      12
#define    DSI1_INT_STAT_TA_TO_MASK                                       0x00001000
#define    DSI1_INT_STAT_LPRX_TO_SHIFT                                    11
#define    DSI1_INT_STAT_LPRX_TO_MASK                                     0x00000800
#define    DSI1_INT_STAT_HSTX_TO_SHIFT                                    10
#define    DSI1_INT_STAT_HSTX_TO_MASK                                     0x00000400
#define    DSI1_INT_STAT_ERRCONTLP1_SHIFT                                 9
#define    DSI1_INT_STAT_ERRCONTLP1_MASK                                  0x00000200
#define    DSI1_INT_STAT_ERRCONTLP0_SHIFT                                 8
#define    DSI1_INT_STAT_ERRCONTLP0_MASK                                  0x00000100
#define    DSI1_INT_STAT_ERRCONTROL_SHIFT                                 7
#define    DSI1_INT_STAT_ERRCONTROL_MASK                                  0x00000080
#define    DSI1_INT_STAT_ERRSYNCESC_SHIFT                                 6
#define    DSI1_INT_STAT_ERRSYNCESC_MASK                                  0x00000040
#define    DSI1_INT_STAT_RX2_PKT_SHIFT                                    5
#define    DSI1_INT_STAT_RX2_PKT_MASK                                     0x00000020
#define    DSI1_INT_STAT_RX1_PKT_SHIFT                                    4
#define    DSI1_INT_STAT_RX1_PKT_MASK                                     0x00000010
#define    DSI1_INT_STAT_TXPKT2_DONE_SHIFT                                3
#define    DSI1_INT_STAT_TXPKT2_DONE_MASK                                 0x00000008
#define    DSI1_INT_STAT_TXPKT2_END_SHIFT                                 2
#define    DSI1_INT_STAT_TXPKT2_END_MASK                                  0x00000004
#define    DSI1_INT_STAT_TXPKT1_DONE_SHIFT                                1
#define    DSI1_INT_STAT_TXPKT1_DONE_MASK                                 0x00000002
#define    DSI1_INT_STAT_TXPKT1_END_SHIFT                                 0
#define    DSI1_INT_STAT_TXPKT1_END_MASK                                  0x00000001

#define DSI1_INT_EN_OFFSET                                                0x00000034
#define DSI1_INT_EN_TYPE                                                  UInt32
#define DSI1_INT_EN_RESERVED_MASK                                         0x80000000
#define    DSI1_INT_EN_PHY_D3_ULPS_SHIFT                                  30
#define    DSI1_INT_EN_PHY_D3_ULPS_MASK                                   0x40000000
#define    DSI1_INT_EN_PHY_D3_STOP_SHIFT                                  29
#define    DSI1_INT_EN_PHY_D3_STOP_MASK                                   0x20000000
#define    DSI1_INT_EN_PHY_D2_ULPS_SHIFT                                  28
#define    DSI1_INT_EN_PHY_D2_ULPS_MASK                                   0x10000000
#define    DSI1_INT_EN_PHY_D2_STOP_SHIFT                                  27
#define    DSI1_INT_EN_PHY_D2_STOP_MASK                                   0x08000000
#define    DSI1_INT_EN_PHY_D1_ULPS_SHIFT                                  26
#define    DSI1_INT_EN_PHY_D1_ULPS_MASK                                   0x04000000
#define    DSI1_INT_EN_PHY_D1_STOP_SHIFT                                  25
#define    DSI1_INT_EN_PHY_D1_STOP_MASK                                   0x02000000
#define    DSI1_INT_EN_PHY_D0_ULPS_SHIFT                                  24
#define    DSI1_INT_EN_PHY_D0_ULPS_MASK                                   0x01000000
#define    DSI1_INT_EN_PHY_D0_STOP_SHIFT                                  23
#define    DSI1_INT_EN_PHY_D0_STOP_MASK                                   0x00800000
#define    DSI1_INT_EN_FIFO_ERR_SHIFT                                     22
#define    DSI1_INT_EN_FIFO_ERR_MASK                                      0x00400000
#define    DSI1_INT_EN_PHY_DIR_RTF_SHIFT                                  21
#define    DSI1_INT_EN_PHY_DIR_RTF_MASK                                   0x00200000
#define    DSI1_INT_EN_PHY_RXLPDT_SHIFT                                   20
#define    DSI1_INT_EN_PHY_RXLPDT_MASK                                    0x00100000
#define    DSI1_INT_EN_PHY_RXTRIG_SHIFT                                   19
#define    DSI1_INT_EN_PHY_RXTRIG_MASK                                    0x00080000
#define    DSI1_INT_EN_PHY_D0_LPDT_SHIFT                                  18
#define    DSI1_INT_EN_PHY_D0_LPDT_MASK                                   0x00040000
#define    DSI1_INT_EN_PHY_DIR_FTR_SHIFT                                  17
#define    DSI1_INT_EN_PHY_DIR_FTR_MASK                                   0x00020000
#define    DSI1_INT_EN_PHY_CLK_ULPS_SHIFT                                 16
#define    DSI1_INT_EN_PHY_CLK_ULPS_MASK                                  0x00010000
#define    DSI1_INT_EN_PHY_CLK_HS_SHIFT                                   15
#define    DSI1_INT_EN_PHY_CLK_HS_MASK                                    0x00008000
#define    DSI1_INT_EN_PHY_CLK_STOP_SHIFT                                 14
#define    DSI1_INT_EN_PHY_CLK_STOP_MASK                                  0x00004000
#define    DSI1_INT_EN_PR_TO_SHIFT                                        13
#define    DSI1_INT_EN_PR_TO_MASK                                         0x00002000
#define    DSI1_INT_EN_TA_TO_SHIFT                                        12
#define    DSI1_INT_EN_TA_TO_MASK                                         0x00001000
#define    DSI1_INT_EN_LRX_H_TO_SHIFT                                     11
#define    DSI1_INT_EN_LRX_H_TO_MASK                                      0x00000800
#define    DSI1_INT_EN_HTX_TO_SHIFT                                       10
#define    DSI1_INT_EN_HTX_TO_MASK                                        0x00000400
#define    DSI1_INT_EN_ERRCONTLP1_SHIFT                                   9
#define    DSI1_INT_EN_ERRCONTLP1_MASK                                    0x00000200
#define    DSI1_INT_EN_ERRCONTLP0_SHIFT                                   8
#define    DSI1_INT_EN_ERRCONTLP0_MASK                                    0x00000100
#define    DSI1_INT_EN_ERRCONTROL_SHIFT                                   7
#define    DSI1_INT_EN_ERRCONTROL_MASK                                    0x00000080
#define    DSI1_INT_EN_ERRSYNCESC_SHIFT                                   6
#define    DSI1_INT_EN_ERRSYNCESC_MASK                                    0x00000040
#define    DSI1_INT_EN_RX2_PKT_SHIFT                                      5
#define    DSI1_INT_EN_RX2_PKT_MASK                                       0x00000020
#define    DSI1_INT_EN_RX1_PKT_SHIFT                                      4
#define    DSI1_INT_EN_RX1_PKT_MASK                                       0x00000010
#define    DSI1_INT_EN_TXPKT3_DONE_SHIFT                                  3
#define    DSI1_INT_EN_TXPKT3_DONE_MASK                                   0x00000008
#define    DSI1_INT_EN_TXPKT2_END_SHIFT                                   2
#define    DSI1_INT_EN_TXPKT2_END_MASK                                    0x00000004
#define    DSI1_INT_EN_TXPKT1_DONE_SHIFT                                  1
#define    DSI1_INT_EN_TXPKT1_DONE_MASK                                   0x00000002
#define    DSI1_INT_EN_TXPKT1_END_SHIFT                                   0
#define    DSI1_INT_EN_TXPKT1_END_MASK                                    0x00000001

#define DSI1_STAT_OFFSET                                                  0x00000038
#define DSI1_STAT_TYPE                                                    UInt32
#define DSI1_STAT_RESERVED_MASK                                           0x00000000
#define    DSI1_STAT_PHY_D3_ULPS_SHIFT                                    31
#define    DSI1_STAT_PHY_D3_ULPS_MASK                                     0x80000000
#define    DSI1_STAT_PHY_D3_STOP_SHIFT                                    30
#define    DSI1_STAT_PHY_D3_STOP_MASK                                     0x40000000
#define    DSI1_STAT_PHY_D2_ULPS_SHIFT                                    29
#define    DSI1_STAT_PHY_D2_ULPS_MASK                                     0x20000000
#define    DSI1_STAT_PHY_D2_STOP_SHIFT                                    28
#define    DSI1_STAT_PHY_D2_STOP_MASK                                     0x10000000
#define    DSI1_STAT_PHY_D1_ULPS_SHIFT                                    27
#define    DSI1_STAT_PHY_D1_ULPS_MASK                                     0x08000000
#define    DSI1_STAT_PHY_D1_STOP_SHIFT                                    26
#define    DSI1_STAT_PHY_D1_STOP_MASK                                     0x04000000
#define    DSI1_STAT_PHY_D0_ULPS_SHIFT                                    25
#define    DSI1_STAT_PHY_D0_ULPS_MASK                                     0x02000000
#define    DSI1_STAT_PHY_D0_STOP_SHIFT                                    24
#define    DSI1_STAT_PHY_D0_STOP_MASK                                     0x01000000
#define    DSI1_STAT_FIFO_ERR_SHIFT                                       23
#define    DSI1_STAT_FIFO_ERR_MASK                                        0x00800000
#define    DSI1_STAT_PHY_RXLPDT_SHIFT                                     22
#define    DSI1_STAT_PHY_RXLPDT_MASK                                      0x00400000
#define    DSI1_STAT_PHY_RXTRIG_SHIFT                                     21
#define    DSI1_STAT_PHY_RXTRIG_MASK                                      0x00200000
#define    DSI1_STAT_PHY_D0_LPDT_SHIFT                                    20
#define    DSI1_STAT_PHY_D0_LPDT_MASK                                     0x00100000
#define    DSI1_STAT_PHY_DIR_SHIFT                                        19
#define    DSI1_STAT_PHY_DIR_MASK                                         0x00080000
#define    DSI1_STAT_PHY_CLK_ULPS_SHIFT                                   18
#define    DSI1_STAT_PHY_CLK_ULPS_MASK                                    0x00040000
#define    DSI1_STAT_PHY_CLK_HS_SHIFT                                     17
#define    DSI1_STAT_PHY_CLK_HS_MASK                                      0x00020000
#define    DSI1_STAT_PHY_CLK_STOP_SHIFT                                   16
#define    DSI1_STAT_PHY_CLK_STOP_MASK                                    0x00010000
#define    DSI1_STAT_PR_TO_SHIFT                                          15
#define    DSI1_STAT_PR_TO_MASK                                           0x00008000
#define    DSI1_STAT_TA_TO_SHIFT                                          14
#define    DSI1_STAT_TA_TO_MASK                                           0x00004000
#define    DSI1_STAT_LPRX_TO_SHIFT                                        13
#define    DSI1_STAT_LPRX_TO_MASK                                         0x00002000
#define    DSI1_STAT_HSTX_TO_SHIFT                                        12
#define    DSI1_STAT_HSTX_TO_MASK                                         0x00001000
#define    DSI1_STAT_ERRCONTLP1_SHIFT                                     11
#define    DSI1_STAT_ERRCONTLP1_MASK                                      0x00000800
#define    DSI1_STAT_ERRCONTLP0_SHIFT                                     10
#define    DSI1_STAT_ERRCONTLP0_MASK                                      0x00000400
#define    DSI1_STAT_ERRCONTROL_SHIFT                                     9
#define    DSI1_STAT_ERRCONTROL_MASK                                      0x00000200
#define    DSI1_STAT_ERRSYNCESC_SHIFT                                     8
#define    DSI1_STAT_ERRSYNCESC_MASK                                      0x00000100
#define    DSI1_STAT_RX2_PKT_SHIFT                                        7
#define    DSI1_STAT_RX2_PKT_MASK                                         0x00000080
#define    DSI1_STAT_RX1_PKT_SHIFT                                        6
#define    DSI1_STAT_RX1_PKT_MASK                                         0x00000040
#define    DSI1_STAT_TXPKT2_BUSY_SHIFT                                    5
#define    DSI1_STAT_TXPKT2_BUSY_MASK                                     0x00000020
#define    DSI1_STAT_TXPKT2_DONE_SHIFT                                    4
#define    DSI1_STAT_TXPKT2_DONE_MASK                                     0x00000010
#define    DSI1_STAT_TXPKT2_END_SHIFT                                     3
#define    DSI1_STAT_TXPKT2_END_MASK                                      0x00000008
#define    DSI1_STAT_TXPKT1_BUSY_SHIFT                                    2
#define    DSI1_STAT_TXPKT1_BUSY_MASK                                     0x00000004
#define    DSI1_STAT_TXPKT1_DONE_SHIFT                                    1
#define    DSI1_STAT_TXPKT1_DONE_MASK                                     0x00000002
#define    DSI1_STAT_TXPKT1_END_SHIFT                                     0
#define    DSI1_STAT_TXPKT1_END_MASK                                      0x00000001

#define DSI1_HSTX_TO_CNT_OFFSET                                           0x0000003C
#define DSI1_HSTX_TO_CNT_TYPE                                             UInt32
#define DSI1_HSTX_TO_CNT_RESERVED_MASK                                    0xFF000000
#define    DSI1_HSTX_TO_CNT_HSTX_TO_SHIFT                                 0
#define    DSI1_HSTX_TO_CNT_HSTX_TO_MASK                                  0x00FFFFFF

#define DSI1_LPRX_TO_CNT_OFFSET                                           0x00000040
#define DSI1_LPRX_TO_CNT_TYPE                                             UInt32
#define DSI1_LPRX_TO_CNT_RESERVED_MASK                                    0xFF000000
#define    DSI1_LPRX_TO_CNT_LPRX_TO_SHIFT                                 0
#define    DSI1_LPRX_TO_CNT_LPRX_TO_MASK                                  0x00FFFFFF

#define DSI1_TA_TO_CNT_OFFSET                                             0x00000044
#define DSI1_TA_TO_CNT_TYPE                                               UInt32
#define DSI1_TA_TO_CNT_RESERVED_MASK                                      0xFF000000
#define    DSI1_TA_TO_CNT_TA_TO_SHIFT                                     0
#define    DSI1_TA_TO_CNT_TA_TO_MASK                                      0x00FFFFFF

#define DSI1_PR_TO_CNT_OFFSET                                             0x00000048
#define DSI1_PR_TO_CNT_TYPE                                               UInt32
#define DSI1_PR_TO_CNT_RESERVED_MASK                                      0xFF000000
#define    DSI1_PR_TO_CNT_PR_TO_SHIFT                                     0
#define    DSI1_PR_TO_CNT_PR_TO_MASK                                      0x00FFFFFF

#define DSI1_PHYC_OFFSET                                                  0x0000004C
#define DSI1_PHYC_TYPE                                                    UInt32
#define DSI1_PHYC_RESERVED_MASK                                           0xFC08CCC8
#define    DSI1_PHYC_ESC_CLK_LPDT_SHIFT                                   20
#define    DSI1_PHYC_ESC_CLK_LPDT_MASK                                    0x03F00000
#define    DSI1_PHYC_TX_HSCLK_CONT_SHIFT                                  18
#define    DSI1_PHYC_TX_HSCLK_CONT_MASK                                   0x00040000
#define    DSI1_PHYC_TXULPSCLK_SHIFT                                      17
#define    DSI1_PHYC_TXULPSCLK_MASK                                       0x00020000
#define    DSI1_PHYC_PHY_CLANE_EN_SHIFT                                   16
#define    DSI1_PHYC_PHY_CLANE_EN_MASK                                    0x00010000
#define    DSI1_PHYC_TXULPSESC_3_SHIFT                                    13
#define    DSI1_PHYC_TXULPSESC_3_MASK                                     0x00002000
#define    DSI1_PHYC_PHY_DLANE3_EN_SHIFT                                  12
#define    DSI1_PHYC_PHY_DLANE3_EN_MASK                                   0x00001000
#define    DSI1_PHYC_TXULPSESC_2_SHIFT                                    9
#define    DSI1_PHYC_TXULPSESC_2_MASK                                     0x00000200
#define    DSI1_PHYC_PHY_DLANE2_EN_SHIFT                                  8
#define    DSI1_PHYC_PHY_DLANE2_EN_MASK                                   0x00000100
#define    DSI1_PHYC_TXULPSESC_1_SHIFT                                    5
#define    DSI1_PHYC_TXULPSESC_1_MASK                                     0x00000020
#define    DSI1_PHYC_PHY_DLANE1_EN_SHIFT                                  4
#define    DSI1_PHYC_PHY_DLANE1_EN_MASK                                   0x00000010
#define    DSI1_PHYC_FORCE_TXSTOP_0_SHIFT                                 2
#define    DSI1_PHYC_FORCE_TXSTOP_0_MASK                                  0x00000004
#define    DSI1_PHYC_TXULPSESC_0_SHIFT                                    1
#define    DSI1_PHYC_TXULPSESC_0_MASK                                     0x00000002
#define    DSI1_PHYC_PHY_DLANE0_EN_SHIFT                                  0
#define    DSI1_PHYC_PHY_DLANE0_EN_MASK                                   0x00000001

#define DSI1_HS_CLT0_OFFSET                                               0x00000050
#define DSI1_HS_CLT0_TYPE                                                 UInt32
#define DSI1_HS_CLT0_RESERVED_MASK                                        0xF8000000
#define    DSI1_HS_CLT0_HS_CZERO_SHIFT                                    18
#define    DSI1_HS_CLT0_HS_CZERO_MASK                                     0x07FC0000
#define    DSI1_HS_CLT0_HS_CPRE_SHIFT                                     9
#define    DSI1_HS_CLT0_HS_CPRE_MASK                                      0x0003FE00
#define    DSI1_HS_CLT0_HS_CPREP_SHIFT                                    0
#define    DSI1_HS_CLT0_HS_CPREP_MASK                                     0x000001FF

#define DSI1_HS_CLT1_OFFSET                                               0x00000054
#define DSI1_HS_CLT1_TYPE                                                 UInt32
#define DSI1_HS_CLT1_RESERVED_MASK                                        0xFFFC0000
#define    DSI1_HS_CLT1_HS_CTRAIL_SHIFT                                   9
#define    DSI1_HS_CLT1_HS_CTRAIL_MASK                                    0x0003FE00
#define    DSI1_HS_CLT1_HS_CPOST_SHIFT                                    0
#define    DSI1_HS_CLT1_HS_CPOST_MASK                                     0x000001FF

#define DSI1_HS_CLT2_OFFSET                                               0x00000058
#define DSI1_HS_CLT2_TYPE                                                 UInt32
#define DSI1_HS_CLT2_RESERVED_MASK                                        0xFF000000
#define    DSI1_HS_CLT2_HS_WUP_SHIFT                                      0
#define    DSI1_HS_CLT2_HS_WUP_MASK                                       0x00FFFFFF

#define DSI1_HS_DLT3_OFFSET                                               0x0000005C
#define DSI1_HS_DLT3_TYPE                                                 UInt32
#define DSI1_HS_DLT3_RESERVED_MASK                                        0xF8000000
#define    DSI1_HS_DLT3_HS_EXIT_SHIFT                                     18
#define    DSI1_HS_DLT3_HS_EXIT_MASK                                      0x07FC0000
#define    DSI1_HS_DLT3_HS_ZERO_SHIFT                                     9
#define    DSI1_HS_DLT3_HS_ZERO_MASK                                      0x0003FE00
#define    DSI1_HS_DLT3_HS_PRE_SHIFT                                      0
#define    DSI1_HS_DLT3_HS_PRE_MASK                                       0x000001FF

#define DSI1_HS_DLT4_OFFSET                                               0x00000060
#define DSI1_HS_DLT4_TYPE                                                 UInt32
#define DSI1_HS_DLT4_RESERVED_MASK                                        0xFF800000
#define    DSI1_HS_DLT4_HS_ANLAT_SHIFT                                    18
#define    DSI1_HS_DLT4_HS_ANLAT_MASK                                     0x007C0000
#define    DSI1_HS_DLT4_HS_TRAIL_SHIFT                                    9
#define    DSI1_HS_DLT4_HS_TRAIL_MASK                                     0x0003FE00
#define    DSI1_HS_DLT4_HS_LPX_SHIFT                                      0
#define    DSI1_HS_DLT4_HS_LPX_MASK                                       0x000001FF

#define DSI1_HS_DLT5_OFFSET                                               0x00000064
#define DSI1_HS_DLT5_TYPE                                                 UInt32
#define DSI1_HS_DLT5_RESERVED_MASK                                        0xFF000000
#define    DSI1_HS_DLT5_HS_INIT_SHIFT                                     0
#define    DSI1_HS_DLT5_HS_INIT_MASK                                      0x00FFFFFF

#define DSI1_LP_DLT6_OFFSET                                               0x00000068
#define DSI1_LP_DLT6_TYPE                                                 UInt32
#define DSI1_LP_DLT6_RESERVED_MASK                                        0x00000000
#define    DSI1_LP_DLT6_TA_GET_SHIFT                                      24
#define    DSI1_LP_DLT6_TA_GET_MASK                                       0xFF000000
#define    DSI1_LP_DLT6_TA_SURE_SHIFT                                     16
#define    DSI1_LP_DLT6_TA_SURE_MASK                                      0x00FF0000
#define    DSI1_LP_DLT6_TA_GO_SHIFT                                       8
#define    DSI1_LP_DLT6_TA_GO_MASK                                        0x0000FF00
#define    DSI1_LP_DLT6_LP_LPX_SHIFT                                      0
#define    DSI1_LP_DLT6_LP_LPX_MASK                                       0x000000FF

#define DSI1_LP_DLT7_OFFSET                                               0x0000006C
#define DSI1_LP_DLT7_TYPE                                                 UInt32
#define DSI1_LP_DLT7_RESERVED_MASK                                        0xFF000000
#define    DSI1_LP_DLT7_LP_WUP_SHIFT                                      0
#define    DSI1_LP_DLT7_LP_WUP_MASK                                       0x00FFFFFF

#define DSI1_PHY_AFEC0_OFFSET                                             0x00000070
#define DSI1_PHY_AFEC0_TYPE                                               UInt32
#define DSI1_PHY_AFEC0_RESERVED_MASK                                      0x00004000
#define    DSI1_PHY_AFEC0_IDR_DLANE3_SHIFT                                29
#define    DSI1_PHY_AFEC0_IDR_DLANE3_MASK                                 0xE0000000
#define    DSI1_PHY_AFEC0_IDR_DLANE2_SHIFT                                26
#define    DSI1_PHY_AFEC0_IDR_DLANE2_MASK                                 0x1C000000
#define    DSI1_PHY_AFEC0_IDR_DLANE1_SHIFT                                23
#define    DSI1_PHY_AFEC0_IDR_DLANE1_MASK                                 0x03800000
#define    DSI1_PHY_AFEC0_IDR_DLANE0_SHIFT                                20
#define    DSI1_PHY_AFEC0_IDR_DLANE0_MASK                                 0x00700000
#define    DSI1_PHY_AFEC0_IDR_CLANE_SHIFT                                 17
#define    DSI1_PHY_AFEC0_IDR_CLANE_MASK                                  0x000E0000
#define    DSI1_PHY_AFEC0_DDR2CLK_EN_SHIFT                                16
#define    DSI1_PHY_AFEC0_DDR2CLK_EN_MASK                                 0x00010000
#define    DSI1_PHY_AFEC0_DDRCLK_EN_SHIFT                                 15
#define    DSI1_PHY_AFEC0_DDRCLK_EN_MASK                                  0x00008000
#define    DSI1_PHY_AFEC0_RESET_SHIFT                                     13
#define    DSI1_PHY_AFEC0_RESET_MASK                                      0x00002000
#define    DSI1_PHY_AFEC0_PD_SHIFT                                        12
#define    DSI1_PHY_AFEC0_PD_MASK                                         0x00001000
#define    DSI1_PHY_AFEC0_PD_BG_SHIFT                                     11
#define    DSI1_PHY_AFEC0_PD_BG_MASK                                      0x00000800
#define    DSI1_PHY_AFEC0_PD_DLANE1_SHIFT                                 10
#define    DSI1_PHY_AFEC0_PD_DLANE1_MASK                                  0x00000400
#define    DSI1_PHY_AFEC0_PD_DLANE2_SHIFT                                 9
#define    DSI1_PHY_AFEC0_PD_DLANE2_MASK                                  0x00000200
#define    DSI1_PHY_AFEC0_PD_DLANE3_SHIFT                                 8
#define    DSI1_PHY_AFEC0_PD_DLANE3_MASK                                  0x00000100
#define    DSI1_PHY_AFEC0_PTATADJ_SHIFT                                   4
#define    DSI1_PHY_AFEC0_PTATADJ_MASK                                    0x000000F0
#define    DSI1_PHY_AFEC0_CTATADJ_SHIFT                                   0
#define    DSI1_PHY_AFEC0_CTATADJ_MASK                                    0x0000000F

#define DSI1_PHY_AFEC1_OFFSET                                             0x00000074
#define DSI1_PHY_AFEC1_TYPE                                               UInt32
#define DSI1_PHY_AFEC1_RESERVED_MASK                                      0xFFF00000
#define    DSI1_PHY_AFEC1_ACTRL_DLANE3_SHIFT                              16
#define    DSI1_PHY_AFEC1_ACTRL_DLANE3_MASK                               0x000F0000
#define    DSI1_PHY_AFEC1_ACTRL_DLANE2_SHIFT                              12
#define    DSI1_PHY_AFEC1_ACTRL_DLANE2_MASK                               0x0000F000
#define    DSI1_PHY_AFEC1_ACTRL_DLANE1_SHIFT                              8
#define    DSI1_PHY_AFEC1_ACTRL_DLANE1_MASK                               0x00000F00
#define    DSI1_PHY_AFEC1_ACTRL_DLANE0_SHIFT                              4
#define    DSI1_PHY_AFEC1_ACTRL_DLANE0_MASK                               0x000000F0
#define    DSI1_PHY_AFEC1_ACTRL_CLANE_SHIFT                               0
#define    DSI1_PHY_AFEC1_ACTRL_CLANE_MASK                                0x0000000F

#define DSI1_TST_SEL_OFFSET                                               0x00000078
#define DSI1_TST_SEL_TYPE                                                 UInt32
#define DSI1_TST_SEL_RESERVED_MASK                                        0x00000000
#define    DSI1_TST_SEL_TST_SEL_VS_TST_MON_SHIFT                          0
#define    DSI1_TST_SEL_TST_SEL_VS_TST_MON_MASK                           0xFFFFFFFF

#define DSI1_TST_MON_OFFSET                                               0x0000007C
#define DSI1_TST_MON_TYPE                                                 UInt32
#define DSI1_TST_MON_RESERVED_MASK                                        0x00000000
#define    DSI1_TST_MON_TST_MON_SHIFT                                     0
#define    DSI1_TST_MON_TST_MON_MASK                                      0xFFFFFFFF

#define DSI1_PHY_TST1_OFFSET                                              0x00000080
#define DSI1_PHY_TST1_TYPE                                                UInt32
#define DSI1_PHY_TST1_RESERVED_MASK                                       0xFFE00000
#define    DSI1_PHY_TST1_PHYC_TEST_HSRST_SHIFT                            20
#define    DSI1_PHY_TST1_PHYC_TEST_HSRST_MASK                             0x00100000
#define    DSI1_PHY_TST1_PHYC_TEST_DM_SHIFT                               19
#define    DSI1_PHY_TST1_PHYC_TEST_DM_MASK                                0x00080000
#define    DSI1_PHY_TST1_PHYC_TEST_DP_SHIFT                               18
#define    DSI1_PHY_TST1_PHYC_TEST_DP_MASK                                0x00040000
#define    DSI1_PHY_TST1_PHYC_TEST_LP_SHIFT                               17
#define    DSI1_PHY_TST1_PHYC_TEST_LP_MASK                                0x00020000
#define    DSI1_PHY_TST1_PHYC_TEST_EN_SHIFT                               16
#define    DSI1_PHY_TST1_PHYC_TEST_EN_MASK                                0x00010000
#define    DSI1_PHY_TST1_PHYD3_TEST_DM_SHIFT                              15
#define    DSI1_PHY_TST1_PHYD3_TEST_DM_MASK                               0x00008000
#define    DSI1_PHY_TST1_PHYD3_TEST_DP_SHIFT                              14
#define    DSI1_PHY_TST1_PHYD3_TEST_DP_MASK                               0x00004000
#define    DSI1_PHY_TST1_PHYD3_TEST_LP_SHIFT                              13
#define    DSI1_PHY_TST1_PHYD3_TEST_LP_MASK                               0x00002000
#define    DSI1_PHY_TST1_PHYD3_TEST_EN_SHIFT                              12
#define    DSI1_PHY_TST1_PHYD3_TEST_EN_MASK                               0x00001000
#define    DSI1_PHY_TST1_PHYD2_TEST_DM_SHIFT                              11
#define    DSI1_PHY_TST1_PHYD2_TEST_DM_MASK                               0x00000800
#define    DSI1_PHY_TST1_PHYD2_TEST_DP_SHIFT                              10
#define    DSI1_PHY_TST1_PHYD2_TEST_DP_MASK                               0x00000400
#define    DSI1_PHY_TST1_PHYD2_TEST_LP_SHIFT                              9
#define    DSI1_PHY_TST1_PHYD2_TEST_LP_MASK                               0x00000200
#define    DSI1_PHY_TST1_PHYD2_TEST_EN_SHIFT                              8
#define    DSI1_PHY_TST1_PHYD2_TEST_EN_MASK                               0x00000100
#define    DSI1_PHY_TST1_PHYD1_TEST_DM_SHIFT                              7
#define    DSI1_PHY_TST1_PHYD1_TEST_DM_MASK                               0x00000080
#define    DSI1_PHY_TST1_PHYD1_TEST_DP_SHIFT                              6
#define    DSI1_PHY_TST1_PHYD1_TEST_DP_MASK                               0x00000040
#define    DSI1_PHY_TST1_PHYD1_TEST_LP_SHIFT                              5
#define    DSI1_PHY_TST1_PHYD1_TEST_LP_MASK                               0x00000020
#define    DSI1_PHY_TST1_PHYD1_TEST_EN_SHIFT                              4
#define    DSI1_PHY_TST1_PHYD1_TEST_EN_MASK                               0x00000010
#define    DSI1_PHY_TST1_PHYD0_TEST_DM_SHIFT                              3
#define    DSI1_PHY_TST1_PHYD0_TEST_DM_MASK                               0x00000008
#define    DSI1_PHY_TST1_PHYD0_TEST_DP_SHIFT                              2
#define    DSI1_PHY_TST1_PHYD0_TEST_DP_MASK                               0x00000004
#define    DSI1_PHY_TST1_PHYD0_TEST_LP_SHIFT                              1
#define    DSI1_PHY_TST1_PHYD0_TEST_LP_MASK                               0x00000002
#define    DSI1_PHY_TST1_PHYD0_TEST_EN_SHIFT                              0
#define    DSI1_PHY_TST1_PHYD0_TEST_EN_MASK                               0x00000001

#define DSI1_PHY_TST2_OFFSET                                              0x00000084
#define DSI1_PHY_TST2_TYPE                                                UInt32
#define DSI1_PHY_TST2_RESERVED_MASK                                       0x00000000
#define    DSI1_PHY_TST2_PHYD3_HSDATA_SHIFT                               24
#define    DSI1_PHY_TST2_PHYD3_HSDATA_MASK                                0xFF000000
#define    DSI1_PHY_TST2_PHYD2_HSDATA_SHIFT                               16
#define    DSI1_PHY_TST2_PHYD2_HSDATA_MASK                                0x00FF0000
#define    DSI1_PHY_TST2_PHYD1_HSDATA_SHIFT                               8
#define    DSI1_PHY_TST2_PHYD1_HSDATA_MASK                                0x0000FF00
#define    DSI1_PHY_TST2_PHYD0_HSDATA_SHIFT                               0
#define    DSI1_PHY_TST2_PHYD0_HSDATA_MASK                                0x000000FF

#define DSI1_ID_OFFSET                                                    0x00000088
#define DSI1_ID_TYPE                                                      UInt32
#define DSI1_ID_RESERVED_MASK                                             0x00000000
#define    DSI1_ID_DSI1_ID_SHIFT                                          0
#define    DSI1_ID_DSI1_ID_MASK                                           0xFFFFFFFF

#endif /* __BRCM_RDB_DSI1_H__ */



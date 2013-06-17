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

#ifndef __BRCM_RDB_MM_CFG_H__
#define __BRCM_RDB_MM_CFG_H__

#define MM_CFG_CSI0_PHY_CTRL_OFFSET                                       0x00000000
#define MM_CFG_CSI0_PHY_CTRL_TYPE                                         UInt32
#define MM_CFG_CSI0_PHY_CTRL_RESERVED_MASK                                0x4800C000
#define    MM_CFG_CSI0_PHY_CTRL_CSI_PHY_SEL_SHIFT                         31
#define    MM_CFG_CSI0_PHY_CTRL_CSI_PHY_SEL_MASK                          0x80000000
#define    MM_CFG_CSI0_PHY_CTRL_HSTX1_EN_SHIFT                            29
#define    MM_CFG_CSI0_PHY_CTRL_HSTX1_EN_MASK                             0x20000000
#define    MM_CFG_CSI0_PHY_CTRL_HSTX0_EN_SHIFT                            28
#define    MM_CFG_CSI0_PHY_CTRL_HSTX0_EN_MASK                             0x10000000
#define    MM_CFG_CSI0_PHY_CTRL_TXDDRCLK2EN_SHIFT                         26
#define    MM_CFG_CSI0_PHY_CTRL_TXDDRCLK2EN_MASK                          0x04000000
#define    MM_CFG_CSI0_PHY_CTRL_TXDDRCLKEN_SHIFT                          25
#define    MM_CFG_CSI0_PHY_CTRL_TXDDRCLKEN_MASK                           0x02000000
#define    MM_CFG_CSI0_PHY_CTRL_HSTX_RESET_CLK_SHIFT                      24
#define    MM_CFG_CSI0_PHY_CTRL_HSTX_RESET_CLK_MASK                       0x01000000
#define    MM_CFG_CSI0_PHY_CTRL_LPTX_DM_IN_CLK_SHIFT                      23
#define    MM_CFG_CSI0_PHY_CTRL_LPTX_DM_IN_CLK_MASK                       0x00800000
#define    MM_CFG_CSI0_PHY_CTRL_LPTX_DP_IN_CLK_SHIFT                      22
#define    MM_CFG_CSI0_PHY_CTRL_LPTX_DP_IN_CLK_MASK                       0x00400000
#define    MM_CFG_CSI0_PHY_CTRL_LPTX_EN_CLK_SHIFT                         21
#define    MM_CFG_CSI0_PHY_CTRL_LPTX_EN_CLK_MASK                          0x00200000
#define    MM_CFG_CSI0_PHY_CTRL_FS2X_EN_CLK_SHIFT                         20
#define    MM_CFG_CSI0_PHY_CTRL_FS2X_EN_CLK_MASK                          0x00100000
#define    MM_CFG_CSI0_PHY_CTRL_HSTX_CLK_IDR_SHIFT                        17
#define    MM_CFG_CSI0_PHY_CTRL_HSTX_CLK_IDR_MASK                         0x000E0000
#define    MM_CFG_CSI0_PHY_CTRL_HSRX_EN_CLK_SHIFT                         16
#define    MM_CFG_CSI0_PHY_CTRL_HSRX_EN_CLK_MASK                          0x00010000
#define    MM_CFG_CSI0_PHY_CTRL_LPTX1_DM_IN_SHIFT                         13
#define    MM_CFG_CSI0_PHY_CTRL_LPTX1_DM_IN_MASK                          0x00002000
#define    MM_CFG_CSI0_PHY_CTRL_LPTX0_DM_IN_SHIFT                         12
#define    MM_CFG_CSI0_PHY_CTRL_LPTX0_DM_IN_MASK                          0x00001000
#define    MM_CFG_CSI0_PHY_CTRL_LPTX1_DP_IN_SHIFT                         11
#define    MM_CFG_CSI0_PHY_CTRL_LPTX1_DP_IN_MASK                          0x00000800
#define    MM_CFG_CSI0_PHY_CTRL_LPTX0_DP_IN_SHIFT                         10
#define    MM_CFG_CSI0_PHY_CTRL_LPTX0_DP_IN_MASK                          0x00000400
#define    MM_CFG_CSI0_PHY_CTRL_LPCD1_EN_SHIFT                            9
#define    MM_CFG_CSI0_PHY_CTRL_LPCD1_EN_MASK                             0x00000200
#define    MM_CFG_CSI0_PHY_CTRL_LPCD0_EN_SHIFT                            8
#define    MM_CFG_CSI0_PHY_CTRL_LPCD0_EN_MASK                             0x00000100
#define    MM_CFG_CSI0_PHY_CTRL_LPTX1_EN_SHIFT                            7
#define    MM_CFG_CSI0_PHY_CTRL_LPTX1_EN_MASK                             0x00000080
#define    MM_CFG_CSI0_PHY_CTRL_LPTX0_EN_SHIFT                            6
#define    MM_CFG_CSI0_PHY_CTRL_LPTX0_EN_MASK                             0x00000040
#define    MM_CFG_CSI0_PHY_CTRL_HSTX1_IDR_SHIFT                           3
#define    MM_CFG_CSI0_PHY_CTRL_HSTX1_IDR_MASK                            0x00000038
#define    MM_CFG_CSI0_PHY_CTRL_HSTX0_IDR_SHIFT                           0
#define    MM_CFG_CSI0_PHY_CTRL_HSTX0_IDR_MASK                            0x00000007

#define MM_CFG_CSI1_PHY_CTRL_OFFSET                                       0x00000004
#define MM_CFG_CSI1_PHY_CTRL_TYPE                                         UInt32
#define MM_CFG_CSI1_PHY_CTRL_RESERVED_MASK                                0xC800C000
#define    MM_CFG_CSI1_PHY_CTRL_HSTX1_EN_SHIFT                            29
#define    MM_CFG_CSI1_PHY_CTRL_HSTX1_EN_MASK                             0x20000000
#define    MM_CFG_CSI1_PHY_CTRL_HSTX0_EN_SHIFT                            28
#define    MM_CFG_CSI1_PHY_CTRL_HSTX0_EN_MASK                             0x10000000
#define    MM_CFG_CSI1_PHY_CTRL_TXDDRCLK2EN_SHIFT                         26
#define    MM_CFG_CSI1_PHY_CTRL_TXDDRCLK2EN_MASK                          0x04000000
#define    MM_CFG_CSI1_PHY_CTRL_TXDDRCLKEN_SHIFT                          25
#define    MM_CFG_CSI1_PHY_CTRL_TXDDRCLKEN_MASK                           0x02000000
#define    MM_CFG_CSI1_PHY_CTRL_HSTX_RESET_CLK_SHIFT                      24
#define    MM_CFG_CSI1_PHY_CTRL_HSTX_RESET_CLK_MASK                       0x01000000
#define    MM_CFG_CSI1_PHY_CTRL_LPTX_DM_IN_CLK_SHIFT                      23
#define    MM_CFG_CSI1_PHY_CTRL_LPTX_DM_IN_CLK_MASK                       0x00800000
#define    MM_CFG_CSI1_PHY_CTRL_LPTX_DP_IN_CLK_SHIFT                      22
#define    MM_CFG_CSI1_PHY_CTRL_LPTX_DP_IN_CLK_MASK                       0x00400000
#define    MM_CFG_CSI1_PHY_CTRL_LPTX_EN_CLK_SHIFT                         21
#define    MM_CFG_CSI1_PHY_CTRL_LPTX_EN_CLK_MASK                          0x00200000
#define    MM_CFG_CSI1_PHY_CTRL_FS2X_EN_CLK_SHIFT                         20
#define    MM_CFG_CSI1_PHY_CTRL_FS2X_EN_CLK_MASK                          0x00100000
#define    MM_CFG_CSI1_PHY_CTRL_HSTX_CLK_IDR_SHIFT                        17
#define    MM_CFG_CSI1_PHY_CTRL_HSTX_CLK_IDR_MASK                         0x000E0000
#define    MM_CFG_CSI1_PHY_CTRL_HSRX_EN_CLK_SHIFT                         16
#define    MM_CFG_CSI1_PHY_CTRL_HSRX_EN_CLK_MASK                          0x00010000
#define    MM_CFG_CSI1_PHY_CTRL_LPTX1_DM_IN_SHIFT                         13
#define    MM_CFG_CSI1_PHY_CTRL_LPTX1_DM_IN_MASK                          0x00002000
#define    MM_CFG_CSI1_PHY_CTRL_LPTX0_DM_IN_SHIFT                         12
#define    MM_CFG_CSI1_PHY_CTRL_LPTX0_DM_IN_MASK                          0x00001000
#define    MM_CFG_CSI1_PHY_CTRL_LPTX1_DP_IN_SHIFT                         11
#define    MM_CFG_CSI1_PHY_CTRL_LPTX1_DP_IN_MASK                          0x00000800
#define    MM_CFG_CSI1_PHY_CTRL_LPTX0_DP_IN_SHIFT                         10
#define    MM_CFG_CSI1_PHY_CTRL_LPTX0_DP_IN_MASK                          0x00000400
#define    MM_CFG_CSI1_PHY_CTRL_LPCD1_EN_SHIFT                            9
#define    MM_CFG_CSI1_PHY_CTRL_LPCD1_EN_MASK                             0x00000200
#define    MM_CFG_CSI1_PHY_CTRL_LPCD0_EN_SHIFT                            8
#define    MM_CFG_CSI1_PHY_CTRL_LPCD0_EN_MASK                             0x00000100
#define    MM_CFG_CSI1_PHY_CTRL_LPTX1_EN_SHIFT                            7
#define    MM_CFG_CSI1_PHY_CTRL_LPTX1_EN_MASK                             0x00000080
#define    MM_CFG_CSI1_PHY_CTRL_LPTX0_EN_SHIFT                            6
#define    MM_CFG_CSI1_PHY_CTRL_LPTX0_EN_MASK                             0x00000040
#define    MM_CFG_CSI1_PHY_CTRL_HSTX1_IDR_SHIFT                           3
#define    MM_CFG_CSI1_PHY_CTRL_HSTX1_IDR_MASK                            0x00000038
#define    MM_CFG_CSI1_PHY_CTRL_HSTX0_IDR_SHIFT                           0
#define    MM_CFG_CSI1_PHY_CTRL_HSTX0_IDR_MASK                            0x00000007

#define MM_CFG_CSI_LN0_PRBS_CTRL_OFFSET                                   0x00000008
#define MM_CFG_CSI_LN0_PRBS_CTRL_TYPE                                     UInt32
#define MM_CFG_CSI_LN0_PRBS_CTRL_RESERVED_MASK                            0xFE00C000
#define    MM_CFG_CSI_LN0_PRBS_CTRL_CONST_REG_EN_SHIFT                    24
#define    MM_CFG_CSI_LN0_PRBS_CTRL_CONST_REG_EN_MASK                     0x01000000
#define    MM_CFG_CSI_LN0_PRBS_CTRL_CONSTANT_REG_SHIFT                    16
#define    MM_CFG_CSI_LN0_PRBS_CTRL_CONSTANT_REG_MASK                     0x00FF0000
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_INV_SHIFT                          13
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_INV_MASK                           0x00002000
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PHY_SELECT_SHIFT                      12
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PHY_SELECT_MASK                       0x00001000
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_ORDER_SHIFT                        10
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_ORDER_MASK                         0x00000C00
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_ERR_CLR_SHIFT                      9
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_ERR_CLR_MASK                       0x00000200
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_ENABLE_SHIFT                       8
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_ENABLE_MASK                        0x00000100
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_WIDTH_SEL_SHIFT                    7
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_WIDTH_SEL_MASK                     0x00000080
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_SOFTRST_N_SHIFT                    6
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PM_SOFTRST_N_MASK                     0x00000040
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_ORDER_SHIFT                        4
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_ORDER_MASK                         0x00000030
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_INV_SHIFT                          3
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_INV_MASK                           0x00000008
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_ENABLE_SHIFT                       2
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_ENABLE_MASK                        0x00000004
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_WIDTH_SEL_SHIFT                    1
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_WIDTH_SEL_MASK                     0x00000002
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_SOFTRST_N_SHIFT                    0
#define    MM_CFG_CSI_LN0_PRBS_CTRL_PG_SOFTRST_N_MASK                     0x00000001

#define MM_CFG_CSI_LN0_PRBS_ST_OFFSET                                     0x0000000C
#define MM_CFG_CSI_LN0_PRBS_ST_TYPE                                       UInt32
#define MM_CFG_CSI_LN0_PRBS_ST_RESERVED_MASK                              0xC000FF0C
#define    MM_CFG_CSI_LN0_PRBS_ST_RPBS_ERRORS_SHIFT                       16
#define    MM_CFG_CSI_LN0_PRBS_ST_RPBS_ERRORS_MASK                        0x3FFF0000
#define    MM_CFG_CSI_LN0_PRBS_ST_RPBS_STATE_SHIFT                        4
#define    MM_CFG_CSI_LN0_PRBS_ST_RPBS_STATE_MASK                         0x000000F0
#define    MM_CFG_CSI_LN0_PRBS_ST_PRBS_STKY_SHIFT                         1
#define    MM_CFG_CSI_LN0_PRBS_ST_PRBS_STKY_MASK                          0x00000002
#define    MM_CFG_CSI_LN0_PRBS_ST_PRBS_LOCK_SHIFT                         0
#define    MM_CFG_CSI_LN0_PRBS_ST_PRBS_LOCK_MASK                          0x00000001

#define MM_CFG_CSI_LN1_PRBS_CTRL_OFFSET                                   0x00000010
#define MM_CFG_CSI_LN1_PRBS_CTRL_TYPE                                     UInt32
#define MM_CFG_CSI_LN1_PRBS_CTRL_RESERVED_MASK                            0xFE00C000
#define    MM_CFG_CSI_LN1_PRBS_CTRL_CONST_REG_EN_SHIFT                    24
#define    MM_CFG_CSI_LN1_PRBS_CTRL_CONST_REG_EN_MASK                     0x01000000
#define    MM_CFG_CSI_LN1_PRBS_CTRL_CONSTANT_REG_SHIFT                    16
#define    MM_CFG_CSI_LN1_PRBS_CTRL_CONSTANT_REG_MASK                     0x00FF0000
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_INV_SHIFT                          13
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_INV_MASK                           0x00002000
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PHY_SELECT_SHIFT                      12
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PHY_SELECT_MASK                       0x00001000
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_ORDER_SHIFT                        10
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_ORDER_MASK                         0x00000C00
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_ERR_CLR_SHIFT                      9
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_ERR_CLR_MASK                       0x00000200
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_ENABLE_SHIFT                       8
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_ENABLE_MASK                        0x00000100
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_WIDTH_SEL_SHIFT                    7
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_WIDTH_SEL_MASK                     0x00000080
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_SOFTRST_N_SHIFT                    6
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PM_SOFTRST_N_MASK                     0x00000040
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_ORDER_SHIFT                        4
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_ORDER_MASK                         0x00000030
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_INV_SHIFT                          3
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_INV_MASK                           0x00000008
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_ENABLE_SHIFT                       2
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_ENABLE_MASK                        0x00000004
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_WIDTH_SEL_SHIFT                    1
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_WIDTH_SEL_MASK                     0x00000002
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_SOFTRST_N_SHIFT                    0
#define    MM_CFG_CSI_LN1_PRBS_CTRL_PG_SOFTRST_N_MASK                     0x00000001

#define MM_CFG_CSI_LN1_PRBS_ST_OFFSET                                     0x00000014
#define MM_CFG_CSI_LN1_PRBS_ST_TYPE                                       UInt32
#define MM_CFG_CSI_LN1_PRBS_ST_RESERVED_MASK                              0xC000FF0C
#define    MM_CFG_CSI_LN1_PRBS_ST_RPBS_ERRORS_SHIFT                       16
#define    MM_CFG_CSI_LN1_PRBS_ST_RPBS_ERRORS_MASK                        0x3FFF0000
#define    MM_CFG_CSI_LN1_PRBS_ST_RPBS_STATE_SHIFT                        4
#define    MM_CFG_CSI_LN1_PRBS_ST_RPBS_STATE_MASK                         0x000000F0
#define    MM_CFG_CSI_LN1_PRBS_ST_PRBS_STKY_SHIFT                         1
#define    MM_CFG_CSI_LN1_PRBS_ST_PRBS_STKY_MASK                          0x00000002
#define    MM_CFG_CSI_LN1_PRBS_ST_PRBS_LOCK_SHIFT                         0
#define    MM_CFG_CSI_LN1_PRBS_ST_PRBS_LOCK_MASK                          0x00000001

#define MM_CFG_DSI0_PHY_CTRL_OFFSET                                       0x00000018
#define MM_CFG_DSI0_PHY_CTRL_TYPE                                         UInt32
#define MM_CFG_DSI0_PHY_CTRL_RESERVED_MASK                                0xFFFF0000
#define    MM_CFG_DSI0_PHY_CTRL_LANE0_PD_CLK_SHIFT                        15
#define    MM_CFG_DSI0_PHY_CTRL_LANE0_PD_CLK_MASK                         0x00008000
#define    MM_CFG_DSI0_PHY_CTRL_TERMRES_EN_CLK_SHIFT                      14
#define    MM_CFG_DSI0_PHY_CTRL_TERMRES_EN_CLK_MASK                       0x00004000
#define    MM_CFG_DSI0_PHY_CTRL_LPRX_EN_CLK_SHIFT                         13
#define    MM_CFG_DSI0_PHY_CTRL_LPRX_EN_CLK_MASK                          0x00002000
#define    MM_CFG_DSI0_PHY_CTRL_FS2X_EN_CLK_SHIFT                         12
#define    MM_CFG_DSI0_PHY_CTRL_FS2X_EN_CLK_MASK                          0x00001000
#define    MM_CFG_DSI0_PHY_CTRL_HSRX_EN_CLK_SHIFT                         11
#define    MM_CFG_DSI0_PHY_CTRL_HSRX_EN_CLK_MASK                          0x00000800
#define    MM_CFG_DSI0_PHY_CTRL_LPTX0_DM_IN_SHIFT                         10
#define    MM_CFG_DSI0_PHY_CTRL_LPTX0_DM_IN_MASK                          0x00000400
#define    MM_CFG_DSI0_PHY_CTRL_LPTX0_DP_IN_SHIFT                         9
#define    MM_CFG_DSI0_PHY_CTRL_LPTX0_DP_IN_MASK                          0x00000200
#define    MM_CFG_DSI0_PHY_CTRL_LANE0_PD_SHIFT                            8
#define    MM_CFG_DSI0_PHY_CTRL_LANE0_PD_MASK                             0x00000100
#define    MM_CFG_DSI0_PHY_CTRL_TERMRES0_EN_SHIFT                         7
#define    MM_CFG_DSI0_PHY_CTRL_TERMRES0_EN_MASK                          0x00000080
#define    MM_CFG_DSI0_PHY_CTRL_LPCD0_EN_SHIFT                            6
#define    MM_CFG_DSI0_PHY_CTRL_LPCD0_EN_MASK                             0x00000040
#define    MM_CFG_DSI0_PHY_CTRL_LPTX0_EN_SHIFT                            5
#define    MM_CFG_DSI0_PHY_CTRL_LPTX0_EN_MASK                             0x00000020
#define    MM_CFG_DSI0_PHY_CTRL_LPRX0_EN_SHIFT                            4
#define    MM_CFG_DSI0_PHY_CTRL_LPRX0_EN_MASK                             0x00000010
#define    MM_CFG_DSI0_PHY_CTRL_HSTX0_EN_SHIFT                            3
#define    MM_CFG_DSI0_PHY_CTRL_HSTX0_EN_MASK                             0x00000008
#define    MM_CFG_DSI0_PHY_CTRL_HSRX0_EN_SHIFT                            2
#define    MM_CFG_DSI0_PHY_CTRL_HSRX0_EN_MASK                             0x00000004
#define    MM_CFG_DSI0_PHY_CTRL_CKMODE_SHIFT                              1
#define    MM_CFG_DSI0_PHY_CTRL_CKMODE_MASK                               0x00000002
#define    MM_CFG_DSI0_PHY_CTRL_CCP2_CSIB_SHIFT                           0
#define    MM_CFG_DSI0_PHY_CTRL_CCP2_CSIB_MASK                            0x00000001

#define MM_CFG_DSI1_PHY_CTRL_OFFSET                                       0x0000001C
#define MM_CFG_DSI1_PHY_CTRL_TYPE                                         UInt32
#define MM_CFG_DSI1_PHY_CTRL_RESERVED_MASK                                0xFFFF0000
#define    MM_CFG_DSI1_PHY_CTRL_LANE0_PD_CLK_SHIFT                        15
#define    MM_CFG_DSI1_PHY_CTRL_LANE0_PD_CLK_MASK                         0x00008000
#define    MM_CFG_DSI1_PHY_CTRL_TERMRES_EN_CLK_SHIFT                      14
#define    MM_CFG_DSI1_PHY_CTRL_TERMRES_EN_CLK_MASK                       0x00004000
#define    MM_CFG_DSI1_PHY_CTRL_LPRX_EN_CLK_SHIFT                         13
#define    MM_CFG_DSI1_PHY_CTRL_LPRX_EN_CLK_MASK                          0x00002000
#define    MM_CFG_DSI1_PHY_CTRL_FS2X_EN_CLK_SHIFT                         12
#define    MM_CFG_DSI1_PHY_CTRL_FS2X_EN_CLK_MASK                          0x00001000
#define    MM_CFG_DSI1_PHY_CTRL_HSRX_EN_CLK_SHIFT                         11
#define    MM_CFG_DSI1_PHY_CTRL_HSRX_EN_CLK_MASK                          0x00000800
#define    MM_CFG_DSI1_PHY_CTRL_LPTX0_DM_IN_SHIFT                         10
#define    MM_CFG_DSI1_PHY_CTRL_LPTX0_DM_IN_MASK                          0x00000400
#define    MM_CFG_DSI1_PHY_CTRL_LPTX0_DP_IN_SHIFT                         9
#define    MM_CFG_DSI1_PHY_CTRL_LPTX0_DP_IN_MASK                          0x00000200
#define    MM_CFG_DSI1_PHY_CTRL_LANE0_PD_SHIFT                            8
#define    MM_CFG_DSI1_PHY_CTRL_LANE0_PD_MASK                             0x00000100
#define    MM_CFG_DSI1_PHY_CTRL_TERMRES0_EN_SHIFT                         7
#define    MM_CFG_DSI1_PHY_CTRL_TERMRES0_EN_MASK                          0x00000080
#define    MM_CFG_DSI1_PHY_CTRL_LPCD0_EN_SHIFT                            6
#define    MM_CFG_DSI1_PHY_CTRL_LPCD0_EN_MASK                             0x00000040
#define    MM_CFG_DSI1_PHY_CTRL_LPTX0_EN_SHIFT                            5
#define    MM_CFG_DSI1_PHY_CTRL_LPTX0_EN_MASK                             0x00000020
#define    MM_CFG_DSI1_PHY_CTRL_LPRX0_EN_SHIFT                            4
#define    MM_CFG_DSI1_PHY_CTRL_LPRX0_EN_MASK                             0x00000010
#define    MM_CFG_DSI1_PHY_CTRL_HSTX0_EN_SHIFT                            3
#define    MM_CFG_DSI1_PHY_CTRL_HSTX0_EN_MASK                             0x00000008
#define    MM_CFG_DSI1_PHY_CTRL_HSRX0_EN_SHIFT                            2
#define    MM_CFG_DSI1_PHY_CTRL_HSRX0_EN_MASK                             0x00000004
#define    MM_CFG_DSI1_PHY_CTRL_CKMODE_SHIFT                              1
#define    MM_CFG_DSI1_PHY_CTRL_CKMODE_MASK                               0x00000002
#define    MM_CFG_DSI1_PHY_CTRL_CCP2_CSIB_SHIFT                           0
#define    MM_CFG_DSI1_PHY_CTRL_CCP2_CSIB_MASK                            0x00000001

#define MM_CFG_DSI_PRBS_CTRL_OFFSET                                       0x00000020
#define MM_CFG_DSI_PRBS_CTRL_TYPE                                         UInt32
#define MM_CFG_DSI_PRBS_CTRL_RESERVED_MASK                                0x0C008000
#define    MM_CFG_DSI_PRBS_CTRL_ISP_ECO_BYPASS_SHIFT                      31
#define    MM_CFG_DSI_PRBS_CTRL_ISP_ECO_BYPASS_MASK                       0x80000000
#define    MM_CFG_DSI_PRBS_CTRL_SMI_ECO_BYPASS_SHIFT                      30
#define    MM_CFG_DSI_PRBS_CTRL_SMI_ECO_BYPASS_MASK                       0x40000000
#define    MM_CFG_DSI_PRBS_CTRL_VCE_ECO_BYPASS_SHIFT                      29
#define    MM_CFG_DSI_PRBS_CTRL_VCE_ECO_BYPASS_MASK                       0x20000000
#define    MM_CFG_DSI_PRBS_CTRL_DSI_ECO_BYPASS_SHIFT                      28
#define    MM_CFG_DSI_PRBS_CTRL_DSI_ECO_BYPASS_MASK                       0x10000000
#define    MM_CFG_DSI_PRBS_CTRL_PM_INV_SHIFT                              25
#define    MM_CFG_DSI_PRBS_CTRL_PM_INV_MASK                               0x02000000
#define    MM_CFG_DSI_PRBS_CTRL_DSI_CONST_REG_EN_SHIFT                    24
#define    MM_CFG_DSI_PRBS_CTRL_DSI_CONST_REG_EN_MASK                     0x01000000
#define    MM_CFG_DSI_PRBS_CTRL_DSI_CONSTANT_REG_SHIFT                    16
#define    MM_CFG_DSI_PRBS_CTRL_DSI_CONSTANT_REG_MASK                     0x00FF0000
#define    MM_CFG_DSI_PRBS_CTRL_DSI1_DIRECT_REGCTRL_EN_SHIFT              14
#define    MM_CFG_DSI_PRBS_CTRL_DSI1_DIRECT_REGCTRL_EN_MASK               0x00004000
#define    MM_CFG_DSI_PRBS_CTRL_DSI0_DIRECT_REGCTRL_EN_SHIFT              13
#define    MM_CFG_DSI_PRBS_CTRL_DSI0_DIRECT_REGCTRL_EN_MASK               0x00002000
#define    MM_CFG_DSI_PRBS_CTRL_PHY_SELECT_SHIFT                          12
#define    MM_CFG_DSI_PRBS_CTRL_PHY_SELECT_MASK                           0x00001000
#define    MM_CFG_DSI_PRBS_CTRL_PM_ORDER_SHIFT                            10
#define    MM_CFG_DSI_PRBS_CTRL_PM_ORDER_MASK                             0x00000C00
#define    MM_CFG_DSI_PRBS_CTRL_PM_ERR_CLR_SHIFT                          9
#define    MM_CFG_DSI_PRBS_CTRL_PM_ERR_CLR_MASK                           0x00000200
#define    MM_CFG_DSI_PRBS_CTRL_PM_ENABLE_SHIFT                           8
#define    MM_CFG_DSI_PRBS_CTRL_PM_ENABLE_MASK                            0x00000100
#define    MM_CFG_DSI_PRBS_CTRL_PM_WIDTH_SEL_SHIFT                        7
#define    MM_CFG_DSI_PRBS_CTRL_PM_WIDTH_SEL_MASK                         0x00000080
#define    MM_CFG_DSI_PRBS_CTRL_PM_SOFTRST_N_SHIFT                        6
#define    MM_CFG_DSI_PRBS_CTRL_PM_SOFTRST_N_MASK                         0x00000040
#define    MM_CFG_DSI_PRBS_CTRL_PG_ORDER_SHIFT                            4
#define    MM_CFG_DSI_PRBS_CTRL_PG_ORDER_MASK                             0x00000030
#define    MM_CFG_DSI_PRBS_CTRL_PG_INV_SHIFT                              3
#define    MM_CFG_DSI_PRBS_CTRL_PG_INV_MASK                               0x00000008
#define    MM_CFG_DSI_PRBS_CTRL_PG_ENABLE_SHIFT                           2
#define    MM_CFG_DSI_PRBS_CTRL_PG_ENABLE_MASK                            0x00000004
#define    MM_CFG_DSI_PRBS_CTRL_PG_WIDTH_SEL_SHIFT                        1
#define    MM_CFG_DSI_PRBS_CTRL_PG_WIDTH_SEL_MASK                         0x00000002
#define    MM_CFG_DSI_PRBS_CTRL_PG_SOFTRST_N_SHIFT                        0
#define    MM_CFG_DSI_PRBS_CTRL_PG_SOFTRST_N_MASK                         0x00000001

#define MM_CFG_DSI_PRBS_ST_OFFSET                                         0x00000024
#define MM_CFG_DSI_PRBS_ST_TYPE                                           UInt32
#define MM_CFG_DSI_PRBS_ST_RESERVED_MASK                                  0xC000FF0C
#define    MM_CFG_DSI_PRBS_ST_RPBS_ERRORS_SHIFT                           16
#define    MM_CFG_DSI_PRBS_ST_RPBS_ERRORS_MASK                            0x3FFF0000
#define    MM_CFG_DSI_PRBS_ST_RPBS_STATE_SHIFT                            4
#define    MM_CFG_DSI_PRBS_ST_RPBS_STATE_MASK                             0x000000F0
#define    MM_CFG_DSI_PRBS_ST_PRBS_STKY_SHIFT                             1
#define    MM_CFG_DSI_PRBS_ST_PRBS_STKY_MASK                              0x00000002
#define    MM_CFG_DSI_PRBS_ST_PRBS_LOCK_SHIFT                             0
#define    MM_CFG_DSI_PRBS_ST_PRBS_LOCK_MASK                              0x00000001

#define MM_CFG_CSI0_LDO_CTL_OFFSET                                        0x00000028
#define MM_CFG_CSI0_LDO_CTL_TYPE                                          UInt32
#define MM_CFG_CSI0_LDO_CTL_RESERVED_MASK                                 0xFF000000
#define    MM_CFG_CSI0_LDO_CTL_ISO_OUT_SHIFT                              23
#define    MM_CFG_CSI0_LDO_CTL_ISO_OUT_MASK                               0x00800000
#define    MM_CFG_CSI0_LDO_CTL_ISO_IN_SHIFT                               22
#define    MM_CFG_CSI0_LDO_CTL_ISO_IN_MASK                                0x00400000
#define    MM_CFG_CSI0_LDO_CTL_LDOCNTL_SHIFT                              4
#define    MM_CFG_CSI0_LDO_CTL_LDOCNTL_MASK                               0x003FFFF0
#define    MM_CFG_CSI0_LDO_CTL_LDORSTB_1P8_SHIFT                          3
#define    MM_CFG_CSI0_LDO_CTL_LDORSTB_1P8_MASK                           0x00000008
#define    MM_CFG_CSI0_LDO_CTL_LDOCNTLEN_SHIFT                            2
#define    MM_CFG_CSI0_LDO_CTL_LDOCNTLEN_MASK                             0x00000004
#define    MM_CFG_CSI0_LDO_CTL_LP_EN_SHIFT                                1
#define    MM_CFG_CSI0_LDO_CTL_LP_EN_MASK                                 0x00000002
#define    MM_CFG_CSI0_LDO_CTL_HP_EN_SHIFT                                0
#define    MM_CFG_CSI0_LDO_CTL_HP_EN_MASK                                 0x00000001

#define MM_CFG_CSI1_LDO_CTL_OFFSET                                        0x0000002C
#define MM_CFG_CSI1_LDO_CTL_TYPE                                          UInt32
#define MM_CFG_CSI1_LDO_CTL_RESERVED_MASK                                 0xFF000000
#define    MM_CFG_CSI1_LDO_CTL_ISO_OUT_SHIFT                              23
#define    MM_CFG_CSI1_LDO_CTL_ISO_OUT_MASK                               0x00800000
#define    MM_CFG_CSI1_LDO_CTL_ISO_IN_SHIFT                               22
#define    MM_CFG_CSI1_LDO_CTL_ISO_IN_MASK                                0x00400000
#define    MM_CFG_CSI1_LDO_CTL_LDOCNTL_SHIFT                              4
#define    MM_CFG_CSI1_LDO_CTL_LDOCNTL_MASK                               0x003FFFF0
#define    MM_CFG_CSI1_LDO_CTL_LDORSTB_1P8_SHIFT                          3
#define    MM_CFG_CSI1_LDO_CTL_LDORSTB_1P8_MASK                           0x00000008
#define    MM_CFG_CSI1_LDO_CTL_LDOCNTLEN_SHIFT                            2
#define    MM_CFG_CSI1_LDO_CTL_LDOCNTLEN_MASK                             0x00000004
#define    MM_CFG_CSI1_LDO_CTL_LP_EN_SHIFT                                1
#define    MM_CFG_CSI1_LDO_CTL_LP_EN_MASK                                 0x00000002
#define    MM_CFG_CSI1_LDO_CTL_HP_EN_SHIFT                                0
#define    MM_CFG_CSI1_LDO_CTL_HP_EN_MASK                                 0x00000001

#define MM_CFG_DSI0_LDO_CTL_OFFSET                                        0x00000030
#define MM_CFG_DSI0_LDO_CTL_TYPE                                          UInt32
#define MM_CFG_DSI0_LDO_CTL_RESERVED_MASK                                 0xFF000000
#define    MM_CFG_DSI0_LDO_CTL_ISO_OUT_SHIFT                              23
#define    MM_CFG_DSI0_LDO_CTL_ISO_OUT_MASK                               0x00800000
#define    MM_CFG_DSI0_LDO_CTL_ISO_IN_SHIFT                               22
#define    MM_CFG_DSI0_LDO_CTL_ISO_IN_MASK                                0x00400000
#define    MM_CFG_DSI0_LDO_CTL_LDOCNTL_SHIFT                              4
#define    MM_CFG_DSI0_LDO_CTL_LDOCNTL_MASK                               0x003FFFF0
#define    MM_CFG_DSI0_LDO_CTL_LDORSTB_1P8_SHIFT                          3
#define    MM_CFG_DSI0_LDO_CTL_LDORSTB_1P8_MASK                           0x00000008
#define    MM_CFG_DSI0_LDO_CTL_LDOCNTLEN_SHIFT                            2
#define    MM_CFG_DSI0_LDO_CTL_LDOCNTLEN_MASK                             0x00000004
#define    MM_CFG_DSI0_LDO_CTL_LP_EN_SHIFT                                1
#define    MM_CFG_DSI0_LDO_CTL_LP_EN_MASK                                 0x00000002
#define    MM_CFG_DSI0_LDO_CTL_HP_EN_SHIFT                                0
#define    MM_CFG_DSI0_LDO_CTL_HP_EN_MASK                                 0x00000001

#define MM_CFG_DSI1_LDO_CTL_OFFSET                                        0x00000034
#define MM_CFG_DSI1_LDO_CTL_TYPE                                          UInt32
#define MM_CFG_DSI1_LDO_CTL_RESERVED_MASK                                 0xFF000000
#define    MM_CFG_DSI1_LDO_CTL_ISO_OUT_SHIFT                              23
#define    MM_CFG_DSI1_LDO_CTL_ISO_OUT_MASK                               0x00800000
#define    MM_CFG_DSI1_LDO_CTL_ISO_IN_SHIFT                               22
#define    MM_CFG_DSI1_LDO_CTL_ISO_IN_MASK                                0x00400000
#define    MM_CFG_DSI1_LDO_CTL_LDOCNTL_SHIFT                              4
#define    MM_CFG_DSI1_LDO_CTL_LDOCNTL_MASK                               0x003FFFF0
#define    MM_CFG_DSI1_LDO_CTL_LDORSTB_1P8_SHIFT                          3
#define    MM_CFG_DSI1_LDO_CTL_LDORSTB_1P8_MASK                           0x00000008
#define    MM_CFG_DSI1_LDO_CTL_LDOCNTLEN_SHIFT                            2
#define    MM_CFG_DSI1_LDO_CTL_LDOCNTLEN_MASK                             0x00000004
#define    MM_CFG_DSI1_LDO_CTL_LP_EN_SHIFT                                1
#define    MM_CFG_DSI1_LDO_CTL_LP_EN_MASK                                 0x00000002
#define    MM_CFG_DSI1_LDO_CTL_HP_EN_SHIFT                                0
#define    MM_CFG_DSI1_LDO_CTL_HP_EN_MASK                                 0x00000001

#endif /* __BRCM_RDB_MM_CFG_H__ */



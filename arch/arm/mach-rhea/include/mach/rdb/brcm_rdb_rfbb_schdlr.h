/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:17:6                                             */
/*     RDB file : //R4/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_RFBB_SCHDLR_H__
#define __BRCM_RDB_RFBB_SCHDLR_H__

#define RFBB_SCHDLR_SMCESR0_OFFSET                                        0x00000000
#define RFBB_SCHDLR_SMCESR0_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCESR0_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCESR0_SMCESR0_SHIFT                              0
#define    RFBB_SCHDLR_SMCESR0_SMCESR0_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCESR1_OFFSET                                        0x00000004
#define RFBB_SCHDLR_SMCESR1_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCESR1_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCESR1_SMCESR1_SHIFT                              0
#define    RFBB_SCHDLR_SMCESR1_SMCESR1_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_CMDBUF_ADDR_OFFSET                                    0x00000008
#define RFBB_SCHDLR_CMDBUF_ADDR_TYPE                                      UInt32
#define RFBB_SCHDLR_CMDBUF_ADDR_RESERVED_MASK                             0xFFFFFF00
#define    RFBB_SCHDLR_CMDBUF_ADDR_CMDBUF_ADDR_SHIFT                      0
#define    RFBB_SCHDLR_CMDBUF_ADDR_CMDBUF_ADDR_MASK                       0x000000FF

#define RFBB_SCHDLR_CMDBUF_RD_OFFSET                                      0x00000010
#define RFBB_SCHDLR_CMDBUF_RD_TYPE                                        UInt32
#define RFBB_SCHDLR_CMDBUF_RD_RESERVED_MASK                               0x00000000
#define    RFBB_SCHDLR_CMDBUF_RD_CMDBUF_RD_SHIFT                          0
#define    RFBB_SCHDLR_CMDBUF_RD_CMDBUF_RD_MASK                           0xFFFFFFFF

#define RFBB_SCHDLR_CMDBUF_WR_OFFSET                                      0x00000014
#define RFBB_SCHDLR_CMDBUF_WR_TYPE                                        UInt32
#define RFBB_SCHDLR_CMDBUF_WR_RESERVED_MASK                               0x00000000
#define    RFBB_SCHDLR_CMDBUF_WR_CMDBUF_WR_SHIFT                          0
#define    RFBB_SCHDLR_CMDBUF_WR_CMDBUF_WR_MASK                           0xFFFFFFFF

#define RFBB_SCHDLR_CMDBUF_SEL_OFFSET                                     0x00000018
#define RFBB_SCHDLR_CMDBUF_SEL_TYPE                                       UInt32
#define RFBB_SCHDLR_CMDBUF_SEL_RESERVED_MASK                              0xFFFFFFEE
#define    RFBB_SCHDLR_CMDBUF_SEL_CMDBUF_MODE_SHIFT                       4
#define    RFBB_SCHDLR_CMDBUF_SEL_CMDBUF_MODE_MASK                        0x00000010
#define    RFBB_SCHDLR_CMDBUF_SEL_CMDBUF_SEL_SHIFT                        0
#define    RFBB_SCHDLR_CMDBUF_SEL_CMDBUF_SEL_MASK                         0x00000001

#define RFBB_SCHDLR_SPI_CLK_FREQ_CNT_OFFSET                               0x0000001C
#define RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TYPE                                 UInt32
#define RFBB_SCHDLR_SPI_CLK_FREQ_CNT_RESERVED_MASK                        0xEE00EE00
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TWIF_WR_BIT_ORDER_SHIFT           28
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TWIF_WR_BIT_ORDER_MASK            0x10000000
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TWIF_RD_BIT_ORDER_SHIFT           24
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TWIF_RD_BIT_ORDER_MASK            0x01000000
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TWIF_CLK_FREQ_CNT_SHIFT           16
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_TWIF_CLK_FREQ_CNT_MASK            0x00FF0000
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_SPI_WR_BIT_ORDER_SHIFT            12
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_SPI_WR_BIT_ORDER_MASK             0x00001000
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_SPI_RD_BIT_ORDER_SHIFT            8
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_SPI_RD_BIT_ORDER_MASK             0x00000100
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_SPI_CLK_FREQ_CNT_SHIFT            0
#define    RFBB_SCHDLR_SPI_CLK_FREQ_CNT_SPI_CLK_FREQ_CNT_MASK             0x000000FF

#define RFBB_SCHDLR_CMD_GAP_OFFSET                                        0x00000020
#define RFBB_SCHDLR_CMD_GAP_TYPE                                          UInt32
#define RFBB_SCHDLR_CMD_GAP_RESERVED_MASK                                 0xFF000000
#define    RFBB_SCHDLR_CMD_GAP_CMD_DLY_SPI_SHIFT                          16
#define    RFBB_SCHDLR_CMD_GAP_CMD_DLY_SPI_MASK                           0x00FF0000
#define    RFBB_SCHDLR_CMD_GAP_CMD_DLY_GP_SHIFT                           8
#define    RFBB_SCHDLR_CMD_GAP_CMD_DLY_GP_MASK                            0x0000FF00
#define    RFBB_SCHDLR_CMD_GAP_CMD_DLY_TWIF_SHIFT                         0
#define    RFBB_SCHDLR_CMD_GAP_CMD_DLY_TWIF_MASK                          0x000000FF

#define RFBB_SCHDLR_RFIC_SCHEDULER_EN_OFFSET                              0x00000024
#define RFBB_SCHDLR_RFIC_SCHEDULER_EN_TYPE                                UInt32
#define RFBB_SCHDLR_RFIC_SCHEDULER_EN_RESERVED_MASK                       0xFFFFFEEE
#define    RFBB_SCHDLR_RFIC_SCHEDULER_EN_STBY_SHIFT                       8
#define    RFBB_SCHDLR_RFIC_SCHEDULER_EN_STBY_MASK                        0x00000100
#define    RFBB_SCHDLR_RFIC_SCHEDULER_EN_CLK_GATE_DISABLE_SHIFT           4
#define    RFBB_SCHDLR_RFIC_SCHEDULER_EN_CLK_GATE_DISABLE_MASK            0x00000010
#define    RFBB_SCHDLR_RFIC_SCHEDULER_EN_RFIC_SCHEDULER_EN_SHIFT          0
#define    RFBB_SCHDLR_RFIC_SCHEDULER_EN_RFIC_SCHEDULER_EN_MASK           0x00000001

#define RFBB_SCHDLR_MT_SHADOW_EN_OFFSET                                   0x00000028
#define RFBB_SCHDLR_MT_SHADOW_EN_TYPE                                     UInt32
#define RFBB_SCHDLR_MT_SHADOW_EN_RESERVED_MASK                            0xEEEEFEEE
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_SYNC_SHIFT                  28
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_SYNC_MASK                   0x10000000
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_FREEZE_SPI_SHIFT            24
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_FREEZE_SPI_MASK             0x01000000
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_FREEZE_GP_SHIFT             20
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_FREEZE_GP_MASK              0x00100000
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_FREEZE_TWIF_SHIFT           16
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_FREEZE_TWIF_MASK            0x00010000
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_EN_SPI_SHIFT                8
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_EN_SPI_MASK                 0x00000100
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_EN_GP_SHIFT                 4
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_EN_GP_MASK                  0x00000010
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_EN_TWIF_SHIFT               0
#define    RFBB_SCHDLR_MT_SHADOW_EN_MT_SHADOW_EN_TWIF_MASK                0x00000001

#define RFBB_SCHDLR_IMR_OFFSET                                            0x0000002C
#define RFBB_SCHDLR_IMR_TYPE                                              UInt32
#define RFBB_SCHDLR_IMR_RESERVED_MASK                                     0x00000000
#define    RFBB_SCHDLR_IMR_IMR_SHIFT                                      0
#define    RFBB_SCHDLR_IMR_IMR_MASK                                       0xFFFFFFFF

#define RFBB_SCHDLR_ISTAT_OFFSET                                          0x00000030
#define RFBB_SCHDLR_ISTAT_TYPE                                            UInt32
#define RFBB_SCHDLR_ISTAT_RESERVED_MASK                                   0x00000000
#define    RFBB_SCHDLR_ISTAT_ISTAT_SHIFT                                  0
#define    RFBB_SCHDLR_ISTAT_ISTAT_MASK                                   0xFFFFFFFF

#define RFBB_SCHDLR_ISTAT_CLR_OFFSET                                      0x00000034
#define RFBB_SCHDLR_ISTAT_CLR_TYPE                                        UInt32
#define RFBB_SCHDLR_ISTAT_CLR_RESERVED_MASK                               0x00000000
#define    RFBB_SCHDLR_ISTAT_CLR_ISTAT_CLR_SHIFT                          0
#define    RFBB_SCHDLR_ISTAT_CLR_ISTAT_CLR_MASK                           0xFFFFFFFF

#define RFBB_SCHDLR_TWIF_RD_CFG_OFFSET                                    0x00000038
#define RFBB_SCHDLR_TWIF_RD_CFG_TYPE                                      UInt32
#define RFBB_SCHDLR_TWIF_RD_CFG_RESERVED_MASK                             0xFFFFFEE0
#define    RFBB_SCHDLR_TWIF_RD_CFG_TWIF_RD_POL_SHIFT                      8
#define    RFBB_SCHDLR_TWIF_RD_CFG_TWIF_RD_POL_MASK                       0x00000100
#define    RFBB_SCHDLR_TWIF_RD_CFG_TWIF_RD_BIT_SHIFT                      0
#define    RFBB_SCHDLR_TWIF_RD_CFG_TWIF_RD_BIT_MASK                       0x0000001F

#define RFBB_SCHDLR_RF_SPI_CLK_SEL_OFFSET                                 0x0000003C
#define RFBB_SCHDLR_RF_SPI_CLK_SEL_TYPE                                   UInt32
#define RFBB_SCHDLR_RF_SPI_CLK_SEL_RESERVED_MASK                          0xFFFFFFFE
#define    RFBB_SCHDLR_RF_SPI_CLK_SEL_RF_SPI_CLK_SEL_SHIFT                0
#define    RFBB_SCHDLR_RF_SPI_CLK_SEL_RF_SPI_CLK_SEL_MASK                 0x00000001

#define RFBB_SCHDLR_SMCECR0_OFFSET                                        0x00000040
#define RFBB_SCHDLR_SMCECR0_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR0_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR0_SMCECR0_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR0_SMCECR0_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR1_OFFSET                                        0x00000044
#define RFBB_SCHDLR_SMCECR1_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR1_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR1_SMCECR1_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR1_SMCECR1_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR2_OFFSET                                        0x00000048
#define RFBB_SCHDLR_SMCECR2_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR2_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR2_SMCECR2_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR2_SMCECR2_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR3_OFFSET                                        0x0000004C
#define RFBB_SCHDLR_SMCECR3_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR3_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR3_SMCECR3_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR3_SMCECR3_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR4_OFFSET                                        0x00000050
#define RFBB_SCHDLR_SMCECR4_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR4_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR4_SMCECR4_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR4_SMCECR4_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR5_OFFSET                                        0x00000054
#define RFBB_SCHDLR_SMCECR5_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR5_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR5_SMCECR5_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR5_SMCECR5_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR6_OFFSET                                        0x00000058
#define RFBB_SCHDLR_SMCECR6_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR6_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR6_SMCECR6_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR6_SMCECR6_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR7_OFFSET                                        0x0000005C
#define RFBB_SCHDLR_SMCECR7_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR7_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR7_SMCECR7_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR7_SMCECR7_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR8_OFFSET                                        0x00000060
#define RFBB_SCHDLR_SMCECR8_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR8_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR8_SMCECR8_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR8_SMCECR8_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR9_OFFSET                                        0x00000064
#define RFBB_SCHDLR_SMCECR9_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCECR9_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCECR9_SMCECR9_SHIFT                              0
#define    RFBB_SCHDLR_SMCECR9_SMCECR9_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR10_OFFSET                                       0x00000068
#define RFBB_SCHDLR_SMCECR10_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR10_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR10_SMCECR10_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR10_SMCECR10_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR11_OFFSET                                       0x0000006C
#define RFBB_SCHDLR_SMCECR11_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR11_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR11_SMCECR11_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR11_SMCECR11_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR12_OFFSET                                       0x00000070
#define RFBB_SCHDLR_SMCECR12_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR12_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR12_SMCECR12_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR12_SMCECR12_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR13_OFFSET                                       0x00000074
#define RFBB_SCHDLR_SMCECR13_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR13_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR13_SMCECR13_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR13_SMCECR13_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR14_OFFSET                                       0x00000078
#define RFBB_SCHDLR_SMCECR14_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR14_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR14_SMCECR14_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR14_SMCECR14_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR15_OFFSET                                       0x0000007C
#define RFBB_SCHDLR_SMCECR15_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR15_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR15_SMCECR15_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR15_SMCECR15_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR16_OFFSET                                       0x00000080
#define RFBB_SCHDLR_SMCECR16_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR16_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR16_SMCECR16_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR16_SMCECR16_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR17_OFFSET                                       0x00000084
#define RFBB_SCHDLR_SMCECR17_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR17_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR17_SMCECR17_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR17_SMCECR17_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR18_OFFSET                                       0x00000088
#define RFBB_SCHDLR_SMCECR18_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR18_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR18_SMCECR18_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR18_SMCECR18_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR19_OFFSET                                       0x0000008C
#define RFBB_SCHDLR_SMCECR19_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR19_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR19_SMCECR19_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR19_SMCECR19_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR20_OFFSET                                       0x00000090
#define RFBB_SCHDLR_SMCECR20_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR20_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR20_SMCECR20_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR20_SMCECR20_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR21_OFFSET                                       0x00000094
#define RFBB_SCHDLR_SMCECR21_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR21_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR21_SMCECR21_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR21_SMCECR21_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR22_OFFSET                                       0x00000098
#define RFBB_SCHDLR_SMCECR22_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR22_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR22_SMCECR22_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR22_SMCECR22_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR23_OFFSET                                       0x0000009C
#define RFBB_SCHDLR_SMCECR23_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR23_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR23_SMCECR23_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR23_SMCECR23_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR24_OFFSET                                       0x000000A0
#define RFBB_SCHDLR_SMCECR24_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR24_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR24_SMCECR24_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR24_SMCECR24_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR25_OFFSET                                       0x000000A4
#define RFBB_SCHDLR_SMCECR25_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR25_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR25_SMCECR25_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR25_SMCECR25_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR26_OFFSET                                       0x000000A8
#define RFBB_SCHDLR_SMCECR26_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR26_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR26_SMCECR26_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR26_SMCECR26_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR27_OFFSET                                       0x000000AC
#define RFBB_SCHDLR_SMCECR27_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR27_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR27_SMCECR27_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR27_SMCECR27_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR28_OFFSET                                       0x000000B0
#define RFBB_SCHDLR_SMCECR28_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR28_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR28_SMCECR28_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR28_SMCECR28_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR29_OFFSET                                       0x000000B4
#define RFBB_SCHDLR_SMCECR29_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR29_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR29_SMCECR29_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR29_SMCECR29_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR30_OFFSET                                       0x000000B8
#define RFBB_SCHDLR_SMCECR30_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR30_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR30_SMCECR30_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR30_SMCECR30_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR31_OFFSET                                       0x000000BC
#define RFBB_SCHDLR_SMCECR31_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCECR31_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCECR31_SMCECR31_SHIFT                            0
#define    RFBB_SCHDLR_SMCECR31_SMCECR31_MASK                             0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD0_OFFSET                                         0x000000C0
#define RFBB_SCHDLR_SMCRD0_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD0_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD0_SMCRD0_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD0_SMCRD0_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD1_OFFSET                                         0x000000C4
#define RFBB_SCHDLR_SMCRD1_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD1_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD1_SMCRD1_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD1_SMCRD1_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD2_OFFSET                                         0x000000C8
#define RFBB_SCHDLR_SMCRD2_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD2_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD2_SMCRD2_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD2_SMCRD2_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD3_OFFSET                                         0x000000CC
#define RFBB_SCHDLR_SMCRD3_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD3_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD3_SMCRD3_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD3_SMCRD3_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD4_OFFSET                                         0x000000D0
#define RFBB_SCHDLR_SMCRD4_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD4_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD4_SMCRD4_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD4_SMCRD4_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD5_OFFSET                                         0x000000D4
#define RFBB_SCHDLR_SMCRD5_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD5_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD5_SMCRD5_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD5_SMCRD5_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD6_OFFSET                                         0x000000D8
#define RFBB_SCHDLR_SMCRD6_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD6_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD6_SMCRD6_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD6_SMCRD6_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD7_OFFSET                                         0x000000DC
#define RFBB_SCHDLR_SMCRD7_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD7_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD7_SMCRD7_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD7_SMCRD7_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD8_OFFSET                                         0x000000E0
#define RFBB_SCHDLR_SMCRD8_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD8_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD8_SMCRD8_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD8_SMCRD8_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD9_OFFSET                                         0x000000E4
#define RFBB_SCHDLR_SMCRD9_TYPE                                           UInt32
#define RFBB_SCHDLR_SMCRD9_RESERVED_MASK                                  0x00000000
#define    RFBB_SCHDLR_SMCRD9_SMCRD9_SHIFT                                0
#define    RFBB_SCHDLR_SMCRD9_SMCRD9_MASK                                 0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD10_OFFSET                                        0x000000E8
#define RFBB_SCHDLR_SMCRD10_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD10_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD10_SMCRD10_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD10_SMCRD10_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD11_OFFSET                                        0x000000EC
#define RFBB_SCHDLR_SMCRD11_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD11_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD11_SMCRD11_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD11_SMCRD11_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD12_OFFSET                                        0x000000F0
#define RFBB_SCHDLR_SMCRD12_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD12_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD12_SMCRD12_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD12_SMCRD12_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD13_OFFSET                                        0x000000F4
#define RFBB_SCHDLR_SMCRD13_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD13_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD13_SMCRD13_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD13_SMCRD13_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD14_OFFSET                                        0x000000F8
#define RFBB_SCHDLR_SMCRD14_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD14_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD14_SMCRD14_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD14_SMCRD14_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD15_OFFSET                                        0x000000FC
#define RFBB_SCHDLR_SMCRD15_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD15_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD15_SMCRD15_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD15_SMCRD15_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD16_OFFSET                                        0x00000100
#define RFBB_SCHDLR_SMCRD16_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD16_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD16_SMCRD16_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD16_SMCRD16_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD17_OFFSET                                        0x00000104
#define RFBB_SCHDLR_SMCRD17_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD17_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD17_SMCRD17_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD17_SMCRD17_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD18_OFFSET                                        0x00000108
#define RFBB_SCHDLR_SMCRD18_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD18_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD18_SMCRD18_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD18_SMCRD18_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD19_OFFSET                                        0x0000010C
#define RFBB_SCHDLR_SMCRD19_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD19_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD19_SMCRD19_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD19_SMCRD19_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD20_OFFSET                                        0x00000110
#define RFBB_SCHDLR_SMCRD20_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD20_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD20_SMCRD20_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD20_SMCRD20_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD21_OFFSET                                        0x00000114
#define RFBB_SCHDLR_SMCRD21_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD21_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD21_SMCRD21_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD21_SMCRD21_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD22_OFFSET                                        0x00000118
#define RFBB_SCHDLR_SMCRD22_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD22_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD22_SMCRD22_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD22_SMCRD22_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD23_OFFSET                                        0x0000011C
#define RFBB_SCHDLR_SMCRD23_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD23_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD23_SMCRD23_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD23_SMCRD23_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD24_OFFSET                                        0x00000120
#define RFBB_SCHDLR_SMCRD24_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD24_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD24_SMCRD24_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD24_SMCRD24_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD25_OFFSET                                        0x00000124
#define RFBB_SCHDLR_SMCRD25_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD25_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD25_SMCRD25_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD25_SMCRD25_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD26_OFFSET                                        0x00000128
#define RFBB_SCHDLR_SMCRD26_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD26_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD26_SMCRD26_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD26_SMCRD26_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD27_OFFSET                                        0x0000012C
#define RFBB_SCHDLR_SMCRD27_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD27_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD27_SMCRD27_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD27_SMCRD27_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD28_OFFSET                                        0x00000130
#define RFBB_SCHDLR_SMCRD28_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD28_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD28_SMCRD28_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD28_SMCRD28_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD29_OFFSET                                        0x00000134
#define RFBB_SCHDLR_SMCRD29_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD29_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD29_SMCRD29_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD29_SMCRD29_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD30_OFFSET                                        0x00000138
#define RFBB_SCHDLR_SMCRD30_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD30_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD30_SMCRD30_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD30_SMCRD30_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD31_OFFSET                                        0x0000013C
#define RFBB_SCHDLR_SMCRD31_TYPE                                          UInt32
#define RFBB_SCHDLR_SMCRD31_RESERVED_MASK                                 0x00000000
#define    RFBB_SCHDLR_SMCRD31_SMCRD31_SHIFT                              0
#define    RFBB_SCHDLR_SMCRD31_SMCRD31_MASK                               0xFFFFFFFF

#define RFBB_SCHDLR_SMCGPPC0_OFFSET                                       0x00000140
#define RFBB_SCHDLR_SMCGPPC0_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCGPPC0_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC1_POL_SHIFT                        31
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC1_POL_MASK                         0x80000000
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC1_PW_SHIFT                         16
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC1_PW_MASK                          0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC0_POL_SHIFT                        15
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC0_POL_MASK                         0x00008000
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC0_PW_SHIFT                         0
#define    RFBB_SCHDLR_SMCGPPC0_SMCGPPC0_PW_MASK                          0x00007FFF

#define RFBB_SCHDLR_SMCGPPC2_OFFSET                                       0x00000144
#define RFBB_SCHDLR_SMCGPPC2_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCGPPC2_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC3_POL_SHIFT                        31
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC3_POL_MASK                         0x80000000
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC3_PW_SHIFT                         16
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC3_PW_MASK                          0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC2_POL_SHIFT                        15
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC2_POL_MASK                         0x00008000
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC2_PW_SHIFT                         0
#define    RFBB_SCHDLR_SMCGPPC2_SMCGPPC2_PW_MASK                          0x00007FFF

#define RFBB_SCHDLR_SMCGPPC4_OFFSET                                       0x00000148
#define RFBB_SCHDLR_SMCGPPC4_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCGPPC4_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC5_POL_SHIFT                        31
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC5_POL_MASK                         0x80000000
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC5_PW_SHIFT                         16
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC5_PW_MASK                          0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC4_POL_SHIFT                        15
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC4_POL_MASK                         0x00008000
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC4_PW_SHIFT                         0
#define    RFBB_SCHDLR_SMCGPPC4_SMCGPPC4_PW_MASK                          0x00007FFF

#define RFBB_SCHDLR_SMCGPPC6_OFFSET                                       0x0000014C
#define RFBB_SCHDLR_SMCGPPC6_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCGPPC6_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC7_POL_SHIFT                        31
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC7_POL_MASK                         0x80000000
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC7_PW_SHIFT                         16
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC7_PW_MASK                          0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC6_POL_SHIFT                        15
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC6_POL_MASK                         0x00008000
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC6_PW_SHIFT                         0
#define    RFBB_SCHDLR_SMCGPPC6_SMCGPPC6_PW_MASK                          0x00007FFF

#define RFBB_SCHDLR_SMCGPPC8_OFFSET                                       0x00000150
#define RFBB_SCHDLR_SMCGPPC8_TYPE                                         UInt32
#define RFBB_SCHDLR_SMCGPPC8_RESERVED_MASK                                0x00000000
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC9_POL_SHIFT                        31
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC9_POL_MASK                         0x80000000
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC9_PW_SHIFT                         16
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC9_PW_MASK                          0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC8_POL_SHIFT                        15
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC8_POL_MASK                         0x00008000
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC8_PW_SHIFT                         0
#define    RFBB_SCHDLR_SMCGPPC8_SMCGPPC8_PW_MASK                          0x00007FFF

#define RFBB_SCHDLR_SMCGPPC10_OFFSET                                      0x00000154
#define RFBB_SCHDLR_SMCGPPC10_TYPE                                        UInt32
#define RFBB_SCHDLR_SMCGPPC10_RESERVED_MASK                               0x00000000
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC11_POL_SHIFT                      31
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC11_POL_MASK                       0x80000000
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC11_PW_SHIFT                       16
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC11_PW_MASK                        0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC10_POL_SHIFT                      15
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC10_POL_MASK                       0x00008000
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC10_PW_SHIFT                       0
#define    RFBB_SCHDLR_SMCGPPC10_SMCGPPC10_PW_MASK                        0x00007FFF

#define RFBB_SCHDLR_SMCGPPC12_OFFSET                                      0x00000158
#define RFBB_SCHDLR_SMCGPPC12_TYPE                                        UInt32
#define RFBB_SCHDLR_SMCGPPC12_RESERVED_MASK                               0x00000000
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC13_POL_SHIFT                      31
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC13_POL_MASK                       0x80000000
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC13_PW_SHIFT                       16
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC13_PW_MASK                        0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC12_POL_SHIFT                      15
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC12_POL_MASK                       0x00008000
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC12_PW_SHIFT                       0
#define    RFBB_SCHDLR_SMCGPPC12_SMCGPPC12_PW_MASK                        0x00007FFF

#define RFBB_SCHDLR_SMCGPPC14_OFFSET                                      0x0000015C
#define RFBB_SCHDLR_SMCGPPC14_TYPE                                        UInt32
#define RFBB_SCHDLR_SMCGPPC14_RESERVED_MASK                               0x00000000
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC15_POL_SHIFT                      31
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC15_POL_MASK                       0x80000000
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC15_PW_SHIFT                       16
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC15_PW_MASK                        0x7FFF0000
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC14_POL_SHIFT                      15
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC14_POL_MASK                       0x00008000
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC14_PW_SHIFT                       0
#define    RFBB_SCHDLR_SMCGPPC14_SMCGPPC14_PW_MASK                        0x00007FFF

#define RFBB_SCHDLR_TWIF_CFG_RD0_OFFSET                                   0x00000180
#define RFBB_SCHDLR_TWIF_CFG_RD0_TYPE                                     UInt32
#define RFBB_SCHDLR_TWIF_CFG_RD0_RESERVED_MASK                            0x00000000
#define    RFBB_SCHDLR_TWIF_CFG_RD0_TWIF_CFG_RD0_SHIFT                    0
#define    RFBB_SCHDLR_TWIF_CFG_RD0_TWIF_CFG_RD0_MASK                     0xFFFFFFFF

#define RFBB_SCHDLR_TWIF_CFG_RD1_OFFSET                                   0x00000184
#define RFBB_SCHDLR_TWIF_CFG_RD1_TYPE                                     UInt32
#define RFBB_SCHDLR_TWIF_CFG_RD1_RESERVED_MASK                            0x00000000
#define    RFBB_SCHDLR_TWIF_CFG_RD1_TWIF_CFG_RD1_SHIFT                    0
#define    RFBB_SCHDLR_TWIF_CFG_RD1_TWIF_CFG_RD1_MASK                     0xFFFFFFFF

#define RFBB_SCHDLR_TWIF_CFG_WR0_OFFSET                                   0x00000188
#define RFBB_SCHDLR_TWIF_CFG_WR0_TYPE                                     UInt32
#define RFBB_SCHDLR_TWIF_CFG_WR0_RESERVED_MASK                            0x00000000
#define    RFBB_SCHDLR_TWIF_CFG_WR0_TWIF_CFG_WR0_SHIFT                    0
#define    RFBB_SCHDLR_TWIF_CFG_WR0_TWIF_CFG_WR0_MASK                     0xFFFFFFFF

#define RFBB_SCHDLR_TWIF_CFG_WR1_OFFSET                                   0x0000018C
#define RFBB_SCHDLR_TWIF_CFG_WR1_TYPE                                     UInt32
#define RFBB_SCHDLR_TWIF_CFG_WR1_RESERVED_MASK                            0x00000000
#define    RFBB_SCHDLR_TWIF_CFG_WR1_TWIF_CFG_WR1_SHIFT                    0
#define    RFBB_SCHDLR_TWIF_CFG_WR1_TWIF_CFG_WR1_MASK                     0xFFFFFFFF

#define RFBB_SCHDLR_TWIF_DATA_LOC_OFFSET                                  0x00000190
#define RFBB_SCHDLR_TWIF_DATA_LOC_TYPE                                    UInt32
#define RFBB_SCHDLR_TWIF_DATA_LOC_RESERVED_MASK                           0xFFFFFFE0
#define    RFBB_SCHDLR_TWIF_DATA_LOC_TWIF_DATA_LOC_SHIFT                  0
#define    RFBB_SCHDLR_TWIF_DATA_LOC_TWIF_DATA_LOC_MASK                   0x0000001F

#define RFBB_SCHDLR_SMCECR_ENABLE_OFFSET                                  0x00000194
#define RFBB_SCHDLR_SMCECR_ENABLE_TYPE                                    UInt32
#define RFBB_SCHDLR_SMCECR_ENABLE_RESERVED_MASK                           0x00000000
#define    RFBB_SCHDLR_SMCECR_ENABLE_SMCECR_ENABLE_SHIFT                  0
#define    RFBB_SCHDLR_SMCECR_ENABLE_SMCECR_ENABLE_MASK                   0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR_ENABLE_SET_OFFSET                              0x00000198
#define RFBB_SCHDLR_SMCECR_ENABLE_SET_TYPE                                UInt32
#define RFBB_SCHDLR_SMCECR_ENABLE_SET_RESERVED_MASK                       0x00000000
#define    RFBB_SCHDLR_SMCECR_ENABLE_SET_SMCECR_ENABLE_SET_SHIFT          0
#define    RFBB_SCHDLR_SMCECR_ENABLE_SET_SMCECR_ENABLE_SET_MASK           0xFFFFFFFF

#define RFBB_SCHDLR_SMCECR_ENABLE_CLR_OFFSET                              0x0000019C
#define RFBB_SCHDLR_SMCECR_ENABLE_CLR_TYPE                                UInt32
#define RFBB_SCHDLR_SMCECR_ENABLE_CLR_RESERVED_MASK                       0x00000000
#define    RFBB_SCHDLR_SMCECR_ENABLE_CLR_SMCECR_ENABLE_CLR_SHIFT          0
#define    RFBB_SCHDLR_SMCECR_ENABLE_CLR_SMCECR_ENABLE_CLR_MASK           0xFFFFFFFF

#define RFBB_SCHDLR_SMCRD_EXT0_OFFSET                                     0x000001C0
#define RFBB_SCHDLR_SMCRD_EXT0_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT0_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT0_SMCRD_EXT0_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT0_SMCRD_EXT0_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT1_OFFSET                                     0x000001C4
#define RFBB_SCHDLR_SMCRD_EXT1_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT1_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT1_SMCRD_EXT1_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT1_SMCRD_EXT1_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT2_OFFSET                                     0x000001C8
#define RFBB_SCHDLR_SMCRD_EXT2_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT2_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT2_SMCRD_EXT2_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT2_SMCRD_EXT2_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT3_OFFSET                                     0x000001CC
#define RFBB_SCHDLR_SMCRD_EXT3_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT3_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT3_SMCRD_EXT3_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT3_SMCRD_EXT3_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT4_OFFSET                                     0x000001D0
#define RFBB_SCHDLR_SMCRD_EXT4_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT4_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT4_SMCRD_EXT4_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT4_SMCRD_EXT4_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT5_OFFSET                                     0x000001D4
#define RFBB_SCHDLR_SMCRD_EXT5_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT5_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT5_SMCRD_EXT5_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT5_SMCRD_EXT5_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT6_OFFSET                                     0x000001D8
#define RFBB_SCHDLR_SMCRD_EXT6_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT6_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT6_SMCRD_EXT6_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT6_SMCRD_EXT6_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT7_OFFSET                                     0x000001DC
#define RFBB_SCHDLR_SMCRD_EXT7_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT7_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT7_SMCRD_EXT7_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT7_SMCRD_EXT7_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT8_OFFSET                                     0x000001E0
#define RFBB_SCHDLR_SMCRD_EXT8_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT8_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT8_SMCRD_EXT8_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT8_SMCRD_EXT8_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT9_OFFSET                                     0x000001E4
#define RFBB_SCHDLR_SMCRD_EXT9_TYPE                                       UInt32
#define RFBB_SCHDLR_SMCRD_EXT9_RESERVED_MASK                              0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT9_SMCRD_EXT9_SHIFT                        0
#define    RFBB_SCHDLR_SMCRD_EXT9_SMCRD_EXT9_MASK                         0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT10_OFFSET                                    0x000001E8
#define RFBB_SCHDLR_SMCRD_EXT10_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT10_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT10_SMCRD_EXT10_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT10_SMCRD_EXT10_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT11_OFFSET                                    0x000001EC
#define RFBB_SCHDLR_SMCRD_EXT11_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT11_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT11_SMCRD_EXT11_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT11_SMCRD_EXT11_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT12_OFFSET                                    0x000001F0
#define RFBB_SCHDLR_SMCRD_EXT12_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT12_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT12_SMCRD_EXT12_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT12_SMCRD_EXT12_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT13_OFFSET                                    0x000001F4
#define RFBB_SCHDLR_SMCRD_EXT13_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT13_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT13_SMCRD_EXT13_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT13_SMCRD_EXT13_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT14_OFFSET                                    0x000001F8
#define RFBB_SCHDLR_SMCRD_EXT14_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT14_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT14_SMCRD_EXT14_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT14_SMCRD_EXT14_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT15_OFFSET                                    0x000001FC
#define RFBB_SCHDLR_SMCRD_EXT15_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT15_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT15_SMCRD_EXT15_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT15_SMCRD_EXT15_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT16_OFFSET                                    0x00000200
#define RFBB_SCHDLR_SMCRD_EXT16_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT16_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT16_SMCRD_EXT16_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT16_SMCRD_EXT16_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT17_OFFSET                                    0x00000204
#define RFBB_SCHDLR_SMCRD_EXT17_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT17_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT17_SMCRD_EXT17_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT17_SMCRD_EXT17_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT18_OFFSET                                    0x00000208
#define RFBB_SCHDLR_SMCRD_EXT18_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT18_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT18_SMCRD_EXT18_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT18_SMCRD_EXT18_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT19_OFFSET                                    0x0000020C
#define RFBB_SCHDLR_SMCRD_EXT19_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT19_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT19_SMCRD_EXT19_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT19_SMCRD_EXT19_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT20_OFFSET                                    0x00000210
#define RFBB_SCHDLR_SMCRD_EXT20_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT20_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT20_SMCRD_EXT20_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT20_SMCRD_EXT20_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT21_OFFSET                                    0x00000214
#define RFBB_SCHDLR_SMCRD_EXT21_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT21_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT21_SMCRD_EXT21_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT21_SMCRD_EXT21_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT22_OFFSET                                    0x00000218
#define RFBB_SCHDLR_SMCRD_EXT22_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT22_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT22_SMCRD_EXT22_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT22_SMCRD_EXT22_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT23_OFFSET                                    0x0000021C
#define RFBB_SCHDLR_SMCRD_EXT23_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT23_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT23_SMCRD_EXT23_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT23_SMCRD_EXT23_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT24_OFFSET                                    0x00000220
#define RFBB_SCHDLR_SMCRD_EXT24_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT24_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT24_SMCRD_EXT24_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT24_SMCRD_EXT24_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT25_OFFSET                                    0x00000224
#define RFBB_SCHDLR_SMCRD_EXT25_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT25_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT25_SMCRD_EXT25_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT25_SMCRD_EXT25_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT26_OFFSET                                    0x00000228
#define RFBB_SCHDLR_SMCRD_EXT26_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT26_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT26_SMCRD_EXT26_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT26_SMCRD_EXT26_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT27_OFFSET                                    0x0000022C
#define RFBB_SCHDLR_SMCRD_EXT27_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT27_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT27_SMCRD_EXT27_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT27_SMCRD_EXT27_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT28_OFFSET                                    0x00000230
#define RFBB_SCHDLR_SMCRD_EXT28_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT28_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT28_SMCRD_EXT28_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT28_SMCRD_EXT28_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT29_OFFSET                                    0x00000234
#define RFBB_SCHDLR_SMCRD_EXT29_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT29_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT29_SMCRD_EXT29_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT29_SMCRD_EXT29_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT30_OFFSET                                    0x00000238
#define RFBB_SCHDLR_SMCRD_EXT30_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT30_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT30_SMCRD_EXT30_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT30_SMCRD_EXT30_MASK                       0x0000000F

#define RFBB_SCHDLR_SMCRD_EXT31_OFFSET                                    0x0000023C
#define RFBB_SCHDLR_SMCRD_EXT31_TYPE                                      UInt32
#define RFBB_SCHDLR_SMCRD_EXT31_RESERVED_MASK                             0xFFFFFFF0
#define    RFBB_SCHDLR_SMCRD_EXT31_SMCRD_EXT31_SHIFT                      0
#define    RFBB_SCHDLR_SMCRD_EXT31_SMCRD_EXT31_MASK                       0x0000000F

#define RFBB_SCHDLR_POWER_HOLD_OFFSET                                     0x00000240
#define RFBB_SCHDLR_POWER_HOLD_TYPE                                       UInt32
#define RFBB_SCHDLR_POWER_HOLD_RESERVED_MASK                              0xFFFFFEEE
#define    RFBB_SCHDLR_POWER_HOLD_POWER_HOLD_MASK_SHIFT                   8
#define    RFBB_SCHDLR_POWER_HOLD_POWER_HOLD_MASK_MASK                    0x00000100
#define    RFBB_SCHDLR_POWER_HOLD_POWER_HOLD_FORCE_SHIFT                  4
#define    RFBB_SCHDLR_POWER_HOLD_POWER_HOLD_FORCE_MASK                   0x00000010
#define    RFBB_SCHDLR_POWER_HOLD_POWER_HOLD_RAW_SHIFT                    0
#define    RFBB_SCHDLR_POWER_HOLD_POWER_HOLD_RAW_MASK                     0x00000001

typedef volatile struct {
   UInt32 m_SMCESR0;                 // 0x0000
   UInt32 m_SMCESR1;                 // 0x0004
   UInt32 m_CMDBUF_ADDR;             // 0x0008
   UInt32 RESERVED_10_8[1];
   UInt32 m_CMDBUF_RD;               // 0x0010
   UInt32 m_CMDBUF_WR;               // 0x0014
   UInt32 m_CMDBUF_SEL;              // 0x0018
   UInt32 m_SPI_CLK_FREQ_CNT;        // 0x001C
   UInt32 m_CMD_GAP;                 // 0x0020
   UInt32 m_RFIC_SCHEDULER_EN;       // 0x0024
   UInt32 m_MT_SHADOW_EN;            // 0x0028
   UInt32 m_IMR;                     // 0x002C
   UInt32 m_ISTAT;                   // 0x0030
   UInt32 m_ISTAT_CLR;               // 0x0034
   UInt32 m_TWIF_RD_CFG;             // 0x0038
   UInt32 m_RF_SPI_CLK_SEL;          // 0x003C
   UInt32 m_SMCECR0;                 // 0x0040
   UInt32 m_SMCECR1;                 // 0x0044
   UInt32 m_SMCECR2;                 // 0x0048
   UInt32 m_SMCECR3;                 // 0x004C
   UInt32 m_SMCECR4;                 // 0x0050
   UInt32 m_SMCECR5;                 // 0x0054
   UInt32 m_SMCECR6;                 // 0x0058
   UInt32 m_SMCECR7;                 // 0x005C
   UInt32 m_SMCECR8;                 // 0x0060
   UInt32 m_SMCECR9;                 // 0x0064
   UInt32 m_SMCECR10;                // 0x0068
   UInt32 m_SMCECR11;                // 0x006C
   UInt32 m_SMCECR12;                // 0x0070
   UInt32 m_SMCECR13;                // 0x0074
   UInt32 m_SMCECR14;                // 0x0078
   UInt32 m_SMCECR15;                // 0x007C
   UInt32 m_SMCECR16;                // 0x0080
   UInt32 m_SMCECR17;                // 0x0084
   UInt32 m_SMCECR18;                // 0x0088
   UInt32 m_SMCECR19;                // 0x008C
   UInt32 m_SMCECR20;                // 0x0090
   UInt32 m_SMCECR21;                // 0x0094
   UInt32 m_SMCECR22;                // 0x0098
   UInt32 m_SMCECR23;                // 0x009C
   UInt32 m_SMCECR24;                // 0x00A0
   UInt32 m_SMCECR25;                // 0x00A4
   UInt32 m_SMCECR26;                // 0x00A8
   UInt32 m_SMCECR27;                // 0x00AC
   UInt32 m_SMCECR28;                // 0x00B0
   UInt32 m_SMCECR29;                // 0x00B4
   UInt32 m_SMCECR30;                // 0x00B8
   UInt32 m_SMCECR31;                // 0x00BC
   UInt32 m_SMCRD0;                  // 0x00C0
   UInt32 m_SMCRD1;                  // 0x00C4
   UInt32 m_SMCRD2;                  // 0x00C8
   UInt32 m_SMCRD3;                  // 0x00CC
   UInt32 m_SMCRD4;                  // 0x00D0
   UInt32 m_SMCRD5;                  // 0x00D4
   UInt32 m_SMCRD6;                  // 0x00D8
   UInt32 m_SMCRD7;                  // 0x00DC
   UInt32 m_SMCRD8;                  // 0x00E0
   UInt32 m_SMCRD9;                  // 0x00E4
   UInt32 m_SMCRD10;                 // 0x00E8
   UInt32 m_SMCRD11;                 // 0x00EC
   UInt32 m_SMCRD12;                 // 0x00F0
   UInt32 m_SMCRD13;                 // 0x00F4
   UInt32 m_SMCRD14;                 // 0x00F8
   UInt32 m_SMCRD15;                 // 0x00FC
   UInt32 m_SMCRD16;                 // 0x0100
   UInt32 m_SMCRD17;                 // 0x0104
   UInt32 m_SMCRD18;                 // 0x0108
   UInt32 m_SMCRD19;                 // 0x010C
   UInt32 m_SMCRD20;                 // 0x0110
   UInt32 m_SMCRD21;                 // 0x0114
   UInt32 m_SMCRD22;                 // 0x0118
   UInt32 m_SMCRD23;                 // 0x011C
   UInt32 m_SMCRD24;                 // 0x0120
   UInt32 m_SMCRD25;                 // 0x0124
   UInt32 m_SMCRD26;                 // 0x0128
   UInt32 m_SMCRD27;                 // 0x012C
   UInt32 m_SMCRD28;                 // 0x0130
   UInt32 m_SMCRD29;                 // 0x0134
   UInt32 m_SMCRD30;                 // 0x0138
   UInt32 m_SMCRD31;                 // 0x013C
   UInt32 m_SMCGPPC0;                // 0x0140
   UInt32 m_SMCGPPC2;                // 0x0144
   UInt32 m_SMCGPPC4;                // 0x0148
   UInt32 m_SMCGPPC6;                // 0x014C
   UInt32 m_SMCGPPC8;                // 0x0150
   UInt32 m_SMCGPPC10;               // 0x0154
   UInt32 m_SMCGPPC12;               // 0x0158
   UInt32 m_SMCGPPC14;               // 0x015C
   UInt32 RESERVED_180_15C[8];
   UInt32 m_TWIF_CFG_RD0;            // 0x0180
   UInt32 m_TWIF_CFG_RD1;            // 0x0184
   UInt32 m_TWIF_CFG_WR0;            // 0x0188
   UInt32 m_TWIF_CFG_WR1;            // 0x018C
   UInt32 m_TWIF_DATA_LOC;           // 0x0190
   UInt32 m_SMCECR_ENABLE;           // 0x0194
   UInt32 m_SMCECR_ENABLE_SET;       // 0x0198
   UInt32 m_SMCECR_ENABLE_CLR;       // 0x019C
   UInt32 RESERVED_1C0_19C[8];
   UInt32 m_SMCRD_EXT0;              // 0x01C0
   UInt32 m_SMCRD_EXT1;              // 0x01C4
   UInt32 m_SMCRD_EXT2;              // 0x01C8
   UInt32 m_SMCRD_EXT3;              // 0x01CC
   UInt32 m_SMCRD_EXT4;              // 0x01D0
   UInt32 m_SMCRD_EXT5;              // 0x01D4
   UInt32 m_SMCRD_EXT6;              // 0x01D8
   UInt32 m_SMCRD_EXT7;              // 0x01DC
   UInt32 m_SMCRD_EXT8;              // 0x01E0
   UInt32 m_SMCRD_EXT9;              // 0x01E4
   UInt32 m_SMCRD_EXT10;             // 0x01E8
   UInt32 m_SMCRD_EXT11;             // 0x01EC
   UInt32 m_SMCRD_EXT12;             // 0x01F0
   UInt32 m_SMCRD_EXT13;             // 0x01F4
   UInt32 m_SMCRD_EXT14;             // 0x01F8
   UInt32 m_SMCRD_EXT15;             // 0x01FC
   UInt32 m_SMCRD_EXT16;             // 0x0200
   UInt32 m_SMCRD_EXT17;             // 0x0204
   UInt32 m_SMCRD_EXT18;             // 0x0208
   UInt32 m_SMCRD_EXT19;             // 0x020C
   UInt32 m_SMCRD_EXT20;             // 0x0210
   UInt32 m_SMCRD_EXT21;             // 0x0214
   UInt32 m_SMCRD_EXT22;             // 0x0218
   UInt32 m_SMCRD_EXT23;             // 0x021C
   UInt32 m_SMCRD_EXT24;             // 0x0220
   UInt32 m_SMCRD_EXT25;             // 0x0224
   UInt32 m_SMCRD_EXT26;             // 0x0228
   UInt32 m_SMCRD_EXT27;             // 0x022C
   UInt32 m_SMCRD_EXT28;             // 0x0230
   UInt32 m_SMCRD_EXT29;             // 0x0234
   UInt32 m_SMCRD_EXT30;             // 0x0238
   UInt32 m_SMCRD_EXT31;             // 0x023C
   UInt32 m_POWER_HOLD;              // 0x0240
} BRCM_RFBB_SCHDLR_REGS;


#endif /* __BRCM_RDB_RFBB_SCHDLR_H__ */



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

#ifndef __BRCM_RDB_RFIC_SCHEDULER_H__
#define __BRCM_RDB_RFIC_SCHEDULER_H__

#define RFIC_SCHEDULER_SMCESR0_OFFSET                                     0x00000000
#define RFIC_SCHEDULER_SMCESR0_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCESR0_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCESR0_SMCESR0_SHIFT                           0
#define    RFIC_SCHEDULER_SMCESR0_SMCESR0_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCESR1_OFFSET                                     0x00000004
#define RFIC_SCHEDULER_SMCESR1_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCESR1_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCESR1_SMCESR1_SHIFT                           0
#define    RFIC_SCHEDULER_SMCESR1_SMCESR1_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_CMDBUF_ADDR_OFFSET                                 0x00000008
#define RFIC_SCHEDULER_CMDBUF_ADDR_TYPE                                   UInt32
#define RFIC_SCHEDULER_CMDBUF_ADDR_RESERVED_MASK                          0xFFFFFF00
#define    RFIC_SCHEDULER_CMDBUF_ADDR_CMDBUF_ADDR_SHIFT                   0
#define    RFIC_SCHEDULER_CMDBUF_ADDR_CMDBUF_ADDR_MASK                    0x000000FF

#define RFIC_SCHEDULER_CMDBUF_RD_OFFSET                                   0x00000010
#define RFIC_SCHEDULER_CMDBUF_RD_TYPE                                     UInt32
#define RFIC_SCHEDULER_CMDBUF_RD_RESERVED_MASK                            0x00000000
#define    RFIC_SCHEDULER_CMDBUF_RD_CMDBUF_RD_SHIFT                       0
#define    RFIC_SCHEDULER_CMDBUF_RD_CMDBUF_RD_MASK                        0xFFFFFFFF

#define RFIC_SCHEDULER_CMDBUF_WR_OFFSET                                   0x00000014
#define RFIC_SCHEDULER_CMDBUF_WR_TYPE                                     UInt32
#define RFIC_SCHEDULER_CMDBUF_WR_RESERVED_MASK                            0x00000000
#define    RFIC_SCHEDULER_CMDBUF_WR_CMDBUF_WR_SHIFT                       0
#define    RFIC_SCHEDULER_CMDBUF_WR_CMDBUF_WR_MASK                        0xFFFFFFFF

#define RFIC_SCHEDULER_CMDBUF_SEL_OFFSET                                  0x00000018
#define RFIC_SCHEDULER_CMDBUF_SEL_TYPE                                    UInt32
#define RFIC_SCHEDULER_CMDBUF_SEL_RESERVED_MASK                           0xFFFFFFEE
#define    RFIC_SCHEDULER_CMDBUF_SEL_CMDBUF_MODE_SHIFT                    4
#define    RFIC_SCHEDULER_CMDBUF_SEL_CMDBUF_MODE_MASK                     0x00000010
#define    RFIC_SCHEDULER_CMDBUF_SEL_CMDBUF_SEL_SHIFT                     0
#define    RFIC_SCHEDULER_CMDBUF_SEL_CMDBUF_SEL_MASK                      0x00000001

#define RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_OFFSET                            0x0000001C
#define RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_TYPE                              UInt32
#define RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_RESERVED_MASK                     0xFFFFEE00
#define    RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_SPI_WR_BIT_ORDER_SHIFT         12
#define    RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_SPI_WR_BIT_ORDER_MASK          0x00001000
#define    RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_SPI_RD_BIT_ORDER_SHIFT         8
#define    RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_SPI_RD_BIT_ORDER_MASK          0x00000100
#define    RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_SPI_CLK_FREQ_CNT_SHIFT         0
#define    RFIC_SCHEDULER_SPI_CLK_FREQ_CNT_SPI_CLK_FREQ_CNT_MASK          0x000000FF

#define RFIC_SCHEDULER_CMD_GAP_OFFSET                                     0x00000020
#define RFIC_SCHEDULER_CMD_GAP_TYPE                                       UInt32
#define RFIC_SCHEDULER_CMD_GAP_RESERVED_MASK                              0xFF000000
#define    RFIC_SCHEDULER_CMD_GAP_CMD_DLY_SPI_SHIFT                       16
#define    RFIC_SCHEDULER_CMD_GAP_CMD_DLY_SPI_MASK                        0x00FF0000
#define    RFIC_SCHEDULER_CMD_GAP_CMD_DLY_GP_SHIFT                        8
#define    RFIC_SCHEDULER_CMD_GAP_CMD_DLY_GP_MASK                         0x0000FF00
#define    RFIC_SCHEDULER_CMD_GAP_CMD_DLY_TWIF_SHIFT                      0
#define    RFIC_SCHEDULER_CMD_GAP_CMD_DLY_TWIF_MASK                       0x000000FF

#define RFIC_SCHEDULER_RFIC_SCHEDULER_EN_OFFSET                           0x00000024
#define RFIC_SCHEDULER_RFIC_SCHEDULER_EN_TYPE                             UInt32
#define RFIC_SCHEDULER_RFIC_SCHEDULER_EN_RESERVED_MASK                    0xFFFFFFFE
#define    RFIC_SCHEDULER_RFIC_SCHEDULER_EN_RFIC_SCHEDULER_EN_SHIFT       0
#define    RFIC_SCHEDULER_RFIC_SCHEDULER_EN_RFIC_SCHEDULER_EN_MASK        0x00000001

#define RFIC_SCHEDULER_MT_SHADOW_EN_OFFSET                                0x00000028
#define RFIC_SCHEDULER_MT_SHADOW_EN_TYPE                                  UInt32
#define RFIC_SCHEDULER_MT_SHADOW_EN_RESERVED_MASK                         0xEEEEFEEE
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_SYNC_SHIFT               28
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_SYNC_MASK                0x10000000
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_FREEZE_SPI_SHIFT         24
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_FREEZE_SPI_MASK          0x01000000
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_FREEZE_GP_SHIFT          20
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_FREEZE_GP_MASK           0x00100000
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_FREEZE_TWIF_SHIFT        16
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_FREEZE_TWIF_MASK         0x00010000
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_EN_SPI_SHIFT             8
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_EN_SPI_MASK              0x00000100
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_EN_GP_SHIFT              4
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_EN_GP_MASK               0x00000010
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_EN_TWIF_SHIFT            0
#define    RFIC_SCHEDULER_MT_SHADOW_EN_MT_SHADOW_EN_TWIF_MASK             0x00000001

#define RFIC_SCHEDULER_IMR_OFFSET                                         0x0000002C
#define RFIC_SCHEDULER_IMR_TYPE                                           UInt32
#define RFIC_SCHEDULER_IMR_RESERVED_MASK                                  0x00000000
#define    RFIC_SCHEDULER_IMR_IMR_SHIFT                                   0
#define    RFIC_SCHEDULER_IMR_IMR_MASK                                    0xFFFFFFFF

#define RFIC_SCHEDULER_ISTAT_OFFSET                                       0x00000030
#define RFIC_SCHEDULER_ISTAT_TYPE                                         UInt32
#define RFIC_SCHEDULER_ISTAT_RESERVED_MASK                                0x00000000
#define    RFIC_SCHEDULER_ISTAT_ISTAT_SHIFT                               0
#define    RFIC_SCHEDULER_ISTAT_ISTAT_MASK                                0xFFFFFFFF

#define RFIC_SCHEDULER_ISTAT_CLR_OFFSET                                   0x00000034
#define RFIC_SCHEDULER_ISTAT_CLR_TYPE                                     UInt32
#define RFIC_SCHEDULER_ISTAT_CLR_RESERVED_MASK                            0x00000000
#define    RFIC_SCHEDULER_ISTAT_CLR_ISTAT_CLR_SHIFT                       0
#define    RFIC_SCHEDULER_ISTAT_CLR_ISTAT_CLR_MASK                        0xFFFFFFFF

#define RFIC_SCHEDULER_TWIF_RD_CFG_OFFSET                                 0x00000038
#define RFIC_SCHEDULER_TWIF_RD_CFG_TYPE                                   UInt32
#define RFIC_SCHEDULER_TWIF_RD_CFG_RESERVED_MASK                          0xFFFFFEE0
#define    RFIC_SCHEDULER_TWIF_RD_CFG_TWIF_RD_POL_SHIFT                   8
#define    RFIC_SCHEDULER_TWIF_RD_CFG_TWIF_RD_POL_MASK                    0x00000100
#define    RFIC_SCHEDULER_TWIF_RD_CFG_TWIF_RD_BIT_SHIFT                   0
#define    RFIC_SCHEDULER_TWIF_RD_CFG_TWIF_RD_BIT_MASK                    0x0000001F

#define RFIC_SCHEDULER_SMCESR_SUMMARY_OFFSET                              0x0000003C
#define RFIC_SCHEDULER_SMCESR_SUMMARY_TYPE                                UInt32
#define RFIC_SCHEDULER_SMCESR_SUMMARY_RESERVED_MASK                       0x00000000
#define    RFIC_SCHEDULER_SMCESR_SUMMARY_SMCESR_SUMMARY_SHIFT             0
#define    RFIC_SCHEDULER_SMCESR_SUMMARY_SMCESR_SUMMARY_MASK              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR0_OFFSET                                     0x00000040
#define RFIC_SCHEDULER_SMCECR0_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR0_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR0_SMCECR0_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR0_SMCECR0_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR1_OFFSET                                     0x00000044
#define RFIC_SCHEDULER_SMCECR1_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR1_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR1_SMCECR1_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR1_SMCECR1_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR2_OFFSET                                     0x00000048
#define RFIC_SCHEDULER_SMCECR2_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR2_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR2_SMCECR2_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR2_SMCECR2_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR3_OFFSET                                     0x0000004C
#define RFIC_SCHEDULER_SMCECR3_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR3_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR3_SMCECR3_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR3_SMCECR3_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR4_OFFSET                                     0x00000050
#define RFIC_SCHEDULER_SMCECR4_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR4_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR4_SMCECR4_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR4_SMCECR4_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR5_OFFSET                                     0x00000054
#define RFIC_SCHEDULER_SMCECR5_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR5_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR5_SMCECR5_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR5_SMCECR5_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR6_OFFSET                                     0x00000058
#define RFIC_SCHEDULER_SMCECR6_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR6_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR6_SMCECR6_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR6_SMCECR6_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR7_OFFSET                                     0x0000005C
#define RFIC_SCHEDULER_SMCECR7_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR7_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR7_SMCECR7_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR7_SMCECR7_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR8_OFFSET                                     0x00000060
#define RFIC_SCHEDULER_SMCECR8_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR8_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR8_SMCECR8_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR8_SMCECR8_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR9_OFFSET                                     0x00000064
#define RFIC_SCHEDULER_SMCECR9_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCECR9_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCECR9_SMCECR9_SHIFT                           0
#define    RFIC_SCHEDULER_SMCECR9_SMCECR9_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR10_OFFSET                                    0x00000068
#define RFIC_SCHEDULER_SMCECR10_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR10_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR10_SMCECR10_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR10_SMCECR10_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR11_OFFSET                                    0x0000006C
#define RFIC_SCHEDULER_SMCECR11_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR11_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR11_SMCECR11_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR11_SMCECR11_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR12_OFFSET                                    0x00000070
#define RFIC_SCHEDULER_SMCECR12_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR12_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR12_SMCECR12_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR12_SMCECR12_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR13_OFFSET                                    0x00000074
#define RFIC_SCHEDULER_SMCECR13_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR13_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR13_SMCECR13_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR13_SMCECR13_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR14_OFFSET                                    0x00000078
#define RFIC_SCHEDULER_SMCECR14_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR14_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR14_SMCECR14_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR14_SMCECR14_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR15_OFFSET                                    0x0000007C
#define RFIC_SCHEDULER_SMCECR15_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR15_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR15_SMCECR15_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR15_SMCECR15_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR16_OFFSET                                    0x00000080
#define RFIC_SCHEDULER_SMCECR16_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR16_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR16_SMCECR16_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR16_SMCECR16_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR17_OFFSET                                    0x00000084
#define RFIC_SCHEDULER_SMCECR17_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR17_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR17_SMCECR17_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR17_SMCECR17_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR18_OFFSET                                    0x00000088
#define RFIC_SCHEDULER_SMCECR18_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR18_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR18_SMCECR18_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR18_SMCECR18_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR19_OFFSET                                    0x0000008C
#define RFIC_SCHEDULER_SMCECR19_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR19_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR19_SMCECR19_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR19_SMCECR19_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR20_OFFSET                                    0x00000090
#define RFIC_SCHEDULER_SMCECR20_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR20_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR20_SMCECR20_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR20_SMCECR20_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR21_OFFSET                                    0x00000094
#define RFIC_SCHEDULER_SMCECR21_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR21_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR21_SMCECR21_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR21_SMCECR21_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR22_OFFSET                                    0x00000098
#define RFIC_SCHEDULER_SMCECR22_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR22_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR22_SMCECR22_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR22_SMCECR22_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR23_OFFSET                                    0x0000009C
#define RFIC_SCHEDULER_SMCECR23_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR23_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR23_SMCECR23_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR23_SMCECR23_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR24_OFFSET                                    0x000000A0
#define RFIC_SCHEDULER_SMCECR24_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR24_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR24_SMCECR24_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR24_SMCECR24_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR25_OFFSET                                    0x000000A4
#define RFIC_SCHEDULER_SMCECR25_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR25_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR25_SMCECR25_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR25_SMCECR25_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR26_OFFSET                                    0x000000A8
#define RFIC_SCHEDULER_SMCECR26_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR26_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR26_SMCECR26_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR26_SMCECR26_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR27_OFFSET                                    0x000000AC
#define RFIC_SCHEDULER_SMCECR27_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR27_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR27_SMCECR27_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR27_SMCECR27_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR28_OFFSET                                    0x000000B0
#define RFIC_SCHEDULER_SMCECR28_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR28_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR28_SMCECR28_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR28_SMCECR28_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR29_OFFSET                                    0x000000B4
#define RFIC_SCHEDULER_SMCECR29_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR29_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR29_SMCECR29_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR29_SMCECR29_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR30_OFFSET                                    0x000000B8
#define RFIC_SCHEDULER_SMCECR30_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR30_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR30_SMCECR30_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR30_SMCECR30_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCECR31_OFFSET                                    0x000000BC
#define RFIC_SCHEDULER_SMCECR31_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCECR31_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCECR31_SMCECR31_SHIFT                         0
#define    RFIC_SCHEDULER_SMCECR31_SMCECR31_MASK                          0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD0_OFFSET                                      0x000000C0
#define RFIC_SCHEDULER_SMCRD0_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD0_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD0_SMCRD0_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD0_SMCRD0_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD1_OFFSET                                      0x000000C4
#define RFIC_SCHEDULER_SMCRD1_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD1_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD1_SMCRD1_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD1_SMCRD1_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD2_OFFSET                                      0x000000C8
#define RFIC_SCHEDULER_SMCRD2_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD2_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD2_SMCRD2_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD2_SMCRD2_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD3_OFFSET                                      0x000000CC
#define RFIC_SCHEDULER_SMCRD3_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD3_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD3_SMCRD3_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD3_SMCRD3_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD4_OFFSET                                      0x000000D0
#define RFIC_SCHEDULER_SMCRD4_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD4_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD4_SMCRD4_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD4_SMCRD4_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD5_OFFSET                                      0x000000D4
#define RFIC_SCHEDULER_SMCRD5_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD5_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD5_SMCRD5_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD5_SMCRD5_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD6_OFFSET                                      0x000000D8
#define RFIC_SCHEDULER_SMCRD6_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD6_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD6_SMCRD6_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD6_SMCRD6_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD7_OFFSET                                      0x000000DC
#define RFIC_SCHEDULER_SMCRD7_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD7_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD7_SMCRD7_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD7_SMCRD7_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD8_OFFSET                                      0x000000E0
#define RFIC_SCHEDULER_SMCRD8_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD8_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD8_SMCRD8_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD8_SMCRD8_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD9_OFFSET                                      0x000000E4
#define RFIC_SCHEDULER_SMCRD9_TYPE                                        UInt32
#define RFIC_SCHEDULER_SMCRD9_RESERVED_MASK                               0x00000000
#define    RFIC_SCHEDULER_SMCRD9_SMCRD9_SHIFT                             0
#define    RFIC_SCHEDULER_SMCRD9_SMCRD9_MASK                              0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD10_OFFSET                                     0x000000E8
#define RFIC_SCHEDULER_SMCRD10_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD10_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD10_SMCRD10_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD10_SMCRD10_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD11_OFFSET                                     0x000000EC
#define RFIC_SCHEDULER_SMCRD11_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD11_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD11_SMCRD11_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD11_SMCRD11_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD12_OFFSET                                     0x000000F0
#define RFIC_SCHEDULER_SMCRD12_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD12_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD12_SMCRD12_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD12_SMCRD12_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD13_OFFSET                                     0x000000F4
#define RFIC_SCHEDULER_SMCRD13_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD13_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD13_SMCRD13_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD13_SMCRD13_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD14_OFFSET                                     0x000000F8
#define RFIC_SCHEDULER_SMCRD14_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD14_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD14_SMCRD14_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD14_SMCRD14_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD15_OFFSET                                     0x000000FC
#define RFIC_SCHEDULER_SMCRD15_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD15_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD15_SMCRD15_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD15_SMCRD15_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD16_OFFSET                                     0x00000100
#define RFIC_SCHEDULER_SMCRD16_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD16_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD16_SMCRD16_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD16_SMCRD16_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD17_OFFSET                                     0x00000104
#define RFIC_SCHEDULER_SMCRD17_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD17_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD17_SMCRD17_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD17_SMCRD17_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD18_OFFSET                                     0x00000108
#define RFIC_SCHEDULER_SMCRD18_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD18_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD18_SMCRD18_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD18_SMCRD18_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD19_OFFSET                                     0x0000010C
#define RFIC_SCHEDULER_SMCRD19_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD19_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD19_SMCRD19_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD19_SMCRD19_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD20_OFFSET                                     0x00000110
#define RFIC_SCHEDULER_SMCRD20_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD20_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD20_SMCRD20_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD20_SMCRD20_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD21_OFFSET                                     0x00000114
#define RFIC_SCHEDULER_SMCRD21_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD21_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD21_SMCRD21_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD21_SMCRD21_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD22_OFFSET                                     0x00000118
#define RFIC_SCHEDULER_SMCRD22_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD22_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD22_SMCRD22_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD22_SMCRD22_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD23_OFFSET                                     0x0000011C
#define RFIC_SCHEDULER_SMCRD23_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD23_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD23_SMCRD23_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD23_SMCRD23_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD24_OFFSET                                     0x00000120
#define RFIC_SCHEDULER_SMCRD24_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD24_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD24_SMCRD24_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD24_SMCRD24_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD25_OFFSET                                     0x00000124
#define RFIC_SCHEDULER_SMCRD25_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD25_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD25_SMCRD25_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD25_SMCRD25_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD26_OFFSET                                     0x00000128
#define RFIC_SCHEDULER_SMCRD26_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD26_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD26_SMCRD26_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD26_SMCRD26_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD27_OFFSET                                     0x0000012C
#define RFIC_SCHEDULER_SMCRD27_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD27_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD27_SMCRD27_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD27_SMCRD27_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD28_OFFSET                                     0x00000130
#define RFIC_SCHEDULER_SMCRD28_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD28_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD28_SMCRD28_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD28_SMCRD28_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD29_OFFSET                                     0x00000134
#define RFIC_SCHEDULER_SMCRD29_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD29_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD29_SMCRD29_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD29_SMCRD29_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD30_OFFSET                                     0x00000138
#define RFIC_SCHEDULER_SMCRD30_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD30_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD30_SMCRD30_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD30_SMCRD30_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCRD31_OFFSET                                     0x0000013C
#define RFIC_SCHEDULER_SMCRD31_TYPE                                       UInt32
#define RFIC_SCHEDULER_SMCRD31_RESERVED_MASK                              0x00000000
#define    RFIC_SCHEDULER_SMCRD31_SMCRD31_SHIFT                           0
#define    RFIC_SCHEDULER_SMCRD31_SMCRD31_MASK                            0xFFFFFFFF

#define RFIC_SCHEDULER_SMCGPPC0_OFFSET                                    0x00000140
#define RFIC_SCHEDULER_SMCGPPC0_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCGPPC0_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC1_POL_SHIFT                     31
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC1_POL_MASK                      0x80000000
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC1_PW_SHIFT                      16
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC1_PW_MASK                       0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC0_POL_SHIFT                     15
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC0_POL_MASK                      0x00008000
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC0_PW_SHIFT                      0
#define    RFIC_SCHEDULER_SMCGPPC0_SMCGPPC0_PW_MASK                       0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC2_OFFSET                                    0x00000144
#define RFIC_SCHEDULER_SMCGPPC2_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCGPPC2_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC3_POL_SHIFT                     31
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC3_POL_MASK                      0x80000000
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC3_PW_SHIFT                      16
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC3_PW_MASK                       0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC2_POL_SHIFT                     15
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC2_POL_MASK                      0x00008000
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC2_PW_SHIFT                      0
#define    RFIC_SCHEDULER_SMCGPPC2_SMCGPPC2_PW_MASK                       0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC4_OFFSET                                    0x00000148
#define RFIC_SCHEDULER_SMCGPPC4_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCGPPC4_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC5_POL_SHIFT                     31
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC5_POL_MASK                      0x80000000
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC5_PW_SHIFT                      16
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC5_PW_MASK                       0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC4_POL_SHIFT                     15
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC4_POL_MASK                      0x00008000
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC4_PW_SHIFT                      0
#define    RFIC_SCHEDULER_SMCGPPC4_SMCGPPC4_PW_MASK                       0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC6_OFFSET                                    0x0000014C
#define RFIC_SCHEDULER_SMCGPPC6_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCGPPC6_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC7_POL_SHIFT                     31
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC7_POL_MASK                      0x80000000
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC7_PW_SHIFT                      16
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC7_PW_MASK                       0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC6_POL_SHIFT                     15
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC6_POL_MASK                      0x00008000
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC6_PW_SHIFT                      0
#define    RFIC_SCHEDULER_SMCGPPC6_SMCGPPC6_PW_MASK                       0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC8_OFFSET                                    0x00000150
#define RFIC_SCHEDULER_SMCGPPC8_TYPE                                      UInt32
#define RFIC_SCHEDULER_SMCGPPC8_RESERVED_MASK                             0x00000000
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC9_POL_SHIFT                     31
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC9_POL_MASK                      0x80000000
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC9_PW_SHIFT                      16
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC9_PW_MASK                       0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC8_POL_SHIFT                     15
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC8_POL_MASK                      0x00008000
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC8_PW_SHIFT                      0
#define    RFIC_SCHEDULER_SMCGPPC8_SMCGPPC8_PW_MASK                       0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC10_OFFSET                                   0x00000154
#define RFIC_SCHEDULER_SMCGPPC10_TYPE                                     UInt32
#define RFIC_SCHEDULER_SMCGPPC10_RESERVED_MASK                            0x00000000
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC11_POL_SHIFT                   31
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC11_POL_MASK                    0x80000000
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC11_PW_SHIFT                    16
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC11_PW_MASK                     0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC10_POL_SHIFT                   15
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC10_POL_MASK                    0x00008000
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC10_PW_SHIFT                    0
#define    RFIC_SCHEDULER_SMCGPPC10_SMCGPPC10_PW_MASK                     0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC12_OFFSET                                   0x00000158
#define RFIC_SCHEDULER_SMCGPPC12_TYPE                                     UInt32
#define RFIC_SCHEDULER_SMCGPPC12_RESERVED_MASK                            0x00000000
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC13_POL_SHIFT                   31
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC13_POL_MASK                    0x80000000
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC13_PW_SHIFT                    16
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC13_PW_MASK                     0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC12_POL_SHIFT                   15
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC12_POL_MASK                    0x00008000
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC12_PW_SHIFT                    0
#define    RFIC_SCHEDULER_SMCGPPC12_SMCGPPC12_PW_MASK                     0x00007FFF

#define RFIC_SCHEDULER_SMCGPPC14_OFFSET                                   0x0000015C
#define RFIC_SCHEDULER_SMCGPPC14_TYPE                                     UInt32
#define RFIC_SCHEDULER_SMCGPPC14_RESERVED_MASK                            0x00000000
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC15_POL_SHIFT                   31
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC15_POL_MASK                    0x80000000
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC15_PW_SHIFT                    16
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC15_PW_MASK                     0x7FFF0000
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC14_POL_SHIFT                   15
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC14_POL_MASK                    0x00008000
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC14_PW_SHIFT                    0
#define    RFIC_SCHEDULER_SMCGPPC14_SMCGPPC14_PW_MASK                     0x00007FFF

#define RFIC_SCHEDULER_GPIO_STATUS_OFFSET                                 0x00000160
#define RFIC_SCHEDULER_GPIO_STATUS_TYPE                                   UInt32
#define RFIC_SCHEDULER_GPIO_STATUS_RESERVED_MASK                          0xFFFF0000
#define    RFIC_SCHEDULER_GPIO_STATUS_GPIO_STATUS_SHIFT                   0
#define    RFIC_SCHEDULER_GPIO_STATUS_GPIO_STATUS_MASK                    0x0000FFFF

#define RFIC_SCHEDULER_SMCRD_EXT0_OFFSET                                  0x00000180
#define RFIC_SCHEDULER_SMCRD_EXT0_TYPE                                    UInt32
#define RFIC_SCHEDULER_SMCRD_EXT0_RESERVED_MASK                           0x00000000
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT7_SHIFT                     28
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT7_MASK                      0xF0000000
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT6_SHIFT                     24
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT6_MASK                      0x0F000000
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT5_SHIFT                     20
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT5_MASK                      0x00F00000
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT4_SHIFT                     16
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT4_MASK                      0x000F0000
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT3_SHIFT                     12
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT3_MASK                      0x0000F000
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT2_SHIFT                     8
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT2_MASK                      0x00000F00
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT1_SHIFT                     4
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT1_MASK                      0x000000F0
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT0_SHIFT                     0
#define    RFIC_SCHEDULER_SMCRD_EXT0_SMCRD_EXT0_MASK                      0x0000000F

#define RFIC_SCHEDULER_SMCRD_EXT1_OFFSET                                  0x00000184
#define RFIC_SCHEDULER_SMCRD_EXT1_TYPE                                    UInt32
#define RFIC_SCHEDULER_SMCRD_EXT1_RESERVED_MASK                           0x00000000
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT15_SHIFT                    28
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT15_MASK                     0xF0000000
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT14_SHIFT                    24
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT14_MASK                     0x0F000000
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT13_SHIFT                    20
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT13_MASK                     0x00F00000
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT12_SHIFT                    16
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT12_MASK                     0x000F0000
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT11_SHIFT                    12
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT11_MASK                     0x0000F000
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT10_SHIFT                    8
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT10_MASK                     0x00000F00
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT9_SHIFT                     4
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT9_MASK                      0x000000F0
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT8_SHIFT                     0
#define    RFIC_SCHEDULER_SMCRD_EXT1_SMCRD_EXT8_MASK                      0x0000000F

#define RFIC_SCHEDULER_SMCRD_EXT2_OFFSET                                  0x00000188
#define RFIC_SCHEDULER_SMCRD_EXT2_TYPE                                    UInt32
#define RFIC_SCHEDULER_SMCRD_EXT2_RESERVED_MASK                           0x00000000
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT23_SHIFT                    28
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT23_MASK                     0xF0000000
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT22_SHIFT                    24
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT22_MASK                     0x0F000000
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT21_SHIFT                    20
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT21_MASK                     0x00F00000
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT20_SHIFT                    16
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT20_MASK                     0x000F0000
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT19_SHIFT                    12
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT19_MASK                     0x0000F000
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT18_SHIFT                    8
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT18_MASK                     0x00000F00
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT17_SHIFT                    4
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT17_MASK                     0x000000F0
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT16_SHIFT                    0
#define    RFIC_SCHEDULER_SMCRD_EXT2_SMCRD_EXT16_MASK                     0x0000000F

#define RFIC_SCHEDULER_SMCRD_EXT3_OFFSET                                  0x0000018C
#define RFIC_SCHEDULER_SMCRD_EXT3_TYPE                                    UInt32
#define RFIC_SCHEDULER_SMCRD_EXT3_RESERVED_MASK                           0x00000000
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT31_SHIFT                    28
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT31_MASK                     0xF0000000
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT30_SHIFT                    24
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT30_MASK                     0x0F000000
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT29_SHIFT                    20
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT29_MASK                     0x00F00000
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT28_SHIFT                    16
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT28_MASK                     0x000F0000
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT27_SHIFT                    12
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT27_MASK                     0x0000F000
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT26_SHIFT                    8
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT26_MASK                     0x00000F00
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT25_SHIFT                    4
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT25_MASK                     0x000000F0
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT24_SHIFT                    0
#define    RFIC_SCHEDULER_SMCRD_EXT3_SMCRD_EXT24_MASK                     0x0000000F

#endif /* __BRCM_RDB_RFIC_SCHEDULER_H__ */



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

#ifndef __BRCM_RDB_DSP_INTC_H__
#define __BRCM_RDB_DSP_INTC_H__

#define DSP_INTC_SPAD0_R_OFFSET                                           0x00000000
#define DSP_INTC_SPAD0_R_TYPE                                             UInt16
#define DSP_INTC_SPAD0_R_RESERVED_MASK                                    0x00000000
#define    DSP_INTC_SPAD0_R_SCRATCH_PAD0_REG_15TO0_SHIFT                  0
#define    DSP_INTC_SPAD0_R_SCRATCH_PAD0_REG_15TO0_MASK                   0x0000FFFF

#define DSP_INTC_SCR_R_OFFSET                                             0x00000002
#define DSP_INTC_SCR_R_TYPE                                               UInt16
#define DSP_INTC_SCR_R_RESERVED_MASK                                      0x00000000
#define    DSP_INTC_SCR_R_OTOCFINT_SHIFT                                  15
#define    DSP_INTC_SCR_R_OTOCFINT_MASK                                   0x00008000
#define    DSP_INTC_SCR_R_OTOCINT_SHIFT                                   14
#define    DSP_INTC_SCR_R_OTOCINT_MASK                                    0x00004000
#define    DSP_INTC_SCR_R_EN_DEROTATOR_SHIFT                              13
#define    DSP_INTC_SCR_R_EN_DEROTATOR_MASK                               0x00002000
#define    DSP_INTC_SCR_R_SEL_DIG_RF_TX_SHIFT                             12
#define    DSP_INTC_SCR_R_SEL_DIG_RF_TX_MASK                              0x00001000
#define    DSP_INTC_SCR_R_OTOAFINT_SHIFT                                  11
#define    DSP_INTC_SCR_R_OTOAFINT_MASK                                   0x00000800
#define    DSP_INTC_SCR_R_EN8PSK_MANU_SHIFT                               10
#define    DSP_INTC_SCR_R_EN8PSK_MANU_MASK                                0x00000400
#define    DSP_INTC_SCR_R_RESERVED_9_SHIFT                                9
#define    DSP_INTC_SCR_R_RESERVED_9_MASK                                 0x00000200
#define    DSP_INTC_SCR_R_OTOAINT_SHIFT                                   8
#define    DSP_INTC_SCR_R_OTOAINT_MASK                                    0x00000100
#define    DSP_INTC_SCR_R_SEL_DIGRF2_SHIFT                                7
#define    DSP_INTC_SCR_R_SEL_DIGRF2_MASK                                 0x00000080
#define    DSP_INTC_SCR_R_RESERVED_6_SHIFT                                6
#define    DSP_INTC_SCR_R_RESERVED_6_MASK                                 0x00000040
#define    DSP_INTC_SCR_R_A5_EN_SHIFT                                     5
#define    DSP_INTC_SCR_R_A5_EN_MASK                                      0x00000020
#define    DSP_INTC_SCR_R_RESERVED_4TO1_SHIFT                             1
#define    DSP_INTC_SCR_R_RESERVED_4TO1_MASK                              0x0000001E
#define    DSP_INTC_SCR_R_SEL_DIG_RF_RX_SHIFT                             0
#define    DSP_INTC_SCR_R_SEL_DIG_RF_RX_MASK                              0x00000001

#define DSP_INTC_SAMPLE_BUF_STA_R_OFFSET                                  0x00000004
#define DSP_INTC_SAMPLE_BUF_STA_R_TYPE                                    UInt16
#define DSP_INTC_SAMPLE_BUF_STA_R_RESERVED_MASK                           0x00000000
#define    DSP_INTC_SAMPLE_BUF_STA_R_RESERVED_15TO12_SHIFT                12
#define    DSP_INTC_SAMPLE_BUF_STA_R_RESERVED_15TO12_MASK                 0x0000F000
#define    DSP_INTC_SAMPLE_BUF_STA_R_SWTBUF_SHIFT                         11
#define    DSP_INTC_SAMPLE_BUF_STA_R_SWTBUF_MASK                          0x00000800
#define    DSP_INTC_SAMPLE_BUF_STA_R_RESERVED_10TO0_SHIFT                 0
#define    DSP_INTC_SAMPLE_BUF_STA_R_RESERVED_10TO0_MASK                  0x000007FF

#define DSP_INTC_ISR_R_OFFSET                                             0x00000006
#define DSP_INTC_ISR_R_TYPE                                               UInt16
#define DSP_INTC_ISR_R_RESERVED_MASK                                      0x00000000
#define    DSP_INTC_ISR_R_ADDR_PLACE_HOLDER_SHIFT                         0
#define    DSP_INTC_ISR_R_ADDR_PLACE_HOLDER_MASK                          0x0000FFFF

#define DSP_INTC_IMR_R_OFFSET                                             0x00000008
#define DSP_INTC_IMR_R_TYPE                                               UInt16
#define DSP_INTC_IMR_R_RESERVED_MASK                                      0x00000000
#define    DSP_INTC_IMR_R_ADDR_PLACE_HOLDER_SHIFT                         0
#define    DSP_INTC_IMR_R_ADDR_PLACE_HOLDER_MASK                          0x0000FFFF

#define DSP_INTC_IPR_R_OFFSET                                             0x0000000A
#define DSP_INTC_IPR_R_TYPE                                               UInt16
#define DSP_INTC_IPR_R_RESERVED_MASK                                      0x00000000
#define    DSP_INTC_IPR_R_ADDR_PLACE_HOLDER_SHIFT                         0
#define    DSP_INTC_IPR_R_ADDR_PLACE_HOLDER_MASK                          0x0000FFFF

#define DSP_INTC_IRR_R_OFFSET                                             0x0000000C
#define DSP_INTC_IRR_R_TYPE                                               UInt16
#define DSP_INTC_IRR_R_RESERVED_MASK                                      0x00000000
#define    DSP_INTC_IRR_R_ADDR_PLACE_HOLDER_SHIFT                         0
#define    DSP_INTC_IRR_R_ADDR_PLACE_HOLDER_MASK                          0x0000FFFF

#define DSP_INTC_PIN_STA_CTL_R_OFFSET                                     0x0000000E
#define DSP_INTC_PIN_STA_CTL_R_TYPE                                       UInt16
#define DSP_INTC_PIN_STA_CTL_R_RESERVED_MASK                              0x00000000
#define    DSP_INTC_PIN_STA_CTL_R_RESERVED_15TO14_SHIFT                   14
#define    DSP_INTC_PIN_STA_CTL_R_RESERVED_15TO14_MASK                    0x0000C000
#define    DSP_INTC_PIN_STA_CTL_R_ARM_PLLPDEN_SHIFT                       13
#define    DSP_INTC_PIN_STA_CTL_R_ARM_PLLPDEN_MASK                        0x00002000
#define    DSP_INTC_PIN_STA_CTL_R_RESERVED_12_SHIFT                       12
#define    DSP_INTC_PIN_STA_CTL_R_RESERVED_12_MASK                        0x00001000
#define    DSP_INTC_PIN_STA_CTL_R_OAKRST_SHIFT                            11
#define    DSP_INTC_PIN_STA_CTL_R_OAKRST_MASK                             0x00000800
#define    DSP_INTC_PIN_STA_CTL_R_EXTID_2TO0_SHIFT                        8
#define    DSP_INTC_PIN_STA_CTL_R_EXTID_2TO0_MASK                         0x00000700
#define    DSP_INTC_PIN_STA_CTL_R_RESERVED_7TO0_SHIFT                     0
#define    DSP_INTC_PIN_STA_CTL_R_RESERVED_7TO0_MASK                      0x000000FF

#define DSP_INTC_TEST_EN_R_OFFSET                                         0x0000001C
#define DSP_INTC_TEST_EN_R_TYPE                                           UInt16
#define DSP_INTC_TEST_EN_R_RESERVED_MASK                                  0x00000000
#define    DSP_INTC_TEST_EN_R_ADDR_PLACE_HOLDER_SHIFT                     0
#define    DSP_INTC_TEST_EN_R_ADDR_PLACE_HOLDER_MASK                      0x0000FFFF

#define DSP_INTC_TEST_INT_R_OFFSET                                        0x0000001E
#define DSP_INTC_TEST_INT_R_TYPE                                          UInt16
#define DSP_INTC_TEST_INT_R_RESERVED_MASK                                 0x00000000
#define    DSP_INTC_TEST_INT_R_ADDR_PLACE_HOLDER_SHIFT                    0
#define    DSP_INTC_TEST_INT_R_ADDR_PLACE_HOLDER_MASK                     0x0000FFFF

#define DSP_INTC_ISR_32_R_OFFSET                                          0x00000020
#define DSP_INTC_ISR_32_R_TYPE                                            UInt32
#define DSP_INTC_ISR_32_R_RESERVED_MASK                                   0x00000000
#define    DSP_INTC_ISR_32_R_RESERVED_31TO28_SHIFT                        28
#define    DSP_INTC_ISR_32_R_RESERVED_31TO28_MASK                         0xF0000000
#define    DSP_INTC_ISR_32_R_SPARE2DSP_INT_7TO0_SHIFT                     20
#define    DSP_INTC_ISR_32_R_SPARE2DSP_INT_7TO0_MASK                      0x0FF00000
#define    DSP_INTC_ISR_32_R_CAPH_ERR_INT_SHIFT                           19
#define    DSP_INTC_ISR_32_R_CAPH_ERR_INT_MASK                            0x00080000
#define    DSP_INTC_ISR_32_R_CAPH_NORM_INT_SHIFT                          18
#define    DSP_INTC_ISR_32_R_CAPH_NORM_INT_MASK                           0x00040000
#define    DSP_INTC_ISR_32_R_CP2DSP_INT_SHIFT                             17
#define    DSP_INTC_ISR_32_R_CP2DSP_INT_MASK                              0x00020000
#define    DSP_INTC_ISR_32_R_AP2DSP_INT_SHIFT                             16
#define    DSP_INTC_ISR_32_R_AP2DSP_INT_MASK                              0x00010000
#define    DSP_INTC_ISR_32_R_TX_ON_INT_SHIFT                              15
#define    DSP_INTC_ISR_32_R_TX_ON_INT_MASK                               0x00008000
#define    DSP_INTC_ISR_32_R_RESERVED_14_SHIFT                            14
#define    DSP_INTC_ISR_32_R_RESERVED_14_MASK                             0x00004000
#define    DSP_INTC_ISR_32_R_RESERVED_13TO10_SHIFT                        10
#define    DSP_INTC_ISR_32_R_RESERVED_13TO10_MASK                         0x00003C00
#define    DSP_INTC_ISR_32_R_ZSM_UAL_INT_SHIFT                            9
#define    DSP_INTC_ISR_32_R_ZSM_UAL_INT_MASK                             0x00000200
#define    DSP_INTC_ISR_32_R_RESERVED_8TO7_SHIFT                          7
#define    DSP_INTC_ISR_32_R_RESERVED_8TO7_MASK                           0x00000180
#define    DSP_INTC_ISR_32_R_A5_INT_SHIFT                                 6
#define    DSP_INTC_ISR_32_R_A5_INT_MASK                                  0x00000040
#define    DSP_INTC_ISR_32_R_RESERVED_5TO4_SHIFT                          4
#define    DSP_INTC_ISR_32_R_RESERVED_5TO4_MASK                           0x00000030
#define    DSP_INTC_ISR_32_R_TXEND_INT_N_SHIFT                            3
#define    DSP_INTC_ISR_32_R_TXEND_INT_N_MASK                             0x00000008
#define    DSP_INTC_ISR_32_R_DEMRDY_INT_SHIFT                             2
#define    DSP_INTC_ISR_32_R_DEMRDY_INT_MASK                              0x00000004
#define    DSP_INTC_ISR_32_R_RXRDY_INT_N_SHIFT                            1
#define    DSP_INTC_ISR_32_R_RXRDY_INT_N_MASK                             0x00000002
#define    DSP_INTC_ISR_32_R_ISMC_INT_SHIFT                               0
#define    DSP_INTC_ISR_32_R_ISMC_INT_MASK                                0x00000001

#define DSP_INTC_IMR_32_R_OFFSET                                          0x00000024
#define DSP_INTC_IMR_32_R_TYPE                                            UInt32
#define DSP_INTC_IMR_32_R_RESERVED_MASK                                   0x00000000
#define    DSP_INTC_IMR_32_R_INT_MASK_31TO16_SHIFT                        16
#define    DSP_INTC_IMR_32_R_INT_MASK_31TO16_MASK                         0xFFFF0000
#define    DSP_INTC_IMR_32_R_INT_MASK_15_SHIFT                            15
#define    DSP_INTC_IMR_32_R_INT_MASK_15_MASK                             0x00008000
#define    DSP_INTC_IMR_32_R_DSP_SLEEP_SHIFT                              14
#define    DSP_INTC_IMR_32_R_DSP_SLEEP_MASK                               0x00004000
#define    DSP_INTC_IMR_32_R_INT_MASK_13TO0_SHIFT                         0
#define    DSP_INTC_IMR_32_R_INT_MASK_13TO0_MASK                          0x00003FFF

#define DSP_INTC_IPR_32_R_OFFSET                                          0x00000028
#define DSP_INTC_IPR_32_R_TYPE                                            UInt32
#define DSP_INTC_IPR_32_R_RESERVED_MASK                                   0x00000000
#define    DSP_INTC_IPR_32_R_INT_PENDING_31TO16_SHIFT                     16
#define    DSP_INTC_IPR_32_R_INT_PENDING_31TO16_MASK                      0xFFFF0000
#define    DSP_INTC_IPR_32_R_INT_PENDING_15_SHIFT                         15
#define    DSP_INTC_IPR_32_R_INT_PENDING_15_MASK                          0x00008000
#define    DSP_INTC_IPR_32_R_RESERVED_14_SHIFT                            14
#define    DSP_INTC_IPR_32_R_RESERVED_14_MASK                             0x00004000
#define    DSP_INTC_IPR_32_R_INT_PENDING_13TO0_SHIFT                      0
#define    DSP_INTC_IPR_32_R_INT_PENDING_13TO0_MASK                       0x00003FFF

#define DSP_INTC_IRR_32_R_OFFSET                                          0x0000002C
#define DSP_INTC_IRR_32_R_TYPE                                            UInt32
#define DSP_INTC_IRR_32_R_RESERVED_MASK                                   0x00000000
#define    DSP_INTC_IRR_32_R_INT_RST_31TO16_SHIFT                         16
#define    DSP_INTC_IRR_32_R_INT_RST_31TO16_MASK                          0xFFFF0000
#define    DSP_INTC_IRR_32_R_INT_RST_15_SHIFT                             15
#define    DSP_INTC_IRR_32_R_INT_RST_15_MASK                              0x00008000
#define    DSP_INTC_IRR_32_R_RESERVED_14_SHIFT                            14
#define    DSP_INTC_IRR_32_R_RESERVED_14_MASK                             0x00004000
#define    DSP_INTC_IRR_32_R_INT_RST_13TO0_SHIFT                          0
#define    DSP_INTC_IRR_32_R_INT_RST_13TO0_MASK                           0x00003FFF

#define DSP_INTC_TEST_EN_32_R_OFFSET                                      0x00000030
#define DSP_INTC_TEST_EN_32_R_TYPE                                        UInt32
#define DSP_INTC_TEST_EN_32_R_RESERVED_MASK                               0x00000000
#define    DSP_INTC_TEST_EN_32_R_RESERVED_31TO1_SHIFT                     1
#define    DSP_INTC_TEST_EN_32_R_RESERVED_31TO1_MASK                      0xFFFFFFFE
#define    DSP_INTC_TEST_EN_32_R_TEST_EN_SHIFT                            0
#define    DSP_INTC_TEST_EN_32_R_TEST_EN_MASK                             0x00000001

#define DSP_INTC_TEST_INT_32_R_OFFSET                                     0x00000034
#define DSP_INTC_TEST_INT_32_R_TYPE                                       UInt32
#define DSP_INTC_TEST_INT_32_R_RESERVED_MASK                              0x00000000
#define    DSP_INTC_TEST_INT_32_R_TEST_INT_31TO16_SHIFT                   16
#define    DSP_INTC_TEST_INT_32_R_TEST_INT_31TO16_MASK                    0xFFFF0000
#define    DSP_INTC_TEST_INT_32_R_TEST_INT_15_SHIFT                       15
#define    DSP_INTC_TEST_INT_32_R_TEST_INT_15_MASK                        0x00008000
#define    DSP_INTC_TEST_INT_32_R_RESERVED_14_SHIFT                       14
#define    DSP_INTC_TEST_INT_32_R_RESERVED_14_MASK                        0x00004000
#define    DSP_INTC_TEST_INT_32_R_TEST_INT_13TO0_SHIFT                    0
#define    DSP_INTC_TEST_INT_32_R_TEST_INT_13TO0_MASK                     0x00003FFF

#define DSP_INTC_INT_MAP_R0_OFFSET                                        0x00000038
#define DSP_INTC_INT_MAP_R0_TYPE                                          UInt32
#define DSP_INTC_INT_MAP_R0_RESERVED_MASK                                 0x00000000
#define    DSP_INTC_INT_MAP_R0_INT_SRC15_MAP_31TO30_SHIFT                 30
#define    DSP_INTC_INT_MAP_R0_INT_SRC15_MAP_31TO30_MASK                  0xC0000000
#define    DSP_INTC_INT_MAP_R0_RESERVED_29TO28_SHIFT                      28
#define    DSP_INTC_INT_MAP_R0_RESERVED_29TO28_MASK                       0x30000000
#define    DSP_INTC_INT_MAP_R0_INT_SRC13_MAP_27TO26_SHIFT                 26
#define    DSP_INTC_INT_MAP_R0_INT_SRC13_MAP_27TO26_MASK                  0x0C000000
#define    DSP_INTC_INT_MAP_R0_INT_SRC12_MAP_25TO24_SHIFT                 24
#define    DSP_INTC_INT_MAP_R0_INT_SRC12_MAP_25TO24_MASK                  0x03000000
#define    DSP_INTC_INT_MAP_R0_INT_SRC11_MAP_23TO22_SHIFT                 22
#define    DSP_INTC_INT_MAP_R0_INT_SRC11_MAP_23TO22_MASK                  0x00C00000
#define    DSP_INTC_INT_MAP_R0_INT_SRC10_MAP_21TO20_SHIFT                 20
#define    DSP_INTC_INT_MAP_R0_INT_SRC10_MAP_21TO20_MASK                  0x00300000
#define    DSP_INTC_INT_MAP_R0_INT_SRC9_MAP_19TO18_SHIFT                  18
#define    DSP_INTC_INT_MAP_R0_INT_SRC9_MAP_19TO18_MASK                   0x000C0000
#define    DSP_INTC_INT_MAP_R0_INT_SRC8_MAP_17TO16_SHIFT                  16
#define    DSP_INTC_INT_MAP_R0_INT_SRC8_MAP_17TO16_MASK                   0x00030000
#define    DSP_INTC_INT_MAP_R0_INT_SRC7_MAP_15TO14_SHIFT                  14
#define    DSP_INTC_INT_MAP_R0_INT_SRC7_MAP_15TO14_MASK                   0x0000C000
#define    DSP_INTC_INT_MAP_R0_INT_SRC6_MAP_13TO12_SHIFT                  12
#define    DSP_INTC_INT_MAP_R0_INT_SRC6_MAP_13TO12_MASK                   0x00003000
#define    DSP_INTC_INT_MAP_R0_INT_SRC5_MAP_11TO10_SHIFT                  10
#define    DSP_INTC_INT_MAP_R0_INT_SRC5_MAP_11TO10_MASK                   0x00000C00
#define    DSP_INTC_INT_MAP_R0_INT_SRC4_MAP_9TO8_SHIFT                    8
#define    DSP_INTC_INT_MAP_R0_INT_SRC4_MAP_9TO8_MASK                     0x00000300
#define    DSP_INTC_INT_MAP_R0_INT_SRC3_MAP_7TO6_SHIFT                    6
#define    DSP_INTC_INT_MAP_R0_INT_SRC3_MAP_7TO6_MASK                     0x000000C0
#define    DSP_INTC_INT_MAP_R0_INT_SRC2_MAP_5TO4_SHIFT                    4
#define    DSP_INTC_INT_MAP_R0_INT_SRC2_MAP_5TO4_MASK                     0x00000030
#define    DSP_INTC_INT_MAP_R0_INT_SRC1_MAP_3TO2_SHIFT                    2
#define    DSP_INTC_INT_MAP_R0_INT_SRC1_MAP_3TO2_MASK                     0x0000000C
#define    DSP_INTC_INT_MAP_R0_INT_SRC0_MAP_1TO0_SHIFT                    0
#define    DSP_INTC_INT_MAP_R0_INT_SRC0_MAP_1TO0_MASK                     0x00000003

#define DSP_INTC_INT_MAP_R1_OFFSET                                        0x0000003C
#define DSP_INTC_INT_MAP_R1_TYPE                                          UInt32
#define DSP_INTC_INT_MAP_R1_RESERVED_MASK                                 0x00000000
#define    DSP_INTC_INT_MAP_R1_INT_SRC31_MAP_31TO30_SHIFT                 30
#define    DSP_INTC_INT_MAP_R1_INT_SRC31_MAP_31TO30_MASK                  0xC0000000
#define    DSP_INTC_INT_MAP_R1_INT_SRC30_MAP_29TO28_SHIFT                 28
#define    DSP_INTC_INT_MAP_R1_INT_SRC30_MAP_29TO28_MASK                  0x30000000
#define    DSP_INTC_INT_MAP_R1_INT_SRC29_MAP_27TO26_SHIFT                 26
#define    DSP_INTC_INT_MAP_R1_INT_SRC29_MAP_27TO26_MASK                  0x0C000000
#define    DSP_INTC_INT_MAP_R1_INT_SRC28_MAP_25TO24_SHIFT                 24
#define    DSP_INTC_INT_MAP_R1_INT_SRC28_MAP_25TO24_MASK                  0x03000000
#define    DSP_INTC_INT_MAP_R1_INT_SRC27_MAP_23TO22_SHIFT                 22
#define    DSP_INTC_INT_MAP_R1_INT_SRC27_MAP_23TO22_MASK                  0x00C00000
#define    DSP_INTC_INT_MAP_R1_INT_SRC26_MAP_21TO20_SHIFT                 20
#define    DSP_INTC_INT_MAP_R1_INT_SRC26_MAP_21TO20_MASK                  0x00300000
#define    DSP_INTC_INT_MAP_R1_INT_SRC25_MAP_19TO18_SHIFT                 18
#define    DSP_INTC_INT_MAP_R1_INT_SRC25_MAP_19TO18_MASK                  0x000C0000
#define    DSP_INTC_INT_MAP_R1_INT_SRC24_MAP_17TO16_SHIFT                 16
#define    DSP_INTC_INT_MAP_R1_INT_SRC24_MAP_17TO16_MASK                  0x00030000
#define    DSP_INTC_INT_MAP_R1_INT_SRC23_MAP_15TO14_SHIFT                 14
#define    DSP_INTC_INT_MAP_R1_INT_SRC23_MAP_15TO14_MASK                  0x0000C000
#define    DSP_INTC_INT_MAP_R1_INT_SRC22_MAP_13TO12_SHIFT                 12
#define    DSP_INTC_INT_MAP_R1_INT_SRC22_MAP_13TO12_MASK                  0x00003000
#define    DSP_INTC_INT_MAP_R1_INT_SRC21_MAP_11TO10_SHIFT                 10
#define    DSP_INTC_INT_MAP_R1_INT_SRC21_MAP_11TO10_MASK                  0x00000C00
#define    DSP_INTC_INT_MAP_R1_INT_SRC20_MAP_9TO8_SHIFT                   8
#define    DSP_INTC_INT_MAP_R1_INT_SRC20_MAP_9TO8_MASK                    0x00000300
#define    DSP_INTC_INT_MAP_R1_INT_SRC19_MAP_7TO6_SHIFT                   6
#define    DSP_INTC_INT_MAP_R1_INT_SRC19_MAP_7TO6_MASK                    0x000000C0
#define    DSP_INTC_INT_MAP_R1_INT_SRC18_MAP_5TO4_SHIFT                   4
#define    DSP_INTC_INT_MAP_R1_INT_SRC18_MAP_5TO4_MASK                    0x00000030
#define    DSP_INTC_INT_MAP_R1_INT_SRC17_MAP_3TO2_SHIFT                   2
#define    DSP_INTC_INT_MAP_R1_INT_SRC17_MAP_3TO2_MASK                    0x0000000C
#define    DSP_INTC_INT_MAP_R1_INT_SRC16_MAP_1TO0_SHIFT                   0
#define    DSP_INTC_INT_MAP_R1_INT_SRC16_MAP_1TO0_MASK                    0x00000003

#define DSP_INTC_SPAD0_REG_OFFSET                                         0x0000E000
#define DSP_INTC_SPAD0_REG_TYPE                                           UInt16
#define DSP_INTC_SPAD0_REG_RESERVED_MASK                                  0x00000000
#define    DSP_INTC_SPAD0_REG_SCRATCH_PAD0_REG_15TO0_SHIFT                0
#define    DSP_INTC_SPAD0_REG_SCRATCH_PAD0_REG_15TO0_MASK                 0x0000FFFF

#define DSP_INTC_SCR_OFFSET                                               0x0000E001
#define DSP_INTC_SCR_TYPE                                                 UInt16
#define DSP_INTC_SCR_RESERVED_MASK                                        0x00000000
#define    DSP_INTC_SCR_OTOCFINT_SHIFT                                    15
#define    DSP_INTC_SCR_OTOCFINT_MASK                                     0x00008000
#define    DSP_INTC_SCR_OTOCINT_SHIFT                                     14
#define    DSP_INTC_SCR_OTOCINT_MASK                                      0x00004000
#define    DSP_INTC_SCR_EN_DEROTATOR_SHIFT                                13
#define    DSP_INTC_SCR_EN_DEROTATOR_MASK                                 0x00002000
#define    DSP_INTC_SCR_SEL_DIG_RF_TX_SHIFT                               12
#define    DSP_INTC_SCR_SEL_DIG_RF_TX_MASK                                0x00001000
#define    DSP_INTC_SCR_OTOAFINT_SHIFT                                    11
#define    DSP_INTC_SCR_OTOAFINT_MASK                                     0x00000800
#define    DSP_INTC_SCR_EN8PSK_MANU_SHIFT                                 10
#define    DSP_INTC_SCR_EN8PSK_MANU_MASK                                  0x00000400
#define    DSP_INTC_SCR_RESERVED_9_SHIFT                                  9
#define    DSP_INTC_SCR_RESERVED_9_MASK                                   0x00000200
#define    DSP_INTC_SCR_OTOAINT_SHIFT                                     8
#define    DSP_INTC_SCR_OTOAINT_MASK                                      0x00000100
#define    DSP_INTC_SCR_SEL_DIGRF2_SHIFT                                  7
#define    DSP_INTC_SCR_SEL_DIGRF2_MASK                                   0x00000080
#define    DSP_INTC_SCR_RESERVED_6_SHIFT                                  6
#define    DSP_INTC_SCR_RESERVED_6_MASK                                   0x00000040
#define    DSP_INTC_SCR_A5_EN_SHIFT                                       5
#define    DSP_INTC_SCR_A5_EN_MASK                                        0x00000020
#define    DSP_INTC_SCR_RESERVED_4TO1_SHIFT                               1
#define    DSP_INTC_SCR_RESERVED_4TO1_MASK                                0x0000001E
#define    DSP_INTC_SCR_SEL_DIG_RF_RX_SHIFT                               0
#define    DSP_INTC_SCR_SEL_DIG_RF_RX_MASK                                0x00000001

#define DSP_INTC_SAMPLE_BUF_STA_REG_OFFSET                                0x0000E002
#define DSP_INTC_SAMPLE_BUF_STA_REG_TYPE                                  UInt16
#define DSP_INTC_SAMPLE_BUF_STA_REG_RESERVED_MASK                         0x00000000
#define    DSP_INTC_SAMPLE_BUF_STA_REG_RESERVED_15TO12_SHIFT              12
#define    DSP_INTC_SAMPLE_BUF_STA_REG_RESERVED_15TO12_MASK               0x0000F000
#define    DSP_INTC_SAMPLE_BUF_STA_REG_SWTBUF_SHIFT                       11
#define    DSP_INTC_SAMPLE_BUF_STA_REG_SWTBUF_MASK                        0x00000800
#define    DSP_INTC_SAMPLE_BUF_STA_REG_RESERVED_10TO0_SHIFT               0
#define    DSP_INTC_SAMPLE_BUF_STA_REG_RESERVED_10TO0_MASK                0x000007FF

#define DSP_INTC_ISR_OFFSET                                               0x0000E003
#define DSP_INTC_ISR_TYPE                                                 UInt16
#define DSP_INTC_ISR_RESERVED_MASK                                        0x00000000
#define    DSP_INTC_ISR_ADDR_PLACE_HOLDER_SHIFT                           0
#define    DSP_INTC_ISR_ADDR_PLACE_HOLDER_MASK                            0x0000FFFF

#define DSP_INTC_IMR_OFFSET                                               0x0000E004
#define DSP_INTC_IMR_TYPE                                                 UInt16
#define DSP_INTC_IMR_RESERVED_MASK                                        0x00000000
#define    DSP_INTC_IMR_ADDR_PLACE_HOLDER_SHIFT                           0
#define    DSP_INTC_IMR_ADDR_PLACE_HOLDER_MASK                            0x0000FFFF

#define DSP_INTC_IPR_OFFSET                                               0x0000E005
#define DSP_INTC_IPR_TYPE                                                 UInt16
#define DSP_INTC_IPR_RESERVED_MASK                                        0x00000000
#define    DSP_INTC_IPR_ADDR_PLACE_HOLDER_SHIFT                           0
#define    DSP_INTC_IPR_ADDR_PLACE_HOLDER_MASK                            0x0000FFFF

#define DSP_INTC_IRR_OFFSET                                               0x0000E006
#define DSP_INTC_IRR_TYPE                                                 UInt16
#define DSP_INTC_IRR_RESERVED_MASK                                        0x00000000
#define    DSP_INTC_IRR_ADDR_PLACE_HOLDER_SHIFT                           0
#define    DSP_INTC_IRR_ADDR_PLACE_HOLDER_MASK                            0x0000FFFF

#define DSP_INTC_PIN_STA_CTL_REG_OFFSET                                   0x0000E007
#define DSP_INTC_PIN_STA_CTL_REG_TYPE                                     UInt16
#define DSP_INTC_PIN_STA_CTL_REG_RESERVED_MASK                            0x00000000
#define    DSP_INTC_PIN_STA_CTL_REG_RESERVED_15TO14_SHIFT                 14
#define    DSP_INTC_PIN_STA_CTL_REG_RESERVED_15TO14_MASK                  0x0000C000
#define    DSP_INTC_PIN_STA_CTL_REG_ARM_PLLPDEN_SHIFT                     13
#define    DSP_INTC_PIN_STA_CTL_REG_ARM_PLLPDEN_MASK                      0x00002000
#define    DSP_INTC_PIN_STA_CTL_REG_RESERVED_12_SHIFT                     12
#define    DSP_INTC_PIN_STA_CTL_REG_RESERVED_12_MASK                      0x00001000
#define    DSP_INTC_PIN_STA_CTL_REG_OAKRST_SHIFT                          11
#define    DSP_INTC_PIN_STA_CTL_REG_OAKRST_MASK                           0x00000800
#define    DSP_INTC_PIN_STA_CTL_REG_EXTID_2TO0_SHIFT                      8
#define    DSP_INTC_PIN_STA_CTL_REG_EXTID_2TO0_MASK                       0x00000700
#define    DSP_INTC_PIN_STA_CTL_REG_RESERVED_7TO0_SHIFT                   0
#define    DSP_INTC_PIN_STA_CTL_REG_RESERVED_7TO0_MASK                    0x000000FF

#define DSP_INTC_TEST_EN_REG_OFFSET                                       0x0000E00E
#define DSP_INTC_TEST_EN_REG_TYPE                                         UInt16
#define DSP_INTC_TEST_EN_REG_RESERVED_MASK                                0x00000000
#define    DSP_INTC_TEST_EN_REG_ADDR_PLACE_HOLDER_SHIFT                   0
#define    DSP_INTC_TEST_EN_REG_ADDR_PLACE_HOLDER_MASK                    0x0000FFFF

#define DSP_INTC_TEST_INT_REG_OFFSET                                      0x0000E00F
#define DSP_INTC_TEST_INT_REG_TYPE                                        UInt16
#define DSP_INTC_TEST_INT_REG_RESERVED_MASK                               0x00000000
#define    DSP_INTC_TEST_INT_REG_ADDR_PLACE_HOLDER_SHIFT                  0
#define    DSP_INTC_TEST_INT_REG_ADDR_PLACE_HOLDER_MASK                   0x0000FFFF

#define DSP_INTC_ISR_32_OFFSET                                            0x0000E010
#define DSP_INTC_ISR_32_TYPE                                              UInt32
#define DSP_INTC_ISR_32_RESERVED_MASK                                     0x00000000
#define    DSP_INTC_ISR_32_RESERVED_31TO28_SHIFT                          28
#define    DSP_INTC_ISR_32_RESERVED_31TO28_MASK                           0xF0000000
#define    DSP_INTC_ISR_32_SPARE2DSP_INT_7TO0_SHIFT                       20
#define    DSP_INTC_ISR_32_SPARE2DSP_INT_7TO0_MASK                        0x0FF00000
#define    DSP_INTC_ISR_32_CAPH_ERR_INT_SHIFT                             19
#define    DSP_INTC_ISR_32_CAPH_ERR_INT_MASK                              0x00080000
#define    DSP_INTC_ISR_32_CAPH_NORM_INT_SHIFT                            18
#define    DSP_INTC_ISR_32_CAPH_NORM_INT_MASK                             0x00040000
#define    DSP_INTC_ISR_32_CP2DSP_INT_SHIFT                               17
#define    DSP_INTC_ISR_32_CP2DSP_INT_MASK                                0x00020000
#define    DSP_INTC_ISR_32_AP2DSP_INT_SHIFT                               16
#define    DSP_INTC_ISR_32_AP2DSP_INT_MASK                                0x00010000
#define    DSP_INTC_ISR_32_TX_ON_INT_SHIFT                                15
#define    DSP_INTC_ISR_32_TX_ON_INT_MASK                                 0x00008000
#define    DSP_INTC_ISR_32_RESERVED_14_SHIFT                              14
#define    DSP_INTC_ISR_32_RESERVED_14_MASK                               0x00004000
#define    DSP_INTC_ISR_32_RESERVED_13TO10_SHIFT                          10
#define    DSP_INTC_ISR_32_RESERVED_13TO10_MASK                           0x00003C00
#define    DSP_INTC_ISR_32_ZSM_UAL_INT_SHIFT                              9
#define    DSP_INTC_ISR_32_ZSM_UAL_INT_MASK                               0x00000200
#define    DSP_INTC_ISR_32_RESERVED_8TO7_SHIFT                            7
#define    DSP_INTC_ISR_32_RESERVED_8TO7_MASK                             0x00000180
#define    DSP_INTC_ISR_32_A5_INT_SHIFT                                   6
#define    DSP_INTC_ISR_32_A5_INT_MASK                                    0x00000040
#define    DSP_INTC_ISR_32_RESERVED_5TO4_SHIFT                            4
#define    DSP_INTC_ISR_32_RESERVED_5TO4_MASK                             0x00000030
#define    DSP_INTC_ISR_32_TXEND_INT_N_SHIFT                              3
#define    DSP_INTC_ISR_32_TXEND_INT_N_MASK                               0x00000008
#define    DSP_INTC_ISR_32_DEMRDY_INT_SHIFT                               2
#define    DSP_INTC_ISR_32_DEMRDY_INT_MASK                                0x00000004
#define    DSP_INTC_ISR_32_RXRDY_INT_N_SHIFT                              1
#define    DSP_INTC_ISR_32_RXRDY_INT_N_MASK                               0x00000002
#define    DSP_INTC_ISR_32_ISMC_INT_SHIFT                                 0
#define    DSP_INTC_ISR_32_ISMC_INT_MASK                                  0x00000001

#define DSP_INTC_IMR_32_OFFSET                                            0x0000E012
#define DSP_INTC_IMR_32_TYPE                                              UInt32
#define DSP_INTC_IMR_32_RESERVED_MASK                                     0x00000000
#define    DSP_INTC_IMR_32_INT_MASK_31TO16_SHIFT                          16
#define    DSP_INTC_IMR_32_INT_MASK_31TO16_MASK                           0xFFFF0000
#define    DSP_INTC_IMR_32_INT_MASK_15_SHIFT                              15
#define    DSP_INTC_IMR_32_INT_MASK_15_MASK                               0x00008000
#define    DSP_INTC_IMR_32_DSP_SLEEP_SHIFT                                14
#define    DSP_INTC_IMR_32_DSP_SLEEP_MASK                                 0x00004000
#define    DSP_INTC_IMR_32_INT_MASK_13TO0_SHIFT                           0
#define    DSP_INTC_IMR_32_INT_MASK_13TO0_MASK                            0x00003FFF

#define DSP_INTC_IPR_32_OFFSET                                            0x0000E014
#define DSP_INTC_IPR_32_TYPE                                              UInt32
#define DSP_INTC_IPR_32_RESERVED_MASK                                     0x00000000
#define    DSP_INTC_IPR_32_INT_PENDING_31TO16_SHIFT                       16
#define    DSP_INTC_IPR_32_INT_PENDING_31TO16_MASK                        0xFFFF0000
#define    DSP_INTC_IPR_32_INT_PENDING_15_SHIFT                           15
#define    DSP_INTC_IPR_32_INT_PENDING_15_MASK                            0x00008000
#define    DSP_INTC_IPR_32_RESERVED_14_SHIFT                              14
#define    DSP_INTC_IPR_32_RESERVED_14_MASK                               0x00004000
#define    DSP_INTC_IPR_32_INT_PENDING_13TO0_SHIFT                        0
#define    DSP_INTC_IPR_32_INT_PENDING_13TO0_MASK                         0x00003FFF

#define DSP_INTC_IRR_32_OFFSET                                            0x0000E016
#define DSP_INTC_IRR_32_TYPE                                              UInt32
#define DSP_INTC_IRR_32_RESERVED_MASK                                     0x00000000
#define    DSP_INTC_IRR_32_INT_RST_31TO16_SHIFT                           16
#define    DSP_INTC_IRR_32_INT_RST_31TO16_MASK                            0xFFFF0000
#define    DSP_INTC_IRR_32_INT_RST_15_SHIFT                               15
#define    DSP_INTC_IRR_32_INT_RST_15_MASK                                0x00008000
#define    DSP_INTC_IRR_32_RESERVED_14_SHIFT                              14
#define    DSP_INTC_IRR_32_RESERVED_14_MASK                               0x00004000
#define    DSP_INTC_IRR_32_INT_RST_13TO0_SHIFT                            0
#define    DSP_INTC_IRR_32_INT_RST_13TO0_MASK                             0x00003FFF

#define DSP_INTC_TEST_EN_REG_32_OFFSET                                    0x0000E018
#define DSP_INTC_TEST_EN_REG_32_TYPE                                      UInt32
#define DSP_INTC_TEST_EN_REG_32_RESERVED_MASK                             0x00000000
#define    DSP_INTC_TEST_EN_REG_32_RESERVED_31TO1_SHIFT                   1
#define    DSP_INTC_TEST_EN_REG_32_RESERVED_31TO1_MASK                    0xFFFFFFFE
#define    DSP_INTC_TEST_EN_REG_32_TEST_EN_SHIFT                          0
#define    DSP_INTC_TEST_EN_REG_32_TEST_EN_MASK                           0x00000001

#define DSP_INTC_TEST_INT_REG_32_OFFSET                                   0x0000E01A
#define DSP_INTC_TEST_INT_REG_32_TYPE                                     UInt32
#define DSP_INTC_TEST_INT_REG_32_RESERVED_MASK                            0x00000000
#define    DSP_INTC_TEST_INT_REG_32_TEST_INT_31TO16_SHIFT                 16
#define    DSP_INTC_TEST_INT_REG_32_TEST_INT_31TO16_MASK                  0xFFFF0000
#define    DSP_INTC_TEST_INT_REG_32_TEST_INT_15_SHIFT                     15
#define    DSP_INTC_TEST_INT_REG_32_TEST_INT_15_MASK                      0x00008000
#define    DSP_INTC_TEST_INT_REG_32_RESERVED_14_SHIFT                     14
#define    DSP_INTC_TEST_INT_REG_32_RESERVED_14_MASK                      0x00004000
#define    DSP_INTC_TEST_INT_REG_32_TEST_INT_13TO0_SHIFT                  0
#define    DSP_INTC_TEST_INT_REG_32_TEST_INT_13TO0_MASK                   0x00003FFF

#define DSP_INTC_INT_MAP_REG0_OFFSET                                      0x0000E01C
#define DSP_INTC_INT_MAP_REG0_TYPE                                        UInt32
#define DSP_INTC_INT_MAP_REG0_RESERVED_MASK                               0x00000000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC15_MAP_31TO30_SHIFT               30
#define    DSP_INTC_INT_MAP_REG0_INT_SRC15_MAP_31TO30_MASK                0xC0000000
#define    DSP_INTC_INT_MAP_REG0_RESERVED_29TO28_SHIFT                    28
#define    DSP_INTC_INT_MAP_REG0_RESERVED_29TO28_MASK                     0x30000000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC13_MAP_27TO26_SHIFT               26
#define    DSP_INTC_INT_MAP_REG0_INT_SRC13_MAP_27TO26_MASK                0x0C000000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC12_MAP_25TO24_SHIFT               24
#define    DSP_INTC_INT_MAP_REG0_INT_SRC12_MAP_25TO24_MASK                0x03000000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC11_MAP_23TO22_SHIFT               22
#define    DSP_INTC_INT_MAP_REG0_INT_SRC11_MAP_23TO22_MASK                0x00C00000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC10_MAP_21TO20_SHIFT               20
#define    DSP_INTC_INT_MAP_REG0_INT_SRC10_MAP_21TO20_MASK                0x00300000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC9_MAP_19TO18_SHIFT                18
#define    DSP_INTC_INT_MAP_REG0_INT_SRC9_MAP_19TO18_MASK                 0x000C0000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC8_MAP_17TO16_SHIFT                16
#define    DSP_INTC_INT_MAP_REG0_INT_SRC8_MAP_17TO16_MASK                 0x00030000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC7_MAP_15TO14_SHIFT                14
#define    DSP_INTC_INT_MAP_REG0_INT_SRC7_MAP_15TO14_MASK                 0x0000C000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC6_MAP_13TO12_SHIFT                12
#define    DSP_INTC_INT_MAP_REG0_INT_SRC6_MAP_13TO12_MASK                 0x00003000
#define    DSP_INTC_INT_MAP_REG0_INT_SRC5_MAP_11TO10_SHIFT                10
#define    DSP_INTC_INT_MAP_REG0_INT_SRC5_MAP_11TO10_MASK                 0x00000C00
#define    DSP_INTC_INT_MAP_REG0_INT_SRC4_MAP_9TO8_SHIFT                  8
#define    DSP_INTC_INT_MAP_REG0_INT_SRC4_MAP_9TO8_MASK                   0x00000300
#define    DSP_INTC_INT_MAP_REG0_INT_SRC3_MAP_7TO6_SHIFT                  6
#define    DSP_INTC_INT_MAP_REG0_INT_SRC3_MAP_7TO6_MASK                   0x000000C0
#define    DSP_INTC_INT_MAP_REG0_INT_SRC2_MAP_5TO4_SHIFT                  4
#define    DSP_INTC_INT_MAP_REG0_INT_SRC2_MAP_5TO4_MASK                   0x00000030
#define    DSP_INTC_INT_MAP_REG0_INT_SRC1_MAP_3TO2_SHIFT                  2
#define    DSP_INTC_INT_MAP_REG0_INT_SRC1_MAP_3TO2_MASK                   0x0000000C
#define    DSP_INTC_INT_MAP_REG0_INT_SRC0_MAP_1TO0_SHIFT                  0
#define    DSP_INTC_INT_MAP_REG0_INT_SRC0_MAP_1TO0_MASK                   0x00000003

#define DSP_INTC_INT_MAP_REG1_OFFSET                                      0x0000E01E
#define DSP_INTC_INT_MAP_REG1_TYPE                                        UInt32
#define DSP_INTC_INT_MAP_REG1_RESERVED_MASK                               0x00000000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC31_MAP_31TO30_SHIFT               30
#define    DSP_INTC_INT_MAP_REG1_INT_SRC31_MAP_31TO30_MASK                0xC0000000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC30_MAP_29TO28_SHIFT               28
#define    DSP_INTC_INT_MAP_REG1_INT_SRC30_MAP_29TO28_MASK                0x30000000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC29_MAP_27TO26_SHIFT               26
#define    DSP_INTC_INT_MAP_REG1_INT_SRC29_MAP_27TO26_MASK                0x0C000000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC28_MAP_25TO24_SHIFT               24
#define    DSP_INTC_INT_MAP_REG1_INT_SRC28_MAP_25TO24_MASK                0x03000000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC27_MAP_23TO22_SHIFT               22
#define    DSP_INTC_INT_MAP_REG1_INT_SRC27_MAP_23TO22_MASK                0x00C00000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC26_MAP_21TO20_SHIFT               20
#define    DSP_INTC_INT_MAP_REG1_INT_SRC26_MAP_21TO20_MASK                0x00300000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC25_MAP_19TO18_SHIFT               18
#define    DSP_INTC_INT_MAP_REG1_INT_SRC25_MAP_19TO18_MASK                0x000C0000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC24_MAP_17TO16_SHIFT               16
#define    DSP_INTC_INT_MAP_REG1_INT_SRC24_MAP_17TO16_MASK                0x00030000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC23_MAP_15TO14_SHIFT               14
#define    DSP_INTC_INT_MAP_REG1_INT_SRC23_MAP_15TO14_MASK                0x0000C000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC22_MAP_13TO12_SHIFT               12
#define    DSP_INTC_INT_MAP_REG1_INT_SRC22_MAP_13TO12_MASK                0x00003000
#define    DSP_INTC_INT_MAP_REG1_INT_SRC21_MAP_11TO10_SHIFT               10
#define    DSP_INTC_INT_MAP_REG1_INT_SRC21_MAP_11TO10_MASK                0x00000C00
#define    DSP_INTC_INT_MAP_REG1_INT_SRC20_MAP_9TO8_SHIFT                 8
#define    DSP_INTC_INT_MAP_REG1_INT_SRC20_MAP_9TO8_MASK                  0x00000300
#define    DSP_INTC_INT_MAP_REG1_INT_SRC19_MAP_7TO6_SHIFT                 6
#define    DSP_INTC_INT_MAP_REG1_INT_SRC19_MAP_7TO6_MASK                  0x000000C0
#define    DSP_INTC_INT_MAP_REG1_INT_SRC18_MAP_5TO4_SHIFT                 4
#define    DSP_INTC_INT_MAP_REG1_INT_SRC18_MAP_5TO4_MASK                  0x00000030
#define    DSP_INTC_INT_MAP_REG1_INT_SRC17_MAP_3TO2_SHIFT                 2
#define    DSP_INTC_INT_MAP_REG1_INT_SRC17_MAP_3TO2_MASK                  0x0000000C
#define    DSP_INTC_INT_MAP_REG1_INT_SRC16_MAP_1TO0_SHIFT                 0
#define    DSP_INTC_INT_MAP_REG1_INT_SRC16_MAP_1TO0_MASK                  0x00000003

#endif /* __BRCM_RDB_DSP_INTC_H__ */



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

#ifndef __BRCM_RDB_BMODEM_SYSCFG_H__
#define __BRCM_RDB_BMODEM_SYSCFG_H__

#define BMODEM_SYSCFG_R4_CFG0_OFFSET                                      0x00000000
#define BMODEM_SYSCFG_R4_CFG0_TYPE                                        UInt32
#define BMODEM_SYSCFG_R4_CFG0_RESERVED_MASK                               0xFFFFFFC0
#define    BMODEM_SYSCFG_R4_CFG0_R4_AWADDRS_BIT20_SHIFT                   5
#define    BMODEM_SYSCFG_R4_CFG0_R4_AWADDRS_BIT20_MASK                    0x00000020
#define    BMODEM_SYSCFG_R4_CFG0_R4_ARADDRS_BIT20_SHIFT                   4
#define    BMODEM_SYSCFG_R4_CFG0_R4_ARADDRS_BIT20_MASK                    0x00000010
#define    BMODEM_SYSCFG_R4_CFG0_INITRAMA_SHIFT                           3
#define    BMODEM_SYSCFG_R4_CFG0_INITRAMA_MASK                            0x00000008
#define    BMODEM_SYSCFG_R4_CFG0_LOCZRAMB_SHIFT                           2
#define    BMODEM_SYSCFG_R4_CFG0_LOCZRAMB_MASK                            0x00000004
#define    BMODEM_SYSCFG_R4_CFG0_INITRAMB_SHIFT                           1
#define    BMODEM_SYSCFG_R4_CFG0_INITRAMB_MASK                            0x00000002
#define    BMODEM_SYSCFG_R4_CFG0_NCPUHALT_SHIFT                           0
#define    BMODEM_SYSCFG_R4_CFG0_NCPUHALT_MASK                            0x00000001

#define BMODEM_SYSCFG_CR_OFFSET                                           0x00000040
#define BMODEM_SYSCFG_CR_TYPE                                             UInt32
#define BMODEM_SYSCFG_CR_RESERVED_MASK                                    0xFFFFFFEC
#define    BMODEM_SYSCFG_CR_TRACE_2G3G_SELECT_SHIFT                       4
#define    BMODEM_SYSCFG_CR_TRACE_2G3G_SELECT_MASK                        0x00000010
#define    BMODEM_SYSCFG_CR_RFSPI_FB_SEL_3G_SHIFT                         1
#define    BMODEM_SYSCFG_CR_RFSPI_FB_SEL_3G_MASK                          0x00000002
#define    BMODEM_SYSCFG_CR_RFSPI_FB_SEL_2G_SHIFT                         0
#define    BMODEM_SYSCFG_CR_RFSPI_FB_SEL_2G_MASK                          0x00000001

#define BMODEM_SYSCFG_PORTCR_OFFSET                                       0x00000044
#define BMODEM_SYSCFG_PORTCR_TYPE                                         UInt32
#define BMODEM_SYSCFG_PORTCR_RESERVED_MASK                                0x0000F0E0
#define    BMODEM_SYSCFG_PORTCR_GPEN_MODE_STATUS_SHIFT                    16
#define    BMODEM_SYSCFG_PORTCR_GPEN_MODE_STATUS_MASK                     0xFFFF0000
#define    BMODEM_SYSCFG_PORTCR_TWIF_MODE_STATUS_SHIFT                    11
#define    BMODEM_SYSCFG_PORTCR_TWIF_MODE_STATUS_MASK                     0x00000800
#define    BMODEM_SYSCFG_PORTCR_TWIF_MODE_TIMED_SHIFT                     9
#define    BMODEM_SYSCFG_PORTCR_TWIF_MODE_TIMED_MASK                      0x00000600
#define    BMODEM_SYSCFG_PORTCR_TWIF_MODE_SHIFT                           8
#define    BMODEM_SYSCFG_PORTCR_TWIF_MODE_MASK                            0x00000100
#define    BMODEM_SYSCFG_PORTCR_COM_GPEN_SEL_SHIFT                        4
#define    BMODEM_SYSCFG_PORTCR_COM_GPEN_SEL_MASK                         0x00000010
#define    BMODEM_SYSCFG_PORTCR_COM_MODE_STATUS_SHIFT                     3
#define    BMODEM_SYSCFG_PORTCR_COM_MODE_STATUS_MASK                      0x00000008
#define    BMODEM_SYSCFG_PORTCR_COM_MODE_TIMED_SHIFT                      1
#define    BMODEM_SYSCFG_PORTCR_COM_MODE_TIMED_MASK                       0x00000006
#define    BMODEM_SYSCFG_PORTCR_COM_MODE_SHIFT                            0
#define    BMODEM_SYSCFG_PORTCR_COM_MODE_MASK                             0x00000001

#define BMODEM_SYSCFG_GPENCR_OFFSET                                       0x00000048
#define BMODEM_SYSCFG_GPENCR_TYPE                                         UInt32
#define BMODEM_SYSCFG_GPENCR_RESERVED_MASK                                0xFFE00000
#define    BMODEM_SYSCFG_GPENCR_RFSPI_2OR3BITS_SHIFT                      20
#define    BMODEM_SYSCFG_GPENCR_RFSPI_2OR3BITS_MASK                       0x00100000
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE_TIMED_SHIFT                     18
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE_TIMED_MASK                      0x000C0000
#define    BMODEM_SYSCFG_GPENCR_RFSPI_MODE_TIMED_SHIFT                    16
#define    BMODEM_SYSCFG_GPENCR_RFSPI_MODE_TIMED_MASK                     0x00030000
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE15TO3_SHIFT                      3
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE15TO3_MASK                       0x0000FFF8
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE2TO1_SHIFT                       1
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE2TO1_MASK                        0x00000006
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE0_SHIFT                          0
#define    BMODEM_SYSCFG_GPENCR_GPEN_MODE0_MASK                           0x00000001

#define BMODEM_SYSCFG_PORTTR_0_OFFSET                                     0x00000050
#define BMODEM_SYSCFG_PORTTR_0_TYPE                                       UInt32
#define BMODEM_SYSCFG_PORTTR_0_RESERVED_MASK                              0xFFFC0000
#define    BMODEM_SYSCFG_PORTTR_0_PORTTIME_SHIFT                          0
#define    BMODEM_SYSCFG_PORTTR_0_PORTTIME_MASK                           0x0003FFFF

#define BMODEM_SYSCFG_PORTTR_1_OFFSET                                     0x00000054
#define BMODEM_SYSCFG_PORTTR_1_TYPE                                       UInt32
#define BMODEM_SYSCFG_PORTTR_1_RESERVED_MASK                              0xFFFC0000
#define    BMODEM_SYSCFG_PORTTR_1_PORTTIME_SHIFT                          0
#define    BMODEM_SYSCFG_PORTTR_1_PORTTIME_MASK                           0x0003FFFF

#define BMODEM_SYSCFG_PORTTR_2_OFFSET                                     0x00000058
#define BMODEM_SYSCFG_PORTTR_2_TYPE                                       UInt32
#define BMODEM_SYSCFG_PORTTR_2_RESERVED_MASK                              0xFFFC0000
#define    BMODEM_SYSCFG_PORTTR_2_PORTTIME_SHIFT                          0
#define    BMODEM_SYSCFG_PORTTR_2_PORTTIME_MASK                           0x0003FFFF

#define BMODEM_SYSCFG_PORTTR_3_OFFSET                                     0x0000005C
#define BMODEM_SYSCFG_PORTTR_3_TYPE                                       UInt32
#define BMODEM_SYSCFG_PORTTR_3_RESERVED_MASK                              0xFFFC0000
#define    BMODEM_SYSCFG_PORTTR_3_PORTTIME_SHIFT                          0
#define    BMODEM_SYSCFG_PORTTR_3_PORTTIME_MASK                           0x0003FFFF

#define BMODEM_SYSCFG_DSER_0_OFFSET                                       0x00000060
#define BMODEM_SYSCFG_DSER_0_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSER_0_RESERVED_MASK                                0x00CC0000
#define    BMODEM_SYSCFG_DSER_0_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSER_0_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSER_0_EXE_CNT_FRAME_SHIFT                       21
#define    BMODEM_SYSCFG_DSER_0_EXE_CNT_FRAME_MASK                        0x00200000
#define    BMODEM_SYSCFG_DSER_0_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSER_0_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSER_0_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSER_0_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSER_0_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSER_0_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSER_1_OFFSET                                       0x00000064
#define BMODEM_SYSCFG_DSER_1_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSER_1_RESERVED_MASK                                0x00CC0000
#define    BMODEM_SYSCFG_DSER_1_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSER_1_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSER_1_EXE_CNT_FRAME_SHIFT                       21
#define    BMODEM_SYSCFG_DSER_1_EXE_CNT_FRAME_MASK                        0x00200000
#define    BMODEM_SYSCFG_DSER_1_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSER_1_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSER_1_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSER_1_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSER_1_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSER_1_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSER_2_OFFSET                                       0x00000068
#define BMODEM_SYSCFG_DSER_2_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSER_2_RESERVED_MASK                                0x00CC0000
#define    BMODEM_SYSCFG_DSER_2_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSER_2_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSER_2_EXE_CNT_FRAME_SHIFT                       21
#define    BMODEM_SYSCFG_DSER_2_EXE_CNT_FRAME_MASK                        0x00200000
#define    BMODEM_SYSCFG_DSER_2_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSER_2_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSER_2_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSER_2_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSER_2_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSER_2_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSER_3_OFFSET                                       0x0000006C
#define BMODEM_SYSCFG_DSER_3_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSER_3_RESERVED_MASK                                0x00CC0000
#define    BMODEM_SYSCFG_DSER_3_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSER_3_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSER_3_EXE_CNT_FRAME_SHIFT                       21
#define    BMODEM_SYSCFG_DSER_3_EXE_CNT_FRAME_MASK                        0x00200000
#define    BMODEM_SYSCFG_DSER_3_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSER_3_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSER_3_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSER_3_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSER_3_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSER_3_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSSR_0_OFFSET                                       0x00000070
#define BMODEM_SYSCFG_DSSR_0_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSSR_0_RESERVED_MASK                                0x006C0000
#define    BMODEM_SYSCFG_DSSR_0_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSSR_0_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSSR_0_ISVALID_SHIFT                             23
#define    BMODEM_SYSCFG_DSSR_0_ISVALID_MASK                              0x00800000
#define    BMODEM_SYSCFG_DSSR_0_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSSR_0_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSSR_0_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSSR_0_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSSR_0_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSSR_0_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSSR_1_OFFSET                                       0x00000074
#define BMODEM_SYSCFG_DSSR_1_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSSR_1_RESERVED_MASK                                0x006C0000
#define    BMODEM_SYSCFG_DSSR_1_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSSR_1_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSSR_1_ISVALID_SHIFT                             23
#define    BMODEM_SYSCFG_DSSR_1_ISVALID_MASK                              0x00800000
#define    BMODEM_SYSCFG_DSSR_1_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSSR_1_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSSR_1_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSSR_1_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSSR_1_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSSR_1_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSSR_2_OFFSET                                       0x00000078
#define BMODEM_SYSCFG_DSSR_2_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSSR_2_RESERVED_MASK                                0x006C0000
#define    BMODEM_SYSCFG_DSSR_2_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSSR_2_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSSR_2_ISVALID_SHIFT                             23
#define    BMODEM_SYSCFG_DSSR_2_ISVALID_MASK                              0x00800000
#define    BMODEM_SYSCFG_DSSR_2_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSSR_2_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSSR_2_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSSR_2_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSSR_2_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSSR_2_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_DSSR_3_OFFSET                                       0x0000007C
#define BMODEM_SYSCFG_DSSR_3_TYPE                                         UInt32
#define BMODEM_SYSCFG_DSSR_3_RESERVED_MASK                                0x006C0000
#define    BMODEM_SYSCFG_DSSR_3_PLL_PLAN_SHIFT                            24
#define    BMODEM_SYSCFG_DSSR_3_PLL_PLAN_MASK                             0xFF000000
#define    BMODEM_SYSCFG_DSSR_3_ISVALID_SHIFT                             23
#define    BMODEM_SYSCFG_DSSR_3_ISVALID_MASK                              0x00800000
#define    BMODEM_SYSCFG_DSSR_3_MODE_SHIFT                                20
#define    BMODEM_SYSCFG_DSSR_3_MODE_MASK                                 0x00100000
#define    BMODEM_SYSCFG_DSSR_3_SLOTCNT_SHIFT                             14
#define    BMODEM_SYSCFG_DSSR_3_SLOTCNT_MASK                              0x0003C000
#define    BMODEM_SYSCFG_DSSR_3_QBC_SAMPLECNT_SHIFT                       0
#define    BMODEM_SYSCFG_DSSR_3_QBC_SAMPLECNT_MASK                        0x00003FFF

#define BMODEM_SYSCFG_INTCR_OFFSET                                        0x00000080
#define BMODEM_SYSCFG_INTCR_TYPE                                          UInt32
#define BMODEM_SYSCFG_INTCR_RESERVED_MASK                                 0xFFFFFF88
#define    BMODEM_SYSCFG_INTCR_INT_DATA_SWITCH_CLR_SHIFT                  6
#define    BMODEM_SYSCFG_INTCR_INT_DATA_SWITCH_CLR_MASK                   0x00000040
#define    BMODEM_SYSCFG_INTCR_INT_RFSPI_SWITCH_CLR_SHIFT                 5
#define    BMODEM_SYSCFG_INTCR_INT_RFSPI_SWITCH_CLR_MASK                  0x00000020
#define    BMODEM_SYSCFG_INTCR_INT_TWIF_SWITCH_CLR_SHIFT                  4
#define    BMODEM_SYSCFG_INTCR_INT_TWIF_SWITCH_CLR_MASK                   0x00000010
#define    BMODEM_SYSCFG_INTCR_INT_DATA_SWITCH_ST_SHIFT                   2
#define    BMODEM_SYSCFG_INTCR_INT_DATA_SWITCH_ST_MASK                    0x00000004
#define    BMODEM_SYSCFG_INTCR_INT_RFSPI_SWITCH_ST_SHIFT                  1
#define    BMODEM_SYSCFG_INTCR_INT_RFSPI_SWITCH_ST_MASK                   0x00000002
#define    BMODEM_SYSCFG_INTCR_INT_TWIF_SWITCH_ST_SHIFT                   0
#define    BMODEM_SYSCFG_INTCR_INT_TWIF_SWITCH_ST_MASK                    0x00000001

#define BMODEM_SYSCFG_RCR0_OFFSET                                         0x00000090
#define BMODEM_SYSCFG_RCR0_TYPE                                           UInt32
#define BMODEM_SYSCFG_RCR0_RESERVED_MASK                                  0xFFFFFF0E
#define    BMODEM_SYSCFG_RCR0_RGN_SIZE_SHIFT                              4
#define    BMODEM_SYSCFG_RCR0_RGN_SIZE_MASK                               0x000000F0
#define    BMODEM_SYSCFG_RCR0_RGN_REMAP_EN_SHIFT                          0
#define    BMODEM_SYSCFG_RCR0_RGN_REMAP_EN_MASK                           0x00000001

#define BMODEM_SYSCFG_RBAR0_OFFSET                                        0x00000094
#define BMODEM_SYSCFG_RBAR0_TYPE                                          UInt32
#define BMODEM_SYSCFG_RBAR0_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RBAR0_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RBAR0_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RBAR0_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RBAR0_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RRAR0_OFFSET                                        0x00000098
#define BMODEM_SYSCFG_RRAR0_TYPE                                          UInt32
#define BMODEM_SYSCFG_RRAR0_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RRAR0_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RRAR0_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RRAR0_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RRAR0_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RCR1_OFFSET                                         0x0000009C
#define BMODEM_SYSCFG_RCR1_TYPE                                           UInt32
#define BMODEM_SYSCFG_RCR1_RESERVED_MASK                                  0xFFFFFF0E
#define    BMODEM_SYSCFG_RCR1_RGN_SIZE_SHIFT                              4
#define    BMODEM_SYSCFG_RCR1_RGN_SIZE_MASK                               0x000000F0
#define    BMODEM_SYSCFG_RCR1_RGN_REMAP_EN_SHIFT                          0
#define    BMODEM_SYSCFG_RCR1_RGN_REMAP_EN_MASK                           0x00000001

#define BMODEM_SYSCFG_RBAR1_OFFSET                                        0x000000A0
#define BMODEM_SYSCFG_RBAR1_TYPE                                          UInt32
#define BMODEM_SYSCFG_RBAR1_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RBAR1_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RBAR1_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RBAR1_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RBAR1_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RRAR1_OFFSET                                        0x000000A4
#define BMODEM_SYSCFG_RRAR1_TYPE                                          UInt32
#define BMODEM_SYSCFG_RRAR1_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RRAR1_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RRAR1_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RRAR1_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RRAR1_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RCR2_OFFSET                                         0x000000A8
#define BMODEM_SYSCFG_RCR2_TYPE                                           UInt32
#define BMODEM_SYSCFG_RCR2_RESERVED_MASK                                  0xFFFFFF0E
#define    BMODEM_SYSCFG_RCR2_RGN_SIZE_SHIFT                              4
#define    BMODEM_SYSCFG_RCR2_RGN_SIZE_MASK                               0x000000F0
#define    BMODEM_SYSCFG_RCR2_RGN_REMAP_EN_SHIFT                          0
#define    BMODEM_SYSCFG_RCR2_RGN_REMAP_EN_MASK                           0x00000001

#define BMODEM_SYSCFG_RBAR2_OFFSET                                        0x000000AC
#define BMODEM_SYSCFG_RBAR2_TYPE                                          UInt32
#define BMODEM_SYSCFG_RBAR2_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RBAR2_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RBAR2_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RBAR2_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RBAR2_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RRAR2_OFFSET                                        0x000000B0
#define BMODEM_SYSCFG_RRAR2_TYPE                                          UInt32
#define BMODEM_SYSCFG_RRAR2_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RRAR2_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RRAR2_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RRAR2_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RRAR2_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RCR3_OFFSET                                         0x000000B4
#define BMODEM_SYSCFG_RCR3_TYPE                                           UInt32
#define BMODEM_SYSCFG_RCR3_RESERVED_MASK                                  0xFFFFFF0E
#define    BMODEM_SYSCFG_RCR3_RGN_SIZE_SHIFT                              4
#define    BMODEM_SYSCFG_RCR3_RGN_SIZE_MASK                               0x000000F0
#define    BMODEM_SYSCFG_RCR3_RGN_REMAP_EN_SHIFT                          0
#define    BMODEM_SYSCFG_RCR3_RGN_REMAP_EN_MASK                           0x00000001

#define BMODEM_SYSCFG_RBAR3_OFFSET                                        0x000000B8
#define BMODEM_SYSCFG_RBAR3_TYPE                                          UInt32
#define BMODEM_SYSCFG_RBAR3_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RBAR3_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RBAR3_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RBAR3_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RBAR3_RGN_BASE_ADDR_MASK                         0x7FFF0000

#define BMODEM_SYSCFG_RRAR3_OFFSET                                        0x000000BC
#define BMODEM_SYSCFG_RRAR3_TYPE                                          UInt32
#define BMODEM_SYSCFG_RRAR3_RESERVED_MASK                                 0x0000FFFF
#define    BMODEM_SYSCFG_RRAR3_RGN_BASE_ADDR_MSB_SHIFT                    31
#define    BMODEM_SYSCFG_RRAR3_RGN_BASE_ADDR_MSB_MASK                     0x80000000
#define    BMODEM_SYSCFG_RRAR3_RGN_BASE_ADDR_SHIFT                        16
#define    BMODEM_SYSCFG_RRAR3_RGN_BASE_ADDR_MASK                         0x7FFF0000

#endif /* __BRCM_RDB_BMODEM_SYSCFG_H__ */



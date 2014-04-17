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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_HSMCFG_H__
#define __BRCM_RDB_HSMCFG_H__

#define HSMCFG_SFTRST_OFFSET                                              0x00000000
#define HSMCFG_SFTRST_TYPE                                                UInt32
#define HSMCFG_SFTRST_RESERVED_MASK                                       0xFFFFF800
#define    HSMCFG_SFTRST_IPC_REG_SOFT_RSTN_SHIFT                          10
#define    HSMCFG_SFTRST_IPC_REG_SOFT_RSTN_MASK                           0x00000400
#define    HSMCFG_SFTRST_IPC_RAM_SOFT_RSTN_SHIFT                          9
#define    HSMCFG_SFTRST_IPC_RAM_SOFT_RSTN_MASK                           0x00000200
#define    HSMCFG_SFTRST_SRAM_SOFT_RSTN_SHIFT                             8
#define    HSMCFG_SFTRST_SRAM_SOFT_RSTN_MASK                              0x00000100
#define    HSMCFG_SFTRST_BROM_SOFT_RSTN_SHIFT                             7
#define    HSMCFG_SFTRST_BROM_SOFT_RSTN_MASK                              0x00000080
#define    HSMCFG_SFTRST_O_SPUM_KC_SOFT_RSTN_SHIFT                        6
#define    HSMCFG_SFTRST_O_SPUM_KC_SOFT_RSTN_MASK                         0x00000040
#define    HSMCFG_SFTRST_S_SPUM_KC_SOFT_RSTN_SHIFT                        5
#define    HSMCFG_SFTRST_S_SPUM_KC_SOFT_RSTN_MASK                         0x00000020
#define    HSMCFG_SFTRST_WD_SOFT_RSTN_SHIFT                               4
#define    HSMCFG_SFTRST_WD_SOFT_RSTN_MASK                                0x00000010
#define    HSMCFG_SFTRST_KEK_SOFT_RSTN_SHIFT                              3
#define    HSMCFG_SFTRST_KEK_SOFT_RSTN_MASK                               0x00000008
#define    HSMCFG_SFTRST_RNG_SOFT_RSTN_SHIFT                              2
#define    HSMCFG_SFTRST_RNG_SOFT_RSTN_MASK                               0x00000004
#define    HSMCFG_SFTRST_OTP_SOFT_RSTN_SHIFT                              1
#define    HSMCFG_SFTRST_OTP_SOFT_RSTN_MASK                               0x00000002
#define    HSMCFG_SFTRST_PKA_SOFT_RSTN_SHIFT                              0
#define    HSMCFG_SFTRST_PKA_SOFT_RSTN_MASK                               0x00000001

#define HSMCFG_CLKEN_OFFSET                                               0x00000004
#define HSMCFG_CLKEN_TYPE                                                 UInt32
#define HSMCFG_CLKEN_RESERVED_MASK                                        0xFFFFF800
#define    HSMCFG_CLKEN_IPC_REG_CLK_EN_SHIFT                              10
#define    HSMCFG_CLKEN_IPC_REG_CLK_EN_MASK                               0x00000400
#define    HSMCFG_CLKEN_IPC_RAM_CLK_EN_SHIFT                              9
#define    HSMCFG_CLKEN_IPC_RAM_CLK_EN_MASK                               0x00000200
#define    HSMCFG_CLKEN_SRAM_CLK_EN_SHIFT                                 8
#define    HSMCFG_CLKEN_SRAM_CLK_EN_MASK                                  0x00000100
#define    HSMCFG_CLKEN_BROM_CLK_EN_SHIFT                                 7
#define    HSMCFG_CLKEN_BROM_CLK_EN_MASK                                  0x00000080
#define    HSMCFG_CLKEN_O_SPUM_KC_CLK_EN_SHIFT                            6
#define    HSMCFG_CLKEN_O_SPUM_KC_CLK_EN_MASK                             0x00000040
#define    HSMCFG_CLKEN_S_SPUM_KC_CLK_EN_SHIFT                            5
#define    HSMCFG_CLKEN_S_SPUM_KC_CLK_EN_MASK                             0x00000020
#define    HSMCFG_CLKEN_WD_CLK_EN_SHIFT                                   4
#define    HSMCFG_CLKEN_WD_CLK_EN_MASK                                    0x00000010
#define    HSMCFG_CLKEN_KEK_CLK_EN_SHIFT                                  3
#define    HSMCFG_CLKEN_KEK_CLK_EN_MASK                                   0x00000008
#define    HSMCFG_CLKEN_RNG_CLK_EN_SHIFT                                  2
#define    HSMCFG_CLKEN_RNG_CLK_EN_MASK                                   0x00000004
#define    HSMCFG_CLKEN_OTP_CLK_EN_SHIFT                                  1
#define    HSMCFG_CLKEN_OTP_CLK_EN_MASK                                   0x00000002
#define    HSMCFG_CLKEN_PKA_CLK_EN_SHIFT                                  0
#define    HSMCFG_CLKEN_PKA_CLK_EN_MASK                                   0x00000001

#define HSMCFG_PWRMODE_OFFSET                                             0x00000008
#define HSMCFG_PWRMODE_TYPE                                               UInt32
#define HSMCFG_PWRMODE_RESERVED_MASK                                      0xFFFFFFFC
#define    HSMCFG_PWRMODE_HSM_PWRMODE_SHIFT                               0
#define    HSMCFG_PWRMODE_HSM_PWRMODE_MASK                                0x00000003

#define HSMCFG_OTP_RD_ACCESS_OFFSET                                       0x0000000C
#define HSMCFG_OTP_RD_ACCESS_TYPE                                         UInt32
#define HSMCFG_OTP_RD_ACCESS_RESERVED_MASK                                0x00000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_31_SHIFT                   31
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_31_MASK                    0x80000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_30_SHIFT                   30
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_30_MASK                    0x40000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_29_SHIFT                   29
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_29_MASK                    0x20000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_28_SHIFT                   28
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_28_MASK                    0x10000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_27_SHIFT                   27
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_27_MASK                    0x08000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_26_SHIFT                   26
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_26_MASK                    0x04000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_25_SHIFT                   25
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_25_MASK                    0x02000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_24_SHIFT                   24
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_24_MASK                    0x01000000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_23_SHIFT                   23
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_23_MASK                    0x00800000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_22_SHIFT                   22
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_22_MASK                    0x00400000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_21_SHIFT                   21
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_21_MASK                    0x00200000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_20_SHIFT                   20
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_20_MASK                    0x00100000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_19_SHIFT                   19
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_19_MASK                    0x00080000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_18_SHIFT                   18
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_18_MASK                    0x00040000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_17_SHIFT                   17
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_17_MASK                    0x00020000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_16_SHIFT                   16
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_16_MASK                    0x00010000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_15_SHIFT                   15
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_15_MASK                    0x00008000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_14_SHIFT                   14
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_14_MASK                    0x00004000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_13_SHIFT                   13
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_13_MASK                    0x00002000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_12_SHIFT                   12
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_12_MASK                    0x00001000
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_11_SHIFT                   11
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_11_MASK                    0x00000800
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_10_SHIFT                   10
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_10_MASK                    0x00000400
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_9_SHIFT                    9
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_9_MASK                     0x00000200
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_8_SHIFT                    8
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_8_MASK                     0x00000100
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_7_SHIFT                    7
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_7_MASK                     0x00000080
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_6_SHIFT                    6
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_6_MASK                     0x00000040
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_5_SHIFT                    5
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_5_MASK                     0x00000020
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_4_SHIFT                    4
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_4_MASK                     0x00000010
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_3_SHIFT                    3
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_3_MASK                     0x00000008
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_2_SHIFT                    2
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_2_MASK                     0x00000004
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_1_SHIFT                    1
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_1_MASK                     0x00000002
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_0_SHIFT                    0
#define    HSMCFG_OTP_RD_ACCESS_DIS_OTP_RGN_RD_0_MASK                     0x00000001

#define HSMCFG_OTP_WR_ACCESS_OFFSET                                       0x00000010
#define HSMCFG_OTP_WR_ACCESS_TYPE                                         UInt32
#define HSMCFG_OTP_WR_ACCESS_RESERVED_MASK                                0x00000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_31_SHIFT                   31
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_31_MASK                    0x80000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_30_SHIFT                   30
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_30_MASK                    0x40000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_29_SHIFT                   29
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_29_MASK                    0x20000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_28_SHIFT                   28
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_28_MASK                    0x10000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_27_SHIFT                   27
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_27_MASK                    0x08000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_26_SHIFT                   26
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_26_MASK                    0x04000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_25_SHIFT                   25
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_25_MASK                    0x02000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_24_SHIFT                   24
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_24_MASK                    0x01000000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_23_SHIFT                   23
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_23_MASK                    0x00800000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_22_SHIFT                   22
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_22_MASK                    0x00400000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_21_SHIFT                   21
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_21_MASK                    0x00200000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_20_SHIFT                   20
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_20_MASK                    0x00100000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_19_SHIFT                   19
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_19_MASK                    0x00080000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_18_SHIFT                   18
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_18_MASK                    0x00040000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_17_SHIFT                   17
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_17_MASK                    0x00020000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_16_SHIFT                   16
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_16_MASK                    0x00010000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_15_SHIFT                   15
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_15_MASK                    0x00008000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_14_SHIFT                   14
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_14_MASK                    0x00004000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_13_SHIFT                   13
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_13_MASK                    0x00002000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_12_SHIFT                   12
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_12_MASK                    0x00001000
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_11_SHIFT                   11
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_11_MASK                    0x00000800
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_10_SHIFT                   10
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_10_MASK                    0x00000400
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_9_SHIFT                    9
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_9_MASK                     0x00000200
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_8_SHIFT                    8
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_8_MASK                     0x00000100
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_7_SHIFT                    7
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_7_MASK                     0x00000080
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_6_SHIFT                    6
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_6_MASK                     0x00000040
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_5_SHIFT                    5
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_5_MASK                     0x00000020
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_4_SHIFT                    4
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_4_MASK                     0x00000010
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_3_SHIFT                    3
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_3_MASK                     0x00000008
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_2_SHIFT                    2
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_2_MASK                     0x00000004
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_1_SHIFT                    1
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_1_MASK                     0x00000002
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_0_SHIFT                    0
#define    HSMCFG_OTP_WR_ACCESS_DIS_OTP_RGN_WR_0_MASK                     0x00000001

#endif /* __BRCM_RDB_HSMCFG_H__ */



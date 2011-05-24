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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_NVSRAM_AXI_H__
#define __BRCM_RDB_NVSRAM_AXI_H__

#define NVSRAM_AXI_MEMC_STATUS_OFFSET                                     0x00000000
#define NVSRAM_AXI_MEMC_STATUS_TYPE                                       UInt32
#define NVSRAM_AXI_MEMC_STATUS_RESERVED_MASK                              0xFFFFE000
#define    NVSRAM_AXI_MEMC_STATUS_RAW_ECC_INT1_SHIFT                      12
#define    NVSRAM_AXI_MEMC_STATUS_RAW_ECC_INT1_MASK                       0x00001000
#define    NVSRAM_AXI_MEMC_STATUS_RAW_ECC_INT0_SHIFT                      11
#define    NVSRAM_AXI_MEMC_STATUS_RAW_ECC_INT0_MASK                       0x00000800
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT1_SHIFT                          10
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT1_MASK                           0x00000400
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT0_SHIFT                          9
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT0_MASK                           0x00000200
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT1_EN_SHIFT                       8
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT1_EN_MASK                        0x00000100
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT0_EN_SHIFT                       7
#define    NVSRAM_AXI_MEMC_STATUS_ECC_INT0_EN_MASK                        0x00000080
#define    NVSRAM_AXI_MEMC_STATUS_RAW_INT1_STATUS_SHIFT                   6
#define    NVSRAM_AXI_MEMC_STATUS_RAW_INT1_STATUS_MASK                    0x00000040
#define    NVSRAM_AXI_MEMC_STATUS_RAW_INT0_STATUS_SHIFT                   5
#define    NVSRAM_AXI_MEMC_STATUS_RAW_INT0_STATUS_MASK                    0x00000020
#define    NVSRAM_AXI_MEMC_STATUS_INT_STATUS1_SHIFT                       4
#define    NVSRAM_AXI_MEMC_STATUS_INT_STATUS1_MASK                        0x00000010
#define    NVSRAM_AXI_MEMC_STATUS_INT_STATUS0_SHIFT                       3
#define    NVSRAM_AXI_MEMC_STATUS_INT_STATUS0_MASK                        0x00000008
#define    NVSRAM_AXI_MEMC_STATUS_INT_EN1_SHIFT                           2
#define    NVSRAM_AXI_MEMC_STATUS_INT_EN1_MASK                            0x00000004
#define    NVSRAM_AXI_MEMC_STATUS_INT_EN0_SHIFT                           1
#define    NVSRAM_AXI_MEMC_STATUS_INT_EN0_MASK                            0x00000002
#define    NVSRAM_AXI_MEMC_STATUS_STATE_SHIFT                             0
#define    NVSRAM_AXI_MEMC_STATUS_STATE_MASK                              0x00000001

#define NVSRAM_AXI_MEMIF_CFG_OFFSET                                       0x00000004
#define NVSRAM_AXI_MEMIF_CFG_TYPE                                         UInt32
#define NVSRAM_AXI_MEMIF_CFG_RESERVED_MASK                                0xFFFC8080
#define    NVSRAM_AXI_MEMIF_CFG_EXCLUSIVE_MONITORS_SHIFT                  16
#define    NVSRAM_AXI_MEMIF_CFG_EXCLUSIVE_MONITORS_MASK                   0x00030000
#define    NVSRAM_AXI_MEMIF_CFG_REMAP1_SHIFT                              14
#define    NVSRAM_AXI_MEMIF_CFG_REMAP1_MASK                               0x00004000
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_WIDTH1_SHIFT                       12
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_WIDTH1_MASK                        0x00003000
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_CHIPS1_SHIFT                       10
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_CHIPS1_MASK                        0x00000C00
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_TYPE1_SHIFT                        8
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_TYPE1_MASK                         0x00000300
#define    NVSRAM_AXI_MEMIF_CFG_REMAP0_SHIFT                              6
#define    NVSRAM_AXI_MEMIF_CFG_REMAP0_MASK                               0x00000040
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_WIDTH0_SHIFT                       4
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_WIDTH0_MASK                        0x00000030
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_CHIPS0_SHIFT                       2
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_CHIPS0_MASK                        0x0000000C
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_TYPE0_SHIFT                        0
#define    NVSRAM_AXI_MEMIF_CFG_MEMORY_TYPE0_MASK                         0x00000003

#define NVSRAM_AXI_MEMC_CFG_SET_OFFSET                                    0x00000008
#define NVSRAM_AXI_MEMC_CFG_SET_TYPE                                      UInt32
#define NVSRAM_AXI_MEMC_CFG_SET_RESERVED_MASK                             0xFFFFFF98
#define    NVSRAM_AXI_MEMC_CFG_SET_ECC_INT_ENABLE1_SHIFT                  6
#define    NVSRAM_AXI_MEMC_CFG_SET_ECC_INT_ENABLE1_MASK                   0x00000040
#define    NVSRAM_AXI_MEMC_CFG_SET_ECC_INT_ENABLE0_SHIFT                  5
#define    NVSRAM_AXI_MEMC_CFG_SET_ECC_INT_ENABLE0_MASK                   0x00000020
#define    NVSRAM_AXI_MEMC_CFG_SET_LOW_POWER_REQ_SHIFT                    2
#define    NVSRAM_AXI_MEMC_CFG_SET_LOW_POWER_REQ_MASK                     0x00000004
#define    NVSRAM_AXI_MEMC_CFG_SET_INT_ENABLE1_SHIFT                      1
#define    NVSRAM_AXI_MEMC_CFG_SET_INT_ENABLE1_MASK                       0x00000002
#define    NVSRAM_AXI_MEMC_CFG_SET_INT_ENABLE0_SHIFT                      0
#define    NVSRAM_AXI_MEMC_CFG_SET_INT_ENABLE0_MASK                       0x00000001

#define NVSRAM_AXI_MEMC_CFG_CLR_OFFSET                                    0x0000000C
#define NVSRAM_AXI_MEMC_CFG_CLR_TYPE                                      UInt32
#define NVSRAM_AXI_MEMC_CFG_CLR_RESERVED_MASK                             0xFFFFFF80
#define    NVSRAM_AXI_MEMC_CFG_CLR_ECC_INT_DISABLE1_SHIFT                 6
#define    NVSRAM_AXI_MEMC_CFG_CLR_ECC_INT_DISABLE1_MASK                  0x00000040
#define    NVSRAM_AXI_MEMC_CFG_CLR_ECC_INT_DISABLE0_SHIFT                 5
#define    NVSRAM_AXI_MEMC_CFG_CLR_ECC_INT_DISABLE0_MASK                  0x00000020
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_CLR_1_SHIFT                        4
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_CLR_1_MASK                         0x00000010
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_CLR_0_SHIFT                        3
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_CLR_0_MASK                         0x00000008
#define    NVSRAM_AXI_MEMC_CFG_CLR_LOW_POWER_EXIT_SHIFT                   2
#define    NVSRAM_AXI_MEMC_CFG_CLR_LOW_POWER_EXIT_MASK                    0x00000004
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_DISABLE1_SHIFT                     1
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_DISABLE1_MASK                      0x00000002
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_DISABLE0_SHIFT                     0
#define    NVSRAM_AXI_MEMC_CFG_CLR_INT_DISABLE0_MASK                      0x00000001

#define NVSRAM_AXI_DIRECT_CMD_OFFSET                                      0x00000010
#define NVSRAM_AXI_DIRECT_CMD_TYPE                                        UInt32
#define NVSRAM_AXI_DIRECT_CMD_RESERVED_MASK                               0xFC000000
#define    NVSRAM_AXI_DIRECT_CMD_CHIP_SELECT_SHIFT                        23
#define    NVSRAM_AXI_DIRECT_CMD_CHIP_SELECT_MASK                         0x03800000
#define    NVSRAM_AXI_DIRECT_CMD_CMD_TYPE_SHIFT                           21
#define    NVSRAM_AXI_DIRECT_CMD_CMD_TYPE_MASK                            0x00600000
#define    NVSRAM_AXI_DIRECT_CMD_SET_CRE_SHIFT                            20
#define    NVSRAM_AXI_DIRECT_CMD_SET_CRE_MASK                             0x00100000
#define    NVSRAM_AXI_DIRECT_CMD_ADDR_SHIFT                               0
#define    NVSRAM_AXI_DIRECT_CMD_ADDR_MASK                                0x000FFFFF

#define NVSRAM_AXI_SET_CYCLES_OFFSET                                      0x00000014
#define NVSRAM_AXI_SET_CYCLES_TYPE                                        UInt32
#define NVSRAM_AXI_SET_CYCLES_RESERVED_MASK                               0xFF000000
#define    NVSRAM_AXI_SET_CYCLES_SET_T6_SHIFT                             20
#define    NVSRAM_AXI_SET_CYCLES_SET_T6_MASK                              0x00F00000
#define    NVSRAM_AXI_SET_CYCLES_SET_T5_SHIFT                             17
#define    NVSRAM_AXI_SET_CYCLES_SET_T5_MASK                              0x000E0000
#define    NVSRAM_AXI_SET_CYCLES_SET_T4_SHIFT                             14
#define    NVSRAM_AXI_SET_CYCLES_SET_T4_MASK                              0x0001C000
#define    NVSRAM_AXI_SET_CYCLES_SET_T3_SHIFT                             11
#define    NVSRAM_AXI_SET_CYCLES_SET_T3_MASK                              0x00003800
#define    NVSRAM_AXI_SET_CYCLES_SET_T2_SHIFT                             8
#define    NVSRAM_AXI_SET_CYCLES_SET_T2_MASK                              0x00000700
#define    NVSRAM_AXI_SET_CYCLES_SET_T1_SHIFT                             4
#define    NVSRAM_AXI_SET_CYCLES_SET_T1_MASK                              0x000000F0
#define    NVSRAM_AXI_SET_CYCLES_SET_T0_SHIFT                             0
#define    NVSRAM_AXI_SET_CYCLES_SET_T0_MASK                              0x0000000F

#define NVSRAM_AXI_SET_OPMODE_OFFSET                                      0x00000018
#define NVSRAM_AXI_SET_OPMODE_TYPE                                        UInt32
#define NVSRAM_AXI_SET_OPMODE_RESERVED_MASK                               0xFFFF0000
#define    NVSRAM_AXI_SET_OPMODE_SET_BURST_ALIGN_SHIFT                    13
#define    NVSRAM_AXI_SET_OPMODE_SET_BURST_ALIGN_MASK                     0x0000E000
#define    NVSRAM_AXI_SET_OPMODE_SET_BLS_SHIFT                            12
#define    NVSRAM_AXI_SET_OPMODE_SET_BLS_MASK                             0x00001000
#define    NVSRAM_AXI_SET_OPMODE_SET_ADV_SHIFT                            11
#define    NVSRAM_AXI_SET_OPMODE_SET_ADV_MASK                             0x00000800
#define    NVSRAM_AXI_SET_OPMODE_SET_BAA_SHIFT                            10
#define    NVSRAM_AXI_SET_OPMODE_SET_BAA_MASK                             0x00000400
#define    NVSRAM_AXI_SET_OPMODE_SET_WR_BL_SHIFT                          7
#define    NVSRAM_AXI_SET_OPMODE_SET_WR_BL_MASK                           0x00000380
#define    NVSRAM_AXI_SET_OPMODE_SET_WR_SYNC_SHIFT                        6
#define    NVSRAM_AXI_SET_OPMODE_SET_WR_SYNC_MASK                         0x00000040
#define    NVSRAM_AXI_SET_OPMODE_SET_RD_BL_SHIFT                          3
#define    NVSRAM_AXI_SET_OPMODE_SET_RD_BL_MASK                           0x00000038
#define    NVSRAM_AXI_SET_OPMODE_SET_RD_SYNC_SHIFT                        2
#define    NVSRAM_AXI_SET_OPMODE_SET_RD_SYNC_MASK                         0x00000004
#define    NVSRAM_AXI_SET_OPMODE_SET_MW_SHIFT                             0
#define    NVSRAM_AXI_SET_OPMODE_SET_MW_MASK                              0x00000003

#define NVSRAM_AXI_REFRESH_PERIOD_0_OFFSET                                0x00000020
#define NVSRAM_AXI_REFRESH_PERIOD_0_TYPE                                  UInt32
#define NVSRAM_AXI_REFRESH_PERIOD_0_RESERVED_MASK                         0xFFFFFFF0
#define    NVSRAM_AXI_REFRESH_PERIOD_0_PERIOD_SHIFT                       0
#define    NVSRAM_AXI_REFRESH_PERIOD_0_PERIOD_MASK                        0x0000000F

#define NVSRAM_AXI_REFRESH_PERIOD_1_OFFSET                                0x00000024
#define NVSRAM_AXI_REFRESH_PERIOD_1_TYPE                                  UInt32
#define NVSRAM_AXI_REFRESH_PERIOD_1_RESERVED_MASK                         0xFFFFFFF0
#define    NVSRAM_AXI_REFRESH_PERIOD_1_PERIOD_SHIFT                       0
#define    NVSRAM_AXI_REFRESH_PERIOD_1_PERIOD_MASK                        0x0000000F

#define NVSRAM_AXI_SRAM_CYCLES_CS0_OFFSET                                 0x00000100
#define NVSRAM_AXI_SRAM_CYCLES_CS0_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_CYCLES_CS0_RESERVED_MASK                          0xFFE00000
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_WE_TIME_SHIFT                       20
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_WE_TIME_MASK                        0x00100000
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_TR_SHIFT                          17
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_TR_MASK                           0x000E0000
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_PC_SHIFT                          14
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_PC_MASK                           0x0001C000
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_WP_SHIFT                          11
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_WP_MASK                           0x00003800
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_CEOE_SHIFT                        8
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_CEOE_MASK                         0x00000700
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_WC_SHIFT                          4
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_WC_MASK                           0x000000F0
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_RC_SHIFT                          0
#define    NVSRAM_AXI_SRAM_CYCLES_CS0_T_RC_MASK                           0x0000000F

#define NVSRAM_AXI_SRAM_OPMODE_CS0_OFFSET                                 0x00000104
#define NVSRAM_AXI_SRAM_OPMODE_CS0_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_OPMODE_CS0_RESERVED_MASK                          0x00000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_ADDRESS_MATCH_SHIFT                 24
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_ADDRESS_MATCH_MASK                  0xFF000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_ADDRESS_MASK_SHIFT                  16
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_ADDRESS_MASK_MASK                   0x00FF0000
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_BURST_ALIGN_SHIFT                   13
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_BURST_ALIGN_MASK                    0x0000E000
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_BLS_SHIFT                           12
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_BLS_MASK                            0x00001000
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_ADV_SHIFT                           11
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_ADV_MASK                            0x00000800
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_BAA_SHIFT                           10
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_BAA_MASK                            0x00000400
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_WR_BL_SHIFT                         7
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_WR_BL_MASK                          0x00000380
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_WR_SYNC_SHIFT                       6
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_WR_SYNC_MASK                        0x00000040
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_RD_BL_SHIFT                         3
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_RD_BL_MASK                          0x00000038
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_RD_SYNC_SHIFT                       2
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_RD_SYNC_MASK                        0x00000004
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_MW_SHIFT                            0
#define    NVSRAM_AXI_SRAM_OPMODE_CS0_MW_MASK                             0x00000003

#define NVSRAM_AXI_SRAM_CYCLES_CS1_OFFSET                                 0x00000120
#define NVSRAM_AXI_SRAM_CYCLES_CS1_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_CYCLES_CS1_RESERVED_MASK                          0xFFE00000
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_WE_TIME_SHIFT                       20
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_WE_TIME_MASK                        0x00100000
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_TR_SHIFT                          17
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_TR_MASK                           0x000E0000
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_PC_SHIFT                          14
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_PC_MASK                           0x0001C000
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_WP_SHIFT                          11
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_WP_MASK                           0x00003800
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_CEOE_SHIFT                        8
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_CEOE_MASK                         0x00000700
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_WC_SHIFT                          4
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_WC_MASK                           0x000000F0
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_RC_SHIFT                          0
#define    NVSRAM_AXI_SRAM_CYCLES_CS1_T_RC_MASK                           0x0000000F

#define NVSRAM_AXI_SRAM_OPMODE_CS1_OFFSET                                 0x00000124
#define NVSRAM_AXI_SRAM_OPMODE_CS1_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_OPMODE_CS1_RESERVED_MASK                          0x00000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_ADDRESS_MATCH_SHIFT                 24
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_ADDRESS_MATCH_MASK                  0xFF000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_ADDRESS_MASK_SHIFT                  16
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_ADDRESS_MASK_MASK                   0x00FF0000
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_BURST_ALIGN_SHIFT                   13
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_BURST_ALIGN_MASK                    0x0000E000
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_BLS_SHIFT                           12
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_BLS_MASK                            0x00001000
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_ADV_SHIFT                           11
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_ADV_MASK                            0x00000800
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_BAA_SHIFT                           10
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_BAA_MASK                            0x00000400
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_WR_BL_SHIFT                         7
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_WR_BL_MASK                          0x00000380
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_WR_SYNC_SHIFT                       6
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_WR_SYNC_MASK                        0x00000040
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_RD_BL_SHIFT                         3
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_RD_BL_MASK                          0x00000038
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_RD_SYNC_SHIFT                       2
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_RD_SYNC_MASK                        0x00000004
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_MW_SHIFT                            0
#define    NVSRAM_AXI_SRAM_OPMODE_CS1_MW_MASK                             0x00000003

#define NVSRAM_AXI_SRAM_CYCLES_CS2_OFFSET                                 0x00000140
#define NVSRAM_AXI_SRAM_CYCLES_CS2_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_CYCLES_CS2_RESERVED_MASK                          0xFFE00000
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_WE_TIME_SHIFT                       20
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_WE_TIME_MASK                        0x00100000
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_TR_SHIFT                          17
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_TR_MASK                           0x000E0000
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_PC_SHIFT                          14
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_PC_MASK                           0x0001C000
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_WP_SHIFT                          11
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_WP_MASK                           0x00003800
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_CEOE_SHIFT                        8
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_CEOE_MASK                         0x00000700
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_WC_SHIFT                          4
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_WC_MASK                           0x000000F0
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_RC_SHIFT                          0
#define    NVSRAM_AXI_SRAM_CYCLES_CS2_T_RC_MASK                           0x0000000F

#define NVSRAM_AXI_SRAM_OPMODE_CS2_OFFSET                                 0x00000144
#define NVSRAM_AXI_SRAM_OPMODE_CS2_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_OPMODE_CS2_RESERVED_MASK                          0x00000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_ADDRESS_MATCH_SHIFT                 24
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_ADDRESS_MATCH_MASK                  0xFF000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_ADDRESS_MASK_SHIFT                  16
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_ADDRESS_MASK_MASK                   0x00FF0000
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_BURST_ALIGN_SHIFT                   13
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_BURST_ALIGN_MASK                    0x0000E000
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_BLS_SHIFT                           12
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_BLS_MASK                            0x00001000
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_ADV_SHIFT                           11
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_ADV_MASK                            0x00000800
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_BAA_SHIFT                           10
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_BAA_MASK                            0x00000400
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_WR_BL_SHIFT                         7
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_WR_BL_MASK                          0x00000380
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_WR_SYNC_SHIFT                       6
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_WR_SYNC_MASK                        0x00000040
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_RD_BL_SHIFT                         3
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_RD_BL_MASK                          0x00000038
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_RD_SYNC_SHIFT                       2
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_RD_SYNC_MASK                        0x00000004
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_MW_SHIFT                            0
#define    NVSRAM_AXI_SRAM_OPMODE_CS2_MW_MASK                             0x00000003

#define NVSRAM_AXI_SRAM_CYCLES_CS3_OFFSET                                 0x00000160
#define NVSRAM_AXI_SRAM_CYCLES_CS3_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_CYCLES_CS3_RESERVED_MASK                          0xFFE00000
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_WE_TIME_SHIFT                       20
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_WE_TIME_MASK                        0x00100000
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_TR_SHIFT                          17
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_TR_MASK                           0x000E0000
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_PC_SHIFT                          14
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_PC_MASK                           0x0001C000
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_WP_SHIFT                          11
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_WP_MASK                           0x00003800
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_CEOE_SHIFT                        8
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_CEOE_MASK                         0x00000700
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_WC_SHIFT                          4
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_WC_MASK                           0x000000F0
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_RC_SHIFT                          0
#define    NVSRAM_AXI_SRAM_CYCLES_CS3_T_RC_MASK                           0x0000000F

#define NVSRAM_AXI_SRAM_OPMODE_CS3_OFFSET                                 0x00000164
#define NVSRAM_AXI_SRAM_OPMODE_CS3_TYPE                                   UInt32
#define NVSRAM_AXI_SRAM_OPMODE_CS3_RESERVED_MASK                          0x00000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_ADDRESS_MATCH_SHIFT                 24
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_ADDRESS_MATCH_MASK                  0xFF000000
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_ADDRESS_MASK_SHIFT                  16
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_ADDRESS_MASK_MASK                   0x00FF0000
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_BURST_ALIGN_SHIFT                   13
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_BURST_ALIGN_MASK                    0x0000E000
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_BLS_SHIFT                           12
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_BLS_MASK                            0x00001000
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_ADV_SHIFT                           11
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_ADV_MASK                            0x00000800
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_BAA_SHIFT                           10
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_BAA_MASK                            0x00000400
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_WR_BL_SHIFT                         7
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_WR_BL_MASK                          0x00000380
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_WR_SYNC_SHIFT                       6
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_WR_SYNC_MASK                        0x00000040
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_RD_BL_SHIFT                         3
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_RD_BL_MASK                          0x00000038
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_RD_SYNC_SHIFT                       2
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_RD_SYNC_MASK                        0x00000004
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_MW_SHIFT                            0
#define    NVSRAM_AXI_SRAM_OPMODE_CS3_MW_MASK                             0x00000003

#define NVSRAM_AXI_USER_STATUS_OFFSET                                     0x00000200
#define NVSRAM_AXI_USER_STATUS_TYPE                                       UInt32
#define NVSRAM_AXI_USER_STATUS_RESERVED_MASK                              0xFFFFFF00
#define    NVSRAM_AXI_USER_STATUS_USER_STATUS_SHIFT                       0
#define    NVSRAM_AXI_USER_STATUS_USER_STATUS_MASK                        0x000000FF

#define NVSRAM_AXI_USER_CONFIG_OFFSET                                     0x00000204
#define NVSRAM_AXI_USER_CONFIG_TYPE                                       UInt32
#define NVSRAM_AXI_USER_CONFIG_RESERVED_MASK                              0xFFFFFF00
#define    NVSRAM_AXI_USER_CONFIG_USER_CONFIG_SHIFT                       0
#define    NVSRAM_AXI_USER_CONFIG_USER_CONFIG_MASK                        0x000000FF

#define NVSRAM_AXI_INT_CFG_OFFSET                                         0x00000E00
#define NVSRAM_AXI_INT_CFG_TYPE                                           UInt32
#define NVSRAM_AXI_INT_CFG_RESERVED_MASK                                  0xFFFFFFFE
#define    NVSRAM_AXI_INT_CFG_INT_TEST_EN_SHIFT                           0
#define    NVSRAM_AXI_INT_CFG_INT_TEST_EN_MASK                            0x00000001

#define NVSRAM_AXI_INT_INPUTS_OFFSET                                      0x00000E04
#define NVSRAM_AXI_INT_INPUTS_TYPE                                        UInt32
#define NVSRAM_AXI_INT_INPUTS_RESERVED_MASK                               0xFFFFFC00
#define    NVSRAM_AXI_INT_INPUTS_MSYNC1_SHIFT                             9
#define    NVSRAM_AXI_INT_INPUTS_MSYNC1_MASK                              0x00000200
#define    NVSRAM_AXI_INT_INPUTS_ASYNC1_SHIFT                             8
#define    NVSRAM_AXI_INT_INPUTS_ASYNC1_MASK                              0x00000100
#define    NVSRAM_AXI_INT_INPUTS_EBIBACKOFF1_SHIFT                        7
#define    NVSRAM_AXI_INT_INPUTS_EBIBACKOFF1_MASK                         0x00000080
#define    NVSRAM_AXI_INT_INPUTS_EBIGNT1_SHIFT                            6
#define    NVSRAM_AXI_INT_INPUTS_EBIGNT1_MASK                             0x00000040
#define    NVSRAM_AXI_INT_INPUTS_MSYNC0_SHIFT                             5
#define    NVSRAM_AXI_INT_INPUTS_MSYNC0_MASK                              0x00000020
#define    NVSRAM_AXI_INT_INPUTS_ASYNC0_SHIFT                             4
#define    NVSRAM_AXI_INT_INPUTS_ASYNC0_MASK                              0x00000010
#define    NVSRAM_AXI_INT_INPUTS_EBIBACKOFF0_SHIFT                        3
#define    NVSRAM_AXI_INT_INPUTS_EBIBACKOFF0_MASK                         0x00000008
#define    NVSRAM_AXI_INT_INPUTS_EBIGNT0_SHIFT                            2
#define    NVSRAM_AXI_INT_INPUTS_EBIGNT0_MASK                             0x00000004
#define    NVSRAM_AXI_INT_INPUTS_USE_EBI_SHIFT                            1
#define    NVSRAM_AXI_INT_INPUTS_USE_EBI_MASK                             0x00000002
#define    NVSRAM_AXI_INT_INPUTS_CSYSREQ_SHIFT                            0
#define    NVSRAM_AXI_INT_INPUTS_CSYSREQ_MASK                             0x00000001

#define NVSRAM_AXI_INT_OUTPUTS_OFFSET                                     0x00000E08
#define NVSRAM_AXI_INT_OUTPUTS_TYPE                                       UInt32
#define NVSRAM_AXI_INT_OUTPUTS_RESERVED_MASK                              0xFFFFFE00
#define    NVSRAM_AXI_INT_OUTPUTS_ECC_INT1_SHIFT                          8
#define    NVSRAM_AXI_INT_OUTPUTS_ECC_INT1_MASK                           0x00000100
#define    NVSRAM_AXI_INT_OUTPUTS_ECC_INT0_SHIFT                          7
#define    NVSRAM_AXI_INT_OUTPUTS_ECC_INT0_MASK                           0x00000080
#define    NVSRAM_AXI_INT_OUTPUTS_SMC_INT1_SHIFT                          6
#define    NVSRAM_AXI_INT_OUTPUTS_SMC_INT1_MASK                           0x00000040
#define    NVSRAM_AXI_INT_OUTPUTS_SMC_INT0_SHIFT                          5
#define    NVSRAM_AXI_INT_OUTPUTS_SMC_INT0_MASK                           0x00000020
#define    NVSRAM_AXI_INT_OUTPUTS_SMC_INT_SHIFT                           4
#define    NVSRAM_AXI_INT_OUTPUTS_SMC_INT_MASK                            0x00000010
#define    NVSRAM_AXI_INT_OUTPUTS_EBIREQ1_SHIFT                           3
#define    NVSRAM_AXI_INT_OUTPUTS_EBIREQ1_MASK                            0x00000008
#define    NVSRAM_AXI_INT_OUTPUTS_EBIREQ0_SHIFT                           2
#define    NVSRAM_AXI_INT_OUTPUTS_EBIREQ0_MASK                            0x00000004
#define    NVSRAM_AXI_INT_OUTPUTS_CSYSACK_SHIFT                           1
#define    NVSRAM_AXI_INT_OUTPUTS_CSYSACK_MASK                            0x00000002
#define    NVSRAM_AXI_INT_OUTPUTS_CACTIVE_SHIFT                           0
#define    NVSRAM_AXI_INT_OUTPUTS_CACTIVE_MASK                            0x00000001

#define NVSRAM_AXI_ADDR_MASK_OFFSET                                       0x00000E20
#define NVSRAM_AXI_ADDR_MASK_TYPE                                         UInt32
#define NVSRAM_AXI_ADDR_MASK_RESERVED_MASK                                0x00000000
#define    NVSRAM_AXI_ADDR_MASK_CS3_ADDR_MASK_SHIFT                       24
#define    NVSRAM_AXI_ADDR_MASK_CS3_ADDR_MASK_MASK                        0xFF000000
#define    NVSRAM_AXI_ADDR_MASK_CS2_ADDR_MASK_SHIFT                       16
#define    NVSRAM_AXI_ADDR_MASK_CS2_ADDR_MASK_MASK                        0x00FF0000
#define    NVSRAM_AXI_ADDR_MASK_CS1_ADDR_MASK_SHIFT                       8
#define    NVSRAM_AXI_ADDR_MASK_CS1_ADDR_MASK_MASK                        0x0000FF00
#define    NVSRAM_AXI_ADDR_MASK_CS0_ADDR_MASK_SHIFT                       0
#define    NVSRAM_AXI_ADDR_MASK_CS0_ADDR_MASK_MASK                        0x000000FF

#define NVSRAM_AXI_ADDR_MATCH_OFFSET                                      0x00000E24
#define NVSRAM_AXI_ADDR_MATCH_TYPE                                        UInt32
#define NVSRAM_AXI_ADDR_MATCH_RESERVED_MASK                               0x00000000
#define    NVSRAM_AXI_ADDR_MATCH_CS3_ADDR_MATCH_SHIFT                     24
#define    NVSRAM_AXI_ADDR_MATCH_CS3_ADDR_MATCH_MASK                      0xFF000000
#define    NVSRAM_AXI_ADDR_MATCH_CS2_ADDR_MATCH_SHIFT                     16
#define    NVSRAM_AXI_ADDR_MATCH_CS2_ADDR_MATCH_MASK                      0x00FF0000
#define    NVSRAM_AXI_ADDR_MATCH_CS1_ADDR_MATCH_SHIFT                     8
#define    NVSRAM_AXI_ADDR_MATCH_CS1_ADDR_MATCH_MASK                      0x0000FF00
#define    NVSRAM_AXI_ADDR_MATCH_CS0_ADDR_MATCH_SHIFT                     0
#define    NVSRAM_AXI_ADDR_MATCH_CS0_ADDR_MATCH_MASK                      0x000000FF

#define NVSRAM_AXI_CONF_OFFSET                                            0x00000E28
#define NVSRAM_AXI_CONF_TYPE                                              UInt32
#define NVSRAM_AXI_CONF_RESERVED_MASK                                     0xEF00FFE0
#define    NVSRAM_AXI_CONF_WP_SHIFT                                       28
#define    NVSRAM_AXI_CONF_WP_MASK                                        0x10000000
#define    NVSRAM_AXI_CONF_FORCE_BUFFERABLE_CS_SHIFT                      20
#define    NVSRAM_AXI_CONF_FORCE_BUFFERABLE_CS_MASK                       0x00F00000
#define    NVSRAM_AXI_CONF_SRAM_MUX_MODE_CS_SHIFT                         16
#define    NVSRAM_AXI_CONF_SRAM_MUX_MODE_CS_MASK                          0x000F0000
#define    NVSRAM_AXI_CONF_SRAM_DISABLE_SHIFT                             4
#define    NVSRAM_AXI_CONF_SRAM_DISABLE_MASK                              0x00000010
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS3_EN_SHIFT                         3
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS3_EN_MASK                          0x00000008
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS3_EN_CMD_NAND                   0x00000000
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS3_EN_CMD_SRAM                   0x00000001
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS2_EN_SHIFT                         2
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS2_EN_MASK                          0x00000004
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS2_EN_CMD_NAND                   0x00000000
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS2_EN_CMD_SRAM                   0x00000001
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS1_EN_SHIFT                         1
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS1_EN_MASK                          0x00000002
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS1_EN_CMD_NAND                   0x00000000
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS1_EN_CMD_SRAM                   0x00000001
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS0_EN_SHIFT                         0
#define    NVSRAM_AXI_CONF_SRAM_NAND_CS0_EN_MASK                          0x00000001
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS0_EN_CMD_NAND                   0x00000000
#define       NVSRAM_AXI_CONF_SRAM_NAND_CS0_EN_CMD_SRAM                   0x00000001

#define NVSRAM_AXI_CP_CONF_OFFSET                                         0x00000E2C
#define NVSRAM_AXI_CP_CONF_TYPE                                           UInt32
#define NVSRAM_AXI_CP_CONF_RESERVED_MASK                                  0x0F00FF00
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_SHIFT                            31
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_MASK                             0x80000000
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_GPIO_SEL_SHIFT                   28
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_GPIO_SEL_MASK                    0x70000000
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_WAIT_CNT_SHIFT                   20
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_WAIT_CNT_MASK                    0x00F00000
#define    NVSRAM_AXI_CP_CONF_COPRO_CS_EN_SHIFT                           16
#define    NVSRAM_AXI_CP_CONF_COPRO_CS_EN_MASK                            0x000F0000
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_ADDR1_SHIFT                      4
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_ADDR1_MASK                       0x000000F0
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_ADDR0_SHIFT                      0
#define    NVSRAM_AXI_CP_CONF_COPRO_DREQ_ADDR0_MASK                       0x0000000F

#define NVSRAM_AXI_PERIPH_ID_0_OFFSET                                     0x00000FE0
#define NVSRAM_AXI_PERIPH_ID_0_TYPE                                       UInt32
#define NVSRAM_AXI_PERIPH_ID_0_RESERVED_MASK                              0xFFFFFF00
#define    NVSRAM_AXI_PERIPH_ID_0_PART_NUMBER_0_SHIFT                     0
#define    NVSRAM_AXI_PERIPH_ID_0_PART_NUMBER_0_MASK                      0x000000FF

#define NVSRAM_AXI_PERIPH_ID_1_OFFSET                                     0x00000FE4
#define NVSRAM_AXI_PERIPH_ID_1_TYPE                                       UInt32
#define NVSRAM_AXI_PERIPH_ID_1_RESERVED_MASK                              0xFFFFFF00
#define    NVSRAM_AXI_PERIPH_ID_1_DESIGNER_0_SHIFT                        4
#define    NVSRAM_AXI_PERIPH_ID_1_DESIGNER_0_MASK                         0x000000F0
#define    NVSRAM_AXI_PERIPH_ID_1_PART_NUMBER_1_SHIFT                     0
#define    NVSRAM_AXI_PERIPH_ID_1_PART_NUMBER_1_MASK                      0x0000000F

#define NVSRAM_AXI_PERIPH_ID_2_OFFSET                                     0x00000FE8
#define NVSRAM_AXI_PERIPH_ID_2_TYPE                                       UInt32
#define NVSRAM_AXI_PERIPH_ID_2_RESERVED_MASK                              0xFFFFFF00
#define    NVSRAM_AXI_PERIPH_ID_2_REVISION_SHIFT                          4
#define    NVSRAM_AXI_PERIPH_ID_2_REVISION_MASK                           0x000000F0
#define    NVSRAM_AXI_PERIPH_ID_2_DESIGNER_1_SHIFT                        0
#define    NVSRAM_AXI_PERIPH_ID_2_DESIGNER_1_MASK                         0x0000000F

#define NVSRAM_AXI_PERIPH_ID_3_OFFSET                                     0x00000FEC
#define NVSRAM_AXI_PERIPH_ID_3_TYPE                                       UInt32
#define NVSRAM_AXI_PERIPH_ID_3_RESERVED_MASK                              0xFFFFFFFE
#define    NVSRAM_AXI_PERIPH_ID_3_INTEGRATION_CFG_SHIFT                   0
#define    NVSRAM_AXI_PERIPH_ID_3_INTEGRATION_CFG_MASK                    0x00000001

#define NVSRAM_AXI_PCELL_ID_0_OFFSET                                      0x00000FF0
#define NVSRAM_AXI_PCELL_ID_0_TYPE                                        UInt32
#define NVSRAM_AXI_PCELL_ID_0_RESERVED_MASK                               0xFFFFFF00
#define    NVSRAM_AXI_PCELL_ID_0_PCELL_ID_0_SHIFT                         0
#define    NVSRAM_AXI_PCELL_ID_0_PCELL_ID_0_MASK                          0x000000FF

#define NVSRAM_AXI_PCELL_ID_1_OFFSET                                      0x00000FF4
#define NVSRAM_AXI_PCELL_ID_1_TYPE                                        UInt32
#define NVSRAM_AXI_PCELL_ID_1_RESERVED_MASK                               0xFFFFFF00
#define    NVSRAM_AXI_PCELL_ID_1_PCELL_ID_1_SHIFT                         0
#define    NVSRAM_AXI_PCELL_ID_1_PCELL_ID_1_MASK                          0x000000FF

#define NVSRAM_AXI_PCELL_ID_2_OFFSET                                      0x00000FF8
#define NVSRAM_AXI_PCELL_ID_2_TYPE                                        UInt32
#define NVSRAM_AXI_PCELL_ID_2_RESERVED_MASK                               0xFFFFFF00
#define    NVSRAM_AXI_PCELL_ID_2_PCELL_ID_2_SHIFT                         0
#define    NVSRAM_AXI_PCELL_ID_2_PCELL_ID_2_MASK                          0x000000FF

#define NVSRAM_AXI_PCELL_ID_3_OFFSET                                      0x00000FFC
#define NVSRAM_AXI_PCELL_ID_3_TYPE                                        UInt32
#define NVSRAM_AXI_PCELL_ID_3_RESERVED_MASK                               0xFFFFFF00
#define    NVSRAM_AXI_PCELL_ID_3_PCELL_ID_3_SHIFT                         0
#define    NVSRAM_AXI_PCELL_ID_3_PCELL_ID_3_MASK                          0x000000FF

#endif /* __BRCM_RDB_NVSRAM_AXI_H__ */



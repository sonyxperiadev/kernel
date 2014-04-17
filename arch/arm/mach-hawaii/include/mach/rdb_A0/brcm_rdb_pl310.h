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

#ifndef __BRCM_RDB_PL310_H__
#define __BRCM_RDB_PL310_H__

#define PL310_CACHE_ID_OFFSET                                             0x00000000
#define PL310_CACHE_ID_TYPE                                               UInt32
#define PL310_CACHE_ID_RESERVED_MASK                                      0x00000000
#define    PL310_CACHE_ID_CACHE_ID_SHIFT                                  0
#define    PL310_CACHE_ID_CACHE_ID_MASK                                   0xFFFFFFFF

#define PL310_CACHE_TYPE_OFFSET                                           0x00000004
#define PL310_CACHE_TYPE_TYPE                                             UInt32
#define PL310_CACHE_TYPE_RESERVED_MASK                                    0x00000000
#define    PL310_CACHE_TYPE_DATABANKING_SHIFT                             31
#define    PL310_CACHE_TYPE_DATABANKING_MASK                              0x80000000
#define    PL310_CACHE_TYPE_SBZ_SHIFT                                     29
#define    PL310_CACHE_TYPE_SBZ_MASK                                      0x60000000
#define    PL310_CACHE_TYPE_CTYPE_SHIFT                                   25
#define    PL310_CACHE_TYPE_CTYPE_MASK                                    0x1E000000
#define    PL310_CACHE_TYPE_H_SHIFT                                       24
#define    PL310_CACHE_TYPE_H_MASK                                        0x01000000
#define    PL310_CACHE_TYPE_DSIZE_SHIFT                                   19
#define    PL310_CACHE_TYPE_DSIZE_MASK                                    0x00F80000
#define    PL310_CACHE_TYPE_ASSOCIATIVITY_1_SHIFT                         18
#define    PL310_CACHE_TYPE_ASSOCIATIVITY_1_MASK                          0x00040000
#define    PL310_CACHE_TYPE_SBZ2_SHIFT                                    14
#define    PL310_CACHE_TYPE_SBZ2_MASK                                     0x0003C000
#define    PL310_CACHE_TYPE_LINE_LENGTH_1_SHIFT                           12
#define    PL310_CACHE_TYPE_LINE_LENGTH_1_MASK                            0x00003000
#define    PL310_CACHE_TYPE_ISIZE_SHIFT                                   7
#define    PL310_CACHE_TYPE_ISIZE_MASK                                    0x00000F80
#define    PL310_CACHE_TYPE_ASSOCIATIVITY_2_SHIFT                         6
#define    PL310_CACHE_TYPE_ASSOCIATIVITY_2_MASK                          0x00000040
#define    PL310_CACHE_TYPE_SBZ3_SHIFT                                    2
#define    PL310_CACHE_TYPE_SBZ3_MASK                                     0x0000003C
#define    PL310_CACHE_TYPE_LINE_LENGTH_2_SHIFT                           0
#define    PL310_CACHE_TYPE_LINE_LENGTH_2_MASK                            0x00000003

#define PL310_CONTROL_OFFSET                                              0x00000100
#define PL310_CONTROL_TYPE                                                UInt32
#define PL310_CONTROL_RESERVED_MASK                                       0xFFFFFFFE
#define    PL310_CONTROL_ENABLE_SHIFT                                     0
#define    PL310_CONTROL_ENABLE_MASK                                      0x00000001

#define PL310_AUX_CONTROL_OFFSET                                          0x00000104
#define PL310_AUX_CONTROL_TYPE                                            UInt32
#define PL310_AUX_CONTROL_RESERVED_MASK                                   0x8200CBFE
#define    PL310_AUX_CONTROL_EARLY_BRESP_EN_SHIFT                         30
#define    PL310_AUX_CONTROL_EARLY_BRESP_EN_MASK                          0x40000000
#define    PL310_AUX_CONTROL_INSTR_PREFETCH_EN_SHIFT                      29
#define    PL310_AUX_CONTROL_INSTR_PREFETCH_EN_MASK                       0x20000000
#define    PL310_AUX_CONTROL_DATA_PREFETCH_EN_SHIFT                       28
#define    PL310_AUX_CONTROL_DATA_PREFETCH_EN_MASK                        0x10000000
#define    PL310_AUX_CONTROL_NONSECURE_INT_ACCESS_SHIFT                   27
#define    PL310_AUX_CONTROL_NONSECURE_INT_ACCESS_MASK                    0x08000000
#define    PL310_AUX_CONTROL_NONSECURE_LOCKDOWN_SHIFT                     26
#define    PL310_AUX_CONTROL_NONSECURE_LOCKDOWN_MASK                      0x04000000
#define    PL310_AUX_CONTROL_FORCE_WRITE_ALLOCATE_SHIFT                   23
#define    PL310_AUX_CONTROL_FORCE_WRITE_ALLOCATE_MASK                    0x01800000
#define    PL310_AUX_CONTROL_SHARED_ATTRIBUTE_IGNORE_SHIFT                22
#define    PL310_AUX_CONTROL_SHARED_ATTRIBUTE_IGNORE_MASK                 0x00400000
#define    PL310_AUX_CONTROL_PARITY_EN_SHIFT                              21
#define    PL310_AUX_CONTROL_PARITY_EN_MASK                               0x00200000
#define    PL310_AUX_CONTROL_EVENT_MONITOR_EN_SHIFT                       20
#define    PL310_AUX_CONTROL_EVENT_MONITOR_EN_MASK                        0x00100000
#define    PL310_AUX_CONTROL_WAY_SIZE_SHIFT                               17
#define    PL310_AUX_CONTROL_WAY_SIZE_MASK                                0x000E0000
#define    PL310_AUX_CONTROL_ASSOCIATIVITY_SHIFT                          16
#define    PL310_AUX_CONTROL_ASSOCIATIVITY_MASK                           0x00010000
#define    PL310_AUX_CONTROL_SHARED_ATTRIBUTE_INVAL_SHIFT                 13
#define    PL310_AUX_CONTROL_SHARED_ATTRIBUTE_INVAL_MASK                  0x00002000
#define    PL310_AUX_CONTROL_EXCLUSIVE_EN_SHIFT                           12
#define    PL310_AUX_CONTROL_EXCLUSIVE_EN_MASK                            0x00001000
#define    PL310_AUX_CONTROL_HIGH_PRIORITY_SO_DEV_SHIFT                   10
#define    PL310_AUX_CONTROL_HIGH_PRIORITY_SO_DEV_MASK                    0x00000400
#define    PL310_AUX_CONTROL_FULL_LINE_ZERO_EN_SHIFT                      0
#define    PL310_AUX_CONTROL_FULL_LINE_ZERO_EN_MASK                       0x00000001

#define PL310_TAG_RAM_CTRL_OFFSET                                         0x00000108
#define PL310_TAG_RAM_CTRL_TYPE                                           UInt32
#define PL310_TAG_RAM_CTRL_RESERVED_MASK                                  0xFFFFF888
#define    PL310_TAG_RAM_CTRL_RAM_WR_LATENCYT_SHIFT                       8
#define    PL310_TAG_RAM_CTRL_RAM_WR_LATENCYT_MASK                        0x00000700
#define    PL310_TAG_RAM_CTRL_RAM_RD_LATENCYT_SHIFT                       4
#define    PL310_TAG_RAM_CTRL_RAM_RD_LATENCYT_MASK                        0x00000070
#define    PL310_TAG_RAM_CTRL_RAM_SETUP_LATENCYT_SHIFT                    0
#define    PL310_TAG_RAM_CTRL_RAM_SETUP_LATENCYT_MASK                     0x00000007

#define PL310_DATA_RAM_CTRL_OFFSET                                        0x0000010C
#define PL310_DATA_RAM_CTRL_TYPE                                          UInt32
#define PL310_DATA_RAM_CTRL_RESERVED_MASK                                 0xFFFFF888
#define    PL310_DATA_RAM_CTRL_RAM_WR_LATENCYD_SHIFT                      8
#define    PL310_DATA_RAM_CTRL_RAM_WR_LATENCYD_MASK                       0x00000700
#define    PL310_DATA_RAM_CTRL_RAM_RD_LATENCYD_SHIFT                      4
#define    PL310_DATA_RAM_CTRL_RAM_RD_LATENCYD_MASK                       0x00000070
#define    PL310_DATA_RAM_CTRL_RAM_SETUP_LATENCYD_SHIFT                   0
#define    PL310_DATA_RAM_CTRL_RAM_SETUP_LATENCYD_MASK                    0x00000007

#define PL310_EVENT_CTR_CTRL_OFFSET                                       0x00000200
#define PL310_EVENT_CTR_CTRL_TYPE                                         UInt32
#define PL310_EVENT_CTR_CTRL_RESERVED_MASK                                0xFFFFFFF8
#define    PL310_EVENT_CTR_CTRL_COUNTER_RESET_SHIFT                       1
#define    PL310_EVENT_CTR_CTRL_COUNTER_RESET_MASK                        0x00000006
#define    PL310_EVENT_CTR_CTRL_COUNTER_EN_SHIFT                          0
#define    PL310_EVENT_CTR_CTRL_COUNTER_EN_MASK                           0x00000001

#define PL310_EVENT_CTR_CFG0_OFFSET                                       0x00000204
#define PL310_EVENT_CTR_CFG0_TYPE                                         UInt32
#define PL310_EVENT_CTR_CFG0_RESERVED_MASK                                0xFFFFFFC0
#define    PL310_EVENT_CTR_CFG0_COUNTER_EVENT_SOURCE0_SHIFT               2
#define    PL310_EVENT_CTR_CFG0_COUNTER_EVENT_SOURCE0_MASK                0x0000003C
#define    PL310_EVENT_CTR_CFG0_COUNTER_INTERRUPT_GEN0_SHIFT              0
#define    PL310_EVENT_CTR_CFG0_COUNTER_INTERRUPT_GEN0_MASK               0x00000003

#define PL310_EVENT_CTR_CFG1_OFFSET                                       0x00000208
#define PL310_EVENT_CTR_CFG1_TYPE                                         UInt32
#define PL310_EVENT_CTR_CFG1_RESERVED_MASK                                0xFFFFFFC0
#define    PL310_EVENT_CTR_CFG1_COUNTER_EVENT_SOURCE1_SHIFT               2
#define    PL310_EVENT_CTR_CFG1_COUNTER_EVENT_SOURCE1_MASK                0x0000003C
#define    PL310_EVENT_CTR_CFG1_COUNTER_INTERRUPT_GEN1_SHIFT              0
#define    PL310_EVENT_CTR_CFG1_COUNTER_INTERRUPT_GEN1_MASK               0x00000003

#define PL310_EVENT_CTR0_OFFSET                                           0x0000020C
#define PL310_EVENT_CTR0_TYPE                                             UInt32
#define PL310_EVENT_CTR0_RESERVED_MASK                                    0x00000000
#define    PL310_EVENT_CTR0_COUNTER0_SHIFT                                0
#define    PL310_EVENT_CTR0_COUNTER0_MASK                                 0xFFFFFFFF

#define PL310_EVENT_CTR1_OFFSET                                           0x00000210
#define PL310_EVENT_CTR1_TYPE                                             UInt32
#define PL310_EVENT_CTR1_RESERVED_MASK                                    0x00000000
#define    PL310_EVENT_CTR1_COUNTER1_SHIFT                                0
#define    PL310_EVENT_CTR1_COUNTER1_MASK                                 0xFFFFFFFF

#define PL310_INT_MASK_OFFSET                                             0x00000214
#define PL310_INT_MASK_TYPE                                               UInt32
#define PL310_INT_MASK_RESERVED_MASK                                      0xFFFFFE00
#define    PL310_INT_MASK_DECERR1_SHIFT                                   8
#define    PL310_INT_MASK_DECERR1_MASK                                    0x00000100
#define    PL310_INT_MASK_SLVERR1_SHIFT                                   7
#define    PL310_INT_MASK_SLVERR1_MASK                                    0x00000080
#define    PL310_INT_MASK_ERRRD1_SHIFT                                    6
#define    PL310_INT_MASK_ERRRD1_MASK                                     0x00000040
#define    PL310_INT_MASK_ERRRT1_SHIFT                                    5
#define    PL310_INT_MASK_ERRRT1_MASK                                     0x00000020
#define    PL310_INT_MASK_ERRWD1_SHIFT                                    4
#define    PL310_INT_MASK_ERRWD1_MASK                                     0x00000010
#define    PL310_INT_MASK_ERRWT1_SHIFT                                    3
#define    PL310_INT_MASK_ERRWT1_MASK                                     0x00000008
#define    PL310_INT_MASK_PARRD1_SHIFT                                    2
#define    PL310_INT_MASK_PARRD1_MASK                                     0x00000004
#define    PL310_INT_MASK_PARRT1_SHIFT                                    1
#define    PL310_INT_MASK_PARRT1_MASK                                     0x00000002
#define    PL310_INT_MASK_ECNTR1_SHIFT                                    0
#define    PL310_INT_MASK_ECNTR1_MASK                                     0x00000001

#define PL310_INT_STATUS_OFFSET                                           0x00000218
#define PL310_INT_STATUS_TYPE                                             UInt32
#define PL310_INT_STATUS_RESERVED_MASK                                    0xFFFFFE00
#define    PL310_INT_STATUS_DECERR2_SHIFT                                 8
#define    PL310_INT_STATUS_DECERR2_MASK                                  0x00000100
#define    PL310_INT_STATUS_SLVERR2_SHIFT                                 7
#define    PL310_INT_STATUS_SLVERR2_MASK                                  0x00000080
#define    PL310_INT_STATUS_ERRRD2_SHIFT                                  6
#define    PL310_INT_STATUS_ERRRD2_MASK                                   0x00000040
#define    PL310_INT_STATUS_ERRRT2_SHIFT                                  5
#define    PL310_INT_STATUS_ERRRT2_MASK                                   0x00000020
#define    PL310_INT_STATUS_ERRWD2_SHIFT                                  4
#define    PL310_INT_STATUS_ERRWD2_MASK                                   0x00000010
#define    PL310_INT_STATUS_ERRWT2_SHIFT                                  3
#define    PL310_INT_STATUS_ERRWT2_MASK                                   0x00000008
#define    PL310_INT_STATUS_PARRD2_SHIFT                                  2
#define    PL310_INT_STATUS_PARRD2_MASK                                   0x00000004
#define    PL310_INT_STATUS_PARRT2_SHIFT                                  1
#define    PL310_INT_STATUS_PARRT2_MASK                                   0x00000002
#define    PL310_INT_STATUS_ECNTR2_SHIFT                                  0
#define    PL310_INT_STATUS_ECNTR2_MASK                                   0x00000001

#define PL310_INT_RAW_STATUS_OFFSET                                       0x0000021C
#define PL310_INT_RAW_STATUS_TYPE                                         UInt32
#define PL310_INT_RAW_STATUS_RESERVED_MASK                                0xFFFFFE00
#define    PL310_INT_RAW_STATUS_DECERR3_SHIFT                             8
#define    PL310_INT_RAW_STATUS_DECERR3_MASK                              0x00000100
#define    PL310_INT_RAW_STATUS_SLVERR3_SHIFT                             7
#define    PL310_INT_RAW_STATUS_SLVERR3_MASK                              0x00000080
#define    PL310_INT_RAW_STATUS_ERRRD3_SHIFT                              6
#define    PL310_INT_RAW_STATUS_ERRRD3_MASK                               0x00000040
#define    PL310_INT_RAW_STATUS_ERRRT3_SHIFT                              5
#define    PL310_INT_RAW_STATUS_ERRRT3_MASK                               0x00000020
#define    PL310_INT_RAW_STATUS_ERRWD3_SHIFT                              4
#define    PL310_INT_RAW_STATUS_ERRWD3_MASK                               0x00000010
#define    PL310_INT_RAW_STATUS_ERRWT3_SHIFT                              3
#define    PL310_INT_RAW_STATUS_ERRWT3_MASK                               0x00000008
#define    PL310_INT_RAW_STATUS_PARRD3_SHIFT                              2
#define    PL310_INT_RAW_STATUS_PARRD3_MASK                               0x00000004
#define    PL310_INT_RAW_STATUS_PARRT3_SHIFT                              1
#define    PL310_INT_RAW_STATUS_PARRT3_MASK                               0x00000002
#define    PL310_INT_RAW_STATUS_ECNTR3_SHIFT                              0
#define    PL310_INT_RAW_STATUS_ECNTR3_MASK                               0x00000001

#define PL310_INT_CLEAR_OFFSET                                            0x00000220
#define PL310_INT_CLEAR_TYPE                                              UInt32
#define PL310_INT_CLEAR_RESERVED_MASK                                     0xFFFFFE00
#define    PL310_INT_CLEAR_DECERR4_SHIFT                                  8
#define    PL310_INT_CLEAR_DECERR4_MASK                                   0x00000100
#define    PL310_INT_CLEAR_SLVERR4_SHIFT                                  7
#define    PL310_INT_CLEAR_SLVERR4_MASK                                   0x00000080
#define    PL310_INT_CLEAR_ERRRD4_SHIFT                                   6
#define    PL310_INT_CLEAR_ERRRD4_MASK                                    0x00000040
#define    PL310_INT_CLEAR_ERRRT4_SHIFT                                   5
#define    PL310_INT_CLEAR_ERRRT4_MASK                                    0x00000020
#define    PL310_INT_CLEAR_ERRWD4_SHIFT                                   4
#define    PL310_INT_CLEAR_ERRWD4_MASK                                    0x00000010
#define    PL310_INT_CLEAR_ERRWT4_SHIFT                                   3
#define    PL310_INT_CLEAR_ERRWT4_MASK                                    0x00000008
#define    PL310_INT_CLEAR_PARRD4_SHIFT                                   2
#define    PL310_INT_CLEAR_PARRD4_MASK                                    0x00000004
#define    PL310_INT_CLEAR_PARRT4_SHIFT                                   1
#define    PL310_INT_CLEAR_PARRT4_MASK                                    0x00000002
#define    PL310_INT_CLEAR_ECNTR4_SHIFT                                   0
#define    PL310_INT_CLEAR_ECNTR4_MASK                                    0x00000001

#define PL310_CACHE_SYNC_OFFSET                                           0x00000730
#define PL310_CACHE_SYNC_TYPE                                             UInt32
#define PL310_CACHE_SYNC_RESERVED_MASK                                    0x00000000
#define    PL310_CACHE_SYNC_CACHE_SYNC_SHIFT                              0
#define    PL310_CACHE_SYNC_CACHE_SYNC_MASK                               0xFFFFFFFF

#define PL310_INV_PA_OFFSET                                               0x00000770
#define PL310_INV_PA_TYPE                                                 UInt32
#define PL310_INV_PA_RESERVED_MASK                                        0x0000001E
#define    PL310_INV_PA_TAG_1_SHIFT                                       12
#define    PL310_INV_PA_TAG_1_MASK                                        0xFFFFF000
#define    PL310_INV_PA_INDEX_1_SHIFT                                     5
#define    PL310_INV_PA_INDEX_1_MASK                                      0x00000FE0
#define    PL310_INV_PA_C_1_SHIFT                                         0
#define    PL310_INV_PA_C_1_MASK                                          0x00000001

#define PL310_INV_WAY_OFFSET                                              0x0000077C
#define PL310_INV_WAY_TYPE                                                UInt32
#define PL310_INV_WAY_RESERVED_MASK                                       0xFFFFFF00
#define    PL310_INV_WAY_WAYS_1_SHIFT                                     0
#define    PL310_INV_WAY_WAYS_1_MASK                                      0x000000FF

#define PL310_CLEAN_PA_OFFSET                                             0x000007B0
#define PL310_CLEAN_PA_TYPE                                               UInt32
#define PL310_CLEAN_PA_RESERVED_MASK                                      0x0000001E
#define    PL310_CLEAN_PA_TAG_2_SHIFT                                     12
#define    PL310_CLEAN_PA_TAG_2_MASK                                      0xFFFFF000
#define    PL310_CLEAN_PA_INDEX_2_SHIFT                                   5
#define    PL310_CLEAN_PA_INDEX_2_MASK                                    0x00000FE0
#define    PL310_CLEAN_PA_C_2_SHIFT                                       0
#define    PL310_CLEAN_PA_C_2_MASK                                        0x00000001

#define PL310_CLEAN_INDEX_WAY_OFFSET                                      0x000007B8
#define PL310_CLEAN_INDEX_WAY_TYPE                                        UInt32
#define PL310_CLEAN_INDEX_WAY_RESERVED_MASK                               0x8FFFF01E
#define    PL310_CLEAN_INDEX_WAY_WAY_3_SHIFT                              28
#define    PL310_CLEAN_INDEX_WAY_WAY_3_MASK                               0x70000000
#define    PL310_CLEAN_INDEX_WAY_INDEX_3_SHIFT                            5
#define    PL310_CLEAN_INDEX_WAY_INDEX_3_MASK                             0x00000FE0
#define    PL310_CLEAN_INDEX_WAY_C_3_SHIFT                                0
#define    PL310_CLEAN_INDEX_WAY_C_3_MASK                                 0x00000001

#define PL310_CLEAN_WAY_OFFSET                                            0x000007BC
#define PL310_CLEAN_WAY_TYPE                                              UInt32
#define PL310_CLEAN_WAY_RESERVED_MASK                                     0xFFFFFF00
#define    PL310_CLEAN_WAY_WAYS_2_SHIFT                                   0
#define    PL310_CLEAN_WAY_WAYS_2_MASK                                    0x000000FF

#define PL310_C_I_PA_OFFSET                                               0x000007F0
#define PL310_C_I_PA_TYPE                                                 UInt32
#define PL310_C_I_PA_RESERVED_MASK                                        0x0000001E
#define    PL310_C_I_PA_TAG_4_SHIFT                                       12
#define    PL310_C_I_PA_TAG_4_MASK                                        0xFFFFF000
#define    PL310_C_I_PA_INDEX_4_SHIFT                                     5
#define    PL310_C_I_PA_INDEX_4_MASK                                      0x00000FE0
#define    PL310_C_I_PA_C_4_SHIFT                                         0
#define    PL310_C_I_PA_C_4_MASK                                          0x00000001

#define PL310_C_I_INDEX_WAY_OFFSET                                        0x000007F8
#define PL310_C_I_INDEX_WAY_TYPE                                          UInt32
#define PL310_C_I_INDEX_WAY_RESERVED_MASK                                 0x8FFFF01E
#define    PL310_C_I_INDEX_WAY_WAY_5_SHIFT                                28
#define    PL310_C_I_INDEX_WAY_WAY_5_MASK                                 0x70000000
#define    PL310_C_I_INDEX_WAY_INDEX_5_SHIFT                              5
#define    PL310_C_I_INDEX_WAY_INDEX_5_MASK                               0x00000FE0
#define    PL310_C_I_INDEX_WAY_C_5_SHIFT                                  0
#define    PL310_C_I_INDEX_WAY_C_5_MASK                                   0x00000001

#define PL310_C_I_WAY_OFFSET                                              0x000007FC
#define PL310_C_I_WAY_TYPE                                                UInt32
#define PL310_C_I_WAY_RESERVED_MASK                                       0xFFFFFF00
#define    PL310_C_I_WAY_WAYS_3_SHIFT                                     0
#define    PL310_C_I_WAY_WAYS_3_MASK                                      0x000000FF

#define PL310_DATA_LOCKDOWN_OFFSET                                        0x00000900
#define PL310_DATA_LOCKDOWN_TYPE                                          UInt32
#define PL310_DATA_LOCKDOWN_RESERVED_MASK                                 0xFFFFFF00
#define    PL310_DATA_LOCKDOWN_DATALOCK_SHIFT                             0
#define    PL310_DATA_LOCKDOWN_DATALOCK_MASK                              0x000000FF

#define PL310_INSTR_LOCKDOWN_OFFSET                                       0x00000904
#define PL310_INSTR_LOCKDOWN_TYPE                                         UInt32
#define PL310_INSTR_LOCKDOWN_RESERVED_MASK                                0xFFFFFF00
#define    PL310_INSTR_LOCKDOWN_INSTRLOCK_SHIFT                           0
#define    PL310_INSTR_LOCKDOWN_INSTRLOCK_MASK                            0x000000FF

#define PL310_FILT_START_OFFSET                                           0x00000C00
#define PL310_FILT_START_TYPE                                             UInt32
#define PL310_FILT_START_RESERVED_MASK                                    0x000FFFFE
#define    PL310_FILT_START_FILTER_START_SHIFT                            20
#define    PL310_FILT_START_FILTER_START_MASK                             0xFFF00000
#define    PL310_FILT_START_FILTER_EN_SHIFT                               0
#define    PL310_FILT_START_FILTER_EN_MASK                                0x00000001

#define PL310_FILT_END_OFFSET                                             0x00000C04
#define PL310_FILT_END_TYPE                                               UInt32
#define PL310_FILT_END_RESERVED_MASK                                      0x000FFFFF
#define    PL310_FILT_END_FILTER_END_SHIFT                                20
#define    PL310_FILT_END_FILTER_END_MASK                                 0xFFF00000

#define PL310_DEBUG_CTRL_OFFSET                                           0x00000F40
#define PL310_DEBUG_CTRL_TYPE                                             UInt32
#define PL310_DEBUG_CTRL_RESERVED_MASK                                    0xFFFFFFF8
#define    PL310_DEBUG_CTRL_SPNIDEN_SHIFT                                 2
#define    PL310_DEBUG_CTRL_SPNIDEN_MASK                                  0x00000004
#define    PL310_DEBUG_CTRL_DWB_SHIFT                                     1
#define    PL310_DEBUG_CTRL_DWB_MASK                                      0x00000002
#define    PL310_DEBUG_CTRL_DCL_SHIFT                                     0
#define    PL310_DEBUG_CTRL_DCL_MASK                                      0x00000001

#define PL310_PREFETCH_OFFS_OFFSET                                        0x00000F60
#define PL310_PREFETCH_OFFS_TYPE                                          UInt32
#define PL310_PREFETCH_OFFS_RESERVED_MASK                                 0xFFFFFFE0
#define    PL310_PREFETCH_OFFS_PREFETCH_OFFSET_SHIFT                      0
#define    PL310_PREFETCH_OFFS_PREFETCH_OFFSET_MASK                       0x0000001F

#define PL310_PWR_CTRL_OFFSET                                             0x00000F80
#define PL310_PWR_CTRL_TYPE                                               UInt32
#define PL310_PWR_CTRL_RESERVED_MASK                                      0xFFFFFFFC
#define    PL310_PWR_CTRL_DYN_CLK_EN_SHIFT                                1
#define    PL310_PWR_CTRL_DYN_CLK_EN_MASK                                 0x00000002
#define    PL310_PWR_CTRL_STBY_EN_SHIFT                                   0
#define    PL310_PWR_CTRL_STBY_EN_MASK                                    0x00000001

#endif /* __BRCM_RDB_PL310_H__ */



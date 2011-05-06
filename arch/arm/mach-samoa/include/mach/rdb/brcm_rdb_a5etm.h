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
/*     Date     : Generated on 4/10/2011 22:27:55                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_A5ETM_H__
#define __BRCM_RDB_A5ETM_H__

#define A5ETM_ETMCR_OFFSET                                                0x00000000
#define A5ETM_ETMCR_TYPE                                                  UInt32
#define A5ETM_ETMCR_RESERVED_MASK                                         0xE1C00000
#define    A5ETM_ETMCR_TIMESTAMP_EN_SHIFT                                 28
#define    A5ETM_ETMCR_TIMESTAMP_EN_MASK                                  0x10000000
#define    A5ETM_ETMCR_PROCESSOR_SEL_SHIFT                                25
#define    A5ETM_ETMCR_PROCESSOR_SEL_MASK                                 0x0E000000
#define    A5ETM_ETMCR_PORT_SIZE_3_SHIFT                                  21
#define    A5ETM_ETMCR_PORT_SIZE_3_MASK                                   0x00200000
#define    A5ETM_ETMCR_DATA_ONLY_MODE_SHIFT                               20
#define    A5ETM_ETMCR_DATA_ONLY_MODE_MASK                                0x00100000
#define    A5ETM_ETMCR_FILTER_SHIFT                                       19
#define    A5ETM_ETMCR_FILTER_MASK                                        0x00080000
#define    A5ETM_ETMCR_SUPPRESS_DATA_SHIFT                                18
#define    A5ETM_ETMCR_SUPPRESS_DATA_MASK                                 0x00040000
#define    A5ETM_ETMCR_PORT_MODE_1_0_SHIFT                                16
#define    A5ETM_ETMCR_PORT_MODE_1_0_MASK                                 0x00030000
#define    A5ETM_ETMCR_CONTEXTID_SIZE_SHIFT                               14
#define    A5ETM_ETMCR_CONTEXTID_SIZE_MASK                                0x0000C000
#define    A5ETM_ETMCR_PORT_MODE_2_SHIFT                                  13
#define    A5ETM_ETMCR_PORT_MODE_2_MASK                                   0x00002000
#define    A5ETM_ETMCR_CYCLE_ACC_EN_SHIFT                                 12
#define    A5ETM_ETMCR_CYCLE_ACC_EN_MASK                                  0x00001000
#define    A5ETM_ETMCR_ETMEN_SHIFT                                        11
#define    A5ETM_ETMCR_ETMEN_MASK                                         0x00000800
#define    A5ETM_ETMCR_PROG_BIT_SHIFT                                     10
#define    A5ETM_ETMCR_PROG_BIT_MASK                                      0x00000400
#define    A5ETM_ETMCR_DEBUG_REQ_CTRL_SHIFT                               9
#define    A5ETM_ETMCR_DEBUG_REQ_CTRL_MASK                                0x00000200
#define    A5ETM_ETMCR_BRANCH_OUT_SHIFT                                   8
#define    A5ETM_ETMCR_BRANCH_OUT_MASK                                    0x00000100
#define    A5ETM_ETMCR_STALL_PROC_SHIFT                                   7
#define    A5ETM_ETMCR_STALL_PROC_MASK                                    0x00000080
#define    A5ETM_ETMCR_PORT_SIZE_2_0_SHIFT                                4
#define    A5ETM_ETMCR_PORT_SIZE_2_0_MASK                                 0x00000070
#define    A5ETM_ETMCR_DATA_ACCESS_SHIFT                                  2
#define    A5ETM_ETMCR_DATA_ACCESS_MASK                                   0x0000000C
#define    A5ETM_ETMCR_MONITORCPRT_SHIFT                                  1
#define    A5ETM_ETMCR_MONITORCPRT_MASK                                   0x00000002
#define    A5ETM_ETMCR_POWER_DOWN_SHIFT                                   0
#define    A5ETM_ETMCR_POWER_DOWN_MASK                                    0x00000001

#define A5ETM_ETMCCR_OFFSET                                               0x00000004
#define A5ETM_ETMCCR_TYPE                                                 UInt32
#define A5ETM_ETMCCR_RESERVED_MASK                                        0x70000000
#define    A5ETM_ETMCCR_IDREG_SHIFT                                       31
#define    A5ETM_ETMCCR_IDREG_MASK                                        0x80000000
#define    A5ETM_ETMCCR_SW_ACCESS_SHIFT                                   27
#define    A5ETM_ETMCCR_SW_ACCESS_MASK                                    0x08000000
#define    A5ETM_ETMCCR_START_STOP_SHIFT                                  26
#define    A5ETM_ETMCCR_START_STOP_MASK                                   0x04000000
#define    A5ETM_ETMCCR_CONTEXT_COMPS_SHIFT                               24
#define    A5ETM_ETMCCR_CONTEXT_COMPS_MASK                                0x03000000
#define    A5ETM_ETMCCR_FIFOFULL_SHIFT                                    23
#define    A5ETM_ETMCCR_FIFOFULL_MASK                                     0x00800000
#define    A5ETM_ETMCCR_EXT_OUTS_SHIFT                                    20
#define    A5ETM_ETMCCR_EXT_OUTS_MASK                                     0x00700000
#define    A5ETM_ETMCCR_EXT_INS_SHIFT                                     17
#define    A5ETM_ETMCCR_EXT_INS_MASK                                      0x000E0000
#define    A5ETM_ETMCCR_SEQUENCER_SHIFT                                   16
#define    A5ETM_ETMCCR_SEQUENCER_MASK                                    0x00010000
#define    A5ETM_ETMCCR_COUNTERS_SHIFT                                    13
#define    A5ETM_ETMCCR_COUNTERS_MASK                                     0x0000E000
#define    A5ETM_ETMCCR_MEMMAP_DECS_SHIFT                                 8
#define    A5ETM_ETMCCR_MEMMAP_DECS_MASK                                  0x00001F00
#define    A5ETM_ETMCCR_DATA_COMPS_SHIFT                                  4
#define    A5ETM_ETMCCR_DATA_COMPS_MASK                                   0x000000F0
#define    A5ETM_ETMCCR_ADDR_COMPS_SHIFT                                  0
#define    A5ETM_ETMCCR_ADDR_COMPS_MASK                                   0x0000000F

#define A5ETM_ETMTRIGGER_OFFSET                                           0x00000008
#define A5ETM_ETMTRIGGER_TYPE                                             UInt32
#define A5ETM_ETMTRIGGER_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMTRIGGER_TR_FUNCTION_SHIFT                             14
#define    A5ETM_ETMTRIGGER_TR_FUNCTION_MASK                              0x0001C000
#define    A5ETM_ETMTRIGGER_TR_RESOURCE_B_SHIFT                           7
#define    A5ETM_ETMTRIGGER_TR_RESOURCE_B_MASK                            0x00003F80
#define    A5ETM_ETMTRIGGER_TR_RESOURCE_A_SHIFT                           0
#define    A5ETM_ETMTRIGGER_TR_RESOURCE_A_MASK                            0x0000007F

#define A5ETM_ETMSR_OFFSET                                                0x00000010
#define A5ETM_ETMSR_TYPE                                                  UInt32
#define A5ETM_ETMSR_RESERVED_MASK                                         0xFFFFFFFD
#define    A5ETM_ETMSR_PROG_BIT_S_SHIFT                                   1
#define    A5ETM_ETMSR_PROG_BIT_S_MASK                                    0x00000002

#define A5ETM_ETMSCR_OFFSET                                               0x00000014
#define A5ETM_ETMSCR_TYPE                                                 UInt32
#define A5ETM_ETMSCR_RESERVED_MASK                                        0xFFFD83F0
#define    A5ETM_ETMSCR_NO_FETCH_SHIFT                                    17
#define    A5ETM_ETMSCR_NO_FETCH_MASK                                     0x00020000
#define    A5ETM_ETMSCR_MAXCORES_SHIFT                                    12
#define    A5ETM_ETMSCR_MAXCORES_MASK                                     0x00007000
#define    A5ETM_ETMSCR_PORT_MODE_SHIFT                                   11
#define    A5ETM_ETMSCR_PORT_MODE_MASK                                    0x00000800
#define    A5ETM_ETMSCR_PORT_SIZE_SHIFT                                   10
#define    A5ETM_ETMSCR_PORT_SIZE_MASK                                    0x00000400
#define    A5ETM_ETMSCR_HALF_RATE_SHIFT                                   3
#define    A5ETM_ETMSCR_HALF_RATE_MASK                                    0x00000008
#define    A5ETM_ETMSCR_MAX_PORT_SIZE_SHIFT                               0
#define    A5ETM_ETMSCR_MAX_PORT_SIZE_MASK                                0x00000007

#define A5ETM_ETMSSCR_OFFSET                                              0x00000018
#define A5ETM_ETMSSCR_TYPE                                                UInt32
#define A5ETM_ETMSSCR_RESERVED_MASK                                       0xFF00FF00
#define    A5ETM_ETMSSCR_STOP_ADDRS_SHIFT                                 16
#define    A5ETM_ETMSSCR_STOP_ADDRS_MASK                                  0x00FF0000
#define    A5ETM_ETMSSCR_START_ADDRS_SHIFT                                0
#define    A5ETM_ETMSSCR_START_ADDRS_MASK                                 0x000000FF

#define A5ETM_ETMTEEVR_OFFSET                                             0x00000020
#define A5ETM_ETMTEEVR_TYPE                                               UInt32
#define A5ETM_ETMTEEVR_RESERVED_MASK                                      0xFFFE0000
#define    A5ETM_ETMTEEVR_TE_FUNCTION_SHIFT                               14
#define    A5ETM_ETMTEEVR_TE_FUNCTION_MASK                                0x0001C000
#define    A5ETM_ETMTEEVR_TE_RESOURCE_B_SHIFT                             7
#define    A5ETM_ETMTEEVR_TE_RESOURCE_B_MASK                              0x00003F80
#define    A5ETM_ETMTEEVR_TE_RESOURCE_A_SHIFT                             0
#define    A5ETM_ETMTEEVR_TE_RESOURCE_A_MASK                              0x0000007F

#define A5ETM_ETMTECR1_OFFSET                                             0x00000024
#define A5ETM_ETMTECR1_TYPE                                               UInt32
#define A5ETM_ETMTECR1_RESERVED_MASK                                      0xFCFFFFF0
#define    A5ETM_ETMTECR1_TRACE_CTRL_EN_SHIFT                             25
#define    A5ETM_ETMTECR1_TRACE_CTRL_EN_MASK                              0x02000000
#define    A5ETM_ETMTECR1_EXCLUDE_INCLUDE_SHIFT                           24
#define    A5ETM_ETMTECR1_EXCLUDE_INCLUDE_MASK                            0x01000000
#define    A5ETM_ETMTECR1_INCL_EXCL_ADDRS_SHIFT                           0
#define    A5ETM_ETMTECR1_INCL_EXCL_ADDRS_MASK                            0x0000000F

#define A5ETM_ETMACVR1_OFFSET                                             0x00000040
#define A5ETM_ETMACVR1_TYPE                                               UInt32
#define A5ETM_ETMACVR1_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR1_ADDR_COMP_VAL_1_SHIFT                           0
#define    A5ETM_ETMACVR1_ADDR_COMP_VAL_1_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR2_OFFSET                                             0x00000044
#define A5ETM_ETMACVR2_TYPE                                               UInt32
#define A5ETM_ETMACVR2_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR2_ADDR_COMP_VAL_2_SHIFT                           0
#define    A5ETM_ETMACVR2_ADDR_COMP_VAL_2_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR3_OFFSET                                             0x00000048
#define A5ETM_ETMACVR3_TYPE                                               UInt32
#define A5ETM_ETMACVR3_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR3_ADDR_COMP_VAL_3_SHIFT                           0
#define    A5ETM_ETMACVR3_ADDR_COMP_VAL_3_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR4_OFFSET                                             0x0000004C
#define A5ETM_ETMACVR4_TYPE                                               UInt32
#define A5ETM_ETMACVR4_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR4_ADDR_COMP_VAL_4_SHIFT                           0
#define    A5ETM_ETMACVR4_ADDR_COMP_VAL_4_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR5_OFFSET                                             0x00000050
#define A5ETM_ETMACVR5_TYPE                                               UInt32
#define A5ETM_ETMACVR5_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR5_ADDR_COMP_VAL_5_SHIFT                           0
#define    A5ETM_ETMACVR5_ADDR_COMP_VAL_5_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR6_OFFSET                                             0x00000054
#define A5ETM_ETMACVR6_TYPE                                               UInt32
#define A5ETM_ETMACVR6_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR6_ADDR_COMP_VAL_6_SHIFT                           0
#define    A5ETM_ETMACVR6_ADDR_COMP_VAL_6_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR7_OFFSET                                             0x00000058
#define A5ETM_ETMACVR7_TYPE                                               UInt32
#define A5ETM_ETMACVR7_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR7_ADDR_COMP_VAL_7_SHIFT                           0
#define    A5ETM_ETMACVR7_ADDR_COMP_VAL_7_MASK                            0xFFFFFFFF

#define A5ETM_ETMACVR8_OFFSET                                             0x0000005C
#define A5ETM_ETMACVR8_TYPE                                               UInt32
#define A5ETM_ETMACVR8_RESERVED_MASK                                      0x00000000
#define    A5ETM_ETMACVR8_ADDR_COMP_VAL_8_SHIFT                           0
#define    A5ETM_ETMACVR8_ADDR_COMP_VAL_8_MASK                            0xFFFFFFFF

#define A5ETM_ETMACTR1_OFFSET                                             0x00000080
#define A5ETM_ETMACTR1_TYPE                                               UInt32
#define A5ETM_ETMACTR1_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR1_SEC_LVL_CTRL_1_SHIFT                            10
#define    A5ETM_ETMACTR1_SEC_LVL_CTRL_1_MASK                             0x00000C00
#define    A5ETM_ETMACTR1_CONTEXT_ID_CTRL_1_SHIFT                         8
#define    A5ETM_ETMACTR1_CONTEXT_ID_CTRL_1_MASK                          0x00000300
#define    A5ETM_ETMACTR1_ACCESS_TYPE_1_SHIFT                             0
#define    A5ETM_ETMACTR1_ACCESS_TYPE_1_MASK                              0x00000007

#define A5ETM_ETMACTR2_OFFSET                                             0x00000084
#define A5ETM_ETMACTR2_TYPE                                               UInt32
#define A5ETM_ETMACTR2_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR2_SEC_LVL_CTRL_2_SHIFT                            10
#define    A5ETM_ETMACTR2_SEC_LVL_CTRL_2_MASK                             0x00000C00
#define    A5ETM_ETMACTR2_CONTEXT_ID_CTRL_2_SHIFT                         8
#define    A5ETM_ETMACTR2_CONTEXT_ID_CTRL_2_MASK                          0x00000300
#define    A5ETM_ETMACTR2_ACCESS_TYPE_2_SHIFT                             0
#define    A5ETM_ETMACTR2_ACCESS_TYPE_2_MASK                              0x00000007

#define A5ETM_ETMACTR3_OFFSET                                             0x00000088
#define A5ETM_ETMACTR3_TYPE                                               UInt32
#define A5ETM_ETMACTR3_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR3_SEC_LVL_CTRL_3_SHIFT                            10
#define    A5ETM_ETMACTR3_SEC_LVL_CTRL_3_MASK                             0x00000C00
#define    A5ETM_ETMACTR3_CONTEXT_ID_CTRL_3_SHIFT                         8
#define    A5ETM_ETMACTR3_CONTEXT_ID_CTRL_3_MASK                          0x00000300
#define    A5ETM_ETMACTR3_ACCESS_TYPE_3_SHIFT                             0
#define    A5ETM_ETMACTR3_ACCESS_TYPE_3_MASK                              0x00000007

#define A5ETM_ETMACTR4_OFFSET                                             0x0000008C
#define A5ETM_ETMACTR4_TYPE                                               UInt32
#define A5ETM_ETMACTR4_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR4_SEC_LVL_CTRL_4_SHIFT                            10
#define    A5ETM_ETMACTR4_SEC_LVL_CTRL_4_MASK                             0x00000C00
#define    A5ETM_ETMACTR4_CONTEXT_ID_CTRL_4_SHIFT                         8
#define    A5ETM_ETMACTR4_CONTEXT_ID_CTRL_4_MASK                          0x00000300
#define    A5ETM_ETMACTR4_ACCESS_TYPE_4_SHIFT                             0
#define    A5ETM_ETMACTR4_ACCESS_TYPE_4_MASK                              0x00000007

#define A5ETM_ETMACTR5_OFFSET                                             0x00000090
#define A5ETM_ETMACTR5_TYPE                                               UInt32
#define A5ETM_ETMACTR5_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR5_SEC_LVL_CTRL_5_SHIFT                            10
#define    A5ETM_ETMACTR5_SEC_LVL_CTRL_5_MASK                             0x00000C00
#define    A5ETM_ETMACTR5_CONTEXT_ID_CTRL_5_SHIFT                         8
#define    A5ETM_ETMACTR5_CONTEXT_ID_CTRL_5_MASK                          0x00000300
#define    A5ETM_ETMACTR5_ACCESS_TYPE_5_SHIFT                             0
#define    A5ETM_ETMACTR5_ACCESS_TYPE_5_MASK                              0x00000007

#define A5ETM_ETMACTR6_OFFSET                                             0x00000094
#define A5ETM_ETMACTR6_TYPE                                               UInt32
#define A5ETM_ETMACTR6_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR6_SEC_LVL_CTRL_6_SHIFT                            10
#define    A5ETM_ETMACTR6_SEC_LVL_CTRL_6_MASK                             0x00000C00
#define    A5ETM_ETMACTR6_CONTEXT_ID_CTRL_6_SHIFT                         8
#define    A5ETM_ETMACTR6_CONTEXT_ID_CTRL_6_MASK                          0x00000300
#define    A5ETM_ETMACTR6_ACCESS_TYPE_6_SHIFT                             0
#define    A5ETM_ETMACTR6_ACCESS_TYPE_6_MASK                              0x00000007

#define A5ETM_ETMACTR7_OFFSET                                             0x00000098
#define A5ETM_ETMACTR7_TYPE                                               UInt32
#define A5ETM_ETMACTR7_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR7_SEC_LVL_CTRL_7_SHIFT                            10
#define    A5ETM_ETMACTR7_SEC_LVL_CTRL_7_MASK                             0x00000C00
#define    A5ETM_ETMACTR7_CONTEXT_ID_CTRL_7_SHIFT                         8
#define    A5ETM_ETMACTR7_CONTEXT_ID_CTRL_7_MASK                          0x00000300
#define    A5ETM_ETMACTR7_ACCESS_TYPE_7_SHIFT                             0
#define    A5ETM_ETMACTR7_ACCESS_TYPE_7_MASK                              0x00000007

#define A5ETM_ETMACTR8_OFFSET                                             0x0000009C
#define A5ETM_ETMACTR8_TYPE                                               UInt32
#define A5ETM_ETMACTR8_RESERVED_MASK                                      0xFFFFF0F8
#define    A5ETM_ETMACTR8_SEC_LVL_CTRL_8_SHIFT                            10
#define    A5ETM_ETMACTR8_SEC_LVL_CTRL_8_MASK                             0x00000C00
#define    A5ETM_ETMACTR8_CONTEXT_ID_CTRL_8_SHIFT                         8
#define    A5ETM_ETMACTR8_CONTEXT_ID_CTRL_8_MASK                          0x00000300
#define    A5ETM_ETMACTR8_ACCESS_TYPE_8_SHIFT                             0
#define    A5ETM_ETMACTR8_ACCESS_TYPE_8_MASK                              0x00000007

#define A5ETM_ETMCNTRLDVR1_OFFSET                                         0x00000140
#define A5ETM_ETMCNTRLDVR1_TYPE                                           UInt32
#define A5ETM_ETMCNTRLDVR1_RESERVED_MASK                                  0xFFFF0000
#define    A5ETM_ETMCNTRLDVR1_CTR_INIT_VALUE_1_SHIFT                      0
#define    A5ETM_ETMCNTRLDVR1_CTR_INIT_VALUE_1_MASK                       0x0000FFFF

#define A5ETM_ETMCNTRLDVR2_OFFSET                                         0x00000144
#define A5ETM_ETMCNTRLDVR2_TYPE                                           UInt32
#define A5ETM_ETMCNTRLDVR2_RESERVED_MASK                                  0xFFFF0000
#define    A5ETM_ETMCNTRLDVR2_CTR_INIT_VALUE_2_SHIFT                      0
#define    A5ETM_ETMCNTRLDVR2_CTR_INIT_VALUE_2_MASK                       0x0000FFFF

#define A5ETM_ETMCNTENR1_OFFSET                                           0x00000150
#define A5ETM_ETMCNTENR1_TYPE                                             UInt32
#define A5ETM_ETMCNTENR1_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMCNTENR1_CTR1_FUNCTION_SHIFT                           14
#define    A5ETM_ETMCNTENR1_CTR1_FUNCTION_MASK                            0x0001C000
#define    A5ETM_ETMCNTENR1_CTR1_RESOURCE_B_SHIFT                         7
#define    A5ETM_ETMCNTENR1_CTR1_RESOURCE_B_MASK                          0x00003F80
#define    A5ETM_ETMCNTENR1_CTR1_RESOURCE_A_SHIFT                         0
#define    A5ETM_ETMCNTENR1_CTR1_RESOURCE_A_MASK                          0x0000007F

#define A5ETM_ETMCNTENR2_OFFSET                                           0x00000154
#define A5ETM_ETMCNTENR2_TYPE                                             UInt32
#define A5ETM_ETMCNTENR2_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMCNTENR2_CTR2_FUNCTION_SHIFT                           14
#define    A5ETM_ETMCNTENR2_CTR2_FUNCTION_MASK                            0x0001C000
#define    A5ETM_ETMCNTENR2_CTR2_RESOURCE_B_SHIFT                         7
#define    A5ETM_ETMCNTENR2_CTR2_RESOURCE_B_MASK                          0x00003F80
#define    A5ETM_ETMCNTENR2_CTR2_RESOURCE_A_SHIFT                         0
#define    A5ETM_ETMCNTENR2_CTR2_RESOURCE_A_MASK                          0x0000007F

#define A5ETM_ETMCNTRLDEVR1_OFFSET                                        0x00000160
#define A5ETM_ETMCNTRLDEVR1_TYPE                                          UInt32
#define A5ETM_ETMCNTRLDEVR1_RESERVED_MASK                                 0xFFFE0000
#define    A5ETM_ETMCNTRLDEVR1_CTR1R_FUNCTION_SHIFT                       14
#define    A5ETM_ETMCNTRLDEVR1_CTR1R_FUNCTION_MASK                        0x0001C000
#define    A5ETM_ETMCNTRLDEVR1_CTR1R_RESOURCE_B_SHIFT                     7
#define    A5ETM_ETMCNTRLDEVR1_CTR1R_RESOURCE_B_MASK                      0x00003F80
#define    A5ETM_ETMCNTRLDEVR1_CTR1R_RESOURCE_A_SHIFT                     0
#define    A5ETM_ETMCNTRLDEVR1_CTR1R_RESOURCE_A_MASK                      0x0000007F

#define A5ETM_ETMCNTRLDEVR2_OFFSET                                        0x00000164
#define A5ETM_ETMCNTRLDEVR2_TYPE                                          UInt32
#define A5ETM_ETMCNTRLDEVR2_RESERVED_MASK                                 0xFFFE0000
#define    A5ETM_ETMCNTRLDEVR2_CTR2R_FUNCTION_SHIFT                       14
#define    A5ETM_ETMCNTRLDEVR2_CTR2R_FUNCTION_MASK                        0x0001C000
#define    A5ETM_ETMCNTRLDEVR2_CTR2R_RESOURCE_B_SHIFT                     7
#define    A5ETM_ETMCNTRLDEVR2_CTR2R_RESOURCE_B_MASK                      0x00003F80
#define    A5ETM_ETMCNTRLDEVR2_CTR2R_RESOURCE_A_SHIFT                     0
#define    A5ETM_ETMCNTRLDEVR2_CTR2R_RESOURCE_A_MASK                      0x0000007F

#define A5ETM_ETMCNTVR1_OFFSET                                            0x00000170
#define A5ETM_ETMCNTVR1_TYPE                                              UInt32
#define A5ETM_ETMCNTVR1_RESERVED_MASK                                     0xFFFF0000
#define    A5ETM_ETMCNTVR1_CTR1_VALUE_SHIFT                               0
#define    A5ETM_ETMCNTVR1_CTR1_VALUE_MASK                                0x0000FFFF

#define A5ETM_ETMCNTVR2_OFFSET                                            0x00000174
#define A5ETM_ETMCNTVR2_TYPE                                              UInt32
#define A5ETM_ETMCNTVR2_RESERVED_MASK                                     0xFFFF0000
#define    A5ETM_ETMCNTVR2_CTR2_VALUE_SHIFT                               0
#define    A5ETM_ETMCNTVR2_CTR2_VALUE_MASK                                0x0000FFFF

#define A5ETM_ETMSQ12EVR_OFFSET                                           0x00000180
#define A5ETM_ETMSQ12EVR_TYPE                                             UInt32
#define A5ETM_ETMSQ12EVR_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMSQ12EVR_SEQ12_FUNCTION_SHIFT                          14
#define    A5ETM_ETMSQ12EVR_SEQ12_FUNCTION_MASK                           0x0001C000
#define    A5ETM_ETMSQ12EVR_SEQ12_RESOURCE_B_SHIFT                        7
#define    A5ETM_ETMSQ12EVR_SEQ12_RESOURCE_B_MASK                         0x00003F80
#define    A5ETM_ETMSQ12EVR_SEQ12_RESOURCE_A_SHIFT                        0
#define    A5ETM_ETMSQ12EVR_SEQ12_RESOURCE_A_MASK                         0x0000007F

#define A5ETM_ETMSQ21EVR_OFFSET                                           0x00000184
#define A5ETM_ETMSQ21EVR_TYPE                                             UInt32
#define A5ETM_ETMSQ21EVR_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMSQ21EVR_SEQ21_FUNCTION_SHIFT                          14
#define    A5ETM_ETMSQ21EVR_SEQ21_FUNCTION_MASK                           0x0001C000
#define    A5ETM_ETMSQ21EVR_SEQ21_RESOURCE_B_SHIFT                        7
#define    A5ETM_ETMSQ21EVR_SEQ21_RESOURCE_B_MASK                         0x00003F80
#define    A5ETM_ETMSQ21EVR_SEQ21_RESOURCE_A_SHIFT                        0
#define    A5ETM_ETMSQ21EVR_SEQ21_RESOURCE_A_MASK                         0x0000007F

#define A5ETM_ETMSQ23EVR_OFFSET                                           0x00000188
#define A5ETM_ETMSQ23EVR_TYPE                                             UInt32
#define A5ETM_ETMSQ23EVR_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMSQ23EVR_SEQ23_FUNCTION_SHIFT                          14
#define    A5ETM_ETMSQ23EVR_SEQ23_FUNCTION_MASK                           0x0001C000
#define    A5ETM_ETMSQ23EVR_SEQ23_RESOURCE_B_SHIFT                        7
#define    A5ETM_ETMSQ23EVR_SEQ23_RESOURCE_B_MASK                         0x00003F80
#define    A5ETM_ETMSQ23EVR_SEQ23_RESOURCE_A_SHIFT                        0
#define    A5ETM_ETMSQ23EVR_SEQ23_RESOURCE_A_MASK                         0x0000007F

#define A5ETM_ETMSQ31EVR_OFFSET                                           0x0000018C
#define A5ETM_ETMSQ31EVR_TYPE                                             UInt32
#define A5ETM_ETMSQ31EVR_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMSQ31EVR_SEQ31_FUNCTION_SHIFT                          14
#define    A5ETM_ETMSQ31EVR_SEQ31_FUNCTION_MASK                           0x0001C000
#define    A5ETM_ETMSQ31EVR_SEQ31_RESOURCE_B_SHIFT                        7
#define    A5ETM_ETMSQ31EVR_SEQ31_RESOURCE_B_MASK                         0x00003F80
#define    A5ETM_ETMSQ31EVR_SEQ31_RESOURCE_A_SHIFT                        0
#define    A5ETM_ETMSQ31EVR_SEQ31_RESOURCE_A_MASK                         0x0000007F

#define A5ETM_ETMSQ32EVR_OFFSET                                           0x00000190
#define A5ETM_ETMSQ32EVR_TYPE                                             UInt32
#define A5ETM_ETMSQ32EVR_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMSQ32EVR_SEQ32_FUNCTION_SHIFT                          14
#define    A5ETM_ETMSQ32EVR_SEQ32_FUNCTION_MASK                           0x0001C000
#define    A5ETM_ETMSQ32EVR_SEQ32_RESOURCE_B_SHIFT                        7
#define    A5ETM_ETMSQ32EVR_SEQ32_RESOURCE_B_MASK                         0x00003F80
#define    A5ETM_ETMSQ32EVR_SEQ32_RESOURCE_A_SHIFT                        0
#define    A5ETM_ETMSQ32EVR_SEQ32_RESOURCE_A_MASK                         0x0000007F

#define A5ETM_ETMSQ13EVR_OFFSET                                           0x00000194
#define A5ETM_ETMSQ13EVR_TYPE                                             UInt32
#define A5ETM_ETMSQ13EVR_RESERVED_MASK                                    0xFFFE0000
#define    A5ETM_ETMSQ13EVR_SEQ13_FUNCTION_SHIFT                          14
#define    A5ETM_ETMSQ13EVR_SEQ13_FUNCTION_MASK                           0x0001C000
#define    A5ETM_ETMSQ13EVR_SEQ13_RESOURCE_B_SHIFT                        7
#define    A5ETM_ETMSQ13EVR_SEQ13_RESOURCE_B_MASK                         0x00003F80
#define    A5ETM_ETMSQ13EVR_SEQ13_RESOURCE_A_SHIFT                        0
#define    A5ETM_ETMSQ13EVR_SEQ13_RESOURCE_A_MASK                         0x0000007F

#define A5ETM_ETMSQR_OFFSET                                               0x0000019C
#define A5ETM_ETMSQR_TYPE                                                 UInt32
#define A5ETM_ETMSQR_RESERVED_MASK                                        0xFFFFFFFC
#define    A5ETM_ETMSQR_CUR_SEQ_STATE_SHIFT                               0
#define    A5ETM_ETMSQR_CUR_SEQ_STATE_MASK                                0x00000003

#define A5ETM_ETMEXTOUTEVR1_OFFSET                                        0x000001A0
#define A5ETM_ETMEXTOUTEVR1_TYPE                                          UInt32
#define A5ETM_ETMEXTOUTEVR1_RESERVED_MASK                                 0xFFFE0000
#define    A5ETM_ETMEXTOUTEVR1_EXTO1_FUNCTION_SHIFT                       14
#define    A5ETM_ETMEXTOUTEVR1_EXTO1_FUNCTION_MASK                        0x0001C000
#define    A5ETM_ETMEXTOUTEVR1_EXTO1_RESOURCE_B_SHIFT                     7
#define    A5ETM_ETMEXTOUTEVR1_EXTO1_RESOURCE_B_MASK                      0x00003F80
#define    A5ETM_ETMEXTOUTEVR1_EXTO1_RESOURCE_A_SHIFT                     0
#define    A5ETM_ETMEXTOUTEVR1_EXTO1_RESOURCE_A_MASK                      0x0000007F

#define A5ETM_ETMEXTOUTEVR2_OFFSET                                        0x000001A4
#define A5ETM_ETMEXTOUTEVR2_TYPE                                          UInt32
#define A5ETM_ETMEXTOUTEVR2_RESERVED_MASK                                 0xFFFE0000
#define    A5ETM_ETMEXTOUTEVR2_EXTO2_FUNCTION_SHIFT                       14
#define    A5ETM_ETMEXTOUTEVR2_EXTO2_FUNCTION_MASK                        0x0001C000
#define    A5ETM_ETMEXTOUTEVR2_EXTO2_RESOURCE_B_SHIFT                     7
#define    A5ETM_ETMEXTOUTEVR2_EXTO2_RESOURCE_B_MASK                      0x00003F80
#define    A5ETM_ETMEXTOUTEVR2_EXTO2_RESOURCE_A_SHIFT                     0
#define    A5ETM_ETMEXTOUTEVR2_EXTO2_RESOURCE_A_MASK                      0x0000007F

#define A5ETM_ETMCIDCVR1_OFFSET                                           0x000001B0
#define A5ETM_ETMCIDCVR1_TYPE                                             UInt32
#define A5ETM_ETMCIDCVR1_RESERVED_MASK                                    0x00000000
#define    A5ETM_ETMCIDCVR1_CONTEXT_ID_VAL_1_SHIFT                        0
#define    A5ETM_ETMCIDCVR1_CONTEXT_ID_VAL_1_MASK                         0xFFFFFFFF

#define A5ETM_ETMCIDCMR_OFFSET                                            0x000001BC
#define A5ETM_ETMCIDCMR_TYPE                                              UInt32
#define A5ETM_ETMCIDCMR_RESERVED_MASK                                     0x00000000
#define    A5ETM_ETMCIDCMR_CONTEXT_ID_MASK_SHIFT                          0
#define    A5ETM_ETMCIDCMR_CONTEXT_ID_MASK_MASK                           0xFFFFFFFF

#define A5ETM_ETMSYNCFR_OFFSET                                            0x000001E0
#define A5ETM_ETMSYNCFR_TYPE                                              UInt32
#define A5ETM_ETMSYNCFR_RESERVED_MASK                                     0xFFFFF007
#define    A5ETM_ETMSYNCFR_SYNC_FREQ_SHIFT                                3
#define    A5ETM_ETMSYNCFR_SYNC_FREQ_MASK                                 0x00000FF8

#define A5ETM_ETMIDR_OFFSET                                               0x000001E4
#define A5ETM_ETMIDR_TYPE                                                 UInt32
#define A5ETM_ETMIDR_RESERVED_MASK                                        0x00F30000
#define    A5ETM_ETMIDR_IMPLEMENTOR_SHIFT                                 24
#define    A5ETM_ETMIDR_IMPLEMENTOR_MASK                                  0xFF000000
#define    A5ETM_ETMIDR_SEC_EXT_SHIFT                                     19
#define    A5ETM_ETMIDR_SEC_EXT_MASK                                      0x00080000
#define    A5ETM_ETMIDR_THUMB2_SHIFT                                      18
#define    A5ETM_ETMIDR_THUMB2_MASK                                       0x00040000
#define    A5ETM_ETMIDR_ARM_CORE_SHIFT                                    12
#define    A5ETM_ETMIDR_ARM_CORE_MASK                                     0x0000F000
#define    A5ETM_ETMIDR_MAJOR_REV_SHIFT                                   8
#define    A5ETM_ETMIDR_MAJOR_REV_MASK                                    0x00000F00
#define    A5ETM_ETMIDR_MINOR_REV_SHIFT                                   4
#define    A5ETM_ETMIDR_MINOR_REV_MASK                                    0x000000F0
#define    A5ETM_ETMIDR_REV_SHIFT                                         0
#define    A5ETM_ETMIDR_REV_MASK                                          0x0000000F

#define A5ETM_ETMCCER_OFFSET                                              0x000001E8
#define A5ETM_ETMCCER_TYPE                                                UInt32
#define A5ETM_ETMCCER_RESERVED_MASK                                       0xC38F0000
#define    A5ETM_ETMCCER_TIMESTAMP_SIZE_SHIFT                             29
#define    A5ETM_ETMCCER_TIMESTAMP_SIZE_MASK                              0x20000000
#define    A5ETM_ETMCCER_TIMESTAMP_ENC_SHIFT                              28
#define    A5ETM_ETMCCER_TIMESTAMP_ENC_MASK                               0x10000000
#define    A5ETM_ETMCCER_REDUCED_FUNC_SHIFT                               27
#define    A5ETM_ETMCCER_REDUCED_FUNC_MASK                                0x08000000
#define    A5ETM_ETMCCER_VIRT_EXT_SHIFT                                   26
#define    A5ETM_ETMCCER_VIRT_EXT_MASK                                    0x04000000
#define    A5ETM_ETMCCER_TIMESTAMPING_SHIFT                               22
#define    A5ETM_ETMCCER_TIMESTAMPING_MASK                                0x00400000
#define    A5ETM_ETMCCER_ETMEIBCR_SHIFT                                   21
#define    A5ETM_ETMCCER_ETMEIBCR_MASK                                    0x00200000
#define    A5ETM_ETMCCER_TRACE_START_STOP_SHIFT                           20
#define    A5ETM_ETMCCER_TRACE_START_STOP_MASK                            0x00100000
#define    A5ETM_ETMCCER_INSTR_RESOURCES_SHIFT                            13
#define    A5ETM_ETMCCER_INSTR_RESOURCES_MASK                             0x0000E000
#define    A5ETM_ETMCCER_DATA_ADDR_COMP_SHIFT                             12
#define    A5ETM_ETMCCER_DATA_ADDR_COMP_MASK                              0x00001000
#define    A5ETM_ETMCCER_READABLE_REGS_SHIFT                              11
#define    A5ETM_ETMCCER_READABLE_REGS_MASK                               0x00000800
#define    A5ETM_ETMCCER_EXTENDED_SIZE_SHIFT                              3
#define    A5ETM_ETMCCER_EXTENDED_SIZE_MASK                               0x000007F8
#define    A5ETM_ETMCCER_EXTENDED_SELECTS_SHIFT                           0
#define    A5ETM_ETMCCER_EXTENDED_SELECTS_MASK                            0x00000007

#define A5ETM_ETMEXTINSELR_OFFSET                                         0x000001EC
#define A5ETM_ETMEXTINSELR_TYPE                                           UInt32
#define A5ETM_ETMEXTINSELR_RESERVED_MASK                                  0xFFFFC0C0
#define    A5ETM_ETMEXTINSELR_SEC_EXT_IN_SEL_SHIFT                        8
#define    A5ETM_ETMEXTINSELR_SEC_EXT_IN_SEL_MASK                         0x00003F00
#define    A5ETM_ETMEXTINSELR_FST_EXT_IN_SEL_SHIFT                        0
#define    A5ETM_ETMEXTINSELR_FST_EXT_IN_SEL_MASK                         0x0000003F

#define A5ETM_ETMTSEVR_OFFSET                                             0x000001F8
#define A5ETM_ETMTSEVR_TYPE                                               UInt32
#define A5ETM_ETMTSEVR_RESERVED_MASK                                      0xFFFE0000
#define    A5ETM_ETMTSEVR_TS_FUNCTION_SHIFT                               14
#define    A5ETM_ETMTSEVR_TS_FUNCTION_MASK                                0x0001C000
#define    A5ETM_ETMTSEVR_TS_RESOURCE_B_SHIFT                             7
#define    A5ETM_ETMTSEVR_TS_RESOURCE_B_MASK                              0x00003F80
#define    A5ETM_ETMTSEVR_TS_RESOURCE_A_SHIFT                             0
#define    A5ETM_ETMTSEVR_TS_RESOURCE_A_MASK                              0x0000007F

#define A5ETM_ETMAUXCR_OFFSET                                             0x000001FC
#define A5ETM_ETMAUXCR_TYPE                                               UInt32
#define A5ETM_ETMAUXCR_RESERVED_MASK                                      0xFFFFFFF8
#define    A5ETM_ETMAUXCR_DIS_TRACE_DATA_SHIFT                            2
#define    A5ETM_ETMAUXCR_DIS_TRACE_DATA_MASK                             0x00000004
#define    A5ETM_ETMAUXCR_TIMESTAMP_DIS_SHIFT                             1
#define    A5ETM_ETMAUXCR_TIMESTAMP_DIS_MASK                              0x00000002
#define    A5ETM_ETMAUXCR_FORCED_OVF_DIS_SHIFT                            0
#define    A5ETM_ETMAUXCR_FORCED_OVF_DIS_MASK                             0x00000001

#define A5ETM_ETMTRACEIDR_OFFSET                                          0x00000200
#define A5ETM_ETMTRACEIDR_TYPE                                            UInt32
#define A5ETM_ETMTRACEIDR_RESERVED_MASK                                   0xFFFFFF80
#define    A5ETM_ETMTRACEIDR_ATBID_SHIFT                                  0
#define    A5ETM_ETMTRACEIDR_ATBID_MASK                                   0x0000007F

#define A5ETM_ETMIDR2_OFFSET                                              0x00000208
#define A5ETM_ETMIDR2_TYPE                                                UInt32
#define A5ETM_ETMIDR2_RESERVED_MASK                                       0xFFFFFFFC
#define    A5ETM_ETMIDR2_SWP_ORDER_SHIFT                                  1
#define    A5ETM_ETMIDR2_SWP_ORDER_MASK                                   0x00000002
#define    A5ETM_ETMIDR2_RFE_ORDER_SHIFT                                  0
#define    A5ETM_ETMIDR2_RFE_ORDER_MASK                                   0x00000001

#define A5ETM_ETMPDSR_OFFSET                                              0x00000314
#define A5ETM_ETMPDSR_TYPE                                                UInt32
#define A5ETM_ETMPDSR_RESERVED_MASK                                       0xFFFFFFDC
#define    A5ETM_ETMPDSR_OS_LOCK_SHIFT                                    5
#define    A5ETM_ETMPDSR_OS_LOCK_MASK                                     0x00000020
#define    A5ETM_ETMPDSR_STICKY_STATE_SHIFT                               1
#define    A5ETM_ETMPDSR_STICKY_STATE_MASK                                0x00000002
#define    A5ETM_ETMPDSR_ETM_PWREDUP_SHIFT                                0
#define    A5ETM_ETMPDSR_ETM_PWREDUP_MASK                                 0x00000001

#define A5ETM_ITMISCOUT_OFFSET                                            0x00000EDC
#define A5ETM_ITMISCOUT_TYPE                                              UInt32
#define A5ETM_ITMISCOUT_RESERVED_MASK                                     0xFFFFFCCF
#define    A5ETM_ITMISCOUT_EXTOUT_SHIFT                                   8
#define    A5ETM_ITMISCOUT_EXTOUT_MASK                                    0x00000300
#define    A5ETM_ITMISCOUT_ETMWFXREADY_SHIFT                              5
#define    A5ETM_ITMISCOUT_ETMWFXREADY_MASK                               0x00000020
#define    A5ETM_ITMISCOUT_ETMDBGREQ_SHIFT                                4
#define    A5ETM_ITMISCOUT_ETMDBGREQ_MASK                                 0x00000010

#define A5ETM_ITMISCIN_OFFSET                                             0x00000EE0
#define A5ETM_ITMISCIN_TYPE                                               UInt32
#define A5ETM_ITMISCIN_RESERVED_MASK                                      0xFFFFFFC0
#define    A5ETM_ITMISCIN_ETMWFXPENDING_SHIFT                             5
#define    A5ETM_ITMISCIN_ETMWFXPENDING_MASK                              0x00000020
#define    A5ETM_ITMISCIN_DBGACK_SHIFT                                    4
#define    A5ETM_ITMISCIN_DBGACK_MASK                                     0x00000010
#define    A5ETM_ITMISCIN_EXTIN_SHIFT                                     0
#define    A5ETM_ITMISCIN_EXTIN_MASK                                      0x0000000F

#define A5ETM_ITTRIGGER_OFFSET                                            0x00000EE8
#define A5ETM_ITTRIGGER_TYPE                                              UInt32
#define A5ETM_ITTRIGGER_RESERVED_MASK                                     0xFFFFFFFE
#define    A5ETM_ITTRIGGER_PTMTRIGGER_SHIFT                               0
#define    A5ETM_ITTRIGGER_PTMTRIGGER_MASK                                0x00000001

#define A5ETM_ITATBDATA0_OFFSET                                           0x00000EEC
#define A5ETM_ITATBDATA0_TYPE                                             UInt32
#define A5ETM_ITATBDATA0_RESERVED_MASK                                    0xFFFFFFE0
#define    A5ETM_ITATBDATA0_ATDATAMM_SHIFT                                0
#define    A5ETM_ITATBDATA0_ATDATAMM_MASK                                 0x0000001F

#define A5ETM_ITATBCTR2_OFFSET                                            0x00000EF0
#define A5ETM_ITATBCTR2_TYPE                                              UInt32
#define A5ETM_ITATBCTR2_RESERVED_MASK                                     0xFFFFFFF8
#define    A5ETM_ITATBCTR2_SYNCREQ_SHIFT                                  2
#define    A5ETM_ITATBCTR2_SYNCREQ_MASK                                   0x00000004
#define    A5ETM_ITATBCTR2_AFVALIDMM_SHIFT                                1
#define    A5ETM_ITATBCTR2_AFVALIDMM_MASK                                 0x00000002
#define    A5ETM_ITATBCTR2_ATREADYMM_SHIFT                                0
#define    A5ETM_ITATBCTR2_ATREADYMM_MASK                                 0x00000001

#define A5ETM_ITATBCTR1_OFFSET                                            0x00000EF4
#define A5ETM_ITATBCTR1_TYPE                                              UInt32
#define A5ETM_ITATBCTR1_RESERVED_MASK                                     0xFFFFFF80
#define    A5ETM_ITATBCTR1_ATIDM_SHIFT                                    0
#define    A5ETM_ITATBCTR1_ATIDM_MASK                                     0x0000007F

#define A5ETM_ITATBCTR0_OFFSET                                            0x00000EF8
#define A5ETM_ITATBCTR0_TYPE                                              UInt32
#define A5ETM_ITATBCTR0_RESERVED_MASK                                     0xFFFFFCFC
#define    A5ETM_ITATBCTR0_ATBYTESM_SHIFT                                 8
#define    A5ETM_ITATBCTR0_ATBYTESM_MASK                                  0x00000300
#define    A5ETM_ITATBCTR0_AFREADYM_SHIFT                                 1
#define    A5ETM_ITATBCTR0_AFREADYM_MASK                                  0x00000002
#define    A5ETM_ITATBCTR0_ATVALIDM_SHIFT                                 0
#define    A5ETM_ITATBCTR0_ATVALIDM_MASK                                  0x00000001

#define A5ETM_ITCTRL_OFFSET                                               0x00000F00
#define A5ETM_ITCTRL_TYPE                                                 UInt32
#define A5ETM_ITCTRL_RESERVED_MASK                                        0xFFFFFFFE
#define    A5ETM_ITCTRL_INTEGRATION_MODE_SHIFT                            0
#define    A5ETM_ITCTRL_INTEGRATION_MODE_MASK                             0x00000001

#define A5ETM_PERID4_OFFSET                                               0x00000FD0
#define A5ETM_PERID4_TYPE                                                 UInt32
#define A5ETM_PERID4_RESERVED_MASK                                        0xFFFFFF00
#define    A5ETM_PERID4_PID4_SHIFT                                        0
#define    A5ETM_PERID4_PID4_MASK                                         0x000000FF

#define A5ETM_PERID0_OFFSET                                               0x00000FE0
#define A5ETM_PERID0_TYPE                                                 UInt32
#define A5ETM_PERID0_RESERVED_MASK                                        0xFFFFFF00
#define    A5ETM_PERID0_PID0_SHIFT                                        0
#define    A5ETM_PERID0_PID0_MASK                                         0x000000FF

#define A5ETM_PERID1_OFFSET                                               0x00000FE4
#define A5ETM_PERID1_TYPE                                                 UInt32
#define A5ETM_PERID1_RESERVED_MASK                                        0xFFFFFF00
#define    A5ETM_PERID1_PID1_SHIFT                                        0
#define    A5ETM_PERID1_PID1_MASK                                         0x000000FF

#define A5ETM_PERID2_OFFSET                                               0x00000FE8
#define A5ETM_PERID2_TYPE                                                 UInt32
#define A5ETM_PERID2_RESERVED_MASK                                        0xFFFFFF00
#define    A5ETM_PERID2_PID2_SHIFT                                        0
#define    A5ETM_PERID2_PID2_MASK                                         0x000000FF

#define A5ETM_PERID3_OFFSET                                               0x00000FEC
#define A5ETM_PERID3_TYPE                                                 UInt32
#define A5ETM_PERID3_RESERVED_MASK                                        0xFFFFFF00
#define    A5ETM_PERID3_PID3_SHIFT                                        0
#define    A5ETM_PERID3_PID3_MASK                                         0x000000FF

#define A5ETM_COMPID0_OFFSET                                              0x00000FF0
#define A5ETM_COMPID0_TYPE                                                UInt32
#define A5ETM_COMPID0_RESERVED_MASK                                       0xFFFFFF00
#define    A5ETM_COMPID0_CID0_SHIFT                                       0
#define    A5ETM_COMPID0_CID0_MASK                                        0x000000FF

#define A5ETM_COMPID1_OFFSET                                              0x00000FF4
#define A5ETM_COMPID1_TYPE                                                UInt32
#define A5ETM_COMPID1_RESERVED_MASK                                       0xFFFFFF00
#define    A5ETM_COMPID1_CID1_SHIFT                                       0
#define    A5ETM_COMPID1_CID1_MASK                                        0x000000FF

#define A5ETM_COMPID2_OFFSET                                              0x00000FF8
#define A5ETM_COMPID2_TYPE                                                UInt32
#define A5ETM_COMPID2_RESERVED_MASK                                       0xFFFFFF00
#define    A5ETM_COMPID2_CID2_SHIFT                                       0
#define    A5ETM_COMPID2_CID2_MASK                                        0x000000FF

#define A5ETM_COMPID3_OFFSET                                              0x00000FFC
#define A5ETM_COMPID3_TYPE                                                UInt32
#define A5ETM_COMPID3_RESERVED_MASK                                       0xFFFFFF00
#define    A5ETM_COMPID3_CID3_SHIFT                                       0
#define    A5ETM_COMPID3_CID3_MASK                                        0x000000FF

#endif /* __BRCM_RDB_A5ETM_H__ */



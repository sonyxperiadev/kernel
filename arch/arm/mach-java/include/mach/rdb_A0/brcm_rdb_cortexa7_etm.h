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

#ifndef __BRCM_RDB_CORTEXA7_ETM_H__
#define __BRCM_RDB_CORTEXA7_ETM_H__

#define CORTEXA7_ETM_CR_OFFSET                                            0x00000000
#define CORTEXA7_ETM_CR_TYPE                                              UInt32
#define CORTEXA7_ETM_CR_RESERVED_MASK                                     0x80000000
#define    CORTEXA7_ETM_CR_VMID_TRACE_EN_SHIFT                            30
#define    CORTEXA7_ETM_CR_VMID_TRACE_EN_MASK                             0x40000000
#define    CORTEXA7_ETM_CR_RETURN_STACK_EN_SHIFT                          29
#define    CORTEXA7_ETM_CR_RETURN_STACK_EN_MASK                           0x20000000
#define    CORTEXA7_ETM_CR_TIMESTAMP_EN_SHIFT                             28
#define    CORTEXA7_ETM_CR_TIMESTAMP_EN_MASK                              0x10000000
#define    CORTEXA7_ETM_CR_PROCESSOR_SEL_SHIFT                            25
#define    CORTEXA7_ETM_CR_PROCESSOR_SEL_MASK                             0x0E000000
#define    CORTEXA7_ETM_CR_INSTR_RES_ACC_SHIFT                            24
#define    CORTEXA7_ETM_CR_INSTR_RES_ACC_MASK                             0x01000000
#define    CORTEXA7_ETM_CR_DIS_SOFT_WR_SHIFT                              23
#define    CORTEXA7_ETM_CR_DIS_SOFT_WR_MASK                               0x00800000
#define    CORTEXA7_ETM_CR_DIS_REG_WR_SHIFT                               22
#define    CORTEXA7_ETM_CR_DIS_REG_WR_MASK                                0x00400000
#define    CORTEXA7_ETM_CR_PORT_SIZE_3_SHIFT                              21
#define    CORTEXA7_ETM_CR_PORT_SIZE_3_MASK                               0x00200000
#define    CORTEXA7_ETM_CR_DATA_ONLY_SHIFT                                20
#define    CORTEXA7_ETM_CR_DATA_ONLY_MASK                                 0x00100000
#define    CORTEXA7_ETM_CR_FIL_CPRT_SHIFT                                 19
#define    CORTEXA7_ETM_CR_FIL_CPRT_MASK                                  0x00080000
#define    CORTEXA7_ETM_CR_SUP_DATA_SHIFT                                 18
#define    CORTEXA7_ETM_CR_SUP_DATA_MASK                                  0x00040000
#define    CORTEXA7_ETM_CR_PORT_MOD_1_0_SHIFT                             16
#define    CORTEXA7_ETM_CR_PORT_MOD_1_0_MASK                              0x00030000
#define    CORTEXA7_ETM_CR_CONTEXTID_SIZE_SHIFT                           14
#define    CORTEXA7_ETM_CR_CONTEXTID_SIZE_MASK                            0x0000C000
#define    CORTEXA7_ETM_CR_PORT_MOD_2_SHIFT                               13
#define    CORTEXA7_ETM_CR_PORT_MOD_2_MASK                                0x00002000
#define    CORTEXA7_ETM_CR_CYCLE_ACC_EN_SHIFT                             12
#define    CORTEXA7_ETM_CR_CYCLE_ACC_EN_MASK                              0x00001000
#define    CORTEXA7_ETM_CR_PORT_SEL_SHIFT                                 11
#define    CORTEXA7_ETM_CR_PORT_SEL_MASK                                  0x00000800
#define    CORTEXA7_ETM_CR_PROG_BIT_SHIFT                                 10
#define    CORTEXA7_ETM_CR_PROG_BIT_MASK                                  0x00000400
#define    CORTEXA7_ETM_CR_DEBUG_REQ_CTRL_SHIFT                           9
#define    CORTEXA7_ETM_CR_DEBUG_REQ_CTRL_MASK                            0x00000200
#define    CORTEXA7_ETM_CR_BRANCH_OUT_SHIFT                               8
#define    CORTEXA7_ETM_CR_BRANCH_OUT_MASK                                0x00000100
#define    CORTEXA7_ETM_CR_STALL_PROC_SHIFT                               7
#define    CORTEXA7_ETM_CR_STALL_PROC_MASK                                0x00000080
#define    CORTEXA7_ETM_CR_PORT_SIZE_2_0_SHIFT                            4
#define    CORTEXA7_ETM_CR_PORT_SIZE_2_0_MASK                             0x00000070
#define    CORTEXA7_ETM_CR_DATA_ACC_SHIFT                                 2
#define    CORTEXA7_ETM_CR_DATA_ACC_MASK                                  0x0000000C
#define    CORTEXA7_ETM_CR_MON_CPRT_SHIFT                                 1
#define    CORTEXA7_ETM_CR_MON_CPRT_MASK                                  0x00000002
#define    CORTEXA7_ETM_CR_POWER_DOWN_SHIFT                               0
#define    CORTEXA7_ETM_CR_POWER_DOWN_MASK                                0x00000001

#define CORTEXA7_ETM_CCR_OFFSET                                           0x00000004
#define CORTEXA7_ETM_CCR_TYPE                                             UInt32
#define CORTEXA7_ETM_CCR_RESERVED_MASK                                    0x00000000
#define    CORTEXA7_ETM_CCR_CCR_SHIFT                                     0
#define    CORTEXA7_ETM_CCR_CCR_MASK                                      0xFFFFFFFF

#define CORTEXA7_ETM_TRIGGER_OFFSET                                       0x00000008
#define CORTEXA7_ETM_TRIGGER_TYPE                                         UInt32
#define CORTEXA7_ETM_TRIGGER_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_TRIGGER_TRIGGER_SHIFT                             0
#define    CORTEXA7_ETM_TRIGGER_TRIGGER_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_ASICCR_OFFSET                                        0x0000000C
#define CORTEXA7_ETM_ASICCR_TYPE                                          UInt32
#define CORTEXA7_ETM_ASICCR_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_ETM_ASICCR_ASICCR_SHIFT                               0
#define    CORTEXA7_ETM_ASICCR_ASICCR_MASK                                0xFFFFFFFF

#define CORTEXA7_ETM_SR_OFFSET                                            0x00000010
#define CORTEXA7_ETM_SR_TYPE                                              UInt32
#define CORTEXA7_ETM_SR_RESERVED_MASK                                     0xFFFFFFF0
#define    CORTEXA7_ETM_SR_TRIG_BIT_S_SHIFT                               3
#define    CORTEXA7_ETM_SR_TRIG_BIT_S_MASK                                0x00000008
#define    CORTEXA7_ETM_SR_TRACE_RES_S_SHIFT                              2
#define    CORTEXA7_ETM_SR_TRACE_RES_S_MASK                               0x00000004
#define    CORTEXA7_ETM_SR_PROG_BIT_S_SHIFT                               1
#define    CORTEXA7_ETM_SR_PROG_BIT_S_MASK                                0x00000002
#define    CORTEXA7_ETM_SR_UNTR_OVERFLOW_S_SHIFT                          0
#define    CORTEXA7_ETM_SR_UNTR_OVERFLOW_S_MASK                           0x00000001

#define CORTEXA7_ETM_SCR_OFFSET                                           0x00000014
#define CORTEXA7_ETM_SCR_TYPE                                             UInt32
#define CORTEXA7_ETM_SCR_RESERVED_MASK                                    0x00000000
#define    CORTEXA7_ETM_SCR_SCR_SHIFT                                     0
#define    CORTEXA7_ETM_SCR_SCR_MASK                                      0xFFFFFFFF

#define CORTEXA7_ETM_TSSCR_OFFSET                                         0x00000018
#define CORTEXA7_ETM_TSSCR_TYPE                                           UInt32
#define CORTEXA7_ETM_TSSCR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_TSSCR_TSSCR_SHIFT                                 0
#define    CORTEXA7_ETM_TSSCR_TSSCR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_TECR2_OFFSET                                         0x0000001C
#define CORTEXA7_ETM_TECR2_TYPE                                           UInt32
#define CORTEXA7_ETM_TECR2_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_TECR2_TECR2_SHIFT                                 0
#define    CORTEXA7_ETM_TECR2_TECR2_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_TEEVR_OFFSET                                         0x00000020
#define CORTEXA7_ETM_TEEVR_TYPE                                           UInt32
#define CORTEXA7_ETM_TEEVR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_TEEVR_TEEVR_SHIFT                                 0
#define    CORTEXA7_ETM_TEEVR_TEEVR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_TECR1_OFFSET                                         0x00000024
#define CORTEXA7_ETM_TECR1_TYPE                                           UInt32
#define CORTEXA7_ETM_TECR1_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_TECR1_TECR1_SHIFT                                 0
#define    CORTEXA7_ETM_TECR1_TECR1_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_FFLR_OFFSET                                          0x0000002C
#define CORTEXA7_ETM_FFLR_TYPE                                            UInt32
#define CORTEXA7_ETM_FFLR_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_FFLR_FFLR_SHIFT                                   0
#define    CORTEXA7_ETM_FFLR_FFLR_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_DEVR_OFFSET                                          0x00000030
#define CORTEXA7_ETM_DEVR_TYPE                                            UInt32
#define CORTEXA7_ETM_DEVR_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_DEVR_DEVR_SHIFT                                   0
#define    CORTEXA7_ETM_DEVR_DEVR_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_VDCR1_OFFSET                                         0x00000034
#define CORTEXA7_ETM_VDCR1_TYPE                                           UInt32
#define CORTEXA7_ETM_VDCR1_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_VDCR1_VDCR1_SHIFT                                 0
#define    CORTEXA7_ETM_VDCR1_VDCR1_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_VDCR3_OFFSET                                         0x0000003C
#define CORTEXA7_ETM_VDCR3_TYPE                                           UInt32
#define CORTEXA7_ETM_VDCR3_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_VDCR3_VDCR3_SHIFT                                 0
#define    CORTEXA7_ETM_VDCR3_VDCR3_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR1_OFFSET                                         0x00000040
#define CORTEXA7_ETM_ACVR1_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR1_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR1_ACVR1_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR1_ACVR1_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR2_OFFSET                                         0x00000044
#define CORTEXA7_ETM_ACVR2_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR2_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR2_ACVR2_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR2_ACVR2_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR3_OFFSET                                         0x00000048
#define CORTEXA7_ETM_ACVR3_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR3_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR3_ACVR3_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR3_ACVR3_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR4_OFFSET                                         0x0000004C
#define CORTEXA7_ETM_ACVR4_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR4_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR4_ACVR4_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR4_ACVR4_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR5_OFFSET                                         0x00000050
#define CORTEXA7_ETM_ACVR5_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR5_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR5_ACVR5_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR5_ACVR5_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR6_OFFSET                                         0x00000054
#define CORTEXA7_ETM_ACVR6_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR6_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR6_ACVR6_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR6_ACVR6_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR7_OFFSET                                         0x00000058
#define CORTEXA7_ETM_ACVR7_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR7_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR7_ACVR7_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR7_ACVR7_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACVR8_OFFSET                                         0x0000005C
#define CORTEXA7_ETM_ACVR8_TYPE                                           UInt32
#define CORTEXA7_ETM_ACVR8_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACVR8_ACVR8_SHIFT                                 0
#define    CORTEXA7_ETM_ACVR8_ACVR8_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR1_OFFSET                                         0x00000080
#define CORTEXA7_ETM_ACTR1_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR1_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR1_ACTR1_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR1_ACTR1_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR2_OFFSET                                         0x00000084
#define CORTEXA7_ETM_ACTR2_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR2_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR2_ACTR2_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR2_ACTR2_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR3_OFFSET                                         0x00000088
#define CORTEXA7_ETM_ACTR3_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR3_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR3_ACTR3_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR3_ACTR3_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR4_OFFSET                                         0x0000008C
#define CORTEXA7_ETM_ACTR4_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR4_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR4_ACTR4_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR4_ACTR4_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR5_OFFSET                                         0x00000090
#define CORTEXA7_ETM_ACTR5_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR5_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR5_ACTR5_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR5_ACTR5_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR6_OFFSET                                         0x00000094
#define CORTEXA7_ETM_ACTR6_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR6_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR6_ACTR6_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR6_ACTR6_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR7_OFFSET                                         0x00000098
#define CORTEXA7_ETM_ACTR7_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR7_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR7_ACTR7_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR7_ACTR7_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_ACTR8_OFFSET                                         0x0000009C
#define CORTEXA7_ETM_ACTR8_TYPE                                           UInt32
#define CORTEXA7_ETM_ACTR8_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_ACTR8_ACTR8_SHIFT                                 0
#define    CORTEXA7_ETM_ACTR8_ACTR8_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_DCVR1_OFFSET                                         0x000000C0
#define CORTEXA7_ETM_DCVR1_TYPE                                           UInt32
#define CORTEXA7_ETM_DCVR1_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_DCVR1_DCVR1_SHIFT                                 0
#define    CORTEXA7_ETM_DCVR1_DCVR1_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_DCVR3_OFFSET                                         0x000000C8
#define CORTEXA7_ETM_DCVR3_TYPE                                           UInt32
#define CORTEXA7_ETM_DCVR3_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_DCVR3_DCVR3_SHIFT                                 0
#define    CORTEXA7_ETM_DCVR3_DCVR3_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_DCMR1_OFFSET                                         0x00000100
#define CORTEXA7_ETM_DCMR1_TYPE                                           UInt32
#define CORTEXA7_ETM_DCMR1_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_DCMR1_DCMR1_SHIFT                                 0
#define    CORTEXA7_ETM_DCMR1_DCMR1_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_DCMR3_OFFSET                                         0x00000108
#define CORTEXA7_ETM_DCMR3_TYPE                                           UInt32
#define CORTEXA7_ETM_DCMR3_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_DCMR3_DCMR3_SHIFT                                 0
#define    CORTEXA7_ETM_DCMR3_DCMR3_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_CNTRLDVR1_OFFSET                                     0x00000140
#define CORTEXA7_ETM_CNTRLDVR1_TYPE                                       UInt32
#define CORTEXA7_ETM_CNTRLDVR1_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_CNTRLDVR1_CNTRLDVR1_SHIFT                         0
#define    CORTEXA7_ETM_CNTRLDVR1_CNTRLDVR1_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_CNTRLDVR2_OFFSET                                     0x00000144
#define CORTEXA7_ETM_CNTRLDVR2_TYPE                                       UInt32
#define CORTEXA7_ETM_CNTRLDVR2_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_CNTRLDVR2_CNTRLDVR2_SHIFT                         0
#define    CORTEXA7_ETM_CNTRLDVR2_CNTRLDVR2_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_CNTENR1_OFFSET                                       0x00000150
#define CORTEXA7_ETM_CNTENR1_TYPE                                         UInt32
#define CORTEXA7_ETM_CNTENR1_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_CNTENR1_CNTENR1_SHIFT                             0
#define    CORTEXA7_ETM_CNTENR1_CNTENR1_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_CNTENR2_OFFSET                                       0x00000154
#define CORTEXA7_ETM_CNTENR2_TYPE                                         UInt32
#define CORTEXA7_ETM_CNTENR2_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_CNTENR2_CNTENR2_SHIFT                             0
#define    CORTEXA7_ETM_CNTENR2_CNTENR2_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_CNTRLDEVR1_OFFSET                                    0x00000160
#define CORTEXA7_ETM_CNTRLDEVR1_TYPE                                      UInt32
#define CORTEXA7_ETM_CNTRLDEVR1_RESERVED_MASK                             0x00000000
#define    CORTEXA7_ETM_CNTRLDEVR1_CNTRLDEVR1_SHIFT                       0
#define    CORTEXA7_ETM_CNTRLDEVR1_CNTRLDEVR1_MASK                        0xFFFFFFFF

#define CORTEXA7_ETM_CNTRLDEVR2_OFFSET                                    0x00000164
#define CORTEXA7_ETM_CNTRLDEVR2_TYPE                                      UInt32
#define CORTEXA7_ETM_CNTRLDEVR2_RESERVED_MASK                             0x00000000
#define    CORTEXA7_ETM_CNTRLDEVR2_CNTRLDEVR2_SHIFT                       0
#define    CORTEXA7_ETM_CNTRLDEVR2_CNTRLDEVR2_MASK                        0xFFFFFFFF

#define CORTEXA7_ETM_CNTVR1_OFFSET                                        0x00000170
#define CORTEXA7_ETM_CNTVR1_TYPE                                          UInt32
#define CORTEXA7_ETM_CNTVR1_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_ETM_CNTVR1_CNTVR1_SHIFT                               0
#define    CORTEXA7_ETM_CNTVR1_CNTVR1_MASK                                0xFFFFFFFF

#define CORTEXA7_ETM_CNTVR2_OFFSET                                        0x00000174
#define CORTEXA7_ETM_CNTVR2_TYPE                                          UInt32
#define CORTEXA7_ETM_CNTVR2_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_ETM_CNTVR2_CNTVR2_SHIFT                               0
#define    CORTEXA7_ETM_CNTVR2_CNTVR2_MASK                                0xFFFFFFFF

#define CORTEXA7_ETM_SQ12EVR_OFFSET                                       0x00000180
#define CORTEXA7_ETM_SQ12EVR_TYPE                                         UInt32
#define CORTEXA7_ETM_SQ12EVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_SQ12EVR_SQ12EVR_SHIFT                             0
#define    CORTEXA7_ETM_SQ12EVR_SQ12EVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_SQ21EVR_OFFSET                                       0x00000184
#define CORTEXA7_ETM_SQ21EVR_TYPE                                         UInt32
#define CORTEXA7_ETM_SQ21EVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_SQ21EVR_SQ21EVR_SHIFT                             0
#define    CORTEXA7_ETM_SQ21EVR_SQ21EVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_SQ23EVR_OFFSET                                       0x00000188
#define CORTEXA7_ETM_SQ23EVR_TYPE                                         UInt32
#define CORTEXA7_ETM_SQ23EVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_SQ23EVR_SQ23EVR_SHIFT                             0
#define    CORTEXA7_ETM_SQ23EVR_SQ23EVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_SQ31EVR_OFFSET                                       0x0000018C
#define CORTEXA7_ETM_SQ31EVR_TYPE                                         UInt32
#define CORTEXA7_ETM_SQ31EVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_SQ31EVR_SQ31EVR_SHIFT                             0
#define    CORTEXA7_ETM_SQ31EVR_SQ31EVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_SQ32EVR_OFFSET                                       0x00000190
#define CORTEXA7_ETM_SQ32EVR_TYPE                                         UInt32
#define CORTEXA7_ETM_SQ32EVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_SQ32EVR_SQ32EVR_SHIFT                             0
#define    CORTEXA7_ETM_SQ32EVR_SQ32EVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_SQ13EVR_OFFSET                                       0x00000194
#define CORTEXA7_ETM_SQ13EVR_TYPE                                         UInt32
#define CORTEXA7_ETM_SQ13EVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_SQ13EVR_SQ13EVR_SHIFT                             0
#define    CORTEXA7_ETM_SQ13EVR_SQ13EVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_SQR_OFFSET                                           0x0000019C
#define CORTEXA7_ETM_SQR_TYPE                                             UInt32
#define CORTEXA7_ETM_SQR_RESERVED_MASK                                    0x00000000
#define    CORTEXA7_ETM_SQR_SQR_SHIFT                                     0
#define    CORTEXA7_ETM_SQR_SQR_MASK                                      0xFFFFFFFF

#define CORTEXA7_ETM_EXTOUTEVR1_OFFSET                                    0x000001A0
#define CORTEXA7_ETM_EXTOUTEVR1_TYPE                                      UInt32
#define CORTEXA7_ETM_EXTOUTEVR1_RESERVED_MASK                             0x00000000
#define    CORTEXA7_ETM_EXTOUTEVR1_EXTOUTEVR1_SHIFT                       0
#define    CORTEXA7_ETM_EXTOUTEVR1_EXTOUTEVR1_MASK                        0xFFFFFFFF

#define CORTEXA7_ETM_EXTOUTEVR2_OFFSET                                    0x000001A4
#define CORTEXA7_ETM_EXTOUTEVR2_TYPE                                      UInt32
#define CORTEXA7_ETM_EXTOUTEVR2_RESERVED_MASK                             0x00000000
#define    CORTEXA7_ETM_EXTOUTEVR2_EXTOUTEVR2_SHIFT                       0
#define    CORTEXA7_ETM_EXTOUTEVR2_EXTOUTEVR2_MASK                        0xFFFFFFFF

#define CORTEXA7_ETM_CIDCVR1_OFFSET                                       0x000001B0
#define CORTEXA7_ETM_CIDCVR1_TYPE                                         UInt32
#define CORTEXA7_ETM_CIDCVR1_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_CIDCVR1_CIDCVR1_SHIFT                             0
#define    CORTEXA7_ETM_CIDCVR1_CIDCVR1_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_CIDCMR_OFFSET                                        0x000001BC
#define CORTEXA7_ETM_CIDCMR_TYPE                                          UInt32
#define CORTEXA7_ETM_CIDCMR_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_ETM_CIDCMR_CIDCMR_SHIFT                               0
#define    CORTEXA7_ETM_CIDCMR_CIDCMR_MASK                                0xFFFFFFFF

#define CORTEXA7_ETM_SYNCFR_OFFSET                                        0x000001E0
#define CORTEXA7_ETM_SYNCFR_TYPE                                          UInt32
#define CORTEXA7_ETM_SYNCFR_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_ETM_SYNCFR_SYNCFR_SHIFT                               0
#define    CORTEXA7_ETM_SYNCFR_SYNCFR_MASK                                0xFFFFFFFF

#define CORTEXA7_ETM_MDIR_OFFSET                                          0x000001E4
#define CORTEXA7_ETM_MDIR_TYPE                                            UInt32
#define CORTEXA7_ETM_MDIR_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_MDIR_MDIR_SHIFT                                   0
#define    CORTEXA7_ETM_MDIR_MDIR_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_CCER_OFFSET                                          0x000001E8
#define CORTEXA7_ETM_CCER_TYPE                                            UInt32
#define CORTEXA7_ETM_CCER_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_CCER_CCER_SHIFT                                   0
#define    CORTEXA7_ETM_CCER_CCER_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_EXTINSELR_OFFSET                                     0x000001EC
#define CORTEXA7_ETM_EXTINSELR_TYPE                                       UInt32
#define CORTEXA7_ETM_EXTINSELR_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_EXTINSELR_EXTINSELR_SHIFT                         0
#define    CORTEXA7_ETM_EXTINSELR_EXTINSELR_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_TSEVR_OFFSET                                         0x000001F8
#define CORTEXA7_ETM_TSEVR_TYPE                                           UInt32
#define CORTEXA7_ETM_TSEVR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_TSEVR_TSEVR_SHIFT                                 0
#define    CORTEXA7_ETM_TSEVR_TSEVR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_AUXCR_OFFSET                                         0x000001FC
#define CORTEXA7_ETM_AUXCR_TYPE                                           UInt32
#define CORTEXA7_ETM_AUXCR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_AUXCR_AUXCR_SHIFT                                 0
#define    CORTEXA7_ETM_AUXCR_AUXCR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_TRACEIDR_OFFSET                                      0x00000200
#define CORTEXA7_ETM_TRACEIDR_TYPE                                        UInt32
#define CORTEXA7_ETM_TRACEIDR_RESERVED_MASK                               0x00000000
#define    CORTEXA7_ETM_TRACEIDR_TRACEIDR_SHIFT                           0
#define    CORTEXA7_ETM_TRACEIDR_TRACEIDR_MASK                            0xFFFFFFFF

#define CORTEXA7_ETM_MIDR2_OFFSET                                         0x00000208
#define CORTEXA7_ETM_MIDR2_TYPE                                           UInt32
#define CORTEXA7_ETM_MIDR2_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_MIDR2_MIDR2_SHIFT                                 0
#define    CORTEXA7_ETM_MIDR2_MIDR2_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_VMIDCVR_OFFSET                                       0x00000240
#define CORTEXA7_ETM_VMIDCVR_TYPE                                         UInt32
#define CORTEXA7_ETM_VMIDCVR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_VMIDCVR_VMIDCVR_SHIFT                             0
#define    CORTEXA7_ETM_VMIDCVR_VMIDCVR_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_OSLAR_OFFSET                                         0x00000300
#define CORTEXA7_ETM_OSLAR_TYPE                                           UInt32
#define CORTEXA7_ETM_OSLAR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_OSLAR_OSLAR_SHIFT                                 0
#define    CORTEXA7_ETM_OSLAR_OSLAR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_OSLSR_OFFSET                                         0x00000304
#define CORTEXA7_ETM_OSLSR_TYPE                                           UInt32
#define CORTEXA7_ETM_OSLSR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_OSLSR_OSLSR_SHIFT                                 0
#define    CORTEXA7_ETM_OSLSR_OSLSR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_OSSRR_OFFSET                                         0x00000308
#define CORTEXA7_ETM_OSSRR_TYPE                                           UInt32
#define CORTEXA7_ETM_OSSRR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_OSSRR_OSSRR_SHIFT                                 0
#define    CORTEXA7_ETM_OSSRR_OSSRR_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_PDCR_OFFSET                                          0x00000310
#define CORTEXA7_ETM_PDCR_TYPE                                            UInt32
#define CORTEXA7_ETM_PDCR_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PDCR_PDCR_SHIFT                                   0
#define    CORTEXA7_ETM_PDCR_PDCR_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PDSR_OFFSET                                          0x00000314
#define CORTEXA7_ETM_PDSR_TYPE                                            UInt32
#define CORTEXA7_ETM_PDSR_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PDSR_PDSR_SHIFT                                   0
#define    CORTEXA7_ETM_PDSR_PDSR_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_ITMISCOUT_OFFSET                                     0x00000EDC
#define CORTEXA7_ETM_ITMISCOUT_TYPE                                       UInt32
#define CORTEXA7_ETM_ITMISCOUT_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_ITMISCOUT_ITMISCOUT_SHIFT                         0
#define    CORTEXA7_ETM_ITMISCOUT_ITMISCOUT_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_ITMISCIN_OFFSET                                      0x00000EE0
#define CORTEXA7_ETM_ITMISCIN_TYPE                                        UInt32
#define CORTEXA7_ETM_ITMISCIN_RESERVED_MASK                               0x00000000
#define    CORTEXA7_ETM_ITMISCIN_ITMISCIN_SHIFT                           0
#define    CORTEXA7_ETM_ITMISCIN_ITMISCIN_MASK                            0xFFFFFFFF

#define CORTEXA7_ETM_ITTRIGGERREQ_OFFSET                                  0x00000EE8
#define CORTEXA7_ETM_ITTRIGGERREQ_TYPE                                    UInt32
#define CORTEXA7_ETM_ITTRIGGERREQ_RESERVED_MASK                           0x00000000
#define    CORTEXA7_ETM_ITTRIGGERREQ_ITTRIGGERREQ_SHIFT                   0
#define    CORTEXA7_ETM_ITTRIGGERREQ_ITTRIGGERREQ_MASK                    0xFFFFFFFF

#define CORTEXA7_ETM_ITATBDATA0_OFFSET                                    0x00000EEC
#define CORTEXA7_ETM_ITATBDATA0_TYPE                                      UInt32
#define CORTEXA7_ETM_ITATBDATA0_RESERVED_MASK                             0x00000000
#define    CORTEXA7_ETM_ITATBDATA0_ITATBDATA0_SHIFT                       0
#define    CORTEXA7_ETM_ITATBDATA0_ITATBDATA0_MASK                        0xFFFFFFFF

#define CORTEXA7_ETM_ITATBCTR2_OFFSET                                     0x00000EF0
#define CORTEXA7_ETM_ITATBCTR2_TYPE                                       UInt32
#define CORTEXA7_ETM_ITATBCTR2_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_ITATBCTR2_ITATBCTR2_SHIFT                         0
#define    CORTEXA7_ETM_ITATBCTR2_ITATBCTR2_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_ITATBCTR1_OFFSET                                     0x00000EF4
#define CORTEXA7_ETM_ITATBCTR1_TYPE                                       UInt32
#define CORTEXA7_ETM_ITATBCTR1_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_ITATBCTR1_ITATBCTR1_SHIFT                         0
#define    CORTEXA7_ETM_ITATBCTR1_ITATBCTR1_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_ITATBCTR0_OFFSET                                     0x00000EF8
#define CORTEXA7_ETM_ITATBCTR0_TYPE                                       UInt32
#define CORTEXA7_ETM_ITATBCTR0_RESERVED_MASK                              0x00000000
#define    CORTEXA7_ETM_ITATBCTR0_ITATBCTR0_SHIFT                         0
#define    CORTEXA7_ETM_ITATBCTR0_ITATBCTR0_MASK                          0xFFFFFFFF

#define CORTEXA7_ETM_ITCTRL_OFFSET                                        0x00000F00
#define CORTEXA7_ETM_ITCTRL_TYPE                                          UInt32
#define CORTEXA7_ETM_ITCTRL_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_ETM_ITCTRL_ITCTRL_SHIFT                               0
#define    CORTEXA7_ETM_ITCTRL_ITCTRL_MASK                                0xFFFFFFFF

#define CORTEXA7_ETM_CLAIMSET_OFFSET                                      0x00000FA0
#define CORTEXA7_ETM_CLAIMSET_TYPE                                        UInt32
#define CORTEXA7_ETM_CLAIMSET_RESERVED_MASK                               0x00000000
#define    CORTEXA7_ETM_CLAIMSET_CLAIMSET_SHIFT                           0
#define    CORTEXA7_ETM_CLAIMSET_CLAIMSET_MASK                            0xFFFFFFFF

#define CORTEXA7_ETM_CLAIMCLR_OFFSET                                      0x00000FA4
#define CORTEXA7_ETM_CLAIMCLR_TYPE                                        UInt32
#define CORTEXA7_ETM_CLAIMCLR_RESERVED_MASK                               0x00000000
#define    CORTEXA7_ETM_CLAIMCLR_CLAIMCLR_SHIFT                           0
#define    CORTEXA7_ETM_CLAIMCLR_CLAIMCLR_MASK                            0xFFFFFFFF

#define CORTEXA7_ETM_LAR_OFFSET                                           0x00000FB0
#define CORTEXA7_ETM_LAR_TYPE                                             UInt32
#define CORTEXA7_ETM_LAR_RESERVED_MASK                                    0x00000000
#define    CORTEXA7_ETM_LAR_LAR_SHIFT                                     0
#define    CORTEXA7_ETM_LAR_LAR_MASK                                      0xFFFFFFFF

#define CORTEXA7_ETM_LSR_OFFSET                                           0x00000FB4
#define CORTEXA7_ETM_LSR_TYPE                                             UInt32
#define CORTEXA7_ETM_LSR_RESERVED_MASK                                    0x00000000
#define    CORTEXA7_ETM_LSR_LSR_SHIFT                                     0
#define    CORTEXA7_ETM_LSR_LSR_MASK                                      0xFFFFFFFF

#define CORTEXA7_ETM_AUTHSTATUS_OFFSET                                    0x00000FB8
#define CORTEXA7_ETM_AUTHSTATUS_TYPE                                      UInt32
#define CORTEXA7_ETM_AUTHSTATUS_RESERVED_MASK                             0x00000000
#define    CORTEXA7_ETM_AUTHSTATUS_AUTHSTATUS_SHIFT                       0
#define    CORTEXA7_ETM_AUTHSTATUS_AUTHSTATUS_MASK                        0xFFFFFFFF

#define CORTEXA7_ETM_DEVID_OFFSET                                         0x00000FC8
#define CORTEXA7_ETM_DEVID_TYPE                                           UInt32
#define CORTEXA7_ETM_DEVID_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_ETM_DEVID_DEVID_SHIFT                                 0
#define    CORTEXA7_ETM_DEVID_DEVID_MASK                                  0xFFFFFFFF

#define CORTEXA7_ETM_DEVTYPE_OFFSET                                       0x00000FCC
#define CORTEXA7_ETM_DEVTYPE_TYPE                                         UInt32
#define CORTEXA7_ETM_DEVTYPE_RESERVED_MASK                                0x00000000
#define    CORTEXA7_ETM_DEVTYPE_DEVTYPE_SHIFT                             0
#define    CORTEXA7_ETM_DEVTYPE_DEVTYPE_MASK                              0xFFFFFFFF

#define CORTEXA7_ETM_PIR4_OFFSET                                          0x00000FD0
#define CORTEXA7_ETM_PIR4_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR4_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR4_PIR4_SHIFT                                   0
#define    CORTEXA7_ETM_PIR4_PIR4_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR5_OFFSET                                          0x00000FD4
#define CORTEXA7_ETM_PIR5_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR5_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR5_PIR5_SHIFT                                   0
#define    CORTEXA7_ETM_PIR5_PIR5_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR6_OFFSET                                          0x00000FD8
#define CORTEXA7_ETM_PIR6_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR6_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR6_PIR6_SHIFT                                   0
#define    CORTEXA7_ETM_PIR6_PIR6_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR7_OFFSET                                          0x00000FDC
#define CORTEXA7_ETM_PIR7_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR7_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR7_PIR7_SHIFT                                   0
#define    CORTEXA7_ETM_PIR7_PIR7_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR0_OFFSET                                          0x00000FE0
#define CORTEXA7_ETM_PIR0_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR0_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR0_PIR0_SHIFT                                   0
#define    CORTEXA7_ETM_PIR0_PIR0_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR1_OFFSET                                          0x00000FE4
#define CORTEXA7_ETM_PIR1_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR1_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR1_PIR1_SHIFT                                   0
#define    CORTEXA7_ETM_PIR1_PIR1_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR2_OFFSET                                          0x00000FE8
#define CORTEXA7_ETM_PIR2_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR2_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR2_PIR2_SHIFT                                   0
#define    CORTEXA7_ETM_PIR2_PIR2_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_PIR3_OFFSET                                          0x00000FEC
#define CORTEXA7_ETM_PIR3_TYPE                                            UInt32
#define CORTEXA7_ETM_PIR3_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_PIR3_PIR3_SHIFT                                   0
#define    CORTEXA7_ETM_PIR3_PIR3_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_CIR0_OFFSET                                          0x00000FF0
#define CORTEXA7_ETM_CIR0_TYPE                                            UInt32
#define CORTEXA7_ETM_CIR0_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_CIR0_CIR0_SHIFT                                   0
#define    CORTEXA7_ETM_CIR0_CIR0_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_CIR1_OFFSET                                          0x00000FF4
#define CORTEXA7_ETM_CIR1_TYPE                                            UInt32
#define CORTEXA7_ETM_CIR1_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_CIR1_CIR1_SHIFT                                   0
#define    CORTEXA7_ETM_CIR1_CIR1_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_CIR2_OFFSET                                          0x00000FF8
#define CORTEXA7_ETM_CIR2_TYPE                                            UInt32
#define CORTEXA7_ETM_CIR2_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_CIR2_CIR2_SHIFT                                   0
#define    CORTEXA7_ETM_CIR2_CIR2_MASK                                    0xFFFFFFFF

#define CORTEXA7_ETM_CIR3_OFFSET                                          0x00000FFC
#define CORTEXA7_ETM_CIR3_TYPE                                            UInt32
#define CORTEXA7_ETM_CIR3_RESERVED_MASK                                   0x00000000
#define    CORTEXA7_ETM_CIR3_CIR3_SHIFT                                   0
#define    CORTEXA7_ETM_CIR3_CIR3_MASK                                    0xFFFFFFFF

#endif /* __BRCM_RDB_CORTEXA7_ETM_H__ */



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

#ifndef __BRCM_RDB_CDC_H__
#define __BRCM_RDB_CDC_H__

#define CDC_CORE0_COMMAND_OFFSET                                          0x00000000
#define CDC_CORE0_COMMAND_TYPE                                            UInt32
#define CDC_CORE0_COMMAND_RESERVED_MASK                                   0xFFFFFFF0
#define    CDC_CORE0_COMMAND_CORE0_COMMAND_SHIFT                          0
#define    CDC_CORE0_COMMAND_CORE0_COMMAND_MASK                           0x0000000F

#define CDC_CORE1_COMMAND_OFFSET                                          0x00000004
#define CDC_CORE1_COMMAND_TYPE                                            UInt32
#define CDC_CORE1_COMMAND_RESERVED_MASK                                   0xFFFFFFF0
#define    CDC_CORE1_COMMAND_CORE1_COMMAND_SHIFT                          0
#define    CDC_CORE1_COMMAND_CORE1_COMMAND_MASK                           0x0000000F

#define CDC_CORE2_COMMAND_OFFSET                                          0x00000008
#define CDC_CORE2_COMMAND_TYPE                                            UInt32
#define CDC_CORE2_COMMAND_RESERVED_MASK                                   0xFFFFFFF0
#define    CDC_CORE2_COMMAND_CORE2_COMMAND_SHIFT                          0
#define    CDC_CORE2_COMMAND_CORE2_COMMAND_MASK                           0x0000000F

#define CDC_CORE3_COMMAND_OFFSET                                          0x0000000C
#define CDC_CORE3_COMMAND_TYPE                                            UInt32
#define CDC_CORE3_COMMAND_RESERVED_MASK                                   0xFFFFFFF0
#define    CDC_CORE3_COMMAND_CORE3_COMMAND_SHIFT                          0
#define    CDC_CORE3_COMMAND_CORE3_COMMAND_MASK                           0x0000000F

#define CDC_CORE0_COMMAND_STATUS_OFFSET                                   0x00000010
#define CDC_CORE0_COMMAND_STATUS_TYPE                                     UInt32
#define CDC_CORE0_COMMAND_STATUS_RESERVED_MASK                            0x00000000
#define    CDC_CORE0_COMMAND_STATUS_CORE0_COMMAND_STATUS_SHIFT            0
#define    CDC_CORE0_COMMAND_STATUS_CORE0_COMMAND_STATUS_MASK             0xFFFFFFFF

#define CDC_CORE1_COMMAND_STATUS_OFFSET                                   0x00000014
#define CDC_CORE1_COMMAND_STATUS_TYPE                                     UInt32
#define CDC_CORE1_COMMAND_STATUS_RESERVED_MASK                            0x00000000
#define    CDC_CORE1_COMMAND_STATUS_CORE1_COMMAND_STATUS_SHIFT            0
#define    CDC_CORE1_COMMAND_STATUS_CORE1_COMMAND_STATUS_MASK             0xFFFFFFFF

#define CDC_CORE2_COMMAND_STATUS_OFFSET                                   0x00000018
#define CDC_CORE2_COMMAND_STATUS_TYPE                                     UInt32
#define CDC_CORE2_COMMAND_STATUS_RESERVED_MASK                            0x00000000
#define    CDC_CORE2_COMMAND_STATUS_CORE2_COMMAND_STATUS_SHIFT            0
#define    CDC_CORE2_COMMAND_STATUS_CORE2_COMMAND_STATUS_MASK             0xFFFFFFFF

#define CDC_CORE3_COMMAND_STATUS_OFFSET                                   0x0000001C
#define CDC_CORE3_COMMAND_STATUS_TYPE                                     UInt32
#define CDC_CORE3_COMMAND_STATUS_RESERVED_MASK                            0x00000000
#define    CDC_CORE3_COMMAND_STATUS_CORE3_COMMAND_STATUS_SHIFT            0
#define    CDC_CORE3_COMMAND_STATUS_CORE3_COMMAND_STATUS_MASK             0xFFFFFFFF

#define CDC_CONFIG_OFFSET                                                 0x00000020
#define CDC_CONFIG_TYPE                                                   UInt32
#define CDC_CONFIG_RESERVED_MASK                                          0x0000FF00
#define    CDC_CONFIG_DEBUG_BUS_STATUS_SHIFT                              16
#define    CDC_CONFIG_DEBUG_BUS_STATUS_MASK                               0xFFFF0000
#define    CDC_CONFIG_OVERRIDE_POWER_SWITCH_SHIFT                         4
#define    CDC_CONFIG_OVERRIDE_POWER_SWITCH_MASK                          0x000000F0
#define    CDC_CONFIG_DEBUG_BUS_SELECTION_SHIFT                           0
#define    CDC_CONFIG_DEBUG_BUS_SELECTION_MASK                            0x0000000F

#define CDC_USER_REGISTER_OFFSET                                          0x00000024
#define CDC_USER_REGISTER_TYPE                                            UInt32
#define CDC_USER_REGISTER_RESERVED_MASK                                   0x00000000
#define    CDC_USER_REGISTER_USER_REGISTER_SHIFT                          0
#define    CDC_USER_REGISTER_USER_REGISTER_MASK                           0xFFFFFFFF

#define CDC_USER_REGISTER_SET_OFFSET                                      0x00000028
#define CDC_USER_REGISTER_SET_TYPE                                        UInt32
#define CDC_USER_REGISTER_SET_RESERVED_MASK                               0x00000000
#define    CDC_USER_REGISTER_SET_USER_REGISTER_SET_SHIFT                  0
#define    CDC_USER_REGISTER_SET_USER_REGISTER_SET_MASK                   0xFFFFFFFF

#define CDC_USER_REGISTER_CLR_OFFSET                                      0x0000002C
#define CDC_USER_REGISTER_CLR_TYPE                                        UInt32
#define CDC_USER_REGISTER_CLR_RESERVED_MASK                               0x00000000
#define    CDC_USER_REGISTER_CLR_USER_REGISTER_CLR_SHIFT                  0
#define    CDC_USER_REGISTER_CLR_USER_REGISTER_CLR_MASK                   0xFFFFFFFF

#define CDC_ISOLATION_STATE_ENABLE_OFFSET                                 0x00000030
#define CDC_ISOLATION_STATE_ENABLE_TYPE                                   UInt32
#define CDC_ISOLATION_STATE_ENABLE_RESERVED_MASK                          0x00000000
#define    CDC_ISOLATION_STATE_ENABLE_ISOLATION_STATE_ENABLE_SHIFT        0
#define    CDC_ISOLATION_STATE_ENABLE_ISOLATION_STATE_ENABLE_MASK         0xFFFFFFFF

#define CDC_RESET_STATE_ENABLE_OFFSET                                     0x00000034
#define CDC_RESET_STATE_ENABLE_TYPE                                       UInt32
#define CDC_RESET_STATE_ENABLE_RESERVED_MASK                              0x00000000
#define    CDC_RESET_STATE_ENABLE_RESET_STATE_ENABLE_SHIFT                0
#define    CDC_RESET_STATE_ENABLE_RESET_STATE_ENABLE_MASK                 0xFFFFFFFF

#define CDC_INTP_BLOCK_STATE_ENABLE_OFFSET                                0x00000038
#define CDC_INTP_BLOCK_STATE_ENABLE_TYPE                                  UInt32
#define CDC_INTP_BLOCK_STATE_ENABLE_RESERVED_MASK                         0x00000000
#define    CDC_INTP_BLOCK_STATE_ENABLE_INTP_BLOCK_STATE_ENABLE_SHIFT      0
#define    CDC_INTP_BLOCK_STATE_ENABLE_INTP_BLOCK_STATE_ENABLE_MASK       0xFFFFFFFF

#define CDC_CDCBUSY_STATE_ENABLE_OFFSET                                   0x0000003C
#define CDC_CDCBUSY_STATE_ENABLE_TYPE                                     UInt32
#define CDC_CDCBUSY_STATE_ENABLE_RESERVED_MASK                            0x00000000
#define    CDC_CDCBUSY_STATE_ENABLE_CDCBUSY_STATE_ENABLE_SHIFT            0
#define    CDC_CDCBUSY_STATE_ENABLE_CDCBUSY_STATE_ENABLE_MASK             0xFFFFFFFF

#define CDC_RESET_COUNTER_VALUES_OFFSET                                   0x00000040
#define CDC_RESET_COUNTER_VALUES_TYPE                                     UInt32
#define CDC_RESET_COUNTER_VALUES_RESERVED_MASK                            0xFFFFFF00
#define    CDC_RESET_COUNTER_VALUES_FD_RESET_TIMER_SHIFT                  4
#define    CDC_RESET_COUNTER_VALUES_FD_RESET_TIMER_MASK                   0x000000F0
#define    CDC_RESET_COUNTER_VALUES_CD_RESET_TIMER_SHIFT                  0
#define    CDC_RESET_COUNTER_VALUES_CD_RESET_TIMER_MASK                   0x0000000F

#define CDC_SWITCH_COUNTER_VALUES_OFFSET                                  0x00000044
#define CDC_SWITCH_COUNTER_VALUES_TYPE                                    UInt32
#define CDC_SWITCH_COUNTER_VALUES_RESERVED_MASK                           0xFFFF0000
#define    CDC_SWITCH_COUNTER_VALUES_WEAK_SWITCH_TIMER_SHIFT              8
#define    CDC_SWITCH_COUNTER_VALUES_WEAK_SWITCH_TIMER_MASK               0x0000FF00
#define    CDC_SWITCH_COUNTER_VALUES_STRONG_SWITCH_TIMER_SHIFT            0
#define    CDC_SWITCH_COUNTER_VALUES_STRONG_SWITCH_TIMER_MASK             0x000000FF

#define CDC_IS_IDLE_OVERRIDE_OFFSET                                       0x00000048
#define CDC_IS_IDLE_OVERRIDE_TYPE                                         UInt32
#define CDC_IS_IDLE_OVERRIDE_RESERVED_MASK                                0x00000000
#define    CDC_IS_IDLE_OVERRIDE_WAIT_IDLE_TIMEOUT_SHIFT                   27
#define    CDC_IS_IDLE_OVERRIDE_WAIT_IDLE_TIMEOUT_MASK                    0xF8000000
#define    CDC_IS_IDLE_OVERRIDE_WAIT_IDLE_TIMEOUT_STATUS_SHIFT            26
#define    CDC_IS_IDLE_OVERRIDE_WAIT_IDLE_TIMEOUT_STATUS_MASK             0x04000000
#define    CDC_IS_IDLE_OVERRIDE_STBYWFIL2_STATUS_SHIFT                    25
#define    CDC_IS_IDLE_OVERRIDE_STBYWFIL2_STATUS_MASK                     0x02000000
#define    CDC_IS_IDLE_OVERRIDE_STBYWFIL2_OVERRIDE_SHIFT                  24
#define    CDC_IS_IDLE_OVERRIDE_STBYWFIL2_OVERRIDE_MASK                   0x01000000
#define    CDC_IS_IDLE_OVERRIDE_ARMSYSIDLE_TIMER_SHIFT                    16
#define    CDC_IS_IDLE_OVERRIDE_ARMSYSIDLE_TIMER_MASK                     0x00FF0000
#define    CDC_IS_IDLE_OVERRIDE_IS_IDLE_OVERRIDE_SHIFT                    0
#define    CDC_IS_IDLE_OVERRIDE_IS_IDLE_OVERRIDE_MASK                     0x0000FFFF

#define CDC_CDC_POWER_STATUS_OFFSET                                       0x0000004C
#define CDC_CDC_POWER_STATUS_TYPE                                         UInt32
#define CDC_CDC_POWER_STATUS_RESERVED_MASK                                0xFFFFFFF0
#define    CDC_CDC_POWER_STATUS_PWRCTLI_SHIFT                             2
#define    CDC_CDC_POWER_STATUS_PWRCTLI_MASK                              0x0000000C
#define    CDC_CDC_POWER_STATUS_PWRCTLO_SHIFT                             0
#define    CDC_CDC_POWER_STATUS_PWRCTLO_MASK                              0x00000003

#define CDC_A7_DEBUG_BUS_SELECT_OFFSET                                    0x00000050
#define CDC_A7_DEBUG_BUS_SELECT_TYPE                                      UInt32
#define CDC_A7_DEBUG_BUS_SELECT_RESERVED_MASK                             0xFFFE0000
#define    CDC_A7_DEBUG_BUS_SELECT_ENABLE_DORMANT_ARBITRATION_SHIFT       16
#define    CDC_A7_DEBUG_BUS_SELECT_ENABLE_DORMANT_ARBITRATION_MASK        0x00010000
#define    CDC_A7_DEBUG_BUS_SELECT_A7_DEBUG_BUS_SELECT_SHIFT              0
#define    CDC_A7_DEBUG_BUS_SELECT_A7_DEBUG_BUS_SELECT_MASK               0x0000FFFF

#define CDC_FSM_CONTROL_OFFSET                                            0x00000054
#define CDC_FSM_CONTROL_TYPE                                              UInt32
#define CDC_FSM_CONTROL_RESERVED_MASK                                     0xFFFFFFF0
#define    CDC_FSM_CONTROL_CLR_WAIT_IDLE_TIMEOUT_SHIFT                    3
#define    CDC_FSM_CONTROL_CLR_WAIT_IDLE_TIMEOUT_MASK                     0x00000008
#define    CDC_FSM_CONTROL_CLR_FIRST_TO_POLL_SHIFT                        2
#define    CDC_FSM_CONTROL_CLR_FIRST_TO_POLL_MASK                         0x00000004
#define    CDC_FSM_CONTROL_CLR_L2_IS_ON_SHIFT                             1
#define    CDC_FSM_CONTROL_CLR_L2_IS_ON_MASK                              0x00000002
#define    CDC_FSM_CONTROL_CLR_TIMEOUT_INT_SHIFT                          0
#define    CDC_FSM_CONTROL_CLR_TIMEOUT_INT_MASK                           0x00000001

#define CDC_ENABLE_MASTER_CLK_GATING_OFFSET                               0x00000058
#define CDC_ENABLE_MASTER_CLK_GATING_TYPE                                 UInt32
#define CDC_ENABLE_MASTER_CLK_GATING_RESERVED_MASK                        0xFFFFFFFE
#define    CDC_ENABLE_MASTER_CLK_GATING_ENABLE_MASTER_CLK_GATING_SHIFT    0
#define    CDC_ENABLE_MASTER_CLK_GATING_ENABLE_MASTER_CLK_GATING_MASK     0x00000001

#define CDC_CORE_TIMER_IN_USE_OFFSET                                      0x0000005C
#define CDC_CORE_TIMER_IN_USE_TYPE                                        UInt32
#define CDC_CORE_TIMER_IN_USE_RESERVED_MASK                               0xFFFFFFFE
#define    CDC_CORE_TIMER_IN_USE_CORE_TIMER_IN_USE_SHIFT                  0
#define    CDC_CORE_TIMER_IN_USE_CORE_TIMER_IN_USE_MASK                   0x00000001

#define CDC_ACP_IS_ACTIVE_OFFSET                                          0x00000060
#define CDC_ACP_IS_ACTIVE_TYPE                                            UInt32
#define CDC_ACP_IS_ACTIVE_RESERVED_MASK                                   0xFFFFFFFE
#define    CDC_ACP_IS_ACTIVE_ACP_IS_ACTIVE_SHIFT                          0
#define    CDC_ACP_IS_ACTIVE_ACP_IS_ACTIVE_MASK                           0x00000001

#define CDC_CORE0_DEBUG_RESET_OFFSET                                      0x00000064
#define CDC_CORE0_DEBUG_RESET_TYPE                                        UInt32
#define CDC_CORE0_DEBUG_RESET_RESERVED_MASK                               0xFFFFFFFC
#define    CDC_CORE0_DEBUG_RESET_DISABLE_DEBUG0_RESET_SHIFT               1
#define    CDC_CORE0_DEBUG_RESET_DISABLE_DEBUG0_RESET_MASK                0x00000002
#define    CDC_CORE0_DEBUG_RESET_FORCE_DEBUG0_RESET_SHIFT                 0
#define    CDC_CORE0_DEBUG_RESET_FORCE_DEBUG0_RESET_MASK                  0x00000001

#define CDC_CORE1_DEBUG_RESET_OFFSET                                      0x00000068
#define CDC_CORE1_DEBUG_RESET_TYPE                                        UInt32
#define CDC_CORE1_DEBUG_RESET_RESERVED_MASK                               0xFFFFFFFC
#define    CDC_CORE1_DEBUG_RESET_DISABLE_DEBUG1_RESET_SHIFT               1
#define    CDC_CORE1_DEBUG_RESET_DISABLE_DEBUG1_RESET_MASK                0x00000002
#define    CDC_CORE1_DEBUG_RESET_FORCE_DEBUG1_RESET_SHIFT                 0
#define    CDC_CORE1_DEBUG_RESET_FORCE_DEBUG1_RESET_MASK                  0x00000001

#define CDC_CORE2_DEBUG_RESET_OFFSET                                      0x0000006C
#define CDC_CORE2_DEBUG_RESET_TYPE                                        UInt32
#define CDC_CORE2_DEBUG_RESET_RESERVED_MASK                               0xFFFFFFFC
#define    CDC_CORE2_DEBUG_RESET_DISABLE_DEBUG2_RESET_SHIFT               1
#define    CDC_CORE2_DEBUG_RESET_DISABLE_DEBUG2_RESET_MASK                0x00000002
#define    CDC_CORE2_DEBUG_RESET_FORCE_DEBUG2_RESET_SHIFT                 0
#define    CDC_CORE2_DEBUG_RESET_FORCE_DEBUG2_RESET_MASK                  0x00000001

#define CDC_CORE3_DEBUG_RESET_OFFSET                                      0x00000070
#define CDC_CORE3_DEBUG_RESET_TYPE                                        UInt32
#define CDC_CORE3_DEBUG_RESET_RESERVED_MASK                               0xFFFFFFFC
#define    CDC_CORE3_DEBUG_RESET_DISABLE_DEBUG3_RESET_SHIFT               1
#define    CDC_CORE3_DEBUG_RESET_DISABLE_DEBUG3_RESET_MASK                0x00000002
#define    CDC_CORE3_DEBUG_RESET_FORCE_DEBUG3_RESET_SHIFT                 0
#define    CDC_CORE3_DEBUG_RESET_FORCE_DEBUG3_RESET_MASK                  0x00000001

#define CDC_RESET_CONTROL_OFFSET                                          0x00000074
#define CDC_RESET_CONTROL_TYPE                                            UInt32
#define CDC_RESET_CONTROL_RESERVED_MASK                                   0xFFFFFFFC
#define    CDC_RESET_CONTROL_FORCE_ATB_RESET_SHIFT                        1
#define    CDC_RESET_CONTROL_FORCE_ATB_RESET_MASK                         0x00000002
#define    CDC_RESET_CONTROL_FORCE_SOC_DBG_RESET_SHIFT                    0
#define    CDC_RESET_CONTROL_FORCE_SOC_DBG_RESET_MASK                     0x00000001

#endif /* __BRCM_RDB_CDC_H__ */



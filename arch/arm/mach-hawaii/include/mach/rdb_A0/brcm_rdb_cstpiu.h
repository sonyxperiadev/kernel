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

#ifndef __BRCM_RDB_CSTPIU_H__
#define __BRCM_RDB_CSTPIU_H__

#define CSTPIU_SUPPORTED_PORT_SIZES_OFFSET                                0x00000000
#define CSTPIU_SUPPORTED_PORT_SIZES_TYPE                                  UInt32
#define CSTPIU_SUPPORTED_PORT_SIZES_RESERVED_MASK                         0x00000000
#define    CSTPIU_SUPPORTED_PORT_SIZES_PORT_SIZE_SUPPORT_SHIFT            0
#define    CSTPIU_SUPPORTED_PORT_SIZES_PORT_SIZE_SUPPORT_MASK             0xFFFFFFFF

#define CSTPIU_CURRENT_PORT_SIZE_OFFSET                                   0x00000004
#define CSTPIU_CURRENT_PORT_SIZE_TYPE                                     UInt32
#define CSTPIU_CURRENT_PORT_SIZE_RESERVED_MASK                            0x00000000
#define    CSTPIU_CURRENT_PORT_SIZE_PORT_SIZE_SHIFT                       0
#define    CSTPIU_CURRENT_PORT_SIZE_PORT_SIZE_MASK                        0xFFFFFFFF

#define CSTPIU_SUPPORTED_TRIGGER_MODES_OFFSET                             0x00000100
#define CSTPIU_SUPPORTED_TRIGGER_MODES_TYPE                               UInt32
#define CSTPIU_SUPPORTED_TRIGGER_MODES_RESERVED_MASK                      0xFFFCFEE0
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_TRGRUN_SHIFT                    17
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_TRGRUN_MASK                     0x00020000
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_TRIGGERED_SHIFT                 16
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_TRIGGERED_MASK                  0x00010000
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_TCOUNT8_SHIFT                   8
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_TCOUNT8_MASK                    0x00000100
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT64K_SHIFT                   4
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT64K_MASK                    0x00000010
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT256_SHIFT                   3
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT256_MASK                    0x00000008
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT16_SHIFT                    2
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT16_MASK                     0x00000004
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT4_SHIFT                     1
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT4_MASK                      0x00000002
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT2_SHIFT                     0
#define    CSTPIU_SUPPORTED_TRIGGER_MODES_MULT2_MASK                      0x00000001

#define CSTPIU_TRIGGER_COUNTER_VALUE_OFFSET                               0x00000104
#define CSTPIU_TRIGGER_COUNTER_VALUE_TYPE                                 UInt32
#define CSTPIU_TRIGGER_COUNTER_VALUE_RESERVED_MASK                        0xFFFFFF00
#define    CSTPIU_TRIGGER_COUNTER_VALUE_TRIGCOUNT_SHIFT                   0
#define    CSTPIU_TRIGGER_COUNTER_VALUE_TRIGCOUNT_MASK                    0x000000FF

#define CSTPIU_TRIGGER_MULTIPLIER_OFFSET                                  0x00000108
#define CSTPIU_TRIGGER_MULTIPLIER_TYPE                                    UInt32
#define CSTPIU_TRIGGER_MULTIPLIER_RESERVED_MASK                           0xFFFFFFE0
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT64K_SHIFT                        4
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT64K_MASK                         0x00000010
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT256_SHIFT                        3
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT256_MASK                         0x00000008
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT16_SHIFT                         2
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT16_MASK                          0x00000004
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT4_SHIFT                          1
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT4_MASK                           0x00000002
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT2_SHIFT                          0
#define    CSTPIU_TRIGGER_MULTIPLIER_MULT2_MASK                           0x00000001

#define CSTPIU_SUPPORTED_TEST_PATTERN_MODES_OFFSET                        0x00000200
#define CSTPIU_SUPPORTED_TEST_PATTERN_MODES_TYPE                          UInt32
#define CSTPIU_SUPPORTED_TEST_PATTERN_MODES_RESERVED_MASK                 0xFFFCFFF0
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PCONTEN_SHIFT              17
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PCONTEN_MASK               0x00020000
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PTIMEEN_SHIFT              16
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PTIMEEN_MASK               0x00010000
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATF0_SHIFT                3
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATF0_MASK                 0x00000008
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATA5_SHIFT                2
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATA5_MASK                 0x00000004
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATW1_SHIFT                1
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATW1_MASK                 0x00000002
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATW0_SHIFT                0
#define    CSTPIU_SUPPORTED_TEST_PATTERN_MODES_PATW0_MASK                 0x00000001

#define CSTPIU_CURRENT_TEST_PATTERN_MODE_OFFSET                           0x00000204
#define CSTPIU_CURRENT_TEST_PATTERN_MODE_TYPE                             UInt32
#define CSTPIU_CURRENT_TEST_PATTERN_MODE_RESERVED_MASK                    0xFFFCFFF0
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PCONTEN_SHIFT                 17
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PCONTEN_MASK                  0x00020000
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PTIMEEN_SHIFT                 16
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PTIMEEN_MASK                  0x00010000
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATF0_SHIFT                   3
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATF0_MASK                    0x00000008
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATA5_SHIFT                   2
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATA5_MASK                    0x00000004
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATW1_SHIFT                   1
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATW1_MASK                    0x00000002
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATW0_SHIFT                   0
#define    CSTPIU_CURRENT_TEST_PATTERN_MODE_PATW0_MASK                    0x00000001

#define CSTPIU_TEST_PATTERN_REPEAT_COUNTER_OFFSET                         0x00000208
#define CSTPIU_TEST_PATTERN_REPEAT_COUNTER_TYPE                           UInt32
#define CSTPIU_TEST_PATTERN_REPEAT_COUNTER_RESERVED_MASK                  0xFFFFFF00
#define    CSTPIU_TEST_PATTERN_REPEAT_COUNTER_PATTCOUNT_SHIFT             0
#define    CSTPIU_TEST_PATTERN_REPEAT_COUNTER_PATTCOUNT_MASK              0x000000FF

#define CSTPIU_FORMATTER_AND_FLUSH_STATUS_OFFSET                          0x00000300
#define CSTPIU_FORMATTER_AND_FLUSH_STATUS_TYPE                            UInt32
#define CSTPIU_FORMATTER_AND_FLUSH_STATUS_RESERVED_MASK                   0xFFFFFFF8
#define    CSTPIU_FORMATTER_AND_FLUSH_STATUS_TCPRESENT_SHIFT              2
#define    CSTPIU_FORMATTER_AND_FLUSH_STATUS_TCPRESENT_MASK               0x00000004
#define    CSTPIU_FORMATTER_AND_FLUSH_STATUS_FTSTOPPED_SHIFT              1
#define    CSTPIU_FORMATTER_AND_FLUSH_STATUS_FTSTOPPED_MASK               0x00000002
#define    CSTPIU_FORMATTER_AND_FLUSH_STATUS_FLINPROG_SHIFT               0
#define    CSTPIU_FORMATTER_AND_FLUSH_STATUS_FLINPROG_MASK                0x00000001

#define CSTPIU_FORMATTER_AND_FLUSH_CONTROL_OFFSET                         0x00000304
#define CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TYPE                           UInt32
#define CSTPIU_FORMATTER_AND_FLUSH_CONTROL_RESERVED_MASK                  0xFFFFC88C
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_STOPTRIG_SHIFT              13
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_STOPTRIG_MASK               0x00002000
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_STOPFL_SHIFT                12
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_STOPFL_MASK                 0x00001000
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TRIGFL_SHIFT                10
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TRIGFL_MASK                 0x00000400
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TRIGEVT_SHIFT               9
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TRIGEVT_MASK                0x00000200
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TRIGIN_SHIFT                8
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_TRIGIN_MASK                 0x00000100
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_FONMAN_SHIFT                6
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_FONMAN_MASK                 0x00000040
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_FONTRIG_SHIFT               5
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_FONTRIG_MASK                0x00000020
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_FONFLIN_SHIFT               4
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_FONFLIN_MASK                0x00000010
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_ENFCONT_SHIFT               1
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_ENFCONT_MASK                0x00000002
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_ENFTC_SHIFT                 0
#define    CSTPIU_FORMATTER_AND_FLUSH_CONTROL_ENFTC_MASK                  0x00000001

#define CSTPIU_FORMATTER_SYNCHRONIZATION_COUNTER_OFFSET                   0x00000308
#define CSTPIU_FORMATTER_SYNCHRONIZATION_COUNTER_TYPE                     UInt32
#define CSTPIU_FORMATTER_SYNCHRONIZATION_COUNTER_RESERVED_MASK            0xFFFFF000
#define    CSTPIU_FORMATTER_SYNCHRONIZATION_COUNTER_CYCCOUNT_SHIFT        0
#define    CSTPIU_FORMATTER_SYNCHRONIZATION_COUNTER_CYCCOUNT_MASK         0x00000FFF

#define CSTPIU_EXTCTL_IN_PORT_OFFSET                                      0x00000400
#define CSTPIU_EXTCTL_IN_PORT_TYPE                                        UInt32
#define CSTPIU_EXTCTL_IN_PORT_RESERVED_MASK                               0xFFFFFF00
#define    CSTPIU_EXTCTL_IN_PORT_INPUT_SAMPLE_SHIFT                       0
#define    CSTPIU_EXTCTL_IN_PORT_INPUT_SAMPLE_MASK                        0x000000FF

#define CSTPIU_EXTCTL_OUT_PORT_OFFSET                                     0x00000404
#define CSTPIU_EXTCTL_OUT_PORT_TYPE                                       UInt32
#define CSTPIU_EXTCTL_OUT_PORT_RESERVED_MASK                              0xFFFFFF00
#define    CSTPIU_EXTCTL_OUT_PORT_EXPORT_FEEDBACK_SHIFT                   0
#define    CSTPIU_EXTCTL_OUT_PORT_EXPORT_FEEDBACK_MASK                    0x000000FF

#define CSTPIU_ITTRFLINACK_OFFSET                                         0x00000EE4
#define CSTPIU_ITTRFLINACK_TYPE                                           UInt32
#define CSTPIU_ITTRFLINACK_RESERVED_MASK                                  0xFFFFFFFC
#define    CSTPIU_ITTRFLINACK_FLUSHINACK_SHIFT                            1
#define    CSTPIU_ITTRFLINACK_FLUSHINACK_MASK                             0x00000002
#define    CSTPIU_ITTRFLINACK_TRIGINACK_SHIFT                             0
#define    CSTPIU_ITTRFLINACK_TRIGINACK_MASK                              0x00000001

#define CSTPIU_ITTRFLIN_OFFSET                                            0x00000EE8
#define CSTPIU_ITTRFLIN_TYPE                                              UInt32
#define CSTPIU_ITTRFLIN_RESERVED_MASK                                     0xFFFFFFFC
#define    CSTPIU_ITTRFLIN_FLUSHIN_SHIFT                                  1
#define    CSTPIU_ITTRFLIN_FLUSHIN_MASK                                   0x00000002
#define    CSTPIU_ITTRFLIN_TRIGIN_SHIFT                                   0
#define    CSTPIU_ITTRFLIN_TRIGIN_MASK                                    0x00000001

#define CSTPIU_ITATBDATA0_OFFSET                                          0x00000EEC
#define CSTPIU_ITATBDATA0_TYPE                                            UInt32
#define CSTPIU_ITATBDATA0_RESERVED_MASK                                   0xFFFFFFE0
#define    CSTPIU_ITATBDATA0_ATDATA_31_SHIFT                              4
#define    CSTPIU_ITATBDATA0_ATDATA_31_MASK                               0x00000010
#define    CSTPIU_ITATBDATA0_ATDATA_23_SHIFT                              3
#define    CSTPIU_ITATBDATA0_ATDATA_23_MASK                               0x00000008
#define    CSTPIU_ITATBDATA0_ATDATA_15_SHIFT                              2
#define    CSTPIU_ITATBDATA0_ATDATA_15_MASK                               0x00000004
#define    CSTPIU_ITATBDATA0_ATDATA_7_SHIFT                               1
#define    CSTPIU_ITATBDATA0_ATDATA_7_MASK                                0x00000002
#define    CSTPIU_ITATBDATA0_ATDATA_0_SHIFT                               0
#define    CSTPIU_ITATBDATA0_ATDATA_0_MASK                                0x00000001

#define CSTPIU_ITATBCTR2_OFFSET                                           0x00000EF0
#define CSTPIU_ITATBCTR2_TYPE                                             UInt32
#define CSTPIU_ITATBCTR2_RESERVED_MASK                                    0xFFFFFFFC
#define    CSTPIU_ITATBCTR2_AFVALID_SHIFT                                 1
#define    CSTPIU_ITATBCTR2_AFVALID_MASK                                  0x00000002
#define    CSTPIU_ITATBCTR2_ATREADY_SHIFT                                 0
#define    CSTPIU_ITATBCTR2_ATREADY_MASK                                  0x00000001

#define CSTPIU_ITATBCTR1_OFFSET                                           0x00000EF4
#define CSTPIU_ITATBCTR1_TYPE                                             UInt32
#define CSTPIU_ITATBCTR1_RESERVED_MASK                                    0xFFFFFF80
#define    CSTPIU_ITATBCTR1_ATID_SHIFT                                    0
#define    CSTPIU_ITATBCTR1_ATID_MASK                                     0x0000007F

#define CSTPIU_ITATBCTR0_OFFSET                                           0x00000EF8
#define CSTPIU_ITATBCTR0_TYPE                                             UInt32
#define CSTPIU_ITATBCTR0_RESERVED_MASK                                    0xFFFFFCFC
#define    CSTPIU_ITATBCTR0_ATBYTES_SHIFT                                 8
#define    CSTPIU_ITATBCTR0_ATBYTES_MASK                                  0x00000300
#define    CSTPIU_ITATBCTR0_AFREADYS_SHIFT                                1
#define    CSTPIU_ITATBCTR0_AFREADYS_MASK                                 0x00000002
#define    CSTPIU_ITATBCTR0_ATVALID_SHIFT                                 0
#define    CSTPIU_ITATBCTR0_ATVALID_MASK                                  0x00000001

#define CSTPIU_ITCTRL_OFFSET                                              0x00000F00
#define CSTPIU_ITCTRL_TYPE                                                UInt32
#define CSTPIU_ITCTRL_RESERVED_MASK                                       0xFFFFFFFE
#define    CSTPIU_ITCTRL_MODE_SHIFT                                       0
#define    CSTPIU_ITCTRL_MODE_MASK                                        0x00000001

#define CSTPIU_CLAIM_TAG_SET_OFFSET                                       0x00000FA0
#define CSTPIU_CLAIM_TAG_SET_TYPE                                         UInt32
#define CSTPIU_CLAIM_TAG_SET_RESERVED_MASK                                0xFFFFFFF0
#define    CSTPIU_CLAIM_TAG_SET_CTV_SHIFT                                 0
#define    CSTPIU_CLAIM_TAG_SET_CTV_MASK                                  0x0000000F

#define CSTPIU_CLAIM_TAG_CLEAR_OFFSET                                     0x00000FA4
#define CSTPIU_CLAIM_TAG_CLEAR_TYPE                                       UInt32
#define CSTPIU_CLAIM_TAG_CLEAR_RESERVED_MASK                              0xFFFFFFF0
#define    CSTPIU_CLAIM_TAG_CLEAR_CTV_SHIFT                               0
#define    CSTPIU_CLAIM_TAG_CLEAR_CTV_MASK                                0x0000000F

#define CSTPIU_LOCKACCESS_OFFSET                                          0x00000FB0
#define CSTPIU_LOCKACCESS_TYPE                                            UInt32
#define CSTPIU_LOCKACCESS_RESERVED_MASK                                   0x00000000
#define    CSTPIU_LOCKACCESS_WRITE_ACCESS_CODE_SHIFT                      0
#define    CSTPIU_LOCKACCESS_WRITE_ACCESS_CODE_MASK                       0xFFFFFFFF

#define CSTPIU_LOCKSTATUS_OFFSET                                          0x00000FB4
#define CSTPIU_LOCKSTATUS_TYPE                                            UInt32
#define CSTPIU_LOCKSTATUS_RESERVED_MASK                                   0xFFFFFFF8
#define    CSTPIU_LOCKSTATUS_LOCK_SIZE_SHIFT                              2
#define    CSTPIU_LOCKSTATUS_LOCK_SIZE_MASK                               0x00000004
#define    CSTPIU_LOCKSTATUS_LOCK_STATUS_SHIFT                            1
#define    CSTPIU_LOCKSTATUS_LOCK_STATUS_MASK                             0x00000002
#define    CSTPIU_LOCKSTATUS_LOCK_MECHANISM_SHIFT                         0
#define    CSTPIU_LOCKSTATUS_LOCK_MECHANISM_MASK                          0x00000001

#define CSTPIU_AUTHSTATUS_OFFSET                                          0x00000FB8
#define CSTPIU_AUTHSTATUS_TYPE                                            UInt32
#define CSTPIU_AUTHSTATUS_RESERVED_MASK                                   0xFFFFFF00
#define    CSTPIU_AUTHSTATUS_SECURE_NONINVASIVE_DEBUG_SHIFT               6
#define    CSTPIU_AUTHSTATUS_SECURE_NONINVASIVE_DEBUG_MASK                0x000000C0
#define    CSTPIU_AUTHSTATUS_SECURE_INVASIVE_DEBUG_SHIFT                  4
#define    CSTPIU_AUTHSTATUS_SECURE_INVASIVE_DEBUG_MASK                   0x00000030
#define    CSTPIU_AUTHSTATUS_NONSECURE_NONINVASIVE_DEBUG_SHIFT            2
#define    CSTPIU_AUTHSTATUS_NONSECURE_NONINVASIVE_DEBUG_MASK             0x0000000C
#define    CSTPIU_AUTHSTATUS_NONSECURE_INVASIVE_DEBUG_SHIFT               0
#define    CSTPIU_AUTHSTATUS_NONSECURE_INVASIVE_DEBUG_MASK                0x00000003

#define CSTPIU_DEVICE_ID_OFFSET                                           0x00000FC8
#define CSTPIU_DEVICE_ID_TYPE                                             UInt32
#define CSTPIU_DEVICE_ID_RESERVED_MASK                                    0xFFFFF000
#define    CSTPIU_DEVICE_ID_SERIAL_WIRE_OUTPUT_UART_NRZ_SHIFT             11
#define    CSTPIU_DEVICE_ID_SERIAL_WIRE_OUTPUT_UART_NRZ_MASK              0x00000800
#define    CSTPIU_DEVICE_ID_SERIAL_WIRE_OUTPUT_MANCHESTER_SHIFT           10
#define    CSTPIU_DEVICE_ID_SERIAL_WIRE_OUTPUT_MANCHESTER_MASK            0x00000400
#define    CSTPIU_DEVICE_ID_TRACE_CLOCK_DATA_SHIFT                        9
#define    CSTPIU_DEVICE_ID_TRACE_CLOCK_DATA_MASK                         0x00000200
#define    CSTPIU_DEVICE_ID_FIFO_SIZE_SHIFT                               6
#define    CSTPIU_DEVICE_ID_FIFO_SIZE_MASK                                0x000001C0
#define    CSTPIU_DEVICE_ID_ATCLK_TRACECLKIN_SHIFT                        5
#define    CSTPIU_DEVICE_ID_ATCLK_TRACECLKIN_MASK                         0x00000020
#define    CSTPIU_DEVICE_ID_HIDDEN_LEVEL_OF_INPUT_MULTIPLEXING_SHIFT      0
#define    CSTPIU_DEVICE_ID_HIDDEN_LEVEL_OF_INPUT_MULTIPLEXING_MASK       0x0000001F

#define CSTPIU_DEVICE_TYPE_IDENTIFIER_OFFSET                              0x00000FCC
#define CSTPIU_DEVICE_TYPE_IDENTIFIER_TYPE                                UInt32
#define CSTPIU_DEVICE_TYPE_IDENTIFIER_RESERVED_MASK                       0xFFFFFF00
#define    CSTPIU_DEVICE_TYPE_IDENTIFIER_SUB_TYPE_SHIFT                   4
#define    CSTPIU_DEVICE_TYPE_IDENTIFIER_SUB_TYPE_MASK                    0x000000F0
#define    CSTPIU_DEVICE_TYPE_IDENTIFIER_MAIN_TYPE_SHIFT                  0
#define    CSTPIU_DEVICE_TYPE_IDENTIFIER_MAIN_TYPE_MASK                   0x0000000F

#define CSTPIU_PERIPHERAL_ID4_OFFSET                                      0x00000FD0
#define CSTPIU_PERIPHERAL_ID4_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID4_RESERVED_MASK                               0xFFFFFF00
#define    CSTPIU_PERIPHERAL_ID4_COUNT_4KB_SHIFT                          4
#define    CSTPIU_PERIPHERAL_ID4_COUNT_4KB_MASK                           0x000000F0
#define    CSTPIU_PERIPHERAL_ID4_JEP_CONTINUATION_CODE_SHIFT              0
#define    CSTPIU_PERIPHERAL_ID4_JEP_CONTINUATION_CODE_MASK               0x0000000F

#define CSTPIU_PERIPHERAL_ID5_OFFSET                                      0x00000FD4
#define CSTPIU_PERIPHERAL_ID5_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID5_RESERVED_MASK                               0xFFFFFFFF

#define CSTPIU_PERIPHERAL_ID6_OFFSET                                      0x00000FD8
#define CSTPIU_PERIPHERAL_ID6_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID6_RESERVED_MASK                               0xFFFFFFFF

#define CSTPIU_PERIPHERAL_ID7_OFFSET                                      0x00000FDC
#define CSTPIU_PERIPHERAL_ID7_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID7_RESERVED_MASK                               0xFFFFFFFF

#define CSTPIU_PERIPHERAL_ID0_OFFSET                                      0x00000FE0
#define CSTPIU_PERIPHERAL_ID0_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID0_RESERVED_MASK                               0xFFFFFF00
#define    CSTPIU_PERIPHERAL_ID0_PART_NUMBER_7_0_SHIFT                    0
#define    CSTPIU_PERIPHERAL_ID0_PART_NUMBER_7_0_MASK                     0x000000FF

#define CSTPIU_PERIPHERAL_ID1_OFFSET                                      0x00000FE4
#define CSTPIU_PERIPHERAL_ID1_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID1_RESERVED_MASK                               0xFFFFFF00
#define    CSTPIU_PERIPHERAL_ID1_JEP_IDENTITY_CODE_3_0_SHIFT              4
#define    CSTPIU_PERIPHERAL_ID1_JEP_IDENTITY_CODE_3_0_MASK               0x000000F0
#define    CSTPIU_PERIPHERAL_ID1_PART_NUMBER_1_SHIFT                      0
#define    CSTPIU_PERIPHERAL_ID1_PART_NUMBER_1_MASK                       0x0000000F

#define CSTPIU_PERIPHERAL_ID2_OFFSET                                      0x00000FE8
#define CSTPIU_PERIPHERAL_ID2_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID2_RESERVED_MASK                               0xFFFFFF00
#define    CSTPIU_PERIPHERAL_ID2_REVISION_SHIFT                           4
#define    CSTPIU_PERIPHERAL_ID2_REVISION_MASK                            0x000000F0
#define    CSTPIU_PERIPHERAL_ID2_JEP_1_SHIFT                              3
#define    CSTPIU_PERIPHERAL_ID2_JEP_1_MASK                               0x00000008
#define    CSTPIU_PERIPHERAL_ID2_JEP_IDENTITY_CODE_6_4_SHIFT              0
#define    CSTPIU_PERIPHERAL_ID2_JEP_IDENTITY_CODE_6_4_MASK               0x00000007

#define CSTPIU_PERIPHERAL_ID3_OFFSET                                      0x00000FEC
#define CSTPIU_PERIPHERAL_ID3_TYPE                                        UInt32
#define CSTPIU_PERIPHERAL_ID3_RESERVED_MASK                               0xFFFFFF00
#define    CSTPIU_PERIPHERAL_ID3_REVAND_SHIFT                             4
#define    CSTPIU_PERIPHERAL_ID3_REVAND_MASK                              0x000000F0
#define    CSTPIU_PERIPHERAL_ID3_CUSTOMER_MODIFIED_SHIFT                  0
#define    CSTPIU_PERIPHERAL_ID3_CUSTOMER_MODIFIED_MASK                   0x0000000F

#define CSTPIU_COMPONENT_ID0_OFFSET                                       0x00000FF0
#define CSTPIU_COMPONENT_ID0_TYPE                                         UInt32
#define CSTPIU_COMPONENT_ID0_RESERVED_MASK                                0xFFFFFF00
#define    CSTPIU_COMPONENT_ID0_ID0_SHIFT                                 0
#define    CSTPIU_COMPONENT_ID0_ID0_MASK                                  0x000000FF

#define CSTPIU_COMPONENT_ID1_OFFSET                                       0x00000FF4
#define CSTPIU_COMPONENT_ID1_TYPE                                         UInt32
#define CSTPIU_COMPONENT_ID1_RESERVED_MASK                                0xFFFFFF00
#define    CSTPIU_COMPONENT_ID1_COMPONENT_CLASS_SHIFT                     4
#define    CSTPIU_COMPONENT_ID1_COMPONENT_CLASS_MASK                      0x000000F0
#define    CSTPIU_COMPONENT_ID1_ID1_SHIFT                                 0
#define    CSTPIU_COMPONENT_ID1_ID1_MASK                                  0x0000000F

#define CSTPIU_COMPONENT_ID2_OFFSET                                       0x00000FF8
#define CSTPIU_COMPONENT_ID2_TYPE                                         UInt32
#define CSTPIU_COMPONENT_ID2_RESERVED_MASK                                0xFFFFFF00
#define    CSTPIU_COMPONENT_ID2_ID2_SHIFT                                 0
#define    CSTPIU_COMPONENT_ID2_ID2_MASK                                  0x000000FF

#define CSTPIU_COMPONENT_ID3_OFFSET                                       0x00000FFC
#define CSTPIU_COMPONENT_ID3_TYPE                                         UInt32
#define CSTPIU_COMPONENT_ID3_RESERVED_MASK                                0xFFFFFF00
#define    CSTPIU_COMPONENT_ID3_ID3_SHIFT                                 0
#define    CSTPIU_COMPONENT_ID3_ID3_MASK                                  0x000000FF

#endif /* __BRCM_RDB_CSTPIU_H__ */



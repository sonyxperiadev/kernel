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

#ifndef __BRCM_RDB_IRDA_H__
#define __BRCM_RDB_IRDA_H__

#define IRDA_CONTROL_OFFSET                                               0x00000000
#define IRDA_CONTROL_TYPE                                                 UInt32
#define IRDA_CONTROL_RESERVED_MASK                                        0x7FFFFFCC
#define    IRDA_CONTROL_IRDA_SOFT_RESET_SHIFT                             31
#define    IRDA_CONTROL_IRDA_SOFT_RESET_MASK                              0x80000000
#define    IRDA_CONTROL_IRDA_RX_MONOTORING_SHIFT                          5
#define    IRDA_CONTROL_IRDA_RX_MONOTORING_MASK                           0x00000020
#define    IRDA_CONTROL_IRDA_SIP_MODE_SHIFT                               4
#define    IRDA_CONTROL_IRDA_SIP_MODE_MASK                                0x00000010
#define    IRDA_CONTROL_IRDA_INVERT_INOUT_SHIFT                           1
#define    IRDA_CONTROL_IRDA_INVERT_INOUT_MASK                            0x00000002
#define    IRDA_CONTROL_IRDA_ENABLE_SHIFT                                 0
#define    IRDA_CONTROL_IRDA_ENABLE_MASK                                  0x00000001

#define IRDA_OPERATION_CONTROL_OFFSET                                     0x00000004
#define IRDA_OPERATION_CONTROL_TYPE                                       UInt32
#define IRDA_OPERATION_CONTROL_RESERVED_MASK                              0x0800FFFC
#define    IRDA_OPERATION_CONTROL_IRDA_TRANSMIT_NUMBERBLOCK_SHIFT         28
#define    IRDA_OPERATION_CONTROL_IRDA_TRANSMIT_NUMBERBLOCK_MASK          0xF0000000
#define    IRDA_OPERATION_CONTROL_IRDA_TRANSMIT_BLOCKSIZE_SHIFT           16
#define    IRDA_OPERATION_CONTROL_IRDA_TRANSMIT_BLOCKSIZE_MASK            0x07FF0000
#define    IRDA_OPERATION_CONTROL_IRDA_RECEIVE_ON_SHIFT                   1
#define    IRDA_OPERATION_CONTROL_IRDA_RECEIVE_ON_MASK                    0x00000002
#define    IRDA_OPERATION_CONTROL_IRDA_TRANSMIT_ON_SHIFT                  0
#define    IRDA_OPERATION_CONTROL_IRDA_TRANSMIT_ON_MASK                   0x00000001

#define IRDA_INTERRUPT_STATUS_OFFSET                                      0x00000008
#define IRDA_INTERRUPT_STATUS_TYPE                                        UInt32
#define IRDA_INTERRUPT_STATUS_RESERVED_MASK                               0x07F00000
#define    IRDA_INTERRUPT_STATUS_TRANSMIT_NUMBERBLOCKLEFT_SHIFT           28
#define    IRDA_INTERRUPT_STATUS_TRANSMIT_NUMBERBLOCKLEFT_MASK            0xF0000000
#define    IRDA_INTERRUPT_STATUS_RX_ACTIVITYLINE_STATUS_SHIFT             27
#define    IRDA_INTERRUPT_STATUS_RX_ACTIVITYLINE_STATUS_MASK              0x08000000
#define    IRDA_INTERRUPT_STATUS_PIO_TX_START_STATUS_SHIFT                19
#define    IRDA_INTERRUPT_STATUS_PIO_TX_START_STATUS_MASK                 0x00080000
#define    IRDA_INTERRUPT_STATUS_PIO_TX_STOP_STATUS_SHIFT                 18
#define    IRDA_INTERRUPT_STATUS_PIO_TX_STOP_STATUS_MASK                  0x00040000
#define    IRDA_INTERRUPT_STATUS_PIO_RX_START_STATUS_SHIFT                17
#define    IRDA_INTERRUPT_STATUS_PIO_RX_START_STATUS_MASK                 0x00020000
#define    IRDA_INTERRUPT_STATUS_PIO_RX_STOP_STATUS_SHIFT                 16
#define    IRDA_INTERRUPT_STATUS_PIO_RX_STOP_STATUS_MASK                  0x00010000
#define    IRDA_INTERRUPT_STATUS_APB_TX_ERROR_STATUS_SHIFT                15
#define    IRDA_INTERRUPT_STATUS_APB_TX_ERROR_STATUS_MASK                 0x00008000
#define    IRDA_INTERRUPT_STATUS_APB_RX_ERROR_STATUS_SHIFT                14
#define    IRDA_INTERRUPT_STATUS_APB_RX_ERROR_STATUS_MASK                 0x00004000
#define    IRDA_INTERRUPT_STATUS_BLOCKUPDATED_STATUS_SHIFT                13
#define    IRDA_INTERRUPT_STATUS_BLOCKUPDATED_STATUS_MASK                 0x00002000
#define    IRDA_INTERRUPT_STATUS_TXDONE_STATUS_SHIFT                      12
#define    IRDA_INTERRUPT_STATUS_TXDONE_STATUS_MASK                       0x00001000
#define    IRDA_INTERRUPT_STATUS_DMA_TX_STATUS_SHIFT                      11
#define    IRDA_INTERRUPT_STATUS_DMA_TX_STATUS_MASK                       0x00000800
#define    IRDA_INTERRUPT_STATUS_DMA_RX_STATUS_SHIFT                      10
#define    IRDA_INTERRUPT_STATUS_DMA_RX_STATUS_MASK                       0x00000400
#define    IRDA_INTERRUPT_STATUS_FIFO_TX_UNDERRUN_STATUS_SHIFT            9
#define    IRDA_INTERRUPT_STATUS_FIFO_TX_UNDERRUN_STATUS_MASK             0x00000200
#define    IRDA_INTERRUPT_STATUS_FIFO_RX_OVERRUN_STATUS_SHIFT             8
#define    IRDA_INTERRUPT_STATUS_FIFO_RX_OVERRUN_STATUS_MASK              0x00000100
#define    IRDA_INTERRUPT_STATUS_RX_NOACTIVITY_STATUS_SHIFT               7
#define    IRDA_INTERRUPT_STATUS_RX_NOACTIVITY_STATUS_MASK                0x00000080
#define    IRDA_INTERRUPT_STATUS_SIP_DONE_STATUS_SHIFT                    6
#define    IRDA_INTERRUPT_STATUS_SIP_DONE_STATUS_MASK                     0x00000040
#define    IRDA_INTERRUPT_STATUS_PREAMBLE_STATUS_SHIFT                    5
#define    IRDA_INTERRUPT_STATUS_PREAMBLE_STATUS_MASK                     0x00000020
#define    IRDA_INTERRUPT_STATUS_ABORT_STATUS_SHIFT                       4
#define    IRDA_INTERRUPT_STATUS_ABORT_STATUS_MASK                        0x00000010
#define    IRDA_INTERRUPT_STATUS_STOP_STATUS_SHIFT                        3
#define    IRDA_INTERRUPT_STATUS_STOP_STATUS_MASK                         0x00000008
#define    IRDA_INTERRUPT_STATUS_START_STATUS_SHIFT                       2
#define    IRDA_INTERRUPT_STATUS_START_STATUS_MASK                        0x00000004
#define    IRDA_INTERRUPT_STATUS_RECEPTIONERROR_STATUS_SHIFT              1
#define    IRDA_INTERRUPT_STATUS_RECEPTIONERROR_STATUS_MASK               0x00000002
#define    IRDA_INTERRUPT_STATUS_INTERRUPT_LINE_SHIFT                     0
#define    IRDA_INTERRUPT_STATUS_INTERRUPT_LINE_MASK                      0x00000001

#define IRDA_INTERRUPT_ENABLE_OFFSET                                      0x0000000C
#define IRDA_INTERRUPT_ENABLE_TYPE                                        UInt32
#define IRDA_INTERRUPT_ENABLE_RESERVED_MASK                               0xFFF00001
#define    IRDA_INTERRUPT_ENABLE_PIO_TX_START_INTERRUPT_ENB_SHIFT         19
#define    IRDA_INTERRUPT_ENABLE_PIO_TX_START_INTERRUPT_ENB_MASK          0x00080000
#define    IRDA_INTERRUPT_ENABLE_PIO_TX_STOP_INTERRUPT_ENB_SHIFT          18
#define    IRDA_INTERRUPT_ENABLE_PIO_TX_STOP_INTERRUPT_ENB_MASK           0x00040000
#define    IRDA_INTERRUPT_ENABLE_PIO_RX_START_INTERRUPT_ENB_SHIFT         17
#define    IRDA_INTERRUPT_ENABLE_PIO_RX_START_INTERRUPT_ENB_MASK          0x00020000
#define    IRDA_INTERRUPT_ENABLE_PIO_RX_STOP_INTERRUPT_ENB_SHIFT          16
#define    IRDA_INTERRUPT_ENABLE_PIO_RX_STOP_INTERRUPT_ENB_MASK           0x00010000
#define    IRDA_INTERRUPT_ENABLE_APB_TX_ERROR_INTERRUPT_ENB_SHIFT         15
#define    IRDA_INTERRUPT_ENABLE_APB_TX_ERROR_INTERRUPT_ENB_MASK          0x00008000
#define    IRDA_INTERRUPT_ENABLE_APB_RX_ERROR_INTERRUPT_ENB_SHIFT         14
#define    IRDA_INTERRUPT_ENABLE_APB_RX_ERROR_INTERRUPT_ENB_MASK          0x00004000
#define    IRDA_INTERRUPT_ENABLE_BLOCKUPDATED_INTERRUPT_ENB_SHIFT         13
#define    IRDA_INTERRUPT_ENABLE_BLOCKUPDATED_INTERRUPT_ENB_MASK          0x00002000
#define    IRDA_INTERRUPT_ENABLE_TXDONE_INTERRUPT_ENB_SHIFT               12
#define    IRDA_INTERRUPT_ENABLE_TXDONE_INTERRUPT_ENB_MASK                0x00001000
#define    IRDA_INTERRUPT_ENABLE_DMA_TX_INTERRUPT_ENB_SHIFT               11
#define    IRDA_INTERRUPT_ENABLE_DMA_TX_INTERRUPT_ENB_MASK                0x00000800
#define    IRDA_INTERRUPT_ENABLE_DMA_RX_INTERRUPT_ENB_SHIFT               10
#define    IRDA_INTERRUPT_ENABLE_DMA_RX_INTERRUPT_ENB_MASK                0x00000400
#define    IRDA_INTERRUPT_ENABLE_FIFO_TX_UNDERRUN_ENB_SHIFT               9
#define    IRDA_INTERRUPT_ENABLE_FIFO_TX_UNDERRUN_ENB_MASK                0x00000200
#define    IRDA_INTERRUPT_ENABLE_FIFO_RX_OVERRUN_ENB_SHIFT                8
#define    IRDA_INTERRUPT_ENABLE_FIFO_RX_OVERRUN_ENB_MASK                 0x00000100
#define    IRDA_INTERRUPT_ENABLE_RX_NOACTIVITY_INTERRUPT_ENB_SHIFT        7
#define    IRDA_INTERRUPT_ENABLE_RX_NOACTIVITY_INTERRUPT_ENB_MASK         0x00000080
#define    IRDA_INTERRUPT_ENABLE_SIP_DONE_INTERRUPT_ENB_SHIFT             6
#define    IRDA_INTERRUPT_ENABLE_SIP_DONE_INTERRUPT_ENB_MASK              0x00000040
#define    IRDA_INTERRUPT_ENABLE_PREAMBLE_INTERRUPT_ENB_SHIFT             5
#define    IRDA_INTERRUPT_ENABLE_PREAMBLE_INTERRUPT_ENB_MASK              0x00000020
#define    IRDA_INTERRUPT_ENABLE_ABORT_INTERRUPT_ENB_SHIFT                4
#define    IRDA_INTERRUPT_ENABLE_ABORT_INTERRUPT_ENB_MASK                 0x00000010
#define    IRDA_INTERRUPT_ENABLE_STOP_INTERRUPT_ENB_SHIFT                 3
#define    IRDA_INTERRUPT_ENABLE_STOP_INTERRUPT_ENB_MASK                  0x00000008
#define    IRDA_INTERRUPT_ENABLE_START_INTERRUPT_ENB_SHIFT                2
#define    IRDA_INTERRUPT_ENABLE_START_INTERRUPT_ENB_MASK                 0x00000004
#define    IRDA_INTERRUPT_ENABLE_RECEPTIONERROR_INTERRUPT_ENB_SHIFT       1
#define    IRDA_INTERRUPT_ENABLE_RECEPTIONERROR_INTERRUPT_ENB_MASK        0x00000002

#define IRDA_LAST_FRAME_RECEIVED_STATUS_OFFSET                            0x00000010
#define IRDA_LAST_FRAME_RECEIVED_STATUS_TYPE                              UInt32
#define IRDA_LAST_FRAME_RECEIVED_STATUS_RESERVED_MASK                     0x00000000
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_ADDRESS_CONTROL_SHIFT    16
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_ADDRESS_CONTROL_MASK     0xFFFF0000
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_COMPLETE_SHIFT           15
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_COMPLETE_MASK            0x00008000
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_DISCARD_SHIFT            14
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_DISCARD_MASK             0x00004000
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_RECEPTION_ERROR_SHIFT    13
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_RECEPTION_ERROR_MASK     0x00002000
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_TRANSFERT_SIZE_SHIFT     0
#define    IRDA_LAST_FRAME_RECEIVED_STATUS_FRAME_TRANSFERT_SIZE_MASK      0x00001FFF

#define IRDA_RECEPTION_ERROR_STATUS_OFFSET                                0x00000014
#define IRDA_RECEPTION_ERROR_STATUS_TYPE                                  UInt32
#define IRDA_RECEPTION_ERROR_STATUS_RESERVED_MASK                         0x001FFFFF
#define    IRDA_RECEPTION_ERROR_STATUS_RECEPTIONERROR_STATUS_VECTOR_SHIFT 21
#define    IRDA_RECEPTION_ERROR_STATUS_RECEPTIONERROR_STATUS_VECTOR_MASK  0xFFE00000

#define IRDA_SIP_CONTROL_OFFSET                                           0x00000020
#define IRDA_SIP_CONTROL_TYPE                                             UInt32
#define IRDA_SIP_CONTROL_RESERVED_MASK                                    0x7FFFFF00
#define    IRDA_SIP_CONTROL_SIP_BLANK_DISABLE_SHIFT                       31
#define    IRDA_SIP_CONTROL_SIP_BLANK_DISABLE_MASK                        0x80000000
#define    IRDA_SIP_CONTROL_SIP_HIGH_CYCLE_SHIFT                          0
#define    IRDA_SIP_CONTROL_SIP_HIGH_CYCLE_MASK                           0x000000FF

#define IRDA_MODE_CONTROL_OFFSET                                          0x00000024
#define IRDA_MODE_CONTROL_TYPE                                            UInt32
#define IRDA_MODE_CONTROL_RESERVED_MASK                                   0x7F80FC00
#define    IRDA_MODE_CONTROL_RECEPTION_MODE_USE_FLAGS_SHIFT               31
#define    IRDA_MODE_CONTROL_RECEPTION_MODE_USE_FLAGS_MASK                0x80000000
#define    IRDA_MODE_CONTROL_RECEPTION_FIR_UN_CHIP_SHIFT                  22
#define    IRDA_MODE_CONTROL_RECEPTION_FIR_UN_CHIP_MASK                   0x00400000
#define    IRDA_MODE_CONTROL_RECEPTION_MIR_FLOAT_COMP_SHIFT               21
#define    IRDA_MODE_CONTROL_RECEPTION_MIR_FLOAT_COMP_MASK                0x00200000
#define    IRDA_MODE_CONTROL_RECEPTION_MIR_UN_STUFFING_SHIFT              20
#define    IRDA_MODE_CONTROL_RECEPTION_MIR_UN_STUFFING_MASK               0x00100000
#define    IRDA_MODE_CONTROL_RECEPTION_SIR_EXTRACTBYTE_SHIFT              19
#define    IRDA_MODE_CONTROL_RECEPTION_SIR_EXTRACTBYTE_MASK               0x00080000
#define    IRDA_MODE_CONTROL_RECEPTION_FIR_LEVELDETECTION_SHIFT           18
#define    IRDA_MODE_CONTROL_RECEPTION_FIR_LEVELDETECTION_MASK            0x00040000
#define    IRDA_MODE_CONTROL_RECEPTION_NO_DETECTION_SHIFT                 17
#define    IRDA_MODE_CONTROL_RECEPTION_NO_DETECTION_MASK                  0x00020000
#define    IRDA_MODE_CONTROL_RECEPTION_FILTER_ON_SHIFT                    16
#define    IRDA_MODE_CONTROL_RECEPTION_FILTER_ON_MASK                     0x00010000
#define    IRDA_MODE_CONTROL_TRANSMISSION_CIR_RATIO_SHIFT                 9
#define    IRDA_MODE_CONTROL_TRANSMISSION_CIR_RATIO_MASK                  0x00000200
#define    IRDA_MODE_CONTROL_TRANSMISSION_FIR_CHIP_OFF_SHIFT              8
#define    IRDA_MODE_CONTROL_TRANSMISSION_FIR_CHIP_OFF_MASK               0x00000100
#define    IRDA_MODE_CONTROL_TRANSMISSION_MIR_STUFFING_OFF_SHIFT          7
#define    IRDA_MODE_CONTROL_TRANSMISSION_MIR_STUFFING_OFF_MASK           0x00000080
#define    IRDA_MODE_CONTROL_TRANSMISSION_SIR_FREQUENCY_SHIFT             4
#define    IRDA_MODE_CONTROL_TRANSMISSION_SIR_FREQUENCY_MASK              0x00000070
#define    IRDA_MODE_CONTROL_TRANSMISSION_SIR_MINIMUMPULSE_ON_SHIFT       3
#define    IRDA_MODE_CONTROL_TRANSMISSION_SIR_MINIMUMPULSE_ON_MASK        0x00000008
#define    IRDA_MODE_CONTROL_TRANSMISSION_IRDA_MODE_SHIFT                 1
#define    IRDA_MODE_CONTROL_TRANSMISSION_IRDA_MODE_MASK                  0x00000006
#define    IRDA_MODE_CONTROL_TRANSMISSION_MODE_USE_FLAGS_SHIFT            0
#define    IRDA_MODE_CONTROL_TRANSMISSION_MODE_USE_FLAGS_MASK             0x00000001

#define IRDA_DETECTION_CONTROL_OFFSET                                     0x00000028
#define IRDA_DETECTION_CONTROL_TYPE                                       UInt32
#define IRDA_DETECTION_CONTROL_RESERVED_MASK                              0x8600FC00
#define    IRDA_DETECTION_CONTROL_RECEPTION_MAX_ONEZERO_SHIFT             27
#define    IRDA_DETECTION_CONTROL_RECEPTION_MAX_ONEZERO_MASK              0x78000000
#define    IRDA_DETECTION_CONTROL_RECEPTION_DELTA_CYCLES_SHIFT            16
#define    IRDA_DETECTION_CONTROL_RECEPTION_DELTA_CYCLES_MASK             0x01FF0000
#define    IRDA_DETECTION_CONTROL_RECEPTION_MEDIAN_CYCLES_SHIFT           0
#define    IRDA_DETECTION_CONTROL_RECEPTION_MEDIAN_CYCLES_MASK            0x000003FF

#define IRDA_ACTIVITY_CONTROL_OFFSET                                      0x0000002C
#define IRDA_ACTIVITY_CONTROL_TYPE                                        UInt32
#define IRDA_ACTIVITY_CONTROL_RESERVED_MASK                               0xFFE00000
#define    IRDA_ACTIVITY_CONTROL_NOACTIVITY_CYCLES_SHIFT                  0
#define    IRDA_ACTIVITY_CONTROL_NOACTIVITY_CYCLES_MASK                   0x001FFFFF

#define IRDA_CIR_CONTROL_OFFSET                                           0x00000030
#define IRDA_CIR_CONTROL_TYPE                                             UInt32
#define IRDA_CIR_CONTROL_RESERVED_MASK                                    0x00000000
#define    IRDA_CIR_CONTROL_CIR_HIGH_UNIT_SHIFT                           24
#define    IRDA_CIR_CONTROL_CIR_HIGH_UNIT_MASK                            0xFF000000
#define    IRDA_CIR_CONTROL_CIR_LOW_UNIT_SHIFT                            16
#define    IRDA_CIR_CONTROL_CIR_LOW_UNIT_MASK                             0x00FF0000
#define    IRDA_CIR_CONTROL_CIR_REPEAT_SHIFT                              0
#define    IRDA_CIR_CONTROL_CIR_REPEAT_MASK                               0x0000FFFF

#define IRDA_START_FLAG_CONTROL_OFFSET                                    0x00000040
#define IRDA_START_FLAG_CONTROL_TYPE                                      UInt32
#define IRDA_START_FLAG_CONTROL_RESERVED_MASK                             0x73FFFFC0
#define    IRDA_START_FLAG_CONTROL_START_TRANSMISSION_VALID_SHIFT         31
#define    IRDA_START_FLAG_CONTROL_START_TRANSMISSION_VALID_MASK          0x80000000
#define    IRDA_START_FLAG_CONTROL_START_RECEPTION_VALID_SHIFT            27
#define    IRDA_START_FLAG_CONTROL_START_RECEPTION_VALID_MASK             0x08000000
#define    IRDA_START_FLAG_CONTROL_START_RECEPTION_EXACTREPEAT_SHIFT      26
#define    IRDA_START_FLAG_CONTROL_START_RECEPTION_EXACTREPEAT_MASK       0x04000000
#define    IRDA_START_FLAG_CONTROL_START_REPEAT_SHIFT                     2
#define    IRDA_START_FLAG_CONTROL_START_REPEAT_MASK                      0x0000003C
#define    IRDA_START_FLAG_CONTROL_START_SIZE_SHIFT                       0
#define    IRDA_START_FLAG_CONTROL_START_SIZE_MASK                        0x00000003

#define IRDA_START_FLAG_OFFSET                                            0x00000044
#define IRDA_START_FLAG_TYPE                                              UInt32
#define IRDA_START_FLAG_RESERVED_MASK                                     0x00000000
#define    IRDA_START_FLAG_START_VALUE_SHIFT                              0
#define    IRDA_START_FLAG_START_VALUE_MASK                               0xFFFFFFFF

#define IRDA_STOP_FLAG_CONTROL_OFFSET                                     0x00000048
#define IRDA_STOP_FLAG_CONTROL_TYPE                                       UInt32
#define IRDA_STOP_FLAG_CONTROL_RESERVED_MASK                              0x73FFFFC0
#define    IRDA_STOP_FLAG_CONTROL_STOP_TRANSMISSION_VALID_SHIFT           31
#define    IRDA_STOP_FLAG_CONTROL_STOP_TRANSMISSION_VALID_MASK            0x80000000
#define    IRDA_STOP_FLAG_CONTROL_STOP_RECEPTION_VALID_SHIFT              27
#define    IRDA_STOP_FLAG_CONTROL_STOP_RECEPTION_VALID_MASK               0x08000000
#define    IRDA_STOP_FLAG_CONTROL_STOP_RECEPTION_EXACTREPEAT_SHIFT        26
#define    IRDA_STOP_FLAG_CONTROL_STOP_RECEPTION_EXACTREPEAT_MASK         0x04000000
#define    IRDA_STOP_FLAG_CONTROL_STOP_REPEAT_SHIFT                       2
#define    IRDA_STOP_FLAG_CONTROL_STOP_REPEAT_MASK                        0x0000003C
#define    IRDA_STOP_FLAG_CONTROL_STOP_SIZE_SHIFT                         0
#define    IRDA_STOP_FLAG_CONTROL_STOP_SIZE_MASK                          0x00000003

#define IRDA_STOP_FLAG_OFFSET                                             0x0000004C
#define IRDA_STOP_FLAG_TYPE                                               UInt32
#define IRDA_STOP_FLAG_RESERVED_MASK                                      0x00000000
#define    IRDA_STOP_FLAG_STOP_VALUE_SHIFT                                0
#define    IRDA_STOP_FLAG_STOP_VALUE_MASK                                 0xFFFFFFFF

#define IRDA_PREAMBLE_FLAG_CONTROL_OFFSET                                 0x00000050
#define IRDA_PREAMBLE_FLAG_CONTROL_TYPE                                   UInt32
#define IRDA_PREAMBLE_FLAG_CONTROL_RESERVED_MASK                          0x30FFFFC0
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_TRANSMISSION_VALID_SHIFT   31
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_TRANSMISSION_VALID_MASK    0x80000000
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_TRANSMISSION_FILLING_SHIFT 30
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_TRANSMISSION_FILLING_MASK  0x40000000
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_VALID_SHIFT      27
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_VALID_MASK       0x08000000
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_EXACTREPEAT_SHIFT 26
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_EXACTREPEAT_MASK 0x04000000
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_OPTIONAL_SHIFT   25
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_OPTIONAL_MASK    0x02000000
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_IGNORE_INSIDEDATA_SHIFT 24
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_RECEPTION_IGNORE_INSIDEDATA_MASK 0x01000000
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_REPEAT_SHIFT               2
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_REPEAT_MASK                0x0000003C
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_SIZE_SHIFT                 0
#define    IRDA_PREAMBLE_FLAG_CONTROL_PREAMBLE_SIZE_MASK                  0x00000003

#define IRDA_PREAMBLE_FLAG_OFFSET                                         0x00000054
#define IRDA_PREAMBLE_FLAG_TYPE                                           UInt32
#define IRDA_PREAMBLE_FLAG_RESERVED_MASK                                  0x00000000
#define    IRDA_PREAMBLE_FLAG_PREAMBLE_VALUE_SHIFT                        0
#define    IRDA_PREAMBLE_FLAG_PREAMBLE_VALUE_MASK                         0xFFFFFFFF

#define IRDA_ABORT_FLAG_CONTROL_OFFSET                                    0x00000058
#define IRDA_ABORT_FLAG_CONTROL_TYPE                                      UInt32
#define IRDA_ABORT_FLAG_CONTROL_RESERVED_MASK                             0x33FFFFC0
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_TRANSMISSION_VALID_SHIFT         31
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_TRANSMISSION_VALID_MASK          0x80000000
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_TRANSMISSION_FILLING_SHIFT       30
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_TRANSMISSION_FILLING_MASK        0x40000000
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_RECEPTION_VALID_SHIFT            27
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_RECEPTION_VALID_MASK             0x08000000
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_RECEPTION_EXACTREPEAT_SHIFT      26
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_RECEPTION_EXACTREPEAT_MASK       0x04000000
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_REPEAT_SHIFT                     2
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_REPEAT_MASK                      0x0000003C
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_SIZE_SHIFT                       0
#define    IRDA_ABORT_FLAG_CONTROL_ABORT_SIZE_MASK                        0x00000003

#define IRDA_ABORT_FLAG_OFFSET                                            0x0000005C
#define IRDA_ABORT_FLAG_TYPE                                              UInt32
#define IRDA_ABORT_FLAG_RESERVED_MASK                                     0x00000000
#define    IRDA_ABORT_FLAG_ABORT_VALUE_SHIFT                              0
#define    IRDA_ABORT_FLAG_ABORT_VALUE_MASK                               0xFFFFFFFF

#define IRDA_DMA_RX_CONTROL_OFFSET                                        0x00000080
#define IRDA_DMA_RX_CONTROL_TYPE                                          UInt32
#define IRDA_DMA_RX_CONTROL_RESERVED_MASK                                 0xFFFFFFF8
#define    IRDA_DMA_RX_CONTROL_DMA_RX_FLUSH_SHIFT                         2
#define    IRDA_DMA_RX_CONTROL_DMA_RX_FLUSH_MASK                          0x00000004
#define    IRDA_DMA_RX_CONTROL_DMA_RX_BURSTSIZE_SHIFT                     1
#define    IRDA_DMA_RX_CONTROL_DMA_RX_BURSTSIZE_MASK                      0x00000002
#define    IRDA_DMA_RX_CONTROL_DMA_RX_ON_SHIFT                            0
#define    IRDA_DMA_RX_CONTROL_DMA_RX_ON_MASK                             0x00000001

#define IRDA_DMA_TX_CONTROL_OFFSET                                        0x00000084
#define IRDA_DMA_TX_CONTROL_TYPE                                          UInt32
#define IRDA_DMA_TX_CONTROL_RESERVED_MASK                                 0x0003FFFC
#define    IRDA_DMA_TX_CONTROL_DMA_TX_SIZE_SHIFT                          18
#define    IRDA_DMA_TX_CONTROL_DMA_TX_SIZE_MASK                           0xFFFC0000
#define    IRDA_DMA_TX_CONTROL_DMA_TX_BURSTSIZE_SHIFT                     1
#define    IRDA_DMA_TX_CONTROL_DMA_TX_BURSTSIZE_MASK                      0x00000002
#define    IRDA_DMA_TX_CONTROL_DMA_TX_ON_SHIFT                            0
#define    IRDA_DMA_TX_CONTROL_DMA_TX_ON_MASK                             0x00000001

#define IRDA_FIFO_RX_CONTROL_OFFSET                                       0x00000090
#define IRDA_FIFO_RX_CONTROL_TYPE                                         UInt32
#define IRDA_FIFO_RX_CONTROL_RESERVED_MASK                                0x000CFFC8
#define    IRDA_FIFO_RX_CONTROL_FIFORX_LEVEL_SHIFT                        20
#define    IRDA_FIFO_RX_CONTROL_FIFORX_LEVEL_MASK                         0xFFF00000
#define    IRDA_FIFO_RX_CONTROL_FIFORX_EMPTY_SHIFT                        17
#define    IRDA_FIFO_RX_CONTROL_FIFORX_EMPTY_MASK                         0x00020000
#define    IRDA_FIFO_RX_CONTROL_FIFORX_FULL_SHIFT                         16
#define    IRDA_FIFO_RX_CONTROL_FIFORX_FULL_MASK                          0x00010000
#define    IRDA_FIFO_RX_CONTROL_FIFORX_PIO_START_ENABLE_SHIFT             5
#define    IRDA_FIFO_RX_CONTROL_FIFORX_PIO_START_ENABLE_MASK              0x00000020
#define    IRDA_FIFO_RX_CONTROL_FIFORX_PIO_STOP_ENABLE_SHIFT              4
#define    IRDA_FIFO_RX_CONTROL_FIFORX_PIO_STOP_ENABLE_MASK               0x00000010
#define    IRDA_FIFO_RX_CONTROL_FIFOTX_DATA_READ_SIZE_SHIFT               2
#define    IRDA_FIFO_RX_CONTROL_FIFOTX_DATA_READ_SIZE_MASK                0x00000004
#define    IRDA_FIFO_RX_CONTROL_FIFORX_ENDIANESS_SHIFT                    1
#define    IRDA_FIFO_RX_CONTROL_FIFORX_ENDIANESS_MASK                     0x00000002
#define    IRDA_FIFO_RX_CONTROL_FIFORX_RESET_SHIFT                        0
#define    IRDA_FIFO_RX_CONTROL_FIFORX_RESET_MASK                         0x00000001

#define IRDA_FIFO_RX_THRESHOLD_OFFSET                                     0x00000094
#define IRDA_FIFO_RX_THRESHOLD_TYPE                                       UInt32
#define IRDA_FIFO_RX_THRESHOLD_RESERVED_MASK                              0xF000F000
#define    IRDA_FIFO_RX_THRESHOLD_FIFORX_PIO_STOP_THRESHOLD_SHIFT         16
#define    IRDA_FIFO_RX_THRESHOLD_FIFORX_PIO_STOP_THRESHOLD_MASK          0x0FFF0000
#define    IRDA_FIFO_RX_THRESHOLD_FIFORX_PIO_START_THRESHOLD_SHIFT        0
#define    IRDA_FIFO_RX_THRESHOLD_FIFORX_PIO_START_THRESHOLD_MASK         0x00000FFF

#define IRDA_FIFO_TX_CONTROL_OFFSET                                       0x00000098
#define IRDA_FIFO_TX_CONTROL_TYPE                                         UInt32
#define IRDA_FIFO_TX_CONTROL_RESERVED_MASK                                0x000CFFC8
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_LEVEL_SHIFT                        20
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_LEVEL_MASK                         0xFFF00000
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_EMPTY_SHIFT                        17
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_EMPTY_MASK                         0x00020000
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_FULL_SHIFT                         16
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_FULL_MASK                          0x00010000
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_PIO_START_ENABLE_SHIFT             5
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_PIO_START_ENABLE_MASK              0x00000020
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_PIO_STOP_ENABLE_SHIFT              4
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_PIO_STOP_ENABLE_MASK               0x00000010
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_DATA_WRITE_SIZE_SHIFT              2
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_DATA_WRITE_SIZE_MASK               0x00000004
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_ENDIANESS_SHIFT                    1
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_ENDIANESS_MASK                     0x00000002
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_RESET_SHIFT                        0
#define    IRDA_FIFO_TX_CONTROL_FIFOTX_RESET_MASK                         0x00000001

#define IRDA_FIFO_TX_THRESHOLD_OFFSET                                     0x0000009C
#define IRDA_FIFO_TX_THRESHOLD_TYPE                                       UInt32
#define IRDA_FIFO_TX_THRESHOLD_RESERVED_MASK                              0xF000F000
#define    IRDA_FIFO_TX_THRESHOLD_FIFOTX_PIO_STOP_THRESHOLD_SHIFT         16
#define    IRDA_FIFO_TX_THRESHOLD_FIFOTX_PIO_STOP_THRESHOLD_MASK          0x0FFF0000
#define    IRDA_FIFO_TX_THRESHOLD_FIFOTX_PIO_START_THRESHOLD_SHIFT        0
#define    IRDA_FIFO_TX_THRESHOLD_FIFOTX_PIO_START_THRESHOLD_MASK         0x00000FFF

#define IRDA_FIFO_ENTRYRX_OFFSET                                          0x00000100
#define IRDA_FIFO_ENTRYRX_TYPE                                            UInt32
#define IRDA_FIFO_ENTRYRX_RESERVED_MASK                                   0x00000000
#define    IRDA_FIFO_ENTRYRX_FIFO_ENTRYRX_SHIFT                           0
#define    IRDA_FIFO_ENTRYRX_FIFO_ENTRYRX_MASK                            0xFFFFFFFF

#define IRDA_FIFO_ENTRYTX_OFFSET                                          0x00000200
#define IRDA_FIFO_ENTRYTX_TYPE                                            UInt32
#define IRDA_FIFO_ENTRYTX_RESERVED_MASK                                   0x00000000
#define    IRDA_FIFO_ENTRYTX_FIFO_ENTRYTX_SHIFT                           0
#define    IRDA_FIFO_ENTRYTX_FIFO_ENTRYTX_MASK                            0xFFFFFFFF

#endif /* __BRCM_RDB_IRDA_H__ */



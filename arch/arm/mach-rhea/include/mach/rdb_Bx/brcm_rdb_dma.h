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

#ifndef __BRCM_RDB_DMA_H__
#define __BRCM_RDB_DMA_H__

#define DMA_CS_OFFSET                                                     0x00000000
#define DMA_CS_TYPE                                                       UInt32
#define DMA_CS_RESERVED_MASK                                              0x0F00FE80
#define    DMA_CS_RESET_SHIFT                                             31
#define    DMA_CS_RESET_MASK                                              0x80000000
#define    DMA_CS_ABORT_SHIFT                                             30
#define    DMA_CS_ABORT_MASK                                              0x40000000
#define    DMA_CS_DISDEBUG_SHIFT                                          29
#define    DMA_CS_DISDEBUG_MASK                                           0x20000000
#define    DMA_CS_WAIT_FOR_OUTSTANDING_WRITES_SHIFT                       28
#define    DMA_CS_WAIT_FOR_OUTSTANDING_WRITES_MASK                        0x10000000
#define    DMA_CS_PANIC_PRIORITY_SHIFT                                    20
#define    DMA_CS_PANIC_PRIORITY_MASK                                     0x00F00000
#define    DMA_CS_PRIORITY_SHIFT                                          16
#define    DMA_CS_PRIORITY_MASK                                           0x000F0000
#define    DMA_CS_ERROR_SHIFT                                             8
#define    DMA_CS_ERROR_MASK                                              0x00000100
#define    DMA_CS_WAITING_FOR_OUTSTANDING_WRITES_SHIFT                    6
#define    DMA_CS_WAITING_FOR_OUTSTANDING_WRITES_MASK                     0x00000040
#define    DMA_CS_DREQ_STOPS_DMA_SHIFT                                    5
#define    DMA_CS_DREQ_STOPS_DMA_MASK                                     0x00000020
#define    DMA_CS_PAUSED_SHIFT                                            4
#define    DMA_CS_PAUSED_MASK                                             0x00000010
#define    DMA_CS_DREQ_SHIFT                                              3
#define    DMA_CS_DREQ_MASK                                               0x00000008
#define    DMA_CS_INT_SHIFT                                               2
#define    DMA_CS_INT_MASK                                                0x00000004
#define    DMA_CS_END_SHIFT                                               1
#define    DMA_CS_END_MASK                                                0x00000002
#define    DMA_CS_ACTIVE_SHIFT                                            0
#define    DMA_CS_ACTIVE_MASK                                             0x00000001

#define DMA_CONBLK_AD_OFFSET                                              0x00000004
#define DMA_CONBLK_AD_TYPE                                                UInt32
#define DMA_CONBLK_AD_RESERVED_MASK                                       0x00000000
#define    DMA_CONBLK_AD_SCB_ADDR_SHIFT                                   0
#define    DMA_CONBLK_AD_SCB_ADDR_MASK                                    0xFFFFFFFF

#define DMA_TI_OFFSET                                                     0x00000008
#define DMA_TI_TYPE                                                       UInt32
#define DMA_TI_RESERVED_MASK                                              0xF8000004
#define    DMA_TI_NO_WIDE_BURSTS_SHIFT                                    26
#define    DMA_TI_NO_WIDE_BURSTS_MASK                                     0x04000000
#define    DMA_TI_WAITS_SHIFT                                             21
#define    DMA_TI_WAITS_MASK                                              0x03E00000
#define    DMA_TI_PERMAP_SHIFT                                            16
#define    DMA_TI_PERMAP_MASK                                             0x001F0000
#define    DMA_TI_BURST_LENGTH_SHIFT                                      12
#define    DMA_TI_BURST_LENGTH_MASK                                       0x0000F000
#define    DMA_TI_SRC_IGNORE_SHIFT                                        11
#define    DMA_TI_SRC_IGNORE_MASK                                         0x00000800
#define    DMA_TI_SRC_DREQ_SHIFT                                          10
#define    DMA_TI_SRC_DREQ_MASK                                           0x00000400
#define    DMA_TI_SRC_WIDTH_SHIFT                                         9
#define    DMA_TI_SRC_WIDTH_MASK                                          0x00000200
#define    DMA_TI_SRC_INC_SHIFT                                           8
#define    DMA_TI_SRC_INC_MASK                                            0x00000100
#define    DMA_TI_DEST_IGNORE_SHIFT                                       7
#define    DMA_TI_DEST_IGNORE_MASK                                        0x00000080
#define    DMA_TI_DEST_DREQ_SHIFT                                         6
#define    DMA_TI_DEST_DREQ_MASK                                          0x00000040
#define    DMA_TI_DEST_WIDTH_SHIFT                                        5
#define    DMA_TI_DEST_WIDTH_MASK                                         0x00000020
#define    DMA_TI_DEST_INC_SHIFT                                          4
#define    DMA_TI_DEST_INC_MASK                                           0x00000010
#define    DMA_TI_WAIT_RESP_SHIFT                                         3
#define    DMA_TI_WAIT_RESP_MASK                                          0x00000008
#define    DMA_TI_TDMODE_SHIFT                                            1
#define    DMA_TI_TDMODE_MASK                                             0x00000002
#define    DMA_TI_INTEN_SHIFT                                             0
#define    DMA_TI_INTEN_MASK                                              0x00000001

#define DMA_SOURCE_AD_OFFSET                                              0x0000000C
#define DMA_SOURCE_AD_TYPE                                                UInt32
#define DMA_SOURCE_AD_RESERVED_MASK                                       0x00000000
#define    DMA_SOURCE_AD_S_ADDR_SHIFT                                     0
#define    DMA_SOURCE_AD_S_ADDR_MASK                                      0xFFFFFFFF

#define DMA_DEST_AD_OFFSET                                                0x00000010
#define DMA_DEST_AD_TYPE                                                  UInt32
#define DMA_DEST_AD_RESERVED_MASK                                         0x00000000
#define    DMA_DEST_AD_D_ADDR_SHIFT                                       0
#define    DMA_DEST_AD_D_ADDR_MASK                                        0xFFFFFFFF

#define DMA_TXFR_LEN_OFFSET                                               0x00000014
#define DMA_TXFR_LEN_TYPE                                                 UInt32
#define DMA_TXFR_LEN_RESERVED_MASK                                        0xC0000000
#define    DMA_TXFR_LEN_YLENGTH_SHIFT                                     16
#define    DMA_TXFR_LEN_YLENGTH_MASK                                      0x3FFF0000
#define    DMA_TXFR_LEN_XLENGTH_SHIFT                                     0
#define    DMA_TXFR_LEN_XLENGTH_MASK                                      0x0000FFFF

#define DMA_STRIDE_OFFSET                                                 0x00000018
#define DMA_STRIDE_TYPE                                                   UInt32
#define DMA_STRIDE_RESERVED_MASK                                          0x00000000
#define    DMA_STRIDE_D_STRIDE_SHIFT                                      16
#define    DMA_STRIDE_D_STRIDE_MASK                                       0xFFFF0000
#define    DMA_STRIDE_S_STRIDE_SHIFT                                      0
#define    DMA_STRIDE_S_STRIDE_MASK                                       0x0000FFFF

#define DMA_NEXTCONBK_OFFSET                                              0x0000001C
#define DMA_NEXTCONBK_TYPE                                                UInt32
#define DMA_NEXTCONBK_RESERVED_MASK                                       0x00000000
#define    DMA_NEXTCONBK_NEXTCONBK_SHIFT                                  0
#define    DMA_NEXTCONBK_NEXTCONBK_MASK                                   0xFFFFFFFF

#define DMA_DEBUG_OFFSET                                                  0x00000020
#define DMA_DEBUG_TYPE                                                    UInt32
#define DMA_DEBUG_RESERVED_MASK                                           0xE0000008
#define    DMA_DEBUG_LITE_SHIFT                                           28
#define    DMA_DEBUG_LITE_MASK                                            0x10000000
#define    DMA_DEBUG_VERSION_SHIFT                                        25
#define    DMA_DEBUG_VERSION_MASK                                         0x0E000000
#define    DMA_DEBUG_DMA_STATE_SHIFT                                      16
#define    DMA_DEBUG_DMA_STATE_MASK                                       0x01FF0000
#define    DMA_DEBUG_DMA_ID_SHIFT                                         8
#define    DMA_DEBUG_DMA_ID_MASK                                          0x0000FF00
#define    DMA_DEBUG_OUTSTANDING_WRITES_SHIFT                             4
#define    DMA_DEBUG_OUTSTANDING_WRITES_MASK                              0x000000F0
#define    DMA_DEBUG_READ_ERROR_SHIFT                                     2
#define    DMA_DEBUG_READ_ERROR_MASK                                      0x00000004
#define    DMA_DEBUG_FIFO_ERROR_SHIFT                                     1
#define    DMA_DEBUG_FIFO_ERROR_MASK                                      0x00000002
#define    DMA_DEBUG_READ_LAST_NOT_SET_ERROR_SHIFT                        0
#define    DMA_DEBUG_READ_LAST_NOT_SET_ERROR_MASK                         0x00000001

#define DMA_CS1_OFFSET                                                    0x00000100
#define DMA_CS1_TYPE                                                      UInt32
#define DMA_CS1_RESERVED_MASK                                             0x0F00FE80
#define    DMA_CS1_RESET_SHIFT                                            31
#define    DMA_CS1_RESET_MASK                                             0x80000000
#define    DMA_CS1_ABORT_SHIFT                                            30
#define    DMA_CS1_ABORT_MASK                                             0x40000000
#define    DMA_CS1_DISDEBUG_SHIFT                                         29
#define    DMA_CS1_DISDEBUG_MASK                                          0x20000000
#define    DMA_CS1_WAIT_FOR_OUTSTANDING_WRITES_SHIFT                      28
#define    DMA_CS1_WAIT_FOR_OUTSTANDING_WRITES_MASK                       0x10000000
#define    DMA_CS1_PANIC_PRIORITY_SHIFT                                   20
#define    DMA_CS1_PANIC_PRIORITY_MASK                                    0x00F00000
#define    DMA_CS1_PRIORITY_SHIFT                                         16
#define    DMA_CS1_PRIORITY_MASK                                          0x000F0000
#define    DMA_CS1_ERROR_SHIFT                                            8
#define    DMA_CS1_ERROR_MASK                                             0x00000100
#define    DMA_CS1_WAITING_FOR_OUTSTANDING_WRITES_SHIFT                   6
#define    DMA_CS1_WAITING_FOR_OUTSTANDING_WRITES_MASK                    0x00000040
#define    DMA_CS1_DREQ_STOPS_DMA_SHIFT                                   5
#define    DMA_CS1_DREQ_STOPS_DMA_MASK                                    0x00000020
#define    DMA_CS1_PAUSED_SHIFT                                           4
#define    DMA_CS1_PAUSED_MASK                                            0x00000010
#define    DMA_CS1_DREQ_SHIFT                                             3
#define    DMA_CS1_DREQ_MASK                                              0x00000008
#define    DMA_CS1_INT_SHIFT                                              2
#define    DMA_CS1_INT_MASK                                               0x00000004
#define    DMA_CS1_END_SHIFT                                              1
#define    DMA_CS1_END_MASK                                               0x00000002
#define    DMA_CS1_ACTIVE_SHIFT                                           0
#define    DMA_CS1_ACTIVE_MASK                                            0x00000001

#define DMA_CONBLK_AD1_OFFSET                                             0x00000104
#define DMA_CONBLK_AD1_TYPE                                               UInt32
#define DMA_CONBLK_AD1_RESERVED_MASK                                      0x00000000
#define    DMA_CONBLK_AD1_SCB_ADDR_SHIFT                                  0
#define    DMA_CONBLK_AD1_SCB_ADDR_MASK                                   0xFFFFFFFF

#define DMA_TI1_OFFSET                                                    0x00000108
#define DMA_TI1_TYPE                                                      UInt32
#define DMA_TI1_RESERVED_MASK                                             0xF8000004
#define    DMA_TI1_NO_WIDE_BURSTS_SHIFT                                   26
#define    DMA_TI1_NO_WIDE_BURSTS_MASK                                    0x04000000
#define    DMA_TI1_WAITS_SHIFT                                            21
#define    DMA_TI1_WAITS_MASK                                             0x03E00000
#define    DMA_TI1_PERMAP_SHIFT                                           16
#define    DMA_TI1_PERMAP_MASK                                            0x001F0000
#define    DMA_TI1_BURST_LENGTH_SHIFT                                     12
#define    DMA_TI1_BURST_LENGTH_MASK                                      0x0000F000
#define    DMA_TI1_SRC_IGNORE_SHIFT                                       11
#define    DMA_TI1_SRC_IGNORE_MASK                                        0x00000800
#define    DMA_TI1_SRC_DREQ_SHIFT                                         10
#define    DMA_TI1_SRC_DREQ_MASK                                          0x00000400
#define    DMA_TI1_SRC_WIDTH_SHIFT                                        9
#define    DMA_TI1_SRC_WIDTH_MASK                                         0x00000200
#define    DMA_TI1_SRC_INC_SHIFT                                          8
#define    DMA_TI1_SRC_INC_MASK                                           0x00000100
#define    DMA_TI1_DEST_IGNORE_SHIFT                                      7
#define    DMA_TI1_DEST_IGNORE_MASK                                       0x00000080
#define    DMA_TI1_DEST_DREQ_SHIFT                                        6
#define    DMA_TI1_DEST_DREQ_MASK                                         0x00000040
#define    DMA_TI1_DEST_WIDTH_SHIFT                                       5
#define    DMA_TI1_DEST_WIDTH_MASK                                        0x00000020
#define    DMA_TI1_DEST_INC_SHIFT                                         4
#define    DMA_TI1_DEST_INC_MASK                                          0x00000010
#define    DMA_TI1_WAIT_RESP_SHIFT                                        3
#define    DMA_TI1_WAIT_RESP_MASK                                         0x00000008
#define    DMA_TI1_TDMODE_SHIFT                                           1
#define    DMA_TI1_TDMODE_MASK                                            0x00000002
#define    DMA_TI1_INTEN_SHIFT                                            0
#define    DMA_TI1_INTEN_MASK                                             0x00000001

#define DMA_SOURCE_AD1_OFFSET                                             0x0000010C
#define DMA_SOURCE_AD1_TYPE                                               UInt32
#define DMA_SOURCE_AD1_RESERVED_MASK                                      0x00000000
#define    DMA_SOURCE_AD1_S_ADDR_SHIFT                                    0
#define    DMA_SOURCE_AD1_S_ADDR_MASK                                     0xFFFFFFFF

#define DMA_DEST_AD1_OFFSET                                               0x00000110
#define DMA_DEST_AD1_TYPE                                                 UInt32
#define DMA_DEST_AD1_RESERVED_MASK                                        0x00000000
#define    DMA_DEST_AD1_D_ADDR_SHIFT                                      0
#define    DMA_DEST_AD1_D_ADDR_MASK                                       0xFFFFFFFF

#define DMA_TXFR_LEN1_OFFSET                                              0x00000114
#define DMA_TXFR_LEN1_TYPE                                                UInt32
#define DMA_TXFR_LEN1_RESERVED_MASK                                       0xC0000000
#define    DMA_TXFR_LEN1_YLENGTH_SHIFT                                    16
#define    DMA_TXFR_LEN1_YLENGTH_MASK                                     0x3FFF0000
#define    DMA_TXFR_LEN1_XLENGTH_SHIFT                                    0
#define    DMA_TXFR_LEN1_XLENGTH_MASK                                     0x0000FFFF

#define DMA_STRIDE1_OFFSET                                                0x00000118
#define DMA_STRIDE1_TYPE                                                  UInt32
#define DMA_STRIDE1_RESERVED_MASK                                         0x00000000
#define    DMA_STRIDE1_D_STRIDE_SHIFT                                     16
#define    DMA_STRIDE1_D_STRIDE_MASK                                      0xFFFF0000
#define    DMA_STRIDE1_S_STRIDE_SHIFT                                     0
#define    DMA_STRIDE1_S_STRIDE_MASK                                      0x0000FFFF

#define DMA_NEXTCONBK1_OFFSET                                             0x0000011C
#define DMA_NEXTCONBK1_TYPE                                               UInt32
#define DMA_NEXTCONBK1_RESERVED_MASK                                      0x00000000
#define    DMA_NEXTCONBK1_NEXTCONBK_SHIFT                                 0
#define    DMA_NEXTCONBK1_NEXTCONBK_MASK                                  0xFFFFFFFF

#define DMA_DEBUG1_OFFSET                                                 0x00000120
#define DMA_DEBUG1_TYPE                                                   UInt32
#define DMA_DEBUG1_RESERVED_MASK                                          0xE0000008
#define    DMA_DEBUG1_LITE_SHIFT                                          28
#define    DMA_DEBUG1_LITE_MASK                                           0x10000000
#define    DMA_DEBUG1_VERSION_SHIFT                                       25
#define    DMA_DEBUG1_VERSION_MASK                                        0x0E000000
#define    DMA_DEBUG1_DMA_STATE_SHIFT                                     16
#define    DMA_DEBUG1_DMA_STATE_MASK                                      0x01FF0000
#define    DMA_DEBUG1_DMA_ID_SHIFT                                        8
#define    DMA_DEBUG1_DMA_ID_MASK                                         0x0000FF00
#define    DMA_DEBUG1_OUTSTANDING_WRITES_SHIFT                            4
#define    DMA_DEBUG1_OUTSTANDING_WRITES_MASK                             0x000000F0
#define    DMA_DEBUG1_READ_ERROR_SHIFT                                    2
#define    DMA_DEBUG1_READ_ERROR_MASK                                     0x00000004
#define    DMA_DEBUG1_FIFO_ERROR_SHIFT                                    1
#define    DMA_DEBUG1_FIFO_ERROR_MASK                                     0x00000002
#define    DMA_DEBUG1_READ_LAST_NOT_SET_ERROR_SHIFT                       0
#define    DMA_DEBUG1_READ_LAST_NOT_SET_ERROR_MASK                        0x00000001

#define DMA_CS2_OFFSET                                                    0x00000200
#define DMA_CS2_TYPE                                                      UInt32
#define DMA_CS2_RESERVED_MASK                                             0x0F00FE80
#define    DMA_CS2_RESET_SHIFT                                            31
#define    DMA_CS2_RESET_MASK                                             0x80000000
#define    DMA_CS2_ABORT_SHIFT                                            30
#define    DMA_CS2_ABORT_MASK                                             0x40000000
#define    DMA_CS2_DISDEBUG_SHIFT                                         29
#define    DMA_CS2_DISDEBUG_MASK                                          0x20000000
#define    DMA_CS2_WAIT_FOR_OUTSTANDING_WRITES_SHIFT                      28
#define    DMA_CS2_WAIT_FOR_OUTSTANDING_WRITES_MASK                       0x10000000
#define    DMA_CS2_PANIC_PRIORITY_SHIFT                                   20
#define    DMA_CS2_PANIC_PRIORITY_MASK                                    0x00F00000
#define    DMA_CS2_PRIORITY_SHIFT                                         16
#define    DMA_CS2_PRIORITY_MASK                                          0x000F0000
#define    DMA_CS2_ERROR_SHIFT                                            8
#define    DMA_CS2_ERROR_MASK                                             0x00000100
#define    DMA_CS2_WAITING_FOR_OUTSTANDING_WRITES_SHIFT                   6
#define    DMA_CS2_WAITING_FOR_OUTSTANDING_WRITES_MASK                    0x00000040
#define    DMA_CS2_DREQ_STOPS_DMA_SHIFT                                   5
#define    DMA_CS2_DREQ_STOPS_DMA_MASK                                    0x00000020
#define    DMA_CS2_PAUSED_SHIFT                                           4
#define    DMA_CS2_PAUSED_MASK                                            0x00000010
#define    DMA_CS2_DREQ_SHIFT                                             3
#define    DMA_CS2_DREQ_MASK                                              0x00000008
#define    DMA_CS2_INT_SHIFT                                              2
#define    DMA_CS2_INT_MASK                                               0x00000004
#define    DMA_CS2_END_SHIFT                                              1
#define    DMA_CS2_END_MASK                                               0x00000002
#define    DMA_CS2_ACTIVE_SHIFT                                           0
#define    DMA_CS2_ACTIVE_MASK                                            0x00000001

#define DMA_CONBLK_AD2_OFFSET                                             0x00000204
#define DMA_CONBLK_AD2_TYPE                                               UInt32
#define DMA_CONBLK_AD2_RESERVED_MASK                                      0x00000000
#define    DMA_CONBLK_AD2_SCB_ADDR_SHIFT                                  0
#define    DMA_CONBLK_AD2_SCB_ADDR_MASK                                   0xFFFFFFFF

#define DMA_TI2_OFFSET                                                    0x00000208
#define DMA_TI2_TYPE                                                      UInt32
#define DMA_TI2_RESERVED_MASK                                             0xF8000004
#define    DMA_TI2_NO_WIDE_BURSTS_SHIFT                                   26
#define    DMA_TI2_NO_WIDE_BURSTS_MASK                                    0x04000000
#define    DMA_TI2_WAITS_SHIFT                                            21
#define    DMA_TI2_WAITS_MASK                                             0x03E00000
#define    DMA_TI2_PERMAP_SHIFT                                           16
#define    DMA_TI2_PERMAP_MASK                                            0x001F0000
#define    DMA_TI2_BURST_LENGTH_SHIFT                                     12
#define    DMA_TI2_BURST_LENGTH_MASK                                      0x0000F000
#define    DMA_TI2_SRC_IGNORE_SHIFT                                       11
#define    DMA_TI2_SRC_IGNORE_MASK                                        0x00000800
#define    DMA_TI2_SRC_DREQ_SHIFT                                         10
#define    DMA_TI2_SRC_DREQ_MASK                                          0x00000400
#define    DMA_TI2_SRC_WIDTH_SHIFT                                        9
#define    DMA_TI2_SRC_WIDTH_MASK                                         0x00000200
#define    DMA_TI2_SRC_INC_SHIFT                                          8
#define    DMA_TI2_SRC_INC_MASK                                           0x00000100
#define    DMA_TI2_DEST_IGNORE_SHIFT                                      7
#define    DMA_TI2_DEST_IGNORE_MASK                                       0x00000080
#define    DMA_TI2_DEST_DREQ_SHIFT                                        6
#define    DMA_TI2_DEST_DREQ_MASK                                         0x00000040
#define    DMA_TI2_DEST_WIDTH_SHIFT                                       5
#define    DMA_TI2_DEST_WIDTH_MASK                                        0x00000020
#define    DMA_TI2_DEST_INC_SHIFT                                         4
#define    DMA_TI2_DEST_INC_MASK                                          0x00000010
#define    DMA_TI2_WAIT_RESP_SHIFT                                        3
#define    DMA_TI2_WAIT_RESP_MASK                                         0x00000008
#define    DMA_TI2_TDMODE_SHIFT                                           1
#define    DMA_TI2_TDMODE_MASK                                            0x00000002
#define    DMA_TI2_INTEN_SHIFT                                            0
#define    DMA_TI2_INTEN_MASK                                             0x00000001

#define DMA_SOURCE_AD2_OFFSET                                             0x0000020C
#define DMA_SOURCE_AD2_TYPE                                               UInt32
#define DMA_SOURCE_AD2_RESERVED_MASK                                      0x00000000
#define    DMA_SOURCE_AD2_S_ADDR_SHIFT                                    0
#define    DMA_SOURCE_AD2_S_ADDR_MASK                                     0xFFFFFFFF

#define DMA_DEST_AD2_OFFSET                                               0x00000210
#define DMA_DEST_AD2_TYPE                                                 UInt32
#define DMA_DEST_AD2_RESERVED_MASK                                        0x00000000
#define    DMA_DEST_AD2_D_ADDR_SHIFT                                      0
#define    DMA_DEST_AD2_D_ADDR_MASK                                       0xFFFFFFFF

#define DMA_TXFR_LEN2_OFFSET                                              0x00000214
#define DMA_TXFR_LEN2_TYPE                                                UInt32
#define DMA_TXFR_LEN2_RESERVED_MASK                                       0xC0000000
#define    DMA_TXFR_LEN2_YLENGTH_SHIFT                                    16
#define    DMA_TXFR_LEN2_YLENGTH_MASK                                     0x3FFF0000
#define    DMA_TXFR_LEN2_XLENGTH_SHIFT                                    0
#define    DMA_TXFR_LEN2_XLENGTH_MASK                                     0x0000FFFF

#define DMA_STRIDE2_OFFSET                                                0x00000218
#define DMA_STRIDE2_TYPE                                                  UInt32
#define DMA_STRIDE2_RESERVED_MASK                                         0x00000000
#define    DMA_STRIDE2_D_STRIDE_SHIFT                                     16
#define    DMA_STRIDE2_D_STRIDE_MASK                                      0xFFFF0000
#define    DMA_STRIDE2_S_STRIDE_SHIFT                                     0
#define    DMA_STRIDE2_S_STRIDE_MASK                                      0x0000FFFF

#define DMA_NEXTCONBK2_OFFSET                                             0x0000021C
#define DMA_NEXTCONBK2_TYPE                                               UInt32
#define DMA_NEXTCONBK2_RESERVED_MASK                                      0x00000000
#define    DMA_NEXTCONBK2_NEXTCONBK_SHIFT                                 0
#define    DMA_NEXTCONBK2_NEXTCONBK_MASK                                  0xFFFFFFFF

#define DMA_DEBUG2_OFFSET                                                 0x00000220
#define DMA_DEBUG2_TYPE                                                   UInt32
#define DMA_DEBUG2_RESERVED_MASK                                          0xE0000008
#define    DMA_DEBUG2_LITE_SHIFT                                          28
#define    DMA_DEBUG2_LITE_MASK                                           0x10000000
#define    DMA_DEBUG2_VERSION_SHIFT                                       25
#define    DMA_DEBUG2_VERSION_MASK                                        0x0E000000
#define    DMA_DEBUG2_DMA_STATE_SHIFT                                     16
#define    DMA_DEBUG2_DMA_STATE_MASK                                      0x01FF0000
#define    DMA_DEBUG2_DMA_ID_SHIFT                                        8
#define    DMA_DEBUG2_DMA_ID_MASK                                         0x0000FF00
#define    DMA_DEBUG2_OUTSTANDING_WRITES_SHIFT                            4
#define    DMA_DEBUG2_OUTSTANDING_WRITES_MASK                             0x000000F0
#define    DMA_DEBUG2_READ_ERROR_SHIFT                                    2
#define    DMA_DEBUG2_READ_ERROR_MASK                                     0x00000004
#define    DMA_DEBUG2_FIFO_ERROR_SHIFT                                    1
#define    DMA_DEBUG2_FIFO_ERROR_MASK                                     0x00000002
#define    DMA_DEBUG2_READ_LAST_NOT_SET_ERROR_SHIFT                       0
#define    DMA_DEBUG2_READ_LAST_NOT_SET_ERROR_MASK                        0x00000001

#endif /* __BRCM_RDB_DMA_H__ */



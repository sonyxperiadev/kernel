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

#ifndef __BRCM_RDB_ESW_BUSIF_MTP_H__
#define __BRCM_RDB_ESW_BUSIF_MTP_H__

#define ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_OFFSET                   0x00000000
#define ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_TYPE                     UInt64
#define ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_RESERVED_MASK            0xFFFFFFFF0FFFFFC0
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_CHANNEL_ENABLE_SHIFT 31
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_CHANNEL_ENABLE_MASK 0x80000000
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_SPEED_CHANGE_REQ_SHIFT 30
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_SPEED_CHANGE_REQ_MASK 0x40000000
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_SPEED_CHANGE_DONE_SHIFT 29
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_SPEED_CHANGE_DONE_MASK 0x20000000
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_SPEED_MODE_SHIFT  28
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_SPEED_MODE_MASK   0x10000000
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_ENABLE_SINGLE_ACCESS_SHIFT 5
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_ENABLE_SINGLE_ACCESS_MASK 0x00000020
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_BURST_SIZE_SHIFT  1
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_BURST_SIZE_MASK   0x0000001E
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_INITIAL_OFFSET_ENABLE_SHIFT 0
#define    ESW_BUSIF_MTP_MTP_TRANSFER_CONFIGURATION_MTP_INITIAL_OFFSET_ENABLE_MASK 0x00000001

#define ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_OFFSET                  0x00000008
#define ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_TYPE                    UInt64
#define ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_RESERVED_MASK           0xFFFFFFFFFFFE0000
#define    ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_MTP_EOP_BIT_SHIFT    16
#define    ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_MTP_EOP_BIT_MASK     0x00010000
#define    ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_MTP_LAST_OFFSET_SHIFT 8
#define    ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_MTP_LAST_OFFSET_MASK 0x0000FF00
#define    ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_MTP_INIT_OFFSET_SHIFT 0
#define    ESW_BUSIF_MTP_MTP_BLOCK_DMA_CONFIGURATION_MTP_INIT_OFFSET_MASK 0x000000FF

#define ESW_BUSIF_MTP_MTP_DMA_STATUS_OFFSET                               0x00000010
#define ESW_BUSIF_MTP_MTP_DMA_STATUS_TYPE                                 UInt64
#define ESW_BUSIF_MTP_MTP_DMA_STATUS_RESERVED_MASK                        0xFFFFFFFFB001FFFF
#define    ESW_BUSIF_MTP_MTP_DMA_STATUS_FIRST_MTP_TRANS_SHIFT             30
#define    ESW_BUSIF_MTP_MTP_DMA_STATUS_FIRST_MTP_TRANS_MASK              0x40000000
#define    ESW_BUSIF_MTP_MTP_DMA_STATUS_MTP_BLOCK_TRANS_COUNT_SHIFT       17
#define    ESW_BUSIF_MTP_MTP_DMA_STATUS_MTP_BLOCK_TRANS_COUNT_MASK        0x0FFE0000

#define ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_OFFSET                       0x00000018
#define ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_TYPE                         UInt64
#define ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_RESERVED_MASK                0xFFFFFFFFFFF8000C
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_REQ_SHIFT         18
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_REQ_MASK          0x00040000
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_SINGLE_SHIFT      17
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_SINGLE_MASK       0x00020000
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_LAST_SHIFT        16
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_LAST_MASK         0x00010000
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_BLOCK_TRANS_COUNT_SHIFT 6
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_BLOCK_TRANS_COUNT_MASK 0x0000FFC0
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_FIFO_FULL_SHIFT       5
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_FIFO_FULL_MASK        0x00000020
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_FIFO_EMPTY_SHIFT      4
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_FIFO_EMPTY_MASK       0x00000010
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_FINISH_SHIFT      1
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_FINISH_MASK       0x00000002
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_ACK_SHIFT         0
#define    ESW_BUSIF_MTP_MTP_DMA_CONTROL_STATUS_MTP_DMA_ACK_MASK          0x00000001

#define ESW_BUSIF_MTP_MTP_IPG_SETTING_OFFSET                              0x00000020
#define ESW_BUSIF_MTP_MTP_IPG_SETTING_TYPE                                UInt64
#define ESW_BUSIF_MTP_MTP_IPG_SETTING_RESERVED_MASK                       0xFFFFFFFF00000000
#define    ESW_BUSIF_MTP_MTP_IPG_SETTING_MTP_IPG_SHIFT                    0
#define    ESW_BUSIF_MTP_MTP_IPG_SETTING_MTP_IPG_MASK                     0xFFFFFFFF

#endif /* __BRCM_RDB_ESW_BUSIF_MTP_H__ */



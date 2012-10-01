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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_SPUM_AXI_H__
#define __BRCM_RDB_SPUM_AXI_H__

#define SPUM_AXI_AXI_REACT_OFFSET                                         0x00000000
#define SPUM_AXI_AXI_REACT_TYPE                                           UInt32
#define SPUM_AXI_AXI_REACT_RESERVED_MASK                                  0xFFFFFFFC
#define    SPUM_AXI_AXI_REACT_REACTION_SHIFT                              0
#define    SPUM_AXI_AXI_REACT_REACTION_MASK                               0x00000003

#define SPUM_AXI_INT_STAT_OFFSET                                          0x00000004
#define SPUM_AXI_INT_STAT_TYPE                                            UInt32
#define SPUM_AXI_INT_STAT_RESERVED_MASK                                   0xFFFFFFF8
#define    SPUM_AXI_INT_STAT_PKT_ERR_INT_SHIFT                            2
#define    SPUM_AXI_INT_STAT_PKT_ERR_INT_MASK                             0x00000004
#define    SPUM_AXI_INT_STAT_KEY_CACHE_ERR_INT_SHIFT                      1
#define    SPUM_AXI_INT_STAT_KEY_CACHE_ERR_INT_MASK                       0x00000002
#define    SPUM_AXI_INT_STAT_ACCESS_VIO_INT_SHIFT                         0
#define    SPUM_AXI_INT_STAT_ACCESS_VIO_INT_MASK                          0x00000001

#define SPUM_AXI_INT_MASK_OFFSET                                          0x00000008
#define SPUM_AXI_INT_MASK_TYPE                                            UInt32
#define SPUM_AXI_INT_MASK_RESERVED_MASK                                   0xFFFFFFF8
#define    SPUM_AXI_INT_MASK_PKT_ERR_INT_MASK_SHIFT                       2
#define    SPUM_AXI_INT_MASK_PKT_ERR_INT_MASK_MASK                        0x00000004
#define    SPUM_AXI_INT_MASK_KEY_CACHE_ERRO_INT_MASK_SHIFT                1
#define    SPUM_AXI_INT_MASK_KEY_CACHE_ERRO_INT_MASK_MASK                 0x00000002
#define    SPUM_AXI_INT_MASK_ACCESS_VIO_INT_MASK_SHIFT                    0
#define    SPUM_AXI_INT_MASK_ACCESS_VIO_INT_MASK_MASK                     0x00000001

#define SPUM_AXI_VIO_ADDR_STAT_OFFSET                                     0x0000000C
#define SPUM_AXI_VIO_ADDR_STAT_TYPE                                       UInt32
#define SPUM_AXI_VIO_ADDR_STAT_RESERVED_MASK                              0x00000000
#define    SPUM_AXI_VIO_ADDR_STAT_VIO_ADDR_STATUS_SHIFT                   0
#define    SPUM_AXI_VIO_ADDR_STAT_VIO_ADDR_STATUS_MASK                    0xFFFFFFFF

#define SPUM_AXI_VIO_CTL_STAT_OFFSET                                      0x00000010
#define SPUM_AXI_VIO_CTL_STAT_TYPE                                        UInt32
#define SPUM_AXI_VIO_CTL_STAT_RESERVED_MASK                               0x3FFC0000
#define    SPUM_AXI_VIO_CTL_STAT_INT_OVERRUN_SHIFT                        31
#define    SPUM_AXI_VIO_CTL_STAT_INT_OVERRUN_MASK                         0x80000000
#define    SPUM_AXI_VIO_CTL_STAT_INT_SET_SHIFT                            30
#define    SPUM_AXI_VIO_CTL_STAT_INT_SET_MASK                             0x40000000
#define    SPUM_AXI_VIO_CTL_STAT_ID_SHIFT                                 2
#define    SPUM_AXI_VIO_CTL_STAT_ID_MASK                                  0x0003FFFC
#define    SPUM_AXI_VIO_CTL_STAT_SECURE_SHIFT                             1
#define    SPUM_AXI_VIO_CTL_STAT_SECURE_MASK                              0x00000002
#define    SPUM_AXI_VIO_CTL_STAT_RD_WR_SHIFT                              0
#define    SPUM_AXI_VIO_CTL_STAT_RD_WR_MASK                               0x00000001

#define SPUM_AXI_AXI_ACCESS_OFFSET                                        0x00000014
#define SPUM_AXI_AXI_ACCESS_TYPE                                          UInt32
#define SPUM_AXI_AXI_ACCESS_RESERVED_MASK                                 0xFFFFFFFC
#define    SPUM_AXI_AXI_ACCESS_ACCESS_SHIFT                               0
#define    SPUM_AXI_AXI_ACCESS_ACCESS_MASK                                0x00000003

#define SPUM_AXI_AXI_DBG0_OFFSET                                          0x00000018
#define SPUM_AXI_AXI_DBG0_TYPE                                            UInt32
#define SPUM_AXI_AXI_DBG0_RESERVED_MASK                                   0x00000000
#define    SPUM_AXI_AXI_DBG0_SPU_DEBUG_0_SHIFT                            0
#define    SPUM_AXI_AXI_DBG0_SPU_DEBUG_0_MASK                             0xFFFFFFFF

#define SPUM_AXI_AXI_DBG1_OFFSET                                          0x0000001C
#define SPUM_AXI_AXI_DBG1_TYPE                                            UInt32
#define SPUM_AXI_AXI_DBG1_RESERVED_MASK                                   0x00000000
#define    SPUM_AXI_AXI_DBG1_SPU_DEBUG_1_SHIFT                            0
#define    SPUM_AXI_AXI_DBG1_SPU_DEBUG_1_MASK                             0xFFFFFFFF

#define SPUM_AXI_DMA_CTL_OFFSET                                           0x00000020
#define SPUM_AXI_DMA_CTL_TYPE                                             UInt32
#define SPUM_AXI_DMA_CTL_RESERVED_MASK                                    0xFFFFFFFC
#define    SPUM_AXI_DMA_CTL_OUT_EXTFLOW_SHIFT                             1
#define    SPUM_AXI_DMA_CTL_OUT_EXTFLOW_MASK                              0x00000002
#define    SPUM_AXI_DMA_CTL_IN_EXTFLOW_SHIFT                              0
#define    SPUM_AXI_DMA_CTL_IN_EXTFLOW_MASK                               0x00000001

#define SPUM_AXI_DMA_STAT_OFFSET                                          0x00000024
#define SPUM_AXI_DMA_STAT_TYPE                                            UInt32
#define SPUM_AXI_DMA_STAT_RESERVED_MASK                                   0xFFFFFFF0
#define    SPUM_AXI_DMA_STAT_PKT_CLEAR_SHIFT                              3
#define    SPUM_AXI_DMA_STAT_PKT_CLEAR_MASK                               0x00000008
#define    SPUM_AXI_DMA_STAT_PKT_DONE_SHIFT                               2
#define    SPUM_AXI_DMA_STAT_PKT_DONE_MASK                                0x00000004
#define    SPUM_AXI_DMA_STAT_PKT_BUSY_SHIFT                               1
#define    SPUM_AXI_DMA_STAT_PKT_BUSY_MASK                                0x00000002
#define    SPUM_AXI_DMA_STAT_PKT_START_SHIFT                              0
#define    SPUM_AXI_DMA_STAT_PKT_START_MASK                               0x00000001

#define SPUM_AXI_IN_DMA_SIZE_OFFSET                                       0x00000028
#define SPUM_AXI_IN_DMA_SIZE_TYPE                                         UInt32
#define SPUM_AXI_IN_DMA_SIZE_RESERVED_MASK                                0x00000000
#define    SPUM_AXI_IN_DMA_SIZE_IFLOW_PKTSIZE_SHIFT                       0
#define    SPUM_AXI_IN_DMA_SIZE_IFLOW_PKTSIZE_MASK                        0xFFFFFFFF

#define SPUM_AXI_OUT_DMA_SIZE_OFFSET                                      0x0000002C
#define SPUM_AXI_OUT_DMA_SIZE_TYPE                                        UInt32
#define SPUM_AXI_OUT_DMA_SIZE_RESERVED_MASK                               0x00000000
#define    SPUM_AXI_OUT_DMA_SIZE_OFLOW_PKTSIZE_SHIFT                      0
#define    SPUM_AXI_OUT_DMA_SIZE_OFLOW_PKTSIZE_MASK                       0xFFFFFFFF

#define SPUM_AXI_FIFO_STAT_OFFSET                                         0x00000030
#define SPUM_AXI_FIFO_STAT_TYPE                                           UInt32
#define SPUM_AXI_FIFO_STAT_RESERVED_MASK                                  0xFF80FF80
#define    SPUM_AXI_FIFO_STAT_OFIFO_LVL_SHIFT                             17
#define    SPUM_AXI_FIFO_STAT_OFIFO_LVL_MASK                              0x007E0000
#define    SPUM_AXI_FIFO_STAT_OFIFO_RDY_SHIFT                             16
#define    SPUM_AXI_FIFO_STAT_OFIFO_RDY_MASK                              0x00010000
#define    SPUM_AXI_FIFO_STAT_IFIFO_SPACE_SHIFT                           1
#define    SPUM_AXI_FIFO_STAT_IFIFO_SPACE_MASK                            0x0000007E
#define    SPUM_AXI_FIFO_STAT_IFIFO_RDY_SHIFT                             0
#define    SPUM_AXI_FIFO_STAT_IFIFO_RDY_MASK                              0x00000001

#define SPUM_AXI_FIFO_IN_OFFSET                                           0x00000034
#define SPUM_AXI_FIFO_IN_TYPE                                             UInt32
#define SPUM_AXI_FIFO_IN_RESERVED_MASK                                    0x00000000
#define    SPUM_AXI_FIFO_IN_SPM_FIFO_IN_SHIFT                             0
#define    SPUM_AXI_FIFO_IN_SPM_FIFO_IN_MASK                              0xFFFFFFFF

#define SPUM_AXI_FIFO_OUT_OFFSET                                          0x00000038
#define SPUM_AXI_FIFO_OUT_TYPE                                            UInt32
#define SPUM_AXI_FIFO_OUT_RESERVED_MASK                                   0x00000000
#define    SPUM_AXI_FIFO_OUT_SPM_FIFO_OUT_SHIFT                           0
#define    SPUM_AXI_FIFO_OUT_SPM_FIFO_OUT_MASK                            0xFFFFFFFF

#endif /* __BRCM_RDB_SPUM_AXI_H__ */



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

#ifndef __BRCM_RDB_PMU_FIFO_H__
#define __BRCM_RDB_PMU_FIFO_H__

#define PMU_FIFO_FIFO_STS_OFFSET                                          0x00000C00
#define PMU_FIFO_FIFO_STS_TYPE                                            UInt32
#define PMU_FIFO_FIFO_STS_RESERVED_MASK                                   0xFFFFFFC0
#define    PMU_FIFO_FIFO_STS_FIFO_CNT_SHIFT                               2
#define    PMU_FIFO_FIFO_STS_FIFO_CNT_MASK                                0x0000003C
#define    PMU_FIFO_FIFO_STS_FIFO_FULL_SHIFT                              1
#define    PMU_FIFO_FIFO_STS_FIFO_FULL_MASK                               0x00000002
#define    PMU_FIFO_FIFO_STS_FIFO_EMPTY_SHIFT                             0
#define    PMU_FIFO_FIFO_STS_FIFO_EMPTY_MASK                              0x00000001

#define PMU_FIFO_FIFO_INT_EN_OFFSET                                       0x00000C04
#define PMU_FIFO_FIFO_INT_EN_TYPE                                         UInt32
#define PMU_FIFO_FIFO_INT_EN_RESERVED_MASK                                0xFFFFFFF0
#define    PMU_FIFO_FIFO_INT_EN_SW_OVR_WR_DONE_INT_EN_SHIFT               3
#define    PMU_FIFO_FIFO_INT_EN_SW_OVR_WR_DONE_INT_EN_MASK                0x00000008
#define    PMU_FIFO_FIFO_INT_EN_WR_DONE_INT_EN_SHIFT                      2
#define    PMU_FIFO_FIFO_INT_EN_WR_DONE_INT_EN_MASK                       0x00000004
#define    PMU_FIFO_FIFO_INT_EN_FIFO_FULL_INT_EN_SHIFT                    1
#define    PMU_FIFO_FIFO_INT_EN_FIFO_FULL_INT_EN_MASK                     0x00000002
#define    PMU_FIFO_FIFO_INT_EN_FIFO_EMPTY_INT_EN_SHIFT                   0
#define    PMU_FIFO_FIFO_INT_EN_FIFO_EMPTY_INT_EN_MASK                    0x00000001

#define PMU_FIFO_FIFO_STS_CLR_OFFSET                                      0x00000C08
#define PMU_FIFO_FIFO_STS_CLR_TYPE                                        UInt32
#define PMU_FIFO_FIFO_STS_CLR_RESERVED_MASK                               0xFFFFFFF0
#define    PMU_FIFO_FIFO_STS_CLR_SW_OVR_WR_DONE_STS_CLR_SHIFT             3
#define    PMU_FIFO_FIFO_STS_CLR_SW_OVR_WR_DONE_STS_CLR_MASK              0x00000008
#define    PMU_FIFO_FIFO_STS_CLR_WR_DONE_STS_CLR_SHIFT                    2
#define    PMU_FIFO_FIFO_STS_CLR_WR_DONE_STS_CLR_MASK                     0x00000004
#define    PMU_FIFO_FIFO_STS_CLR_FIFO_FULL_STS_CLR_SHIFT                  1
#define    PMU_FIFO_FIFO_STS_CLR_FIFO_FULL_STS_CLR_MASK                   0x00000002
#define    PMU_FIFO_FIFO_STS_CLR_FIFO_EMPTY_STS_CLR_SHIFT                 0
#define    PMU_FIFO_FIFO_STS_CLR_FIFO_EMPTY_STS_CLR_MASK                  0x00000001

#define PMU_FIFO_FIFO_INT_STS_OFFSET                                      0x00000C0C
#define PMU_FIFO_FIFO_INT_STS_TYPE                                        UInt32
#define PMU_FIFO_FIFO_INT_STS_RESERVED_MASK                               0xFFFFFFF0
#define    PMU_FIFO_FIFO_INT_STS_SW_OVR_WR_DONE_INT_STS_SHIFT             3
#define    PMU_FIFO_FIFO_INT_STS_SW_OVR_WR_DONE_INT_STS_MASK              0x00000008
#define    PMU_FIFO_FIFO_INT_STS_WR_DONE_INT_STS_SHIFT                    2
#define    PMU_FIFO_FIFO_INT_STS_WR_DONE_INT_STS_MASK                     0x00000004
#define    PMU_FIFO_FIFO_INT_STS_FIFO_FULL_INT_STS_SHIFT                  1
#define    PMU_FIFO_FIFO_INT_STS_FIFO_FULL_INT_STS_MASK                   0x00000002
#define    PMU_FIFO_FIFO_INT_STS_FIFO_EMPTY_INT_STS_SHIFT                 0
#define    PMU_FIFO_FIFO_INT_STS_FIFO_EMPTY_INT_STS_MASK                  0x00000001

#define PMU_FIFO_SW_OVR_CTRL_OFFSET                                       0x00000C10
#define PMU_FIFO_SW_OVR_CTRL_TYPE                                         UInt32
#define PMU_FIFO_SW_OVR_CTRL_RESERVED_MASK                                0xFFFFC000
#define    PMU_FIFO_SW_OVR_CTRL_SW_OVR_ADDR_SHIFT                         4
#define    PMU_FIFO_SW_OVR_CTRL_SW_OVR_ADDR_MASK                          0x00003FF0
#define    PMU_FIFO_SW_OVR_CTRL_RESEREVED_SW_OVR_EN_SHIFT                 1
#define    PMU_FIFO_SW_OVR_CTRL_RESEREVED_SW_OVR_EN_MASK                  0x0000000E
#define    PMU_FIFO_SW_OVR_CTRL_SW_OVR_EN_SHIFT                           0
#define    PMU_FIFO_SW_OVR_CTRL_SW_OVR_EN_MASK                            0x00000001

#define PMU_FIFO_SW_OVR_DATA_OFFSET                                       0x00000C14
#define PMU_FIFO_SW_OVR_DATA_TYPE                                         UInt32
#define PMU_FIFO_SW_OVR_DATA_RESERVED_MASK                                0x00000000
#define    PMU_FIFO_SW_OVR_DATA_SW_OVR_DATA_SHIFT                         0
#define    PMU_FIFO_SW_OVR_DATA_SW_OVR_DATA_MASK                          0xFFFFFFFF

#define PMU_FIFO_PMU_REN_CNT_REG_OFFSET                                   0x00000C18
#define PMU_FIFO_PMU_REN_CNT_REG_TYPE                                     UInt32
#define PMU_FIFO_PMU_REN_CNT_REG_RESERVED_MASK                            0xFFFFFC00
#define    PMU_FIFO_PMU_REN_CNT_REG_PMU_REN_LATCH_CNT_SHIFT               5
#define    PMU_FIFO_PMU_REN_CNT_REG_PMU_REN_LATCH_CNT_MASK                0x000003E0
#define    PMU_FIFO_PMU_REN_CNT_REG_PMU_REN_CNT_SHIFT                     0
#define    PMU_FIFO_PMU_REN_CNT_REG_PMU_REN_CNT_MASK                      0x0000001F

#endif /* __BRCM_RDB_PMU_FIFO_H__ */



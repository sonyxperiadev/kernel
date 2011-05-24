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

#ifndef __BRCM_RDB_KEK_APB_H__
#define __BRCM_RDB_KEK_APB_H__

#define KEK_APB_KEK_CTRL_OFFSET                                           0x00000000
#define KEK_APB_KEK_CTRL_TYPE                                             UInt32
#define KEK_APB_KEK_CTRL_RESERVED_MASK                                    0xFFFFFFFB
#define    KEK_APB_KEK_CTRL_START_SHIFT                                   2
#define    KEK_APB_KEK_CTRL_START_MASK                                    0x00000004

#define KEK_APB_ROOT_KEY_TIMER_OFFSET                                     0x00000004
#define KEK_APB_ROOT_KEY_TIMER_TYPE                                       UInt32
#define KEK_APB_ROOT_KEY_TIMER_RESERVED_MASK                              0x00000000
#define    KEK_APB_ROOT_KEY_TIMER_TIME_OUT_SHIFT                          0
#define    KEK_APB_ROOT_KEY_TIMER_TIME_OUT_MASK                           0xFFFFFFFF

#define KEK_APB_PKT_DETAILS_OFFSET                                        0x00000008
#define KEK_APB_PKT_DETAILS_TYPE                                          UInt32
#define KEK_APB_PKT_DETAILS_RESERVED_MASK                                 0xFFFFFC00
#define    KEK_APB_PKT_DETAILS_SZ_INPKT_SHIFT                             0
#define    KEK_APB_PKT_DETAILS_SZ_INPKT_MASK                              0x000003FF

#define KEK_APB_IN_FIFO_STS_OFFSET                                        0x0000000C
#define KEK_APB_IN_FIFO_STS_TYPE                                          UInt32
#define KEK_APB_IN_FIFO_STS_RESERVED_MASK                                 0xFFFFF000
#define    KEK_APB_IN_FIFO_STS_SPC_AVAIL_SHIFT                            2
#define    KEK_APB_IN_FIFO_STS_SPC_AVAIL_MASK                             0x00000FFC
#define    KEK_APB_IN_FIFO_STS_IN_BUSY_SHIFT                              1
#define    KEK_APB_IN_FIFO_STS_IN_BUSY_MASK                               0x00000002
#define    KEK_APB_IN_FIFO_STS_FIFO_FULL_SHIFT                            0
#define    KEK_APB_IN_FIFO_STS_FIFO_FULL_MASK                             0x00000001

#define KEK_APB_RES_FIFO_STS_OFFSET                                       0x00000010
#define KEK_APB_RES_FIFO_STS_TYPE                                         UInt32
#define KEK_APB_RES_FIFO_STS_RESERVED_MASK                                0xFFF00000
#define    KEK_APB_RES_FIFO_STS_RESULT_SZ_SHIFT                           10
#define    KEK_APB_RES_FIFO_STS_RESULT_SZ_MASK                            0x000FFC00
#define    KEK_APB_RES_FIFO_STS_DATA_AVAIL_SHIFT                          2
#define    KEK_APB_RES_FIFO_STS_DATA_AVAIL_MASK                           0x000003FC
#define    KEK_APB_RES_FIFO_STS_OUT_BUSY_SHIFT                            1
#define    KEK_APB_RES_FIFO_STS_OUT_BUSY_MASK                             0x00000002
#define    KEK_APB_RES_FIFO_STS_FIFO_EMPTY_SHIFT                          0
#define    KEK_APB_RES_FIFO_STS_FIFO_EMPTY_MASK                           0x00000001

#define KEK_APB_INT_STATUS_OFFSET                                         0x00000014
#define KEK_APB_INT_STATUS_TYPE                                           UInt32
#define KEK_APB_INT_STATUS_RESERVED_MASK                                  0xFFFFFE00
#define    KEK_APB_INT_STATUS_KEK_UNWRAP_CORR_SHIFT                       8
#define    KEK_APB_INT_STATUS_KEK_UNWRAP_CORR_MASK                        0x00000100
#define    KEK_APB_INT_STATUS_KEK_DONE_SHIFT                              7
#define    KEK_APB_INT_STATUS_KEK_DONE_MASK                               0x00000080
#define    KEK_APB_INT_STATUS_FL_CTRL_RES_SHIFT                           6
#define    KEK_APB_INT_STATUS_FL_CTRL_RES_MASK                            0x00000040
#define    KEK_APB_INT_STATUS_FL_CTRL_IN_SHIFT                            5
#define    KEK_APB_INT_STATUS_FL_CTRL_IN_MASK                             0x00000020
#define    KEK_APB_INT_STATUS_APB_GNT_SHIFT                               4
#define    KEK_APB_INT_STATUS_APB_GNT_MASK                                0x00000010
#define    KEK_APB_INT_STATUS_CLIENT_SHIFT                                1
#define    KEK_APB_INT_STATUS_CLIENT_MASK                                 0x0000000E
#define    KEK_APB_INT_STATUS_ERR_INT_SHIFT                               0
#define    KEK_APB_INT_STATUS_ERR_INT_MASK                                0x00000001

#define KEK_APB_INT_EN_OFFSET                                             0x00000018
#define KEK_APB_INT_EN_TYPE                                               UInt32
#define KEK_APB_INT_EN_RESERVED_MASK                                      0xFFFFFC0E
#define    KEK_APB_INT_EN_KEK_DONE_CL2_SHIFT                              9
#define    KEK_APB_INT_EN_KEK_DONE_CL2_MASK                               0x00000200
#define    KEK_APB_INT_EN_KEK_DONE_CL1_SHIFT                              8
#define    KEK_APB_INT_EN_KEK_DONE_CL1_MASK                               0x00000100
#define    KEK_APB_INT_EN_KEK_DONE_CL0_SHIFT                              7
#define    KEK_APB_INT_EN_KEK_DONE_CL0_MASK                               0x00000080
#define    KEK_APB_INT_EN_FL_CTRL_RES_EN_SHIFT                            6
#define    KEK_APB_INT_EN_FL_CTRL_RES_EN_MASK                             0x00000040
#define    KEK_APB_INT_EN_FL_CTRL_IN_EN_SHIFT                             5
#define    KEK_APB_INT_EN_FL_CTRL_IN_EN_MASK                              0x00000020
#define    KEK_APB_INT_EN_APB_GNT_EN_SHIFT                                4
#define    KEK_APB_INT_EN_APB_GNT_EN_MASK                                 0x00000010
#define    KEK_APB_INT_EN_ERR_INT_EN_SHIFT                                0
#define    KEK_APB_INT_EN_ERR_INT_EN_MASK                                 0x00000001

#define KEK_APB_ERR_STS_OFFSET                                            0x0000001C
#define KEK_APB_ERR_STS_TYPE                                              UInt32
#define KEK_APB_ERR_STS_RESERVED_MASK                                     0xFFFD00FF
#define    KEK_APB_ERR_STS_ERR_FLAG_SHIFT                                 17
#define    KEK_APB_ERR_STS_ERR_FLAG_MASK                                  0x00020000
#define    KEK_APB_ERR_STS_ERROR_CODE_SHIFT                               8
#define    KEK_APB_ERR_STS_ERROR_CODE_MASK                                0x0000FF00

#define KEK_APB_IN_FIFO_OFFSET                                            0x00000020
#define KEK_APB_IN_FIFO_TYPE                                              UInt32
#define KEK_APB_IN_FIFO_RESERVED_MASK                                     0x00000000
#define    KEK_APB_IN_FIFO_IN_DATA_SHIFT                                  0
#define    KEK_APB_IN_FIFO_IN_DATA_MASK                                   0xFFFFFFFF

#define KEK_APB_RESULT_FIFO_OFFSET                                        0x00000024
#define KEK_APB_RESULT_FIFO_TYPE                                          UInt32
#define KEK_APB_RESULT_FIFO_RESERVED_MASK                                 0x00000000
#define    KEK_APB_RESULT_FIFO_OUT_DATA_SHIFT                             0
#define    KEK_APB_RESULT_FIFO_OUT_DATA_MASK                              0xFFFFFFFF

#endif /* __BRCM_RDB_KEK_APB_H__ */



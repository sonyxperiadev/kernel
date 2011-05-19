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

#ifndef __BRCM_RDB_BBL_RTC_H__
#define __BRCM_RDB_BBL_RTC_H__

#define BBL_RTC_SET_DIV_OFFSET                                            0x00000000
#define BBL_RTC_SET_DIV_TYPE                                              UInt32
#define BBL_RTC_SET_DIV_RESERVED_MASK                                     0x00000000
#define    BBL_RTC_SET_DIV_BBL_SET_DIV_UNUSED_SHIFT                       15
#define    BBL_RTC_SET_DIV_BBL_SET_DIV_UNUSED_MASK                        0xFFFF8000
#define    BBL_RTC_SET_DIV_BBL_RTC_SET_DIV_SHIFT                          0
#define    BBL_RTC_SET_DIV_BBL_RTC_SET_DIV_MASK                           0x00007FFF

#define BBL_RTC_SEC_0_OFFSET                                              0x00000004
#define BBL_RTC_SEC_0_TYPE                                                UInt32
#define BBL_RTC_SEC_0_RESERVED_MASK                                       0x00000000
#define    BBL_RTC_SEC_0_BBL_RTC_SEC_0_SHIFT                              0
#define    BBL_RTC_SEC_0_BBL_RTC_SEC_0_MASK                               0xFFFFFFFF

#define BBL_RTC_CTRL_OFFSET                                               0x00000008
#define BBL_RTC_CTRL_TYPE                                                 UInt32
#define BBL_RTC_CTRL_RESERVED_MASK                                        0x00000000
#define    BBL_RTC_CTRL_BBL_RTC_STOP_UNUSED_SHIFT                         2
#define    BBL_RTC_CTRL_BBL_RTC_STOP_UNUSED_MASK                          0xFFFFFFFC
#define    BBL_RTC_CTRL_BBL_RTC_LOCK_SHIFT                                1
#define    BBL_RTC_CTRL_BBL_RTC_LOCK_MASK                                 0x00000002
#define    BBL_RTC_CTRL_BBL_RTC_STOP_SHIFT                                0
#define    BBL_RTC_CTRL_BBL_RTC_STOP_MASK                                 0x00000001

#define BBL_RTC_PER_OFFSET                                                0x0000000C
#define BBL_RTC_PER_TYPE                                                  UInt32
#define BBL_RTC_PER_RESERVED_MASK                                         0x00000000
#define    BBL_RTC_PER_BBL_PER_UNUSED_SHIFT                               12
#define    BBL_RTC_PER_BBL_PER_UNUSED_MASK                                0xFFFFF000
#define    BBL_RTC_PER_BBL_RTC_PER_SHIFT                                  0
#define    BBL_RTC_PER_BBL_RTC_PER_MASK                                   0x00000FFF

#define BBL_RTC_MATCH_OFFSET                                              0x00000010
#define BBL_RTC_MATCH_TYPE                                                UInt32
#define BBL_RTC_MATCH_RESERVED_MASK                                       0x00000000
#define    BBL_RTC_MATCH_BBL_MATCH_UNUSED_SHIFT                           16
#define    BBL_RTC_MATCH_BBL_MATCH_UNUSED_MASK                            0xFFFF0000
#define    BBL_RTC_MATCH_BBL_RTC_MATCH_SHIFT                              0
#define    BBL_RTC_MATCH_BBL_RTC_MATCH_MASK                               0x0000FFFF

#define BBL_RTC_CLR_INT_OFFSET                                            0x00000014
#define BBL_RTC_CLR_INT_TYPE                                              UInt32
#define BBL_RTC_CLR_INT_RESERVED_MASK                                     0x00000000
#define    BBL_RTC_CLR_INT_BBL_CLR_INT_UNUSED_SHIFT                       2
#define    BBL_RTC_CLR_INT_BBL_CLR_INT_UNUSED_MASK                        0xFFFFFFFC
#define    BBL_RTC_CLR_INT_BBL_RTC_CLR_INT_MATCH_SHIFT                    1
#define    BBL_RTC_CLR_INT_BBL_RTC_CLR_INT_MATCH_MASK                     0x00000002
#define    BBL_RTC_CLR_INT_BBL_RTC_CLR_INT_PER_SHIFT                      0
#define    BBL_RTC_CLR_INT_BBL_RTC_CLR_INT_PER_MASK                       0x00000001

#define BBL_RTC_INT_STS_OFFSET                                            0x00000018
#define BBL_RTC_INT_STS_TYPE                                              UInt32
#define BBL_RTC_INT_STS_RESERVED_MASK                                     0x00000000
#define    BBL_RTC_INT_STS_BBL_INT_MASK_UNUSED_SHIFT                      2
#define    BBL_RTC_INT_STS_BBL_INT_MASK_UNUSED_MASK                       0xFFFFFFFC
#define    BBL_RTC_INT_STS_BBL_RTC_INT_MATCH_SHIFT                        1
#define    BBL_RTC_INT_STS_BBL_RTC_INT_MATCH_MASK                         0x00000002
#define    BBL_RTC_INT_STS_BBL_RTC_INT_PER_SHIFT                          0
#define    BBL_RTC_INT_STS_BBL_RTC_INT_PER_MASK                           0x00000001

#define BBL_RTC_INT_ENABLE_OFFSET                                         0x0000001C
#define BBL_RTC_INT_ENABLE_TYPE                                           UInt32
#define BBL_RTC_INT_ENABLE_RESERVED_MASK                                  0x00000000
#define    BBL_RTC_INT_ENABLE_BBL_INT_ENABLE_UNUSED_SHIFT                 2
#define    BBL_RTC_INT_ENABLE_BBL_INT_ENABLE_UNUSED_MASK                  0xFFFFFFFC
#define    BBL_RTC_INT_ENABLE_BBL_RTC_MATCH_INTR_ENABLE_SHIFT             1
#define    BBL_RTC_INT_ENABLE_BBL_RTC_MATCH_INTR_ENABLE_MASK              0x00000002
#define    BBL_RTC_INT_ENABLE_BBL_RTC_PER_INTR_ENABLE_SHIFT               0
#define    BBL_RTC_INT_ENABLE_BBL_RTC_PER_INTR_ENABLE_MASK                0x00000001

#define BBL_RTC_RESET_ACCESS_OFFSET                                       0x00000020
#define BBL_RTC_RESET_ACCESS_TYPE                                         UInt32
#define BBL_RTC_RESET_ACCESS_RESERVED_MASK                                0x00000000
#define    BBL_RTC_RESET_ACCESS_BBL_RTC_RESET_ACCESS_STATUS_SHIFT         0
#define    BBL_RTC_RESET_ACCESS_BBL_RTC_RESET_ACCESS_STATUS_MASK          0xFFFFFFFF

#define BBL_RTC_MTC_LSB_OFFSET                                            0x00000024
#define BBL_RTC_MTC_LSB_TYPE                                              UInt32
#define BBL_RTC_MTC_LSB_RESERVED_MASK                                     0x00000000
#define    BBL_RTC_MTC_LSB_BBL_MT_CTR_LSB_SHIFT                           0
#define    BBL_RTC_MTC_LSB_BBL_MT_CTR_LSB_MASK                            0xFFFFFFFF

#define BBL_RTC_MTC_MSB_OFFSET                                            0x00000028
#define BBL_RTC_MTC_MSB_TYPE                                              UInt32
#define BBL_RTC_MTC_MSB_RESERVED_MASK                                     0x00000000
#define    BBL_RTC_MTC_MSB_BBL_MT_CTR_MSB_SHIFT                           0
#define    BBL_RTC_MTC_MSB_BBL_MT_CTR_MSB_MASK                            0xFFFFFFFF

#define BBL_RTC_MTC_CTRL_OFFSET                                           0x0000002C
#define BBL_RTC_MTC_CTRL_TYPE                                             UInt32
#define BBL_RTC_MTC_CTRL_RESERVED_MASK                                    0x00000000
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_AMT_UNUSED_SHIFT              24
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_AMT_UNUSED_MASK               0xFF000000
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_AMT_SHIFT                     8
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_AMT_MASK                      0x00FFFF00
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_UNUSED_SHIFT                       2
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_UNUSED_MASK                        0x000000FC
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_SHIFT                         1
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_INCR_MASK                          0x00000002
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_LOCK_SHIFT                         0
#define    BBL_RTC_MTC_CTRL_BBL_MT_CTR_LOCK_MASK                          0x00000001

#endif /* __BRCM_RDB_BBL_RTC_H__ */



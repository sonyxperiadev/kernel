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

#ifndef __BRCM_RDB_SLEEPTIMER3G_H__
#define __BRCM_RDB_SLEEPTIMER3G_H__

#define SLEEPTIMER3G_SLEEP_CNT_OFFSET                                     0x00000000
#define SLEEPTIMER3G_SLEEP_CNT_TYPE                                       UInt32
#define SLEEPTIMER3G_SLEEP_CNT_RESERVED_MASK                              0xFFFC0000
#define    SLEEPTIMER3G_SLEEP_CNT_SLEEP_PERIOD_SHIFT                      0
#define    SLEEPTIMER3G_SLEEP_CNT_SLEEP_PERIOD_MASK                       0x0003FFFF

#define SLEEPTIMER3G_PWD_CNT_OFFSET                                       0x00000004
#define SLEEPTIMER3G_PWD_CNT_TYPE                                         UInt32
#define SLEEPTIMER3G_PWD_CNT_RESERVED_MASK                                0xFFFC0000
#define    SLEEPTIMER3G_PWD_CNT_PWD_CNT_SHIFT                             0
#define    SLEEPTIMER3G_PWD_CNT_PWD_CNT_MASK                              0x0003FFFF

#define SLEEPTIMER3G_TIMER_RESET_OFFSET                                   0x00000008
#define SLEEPTIMER3G_TIMER_RESET_TYPE                                     UInt32
#define SLEEPTIMER3G_TIMER_RESET_RESERVED_MASK                            0xFFFFFFFE
#define    SLEEPTIMER3G_TIMER_RESET_TIMER_RESET_SHIFT                     0
#define    SLEEPTIMER3G_TIMER_RESET_TIMER_RESET_MASK                      0x00000001

#define SLEEPTIMER3G_STATUS_OFFSET                                        0x0000000C
#define SLEEPTIMER3G_STATUS_TYPE                                          UInt32
#define SLEEPTIMER3G_STATUS_RESERVED_MASK                                 0xFEE00000
#define    SLEEPTIMER3G_STATUS_PWD_OVERRIDE_VAL_SHIFT                     24
#define    SLEEPTIMER3G_STATUS_PWD_OVERRIDE_VAL_MASK                      0x01000000
#define    SLEEPTIMER3G_STATUS_PWD_OVERRIDE_SHIFT                         20
#define    SLEEPTIMER3G_STATUS_PWD_OVERRIDE_MASK                          0x00100000
#define    SLEEPTIMER3G_STATUS_STATUS_SHIFT                               0
#define    SLEEPTIMER3G_STATUS_STATUS_MASK                                0x000FFFFF

#define SLEEPTIMER3G_SFN_CTRL_OFFSET                                      0x00000010
#define SLEEPTIMER3G_SFN_CTRL_TYPE                                        UInt32
#define SLEEPTIMER3G_SFN_CTRL_RESERVED_MASK                               0x1FFFFFFE
#define    SLEEPTIMER3G_SFN_CTRL_SFN_STATE_DBG_SHIFT                      29
#define    SLEEPTIMER3G_SFN_CTRL_SFN_STATE_DBG_MASK                       0xE0000000
#define    SLEEPTIMER3G_SFN_CTRL_SFN_FREE_RUN_SHIFT                       0
#define    SLEEPTIMER3G_SFN_CTRL_SFN_FREE_RUN_MASK                        0x00000001

#define SLEEPTIMER3G_SFN_CNT_MAX_R1_OFFSET                                0x00000014
#define SLEEPTIMER3G_SFN_CNT_MAX_R1_TYPE                                  UInt32
#define SLEEPTIMER3G_SFN_CNT_MAX_R1_RESERVED_MASK                         0xFFFFF000
#define    SLEEPTIMER3G_SFN_CNT_MAX_R1_SFN_CNT_MAX_R1_SHIFT               0
#define    SLEEPTIMER3G_SFN_CNT_MAX_R1_SFN_CNT_MAX_R1_MASK                0x00000FFF

#define SLEEPTIMER3G_SFN_CNT_LOAD_OFFSET                                  0x00000018
#define SLEEPTIMER3G_SFN_CNT_LOAD_TYPE                                    UInt32
#define SLEEPTIMER3G_SFN_CNT_LOAD_RESERVED_MASK                           0xFFFFF000
#define    SLEEPTIMER3G_SFN_CNT_LOAD_SFN_CNT_LOAD_VAL_SHIFT               0
#define    SLEEPTIMER3G_SFN_CNT_LOAD_SFN_CNT_LOAD_VAL_MASK                0x00000FFF

#define SLEEPTIMER3G_SFN_ACC_INIT_VAL_OFFSET                              0x0000001C
#define SLEEPTIMER3G_SFN_ACC_INIT_VAL_TYPE                                UInt32
#define SLEEPTIMER3G_SFN_ACC_INIT_VAL_RESERVED_MASK                       0xFFFFFE00
#define    SLEEPTIMER3G_SFN_ACC_INIT_VAL_SFN_ACC_INIT_VAL_SHIFT           0
#define    SLEEPTIMER3G_SFN_ACC_INIT_VAL_SFN_ACC_INIT_VAL_MASK            0x000001FF

#define SLEEPTIMER3G_SFN_ACC_Y_R1_OFFSET                                  0x00000020
#define SLEEPTIMER3G_SFN_ACC_Y_R1_TYPE                                    UInt32
#define SLEEPTIMER3G_SFN_ACC_Y_R1_RESERVED_MASK                           0xFFFC0000
#define    SLEEPTIMER3G_SFN_ACC_Y_R1_SFN_ACC_Y_R1_SHIFT                   0
#define    SLEEPTIMER3G_SFN_ACC_Y_R1_SFN_ACC_Y_R1_MASK                    0x0003FFFF

#define SLEEPTIMER3G_SFN_CNT_OFFSET                                       0x00000024
#define SLEEPTIMER3G_SFN_CNT_TYPE                                         UInt32
#define SLEEPTIMER3G_SFN_CNT_RESERVED_MASK                                0xFFFFF000
#define    SLEEPTIMER3G_SFN_CNT_SFN_CNT_SHIFT                             0
#define    SLEEPTIMER3G_SFN_CNT_SFN_CNT_MASK                              0x00000FFF

#define SLEEPTIMER3G_SFN_STOP_OFFSET                                      0x00000028
#define SLEEPTIMER3G_SFN_STOP_TYPE                                        UInt32
#define SLEEPTIMER3G_SFN_STOP_RESERVED_MASK                               0xFFFFFFFE
#define    SLEEPTIMER3G_SFN_STOP_SFN_STOP_SHIFT                           0
#define    SLEEPTIMER3G_SFN_STOP_SFN_STOP_MASK                            0x00000001

#define SLEEPTIMER3G_SFN_START_OFFSET                                     0x0000002C
#define SLEEPTIMER3G_SFN_START_TYPE                                       UInt32
#define SLEEPTIMER3G_SFN_START_RESERVED_MASK                              0xFFFFFFFE
#define    SLEEPTIMER3G_SFN_START_SFN_START_SHIFT                         0
#define    SLEEPTIMER3G_SFN_START_SFN_START_MASK                          0x00000001

#endif /* __BRCM_RDB_SLEEPTIMER3G_H__ */



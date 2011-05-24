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

#ifndef __BRCM_RDB_PTMR_WD_H__
#define __BRCM_RDB_PTMR_WD_H__

#define PTMR_WD_TIMER_LOAD_OFFSET                                         0x00000000
#define PTMR_WD_TIMER_LOAD_TYPE                                           UInt32
#define PTMR_WD_TIMER_LOAD_RESERVED_MASK                                  0x00000000
#define    PTMR_WD_TIMER_LOAD_LOAD_SHIFT                                  0
#define    PTMR_WD_TIMER_LOAD_LOAD_MASK                                   0xFFFFFFFF

#define PTMR_WD_TIMER_COUNTER_OFFSET                                      0x00000004
#define PTMR_WD_TIMER_COUNTER_TYPE                                        UInt32
#define PTMR_WD_TIMER_COUNTER_RESERVED_MASK                               0x00000000
#define    PTMR_WD_TIMER_COUNTER_COUNTER_SHIFT                            0
#define    PTMR_WD_TIMER_COUNTER_COUNTER_MASK                             0xFFFFFFFF

#define PTMR_WD_TIMER_CTRL_OFFSET                                         0x00000008
#define PTMR_WD_TIMER_CTRL_TYPE                                           UInt32
#define PTMR_WD_TIMER_CTRL_RESERVED_MASK                                  0xFFFF00F8
#define    PTMR_WD_TIMER_CTRL_PRESCALER_SHIFT                             8
#define    PTMR_WD_TIMER_CTRL_PRESCALER_MASK                              0x0000FF00
#define    PTMR_WD_TIMER_CTRL_IRQ_EN_SHIFT                                2
#define    PTMR_WD_TIMER_CTRL_IRQ_EN_MASK                                 0x00000004
#define    PTMR_WD_TIMER_CTRL_AUTO_RELOAD_EN_SHIFT                        1
#define    PTMR_WD_TIMER_CTRL_AUTO_RELOAD_EN_MASK                         0x00000002
#define    PTMR_WD_TIMER_CTRL_TIMER_EN_SHIFT                              0
#define    PTMR_WD_TIMER_CTRL_TIMER_EN_MASK                               0x00000001

#define PTMR_WD_TIMER_STATUS_OFFSET                                       0x0000000C
#define PTMR_WD_TIMER_STATUS_TYPE                                         UInt32
#define PTMR_WD_TIMER_STATUS_RESERVED_MASK                                0xFFFFFFFE
#define    PTMR_WD_TIMER_STATUS_EVENT_SHIFT                               0
#define    PTMR_WD_TIMER_STATUS_EVENT_MASK                                0x00000001

#define PTMR_WD_WATCHDOG_LOAD_OFFSET                                      0x00000020
#define PTMR_WD_WATCHDOG_LOAD_TYPE                                        UInt32
#define PTMR_WD_WATCHDOG_LOAD_RESERVED_MASK                               0x00000000
#define    PTMR_WD_WATCHDOG_LOAD_LOAD_W_SHIFT                             0
#define    PTMR_WD_WATCHDOG_LOAD_LOAD_W_MASK                              0xFFFFFFFF

#define PTMR_WD_WATCHDOG_COUNTER_OFFSET                                   0x00000024
#define PTMR_WD_WATCHDOG_COUNTER_TYPE                                     UInt32
#define PTMR_WD_WATCHDOG_COUNTER_RESERVED_MASK                            0x00000000
#define    PTMR_WD_WATCHDOG_COUNTER_COUNTER_W_SHIFT                       0
#define    PTMR_WD_WATCHDOG_COUNTER_COUNTER_W_MASK                        0xFFFFFFFF

#define PTMR_WD_WATCHDOG_CTRL_OFFSET                                      0x00000028
#define PTMR_WD_WATCHDOG_CTRL_TYPE                                        UInt32
#define PTMR_WD_WATCHDOG_CTRL_RESERVED_MASK                               0xFFFF00F0
#define    PTMR_WD_WATCHDOG_CTRL_PRESCALER_W_SHIFT                        8
#define    PTMR_WD_WATCHDOG_CTRL_PRESCALER_W_MASK                         0x0000FF00
#define    PTMR_WD_WATCHDOG_CTRL_WD_MODE_SHIFT                            3
#define    PTMR_WD_WATCHDOG_CTRL_WD_MODE_MASK                             0x00000008
#define    PTMR_WD_WATCHDOG_CTRL_IT_EN_SHIFT                              2
#define    PTMR_WD_WATCHDOG_CTRL_IT_EN_MASK                               0x00000004
#define    PTMR_WD_WATCHDOG_CTRL_AUTORELOAD_EN_SHIFT                      1
#define    PTMR_WD_WATCHDOG_CTRL_AUTORELOAD_EN_MASK                       0x00000002
#define    PTMR_WD_WATCHDOG_CTRL_WATCHDOG_EN_SHIFT                        0
#define    PTMR_WD_WATCHDOG_CTRL_WATCHDOG_EN_MASK                         0x00000001

#define PTMR_WD_WATCHDOG_STATUS_OFFSET                                    0x0000002C
#define PTMR_WD_WATCHDOG_STATUS_TYPE                                      UInt32
#define PTMR_WD_WATCHDOG_STATUS_RESERVED_MASK                             0xFFFFFFFE
#define    PTMR_WD_WATCHDOG_STATUS_EVENT_W_SHIFT                          0
#define    PTMR_WD_WATCHDOG_STATUS_EVENT_W_MASK                           0x00000001

#define PTMR_WD_WATCHDOG_RESET_STATUS_OFFSET                              0x00000030
#define PTMR_WD_WATCHDOG_RESET_STATUS_TYPE                                UInt32
#define PTMR_WD_WATCHDOG_RESET_STATUS_RESERVED_MASK                       0xFFFFFFFE
#define    PTMR_WD_WATCHDOG_RESET_STATUS_RESET_FLAG_SHIFT                 0
#define    PTMR_WD_WATCHDOG_RESET_STATUS_RESET_FLAG_MASK                  0x00000001

#define PTMR_WD_WATCHDOG_DISABLE_OFFSET                                   0x00000034
#define PTMR_WD_WATCHDOG_DISABLE_TYPE                                     UInt32
#define PTMR_WD_WATCHDOG_DISABLE_RESERVED_MASK                            0x00000000
#define    PTMR_WD_WATCHDOG_DISABLE_DISABLE_SHIFT                         0
#define    PTMR_WD_WATCHDOG_DISABLE_DISABLE_MASK                          0xFFFFFFFF

#endif /* __BRCM_RDB_PTMR_WD_H__ */



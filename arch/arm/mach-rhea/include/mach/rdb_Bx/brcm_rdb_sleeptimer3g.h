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

#endif /* __BRCM_RDB_SLEEPTIMER3G_H__ */



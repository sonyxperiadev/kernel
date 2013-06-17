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

#ifndef __BRCM_RDB_GLBTMR_H__
#define __BRCM_RDB_GLBTMR_H__

#define GLBTMR_GLOB_LOW_OFFSET                                            0x00000000
#define GLBTMR_GLOB_LOW_TYPE                                              UInt32
#define GLBTMR_GLOB_LOW_RESERVED_MASK                                     0x00000000
#define    GLBTMR_GLOB_LOW_LOWER_COUNTER_SHIFT                            0
#define    GLBTMR_GLOB_LOW_LOWER_COUNTER_MASK                             0xFFFFFFFF

#define GLBTMR_GLOB_HI_OFFSET                                             0x00000004
#define GLBTMR_GLOB_HI_TYPE                                               UInt32
#define GLBTMR_GLOB_HI_RESERVED_MASK                                      0x00000000
#define    GLBTMR_GLOB_HI_HIGHER_COUNTER_SHIFT                            0
#define    GLBTMR_GLOB_HI_HIGHER_COUNTER_MASK                             0xFFFFFFFF

#define GLBTMR_GLOB_CTRL_OFFSET                                           0x00000008
#define GLBTMR_GLOB_CTRL_TYPE                                             UInt32
#define GLBTMR_GLOB_CTRL_RESERVED_MASK                                    0xFFFF00F0
#define    GLBTMR_GLOB_CTRL_PRESCALER_G_SHIFT                             8
#define    GLBTMR_GLOB_CTRL_PRESCALER_G_MASK                              0x0000FF00
#define    GLBTMR_GLOB_CTRL_AUTOINCR_EN_G_SHIFT                           3
#define    GLBTMR_GLOB_CTRL_AUTOINCR_EN_G_MASK                            0x00000008
#define    GLBTMR_GLOB_CTRL_IRQ_EN_G_SHIFT                                2
#define    GLBTMR_GLOB_CTRL_IRQ_EN_G_MASK                                 0x00000004
#define    GLBTMR_GLOB_CTRL_COMP_EN_G_SHIFT                               1
#define    GLBTMR_GLOB_CTRL_COMP_EN_G_MASK                                0x00000002
#define    GLBTMR_GLOB_CTRL_TIMER_EN_G_SHIFT                              0
#define    GLBTMR_GLOB_CTRL_TIMER_EN_G_MASK                               0x00000001

#define GLBTMR_GLOB_STATUS_OFFSET                                         0x0000000C
#define GLBTMR_GLOB_STATUS_TYPE                                           UInt32
#define GLBTMR_GLOB_STATUS_RESERVED_MASK                                  0xFFFFFFFE
#define    GLBTMR_GLOB_STATUS_EVENT_G_SHIFT                               0
#define    GLBTMR_GLOB_STATUS_EVENT_G_MASK                                0x00000001

#define GLBTMR_GLOB_COMP_LOW_OFFSET                                       0x00000010
#define GLBTMR_GLOB_COMP_LOW_TYPE                                         UInt32
#define GLBTMR_GLOB_COMP_LOW_RESERVED_MASK                                0x00000000
#define    GLBTMR_GLOB_COMP_LOW_LOWER_COMPARE_G_SHIFT                     0
#define    GLBTMR_GLOB_COMP_LOW_LOWER_COMPARE_G_MASK                      0xFFFFFFFF

#define GLBTMR_GLOB_COMP_HI_OFFSET                                        0x00000014
#define GLBTMR_GLOB_COMP_HI_TYPE                                          UInt32
#define GLBTMR_GLOB_COMP_HI_RESERVED_MASK                                 0x00000000
#define    GLBTMR_GLOB_COMP_HI_HIGHER_COMPARE_G_SHIFT                     0
#define    GLBTMR_GLOB_COMP_HI_HIGHER_COMPARE_G_MASK                      0xFFFFFFFF

#define GLBTMR_GLOB_INCR_OFFSET                                           0x00000018
#define GLBTMR_GLOB_INCR_TYPE                                             UInt32
#define GLBTMR_GLOB_INCR_RESERVED_MASK                                    0x00000000
#define    GLBTMR_GLOB_INCR_AUTOINCR_G_SHIFT                              0
#define    GLBTMR_GLOB_INCR_AUTOINCR_G_MASK                               0xFFFFFFFF

#endif /* __BRCM_RDB_GLBTMR_H__ */



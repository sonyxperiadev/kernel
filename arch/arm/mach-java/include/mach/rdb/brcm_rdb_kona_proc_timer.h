/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_KONA_PROC_TIMER_H__
#define __BRCM_RDB_KONA_PROC_TIMER_H__

#define KONA_PROC_TIMER_CTRL_OFFSET                                       0x00000000
#define KONA_PROC_TIMER_CTRL_TYPE                                         UInt32
#define KONA_PROC_TIMER_CTRL_RESERVED_MASK                                0xFFFFFFFC
#define    KONA_PROC_TIMER_CTRL_SINGLE_CLK_MODE_SHIFT                     1
#define    KONA_PROC_TIMER_CTRL_SINGLE_CLK_MODE_MASK                      0x00000002
#define    KONA_PROC_TIMER_CTRL_STOP_SHIFT                                0
#define    KONA_PROC_TIMER_CTRL_STOP_MASK                                 0x00000001

#define KONA_PROC_TIMER_GET_L_OFFSET                                      0x00000004
#define KONA_PROC_TIMER_GET_L_TYPE                                        UInt32
#define KONA_PROC_TIMER_GET_L_RESERVED_MASK                               0x00000000
#define    KONA_PROC_TIMER_GET_L_GET_L_SHIFT                              0
#define    KONA_PROC_TIMER_GET_L_GET_L_MASK                               0xFFFFFFFF

#define KONA_PROC_TIMER_GET_H_OFFSET                                      0x00000008
#define KONA_PROC_TIMER_GET_H_TYPE                                        UInt32
#define KONA_PROC_TIMER_GET_H_RESERVED_MASK                               0x00000000
#define    KONA_PROC_TIMER_GET_H_GET_H_SHIFT                              0
#define    KONA_PROC_TIMER_GET_H_GET_H_MASK                               0xFFFFFFFF

#define KONA_PROC_TIMER_SET_L_OFFSET                                      0x0000000C
#define KONA_PROC_TIMER_SET_L_TYPE                                        UInt32
#define KONA_PROC_TIMER_SET_L_RESERVED_MASK                               0x00000000
#define    KONA_PROC_TIMER_SET_L_SET_L_SHIFT                              0
#define    KONA_PROC_TIMER_SET_L_SET_L_MASK                               0xFFFFFFFF

#define KONA_PROC_TIMER_SET_H_OFFSET                                      0x00000010
#define KONA_PROC_TIMER_SET_H_TYPE                                        UInt32
#define KONA_PROC_TIMER_SET_H_RESERVED_MASK                               0x00000000
#define    KONA_PROC_TIMER_SET_H_SET_H_SHIFT                              0
#define    KONA_PROC_TIMER_SET_H_SET_H_MASK                               0xFFFFFFFF

#define KONA_PROC_TIMER_DEC_OFFSET                                        0x00000014
#define KONA_PROC_TIMER_DEC_TYPE                                          UInt32
#define KONA_PROC_TIMER_DEC_RESERVED_MASK                                 0x00000000
#define    KONA_PROC_TIMER_DEC_DEC_SHIFT                                  0
#define    KONA_PROC_TIMER_DEC_DEC_MASK                                   0xFFFFFFFF

#define KONA_PROC_TIMER_INC_OFFSET                                        0x00000018
#define KONA_PROC_TIMER_INC_TYPE                                          UInt32
#define KONA_PROC_TIMER_INC_RESERVED_MASK                                 0x00000000
#define    KONA_PROC_TIMER_INC_INC_SHIFT                                  0
#define    KONA_PROC_TIMER_INC_INC_MASK                                   0xFFFFFFFF

#endif /* __BRCM_RDB_KONA_PROC_TIMER_H__ */



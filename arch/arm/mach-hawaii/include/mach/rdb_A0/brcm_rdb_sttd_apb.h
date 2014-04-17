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

#ifndef __BRCM_RDB_STTD_APB_H__
#define __BRCM_RDB_STTD_APB_H__

#define STTD_APB_STARTPOS_OFFSET                                          0x00000000
#define STTD_APB_STARTPOS_TYPE                                            UInt32
#define STTD_APB_STARTPOS_RESERVED_MASK                                   0x6EE0C000
#define    STTD_APB_STARTPOS_STTD_GO_SHIFT                                31
#define    STTD_APB_STARTPOS_STTD_GO_MASK                                 0x80000000
#define    STTD_APB_STARTPOS_DATA_VALID_SHIFT                             28
#define    STTD_APB_STARTPOS_DATA_VALID_MASK                              0x10000000
#define    STTD_APB_STARTPOS_SW_RESET_SHIFT                               24
#define    STTD_APB_STARTPOS_SW_RESET_MASK                                0x01000000
#define    STTD_APB_STARTPOS_DEBUG_MODE_SHIFT                             20
#define    STTD_APB_STARTPOS_DEBUG_MODE_MASK                              0x00100000
#define    STTD_APB_STARTPOS_START_SLOT_POS_SHIFT                         16
#define    STTD_APB_STARTPOS_START_SLOT_POS_MASK                          0x000F0000
#define    STTD_APB_STARTPOS_START_SAMPLE_POS_SHIFT                       0
#define    STTD_APB_STARTPOS_START_SAMPLE_POS_MASK                        0x00003FFF

#define STTD_APB_REGX_WAKE_OFFSET                                         0x00000004
#define STTD_APB_REGX_WAKE_TYPE                                           UInt32
#define STTD_APB_REGX_WAKE_RESERVED_MASK                                  0xFFFC0000
#define    STTD_APB_REGX_WAKE_REGX_WAKE_SHIFT                             0
#define    STTD_APB_REGX_WAKE_REGX_WAKE_MASK                              0x0003FFFF

#define STTD_APB_REGY_WAKE_OFFSET                                         0x00000008
#define STTD_APB_REGY_WAKE_TYPE                                           UInt32
#define STTD_APB_REGY_WAKE_RESERVED_MASK                                  0xFFFC0000
#define    STTD_APB_REGY_WAKE_REGY_WAKE_SHIFT                             0
#define    STTD_APB_REGY_WAKE_REGY_WAKE_MASK                              0x0003FFFF

#define STTD_APB_ENERGY_OFFSET                                            0x0000000C
#define STTD_APB_ENERGY_TYPE                                              UInt32
#define STTD_APB_ENERGY_RESERVED_MASK                                     0x00000000
#define    STTD_APB_ENERGY_IO_ENERGY_SHIFT                                16
#define    STTD_APB_ENERGY_IO_ENERGY_MASK                                 0xFFFF0000
#define    STTD_APB_ENERGY_ANT_ENERGY_SHIFT                               0
#define    STTD_APB_ENERGY_ANT_ENERGY_MASK                                0x0000FFFF

#endif /* __BRCM_RDB_STTD_APB_H__ */



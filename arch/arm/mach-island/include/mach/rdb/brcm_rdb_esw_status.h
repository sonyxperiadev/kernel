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

#ifndef __BRCM_RDB_ESW_STATUS_H__
#define __BRCM_RDB_ESW_STATUS_H__

#define ESW_STATUS_LINK_STATUS_SUMMARY_0_OFFSET                           0x00000000
#define ESW_STATUS_LINK_STATUS_SUMMARY_0_TYPE                             UInt16
#define ESW_STATUS_LINK_STATUS_SUMMARY_0_RESERVED_MASK                    0x0000FE00
#define    ESW_STATUS_LINK_STATUS_SUMMARY_0_LINK_STATUS_SHIFT             0
#define    ESW_STATUS_LINK_STATUS_SUMMARY_0_LINK_STATUS_MASK              0x000001FF

#define ESW_STATUS_LINK_STATUS_CHANGE_0_OFFSET                            0x00000010
#define ESW_STATUS_LINK_STATUS_CHANGE_0_TYPE                              UInt16
#define ESW_STATUS_LINK_STATUS_CHANGE_0_RESERVED_MASK                     0x0000FE00
#define    ESW_STATUS_LINK_STATUS_CHANGE_0_LINK_STATUS_CHANGE_SHIFT       0
#define    ESW_STATUS_LINK_STATUS_CHANGE_0_LINK_STATUS_CHANGE_MASK        0x000001FF

#define ESW_STATUS_SPEED_SUMMARY_0_OFFSET                                 0x00000020
#define ESW_STATUS_SPEED_SUMMARY_0_TYPE                                   UInt32
#define ESW_STATUS_SPEED_SUMMARY_0_RESERVED_MASK                          0xFFFC0000
#define    ESW_STATUS_SPEED_SUMMARY_0_PORT_SPEED_SHIFT                    0
#define    ESW_STATUS_SPEED_SUMMARY_0_PORT_SPEED_MASK                     0x0003FFFF

#define ESW_STATUS_DUPLEX_STATUS_SUMMARY_OFFSET                           0x00000040
#define ESW_STATUS_DUPLEX_STATUS_SUMMARY_TYPE                             UInt16
#define ESW_STATUS_DUPLEX_STATUS_SUMMARY_RESERVED_MASK                    0x0000FE00
#define    ESW_STATUS_DUPLEX_STATUS_SUMMARY_DUPLEX_STATUS_SUMMARY_SHIFT   0
#define    ESW_STATUS_DUPLEX_STATUS_SUMMARY_DUPLEX_STATUS_SUMMARY_MASK    0x000001FF

#define ESW_STATUS_PAUSE_STATUS_SUMMARY_0_OFFSET                          0x00000050
#define ESW_STATUS_PAUSE_STATUS_SUMMARY_0_TYPE                            UInt32
#define ESW_STATUS_PAUSE_STATUS_SUMMARY_0_RESERVED_MASK                   0xFFFC0000
#define    ESW_STATUS_PAUSE_STATUS_SUMMARY_0_PAUSE_STATE_SHIFT            0
#define    ESW_STATUS_PAUSE_STATUS_SUMMARY_0_PAUSE_STATE_MASK             0x0003FFFF

#endif /* __BRCM_RDB_ESW_STATUS_H__ */



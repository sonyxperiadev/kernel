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

#ifndef __BRCM_RDB_MEMCSPARE_H__
#define __BRCM_RDB_MEMCSPARE_H__

#define MEMCSPARE_MEMCSPARE_CTRL_OFFSET                                   0x00000000
#define MEMCSPARE_MEMCSPARE_CTRL_TYPE                                     UInt32
#define MEMCSPARE_MEMCSPARE_CTRL_RESERVED_MASK                            0x00FFFFFC
#define    MEMCSPARE_MEMCSPARE_CTRL_CFG_DBG_SEL_SHIFT                     29
#define    MEMCSPARE_MEMCSPARE_CTRL_CFG_DBG_SEL_MASK                      0xE0000000
#define    MEMCSPARE_MEMCSPARE_CTRL_CFG_MODE_SHIFT                        27
#define    MEMCSPARE_MEMCSPARE_CTRL_CFG_MODE_MASK                         0x18000000
#define    MEMCSPARE_MEMCSPARE_CTRL_MEMC_DEBUG_BUS_SEL_SHIFT              26
#define    MEMCSPARE_MEMCSPARE_CTRL_MEMC_DEBUG_BUS_SEL_MASK               0x04000000
#define    MEMCSPARE_MEMCSPARE_CTRL_SWITCH_DEBUG_BUS_SEL_SHIFT            25
#define    MEMCSPARE_MEMCSPARE_CTRL_SWITCH_DEBUG_BUS_SEL_MASK             0x02000000
#define    MEMCSPARE_MEMCSPARE_CTRL_ENABLE_APB_SWITCH_SHIFT               24
#define    MEMCSPARE_MEMCSPARE_CTRL_ENABLE_APB_SWITCH_MASK                0x01000000
#define    MEMCSPARE_MEMCSPARE_CTRL_TIMER_WIDTH_SHIFT                     1
#define    MEMCSPARE_MEMCSPARE_CTRL_TIMER_WIDTH_MASK                      0x00000002
#define    MEMCSPARE_MEMCSPARE_CTRL_FORCE_DISABLE_UPDATE_SHIFT            0
#define    MEMCSPARE_MEMCSPARE_CTRL_FORCE_DISABLE_UPDATE_MASK             0x00000001

#define MEMCSPARE_MEMCSPARE_STATUS_OFFSET                                 0x00000004
#define MEMCSPARE_MEMCSPARE_STATUS_TYPE                                   UInt32
#define MEMCSPARE_MEMCSPARE_STATUS_RESERVED_MASK                          0x00000000
#define    MEMCSPARE_MEMCSPARE_STATUS_O_DBG_BUS_SHIFT                     0
#define    MEMCSPARE_MEMCSPARE_STATUS_O_DBG_BUS_MASK                      0xFFFFFFFF

#endif /* __BRCM_RDB_MEMCSPARE_H__ */



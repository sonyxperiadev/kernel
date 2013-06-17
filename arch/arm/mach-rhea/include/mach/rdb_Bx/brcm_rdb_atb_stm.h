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

#ifndef __BRCM_RDB_ATB_STM_H__
#define __BRCM_RDB_ATB_STM_H__

#define ATB_STM_CONFIG_OFFSET                                             0x00000000
#define ATB_STM_CONFIG_TYPE                                               UInt32
#define ATB_STM_CONFIG_RESERVED_MASK                                      0xFFFFE080
#define    ATB_STM_CONFIG_TWOBIT_MODE_SHIFT                               12
#define    ATB_STM_CONFIG_TWOBIT_MODE_MASK                                0x00001000
#define    ATB_STM_CONFIG_BREAK_LIMIT_SHIFT                               10
#define    ATB_STM_CONFIG_BREAK_LIMIT_MASK                                0x00000C00
#define    ATB_STM_CONFIG_OUTPUT_MODE_SHIFT                               8
#define    ATB_STM_CONFIG_OUTPUT_MODE_MASK                                0x00000300
#define    ATB_STM_CONFIG_ATB_OUT_ID_SHIFT                                0
#define    ATB_STM_CONFIG_ATB_OUT_ID_MASK                                 0x0000007F

#define ATB_STM_EN_LO_OFFSET                                              0x00000010
#define ATB_STM_EN_LO_TYPE                                                UInt32
#define ATB_STM_EN_LO_RESERVED_MASK                                       0x00000000
#define    ATB_STM_EN_LO_MASTER_EN_LO_SHIFT                               0
#define    ATB_STM_EN_LO_MASTER_EN_LO_MASK                                0xFFFFFFFF

#define ATB_STM_EN_HI_OFFSET                                              0x00000014
#define ATB_STM_EN_HI_TYPE                                                UInt32
#define ATB_STM_EN_HI_RESERVED_MASK                                       0x00000000
#define    ATB_STM_EN_HI_MASTER_EN_HI_SHIFT                               0
#define    ATB_STM_EN_HI_MASTER_EN_HI_MASK                                0xFFFFFFFF

#define ATB_STM_SW_LO_OFFSET                                              0x00000018
#define ATB_STM_SW_LO_TYPE                                                UInt32
#define ATB_STM_SW_LO_RESERVED_MASK                                       0x00000000
#define    ATB_STM_SW_LO_MASTER_SW_LO_SHIFT                               0
#define    ATB_STM_SW_LO_MASTER_SW_LO_MASK                                0xFFFFFFFF

#define ATB_STM_SW_HI_OFFSET                                              0x0000001C
#define ATB_STM_SW_HI_TYPE                                                UInt32
#define ATB_STM_SW_HI_RESERVED_MASK                                       0x00000000
#define    ATB_STM_SW_HI_MASTER_SW_HI_SHIFT                               0
#define    ATB_STM_SW_HI_MASTER_SW_HI_MASK                                0xFFFFFFFF

#endif /* __BRCM_RDB_ATB_STM_H__ */



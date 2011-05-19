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

#ifndef __BRCM_RDB_TRUNKING_REGISTERS_H__
#define __BRCM_RDB_TRUNKING_REGISTERS_H__

#define TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_OFFSET           0x00000000
#define TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_TYPE             UInt8
#define TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_RESERVED_MASK    0x000000F4
#define    TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_ENABLE_MAC_BASED_TRUNKING_SHIFT 3
#define    TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_ENABLE_MAC_BASED_TRUNKING_MASK 0x00000008
#define    TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_MAC_BASED_TRUNKING_HASH_INDEX_SELECTION_SHIFT 0
#define    TRUNKING_REGISTERS_MAC_TRUNKING_CONTROL_REGISTER_MAC_BASED_TRUNKING_HASH_INDEX_SELECTION_MASK 0x00000003

#define TRUNKING_REGISTERS_TRUNKING_GROUP_N_REGISTER_OFFSET               0x00000080
#define TRUNKING_REGISTERS_TRUNKING_GROUP_N_REGISTER_TYPE                 UInt16
#define TRUNKING_REGISTERS_TRUNKING_GROUP_N_REGISTER_RESERVED_MASK        0x0000FE00
#define    TRUNKING_REGISTERS_TRUNKING_GROUP_N_REGISTER_TRUNK_GRP_N_SHIFT 0
#define    TRUNKING_REGISTERS_TRUNKING_GROUP_N_REGISTER_TRUNK_GRP_N_MASK  0x000001FF

#endif /* __BRCM_RDB_TRUNKING_REGISTERS_H__ */



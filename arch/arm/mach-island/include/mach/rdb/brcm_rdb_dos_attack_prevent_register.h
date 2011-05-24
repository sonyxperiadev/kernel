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

#ifndef __BRCM_RDB_DOS_ATTACK_PREVENT_REGISTER_H__
#define __BRCM_RDB_DOS_ATTACK_PREVENT_REGISTER_H__

#define DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_OFFSET           0x00000000
#define DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TYPE             UInt32
#define DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_RESERVED_MASK    0xFFFFC001
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV6_LONGPING_DROP_EN_SHIFT 13
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV6_LONGPING_DROP_EN_MASK 0x00002000
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV4_LONGPING_DROP_EN_SHIFT 12
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV4_LONGPING_DROP_EN_MASK 0x00001000
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV6_FRAGMENT_DROP_EN_SHIFT 11
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV6_FRAGMENT_DROP_EN_MASK 0x00000800
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV4_FRAGMENT_DROP_EN_SHIFT 10
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_ICMPV4_FRAGMENT_DROP_EN_MASK 0x00000400
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_FRAGERROR_DROP_EN_SHIFT 9
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_FRAGERROR_DROP_EN_MASK 0x00000200
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_SHORTHDR_DROP_EN_SHIFT 8
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_SHORTHDR_DROP_EN_MASK 0x00000100
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_SYNERROR_DROP_EN_SHIFT 7
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_SYNERROR_DROP_EN_MASK 0x00000080
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_SYNFINSCAN_DROP_EN_SHIFT 6
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_SYNFINSCAN_DROP_EN_MASK 0x00000040
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_XMASSCAN_DROP_EN_SHIFT 5
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_XMASSCAN_DROP_EN_MASK 0x00000020
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_NULLSCAN_DROP_EN_SHIFT 4
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_NULLSCAN_DROP_EN_MASK 0x00000010
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_UDP_BLAT_DROP_EN_SHIFT 3
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_UDP_BLAT_DROP_EN_MASK 0x00000008
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_BLAT_DROP_EN_SHIFT 2
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_TCP_BLAT_DROP_EN_MASK 0x00000004
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_IP_LAND_DROP_EN_SHIFT 1
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_CONTROL_REGISTER_IP_LAND_DROP_EN_MASK 0x00000002

#define DOS_ATTACK_PREVENT_REGISTER_MINIMUM_TCP_HEADER_SIZE_REGISTER_OFFSET 0x00000020
#define DOS_ATTACK_PREVENT_REGISTER_MINIMUM_TCP_HEADER_SIZE_REGISTER_TYPE UInt8
#define DOS_ATTACK_PREVENT_REGISTER_MINIMUM_TCP_HEADER_SIZE_REGISTER_RESERVED_MASK 0x00000000
#define    DOS_ATTACK_PREVENT_REGISTER_MINIMUM_TCP_HEADER_SIZE_REGISTER_MIN_TCP_HDR_SIZE_SHIFT 0
#define    DOS_ATTACK_PREVENT_REGISTER_MINIMUM_TCP_HEADER_SIZE_REGISTER_MIN_TCP_HDR_SIZE_MASK 0x000000FF

#define DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV4_SIZE_REGISTER_OFFSET   0x00000040
#define DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV4_SIZE_REGISTER_TYPE     UInt32
#define DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV4_SIZE_REGISTER_RESERVED_MASK 0x00000000
#define    DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV4_SIZE_REGISTER_MAX_ICMPV4_SIZE_SHIFT 0
#define    DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV4_SIZE_REGISTER_MAX_ICMPV4_SIZE_MASK 0xFFFFFFFF

#define DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV6_SIZE_REGISTER_OFFSET   0x00000060
#define DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV6_SIZE_REGISTER_TYPE     UInt32
#define DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV6_SIZE_REGISTER_RESERVED_MASK 0x00000000
#define    DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV6_SIZE_REGISTER_MAX_ICMPV6_SIZE_SHIFT 0
#define    DOS_ATTACK_PREVENT_REGISTER_MAXIMUM_ICMPV6_SIZE_REGISTER_MAX_ICMPV6_SIZE_MASK 0xFFFFFFFF

#define DOS_ATTACK_PREVENT_REGISTER_DOS_DISABLE_LEARN_REGISTER_OFFSET     0x00000080
#define DOS_ATTACK_PREVENT_REGISTER_DOS_DISABLE_LEARN_REGISTER_TYPE       UInt8
#define DOS_ATTACK_PREVENT_REGISTER_DOS_DISABLE_LEARN_REGISTER_RESERVED_MASK 0x000000FE
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_DISABLE_LEARN_REGISTER_DOS_DIS_LRN_SHIFT 0
#define    DOS_ATTACK_PREVENT_REGISTER_DOS_DISABLE_LEARN_REGISTER_DOS_DIS_LRN_MASK 0x00000001

#endif /* __BRCM_RDB_DOS_ATTACK_PREVENT_REGISTER_H__ */



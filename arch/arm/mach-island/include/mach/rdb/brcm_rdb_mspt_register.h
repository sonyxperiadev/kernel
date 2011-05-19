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

#ifndef __BRCM_RDB_MSPT_REGISTER_H__
#define __BRCM_RDB_MSPT_REGISTER_H__

#define MSPT_REGISTER_MSPT_CONTROL_REGISTER_OFFSET                        0x00000000
#define MSPT_REGISTER_MSPT_CONTROL_REGISTER_TYPE                          UInt8
#define MSPT_REGISTER_MSPT_CONTROL_REGISTER_RESERVED_MASK                 0x000000FE
#define    MSPT_REGISTER_MSPT_CONTROL_REGISTER_EN_802_1S_SHIFT            0
#define    MSPT_REGISTER_MSPT_CONTROL_REGISTER_EN_802_1S_MASK             0x00000001

#define MSPT_REGISTER_MSPT_AGING_CONTROL_REGISTER_OFFSET                  0x00000010
#define MSPT_REGISTER_MSPT_AGING_CONTROL_REGISTER_TYPE                    UInt32
#define MSPT_REGISTER_MSPT_AGING_CONTROL_REGISTER_RESERVED_MASK           0xFFFFFF00
#define    MSPT_REGISTER_MSPT_AGING_CONTROL_REGISTER_MSPT_AGE_MAP_SHIFT   0
#define    MSPT_REGISTER_MSPT_AGING_CONTROL_REGISTER_MSPT_AGE_MAP_MASK    0x000000FF

#define MSPT_REGISTER_MSPT_0_TABLE_REGISTER_OFFSET                        0x00000080
#define MSPT_REGISTER_MSPT_0_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_0_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_0_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_0_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_0_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_0_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_1_TABLE_REGISTER_OFFSET                        0x000000A0
#define MSPT_REGISTER_MSPT_1_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_1_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_1_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_1_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_1_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_1_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_2_TABLE_REGISTER_OFFSET                        0x000000C0
#define MSPT_REGISTER_MSPT_2_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_2_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_2_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_2_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_2_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_2_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_3_TABLE_REGISTER_OFFSET                        0x000000E0
#define MSPT_REGISTER_MSPT_3_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_3_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_3_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_3_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_3_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_3_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_4_TABLE_REGISTER_OFFSET                        0x00000100
#define MSPT_REGISTER_MSPT_4_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_4_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_4_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_4_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_4_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_4_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_5_TABLE_REGISTER_OFFSET                        0x00000120
#define MSPT_REGISTER_MSPT_5_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_5_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_5_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_5_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_5_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_5_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_6_TABLE_REGISTER_OFFSET                        0x00000140
#define MSPT_REGISTER_MSPT_6_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_6_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_6_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_6_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_6_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_6_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_MSPT_7_TABLE_REGISTER_OFFSET                        0x00000160
#define MSPT_REGISTER_MSPT_7_TABLE_REGISTER_TYPE                          UInt32
#define MSPT_REGISTER_MSPT_7_TABLE_REGISTER_RESERVED_MASK                 0xFFFFFFC0
#define    MSPT_REGISTER_MSPT_7_TABLE_REGISTER_SPT_STA1_SHIFT             3
#define    MSPT_REGISTER_MSPT_7_TABLE_REGISTER_SPT_STA1_MASK              0x00000038
#define    MSPT_REGISTER_MSPT_7_TABLE_REGISTER_SPT_STA0_SHIFT             0
#define    MSPT_REGISTER_MSPT_7_TABLE_REGISTER_SPT_STA0_MASK              0x00000007

#define MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_OFFSET 0x00000280
#define MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_TYPE  UInt16
#define MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_RESERVED_MASK 0x0000FFC0
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT5_BYPASS_SPT_SHIFT 5
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT5_BYPASS_SPT_MASK 0x00000020
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT4_BYPASS_SPT_SHIFT 4
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT4_BYPASS_SPT_MASK 0x00000010
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT3_BYPASS_SPT_SHIFT 3
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT3_BYPASS_SPT_MASK 0x00000008
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT2_BYPASS_SPT_SHIFT 2
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT2_BYPASS_SPT_MASK 0x00000004
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT1_BYPASS_SPT_SHIFT 1
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT1_BYPASS_SPT_MASK 0x00000002
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT0_BYPASS_SPT_SHIFT 0
#define    MSPT_REGISTER_SPT_MULTIPORT_ADDRESS_BYPASS_CONTROL_REGISTER_EN_MPORT0_BYPASS_SPT_MASK 0x00000001

#endif /* __BRCM_RDB_MSPT_REGISTER_H__ */



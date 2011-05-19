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

#ifndef __BRCM_RDB_ESW_ARL_VTBL_ACC_H__
#define __BRCM_RDB_ESW_ARL_VTBL_ACC_H__

#define ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_OFFSET                    0x00000000
#define ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_TYPE                      UInt8
#define ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_RESERVED_MASK             0x0000007E
#define    ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_START_DONE_SHIFT       7
#define    ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_START_DONE_MASK        0x00000080
#define    ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_ARL_RW_SHIFT           0
#define    ESW_ARL_VTBL_ACC_ARL_READ_WRITE_CONTROL_ARL_RW_MASK            0x00000001

#define ESW_ARL_VTBL_ACC_MAC_ADDRESS_INDEX_OFFSET                         0x00000010
#define ESW_ARL_VTBL_ACC_MAC_ADDRESS_INDEX_TYPE                           UInt48
#define ESW_ARL_VTBL_ACC_MAC_ADDRESS_INDEX_RESERVED_MASK                  0x00000000
#define    ESW_ARL_VTBL_ACC_MAC_ADDRESS_INDEX_MAC_ADDR_INDX_SHIFT         0
#define    ESW_ARL_VTBL_ACC_MAC_ADDRESS_INDEX_MAC_ADDR_INDX_MASK          0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_VID_INDEX_OFFSET                                 0x00000040
#define ESW_ARL_VTBL_ACC_VID_INDEX_TYPE                                   UInt16
#define ESW_ARL_VTBL_ACC_VID_INDEX_RESERVED_MASK                          0x0000F000
#define    ESW_ARL_VTBL_ACC_VID_INDEX_VID_INDX_SHIFT                      0
#define    ESW_ARL_VTBL_ACC_VID_INDEX_VID_INDX_MASK                       0x00000FFF

#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_OFFSET                       0x00000080
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_TYPE                         UInt64
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_RESERVED_MASK                0xF000000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_VID_SHIFT                 48
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_VID_MASK                  0xFFF000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_MAC_ADDR_SHIFT            0
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_0_MAC_ADDR_MASK             0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_OFFSET                           0x000000C0
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_TYPE                             UInt32
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_RESERVED_MASK                    0xFFFE0000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_VALID_N_SHIFT                 16
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_VALID_N_MASK                  0x00010000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_STATIC_N_SHIFT                15
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_STATIC_N_MASK                 0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_AGE_N_SHIFT                   14
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_AGE_N_MASK                    0x00004000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_PRIORITY_N_SHIFT              11
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_PRIORITY_N_MASK               0x00003800
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_ARL_CON_N_SHIFT               9
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_ARL_CON_N_MASK                0x00000600
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_PORTMAP_PORTID_SHIFT          0
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_0_PORTMAP_PORTID_MASK           0x000001FF

#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_OFFSET                       0x00000100
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_TYPE                         UInt64
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_RESERVED_MASK                0xF000000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_VID_SHIFT                 48
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_VID_MASK                  0xFFF000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_MAC_ADDR_SHIFT            0
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_1_MAC_ADDR_MASK             0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_OFFSET                           0x00000140
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_TYPE                             UInt32
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_RESERVED_MASK                    0xFFFE0000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_VALID_N_SHIFT                 16
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_VALID_N_MASK                  0x00010000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_STATIC_N_SHIFT                15
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_STATIC_N_MASK                 0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_AGE_N_SHIFT                   14
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_AGE_N_MASK                    0x00004000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_PRIORITY_N_SHIFT              11
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_PRIORITY_N_MASK               0x00003800
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_ARL_CON_N_SHIFT               9
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_ARL_CON_N_MASK                0x00000600
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_PORTMAP_PORTID_SHIFT          0
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_1_PORTMAP_PORTID_MASK           0x000001FF

#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_OFFSET                       0x00000180
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_TYPE                         UInt64
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_RESERVED_MASK                0xF000000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_VID_SHIFT                 48
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_VID_MASK                  0xFFF000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_MAC_ADDR_SHIFT            0
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_2_MAC_ADDR_MASK             0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_OFFSET                           0x000001C0
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_TYPE                             UInt32
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_RESERVED_MASK                    0xFFFE0000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_VALID_N_SHIFT                 16
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_VALID_N_MASK                  0x00010000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_STATIC_N_SHIFT                15
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_STATIC_N_MASK                 0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_AGE_N_SHIFT                   14
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_AGE_N_MASK                    0x00004000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_PRIORITY_N_SHIFT              11
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_PRIORITY_N_MASK               0x00003800
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_ARL_CON_N_SHIFT               9
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_ARL_CON_N_MASK                0x00000600
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_PORTMAP_PORTID_SHIFT          0
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_2_PORTMAP_PORTID_MASK           0x000001FF

#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_OFFSET                       0x00000200
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_TYPE                         UInt64
#define ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_RESERVED_MASK                0xF000000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_VID_SHIFT                 48
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_VID_MASK                  0xFFF000000000000
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_MAC_ADDR_SHIFT            0
#define    ESW_ARL_VTBL_ACC_ARL_MAC_VID_INDEX_3_MAC_ADDR_MASK             0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_OFFSET                           0x00000240
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_TYPE                             UInt32
#define ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_RESERVED_MASK                    0xFFFE0000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_VALID_N_SHIFT                 16
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_VALID_N_MASK                  0x00010000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_STATIC_N_SHIFT                15
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_STATIC_N_MASK                 0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_AGE_N_SHIFT                   14
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_AGE_N_MASK                    0x00004000
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_PRIORITY_N_SHIFT              11
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_PRIORITY_N_MASK               0x00003800
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_ARL_CON_N_SHIFT               9
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_ARL_CON_N_MASK                0x00000600
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_PORTMAP_PORTID_SHIFT          0
#define    ESW_ARL_VTBL_ACC_ARL_FWD_ENTRY_3_PORTMAP_PORTID_MASK           0x000001FF

#define ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_OFFSET                        0x00000280
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_TYPE                          UInt8
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_RESERVED_MASK                 0x0000007E
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_START_DONE_SHIFT           7
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_START_DONE_MASK            0x00000080
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_ARL_SEARCH_RESULT_VALID_SHIFT 0
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_CONTROL_ARL_SEARCH_RESULT_VALID_MASK 0x00000001

#define ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_OFFSET                        0x00000288
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_TYPE                          UInt16
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_RESERVED_MASK                 0x00000000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_ARL_ADDR_VALID_SHIFT       15
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_ARL_ADDR_VALID_MASK        0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_SPEED_SHIFT                0
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_ADDRESS_SPEED_MASK                 0x00007FFF

#define ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_OFFSET               0x00000300
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_TYPE                 UInt64
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_RESERVED_MASK        0xF000000000000000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_ARL_SEARCH_VID_RESULT_SHIFT 48
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_ARL_SEARCH_VID_RESULT_MASK 0xFFF000000000000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_ARL_SEARCH_MACADDR_SHIFT 0
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_0_ARL_SEARCH_MACADDR_MASK 0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_OFFSET                       0x00000340
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_TYPE                         UInt32
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_RESERVED_MASK                0xFFFE0000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_VALID_N_SHIFT             16
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_VALID_N_MASK              0x00010000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_STATIC_SHIFT              15
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_STATIC_MASK               0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_AGE_SHIFT                 14
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_AGE_MASK                  0x00004000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_PRIORITY_SHIFT            11
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_PRIORITY_MASK             0x00003800
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_ARL_CON_SHIFT             9
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_ARL_CON_MASK              0x00000600
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_PORTMAP_PORTID_SHIFT      0
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_0_PORTMAP_PORTID_MASK       0x000001FF

#define ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_OFFSET               0x00000380
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_TYPE                 UInt64
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_RESERVED_MASK        0xF000000000000000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_ARL_SEARCH_VID_RESULT_SHIFT 48
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_ARL_SEARCH_VID_RESULT_MASK 0xFFF000000000000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_ARL_SEARCH_MACADDR_SHIFT 0
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_MAC_VID_RESULT_1_ARL_SEARCH_MACADDR_MASK 0xFFFFFFFFFFFF

#define ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_OFFSET                       0x000003C0
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_TYPE                         UInt32
#define ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_RESERVED_MASK                0xFFFE0000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_VALID_N_SHIFT             16
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_VALID_N_MASK              0x00010000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_STATIC_SHIFT              15
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_STATIC_MASK               0x00008000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_AGE_SHIFT                 14
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_AGE_MASK                  0x00004000
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_PRIORITY_SHIFT            11
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_PRIORITY_MASK             0x00003800
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_ARL_CON_SHIFT             9
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_ARL_CON_MASK              0x00000600
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_PORTMAP_PORTID_SHIFT      0
#define    ESW_ARL_VTBL_ACC_ARL_SEARCH_RESULT_1_PORTMAP_PORTID_MASK       0x000001FF

#define ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_OFFSET                     0x00000400
#define ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_TYPE                       UInt8
#define ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_RESERVED_MASK              0x0000007C
#define    ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_START_DONE_SHIFT        7
#define    ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_START_DONE_MASK         0x00000080
#define    ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_VTBL_RW_CLR_SHIFT       0
#define    ESW_ARL_VTBL_ACC_VTBL_READ_WRITE_CLEAR_VTBL_RW_CLR_MASK        0x00000003

#define ESW_ARL_VTBL_ACC_VTBL_ADDRESS_INDEX_OFFSET                        0x00000408
#define ESW_ARL_VTBL_ACC_VTBL_ADDRESS_INDEX_TYPE                          UInt16
#define ESW_ARL_VTBL_ACC_VTBL_ADDRESS_INDEX_RESERVED_MASK                 0x0000F000
#define    ESW_ARL_VTBL_ACC_VTBL_ADDRESS_INDEX_VTBL_ADDR_INDEX_SHIFT      0
#define    ESW_ARL_VTBL_ACC_VTBL_ADDRESS_INDEX_VTBL_ADDR_INDEX_MASK       0x00000FFF

#define ESW_ARL_VTBL_ACC_VTBL_ENTRY_OFFSET                                0x00000418
#define ESW_ARL_VTBL_ACC_VTBL_ENTRY_TYPE                                  UInt32
#define ESW_ARL_VTBL_ACC_VTBL_ENTRY_RESERVED_MASK                         0xFF800000
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_DROP_JUMBO_SHIFT                   22
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_DROP_JUMBO_MASK                    0x00400000
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_FWD_MODE_SHIFT                     21
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_FWD_MODE_MASK                      0x00200000
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_MSPT_INDEX_SHIFT                   18
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_MSPT_INDEX_MASK                    0x001C0000
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_UNTAG_MAP_SHIFT                    9
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_UNTAG_MAP_MASK                     0x0003FE00
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_FWD_MAP_SHIFT                      0
#define    ESW_ARL_VTBL_ACC_VTBL_ENTRY_FWD_MAP_MASK                       0x000001FF

#endif /* __BRCM_RDB_ESW_ARL_VTBL_ACC_H__ */



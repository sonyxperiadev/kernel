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

#ifndef __BRCM_RDB_KHUB_RST_MGR_REG_H__
#define __BRCM_RDB_KHUB_RST_MGR_REG_H__

#define KHUB_RST_MGR_REG_WR_ACCESS_OFFSET                                 0x00000000
#define KHUB_RST_MGR_REG_WR_ACCESS_TYPE                                   UInt32
#define KHUB_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                          0x7F0000FE
#define    KHUB_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT              31
#define    KHUB_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK               0x80000000
#define    KHUB_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                      8
#define    KHUB_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                       0x00FFFF00
#define    KHUB_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                    0
#define    KHUB_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                     0x00000001

#define KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET                                0x00000004
#define KHUB_RST_MGR_REG_SOFT_RSTN0_TYPE                                  UInt32
#define KHUB_RST_MGR_REG_SOFT_RSTN0_RESERVED_MASK                         0x7BF007F0
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_SHIFT             31
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_MASK              0x80000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_ETB_SOFT_RSTN_SHIFT                26
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_ETB_SOFT_RSTN_MASK                 0x04000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_7_SOFT_RSTN_SHIFT    19
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_7_SOFT_RSTN_MASK     0x00080000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_4_SOFT_RSTN_SHIFT    18
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_4_SOFT_RSTN_MASK     0x00040000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_TPIU_SOFT_RSTN_SHIFT               17
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_TPIU_SOFT_RSTN_MASK                0x00020000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_FUNNEL_SOFT_RSTN_SHIFT             16
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_FUNNEL_SOFT_RSTN_MASK              0x00010000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_VC_ITM_SOFT_RSTN_SHIFT             15
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_VC_ITM_SOFT_RSTN_MASK              0x00008000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_CTI_SOFT_RSTN_SHIFT                14
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_CTI_SOFT_RSTN_MASK                 0x00004000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_IPC_SOFT_RSTN_SHIFT                13
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_IPC_SOFT_RSTN_MASK                 0x00002000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SRAM_MPU_SOFT_RSTN_SHIFT           12
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SRAM_MPU_SOFT_RSTN_MASK            0x00001000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_KONAHUB_TZCFG_SOFT_RSTN_SHIFT      11
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_KONAHUB_TZCFG_SOFT_RSTN_MASK       0x00000800
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_APB5_SOFT_RSTN_SHIFT               3
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_APB5_SOFT_RSTN_MASK                0x00000008
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_BROM_SOFT_RSTN_SHIFT               2
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_BROM_SOFT_RSTN_MASK                0x00000004
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_NOR_SOFT_RSTN_SHIFT                1
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_NOR_SOFT_RSTN_MASK                 0x00000002
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_HUB_SOFT_RSTN_SHIFT                0
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_HUB_SOFT_RSTN_MASK                 0x00000001

#define KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET                                0x00000008
#define KHUB_RST_MGR_REG_SOFT_RSTN1_TYPE                                  UInt32
#define KHUB_RST_MGR_REG_SOFT_RSTN1_RESERVED_MASK                         0x7FF0062C
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_SHIFT             31
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_MASK              0x80000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_13_SOFT_RSTN_SHIFT       19
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_13_SOFT_RSTN_MASK        0x00080000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_SHIFT               18
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_MASK                0x00040000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_VAR_SPM_SOFT_RSTN_SHIFT            17
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_VAR_SPM_SOFT_RSTN_MASK             0x00020000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_TMON_SOFT_RSTN_SHIFT               16
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_TMON_SOFT_RSTN_MASK                0x00010000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_SHIFT             15
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_MASK              0x00008000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_12_SOFT_RSTN_SHIFT       14
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_12_SOFT_RSTN_MASK        0x00004000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SEC_VIOL_TRAP_5_SOFT_RSTN_SHIFT    13
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SEC_VIOL_TRAP_5_SOFT_RSTN_MASK     0x00002000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_BT_SLIM_SOFT_RSTN_SHIFT            12
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_BT_SLIM_SOFT_RSTN_MASK             0x00001000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_SHIFT               11
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_MASK                0x00000800
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_APB10_SOFT_RSTN_SHIFT              8
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_APB10_SOFT_RSTN_MASK               0x00000100
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_APB9_SOFT_RSTN_SHIFT               7
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_APB9_SOFT_RSTN_MASK                0x00000080
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_ETB2AXI_SOFT_RSTN_SHIFT            6
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_ETB2AXI_SOFT_RSTN_MASK             0x00000040
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_MDIOMASTER_SOFT_RSTN_SHIFT         4
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_MDIOMASTER_SOFT_RSTN_MASK          0x00000010
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_11_SOFT_RSTN_SHIFT       1
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_11_SOFT_RSTN_MASK        0x00000002
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_19_SOFT_RSTN_SHIFT       0
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AXI_TRACE_19_SOFT_RSTN_MASK        0x00000001

#endif /* __BRCM_RDB_KHUB_RST_MGR_REG_H__ */



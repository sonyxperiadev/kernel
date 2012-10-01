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

#ifndef __BRCM_RDB_KHUBAON_RST_MGR_REG_H__
#define __BRCM_RDB_KHUBAON_RST_MGR_REG_H__

#define KHUBAON_RST_MGR_REG_WR_ACCESS_OFFSET                              0x00000000
#define KHUBAON_RST_MGR_REG_WR_ACCESS_TYPE                                UInt32
#define KHUBAON_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                       0x7F0000FE
#define    KHUBAON_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT           31
#define    KHUBAON_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK            0x80000000
#define    KHUBAON_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                   8
#define    KHUBAON_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                    0x00FFFF00
#define    KHUBAON_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                 0
#define    KHUBAON_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                  0x00000001

#define KHUBAON_RST_MGR_REG_SOFT_RSTN0_OFFSET                             0x00000004
#define KHUBAON_RST_MGR_REG_SOFT_RSTN0_TYPE                               UInt32
#define KHUBAON_RST_MGR_REG_SOFT_RSTN0_RESERVED_MASK                      0x7F7C000E
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_SHIFT          31
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_MASK           0x80000000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_PWRMGR_SOFT_RSTN_SHIFT          23
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_PWRMGR_SOFT_RSTN_MASK           0x00800000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SIM2_SOFT_RSTN_SHIFT            17
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SIM2_SOFT_RSTN_MASK             0x00020000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SIM_SOFT_RSTN_SHIFT             16
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SIM_SOFT_RSTN_MASK              0x00010000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_ACI_SOFT_RSTN_SHIFT             15
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_ACI_SOFT_RSTN_MASK              0x00008000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SPM_SOFT_RSTN_SHIFT             14
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SPM_SOFT_RSTN_MASK              0x00004000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SEC_WD_SOFT_RSTN_SHIFT          13
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SEC_WD_SOFT_RSTN_MASK           0x00002000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_FMON_SOFT_RSTN_SHIFT            12
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_FMON_SOFT_RSTN_MASK             0x00001000
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_CHIPREG_SOFT_RSTN_SHIFT         11
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_CHIPREG_SOFT_RSTN_MASK          0x00000800
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_VCEMI_SOFT_RSTN_SHIFT           10
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_VCEMI_SOFT_RSTN_MASK            0x00000400
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SYSEMI_SOFT_RSTN_SHIFT          9
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_SYSEMI_SOFT_RSTN_MASK           0x00000200
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_SHIFT       8
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TIMER_SOFT_RSTN_MASK        0x00000100
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_GPIOKP_SOFT_RSTN_SHIFT          7
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_GPIOKP_SOFT_RSTN_MASK           0x00000080
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TZCFG_SOFT_RSTN_SHIFT       6
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUB_TZCFG_SOFT_RSTN_MASK        0x00000040
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_PMU_BSC_SOFT_RSTN_SHIFT         5
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_PMU_BSC_SOFT_RSTN_MASK          0x00000020
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_APB6_SOFT_RSTN_SHIFT            4
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_APB6_SOFT_RSTN_MASK             0x00000010
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUBAON_SOFT_RSTN_SHIFT          0
#define    KHUBAON_RST_MGR_REG_SOFT_RSTN0_HUBAON_SOFT_RSTN_MASK           0x00000001

#endif /* __BRCM_RDB_KHUBAON_RST_MGR_REG_H__ */



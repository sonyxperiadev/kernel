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

#ifndef __BRCM_RDB_BMDM_RST_MGR_REG_H__
#define __BRCM_RDB_BMDM_RST_MGR_REG_H__

#define BMDM_RST_MGR_REG_WR_ACCESS_OFFSET                                 0x00000000
#define BMDM_RST_MGR_REG_WR_ACCESS_TYPE                                   UInt32
#define BMDM_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                          0x7FFFFFFE
#define    BMDM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT              31
#define    BMDM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK               0x80000000
#define    BMDM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                    0
#define    BMDM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                     0x00000001

#define BMDM_RST_MGR_REG_APB_CORE_RSTN_OFFSET                             0x00000004
#define BMDM_RST_MGR_REG_APB_CORE_RSTN_TYPE                               UInt32
#define BMDM_RST_MGR_REG_APB_CORE_RSTN_RESERVED_MASK                      0x7FFFFFFE
#define    BMDM_RST_MGR_REG_APB_CORE_RSTN_PRIV_ACCESS_MODE_SHIFT          31
#define    BMDM_RST_MGR_REG_APB_CORE_RSTN_PRIV_ACCESS_MODE_MASK           0x80000000
#define    BMDM_RST_MGR_REG_APB_CORE_RSTN_APB_CORE_SOFT_RSTN_SHIFT        0
#define    BMDM_RST_MGR_REG_APB_CORE_RSTN_APB_CORE_SOFT_RSTN_MASK         0x00000001

#define BMDM_RST_MGR_REG_WCDMA_RSTN_OFFSET                                0x00000008
#define BMDM_RST_MGR_REG_WCDMA_RSTN_TYPE                                  UInt32
#define BMDM_RST_MGR_REG_WCDMA_RSTN_RESERVED_MASK                         0x7FFFFFFE
#define    BMDM_RST_MGR_REG_WCDMA_RSTN_PRIV_ACCESS_MODE_SHIFT             31
#define    BMDM_RST_MGR_REG_WCDMA_RSTN_PRIV_ACCESS_MODE_MASK              0x80000000
#define    BMDM_RST_MGR_REG_WCDMA_RSTN_WCDMA_SOFT_RSTN_SHIFT              0
#define    BMDM_RST_MGR_REG_WCDMA_RSTN_WCDMA_SOFT_RSTN_MASK               0x00000001

#define BMDM_RST_MGR_REG_SOFT_RSTN0_OFFSET                                0x0000000C
#define BMDM_RST_MGR_REG_SOFT_RSTN0_TYPE                                  UInt32
#define BMDM_RST_MGR_REG_SOFT_RSTN0_RESERVED_MASK                         0x7FFFFFF0
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_SHIFT             31
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_MASK              0x80000000
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_EDGE_MP_SOFT_RSTN_SHIFT            3
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_EDGE_MP_SOFT_RSTN_MASK             0x00000008
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_WCDMA_DATAPACKER_SOFT_RSTN_SHIFT   2
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_WCDMA_DATAPACKER_SOFT_RSTN_MASK    0x00000004
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_WCDMA_HUCM_SOFT_RSTN_SHIFT         1
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_WCDMA_HUCM_SOFT_RSTN_MASK          0x00000002
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_WCDMA_CIPHER_SOFT_RSTN_SHIFT       0
#define    BMDM_RST_MGR_REG_SOFT_RSTN0_WCDMA_CIPHER_SOFT_RSTN_MASK        0x00000001

#define BMDM_RST_MGR_REG_SOFT_RSTN1_OFFSET                                0x00000010
#define BMDM_RST_MGR_REG_SOFT_RSTN1_TYPE                                  UInt32
#define BMDM_RST_MGR_REG_SOFT_RSTN1_RESERVED_MASK                         0x7FFFFFF1
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_SHIFT             31
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_MASK              0x80000000
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_RFBB_SOFT_RSTN_SHIFT               3
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_RFBB_SOFT_RSTN_MASK                0x00000008
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_SCLKCAL_23G_SOFT_RSTN_SHIFT        2
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_SCLKCAL_23G_SOFT_RSTN_MASK         0x00000004
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_SCLKCAL_SOFT_RSTN_SHIFT            1
#define    BMDM_RST_MGR_REG_SOFT_RSTN1_SCLKCAL_SOFT_RSTN_MASK             0x00000002

#define BMDM_RST_MGR_REG_RSTN_REASON_OFFSET                               0x00000014
#define BMDM_RST_MGR_REG_RSTN_REASON_TYPE                                 UInt32
#define BMDM_RST_MGR_REG_RSTN_REASON_RESERVED_MASK                        0x7FFFFFFA
#define    BMDM_RST_MGR_REG_RSTN_REASON_PRIV_ACCESS_MODE_SHIFT            31
#define    BMDM_RST_MGR_REG_RSTN_REASON_PRIV_ACCESS_MODE_MASK             0x80000000
#define    BMDM_RST_MGR_REG_RSTN_REASON_MDM_WD_RSTN_V_SHIFT               2
#define    BMDM_RST_MGR_REG_RSTN_REASON_MDM_WD_RSTN_V_MASK                0x00000004
#define    BMDM_RST_MGR_REG_RSTN_REASON_MDM_CORE_RSTN_V_SHIFT             0
#define    BMDM_RST_MGR_REG_RSTN_REASON_MDM_CORE_RSTN_V_MASK              0x00000001

#define BMDM_RST_MGR_REG_CP_RSTN_OFFSET                                   0x00000018
#define BMDM_RST_MGR_REG_CP_RSTN_TYPE                                     UInt32
#define BMDM_RST_MGR_REG_CP_RSTN_RESERVED_MASK                            0x7FFFFFFC
#define    BMDM_RST_MGR_REG_CP_RSTN_PRIV_ACCESS_MODE_SHIFT                31
#define    BMDM_RST_MGR_REG_CP_RSTN_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    BMDM_RST_MGR_REG_CP_RSTN_CP_DEBUG_RSTN_SHIFT                   1
#define    BMDM_RST_MGR_REG_CP_RSTN_CP_DEBUG_RSTN_MASK                    0x00000002
#define    BMDM_RST_MGR_REG_CP_RSTN_CP_RSTN_SHIFT                         0
#define    BMDM_RST_MGR_REG_CP_RSTN_CP_RSTN_MASK                          0x00000001

#define BMDM_RST_MGR_REG_RST_WIDTH_OFFSET                                 0x0000001C
#define BMDM_RST_MGR_REG_RST_WIDTH_TYPE                                   UInt32
#define BMDM_RST_MGR_REG_RST_WIDTH_RESERVED_MASK                          0x7FFF0000
#define    BMDM_RST_MGR_REG_RST_WIDTH_PRIV_ACCESS_MODE_SHIFT              31
#define    BMDM_RST_MGR_REG_RST_WIDTH_PRIV_ACCESS_MODE_MASK               0x80000000
#define    BMDM_RST_MGR_REG_RST_WIDTH_WCDMA_WAKEUP_RST_WIDTH_SHIFT        8
#define    BMDM_RST_MGR_REG_RST_WIDTH_WCDMA_WAKEUP_RST_WIDTH_MASK         0x0000FF00
#define    BMDM_RST_MGR_REG_RST_WIDTH_CP_WAKEUP_RST_WIDTH_SHIFT           0
#define    BMDM_RST_MGR_REG_RST_WIDTH_CP_WAKEUP_RST_WIDTH_MASK            0x000000FF

#endif /* __BRCM_RDB_BMDM_RST_MGR_REG_H__ */



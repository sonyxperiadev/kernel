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

#ifndef __BRCM_RDB_ROOT_RST_MGR_REG_H__
#define __BRCM_RDB_ROOT_RST_MGR_REG_H__

#define ROOT_RST_MGR_REG_WR_ACCESS_OFFSET                                 0x00000000
#define ROOT_RST_MGR_REG_WR_ACCESS_TYPE                                   UInt32
#define ROOT_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                          0x7F0000FE
#define    ROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT              31
#define    ROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK               0x80000000
#define    ROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                      8
#define    ROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                       0x00FFFF00
#define    ROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                    0
#define    ROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                     0x00000001

#define ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET                            0x00000004
#define ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_TYPE                              UInt32
#define ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_RESERVED_MASK                     0x7FFFFFFE
#define    ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_SHIFT         31
#define    ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK          0x80000000
#define    ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_CHIP_SOFT_RSTN_SHIFT           0
#define    ROOT_RST_MGR_REG_CHIP_SOFT_RSTN_CHIP_SOFT_RSTN_MASK            0x00000001

#define ROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET                              0x00000008
#define ROOT_RST_MGR_REG_PD_SOFT_RSTN_TYPE                                UInt32
#define ROOT_RST_MGR_REG_PD_SOFT_RSTN_RESERVED_MASK                       0x7FFFFF02
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_PRIV_ACCESS_MODE_SHIFT           31
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_PRIV_ACCESS_MODE_MASK            0x80000000
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SUB_SOFT_RSTN_SHIFT          7
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SUB_SOFT_RSTN_MASK           0x00000080
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_SHIFT              6
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_MASK               0x00000040
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SUB_SOFT_RSTN_SHIFT           5
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SUB_SOFT_RSTN_MASK            0x00000020
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SOFT_RSTN_SHIFT               4
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SOFT_RSTN_MASK                0x00000010
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_KSLV_SOFT_RSTN_SHIFT             3
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_KSLV_SOFT_RSTN_MASK              0x00000008
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_KMST_SOFT_RSTN_SHIFT             2
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_KMST_SOFT_RSTN_MASK              0x00000004
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_HUB_SOFT_RSTN_SHIFT              0
#define    ROOT_RST_MGR_REG_PD_SOFT_RSTN_HUB_SOFT_RSTN_MASK               0x00000001

#define ROOT_RST_MGR_REG_SYSWKUP_OFFSET                                   0x0000000C
#define ROOT_RST_MGR_REG_SYSWKUP_TYPE                                     UInt32
#define ROOT_RST_MGR_REG_SYSWKUP_RESERVED_MASK                            0x7FFF0000
#define    ROOT_RST_MGR_REG_SYSWKUP_PRIV_ACCESS_MODE_SHIFT                31
#define    ROOT_RST_MGR_REG_SYSWKUP_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    ROOT_RST_MGR_REG_SYSWKUP_HUB_WAKEUP_RST_WIDTH_SHIFT            8
#define    ROOT_RST_MGR_REG_SYSWKUP_HUB_WAKEUP_RST_WIDTH_MASK             0x0000FF00
#define    ROOT_RST_MGR_REG_SYSWKUP_ARM_WAKEUP_RST_WIDTH_SHIFT            0
#define    ROOT_RST_MGR_REG_SYSWKUP_ARM_WAKEUP_RST_WIDTH_MASK             0x000000FF

#define ROOT_RST_MGR_REG_MMWKUP_OFFSET                                    0x00000010
#define ROOT_RST_MGR_REG_MMWKUP_TYPE                                      UInt32
#define ROOT_RST_MGR_REG_MMWKUP_RESERVED_MASK                             0x7FFF0000
#define    ROOT_RST_MGR_REG_MMWKUP_PRIV_ACCESS_MODE_SHIFT                 31
#define    ROOT_RST_MGR_REG_MMWKUP_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    ROOT_RST_MGR_REG_MMWKUP_MM_SUB_WAKEUP_RST_WIDTH_SHIFT          8
#define    ROOT_RST_MGR_REG_MMWKUP_MM_SUB_WAKEUP_RST_WIDTH_MASK           0x0000FF00
#define    ROOT_RST_MGR_REG_MMWKUP_MM_WAKEUP_RST_WIDTH_SHIFT              0
#define    ROOT_RST_MGR_REG_MMWKUP_MM_WAKEUP_RST_WIDTH_MASK               0x000000FF

#define ROOT_RST_MGR_REG_KONAWKUP_OFFSET                                  0x00000014
#define ROOT_RST_MGR_REG_KONAWKUP_TYPE                                    UInt32
#define ROOT_RST_MGR_REG_KONAWKUP_RESERVED_MASK                           0x7FFF0000
#define    ROOT_RST_MGR_REG_KONAWKUP_PRIV_ACCESS_MODE_SHIFT               31
#define    ROOT_RST_MGR_REG_KONAWKUP_PRIV_ACCESS_MODE_MASK                0x80000000
#define    ROOT_RST_MGR_REG_KONAWKUP_KSLV_WAKEUP_RST_WIDTH_SHIFT          8
#define    ROOT_RST_MGR_REG_KONAWKUP_KSLV_WAKEUP_RST_WIDTH_MASK           0x0000FF00
#define    ROOT_RST_MGR_REG_KONAWKUP_KMST_WAKEUP_RST_WIDTH_SHIFT          0
#define    ROOT_RST_MGR_REG_KONAWKUP_KMST_WAKEUP_RST_WIDTH_MASK           0x000000FF

#define ROOT_RST_MGR_REG_RSTSTS_OFFSET                                    0x00000018
#define ROOT_RST_MGR_REG_RSTSTS_TYPE                                      UInt32
#define ROOT_RST_MGR_REG_RSTSTS_RESERVED_MASK                             0x7FFFE000
#define    ROOT_RST_MGR_REG_RSTSTS_PRIV_ACCESS_MODE_SHIFT                 31
#define    ROOT_RST_MGR_REG_RSTSTS_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    ROOT_RST_MGR_REG_RSTSTS_DAPRST_DET_SHIFT                       12
#define    ROOT_RST_MGR_REG_RSTSTS_DAPRST_DET_MASK                        0x00001000
#define    ROOT_RST_MGR_REG_RSTSTS_FMON_HIGH_WARNINGRST_DET_SHIFT         11
#define    ROOT_RST_MGR_REG_RSTSTS_FMON_HIGH_WARNINGRST_DET_MASK          0x00000800
#define    ROOT_RST_MGR_REG_RSTSTS_FMON_LOW_WARNINGRST_DET_SHIFT          10
#define    ROOT_RST_MGR_REG_RSTSTS_FMON_LOW_WARNINGRST_DET_MASK           0x00000400
#define    ROOT_RST_MGR_REG_RSTSTS_HUB_SECWDCHKRST_DET_SHIFT              9
#define    ROOT_RST_MGR_REG_RSTSTS_HUB_SECWDCHKRST_DET_MASK               0x00000200
#define    ROOT_RST_MGR_REG_RSTSTS_BBL_SECWDCHKRST_DET_SHIFT              8
#define    ROOT_RST_MGR_REG_RSTSTS_BBL_SECWDCHKRST_DET_MASK               0x00000100
#define    ROOT_RST_MGR_REG_RSTSTS_TMON_WARNINGRST_DET_SHIFT              7
#define    ROOT_RST_MGR_REG_RSTSTS_TMON_WARNINGRST_DET_MASK               0x00000080
#define    ROOT_RST_MGR_REG_RSTSTS_HUB_SECWDRST_DET_SHIFT                 6
#define    ROOT_RST_MGR_REG_RSTSTS_HUB_SECWDRST_DET_MASK                  0x00000040
#define    ROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_SHIFT                   5
#define    ROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_MASK                    0x00000020
#define    ROOT_RST_MGR_REG_RSTSTS_BBL_SECWDRST_DET_SHIFT                 4
#define    ROOT_RST_MGR_REG_RSTSTS_BBL_SECWDRST_DET_MASK                  0x00000010
#define    ROOT_RST_MGR_REG_RSTSTS_OPENWDRST_DET_SHIFT                    3
#define    ROOT_RST_MGR_REG_RSTSTS_OPENWDRST_DET_MASK                     0x00000008
#define    ROOT_RST_MGR_REG_RSTSTS_CHIPRST_DET_SHIFT                      2
#define    ROOT_RST_MGR_REG_RSTSTS_CHIPRST_DET_MASK                       0x00000004
#define    ROOT_RST_MGR_REG_RSTSTS_PORRST_DET_SHIFT                       1
#define    ROOT_RST_MGR_REG_RSTSTS_PORRST_DET_MASK                        0x00000002
#define    ROOT_RST_MGR_REG_RSTSTS_SFTRST_CNT_OVFLW_SHIFT                 0
#define    ROOT_RST_MGR_REG_RSTSTS_SFTRST_CNT_OVFLW_MASK                  0x00000001

#define ROOT_RST_MGR_REG_MDMWKUP_OFFSET                                   0x0000001C
#define ROOT_RST_MGR_REG_MDMWKUP_TYPE                                     UInt32
#define ROOT_RST_MGR_REG_MDMWKUP_RESERVED_MASK                            0x7FFF0000
#define    ROOT_RST_MGR_REG_MDMWKUP_PRIV_ACCESS_MODE_SHIFT                31
#define    ROOT_RST_MGR_REG_MDMWKUP_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    ROOT_RST_MGR_REG_MDMWKUP_MDM_SUB_WAKEUP_RST_WIDTH_SHIFT        8
#define    ROOT_RST_MGR_REG_MDMWKUP_MDM_SUB_WAKEUP_RST_WIDTH_MASK         0x0000FF00
#define    ROOT_RST_MGR_REG_MDMWKUP_MDM_WAKEUP_RST_WIDTH_SHIFT            0
#define    ROOT_RST_MGR_REG_MDMWKUP_MDM_WAKEUP_RST_WIDTH_MASK             0x000000FF

#define ROOT_RST_MGR_REG_RSTSTS_SFTCLR_OFFSET                             0x00000020
#define ROOT_RST_MGR_REG_RSTSTS_SFTCLR_TYPE                               UInt32
#define ROOT_RST_MGR_REG_RSTSTS_SFTCLR_RESERVED_MASK                      0x7FFFE001
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_PRIV_ACCESS_MODE_SHIFT          31
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_PRIV_ACCESS_MODE_MASK           0x80000000
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_DAPRST_DET_V_SHIFT              12
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_DAPRST_DET_V_MASK               0x00001000
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_HIGH_WARNINGRST_DET_V_SHIFT 11
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_HIGH_WARNINGRST_DET_V_MASK 0x00000800
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_LOW_WARNINGRST_DET_V_SHIFT 10
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_LOW_WARNINGRST_DET_V_MASK  0x00000400
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDCHKRST_DET_V_SHIFT     9
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDCHKRST_DET_V_MASK      0x00000200
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDCHKRST_DET_V_SHIFT     8
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDCHKRST_DET_V_MASK      0x00000100
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_TMON_WARNINGRST_DET_V_SHIFT     7
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_TMON_WARNINGRST_DET_V_MASK      0x00000080
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDRST_DET_V_SHIFT        6
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDRST_DET_V_MASK         0x00000040
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPSFTRST_DET_V_SHIFT          5
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPSFTRST_DET_V_MASK           0x00000020
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDRST_DET_V_SHIFT        4
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDRST_DET_V_MASK         0x00000010
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_OPENWDRST_DET_V_SHIFT           3
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_OPENWDRST_DET_V_MASK            0x00000008
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPRST_DET_V_SHIFT             2
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPRST_DET_V_MASK              0x00000004
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_PORRST_DET_V_SHIFT              1
#define    ROOT_RST_MGR_REG_RSTSTS_SFTCLR_PORRST_DET_V_MASK               0x00000002

#endif /* __BRCM_RDB_ROOT_RST_MGR_REG_H__ */



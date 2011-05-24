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

#ifndef __BRCM_RDB_IROOT_RST_MGR_REG_H__
#define __BRCM_RDB_IROOT_RST_MGR_REG_H__

#define IROOT_RST_MGR_REG_WR_ACCESS_OFFSET                                0x00000000
#define IROOT_RST_MGR_REG_WR_ACCESS_TYPE                                  UInt32
#define IROOT_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                         0x7F0000FE
#define    IROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT             31
#define    IROOT_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK              0x80000000
#define    IROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                     8
#define    IROOT_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                      0x00FFFF00
#define    IROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                   0
#define    IROOT_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                    0x00000001

#define IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_OFFSET                           0x00000004
#define IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_TYPE                             UInt32
#define IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_RESERVED_MASK                    0x7FFFFFFE
#define    IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_SHIFT        31
#define    IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_PRIV_ACCESS_MODE_MASK         0x80000000
#define    IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_CHIP_SOFT_RSTN_SHIFT          0
#define    IROOT_RST_MGR_REG_CHIP_SOFT_RSTN_CHIP_SOFT_RSTN_MASK           0x00000001

#define IROOT_RST_MGR_REG_PD_SOFT_RSTN_OFFSET                             0x00000008
#define IROOT_RST_MGR_REG_PD_SOFT_RSTN_TYPE                               UInt32
#define IROOT_RST_MGR_REG_PD_SOFT_RSTN_RESERVED_MASK                      0x7FFFFE02
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_PRIV_ACCESS_MODE_SHIFT          31
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_PRIV_ACCESS_MODE_MASK           0x80000000
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_ESUB_SOFT_RSTN_SHIFT            8
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_ESUB_SOFT_RSTN_MASK             0x00000100
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SUB_SOFT_RSTN_SHIFT         7
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SUB_SOFT_RSTN_MASK          0x00000080
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_SHIFT             6
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MDM_SOFT_RSTN_MASK              0x00000040
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SUB_SOFT_RSTN_SHIFT          5
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SUB_SOFT_RSTN_MASK           0x00000020
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SOFT_RSTN_SHIFT              4
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_MM_SOFT_RSTN_MASK               0x00000010
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_KSLV_SOFT_RSTN_SHIFT            3
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_KSLV_SOFT_RSTN_MASK             0x00000008
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_KMST_SOFT_RSTN_SHIFT            2
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_KMST_SOFT_RSTN_MASK             0x00000004
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_HUB_SOFT_RSTN_SHIFT             0
#define    IROOT_RST_MGR_REG_PD_SOFT_RSTN_HUB_SOFT_RSTN_MASK              0x00000001

#define IROOT_RST_MGR_REG_SYSWKUP_OFFSET                                  0x0000000C
#define IROOT_RST_MGR_REG_SYSWKUP_TYPE                                    UInt32
#define IROOT_RST_MGR_REG_SYSWKUP_RESERVED_MASK                           0x7FFF0000
#define    IROOT_RST_MGR_REG_SYSWKUP_PRIV_ACCESS_MODE_SHIFT               31
#define    IROOT_RST_MGR_REG_SYSWKUP_PRIV_ACCESS_MODE_MASK                0x80000000
#define    IROOT_RST_MGR_REG_SYSWKUP_HUB_WAKEUP_RST_WIDTH_SHIFT           8
#define    IROOT_RST_MGR_REG_SYSWKUP_HUB_WAKEUP_RST_WIDTH_MASK            0x0000FF00
#define    IROOT_RST_MGR_REG_SYSWKUP_ARM_WAKEUP_RST_WIDTH_SHIFT           0
#define    IROOT_RST_MGR_REG_SYSWKUP_ARM_WAKEUP_RST_WIDTH_MASK            0x000000FF

#define IROOT_RST_MGR_REG_MMWKUP_OFFSET                                   0x00000010
#define IROOT_RST_MGR_REG_MMWKUP_TYPE                                     UInt32
#define IROOT_RST_MGR_REG_MMWKUP_RESERVED_MASK                            0x7FFF0000
#define    IROOT_RST_MGR_REG_MMWKUP_PRIV_ACCESS_MODE_SHIFT                31
#define    IROOT_RST_MGR_REG_MMWKUP_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    IROOT_RST_MGR_REG_MMWKUP_MM_SUB_WAKEUP_RST_WIDTH_SHIFT         8
#define    IROOT_RST_MGR_REG_MMWKUP_MM_SUB_WAKEUP_RST_WIDTH_MASK          0x0000FF00
#define    IROOT_RST_MGR_REG_MMWKUP_MM_WAKEUP_RST_WIDTH_SHIFT             0
#define    IROOT_RST_MGR_REG_MMWKUP_MM_WAKEUP_RST_WIDTH_MASK              0x000000FF

#define IROOT_RST_MGR_REG_KONAWKUP_OFFSET                                 0x00000014
#define IROOT_RST_MGR_REG_KONAWKUP_TYPE                                   UInt32
#define IROOT_RST_MGR_REG_KONAWKUP_RESERVED_MASK                          0x7FFF0000
#define    IROOT_RST_MGR_REG_KONAWKUP_PRIV_ACCESS_MODE_SHIFT              31
#define    IROOT_RST_MGR_REG_KONAWKUP_PRIV_ACCESS_MODE_MASK               0x80000000
#define    IROOT_RST_MGR_REG_KONAWKUP_KSLV_WAKEUP_RST_WIDTH_SHIFT         8
#define    IROOT_RST_MGR_REG_KONAWKUP_KSLV_WAKEUP_RST_WIDTH_MASK          0x0000FF00
#define    IROOT_RST_MGR_REG_KONAWKUP_KMST_WAKEUP_RST_WIDTH_SHIFT         0
#define    IROOT_RST_MGR_REG_KONAWKUP_KMST_WAKEUP_RST_WIDTH_MASK          0x000000FF

#define IROOT_RST_MGR_REG_RSTSTS_OFFSET                                   0x00000018
#define IROOT_RST_MGR_REG_RSTSTS_TYPE                                     UInt32
#define IROOT_RST_MGR_REG_RSTSTS_RESERVED_MASK                            0x7FFFC000
#define    IROOT_RST_MGR_REG_RSTSTS_PRIV_ACCESS_MODE_SHIFT                31
#define    IROOT_RST_MGR_REG_RSTSTS_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    IROOT_RST_MGR_REG_RSTSTS_MDMWDRST_DET_SHIFT                    13
#define    IROOT_RST_MGR_REG_RSTSTS_MDMWDRST_DET_MASK                     0x00002000
#define    IROOT_RST_MGR_REG_RSTSTS_DAPRST_DET_SHIFT                      12
#define    IROOT_RST_MGR_REG_RSTSTS_DAPRST_DET_MASK                       0x00001000
#define    IROOT_RST_MGR_REG_RSTSTS_FMON_HIGH_WARNINGRST_DET_SHIFT        11
#define    IROOT_RST_MGR_REG_RSTSTS_FMON_HIGH_WARNINGRST_DET_MASK         0x00000800
#define    IROOT_RST_MGR_REG_RSTSTS_FMON_LOW_WARNINGRST_DET_SHIFT         10
#define    IROOT_RST_MGR_REG_RSTSTS_FMON_LOW_WARNINGRST_DET_MASK          0x00000400
#define    IROOT_RST_MGR_REG_RSTSTS_HUB_SECWDCHKRST_DET_SHIFT             9
#define    IROOT_RST_MGR_REG_RSTSTS_HUB_SECWDCHKRST_DET_MASK              0x00000200
#define    IROOT_RST_MGR_REG_RSTSTS_BBL_SECWDCHKRST_DET_SHIFT             8
#define    IROOT_RST_MGR_REG_RSTSTS_BBL_SECWDCHKRST_DET_MASK              0x00000100
#define    IROOT_RST_MGR_REG_RSTSTS_TMON_WARNINGRST_DET_SHIFT             7
#define    IROOT_RST_MGR_REG_RSTSTS_TMON_WARNINGRST_DET_MASK              0x00000080
#define    IROOT_RST_MGR_REG_RSTSTS_HUB_SECWDRST_DET_SHIFT                6
#define    IROOT_RST_MGR_REG_RSTSTS_HUB_SECWDRST_DET_MASK                 0x00000040
#define    IROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_SHIFT                  5
#define    IROOT_RST_MGR_REG_RSTSTS_CHIPSFTRST_DET_MASK                   0x00000020
#define    IROOT_RST_MGR_REG_RSTSTS_BBL_SECWDRST_DET_SHIFT                4
#define    IROOT_RST_MGR_REG_RSTSTS_BBL_SECWDRST_DET_MASK                 0x00000010
#define    IROOT_RST_MGR_REG_RSTSTS_OPENWDRST_DET_SHIFT                   3
#define    IROOT_RST_MGR_REG_RSTSTS_OPENWDRST_DET_MASK                    0x00000008
#define    IROOT_RST_MGR_REG_RSTSTS_CHIPRST_DET_SHIFT                     2
#define    IROOT_RST_MGR_REG_RSTSTS_CHIPRST_DET_MASK                      0x00000004
#define    IROOT_RST_MGR_REG_RSTSTS_PORRST_DET_SHIFT                      1
#define    IROOT_RST_MGR_REG_RSTSTS_PORRST_DET_MASK                       0x00000002
#define    IROOT_RST_MGR_REG_RSTSTS_SFTRST_CNT_OVFLW_SHIFT                0
#define    IROOT_RST_MGR_REG_RSTSTS_SFTRST_CNT_OVFLW_MASK                 0x00000001

#define IROOT_RST_MGR_REG_MDMWKUP_OFFSET                                  0x0000001C
#define IROOT_RST_MGR_REG_MDMWKUP_TYPE                                    UInt32
#define IROOT_RST_MGR_REG_MDMWKUP_RESERVED_MASK                           0x7FFE0000
#define    IROOT_RST_MGR_REG_MDMWKUP_PRIV_ACCESS_MODE_SHIFT               31
#define    IROOT_RST_MGR_REG_MDMWKUP_PRIV_ACCESS_MODE_MASK                0x80000000
#define    IROOT_RST_MGR_REG_MDMWKUP_MDM_WD_RST_ENABLE_SHIFT              16
#define    IROOT_RST_MGR_REG_MDMWKUP_MDM_WD_RST_ENABLE_MASK               0x00010000
#define    IROOT_RST_MGR_REG_MDMWKUP_MDM_SUB_WAKEUP_RST_WIDTH_SHIFT       8
#define    IROOT_RST_MGR_REG_MDMWKUP_MDM_SUB_WAKEUP_RST_WIDTH_MASK        0x0000FF00
#define    IROOT_RST_MGR_REG_MDMWKUP_MDM_WAKEUP_RST_WIDTH_SHIFT           0
#define    IROOT_RST_MGR_REG_MDMWKUP_MDM_WAKEUP_RST_WIDTH_MASK            0x000000FF

#define IROOT_RST_MGR_REG_RSTSTS_SFTCLR_OFFSET                            0x00000020
#define IROOT_RST_MGR_REG_RSTSTS_SFTCLR_TYPE                              UInt32
#define IROOT_RST_MGR_REG_RSTSTS_SFTCLR_RESERVED_MASK                     0x7FFFC001
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_PRIV_ACCESS_MODE_SHIFT         31
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_PRIV_ACCESS_MODE_MASK          0x80000000
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_MDMWDRST_DET_V_SHIFT           13
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_MDMWDRST_DET_V_MASK            0x00002000
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_DAPRST_DET_V_SHIFT             12
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_DAPRST_DET_V_MASK              0x00001000
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_HIGH_WARNINGRST_DET_V_SHIFT 11
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_HIGH_WARNINGRST_DET_V_MASK 0x00000800
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_LOW_WARNINGRST_DET_V_SHIFT 10
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_FMON_LOW_WARNINGRST_DET_V_MASK 0x00000400
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDCHKRST_DET_V_SHIFT    9
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDCHKRST_DET_V_MASK     0x00000200
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDCHKRST_DET_V_SHIFT    8
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDCHKRST_DET_V_MASK     0x00000100
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_TMON_WARNINGRST_DET_V_SHIFT    7
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_TMON_WARNINGRST_DET_V_MASK     0x00000080
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDRST_DET_V_SHIFT       6
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_HUB_SECWDRST_DET_V_MASK        0x00000040
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPSFTRST_DET_V_SHIFT         5
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPSFTRST_DET_V_MASK          0x00000020
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDRST_DET_V_SHIFT       4
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_BBL_SECWDRST_DET_V_MASK        0x00000010
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_OPENWDRST_DET_V_SHIFT          3
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_OPENWDRST_DET_V_MASK           0x00000008
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPRST_DET_V_SHIFT            2
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_CHIPRST_DET_V_MASK             0x00000004
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_PORRST_DET_V_SHIFT             1
#define    IROOT_RST_MGR_REG_RSTSTS_SFTCLR_PORRST_DET_V_MASK              0x00000002

#define IROOT_RST_MGR_REG_ESUBWKUP_OFFSET                                 0x00000024
#define IROOT_RST_MGR_REG_ESUBWKUP_TYPE                                   UInt32
#define IROOT_RST_MGR_REG_ESUBWKUP_RESERVED_MASK                          0x7FFFFF00
#define    IROOT_RST_MGR_REG_ESUBWKUP_PRIV_ACCESS_MODE_SHIFT              31
#define    IROOT_RST_MGR_REG_ESUBWKUP_PRIV_ACCESS_MODE_MASK               0x80000000
#define    IROOT_RST_MGR_REG_ESUBWKUP_ESUB_WAKEUP_RST_WIDTH_SHIFT         0
#define    IROOT_RST_MGR_REG_ESUBWKUP_ESUB_WAKEUP_RST_WIDTH_MASK          0x000000FF

#endif /* __BRCM_RDB_IROOT_RST_MGR_REG_H__ */



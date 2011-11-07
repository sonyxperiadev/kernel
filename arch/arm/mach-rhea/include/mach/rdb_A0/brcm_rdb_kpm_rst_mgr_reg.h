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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_KPM_RST_MGR_REG_H__
#define __BRCM_RDB_KPM_RST_MGR_REG_H__

#define KPM_RST_MGR_REG_WR_ACCESS_OFFSET                                  0x00000000
#define KPM_RST_MGR_REG_WR_ACCESS_TYPE                                    UInt32
#define KPM_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                           0x7F0000FE
#define    KPM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT               31
#define    KPM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                0x80000000
#define    KPM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                       8
#define    KPM_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                        0x00FFFF00
#define    KPM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                     0
#define    KPM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                      0x00000001

#define KPM_RST_MGR_REG_AHB_MST_SOFTRST_OFFSET                            0x00000008
#define KPM_RST_MGR_REG_AHB_MST_SOFTRST_TYPE                              UInt32
#define KPM_RST_MGR_REG_AHB_MST_SOFTRST_RESERVED_MASK                     0x7FFFE409
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_PRIV_ACCESS_MODE_SHIFT         31
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_PRIV_ACCESS_MODE_MASK          0x80000000
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_DMA_SOFT_RSTN_SHIFT            12
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_DMA_SOFT_RSTN_MASK             0x00001000
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_PERF_CTRS_SOFT_RSTN_SHIFT      11
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_PERF_CTRS_SOFT_RSTN_MASK       0x00000800
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO3_SOFT_RSTN_SHIFT          9
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO3_SOFT_RSTN_MASK           0x00000200
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO4_SOFT_RSTN_SHIFT          8
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO4_SOFT_RSTN_MASK           0x00000100
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO2_SOFT_RSTN_SHIFT          7
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO2_SOFT_RSTN_MASK           0x00000080
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO1_SOFT_RSTN_SHIFT          6
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_SDIO1_SOFT_RSTN_MASK           0x00000040
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_USB_IC_SOFT_RSTN_SHIFT         5
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_USB_IC_SOFT_RSTN_MASK          0x00000020
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_USBH_SOFT_RSTN_SHIFT           4
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_USBH_SOFT_RSTN_MASK            0x00000010
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_USB_OTG_SOFT_RSTN_SHIFT        2
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_USB_OTG_SOFT_RSTN_MASK         0x00000004
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_NAND_SOFT_RSTN_SHIFT           1
#define    KPM_RST_MGR_REG_AHB_MST_SOFTRST_NAND_SOFT_RSTN_MASK            0x00000002

#define KPM_RST_MGR_REG_AXI_APB0_SOFTRST_OFFSET                           0x00000010
#define KPM_RST_MGR_REG_AXI_APB0_SOFTRST_TYPE                             UInt32
#define KPM_RST_MGR_REG_AXI_APB0_SOFTRST_RESERVED_MASK                    0x7FFFFFFE
#define    KPM_RST_MGR_REG_AXI_APB0_SOFTRST_PRIV_ACCESS_MODE_SHIFT        31
#define    KPM_RST_MGR_REG_AXI_APB0_SOFTRST_PRIV_ACCESS_MODE_MASK         0x80000000
#define    KPM_RST_MGR_REG_AXI_APB0_SOFTRST_ARMCORE_SOFT_RSTN_SHIFT       0
#define    KPM_RST_MGR_REG_AXI_APB0_SOFTRST_ARMCORE_SOFT_RSTN_MASK        0x00000001

#define KPM_RST_MGR_REG_APB8_SOFTRST_OFFSET                               0x00000014
#define KPM_RST_MGR_REG_APB8_SOFTRST_TYPE                                 UInt32
#define KPM_RST_MGR_REG_APB8_SOFTRST_RESERVED_MASK                        0x7FFFFFFE
#define    KPM_RST_MGR_REG_APB8_SOFTRST_PRIV_ACCESS_MODE_SHIFT            31
#define    KPM_RST_MGR_REG_APB8_SOFTRST_PRIV_ACCESS_MODE_MASK             0x80000000
#define    KPM_RST_MGR_REG_APB8_SOFTRST_APB8_SOFT_RSTN_SHIFT              0
#define    KPM_RST_MGR_REG_APB8_SOFTRST_APB8_SOFT_RSTN_MASK               0x00000001

#endif /* __BRCM_RDB_KPM_RST_MGR_REG_H__ */



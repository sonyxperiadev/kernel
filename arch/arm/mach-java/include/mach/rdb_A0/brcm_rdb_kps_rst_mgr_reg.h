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

#ifndef __BRCM_RDB_KPS_RST_MGR_REG_H__
#define __BRCM_RDB_KPS_RST_MGR_REG_H__

#define KPS_RST_MGR_REG_WR_ACCESS_OFFSET                                  0x00000000
#define KPS_RST_MGR_REG_WR_ACCESS_TYPE                                    UInt32
#define KPS_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                           0x7F0000FE
#define    KPS_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT               31
#define    KPS_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                0x80000000
#define    KPS_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                       8
#define    KPS_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                        0x00FFFF00
#define    KPS_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                     0
#define    KPS_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                      0x00000001

#define KPS_RST_MGR_REG_HSM_SOFTRST_OFFSET                                0x00000008
#define KPS_RST_MGR_REG_HSM_SOFTRST_TYPE                                  UInt32
#define KPS_RST_MGR_REG_HSM_SOFTRST_RESERVED_MASK                         0x7FFFFFFC
#define    KPS_RST_MGR_REG_HSM_SOFTRST_PRIV_ACCESS_MODE_SHIFT             31
#define    KPS_RST_MGR_REG_HSM_SOFTRST_PRIV_ACCESS_MODE_MASK              0x80000000
#define    KPS_RST_MGR_REG_HSM_SOFTRST_SPUM_OPEN_SOFT_RSTN_SHIFT          1
#define    KPS_RST_MGR_REG_HSM_SOFTRST_SPUM_OPEN_SOFT_RSTN_MASK           0x00000002
#define    KPS_RST_MGR_REG_HSM_SOFTRST_SPUM_SEC_SOFT_RSTN_SHIFT           0
#define    KPS_RST_MGR_REG_HSM_SOFTRST_SPUM_SEC_SOFT_RSTN_MASK            0x00000001

#define KPS_RST_MGR_REG_APB1_SOFTRST_OFFSET                               0x00000018
#define KPS_RST_MGR_REG_APB1_SOFTRST_TYPE                                 UInt32
#define KPS_RST_MGR_REG_APB1_SOFTRST_RESERVED_MASK                        0x7FFFDFC8
#define    KPS_RST_MGR_REG_APB1_SOFTRST_PRIV_ACCESS_MODE_SHIFT            31
#define    KPS_RST_MGR_REG_APB1_SOFTRST_PRIV_ACCESS_MODE_MASK             0x80000000
#define    KPS_RST_MGR_REG_APB1_SOFTRST_TIMERS_SOFT_RSTN_SHIFT            13
#define    KPS_RST_MGR_REG_APB1_SOFTRST_TIMERS_SOFT_RSTN_MASK             0x00002000
#define    KPS_RST_MGR_REG_APB1_SOFTRST_SSP1_SOFT_RSTN_SHIFT              5
#define    KPS_RST_MGR_REG_APB1_SOFTRST_SSP1_SOFT_RSTN_MASK               0x00000020
#define    KPS_RST_MGR_REG_APB1_SOFTRST_SSP0_SOFT_RSTN_SHIFT              4
#define    KPS_RST_MGR_REG_APB1_SOFTRST_SSP0_SOFT_RSTN_MASK               0x00000010
#define    KPS_RST_MGR_REG_APB1_SOFTRST_UARTB3_SOFT_RSTN_SHIFT            2
#define    KPS_RST_MGR_REG_APB1_SOFTRST_UARTB3_SOFT_RSTN_MASK             0x00000004
#define    KPS_RST_MGR_REG_APB1_SOFTRST_UARTB2_SOFT_RSTN_SHIFT            1
#define    KPS_RST_MGR_REG_APB1_SOFTRST_UARTB2_SOFT_RSTN_MASK             0x00000002
#define    KPS_RST_MGR_REG_APB1_SOFTRST_UARTB_SOFT_RSTN_SHIFT             0
#define    KPS_RST_MGR_REG_APB1_SOFTRST_UARTB_SOFT_RSTN_MASK              0x00000001

#define KPS_RST_MGR_REG_APB2_SOFTRST_OFFSET                               0x00000020
#define KPS_RST_MGR_REG_APB2_SOFTRST_TYPE                                 UInt32
#define KPS_RST_MGR_REG_APB2_SOFTRST_RESERVED_MASK                        0x7FFFCB37
#define    KPS_RST_MGR_REG_APB2_SOFTRST_PRIV_ACCESS_MODE_SHIFT            31
#define    KPS_RST_MGR_REG_APB2_SOFTRST_PRIV_ACCESS_MODE_MASK             0x80000000
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC4_SOFT_RSTN_SHIFT              13
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC4_SOFT_RSTN_MASK               0x00002000
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC3_SOFT_RSTN_SHIFT              12
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC3_SOFT_RSTN_MASK               0x00001000
#define    KPS_RST_MGR_REG_APB2_SOFTRST_PWM_SOFT_RSTN_SHIFT               10
#define    KPS_RST_MGR_REG_APB2_SOFTRST_PWM_SOFT_RSTN_MASK                0x00000400
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC2_SOFT_RSTN_SHIFT              7
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC2_SOFT_RSTN_MASK               0x00000080
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC1_SOFT_RSTN_SHIFT              6
#define    KPS_RST_MGR_REG_APB2_SOFTRST_BSC1_SOFT_RSTN_MASK               0x00000040
#define    KPS_RST_MGR_REG_APB2_SOFTRST_DMAC_MUX_SOFT_RSTN_SHIFT          3
#define    KPS_RST_MGR_REG_APB2_SOFTRST_DMAC_MUX_SOFT_RSTN_MASK           0x00000008

#endif /* __BRCM_RDB_KPS_RST_MGR_REG_H__ */



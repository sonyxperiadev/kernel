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
/*     Date     : Generated on 5/15/2011 11:41:39                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_KHUB_RST_MGR_REG_H__
#define __BRCM_RDB_KHUB_RST_MGR_REG_H__

#define KHUB_RST_MGR_REG_SOFT_RSTN0_OFFSET                                0x00000004
#define KHUB_RST_MGR_REG_SOFT_RSTN0_TYPE                                  UInt32
#define KHUB_RST_MGR_REG_SOFT_RSTN0_RESERVED_MASK                         0x69F0AFFB
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_SHIFT             31
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_MASK              0x80000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SRAM_SOFT_RSTN_SHIFT               28
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SRAM_SOFT_RSTN_MASK                0x10000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_ETB_SOFT_RSTN_SHIFT                26
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_ETB_SOFT_RSTN_MASK                 0x04000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_HSI_SOFT_RSTN_SHIFT                25
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_HSI_SOFT_RSTN_MASK                 0x02000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_7_SOFT_RSTN_SHIFT    19
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_7_SOFT_RSTN_MASK     0x00080000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_5_SOFT_RSTN_SHIFT    18
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SEC_VIOL_TRAP_5_SOFT_RSTN_MASK     0x00040000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_TPIU_SOFT_RSTN_SHIFT               17
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_TPIU_SOFT_RSTN_MASK                0x00020000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_FUNNEL_SOFT_RSTN_SHIFT             16
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_FUNNEL_SOFT_RSTN_MASK              0x00010000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_CTI_SOFT_RSTN_SHIFT                14
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_CTI_SOFT_RSTN_MASK                 0x00004000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SRAM_MPU_SOFT_RSTN_SHIFT           12
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_SRAM_MPU_SOFT_RSTN_MASK            0x00001000
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_BROM_SOFT_RSTN_SHIFT               2
#define    KHUB_RST_MGR_REG_SOFT_RSTN0_BROM_SOFT_RSTN_MASK                0x00000004

#define KHUB_RST_MGR_REG_SOFT_RSTN1_OFFSET                                0x00000008
#define KHUB_RST_MGR_REG_SOFT_RSTN1_TYPE                                  UInt32
#define KHUB_RST_MGR_REG_SOFT_RSTN1_RESERVED_MASK                         0x6F89778F
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_SHIFT             31
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_MASK              0x80000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_STM_SOFT_RSTN_SHIFT                28
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_STM_SOFT_RSTN_MASK                 0x10000000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AUDREG_CTRL_SOFT_RSTN_SHIFT        22
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AUDREG_CTRL_SOFT_RSTN_MASK         0x00400000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_CAPH_INTC_SOFT_RSTN_SHIFT          21
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_CAPH_INTC_SOFT_RSTN_MASK           0x00200000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_CAPH_CNTR_FIFO_SOFT_RSTN_SHIFT     20
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_CAPH_CNTR_FIFO_SOFT_RSTN_MASK      0x00100000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_SHIFT               18
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP4_SOFT_RSTN_MASK                0x00040000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_VAR_SPM_SOFT_RSTN_SHIFT            17
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_VAR_SPM_SOFT_RSTN_MASK             0x00020000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_SHIFT             15
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_AUDIOH_SOFT_RSTN_MASK              0x00008000
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_SHIFT               11
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_SSP3_SOFT_RSTN_MASK                0x00000800
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_ETB2AXI_SOFT_RSTN_SHIFT            6
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_ETB2AXI_SOFT_RSTN_MASK             0x00000040
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_ATB_FILTER_SOFT_RSTN_SHIFT         5
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_ATB_FILTER_SOFT_RSTN_MASK          0x00000020
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_MDIOMASTER_SOFT_RSTN_SHIFT         4
#define    KHUB_RST_MGR_REG_SOFT_RSTN1_MDIOMASTER_SOFT_RSTN_MASK          0x00000010

#endif /* __BRCM_RDB_KHUB_RST_MGR_REG_H__ */



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

#ifndef __BRCM_RDB_MM_RST_MGR_REG_H__
#define __BRCM_RDB_MM_RST_MGR_REG_H__

#define MM_RST_MGR_REG_WR_ACCESS_OFFSET                                   0x00000000
#define MM_RST_MGR_REG_WR_ACCESS_TYPE                                     UInt32
#define MM_RST_MGR_REG_WR_ACCESS_RESERVED_MASK                            0x7F0000FE
#define    MM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT                31
#define    MM_RST_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MM_RST_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                        8
#define    MM_RST_MGR_REG_WR_ACCESS_PASSWORD_MASK                         0x00FFFF00
#define    MM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_SHIFT                      0
#define    MM_RST_MGR_REG_WR_ACCESS_RSTMGR_ACC_MASK                       0x00000001

#define MM_RST_MGR_REG_SOFT_RSTN0_OFFSET                                  0x00000004
#define MM_RST_MGR_REG_SOFT_RSTN0_TYPE                                    UInt32
#define MM_RST_MGR_REG_SOFT_RSTN0_RESERVED_MASK                           0x7FFFFE00
#define    MM_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_SHIFT               31
#define    MM_RST_MGR_REG_SOFT_RSTN0_PRIV_ACCESS_MODE_MASK                0x80000000
#define    MM_RST_MGR_REG_SOFT_RSTN0_AXIPV_SOFT_RSTN_SHIFT                8
#define    MM_RST_MGR_REG_SOFT_RSTN0_AXIPV_SOFT_RSTN_MASK                 0x00000100
#define    MM_RST_MGR_REG_SOFT_RSTN0_H264_SOFT_RSTN_SHIFT                 7
#define    MM_RST_MGR_REG_SOFT_RSTN0_H264_SOFT_RSTN_MASK                  0x00000080
#define    MM_RST_MGR_REG_SOFT_RSTN0_VCE_SOFT_RSTN_SHIFT                  6
#define    MM_RST_MGR_REG_SOFT_RSTN0_VCE_SOFT_RSTN_MASK                   0x00000040
#define    MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_SHIFT                  5
#define    MM_RST_MGR_REG_SOFT_RSTN0_V3D_SOFT_RSTN_MASK                   0x00000020
#define    MM_RST_MGR_REG_SOFT_RSTN0_SMI_SOFT_RSTN_SHIFT                  4
#define    MM_RST_MGR_REG_SOFT_RSTN0_SMI_SOFT_RSTN_MASK                   0x00000010
#define    MM_RST_MGR_REG_SOFT_RSTN0_ISP_SOFT_RSTN_SHIFT                  3
#define    MM_RST_MGR_REG_SOFT_RSTN0_ISP_SOFT_RSTN_MASK                   0x00000008
#define    MM_RST_MGR_REG_SOFT_RSTN0_CSI1_SOFT_RSTN_SHIFT                 2
#define    MM_RST_MGR_REG_SOFT_RSTN0_CSI1_SOFT_RSTN_MASK                  0x00000004
#define    MM_RST_MGR_REG_SOFT_RSTN0_CSI0_SOFT_RSTN_SHIFT                 1
#define    MM_RST_MGR_REG_SOFT_RSTN0_CSI0_SOFT_RSTN_MASK                  0x00000002
#define    MM_RST_MGR_REG_SOFT_RSTN0_MM_DMA_SOFT_RSTN_SHIFT               0
#define    MM_RST_MGR_REG_SOFT_RSTN0_MM_DMA_SOFT_RSTN_MASK                0x00000001

#define MM_RST_MGR_REG_SOFT_RSTN1_OFFSET                                  0x00000008
#define MM_RST_MGR_REG_SOFT_RSTN1_TYPE                                    UInt32
#define MM_RST_MGR_REG_SOFT_RSTN1_RESERVED_MASK                           0x7FFFFFE4
#define    MM_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_SHIFT               31
#define    MM_RST_MGR_REG_SOFT_RSTN1_PRIV_ACCESS_MODE_MASK                0x80000000
#define    MM_RST_MGR_REG_SOFT_RSTN1_PIXELV_SOFT_RSTN_SHIFT               4
#define    MM_RST_MGR_REG_SOFT_RSTN1_PIXELV_SOFT_RSTN_MASK                0x00000010
#define    MM_RST_MGR_REG_SOFT_RSTN1_SPI_SOFT_RSTN_SHIFT                  3
#define    MM_RST_MGR_REG_SOFT_RSTN1_SPI_SOFT_RSTN_MASK                   0x00000008
#define    MM_RST_MGR_REG_SOFT_RSTN1_DSI0_SOFT_RSTN_SHIFT                 1
#define    MM_RST_MGR_REG_SOFT_RSTN1_DSI0_SOFT_RSTN_MASK                  0x00000002
#define    MM_RST_MGR_REG_SOFT_RSTN1_MM_APB_SOFT_RSTN_SHIFT               0
#define    MM_RST_MGR_REG_SOFT_RSTN1_MM_APB_SOFT_RSTN_MASK                0x00000001

#endif /* __BRCM_RDB_MM_RST_MGR_REG_H__ */



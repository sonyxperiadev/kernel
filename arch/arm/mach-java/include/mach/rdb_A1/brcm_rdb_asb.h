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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_ASB_H__
#define __BRCM_RDB_ASB_H__

#define ASB_AXI_BRDG_VERSION_OFFSET                                       0x00000000
#define ASB_AXI_BRDG_VERSION_TYPE                                         UInt32
#define ASB_AXI_BRDG_VERSION_RESERVED_MASK                                0x00000000
#define    ASB_AXI_BRDG_VERSION_ASB_AXI_BRDG_VERSION_SHIFT                0
#define    ASB_AXI_BRDG_VERSION_ASB_AXI_BRDG_VERSION_MASK                 0xFFFFFFFF

#define ASB_H264_S_CTRL_OFFSET                                            0x00000004
#define ASB_H264_S_CTRL_TYPE                                              UInt32
#define ASB_H264_S_CTRL_RESERVED_MASK                                     0xFF000000
#define    ASB_H264_S_CTRL_WCOUNT_SHIFT                                   14
#define    ASB_H264_S_CTRL_WCOUNT_MASK                                    0x00FFC000
#define    ASB_H264_S_CTRL_RCOUNT_SHIFT                                   4
#define    ASB_H264_S_CTRL_RCOUNT_MASK                                    0x00003FF0
#define    ASB_H264_S_CTRL_FULL_SHIFT                                     3
#define    ASB_H264_S_CTRL_FULL_MASK                                      0x00000008
#define    ASB_H264_S_CTRL_EMPTY_SHIFT                                    2
#define    ASB_H264_S_CTRL_EMPTY_MASK                                     0x00000004
#define    ASB_H264_S_CTRL_CLR_ACK_SHIFT                                  1
#define    ASB_H264_S_CTRL_CLR_ACK_MASK                                   0x00000002
#define    ASB_H264_S_CTRL_CLR_REQ_SHIFT                                  0
#define    ASB_H264_S_CTRL_CLR_REQ_MASK                                   0x00000001

#define ASB_H264_M_CTRL_OFFSET                                            0x00000008
#define ASB_H264_M_CTRL_TYPE                                              UInt32
#define ASB_H264_M_CTRL_RESERVED_MASK                                     0xFF000000
#define    ASB_H264_M_CTRL_WCOUNT_SHIFT                                   14
#define    ASB_H264_M_CTRL_WCOUNT_MASK                                    0x00FFC000
#define    ASB_H264_M_CTRL_RCOUNT_SHIFT                                   4
#define    ASB_H264_M_CTRL_RCOUNT_MASK                                    0x00003FF0
#define    ASB_H264_M_CTRL_FULL_SHIFT                                     3
#define    ASB_H264_M_CTRL_FULL_MASK                                      0x00000008
#define    ASB_H264_M_CTRL_EMPTY_SHIFT                                    2
#define    ASB_H264_M_CTRL_EMPTY_MASK                                     0x00000004
#define    ASB_H264_M_CTRL_CLR_ACK_SHIFT                                  1
#define    ASB_H264_M_CTRL_CLR_ACK_MASK                                   0x00000002
#define    ASB_H264_M_CTRL_CLR_REQ_SHIFT                                  0
#define    ASB_H264_M_CTRL_CLR_REQ_MASK                                   0x00000001

#endif /* __BRCM_RDB_ASB_H__ */



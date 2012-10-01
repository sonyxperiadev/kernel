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

#ifndef __BRCM_RDB_RNG_H__
#define __BRCM_RDB_RNG_H__

#define RNG_CTRL_OFFSET                                                   0x00000000
#define RNG_CTRL_TYPE                                                     UInt32
#define RNG_CTRL_RESERVED_MASK                                            0xF00000CC
#define    RNG_CTRL_RNG_COMBLK2_OSC_DIS_SHIFT                             22
#define    RNG_CTRL_RNG_COMBLK2_OSC_DIS_MASK                              0x0FC00000
#define    RNG_CTRL_RNG_COMBLK1_OSC_DIS_SHIFT                             16
#define    RNG_CTRL_RNG_COMBLK1_OSC_DIS_MASK                              0x003F0000
#define    RNG_CTRL_RNG_JCLK_BYP_DIV_CNT_SHIFT                            8
#define    RNG_CTRL_RNG_JCLK_BYP_DIV_CNT_MASK                             0x0000FF00
#define    RNG_CTRL_RNG_JCLK_BYP_SRC_SHIFT                                5
#define    RNG_CTRL_RNG_JCLK_BYP_SRC_MASK                                 0x00000020
#define    RNG_CTRL_RNG_JCLK_BYP_SEL_SHIFT                                4
#define    RNG_CTRL_RNG_JCLK_BYP_SEL_MASK                                 0x00000010
#define    RNG_CTRL_RNG_RBG2X_SHIFT                                       1
#define    RNG_CTRL_RNG_RBG2X_MASK                                        0x00000002
#define    RNG_CTRL_RNG_RBGEN_SHIFT                                       0
#define    RNG_CTRL_RNG_RBGEN_MASK                                        0x00000001

#define RNG_STATUS_OFFSET                                                 0x00000004
#define RNG_STATUS_TYPE                                                   UInt32
#define RNG_STATUS_RESERVED_MASK                                          0x00F00000
#define    RNG_STATUS_RND_VAL_SHIFT                                       24
#define    RNG_STATUS_RND_VAL_MASK                                        0xFF000000
#define    RNG_STATUS_RNG_WARM_CNT_SHIFT                                  0
#define    RNG_STATUS_RNG_WARM_CNT_MASK                                   0x000FFFFF

#define RNG_DATA_OFFSET                                                   0x00000008
#define RNG_DATA_TYPE                                                     UInt32
#define RNG_DATA_RESERVED_MASK                                            0x00000000
#define    RNG_DATA_RNG_NUM_SHIFT                                         0
#define    RNG_DATA_RNG_NUM_MASK                                          0xFFFFFFFF

#define RNG_FF_THRES_OFFSET                                               0x0000000C
#define RNG_FF_THRES_TYPE                                                 UInt32
#define RNG_FF_THRES_RESERVED_MASK                                        0xFFFFFFE0
#define    RNG_FF_THRES_RNG_FF_THRESH_SHIFT                               0
#define    RNG_FF_THRES_RNG_FF_THRESH_MASK                                0x0000001F

#define RNG_INT_MASK_OFFSET                                               0x00000010
#define RNG_INT_MASK_TYPE                                                 UInt32
#define RNG_INT_MASK_RESERVED_MASK                                        0xFFFFFFFE
#define    RNG_INT_MASK_RNG_INT_OFF_SHIFT                                 0
#define    RNG_INT_MASK_RNG_INT_OFF_MASK                                  0x00000001

#endif /* __BRCM_RDB_RNG_H__ */



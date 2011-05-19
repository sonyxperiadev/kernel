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

#ifndef __BRCM_RDB_BBL_VMON_H__
#define __BRCM_RDB_BBL_VMON_H__

#define BBL_VMON_VMON_INT_STS_OFFSET                                      0x00000000
#define BBL_VMON_VMON_INT_STS_TYPE                                        UInt32
#define BBL_VMON_VMON_INT_STS_RESERVED_MASK                               0xFFFFFFFC
#define    BBL_VMON_VMON_INT_STS_VMON_HIGH_INT_STS_SHIFT                  1
#define    BBL_VMON_VMON_INT_STS_VMON_HIGH_INT_STS_MASK                   0x00000002
#define    BBL_VMON_VMON_INT_STS_VMON_LOW_INT_STS_SHIFT                   0
#define    BBL_VMON_VMON_INT_STS_VMON_LOW_INT_STS_MASK                    0x00000001

#define BBL_VMON_VMON_INT_EN_OFFSET                                       0x00000004
#define BBL_VMON_VMON_INT_EN_TYPE                                         UInt32
#define BBL_VMON_VMON_INT_EN_RESERVED_MASK                                0xFFFFFFFC
#define    BBL_VMON_VMON_INT_EN_VMON_HIGH_INT_EN_SHIFT                    1
#define    BBL_VMON_VMON_INT_EN_VMON_HIGH_INT_EN_MASK                     0x00000002
#define    BBL_VMON_VMON_INT_EN_VMON_LOW_INT_EN_SHIFT                     0
#define    BBL_VMON_VMON_INT_EN_VMON_LOW_INT_EN_MASK                      0x00000001

#endif /* __BRCM_RDB_BBL_VMON_H__ */



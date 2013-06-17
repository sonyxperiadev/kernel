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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_FSCLKCAL_H__
#define __BRCM_RDB_FSCLKCAL_H__

#define FSCLKCAL_START_OFFSET                                             0x00000000
#define FSCLKCAL_START_TYPE                                               UInt32
#define FSCLKCAL_START_RESERVED_MASK                                      0xFFFFFFFE
#define    FSCLKCAL_START_START_SHIFT                                     0
#define    FSCLKCAL_START_START_MASK                                      0x00000001

#define FSCLKCAL_ABORT_OFFSET                                             0x00000004
#define FSCLKCAL_ABORT_TYPE                                               UInt32
#define FSCLKCAL_ABORT_RESERVED_MASK                                      0xFFFFFFFE
#define    FSCLKCAL_ABORT_ABORT_SHIFT                                     0
#define    FSCLKCAL_ABORT_ABORT_MASK                                      0x00000001

#define FSCLKCAL_INT_CLR_OFFSET                                           0x00000008
#define FSCLKCAL_INT_CLR_TYPE                                             UInt32
#define FSCLKCAL_INT_CLR_RESERVED_MASK                                    0xFFFFFFFE
#define    FSCLKCAL_INT_CLR_INT_CLR_SHIFT                                 0
#define    FSCLKCAL_INT_CLR_INT_CLR_MASK                                  0x00000001

#define FSCLKCAL_SCLK_CMP_OFFSET                                          0x0000000C
#define FSCLKCAL_SCLK_CMP_TYPE                                            UInt32
#define FSCLKCAL_SCLK_CMP_RESERVED_MASK                                   0xFFFC0000
#define    FSCLKCAL_SCLK_CMP_SCLK_CMP_SHIFT                               0
#define    FSCLKCAL_SCLK_CMP_SCLK_CMP_MASK                                0x0003FFFF

#define FSCLKCAL_FCLK_CNT_OFFSET                                          0x00000010
#define FSCLKCAL_FCLK_CNT_TYPE                                            UInt32
#define FSCLKCAL_FCLK_CNT_RESERVED_MASK                                   0xC0000000
#define    FSCLKCAL_FCLK_CNT_FCLK_CNT_SHIFT                               0
#define    FSCLKCAL_FCLK_CNT_FCLK_CNT_MASK                                0x3FFFFFFF

#define FSCLKCAL_SCLK_CNT_OFFSET                                          0x00000014
#define FSCLKCAL_SCLK_CNT_TYPE                                            UInt32
#define FSCLKCAL_SCLK_CNT_RESERVED_MASK                                   0xFFFC0000
#define    FSCLKCAL_SCLK_CNT_SCLK_CNT_SHIFT                               0
#define    FSCLKCAL_SCLK_CNT_SCLK_CNT_MASK                                0x0003FFFF

#define FSCLKCAL_STATUS_OFFSET                                            0x00000018
#define FSCLKCAL_STATUS_TYPE                                              UInt32
#define FSCLKCAL_STATUS_RESERVED_MASK                                     0xFFFFFFFE
#define    FSCLKCAL_STATUS_STATUS_SHIFT                                   0
#define    FSCLKCAL_STATUS_STATUS_MASK                                    0x00000001

#endif /* __BRCM_RDB_FSCLKCAL_H__ */



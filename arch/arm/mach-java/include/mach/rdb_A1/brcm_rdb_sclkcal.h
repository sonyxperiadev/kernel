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

#ifndef __BRCM_RDB_SCLKCAL_H__
#define __BRCM_RDB_SCLKCAL_H__

#define SCLKCAL_CAFR_OFFSET                                               0x00000000
#define SCLKCAL_CAFR_TYPE                                                 UInt32
#define SCLKCAL_CAFR_RESERVED_MASK                                        0xFE000000
#define    SCLKCAL_CAFR_CAFR_SHIFT                                        0
#define    SCLKCAL_CAFR_CAFR_MASK                                         0x01FFFFFF

#define SCLKCAL_CASR_OFFSET                                               0x00000004
#define SCLKCAL_CASR_TYPE                                                 UInt32
#define SCLKCAL_CASR_RESERVED_MASK                                        0xFFF00000
#define    SCLKCAL_CASR_CASR_SHIFT                                        0
#define    SCLKCAL_CASR_CASR_MASK                                         0x000FFFFF

#define SCLKCAL_CACMP_OFFSET                                              0x00000008
#define SCLKCAL_CACMP_TYPE                                                UInt32
#define SCLKCAL_CACMP_RESERVED_MASK                                       0xFFFF0000
#define    SCLKCAL_CACMP_MODE13MHZ_SHIFT                                  15
#define    SCLKCAL_CACMP_MODE13MHZ_MASK                                   0x00008000
#define    SCLKCAL_CACMP_CACMP_SHIFT                                      0
#define    SCLKCAL_CACMP_CACMP_MASK                                       0x00007FFF

#define SCLKCAL_CACTRL_OFFSET                                             0x0000000C
#define SCLKCAL_CACTRL_TYPE                                               UInt32
#define SCLKCAL_CACTRL_RESERVED_MASK                                      0xFFFFFCFF
#define    SCLKCAL_CACTRL_CASTAT_SHIFT                                    9
#define    SCLKCAL_CACTRL_CASTAT_MASK                                     0x00000200
#define    SCLKCAL_CACTRL_CAINIT_SHIFT                                    8
#define    SCLKCAL_CACTRL_CAINIT_MASK                                     0x00000100

#endif /* __BRCM_RDB_SCLKCAL_H__ */



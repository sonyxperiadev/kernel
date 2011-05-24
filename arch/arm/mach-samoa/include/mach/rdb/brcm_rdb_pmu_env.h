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

#ifndef __BRCM_RDB_PMU_ENV_H__
#define __BRCM_RDB_PMU_ENV_H__

#define PMU_ENV_PMUID_OFFSET                                              0x00000000
#define PMU_ENV_PMUID_TYPE                                                UInt32
#define PMU_ENV_PMUID_RESERVED_MASK                                       0xFFFFFF00
#define    PMU_ENV_PMUID_PMUID_7_0__SHIFT                                 0
#define    PMU_ENV_PMUID_PMUID_7_0__MASK                                  0x000000FF

#define PMU_ENV_PMUREV_OFFSET                                             0x00000004
#define PMU_ENV_PMUREV_TYPE                                               UInt32
#define PMU_ENV_PMUREV_RESERVED_MASK                                      0xFFFFFF00
#define    PMU_ENV_PMUREV_ANA_REV_3_0__SHIFT                              4
#define    PMU_ENV_PMUREV_ANA_REV_3_0__MASK                               0x000000F0
#define    PMU_ENV_PMUREV_DIG_REV_3_0__SHIFT                              0
#define    PMU_ENV_PMUREV_DIG_REV_3_0__MASK                               0x0000000F

#endif /* __BRCM_RDB_PMU_ENV_H__ */



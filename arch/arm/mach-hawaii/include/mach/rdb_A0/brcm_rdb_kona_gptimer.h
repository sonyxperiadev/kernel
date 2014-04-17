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

#ifndef __BRCM_RDB_KONA_GPTIMER_H__
#define __BRCM_RDB_KONA_GPTIMER_H__

#define KONA_GPTIMER_STCS_OFFSET                                          0x00000000
#define KONA_GPTIMER_STCS_TYPE                                            UInt32
#define KONA_GPTIMER_STCS_RESERVED_MASK                                   0x00000000
#define    KONA_GPTIMER_STCS_NOT_USED_SHIFT                               16
#define    KONA_GPTIMER_STCS_NOT_USED_MASK                                0xFFFF0000
#define    KONA_GPTIMER_STCS_STCM3_SYNC_SHIFT                             15
#define    KONA_GPTIMER_STCS_STCM3_SYNC_MASK                              0x00008000
#define    KONA_GPTIMER_STCS_STCM2_SYNC_SHIFT                             14
#define    KONA_GPTIMER_STCS_STCM2_SYNC_MASK                              0x00004000
#define    KONA_GPTIMER_STCS_STCM1_SYNC_SHIFT                             13
#define    KONA_GPTIMER_STCS_STCM1_SYNC_MASK                              0x00002000
#define    KONA_GPTIMER_STCS_STCM0_SYNC_SHIFT                             12
#define    KONA_GPTIMER_STCS_STCM0_SYNC_MASK                              0x00001000
#define    KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_SHIFT                    8
#define    KONA_GPTIMER_STCS_COMPARE_ENABLE_SYNC_MASK                     0x00000F00
#define    KONA_GPTIMER_STCS_COMPARE_ENABLE_SHIFT                         4
#define    KONA_GPTIMER_STCS_COMPARE_ENABLE_MASK                          0x000000F0
#define    KONA_GPTIMER_STCS_TIMER_MATCH_SHIFT                            0
#define    KONA_GPTIMER_STCS_TIMER_MATCH_MASK                             0x0000000F

#define KONA_GPTIMER_STCLO_OFFSET                                         0x00000004
#define KONA_GPTIMER_STCLO_TYPE                                           UInt32
#define KONA_GPTIMER_STCLO_RESERVED_MASK                                  0x00000000
#define    KONA_GPTIMER_STCLO_STCLO_SHIFT                                 0
#define    KONA_GPTIMER_STCLO_STCLO_MASK                                  0xFFFFFFFF

#define KONA_GPTIMER_STCHI_OFFSET                                         0x00000008
#define KONA_GPTIMER_STCHI_TYPE                                           UInt32
#define KONA_GPTIMER_STCHI_RESERVED_MASK                                  0x00000000
#define    KONA_GPTIMER_STCHI_STCHI_SHIFT                                 0
#define    KONA_GPTIMER_STCHI_STCHI_MASK                                  0xFFFFFFFF

#define KONA_GPTIMER_STCM0_OFFSET                                         0x0000000C
#define KONA_GPTIMER_STCM0_TYPE                                           UInt32
#define KONA_GPTIMER_STCM0_RESERVED_MASK                                  0x00000000
#define    KONA_GPTIMER_STCM0_STCM0_SHIFT                                 0
#define    KONA_GPTIMER_STCM0_STCM0_MASK                                  0xFFFFFFFF

#define KONA_GPTIMER_STCM1_OFFSET                                         0x00000010
#define KONA_GPTIMER_STCM1_TYPE                                           UInt32
#define KONA_GPTIMER_STCM1_RESERVED_MASK                                  0x00000000
#define    KONA_GPTIMER_STCM1_STCM1_SHIFT                                 0
#define    KONA_GPTIMER_STCM1_STCM1_MASK                                  0xFFFFFFFF

#define KONA_GPTIMER_STCM2_OFFSET                                         0x00000014
#define KONA_GPTIMER_STCM2_TYPE                                           UInt32
#define KONA_GPTIMER_STCM2_RESERVED_MASK                                  0x00000000
#define    KONA_GPTIMER_STCM2_STCM2_SHIFT                                 0
#define    KONA_GPTIMER_STCM2_STCM2_MASK                                  0xFFFFFFFF

#define KONA_GPTIMER_STCM3_OFFSET                                         0x00000018
#define KONA_GPTIMER_STCM3_TYPE                                           UInt32
#define KONA_GPTIMER_STCM3_RESERVED_MASK                                  0x00000000
#define    KONA_GPTIMER_STCM3_STCM3_SHIFT                                 0
#define    KONA_GPTIMER_STCM3_STCM3_MASK                                  0xFFFFFFFF

#endif /* __BRCM_RDB_KONA_GPTIMER_H__ */



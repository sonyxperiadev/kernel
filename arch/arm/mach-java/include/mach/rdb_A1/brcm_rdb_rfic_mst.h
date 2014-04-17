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

#ifndef __BRCM_RDB_RFIC_MST_H__
#define __BRCM_RDB_RFIC_MST_H__

#define RFIC_MST_SAMPLES_OFFSET                                           0x00000000
#define RFIC_MST_SAMPLES_TYPE                                             UInt32
#define RFIC_MST_SAMPLES_RESERVED_MASK                                    0xE0040000
#define    RFIC_MST_SAMPLES_INDEX_SHIFT                                   23
#define    RFIC_MST_SAMPLES_INDEX_MASK                                    0x1F800000
#define    RFIC_MST_SAMPLES_INDEX_RO_SHIFT                                20
#define    RFIC_MST_SAMPLES_INDEX_RO_MASK                                 0x00700000
#define    RFIC_MST_SAMPLES_SLEEP_AWAKEN_SHIFT                            19
#define    RFIC_MST_SAMPLES_SLEEP_AWAKEN_MASK                             0x00080000
#define    RFIC_MST_SAMPLES_SAMPLES_R1_SHIFT                              0
#define    RFIC_MST_SAMPLES_SAMPLES_R1_MASK                               0x0003FFFF

#define RFIC_MST_ACCU_OFFSET                                              0x00000004
#define RFIC_MST_ACCU_TYPE                                                UInt32
#define RFIC_MST_ACCU_RESERVED_MASK                                       0x00000000
#define    RFIC_MST_ACCU_ACCU_SHIFT                                       0
#define    RFIC_MST_ACCU_ACCU_MASK                                        0xFFFFFFFF

#define RFIC_MST_FREEZE_OFFSET                                            0x00000008
#define RFIC_MST_FREEZE_TYPE                                              UInt32
#define RFIC_MST_FREEZE_RESERVED_MASK                                     0xFFFFEEEE
#define    RFIC_MST_FREEZE_ACTIVE_SHIFT                                   12
#define    RFIC_MST_FREEZE_ACTIVE_MASK                                    0x00001000
#define    RFIC_MST_FREEZE_SLOT_EN_SHIFT                                  8
#define    RFIC_MST_FREEZE_SLOT_EN_MASK                                   0x00000100
#define    RFIC_MST_FREEZE_SYNC_SHIFT                                     4
#define    RFIC_MST_FREEZE_SYNC_MASK                                      0x00000010
#define    RFIC_MST_FREEZE_FREEZE_SHIFT                                   0
#define    RFIC_MST_FREEZE_FREEZE_MASK                                    0x00000001

#endif /* __BRCM_RDB_RFIC_MST_H__ */



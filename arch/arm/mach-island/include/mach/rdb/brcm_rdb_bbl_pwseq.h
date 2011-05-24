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

#ifndef __BRCM_RDB_BBL_PWSEQ_H__
#define __BRCM_RDB_BBL_PWSEQ_H__

#define BBL_PWSEQ_PSRST_OFFSET                                            0x00000000
#define BBL_PWSEQ_PSRST_TYPE                                              UInt32
#define BBL_PWSEQ_PSRST_RESERVED_MASK                                     0x00000000
#define    BBL_PWSEQ_PSRST_PSRST_UNUSED_SHIFT                             9
#define    BBL_PWSEQ_PSRST_PSRST_UNUSED_MASK                              0xFFFFFE00
#define    BBL_PWSEQ_PSRST_TR_SHIFT                                       8
#define    BBL_PWSEQ_PSRST_TR_MASK                                        0x00000100
#define    BBL_PWSEQ_PSRST_TS_SHIFT                                       7
#define    BBL_PWSEQ_PSRST_TS_MASK                                        0x00000080
#define    BBL_PWSEQ_PSRST_R2_SHIFT                                       6
#define    BBL_PWSEQ_PSRST_R2_MASK                                        0x00000040
#define    BBL_PWSEQ_PSRST_R1_SHIFT                                       5
#define    BBL_PWSEQ_PSRST_R1_MASK                                        0x00000020
#define    BBL_PWSEQ_PSRST_R0_SHIFT                                       4
#define    BBL_PWSEQ_PSRST_R0_MASK                                        0x00000010
#define    BBL_PWSEQ_PSRST_RS_SHIFT                                       3
#define    BBL_PWSEQ_PSRST_RS_MASK                                        0x00000008
#define    BBL_PWSEQ_PSRST_ER2_SHIFT                                      2
#define    BBL_PWSEQ_PSRST_ER2_MASK                                       0x00000004
#define    BBL_PWSEQ_PSRST_ER1_SHIFT                                      1
#define    BBL_PWSEQ_PSRST_ER1_MASK                                       0x00000002
#define    BBL_PWSEQ_PSRST_ER0_SHIFT                                      0
#define    BBL_PWSEQ_PSRST_ER0_MASK                                       0x00000001

#define BBL_PWSEQ_DLY1_OFFSET                                             0x00000004
#define BBL_PWSEQ_DLY1_TYPE                                               UInt32
#define BBL_PWSEQ_DLY1_RESERVED_MASK                                      0x00000000
#define    BBL_PWSEQ_DLY1_BBL_DLY1_UNUSED_SHIFT                           8
#define    BBL_PWSEQ_DLY1_BBL_DLY1_UNUSED_MASK                            0xFFFFFF00
#define    BBL_PWSEQ_DLY1_BBL_DLY1_SHIFT                                  0
#define    BBL_PWSEQ_DLY1_BBL_DLY1_MASK                                   0x000000FF

#define BBL_PWSEQ_DLY2_OFFSET                                             0x00000008
#define BBL_PWSEQ_DLY2_TYPE                                               UInt32
#define BBL_PWSEQ_DLY2_RESERVED_MASK                                      0x00000000
#define    BBL_PWSEQ_DLY2_BBL_DLY2_UNUSED_SHIFT                           8
#define    BBL_PWSEQ_DLY2_BBL_DLY2_UNUSED_MASK                            0xFFFFFF00
#define    BBL_PWSEQ_DLY2_BBL_DLY2_SHIFT                                  0
#define    BBL_PWSEQ_DLY2_BBL_DLY2_MASK                                   0x000000FF

#define BBL_PWSEQ_TIMEOUT_OFFSET                                          0x0000000C
#define BBL_PWSEQ_TIMEOUT_TYPE                                            UInt32
#define BBL_PWSEQ_TIMEOUT_RESERVED_MASK                                   0x00000000
#define    BBL_PWSEQ_TIMEOUT_TIMEOUT_UNUSED_SHIFT                         15
#define    BBL_PWSEQ_TIMEOUT_TIMEOUT_UNUSED_MASK                          0xFFFF8000
#define    BBL_PWSEQ_TIMEOUT_PWREN_POLARITY_SHIFT                         14
#define    BBL_PWSEQ_TIMEOUT_PWREN_POLARITY_MASK                          0x00004000
#define    BBL_PWSEQ_TIMEOUT_ALARM_OUT_POLARITY_SHIFT                     13
#define    BBL_PWSEQ_TIMEOUT_ALARM_OUT_POLARITY_MASK                      0x00002000
#define    BBL_PWSEQ_TIMEOUT_ALARM_OUT_ENABLE_SHIFT                       12
#define    BBL_PWSEQ_TIMEOUT_ALARM_OUT_ENABLE_MASK                        0x00001000
#define    BBL_PWSEQ_TIMEOUT_BBL_TIMEOUT_UNUSED_SHIFT                     9
#define    BBL_PWSEQ_TIMEOUT_BBL_TIMEOUT_UNUSED_MASK                      0x00000E00
#define    BBL_PWSEQ_TIMEOUT_BBL_TIMEOUT_SHIFT                            0
#define    BBL_PWSEQ_TIMEOUT_BBL_TIMEOUT_MASK                             0x000001FF

#define BBL_PWSEQ_EVENT0_OFFSET                                           0x00000010
#define BBL_PWSEQ_EVENT0_TYPE                                             UInt32
#define BBL_PWSEQ_EVENT0_RESERVED_MASK                                    0x00000000
#define    BBL_PWSEQ_EVENT0_BBL_EVENT0_UNUSED_SHIFT                       4
#define    BBL_PWSEQ_EVENT0_BBL_EVENT0_UNUSED_MASK                        0xFFFFFFF0
#define    BBL_PWSEQ_EVENT0_BBL_EVENT0_SHIFT                              0
#define    BBL_PWSEQ_EVENT0_BBL_EVENT0_MASK                               0x0000000F

#define BBL_PWSEQ_EVENT1_OFFSET                                           0x00000014
#define BBL_PWSEQ_EVENT1_TYPE                                             UInt32
#define BBL_PWSEQ_EVENT1_RESERVED_MASK                                    0x00000000
#define    BBL_PWSEQ_EVENT1_BBL_EVENT1_UNUSED_SHIFT                       4
#define    BBL_PWSEQ_EVENT1_BBL_EVENT1_UNUSED_MASK                        0xFFFFFFF0
#define    BBL_PWSEQ_EVENT1_BBL_EVENT1_SHIFT                              0
#define    BBL_PWSEQ_EVENT1_BBL_EVENT1_MASK                               0x0000000F

#define BBL_PWSEQ_EVENT2_OFFSET                                           0x00000018
#define BBL_PWSEQ_EVENT2_TYPE                                             UInt32
#define BBL_PWSEQ_EVENT2_RESERVED_MASK                                    0x00000000
#define    BBL_PWSEQ_EVENT2_BBL_EVENT2_UNUSED_SHIFT                       4
#define    BBL_PWSEQ_EVENT2_BBL_EVENT2_UNUSED_MASK                        0xFFFFFFF0
#define    BBL_PWSEQ_EVENT2_BBL_EVENT2_SHIFT                              0
#define    BBL_PWSEQ_EVENT2_BBL_EVENT2_MASK                               0x0000000F

#define BBL_PWSEQ_TRIG_STAT_OFFSET                                        0x0000001C
#define BBL_PWSEQ_TRIG_STAT_TYPE                                          UInt32
#define BBL_PWSEQ_TRIG_STAT_RESERVED_MASK                                 0x00000000
#define    BBL_PWSEQ_TRIG_STAT_BBL_PWSEQ_TRIG_STAT_UNUSED_SHIFT           5
#define    BBL_PWSEQ_TRIG_STAT_BBL_PWSEQ_TRIG_STAT_UNUSED_MASK            0xFFFFFFE0
#define    BBL_PWSEQ_TRIG_STAT_BBL_TRIG_STAT_SHIFT                        0
#define    BBL_PWSEQ_TRIG_STAT_BBL_TRIG_STAT_MASK                         0x0000001F

#define BBL_PWSEQ_ENABLE_OFFSET                                           0x00000020
#define BBL_PWSEQ_ENABLE_TYPE                                             UInt32
#define BBL_PWSEQ_ENABLE_RESERVED_MASK                                    0x00000000
#define    BBL_PWSEQ_ENABLE_BBL_PWSEQ_MASK_UNUSED_SHIFT                   5
#define    BBL_PWSEQ_ENABLE_BBL_PWSEQ_MASK_UNUSED_MASK                    0xFFFFFFE0
#define    BBL_PWSEQ_ENABLE_BBL_RTC_PER_TRIG_EN_SHIFT                     4
#define    BBL_PWSEQ_ENABLE_BBL_RTC_PER_TRIG_EN_MASK                      0x00000010
#define    BBL_PWSEQ_ENABLE_BBL_RTC_MATCH_TRIG_EN_SHIFT                   3
#define    BBL_PWSEQ_ENABLE_BBL_RTC_MATCH_TRIG_EN_MASK                    0x00000008
#define    BBL_PWSEQ_ENABLE_BBL_TRIG2_EN_SHIFT                            2
#define    BBL_PWSEQ_ENABLE_BBL_TRIG2_EN_MASK                             0x00000004
#define    BBL_PWSEQ_ENABLE_BBL_TRIG1_EN_SHIFT                            1
#define    BBL_PWSEQ_ENABLE_BBL_TRIG1_EN_MASK                             0x00000002
#define    BBL_PWSEQ_ENABLE_BBL_TRIG0_EN_SHIFT                            0
#define    BBL_PWSEQ_ENABLE_BBL_TRIG0_EN_MASK                             0x00000001

#endif /* __BRCM_RDB_BBL_PWSEQ_H__ */



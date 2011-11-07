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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_PWRWDOG_H__
#define __BRCM_RDB_PWRWDOG_H__

#define PWRWDOG_PWRWDOG_CTRL_OFFSET                                       0x00000000
#define PWRWDOG_PWRWDOG_CTRL_TYPE                                         UInt32
#define PWRWDOG_PWRWDOG_CTRL_RESERVED_MASK                                0xFF000000
#define    PWRWDOG_PWRWDOG_CTRL_INTLVL_SHIFT                              23
#define    PWRWDOG_PWRWDOG_CTRL_INTLVL_MASK                               0x00800000
#define    PWRWDOG_PWRWDOG_CTRL_INTEN_SHIFT                               22
#define    PWRWDOG_PWRWDOG_CTRL_INTEN_MASK                                0x00400000
#define    PWRWDOG_PWRWDOG_CTRL_MODE_SHIFT                                21
#define    PWRWDOG_PWRWDOG_CTRL_MODE_MASK                                 0x00200000
#define    PWRWDOG_PWRWDOG_CTRL_START_SHIFT                               15
#define    PWRWDOG_PWRWDOG_CTRL_START_MASK                                0x001F8000
#define    PWRWDOG_PWRWDOG_CTRL_CGFG_SHIFT                                7
#define    PWRWDOG_PWRWDOG_CTRL_CGFG_MASK                                 0x00007F80
#define    PWRWDOG_PWRWDOG_CTRL_RSEL_SHIFT                                4
#define    PWRWDOG_PWRWDOG_CTRL_RSEL_MASK                                 0x00000070
#define    PWRWDOG_PWRWDOG_CTRL_CLRCFG_SHIFT                              1
#define    PWRWDOG_PWRWDOG_CTRL_CLRCFG_MASK                               0x0000000E
#define    PWRWDOG_PWRWDOG_CTRL_EN_SHIFT                                  0
#define    PWRWDOG_PWRWDOG_CTRL_EN_MASK                                   0x00000001

#define PWRWDOG_PWRWDOG_STAT_OFFSET                                       0x00000004
#define PWRWDOG_PWRWDOG_STAT_TYPE                                         UInt32
#define PWRWDOG_PWRWDOG_STAT_RESERVED_MASK                                0xFFFFFFFE
#define    PWRWDOG_PWRWDOG_STAT_PWRWDOG_STAT_SHIFT                        0
#define    PWRWDOG_PWRWDOG_STAT_PWRWDOG_STAT_MASK                         0x00000001

#define PWRWDOG_PWRWDOG_DLYEN_CNT_OFFSET                                  0x00000008
#define PWRWDOG_PWRWDOG_DLYEN_CNT_TYPE                                    UInt32
#define PWRWDOG_PWRWDOG_DLYEN_CNT_RESERVED_MASK                           0x00000000
#define    PWRWDOG_PWRWDOG_DLYEN_CNT_PWRWDOG_CNT_SHIFT                    0
#define    PWRWDOG_PWRWDOG_DLYEN_CNT_PWRWDOG_CNT_MASK                     0xFFFFFFFF

#define PWRWDOG_PWRWDOG_STAT_CNT_OFFSET                                   0x0000000C
#define PWRWDOG_PWRWDOG_STAT_CNT_TYPE                                     UInt32
#define PWRWDOG_PWRWDOG_STAT_CNT_RESERVED_MASK                            0x00000000
#define    PWRWDOG_PWRWDOG_STAT_CNT_PWRWDOG_CNT_SHIFT                     0
#define    PWRWDOG_PWRWDOG_STAT_CNT_PWRWDOG_CNT_MASK                      0xFFFFFFFF

#define PWRWDOG_PWRWDOG_ACCU_CTRL_OFFSET                                  0x00000010
#define PWRWDOG_PWRWDOG_ACCU_CTRL_TYPE                                    UInt32
#define PWRWDOG_PWRWDOG_ACCU_CTRL_RESERVED_MASK                           0xFFFFFFE0
#define    PWRWDOG_PWRWDOG_ACCU_CTRL_SKIP_START_SHIFT                     3
#define    PWRWDOG_PWRWDOG_ACCU_CTRL_SKIP_START_MASK                      0x00000018
#define    PWRWDOG_PWRWDOG_ACCU_CTRL_SKIP_SHIFT                           1
#define    PWRWDOG_PWRWDOG_ACCU_CTRL_SKIP_MASK                            0x00000006
#define    PWRWDOG_PWRWDOG_ACCU_CTRL_RUN_SHIFT                            0
#define    PWRWDOG_PWRWDOG_ACCU_CTRL_RUN_MASK                             0x00000001

#define PWRWDOG_PWRWDOG_ACCU_STAT_OFFSET                                  0x00000014
#define PWRWDOG_PWRWDOG_ACCU_STAT_TYPE                                    UInt32
#define PWRWDOG_PWRWDOG_ACCU_STAT_RESERVED_MASK                           0xFFFFF000
#define    PWRWDOG_PWRWDOG_ACCU_STAT_GOOD_WAS_LOW_SHIFT                   11
#define    PWRWDOG_PWRWDOG_ACCU_STAT_GOOD_WAS_LOW_MASK                    0x00000800
#define    PWRWDOG_PWRWDOG_ACCU_STAT_DONE_SHIFT                           10
#define    PWRWDOG_PWRWDOG_ACCU_STAT_DONE_MASK                            0x00000400
#define    PWRWDOG_PWRWDOG_ACCU_STAT_SUM_GOOD_SHIFT                       0
#define    PWRWDOG_PWRWDOG_ACCU_STAT_SUM_GOOD_MASK                        0x000003FF

#define PWRWDOG_IRDROP_CTRL_OFFSET                                        0x00000018
#define PWRWDOG_IRDROP_CTRL_TYPE                                          UInt32
#define PWRWDOG_IRDROP_CTRL_RESERVED_MASK                                 0xFFFFFFFC
#define    PWRWDOG_IRDROP_CTRL_OSC_OUT_EN_SHIFT                           1
#define    PWRWDOG_IRDROP_CTRL_OSC_OUT_EN_MASK                            0x00000002
#define    PWRWDOG_IRDROP_CTRL_IRDROP_EN_SHIFT                            0
#define    PWRWDOG_IRDROP_CTRL_IRDROP_EN_MASK                             0x00000001

#define PWRWDOG_IRDROP_CNT_OFFSET                                         0x0000001C
#define PWRWDOG_IRDROP_CNT_TYPE                                           UInt32
#define PWRWDOG_IRDROP_CNT_RESERVED_MASK                                  0xFFFFFC00
#define    PWRWDOG_IRDROP_CNT_IRDROP_CNT_SHIFT                            0
#define    PWRWDOG_IRDROP_CNT_IRDROP_CNT_MASK                             0x000003FF

#endif /* __BRCM_RDB_PWRWDOG_H__ */



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

#ifndef __BRCM_RDB_HSMIPC_INT_H__
#define __BRCM_RDB_HSMIPC_INT_H__

#define HSMIPC_INT_DEV_STAT_OFFSET                                        0x00000000
#define HSMIPC_INT_DEV_STAT_TYPE                                          UInt32
#define HSMIPC_INT_DEV_STAT_RESERVED_MASK                                 0xFFFFFFF8
#define    HSMIPC_INT_DEV_STAT_OTP_HSM_EN_SHIFT                           2
#define    HSMIPC_INT_DEV_STAT_OTP_HSM_EN_MASK                            0x00000004
#define    HSMIPC_INT_DEV_STAT_OTP_PROD_SHIFT                             1
#define    HSMIPC_INT_DEV_STAT_OTP_PROD_MASK                              0x00000002
#define    HSMIPC_INT_DEV_STAT_OTP_EMU_SHIFT                              0
#define    HSMIPC_INT_DEV_STAT_OTP_EMU_MASK                               0x00000001

#define HSMIPC_INT_INTR_STAT_OFFSET                                       0x00000004
#define HSMIPC_INT_INTR_STAT_TYPE                                         UInt32
#define HSMIPC_INT_INTR_STAT_RESERVED_MASK                                0x00000000
#define    HSMIPC_INT_INTR_STAT_INT_STAT_N_SHIFT                          0
#define    HSMIPC_INT_INTR_STAT_INT_STAT_N_MASK                           0xFFFFFFFF

#define HSMIPC_INT_INTR_CLR_OFFSET                                        0x00000008
#define HSMIPC_INT_INTR_CLR_TYPE                                          UInt32
#define HSMIPC_INT_INTR_CLR_RESERVED_MASK                                 0x00000000
#define    HSMIPC_INT_INTR_CLR_INT_CLR_N_SHIFT                            0
#define    HSMIPC_INT_INTR_CLR_INT_CLR_N_MASK                             0xFFFFFFFF

#define HSMIPC_INT_INTR_SET_OFFSET                                        0x0000000C
#define HSMIPC_INT_INTR_SET_TYPE                                          UInt32
#define HSMIPC_INT_INTR_SET_RESERVED_MASK                                 0x00000000
#define    HSMIPC_INT_INTR_SET_INT_SET_N_SHIFT                            0
#define    HSMIPC_INT_INTR_SET_INT_SET_N_MASK                             0xFFFFFFFF

#define HSMIPC_INT_HW_INTR_OFFSET                                         0x00000010
#define HSMIPC_INT_HW_INTR_TYPE                                           UInt32
#define HSMIPC_INT_HW_INTR_RESERVED_MASK                                  0xFFFFFFFE
#define    HSMIPC_INT_HW_INTR_HW_SOFT_RST_SHIFT                           0
#define    HSMIPC_INT_HW_INTR_HW_SOFT_RST_MASK                            0x00000001

#define HSMIPC_INT_MAILBOX0_OFFSET                                        0x00000014
#define HSMIPC_INT_MAILBOX0_TYPE                                          UInt32
#define HSMIPC_INT_MAILBOX0_RESERVED_MASK                                 0x00000000
#define    HSMIPC_INT_MAILBOX0_MAILBOX_0_SHIFT                            0
#define    HSMIPC_INT_MAILBOX0_MAILBOX_0_MASK                             0xFFFFFFFF

#define HSMIPC_INT_MAILBOX1_OFFSET                                        0x00000018
#define HSMIPC_INT_MAILBOX1_TYPE                                          UInt32
#define HSMIPC_INT_MAILBOX1_RESERVED_MASK                                 0x00000000
#define    HSMIPC_INT_MAILBOX1_MAILBOX_1_SHIFT                            0
#define    HSMIPC_INT_MAILBOX1_MAILBOX_1_MASK                             0xFFFFFFFF

#define HSMIPC_INT_MAILBOX2_OFFSET                                        0x0000001C
#define HSMIPC_INT_MAILBOX2_TYPE                                          UInt32
#define HSMIPC_INT_MAILBOX2_RESERVED_MASK                                 0x00000000
#define    HSMIPC_INT_MAILBOX2_MAILBOX_2_SHIFT                            0
#define    HSMIPC_INT_MAILBOX2_MAILBOX_2_MASK                             0xFFFFFFFF

#define HSMIPC_INT_MAILBOX3_OFFSET                                        0x00000020
#define HSMIPC_INT_MAILBOX3_TYPE                                          UInt32
#define HSMIPC_INT_MAILBOX3_RESERVED_MASK                                 0x00000000
#define    HSMIPC_INT_MAILBOX3_MAILBOX_3_SHIFT                            0
#define    HSMIPC_INT_MAILBOX3_MAILBOX_3_MASK                             0xFFFFFFFF

#endif /* __BRCM_RDB_HSMIPC_INT_H__ */



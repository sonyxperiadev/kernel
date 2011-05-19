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

#ifndef __BRCM_RDB_OTP_PMU_GASKET_H__
#define __BRCM_RDB_OTP_PMU_GASKET_H__

#define OTP_PMU_GASKET_CFG_OFFSET                                         0x00000100
#define OTP_PMU_GASKET_CFG_TYPE                                           UInt32
#define OTP_PMU_GASKET_CFG_RESERVED_MASK                                  0xFFFFBFEF
#define    OTP_PMU_GASKET_CFG_RST_SHIFT                                   14
#define    OTP_PMU_GASKET_CFG_RST_MASK                                    0x00004000
#define    OTP_PMU_GASKET_CFG_USE_TEST_KEYS_SHIFT                         4
#define    OTP_PMU_GASKET_CFG_USE_TEST_KEYS_MASK                          0x00000010

#define OTP_PMU_GASKET_CFG_LOCK_OFFSET                                    0x00000104
#define OTP_PMU_GASKET_CFG_LOCK_TYPE                                      UInt32
#define OTP_PMU_GASKET_CFG_LOCK_RESERVED_MASK                             0xFFFFBFEF
#define    OTP_PMU_GASKET_CFG_LOCK_RST_LOCK_SHIFT                         14
#define    OTP_PMU_GASKET_CFG_LOCK_RST_LOCK_MASK                          0x00004000
#define    OTP_PMU_GASKET_CFG_LOCK_USE_TEST_KEYS_LOCK_SHIFT               4
#define    OTP_PMU_GASKET_CFG_LOCK_USE_TEST_KEYS_LOCK_MASK                0x00000010

#define OTP_PMU_GASKET_STAT_OFFSET                                        0x00000108
#define OTP_PMU_GASKET_STAT_TYPE                                          UInt32
#define OTP_PMU_GASKET_STAT_RESERVED_MASK                                 0xFFE0F08E
#define    OTP_PMU_GASKET_STAT_EXE_FSM_ST_SHIFT                           16
#define    OTP_PMU_GASKET_STAT_EXE_FSM_ST_MASK                            0x001F0000
#define    OTP_PMU_GASKET_STAT_OTP_CTRL_FSM_ST_SHIFT                      8
#define    OTP_PMU_GASKET_STAT_OTP_CTRL_FSM_ST_MASK                       0x00000F00
#define    OTP_PMU_GASKET_STAT_INSTR_FSM_ST_SHIFT                         4
#define    OTP_PMU_GASKET_STAT_INSTR_FSM_ST_MASK                          0x00000070
#define    OTP_PMU_GASKET_STAT_HW_INIT_DONE_SHIFT                         0
#define    OTP_PMU_GASKET_STAT_HW_INIT_DONE_MASK                          0x00000001

#endif /* __BRCM_RDB_OTP_PMU_GASKET_H__ */



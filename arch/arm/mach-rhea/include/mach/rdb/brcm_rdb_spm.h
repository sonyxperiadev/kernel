/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:16:58                                             */
/*     RDB file : //HERA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_SPM_H__
#define __BRCM_RDB_SPM_H__

#define SPM_SPMENABLE_OFFSET                                              0x00000000
#define SPM_SPMENABLE_TYPE                                                UInt32
#define SPM_SPMENABLE_RESERVED_MASK                                       0xFFFFFFFE
#define    SPM_SPMENABLE_EN_SHIFT                                         0
#define    SPM_SPMENABLE_EN_MASK                                          0x00000001

#define SPM_CTRL_OFFSET                                                   0x00000004
#define SPM_CTRL_TYPE                                                     UInt32
#define SPM_CTRL_RESERVED_MASK                                            0xFFFFFFFF

#define SPM_INTERVAL_VALUE_OFFSET                                         0x00000008
#define SPM_INTERVAL_VALUE_TYPE                                           UInt32
#define SPM_INTERVAL_VALUE_RESERVED_MASK                                  0xFFFF0000
#define    SPM_INTERVAL_VALUE_INTERVAL_VAL_SHIFT                          0
#define    SPM_INTERVAL_VALUE_INTERVAL_VAL_MASK                           0x0000FFFF

#define SPM_INT_ENABLE_OFFSET                                             0x0000000C
#define SPM_INT_ENABLE_TYPE                                               UInt32
#define SPM_INT_ENABLE_RESERVED_MASK                                      0xFFFFFF00
#define    SPM_INT_ENABLE_INTHIGHENABLE3_SHIFT                            7
#define    SPM_INT_ENABLE_INTHIGHENABLE3_MASK                             0x00000080
#define    SPM_INT_ENABLE_INTLOWENABLE3_SHIFT                             6
#define    SPM_INT_ENABLE_INTLOWENABLE3_MASK                              0x00000040
#define    SPM_INT_ENABLE_INTHIGHENABLE2_SHIFT                            5
#define    SPM_INT_ENABLE_INTHIGHENABLE2_MASK                             0x00000020
#define    SPM_INT_ENABLE_INTLOWENABLE2_SHIFT                             4
#define    SPM_INT_ENABLE_INTLOWENABLE2_MASK                              0x00000010
#define    SPM_INT_ENABLE_INTHIGHENABLE1_SHIFT                            3
#define    SPM_INT_ENABLE_INTHIGHENABLE1_MASK                             0x00000008
#define    SPM_INT_ENABLE_INTLOWENABLE1_SHIFT                             2
#define    SPM_INT_ENABLE_INTLOWENABLE1_MASK                              0x00000004
#define    SPM_INT_ENABLE_INTHIGHENABLE0_SHIFT                            1
#define    SPM_INT_ENABLE_INTHIGHENABLE0_MASK                             0x00000002
#define    SPM_INT_ENABLE_INTLOWENABLE0_SHIFT                             0
#define    SPM_INT_ENABLE_INTLOWENABLE0_MASK                              0x00000001

#define SPM_INT_STATUS_OFFSET                                             0x00000010
#define SPM_INT_STATUS_TYPE                                               UInt32
#define SPM_INT_STATUS_RESERVED_MASK                                      0xFFFFFF00
#define    SPM_INT_STATUS_INTSTATUSHIGH3_SHIFT                            7
#define    SPM_INT_STATUS_INTSTATUSHIGH3_MASK                             0x00000080
#define    SPM_INT_STATUS_INTSTATUSLOW3_SHIFT                             6
#define    SPM_INT_STATUS_INTSTATUSLOW3_MASK                              0x00000040
#define    SPM_INT_STATUS_INTSTATUSHIGH2_SHIFT                            5
#define    SPM_INT_STATUS_INTSTATUSHIGH2_MASK                             0x00000020
#define    SPM_INT_STATUS_INTSTATUSLOW2_SHIFT                             4
#define    SPM_INT_STATUS_INTSTATUSLOW2_MASK                              0x00000010
#define    SPM_INT_STATUS_INTSTATUSHIGH1_SHIFT                            3
#define    SPM_INT_STATUS_INTSTATUSHIGH1_MASK                             0x00000008
#define    SPM_INT_STATUS_INTSTATUSLOW1_SHIFT                             2
#define    SPM_INT_STATUS_INTSTATUSLOW1_MASK                              0x00000004
#define    SPM_INT_STATUS_INTSTATUSHIGH0_SHIFT                            1
#define    SPM_INT_STATUS_INTSTATUSHIGH0_MASK                             0x00000002
#define    SPM_INT_STATUS_INTSTATUSLOW0_SHIFT                             0
#define    SPM_INT_STATUS_INTSTATUSLOW0_MASK                              0x00000001

#define SPM_INT_THRESH_LOW0_OFFSET                                        0x00000014
#define SPM_INT_THRESH_LOW0_TYPE                                          UInt32
#define SPM_INT_THRESH_LOW0_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_INT_THRESH_LOW0_INT_THRESH_SHIFT                           0
#define    SPM_INT_THRESH_LOW0_INT_THRESH_MASK                            0x000003FF

#define SPM_INT_THRESH_HIGH0_OFFSET                                       0x00000018
#define SPM_INT_THRESH_HIGH0_TYPE                                         UInt32
#define SPM_INT_THRESH_HIGH0_RESERVED_MASK                                0xFFFFFC00
#define    SPM_INT_THRESH_HIGH0_INT_THRESH_SHIFT                          0
#define    SPM_INT_THRESH_HIGH0_INT_THRESH_MASK                           0x000003FF

#define SPM_INT_THRESH_LOW1_OFFSET                                        0x0000001C
#define SPM_INT_THRESH_LOW1_TYPE                                          UInt32
#define SPM_INT_THRESH_LOW1_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_INT_THRESH_LOW1_INT_THRESH_SHIFT                           0
#define    SPM_INT_THRESH_LOW1_INT_THRESH_MASK                            0x000003FF

#define SPM_INT_THRESH_HIGH1_OFFSET                                       0x00000020
#define SPM_INT_THRESH_HIGH1_TYPE                                         UInt32
#define SPM_INT_THRESH_HIGH1_RESERVED_MASK                                0xFFFFFC00
#define    SPM_INT_THRESH_HIGH1_INT_THRESH_SHIFT                          0
#define    SPM_INT_THRESH_HIGH1_INT_THRESH_MASK                           0x000003FF

#define SPM_INT_THRESH_LOW2_OFFSET                                        0x00000024
#define SPM_INT_THRESH_LOW2_TYPE                                          UInt32
#define SPM_INT_THRESH_LOW2_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_INT_THRESH_LOW2_INT_THRESH_SHIFT                           0
#define    SPM_INT_THRESH_LOW2_INT_THRESH_MASK                            0x000003FF

#define SPM_INT_THRESH_HIGH2_OFFSET                                       0x00000028
#define SPM_INT_THRESH_HIGH2_TYPE                                         UInt32
#define SPM_INT_THRESH_HIGH2_RESERVED_MASK                                0xFFFFFC00
#define    SPM_INT_THRESH_HIGH2_INT_THRESH_SHIFT                          0
#define    SPM_INT_THRESH_HIGH2_INT_THRESH_MASK                           0x000003FF

#define SPM_INT_THRESH_LOW3_OFFSET                                        0x0000002C
#define SPM_INT_THRESH_LOW3_TYPE                                          UInt32
#define SPM_INT_THRESH_LOW3_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_INT_THRESH_LOW3_INT_THRESH_SHIFT                           0
#define    SPM_INT_THRESH_LOW3_INT_THRESH_MASK                            0x000003FF

#define SPM_INT_THRESH_HIGH3_OFFSET                                       0x00000030
#define SPM_INT_THRESH_HIGH3_TYPE                                         UInt32
#define SPM_INT_THRESH_HIGH3_RESERVED_MASK                                0xFFFFFC00
#define    SPM_INT_THRESH_HIGH3_INT_THRESH_SHIFT                          0
#define    SPM_INT_THRESH_HIGH3_INT_THRESH_MASK                           0x000003FF

#define SPM_MONITOR_VALUE0_OFFSET                                         0x00000034
#define SPM_MONITOR_VALUE0_TYPE                                           UInt32
#define SPM_MONITOR_VALUE0_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE0_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE0_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE1_OFFSET                                         0x00000038
#define SPM_MONITOR_VALUE1_TYPE                                           UInt32
#define SPM_MONITOR_VALUE1_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE1_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE1_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE2_OFFSET                                         0x0000003C
#define SPM_MONITOR_VALUE2_TYPE                                           UInt32
#define SPM_MONITOR_VALUE2_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE2_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE2_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE3_OFFSET                                         0x00000040
#define SPM_MONITOR_VALUE3_TYPE                                           UInt32
#define SPM_MONITOR_VALUE3_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE3_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE3_MONITOR_VAL_MASK                            0x000003FF

#endif /* __BRCM_RDB_SPM_H__ */



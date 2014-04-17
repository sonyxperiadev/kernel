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

#ifndef __BRCM_RDB_SPM_H__
#define __BRCM_RDB_SPM_H__

#define SPM_SPMENABLE_OFFSET                                              0x00000000
#define SPM_SPMENABLE_TYPE                                                UInt32
#define SPM_SPMENABLE_RESERVED_MASK                                       0xFFFFFFFE
#define    SPM_SPMENABLE_EN_SHIFT                                         0
#define    SPM_SPMENABLE_EN_MASK                                          0x00000001

#define SPM_CTRL_OFFSET                                                   0x00000004
#define SPM_CTRL_TYPE                                                     UInt32
#define SPM_CTRL_RESERVED_MASK                                            0xFFFFF000
#define    SPM_CTRL_MONITOR5MODE_SHIFT                                    10
#define    SPM_CTRL_MONITOR5MODE_MASK                                     0x00000C00
#define    SPM_CTRL_MONITOR4MODE_SHIFT                                    8
#define    SPM_CTRL_MONITOR4MODE_MASK                                     0x00000300
#define    SPM_CTRL_MONITOR3MODE_SHIFT                                    6
#define    SPM_CTRL_MONITOR3MODE_MASK                                     0x000000C0
#define    SPM_CTRL_MONITOR2MODE_SHIFT                                    4
#define    SPM_CTRL_MONITOR2MODE_MASK                                     0x00000030
#define    SPM_CTRL_MONITOR1MODE_SHIFT                                    2
#define    SPM_CTRL_MONITOR1MODE_MASK                                     0x0000000C
#define    SPM_CTRL_MONITOR0MODE_SHIFT                                    0
#define    SPM_CTRL_MONITOR0MODE_MASK                                     0x00000003

#define SPM_INTERVAL_VALUE_OFFSET                                         0x00000008
#define SPM_INTERVAL_VALUE_TYPE                                           UInt32
#define SPM_INTERVAL_VALUE_RESERVED_MASK                                  0xFFFF0000
#define    SPM_INTERVAL_VALUE_INTERVAL_VAL_SHIFT                          0
#define    SPM_INTERVAL_VALUE_INTERVAL_VAL_MASK                           0x0000FFFF

#define SPM_INT_ENABLE_OFFSET                                             0x0000000C
#define SPM_INT_ENABLE_TYPE                                               UInt32
#define SPM_INT_ENABLE_RESERVED_MASK                                      0xFFFFF000
#define    SPM_INT_ENABLE_INTHIGHENABLE5_SHIFT                            11
#define    SPM_INT_ENABLE_INTHIGHENABLE5_MASK                             0x00000800
#define    SPM_INT_ENABLE_INTLOWENABLE5_SHIFT                             10
#define    SPM_INT_ENABLE_INTLOWENABLE5_MASK                              0x00000400
#define    SPM_INT_ENABLE_INTHIGHENABLE4_SHIFT                            9
#define    SPM_INT_ENABLE_INTHIGHENABLE4_MASK                             0x00000200
#define    SPM_INT_ENABLE_INTLOWENABLE4_SHIFT                             8
#define    SPM_INT_ENABLE_INTLOWENABLE4_MASK                              0x00000100
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
#define SPM_INT_STATUS_RESERVED_MASK                                      0xFFFFF000
#define    SPM_INT_STATUS_INTSTATUSHIGH5_SHIFT                            11
#define    SPM_INT_STATUS_INTSTATUSHIGH5_MASK                             0x00000800
#define    SPM_INT_STATUS_INTSTATUSLOW5_SHIFT                             10
#define    SPM_INT_STATUS_INTSTATUSLOW5_MASK                              0x00000400
#define    SPM_INT_STATUS_INTSTATUSHIGH4_SHIFT                            9
#define    SPM_INT_STATUS_INTSTATUSHIGH4_MASK                             0x00000200
#define    SPM_INT_STATUS_INTSTATUSLOW4_SHIFT                             8
#define    SPM_INT_STATUS_INTSTATUSLOW4_MASK                              0x00000100
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

#define SPM_INT_THRESH_LOW4_OFFSET                                        0x00000034
#define SPM_INT_THRESH_LOW4_TYPE                                          UInt32
#define SPM_INT_THRESH_LOW4_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_INT_THRESH_LOW4_INT_THRESH_SHIFT                           0
#define    SPM_INT_THRESH_LOW4_INT_THRESH_MASK                            0x000003FF

#define SPM_INT_THRESH_HIGH4_OFFSET                                       0x00000038
#define SPM_INT_THRESH_HIGH4_TYPE                                         UInt32
#define SPM_INT_THRESH_HIGH4_RESERVED_MASK                                0xFFFFFC00
#define    SPM_INT_THRESH_HIGH4_INT_THRESH_SHIFT                          0
#define    SPM_INT_THRESH_HIGH4_INT_THRESH_MASK                           0x000003FF

#define SPM_INT_THRESH_LOW5_OFFSET                                        0x0000003C
#define SPM_INT_THRESH_LOW5_TYPE                                          UInt32
#define SPM_INT_THRESH_LOW5_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_INT_THRESH_LOW5_INT_THRESH_SHIFT                           0
#define    SPM_INT_THRESH_LOW5_INT_THRESH_MASK                            0x000003FF

#define SPM_INT_THRESH_HIGH5_OFFSET                                       0x00000040
#define SPM_INT_THRESH_HIGH5_TYPE                                         UInt32
#define SPM_INT_THRESH_HIGH5_RESERVED_MASK                                0xFFFFFC00
#define    SPM_INT_THRESH_HIGH5_INT_THRESH_SHIFT                          0
#define    SPM_INT_THRESH_HIGH5_INT_THRESH_MASK                           0x000003FF

#define SPM_MONITOR_VALUE0_OFFSET                                         0x00000044
#define SPM_MONITOR_VALUE0_TYPE                                           UInt32
#define SPM_MONITOR_VALUE0_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE0_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE0_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE1_OFFSET                                         0x00000048
#define SPM_MONITOR_VALUE1_TYPE                                           UInt32
#define SPM_MONITOR_VALUE1_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE1_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE1_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE2_OFFSET                                         0x0000004C
#define SPM_MONITOR_VALUE2_TYPE                                           UInt32
#define SPM_MONITOR_VALUE2_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE2_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE2_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE3_OFFSET                                         0x00000050
#define SPM_MONITOR_VALUE3_TYPE                                           UInt32
#define SPM_MONITOR_VALUE3_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE3_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE3_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE4_OFFSET                                         0x00000054
#define SPM_MONITOR_VALUE4_TYPE                                           UInt32
#define SPM_MONITOR_VALUE4_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE4_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE4_MONITOR_VAL_MASK                            0x000003FF

#define SPM_MONITOR_VALUE5_OFFSET                                         0x00000058
#define SPM_MONITOR_VALUE5_TYPE                                           UInt32
#define SPM_MONITOR_VALUE5_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_MONITOR_VALUE5_MONITOR_VAL_SHIFT                           0
#define    SPM_MONITOR_VALUE5_MONITOR_VAL_MASK                            0x000003FF

#define SPM_WATERMARK_LOW0_OFFSET                                         0x0000005C
#define SPM_WATERMARK_LOW0_TYPE                                           UInt32
#define SPM_WATERMARK_LOW0_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_WATERMARK_LOW0_WATERMARK_SHIFT                             0
#define    SPM_WATERMARK_LOW0_WATERMARK_MASK                              0x000003FF

#define SPM_WATERMARK_HIGH0_OFFSET                                        0x00000060
#define SPM_WATERMARK_HIGH0_TYPE                                          UInt32
#define SPM_WATERMARK_HIGH0_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_WATERMARK_HIGH0_WATERMARK_SHIFT                            0
#define    SPM_WATERMARK_HIGH0_WATERMARK_MASK                             0x000003FF

#define SPM_WATERMARK_LOW1_OFFSET                                         0x00000064
#define SPM_WATERMARK_LOW1_TYPE                                           UInt32
#define SPM_WATERMARK_LOW1_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_WATERMARK_LOW1_WATERMARK_SHIFT                             0
#define    SPM_WATERMARK_LOW1_WATERMARK_MASK                              0x000003FF

#define SPM_WATERMARK_HIGH1_OFFSET                                        0x00000068
#define SPM_WATERMARK_HIGH1_TYPE                                          UInt32
#define SPM_WATERMARK_HIGH1_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_WATERMARK_HIGH1_WATERMARK_SHIFT                            0
#define    SPM_WATERMARK_HIGH1_WATERMARK_MASK                             0x000003FF

#define SPM_WATERMARK_LOW2_OFFSET                                         0x0000006C
#define SPM_WATERMARK_LOW2_TYPE                                           UInt32
#define SPM_WATERMARK_LOW2_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_WATERMARK_LOW2_WATERMARK_SHIFT                             0
#define    SPM_WATERMARK_LOW2_WATERMARK_MASK                              0x000003FF

#define SPM_WATERMARK_HIGH2_OFFSET                                        0x00000070
#define SPM_WATERMARK_HIGH2_TYPE                                          UInt32
#define SPM_WATERMARK_HIGH2_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_WATERMARK_HIGH2_WATERMARK_SHIFT                            0
#define    SPM_WATERMARK_HIGH2_WATERMARK_MASK                             0x000003FF

#define SPM_WATERMARK_LOW3_OFFSET                                         0x00000074
#define SPM_WATERMARK_LOW3_TYPE                                           UInt32
#define SPM_WATERMARK_LOW3_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_WATERMARK_LOW3_WATERMARK_SHIFT                             0
#define    SPM_WATERMARK_LOW3_WATERMARK_MASK                              0x000003FF

#define SPM_WATERMARK_HIGH3_OFFSET                                        0x00000078
#define SPM_WATERMARK_HIGH3_TYPE                                          UInt32
#define SPM_WATERMARK_HIGH3_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_WATERMARK_HIGH3_WATERMARK_SHIFT                            0
#define    SPM_WATERMARK_HIGH3_WATERMARK_MASK                             0x000003FF

#define SPM_WATERMARK_LOW4_OFFSET                                         0x0000007C
#define SPM_WATERMARK_LOW4_TYPE                                           UInt32
#define SPM_WATERMARK_LOW4_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_WATERMARK_LOW4_WATERMARK_SHIFT                             0
#define    SPM_WATERMARK_LOW4_WATERMARK_MASK                              0x000003FF

#define SPM_WATERMARK_HIGH4_OFFSET                                        0x00000080
#define SPM_WATERMARK_HIGH4_TYPE                                          UInt32
#define SPM_WATERMARK_HIGH4_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_WATERMARK_HIGH4_WATERMARK_SHIFT                            0
#define    SPM_WATERMARK_HIGH4_WATERMARK_MASK                             0x000003FF

#define SPM_WATERMARK_LOW5_OFFSET                                         0x00000084
#define SPM_WATERMARK_LOW5_TYPE                                           UInt32
#define SPM_WATERMARK_LOW5_RESERVED_MASK                                  0xFFFFFC00
#define    SPM_WATERMARK_LOW5_WATERMARK_SHIFT                             0
#define    SPM_WATERMARK_LOW5_WATERMARK_MASK                              0x000003FF

#define SPM_WATERMARK_HIGH5_OFFSET                                        0x00000088
#define SPM_WATERMARK_HIGH5_TYPE                                          UInt32
#define SPM_WATERMARK_HIGH5_RESERVED_MASK                                 0xFFFFFC00
#define    SPM_WATERMARK_HIGH5_WATERMARK_SHIFT                            0
#define    SPM_WATERMARK_HIGH5_WATERMARK_MASK                             0x000003FF

#endif /* __BRCM_RDB_SPM_H__ */



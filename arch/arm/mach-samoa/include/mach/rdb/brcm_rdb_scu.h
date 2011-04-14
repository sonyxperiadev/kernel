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
/*     Date     : Generated on 4/10/2011 22:27:55                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_SCU_H__
#define __BRCM_RDB_SCU_H__

#define SCU_CONTROL_OFFSET                                                0x00000000
#define SCU_CONTROL_TYPE                                                  UInt32
#define SCU_CONTROL_RESERVED_MASK                                         0xFFFFFFFC
#define    SCU_CONTROL_ADDRESS_FILTER_ENABLE_SHIFT                        1
#define    SCU_CONTROL_ADDRESS_FILTER_ENABLE_MASK                         0x00000002
#define    SCU_CONTROL_SCU_ENABLE_SHIFT                                   0
#define    SCU_CONTROL_SCU_ENABLE_MASK                                    0x00000001

#define SCU_CONFIG_OFFSET                                                 0x00000004
#define SCU_CONFIG_TYPE                                                   UInt32
#define SCU_CONFIG_RESERVED_MASK                                          0x3F00000C
#define    SCU_CONFIG_L2C_PRESENT_SHIFT                                   31
#define    SCU_CONFIG_L2C_PRESENT_MASK                                    0x80000000
#define    SCU_CONFIG_ACP_PRESENT_SHIFT                                   30
#define    SCU_CONFIG_ACP_PRESENT_MASK                                    0x40000000
#define    SCU_CONFIG_CPU3_DATA_CACHE_SIZE_SHIFT                          20
#define    SCU_CONFIG_CPU3_DATA_CACHE_SIZE_MASK                           0x00F00000
#define    SCU_CONFIG_CPU2_DATA_CACHE_SIZE_SHIFT                          16
#define    SCU_CONFIG_CPU2_DATA_CACHE_SIZE_MASK                           0x000F0000
#define    SCU_CONFIG_CPU1_DATA_CACHE_SIZE_SHIFT                          12
#define    SCU_CONFIG_CPU1_DATA_CACHE_SIZE_MASK                           0x0000F000
#define    SCU_CONFIG_CPU0_DATA_CACHE_SIZE_SHIFT                          8
#define    SCU_CONFIG_CPU0_DATA_CACHE_SIZE_MASK                           0x00000F00
#define    SCU_CONFIG_CPU3_COHERENCY_MODE_SHIFT                           7
#define    SCU_CONFIG_CPU3_COHERENCY_MODE_MASK                            0x00000080
#define    SCU_CONFIG_CPU2_COHERENCY_MODE_SHIFT                           6
#define    SCU_CONFIG_CPU2_COHERENCY_MODE_MASK                            0x00000040
#define    SCU_CONFIG_CPU1_COHERENCY_MODE_SHIFT                           5
#define    SCU_CONFIG_CPU1_COHERENCY_MODE_MASK                            0x00000020
#define    SCU_CONFIG_CPU0_COHERENCY_MODE_SHIFT                           4
#define    SCU_CONFIG_CPU0_COHERENCY_MODE_MASK                            0x00000010
#define    SCU_CONFIG_NUM_CPUS_SHIFT                                      0
#define    SCU_CONFIG_NUM_CPUS_MASK                                       0x00000003

#define SCU_POWER_STATUS_OFFSET                                           0x00000008
#define SCU_POWER_STATUS_TYPE                                             UInt32
#define SCU_POWER_STATUS_RESERVED_MASK                                    0xFCFCFCFC
#define    SCU_POWER_STATUS_CPU3_STATUS_SHIFT                             24
#define    SCU_POWER_STATUS_CPU3_STATUS_MASK                              0x03000000
#define    SCU_POWER_STATUS_CPU2_STATUS_SHIFT                             16
#define    SCU_POWER_STATUS_CPU2_STATUS_MASK                              0x00030000
#define    SCU_POWER_STATUS_CPU1_STATUS_SHIFT                             8
#define    SCU_POWER_STATUS_CPU1_STATUS_MASK                              0x00000300
#define    SCU_POWER_STATUS_CPU0_STATUS_SHIFT                             0
#define    SCU_POWER_STATUS_CPU0_STATUS_MASK                              0x00000003

#define SCU_INVALIDATE_ALL_OFFSET                                         0x0000000C
#define SCU_INVALIDATE_ALL_TYPE                                           UInt32
#define SCU_INVALIDATE_ALL_RESERVED_MASK                                  0xFFFF0000
#define    SCU_INVALIDATE_ALL_CPU3_WAYS_SHIFT                             12
#define    SCU_INVALIDATE_ALL_CPU3_WAYS_MASK                              0x0000F000
#define    SCU_INVALIDATE_ALL_CPU2_WAYS_SHIFT                             8
#define    SCU_INVALIDATE_ALL_CPU2_WAYS_MASK                              0x00000F00
#define    SCU_INVALIDATE_ALL_CPU1_WAYS_SHIFT                             4
#define    SCU_INVALIDATE_ALL_CPU1_WAYS_MASK                              0x000000F0
#define    SCU_INVALIDATE_ALL_CPU0_WAYS_SHIFT                             0
#define    SCU_INVALIDATE_ALL_CPU0_WAYS_MASK                              0x0000000F

#define SCU_FILTER_START_OFFSET                                           0x00000040
#define SCU_FILTER_START_TYPE                                             UInt32
#define SCU_FILTER_START_RESERVED_MASK                                    0x000FFFFF
#define    SCU_FILTER_START_FILTER_START_SHIFT                            20
#define    SCU_FILTER_START_FILTER_START_MASK                             0xFFF00000

#define SCU_FILTER_END_OFFSET                                             0x00000044
#define SCU_FILTER_END_TYPE                                               UInt32
#define SCU_FILTER_END_RESERVED_MASK                                      0x000FFFFF
#define    SCU_FILTER_END_FILTER_END_SHIFT                                20
#define    SCU_FILTER_END_FILTER_END_MASK                                 0xFFF00000

#define SCU_ACCESS_CONTROL_OFFSET                                         0x00000050
#define SCU_ACCESS_CONTROL_TYPE                                           UInt32
#define SCU_ACCESS_CONTROL_RESERVED_MASK                                  0xFFFFFFF0
#define    SCU_ACCESS_CONTROL_CPU3_ACCESS_EN_SHIFT                        3
#define    SCU_ACCESS_CONTROL_CPU3_ACCESS_EN_MASK                         0x00000008
#define    SCU_ACCESS_CONTROL_CPU2_ACCESS_EN_SHIFT                        2
#define    SCU_ACCESS_CONTROL_CPU2_ACCESS_EN_MASK                         0x00000004
#define    SCU_ACCESS_CONTROL_CPU1_ACCESS_EN_SHIFT                        1
#define    SCU_ACCESS_CONTROL_CPU1_ACCESS_EN_MASK                         0x00000002
#define    SCU_ACCESS_CONTROL_CPU0_ACCESS_EN_SHIFT                        0
#define    SCU_ACCESS_CONTROL_CPU0_ACCESS_EN_MASK                         0x00000001

#define SCU_SECURE_ACCESS_OFFSET                                          0x00000054
#define SCU_SECURE_ACCESS_TYPE                                            UInt32
#define SCU_SECURE_ACCESS_RESERVED_MASK                                   0xFFFFF000
#define    SCU_SECURE_ACCESS_CPU3_GLOBAL_TIMER_SHIFT                      11
#define    SCU_SECURE_ACCESS_CPU3_GLOBAL_TIMER_MASK                       0x00000800
#define    SCU_SECURE_ACCESS_CPU2_GLOBAL_TIMER_SHIFT                      10
#define    SCU_SECURE_ACCESS_CPU2_GLOBAL_TIMER_MASK                       0x00000400
#define    SCU_SECURE_ACCESS_CPU1_GLOBAL_TIMER_SHIFT                      9
#define    SCU_SECURE_ACCESS_CPU1_GLOBAL_TIMER_MASK                       0x00000200
#define    SCU_SECURE_ACCESS_CPU0_GLOBAL_TIMER_SHIFT                      8
#define    SCU_SECURE_ACCESS_CPU0_GLOBAL_TIMER_MASK                       0x00000100
#define    SCU_SECURE_ACCESS_CPU3_PRIVATE_TIMER_SHIFT                     7
#define    SCU_SECURE_ACCESS_CPU3_PRIVATE_TIMER_MASK                      0x00000080
#define    SCU_SECURE_ACCESS_CPU2_PRIVATE_TIMER_SHIFT                     6
#define    SCU_SECURE_ACCESS_CPU2_PRIVATE_TIMER_MASK                      0x00000040
#define    SCU_SECURE_ACCESS_CPU1_PRIVATE_TIMER_SHIFT                     5
#define    SCU_SECURE_ACCESS_CPU1_PRIVATE_TIMER_MASK                      0x00000020
#define    SCU_SECURE_ACCESS_CPU0_PRIVATE_TIMER_SHIFT                     4
#define    SCU_SECURE_ACCESS_CPU0_PRIVATE_TIMER_MASK                      0x00000010
#define    SCU_SECURE_ACCESS_CPU3_COMPONENT_ACCESS_SHIFT                  3
#define    SCU_SECURE_ACCESS_CPU3_COMPONENT_ACCESS_MASK                   0x00000008
#define    SCU_SECURE_ACCESS_CPU2_COMPONENT_ACCESS_SHIFT                  2
#define    SCU_SECURE_ACCESS_CPU2_COMPONENT_ACCESS_MASK                   0x00000004
#define    SCU_SECURE_ACCESS_CPU1_COMPONENT_ACCESS_SHIFT                  1
#define    SCU_SECURE_ACCESS_CPU1_COMPONENT_ACCESS_MASK                   0x00000002
#define    SCU_SECURE_ACCESS_CPU0_COMPONENT_ACCESS_SHIFT                  0
#define    SCU_SECURE_ACCESS_CPU0_COMPONENT_ACCESS_MASK                   0x00000001

#endif /* __BRCM_RDB_SCU_H__ */



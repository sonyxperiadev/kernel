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

#ifndef __BRCM_RDB_GLOBPERF_H__
#define __BRCM_RDB_GLOBPERF_H__

#define GLOBPERF_GLB_CONFIG_OFFSET                                        0x00000000
#define GLOBPERF_GLB_CONFIG_TYPE                                          UInt32
#define GLOBPERF_GLB_CONFIG_RESERVED_MASK                                 0x70000000
#define    GLOBPERF_GLB_CONFIG_COUNTER_STOP_EN_SHIFT                      31
#define    GLOBPERF_GLB_CONFIG_COUNTER_STOP_EN_MASK                       0x80000000
#define    GLOBPERF_GLB_CONFIG_TIMEOUT_SHIFT                              0
#define    GLOBPERF_GLB_CONFIG_TIMEOUT_MASK                               0x0FFFFFFF

#define GLOBPERF_GLB_STATUS_OFFSET                                        0x00000004
#define GLOBPERF_GLB_STATUS_TYPE                                          UInt32
#define GLOBPERF_GLB_STATUS_RESERVED_MASK                                 0xFFE00000
#define    GLOBPERF_GLB_STATUS_REQ_PM_TRACE_SHIFT                         20
#define    GLOBPERF_GLB_STATUS_REQ_PM_TRACE_MASK                          0x00100000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_19_SHIFT                     19
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_19_MASK                      0x00080000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_18_SHIFT                     18
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_18_MASK                      0x00040000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_17_SHIFT                     17
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_17_MASK                      0x00020000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_16_SHIFT                     16
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_16_MASK                      0x00010000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_15_SHIFT                     15
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_15_MASK                      0x00008000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_14_SHIFT                     14
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_14_MASK                      0x00004000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_13_SHIFT                     13
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_13_MASK                      0x00002000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_12_SHIFT                     12
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_12_MASK                      0x00001000
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_11_SHIFT                     11
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_11_MASK                      0x00000800
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_10_SHIFT                     10
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_10_MASK                      0x00000400
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_9_SHIFT                      9
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_9_MASK                       0x00000200
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_8_SHIFT                      8
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_8_MASK                       0x00000100
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_7_SHIFT                      7
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_7_MASK                       0x00000080
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_6_SHIFT                      6
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_6_MASK                       0x00000040
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_5_SHIFT                      5
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_5_MASK                       0x00000020
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_4_SHIFT                      4
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_4_MASK                       0x00000010
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_3_SHIFT                      3
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_3_MASK                       0x00000008
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_2_SHIFT                      2
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_2_MASK                       0x00000004
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_1_SHIFT                      1
#define    GLOBPERF_GLB_STATUS_REQ_AXI_TRACE_1_MASK                       0x00000002
#define    GLOBPERF_GLB_STATUS_REQ_GIC_TRACE_SHIFT                        0
#define    GLOBPERF_GLB_STATUS_REQ_GIC_TRACE_MASK                         0x00000001

#define GLOBPERF_GLB_COUNT_OFFSET                                         0x00000008
#define GLOBPERF_GLB_COUNT_TYPE                                           UInt32
#define GLOBPERF_GLB_COUNT_RESERVED_MASK                                  0x70000000
#define    GLOBPERF_GLB_COUNT_COUNT_EN_SHIFT                              31
#define    GLOBPERF_GLB_COUNT_COUNT_EN_MASK                               0x80000000
#define    GLOBPERF_GLB_COUNT_COUNT_SHIFT                                 0
#define    GLOBPERF_GLB_COUNT_COUNT_MASK                                  0x0FFFFFFF

#define GLOBPERF_GLB_CMD_OFFSET                                           0x0000000C
#define GLOBPERF_GLB_CMD_TYPE                                             UInt32
#define GLOBPERF_GLB_CMD_RESERVED_MASK                                    0xFFFFFFF8
#define    GLOBPERF_GLB_CMD_GLOBAL_COMMAND_SHIFT                          0
#define    GLOBPERF_GLB_CMD_GLOBAL_COMMAND_MASK                           0x00000007

#endif /* __BRCM_RDB_GLOBPERF_H__ */



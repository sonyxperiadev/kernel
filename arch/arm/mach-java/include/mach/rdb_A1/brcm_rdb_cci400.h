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

#ifndef __BRCM_RDB_CCI400_H__
#define __BRCM_RDB_CCI400_H__

#define CCI400_CONTROL_OVERRIDE_OFFSET                                    0x00000000
#define CCI400_CONTROL_OVERRIDE_TYPE                                      UInt32
#define CCI400_CONTROL_OVERRIDE_RESERVED_MASK                             0xFFFFFFE0
#define    CCI400_CONTROL_OVERRIDE_DISABLE_PRIORITY_PROMOTION_SHIFT       4
#define    CCI400_CONTROL_OVERRIDE_DISABLE_PRIORITY_PROMOTION_MASK        0x00000010
#define    CCI400_CONTROL_OVERRIDE_TERMINATE_BARRIERS_SHIFT               3
#define    CCI400_CONTROL_OVERRIDE_TERMINATE_BARRIERS_MASK                0x00000008
#define    CCI400_CONTROL_OVERRIDE_DISABLE_SPECULATIVE_FETCHES_SHIFT      2
#define    CCI400_CONTROL_OVERRIDE_DISABLE_SPECULATIVE_FETCHES_MASK       0x00000004
#define    CCI400_CONTROL_OVERRIDE_DVM_MESSAGE_DISABLE_SHIFT              1
#define    CCI400_CONTROL_OVERRIDE_DVM_MESSAGE_DISABLE_MASK               0x00000002
#define    CCI400_CONTROL_OVERRIDE_SNOOP_DISABLE_SHIFT                    0
#define    CCI400_CONTROL_OVERRIDE_SNOOP_DISABLE_MASK                     0x00000001

#define CCI400_SPECULATION_CONTROL_OFFSET                                 0x00000004
#define CCI400_SPECULATION_CONTROL_TYPE                                   UInt32
#define CCI400_SPECULATION_CONTROL_RESERVED_MASK                          0xFFE0FFF8
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S4_SHIFT 20
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S4_MASK 0x00100000
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S3_SHIFT 19
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S3_MASK 0x00080000
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S2_SHIFT 18
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S2_MASK 0x00040000
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S1_SHIFT 17
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S1_MASK 0x00020000
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S0_SHIFT 16
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_S0_MASK 0x00010000
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_M2_SHIFT 2
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_M2_MASK 0x00000004
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_M1_SHIFT 1
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_M1_MASK 0x00000002
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_M0_SHIFT 0
#define    CCI400_SPECULATION_CONTROL_DISABLE_SPECULATIVE_FETCHES_M0_MASK 0x00000001

#define CCI400_SECURE_ACCESS_OFFSET                                       0x00000008
#define CCI400_SECURE_ACCESS_TYPE                                         UInt32
#define CCI400_SECURE_ACCESS_RESERVED_MASK                                0xFFFFFFFE
#define    CCI400_SECURE_ACCESS_SECURE_ACCESS_CONTROL_SHIFT               0
#define    CCI400_SECURE_ACCESS_SECURE_ACCESS_CONTROL_MASK                0x00000001

#define CCI400_STATUS_OFFSET                                              0x0000000C
#define CCI400_STATUS_TYPE                                                UInt32
#define CCI400_STATUS_RESERVED_MASK                                       0xFFFFFFFE
#define    CCI400_STATUS_CCI_STATUS_SHIFT                                 0
#define    CCI400_STATUS_CCI_STATUS_MASK                                  0x00000001

#define CCI400_IMPRECISE_ERROR_OFFSET                                     0x00000010
#define CCI400_IMPRECISE_ERROR_TYPE                                       UInt32
#define CCI400_IMPRECISE_ERROR_RESERVED_MASK                              0xFFE0FFF8
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S4_SHIFT                        20
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S4_MASK                         0x00100000
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S3_SHIFT                        19
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S3_MASK                         0x00080000
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S2_SHIFT                        18
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S2_MASK                         0x00040000
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S1_SHIFT                        17
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S1_MASK                         0x00020000
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S0_SHIFT                        16
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_S0_MASK                         0x00010000
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_M2_SHIFT                        2
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_M2_MASK                         0x00000004
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_M1_SHIFT                        1
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_M1_MASK                         0x00000002
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_M0_SHIFT                        0
#define    CCI400_IMPRECISE_ERROR_IMP_ERR_M0_MASK                         0x00000001

#define CCI400_PERFORMANCE_MONITOR_CONTROL_OFFSET                         0x00000100
#define CCI400_PERFORMANCE_MONITOR_CONTROL_TYPE                           UInt32
#define CCI400_PERFORMANCE_MONITOR_CONTROL_RESERVED_MASK                  0xFFFF07C0
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_PMU_COUNT_NUM_SHIFT         11
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_PMU_COUNT_NUM_MASK          0x0000F800
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_DP_SHIFT                    5
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_DP_MASK                     0x00000020
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_EX_SHIFT                    4
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_EX_MASK                     0x00000010
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_CCD_SHIFT                   3
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_CCD_MASK                    0x00000008
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_CCR_SHIFT                   2
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_CCR_MASK                    0x00000004
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_RST_SHIFT                   1
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_RST_MASK                    0x00000002
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_CEN_SHIFT                   0
#define    CCI400_PERFORMANCE_MONITOR_CONTROL_CEN_MASK                    0x00000001

#define CCI400_PERIPHERAL_ID4_OFFSET                                      0x00000FD0
#define CCI400_PERIPHERAL_ID4_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID4_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID4_PERIPH_ID_4_SHIFT                        0
#define    CCI400_PERIPHERAL_ID4_PERIPH_ID_4_MASK                         0x000000FF

#define CCI400_PERIPHERAL_ID5_OFFSET                                      0x00000FD4
#define CCI400_PERIPHERAL_ID5_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID5_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID5_PERIPH_ID_5_SHIFT                        0
#define    CCI400_PERIPHERAL_ID5_PERIPH_ID_5_MASK                         0x000000FF

#define CCI400_PERIPHERAL_ID6_OFFSET                                      0x00000FD8
#define CCI400_PERIPHERAL_ID6_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID6_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID6_PERIPH_ID_6_SHIFT                        0
#define    CCI400_PERIPHERAL_ID6_PERIPH_ID_6_MASK                         0x000000FF

#define CCI400_PERIPHERAL_ID7_OFFSET                                      0x00000FDC
#define CCI400_PERIPHERAL_ID7_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID7_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID7_PERIPH_ID_7_SHIFT                        0
#define    CCI400_PERIPHERAL_ID7_PERIPH_ID_7_MASK                         0x000000FF

#define CCI400_PERIPHERAL_ID0_OFFSET                                      0x00000FE0
#define CCI400_PERIPHERAL_ID0_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID0_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID0_PERIPHERAL_ID0_SHIFT                     0
#define    CCI400_PERIPHERAL_ID0_PERIPHERAL_ID0_MASK                      0x000000FF

#define CCI400_PERIPHERAL_ID1_OFFSET                                      0x00000FE4
#define CCI400_PERIPHERAL_ID1_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID1_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID1_PERIPHERAL_ID1_SHIFT                     0
#define    CCI400_PERIPHERAL_ID1_PERIPHERAL_ID1_MASK                      0x000000FF

#define CCI400_PERIPHERAL_ID2_OFFSET                                      0x00000FE8
#define CCI400_PERIPHERAL_ID2_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID2_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID2_PERIPH_ID_2_SHIFT                        0
#define    CCI400_PERIPHERAL_ID2_PERIPH_ID_2_MASK                         0x000000FF

#define CCI400_PERIPHERAL_ID3_OFFSET                                      0x00000FEC
#define CCI400_PERIPHERAL_ID3_TYPE                                        UInt32
#define CCI400_PERIPHERAL_ID3_RESERVED_MASK                               0xFFFFFF00
#define    CCI400_PERIPHERAL_ID3_REV_AND_SHIFT                            4
#define    CCI400_PERIPHERAL_ID3_REV_AND_MASK                             0x000000F0
#define    CCI400_PERIPHERAL_ID3_CUST_MOD_NUM_SHIFT                       0
#define    CCI400_PERIPHERAL_ID3_CUST_MOD_NUM_MASK                        0x0000000F

#define CCI400_COMPONENT_ID0_OFFSET                                       0x00000FF0
#define CCI400_COMPONENT_ID0_TYPE                                         UInt32
#define CCI400_COMPONENT_ID0_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_COMPONENT_ID0_COMPONENT_ID0_SHIFT                       0
#define    CCI400_COMPONENT_ID0_COMPONENT_ID0_MASK                        0x000000FF

#define CCI400_COMPONENT_ID1_OFFSET                                       0x00000FF4
#define CCI400_COMPONENT_ID1_TYPE                                         UInt32
#define CCI400_COMPONENT_ID1_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_COMPONENT_ID1_COMPONENT_ID1_SHIFT                       0
#define    CCI400_COMPONENT_ID1_COMPONENT_ID1_MASK                        0x000000FF

#define CCI400_COMPONENT_ID2_OFFSET                                       0x00000FF8
#define CCI400_COMPONENT_ID2_TYPE                                         UInt32
#define CCI400_COMPONENT_ID2_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_COMPONENT_ID2_COMPONENT_ID2_SHIFT                       0
#define    CCI400_COMPONENT_ID2_COMPONENT_ID2_MASK                        0x000000FF

#define CCI400_COMPONENT_ID3_OFFSET                                       0x00000FFC
#define CCI400_COMPONENT_ID3_TYPE                                         UInt32
#define CCI400_COMPONENT_ID3_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_COMPONENT_ID3_COMPONENT_ID3_SHIFT                       0
#define    CCI400_COMPONENT_ID3_COMPONENT_ID3_MASK                        0x000000FF

#define CCI400_SNOOP_CONTROL_S0_OFFSET                                    0x00001000
#define CCI400_SNOOP_CONTROL_S0_TYPE                                      UInt32
#define CCI400_SNOOP_CONTROL_S0_RESERVED_MASK                             0x3FFFFFFD
#define    CCI400_SNOOP_CONTROL_S0_SUPPORT_DVMS_SHIFT                     31
#define    CCI400_SNOOP_CONTROL_S0_SUPPORT_DVMS_MASK                      0x80000000
#define    CCI400_SNOOP_CONTROL_S0_SUPPORT_SNOOPS_SHIFT                   30
#define    CCI400_SNOOP_CONTROL_S0_SUPPORT_SNOOPS_MASK                    0x40000000
#define    CCI400_SNOOP_CONTROL_S0_ENABLE_DVMS_SHIFT                      1
#define    CCI400_SNOOP_CONTROL_S0_ENABLE_DVMS_MASK                       0x00000002

#define CCI400_SHAREABLE_OVERRIDE_S0_OFFSET                               0x00001004
#define CCI400_SHAREABLE_OVERRIDE_S0_TYPE                                 UInt32
#define CCI400_SHAREABLE_OVERRIDE_S0_RESERVED_MASK                        0xFFFFFFFC
#define    CCI400_SHAREABLE_OVERRIDE_S0_AXDOMAIN_OVERRIDE_SHIFT           0
#define    CCI400_SHAREABLE_OVERRIDE_S0_AXDOMAIN_OVERRIDE_MASK            0x00000003

#define CCI400_READ_QOS_OVERRIDE_S0_OFFSET                                0x00001100
#define CCI400_READ_QOS_OVERRIDE_S0_TYPE                                  UInt32
#define CCI400_READ_QOS_OVERRIDE_S0_RESERVED_MASK                         0xFFFFF0F0
#define    CCI400_READ_QOS_OVERRIDE_S0_ARQOS_OVERRIDE_READBACK_SHIFT      8
#define    CCI400_READ_QOS_OVERRIDE_S0_ARQOS_OVERRIDE_READBACK_MASK       0x00000F00
#define    CCI400_READ_QOS_OVERRIDE_S0_ARQOS_VALUE_SHIFT                  0
#define    CCI400_READ_QOS_OVERRIDE_S0_ARQOS_VALUE_MASK                   0x0000000F

#define CCI400_WRITE_QOS_OVERRIDE_S0_OFFSET                               0x00001104
#define CCI400_WRITE_QOS_OVERRIDE_S0_TYPE                                 UInt32
#define CCI400_WRITE_QOS_OVERRIDE_S0_RESERVED_MASK                        0xFFFFF0F0
#define    CCI400_WRITE_QOS_OVERRIDE_S0_AWQOS_OVERRIDE_READBACK_SHIFT     8
#define    CCI400_WRITE_QOS_OVERRIDE_S0_AWQOS_OVERRIDE_READBACK_MASK      0x00000F00
#define    CCI400_WRITE_QOS_OVERRIDE_S0_AWQOS_VALUE_SHIFT                 0
#define    CCI400_WRITE_QOS_OVERRIDE_S0_AWQOS_VALUE_MASK                  0x0000000F

#define CCI400_QOS_CONTROL_S0_OFFSET                                      0x0000110C
#define CCI400_QOS_CONTROL_S0_TYPE                                        UInt32
#define CCI400_QOS_CONTROL_S0_RESERVED_MASK                               0xFFCEFFF0
#define    CCI400_QOS_CONTROL_S0_BANDWIDTH_REGULATION_MODE_SHIFT          21
#define    CCI400_QOS_CONTROL_S0_BANDWIDTH_REGULATION_MODE_MASK           0x00200000
#define    CCI400_QOS_CONTROL_S0_ARQOS_REGULATION_MODE_SHIFT              20
#define    CCI400_QOS_CONTROL_S0_ARQOS_REGULATION_MODE_MASK               0x00100000
#define    CCI400_QOS_CONTROL_S0_AWQOS_REGULATION_MODE_SHIFT              16
#define    CCI400_QOS_CONTROL_S0_AWQOS_REGULATION_MODE_MASK               0x00010000
#define    CCI400_QOS_CONTROL_S0_AR_OT_REGULATION_SHIFT                   3
#define    CCI400_QOS_CONTROL_S0_AR_OT_REGULATION_MASK                    0x00000008
#define    CCI400_QOS_CONTROL_S0_AW_OT_REGULATION_SHIFT                   2
#define    CCI400_QOS_CONTROL_S0_AW_OT_REGULATION_MASK                    0x00000004
#define    CCI400_QOS_CONTROL_S0_ARQOS_REGULATION_SHIFT                   1
#define    CCI400_QOS_CONTROL_S0_ARQOS_REGULATION_MASK                    0x00000002
#define    CCI400_QOS_CONTROL_S0_AWQOS_REGULATION_SHIFT                   0
#define    CCI400_QOS_CONTROL_S0_AWQOS_REGULATION_MASK                    0x00000001

#define CCI400_MAX_OT_S0_OFFSET                                           0x00001110
#define CCI400_MAX_OT_S0_TYPE                                             UInt32
#define CCI400_MAX_OT_S0_RESERVED_MASK                                    0xC000C000
#define    CCI400_MAX_OT_S0_INT_OT_AR_SHIFT                               24
#define    CCI400_MAX_OT_S0_INT_OT_AR_MASK                                0x3F000000
#define    CCI400_MAX_OT_S0_FRAC_OT_AR_SHIFT                              16
#define    CCI400_MAX_OT_S0_FRAC_OT_AR_MASK                               0x00FF0000
#define    CCI400_MAX_OT_S0_INT_OT_AW_SHIFT                               8
#define    CCI400_MAX_OT_S0_INT_OT_AW_MASK                                0x00003F00
#define    CCI400_MAX_OT_S0_FRAC_OT_AW_SHIFT                              0
#define    CCI400_MAX_OT_S0_FRAC_OT_AW_MASK                               0x000000FF

#define CCI400_TARGET_LATENCY_S0_OFFSET                                   0x00001130
#define CCI400_TARGET_LATENCY_S0_TYPE                                     UInt32
#define CCI400_TARGET_LATENCY_S0_RESERVED_MASK                            0xF000F000
#define    CCI400_TARGET_LATENCY_S0_AR_LAT_SHIFT                          16
#define    CCI400_TARGET_LATENCY_S0_AR_LAT_MASK                           0x0FFF0000
#define    CCI400_TARGET_LATENCY_S0_AW_LAT_SHIFT                          0
#define    CCI400_TARGET_LATENCY_S0_AW_LAT_MASK                           0x00000FFF

#define CCI400_LATENCY_REGULATION_S0_OFFSET                               0x00001134
#define CCI400_LATENCY_REGULATION_S0_TYPE                                 UInt32
#define CCI400_LATENCY_REGULATION_S0_RESERVED_MASK                        0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION_S0_AR_SCALE_FACT_SHIFT               8
#define    CCI400_LATENCY_REGULATION_S0_AR_SCALE_FACT_MASK                0x00000700
#define    CCI400_LATENCY_REGULATION_S0_AW_SCALE_FACT_SHIFT               0
#define    CCI400_LATENCY_REGULATION_S0_AW_SCALE_FACT_MASK                0x00000007

#define CCI400_QOS_RANGE_S0_OFFSET                                        0x00001138
#define CCI400_QOS_RANGE_S0_TYPE                                          UInt32
#define CCI400_QOS_RANGE_S0_RESERVED_MASK                                 0xF0F0F0F0
#define    CCI400_QOS_RANGE_S0_MAX_ARQOS_SHIFT                            24
#define    CCI400_QOS_RANGE_S0_MAX_ARQOS_MASK                             0x0F000000
#define    CCI400_QOS_RANGE_S0_MIN_ARQOS_SHIFT                            16
#define    CCI400_QOS_RANGE_S0_MIN_ARQOS_MASK                             0x000F0000
#define    CCI400_QOS_RANGE_S0_MAX_AWQOS_SHIFT                            8
#define    CCI400_QOS_RANGE_S0_MAX_AWQOS_MASK                             0x00000F00
#define    CCI400_QOS_RANGE_S0_MIN_AWQOS_SHIFT                            0
#define    CCI400_QOS_RANGE_S0_MIN_AWQOS_MASK                             0x0000000F

#define CCI400_SNOOP_CONTROL_S1_OFFSET                                    0x00002000
#define CCI400_SNOOP_CONTROL_S1_TYPE                                      UInt32
#define CCI400_SNOOP_CONTROL_S1_RESERVED_MASK                             0x3FFFFFFD
#define    CCI400_SNOOP_CONTROL_S1_SUPPORT_DVMS_SHIFT                     31
#define    CCI400_SNOOP_CONTROL_S1_SUPPORT_DVMS_MASK                      0x80000000
#define    CCI400_SNOOP_CONTROL_S1_SUPPORT_SNOOPS_SHIFT                   30
#define    CCI400_SNOOP_CONTROL_S1_SUPPORT_SNOOPS_MASK                    0x40000000
#define    CCI400_SNOOP_CONTROL_S1_ENABLE_DVMS_SHIFT                      1
#define    CCI400_SNOOP_CONTROL_S1_ENABLE_DVMS_MASK                       0x00000002

#define CCI400_SHAREABLE_OVERRIDE_S1_OFFSET                               0x00002004
#define CCI400_SHAREABLE_OVERRIDE_S1_TYPE                                 UInt32
#define CCI400_SHAREABLE_OVERRIDE_S1_RESERVED_MASK                        0xFFFFFFFC
#define    CCI400_SHAREABLE_OVERRIDE_S1_AXDOMAIN_OVERRIDE_SHIFT           0
#define    CCI400_SHAREABLE_OVERRIDE_S1_AXDOMAIN_OVERRIDE_MASK            0x00000003

#define CCI400_READ_QOS_OVERRIDE_S1_OFFSET                                0x00002100
#define CCI400_READ_QOS_OVERRIDE_S1_TYPE                                  UInt32
#define CCI400_READ_QOS_OVERRIDE_S1_RESERVED_MASK                         0xFFFFF0F0
#define    CCI400_READ_QOS_OVERRIDE_S1_ARQOS_OVERRIDE_READBACK_SHIFT      8
#define    CCI400_READ_QOS_OVERRIDE_S1_ARQOS_OVERRIDE_READBACK_MASK       0x00000F00
#define    CCI400_READ_QOS_OVERRIDE_S1_ARQOS_VALUE_SHIFT                  0
#define    CCI400_READ_QOS_OVERRIDE_S1_ARQOS_VALUE_MASK                   0x0000000F

#define CCI400_WRITE_QOS_OVERRIDE_S1_OFFSET                               0x00002104
#define CCI400_WRITE_QOS_OVERRIDE_S1_TYPE                                 UInt32
#define CCI400_WRITE_QOS_OVERRIDE_S1_RESERVED_MASK                        0xFFFFF0F0
#define    CCI400_WRITE_QOS_OVERRIDE_S1_AWQOS_OVERRIDE_READBACK_SHIFT     8
#define    CCI400_WRITE_QOS_OVERRIDE_S1_AWQOS_OVERRIDE_READBACK_MASK      0x00000F00
#define    CCI400_WRITE_QOS_OVERRIDE_S1_AWQOS_VALUE_SHIFT                 0
#define    CCI400_WRITE_QOS_OVERRIDE_S1_AWQOS_VALUE_MASK                  0x0000000F

#define CCI400_QOS_CONTROL_S1_OFFSET                                      0x0000210C
#define CCI400_QOS_CONTROL_S1_TYPE                                        UInt32
#define CCI400_QOS_CONTROL_S1_RESERVED_MASK                               0xFFCEFFF0
#define    CCI400_QOS_CONTROL_S1_BANDWIDTH_REGULATION_MODE_SHIFT          21
#define    CCI400_QOS_CONTROL_S1_BANDWIDTH_REGULATION_MODE_MASK           0x00200000
#define    CCI400_QOS_CONTROL_S1_ARQOS_REGULATION_MODE_SHIFT              20
#define    CCI400_QOS_CONTROL_S1_ARQOS_REGULATION_MODE_MASK               0x00100000
#define    CCI400_QOS_CONTROL_S1_AWQOS_REGULATION_MODE_SHIFT              16
#define    CCI400_QOS_CONTROL_S1_AWQOS_REGULATION_MODE_MASK               0x00010000
#define    CCI400_QOS_CONTROL_S1_AR_OT_REGULATION_SHIFT                   3
#define    CCI400_QOS_CONTROL_S1_AR_OT_REGULATION_MASK                    0x00000008
#define    CCI400_QOS_CONTROL_S1_AW_OT_REGULATION_SHIFT                   2
#define    CCI400_QOS_CONTROL_S1_AW_OT_REGULATION_MASK                    0x00000004
#define    CCI400_QOS_CONTROL_S1_ARQOS_REGULATION_SHIFT                   1
#define    CCI400_QOS_CONTROL_S1_ARQOS_REGULATION_MASK                    0x00000002
#define    CCI400_QOS_CONTROL_S1_AWQOS_REGULATION_SHIFT                   0
#define    CCI400_QOS_CONTROL_S1_AWQOS_REGULATION_MASK                    0x00000001

#define CCI400_MAX_OT_S1_OFFSET                                           0x00002110
#define CCI400_MAX_OT_S1_TYPE                                             UInt32
#define CCI400_MAX_OT_S1_RESERVED_MASK                                    0xC000C000
#define    CCI400_MAX_OT_S1_INT_OT_AR_SHIFT                               24
#define    CCI400_MAX_OT_S1_INT_OT_AR_MASK                                0x3F000000
#define    CCI400_MAX_OT_S1_FRAC_OT_AR_SHIFT                              16
#define    CCI400_MAX_OT_S1_FRAC_OT_AR_MASK                               0x00FF0000
#define    CCI400_MAX_OT_S1_INT_OT_AW_SHIFT                               8
#define    CCI400_MAX_OT_S1_INT_OT_AW_MASK                                0x00003F00
#define    CCI400_MAX_OT_S1_FRAC_OT_AW_SHIFT                              0
#define    CCI400_MAX_OT_S1_FRAC_OT_AW_MASK                               0x000000FF

#define CCI400_TARGET_LATENCY_S1_OFFSET                                   0x00002130
#define CCI400_TARGET_LATENCY_S1_TYPE                                     UInt32
#define CCI400_TARGET_LATENCY_S1_RESERVED_MASK                            0xF000F000
#define    CCI400_TARGET_LATENCY_S1_AR_LAT_SHIFT                          16
#define    CCI400_TARGET_LATENCY_S1_AR_LAT_MASK                           0x0FFF0000
#define    CCI400_TARGET_LATENCY_S1_AW_LAT_SHIFT                          0
#define    CCI400_TARGET_LATENCY_S1_AW_LAT_MASK                           0x00000FFF

#define CCI400_LATENCY_REGULATION_S1_OFFSET                               0x00002134
#define CCI400_LATENCY_REGULATION_S1_TYPE                                 UInt32
#define CCI400_LATENCY_REGULATION_S1_RESERVED_MASK                        0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION_S1_AR_SCALE_FACT_SHIFT               8
#define    CCI400_LATENCY_REGULATION_S1_AR_SCALE_FACT_MASK                0x00000700
#define    CCI400_LATENCY_REGULATION_S1_AW_SCALE_FACT_SHIFT               0
#define    CCI400_LATENCY_REGULATION_S1_AW_SCALE_FACT_MASK                0x00000007

#define CCI400_QOS_RANGE_S1_OFFSET                                        0x00002138
#define CCI400_QOS_RANGE_S1_TYPE                                          UInt32
#define CCI400_QOS_RANGE_S1_RESERVED_MASK                                 0xF0F0F0F0
#define    CCI400_QOS_RANGE_S1_MAX_ARQOS_SHIFT                            24
#define    CCI400_QOS_RANGE_S1_MAX_ARQOS_MASK                             0x0F000000
#define    CCI400_QOS_RANGE_S1_MIN_ARQOS_SHIFT                            16
#define    CCI400_QOS_RANGE_S1_MIN_ARQOS_MASK                             0x000F0000
#define    CCI400_QOS_RANGE_S1_MAX_AWQOS_SHIFT                            8
#define    CCI400_QOS_RANGE_S1_MAX_AWQOS_MASK                             0x00000F00
#define    CCI400_QOS_RANGE_S1_MIN_AWQOS_SHIFT                            0
#define    CCI400_QOS_RANGE_S1_MIN_AWQOS_MASK                             0x0000000F

#define CCI400_SNOOP_CONTROL_S2_OFFSET                                    0x00003000
#define CCI400_SNOOP_CONTROL_S2_TYPE                                      UInt32
#define CCI400_SNOOP_CONTROL_S2_RESERVED_MASK                             0x3FFFFFFD
#define    CCI400_SNOOP_CONTROL_S2_SUPPORT_DVMS_SHIFT                     31
#define    CCI400_SNOOP_CONTROL_S2_SUPPORT_DVMS_MASK                      0x80000000
#define    CCI400_SNOOP_CONTROL_S2_SUPPORT_SNOOPS_SHIFT                   30
#define    CCI400_SNOOP_CONTROL_S2_SUPPORT_SNOOPS_MASK                    0x40000000
#define    CCI400_SNOOP_CONTROL_S2_ENABLE_DVMS_SHIFT                      1
#define    CCI400_SNOOP_CONTROL_S2_ENABLE_DVMS_MASK                       0x00000002

#define CCI400_SHAREABLE_OVERRIDE_S2_OFFSET                               0x00003004
#define CCI400_SHAREABLE_OVERRIDE_S2_TYPE                                 UInt32
#define CCI400_SHAREABLE_OVERRIDE_S2_RESERVED_MASK                        0xFFFFFFFC
#define    CCI400_SHAREABLE_OVERRIDE_S2_AXDOMAIN_OVERRIDE_SHIFT           0
#define    CCI400_SHAREABLE_OVERRIDE_S2_AXDOMAIN_OVERRIDE_MASK            0x00000003

#define CCI400_READ_QOS_OVERRIDE_S2_OFFSET                                0x00003100
#define CCI400_READ_QOS_OVERRIDE_S2_TYPE                                  UInt32
#define CCI400_READ_QOS_OVERRIDE_S2_RESERVED_MASK                         0xFFFFF0F0
#define    CCI400_READ_QOS_OVERRIDE_S2_ARQOS_OVERRIDE_READBACK_SHIFT      8
#define    CCI400_READ_QOS_OVERRIDE_S2_ARQOS_OVERRIDE_READBACK_MASK       0x00000F00
#define    CCI400_READ_QOS_OVERRIDE_S2_ARQOS_VALUE_SHIFT                  0
#define    CCI400_READ_QOS_OVERRIDE_S2_ARQOS_VALUE_MASK                   0x0000000F

#define CCI400_WRITE_QOS_OVERRIDE_S2_OFFSET                               0x00003104
#define CCI400_WRITE_QOS_OVERRIDE_S2_TYPE                                 UInt32
#define CCI400_WRITE_QOS_OVERRIDE_S2_RESERVED_MASK                        0xFFFFF0F0
#define    CCI400_WRITE_QOS_OVERRIDE_S2_AWQOS_OVERRIDE_READBACK_SHIFT     8
#define    CCI400_WRITE_QOS_OVERRIDE_S2_AWQOS_OVERRIDE_READBACK_MASK      0x00000F00
#define    CCI400_WRITE_QOS_OVERRIDE_S2_AWQOS_VALUE_SHIFT                 0
#define    CCI400_WRITE_QOS_OVERRIDE_S2_AWQOS_VALUE_MASK                  0x0000000F

#define CCI400_QOS_CONTROL_S2_OFFSET                                      0x0000310C
#define CCI400_QOS_CONTROL_S2_TYPE                                        UInt32
#define CCI400_QOS_CONTROL_S2_RESERVED_MASK                               0xFFCEFFF0
#define    CCI400_QOS_CONTROL_S2_BANDWIDTH_REGULATION_MODE_SHIFT          21
#define    CCI400_QOS_CONTROL_S2_BANDWIDTH_REGULATION_MODE_MASK           0x00200000
#define    CCI400_QOS_CONTROL_S2_ARQOS_REGULATION_MODE_SHIFT              20
#define    CCI400_QOS_CONTROL_S2_ARQOS_REGULATION_MODE_MASK               0x00100000
#define    CCI400_QOS_CONTROL_S2_AWQOS_REGULATION_MODE_SHIFT              16
#define    CCI400_QOS_CONTROL_S2_AWQOS_REGULATION_MODE_MASK               0x00010000
#define    CCI400_QOS_CONTROL_S2_AR_OT_REGULATION_SHIFT                   3
#define    CCI400_QOS_CONTROL_S2_AR_OT_REGULATION_MASK                    0x00000008
#define    CCI400_QOS_CONTROL_S2_AW_OT_REGULATION_SHIFT                   2
#define    CCI400_QOS_CONTROL_S2_AW_OT_REGULATION_MASK                    0x00000004
#define    CCI400_QOS_CONTROL_S2_ARQOS_REGULATION_SHIFT                   1
#define    CCI400_QOS_CONTROL_S2_ARQOS_REGULATION_MASK                    0x00000002
#define    CCI400_QOS_CONTROL_S2_AWQOS_REGULATION_SHIFT                   0
#define    CCI400_QOS_CONTROL_S2_AWQOS_REGULATION_MASK                    0x00000001

#define CCI400_MAX_OT_S2_OFFSET                                           0x00003110
#define CCI400_MAX_OT_S2_TYPE                                             UInt32
#define CCI400_MAX_OT_S2_RESERVED_MASK                                    0xC000C000
#define    CCI400_MAX_OT_S2_INT_OT_AR_SHIFT                               24
#define    CCI400_MAX_OT_S2_INT_OT_AR_MASK                                0x3F000000
#define    CCI400_MAX_OT_S2_FRAC_OT_AR_SHIFT                              16
#define    CCI400_MAX_OT_S2_FRAC_OT_AR_MASK                               0x00FF0000
#define    CCI400_MAX_OT_S2_INT_OT_AW_SHIFT                               8
#define    CCI400_MAX_OT_S2_INT_OT_AW_MASK                                0x00003F00
#define    CCI400_MAX_OT_S2_FRAC_OT_AW_SHIFT                              0
#define    CCI400_MAX_OT_S2_FRAC_OT_AW_MASK                               0x000000FF

#define CCI400_TARGET_LATENCY_S2_OFFSET                                   0x00003130
#define CCI400_TARGET_LATENCY_S2_TYPE                                     UInt32
#define CCI400_TARGET_LATENCY_S2_RESERVED_MASK                            0xF000F000
#define    CCI400_TARGET_LATENCY_S2_AR_LAT_SHIFT                          16
#define    CCI400_TARGET_LATENCY_S2_AR_LAT_MASK                           0x0FFF0000
#define    CCI400_TARGET_LATENCY_S2_AW_LAT_SHIFT                          0
#define    CCI400_TARGET_LATENCY_S2_AW_LAT_MASK                           0x00000FFF

#define CCI400_LATENCY_REGULATION_S2_OFFSET                               0x00003134
#define CCI400_LATENCY_REGULATION_S2_TYPE                                 UInt32
#define CCI400_LATENCY_REGULATION_S2_RESERVED_MASK                        0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION_S2_AR_SCALE_FACT_SHIFT               8
#define    CCI400_LATENCY_REGULATION_S2_AR_SCALE_FACT_MASK                0x00000700
#define    CCI400_LATENCY_REGULATION_S2_AW_SCALE_FACT_SHIFT               0
#define    CCI400_LATENCY_REGULATION_S2_AW_SCALE_FACT_MASK                0x00000007

#define CCI400_QOS_RANGE_S2_OFFSET                                        0x00003138
#define CCI400_QOS_RANGE_S2_TYPE                                          UInt32
#define CCI400_QOS_RANGE_S2_RESERVED_MASK                                 0xF0F0F0F0
#define    CCI400_QOS_RANGE_S2_MAX_ARQOS_SHIFT                            24
#define    CCI400_QOS_RANGE_S2_MAX_ARQOS_MASK                             0x0F000000
#define    CCI400_QOS_RANGE_S2_MIN_ARQOS_SHIFT                            16
#define    CCI400_QOS_RANGE_S2_MIN_ARQOS_MASK                             0x000F0000
#define    CCI400_QOS_RANGE_S2_MAX_AWQOS_SHIFT                            8
#define    CCI400_QOS_RANGE_S2_MAX_AWQOS_MASK                             0x00000F00
#define    CCI400_QOS_RANGE_S2_MIN_AWQOS_SHIFT                            0
#define    CCI400_QOS_RANGE_S2_MIN_AWQOS_MASK                             0x0000000F

#define CCI400_SNOOP_CONTROL_S3_OFFSET                                    0x00004000
#define CCI400_SNOOP_CONTROL_S3_TYPE                                      UInt32
#define CCI400_SNOOP_CONTROL_S3_RESERVED_MASK                             0x3FFFFFFC
#define    CCI400_SNOOP_CONTROL_S3_SUPPORT_DVMS_SHIFT                     31
#define    CCI400_SNOOP_CONTROL_S3_SUPPORT_DVMS_MASK                      0x80000000
#define    CCI400_SNOOP_CONTROL_S3_SUPPORT_SNOOPS_SHIFT                   30
#define    CCI400_SNOOP_CONTROL_S3_SUPPORT_SNOOPS_MASK                    0x40000000
#define    CCI400_SNOOP_CONTROL_S3_ENABLE_DVMS_SHIFT                      1
#define    CCI400_SNOOP_CONTROL_S3_ENABLE_DVMS_MASK                       0x00000002
#define    CCI400_SNOOP_CONTROL_S3_ENABLE_SNOOPS_SHIFT                    0
#define    CCI400_SNOOP_CONTROL_S3_ENABLE_SNOOPS_MASK                     0x00000001

#define CCI400_READ_QOS_OVERRIDE_S3_OFFSET                                0x00004100
#define CCI400_READ_QOS_OVERRIDE_S3_TYPE                                  UInt32
#define CCI400_READ_QOS_OVERRIDE_S3_RESERVED_MASK                         0xFFFFF0F0
#define    CCI400_READ_QOS_OVERRIDE_S3_ARQOS_OVERRIDE_READBACK_SHIFT      8
#define    CCI400_READ_QOS_OVERRIDE_S3_ARQOS_OVERRIDE_READBACK_MASK       0x00000F00
#define    CCI400_READ_QOS_OVERRIDE_S3_ARQOS_VALUE_SHIFT                  0
#define    CCI400_READ_QOS_OVERRIDE_S3_ARQOS_VALUE_MASK                   0x0000000F

#define CCI400_WRITE_QOS_OVERRIDE_S3_OFFSET                               0x00004104
#define CCI400_WRITE_QOS_OVERRIDE_S3_TYPE                                 UInt32
#define CCI400_WRITE_QOS_OVERRIDE_S3_RESERVED_MASK                        0xFFFFF0F0
#define    CCI400_WRITE_QOS_OVERRIDE_S3_AWQOS_OVERRIDE_READBACK_SHIFT     8
#define    CCI400_WRITE_QOS_OVERRIDE_S3_AWQOS_OVERRIDE_READBACK_MASK      0x00000F00
#define    CCI400_WRITE_QOS_OVERRIDE_S3_AWQOS_VALUE_SHIFT                 0
#define    CCI400_WRITE_QOS_OVERRIDE_S3_AWQOS_VALUE_MASK                  0x0000000F

#define CCI400_QOS_CONTROL_S3_OFFSET                                      0x0000410C
#define CCI400_QOS_CONTROL_S3_TYPE                                        UInt32
#define CCI400_QOS_CONTROL_S3_RESERVED_MASK                               0xFFCEFFFC
#define    CCI400_QOS_CONTROL_S3_BANDWIDTH_REGULATION_MODE_SHIFT          21
#define    CCI400_QOS_CONTROL_S3_BANDWIDTH_REGULATION_MODE_MASK           0x00200000
#define    CCI400_QOS_CONTROL_S3_ARQOS_REGULATION_MODE_SHIFT              20
#define    CCI400_QOS_CONTROL_S3_ARQOS_REGULATION_MODE_MASK               0x00100000
#define    CCI400_QOS_CONTROL_S3_AWQOS_REGULATION_MODE_SHIFT              16
#define    CCI400_QOS_CONTROL_S3_AWQOS_REGULATION_MODE_MASK               0x00010000
#define    CCI400_QOS_CONTROL_S3_ARQOS_REGULATION_SHIFT                   1
#define    CCI400_QOS_CONTROL_S3_ARQOS_REGULATION_MASK                    0x00000002
#define    CCI400_QOS_CONTROL_S3_AWQOS_REGULATION_SHIFT                   0
#define    CCI400_QOS_CONTROL_S3_AWQOS_REGULATION_MASK                    0x00000001

#define CCI400_TARGET_LATENCY_S3_OFFSET                                   0x00004130
#define CCI400_TARGET_LATENCY_S3_TYPE                                     UInt32
#define CCI400_TARGET_LATENCY_S3_RESERVED_MASK                            0xF000F000
#define    CCI400_TARGET_LATENCY_S3_AR_LAT_SHIFT                          16
#define    CCI400_TARGET_LATENCY_S3_AR_LAT_MASK                           0x0FFF0000
#define    CCI400_TARGET_LATENCY_S3_AW_LAT_SHIFT                          0
#define    CCI400_TARGET_LATENCY_S3_AW_LAT_MASK                           0x00000FFF

#define CCI400_LATENCY_REGULATION_S3_OFFSET                               0x00004134
#define CCI400_LATENCY_REGULATION_S3_TYPE                                 UInt32
#define CCI400_LATENCY_REGULATION_S3_RESERVED_MASK                        0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION_S3_AR_SCALE_FACT_SHIFT               8
#define    CCI400_LATENCY_REGULATION_S3_AR_SCALE_FACT_MASK                0x00000700
#define    CCI400_LATENCY_REGULATION_S3_AW_SCALE_FACT_SHIFT               0
#define    CCI400_LATENCY_REGULATION_S3_AW_SCALE_FACT_MASK                0x00000007

#define CCI400_QOS_RANGE_S3_OFFSET                                        0x00004138
#define CCI400_QOS_RANGE_S3_TYPE                                          UInt32
#define CCI400_QOS_RANGE_S3_RESERVED_MASK                                 0xF0F0F0F0
#define    CCI400_QOS_RANGE_S3_MAX_ARQOS_SHIFT                            24
#define    CCI400_QOS_RANGE_S3_MAX_ARQOS_MASK                             0x0F000000
#define    CCI400_QOS_RANGE_S3_MIN_ARQOS_SHIFT                            16
#define    CCI400_QOS_RANGE_S3_MIN_ARQOS_MASK                             0x000F0000
#define    CCI400_QOS_RANGE_S3_MAX_AWQOS_SHIFT                            8
#define    CCI400_QOS_RANGE_S3_MAX_AWQOS_MASK                             0x00000F00
#define    CCI400_QOS_RANGE_S3_MIN_AWQOS_SHIFT                            0
#define    CCI400_QOS_RANGE_S3_MIN_AWQOS_MASK                             0x0000000F

#define CCI400_SNOOP_CONTROL_S4_OFFSET                                    0x00005000
#define CCI400_SNOOP_CONTROL_S4_TYPE                                      UInt32
#define CCI400_SNOOP_CONTROL_S4_RESERVED_MASK                             0x3FFFFFFC
#define    CCI400_SNOOP_CONTROL_S4_SUPPORT_DVMS_SHIFT                     31
#define    CCI400_SNOOP_CONTROL_S4_SUPPORT_DVMS_MASK                      0x80000000
#define    CCI400_SNOOP_CONTROL_S4_SUPPORT_SNOOPS_SHIFT                   30
#define    CCI400_SNOOP_CONTROL_S4_SUPPORT_SNOOPS_MASK                    0x40000000
#define    CCI400_SNOOP_CONTROL_S4_ENABLE_DVMS_SHIFT                      1
#define    CCI400_SNOOP_CONTROL_S4_ENABLE_DVMS_MASK                       0x00000002
#define    CCI400_SNOOP_CONTROL_S4_ENABLE_SNOOPS_SHIFT                    0
#define    CCI400_SNOOP_CONTROL_S4_ENABLE_SNOOPS_MASK                     0x00000001

#define CCI400_READ_QOS_OVERRIDE_S4_OFFSET                                0x00005100
#define CCI400_READ_QOS_OVERRIDE_S4_TYPE                                  UInt32
#define CCI400_READ_QOS_OVERRIDE_S4_RESERVED_MASK                         0xFFFFF0F0
#define    CCI400_READ_QOS_OVERRIDE_S4_ARQOS_OVERRIDE_READBACK_SHIFT      8
#define    CCI400_READ_QOS_OVERRIDE_S4_ARQOS_OVERRIDE_READBACK_MASK       0x00000F00
#define    CCI400_READ_QOS_OVERRIDE_S4_ARQOS_VALUE_SHIFT                  0
#define    CCI400_READ_QOS_OVERRIDE_S4_ARQOS_VALUE_MASK                   0x0000000F

#define CCI400_WRITE_QOS_OVERRIDE_S4_OFFSET                               0x00005104
#define CCI400_WRITE_QOS_OVERRIDE_S4_TYPE                                 UInt32
#define CCI400_WRITE_QOS_OVERRIDE_S4_RESERVED_MASK                        0xFFFFF0F0
#define    CCI400_WRITE_QOS_OVERRIDE_S4_AWQOS_OVERRIDE_READBACK_SHIFT     8
#define    CCI400_WRITE_QOS_OVERRIDE_S4_AWQOS_OVERRIDE_READBACK_MASK      0x00000F00
#define    CCI400_WRITE_QOS_OVERRIDE_S4_AWQOS_VALUE_SHIFT                 0
#define    CCI400_WRITE_QOS_OVERRIDE_S4_AWQOS_VALUE_MASK                  0x0000000F

#define CCI400_QOS_CONTROL_S4_OFFSET                                      0x0000510C
#define CCI400_QOS_CONTROL_S4_TYPE                                        UInt32
#define CCI400_QOS_CONTROL_S4_RESERVED_MASK                               0xFFCEFFFC
#define    CCI400_QOS_CONTROL_S4_BANDWIDTH_REGULATION_MODE_SHIFT          21
#define    CCI400_QOS_CONTROL_S4_BANDWIDTH_REGULATION_MODE_MASK           0x00200000
#define    CCI400_QOS_CONTROL_S4_ARQOS_REGULATION_MODE_SHIFT              20
#define    CCI400_QOS_CONTROL_S4_ARQOS_REGULATION_MODE_MASK               0x00100000
#define    CCI400_QOS_CONTROL_S4_AWQOS_REGULATION_MODE_SHIFT              16
#define    CCI400_QOS_CONTROL_S4_AWQOS_REGULATION_MODE_MASK               0x00010000
#define    CCI400_QOS_CONTROL_S4_ARQOS_REGULATION_SHIFT                   1
#define    CCI400_QOS_CONTROL_S4_ARQOS_REGULATION_MASK                    0x00000002
#define    CCI400_QOS_CONTROL_S4_AWQOS_REGULATION_SHIFT                   0
#define    CCI400_QOS_CONTROL_S4_AWQOS_REGULATION_MASK                    0x00000001

#define CCI400_TARGET_LATENCY_S4_OFFSET                                   0x00005130
#define CCI400_TARGET_LATENCY_S4_TYPE                                     UInt32
#define CCI400_TARGET_LATENCY_S4_RESERVED_MASK                            0xF000F000
#define    CCI400_TARGET_LATENCY_S4_AR_LAT_SHIFT                          16
#define    CCI400_TARGET_LATENCY_S4_AR_LAT_MASK                           0x0FFF0000
#define    CCI400_TARGET_LATENCY_S4_AW_LAT_SHIFT                          0
#define    CCI400_TARGET_LATENCY_S4_AW_LAT_MASK                           0x00000FFF

#define CCI400_LATENCY_REGULATION_S4_OFFSET                               0x00005134
#define CCI400_LATENCY_REGULATION_S4_TYPE                                 UInt32
#define CCI400_LATENCY_REGULATION_S4_RESERVED_MASK                        0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION_S4_AR_SCALE_FACT_SHIFT               8
#define    CCI400_LATENCY_REGULATION_S4_AR_SCALE_FACT_MASK                0x00000700
#define    CCI400_LATENCY_REGULATION_S4_AW_SCALE_FACT_SHIFT               0
#define    CCI400_LATENCY_REGULATION_S4_AW_SCALE_FACT_MASK                0x00000007

#define CCI400_QOS_RANGE_S4_OFFSET                                        0x00005138
#define CCI400_QOS_RANGE_S4_TYPE                                          UInt32
#define CCI400_QOS_RANGE_S4_RESERVED_MASK                                 0xF0F0F0F0
#define    CCI400_QOS_RANGE_S4_MAX_ARQOS_SHIFT                            24
#define    CCI400_QOS_RANGE_S4_MAX_ARQOS_MASK                             0x0F000000
#define    CCI400_QOS_RANGE_S4_MIN_ARQOS_SHIFT                            16
#define    CCI400_QOS_RANGE_S4_MIN_ARQOS_MASK                             0x000F0000
#define    CCI400_QOS_RANGE_S4_MAX_AWQOS_SHIFT                            8
#define    CCI400_QOS_RANGE_S4_MAX_AWQOS_MASK                             0x00000F00
#define    CCI400_QOS_RANGE_S4_MIN_AWQOS_SHIFT                            0
#define    CCI400_QOS_RANGE_S4_MIN_AWQOS_MASK                             0x0000000F

#define CCI400_CYCLE_COUNTER_OFFSET                                       0x00009004
#define CCI400_CYCLE_COUNTER_TYPE                                         UInt32
#define CCI400_CYCLE_COUNTER_RESERVED_MASK                                0x00000000
#define    CCI400_CYCLE_COUNTER_CCNT_SHIFT                                0
#define    CCI400_CYCLE_COUNTER_CCNT_MASK                                 0xFFFFFFFF

#define CCI400_CYCLE_COUNTER_CONTROL_OFFSET                               0x00009008
#define CCI400_CYCLE_COUNTER_CONTROL_TYPE                                 UInt32
#define CCI400_CYCLE_COUNTER_CONTROL_RESERVED_MASK                        0xFFFFFFFE
#define    CCI400_CYCLE_COUNTER_CONTROL_CCNT_EN_SHIFT                     0
#define    CCI400_CYCLE_COUNTER_CONTROL_CCNT_EN_MASK                      0x00000001

#define CCI400_CYCLE_COUNT_OVERFLOW_OFFSET                                0x0000900C
#define CCI400_CYCLE_COUNT_OVERFLOW_TYPE                                  UInt32
#define CCI400_CYCLE_COUNT_OVERFLOW_RESERVED_MASK                         0xFFFFFFFE
#define    CCI400_CYCLE_COUNT_OVERFLOW_CCNT_OVERFLOW_SHIFT                0
#define    CCI400_CYCLE_COUNT_OVERFLOW_CCNT_OVERFLOW_MASK                 0x00000001

#define CCI400_ESR0_OFFSET                                                0x0000A000
#define CCI400_ESR0_TYPE                                                  UInt32
#define CCI400_ESR0_RESERVED_MASK                                         0xFFFFFF00
#define    CCI400_ESR0_EVT_CNT0_SHIFT                                     0
#define    CCI400_ESR0_EVT_CNT0_MASK                                      0x000000FF

#define CCI400_EVENT_COUNTER0_OFFSET                                      0x0000A004
#define CCI400_EVENT_COUNTER0_TYPE                                        UInt32
#define CCI400_EVENT_COUNTER0_RESERVED_MASK                               0x00000000
#define    CCI400_EVENT_COUNTER0_CNT0_SHIFT                               0
#define    CCI400_EVENT_COUNTER0_CNT0_MASK                                0xFFFFFFFF

#define CCI400_EVENT_COUNTER0_CONTROL_OFFSET                              0x0000A008
#define CCI400_EVENT_COUNTER0_CONTROL_TYPE                                UInt32
#define CCI400_EVENT_COUNTER0_CONTROL_RESERVED_MASK                       0xFFFFFFFE
#define    CCI400_EVENT_COUNTER0_CONTROL_CNT0_EN_SHIFT                    0
#define    CCI400_EVENT_COUNTER0_CONTROL_CNT0_EN_MASK                     0x00000001

#define CCI400_EVENT_COUNTER0_OVERFLOW_OFFSET                             0x0000A00C
#define CCI400_EVENT_COUNTER0_OVERFLOW_TYPE                               UInt32
#define CCI400_EVENT_COUNTER0_OVERFLOW_RESERVED_MASK                      0xFFFFFFFE
#define    CCI400_EVENT_COUNTER0_OVERFLOW_CNT0_OVERFLOW_SHIFT             0
#define    CCI400_EVENT_COUNTER0_OVERFLOW_CNT0_OVERFLOW_MASK              0x00000001

#define CCI400_ESR1_OFFSET                                                0x0000B000
#define CCI400_ESR1_TYPE                                                  UInt32
#define CCI400_ESR1_RESERVED_MASK                                         0xFFFFFF00
#define    CCI400_ESR1_EVT_CNT1_SHIFT                                     0
#define    CCI400_ESR1_EVT_CNT1_MASK                                      0x000000FF

#define CCI400_EVENT_COUNTER1_OFFSET                                      0x0000B004
#define CCI400_EVENT_COUNTER1_TYPE                                        UInt32
#define CCI400_EVENT_COUNTER1_RESERVED_MASK                               0x00000000
#define    CCI400_EVENT_COUNTER1_CNT1_SHIFT                               0
#define    CCI400_EVENT_COUNTER1_CNT1_MASK                                0xFFFFFFFF

#define CCI400_EVENT_COUNTER1_CONTROL_OFFSET                              0x0000B008
#define CCI400_EVENT_COUNTER1_CONTROL_TYPE                                UInt32
#define CCI400_EVENT_COUNTER1_CONTROL_RESERVED_MASK                       0xFFFFFFFE
#define    CCI400_EVENT_COUNTER1_CONTROL_CNT1_EN_SHIFT                    0
#define    CCI400_EVENT_COUNTER1_CONTROL_CNT1_EN_MASK                     0x00000001

#define CCI400_EVENT_COUNTER1_OVERFLOW_OFFSET                             0x0000B00C
#define CCI400_EVENT_COUNTER1_OVERFLOW_TYPE                               UInt32
#define CCI400_EVENT_COUNTER1_OVERFLOW_RESERVED_MASK                      0xFFFFFFFE
#define    CCI400_EVENT_COUNTER1_OVERFLOW_CNT1_OVERFLOW_SHIFT             0
#define    CCI400_EVENT_COUNTER1_OVERFLOW_CNT1_OVERFLOW_MASK              0x00000001

#define CCI400_ESR2_OFFSET                                                0x0000C000
#define CCI400_ESR2_TYPE                                                  UInt32
#define CCI400_ESR2_RESERVED_MASK                                         0xFFFFFF00
#define    CCI400_ESR2_EVT_CNT2_SHIFT                                     0
#define    CCI400_ESR2_EVT_CNT2_MASK                                      0x000000FF

#define CCI400_EVENT_COUNTER2_OFFSET                                      0x0000C004
#define CCI400_EVENT_COUNTER2_TYPE                                        UInt32
#define CCI400_EVENT_COUNTER2_RESERVED_MASK                               0x00000000
#define    CCI400_EVENT_COUNTER2_CNT2_SHIFT                               0
#define    CCI400_EVENT_COUNTER2_CNT2_MASK                                0xFFFFFFFF

#define CCI400_EVENT_COUNTER2_CONTROL_OFFSET                              0x0000C008
#define CCI400_EVENT_COUNTER2_CONTROL_TYPE                                UInt32
#define CCI400_EVENT_COUNTER2_CONTROL_RESERVED_MASK                       0xFFFFFFFE
#define    CCI400_EVENT_COUNTER2_CONTROL_CNT2_EN_SHIFT                    0
#define    CCI400_EVENT_COUNTER2_CONTROL_CNT2_EN_MASK                     0x00000001

#define CCI400_EVENT_COUNTER2_OVERFLOW_OFFSET                             0x0000C00C
#define CCI400_EVENT_COUNTER2_OVERFLOW_TYPE                               UInt32
#define CCI400_EVENT_COUNTER2_OVERFLOW_RESERVED_MASK                      0xFFFFFFFE
#define    CCI400_EVENT_COUNTER2_OVERFLOW_CNT2_OVERFLOW_SHIFT             0
#define    CCI400_EVENT_COUNTER2_OVERFLOW_CNT2_OVERFLOW_MASK              0x00000001

#define CCI400_ESR3_OFFSET                                                0x0000D000
#define CCI400_ESR3_TYPE                                                  UInt32
#define CCI400_ESR3_RESERVED_MASK                                         0xFFFFFF00
#define    CCI400_ESR3_EVT_CNT3_SHIFT                                     0
#define    CCI400_ESR3_EVT_CNT3_MASK                                      0x000000FF

#define CCI400_EVENT_COUNTER3_OFFSET                                      0x0000D004
#define CCI400_EVENT_COUNTER3_TYPE                                        UInt32
#define CCI400_EVENT_COUNTER3_RESERVED_MASK                               0x00000000
#define    CCI400_EVENT_COUNTER3_CNT3_SHIFT                               0
#define    CCI400_EVENT_COUNTER3_CNT3_MASK                                0xFFFFFFFF

#define CCI400_EVENT_COUNTER3_CONTROL_OFFSET                              0x0000D008
#define CCI400_EVENT_COUNTER3_CONTROL_TYPE                                UInt32
#define CCI400_EVENT_COUNTER3_CONTROL_RESERVED_MASK                       0xFFFFFFFE
#define    CCI400_EVENT_COUNTER3_CONTROL_CNT3_EN_SHIFT                    0
#define    CCI400_EVENT_COUNTER3_CONTROL_CNT3_EN_MASK                     0x00000001

#define CCI400_EVENT_COUNTER3_OVERFLOW_OFFSET                             0x0000D00C
#define CCI400_EVENT_COUNTER3_OVERFLOW_TYPE                               UInt32
#define CCI400_EVENT_COUNTER3_OVERFLOW_RESERVED_MASK                      0xFFFFFFFE
#define    CCI400_EVENT_COUNTER3_OVERFLOW_CNT3_OVERFLOW_SHIFT             0
#define    CCI400_EVENT_COUNTER3_OVERFLOW_CNT3_OVERFLOW_MASK              0x00000001

#endif /* __BRCM_RDB_CCI400_H__ */



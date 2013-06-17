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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_CCI400_H__
#define __BRCM_RDB_CCI400_H__

#define CCI400_CTRL_OVERRIDE_OFFSET                                       0x00000000
#define CCI400_CTRL_OVERRIDE_TYPE                                         UInt32
#define CCI400_CTRL_OVERRIDE_RESERVED_MASK                                0xFFFFFFF0
#define    CCI400_CTRL_OVERRIDE_BARRIERTERMINATE_SHIFT                    3
#define    CCI400_CTRL_OVERRIDE_BARRIERTERMINATE_MASK                     0x00000008
#define    CCI400_CTRL_OVERRIDE_DISABLE_SPEC_FETCH_SHIFT                  2
#define    CCI400_CTRL_OVERRIDE_DISABLE_SPEC_FETCH_MASK                   0x00000004
#define    CCI400_CTRL_OVERRIDE_DISABLE_DVM_SHIFT                         1
#define    CCI400_CTRL_OVERRIDE_DISABLE_DVM_MASK                          0x00000002
#define    CCI400_CTRL_OVERRIDE_DISABLE_SNOOP_SHIFT                       0
#define    CCI400_CTRL_OVERRIDE_DISABLE_SNOOP_MASK                        0x00000001

#define CCI400_SPEC_CTRL_OFFSET                                           0x00000004
#define CCI400_SPEC_CTRL_TYPE                                             UInt32
#define CCI400_SPEC_CTRL_RESERVED_MASK                                    0xFFFFFFF8
#define    CCI400_SPEC_CTRL_DISABLE_SPEC_FETCH_M2_SHIFT                   2
#define    CCI400_SPEC_CTRL_DISABLE_SPEC_FETCH_M2_MASK                    0x00000004
#define    CCI400_SPEC_CTRL_DISABLE_SPEC_FETCH_M1_SHIFT                   1
#define    CCI400_SPEC_CTRL_DISABLE_SPEC_FETCH_M1_MASK                    0x00000002
#define    CCI400_SPEC_CTRL_DISABLE_SPEC_FETCH_M0_SHIFT                   0
#define    CCI400_SPEC_CTRL_DISABLE_SPEC_FETCH_M0_MASK                    0x00000001

#define CCI400_SEC_ACCESS_OFFSET                                          0x00000008
#define CCI400_SEC_ACCESS_TYPE                                            UInt32
#define CCI400_SEC_ACCESS_RESERVED_MASK                                   0xFFFFFFFE
#define    CCI400_SEC_ACCESS_SEC_ACCESS_CTRL_SHIFT                        0
#define    CCI400_SEC_ACCESS_SEC_ACCESS_CTRL_MASK                         0x00000001

#define CCI400_STATUS_OFFSET                                              0x0000000C
#define CCI400_STATUS_TYPE                                                UInt32
#define CCI400_STATUS_RESERVED_MASK                                       0xFFFFFFFE
#define    CCI400_STATUS_CCI_STATUS_SHIFT                                 0
#define    CCI400_STATUS_CCI_STATUS_MASK                                  0x00000001

#define CCI400_ERROR_OFFSET                                               0x00000010
#define CCI400_ERROR_TYPE                                                 UInt32
#define CCI400_ERROR_RESERVED_MASK                                        0xFFE0FFF8
#define    CCI400_ERROR_ERROR_S4_SHIFT                                    20
#define    CCI400_ERROR_ERROR_S4_MASK                                     0x00100000
#define    CCI400_ERROR_ERROR_S3_SHIFT                                    19
#define    CCI400_ERROR_ERROR_S3_MASK                                     0x00080000
#define    CCI400_ERROR_ERROR_S2_SHIFT                                    18
#define    CCI400_ERROR_ERROR_S2_MASK                                     0x00040000
#define    CCI400_ERROR_ERROR_S1_SHIFT                                    17
#define    CCI400_ERROR_ERROR_S1_MASK                                     0x00020000
#define    CCI400_ERROR_ERROR_S0_SHIFT                                    16
#define    CCI400_ERROR_ERROR_S0_MASK                                     0x00010000
#define    CCI400_ERROR_ERROR_M2_SHIFT                                    2
#define    CCI400_ERROR_ERROR_M2_MASK                                     0x00000004
#define    CCI400_ERROR_ERROR_M1_SHIFT                                    1
#define    CCI400_ERROR_ERROR_M1_MASK                                     0x00000002
#define    CCI400_ERROR_ERROR_M0_SHIFT                                    0
#define    CCI400_ERROR_ERROR_M0_MASK                                     0x00000001

#define CCI400_PERF_MON_CTRL_OFFSET                                       0x00000100
#define CCI400_PERF_MON_CTRL_TYPE                                         UInt32
#define CCI400_PERF_MON_CTRL_RESERVED_MASK                                0xFFFF07C0
#define    CCI400_PERF_MON_CTRL_NUM_CORES_SHIFT                           11
#define    CCI400_PERF_MON_CTRL_NUM_CORES_MASK                            0x0000F800
#define    CCI400_PERF_MON_CTRL_DP_SHIFT                                  5
#define    CCI400_PERF_MON_CTRL_DP_MASK                                   0x00000020
#define    CCI400_PERF_MON_CTRL_EX_SHIFT                                  4
#define    CCI400_PERF_MON_CTRL_EX_MASK                                   0x00000010
#define    CCI400_PERF_MON_CTRL_CCD_SHIFT                                 3
#define    CCI400_PERF_MON_CTRL_CCD_MASK                                  0x00000008
#define    CCI400_PERF_MON_CTRL_CCR_SHIFT                                 2
#define    CCI400_PERF_MON_CTRL_CCR_MASK                                  0x00000004
#define    CCI400_PERF_MON_CTRL_RST_SHIFT                                 1
#define    CCI400_PERF_MON_CTRL_RST_MASK                                  0x00000002
#define    CCI400_PERF_MON_CTRL_CEN_SHIFT                                 0
#define    CCI400_PERF_MON_CTRL_CEN_MASK                                  0x00000001

#define CCI400_PERIPH_ID0_OFFSET                                          0x00000FD0
#define CCI400_PERIPH_ID0_TYPE                                            UInt32
#define CCI400_PERIPH_ID0_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID1_OFFSET                                          0x00000FD4
#define CCI400_PERIPH_ID1_TYPE                                            UInt32
#define CCI400_PERIPH_ID1_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID2_OFFSET                                          0x00000FD8
#define CCI400_PERIPH_ID2_TYPE                                            UInt32
#define CCI400_PERIPH_ID2_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID3_OFFSET                                          0x00000FDC
#define CCI400_PERIPH_ID3_TYPE                                            UInt32
#define CCI400_PERIPH_ID3_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID4_OFFSET                                          0x00000FE0
#define CCI400_PERIPH_ID4_TYPE                                            UInt32
#define CCI400_PERIPH_ID4_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID5_OFFSET                                          0x00000FE4
#define CCI400_PERIPH_ID5_TYPE                                            UInt32
#define CCI400_PERIPH_ID5_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID6_OFFSET                                          0x00000FE8
#define CCI400_PERIPH_ID6_TYPE                                            UInt32
#define CCI400_PERIPH_ID6_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID7_OFFSET                                          0x00000FEC
#define CCI400_PERIPH_ID7_TYPE                                            UInt32
#define CCI400_PERIPH_ID7_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID8_OFFSET                                          0x00000FF0
#define CCI400_PERIPH_ID8_TYPE                                            UInt32
#define CCI400_PERIPH_ID8_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID9_OFFSET                                          0x00000FF4
#define CCI400_PERIPH_ID9_TYPE                                            UInt32
#define CCI400_PERIPH_ID9_RESERVED_MASK                                   0xFFFFFFFF

#define CCI400_PERIPH_ID10_OFFSET                                         0x00000FF8
#define CCI400_PERIPH_ID10_TYPE                                           UInt32
#define CCI400_PERIPH_ID10_RESERVED_MASK                                  0xFFFFFFFF

#define CCI400_PERIPH_ID11_OFFSET                                         0x00000FFC
#define CCI400_PERIPH_ID11_TYPE                                           UInt32
#define CCI400_PERIPH_ID11_RESERVED_MASK                                  0xFFFFFFFF

#define CCI400_SNOOP_CTRL0_OFFSET                                         0x00001000
#define CCI400_SNOOP_CTRL0_TYPE                                           UInt32
#define CCI400_SNOOP_CTRL0_RESERVED_MASK                                  0x3FFFFFFC
#define    CCI400_SNOOP_CTRL0_ACCHANNELEN_DVM_SHIFT                       31
#define    CCI400_SNOOP_CTRL0_ACCHANNELEN_DVM_MASK                        0x80000000
#define    CCI400_SNOOP_CTRL0_ACCHANNELEN_SNOOP_SHIFT                     30
#define    CCI400_SNOOP_CTRL0_ACCHANNELEN_SNOOP_MASK                      0x40000000
#define    CCI400_SNOOP_CTRL0_DVM_EN_SHIFT                                1
#define    CCI400_SNOOP_CTRL0_DVM_EN_MASK                                 0x00000002
#define    CCI400_SNOOP_CTRL0_SNOOP_EN_SHIFT                              0
#define    CCI400_SNOOP_CTRL0_SNOOP_EN_MASK                               0x00000001

#define CCI400_SHARE_OVERRIDE0_OFFSET                                     0x00001004
#define CCI400_SHARE_OVERRIDE0_TYPE                                       UInt32
#define CCI400_SHARE_OVERRIDE0_RESERVED_MASK                              0xFFFFFFFC
#define    CCI400_SHARE_OVERRIDE0_DOMAIN_OVERRIDE_SHIFT                   0
#define    CCI400_SHARE_OVERRIDE0_DOMAIN_OVERRIDE_MASK                    0x00000003

#define CCI400_RD_QOS_OVERRIDE0_OFFSET                                    0x00001100
#define CCI400_RD_QOS_OVERRIDE0_TYPE                                      UInt32
#define CCI400_RD_QOS_OVERRIDE0_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_RD_QOS_OVERRIDE0_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_RD_QOS_OVERRIDE0_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_WR_QOS_OVERRIDE0_OFFSET                                    0x00001104
#define CCI400_WR_QOS_OVERRIDE0_TYPE                                      UInt32
#define CCI400_WR_QOS_OVERRIDE0_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_WR_QOS_OVERRIDE0_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_WR_QOS_OVERRIDE0_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_QOS_CTRL0_OFFSET                                           0x0000110C
#define CCI400_QOS_CTRL0_TYPE                                             UInt32
#define CCI400_QOS_CTRL0_RESERVED_MASK                                    0xFFFFFFF0
#define    CCI400_QOS_CTRL0_RD_OT_REG_EN_SHIFT                            3
#define    CCI400_QOS_CTRL0_RD_OT_REG_EN_MASK                             0x00000008
#define    CCI400_QOS_CTRL0_WR_OT_REG_EN_SHIFT                            2
#define    CCI400_QOS_CTRL0_WR_OT_REG_EN_MASK                             0x00000004
#define    CCI400_QOS_CTRL0_RD_LATENCY_REG_EN_SHIFT                       1
#define    CCI400_QOS_CTRL0_RD_LATENCY_REG_EN_MASK                        0x00000002
#define    CCI400_QOS_CTRL0_WR_LATENCY_REG_EN_SHIFT                       0
#define    CCI400_QOS_CTRL0_WR_LATENCY_REG_EN_MASK                        0x00000001

#define CCI400_MAX_OT0_OFFSET                                             0x00001110
#define CCI400_MAX_OT0_TYPE                                               UInt32
#define CCI400_MAX_OT0_RESERVED_MASK                                      0xC000C000
#define    CCI400_MAX_OT0_RD_INT_OT_SHIFT                                 24
#define    CCI400_MAX_OT0_RD_INT_OT_MASK                                  0x3F000000
#define    CCI400_MAX_OT0_RD_FRAC_OT_SHIFT                                16
#define    CCI400_MAX_OT0_RD_FRAC_OT_MASK                                 0x00FF0000
#define    CCI400_MAX_OT0_WR_INT_OT_SHIFT                                 8
#define    CCI400_MAX_OT0_WR_INT_OT_MASK                                  0x00003F00
#define    CCI400_MAX_OT0_WR_FRAC_OT_SHIFT                                0
#define    CCI400_MAX_OT0_WR_FRAC_OT_MASK                                 0x000000FF

#define CCI400_TARGET_LATENCY0_OFFSET                                     0x00001130
#define CCI400_TARGET_LATENCY0_TYPE                                       UInt32
#define CCI400_TARGET_LATENCY0_RESERVED_MASK                              0xF000F000
#define    CCI400_TARGET_LATENCY0_RD_TARGET_LAT_SHIFT                     16
#define    CCI400_TARGET_LATENCY0_RD_TARGET_LAT_MASK                      0x0FFF0000
#define    CCI400_TARGET_LATENCY0_WR_TARGET_LAT_SHIFT                     0
#define    CCI400_TARGET_LATENCY0_WR_TARGET_LAT_MASK                      0x00000FFF

#define CCI400_LATENCY_REGULATION0_OFFSET                                 0x00001134
#define CCI400_LATENCY_REGULATION0_TYPE                                   UInt32
#define CCI400_LATENCY_REGULATION0_RESERVED_MASK                          0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION0_RD_QOS_FACTOR_SHIFT                 8
#define    CCI400_LATENCY_REGULATION0_RD_QOS_FACTOR_MASK                  0x00000700
#define    CCI400_LATENCY_REGULATION0_WR_QOS_FACTOR_SHIFT                 0
#define    CCI400_LATENCY_REGULATION0_WR_QOS_FACTOR_MASK                  0x00000007

#define CCI400_QOS_RANGE0_OFFSET                                          0x00001138
#define CCI400_QOS_RANGE0_TYPE                                            UInt32
#define CCI400_QOS_RANGE0_RESERVED_MASK                                   0xF0F0F0F0
#define    CCI400_QOS_RANGE0_MAX_RD_QOS_SHIFT                             24
#define    CCI400_QOS_RANGE0_MAX_RD_QOS_MASK                              0x0F000000
#define    CCI400_QOS_RANGE0_MIN_RD_QOS_SHIFT                             16
#define    CCI400_QOS_RANGE0_MIN_RD_QOS_MASK                              0x000F0000
#define    CCI400_QOS_RANGE0_MAX_WR_QOS_SHIFT                             8
#define    CCI400_QOS_RANGE0_MAX_WR_QOS_MASK                              0x00000F00
#define    CCI400_QOS_RANGE0_MIN_WR_QOS_SHIFT                             0
#define    CCI400_QOS_RANGE0_MIN_WR_QOS_MASK                              0x0000000F

#define CCI400_SNOOP_CTRL1_OFFSET                                         0x00002000
#define CCI400_SNOOP_CTRL1_TYPE                                           UInt32
#define CCI400_SNOOP_CTRL1_RESERVED_MASK                                  0x3FFFFFFC
#define    CCI400_SNOOP_CTRL1_ACCHANNELEN_DVM_SHIFT                       31
#define    CCI400_SNOOP_CTRL1_ACCHANNELEN_DVM_MASK                        0x80000000
#define    CCI400_SNOOP_CTRL1_ACCHANNELEN_SNOOP_SHIFT                     30
#define    CCI400_SNOOP_CTRL1_ACCHANNELEN_SNOOP_MASK                      0x40000000
#define    CCI400_SNOOP_CTRL1_DVM_EN_SHIFT                                1
#define    CCI400_SNOOP_CTRL1_DVM_EN_MASK                                 0x00000002
#define    CCI400_SNOOP_CTRL1_SNOOP_EN_SHIFT                              0
#define    CCI400_SNOOP_CTRL1_SNOOP_EN_MASK                               0x00000001

#define CCI400_SHARE_OVERRIDE1_OFFSET                                     0x00002004
#define CCI400_SHARE_OVERRIDE1_TYPE                                       UInt32
#define CCI400_SHARE_OVERRIDE1_RESERVED_MASK                              0xFFFFFFFC
#define    CCI400_SHARE_OVERRIDE1_DOMAIN_OVERRIDE_SHIFT                   0
#define    CCI400_SHARE_OVERRIDE1_DOMAIN_OVERRIDE_MASK                    0x00000003

#define CCI400_RD_QOS_OVERRIDE1_OFFSET                                    0x00002100
#define CCI400_RD_QOS_OVERRIDE1_TYPE                                      UInt32
#define CCI400_RD_QOS_OVERRIDE1_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_RD_QOS_OVERRIDE1_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_RD_QOS_OVERRIDE1_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_WR_QOS_OVERRIDE1_OFFSET                                    0x00002104
#define CCI400_WR_QOS_OVERRIDE1_TYPE                                      UInt32
#define CCI400_WR_QOS_OVERRIDE1_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_WR_QOS_OVERRIDE1_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_WR_QOS_OVERRIDE1_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_QOS_CTRL1_OFFSET                                           0x0000210C
#define CCI400_QOS_CTRL1_TYPE                                             UInt32
#define CCI400_QOS_CTRL1_RESERVED_MASK                                    0xFFFFFFF0
#define    CCI400_QOS_CTRL1_RD_OT_REG_EN_SHIFT                            3
#define    CCI400_QOS_CTRL1_RD_OT_REG_EN_MASK                             0x00000008
#define    CCI400_QOS_CTRL1_WR_OT_REG_EN_SHIFT                            2
#define    CCI400_QOS_CTRL1_WR_OT_REG_EN_MASK                             0x00000004
#define    CCI400_QOS_CTRL1_RD_LATENCY_REG_EN_SHIFT                       1
#define    CCI400_QOS_CTRL1_RD_LATENCY_REG_EN_MASK                        0x00000002
#define    CCI400_QOS_CTRL1_WR_LATENCY_REG_EN_SHIFT                       0
#define    CCI400_QOS_CTRL1_WR_LATENCY_REG_EN_MASK                        0x00000001

#define CCI400_MAX_OT1_OFFSET                                             0x00002110
#define CCI400_MAX_OT1_TYPE                                               UInt32
#define CCI400_MAX_OT1_RESERVED_MASK                                      0xC000C000
#define    CCI400_MAX_OT1_RD_INT_OT_SHIFT                                 24
#define    CCI400_MAX_OT1_RD_INT_OT_MASK                                  0x3F000000
#define    CCI400_MAX_OT1_RD_FRAC_OT_SHIFT                                16
#define    CCI400_MAX_OT1_RD_FRAC_OT_MASK                                 0x00FF0000
#define    CCI400_MAX_OT1_WR_INT_OT_SHIFT                                 8
#define    CCI400_MAX_OT1_WR_INT_OT_MASK                                  0x00003F00
#define    CCI400_MAX_OT1_WR_FRAC_OT_SHIFT                                0
#define    CCI400_MAX_OT1_WR_FRAC_OT_MASK                                 0x000000FF

#define CCI400_TARGET_LATENCY1_OFFSET                                     0x00002130
#define CCI400_TARGET_LATENCY1_TYPE                                       UInt32
#define CCI400_TARGET_LATENCY1_RESERVED_MASK                              0xF000F000
#define    CCI400_TARGET_LATENCY1_RD_TARGET_LAT_SHIFT                     16
#define    CCI400_TARGET_LATENCY1_RD_TARGET_LAT_MASK                      0x0FFF0000
#define    CCI400_TARGET_LATENCY1_WR_TARGET_LAT_SHIFT                     0
#define    CCI400_TARGET_LATENCY1_WR_TARGET_LAT_MASK                      0x00000FFF

#define CCI400_LATENCY_REGULATION1_OFFSET                                 0x00002134
#define CCI400_LATENCY_REGULATION1_TYPE                                   UInt32
#define CCI400_LATENCY_REGULATION1_RESERVED_MASK                          0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION1_RD_QOS_FACTOR_SHIFT                 8
#define    CCI400_LATENCY_REGULATION1_RD_QOS_FACTOR_MASK                  0x00000700
#define    CCI400_LATENCY_REGULATION1_WR_QOS_FACTOR_SHIFT                 0
#define    CCI400_LATENCY_REGULATION1_WR_QOS_FACTOR_MASK                  0x00000007

#define CCI400_QOS_RANGE1_OFFSET                                          0x00002138
#define CCI400_QOS_RANGE1_TYPE                                            UInt32
#define CCI400_QOS_RANGE1_RESERVED_MASK                                   0xF0F0F0F0
#define    CCI400_QOS_RANGE1_MAX_RD_QOS_SHIFT                             24
#define    CCI400_QOS_RANGE1_MAX_RD_QOS_MASK                              0x0F000000
#define    CCI400_QOS_RANGE1_MIN_RD_QOS_SHIFT                             16
#define    CCI400_QOS_RANGE1_MIN_RD_QOS_MASK                              0x000F0000
#define    CCI400_QOS_RANGE1_MAX_WR_QOS_SHIFT                             8
#define    CCI400_QOS_RANGE1_MAX_WR_QOS_MASK                              0x00000F00
#define    CCI400_QOS_RANGE1_MIN_WR_QOS_SHIFT                             0
#define    CCI400_QOS_RANGE1_MIN_WR_QOS_MASK                              0x0000000F

#define CCI400_SNOOP_CTRL2_OFFSET                                         0x00003000
#define CCI400_SNOOP_CTRL2_TYPE                                           UInt32
#define CCI400_SNOOP_CTRL2_RESERVED_MASK                                  0x3FFFFFFC
#define    CCI400_SNOOP_CTRL2_ACCHANNELEN_DVM_SHIFT                       31
#define    CCI400_SNOOP_CTRL2_ACCHANNELEN_DVM_MASK                        0x80000000
#define    CCI400_SNOOP_CTRL2_ACCHANNELEN_SNOOP_SHIFT                     30
#define    CCI400_SNOOP_CTRL2_ACCHANNELEN_SNOOP_MASK                      0x40000000
#define    CCI400_SNOOP_CTRL2_DVM_EN_SHIFT                                1
#define    CCI400_SNOOP_CTRL2_DVM_EN_MASK                                 0x00000002
#define    CCI400_SNOOP_CTRL2_SNOOP_EN_SHIFT                              0
#define    CCI400_SNOOP_CTRL2_SNOOP_EN_MASK                               0x00000001

#define CCI400_SHARE_OVERRIDE2_OFFSET                                     0x00003004
#define CCI400_SHARE_OVERRIDE2_TYPE                                       UInt32
#define CCI400_SHARE_OVERRIDE2_RESERVED_MASK                              0xFFFFFFFC
#define    CCI400_SHARE_OVERRIDE2_DOMAIN_OVERRIDE_SHIFT                   0
#define    CCI400_SHARE_OVERRIDE2_DOMAIN_OVERRIDE_MASK                    0x00000003

#define CCI400_RD_QOS_OVERRIDE2_OFFSET                                    0x00003100
#define CCI400_RD_QOS_OVERRIDE2_TYPE                                      UInt32
#define CCI400_RD_QOS_OVERRIDE2_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_RD_QOS_OVERRIDE2_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_RD_QOS_OVERRIDE2_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_WR_QOS_OVERRIDE2_OFFSET                                    0x00003104
#define CCI400_WR_QOS_OVERRIDE2_TYPE                                      UInt32
#define CCI400_WR_QOS_OVERRIDE2_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_WR_QOS_OVERRIDE2_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_WR_QOS_OVERRIDE2_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_QOS_CTRL2_OFFSET                                           0x0000310C
#define CCI400_QOS_CTRL2_TYPE                                             UInt32
#define CCI400_QOS_CTRL2_RESERVED_MASK                                    0xFFFFFFF0
#define    CCI400_QOS_CTRL2_RD_OT_REG_EN_SHIFT                            3
#define    CCI400_QOS_CTRL2_RD_OT_REG_EN_MASK                             0x00000008
#define    CCI400_QOS_CTRL2_WR_OT_REG_EN_SHIFT                            2
#define    CCI400_QOS_CTRL2_WR_OT_REG_EN_MASK                             0x00000004
#define    CCI400_QOS_CTRL2_RD_LATENCY_REG_EN_SHIFT                       1
#define    CCI400_QOS_CTRL2_RD_LATENCY_REG_EN_MASK                        0x00000002
#define    CCI400_QOS_CTRL2_WR_LATENCY_REG_EN_SHIFT                       0
#define    CCI400_QOS_CTRL2_WR_LATENCY_REG_EN_MASK                        0x00000001

#define CCI400_MAX_OT2_OFFSET                                             0x00003110
#define CCI400_MAX_OT2_TYPE                                               UInt32
#define CCI400_MAX_OT2_RESERVED_MASK                                      0xC000C000
#define    CCI400_MAX_OT2_RD_INT_OT_SHIFT                                 24
#define    CCI400_MAX_OT2_RD_INT_OT_MASK                                  0x3F000000
#define    CCI400_MAX_OT2_RD_FRAC_OT_SHIFT                                16
#define    CCI400_MAX_OT2_RD_FRAC_OT_MASK                                 0x00FF0000
#define    CCI400_MAX_OT2_WR_INT_OT_SHIFT                                 8
#define    CCI400_MAX_OT2_WR_INT_OT_MASK                                  0x00003F00
#define    CCI400_MAX_OT2_WR_FRAC_OT_SHIFT                                0
#define    CCI400_MAX_OT2_WR_FRAC_OT_MASK                                 0x000000FF

#define CCI400_TARGET_LATENCY2_OFFSET                                     0x00003130
#define CCI400_TARGET_LATENCY2_TYPE                                       UInt32
#define CCI400_TARGET_LATENCY2_RESERVED_MASK                              0xF000F000
#define    CCI400_TARGET_LATENCY2_RD_TARGET_LAT_SHIFT                     16
#define    CCI400_TARGET_LATENCY2_RD_TARGET_LAT_MASK                      0x0FFF0000
#define    CCI400_TARGET_LATENCY2_WR_TARGET_LAT_SHIFT                     0
#define    CCI400_TARGET_LATENCY2_WR_TARGET_LAT_MASK                      0x00000FFF

#define CCI400_LATENCY_REGULATION2_OFFSET                                 0x00003134
#define CCI400_LATENCY_REGULATION2_TYPE                                   UInt32
#define CCI400_LATENCY_REGULATION2_RESERVED_MASK                          0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION2_RD_QOS_FACTOR_SHIFT                 8
#define    CCI400_LATENCY_REGULATION2_RD_QOS_FACTOR_MASK                  0x00000700
#define    CCI400_LATENCY_REGULATION2_WR_QOS_FACTOR_SHIFT                 0
#define    CCI400_LATENCY_REGULATION2_WR_QOS_FACTOR_MASK                  0x00000007

#define CCI400_QOS_RANGE2_OFFSET                                          0x00003138
#define CCI400_QOS_RANGE2_TYPE                                            UInt32
#define CCI400_QOS_RANGE2_RESERVED_MASK                                   0xF0F0F0F0
#define    CCI400_QOS_RANGE2_MAX_RD_QOS_SHIFT                             24
#define    CCI400_QOS_RANGE2_MAX_RD_QOS_MASK                              0x0F000000
#define    CCI400_QOS_RANGE2_MIN_RD_QOS_SHIFT                             16
#define    CCI400_QOS_RANGE2_MIN_RD_QOS_MASK                              0x000F0000
#define    CCI400_QOS_RANGE2_MAX_WR_QOS_SHIFT                             8
#define    CCI400_QOS_RANGE2_MAX_WR_QOS_MASK                              0x00000F00
#define    CCI400_QOS_RANGE2_MIN_WR_QOS_SHIFT                             0
#define    CCI400_QOS_RANGE2_MIN_WR_QOS_MASK                              0x0000000F

#define CCI400_SNOOP_CTRL3_OFFSET                                         0x00004000
#define CCI400_SNOOP_CTRL3_TYPE                                           UInt32
#define CCI400_SNOOP_CTRL3_RESERVED_MASK                                  0x3FFFFFFC
#define    CCI400_SNOOP_CTRL3_ACCHANNELEN_DVM_SHIFT                       31
#define    CCI400_SNOOP_CTRL3_ACCHANNELEN_DVM_MASK                        0x80000000
#define    CCI400_SNOOP_CTRL3_ACCHANNELEN_SNOOP_SHIFT                     30
#define    CCI400_SNOOP_CTRL3_ACCHANNELEN_SNOOP_MASK                      0x40000000
#define    CCI400_SNOOP_CTRL3_DVM_EN_SHIFT                                1
#define    CCI400_SNOOP_CTRL3_DVM_EN_MASK                                 0x00000002
#define    CCI400_SNOOP_CTRL3_SNOOP_EN_SHIFT                              0
#define    CCI400_SNOOP_CTRL3_SNOOP_EN_MASK                               0x00000001

#define CCI400_RD_QOS_OVERRIDE3_OFFSET                                    0x00004100
#define CCI400_RD_QOS_OVERRIDE3_TYPE                                      UInt32
#define CCI400_RD_QOS_OVERRIDE3_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_RD_QOS_OVERRIDE3_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_RD_QOS_OVERRIDE3_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_WR_QOS_OVERRIDE3_OFFSET                                    0x00004104
#define CCI400_WR_QOS_OVERRIDE3_TYPE                                      UInt32
#define CCI400_WR_QOS_OVERRIDE3_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_WR_QOS_OVERRIDE3_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_WR_QOS_OVERRIDE3_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_QOS_CTRL3_OFFSET                                           0x0000410C
#define CCI400_QOS_CTRL3_TYPE                                             UInt32
#define CCI400_QOS_CTRL3_RESERVED_MASK                                    0xFFFFFFF0
#define    CCI400_QOS_CTRL3_RD_OT_REG_EN_SHIFT                            3
#define    CCI400_QOS_CTRL3_RD_OT_REG_EN_MASK                             0x00000008
#define    CCI400_QOS_CTRL3_WR_OT_REG_EN_SHIFT                            2
#define    CCI400_QOS_CTRL3_WR_OT_REG_EN_MASK                             0x00000004
#define    CCI400_QOS_CTRL3_RD_LATENCY_REG_EN_SHIFT                       1
#define    CCI400_QOS_CTRL3_RD_LATENCY_REG_EN_MASK                        0x00000002
#define    CCI400_QOS_CTRL3_WR_LATENCY_REG_EN_SHIFT                       0
#define    CCI400_QOS_CTRL3_WR_LATENCY_REG_EN_MASK                        0x00000001

#define CCI400_TARGET_LATENCY3_OFFSET                                     0x00004130
#define CCI400_TARGET_LATENCY3_TYPE                                       UInt32
#define CCI400_TARGET_LATENCY3_RESERVED_MASK                              0xF000F000
#define    CCI400_TARGET_LATENCY3_RD_TARGET_LAT_SHIFT                     16
#define    CCI400_TARGET_LATENCY3_RD_TARGET_LAT_MASK                      0x0FFF0000
#define    CCI400_TARGET_LATENCY3_WR_TARGET_LAT_SHIFT                     0
#define    CCI400_TARGET_LATENCY3_WR_TARGET_LAT_MASK                      0x00000FFF

#define CCI400_LATENCY_REGULATION3_OFFSET                                 0x00004134
#define CCI400_LATENCY_REGULATION3_TYPE                                   UInt32
#define CCI400_LATENCY_REGULATION3_RESERVED_MASK                          0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION3_RD_QOS_FACTOR_SHIFT                 8
#define    CCI400_LATENCY_REGULATION3_RD_QOS_FACTOR_MASK                  0x00000700
#define    CCI400_LATENCY_REGULATION3_WR_QOS_FACTOR_SHIFT                 0
#define    CCI400_LATENCY_REGULATION3_WR_QOS_FACTOR_MASK                  0x00000007

#define CCI400_QOS_RANGE3_OFFSET                                          0x00004138
#define CCI400_QOS_RANGE3_TYPE                                            UInt32
#define CCI400_QOS_RANGE3_RESERVED_MASK                                   0xF0F0F0F0
#define    CCI400_QOS_RANGE3_MAX_RD_QOS_SHIFT                             24
#define    CCI400_QOS_RANGE3_MAX_RD_QOS_MASK                              0x0F000000
#define    CCI400_QOS_RANGE3_MIN_RD_QOS_SHIFT                             16
#define    CCI400_QOS_RANGE3_MIN_RD_QOS_MASK                              0x000F0000
#define    CCI400_QOS_RANGE3_MAX_WR_QOS_SHIFT                             8
#define    CCI400_QOS_RANGE3_MAX_WR_QOS_MASK                              0x00000F00
#define    CCI400_QOS_RANGE3_MIN_WR_QOS_SHIFT                             0
#define    CCI400_QOS_RANGE3_MIN_WR_QOS_MASK                              0x0000000F

#define CCI400_SNOOP_CTRL4_OFFSET                                         0x00005000
#define CCI400_SNOOP_CTRL4_TYPE                                           UInt32
#define CCI400_SNOOP_CTRL4_RESERVED_MASK                                  0x3FFFFFFC
#define    CCI400_SNOOP_CTRL4_ACCHANNELEN_DVM_SHIFT                       31
#define    CCI400_SNOOP_CTRL4_ACCHANNELEN_DVM_MASK                        0x80000000
#define    CCI400_SNOOP_CTRL4_ACCHANNELEN_SNOOP_SHIFT                     30
#define    CCI400_SNOOP_CTRL4_ACCHANNELEN_SNOOP_MASK                      0x40000000
#define    CCI400_SNOOP_CTRL4_DVM_EN_SHIFT                                1
#define    CCI400_SNOOP_CTRL4_DVM_EN_MASK                                 0x00000002
#define    CCI400_SNOOP_CTRL4_SNOOP_EN_SHIFT                              0
#define    CCI400_SNOOP_CTRL4_SNOOP_EN_MASK                               0x00000001

#define CCI400_RD_QOS_OVERRIDE4_OFFSET                                    0x00005100
#define CCI400_RD_QOS_OVERRIDE4_TYPE                                      UInt32
#define CCI400_RD_QOS_OVERRIDE4_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_RD_QOS_OVERRIDE4_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_RD_QOS_OVERRIDE4_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_WR_QOS_OVERRIDE4_OFFSET                                    0x00005104
#define CCI400_WR_QOS_OVERRIDE4_TYPE                                      UInt32
#define CCI400_WR_QOS_OVERRIDE4_RESERVED_MASK                             0xFFFFFFF0
#define    CCI400_WR_QOS_OVERRIDE4_QOS_OVERRIDE_SHIFT                     0
#define    CCI400_WR_QOS_OVERRIDE4_QOS_OVERRIDE_MASK                      0x0000000F

#define CCI400_QOS_CTRL4_OFFSET                                           0x0000510C
#define CCI400_QOS_CTRL4_TYPE                                             UInt32
#define CCI400_QOS_CTRL4_RESERVED_MASK                                    0xFFFFFFF0
#define    CCI400_QOS_CTRL4_RD_OT_REG_EN_SHIFT                            3
#define    CCI400_QOS_CTRL4_RD_OT_REG_EN_MASK                             0x00000008
#define    CCI400_QOS_CTRL4_WR_OT_REG_EN_SHIFT                            2
#define    CCI400_QOS_CTRL4_WR_OT_REG_EN_MASK                             0x00000004
#define    CCI400_QOS_CTRL4_RD_LATENCY_REG_EN_SHIFT                       1
#define    CCI400_QOS_CTRL4_RD_LATENCY_REG_EN_MASK                        0x00000002
#define    CCI400_QOS_CTRL4_WR_LATENCY_REG_EN_SHIFT                       0
#define    CCI400_QOS_CTRL4_WR_LATENCY_REG_EN_MASK                        0x00000001

#define CCI400_TARGET_LATENCY4_OFFSET                                     0x00005130
#define CCI400_TARGET_LATENCY4_TYPE                                       UInt32
#define CCI400_TARGET_LATENCY4_RESERVED_MASK                              0xF000F000
#define    CCI400_TARGET_LATENCY4_RD_TARGET_LAT_SHIFT                     16
#define    CCI400_TARGET_LATENCY4_RD_TARGET_LAT_MASK                      0x0FFF0000
#define    CCI400_TARGET_LATENCY4_WR_TARGET_LAT_SHIFT                     0
#define    CCI400_TARGET_LATENCY4_WR_TARGET_LAT_MASK                      0x00000FFF

#define CCI400_LATENCY_REGULATION4_OFFSET                                 0x00005134
#define CCI400_LATENCY_REGULATION4_TYPE                                   UInt32
#define CCI400_LATENCY_REGULATION4_RESERVED_MASK                          0xFFFFF8F8
#define    CCI400_LATENCY_REGULATION4_RD_QOS_FACTOR_SHIFT                 8
#define    CCI400_LATENCY_REGULATION4_RD_QOS_FACTOR_MASK                  0x00000700
#define    CCI400_LATENCY_REGULATION4_WR_QOS_FACTOR_SHIFT                 0
#define    CCI400_LATENCY_REGULATION4_WR_QOS_FACTOR_MASK                  0x00000007

#define CCI400_QOS_RANGE4_OFFSET                                          0x00005138
#define CCI400_QOS_RANGE4_TYPE                                            UInt32
#define CCI400_QOS_RANGE4_RESERVED_MASK                                   0xF0F0F0F0
#define    CCI400_QOS_RANGE4_MAX_RD_QOS_SHIFT                             24
#define    CCI400_QOS_RANGE4_MAX_RD_QOS_MASK                              0x0F000000
#define    CCI400_QOS_RANGE4_MIN_RD_QOS_SHIFT                             16
#define    CCI400_QOS_RANGE4_MIN_RD_QOS_MASK                              0x000F0000
#define    CCI400_QOS_RANGE4_MAX_WR_QOS_SHIFT                             8
#define    CCI400_QOS_RANGE4_MAX_WR_QOS_MASK                              0x00000F00
#define    CCI400_QOS_RANGE4_MIN_WR_QOS_SHIFT                             0
#define    CCI400_QOS_RANGE4_MIN_WR_QOS_MASK                              0x0000000F

#define CCI400_CYCLE_CNT_VAL_OFFSET                                       0x00009004
#define CCI400_CYCLE_CNT_VAL_TYPE                                         UInt32
#define CCI400_CYCLE_CNT_VAL_RESERVED_MASK                                0x00000000
#define    CCI400_CYCLE_CNT_VAL_CNT_VAL_SHIFT                             0
#define    CCI400_CYCLE_CNT_VAL_CNT_VAL_MASK                              0xFFFFFFFF

#define CCI400_CYCLE_CNT_CTRL_OFFSET                                      0x00009008
#define CCI400_CYCLE_CNT_CTRL_TYPE                                        UInt32
#define CCI400_CYCLE_CNT_CTRL_RESERVED_MASK                               0xFFFFFFFE
#define    CCI400_CYCLE_CNT_CTRL_CNT_EN_SHIFT                             0
#define    CCI400_CYCLE_CNT_CTRL_CNT_EN_MASK                              0x00000001

#define CCI400_CYCLE_CNT_OVERFLOW_OFFSET                                  0x0000900C
#define CCI400_CYCLE_CNT_OVERFLOW_TYPE                                    UInt32
#define CCI400_CYCLE_CNT_OVERFLOW_RESERVED_MASK                           0xFFFFFFFE
#define    CCI400_CYCLE_CNT_OVERFLOW_CNT_OVERFLOW_FLAG_SHIFT              0
#define    CCI400_CYCLE_CNT_OVERFLOW_CNT_OVERFLOW_FLAG_MASK               0x00000001

#define CCI400_PMU_EVNT_SEL0_OFFSET                                       0x0000A000
#define CCI400_PMU_EVNT_SEL0_TYPE                                         UInt32
#define CCI400_PMU_EVNT_SEL0_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_PMU_EVNT_SEL0_EVENT_SEL_SHIFT                           0
#define    CCI400_PMU_EVNT_SEL0_EVENT_SEL_MASK                            0x000000FF

#define CCI400_PMU_CNT_VAL0_OFFSET                                        0x0000A004
#define CCI400_PMU_CNT_VAL0_TYPE                                          UInt32
#define CCI400_PMU_CNT_VAL0_RESERVED_MASK                                 0x00000000
#define    CCI400_PMU_CNT_VAL0_CNT_VAL_SHIFT                              0
#define    CCI400_PMU_CNT_VAL0_CNT_VAL_MASK                               0xFFFFFFFF

#define CCI400_PMU_CNT_CTRL0_OFFSET                                       0x0000A008
#define CCI400_PMU_CNT_CTRL0_TYPE                                         UInt32
#define CCI400_PMU_CNT_CTRL0_RESERVED_MASK                                0xFFFFFFFE
#define    CCI400_PMU_CNT_CTRL0_CNT_EN_SHIFT                              0
#define    CCI400_PMU_CNT_CTRL0_CNT_EN_MASK                               0x00000001

#define CCI400_PMU_CNT_OVERFLOW0_OFFSET                                   0x0000A00C
#define CCI400_PMU_CNT_OVERFLOW0_TYPE                                     UInt32
#define CCI400_PMU_CNT_OVERFLOW0_RESERVED_MASK                            0xFFFFFFFE
#define    CCI400_PMU_CNT_OVERFLOW0_CNT_OVERFLOW_FLAG_SHIFT               0
#define    CCI400_PMU_CNT_OVERFLOW0_CNT_OVERFLOW_FLAG_MASK                0x00000001

#define CCI400_PMU_EVNT_SEL1_OFFSET                                       0x0000B000
#define CCI400_PMU_EVNT_SEL1_TYPE                                         UInt32
#define CCI400_PMU_EVNT_SEL1_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_PMU_EVNT_SEL1_EVENT_SEL_SHIFT                           0
#define    CCI400_PMU_EVNT_SEL1_EVENT_SEL_MASK                            0x000000FF

#define CCI400_PMU_CNT_VAL1_OFFSET                                        0x0000B004
#define CCI400_PMU_CNT_VAL1_TYPE                                          UInt32
#define CCI400_PMU_CNT_VAL1_RESERVED_MASK                                 0x00000000
#define    CCI400_PMU_CNT_VAL1_CNT_VAL_SHIFT                              0
#define    CCI400_PMU_CNT_VAL1_CNT_VAL_MASK                               0xFFFFFFFF

#define CCI400_PMU_CNT_CTRL1_OFFSET                                       0x0000B008
#define CCI400_PMU_CNT_CTRL1_TYPE                                         UInt32
#define CCI400_PMU_CNT_CTRL1_RESERVED_MASK                                0xFFFFFFFE
#define    CCI400_PMU_CNT_CTRL1_CNT_EN_SHIFT                              0
#define    CCI400_PMU_CNT_CTRL1_CNT_EN_MASK                               0x00000001

#define CCI400_PMU_CNT_OVERFLOW1_OFFSET                                   0x0000B00C
#define CCI400_PMU_CNT_OVERFLOW1_TYPE                                     UInt32
#define CCI400_PMU_CNT_OVERFLOW1_RESERVED_MASK                            0xFFFFFFFE
#define    CCI400_PMU_CNT_OVERFLOW1_CNT_OVERFLOW_FLAG_SHIFT               0
#define    CCI400_PMU_CNT_OVERFLOW1_CNT_OVERFLOW_FLAG_MASK                0x00000001

#define CCI400_PMU_EVNT_SEL2_OFFSET                                       0x0000C000
#define CCI400_PMU_EVNT_SEL2_TYPE                                         UInt32
#define CCI400_PMU_EVNT_SEL2_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_PMU_EVNT_SEL2_EVENT_SEL_SHIFT                           0
#define    CCI400_PMU_EVNT_SEL2_EVENT_SEL_MASK                            0x000000FF

#define CCI400_PMU_CNT_VAL2_OFFSET                                        0x0000C004
#define CCI400_PMU_CNT_VAL2_TYPE                                          UInt32
#define CCI400_PMU_CNT_VAL2_RESERVED_MASK                                 0x00000000
#define    CCI400_PMU_CNT_VAL2_CNT_VAL_SHIFT                              0
#define    CCI400_PMU_CNT_VAL2_CNT_VAL_MASK                               0xFFFFFFFF

#define CCI400_PMU_CNT_CTRL2_OFFSET                                       0x0000C008
#define CCI400_PMU_CNT_CTRL2_TYPE                                         UInt32
#define CCI400_PMU_CNT_CTRL2_RESERVED_MASK                                0xFFFFFFFE
#define    CCI400_PMU_CNT_CTRL2_CNT_EN_SHIFT                              0
#define    CCI400_PMU_CNT_CTRL2_CNT_EN_MASK                               0x00000001

#define CCI400_PMU_CNT_OVERFLOW2_OFFSET                                   0x0000C00C
#define CCI400_PMU_CNT_OVERFLOW2_TYPE                                     UInt32
#define CCI400_PMU_CNT_OVERFLOW2_RESERVED_MASK                            0xFFFFFFFE
#define    CCI400_PMU_CNT_OVERFLOW2_CNT_OVERFLOW_FLAG_SHIFT               0
#define    CCI400_PMU_CNT_OVERFLOW2_CNT_OVERFLOW_FLAG_MASK                0x00000001

#define CCI400_PMU_EVNT_SEL3_OFFSET                                       0x0000D000
#define CCI400_PMU_EVNT_SEL3_TYPE                                         UInt32
#define CCI400_PMU_EVNT_SEL3_RESERVED_MASK                                0xFFFFFF00
#define    CCI400_PMU_EVNT_SEL3_EVENT_SEL_SHIFT                           0
#define    CCI400_PMU_EVNT_SEL3_EVENT_SEL_MASK                            0x000000FF

#define CCI400_PMU_CNT_VAL3_OFFSET                                        0x0000D004
#define CCI400_PMU_CNT_VAL3_TYPE                                          UInt32
#define CCI400_PMU_CNT_VAL3_RESERVED_MASK                                 0x00000000
#define    CCI400_PMU_CNT_VAL3_CNT_VAL_SHIFT                              0
#define    CCI400_PMU_CNT_VAL3_CNT_VAL_MASK                               0xFFFFFFFF

#define CCI400_PMU_CNT_CTRL3_OFFSET                                       0x0000D008
#define CCI400_PMU_CNT_CTRL3_TYPE                                         UInt32
#define CCI400_PMU_CNT_CTRL3_RESERVED_MASK                                0xFFFFFFFE
#define    CCI400_PMU_CNT_CTRL3_CNT_EN_SHIFT                              0
#define    CCI400_PMU_CNT_CTRL3_CNT_EN_MASK                               0x00000001

#define CCI400_PMU_CNT_OVERFLOW3_OFFSET                                   0x0000D00C
#define CCI400_PMU_CNT_OVERFLOW3_TYPE                                     UInt32
#define CCI400_PMU_CNT_OVERFLOW3_RESERVED_MASK                            0xFFFFFFFE
#define    CCI400_PMU_CNT_OVERFLOW3_CNT_OVERFLOW_FLAG_SHIFT               0
#define    CCI400_PMU_CNT_OVERFLOW3_CNT_OVERFLOW_FLAG_MASK                0x00000001

#endif /* __BRCM_RDB_CCI400_H__ */



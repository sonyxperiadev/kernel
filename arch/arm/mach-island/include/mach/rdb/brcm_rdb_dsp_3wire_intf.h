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

#ifndef __BRCM_RDB_DSP_3WIRE_INTF_H__
#define __BRCM_RDB_DSP_3WIRE_INTF_H__

#define DSP_3WIRE_INTF_FQCR_R_OFFSET                                      0x00000AE0
#define DSP_3WIRE_INTF_FQCR_R_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FQCR_R_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FQCR_R_FREQS_SHIFT                              15
#define    DSP_3WIRE_INTF_FQCR_R_FREQS_MASK                               0x00008000
#define    DSP_3WIRE_INTF_FQCR_R_FRQCP_SHIFT                              14
#define    DSP_3WIRE_INTF_FQCR_R_FRQCP_MASK                               0x00004000
#define    DSP_3WIRE_INTF_FQCR_R_FRQCD_SHIFT                              13
#define    DSP_3WIRE_INTF_FQCR_R_FRQCD_MASK                               0x00002000
#define    DSP_3WIRE_INTF_FQCR_R_FRQC_SHIFT                               8
#define    DSP_3WIRE_INTF_FQCR_R_FRQC_MASK                                0x00001F00
#define    DSP_3WIRE_INTF_FQCR_R_HS_SHIFT                                 7
#define    DSP_3WIRE_INTF_FQCR_R_HS_MASK                                  0x00000080
#define    DSP_3WIRE_INTF_FQCR_R_FRQDP_SHIFT                              6
#define    DSP_3WIRE_INTF_FQCR_R_FRQDP_MASK                               0x00000040
#define    DSP_3WIRE_INTF_FQCR_R_FRQDD_SHIFT                              5
#define    DSP_3WIRE_INTF_FQCR_R_FRQDD_MASK                               0x00000020
#define    DSP_3WIRE_INTF_FQCR_R_FRQD_SHIFT                               0
#define    DSP_3WIRE_INTF_FQCR_R_FRQD_MASK                                0x0000001F

#define DSP_3WIRE_INTF_FQC2R_R_OFFSET                                     0x00000AF2
#define DSP_3WIRE_INTF_FQC2R_R_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FQC2R_R_RESERVED_MASK                              0x00000180
#define    DSP_3WIRE_INTF_FQC2R_R_RDLN_SHIFT                              11
#define    DSP_3WIRE_INTF_FQC2R_R_RDLN_MASK                               0x0000F800
#define    DSP_3WIRE_INTF_FQC2R_R_RDEN_SHIFT                              10
#define    DSP_3WIRE_INTF_FQC2R_R_RDEN_MASK                               0x00000400
#define    DSP_3WIRE_INTF_FQC2R_R_EDGE_SHIFT                              9
#define    DSP_3WIRE_INTF_FQC2R_R_EDGE_MASK                               0x00000200
#define    DSP_3WIRE_INTF_FQC2R_R_FRQXP_SHIFT                             6
#define    DSP_3WIRE_INTF_FQC2R_R_FRQXP_MASK                              0x00000040
#define    DSP_3WIRE_INTF_FQC2R_R_FRQXD_SHIFT                             5
#define    DSP_3WIRE_INTF_FQC2R_R_FRQXD_MASK                              0x00000020
#define    DSP_3WIRE_INTF_FQC2R_R_FRQX_SHIFT                              0
#define    DSP_3WIRE_INTF_FQC2R_R_FRQX_MASK                               0x0000001F

#define DSP_3WIRE_INTF_FSHLR_R0_OFFSET                                    0x00001C00
#define DSP_3WIRE_INTF_FSHLR_R0_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_R0_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_R0_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_R0_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FCHR_R_OFFSET                                      0x00001E80
#define DSP_3WIRE_INTF_FCHR_R_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_R_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_R_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_R_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_R_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_R_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_R_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_R_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_R_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_R_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_R_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_R_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_R_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_R_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_R_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_R_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_R_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_R_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FQCR_OFFSET                                        0x0000E570
#define DSP_3WIRE_INTF_FQCR_TYPE                                          UInt16
#define DSP_3WIRE_INTF_FQCR_RESERVED_MASK                                 0x00000000
#define    DSP_3WIRE_INTF_FQCR_FREQS_SHIFT                                15
#define    DSP_3WIRE_INTF_FQCR_FREQS_MASK                                 0x00008000
#define    DSP_3WIRE_INTF_FQCR_FRQCP_SHIFT                                14
#define    DSP_3WIRE_INTF_FQCR_FRQCP_MASK                                 0x00004000
#define    DSP_3WIRE_INTF_FQCR_FRQCD_SHIFT                                13
#define    DSP_3WIRE_INTF_FQCR_FRQCD_MASK                                 0x00002000
#define    DSP_3WIRE_INTF_FQCR_FRQC_SHIFT                                 8
#define    DSP_3WIRE_INTF_FQCR_FRQC_MASK                                  0x00001F00
#define    DSP_3WIRE_INTF_FQCR_HS_SHIFT                                   7
#define    DSP_3WIRE_INTF_FQCR_HS_MASK                                    0x00000080
#define    DSP_3WIRE_INTF_FQCR_FRQDP_SHIFT                                6
#define    DSP_3WIRE_INTF_FQCR_FRQDP_MASK                                 0x00000040
#define    DSP_3WIRE_INTF_FQCR_FRQDD_SHIFT                                5
#define    DSP_3WIRE_INTF_FQCR_FRQDD_MASK                                 0x00000020
#define    DSP_3WIRE_INTF_FQCR_FRQD_SHIFT                                 0
#define    DSP_3WIRE_INTF_FQCR_FRQD_MASK                                  0x0000001F

#define DSP_3WIRE_INTF_FQC2R_OFFSET                                       0x0000E579
#define DSP_3WIRE_INTF_FQC2R_TYPE                                         UInt16
#define DSP_3WIRE_INTF_FQC2R_RESERVED_MASK                                0x00000180
#define    DSP_3WIRE_INTF_FQC2R_RDLN_SHIFT                                11
#define    DSP_3WIRE_INTF_FQC2R_RDLN_MASK                                 0x0000F800
#define    DSP_3WIRE_INTF_FQC2R_RDEN_SHIFT                                10
#define    DSP_3WIRE_INTF_FQC2R_RDEN_MASK                                 0x00000400
#define    DSP_3WIRE_INTF_FQC2R_EDGE_SHIFT                                9
#define    DSP_3WIRE_INTF_FQC2R_EDGE_MASK                                 0x00000200
#define    DSP_3WIRE_INTF_FQC2R_FRQXP_SHIFT                               6
#define    DSP_3WIRE_INTF_FQC2R_FRQXP_MASK                                0x00000040
#define    DSP_3WIRE_INTF_FQC2R_FRQXD_SHIFT                               5
#define    DSP_3WIRE_INTF_FQC2R_FRQXD_MASK                                0x00000020
#define    DSP_3WIRE_INTF_FQC2R_FRQX_SHIFT                                0
#define    DSP_3WIRE_INTF_FQC2R_FRQX_MASK                                 0x0000001F

#define DSP_3WIRE_INTF_FSHLR_0_OFFSET                                     0x0000EE00
#define DSP_3WIRE_INTF_FSHLR_0_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_0_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_0_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_0_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_1_OFFSET                                     0x0000EE02
#define DSP_3WIRE_INTF_FSHLR_1_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_1_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_1_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_1_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_2_OFFSET                                     0x0000EE04
#define DSP_3WIRE_INTF_FSHLR_2_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_2_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_2_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_2_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_3_OFFSET                                     0x0000EE06
#define DSP_3WIRE_INTF_FSHLR_3_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_3_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_3_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_3_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_4_OFFSET                                     0x0000EE08
#define DSP_3WIRE_INTF_FSHLR_4_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_4_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_4_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_4_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_5_OFFSET                                     0x0000EE0A
#define DSP_3WIRE_INTF_FSHLR_5_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_5_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_5_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_5_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_6_OFFSET                                     0x0000EE0C
#define DSP_3WIRE_INTF_FSHLR_6_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_6_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_6_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_6_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_7_OFFSET                                     0x0000EE0E
#define DSP_3WIRE_INTF_FSHLR_7_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_7_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_7_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_7_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_8_OFFSET                                     0x0000EE10
#define DSP_3WIRE_INTF_FSHLR_8_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_8_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_8_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_8_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_9_OFFSET                                     0x0000EE12
#define DSP_3WIRE_INTF_FSHLR_9_TYPE                                       UInt32
#define DSP_3WIRE_INTF_FSHLR_9_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FSHLR_9_FSN_SHIFT                               0
#define    DSP_3WIRE_INTF_FSHLR_9_FSN_MASK                                0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_10_OFFSET                                    0x0000EE14
#define DSP_3WIRE_INTF_FSHLR_10_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_10_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_10_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_10_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_11_OFFSET                                    0x0000EE16
#define DSP_3WIRE_INTF_FSHLR_11_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_11_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_11_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_11_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_12_OFFSET                                    0x0000EE18
#define DSP_3WIRE_INTF_FSHLR_12_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_12_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_12_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_12_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_13_OFFSET                                    0x0000EE1A
#define DSP_3WIRE_INTF_FSHLR_13_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_13_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_13_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_13_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_14_OFFSET                                    0x0000EE1C
#define DSP_3WIRE_INTF_FSHLR_14_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_14_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_14_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_14_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_15_OFFSET                                    0x0000EE1E
#define DSP_3WIRE_INTF_FSHLR_15_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_15_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_15_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_15_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_16_OFFSET                                    0x0000EE20
#define DSP_3WIRE_INTF_FSHLR_16_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_16_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_16_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_16_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_17_OFFSET                                    0x0000EE22
#define DSP_3WIRE_INTF_FSHLR_17_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_17_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_17_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_17_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_18_OFFSET                                    0x0000EE24
#define DSP_3WIRE_INTF_FSHLR_18_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_18_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_18_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_18_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_19_OFFSET                                    0x0000EE26
#define DSP_3WIRE_INTF_FSHLR_19_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_19_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_19_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_19_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_20_OFFSET                                    0x0000EE28
#define DSP_3WIRE_INTF_FSHLR_20_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_20_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_20_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_20_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_21_OFFSET                                    0x0000EE2A
#define DSP_3WIRE_INTF_FSHLR_21_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_21_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_21_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_21_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_22_OFFSET                                    0x0000EE2C
#define DSP_3WIRE_INTF_FSHLR_22_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_22_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_22_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_22_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_23_OFFSET                                    0x0000EE2E
#define DSP_3WIRE_INTF_FSHLR_23_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_23_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_23_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_23_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_24_OFFSET                                    0x0000EE30
#define DSP_3WIRE_INTF_FSHLR_24_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_24_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_24_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_24_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_25_OFFSET                                    0x0000EE32
#define DSP_3WIRE_INTF_FSHLR_25_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_25_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_25_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_25_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_26_OFFSET                                    0x0000EE34
#define DSP_3WIRE_INTF_FSHLR_26_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_26_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_26_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_26_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_27_OFFSET                                    0x0000EE36
#define DSP_3WIRE_INTF_FSHLR_27_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_27_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_27_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_27_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_28_OFFSET                                    0x0000EE38
#define DSP_3WIRE_INTF_FSHLR_28_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_28_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_28_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_28_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_29_OFFSET                                    0x0000EE3A
#define DSP_3WIRE_INTF_FSHLR_29_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_29_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_29_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_29_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_30_OFFSET                                    0x0000EE3C
#define DSP_3WIRE_INTF_FSHLR_30_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_30_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_30_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_30_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_31_OFFSET                                    0x0000EE3E
#define DSP_3WIRE_INTF_FSHLR_31_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_31_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_31_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_31_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_32_OFFSET                                    0x0000EE40
#define DSP_3WIRE_INTF_FSHLR_32_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_32_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_32_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_32_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_33_OFFSET                                    0x0000EE42
#define DSP_3WIRE_INTF_FSHLR_33_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_33_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_33_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_33_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_34_OFFSET                                    0x0000EE44
#define DSP_3WIRE_INTF_FSHLR_34_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_34_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_34_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_34_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_35_OFFSET                                    0x0000EE46
#define DSP_3WIRE_INTF_FSHLR_35_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_35_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_35_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_35_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_36_OFFSET                                    0x0000EE48
#define DSP_3WIRE_INTF_FSHLR_36_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_36_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_36_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_36_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_37_OFFSET                                    0x0000EE4A
#define DSP_3WIRE_INTF_FSHLR_37_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_37_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_37_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_37_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_38_OFFSET                                    0x0000EE4C
#define DSP_3WIRE_INTF_FSHLR_38_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_38_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_38_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_38_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_39_OFFSET                                    0x0000EE4E
#define DSP_3WIRE_INTF_FSHLR_39_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_39_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_39_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_39_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_40_OFFSET                                    0x0000EE50
#define DSP_3WIRE_INTF_FSHLR_40_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_40_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_40_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_40_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_41_OFFSET                                    0x0000EE52
#define DSP_3WIRE_INTF_FSHLR_41_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_41_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_41_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_41_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_42_OFFSET                                    0x0000EE54
#define DSP_3WIRE_INTF_FSHLR_42_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_42_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_42_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_42_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_43_OFFSET                                    0x0000EE56
#define DSP_3WIRE_INTF_FSHLR_43_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_43_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_43_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_43_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_44_OFFSET                                    0x0000EE58
#define DSP_3WIRE_INTF_FSHLR_44_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_44_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_44_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_44_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_45_OFFSET                                    0x0000EE5A
#define DSP_3WIRE_INTF_FSHLR_45_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_45_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_45_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_45_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_46_OFFSET                                    0x0000EE5C
#define DSP_3WIRE_INTF_FSHLR_46_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_46_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_46_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_46_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_47_OFFSET                                    0x0000EE5E
#define DSP_3WIRE_INTF_FSHLR_47_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_47_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_47_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_47_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_48_OFFSET                                    0x0000EE60
#define DSP_3WIRE_INTF_FSHLR_48_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_48_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_48_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_48_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_49_OFFSET                                    0x0000EE62
#define DSP_3WIRE_INTF_FSHLR_49_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_49_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_49_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_49_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_50_OFFSET                                    0x0000EE64
#define DSP_3WIRE_INTF_FSHLR_50_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_50_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_50_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_50_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_51_OFFSET                                    0x0000EE66
#define DSP_3WIRE_INTF_FSHLR_51_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_51_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_51_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_51_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_52_OFFSET                                    0x0000EE68
#define DSP_3WIRE_INTF_FSHLR_52_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_52_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_52_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_52_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_53_OFFSET                                    0x0000EE6A
#define DSP_3WIRE_INTF_FSHLR_53_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_53_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_53_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_53_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_54_OFFSET                                    0x0000EE6C
#define DSP_3WIRE_INTF_FSHLR_54_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_54_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_54_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_54_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_55_OFFSET                                    0x0000EE6E
#define DSP_3WIRE_INTF_FSHLR_55_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_55_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_55_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_55_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_56_OFFSET                                    0x0000EE70
#define DSP_3WIRE_INTF_FSHLR_56_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_56_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_56_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_56_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_57_OFFSET                                    0x0000EE72
#define DSP_3WIRE_INTF_FSHLR_57_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_57_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_57_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_57_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_58_OFFSET                                    0x0000EE74
#define DSP_3WIRE_INTF_FSHLR_58_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_58_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_58_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_58_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_59_OFFSET                                    0x0000EE76
#define DSP_3WIRE_INTF_FSHLR_59_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_59_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_59_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_59_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_60_OFFSET                                    0x0000EE78
#define DSP_3WIRE_INTF_FSHLR_60_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_60_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_60_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_60_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_61_OFFSET                                    0x0000EE7A
#define DSP_3WIRE_INTF_FSHLR_61_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_61_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_61_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_61_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_62_OFFSET                                    0x0000EE7C
#define DSP_3WIRE_INTF_FSHLR_62_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_62_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_62_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_62_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_63_OFFSET                                    0x0000EE7E
#define DSP_3WIRE_INTF_FSHLR_63_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_63_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_63_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_63_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_64_OFFSET                                    0x0000EE80
#define DSP_3WIRE_INTF_FSHLR_64_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_64_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_64_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_64_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_65_OFFSET                                    0x0000EE82
#define DSP_3WIRE_INTF_FSHLR_65_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_65_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_65_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_65_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_66_OFFSET                                    0x0000EE84
#define DSP_3WIRE_INTF_FSHLR_66_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_66_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_66_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_66_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_67_OFFSET                                    0x0000EE86
#define DSP_3WIRE_INTF_FSHLR_67_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_67_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_67_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_67_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_68_OFFSET                                    0x0000EE88
#define DSP_3WIRE_INTF_FSHLR_68_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_68_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_68_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_68_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_69_OFFSET                                    0x0000EE8A
#define DSP_3WIRE_INTF_FSHLR_69_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_69_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_69_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_69_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_70_OFFSET                                    0x0000EE8C
#define DSP_3WIRE_INTF_FSHLR_70_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_70_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_70_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_70_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_71_OFFSET                                    0x0000EE8E
#define DSP_3WIRE_INTF_FSHLR_71_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_71_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_71_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_71_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_72_OFFSET                                    0x0000EE90
#define DSP_3WIRE_INTF_FSHLR_72_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_72_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_72_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_72_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_73_OFFSET                                    0x0000EE92
#define DSP_3WIRE_INTF_FSHLR_73_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_73_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_73_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_73_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_74_OFFSET                                    0x0000EE94
#define DSP_3WIRE_INTF_FSHLR_74_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_74_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_74_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_74_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_75_OFFSET                                    0x0000EE96
#define DSP_3WIRE_INTF_FSHLR_75_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_75_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_75_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_75_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_76_OFFSET                                    0x0000EE98
#define DSP_3WIRE_INTF_FSHLR_76_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_76_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_76_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_76_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_77_OFFSET                                    0x0000EE9A
#define DSP_3WIRE_INTF_FSHLR_77_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_77_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_77_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_77_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_78_OFFSET                                    0x0000EE9C
#define DSP_3WIRE_INTF_FSHLR_78_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_78_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_78_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_78_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_79_OFFSET                                    0x0000EE9E
#define DSP_3WIRE_INTF_FSHLR_79_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_79_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_79_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_79_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_80_OFFSET                                    0x0000EEA0
#define DSP_3WIRE_INTF_FSHLR_80_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_80_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_80_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_80_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_81_OFFSET                                    0x0000EEA2
#define DSP_3WIRE_INTF_FSHLR_81_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_81_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_81_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_81_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_82_OFFSET                                    0x0000EEA4
#define DSP_3WIRE_INTF_FSHLR_82_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_82_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_82_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_82_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_83_OFFSET                                    0x0000EEA6
#define DSP_3WIRE_INTF_FSHLR_83_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_83_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_83_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_83_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_84_OFFSET                                    0x0000EEA8
#define DSP_3WIRE_INTF_FSHLR_84_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_84_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_84_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_84_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_85_OFFSET                                    0x0000EEAA
#define DSP_3WIRE_INTF_FSHLR_85_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_85_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_85_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_85_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_86_OFFSET                                    0x0000EEAC
#define DSP_3WIRE_INTF_FSHLR_86_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_86_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_86_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_86_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_87_OFFSET                                    0x0000EEAE
#define DSP_3WIRE_INTF_FSHLR_87_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_87_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_87_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_87_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_88_OFFSET                                    0x0000EEB0
#define DSP_3WIRE_INTF_FSHLR_88_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_88_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_88_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_88_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_89_OFFSET                                    0x0000EEB2
#define DSP_3WIRE_INTF_FSHLR_89_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_89_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_89_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_89_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_90_OFFSET                                    0x0000EEB4
#define DSP_3WIRE_INTF_FSHLR_90_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_90_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_90_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_90_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_91_OFFSET                                    0x0000EEB6
#define DSP_3WIRE_INTF_FSHLR_91_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_91_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_91_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_91_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_92_OFFSET                                    0x0000EEB8
#define DSP_3WIRE_INTF_FSHLR_92_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_92_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_92_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_92_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_93_OFFSET                                    0x0000EEBA
#define DSP_3WIRE_INTF_FSHLR_93_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_93_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_93_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_93_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_94_OFFSET                                    0x0000EEBC
#define DSP_3WIRE_INTF_FSHLR_94_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_94_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_94_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_94_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_95_OFFSET                                    0x0000EEBE
#define DSP_3WIRE_INTF_FSHLR_95_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_95_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_95_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_95_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_96_OFFSET                                    0x0000EEC0
#define DSP_3WIRE_INTF_FSHLR_96_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_96_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_96_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_96_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_97_OFFSET                                    0x0000EEC2
#define DSP_3WIRE_INTF_FSHLR_97_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_97_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_97_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_97_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_98_OFFSET                                    0x0000EEC4
#define DSP_3WIRE_INTF_FSHLR_98_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_98_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_98_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_98_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_99_OFFSET                                    0x0000EEC6
#define DSP_3WIRE_INTF_FSHLR_99_TYPE                                      UInt32
#define DSP_3WIRE_INTF_FSHLR_99_RESERVED_MASK                             0x00000000
#define    DSP_3WIRE_INTF_FSHLR_99_FSN_SHIFT                              0
#define    DSP_3WIRE_INTF_FSHLR_99_FSN_MASK                               0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_100_OFFSET                                   0x0000EEC8
#define DSP_3WIRE_INTF_FSHLR_100_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_100_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_100_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_100_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_101_OFFSET                                   0x0000EECA
#define DSP_3WIRE_INTF_FSHLR_101_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_101_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_101_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_101_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_102_OFFSET                                   0x0000EECC
#define DSP_3WIRE_INTF_FSHLR_102_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_102_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_102_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_102_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_103_OFFSET                                   0x0000EECE
#define DSP_3WIRE_INTF_FSHLR_103_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_103_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_103_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_103_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_104_OFFSET                                   0x0000EED0
#define DSP_3WIRE_INTF_FSHLR_104_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_104_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_104_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_104_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_105_OFFSET                                   0x0000EED2
#define DSP_3WIRE_INTF_FSHLR_105_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_105_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_105_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_105_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_106_OFFSET                                   0x0000EED4
#define DSP_3WIRE_INTF_FSHLR_106_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_106_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_106_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_106_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_107_OFFSET                                   0x0000EED6
#define DSP_3WIRE_INTF_FSHLR_107_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_107_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_107_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_107_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_108_OFFSET                                   0x0000EED8
#define DSP_3WIRE_INTF_FSHLR_108_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_108_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_108_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_108_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_109_OFFSET                                   0x0000EEDA
#define DSP_3WIRE_INTF_FSHLR_109_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_109_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_109_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_109_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_110_OFFSET                                   0x0000EEDC
#define DSP_3WIRE_INTF_FSHLR_110_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_110_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_110_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_110_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_111_OFFSET                                   0x0000EEDE
#define DSP_3WIRE_INTF_FSHLR_111_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_111_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_111_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_111_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_112_OFFSET                                   0x0000EEE0
#define DSP_3WIRE_INTF_FSHLR_112_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_112_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_112_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_112_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_113_OFFSET                                   0x0000EEE2
#define DSP_3WIRE_INTF_FSHLR_113_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_113_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_113_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_113_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_114_OFFSET                                   0x0000EEE4
#define DSP_3WIRE_INTF_FSHLR_114_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_114_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_114_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_114_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_115_OFFSET                                   0x0000EEE6
#define DSP_3WIRE_INTF_FSHLR_115_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_115_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_115_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_115_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_116_OFFSET                                   0x0000EEE8
#define DSP_3WIRE_INTF_FSHLR_116_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_116_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_116_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_116_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_117_OFFSET                                   0x0000EEEA
#define DSP_3WIRE_INTF_FSHLR_117_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_117_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_117_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_117_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_118_OFFSET                                   0x0000EEEC
#define DSP_3WIRE_INTF_FSHLR_118_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_118_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_118_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_118_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_119_OFFSET                                   0x0000EEEE
#define DSP_3WIRE_INTF_FSHLR_119_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_119_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_119_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_119_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_120_OFFSET                                   0x0000EEF0
#define DSP_3WIRE_INTF_FSHLR_120_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_120_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_120_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_120_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_121_OFFSET                                   0x0000EEF2
#define DSP_3WIRE_INTF_FSHLR_121_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_121_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_121_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_121_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_122_OFFSET                                   0x0000EEF4
#define DSP_3WIRE_INTF_FSHLR_122_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_122_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_122_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_122_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_123_OFFSET                                   0x0000EEF6
#define DSP_3WIRE_INTF_FSHLR_123_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_123_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_123_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_123_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_124_OFFSET                                   0x0000EEF8
#define DSP_3WIRE_INTF_FSHLR_124_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_124_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_124_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_124_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_125_OFFSET                                   0x0000EEFA
#define DSP_3WIRE_INTF_FSHLR_125_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_125_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_125_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_125_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_126_OFFSET                                   0x0000EEFC
#define DSP_3WIRE_INTF_FSHLR_126_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_126_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_126_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_126_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_127_OFFSET                                   0x0000EEFE
#define DSP_3WIRE_INTF_FSHLR_127_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_127_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_127_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_127_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_128_OFFSET                                   0x0000EF00
#define DSP_3WIRE_INTF_FSHLR_128_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_128_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_128_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_128_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_129_OFFSET                                   0x0000EF02
#define DSP_3WIRE_INTF_FSHLR_129_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_129_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_129_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_129_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_130_OFFSET                                   0x0000EF04
#define DSP_3WIRE_INTF_FSHLR_130_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_130_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_130_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_130_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_131_OFFSET                                   0x0000EF06
#define DSP_3WIRE_INTF_FSHLR_131_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_131_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_131_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_131_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_132_OFFSET                                   0x0000EF08
#define DSP_3WIRE_INTF_FSHLR_132_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_132_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_132_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_132_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_133_OFFSET                                   0x0000EF0A
#define DSP_3WIRE_INTF_FSHLR_133_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_133_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_133_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_133_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_134_OFFSET                                   0x0000EF0C
#define DSP_3WIRE_INTF_FSHLR_134_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_134_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_134_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_134_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_135_OFFSET                                   0x0000EF0E
#define DSP_3WIRE_INTF_FSHLR_135_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_135_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_135_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_135_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_136_OFFSET                                   0x0000EF10
#define DSP_3WIRE_INTF_FSHLR_136_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_136_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_136_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_136_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_137_OFFSET                                   0x0000EF12
#define DSP_3WIRE_INTF_FSHLR_137_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_137_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_137_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_137_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_138_OFFSET                                   0x0000EF14
#define DSP_3WIRE_INTF_FSHLR_138_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_138_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_138_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_138_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_139_OFFSET                                   0x0000EF16
#define DSP_3WIRE_INTF_FSHLR_139_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_139_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_139_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_139_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_140_OFFSET                                   0x0000EF18
#define DSP_3WIRE_INTF_FSHLR_140_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_140_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_140_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_140_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_141_OFFSET                                   0x0000EF1A
#define DSP_3WIRE_INTF_FSHLR_141_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_141_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_141_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_141_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_142_OFFSET                                   0x0000EF1C
#define DSP_3WIRE_INTF_FSHLR_142_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_142_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_142_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_142_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_143_OFFSET                                   0x0000EF1E
#define DSP_3WIRE_INTF_FSHLR_143_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_143_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_143_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_143_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_144_OFFSET                                   0x0000EF20
#define DSP_3WIRE_INTF_FSHLR_144_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_144_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_144_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_144_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_145_OFFSET                                   0x0000EF22
#define DSP_3WIRE_INTF_FSHLR_145_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_145_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_145_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_145_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_146_OFFSET                                   0x0000EF24
#define DSP_3WIRE_INTF_FSHLR_146_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_146_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_146_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_146_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_147_OFFSET                                   0x0000EF26
#define DSP_3WIRE_INTF_FSHLR_147_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_147_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_147_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_147_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_148_OFFSET                                   0x0000EF28
#define DSP_3WIRE_INTF_FSHLR_148_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_148_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_148_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_148_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_149_OFFSET                                   0x0000EF2A
#define DSP_3WIRE_INTF_FSHLR_149_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_149_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_149_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_149_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_150_OFFSET                                   0x0000EF2C
#define DSP_3WIRE_INTF_FSHLR_150_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_150_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_150_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_150_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_151_OFFSET                                   0x0000EF2E
#define DSP_3WIRE_INTF_FSHLR_151_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_151_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_151_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_151_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_152_OFFSET                                   0x0000EF30
#define DSP_3WIRE_INTF_FSHLR_152_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_152_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_152_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_152_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_153_OFFSET                                   0x0000EF32
#define DSP_3WIRE_INTF_FSHLR_153_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_153_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_153_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_153_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_154_OFFSET                                   0x0000EF34
#define DSP_3WIRE_INTF_FSHLR_154_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_154_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_154_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_154_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_155_OFFSET                                   0x0000EF36
#define DSP_3WIRE_INTF_FSHLR_155_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_155_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_155_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_155_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_156_OFFSET                                   0x0000EF38
#define DSP_3WIRE_INTF_FSHLR_156_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_156_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_156_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_156_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_157_OFFSET                                   0x0000EF3A
#define DSP_3WIRE_INTF_FSHLR_157_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_157_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_157_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_157_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_158_OFFSET                                   0x0000EF3C
#define DSP_3WIRE_INTF_FSHLR_158_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_158_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_158_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_158_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FSHLR_159_OFFSET                                   0x0000EF3E
#define DSP_3WIRE_INTF_FSHLR_159_TYPE                                     UInt32
#define DSP_3WIRE_INTF_FSHLR_159_RESERVED_MASK                            0x00000000
#define    DSP_3WIRE_INTF_FSHLR_159_FSN_SHIFT                             0
#define    DSP_3WIRE_INTF_FSHLR_159_FSN_MASK                              0xFFFFFFFF

#define DSP_3WIRE_INTF_FCHR_0_OFFSET                                      0x0000EF40
#define DSP_3WIRE_INTF_FCHR_0_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_0_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_0_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_0_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_0_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_0_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_0_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_0_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_0_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_0_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_0_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_0_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_0_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_0_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_0_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_0_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_0_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_0_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_1_OFFSET                                      0x0000EF41
#define DSP_3WIRE_INTF_FCHR_1_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_1_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_1_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_1_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_1_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_1_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_1_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_1_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_1_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_1_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_1_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_1_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_1_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_1_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_1_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_1_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_1_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_1_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_2_OFFSET                                      0x0000EF42
#define DSP_3WIRE_INTF_FCHR_2_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_2_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_2_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_2_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_2_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_2_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_2_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_2_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_2_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_2_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_2_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_2_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_2_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_2_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_2_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_2_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_2_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_2_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_3_OFFSET                                      0x0000EF43
#define DSP_3WIRE_INTF_FCHR_3_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_3_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_3_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_3_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_3_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_3_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_3_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_3_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_3_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_3_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_3_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_3_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_3_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_3_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_3_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_3_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_3_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_3_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_4_OFFSET                                      0x0000EF44
#define DSP_3WIRE_INTF_FCHR_4_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_4_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_4_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_4_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_4_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_4_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_4_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_4_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_4_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_4_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_4_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_4_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_4_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_4_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_4_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_4_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_4_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_4_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_5_OFFSET                                      0x0000EF45
#define DSP_3WIRE_INTF_FCHR_5_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_5_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_5_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_5_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_5_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_5_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_5_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_5_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_5_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_5_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_5_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_5_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_5_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_5_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_5_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_5_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_5_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_5_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_6_OFFSET                                      0x0000EF46
#define DSP_3WIRE_INTF_FCHR_6_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_6_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_6_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_6_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_6_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_6_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_6_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_6_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_6_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_6_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_6_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_6_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_6_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_6_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_6_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_6_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_6_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_6_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_7_OFFSET                                      0x0000EF47
#define DSP_3WIRE_INTF_FCHR_7_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_7_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_7_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_7_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_7_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_7_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_7_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_7_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_7_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_7_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_7_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_7_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_7_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_7_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_7_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_7_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_7_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_7_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_8_OFFSET                                      0x0000EF48
#define DSP_3WIRE_INTF_FCHR_8_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_8_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_8_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_8_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_8_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_8_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_8_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_8_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_8_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_8_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_8_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_8_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_8_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_8_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_8_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_8_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_8_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_8_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_9_OFFSET                                      0x0000EF49
#define DSP_3WIRE_INTF_FCHR_9_TYPE                                        UInt16
#define DSP_3WIRE_INTF_FCHR_9_RESERVED_MASK                               0x00000000
#define    DSP_3WIRE_INTF_FCHR_9_FSX7_SHIFT                               14
#define    DSP_3WIRE_INTF_FCHR_9_FSX7_MASK                                0x0000C000
#define    DSP_3WIRE_INTF_FCHR_9_FSX6_SHIFT                               12
#define    DSP_3WIRE_INTF_FCHR_9_FSX6_MASK                                0x00003000
#define    DSP_3WIRE_INTF_FCHR_9_FSX5_SHIFT                               10
#define    DSP_3WIRE_INTF_FCHR_9_FSX5_MASK                                0x00000C00
#define    DSP_3WIRE_INTF_FCHR_9_FSX4_SHIFT                               8
#define    DSP_3WIRE_INTF_FCHR_9_FSX4_MASK                                0x00000300
#define    DSP_3WIRE_INTF_FCHR_9_FSX3_SHIFT                               6
#define    DSP_3WIRE_INTF_FCHR_9_FSX3_MASK                                0x000000C0
#define    DSP_3WIRE_INTF_FCHR_9_FSX2_SHIFT                               4
#define    DSP_3WIRE_INTF_FCHR_9_FSX2_MASK                                0x00000030
#define    DSP_3WIRE_INTF_FCHR_9_FSX1_SHIFT                               2
#define    DSP_3WIRE_INTF_FCHR_9_FSX1_MASK                                0x0000000C
#define    DSP_3WIRE_INTF_FCHR_9_FSX0_SHIFT                               0
#define    DSP_3WIRE_INTF_FCHR_9_FSX0_MASK                                0x00000003

#define DSP_3WIRE_INTF_FCHR_10_OFFSET                                     0x0000EF4A
#define DSP_3WIRE_INTF_FCHR_10_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_10_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_10_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_10_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_10_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_10_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_10_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_10_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_10_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_10_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_10_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_10_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_10_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_10_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_10_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_10_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_10_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_10_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_11_OFFSET                                     0x0000EF4B
#define DSP_3WIRE_INTF_FCHR_11_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_11_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_11_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_11_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_11_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_11_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_11_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_11_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_11_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_11_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_11_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_11_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_11_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_11_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_11_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_11_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_11_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_11_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_12_OFFSET                                     0x0000EF4C
#define DSP_3WIRE_INTF_FCHR_12_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_12_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_12_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_12_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_12_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_12_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_12_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_12_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_12_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_12_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_12_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_12_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_12_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_12_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_12_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_12_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_12_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_12_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_13_OFFSET                                     0x0000EF4D
#define DSP_3WIRE_INTF_FCHR_13_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_13_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_13_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_13_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_13_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_13_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_13_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_13_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_13_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_13_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_13_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_13_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_13_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_13_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_13_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_13_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_13_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_13_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_14_OFFSET                                     0x0000EF4E
#define DSP_3WIRE_INTF_FCHR_14_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_14_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_14_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_14_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_14_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_14_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_14_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_14_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_14_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_14_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_14_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_14_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_14_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_14_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_14_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_14_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_14_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_14_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_15_OFFSET                                     0x0000EF4F
#define DSP_3WIRE_INTF_FCHR_15_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_15_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_15_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_15_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_15_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_15_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_15_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_15_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_15_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_15_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_15_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_15_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_15_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_15_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_15_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_15_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_15_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_15_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_16_OFFSET                                     0x0000EF50
#define DSP_3WIRE_INTF_FCHR_16_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_16_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_16_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_16_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_16_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_16_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_16_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_16_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_16_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_16_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_16_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_16_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_16_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_16_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_16_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_16_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_16_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_16_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_17_OFFSET                                     0x0000EF51
#define DSP_3WIRE_INTF_FCHR_17_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_17_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_17_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_17_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_17_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_17_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_17_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_17_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_17_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_17_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_17_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_17_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_17_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_17_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_17_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_17_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_17_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_17_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_18_OFFSET                                     0x0000EF52
#define DSP_3WIRE_INTF_FCHR_18_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_18_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_18_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_18_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_18_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_18_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_18_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_18_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_18_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_18_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_18_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_18_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_18_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_18_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_18_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_18_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_18_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_18_FSX0_MASK                               0x00000003

#define DSP_3WIRE_INTF_FCHR_19_OFFSET                                     0x0000EF53
#define DSP_3WIRE_INTF_FCHR_19_TYPE                                       UInt16
#define DSP_3WIRE_INTF_FCHR_19_RESERVED_MASK                              0x00000000
#define    DSP_3WIRE_INTF_FCHR_19_FSX7_SHIFT                              14
#define    DSP_3WIRE_INTF_FCHR_19_FSX7_MASK                               0x0000C000
#define    DSP_3WIRE_INTF_FCHR_19_FSX6_SHIFT                              12
#define    DSP_3WIRE_INTF_FCHR_19_FSX6_MASK                               0x00003000
#define    DSP_3WIRE_INTF_FCHR_19_FSX5_SHIFT                              10
#define    DSP_3WIRE_INTF_FCHR_19_FSX5_MASK                               0x00000C00
#define    DSP_3WIRE_INTF_FCHR_19_FSX4_SHIFT                              8
#define    DSP_3WIRE_INTF_FCHR_19_FSX4_MASK                               0x00000300
#define    DSP_3WIRE_INTF_FCHR_19_FSX3_SHIFT                              6
#define    DSP_3WIRE_INTF_FCHR_19_FSX3_MASK                               0x000000C0
#define    DSP_3WIRE_INTF_FCHR_19_FSX2_SHIFT                              4
#define    DSP_3WIRE_INTF_FCHR_19_FSX2_MASK                               0x00000030
#define    DSP_3WIRE_INTF_FCHR_19_FSX1_SHIFT                              2
#define    DSP_3WIRE_INTF_FCHR_19_FSX1_MASK                               0x0000000C
#define    DSP_3WIRE_INTF_FCHR_19_FSX0_SHIFT                              0
#define    DSP_3WIRE_INTF_FCHR_19_FSX0_MASK                               0x00000003

#endif /* __BRCM_RDB_DSP_3WIRE_INTF_H__ */



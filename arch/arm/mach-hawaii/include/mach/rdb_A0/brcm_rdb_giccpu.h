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
/*     Date     : Generated on 7/19/2012 6:24:14                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_GICCPU_H__
#define __BRCM_RDB_GICCPU_H__

#define GICCPU_CONTROL_OFFSET                                             0x00000000
#define GICCPU_CONTROL_TYPE                                               UInt32
#define GICCPU_CONTROL_RESERVED_MASK                                      0xFFFFFFE0
#define    GICCPU_CONTROL_SECURE_SBPR_SHIFT                               4
#define    GICCPU_CONTROL_SECURE_SBPR_MASK                                0x00000010
#define    GICCPU_CONTROL_SECURE_FIQEN_SHIFT                              3
#define    GICCPU_CONTROL_SECURE_FIQEN_MASK                               0x00000008
#define       GICCPU_CONTROL_FIQEN_CMD_NIRQ_ONLY                          0x00000000
#define       GICCPU_CONTROL_FIQEN_CMD_NFIQ_NIRQ                          0x00000001
#define    GICCPU_CONTROL_SECURE_ACKCTL_SHIFT                             2
#define    GICCPU_CONTROL_SECURE_ACKCTL_MASK                              0x00000004
#define       GICCPU_CONTROL_ACKCTL_CMD_INTID_PENDING                     0x00000000
#define       GICCPU_CONTROL_ACKCTL_CMD_INTID_ACK                         0x00000001
#define    GICCPU_CONTROL_SECURE_ENABLENS_SHIFT                           1
#define    GICCPU_CONTROL_SECURE_ENABLENS_MASK                            0x00000002
#define       GICCPU_CONTROL_ENABLENS_CMD_DISABLE                         0x00000000
#define       GICCPU_CONTROL_ENABLENS_CMD_ENABLE                          0x00000001
#define    GICCPU_CONTROL_SECURE_ENABLES_SHIFT                            0
#define    GICCPU_CONTROL_SECURE_ENABLES_MASK                             0x00000001
#define       GICCPU_CONTROL_ENABLES_CMD_DISABLE                          0x00000000
#define       GICCPU_CONTROL_ENABLES_CMD_ENABLE                           0x00000001
#define    GICCPU_CONTROL_NONSECURE_ENABLE_SHIFT                          0
#define    GICCPU_CONTROL_NONSECURE_ENABLE_MASK                           0x00000001
#define       GICCPU_CONTROL_ENABLE_CMD_ENABLE                            0x00000001
#define       GICCPU_CONTROL_ENABLE_CMD_DISABLE                           0x00000000

#define GICCPU_PRIORITY_MASK_OFFSET                                       0x00000004
#define GICCPU_PRIORITY_MASK_TYPE                                         UInt32
#define GICCPU_PRIORITY_MASK_RESERVED_MASK                                0xFFFFFF00
#define    GICCPU_PRIORITY_MASK_PRIORITY_SHIFT                            0
#define    GICCPU_PRIORITY_MASK_PRIORITY_MASK                             0x000000FF

#define GICCPU_BIN_PT_OFFSET                                              0x00000008
#define GICCPU_BIN_PT_TYPE                                                UInt32
#define GICCPU_BIN_PT_RESERVED_MASK                                       0xFFFFFFF8
#define    GICCPU_BIN_PT_SECURE_BIN_PT_S_SHIFT                            0
#define    GICCPU_BIN_PT_SECURE_BIN_PT_S_MASK                             0x00000007
#define    GICCPU_BIN_PT_NONSECURE_BIN_PT_NS_SHIFT                        0
#define    GICCPU_BIN_PT_NONSECURE_BIN_PT_NS_MASK                         0x00000007

#define GICCPU_INT_ACK_OFFSET                                             0x0000000C
#define GICCPU_INT_ACK_TYPE                                               UInt32
#define GICCPU_INT_ACK_RESERVED_MASK                                      0xFFFFE000
#define    GICCPU_INT_ACK_SOURCE_CPUID_SHIFT                              10
#define    GICCPU_INT_ACK_SOURCE_CPUID_MASK                               0x00001C00
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU0_STI                    0x00000000
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU1_STI                    0x00000001
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU2_STI                    0x00000002
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU3_STI                    0x00000003
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU4_STI                    0x00000004
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU5_STI                    0x00000005
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU6_STI                    0x00000006
#define       GICCPU_INT_ACK_SOURCE_CPUID_CMD_CPU7_STI                    0x00000007
#define    GICCPU_INT_ACK_ACK_INTID_SHIFT                                 0
#define    GICCPU_INT_ACK_ACK_INTID_MASK                                  0x000003FF

#define GICCPU_EOI_OFFSET                                                 0x00000010
#define GICCPU_EOI_TYPE                                                   UInt32
#define GICCPU_EOI_RESERVED_MASK                                          0xFFFFE000
#define    GICCPU_EOI_SOURCECPUID_SHIFT                                   10
#define    GICCPU_EOI_SOURCECPUID_MASK                                    0x00001C00
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU0_STI                         0x00000000
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU1_STI                         0x00000001
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU2_STI                         0x00000002
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU3_STI                         0x00000003
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU4_STI                         0x00000004
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU5_STI                         0x00000005
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU6_STI                         0x00000006
#define       GICCPU_EOI_SOURCECPUID_CMD_CPU7_STI                         0x00000007
#define    GICCPU_EOI_EOI_INTID_SHIFT                                     0
#define    GICCPU_EOI_EOI_INTID_MASK                                      0x000003FF

#define GICCPU_RUN_PRIORITY_OFFSET                                        0x00000014
#define GICCPU_RUN_PRIORITY_TYPE                                          UInt32
#define GICCPU_RUN_PRIORITY_RESERVED_MASK                                 0xFFFFFF00
#define    GICCPU_RUN_PRIORITY_PRIORITY_SHIFT                             0
#define    GICCPU_RUN_PRIORITY_PRIORITY_MASK                              0x000000FF

#define GICCPU_HI_PEND_OFFSET                                             0x00000018
#define GICCPU_HI_PEND_TYPE                                               UInt32
#define GICCPU_HI_PEND_RESERVED_MASK                                      0xFFFFE000
#define    GICCPU_HI_PEND_SOURCECPUID_SHIFT                               10
#define    GICCPU_HI_PEND_SOURCECPUID_MASK                                0x00001C00
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU0_STI                     0x00000000
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU1_STI                     0x00000001
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU2_STI                     0x00000002
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU3_STI                     0x00000003
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU4_STI                     0x00000004
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU5_STI                     0x00000005
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU6_STI                     0x00000006
#define       GICCPU_HI_PEND_SOURCECPUID_CMD_CPU7_STI                     0x00000007
#define    GICCPU_HI_PEND_PEND_INTID_SHIFT                                0
#define    GICCPU_HI_PEND_PEND_INTID_MASK                                 0x000003FF

#define GICCPU_ALIAS_BIN_PT_NS_OFFSET                                     0x0000001C
#define GICCPU_ALIAS_BIN_PT_NS_TYPE                                       UInt32
#define GICCPU_ALIAS_BIN_PT_NS_RESERVED_MASK                              0xFFFFFFF8
#define    GICCPU_ALIAS_BIN_PT_NS_BIN_PT_NS_SHIFT                         0
#define    GICCPU_ALIAS_BIN_PT_NS_BIN_PT_NS_MASK                          0x00000007

#define GICCPU_INT_CFG_OFFSET                                             0x00000040
#define GICCPU_INT_CFG_TYPE                                               UInt32
#define GICCPU_INT_CFG_RESERVED_MASK                                      0xFFFFFFFE
#define    GICCPU_INT_CFG_INT_TEST_EN_SHIFT                               0
#define    GICCPU_INT_CFG_INT_TEST_EN_MASK                                0x00000001

#define GICCPU_INT_FIQ_SET_OFFSET                                         0x00000044
#define GICCPU_INT_FIQ_SET_TYPE                                           UInt32
#define GICCPU_INT_FIQ_SET_RESERVED_MASK                                  0xFFFFFFFC
#define    GICCPU_INT_FIQ_SET_FIQ_SET_SHIFT                               1
#define    GICCPU_INT_FIQ_SET_FIQ_SET_MASK                                0x00000002
#define    GICCPU_INT_FIQ_SET_IRQ_SET_SHIFT                               0
#define    GICCPU_INT_FIQ_SET_IRQ_SET_MASK                                0x00000001

#define GICCPU_INTEG_MATCH_OFFSET                                         0x00000050
#define GICCPU_INTEG_MATCH_TYPE                                           UInt32
#define GICCPU_INTEG_MATCH_RESERVED_MASK                                  0x00000000
#define    GICCPU_INTEG_MATCH_MATCH_C_31_SHIFT                            31
#define    GICCPU_INTEG_MATCH_MATCH_C_31_MASK                             0x80000000
#define    GICCPU_INTEG_MATCH_MATCH_C_30_SHIFT                            30
#define    GICCPU_INTEG_MATCH_MATCH_C_30_MASK                             0x40000000
#define    GICCPU_INTEG_MATCH_MATCH_C_29_SHIFT                            29
#define    GICCPU_INTEG_MATCH_MATCH_C_29_MASK                             0x20000000
#define    GICCPU_INTEG_MATCH_MATCH_C_28_SHIFT                            28
#define    GICCPU_INTEG_MATCH_MATCH_C_28_MASK                             0x10000000
#define    GICCPU_INTEG_MATCH_MATCH_C_27_SHIFT                            27
#define    GICCPU_INTEG_MATCH_MATCH_C_27_MASK                             0x08000000
#define    GICCPU_INTEG_MATCH_MATCH_C_26_SHIFT                            26
#define    GICCPU_INTEG_MATCH_MATCH_C_26_MASK                             0x04000000
#define    GICCPU_INTEG_MATCH_MATCH_C_25_SHIFT                            25
#define    GICCPU_INTEG_MATCH_MATCH_C_25_MASK                             0x02000000
#define    GICCPU_INTEG_MATCH_MATCH_C_24_SHIFT                            24
#define    GICCPU_INTEG_MATCH_MATCH_C_24_MASK                             0x01000000
#define    GICCPU_INTEG_MATCH_MATCH_C_23_SHIFT                            23
#define    GICCPU_INTEG_MATCH_MATCH_C_23_MASK                             0x00800000
#define    GICCPU_INTEG_MATCH_MATCH_C_22_SHIFT                            22
#define    GICCPU_INTEG_MATCH_MATCH_C_22_MASK                             0x00400000
#define    GICCPU_INTEG_MATCH_MATCH_C_21_SHIFT                            21
#define    GICCPU_INTEG_MATCH_MATCH_C_21_MASK                             0x00200000
#define    GICCPU_INTEG_MATCH_MATCH_C_20_SHIFT                            20
#define    GICCPU_INTEG_MATCH_MATCH_C_20_MASK                             0x00100000
#define    GICCPU_INTEG_MATCH_MATCH_C_19_SHIFT                            19
#define    GICCPU_INTEG_MATCH_MATCH_C_19_MASK                             0x00080000
#define    GICCPU_INTEG_MATCH_MATCH_C_18_SHIFT                            18
#define    GICCPU_INTEG_MATCH_MATCH_C_18_MASK                             0x00040000
#define    GICCPU_INTEG_MATCH_MATCH_C_17_SHIFT                            17
#define    GICCPU_INTEG_MATCH_MATCH_C_17_MASK                             0x00020000
#define    GICCPU_INTEG_MATCH_MATCH_C_16_SHIFT                            16
#define    GICCPU_INTEG_MATCH_MATCH_C_16_MASK                             0x00010000
#define    GICCPU_INTEG_MATCH_MATCH_C_15_SHIFT                            15
#define    GICCPU_INTEG_MATCH_MATCH_C_15_MASK                             0x00008000
#define    GICCPU_INTEG_MATCH_MATCH_C_14_SHIFT                            14
#define    GICCPU_INTEG_MATCH_MATCH_C_14_MASK                             0x00004000
#define    GICCPU_INTEG_MATCH_MATCH_C_13_SHIFT                            13
#define    GICCPU_INTEG_MATCH_MATCH_C_13_MASK                             0x00002000
#define    GICCPU_INTEG_MATCH_MATCH_C_12_SHIFT                            12
#define    GICCPU_INTEG_MATCH_MATCH_C_12_MASK                             0x00001000
#define    GICCPU_INTEG_MATCH_MATCH_C_11_SHIFT                            11
#define    GICCPU_INTEG_MATCH_MATCH_C_11_MASK                             0x00000800
#define    GICCPU_INTEG_MATCH_MATCH_C_10_SHIFT                            10
#define    GICCPU_INTEG_MATCH_MATCH_C_10_MASK                             0x00000400
#define    GICCPU_INTEG_MATCH_MATCH_C_09_SHIFT                            9
#define    GICCPU_INTEG_MATCH_MATCH_C_09_MASK                             0x00000200
#define    GICCPU_INTEG_MATCH_MATCH_C_08_SHIFT                            8
#define    GICCPU_INTEG_MATCH_MATCH_C_08_MASK                             0x00000100
#define    GICCPU_INTEG_MATCH_MATCH_C_07_SHIFT                            7
#define    GICCPU_INTEG_MATCH_MATCH_C_07_MASK                             0x00000080
#define    GICCPU_INTEG_MATCH_MATCH_C_06_SHIFT                            6
#define    GICCPU_INTEG_MATCH_MATCH_C_06_MASK                             0x00000040
#define    GICCPU_INTEG_MATCH_MATCH_C_05_SHIFT                            5
#define    GICCPU_INTEG_MATCH_MATCH_C_05_MASK                             0x00000020
#define    GICCPU_INTEG_MATCH_MATCH_C_04_SHIFT                            4
#define    GICCPU_INTEG_MATCH_MATCH_C_04_MASK                             0x00000010
#define    GICCPU_INTEG_MATCH_MATCH_C_03_SHIFT                            3
#define    GICCPU_INTEG_MATCH_MATCH_C_03_MASK                             0x00000008
#define    GICCPU_INTEG_MATCH_MATCH_C_02_SHIFT                            2
#define    GICCPU_INTEG_MATCH_MATCH_C_02_MASK                             0x00000004
#define    GICCPU_INTEG_MATCH_MATCH_C_01_SHIFT                            1
#define    GICCPU_INTEG_MATCH_MATCH_C_01_MASK                             0x00000002
#define    GICCPU_INTEG_MATCH_MATCH_C_00_SHIFT                            0
#define    GICCPU_INTEG_MATCH_MATCH_C_00_MASK                             0x00000001

#define GICCPU_INTEG_ENABLE_OFFSET                                        0x00000054
#define GICCPU_INTEG_ENABLE_TYPE                                          UInt32
#define GICCPU_INTEG_ENABLE_RESERVED_MASK                                 0x00000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_31_SHIFT                          31
#define    GICCPU_INTEG_ENABLE_ENABLE_C_31_MASK                           0x80000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_30_SHIFT                          30
#define    GICCPU_INTEG_ENABLE_ENABLE_C_30_MASK                           0x40000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_29_SHIFT                          29
#define    GICCPU_INTEG_ENABLE_ENABLE_C_29_MASK                           0x20000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_28_SHIFT                          28
#define    GICCPU_INTEG_ENABLE_ENABLE_C_28_MASK                           0x10000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_27_SHIFT                          27
#define    GICCPU_INTEG_ENABLE_ENABLE_C_27_MASK                           0x08000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_26_SHIFT                          26
#define    GICCPU_INTEG_ENABLE_ENABLE_C_26_MASK                           0x04000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_25_SHIFT                          25
#define    GICCPU_INTEG_ENABLE_ENABLE_C_25_MASK                           0x02000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_24_SHIFT                          24
#define    GICCPU_INTEG_ENABLE_ENABLE_C_24_MASK                           0x01000000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_23_SHIFT                          23
#define    GICCPU_INTEG_ENABLE_ENABLE_C_23_MASK                           0x00800000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_22_SHIFT                          22
#define    GICCPU_INTEG_ENABLE_ENABLE_C_22_MASK                           0x00400000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_21_SHIFT                          21
#define    GICCPU_INTEG_ENABLE_ENABLE_C_21_MASK                           0x00200000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_20_SHIFT                          20
#define    GICCPU_INTEG_ENABLE_ENABLE_C_20_MASK                           0x00100000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_19_SHIFT                          19
#define    GICCPU_INTEG_ENABLE_ENABLE_C_19_MASK                           0x00080000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_18_SHIFT                          18
#define    GICCPU_INTEG_ENABLE_ENABLE_C_18_MASK                           0x00040000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_17_SHIFT                          17
#define    GICCPU_INTEG_ENABLE_ENABLE_C_17_MASK                           0x00020000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_16_SHIFT                          16
#define    GICCPU_INTEG_ENABLE_ENABLE_C_16_MASK                           0x00010000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_15_SHIFT                          15
#define    GICCPU_INTEG_ENABLE_ENABLE_C_15_MASK                           0x00008000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_14_SHIFT                          14
#define    GICCPU_INTEG_ENABLE_ENABLE_C_14_MASK                           0x00004000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_13_SHIFT                          13
#define    GICCPU_INTEG_ENABLE_ENABLE_C_13_MASK                           0x00002000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_12_SHIFT                          12
#define    GICCPU_INTEG_ENABLE_ENABLE_C_12_MASK                           0x00001000
#define    GICCPU_INTEG_ENABLE_ENABLE_C_11_SHIFT                          11
#define    GICCPU_INTEG_ENABLE_ENABLE_C_11_MASK                           0x00000800
#define    GICCPU_INTEG_ENABLE_ENABLE_C_10_SHIFT                          10
#define    GICCPU_INTEG_ENABLE_ENABLE_C_10_MASK                           0x00000400
#define    GICCPU_INTEG_ENABLE_ENABLE_C_09_SHIFT                          9
#define    GICCPU_INTEG_ENABLE_ENABLE_C_09_MASK                           0x00000200
#define    GICCPU_INTEG_ENABLE_ENABLE_C_08_SHIFT                          8
#define    GICCPU_INTEG_ENABLE_ENABLE_C_08_MASK                           0x00000100
#define    GICCPU_INTEG_ENABLE_ENABLE_C_07_SHIFT                          7
#define    GICCPU_INTEG_ENABLE_ENABLE_C_07_MASK                           0x00000080
#define    GICCPU_INTEG_ENABLE_ENABLE_C_06_SHIFT                          6
#define    GICCPU_INTEG_ENABLE_ENABLE_C_06_MASK                           0x00000040
#define    GICCPU_INTEG_ENABLE_ENABLE_C_05_SHIFT                          5
#define    GICCPU_INTEG_ENABLE_ENABLE_C_05_MASK                           0x00000020
#define    GICCPU_INTEG_ENABLE_ENABLE_C_04_SHIFT                          4
#define    GICCPU_INTEG_ENABLE_ENABLE_C_04_MASK                           0x00000010
#define    GICCPU_INTEG_ENABLE_ENABLE_C_03_SHIFT                          3
#define    GICCPU_INTEG_ENABLE_ENABLE_C_03_MASK                           0x00000008
#define    GICCPU_INTEG_ENABLE_ENABLE_C_02_SHIFT                          2
#define    GICCPU_INTEG_ENABLE_ENABLE_C_02_MASK                           0x00000004
#define    GICCPU_INTEG_ENABLE_ENABLE_C_01_SHIFT                          1
#define    GICCPU_INTEG_ENABLE_ENABLE_C_01_MASK                           0x00000002
#define    GICCPU_INTEG_ENABLE_ENABLE_C_00_SHIFT                          0
#define    GICCPU_INTEG_ENABLE_ENABLE_C_00_MASK                           0x00000001

#define GICCPU_CPU_IDENT_OFFSET                                           0x000000FC
#define GICCPU_CPU_IDENT_TYPE                                             UInt32
#define GICCPU_CPU_IDENT_RESERVED_MASK                                    0x00000000
#define    GICCPU_CPU_IDENT_PART_NUMBER_SHIFT                             20
#define    GICCPU_CPU_IDENT_PART_NUMBER_MASK                              0xFFF00000
#define    GICCPU_CPU_IDENT_ARCH_VER_SHIFT                                16
#define    GICCPU_CPU_IDENT_ARCH_VER_MASK                                 0x000F0000
#define    GICCPU_CPU_IDENT_REV_NUM_SHIFT                                 12
#define    GICCPU_CPU_IDENT_REV_NUM_MASK                                  0x0000F000
#define    GICCPU_CPU_IDENT_IMPLEMENTOR_SHIFT                             0
#define    GICCPU_CPU_IDENT_IMPLEMENTOR_MASK                              0x00000FFF

#endif /* __BRCM_RDB_GICCPU_H__ */



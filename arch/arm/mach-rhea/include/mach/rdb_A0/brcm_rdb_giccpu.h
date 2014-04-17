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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_GICCPU_H__
#define __BRCM_RDB_GICCPU_H__

#define GICCPU_CONTROL_OFFSET                                             0x00000000
#define GICCPU_CONTROL_TYPE                                               UInt32
#define GICCPU_CONTROL_RESERVED_MASK                                      0xFFFFFFE0
#define    GICCPU_CONTROL_UNION_SHIFT                                     0
#define    GICCPU_CONTROL_UNION_MASK                                      0x0000001F

#define GICCPU_PRIORITY_MASK_OFFSET                                       0x00000004
#define GICCPU_PRIORITY_MASK_TYPE                                         UInt32
#define GICCPU_PRIORITY_MASK_RESERVED_MASK                                0xFFFFFF00
#define    GICCPU_PRIORITY_MASK_PRIORITY_SHIFT                            0
#define    GICCPU_PRIORITY_MASK_PRIORITY_MASK                             0x000000FF

#define GICCPU_BIN_PT_OFFSET                                              0x00000008
#define GICCPU_BIN_PT_TYPE                                                UInt32
#define GICCPU_BIN_PT_RESERVED_MASK                                       0xFFFFFFF8
#define    GICCPU_BIN_PT_UNION_SHIFT                                      0
#define    GICCPU_BIN_PT_UNION_MASK                                       0x00000007

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

#define GICCPU_ICPIIR_OFFSET                                              0x000000FC
#define GICCPU_ICPIIR_TYPE                                                UInt32
#define GICCPU_ICPIIR_RESERVED_MASK                                       0x00000000
#define    GICCPU_ICPIIR_ID_VALUE_SHIFT                                   0
#define    GICCPU_ICPIIR_ID_VALUE_MASK                                    0xFFFFFFFF

#endif /* __BRCM_RDB_GICCPU_H__ */



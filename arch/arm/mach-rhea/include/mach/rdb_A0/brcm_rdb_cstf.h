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

#ifndef __BRCM_RDB_CSTF_H__
#define __BRCM_RDB_CSTF_H__

#define CSTF_FUNNEL_CONTROL_OFFSET                                        0x00000000
#define CSTF_FUNNEL_CONTROL_TYPE                                          UInt32
#define CSTF_FUNNEL_CONTROL_RESERVED_MASK                                 0xFFFFF000
#define    CSTF_FUNNEL_CONTROL_MINIMUM_HOLD_TIME_3_0_SHIFT                8
#define    CSTF_FUNNEL_CONTROL_MINIMUM_HOLD_TIME_3_0_MASK                 0x00000F00
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_7_SHIFT                  7
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_7_MASK                   0x00000080
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_6_SHIFT                  6
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_6_MASK                   0x00000040
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_5_SHIFT                  5
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_5_MASK                   0x00000020
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_4_SHIFT                  4
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_4_MASK                   0x00000010
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_3_SHIFT                  3
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_3_MASK                   0x00000008
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_2_SHIFT                  2
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_2_MASK                   0x00000004
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_1_SHIFT                  1
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_1_MASK                   0x00000002
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_0_SHIFT                  0
#define    CSTF_FUNNEL_CONTROL_ENABLE_SLAVE_PORT_0_MASK                   0x00000001

#define CSTF_PRIORITY_CONTROL_OFFSET                                      0x00000004
#define CSTF_PRIORITY_CONTROL_TYPE                                        UInt32
#define CSTF_PRIORITY_CONTROL_RESERVED_MASK                               0xFF000000
#define    CSTF_PRIORITY_CONTROL_PRIPORT_7_SHIFT                          21
#define    CSTF_PRIORITY_CONTROL_PRIPORT_7_MASK                           0x00E00000
#define    CSTF_PRIORITY_CONTROL_PRIPORT_6_SHIFT                          18
#define    CSTF_PRIORITY_CONTROL_PRIPORT_6_MASK                           0x001C0000
#define    CSTF_PRIORITY_CONTROL_PRIPORT_5_SHIFT                          15
#define    CSTF_PRIORITY_CONTROL_PRIPORT_5_MASK                           0x00038000
#define    CSTF_PRIORITY_CONTROL_PRIPORT_4_SHIFT                          12
#define    CSTF_PRIORITY_CONTROL_PRIPORT_4_MASK                           0x00007000
#define    CSTF_PRIORITY_CONTROL_PRIPORT_3_SHIFT                          9
#define    CSTF_PRIORITY_CONTROL_PRIPORT_3_MASK                           0x00000E00
#define    CSTF_PRIORITY_CONTROL_PRIPORT_2_SHIFT                          6
#define    CSTF_PRIORITY_CONTROL_PRIPORT_2_MASK                           0x000001C0
#define    CSTF_PRIORITY_CONTROL_PRIPORT_1_SHIFT                          3
#define    CSTF_PRIORITY_CONTROL_PRIPORT_1_MASK                           0x00000038
#define    CSTF_PRIORITY_CONTROL_PRIPORT_0_SHIFT                          0
#define    CSTF_PRIORITY_CONTROL_PRIPORT_0_MASK                           0x00000007

#define CSTF_ITATBDATA0_OFFSET                                            0x00000EEC
#define CSTF_ITATBDATA0_TYPE                                              UInt32
#define CSTF_ITATBDATA0_RESERVED_MASK                                     0xFFFFFFE0
#define    CSTF_ITATBDATA0_UNION_SHIFT                                    0
#define    CSTF_ITATBDATA0_UNION_MASK                                     0x0000001F

#define CSTF_ITATBCTR2_OFFSET                                             0x00000EF0
#define CSTF_ITATBCTR2_TYPE                                               UInt32
#define CSTF_ITATBCTR2_RESERVED_MASK                                      0xFFFFFFFC
#define    CSTF_ITATBCTR2_UNION_SHIFT                                     0
#define    CSTF_ITATBCTR2_UNION_MASK                                      0x00000003

#define CSTF_ITATBCTR1_OFFSET                                             0x00000EF4
#define CSTF_ITATBCTR1_TYPE                                               UInt32
#define CSTF_ITATBCTR1_RESERVED_MASK                                      0xFFFFFF80
#define    CSTF_ITATBCTR1_UNION_SHIFT                                     0
#define    CSTF_ITATBCTR1_UNION_MASK                                      0x0000007F

#define CSTF_ITATBCTR0_OFFSET                                             0x00000EF8
#define CSTF_ITATBCTR0_TYPE                                               UInt32
#define CSTF_ITATBCTR0_RESERVED_MASK                                      0xFFFFFC00
#define    CSTF_ITATBCTR0_UNION_SHIFT                                     0
#define    CSTF_ITATBCTR0_UNION_MASK                                      0x000003FF

#define CSTF_ITCTRL_OFFSET                                                0x00000F00
#define CSTF_ITCTRL_TYPE                                                  UInt32
#define CSTF_ITCTRL_RESERVED_MASK                                         0xFFFFFFFE
#define    CSTF_ITCTRL_MODE_SHIFT                                         0
#define    CSTF_ITCTRL_MODE_MASK                                          0x00000001

#define CSTF_CLAIM_TAG_SET_OFFSET                                         0x00000FA0
#define CSTF_CLAIM_TAG_SET_TYPE                                           UInt32
#define CSTF_CLAIM_TAG_SET_RESERVED_MASK                                  0xFFFFFFF0
#define    CSTF_CLAIM_TAG_SET_CTV_SHIFT                                   0
#define    CSTF_CLAIM_TAG_SET_CTV_MASK                                    0x0000000F

#define CSTF_CLAIM_TAG_CLEAR_OFFSET                                       0x00000FA4
#define CSTF_CLAIM_TAG_CLEAR_TYPE                                         UInt32
#define CSTF_CLAIM_TAG_CLEAR_RESERVED_MASK                                0xFFFFFFF0
#define    CSTF_CLAIM_TAG_CLEAR_CTV_SHIFT                                 0
#define    CSTF_CLAIM_TAG_CLEAR_CTV_MASK                                  0x0000000F

#define CSTF_LOCKACCESS_OFFSET                                            0x00000FB0
#define CSTF_LOCKACCESS_TYPE                                              UInt32
#define CSTF_LOCKACCESS_RESERVED_MASK                                     0x00000000
#define    CSTF_LOCKACCESS_WRITE_ACCESS_CODE_SHIFT                        0
#define    CSTF_LOCKACCESS_WRITE_ACCESS_CODE_MASK                         0xFFFFFFFF

#define CSTF_LOCKSTATUS_OFFSET                                            0x00000FB4
#define CSTF_LOCKSTATUS_TYPE                                              UInt32
#define CSTF_LOCKSTATUS_RESERVED_MASK                                     0xFFFFFFF8
#define    CSTF_LOCKSTATUS_LOCK_SIZE_SHIFT                                2
#define    CSTF_LOCKSTATUS_LOCK_SIZE_MASK                                 0x00000004
#define    CSTF_LOCKSTATUS_LOCK_STATUS_SHIFT                              1
#define    CSTF_LOCKSTATUS_LOCK_STATUS_MASK                               0x00000002
#define    CSTF_LOCKSTATUS_LOCK_MECHANISM_SHIFT                           0
#define    CSTF_LOCKSTATUS_LOCK_MECHANISM_MASK                            0x00000001

#define CSTF_AUTHSTATUS_OFFSET                                            0x00000FB8
#define CSTF_AUTHSTATUS_TYPE                                              UInt32
#define CSTF_AUTHSTATUS_RESERVED_MASK                                     0xFFFFFF00
#define    CSTF_AUTHSTATUS_SECURE_NONINVASIVE_DEBUG_SHIFT                 6
#define    CSTF_AUTHSTATUS_SECURE_NONINVASIVE_DEBUG_MASK                  0x000000C0
#define    CSTF_AUTHSTATUS_SECURE_INVASIVE_DEBUG_SHIFT                    4
#define    CSTF_AUTHSTATUS_SECURE_INVASIVE_DEBUG_MASK                     0x00000030
#define    CSTF_AUTHSTATUS_NONSECURE_NONINVASIVE_DEBUG_SHIFT              2
#define    CSTF_AUTHSTATUS_NONSECURE_NONINVASIVE_DEBUG_MASK               0x0000000C
#define    CSTF_AUTHSTATUS_NONSECURE_INVASIVE_DEBUG_SHIFT                 0
#define    CSTF_AUTHSTATUS_NONSECURE_INVASIVE_DEBUG_MASK                  0x00000003

#define CSTF_DEVICE_ID_OFFSET                                             0x00000FC8
#define CSTF_DEVICE_ID_TYPE                                               UInt32
#define CSTF_DEVICE_ID_RESERVED_MASK                                      0xFFFFFF00
#define    CSTF_DEVICE_ID_PRIORITY_SHIFT                                  4
#define    CSTF_DEVICE_ID_PRIORITY_MASK                                   0x000000F0
#define    CSTF_DEVICE_ID_PORTCOUNT_SHIFT                                 0
#define    CSTF_DEVICE_ID_PORTCOUNT_MASK                                  0x0000000F

#define CSTF_DEVICE_TYPE_IDENTIFIER_OFFSET                                0x00000FCC
#define CSTF_DEVICE_TYPE_IDENTIFIER_TYPE                                  UInt32
#define CSTF_DEVICE_TYPE_IDENTIFIER_RESERVED_MASK                         0xFFFFFF00
#define    CSTF_DEVICE_TYPE_IDENTIFIER_SUB_TYPE_SHIFT                     4
#define    CSTF_DEVICE_TYPE_IDENTIFIER_SUB_TYPE_MASK                      0x000000F0
#define    CSTF_DEVICE_TYPE_IDENTIFIER_MAIN_TYPE_SHIFT                    0
#define    CSTF_DEVICE_TYPE_IDENTIFIER_MAIN_TYPE_MASK                     0x0000000F

#define CSTF_PERIPHERAL_ID4_OFFSET                                        0x00000FD0
#define CSTF_PERIPHERAL_ID4_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID4_RESERVED_MASK                                 0xFFFFFF00
#define    CSTF_PERIPHERAL_ID4_COUNT_4KB_SHIFT                            4
#define    CSTF_PERIPHERAL_ID4_COUNT_4KB_MASK                             0x000000F0
#define    CSTF_PERIPHERAL_ID4_JEP_CONTINUATION_CODE_SHIFT                0
#define    CSTF_PERIPHERAL_ID4_JEP_CONTINUATION_CODE_MASK                 0x0000000F

#define CSTF_PERIPHERAL_ID5_OFFSET                                        0x00000FD4
#define CSTF_PERIPHERAL_ID5_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID5_RESERVED_MASK                                 0xFFFFFFFF

#define CSTF_PERIPHERAL_ID6_OFFSET                                        0x00000FD8
#define CSTF_PERIPHERAL_ID6_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID6_RESERVED_MASK                                 0xFFFFFFFF

#define CSTF_PERIPHERAL_ID7_OFFSET                                        0x00000FDC
#define CSTF_PERIPHERAL_ID7_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID7_RESERVED_MASK                                 0xFFFFFFFF

#define CSTF_PERIPHERAL_ID0_OFFSET                                        0x00000FE0
#define CSTF_PERIPHERAL_ID0_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID0_RESERVED_MASK                                 0xFFFFFF00
#define    CSTF_PERIPHERAL_ID0_PART_NUMBER_7_0_SHIFT                      0
#define    CSTF_PERIPHERAL_ID0_PART_NUMBER_7_0_MASK                       0x000000FF

#define CSTF_PERIPHERAL_ID1_OFFSET                                        0x00000FE4
#define CSTF_PERIPHERAL_ID1_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID1_RESERVED_MASK                                 0xFFFFFF00
#define    CSTF_PERIPHERAL_ID1_JEP_IDENTITY_CODE_3_0_SHIFT                4
#define    CSTF_PERIPHERAL_ID1_JEP_IDENTITY_CODE_3_0_MASK                 0x000000F0
#define    CSTF_PERIPHERAL_ID1_PART_NUMBER_1_SHIFT                        0
#define    CSTF_PERIPHERAL_ID1_PART_NUMBER_1_MASK                         0x0000000F

#define CSTF_PERIPHERAL_ID2_OFFSET                                        0x00000FE8
#define CSTF_PERIPHERAL_ID2_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID2_RESERVED_MASK                                 0xFFFFFF00
#define    CSTF_PERIPHERAL_ID2_REVISION_SHIFT                             4
#define    CSTF_PERIPHERAL_ID2_REVISION_MASK                              0x000000F0
#define    CSTF_PERIPHERAL_ID2_JEP_1_SHIFT                                3
#define    CSTF_PERIPHERAL_ID2_JEP_1_MASK                                 0x00000008
#define    CSTF_PERIPHERAL_ID2_JEP_IDENTITY_CODE_6_4_SHIFT                0
#define    CSTF_PERIPHERAL_ID2_JEP_IDENTITY_CODE_6_4_MASK                 0x00000007

#define CSTF_PERIPHERAL_ID3_OFFSET                                        0x00000FEC
#define CSTF_PERIPHERAL_ID3_TYPE                                          UInt32
#define CSTF_PERIPHERAL_ID3_RESERVED_MASK                                 0xFFFFFF00
#define    CSTF_PERIPHERAL_ID3_REVAND_SHIFT                               4
#define    CSTF_PERIPHERAL_ID3_REVAND_MASK                                0x000000F0
#define    CSTF_PERIPHERAL_ID3_CUSTOMER_MODIFIED_SHIFT                    0
#define    CSTF_PERIPHERAL_ID3_CUSTOMER_MODIFIED_MASK                     0x0000000F

#define CSTF_COMPONENT_ID0_OFFSET                                         0x00000FF0
#define CSTF_COMPONENT_ID0_TYPE                                           UInt32
#define CSTF_COMPONENT_ID0_RESERVED_MASK                                  0xFFFFFF00
#define    CSTF_COMPONENT_ID0_ID0_SHIFT                                   0
#define    CSTF_COMPONENT_ID0_ID0_MASK                                    0x000000FF

#define CSTF_COMPONENT_ID1_OFFSET                                         0x00000FF4
#define CSTF_COMPONENT_ID1_TYPE                                           UInt32
#define CSTF_COMPONENT_ID1_RESERVED_MASK                                  0xFFFFFF00
#define    CSTF_COMPONENT_ID1_COMPONENT_CLASS_SHIFT                       4
#define    CSTF_COMPONENT_ID1_COMPONENT_CLASS_MASK                        0x000000F0
#define    CSTF_COMPONENT_ID1_ID1_SHIFT                                   0
#define    CSTF_COMPONENT_ID1_ID1_MASK                                    0x0000000F

#define CSTF_COMPONENT_ID2_OFFSET                                         0x00000FF8
#define CSTF_COMPONENT_ID2_TYPE                                           UInt32
#define CSTF_COMPONENT_ID2_RESERVED_MASK                                  0xFFFFFF00
#define    CSTF_COMPONENT_ID2_ID2_SHIFT                                   0
#define    CSTF_COMPONENT_ID2_ID2_MASK                                    0x000000FF

#define CSTF_COMPONENT_ID3_OFFSET                                         0x00000FFC
#define CSTF_COMPONENT_ID3_TYPE                                           UInt32
#define CSTF_COMPONENT_ID3_RESERVED_MASK                                  0xFFFFFF00
#define    CSTF_COMPONENT_ID3_ID3_SHIFT                                   0
#define    CSTF_COMPONENT_ID3_ID3_MASK                                    0x000000FF

#endif /* __BRCM_RDB_CSTF_H__ */



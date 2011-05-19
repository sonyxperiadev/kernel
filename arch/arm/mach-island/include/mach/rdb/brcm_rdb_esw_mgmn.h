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

#ifndef __BRCM_RDB_ESW_MGMN_H__
#define __BRCM_RDB_ESW_MGMN_H__

#define ESW_MGMN_MANAGEMENT_CONFIGURATION_OFFSET                          0x00000000
#define ESW_MGMN_MANAGEMENT_CONFIGURATION_TYPE                            UInt8
#define ESW_MGMN_MANAGEMENT_CONFIGURATION_RESERVED_MASK                   0x0000002C
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_FRM_MNGT_PORT_SHIFT          6
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_FRM_MNGT_PORT_MASK           0x000000C0
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_INT_EN_SHIFT                 4
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_INT_EN_MASK                  0x00000010
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_RECEIVE_BPDU_ENABLE_SHIFT    1
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_RECEIVE_BPDU_ENABLE_MASK     0x00000002
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_RST_MIB_CNTRS_SHIFT          0
#define    ESW_MGMN_MANAGEMENT_CONFIGURATION_RST_MIB_CNTRS_MASK           0x00000001

#define ESW_MGMN_BRCM_HEADER_CONTROL_REGISTER_OFFSET                      0x00000018
#define ESW_MGMN_BRCM_HEADER_CONTROL_REGISTER_TYPE                        UInt8
#define ESW_MGMN_BRCM_HEADER_CONTROL_REGISTER_RESERVED_MASK               0x000000FC
#define    ESW_MGMN_BRCM_HEADER_CONTROL_REGISTER_BRCM_HDR_EN_SHIFT        0
#define    ESW_MGMN_BRCM_HEADER_CONTROL_REGISTER_BRCM_HDR_EN_MASK         0x00000003

#define ESW_MGMN_RMON_MIB_STEERING_0_OFFSET                               0x00000020
#define ESW_MGMN_RMON_MIB_STEERING_0_TYPE                                 UInt16
#define ESW_MGMN_RMON_MIB_STEERING_0_RESERVED_MASK                        0x0000FE00
#define    ESW_MGMN_RMON_MIB_STEERING_0_OR_RMON_RCV_SHIFT                 0
#define    ESW_MGMN_RMON_MIB_STEERING_0_OR_RMON_RCV_MASK                  0x000001FF

#define ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_OFFSET                          0x00000080
#define ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_TYPE                            UInt16
#define ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_RESERVED_MASK                   0x00003FF0
#define    ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_MIRROR_ENABLE_SHIFT          15
#define    ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_MIRROR_ENABLE_MASK           0x00008000
#define    ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_BLK_NOT_MIR_SHIFT            14
#define    ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_BLK_NOT_MIR_MASK             0x00004000
#define    ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_MIRROR_CAPTURE_PORT_ID_SHIFT 0
#define    ESW_MGMN_MIRROR_CAPTURE_CONTROL_0_MIRROR_CAPTURE_PORT_ID_MASK  0x0000000F

#define ESW_MGMN_INGRESS_MIRROR_CONTROL_0_OFFSET                          0x00000090
#define ESW_MGMN_INGRESS_MIRROR_CONTROL_0_TYPE                            UInt16
#define ESW_MGMN_INGRESS_MIRROR_CONTROL_0_RESERVED_MASK                   0x00001E00
#define    ESW_MGMN_INGRESS_MIRROR_CONTROL_0_IN_MIRROR_FILTER_SHIFT       14
#define    ESW_MGMN_INGRESS_MIRROR_CONTROL_0_IN_MIRROR_FILTER_MASK        0x0000C000
#define    ESW_MGMN_INGRESS_MIRROR_CONTROL_0_IN_DIV_EN_SHIFT              13
#define    ESW_MGMN_INGRESS_MIRROR_CONTROL_0_IN_DIV_EN_MASK               0x00002000
#define    ESW_MGMN_INGRESS_MIRROR_CONTROL_0_IN_MIRROR_MASK_SHIFT         0
#define    ESW_MGMN_INGRESS_MIRROR_CONTROL_0_IN_MIRROR_MASK_MASK          0x000001FF

#define ESW_MGMN_INGRESS_MIRROR_DIVIDER_0_OFFSET                          0x000000A0
#define ESW_MGMN_INGRESS_MIRROR_DIVIDER_0_TYPE                            UInt16
#define ESW_MGMN_INGRESS_MIRROR_DIVIDER_0_RESERVED_MASK                   0x00000000
#define    ESW_MGMN_INGRESS_MIRROR_DIVIDER_0_IN_MIRROR_DIV_SHIFT          0
#define    ESW_MGMN_INGRESS_MIRROR_DIVIDER_0_IN_MIRROR_DIV_MASK           0x0000FFFF

#define ESW_MGMN_INGRESS_MIRROR_MAC_ADDRESS_0_OFFSET                      0x000000B0
#define ESW_MGMN_INGRESS_MIRROR_MAC_ADDRESS_0_TYPE                        UInt48
#define ESW_MGMN_INGRESS_MIRROR_MAC_ADDRESS_0_RESERVED_MASK               0x00000000
#define    ESW_MGMN_INGRESS_MIRROR_MAC_ADDRESS_0_IN_MIRROR_MAC_SHIFT      0
#define    ESW_MGMN_INGRESS_MIRROR_MAC_ADDRESS_0_IN_MIRROR_MAC_MASK       0xFFFFFFFFFFFF

#define ESW_MGMN_MODEL_ID_OFFSET                                          0x00000180
#define ESW_MGMN_MODEL_ID_TYPE                                            UInt32
#define ESW_MGMN_MODEL_ID_RESERVED_MASK                                   0x00000000
#define    ESW_MGMN_MODEL_ID_MODEL_ID_SHIFT                               0
#define    ESW_MGMN_MODEL_ID_MODEL_ID_MASK                                0xFFFFFFFF

#define ESW_MGMN_REV_ID_OFFSET                                            0x00000200
#define ESW_MGMN_REV_ID_TYPE                                              UInt8
#define ESW_MGMN_REV_ID_RESERVED_MASK                                     0x00000000
#define    ESW_MGMN_REV_ID_VERSION_ID_SHIFT                               0
#define    ESW_MGMN_REV_ID_VERSION_ID_MASK                                0x000000FF

#define ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_OFFSET                       0x00000280
#define ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_TYPE                         UInt32
#define ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_RESERVED_MASK                0xFFF800C0
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_QRY_FWD_MODE_SHIFT    18
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_QRY_FWD_MODE_MASK     0x00040000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_QRY_EN_SHIFT          17
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_QRY_EN_MASK           0x00020000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_RPTDONE_FWD_MODE_SHIFT 16
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_RPTDONE_FWD_MODE_MASK 0x00010000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_RPTDONE_EN_SHIFT      15
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_MLD_RPTDONE_EN_MASK       0x00008000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_UKN_FWD_MODE_SHIFT   14
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_UKN_FWD_MODE_MASK    0x00004000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_UKN_EN_SHIFT         13
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_UKN_EN_MASK          0x00002000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_QRY_FWD_MODE_SHIFT   12
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_QRY_FWD_MODE_MASK    0x00001000
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_QRY_EN_SHIFT         11
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_QRY_EN_MASK          0x00000800
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_RPTLVE_FWD_MODE_SHIFT 10
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_RPTLVE_FWD_MODE_MASK 0x00000400
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_RPTLVE_EN_SHIFT      9
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_RPTLVE_EN_MASK       0x00000200
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_DIP_EN_SHIFT         8
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_IGMP_DIP_EN_MASK          0x00000100
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ICMPV6_FWD_MODE_SHIFT     5
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ICMPV6_FWD_MODE_MASK      0x00000020
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ICMPV6_EN_SHIFT           4
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ICMPV6_EN_MASK            0x00000010
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ICMPV4_EN_SHIFT           3
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ICMPV4_EN_MASK            0x00000008
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_DHCP_EN_SHIFT             2
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_DHCP_EN_MASK              0x00000004
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_RARP_EN_SHIFT             1
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_RARP_EN_MASK              0x00000002
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ARP_EN_SHIFT              0
#define    ESW_MGMN_HIGH_LEVEL_PROTOCOL_CONTROL_ARP_EN_MASK               0x00000001

#endif /* __BRCM_RDB_ESW_MGMN_H__ */



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

#ifndef __BRCM_RDB_EAP_REGISTER_H__
#define __BRCM_RDB_EAP_REGISTER_H__

#define EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_OFFSET                   0x00000000
#define EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_TYPE                     UInt8
#define EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_RESERVED_MASK            0x00000081
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_RARP_SHIFT         6
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_RARP_MASK          0x00000040
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_BPDU_SHIFT         5
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_BPDU_MASK          0x00000020
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_RMC_SHIFT          4
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_RMC_MASK           0x00000010
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_DHCP_SHIFT         3
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_DHCP_MASK          0x00000008
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_ARP_SHIFT          2
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_ARP_MASK           0x00000004
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_2DIP_SHIFT         1
#define    EAP_REGISTER_EAP_GLOBAL_CONTROL_REGISTER_EN_2DIP_MASK          0x00000002

#define EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_OFFSET        0x00000008
#define EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_TYPE          UInt8
#define EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_RESERVED_MASK 0x000000C0
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT5_SHIFT 5
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT5_MASK 0x00000020
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT4_SHIFT 4
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT4_MASK 0x00000010
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT3_SHIFT 3
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT3_MASK 0x00000008
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT2_SHIFT 2
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT2_MASK 0x00000004
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT1_SHIFT 1
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT1_MASK 0x00000002
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT0_SHIFT 0
#define    EAP_REGISTER_EAP_MULTIPORT_ADDRESS_CONTROL_REGISTER_EN_MPORT0_MASK 0x00000001

#define EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_OFFSET                 0x00000010
#define EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_TYPE                   UInt64
#define EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_RESERVED_MASK          0x00000000
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_DIP_SUB0_SHIFT      32
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_DIP_SUB0_MASK       0xFFFFFFFF00000000
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_DIP_MSK0_SHIFT      0
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_0_DIP_MSK0_MASK       0xFFFFFFFF

#define EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_OFFSET                 0x00000050
#define EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_TYPE                   UInt64
#define EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_RESERVED_MASK          0x00000000
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_DIP_SUB1_SHIFT      32
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_DIP_SUB1_MASK       0xFFFFFFFF00000000
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_DIP_MSK1_SHIFT      0
#define    EAP_REGISTER_EAP_DESTINATION_IP_REGISTER_1_DIP_MSK1_MASK       0xFFFFFFFF

#define EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_OFFSET             0x00000100
#define EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_TYPE               UInt64
#define EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_RESERVED_MASK      0xFFE0000000000000
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_MODE_SHIFT  51
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_MODE_MASK   0x18000000000000
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_BLK_MODE_SHIFT 49
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_BLK_MODE_MASK 0x6000000000000
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_EN_DA_SHIFT 48
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_EN_DA_MASK  0x1000000000000
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_DA_SHIFT    0
#define    EAP_REGISTER_PORT_0_EAP_CONFIGURATION_REGISTER_EAP_DA_MASK     0xFFFFFFFFFFFF

#define EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_OFFSET             0x00000140
#define EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_TYPE               UInt64
#define EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_RESERVED_MASK      0xFFE0000000000000
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_MODE_SHIFT  51
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_MODE_MASK   0x18000000000000
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_BLK_MODE_SHIFT 49
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_BLK_MODE_MASK 0x6000000000000
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_EN_DA_SHIFT 48
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_EN_DA_MASK  0x1000000000000
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_DA_SHIFT    0
#define    EAP_REGISTER_PORT_1_EAP_CONFIGURATION_REGISTER_EAP_DA_MASK     0xFFFFFFFFFFFF

#endif /* __BRCM_RDB_EAP_REGISTER_H__ */



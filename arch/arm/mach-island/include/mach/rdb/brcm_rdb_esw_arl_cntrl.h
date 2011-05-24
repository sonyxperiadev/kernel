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

#ifndef __BRCM_RDB_ESW_ARL_CNTRL_H__
#define __BRCM_RDB_ESW_ARL_CNTRL_H__

#define ESW_ARL_CNTRL_ARL_CONFIGURATION_OFFSET                            0x00000000
#define ESW_ARL_CNTRL_ARL_CONFIGURATION_TYPE                              UInt8
#define ESW_ARL_CNTRL_ARL_CONFIGURATION_RESERVED_MASK                     0x000000FA
#define    ESW_ARL_CNTRL_ARL_CONFIGURATION_AGE_ACCELERATE_SHIFT           2
#define    ESW_ARL_CNTRL_ARL_CONFIGURATION_AGE_ACCELERATE_MASK            0x00000004
#define    ESW_ARL_CNTRL_ARL_CONFIGURATION_HASH_DISABLE_SHIFT             0
#define    ESW_ARL_CNTRL_ARL_CONFIGURATION_HASH_DISABLE_MASK              0x00000001

#define ESW_ARL_CNTRL_BPDU_MULTICAST_ADDRESS_OFFSET                       0x00000020
#define ESW_ARL_CNTRL_BPDU_MULTICAST_ADDRESS_TYPE                         UInt48
#define ESW_ARL_CNTRL_BPDU_MULTICAST_ADDRESS_RESERVED_MASK                0x00000000
#define    ESW_ARL_CNTRL_BPDU_MULTICAST_ADDRESS_BPDU_MC_ADDR_SHIFT        0
#define    ESW_ARL_CNTRL_BPDU_MULTICAST_ADDRESS_BPDU_MC_ADDR_MASK         0xFFFFFFFFFFFF

#define ESW_ARL_CNTRL_MULTIPORT_CONTROL_OFFSET                            0x00000070
#define ESW_ARL_CNTRL_MULTIPORT_CONTROL_TYPE                              UInt16
#define ESW_ARL_CNTRL_MULTIPORT_CONTROL_RESERVED_MASK                     0x00007000
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT0_TS_EN_SHIFT             15
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT0_TS_EN_MASK              0x00008000
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL5_SHIFT              10
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL5_MASK               0x00000C00
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL4_SHIFT              8
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL4_MASK               0x00000300
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL3_SHIFT              6
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL3_MASK               0x000000C0
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL2_SHIFT              4
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL2_MASK               0x00000030
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL1_SHIFT              2
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL1_MASK               0x0000000C
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL0_SHIFT              0
#define    ESW_ARL_CNTRL_MULTIPORT_CONTROL_MPORT_CTRL0_MASK               0x00000003

#define ESW_ARL_CNTRL_MULTIPORT_ADDRESS_OFFSET                            0x00000080
#define ESW_ARL_CNTRL_MULTIPORT_ADDRESS_TYPE                              UInt64
#define ESW_ARL_CNTRL_MULTIPORT_ADDRESS_RESERVED_MASK                     0x00000000
#define    ESW_ARL_CNTRL_MULTIPORT_ADDRESS_MPORT_E_TYPE_N_SHIFT           48
#define    ESW_ARL_CNTRL_MULTIPORT_ADDRESS_MPORT_E_TYPE_N_MASK            0xFFFF000000000000
#define    ESW_ARL_CNTRL_MULTIPORT_ADDRESS_MPORT_ADDR_N_SHIFT             0
#define    ESW_ARL_CNTRL_MULTIPORT_ADDRESS_MPORT_ADDR_N_MASK              0xFFFFFFFFFFFF

#define ESW_ARL_CNTRL_MULTIPORT_VECTOR_OFFSET                             0x000000C0
#define ESW_ARL_CNTRL_MULTIPORT_VECTOR_TYPE                               UInt32
#define ESW_ARL_CNTRL_MULTIPORT_VECTOR_RESERVED_MASK                      0xFFFFFE00
#define    ESW_ARL_CNTRL_MULTIPORT_VECTOR_MPORT_VCTR_N_SHIFT              0
#define    ESW_ARL_CNTRL_MULTIPORT_VECTOR_MPORT_VCTR_N_MASK               0x000001FF

#endif /* __BRCM_RDB_ESW_ARL_CNTRL_H__ */



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
/*     Date     : Generated on 2/10/2012 15:44:22                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_HARQ_H__
#define __BRCM_RDB_HARQ_H__

#define HARQ_CONFIG_OFFSET                                                0x00000000
#define HARQ_CONFIG_TYPE                                                  UInt32
#define HARQ_CONFIG_RESERVED_MASK                                         0x00030000
#define    HARQ_CONFIG_PARAM_TURBO_AUTOTRIGGER_SHIFT                      31
#define    HARQ_CONFIG_PARAM_TURBO_AUTOTRIGGER_MASK                       0x80000000
#define    HARQ_CONFIG_PARAM_CONSTELLATION_VER_SHIFT                      29
#define    HARQ_CONFIG_PARAM_CONSTELLATION_VER_MASK                       0x60000000
#define    HARQ_CONFIG_PARAM_QAM_SHIFT                                    28
#define    HARQ_CONFIG_PARAM_QAM_MASK                                     0x10000000
#define    HARQ_CONFIG_PARAM_NUMPHCH_SHIFT                                24
#define    HARQ_CONFIG_PARAM_NUMPHCH_MASK                                 0x0F000000
#define    HARQ_CONFIG_PARAM_GEN_INT_SHIFT                                23
#define    HARQ_CONFIG_PARAM_GEN_INT_MASK                                 0x00800000
#define    HARQ_CONFIG_PARAM_IR_CHASE_SHIFT                               22
#define    HARQ_CONFIG_PARAM_IR_CHASE_MASK                                0x00400000
#define    HARQ_CONFIG_PARAM_PAR_RM_SHIFT                                 20
#define    HARQ_CONFIG_PARAM_PAR_RM_MASK                                  0x00300000
#define    HARQ_CONFIG_PARAM_SYS_RM_SHIFT                                 18
#define    HARQ_CONFIG_PARAM_SYS_RM_MASK                                  0x000C0000
#define    HARQ_CONFIG_PARAM_NR_SHIFT                                     13
#define    HARQ_CONFIG_PARAM_NR_MASK                                      0x0000E000
#define    HARQ_CONFIG_PARAM_NC_SHIFT                                     0
#define    HARQ_CONFIG_PARAM_NC_MASK                                      0x00001FFF

#define HARQ_SYSPAR1PNTR_OFFSET                                           0x00000004
#define HARQ_SYSPAR1PNTR_TYPE                                             UInt32
#define HARQ_SYSPAR1PNTR_RESERVED_MASK                                    0xC000C000
#define    HARQ_SYSPAR1PNTR_PARAM_PARITY1_BASE_SHIFT                      16
#define    HARQ_SYSPAR1PNTR_PARAM_PARITY1_BASE_MASK                       0x3FFF0000
#define    HARQ_SYSPAR1PNTR_PARAM_SYSTEMATIC_BASE_SHIFT                   0
#define    HARQ_SYSPAR1PNTR_PARAM_SYSTEMATIC_BASE_MASK                    0x00003FFF

#define HARQ_PAR2PNTR_OFFSET                                              0x00000008
#define HARQ_PAR2PNTR_TYPE                                                UInt32
#define HARQ_PAR2PNTR_RESERVED_MASK                                       0xFFFFC000
#define    HARQ_PAR2PNTR_PARAM_PARITY2_BASE_SHIFT                         0
#define    HARQ_PAR2PNTR_PARAM_PARITY2_BASE_MASK                          0x00003FFF

#define HARQ_RM2SYS_OFFSET                                                0x0000000C
#define HARQ_RM2SYS_TYPE                                                  UInt32
#define HARQ_RM2SYS_RESERVED_MASK                                         0x00000000
#define    HARQ_RM2SYS_PARAM_SYSTEMATIC_GAMMA_SHIFT                       16
#define    HARQ_RM2SYS_PARAM_SYSTEMATIC_GAMMA_MASK                        0xFFFF0000
#define    HARQ_RM2SYS_PARAM_SYSTEMATIC_OFFSET_SHIFT                      0
#define    HARQ_RM2SYS_PARAM_SYSTEMATIC_OFFSET_MASK                       0x0000FFFF

#define HARQ_RM2EPLUSPAR1_OFFSET                                          0x00000010
#define HARQ_RM2EPLUSPAR1_TYPE                                            UInt32
#define HARQ_RM2EPLUSPAR1_RESERVED_MASK                                   0x00000000
#define    HARQ_RM2EPLUSPAR1_PARAM_SYSTEMATIC_EPLUS_SHIFT                 16
#define    HARQ_RM2EPLUSPAR1_PARAM_SYSTEMATIC_EPLUS_MASK                  0xFFFF0000
#define    HARQ_RM2EPLUSPAR1_PARAM_PARITY1_EPLUS_SHIFT                    0
#define    HARQ_RM2EPLUSPAR1_PARAM_PARITY1_EPLUS_MASK                     0x0000FFFF

#define HARQ_RM2PAR1_OFFSET                                               0x00000014
#define HARQ_RM2PAR1_TYPE                                                 UInt32
#define HARQ_RM2PAR1_RESERVED_MASK                                        0x00000000
#define    HARQ_RM2PAR1_PARAM_PARITY1_GAMMA_SHIFT                         16
#define    HARQ_RM2PAR1_PARAM_PARITY1_GAMMA_MASK                          0xFFFF0000
#define    HARQ_RM2PAR1_PARAM_PARITY1_OFFSET_SHIFT                        0
#define    HARQ_RM2PAR1_PARAM_PARITY1_OFFSET_MASK                         0x0000FFFF

#define HARQ_RM2PAR2_OFFSET                                               0x00000018
#define HARQ_RM2PAR2_TYPE                                                 UInt32
#define HARQ_RM2PAR2_RESERVED_MASK                                        0x00000000
#define    HARQ_RM2PAR2_PARAM_PARITY2_GAMMA_SHIFT                         16
#define    HARQ_RM2PAR2_PARAM_PARITY2_GAMMA_MASK                          0xFFFF0000
#define    HARQ_RM2PAR2_PARAM_PARITY2_OFFSET_SHIFT                        0
#define    HARQ_RM2PAR2_PARAM_PARITY2_OFFSET_MASK                         0x0000FFFF

#define HARQ_RM2EPLUSPAR2_OFFSET                                          0x0000001C
#define HARQ_RM2EPLUSPAR2_TYPE                                            UInt32
#define HARQ_RM2EPLUSPAR2_RESERVED_MASK                                   0x40000000
#define    HARQ_RM2EPLUSPAR2_PARAM_CLEAR_ONLY_SHIFT                       31
#define    HARQ_RM2EPLUSPAR2_PARAM_CLEAR_ONLY_MASK                        0x80000000
#define    HARQ_RM2EPLUSPAR2_PARAM_CLEAR_LENGTH_SHIFT                     16
#define    HARQ_RM2EPLUSPAR2_PARAM_CLEAR_LENGTH_MASK                      0x3FFF0000
#define    HARQ_RM2EPLUSPAR2_PARAM_PARITY2_EPLUS_SHIFT                    0
#define    HARQ_RM2EPLUSPAR2_PARAM_PARITY2_EPLUS_MASK                     0x0000FFFF

#define HARQ_STATUS_OFFSET                                                0x0000002C
#define HARQ_STATUS_TYPE                                                  UInt32
#define HARQ_STATUS_RESERVED_MASK                                         0xFFFFFFFE
#define    HARQ_STATUS_HARQ_BUSY_SHIFT                                    0
#define    HARQ_STATUS_HARQ_BUSY_MASK                                     0x00000001

#endif /* __BRCM_RDB_HARQ_H__ */



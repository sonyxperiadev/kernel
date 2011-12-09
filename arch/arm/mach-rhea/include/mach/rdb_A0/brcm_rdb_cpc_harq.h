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

#ifndef __BRCM_RDB_CPC_HARQ_H__
#define __BRCM_RDB_CPC_HARQ_H__

#define CPC_HARQ_PS0_CONFIG_OFFSET                                        0x00000000
#define CPC_HARQ_PS0_CONFIG_TYPE                                          UInt32
#define CPC_HARQ_PS0_CONFIG_RESERVED_MASK                                 0x00FE0000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_HTDM_TRIGGER_SHIFT               31
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_HTDM_TRIGGER_MASK                0x80000000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_GEN_INTERRUPT_SHIFT              30
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_GEN_INTERRUPT_MASK               0x40000000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_CONT_NEXT_SHIFT                  29
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_CONT_NEXT_MASK                   0x20000000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_WAIT_HTDM_SHIFT                  28
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_WAIT_HTDM_MASK                   0x10000000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_CIRCREADINDEX_SHIFT              24
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_CIRCREADINDEX_MASK               0x0F000000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_IRCOMBINE_SHIFT                  16
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_IRCOMBINE_MASK                   0x00010000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_RM2_P1P2_MODE_SHIFT              14
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_RM2_P1P2_MODE_MASK               0x0000C000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_RM2_SYS_MODE_SHIFT               12
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_RM2_SYS_MODE_MASK                0x00003000
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_BITCOLLECT_NR_SHIFT              10
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_BITCOLLECT_NR_MASK               0x00000C00
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_BITCOLLECT_NC_SHIFT              0
#define    CPC_HARQ_PS0_CONFIG_PARAM_PS0_BITCOLLECT_NC_MASK               0x000003FF

#define CPC_HARQ_PS0_SYS_RATEMATCH1_OFFSET                                0x00000004
#define CPC_HARQ_PS0_SYS_RATEMATCH1_TYPE                                  UInt32
#define CPC_HARQ_PS0_SYS_RATEMATCH1_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS0_SYS_RATEMATCH1_PARAM_PS0_RM2_SYS_EINI_SHIFT       16
#define    CPC_HARQ_PS0_SYS_RATEMATCH1_PARAM_PS0_RM2_SYS_EINI_MASK        0xFFFF0000
#define    CPC_HARQ_PS0_SYS_RATEMATCH1_PARAM_PS0_RM2_SYS_EPLUS_SHIFT      0
#define    CPC_HARQ_PS0_SYS_RATEMATCH1_PARAM_PS0_RM2_SYS_EPLUS_MASK       0x0000FFFF

#define CPC_HARQ_PS0_SYS_RATEMATCH2_OFFSET                                0x00000008
#define CPC_HARQ_PS0_SYS_RATEMATCH2_TYPE                                  UInt32
#define CPC_HARQ_PS0_SYS_RATEMATCH2_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS0_SYS_RATEMATCH2_PARAM_PS0_RM2_SYS_EMINUS_SHIFT     16
#define    CPC_HARQ_PS0_SYS_RATEMATCH2_PARAM_PS0_RM2_SYS_EMINUS_MASK      0xFFFF0000
#define    CPC_HARQ_PS0_SYS_RATEMATCH2_PARAM_PS0_RM2_SYS_XI_SHIFT         0
#define    CPC_HARQ_PS0_SYS_RATEMATCH2_PARAM_PS0_RM2_SYS_XI_MASK          0x0000FFFF

#define CPC_HARQ_PS0_P1_RATEMATCH1_OFFSET                                 0x0000000C
#define CPC_HARQ_PS0_P1_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS0_P1_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS0_P1_RATEMATCH1_PARAM_PS0_RM2_P1_EINI_SHIFT         16
#define    CPC_HARQ_PS0_P1_RATEMATCH1_PARAM_PS0_RM2_P1_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS0_P1_RATEMATCH1_PARAM_PS0_RM2_P1_EPLUS_SHIFT        0
#define    CPC_HARQ_PS0_P1_RATEMATCH1_PARAM_PS0_RM2_P1_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS0_P1_RATEMATCH2_OFFSET                                 0x00000010
#define CPC_HARQ_PS0_P1_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS0_P1_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS0_P1_RATEMATCH2_PARAM_PS0_RM2_P1_EMINUS_SHIFT       16
#define    CPC_HARQ_PS0_P1_RATEMATCH2_PARAM_PS0_RM2_P1_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS0_P1_RATEMATCH2_PARAM_PS0_RM2_P1_XI_SHIFT           0
#define    CPC_HARQ_PS0_P1_RATEMATCH2_PARAM_PS0_RM2_P1_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS0_P2_RATEMATCH1_OFFSET                                 0x00000014
#define CPC_HARQ_PS0_P2_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS0_P2_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS0_P2_RATEMATCH1_PARAM_PS0_RM2_P2_EINI_SHIFT         16
#define    CPC_HARQ_PS0_P2_RATEMATCH1_PARAM_PS0_RM2_P2_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS0_P2_RATEMATCH1_PARAM_PS0_RM2_P2_EPLUS_SHIFT        0
#define    CPC_HARQ_PS0_P2_RATEMATCH1_PARAM_PS0_RM2_P2_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS0_P2_RATEMATCH2_OFFSET                                 0x00000018
#define CPC_HARQ_PS0_P2_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS0_P2_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS0_P2_RATEMATCH2_PARAM_PS0_RM2_P2_EMINUS_SHIFT       16
#define    CPC_HARQ_PS0_P2_RATEMATCH2_PARAM_PS0_RM2_P2_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS0_P2_RATEMATCH2_PARAM_PS0_RM2_P2_XI_SHIFT           0
#define    CPC_HARQ_PS0_P2_RATEMATCH2_PARAM_PS0_RM2_P2_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS0_IR_PTR1_OFFSET                                       0x0000001C
#define CPC_HARQ_PS0_IR_PTR1_TYPE                                         UInt32
#define CPC_HARQ_PS0_IR_PTR1_RESERVED_MASK                                0xF800F800
#define    CPC_HARQ_PS0_IR_PTR1_PARAM_PS0_SYS_IR_BASE_SHIFT               16
#define    CPC_HARQ_PS0_IR_PTR1_PARAM_PS0_SYS_IR_BASE_MASK                0x07FF0000
#define    CPC_HARQ_PS0_IR_PTR1_PARAM_PS0_P1_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS0_IR_PTR1_PARAM_PS0_P1_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS0_IR_PTR2_OFFSET                                       0x00000020
#define CPC_HARQ_PS0_IR_PTR2_TYPE                                         UInt32
#define CPC_HARQ_PS0_IR_PTR2_RESERVED_MASK                                0xFFFFF800
#define    CPC_HARQ_PS0_IR_PTR2_PARAM_PS0_P2_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS0_IR_PTR2_PARAM_PS0_P2_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS1_CONFIG_OFFSET                                        0x00000024
#define CPC_HARQ_PS1_CONFIG_TYPE                                          UInt32
#define CPC_HARQ_PS1_CONFIG_RESERVED_MASK                                 0x00FE0000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_HTDM_TRIGGER_SHIFT               31
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_HTDM_TRIGGER_MASK                0x80000000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_GEN_INTERRUPT_SHIFT              30
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_GEN_INTERRUPT_MASK               0x40000000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_CONT_NEXT_SHIFT                  29
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_CONT_NEXT_MASK                   0x20000000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_WAIT_HTDM_SHIFT                  28
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_WAIT_HTDM_MASK                   0x10000000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_CIRCREADINDEX_SHIFT              24
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_CIRCREADINDEX_MASK               0x0F000000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_IRCOMBINE_SHIFT                  16
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_IRCOMBINE_MASK                   0x00010000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_RM2_P1P2_MODE_SHIFT              14
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_RM2_P1P2_MODE_MASK               0x0000C000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_RM2_SYS_MODE_SHIFT               12
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_RM2_SYS_MODE_MASK                0x00003000
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_BITCOLLECT_NR_SHIFT              10
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_BITCOLLECT_NR_MASK               0x00000C00
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_BITCOLLECT_NC_SHIFT              0
#define    CPC_HARQ_PS1_CONFIG_PARAM_PS1_BITCOLLECT_NC_MASK               0x000003FF

#define CPC_HARQ_PS1_SYS_RATEMATCH1_OFFSET                                0x00000028
#define CPC_HARQ_PS1_SYS_RATEMATCH1_TYPE                                  UInt32
#define CPC_HARQ_PS1_SYS_RATEMATCH1_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS1_SYS_RATEMATCH1_PARAM_PS1_RM2_SYS_EINI_SHIFT       16
#define    CPC_HARQ_PS1_SYS_RATEMATCH1_PARAM_PS1_RM2_SYS_EINI_MASK        0xFFFF0000
#define    CPC_HARQ_PS1_SYS_RATEMATCH1_PARAM_PS1_RM2_SYS_EPLUS_SHIFT      0
#define    CPC_HARQ_PS1_SYS_RATEMATCH1_PARAM_PS1_RM2_SYS_EPLUS_MASK       0x0000FFFF

#define CPC_HARQ_PS1_SYS_RATEMATCH2_OFFSET                                0x0000002C
#define CPC_HARQ_PS1_SYS_RATEMATCH2_TYPE                                  UInt32
#define CPC_HARQ_PS1_SYS_RATEMATCH2_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS1_SYS_RATEMATCH2_PARAM_PS1_RM2_SYS_EMINUS_SHIFT     16
#define    CPC_HARQ_PS1_SYS_RATEMATCH2_PARAM_PS1_RM2_SYS_EMINUS_MASK      0xFFFF0000
#define    CPC_HARQ_PS1_SYS_RATEMATCH2_PARAM_PS1_RM2_SYS_XI_SHIFT         0
#define    CPC_HARQ_PS1_SYS_RATEMATCH2_PARAM_PS1_RM2_SYS_XI_MASK          0x0000FFFF

#define CPC_HARQ_PS1_P1_RATEMATCH1_OFFSET                                 0x00000030
#define CPC_HARQ_PS1_P1_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS1_P1_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS1_P1_RATEMATCH1_PARAM_PS1_RM2_P1_EINI_SHIFT         16
#define    CPC_HARQ_PS1_P1_RATEMATCH1_PARAM_PS1_RM2_P1_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS1_P1_RATEMATCH1_PARAM_PS1_RM2_P1_EPLUS_SHIFT        0
#define    CPC_HARQ_PS1_P1_RATEMATCH1_PARAM_PS1_RM2_P1_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS1_P1_RATEMATCH2_OFFSET                                 0x00000034
#define CPC_HARQ_PS1_P1_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS1_P1_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS1_P1_RATEMATCH2_PARAM_PS1_RM2_P1_EMINUS_SHIFT       16
#define    CPC_HARQ_PS1_P1_RATEMATCH2_PARAM_PS1_RM2_P1_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS1_P1_RATEMATCH2_PARAM_PS1_RM2_P1_XI_SHIFT           0
#define    CPC_HARQ_PS1_P1_RATEMATCH2_PARAM_PS1_RM2_P1_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS1_P2_RATEMATCH1_OFFSET                                 0x00000038
#define CPC_HARQ_PS1_P2_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS1_P2_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS1_P2_RATEMATCH1_PARAM_PS1_RM2_P2_EINI_SHIFT         16
#define    CPC_HARQ_PS1_P2_RATEMATCH1_PARAM_PS1_RM2_P2_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS1_P2_RATEMATCH1_PARAM_PS1_RM2_P2_EPLUS_SHIFT        0
#define    CPC_HARQ_PS1_P2_RATEMATCH1_PARAM_PS1_RM2_P2_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS1_P2_RATEMATCH2_OFFSET                                 0x0000003C
#define CPC_HARQ_PS1_P2_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS1_P2_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS1_P2_RATEMATCH2_PARAM_PS1_RM2_P2_EMINUS_SHIFT       16
#define    CPC_HARQ_PS1_P2_RATEMATCH2_PARAM_PS1_RM2_P2_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS1_P2_RATEMATCH2_PARAM_PS1_RM2_P2_XI_SHIFT           0
#define    CPC_HARQ_PS1_P2_RATEMATCH2_PARAM_PS1_RM2_P2_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS1_IR_PTR1_OFFSET                                       0x00000040
#define CPC_HARQ_PS1_IR_PTR1_TYPE                                         UInt32
#define CPC_HARQ_PS1_IR_PTR1_RESERVED_MASK                                0xF800F800
#define    CPC_HARQ_PS1_IR_PTR1_PARAM_PS1_SYS_IR_BASE_SHIFT               16
#define    CPC_HARQ_PS1_IR_PTR1_PARAM_PS1_SYS_IR_BASE_MASK                0x07FF0000
#define    CPC_HARQ_PS1_IR_PTR1_PARAM_PS1_P1_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS1_IR_PTR1_PARAM_PS1_P1_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS1_IR_PTR2_OFFSET                                       0x00000044
#define CPC_HARQ_PS1_IR_PTR2_TYPE                                         UInt32
#define CPC_HARQ_PS1_IR_PTR2_RESERVED_MASK                                0xFFFFF800
#define    CPC_HARQ_PS1_IR_PTR2_PARAM_PS1_P2_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS1_IR_PTR2_PARAM_PS1_P2_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS2_CONFIG_OFFSET                                        0x00000048
#define CPC_HARQ_PS2_CONFIG_TYPE                                          UInt32
#define CPC_HARQ_PS2_CONFIG_RESERVED_MASK                                 0x00FE0000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_HTDM_TRIGGER_SHIFT               31
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_HTDM_TRIGGER_MASK                0x80000000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_GEN_INTERRUPT_SHIFT              30
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_GEN_INTERRUPT_MASK               0x40000000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_CONT_NEXT_SHIFT                  29
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_CONT_NEXT_MASK                   0x20000000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_WAIT_HTDM_SHIFT                  28
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_WAIT_HTDM_MASK                   0x10000000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_CIRCREADINDEX_SHIFT              24
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_CIRCREADINDEX_MASK               0x0F000000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_IRCOMBINE_SHIFT                  16
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_IRCOMBINE_MASK                   0x00010000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_RM2_P1P2_MODE_SHIFT              14
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_RM2_P1P2_MODE_MASK               0x0000C000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_RM2_SYS_MODE_SHIFT               12
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_RM2_SYS_MODE_MASK                0x00003000
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_BITCOLLECT_NR_SHIFT              10
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_BITCOLLECT_NR_MASK               0x00000C00
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_BITCOLLECT_NC_SHIFT              0
#define    CPC_HARQ_PS2_CONFIG_PARAM_PS2_BITCOLLECT_NC_MASK               0x000003FF

#define CPC_HARQ_PS2_SYS_RATEMATCH1_OFFSET                                0x0000004C
#define CPC_HARQ_PS2_SYS_RATEMATCH1_TYPE                                  UInt32
#define CPC_HARQ_PS2_SYS_RATEMATCH1_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS2_SYS_RATEMATCH1_PARAM_PS2_RM2_SYS_EINI_SHIFT       16
#define    CPC_HARQ_PS2_SYS_RATEMATCH1_PARAM_PS2_RM2_SYS_EINI_MASK        0xFFFF0000
#define    CPC_HARQ_PS2_SYS_RATEMATCH1_PARAM_PS2_RM2_SYS_EPLUS_SHIFT      0
#define    CPC_HARQ_PS2_SYS_RATEMATCH1_PARAM_PS2_RM2_SYS_EPLUS_MASK       0x0000FFFF

#define CPC_HARQ_PS2_SYS_RATEMATCH2_OFFSET                                0x00000050
#define CPC_HARQ_PS2_SYS_RATEMATCH2_TYPE                                  UInt32
#define CPC_HARQ_PS2_SYS_RATEMATCH2_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS2_SYS_RATEMATCH2_PARAM_PS2_RM2_SYS_EMINUS_SHIFT     16
#define    CPC_HARQ_PS2_SYS_RATEMATCH2_PARAM_PS2_RM2_SYS_EMINUS_MASK      0xFFFF0000
#define    CPC_HARQ_PS2_SYS_RATEMATCH2_PARAM_PS2_RM2_SYS_XI_SHIFT         0
#define    CPC_HARQ_PS2_SYS_RATEMATCH2_PARAM_PS2_RM2_SYS_XI_MASK          0x0000FFFF

#define CPC_HARQ_PS2_P1_RATEMATCH1_OFFSET                                 0x00000054
#define CPC_HARQ_PS2_P1_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS2_P1_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS2_P1_RATEMATCH1_PARAM_PS2_RM2_P1_EINI_SHIFT         16
#define    CPC_HARQ_PS2_P1_RATEMATCH1_PARAM_PS2_RM2_P1_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS2_P1_RATEMATCH1_PARAM_PS2_RM2_P1_EPLUS_SHIFT        0
#define    CPC_HARQ_PS2_P1_RATEMATCH1_PARAM_PS2_RM2_P1_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS2_P1_RATEMATCH2_OFFSET                                 0x00000058
#define CPC_HARQ_PS2_P1_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS2_P1_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS2_P1_RATEMATCH2_PARAM_PS2_RM2_P1_EMINUS_SHIFT       16
#define    CPC_HARQ_PS2_P1_RATEMATCH2_PARAM_PS2_RM2_P1_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS2_P1_RATEMATCH2_PARAM_PS2_RM2_P1_XI_SHIFT           0
#define    CPC_HARQ_PS2_P1_RATEMATCH2_PARAM_PS2_RM2_P1_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS2_P2_RATEMATCH1_OFFSET                                 0x0000005C
#define CPC_HARQ_PS2_P2_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS2_P2_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS2_P2_RATEMATCH1_PARAM_PS2_RM2_P2_EINI_SHIFT         16
#define    CPC_HARQ_PS2_P2_RATEMATCH1_PARAM_PS2_RM2_P2_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS2_P2_RATEMATCH1_PARAM_PS2_RM2_P2_EPLUS_SHIFT        0
#define    CPC_HARQ_PS2_P2_RATEMATCH1_PARAM_PS2_RM2_P2_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS2_P2_RATEMATCH2_OFFSET                                 0x00000060
#define CPC_HARQ_PS2_P2_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS2_P2_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS2_P2_RATEMATCH2_PARAM_PS2_RM2_P2_EMINUS_SHIFT       16
#define    CPC_HARQ_PS2_P2_RATEMATCH2_PARAM_PS2_RM2_P2_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS2_P2_RATEMATCH2_PARAM_PS2_RM2_P2_XI_SHIFT           0
#define    CPC_HARQ_PS2_P2_RATEMATCH2_PARAM_PS2_RM2_P2_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS2_IR_PTR1_OFFSET                                       0x00000064
#define CPC_HARQ_PS2_IR_PTR1_TYPE                                         UInt32
#define CPC_HARQ_PS2_IR_PTR1_RESERVED_MASK                                0xF800F800
#define    CPC_HARQ_PS2_IR_PTR1_PARAM_PS2_SYS_IR_BASE_SHIFT               16
#define    CPC_HARQ_PS2_IR_PTR1_PARAM_PS2_SYS_IR_BASE_MASK                0x07FF0000
#define    CPC_HARQ_PS2_IR_PTR1_PARAM_PS2_P1_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS2_IR_PTR1_PARAM_PS2_P1_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS2_IR_PTR2_OFFSET                                       0x00000068
#define CPC_HARQ_PS2_IR_PTR2_TYPE                                         UInt32
#define CPC_HARQ_PS2_IR_PTR2_RESERVED_MASK                                0xFFFFF800
#define    CPC_HARQ_PS2_IR_PTR2_PARAM_PS2_P2_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS2_IR_PTR2_PARAM_PS2_P2_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS3_CONFIG_OFFSET                                        0x0000006C
#define CPC_HARQ_PS3_CONFIG_TYPE                                          UInt32
#define CPC_HARQ_PS3_CONFIG_RESERVED_MASK                                 0x30FE0000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_HTDM_TRIGGER_SHIFT               31
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_HTDM_TRIGGER_MASK                0x80000000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_GEN_INTERRUPT_SHIFT              30
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_GEN_INTERRUPT_MASK               0x40000000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_CIRCREADINDEX_SHIFT              24
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_CIRCREADINDEX_MASK               0x0F000000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_IRCOMBINE_SHIFT                  16
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_IRCOMBINE_MASK                   0x00010000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_RM2_P1P2_MODE_SHIFT              14
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_RM2_P1P2_MODE_MASK               0x0000C000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_RM2_SYS_MODE_SHIFT               12
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_RM2_SYS_MODE_MASK                0x00003000
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_BITCOLLECT_NR_SHIFT              10
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_BITCOLLECT_NR_MASK               0x00000C00
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_BITCOLLECT_NC_SHIFT              0
#define    CPC_HARQ_PS3_CONFIG_PARAM_PS3_BITCOLLECT_NC_MASK               0x000003FF

#define CPC_HARQ_PS3_SYS_RATEMATCH1_OFFSET                                0x00000070
#define CPC_HARQ_PS3_SYS_RATEMATCH1_TYPE                                  UInt32
#define CPC_HARQ_PS3_SYS_RATEMATCH1_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS3_SYS_RATEMATCH1_PARAM_PS3_RM2_SYS_EINI_SHIFT       16
#define    CPC_HARQ_PS3_SYS_RATEMATCH1_PARAM_PS3_RM2_SYS_EINI_MASK        0xFFFF0000
#define    CPC_HARQ_PS3_SYS_RATEMATCH1_PARAM_PS3_RM2_SYS_EPLUS_SHIFT      0
#define    CPC_HARQ_PS3_SYS_RATEMATCH1_PARAM_PS3_RM2_SYS_EPLUS_MASK       0x0000FFFF

#define CPC_HARQ_PS3_SYS_RATEMATCH2_OFFSET                                0x00000074
#define CPC_HARQ_PS3_SYS_RATEMATCH2_TYPE                                  UInt32
#define CPC_HARQ_PS3_SYS_RATEMATCH2_RESERVED_MASK                         0x00000000
#define    CPC_HARQ_PS3_SYS_RATEMATCH2_PARAM_PS3_RM2_SYS_EMINUS_SHIFT     16
#define    CPC_HARQ_PS3_SYS_RATEMATCH2_PARAM_PS3_RM2_SYS_EMINUS_MASK      0xFFFF0000
#define    CPC_HARQ_PS3_SYS_RATEMATCH2_PARAM_PS3_RM2_SYS_XI_SHIFT         0
#define    CPC_HARQ_PS3_SYS_RATEMATCH2_PARAM_PS3_RM2_SYS_XI_MASK          0x0000FFFF

#define CPC_HARQ_PS3_P1_RATEMATCH1_OFFSET                                 0x00000078
#define CPC_HARQ_PS3_P1_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS3_P1_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS3_P1_RATEMATCH1_PARAM_PS3_RM2_P1_EINI_SHIFT         16
#define    CPC_HARQ_PS3_P1_RATEMATCH1_PARAM_PS3_RM2_P1_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS3_P1_RATEMATCH1_PARAM_PS3_RM2_P1_EPLUS_SHIFT        0
#define    CPC_HARQ_PS3_P1_RATEMATCH1_PARAM_PS3_RM2_P1_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS3_P1_RATEMATCH2_OFFSET                                 0x0000007C
#define CPC_HARQ_PS3_P1_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS3_P1_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS3_P1_RATEMATCH2_PARAM_PS3_RM2_P1_EMINUS_SHIFT       16
#define    CPC_HARQ_PS3_P1_RATEMATCH2_PARAM_PS3_RM2_P1_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS3_P1_RATEMATCH2_PARAM_PS3_RM2_P1_XI_SHIFT           0
#define    CPC_HARQ_PS3_P1_RATEMATCH2_PARAM_PS3_RM2_P1_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS3_P2_RATEMATCH1_OFFSET                                 0x00000080
#define CPC_HARQ_PS3_P2_RATEMATCH1_TYPE                                   UInt32
#define CPC_HARQ_PS3_P2_RATEMATCH1_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS3_P2_RATEMATCH1_PARAM_PS3_RM2_P2_EINI_SHIFT         16
#define    CPC_HARQ_PS3_P2_RATEMATCH1_PARAM_PS3_RM2_P2_EINI_MASK          0xFFFF0000
#define    CPC_HARQ_PS3_P2_RATEMATCH1_PARAM_PS3_RM2_P2_EPLUS_SHIFT        0
#define    CPC_HARQ_PS3_P2_RATEMATCH1_PARAM_PS3_RM2_P2_EPLUS_MASK         0x0000FFFF

#define CPC_HARQ_PS3_P2_RATEMATCH2_OFFSET                                 0x00000084
#define CPC_HARQ_PS3_P2_RATEMATCH2_TYPE                                   UInt32
#define CPC_HARQ_PS3_P2_RATEMATCH2_RESERVED_MASK                          0x00000000
#define    CPC_HARQ_PS3_P2_RATEMATCH2_PARAM_PS3_RM2_P2_EMINUS_SHIFT       16
#define    CPC_HARQ_PS3_P2_RATEMATCH2_PARAM_PS3_RM2_P2_EMINUS_MASK        0xFFFF0000
#define    CPC_HARQ_PS3_P2_RATEMATCH2_PARAM_PS3_RM2_P2_XI_SHIFT           0
#define    CPC_HARQ_PS3_P2_RATEMATCH2_PARAM_PS3_RM2_P2_XI_MASK            0x0000FFFF

#define CPC_HARQ_PS3_IR_PTR1_OFFSET                                       0x00000088
#define CPC_HARQ_PS3_IR_PTR1_TYPE                                         UInt32
#define CPC_HARQ_PS3_IR_PTR1_RESERVED_MASK                                0xF800F800
#define    CPC_HARQ_PS3_IR_PTR1_PARAM_PS3_SYS_IR_BASE_SHIFT               16
#define    CPC_HARQ_PS3_IR_PTR1_PARAM_PS3_SYS_IR_BASE_MASK                0x07FF0000
#define    CPC_HARQ_PS3_IR_PTR1_PARAM_PS3_P1_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS3_IR_PTR1_PARAM_PS3_P1_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_PS3_IR_PTR2_OFFSET                                       0x0000008C
#define CPC_HARQ_PS3_IR_PTR2_TYPE                                         UInt32
#define CPC_HARQ_PS3_IR_PTR2_RESERVED_MASK                                0xFFFFF800
#define    CPC_HARQ_PS3_IR_PTR2_PARAM_PS3_P2_IR_BASE_SHIFT                0
#define    CPC_HARQ_PS3_IR_PTR2_PARAM_PS3_P2_IR_BASE_MASK                 0x000007FF

#define CPC_HARQ_CPC_HARQ_CONTROLFLAGS_OFFSET                             0x00000090
#define CPC_HARQ_CPC_HARQ_CONTROLFLAGS_TYPE                               UInt32
#define CPC_HARQ_CPC_HARQ_CONTROLFLAGS_RESERVED_MASK                      0xEEECE0EE
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_P2_COMPLETE_SHIFT           28
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_P2_COMPLETE_MASK            0x10000000
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_P1_COMPLETE_SHIFT           24
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_P1_COMPLETE_MASK            0x01000000
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_SYS_COMPLETE_SHIFT          20
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_SYS_COMPLETE_MASK           0x00100000
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_ACTIVE_PROC_SHIFT           16
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_ACTIVE_PROC_MASK            0x00030000
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_TRCH_STATE_SHIFT            8
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_DBG_TRCH_STATE_MASK             0x00001F00
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_PARAM_DBG_FORCETRCHPROC_SHIFT   4
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_PARAM_DBG_FORCETRCHPROC_MASK    0x00000010
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_PARAM_ENABLETRCHPROC_SHIFT      0
#define    CPC_HARQ_CPC_HARQ_CONTROLFLAGS_PARAM_ENABLETRCHPROC_MASK       0x00000001

#define CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_OFFSET                            0x00000094
#define CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_TYPE                              UInt32
#define CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_RESERVED_MASK                     0x0000F800
#define    CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_DBG_SYS_RDM_OUTCOUNT_SHIFT     16
#define    CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_DBG_SYS_RDM_OUTCOUNT_MASK      0xFFFF0000
#define    CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_DBG_SYS_RDM_INCOUNT_SHIFT      0
#define    CPC_HARQ_DBG_SYS_RATEMATCHCOUNT_DBG_SYS_RDM_INCOUNT_MASK       0x000007FF

#define CPC_HARQ_DBG_P1_RATEMATCHCOUNT_OFFSET                             0x00000098
#define CPC_HARQ_DBG_P1_RATEMATCHCOUNT_TYPE                               UInt32
#define CPC_HARQ_DBG_P1_RATEMATCHCOUNT_RESERVED_MASK                      0x0000F800
#define    CPC_HARQ_DBG_P1_RATEMATCHCOUNT_DBG_P1_RDM_OUTCOUNT_SHIFT       16
#define    CPC_HARQ_DBG_P1_RATEMATCHCOUNT_DBG_P1_RDM_OUTCOUNT_MASK        0xFFFF0000
#define    CPC_HARQ_DBG_P1_RATEMATCHCOUNT_DBG_P1_RDM_INCOUNT_SHIFT        0
#define    CPC_HARQ_DBG_P1_RATEMATCHCOUNT_DBG_P1_RDM_INCOUNT_MASK         0x000007FF

#define CPC_HARQ_DBG_P2_RATEMATCHCOUNT_OFFSET                             0x0000009C
#define CPC_HARQ_DBG_P2_RATEMATCHCOUNT_TYPE                               UInt32
#define CPC_HARQ_DBG_P2_RATEMATCHCOUNT_RESERVED_MASK                      0x0000F800
#define    CPC_HARQ_DBG_P2_RATEMATCHCOUNT_DBG_P2_RDM_OUTCOUNT_SHIFT       16
#define    CPC_HARQ_DBG_P2_RATEMATCHCOUNT_DBG_P2_RDM_OUTCOUNT_MASK        0xFFFF0000
#define    CPC_HARQ_DBG_P2_RATEMATCHCOUNT_DBG_P2_RDM_INCOUNT_SHIFT        0
#define    CPC_HARQ_DBG_P2_RATEMATCHCOUNT_DBG_P2_RDM_INCOUNT_MASK         0x000007FF

#define CPC_HARQ_DBG_CLOCKCOUNT_OFFSET                                    0x000000A0
#define CPC_HARQ_DBG_CLOCKCOUNT_TYPE                                      UInt32
#define CPC_HARQ_DBG_CLOCKCOUNT_RESERVED_MASK                             0xFFF80000
#define    CPC_HARQ_DBG_CLOCKCOUNT_DBG_TRCH_CLOCKCOUNT_SHIFT              0
#define    CPC_HARQ_DBG_CLOCKCOUNT_DBG_TRCH_CLOCKCOUNT_MASK               0x0007FFFF

#define CPC_HARQ_DBG_CIRC_APB_INTERFACE_OFFSET                            0x000000A4
#define CPC_HARQ_DBG_CIRC_APB_INTERFACE_TYPE                              UInt32
#define CPC_HARQ_DBG_CIRC_APB_INTERFACE_RESERVED_MASK                     0xEEE08000
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_WR_EN_SHIFT           28
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_WR_EN_MASK            0x10000000
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_RD_EN_SHIFT           24
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_RD_EN_MASK            0x01000000
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_WR_DATA_SHIFT         16
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_WR_DATA_MASK          0x001F0000
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_ADDR_SHIFT            0
#define    CPC_HARQ_DBG_CIRC_APB_INTERFACE_DBG_CIRC_ADDR_MASK             0x00007FFF

#define CPC_HARQ_DBG_CIRC_APB_READ_DATA_OFFSET                            0x000000A8
#define CPC_HARQ_DBG_CIRC_APB_READ_DATA_TYPE                              UInt32
#define CPC_HARQ_DBG_CIRC_APB_READ_DATA_RESERVED_MASK                     0xFFFEFFE0
#define    CPC_HARQ_DBG_CIRC_APB_READ_DATA_DBG_CIRC_MODE_SHIFT            16
#define    CPC_HARQ_DBG_CIRC_APB_READ_DATA_DBG_CIRC_MODE_MASK             0x00010000
#define    CPC_HARQ_DBG_CIRC_APB_READ_DATA_DBG_CIRC_RD_DATA_SHIFT         0
#define    CPC_HARQ_DBG_CIRC_APB_READ_DATA_DBG_CIRC_RD_DATA_MASK          0x0000001F

#endif /* __BRCM_RDB_CPC_HARQ_H__ */



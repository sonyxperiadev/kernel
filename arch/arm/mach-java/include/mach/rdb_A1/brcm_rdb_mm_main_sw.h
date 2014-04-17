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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_MM_MAIN_SW_H__
#define __BRCM_RDB_MM_MAIN_SW_H__

#define MM_MAIN_SW_RESERVED_MI0_0_OFFSET                                  0x00000400
#define MM_MAIN_SW_RESERVED_MI0_0_TYPE                                    UInt32
#define MM_MAIN_SW_RESERVED_MI0_0_RESERVED_MASK                           0xFFFFFFFF

#define MM_MAIN_SW_RESERVED_MI0_1_OFFSET                                  0x00000404
#define MM_MAIN_SW_RESERVED_MI0_1_TYPE                                    UInt32
#define MM_MAIN_SW_RESERVED_MI0_1_RESERVED_MASK                           0xFFFFFFFF

#define MM_MAIN_SW_AR_ARB_MI0_OFFSET                                      0x00000408
#define MM_MAIN_SW_AR_ARB_MI0_TYPE                                        UInt32
#define MM_MAIN_SW_AR_ARB_MI0_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AR_ARB_MI0_INTERFACE_NUM_MI0_AR_SHIFT               24
#define    MM_MAIN_SW_AR_ARB_MI0_INTERFACE_NUM_MI0_AR_MASK                0xFF000000
#define    MM_MAIN_SW_AR_ARB_MI0_ARB_PRIORITY_MI0_AR_SHIFT                8
#define    MM_MAIN_SW_AR_ARB_MI0_ARB_PRIORITY_MI0_AR_MASK                 0x0000FF00
#define    MM_MAIN_SW_AR_ARB_MI0_READ_PARAM_SI_MI0_AR_SHIFT               0
#define    MM_MAIN_SW_AR_ARB_MI0_READ_PARAM_SI_MI0_AR_MASK                0x000000FF

#define MM_MAIN_SW_AW_ARB_MI0_OFFSET                                      0x0000040C
#define MM_MAIN_SW_AW_ARB_MI0_TYPE                                        UInt32
#define MM_MAIN_SW_AW_ARB_MI0_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AW_ARB_MI0_INTERFACE_NUM_MI0_AW_SHIFT               24
#define    MM_MAIN_SW_AW_ARB_MI0_INTERFACE_NUM_MI0_AW_MASK                0xFF000000
#define    MM_MAIN_SW_AW_ARB_MI0_ARB_PRIORITY_MI0_AW_SHIFT                8
#define    MM_MAIN_SW_AW_ARB_MI0_ARB_PRIORITY_MI0_AW_MASK                 0x0000FF00
#define    MM_MAIN_SW_AW_ARB_MI0_READ_PARAM_SI_MI0_AW_SHIFT               0
#define    MM_MAIN_SW_AW_ARB_MI0_READ_PARAM_SI_MI0_AW_MASK                0x000000FF

#define MM_MAIN_SW_RESERVED_MI1_0_OFFSET                                  0x00000420
#define MM_MAIN_SW_RESERVED_MI1_0_TYPE                                    UInt32
#define MM_MAIN_SW_RESERVED_MI1_0_RESERVED_MASK                           0xFFFFFFFF

#define MM_MAIN_SW_RESERVED_MI1_1_OFFSET                                  0x00000424
#define MM_MAIN_SW_RESERVED_MI1_1_TYPE                                    UInt32
#define MM_MAIN_SW_RESERVED_MI1_1_RESERVED_MASK                           0xFFFFFFFF

#define MM_MAIN_SW_AR_ARB_MI1_OFFSET                                      0x00000428
#define MM_MAIN_SW_AR_ARB_MI1_TYPE                                        UInt32
#define MM_MAIN_SW_AR_ARB_MI1_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AR_ARB_MI1_INTERFACE_NUM_MI1_AR_SHIFT               24
#define    MM_MAIN_SW_AR_ARB_MI1_INTERFACE_NUM_MI1_AR_MASK                0xFF000000
#define    MM_MAIN_SW_AR_ARB_MI1_ARB_PRIORITY_MI1_AR_SHIFT                8
#define    MM_MAIN_SW_AR_ARB_MI1_ARB_PRIORITY_MI1_AR_MASK                 0x0000FF00
#define    MM_MAIN_SW_AR_ARB_MI1_READ_PARAM_SI_MI1_AR_SHIFT               0
#define    MM_MAIN_SW_AR_ARB_MI1_READ_PARAM_SI_MI1_AR_MASK                0x000000FF

#define MM_MAIN_SW_AW_ARB_MI1_OFFSET                                      0x0000042C
#define MM_MAIN_SW_AW_ARB_MI1_TYPE                                        UInt32
#define MM_MAIN_SW_AW_ARB_MI1_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AW_ARB_MI1_INTERFACE_NUM_MI1_AW_SHIFT               24
#define    MM_MAIN_SW_AW_ARB_MI1_INTERFACE_NUM_MI1_AW_MASK                0xFF000000
#define    MM_MAIN_SW_AW_ARB_MI1_ARB_PRIORITY_MI1_AW_SHIFT                8
#define    MM_MAIN_SW_AW_ARB_MI1_ARB_PRIORITY_MI1_AW_MASK                 0x0000FF00
#define    MM_MAIN_SW_AW_ARB_MI1_READ_PARAM_SI_MI1_AW_SHIFT               0
#define    MM_MAIN_SW_AW_ARB_MI1_READ_PARAM_SI_MI1_AW_MASK                0x000000FF

#define MM_MAIN_SW_QOS_TIDEMARK_MI2_OFFSET                                0x00000440
#define MM_MAIN_SW_QOS_TIDEMARK_MI2_TYPE                                  UInt32
#define MM_MAIN_SW_QOS_TIDEMARK_MI2_RESERVED_MASK                         0x00000000
#define    MM_MAIN_SW_QOS_TIDEMARK_MI2_QOS_TIDEMARK_MI2_SHIFT             0
#define    MM_MAIN_SW_QOS_TIDEMARK_MI2_QOS_TIDEMARK_MI2_MASK              0xFFFFFFFF

#define MM_MAIN_SW_QOS_ACCESS_CTRL_MI2_OFFSET                             0x00000444
#define MM_MAIN_SW_QOS_ACCESS_CTRL_MI2_TYPE                               UInt32
#define MM_MAIN_SW_QOS_ACCESS_CTRL_MI2_RESERVED_MASK                      0xFFFFFF00
#define    MM_MAIN_SW_QOS_ACCESS_CTRL_MI2_QOS_ACCESS_CTRL_MI2_SHIFT       0
#define    MM_MAIN_SW_QOS_ACCESS_CTRL_MI2_QOS_ACCESS_CTRL_MI2_MASK        0x000000FF

#define MM_MAIN_SW_AR_ARB_MI2_OFFSET                                      0x00000448
#define MM_MAIN_SW_AR_ARB_MI2_TYPE                                        UInt32
#define MM_MAIN_SW_AR_ARB_MI2_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AR_ARB_MI2_INTERFACE_NUM_MI2_AR_SHIFT               24
#define    MM_MAIN_SW_AR_ARB_MI2_INTERFACE_NUM_MI2_AR_MASK                0xFF000000
#define    MM_MAIN_SW_AR_ARB_MI2_ARB_PRIORITY_MI2_AR_SHIFT                8
#define    MM_MAIN_SW_AR_ARB_MI2_ARB_PRIORITY_MI2_AR_MASK                 0x0000FF00
#define    MM_MAIN_SW_AR_ARB_MI2_READ_PARAM_SI_MI2_AR_SHIFT               0
#define    MM_MAIN_SW_AR_ARB_MI2_READ_PARAM_SI_MI2_AR_MASK                0x000000FF

#define MM_MAIN_SW_AW_ARB_MI2_OFFSET                                      0x0000044C
#define MM_MAIN_SW_AW_ARB_MI2_TYPE                                        UInt32
#define MM_MAIN_SW_AW_ARB_MI2_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AW_ARB_MI2_INTERFACE_NUM_MI2_AW_SHIFT               24
#define    MM_MAIN_SW_AW_ARB_MI2_INTERFACE_NUM_MI2_AW_MASK                0xFF000000
#define    MM_MAIN_SW_AW_ARB_MI2_ARB_PRIORITY_MI2_AW_SHIFT                8
#define    MM_MAIN_SW_AW_ARB_MI2_ARB_PRIORITY_MI2_AW_MASK                 0x0000FF00
#define    MM_MAIN_SW_AW_ARB_MI2_READ_PARAM_SI_MI2_AW_SHIFT               0
#define    MM_MAIN_SW_AW_ARB_MI2_READ_PARAM_SI_MI2_AW_MASK                0x000000FF

#define MM_MAIN_SW_RESERVED_MI3_0_OFFSET                                  0x00000460
#define MM_MAIN_SW_RESERVED_MI3_0_TYPE                                    UInt32
#define MM_MAIN_SW_RESERVED_MI3_0_RESERVED_MASK                           0xFFFFFFFF

#define MM_MAIN_SW_RESERVED_MI3_1_OFFSET                                  0x00000464
#define MM_MAIN_SW_RESERVED_MI3_1_TYPE                                    UInt32
#define MM_MAIN_SW_RESERVED_MI3_1_RESERVED_MASK                           0xFFFFFFFF

#define MM_MAIN_SW_AR_ARB_MI3_OFFSET                                      0x00000468
#define MM_MAIN_SW_AR_ARB_MI3_TYPE                                        UInt32
#define MM_MAIN_SW_AR_ARB_MI3_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AR_ARB_MI3_INTERFACE_NUM_MI3_AR_SHIFT               24
#define    MM_MAIN_SW_AR_ARB_MI3_INTERFACE_NUM_MI3_AR_MASK                0xFF000000
#define    MM_MAIN_SW_AR_ARB_MI3_ARB_PRIORITY_MI3_AR_SHIFT                8
#define    MM_MAIN_SW_AR_ARB_MI3_ARB_PRIORITY_MI3_AR_MASK                 0x0000FF00
#define    MM_MAIN_SW_AR_ARB_MI3_READ_PARAM_SI_MI3_AR_SHIFT               0
#define    MM_MAIN_SW_AR_ARB_MI3_READ_PARAM_SI_MI3_AR_MASK                0x000000FF

#define MM_MAIN_SW_AW_ARB_MI3_OFFSET                                      0x0000046C
#define MM_MAIN_SW_AW_ARB_MI3_TYPE                                        UInt32
#define MM_MAIN_SW_AW_ARB_MI3_RESERVED_MASK                               0x00FF0000
#define    MM_MAIN_SW_AW_ARB_MI3_INTERFACE_NUM_MI3_AW_SHIFT               24
#define    MM_MAIN_SW_AW_ARB_MI3_INTERFACE_NUM_MI3_AW_MASK                0xFF000000
#define    MM_MAIN_SW_AW_ARB_MI3_ARB_PRIORITY_MI3_AW_SHIFT                8
#define    MM_MAIN_SW_AW_ARB_MI3_ARB_PRIORITY_MI3_AW_MASK                 0x0000FF00
#define    MM_MAIN_SW_AW_ARB_MI3_READ_PARAM_SI_MI3_AW_SHIFT               0
#define    MM_MAIN_SW_AW_ARB_MI3_READ_PARAM_SI_MI3_AW_MASK                0x000000FF

#endif /* __BRCM_RDB_MM_MAIN_SW_H__ */



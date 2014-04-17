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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_GIC_H__
#define __BRCM_RDB_GIC_H__

#define GIC_GICD_CTLR_OFFSET                                              0x00001000
#define GIC_GICD_CTLR_TYPE                                                UInt32
#define GIC_GICD_CTLR_RESERVED_MASK                                       0xFFFFFFFC
#define    GIC_GICD_CTLR_ENABLEGRP1_SHIFT                                 1
#define    GIC_GICD_CTLR_ENABLEGRP1_MASK                                  0x00000002
#define    GIC_GICD_CTLR_ENABLEGRP0_SHIFT                                 0
#define    GIC_GICD_CTLR_ENABLEGRP0_MASK                                  0x00000001

#define GIC_GICD_TYPER_OFFSET                                             0x00001004
#define GIC_GICD_TYPER_TYPE                                               UInt32
#define GIC_GICD_TYPER_RESERVED_MASK                                      0xFFFF0300
#define    GIC_GICD_TYPER_LSPI_SHIFT                                      11
#define    GIC_GICD_TYPER_LSPI_MASK                                       0x0000F800
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI0                               0x00000000
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI1                               0x00000001
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI2                               0x00000002
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI3                               0x00000003
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI4                               0x00000004
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI5                               0x00000005
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI6                               0x00000006
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI7                               0x00000007
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI8                               0x00000008
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI9                               0x00000009
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI10                              0x0000000A
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI11                              0x0000000B
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI12                              0x0000000C
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI13                              0x0000000D
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI14                              0x0000000E
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI15                              0x0000000F
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI16                              0x00000010
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI17                              0x00000011
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI18                              0x00000012
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI19                              0x00000013
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI20                              0x00000014
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI21                              0x00000015
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI22                              0x00000016
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI23                              0x00000017
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI24                              0x00000018
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI25                              0x00000019
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI26                              0x0000001A
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI27                              0x0000001B
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI28                              0x0000001C
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI29                              0x0000001D
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI30                              0x0000001E
#define       GIC_GICD_TYPER_LSPI_CMD_LSPI31                              0x0000001F
#define    GIC_GICD_TYPER_SECURITYEXTN_SHIFT                              10
#define    GIC_GICD_TYPER_SECURITYEXTN_MASK                               0x00000400
#define       GIC_GICD_TYPER_SECURITYEXTN_CMD_NONSECURE                   0x00000000
#define       GIC_GICD_TYPER_SECURITYEXTN_CMD_SECURE                      0x00000001
#define    GIC_GICD_TYPER_CPU_NUMBER_SHIFT                                5
#define    GIC_GICD_TYPER_CPU_NUMBER_MASK                                 0x000000E0
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU1                          0x00000000
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU2                          0x00000001
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU3                          0x00000002
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU4                          0x00000003
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU5                          0x00000004
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU6                          0x00000005
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU7                          0x00000006
#define       GIC_GICD_TYPER_CPU_NUMBER_CMD_CPU8                          0x00000007
#define    GIC_GICD_TYPER_IT_LINES_NUMBER_SHIFT                           0
#define    GIC_GICD_TYPER_IT_LINES_NUMBER_MASK                            0x0000001F
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST0              0x00000000
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST1              0x00000001
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST2              0x00000002
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST3              0x00000003
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST4              0x00000004
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST5              0x00000005
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST6              0x00000006
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST7              0x00000007
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST8              0x00000008
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST9              0x00000009
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST10             0x0000000A
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST11             0x0000000B
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST12             0x0000000C
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST13             0x0000000D
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST14             0x0000000E
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST15             0x0000000F
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST16             0x00000010
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST17             0x00000011
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST18             0x00000012
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST19             0x00000013
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST20             0x00000014
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST21             0x00000015
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST22             0x00000016
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST23             0x00000017
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST24             0x00000018
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST25             0x00000019
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST26             0x0000001A
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST27             0x0000001B
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST28             0x0000001C
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST29             0x0000001D
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST30             0x0000001E
#define       GIC_GICD_TYPER_IT_LINES_NUMBER_CMD_INTID_DIST31             0x0000001F

#define GIC_GICD_IIDR_OFFSET                                              0x00001008
#define GIC_GICD_IIDR_TYPE                                                UInt32
#define GIC_GICD_IIDR_RESERVED_MASK                                       0x00F00000
#define    GIC_GICD_IIDR_PRODUCTID_SHIFT                                  24
#define    GIC_GICD_IIDR_PRODUCTID_MASK                                   0xFF000000
#define    GIC_GICD_IIDR_VARIANT_SHIFT                                    16
#define    GIC_GICD_IIDR_VARIANT_MASK                                     0x000F0000
#define    GIC_GICD_IIDR_REVISION_NUMBER_SHIFT                            12
#define    GIC_GICD_IIDR_REVISION_NUMBER_MASK                             0x0000F000
#define    GIC_GICD_IIDR_IMPLEMENTOR_SHIFT                                0
#define    GIC_GICD_IIDR_IMPLEMENTOR_MASK                                 0x00000FFF

#define GIC_GICD_IGROUPRN_0_OFFSET                                        0x00001080
#define GIC_GICD_IGROUPRN_0_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_0_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_0_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_0_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_1_OFFSET                                        0x00001084
#define GIC_GICD_IGROUPRN_1_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_1_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_1_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_1_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_2_OFFSET                                        0x00001088
#define GIC_GICD_IGROUPRN_2_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_2_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_2_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_2_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_3_OFFSET                                        0x0000108C
#define GIC_GICD_IGROUPRN_3_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_3_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_3_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_3_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_4_OFFSET                                        0x00001090
#define GIC_GICD_IGROUPRN_4_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_4_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_4_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_4_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_5_OFFSET                                        0x00001094
#define GIC_GICD_IGROUPRN_5_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_5_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_5_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_5_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_6_OFFSET                                        0x00001098
#define GIC_GICD_IGROUPRN_6_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_6_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_6_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_6_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_IGROUPRN_7_OFFSET                                        0x0000109C
#define GIC_GICD_IGROUPRN_7_TYPE                                          UInt32
#define GIC_GICD_IGROUPRN_7_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_IGROUPRN_7_STATUS_SHIFT                               0
#define    GIC_GICD_IGROUPRN_7_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICD_ISENABLERN_0_OFFSET                                      0x00001100
#define GIC_GICD_ISENABLERN_0_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_0_PPI_ENABLE_SET_SHIFT                     16
#define    GIC_GICD_ISENABLERN_0_PPI_ENABLE_SET_MASK                      0xFFFF0000
#define    GIC_GICD_ISENABLERN_0_SGI_ENABLE_SET_SHIFT                     0
#define    GIC_GICD_ISENABLERN_0_SGI_ENABLE_SET_MASK                      0x0000FFFF

#define GIC_GICD_ISENABLERN_1_OFFSET                                      0x00001104
#define GIC_GICD_ISENABLERN_1_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_1_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_1_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ISENABLERN_2_OFFSET                                      0x00001108
#define GIC_GICD_ISENABLERN_2_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_2_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_2_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ISENABLERN_3_OFFSET                                      0x0000110C
#define GIC_GICD_ISENABLERN_3_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_3_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_3_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ISENABLERN_4_OFFSET                                      0x00001110
#define GIC_GICD_ISENABLERN_4_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_4_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_4_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_4_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ISENABLERN_5_OFFSET                                      0x00001114
#define GIC_GICD_ISENABLERN_5_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_5_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_5_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_5_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ISENABLERN_6_OFFSET                                      0x00001118
#define GIC_GICD_ISENABLERN_6_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_6_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_6_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_6_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ISENABLERN_7_OFFSET                                      0x0000111C
#define GIC_GICD_ISENABLERN_7_TYPE                                        UInt32
#define GIC_GICD_ISENABLERN_7_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISENABLERN_7_ENABLE_SET_SHIFT                         0
#define    GIC_GICD_ISENABLERN_7_ENABLE_SET_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_0_OFFSET                                      0x00001180
#define GIC_GICD_ICENABLERN_0_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_0_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_0_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_1_OFFSET                                      0x00001184
#define GIC_GICD_ICENABLERN_1_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_1_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_1_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_2_OFFSET                                      0x00001188
#define GIC_GICD_ICENABLERN_2_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_2_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_2_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_3_OFFSET                                      0x0000118C
#define GIC_GICD_ICENABLERN_3_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_3_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_3_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_4_OFFSET                                      0x00001190
#define GIC_GICD_ICENABLERN_4_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_4_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_4_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_4_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_5_OFFSET                                      0x00001194
#define GIC_GICD_ICENABLERN_5_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_5_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_5_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_5_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_6_OFFSET                                      0x00001198
#define GIC_GICD_ICENABLERN_6_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_6_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_6_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_6_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ICENABLERN_7_OFFSET                                      0x0000119C
#define GIC_GICD_ICENABLERN_7_TYPE                                        UInt32
#define GIC_GICD_ICENABLERN_7_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICENABLERN_7_ENABLE_CLR_SHIFT                         0
#define    GIC_GICD_ICENABLERN_7_ENABLE_CLR_MASK                          0xFFFFFFFF

#define GIC_GICD_ISPENDRN_0_OFFSET                                        0x00001200
#define GIC_GICD_ISPENDRN_0_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_0_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_0_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_0_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_1_OFFSET                                        0x00001204
#define GIC_GICD_ISPENDRN_1_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_1_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_1_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_1_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_2_OFFSET                                        0x00001208
#define GIC_GICD_ISPENDRN_2_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_2_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_2_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_2_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_3_OFFSET                                        0x0000120C
#define GIC_GICD_ISPENDRN_3_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_3_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_3_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_3_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_4_OFFSET                                        0x00001210
#define GIC_GICD_ISPENDRN_4_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_4_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_4_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_4_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_5_OFFSET                                        0x00001214
#define GIC_GICD_ISPENDRN_5_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_5_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_5_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_5_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_6_OFFSET                                        0x00001218
#define GIC_GICD_ISPENDRN_6_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_6_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_6_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_6_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ISPENDRN_7_OFFSET                                        0x0000121C
#define GIC_GICD_ISPENDRN_7_TYPE                                          UInt32
#define GIC_GICD_ISPENDRN_7_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ISPENDRN_7_PENDING_SET_SHIFT                          0
#define    GIC_GICD_ISPENDRN_7_PENDING_SET_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_0_OFFSET                                        0x00001280
#define GIC_GICD_ICPENDRN_0_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_0_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_0_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_0_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_1_OFFSET                                        0x00001284
#define GIC_GICD_ICPENDRN_1_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_1_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_1_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_1_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_2_OFFSET                                        0x00001288
#define GIC_GICD_ICPENDRN_2_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_2_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_2_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_2_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_3_OFFSET                                        0x0000128C
#define GIC_GICD_ICPENDRN_3_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_3_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_3_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_3_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_4_OFFSET                                        0x00001290
#define GIC_GICD_ICPENDRN_4_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_4_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_4_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_4_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_5_OFFSET                                        0x00001294
#define GIC_GICD_ICPENDRN_5_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_5_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_5_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_5_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_6_OFFSET                                        0x00001298
#define GIC_GICD_ICPENDRN_6_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_6_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_6_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_6_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ICPENDRN_7_OFFSET                                        0x0000129C
#define GIC_GICD_ICPENDRN_7_TYPE                                          UInt32
#define GIC_GICD_ICPENDRN_7_RESERVED_MASK                                 0x00000000
#define    GIC_GICD_ICPENDRN_7_PENDING_CLR_SHIFT                          0
#define    GIC_GICD_ICPENDRN_7_PENDING_CLR_MASK                           0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_0_OFFSET                                      0x00001300
#define GIC_GICD_ISACTIVERN_0_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_0_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_0_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_1_OFFSET                                      0x00001304
#define GIC_GICD_ISACTIVERN_1_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_1_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_1_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_2_OFFSET                                      0x00001308
#define GIC_GICD_ISACTIVERN_2_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_2_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_2_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_3_OFFSET                                      0x0000130C
#define GIC_GICD_ISACTIVERN_3_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_3_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_3_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_4_OFFSET                                      0x00001310
#define GIC_GICD_ISACTIVERN_4_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_4_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_4_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_4_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_5_OFFSET                                      0x00001314
#define GIC_GICD_ISACTIVERN_5_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_5_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_5_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_5_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_6_OFFSET                                      0x00001318
#define GIC_GICD_ISACTIVERN_6_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_6_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_6_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_6_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ISACTIVERN_7_OFFSET                                      0x0000131C
#define GIC_GICD_ISACTIVERN_7_TYPE                                        UInt32
#define GIC_GICD_ISACTIVERN_7_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ISACTIVERN_7_SET_ACTIVE_SHIFT                         0
#define    GIC_GICD_ISACTIVERN_7_SET_ACTIVE_MASK                          0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_0_OFFSET                                      0x00001380
#define GIC_GICD_ICACTIVERN_0_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_0_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_0_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_1_OFFSET                                      0x00001384
#define GIC_GICD_ICACTIVERN_1_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_1_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_1_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_2_OFFSET                                      0x00001388
#define GIC_GICD_ICACTIVERN_2_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_2_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_2_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_3_OFFSET                                      0x0000138C
#define GIC_GICD_ICACTIVERN_3_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_3_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_3_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_4_OFFSET                                      0x00001390
#define GIC_GICD_ICACTIVERN_4_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_4_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_4_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_4_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_5_OFFSET                                      0x00001394
#define GIC_GICD_ICACTIVERN_5_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_5_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_5_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_5_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_6_OFFSET                                      0x00001398
#define GIC_GICD_ICACTIVERN_6_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_6_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_6_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_6_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_ICACTIVERN_7_OFFSET                                      0x0000139C
#define GIC_GICD_ICACTIVERN_7_TYPE                                        UInt32
#define GIC_GICD_ICACTIVERN_7_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ICACTIVERN_7_CLEAR_ACTIVE_SHIFT                       0
#define    GIC_GICD_ICACTIVERN_7_CLEAR_ACTIVE_MASK                        0xFFFFFFFF

#define GIC_GICD_IPRIORITYRN_0_OFFSET                                     0x00001400
#define GIC_GICD_IPRIORITYRN_0_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_0_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_0_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_0_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_1_OFFSET                                     0x00001401
#define GIC_GICD_IPRIORITYRN_1_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_1_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_1_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_1_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_2_OFFSET                                     0x00001402
#define GIC_GICD_IPRIORITYRN_2_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_2_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_2_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_2_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_3_OFFSET                                     0x00001403
#define GIC_GICD_IPRIORITYRN_3_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_3_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_3_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_3_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_4_OFFSET                                     0x00001404
#define GIC_GICD_IPRIORITYRN_4_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_4_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_4_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_4_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_5_OFFSET                                     0x00001405
#define GIC_GICD_IPRIORITYRN_5_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_5_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_5_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_5_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_6_OFFSET                                     0x00001406
#define GIC_GICD_IPRIORITYRN_6_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_6_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_6_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_6_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_7_OFFSET                                     0x00001407
#define GIC_GICD_IPRIORITYRN_7_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_7_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_7_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_7_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_8_OFFSET                                     0x00001408
#define GIC_GICD_IPRIORITYRN_8_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_8_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_8_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_8_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_9_OFFSET                                     0x00001409
#define GIC_GICD_IPRIORITYRN_9_TYPE                                       UInt8
#define GIC_GICD_IPRIORITYRN_9_RESERVED_MASK                              0x00000000
#define    GIC_GICD_IPRIORITYRN_9_PRIORITY_LEVEL_INTID_N_SHIFT            0
#define    GIC_GICD_IPRIORITYRN_9_PRIORITY_LEVEL_INTID_N_MASK             0x000000FF

#define GIC_GICD_IPRIORITYRN_10_OFFSET                                    0x0000140A
#define GIC_GICD_IPRIORITYRN_10_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_10_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_10_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_10_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_11_OFFSET                                    0x0000140B
#define GIC_GICD_IPRIORITYRN_11_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_11_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_11_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_11_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_12_OFFSET                                    0x0000140C
#define GIC_GICD_IPRIORITYRN_12_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_12_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_12_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_12_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_13_OFFSET                                    0x0000140D
#define GIC_GICD_IPRIORITYRN_13_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_13_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_13_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_13_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_14_OFFSET                                    0x0000140E
#define GIC_GICD_IPRIORITYRN_14_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_14_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_14_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_14_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_15_OFFSET                                    0x0000140F
#define GIC_GICD_IPRIORITYRN_15_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_15_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_15_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_15_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_16_OFFSET                                    0x00001410
#define GIC_GICD_IPRIORITYRN_16_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_16_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_16_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_16_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_17_OFFSET                                    0x00001411
#define GIC_GICD_IPRIORITYRN_17_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_17_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_17_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_17_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_18_OFFSET                                    0x00001412
#define GIC_GICD_IPRIORITYRN_18_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_18_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_18_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_18_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_19_OFFSET                                    0x00001413
#define GIC_GICD_IPRIORITYRN_19_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_19_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_19_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_19_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_20_OFFSET                                    0x00001414
#define GIC_GICD_IPRIORITYRN_20_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_20_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_20_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_20_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_21_OFFSET                                    0x00001415
#define GIC_GICD_IPRIORITYRN_21_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_21_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_21_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_21_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_22_OFFSET                                    0x00001416
#define GIC_GICD_IPRIORITYRN_22_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_22_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_22_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_22_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_23_OFFSET                                    0x00001417
#define GIC_GICD_IPRIORITYRN_23_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_23_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_23_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_23_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_24_OFFSET                                    0x00001418
#define GIC_GICD_IPRIORITYRN_24_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_24_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_24_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_24_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_25_OFFSET                                    0x00001419
#define GIC_GICD_IPRIORITYRN_25_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_25_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_25_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_25_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_26_OFFSET                                    0x0000141A
#define GIC_GICD_IPRIORITYRN_26_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_26_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_26_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_26_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_27_OFFSET                                    0x0000141B
#define GIC_GICD_IPRIORITYRN_27_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_27_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_27_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_27_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_28_OFFSET                                    0x0000141C
#define GIC_GICD_IPRIORITYRN_28_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_28_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_28_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_28_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_29_OFFSET                                    0x0000141D
#define GIC_GICD_IPRIORITYRN_29_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_29_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_29_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_29_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_30_OFFSET                                    0x0000141E
#define GIC_GICD_IPRIORITYRN_30_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_30_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_30_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_30_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_31_OFFSET                                    0x0000141F
#define GIC_GICD_IPRIORITYRN_31_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_31_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_31_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_31_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_32_OFFSET                                    0x00001420
#define GIC_GICD_IPRIORITYRN_32_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_32_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_32_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_32_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_33_OFFSET                                    0x00001421
#define GIC_GICD_IPRIORITYRN_33_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_33_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_33_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_33_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_34_OFFSET                                    0x00001422
#define GIC_GICD_IPRIORITYRN_34_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_34_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_34_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_34_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_35_OFFSET                                    0x00001423
#define GIC_GICD_IPRIORITYRN_35_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_35_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_35_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_35_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_36_OFFSET                                    0x00001424
#define GIC_GICD_IPRIORITYRN_36_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_36_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_36_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_36_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_37_OFFSET                                    0x00001425
#define GIC_GICD_IPRIORITYRN_37_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_37_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_37_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_37_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_38_OFFSET                                    0x00001426
#define GIC_GICD_IPRIORITYRN_38_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_38_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_38_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_38_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_39_OFFSET                                    0x00001427
#define GIC_GICD_IPRIORITYRN_39_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_39_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_39_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_39_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_40_OFFSET                                    0x00001428
#define GIC_GICD_IPRIORITYRN_40_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_40_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_40_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_40_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_41_OFFSET                                    0x00001429
#define GIC_GICD_IPRIORITYRN_41_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_41_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_41_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_41_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_42_OFFSET                                    0x0000142A
#define GIC_GICD_IPRIORITYRN_42_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_42_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_42_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_42_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_43_OFFSET                                    0x0000142B
#define GIC_GICD_IPRIORITYRN_43_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_43_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_43_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_43_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_44_OFFSET                                    0x0000142C
#define GIC_GICD_IPRIORITYRN_44_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_44_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_44_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_44_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_45_OFFSET                                    0x0000142D
#define GIC_GICD_IPRIORITYRN_45_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_45_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_45_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_45_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_46_OFFSET                                    0x0000142E
#define GIC_GICD_IPRIORITYRN_46_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_46_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_46_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_46_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_47_OFFSET                                    0x0000142F
#define GIC_GICD_IPRIORITYRN_47_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_47_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_47_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_47_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_48_OFFSET                                    0x00001430
#define GIC_GICD_IPRIORITYRN_48_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_48_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_48_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_48_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_49_OFFSET                                    0x00001431
#define GIC_GICD_IPRIORITYRN_49_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_49_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_49_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_49_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_50_OFFSET                                    0x00001432
#define GIC_GICD_IPRIORITYRN_50_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_50_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_50_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_50_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_51_OFFSET                                    0x00001433
#define GIC_GICD_IPRIORITYRN_51_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_51_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_51_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_51_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_52_OFFSET                                    0x00001434
#define GIC_GICD_IPRIORITYRN_52_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_52_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_52_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_52_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_53_OFFSET                                    0x00001435
#define GIC_GICD_IPRIORITYRN_53_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_53_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_53_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_53_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_54_OFFSET                                    0x00001436
#define GIC_GICD_IPRIORITYRN_54_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_54_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_54_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_54_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_55_OFFSET                                    0x00001437
#define GIC_GICD_IPRIORITYRN_55_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_55_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_55_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_55_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_56_OFFSET                                    0x00001438
#define GIC_GICD_IPRIORITYRN_56_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_56_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_56_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_56_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_57_OFFSET                                    0x00001439
#define GIC_GICD_IPRIORITYRN_57_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_57_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_57_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_57_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_58_OFFSET                                    0x0000143A
#define GIC_GICD_IPRIORITYRN_58_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_58_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_58_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_58_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_59_OFFSET                                    0x0000143B
#define GIC_GICD_IPRIORITYRN_59_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_59_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_59_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_59_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_60_OFFSET                                    0x0000143C
#define GIC_GICD_IPRIORITYRN_60_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_60_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_60_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_60_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_61_OFFSET                                    0x0000143D
#define GIC_GICD_IPRIORITYRN_61_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_61_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_61_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_61_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_62_OFFSET                                    0x0000143E
#define GIC_GICD_IPRIORITYRN_62_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_62_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_62_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_62_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_63_OFFSET                                    0x0000143F
#define GIC_GICD_IPRIORITYRN_63_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_63_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_63_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_63_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_64_OFFSET                                    0x00001440
#define GIC_GICD_IPRIORITYRN_64_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_64_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_64_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_64_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_65_OFFSET                                    0x00001441
#define GIC_GICD_IPRIORITYRN_65_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_65_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_65_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_65_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_66_OFFSET                                    0x00001442
#define GIC_GICD_IPRIORITYRN_66_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_66_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_66_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_66_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_67_OFFSET                                    0x00001443
#define GIC_GICD_IPRIORITYRN_67_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_67_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_67_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_67_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_68_OFFSET                                    0x00001444
#define GIC_GICD_IPRIORITYRN_68_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_68_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_68_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_68_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_69_OFFSET                                    0x00001445
#define GIC_GICD_IPRIORITYRN_69_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_69_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_69_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_69_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_70_OFFSET                                    0x00001446
#define GIC_GICD_IPRIORITYRN_70_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_70_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_70_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_70_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_71_OFFSET                                    0x00001447
#define GIC_GICD_IPRIORITYRN_71_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_71_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_71_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_71_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_72_OFFSET                                    0x00001448
#define GIC_GICD_IPRIORITYRN_72_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_72_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_72_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_72_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_73_OFFSET                                    0x00001449
#define GIC_GICD_IPRIORITYRN_73_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_73_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_73_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_73_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_74_OFFSET                                    0x0000144A
#define GIC_GICD_IPRIORITYRN_74_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_74_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_74_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_74_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_75_OFFSET                                    0x0000144B
#define GIC_GICD_IPRIORITYRN_75_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_75_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_75_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_75_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_76_OFFSET                                    0x0000144C
#define GIC_GICD_IPRIORITYRN_76_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_76_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_76_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_76_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_77_OFFSET                                    0x0000144D
#define GIC_GICD_IPRIORITYRN_77_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_77_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_77_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_77_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_78_OFFSET                                    0x0000144E
#define GIC_GICD_IPRIORITYRN_78_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_78_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_78_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_78_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_79_OFFSET                                    0x0000144F
#define GIC_GICD_IPRIORITYRN_79_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_79_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_79_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_79_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_80_OFFSET                                    0x00001450
#define GIC_GICD_IPRIORITYRN_80_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_80_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_80_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_80_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_81_OFFSET                                    0x00001451
#define GIC_GICD_IPRIORITYRN_81_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_81_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_81_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_81_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_82_OFFSET                                    0x00001452
#define GIC_GICD_IPRIORITYRN_82_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_82_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_82_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_82_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_83_OFFSET                                    0x00001453
#define GIC_GICD_IPRIORITYRN_83_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_83_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_83_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_83_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_84_OFFSET                                    0x00001454
#define GIC_GICD_IPRIORITYRN_84_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_84_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_84_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_84_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_85_OFFSET                                    0x00001455
#define GIC_GICD_IPRIORITYRN_85_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_85_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_85_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_85_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_86_OFFSET                                    0x00001456
#define GIC_GICD_IPRIORITYRN_86_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_86_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_86_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_86_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_87_OFFSET                                    0x00001457
#define GIC_GICD_IPRIORITYRN_87_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_87_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_87_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_87_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_88_OFFSET                                    0x00001458
#define GIC_GICD_IPRIORITYRN_88_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_88_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_88_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_88_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_89_OFFSET                                    0x00001459
#define GIC_GICD_IPRIORITYRN_89_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_89_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_89_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_89_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_90_OFFSET                                    0x0000145A
#define GIC_GICD_IPRIORITYRN_90_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_90_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_90_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_90_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_91_OFFSET                                    0x0000145B
#define GIC_GICD_IPRIORITYRN_91_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_91_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_91_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_91_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_92_OFFSET                                    0x0000145C
#define GIC_GICD_IPRIORITYRN_92_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_92_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_92_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_92_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_93_OFFSET                                    0x0000145D
#define GIC_GICD_IPRIORITYRN_93_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_93_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_93_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_93_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_94_OFFSET                                    0x0000145E
#define GIC_GICD_IPRIORITYRN_94_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_94_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_94_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_94_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_95_OFFSET                                    0x0000145F
#define GIC_GICD_IPRIORITYRN_95_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_95_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_95_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_95_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_96_OFFSET                                    0x00001460
#define GIC_GICD_IPRIORITYRN_96_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_96_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_96_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_96_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_97_OFFSET                                    0x00001461
#define GIC_GICD_IPRIORITYRN_97_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_97_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_97_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_97_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_98_OFFSET                                    0x00001462
#define GIC_GICD_IPRIORITYRN_98_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_98_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_98_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_98_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_99_OFFSET                                    0x00001463
#define GIC_GICD_IPRIORITYRN_99_TYPE                                      UInt8
#define GIC_GICD_IPRIORITYRN_99_RESERVED_MASK                             0x00000000
#define    GIC_GICD_IPRIORITYRN_99_PRIORITY_LEVEL_INTID_N_SHIFT           0
#define    GIC_GICD_IPRIORITYRN_99_PRIORITY_LEVEL_INTID_N_MASK            0x000000FF

#define GIC_GICD_IPRIORITYRN_100_OFFSET                                   0x00001464
#define GIC_GICD_IPRIORITYRN_100_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_100_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_100_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_100_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_101_OFFSET                                   0x00001465
#define GIC_GICD_IPRIORITYRN_101_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_101_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_101_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_101_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_102_OFFSET                                   0x00001466
#define GIC_GICD_IPRIORITYRN_102_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_102_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_102_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_102_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_103_OFFSET                                   0x00001467
#define GIC_GICD_IPRIORITYRN_103_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_103_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_103_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_103_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_104_OFFSET                                   0x00001468
#define GIC_GICD_IPRIORITYRN_104_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_104_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_104_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_104_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_105_OFFSET                                   0x00001469
#define GIC_GICD_IPRIORITYRN_105_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_105_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_105_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_105_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_106_OFFSET                                   0x0000146A
#define GIC_GICD_IPRIORITYRN_106_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_106_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_106_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_106_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_107_OFFSET                                   0x0000146B
#define GIC_GICD_IPRIORITYRN_107_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_107_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_107_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_107_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_108_OFFSET                                   0x0000146C
#define GIC_GICD_IPRIORITYRN_108_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_108_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_108_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_108_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_109_OFFSET                                   0x0000146D
#define GIC_GICD_IPRIORITYRN_109_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_109_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_109_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_109_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_110_OFFSET                                   0x0000146E
#define GIC_GICD_IPRIORITYRN_110_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_110_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_110_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_110_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_111_OFFSET                                   0x0000146F
#define GIC_GICD_IPRIORITYRN_111_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_111_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_111_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_111_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_112_OFFSET                                   0x00001470
#define GIC_GICD_IPRIORITYRN_112_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_112_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_112_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_112_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_113_OFFSET                                   0x00001471
#define GIC_GICD_IPRIORITYRN_113_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_113_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_113_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_113_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_114_OFFSET                                   0x00001472
#define GIC_GICD_IPRIORITYRN_114_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_114_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_114_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_114_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_115_OFFSET                                   0x00001473
#define GIC_GICD_IPRIORITYRN_115_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_115_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_115_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_115_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_116_OFFSET                                   0x00001474
#define GIC_GICD_IPRIORITYRN_116_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_116_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_116_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_116_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_117_OFFSET                                   0x00001475
#define GIC_GICD_IPRIORITYRN_117_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_117_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_117_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_117_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_118_OFFSET                                   0x00001476
#define GIC_GICD_IPRIORITYRN_118_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_118_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_118_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_118_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_119_OFFSET                                   0x00001477
#define GIC_GICD_IPRIORITYRN_119_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_119_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_119_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_119_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_120_OFFSET                                   0x00001478
#define GIC_GICD_IPRIORITYRN_120_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_120_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_120_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_120_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_121_OFFSET                                   0x00001479
#define GIC_GICD_IPRIORITYRN_121_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_121_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_121_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_121_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_122_OFFSET                                   0x0000147A
#define GIC_GICD_IPRIORITYRN_122_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_122_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_122_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_122_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_123_OFFSET                                   0x0000147B
#define GIC_GICD_IPRIORITYRN_123_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_123_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_123_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_123_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_124_OFFSET                                   0x0000147C
#define GIC_GICD_IPRIORITYRN_124_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_124_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_124_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_124_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_125_OFFSET                                   0x0000147D
#define GIC_GICD_IPRIORITYRN_125_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_125_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_125_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_125_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_126_OFFSET                                   0x0000147E
#define GIC_GICD_IPRIORITYRN_126_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_126_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_126_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_126_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_127_OFFSET                                   0x0000147F
#define GIC_GICD_IPRIORITYRN_127_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_127_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_127_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_127_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_128_OFFSET                                   0x00001480
#define GIC_GICD_IPRIORITYRN_128_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_128_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_128_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_128_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_129_OFFSET                                   0x00001481
#define GIC_GICD_IPRIORITYRN_129_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_129_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_129_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_129_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_130_OFFSET                                   0x00001482
#define GIC_GICD_IPRIORITYRN_130_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_130_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_130_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_130_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_131_OFFSET                                   0x00001483
#define GIC_GICD_IPRIORITYRN_131_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_131_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_131_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_131_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_132_OFFSET                                   0x00001484
#define GIC_GICD_IPRIORITYRN_132_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_132_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_132_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_132_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_133_OFFSET                                   0x00001485
#define GIC_GICD_IPRIORITYRN_133_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_133_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_133_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_133_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_134_OFFSET                                   0x00001486
#define GIC_GICD_IPRIORITYRN_134_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_134_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_134_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_134_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_135_OFFSET                                   0x00001487
#define GIC_GICD_IPRIORITYRN_135_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_135_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_135_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_135_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_136_OFFSET                                   0x00001488
#define GIC_GICD_IPRIORITYRN_136_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_136_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_136_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_136_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_137_OFFSET                                   0x00001489
#define GIC_GICD_IPRIORITYRN_137_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_137_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_137_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_137_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_138_OFFSET                                   0x0000148A
#define GIC_GICD_IPRIORITYRN_138_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_138_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_138_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_138_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_139_OFFSET                                   0x0000148B
#define GIC_GICD_IPRIORITYRN_139_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_139_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_139_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_139_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_140_OFFSET                                   0x0000148C
#define GIC_GICD_IPRIORITYRN_140_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_140_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_140_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_140_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_141_OFFSET                                   0x0000148D
#define GIC_GICD_IPRIORITYRN_141_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_141_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_141_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_141_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_142_OFFSET                                   0x0000148E
#define GIC_GICD_IPRIORITYRN_142_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_142_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_142_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_142_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_143_OFFSET                                   0x0000148F
#define GIC_GICD_IPRIORITYRN_143_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_143_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_143_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_143_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_144_OFFSET                                   0x00001490
#define GIC_GICD_IPRIORITYRN_144_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_144_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_144_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_144_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_145_OFFSET                                   0x00001491
#define GIC_GICD_IPRIORITYRN_145_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_145_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_145_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_145_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_146_OFFSET                                   0x00001492
#define GIC_GICD_IPRIORITYRN_146_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_146_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_146_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_146_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_147_OFFSET                                   0x00001493
#define GIC_GICD_IPRIORITYRN_147_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_147_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_147_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_147_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_148_OFFSET                                   0x00001494
#define GIC_GICD_IPRIORITYRN_148_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_148_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_148_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_148_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_149_OFFSET                                   0x00001495
#define GIC_GICD_IPRIORITYRN_149_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_149_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_149_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_149_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_150_OFFSET                                   0x00001496
#define GIC_GICD_IPRIORITYRN_150_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_150_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_150_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_150_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_151_OFFSET                                   0x00001497
#define GIC_GICD_IPRIORITYRN_151_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_151_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_151_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_151_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_152_OFFSET                                   0x00001498
#define GIC_GICD_IPRIORITYRN_152_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_152_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_152_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_152_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_153_OFFSET                                   0x00001499
#define GIC_GICD_IPRIORITYRN_153_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_153_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_153_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_153_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_154_OFFSET                                   0x0000149A
#define GIC_GICD_IPRIORITYRN_154_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_154_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_154_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_154_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_155_OFFSET                                   0x0000149B
#define GIC_GICD_IPRIORITYRN_155_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_155_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_155_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_155_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_156_OFFSET                                   0x0000149C
#define GIC_GICD_IPRIORITYRN_156_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_156_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_156_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_156_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_157_OFFSET                                   0x0000149D
#define GIC_GICD_IPRIORITYRN_157_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_157_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_157_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_157_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_158_OFFSET                                   0x0000149E
#define GIC_GICD_IPRIORITYRN_158_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_158_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_158_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_158_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_159_OFFSET                                   0x0000149F
#define GIC_GICD_IPRIORITYRN_159_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_159_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_159_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_159_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_160_OFFSET                                   0x000014A0
#define GIC_GICD_IPRIORITYRN_160_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_160_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_160_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_160_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_161_OFFSET                                   0x000014A1
#define GIC_GICD_IPRIORITYRN_161_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_161_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_161_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_161_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_162_OFFSET                                   0x000014A2
#define GIC_GICD_IPRIORITYRN_162_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_162_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_162_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_162_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_163_OFFSET                                   0x000014A3
#define GIC_GICD_IPRIORITYRN_163_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_163_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_163_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_163_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_164_OFFSET                                   0x000014A4
#define GIC_GICD_IPRIORITYRN_164_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_164_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_164_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_164_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_165_OFFSET                                   0x000014A5
#define GIC_GICD_IPRIORITYRN_165_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_165_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_165_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_165_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_166_OFFSET                                   0x000014A6
#define GIC_GICD_IPRIORITYRN_166_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_166_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_166_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_166_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_167_OFFSET                                   0x000014A7
#define GIC_GICD_IPRIORITYRN_167_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_167_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_167_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_167_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_168_OFFSET                                   0x000014A8
#define GIC_GICD_IPRIORITYRN_168_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_168_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_168_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_168_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_169_OFFSET                                   0x000014A9
#define GIC_GICD_IPRIORITYRN_169_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_169_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_169_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_169_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_170_OFFSET                                   0x000014AA
#define GIC_GICD_IPRIORITYRN_170_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_170_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_170_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_170_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_171_OFFSET                                   0x000014AB
#define GIC_GICD_IPRIORITYRN_171_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_171_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_171_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_171_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_172_OFFSET                                   0x000014AC
#define GIC_GICD_IPRIORITYRN_172_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_172_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_172_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_172_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_173_OFFSET                                   0x000014AD
#define GIC_GICD_IPRIORITYRN_173_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_173_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_173_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_173_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_174_OFFSET                                   0x000014AE
#define GIC_GICD_IPRIORITYRN_174_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_174_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_174_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_174_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_175_OFFSET                                   0x000014AF
#define GIC_GICD_IPRIORITYRN_175_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_175_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_175_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_175_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_176_OFFSET                                   0x000014B0
#define GIC_GICD_IPRIORITYRN_176_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_176_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_176_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_176_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_177_OFFSET                                   0x000014B1
#define GIC_GICD_IPRIORITYRN_177_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_177_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_177_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_177_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_178_OFFSET                                   0x000014B2
#define GIC_GICD_IPRIORITYRN_178_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_178_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_178_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_178_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_179_OFFSET                                   0x000014B3
#define GIC_GICD_IPRIORITYRN_179_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_179_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_179_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_179_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_180_OFFSET                                   0x000014B4
#define GIC_GICD_IPRIORITYRN_180_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_180_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_180_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_180_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_181_OFFSET                                   0x000014B5
#define GIC_GICD_IPRIORITYRN_181_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_181_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_181_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_181_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_182_OFFSET                                   0x000014B6
#define GIC_GICD_IPRIORITYRN_182_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_182_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_182_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_182_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_183_OFFSET                                   0x000014B7
#define GIC_GICD_IPRIORITYRN_183_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_183_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_183_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_183_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_184_OFFSET                                   0x000014B8
#define GIC_GICD_IPRIORITYRN_184_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_184_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_184_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_184_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_185_OFFSET                                   0x000014B9
#define GIC_GICD_IPRIORITYRN_185_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_185_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_185_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_185_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_186_OFFSET                                   0x000014BA
#define GIC_GICD_IPRIORITYRN_186_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_186_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_186_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_186_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_187_OFFSET                                   0x000014BB
#define GIC_GICD_IPRIORITYRN_187_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_187_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_187_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_187_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_188_OFFSET                                   0x000014BC
#define GIC_GICD_IPRIORITYRN_188_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_188_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_188_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_188_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_189_OFFSET                                   0x000014BD
#define GIC_GICD_IPRIORITYRN_189_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_189_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_189_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_189_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_190_OFFSET                                   0x000014BE
#define GIC_GICD_IPRIORITYRN_190_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_190_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_190_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_190_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_191_OFFSET                                   0x000014BF
#define GIC_GICD_IPRIORITYRN_191_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_191_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_191_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_191_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_192_OFFSET                                   0x000014C0
#define GIC_GICD_IPRIORITYRN_192_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_192_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_192_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_192_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_193_OFFSET                                   0x000014C1
#define GIC_GICD_IPRIORITYRN_193_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_193_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_193_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_193_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_194_OFFSET                                   0x000014C2
#define GIC_GICD_IPRIORITYRN_194_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_194_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_194_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_194_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_195_OFFSET                                   0x000014C3
#define GIC_GICD_IPRIORITYRN_195_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_195_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_195_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_195_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_196_OFFSET                                   0x000014C4
#define GIC_GICD_IPRIORITYRN_196_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_196_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_196_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_196_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_197_OFFSET                                   0x000014C5
#define GIC_GICD_IPRIORITYRN_197_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_197_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_197_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_197_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_198_OFFSET                                   0x000014C6
#define GIC_GICD_IPRIORITYRN_198_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_198_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_198_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_198_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_199_OFFSET                                   0x000014C7
#define GIC_GICD_IPRIORITYRN_199_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_199_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_199_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_199_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_200_OFFSET                                   0x000014C8
#define GIC_GICD_IPRIORITYRN_200_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_200_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_200_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_200_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_201_OFFSET                                   0x000014C9
#define GIC_GICD_IPRIORITYRN_201_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_201_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_201_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_201_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_202_OFFSET                                   0x000014CA
#define GIC_GICD_IPRIORITYRN_202_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_202_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_202_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_202_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_203_OFFSET                                   0x000014CB
#define GIC_GICD_IPRIORITYRN_203_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_203_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_203_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_203_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_204_OFFSET                                   0x000014CC
#define GIC_GICD_IPRIORITYRN_204_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_204_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_204_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_204_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_205_OFFSET                                   0x000014CD
#define GIC_GICD_IPRIORITYRN_205_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_205_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_205_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_205_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_206_OFFSET                                   0x000014CE
#define GIC_GICD_IPRIORITYRN_206_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_206_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_206_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_206_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_207_OFFSET                                   0x000014CF
#define GIC_GICD_IPRIORITYRN_207_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_207_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_207_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_207_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_208_OFFSET                                   0x000014D0
#define GIC_GICD_IPRIORITYRN_208_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_208_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_208_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_208_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_209_OFFSET                                   0x000014D1
#define GIC_GICD_IPRIORITYRN_209_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_209_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_209_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_209_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_210_OFFSET                                   0x000014D2
#define GIC_GICD_IPRIORITYRN_210_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_210_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_210_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_210_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_211_OFFSET                                   0x000014D3
#define GIC_GICD_IPRIORITYRN_211_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_211_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_211_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_211_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_212_OFFSET                                   0x000014D4
#define GIC_GICD_IPRIORITYRN_212_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_212_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_212_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_212_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_213_OFFSET                                   0x000014D5
#define GIC_GICD_IPRIORITYRN_213_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_213_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_213_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_213_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_214_OFFSET                                   0x000014D6
#define GIC_GICD_IPRIORITYRN_214_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_214_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_214_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_214_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_215_OFFSET                                   0x000014D7
#define GIC_GICD_IPRIORITYRN_215_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_215_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_215_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_215_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_216_OFFSET                                   0x000014D8
#define GIC_GICD_IPRIORITYRN_216_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_216_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_216_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_216_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_217_OFFSET                                   0x000014D9
#define GIC_GICD_IPRIORITYRN_217_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_217_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_217_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_217_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_218_OFFSET                                   0x000014DA
#define GIC_GICD_IPRIORITYRN_218_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_218_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_218_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_218_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_219_OFFSET                                   0x000014DB
#define GIC_GICD_IPRIORITYRN_219_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_219_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_219_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_219_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_220_OFFSET                                   0x000014DC
#define GIC_GICD_IPRIORITYRN_220_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_220_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_220_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_220_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_221_OFFSET                                   0x000014DD
#define GIC_GICD_IPRIORITYRN_221_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_221_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_221_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_221_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_222_OFFSET                                   0x000014DE
#define GIC_GICD_IPRIORITYRN_222_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_222_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_222_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_222_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_223_OFFSET                                   0x000014DF
#define GIC_GICD_IPRIORITYRN_223_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_223_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_223_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_223_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_224_OFFSET                                   0x000014E0
#define GIC_GICD_IPRIORITYRN_224_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_224_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_224_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_224_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_225_OFFSET                                   0x000014E1
#define GIC_GICD_IPRIORITYRN_225_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_225_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_225_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_225_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_226_OFFSET                                   0x000014E2
#define GIC_GICD_IPRIORITYRN_226_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_226_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_226_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_226_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_227_OFFSET                                   0x000014E3
#define GIC_GICD_IPRIORITYRN_227_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_227_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_227_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_227_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_228_OFFSET                                   0x000014E4
#define GIC_GICD_IPRIORITYRN_228_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_228_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_228_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_228_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_229_OFFSET                                   0x000014E5
#define GIC_GICD_IPRIORITYRN_229_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_229_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_229_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_229_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_230_OFFSET                                   0x000014E6
#define GIC_GICD_IPRIORITYRN_230_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_230_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_230_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_230_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_231_OFFSET                                   0x000014E7
#define GIC_GICD_IPRIORITYRN_231_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_231_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_231_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_231_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_232_OFFSET                                   0x000014E8
#define GIC_GICD_IPRIORITYRN_232_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_232_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_232_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_232_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_233_OFFSET                                   0x000014E9
#define GIC_GICD_IPRIORITYRN_233_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_233_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_233_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_233_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_234_OFFSET                                   0x000014EA
#define GIC_GICD_IPRIORITYRN_234_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_234_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_234_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_234_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_235_OFFSET                                   0x000014EB
#define GIC_GICD_IPRIORITYRN_235_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_235_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_235_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_235_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_236_OFFSET                                   0x000014EC
#define GIC_GICD_IPRIORITYRN_236_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_236_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_236_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_236_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_237_OFFSET                                   0x000014ED
#define GIC_GICD_IPRIORITYRN_237_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_237_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_237_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_237_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_238_OFFSET                                   0x000014EE
#define GIC_GICD_IPRIORITYRN_238_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_238_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_238_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_238_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_239_OFFSET                                   0x000014EF
#define GIC_GICD_IPRIORITYRN_239_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_239_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_239_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_239_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_240_OFFSET                                   0x000014F0
#define GIC_GICD_IPRIORITYRN_240_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_240_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_240_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_240_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_241_OFFSET                                   0x000014F1
#define GIC_GICD_IPRIORITYRN_241_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_241_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_241_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_241_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_242_OFFSET                                   0x000014F2
#define GIC_GICD_IPRIORITYRN_242_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_242_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_242_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_242_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_243_OFFSET                                   0x000014F3
#define GIC_GICD_IPRIORITYRN_243_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_243_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_243_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_243_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_244_OFFSET                                   0x000014F4
#define GIC_GICD_IPRIORITYRN_244_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_244_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_244_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_244_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_245_OFFSET                                   0x000014F5
#define GIC_GICD_IPRIORITYRN_245_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_245_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_245_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_245_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_246_OFFSET                                   0x000014F6
#define GIC_GICD_IPRIORITYRN_246_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_246_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_246_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_246_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_247_OFFSET                                   0x000014F7
#define GIC_GICD_IPRIORITYRN_247_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_247_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_247_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_247_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_248_OFFSET                                   0x000014F8
#define GIC_GICD_IPRIORITYRN_248_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_248_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_248_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_248_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_249_OFFSET                                   0x000014F9
#define GIC_GICD_IPRIORITYRN_249_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_249_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_249_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_249_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_250_OFFSET                                   0x000014FA
#define GIC_GICD_IPRIORITYRN_250_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_250_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_250_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_250_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_251_OFFSET                                   0x000014FB
#define GIC_GICD_IPRIORITYRN_251_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_251_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_251_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_251_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_252_OFFSET                                   0x000014FC
#define GIC_GICD_IPRIORITYRN_252_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_252_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_252_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_252_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_253_OFFSET                                   0x000014FD
#define GIC_GICD_IPRIORITYRN_253_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_253_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_253_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_253_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_254_OFFSET                                   0x000014FE
#define GIC_GICD_IPRIORITYRN_254_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_254_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_254_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_254_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_IPRIORITYRN_255_OFFSET                                   0x000014FF
#define GIC_GICD_IPRIORITYRN_255_TYPE                                     UInt8
#define GIC_GICD_IPRIORITYRN_255_RESERVED_MASK                            0x00000000
#define    GIC_GICD_IPRIORITYRN_255_PRIORITY_LEVEL_INTID_N_SHIFT          0
#define    GIC_GICD_IPRIORITYRN_255_PRIORITY_LEVEL_INTID_N_MASK           0x000000FF

#define GIC_GICD_ITARGETSRN_0_OFFSET                                      0x00001800
#define GIC_GICD_ITARGETSRN_0_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_0_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_0_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_1_OFFSET                                      0x00001801
#define GIC_GICD_ITARGETSRN_1_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_1_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_1_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_2_OFFSET                                      0x00001802
#define GIC_GICD_ITARGETSRN_2_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_2_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_2_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_3_OFFSET                                      0x00001803
#define GIC_GICD_ITARGETSRN_3_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_3_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_3_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_4_OFFSET                                      0x00001804
#define GIC_GICD_ITARGETSRN_4_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_4_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_4_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_4_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_5_OFFSET                                      0x00001805
#define GIC_GICD_ITARGETSRN_5_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_5_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_5_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_5_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_6_OFFSET                                      0x00001806
#define GIC_GICD_ITARGETSRN_6_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_6_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_6_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_6_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_7_OFFSET                                      0x00001807
#define GIC_GICD_ITARGETSRN_7_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_7_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_7_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_7_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_8_OFFSET                                      0x00001808
#define GIC_GICD_ITARGETSRN_8_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_8_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_8_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_8_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_9_OFFSET                                      0x00001809
#define GIC_GICD_ITARGETSRN_9_TYPE                                        UInt8
#define GIC_GICD_ITARGETSRN_9_RESERVED_MASK                               0x00000000
#define    GIC_GICD_ITARGETSRN_9_SPI_TARGET_INTID_N_SHIFT                 0
#define    GIC_GICD_ITARGETSRN_9_SPI_TARGET_INTID_N_MASK                  0x000000FF

#define GIC_GICD_ITARGETSRN_10_OFFSET                                     0x0000180A
#define GIC_GICD_ITARGETSRN_10_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_10_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_10_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_10_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_11_OFFSET                                     0x0000180B
#define GIC_GICD_ITARGETSRN_11_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_11_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_11_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_11_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_12_OFFSET                                     0x0000180C
#define GIC_GICD_ITARGETSRN_12_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_12_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_12_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_12_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_13_OFFSET                                     0x0000180D
#define GIC_GICD_ITARGETSRN_13_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_13_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_13_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_13_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_14_OFFSET                                     0x0000180E
#define GIC_GICD_ITARGETSRN_14_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_14_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_14_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_14_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_15_OFFSET                                     0x0000180F
#define GIC_GICD_ITARGETSRN_15_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_15_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_15_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_15_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_16_OFFSET                                     0x00001810
#define GIC_GICD_ITARGETSRN_16_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_16_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_16_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_16_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_17_OFFSET                                     0x00001811
#define GIC_GICD_ITARGETSRN_17_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_17_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_17_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_17_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_18_OFFSET                                     0x00001812
#define GIC_GICD_ITARGETSRN_18_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_18_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_18_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_18_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_19_OFFSET                                     0x00001813
#define GIC_GICD_ITARGETSRN_19_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_19_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_19_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_19_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_20_OFFSET                                     0x00001814
#define GIC_GICD_ITARGETSRN_20_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_20_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_20_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_20_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_21_OFFSET                                     0x00001815
#define GIC_GICD_ITARGETSRN_21_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_21_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_21_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_21_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_22_OFFSET                                     0x00001816
#define GIC_GICD_ITARGETSRN_22_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_22_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_22_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_22_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_23_OFFSET                                     0x00001817
#define GIC_GICD_ITARGETSRN_23_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_23_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_23_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_23_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_24_OFFSET                                     0x00001818
#define GIC_GICD_ITARGETSRN_24_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_24_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_24_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_24_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_25_OFFSET                                     0x00001819
#define GIC_GICD_ITARGETSRN_25_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_25_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_25_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_25_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_26_OFFSET                                     0x0000181A
#define GIC_GICD_ITARGETSRN_26_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_26_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_26_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_26_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_27_OFFSET                                     0x0000181B
#define GIC_GICD_ITARGETSRN_27_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_27_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_27_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_27_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_28_OFFSET                                     0x0000181C
#define GIC_GICD_ITARGETSRN_28_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_28_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_28_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_28_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_29_OFFSET                                     0x0000181D
#define GIC_GICD_ITARGETSRN_29_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_29_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_29_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_29_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_30_OFFSET                                     0x0000181E
#define GIC_GICD_ITARGETSRN_30_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_30_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_30_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_30_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_31_OFFSET                                     0x0000181F
#define GIC_GICD_ITARGETSRN_31_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_31_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_31_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_31_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_32_OFFSET                                     0x00001820
#define GIC_GICD_ITARGETSRN_32_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_32_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_32_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_32_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_33_OFFSET                                     0x00001821
#define GIC_GICD_ITARGETSRN_33_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_33_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_33_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_33_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_34_OFFSET                                     0x00001822
#define GIC_GICD_ITARGETSRN_34_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_34_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_34_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_34_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_35_OFFSET                                     0x00001823
#define GIC_GICD_ITARGETSRN_35_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_35_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_35_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_35_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_36_OFFSET                                     0x00001824
#define GIC_GICD_ITARGETSRN_36_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_36_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_36_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_36_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_37_OFFSET                                     0x00001825
#define GIC_GICD_ITARGETSRN_37_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_37_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_37_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_37_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_38_OFFSET                                     0x00001826
#define GIC_GICD_ITARGETSRN_38_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_38_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_38_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_38_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_39_OFFSET                                     0x00001827
#define GIC_GICD_ITARGETSRN_39_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_39_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_39_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_39_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_40_OFFSET                                     0x00001828
#define GIC_GICD_ITARGETSRN_40_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_40_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_40_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_40_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_41_OFFSET                                     0x00001829
#define GIC_GICD_ITARGETSRN_41_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_41_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_41_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_41_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_42_OFFSET                                     0x0000182A
#define GIC_GICD_ITARGETSRN_42_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_42_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_42_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_42_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_43_OFFSET                                     0x0000182B
#define GIC_GICD_ITARGETSRN_43_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_43_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_43_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_43_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_44_OFFSET                                     0x0000182C
#define GIC_GICD_ITARGETSRN_44_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_44_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_44_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_44_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_45_OFFSET                                     0x0000182D
#define GIC_GICD_ITARGETSRN_45_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_45_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_45_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_45_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_46_OFFSET                                     0x0000182E
#define GIC_GICD_ITARGETSRN_46_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_46_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_46_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_46_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_47_OFFSET                                     0x0000182F
#define GIC_GICD_ITARGETSRN_47_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_47_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_47_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_47_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_48_OFFSET                                     0x00001830
#define GIC_GICD_ITARGETSRN_48_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_48_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_48_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_48_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_49_OFFSET                                     0x00001831
#define GIC_GICD_ITARGETSRN_49_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_49_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_49_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_49_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_50_OFFSET                                     0x00001832
#define GIC_GICD_ITARGETSRN_50_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_50_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_50_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_50_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_51_OFFSET                                     0x00001833
#define GIC_GICD_ITARGETSRN_51_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_51_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_51_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_51_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_52_OFFSET                                     0x00001834
#define GIC_GICD_ITARGETSRN_52_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_52_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_52_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_52_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_53_OFFSET                                     0x00001835
#define GIC_GICD_ITARGETSRN_53_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_53_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_53_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_53_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_54_OFFSET                                     0x00001836
#define GIC_GICD_ITARGETSRN_54_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_54_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_54_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_54_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_55_OFFSET                                     0x00001837
#define GIC_GICD_ITARGETSRN_55_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_55_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_55_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_55_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_56_OFFSET                                     0x00001838
#define GIC_GICD_ITARGETSRN_56_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_56_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_56_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_56_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_57_OFFSET                                     0x00001839
#define GIC_GICD_ITARGETSRN_57_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_57_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_57_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_57_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_58_OFFSET                                     0x0000183A
#define GIC_GICD_ITARGETSRN_58_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_58_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_58_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_58_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_59_OFFSET                                     0x0000183B
#define GIC_GICD_ITARGETSRN_59_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_59_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_59_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_59_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_60_OFFSET                                     0x0000183C
#define GIC_GICD_ITARGETSRN_60_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_60_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_60_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_60_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_61_OFFSET                                     0x0000183D
#define GIC_GICD_ITARGETSRN_61_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_61_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_61_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_61_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_62_OFFSET                                     0x0000183E
#define GIC_GICD_ITARGETSRN_62_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_62_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_62_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_62_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_63_OFFSET                                     0x0000183F
#define GIC_GICD_ITARGETSRN_63_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_63_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_63_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_63_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_64_OFFSET                                     0x00001840
#define GIC_GICD_ITARGETSRN_64_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_64_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_64_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_64_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_65_OFFSET                                     0x00001841
#define GIC_GICD_ITARGETSRN_65_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_65_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_65_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_65_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_66_OFFSET                                     0x00001842
#define GIC_GICD_ITARGETSRN_66_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_66_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_66_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_66_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_67_OFFSET                                     0x00001843
#define GIC_GICD_ITARGETSRN_67_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_67_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_67_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_67_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_68_OFFSET                                     0x00001844
#define GIC_GICD_ITARGETSRN_68_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_68_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_68_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_68_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_69_OFFSET                                     0x00001845
#define GIC_GICD_ITARGETSRN_69_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_69_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_69_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_69_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_70_OFFSET                                     0x00001846
#define GIC_GICD_ITARGETSRN_70_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_70_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_70_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_70_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_71_OFFSET                                     0x00001847
#define GIC_GICD_ITARGETSRN_71_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_71_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_71_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_71_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_72_OFFSET                                     0x00001848
#define GIC_GICD_ITARGETSRN_72_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_72_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_72_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_72_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_73_OFFSET                                     0x00001849
#define GIC_GICD_ITARGETSRN_73_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_73_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_73_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_73_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_74_OFFSET                                     0x0000184A
#define GIC_GICD_ITARGETSRN_74_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_74_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_74_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_74_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_75_OFFSET                                     0x0000184B
#define GIC_GICD_ITARGETSRN_75_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_75_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_75_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_75_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_76_OFFSET                                     0x0000184C
#define GIC_GICD_ITARGETSRN_76_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_76_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_76_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_76_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_77_OFFSET                                     0x0000184D
#define GIC_GICD_ITARGETSRN_77_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_77_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_77_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_77_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_78_OFFSET                                     0x0000184E
#define GIC_GICD_ITARGETSRN_78_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_78_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_78_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_78_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_79_OFFSET                                     0x0000184F
#define GIC_GICD_ITARGETSRN_79_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_79_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_79_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_79_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_80_OFFSET                                     0x00001850
#define GIC_GICD_ITARGETSRN_80_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_80_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_80_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_80_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_81_OFFSET                                     0x00001851
#define GIC_GICD_ITARGETSRN_81_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_81_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_81_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_81_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_82_OFFSET                                     0x00001852
#define GIC_GICD_ITARGETSRN_82_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_82_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_82_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_82_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_83_OFFSET                                     0x00001853
#define GIC_GICD_ITARGETSRN_83_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_83_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_83_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_83_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_84_OFFSET                                     0x00001854
#define GIC_GICD_ITARGETSRN_84_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_84_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_84_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_84_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_85_OFFSET                                     0x00001855
#define GIC_GICD_ITARGETSRN_85_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_85_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_85_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_85_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_86_OFFSET                                     0x00001856
#define GIC_GICD_ITARGETSRN_86_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_86_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_86_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_86_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_87_OFFSET                                     0x00001857
#define GIC_GICD_ITARGETSRN_87_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_87_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_87_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_87_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_88_OFFSET                                     0x00001858
#define GIC_GICD_ITARGETSRN_88_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_88_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_88_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_88_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_89_OFFSET                                     0x00001859
#define GIC_GICD_ITARGETSRN_89_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_89_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_89_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_89_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_90_OFFSET                                     0x0000185A
#define GIC_GICD_ITARGETSRN_90_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_90_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_90_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_90_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_91_OFFSET                                     0x0000185B
#define GIC_GICD_ITARGETSRN_91_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_91_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_91_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_91_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_92_OFFSET                                     0x0000185C
#define GIC_GICD_ITARGETSRN_92_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_92_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_92_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_92_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_93_OFFSET                                     0x0000185D
#define GIC_GICD_ITARGETSRN_93_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_93_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_93_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_93_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_94_OFFSET                                     0x0000185E
#define GIC_GICD_ITARGETSRN_94_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_94_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_94_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_94_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_95_OFFSET                                     0x0000185F
#define GIC_GICD_ITARGETSRN_95_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_95_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_95_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_95_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_96_OFFSET                                     0x00001860
#define GIC_GICD_ITARGETSRN_96_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_96_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_96_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_96_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_97_OFFSET                                     0x00001861
#define GIC_GICD_ITARGETSRN_97_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_97_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_97_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_97_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_98_OFFSET                                     0x00001862
#define GIC_GICD_ITARGETSRN_98_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_98_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_98_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_98_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_99_OFFSET                                     0x00001863
#define GIC_GICD_ITARGETSRN_99_TYPE                                       UInt8
#define GIC_GICD_ITARGETSRN_99_RESERVED_MASK                              0x00000000
#define    GIC_GICD_ITARGETSRN_99_SPI_TARGET_INTID_N_SHIFT                0
#define    GIC_GICD_ITARGETSRN_99_SPI_TARGET_INTID_N_MASK                 0x000000FF

#define GIC_GICD_ITARGETSRN_100_OFFSET                                    0x00001864
#define GIC_GICD_ITARGETSRN_100_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_100_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_100_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_100_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_101_OFFSET                                    0x00001865
#define GIC_GICD_ITARGETSRN_101_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_101_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_101_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_101_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_102_OFFSET                                    0x00001866
#define GIC_GICD_ITARGETSRN_102_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_102_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_102_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_102_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_103_OFFSET                                    0x00001867
#define GIC_GICD_ITARGETSRN_103_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_103_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_103_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_103_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_104_OFFSET                                    0x00001868
#define GIC_GICD_ITARGETSRN_104_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_104_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_104_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_104_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_105_OFFSET                                    0x00001869
#define GIC_GICD_ITARGETSRN_105_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_105_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_105_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_105_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_106_OFFSET                                    0x0000186A
#define GIC_GICD_ITARGETSRN_106_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_106_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_106_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_106_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_107_OFFSET                                    0x0000186B
#define GIC_GICD_ITARGETSRN_107_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_107_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_107_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_107_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_108_OFFSET                                    0x0000186C
#define GIC_GICD_ITARGETSRN_108_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_108_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_108_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_108_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_109_OFFSET                                    0x0000186D
#define GIC_GICD_ITARGETSRN_109_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_109_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_109_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_109_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_110_OFFSET                                    0x0000186E
#define GIC_GICD_ITARGETSRN_110_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_110_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_110_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_110_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_111_OFFSET                                    0x0000186F
#define GIC_GICD_ITARGETSRN_111_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_111_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_111_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_111_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_112_OFFSET                                    0x00001870
#define GIC_GICD_ITARGETSRN_112_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_112_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_112_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_112_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_113_OFFSET                                    0x00001871
#define GIC_GICD_ITARGETSRN_113_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_113_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_113_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_113_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_114_OFFSET                                    0x00001872
#define GIC_GICD_ITARGETSRN_114_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_114_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_114_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_114_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_115_OFFSET                                    0x00001873
#define GIC_GICD_ITARGETSRN_115_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_115_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_115_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_115_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_116_OFFSET                                    0x00001874
#define GIC_GICD_ITARGETSRN_116_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_116_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_116_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_116_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_117_OFFSET                                    0x00001875
#define GIC_GICD_ITARGETSRN_117_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_117_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_117_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_117_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_118_OFFSET                                    0x00001876
#define GIC_GICD_ITARGETSRN_118_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_118_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_118_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_118_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_119_OFFSET                                    0x00001877
#define GIC_GICD_ITARGETSRN_119_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_119_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_119_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_119_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_120_OFFSET                                    0x00001878
#define GIC_GICD_ITARGETSRN_120_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_120_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_120_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_120_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_121_OFFSET                                    0x00001879
#define GIC_GICD_ITARGETSRN_121_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_121_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_121_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_121_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_122_OFFSET                                    0x0000187A
#define GIC_GICD_ITARGETSRN_122_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_122_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_122_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_122_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_123_OFFSET                                    0x0000187B
#define GIC_GICD_ITARGETSRN_123_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_123_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_123_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_123_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_124_OFFSET                                    0x0000187C
#define GIC_GICD_ITARGETSRN_124_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_124_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_124_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_124_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_125_OFFSET                                    0x0000187D
#define GIC_GICD_ITARGETSRN_125_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_125_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_125_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_125_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_126_OFFSET                                    0x0000187E
#define GIC_GICD_ITARGETSRN_126_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_126_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_126_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_126_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_127_OFFSET                                    0x0000187F
#define GIC_GICD_ITARGETSRN_127_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_127_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_127_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_127_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_128_OFFSET                                    0x00001880
#define GIC_GICD_ITARGETSRN_128_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_128_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_128_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_128_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_129_OFFSET                                    0x00001881
#define GIC_GICD_ITARGETSRN_129_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_129_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_129_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_129_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_130_OFFSET                                    0x00001882
#define GIC_GICD_ITARGETSRN_130_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_130_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_130_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_130_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_131_OFFSET                                    0x00001883
#define GIC_GICD_ITARGETSRN_131_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_131_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_131_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_131_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_132_OFFSET                                    0x00001884
#define GIC_GICD_ITARGETSRN_132_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_132_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_132_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_132_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_133_OFFSET                                    0x00001885
#define GIC_GICD_ITARGETSRN_133_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_133_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_133_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_133_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_134_OFFSET                                    0x00001886
#define GIC_GICD_ITARGETSRN_134_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_134_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_134_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_134_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_135_OFFSET                                    0x00001887
#define GIC_GICD_ITARGETSRN_135_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_135_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_135_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_135_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_136_OFFSET                                    0x00001888
#define GIC_GICD_ITARGETSRN_136_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_136_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_136_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_136_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_137_OFFSET                                    0x00001889
#define GIC_GICD_ITARGETSRN_137_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_137_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_137_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_137_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_138_OFFSET                                    0x0000188A
#define GIC_GICD_ITARGETSRN_138_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_138_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_138_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_138_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_139_OFFSET                                    0x0000188B
#define GIC_GICD_ITARGETSRN_139_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_139_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_139_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_139_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_140_OFFSET                                    0x0000188C
#define GIC_GICD_ITARGETSRN_140_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_140_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_140_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_140_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_141_OFFSET                                    0x0000188D
#define GIC_GICD_ITARGETSRN_141_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_141_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_141_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_141_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_142_OFFSET                                    0x0000188E
#define GIC_GICD_ITARGETSRN_142_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_142_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_142_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_142_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_143_OFFSET                                    0x0000188F
#define GIC_GICD_ITARGETSRN_143_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_143_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_143_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_143_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_144_OFFSET                                    0x00001890
#define GIC_GICD_ITARGETSRN_144_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_144_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_144_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_144_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_145_OFFSET                                    0x00001891
#define GIC_GICD_ITARGETSRN_145_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_145_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_145_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_145_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_146_OFFSET                                    0x00001892
#define GIC_GICD_ITARGETSRN_146_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_146_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_146_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_146_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_147_OFFSET                                    0x00001893
#define GIC_GICD_ITARGETSRN_147_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_147_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_147_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_147_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_148_OFFSET                                    0x00001894
#define GIC_GICD_ITARGETSRN_148_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_148_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_148_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_148_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_149_OFFSET                                    0x00001895
#define GIC_GICD_ITARGETSRN_149_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_149_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_149_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_149_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_150_OFFSET                                    0x00001896
#define GIC_GICD_ITARGETSRN_150_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_150_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_150_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_150_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_151_OFFSET                                    0x00001897
#define GIC_GICD_ITARGETSRN_151_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_151_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_151_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_151_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_152_OFFSET                                    0x00001898
#define GIC_GICD_ITARGETSRN_152_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_152_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_152_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_152_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_153_OFFSET                                    0x00001899
#define GIC_GICD_ITARGETSRN_153_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_153_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_153_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_153_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_154_OFFSET                                    0x0000189A
#define GIC_GICD_ITARGETSRN_154_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_154_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_154_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_154_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_155_OFFSET                                    0x0000189B
#define GIC_GICD_ITARGETSRN_155_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_155_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_155_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_155_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_156_OFFSET                                    0x0000189C
#define GIC_GICD_ITARGETSRN_156_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_156_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_156_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_156_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_157_OFFSET                                    0x0000189D
#define GIC_GICD_ITARGETSRN_157_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_157_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_157_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_157_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_158_OFFSET                                    0x0000189E
#define GIC_GICD_ITARGETSRN_158_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_158_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_158_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_158_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_159_OFFSET                                    0x0000189F
#define GIC_GICD_ITARGETSRN_159_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_159_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_159_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_159_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_160_OFFSET                                    0x000018A0
#define GIC_GICD_ITARGETSRN_160_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_160_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_160_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_160_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_161_OFFSET                                    0x000018A1
#define GIC_GICD_ITARGETSRN_161_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_161_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_161_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_161_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_162_OFFSET                                    0x000018A2
#define GIC_GICD_ITARGETSRN_162_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_162_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_162_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_162_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_163_OFFSET                                    0x000018A3
#define GIC_GICD_ITARGETSRN_163_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_163_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_163_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_163_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_164_OFFSET                                    0x000018A4
#define GIC_GICD_ITARGETSRN_164_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_164_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_164_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_164_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_165_OFFSET                                    0x000018A5
#define GIC_GICD_ITARGETSRN_165_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_165_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_165_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_165_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_166_OFFSET                                    0x000018A6
#define GIC_GICD_ITARGETSRN_166_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_166_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_166_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_166_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_167_OFFSET                                    0x000018A7
#define GIC_GICD_ITARGETSRN_167_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_167_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_167_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_167_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_168_OFFSET                                    0x000018A8
#define GIC_GICD_ITARGETSRN_168_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_168_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_168_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_168_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_169_OFFSET                                    0x000018A9
#define GIC_GICD_ITARGETSRN_169_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_169_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_169_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_169_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_170_OFFSET                                    0x000018AA
#define GIC_GICD_ITARGETSRN_170_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_170_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_170_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_170_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_171_OFFSET                                    0x000018AB
#define GIC_GICD_ITARGETSRN_171_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_171_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_171_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_171_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_172_OFFSET                                    0x000018AC
#define GIC_GICD_ITARGETSRN_172_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_172_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_172_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_172_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_173_OFFSET                                    0x000018AD
#define GIC_GICD_ITARGETSRN_173_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_173_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_173_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_173_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_174_OFFSET                                    0x000018AE
#define GIC_GICD_ITARGETSRN_174_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_174_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_174_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_174_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_175_OFFSET                                    0x000018AF
#define GIC_GICD_ITARGETSRN_175_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_175_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_175_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_175_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_176_OFFSET                                    0x000018B0
#define GIC_GICD_ITARGETSRN_176_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_176_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_176_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_176_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_177_OFFSET                                    0x000018B1
#define GIC_GICD_ITARGETSRN_177_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_177_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_177_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_177_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_178_OFFSET                                    0x000018B2
#define GIC_GICD_ITARGETSRN_178_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_178_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_178_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_178_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_179_OFFSET                                    0x000018B3
#define GIC_GICD_ITARGETSRN_179_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_179_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_179_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_179_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_180_OFFSET                                    0x000018B4
#define GIC_GICD_ITARGETSRN_180_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_180_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_180_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_180_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_181_OFFSET                                    0x000018B5
#define GIC_GICD_ITARGETSRN_181_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_181_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_181_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_181_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_182_OFFSET                                    0x000018B6
#define GIC_GICD_ITARGETSRN_182_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_182_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_182_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_182_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_183_OFFSET                                    0x000018B7
#define GIC_GICD_ITARGETSRN_183_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_183_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_183_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_183_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_184_OFFSET                                    0x000018B8
#define GIC_GICD_ITARGETSRN_184_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_184_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_184_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_184_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_185_OFFSET                                    0x000018B9
#define GIC_GICD_ITARGETSRN_185_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_185_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_185_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_185_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_186_OFFSET                                    0x000018BA
#define GIC_GICD_ITARGETSRN_186_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_186_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_186_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_186_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_187_OFFSET                                    0x000018BB
#define GIC_GICD_ITARGETSRN_187_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_187_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_187_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_187_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_188_OFFSET                                    0x000018BC
#define GIC_GICD_ITARGETSRN_188_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_188_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_188_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_188_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_189_OFFSET                                    0x000018BD
#define GIC_GICD_ITARGETSRN_189_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_189_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_189_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_189_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_190_OFFSET                                    0x000018BE
#define GIC_GICD_ITARGETSRN_190_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_190_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_190_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_190_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_191_OFFSET                                    0x000018BF
#define GIC_GICD_ITARGETSRN_191_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_191_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_191_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_191_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_192_OFFSET                                    0x000018C0
#define GIC_GICD_ITARGETSRN_192_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_192_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_192_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_192_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_193_OFFSET                                    0x000018C1
#define GIC_GICD_ITARGETSRN_193_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_193_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_193_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_193_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_194_OFFSET                                    0x000018C2
#define GIC_GICD_ITARGETSRN_194_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_194_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_194_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_194_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_195_OFFSET                                    0x000018C3
#define GIC_GICD_ITARGETSRN_195_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_195_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_195_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_195_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_196_OFFSET                                    0x000018C4
#define GIC_GICD_ITARGETSRN_196_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_196_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_196_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_196_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_197_OFFSET                                    0x000018C5
#define GIC_GICD_ITARGETSRN_197_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_197_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_197_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_197_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_198_OFFSET                                    0x000018C6
#define GIC_GICD_ITARGETSRN_198_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_198_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_198_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_198_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_199_OFFSET                                    0x000018C7
#define GIC_GICD_ITARGETSRN_199_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_199_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_199_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_199_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_200_OFFSET                                    0x000018C8
#define GIC_GICD_ITARGETSRN_200_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_200_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_200_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_200_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_201_OFFSET                                    0x000018C9
#define GIC_GICD_ITARGETSRN_201_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_201_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_201_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_201_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_202_OFFSET                                    0x000018CA
#define GIC_GICD_ITARGETSRN_202_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_202_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_202_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_202_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_203_OFFSET                                    0x000018CB
#define GIC_GICD_ITARGETSRN_203_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_203_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_203_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_203_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_204_OFFSET                                    0x000018CC
#define GIC_GICD_ITARGETSRN_204_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_204_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_204_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_204_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_205_OFFSET                                    0x000018CD
#define GIC_GICD_ITARGETSRN_205_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_205_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_205_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_205_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_206_OFFSET                                    0x000018CE
#define GIC_GICD_ITARGETSRN_206_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_206_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_206_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_206_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_207_OFFSET                                    0x000018CF
#define GIC_GICD_ITARGETSRN_207_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_207_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_207_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_207_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_208_OFFSET                                    0x000018D0
#define GIC_GICD_ITARGETSRN_208_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_208_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_208_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_208_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_209_OFFSET                                    0x000018D1
#define GIC_GICD_ITARGETSRN_209_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_209_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_209_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_209_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_210_OFFSET                                    0x000018D2
#define GIC_GICD_ITARGETSRN_210_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_210_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_210_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_210_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_211_OFFSET                                    0x000018D3
#define GIC_GICD_ITARGETSRN_211_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_211_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_211_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_211_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_212_OFFSET                                    0x000018D4
#define GIC_GICD_ITARGETSRN_212_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_212_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_212_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_212_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_213_OFFSET                                    0x000018D5
#define GIC_GICD_ITARGETSRN_213_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_213_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_213_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_213_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_214_OFFSET                                    0x000018D6
#define GIC_GICD_ITARGETSRN_214_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_214_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_214_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_214_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_215_OFFSET                                    0x000018D7
#define GIC_GICD_ITARGETSRN_215_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_215_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_215_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_215_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_216_OFFSET                                    0x000018D8
#define GIC_GICD_ITARGETSRN_216_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_216_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_216_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_216_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_217_OFFSET                                    0x000018D9
#define GIC_GICD_ITARGETSRN_217_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_217_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_217_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_217_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_218_OFFSET                                    0x000018DA
#define GIC_GICD_ITARGETSRN_218_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_218_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_218_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_218_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_219_OFFSET                                    0x000018DB
#define GIC_GICD_ITARGETSRN_219_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_219_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_219_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_219_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_220_OFFSET                                    0x000018DC
#define GIC_GICD_ITARGETSRN_220_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_220_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_220_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_220_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_221_OFFSET                                    0x000018DD
#define GIC_GICD_ITARGETSRN_221_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_221_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_221_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_221_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_222_OFFSET                                    0x000018DE
#define GIC_GICD_ITARGETSRN_222_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_222_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_222_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_222_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_223_OFFSET                                    0x000018DF
#define GIC_GICD_ITARGETSRN_223_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_223_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_223_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_223_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_224_OFFSET                                    0x000018E0
#define GIC_GICD_ITARGETSRN_224_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_224_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_224_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_224_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_225_OFFSET                                    0x000018E1
#define GIC_GICD_ITARGETSRN_225_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_225_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_225_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_225_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_226_OFFSET                                    0x000018E2
#define GIC_GICD_ITARGETSRN_226_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_226_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_226_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_226_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_227_OFFSET                                    0x000018E3
#define GIC_GICD_ITARGETSRN_227_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_227_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_227_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_227_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_228_OFFSET                                    0x000018E4
#define GIC_GICD_ITARGETSRN_228_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_228_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_228_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_228_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_229_OFFSET                                    0x000018E5
#define GIC_GICD_ITARGETSRN_229_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_229_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_229_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_229_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_230_OFFSET                                    0x000018E6
#define GIC_GICD_ITARGETSRN_230_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_230_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_230_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_230_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_231_OFFSET                                    0x000018E7
#define GIC_GICD_ITARGETSRN_231_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_231_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_231_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_231_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_232_OFFSET                                    0x000018E8
#define GIC_GICD_ITARGETSRN_232_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_232_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_232_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_232_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_233_OFFSET                                    0x000018E9
#define GIC_GICD_ITARGETSRN_233_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_233_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_233_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_233_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_234_OFFSET                                    0x000018EA
#define GIC_GICD_ITARGETSRN_234_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_234_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_234_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_234_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_235_OFFSET                                    0x000018EB
#define GIC_GICD_ITARGETSRN_235_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_235_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_235_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_235_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_236_OFFSET                                    0x000018EC
#define GIC_GICD_ITARGETSRN_236_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_236_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_236_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_236_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_237_OFFSET                                    0x000018ED
#define GIC_GICD_ITARGETSRN_237_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_237_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_237_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_237_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_238_OFFSET                                    0x000018EE
#define GIC_GICD_ITARGETSRN_238_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_238_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_238_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_238_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_239_OFFSET                                    0x000018EF
#define GIC_GICD_ITARGETSRN_239_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_239_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_239_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_239_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_240_OFFSET                                    0x000018F0
#define GIC_GICD_ITARGETSRN_240_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_240_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_240_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_240_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_241_OFFSET                                    0x000018F1
#define GIC_GICD_ITARGETSRN_241_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_241_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_241_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_241_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_242_OFFSET                                    0x000018F2
#define GIC_GICD_ITARGETSRN_242_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_242_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_242_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_242_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_243_OFFSET                                    0x000018F3
#define GIC_GICD_ITARGETSRN_243_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_243_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_243_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_243_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_244_OFFSET                                    0x000018F4
#define GIC_GICD_ITARGETSRN_244_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_244_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_244_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_244_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_245_OFFSET                                    0x000018F5
#define GIC_GICD_ITARGETSRN_245_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_245_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_245_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_245_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_246_OFFSET                                    0x000018F6
#define GIC_GICD_ITARGETSRN_246_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_246_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_246_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_246_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_247_OFFSET                                    0x000018F7
#define GIC_GICD_ITARGETSRN_247_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_247_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_247_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_247_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_248_OFFSET                                    0x000018F8
#define GIC_GICD_ITARGETSRN_248_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_248_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_248_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_248_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_249_OFFSET                                    0x000018F9
#define GIC_GICD_ITARGETSRN_249_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_249_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_249_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_249_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_250_OFFSET                                    0x000018FA
#define GIC_GICD_ITARGETSRN_250_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_250_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_250_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_250_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_251_OFFSET                                    0x000018FB
#define GIC_GICD_ITARGETSRN_251_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_251_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_251_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_251_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_252_OFFSET                                    0x000018FC
#define GIC_GICD_ITARGETSRN_252_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_252_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_252_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_252_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_253_OFFSET                                    0x000018FD
#define GIC_GICD_ITARGETSRN_253_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_253_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_253_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_253_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_254_OFFSET                                    0x000018FE
#define GIC_GICD_ITARGETSRN_254_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_254_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_254_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_254_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ITARGETSRN_255_OFFSET                                    0x000018FF
#define GIC_GICD_ITARGETSRN_255_TYPE                                      UInt8
#define GIC_GICD_ITARGETSRN_255_RESERVED_MASK                             0x00000000
#define    GIC_GICD_ITARGETSRN_255_SPI_TARGET_INTID_N_SHIFT               0
#define    GIC_GICD_ITARGETSRN_255_SPI_TARGET_INTID_N_MASK                0x000000FF

#define GIC_GICD_ICFGRN_0_OFFSET                                          0x00001C00
#define GIC_GICD_ICFGRN_0_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_0_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_0_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_0_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_1_OFFSET                                          0x00001C04
#define GIC_GICD_ICFGRN_1_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_1_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_1_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_1_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_2_OFFSET                                          0x00001C08
#define GIC_GICD_ICFGRN_2_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_2_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_2_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_2_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_3_OFFSET                                          0x00001C0C
#define GIC_GICD_ICFGRN_3_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_3_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_3_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_3_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_4_OFFSET                                          0x00001C10
#define GIC_GICD_ICFGRN_4_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_4_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_4_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_4_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_5_OFFSET                                          0x00001C14
#define GIC_GICD_ICFGRN_5_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_5_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_5_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_5_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_6_OFFSET                                          0x00001C18
#define GIC_GICD_ICFGRN_6_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_6_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_6_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_6_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_7_OFFSET                                          0x00001C1C
#define GIC_GICD_ICFGRN_7_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_7_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_7_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_7_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_8_OFFSET                                          0x00001C20
#define GIC_GICD_ICFGRN_8_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_8_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_8_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_8_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_9_OFFSET                                          0x00001C24
#define GIC_GICD_ICFGRN_9_TYPE                                            UInt32
#define GIC_GICD_ICFGRN_9_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_ICFGRN_9_INT_CONFIG_SHIFT                             0
#define    GIC_GICD_ICFGRN_9_INT_CONFIG_MASK                              0xFFFFFFFF

#define GIC_GICD_ICFGRN_10_OFFSET                                         0x00001C28
#define GIC_GICD_ICFGRN_10_TYPE                                           UInt32
#define GIC_GICD_ICFGRN_10_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_ICFGRN_10_INT_CONFIG_SHIFT                            0
#define    GIC_GICD_ICFGRN_10_INT_CONFIG_MASK                             0xFFFFFFFF

#define GIC_GICD_ICFGRN_11_OFFSET                                         0x00001C2C
#define GIC_GICD_ICFGRN_11_TYPE                                           UInt32
#define GIC_GICD_ICFGRN_11_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_ICFGRN_11_INT_CONFIG_SHIFT                            0
#define    GIC_GICD_ICFGRN_11_INT_CONFIG_MASK                             0xFFFFFFFF

#define GIC_GICD_ICFGRN_12_OFFSET                                         0x00001C30
#define GIC_GICD_ICFGRN_12_TYPE                                           UInt32
#define GIC_GICD_ICFGRN_12_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_ICFGRN_12_INT_CONFIG_SHIFT                            0
#define    GIC_GICD_ICFGRN_12_INT_CONFIG_MASK                             0xFFFFFFFF

#define GIC_GICD_ICFGRN_13_OFFSET                                         0x00001C34
#define GIC_GICD_ICFGRN_13_TYPE                                           UInt32
#define GIC_GICD_ICFGRN_13_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_ICFGRN_13_INT_CONFIG_SHIFT                            0
#define    GIC_GICD_ICFGRN_13_INT_CONFIG_MASK                             0xFFFFFFFF

#define GIC_GICD_ICFGRN_14_OFFSET                                         0x00001C38
#define GIC_GICD_ICFGRN_14_TYPE                                           UInt32
#define GIC_GICD_ICFGRN_14_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_ICFGRN_14_INT_CONFIG_SHIFT                            0
#define    GIC_GICD_ICFGRN_14_INT_CONFIG_MASK                             0xFFFFFFFF

#define GIC_GICD_ICFGRN_15_OFFSET                                         0x00001C3C
#define GIC_GICD_ICFGRN_15_TYPE                                           UInt32
#define GIC_GICD_ICFGRN_15_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_ICFGRN_15_INT_CONFIG_SHIFT                            0
#define    GIC_GICD_ICFGRN_15_INT_CONFIG_MASK                             0xFFFFFFFF

#define GIC_GICD_PPISR_OFFSET                                             0x00001D00
#define GIC_GICD_PPISR_TYPE                                               UInt32
#define GIC_GICD_PPISR_RESERVED_MASK                                      0xFFFF01FF
#define    GIC_GICD_PPISR_PPI_STATUS_SHIFT                                9
#define    GIC_GICD_PPISR_PPI_STATUS_MASK                                 0x0000FE00

#define GIC_GICD_SPISRN_0_OFFSET                                          0x00001D04
#define GIC_GICD_SPISRN_0_TYPE                                            UInt32
#define GIC_GICD_SPISRN_0_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_0_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_0_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_1_OFFSET                                          0x00001D08
#define GIC_GICD_SPISRN_1_TYPE                                            UInt32
#define GIC_GICD_SPISRN_1_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_1_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_1_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_2_OFFSET                                          0x00001D0C
#define GIC_GICD_SPISRN_2_TYPE                                            UInt32
#define GIC_GICD_SPISRN_2_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_2_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_2_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_3_OFFSET                                          0x00001D10
#define GIC_GICD_SPISRN_3_TYPE                                            UInt32
#define GIC_GICD_SPISRN_3_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_3_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_3_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_4_OFFSET                                          0x00001D14
#define GIC_GICD_SPISRN_4_TYPE                                            UInt32
#define GIC_GICD_SPISRN_4_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_4_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_4_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_5_OFFSET                                          0x00001D18
#define GIC_GICD_SPISRN_5_TYPE                                            UInt32
#define GIC_GICD_SPISRN_5_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_5_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_5_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_6_OFFSET                                          0x00001D1C
#define GIC_GICD_SPISRN_6_TYPE                                            UInt32
#define GIC_GICD_SPISRN_6_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_6_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_6_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_7_OFFSET                                          0x00001D20
#define GIC_GICD_SPISRN_7_TYPE                                            UInt32
#define GIC_GICD_SPISRN_7_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_7_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_7_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_8_OFFSET                                          0x00001D24
#define GIC_GICD_SPISRN_8_TYPE                                            UInt32
#define GIC_GICD_SPISRN_8_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_8_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_8_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_9_OFFSET                                          0x00001D28
#define GIC_GICD_SPISRN_9_TYPE                                            UInt32
#define GIC_GICD_SPISRN_9_RESERVED_MASK                                   0x00000000
#define    GIC_GICD_SPISRN_9_SPI_STATUS_SHIFT                             0
#define    GIC_GICD_SPISRN_9_SPI_STATUS_MASK                              0xFFFFFFFF

#define GIC_GICD_SPISRN_10_OFFSET                                         0x00001D2C
#define GIC_GICD_SPISRN_10_TYPE                                           UInt32
#define GIC_GICD_SPISRN_10_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_SPISRN_10_SPI_STATUS_SHIFT                            0
#define    GIC_GICD_SPISRN_10_SPI_STATUS_MASK                             0xFFFFFFFF

#define GIC_GICD_SPISRN_11_OFFSET                                         0x00001D30
#define GIC_GICD_SPISRN_11_TYPE                                           UInt32
#define GIC_GICD_SPISRN_11_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_SPISRN_11_SPI_STATUS_SHIFT                            0
#define    GIC_GICD_SPISRN_11_SPI_STATUS_MASK                             0xFFFFFFFF

#define GIC_GICD_SPISRN_12_OFFSET                                         0x00001D34
#define GIC_GICD_SPISRN_12_TYPE                                           UInt32
#define GIC_GICD_SPISRN_12_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_SPISRN_12_SPI_STATUS_SHIFT                            0
#define    GIC_GICD_SPISRN_12_SPI_STATUS_MASK                             0xFFFFFFFF

#define GIC_GICD_SPISRN_13_OFFSET                                         0x00001D38
#define GIC_GICD_SPISRN_13_TYPE                                           UInt32
#define GIC_GICD_SPISRN_13_RESERVED_MASK                                  0x00000000
#define    GIC_GICD_SPISRN_13_SPI_STATUS_SHIFT                            0
#define    GIC_GICD_SPISRN_13_SPI_STATUS_MASK                             0xFFFFFFFF

#define GIC_GICD_SGIR_OFFSET                                              0x00001F00
#define GIC_GICD_SGIR_TYPE                                                UInt32
#define GIC_GICD_SGIR_RESERVED_MASK                                       0xFC007FF0
#define    GIC_GICD_SGIR_TARGET_LIST_FILTER_SHIFT                         24
#define    GIC_GICD_SGIR_TARGET_LIST_FILTER_MASK                          0x03000000
#define       GIC_GICD_SGIR_TARGET_LIST_FILTER_CMD_SEND_INTERRUPT_SPECIFY 0x00000000
#define       GIC_GICD_SGIR_TARGET_LIST_FILTER_CMD_SEND_INTERRUPT_NO_REQUEST 0x00000001
#define       GIC_GICD_SGIR_TARGET_LIST_FILTER_CMD_SEND_INTERRUPT_REQUEST 0x00000002
#define       GIC_GICD_SGIR_TARGET_LIST_FILTER_CMD_RESERVED               0x00000003
#define    GIC_GICD_SGIR_CPU_TARGET_LIST_SHIFT                            16
#define    GIC_GICD_SGIR_CPU_TARGET_LIST_MASK                             0x00FF0000
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_23                    0x00000080
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_22                    0x00000040
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_21                    0x00000020
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_20                    0x00000010
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_19                    0x00000008
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_18                    0x00000004
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_17                    0x00000002
#define       GIC_GICD_SGIR_CPU_TARGET_LIST_CMD_BIT_16                    0x00000001
#define    GIC_GICD_SGIR_SATT_SHIFT                                       15
#define    GIC_GICD_SGIR_SATT_MASK                                        0x00008000
#define    GIC_GICD_SGIR_STI_INTID_SHIFT                                  0
#define    GIC_GICD_SGIR_STI_INTID_MASK                                   0x0000000F
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID0                          0x00000000
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID1                          0x00000001
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID2                          0x00000002
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID3                          0x00000003
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID4                          0x00000004
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID5                          0x00000005
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID6                          0x00000006
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID7                          0x00000007
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID8                          0x00000008
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID9                          0x00000009
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID10                         0x0000000A
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID11                         0x0000000B
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID12                         0x0000000C
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID13                         0x0000000D
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID14                         0x0000000E
#define       GIC_GICD_SGIR_STI_INTID_CMD_INTID15                         0x0000000F

#define GIC_GICD_CPENDSGIRN_0_OFFSET                                      0x00001F10
#define GIC_GICD_CPENDSGIRN_0_TYPE                                        UInt32
#define GIC_GICD_CPENDSGIRN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_CPENDSGIRN_0_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_CPENDSGIRN_1_OFFSET                                      0x00001F14
#define GIC_GICD_CPENDSGIRN_1_TYPE                                        UInt32
#define GIC_GICD_CPENDSGIRN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_CPENDSGIRN_1_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_CPENDSGIRN_2_OFFSET                                      0x00001F18
#define GIC_GICD_CPENDSGIRN_2_TYPE                                        UInt32
#define GIC_GICD_CPENDSGIRN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_CPENDSGIRN_2_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_CPENDSGIRN_3_OFFSET                                      0x00001F1C
#define GIC_GICD_CPENDSGIRN_3_TYPE                                        UInt32
#define GIC_GICD_CPENDSGIRN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_CPENDSGIRN_3_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_SPENDSGIRN_0_OFFSET                                      0x00001F20
#define GIC_GICD_SPENDSGIRN_0_TYPE                                        UInt32
#define GIC_GICD_SPENDSGIRN_0_RESERVED_MASK                               0x00000000
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_SPENDSGIRN_0_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_SPENDSGIRN_1_OFFSET                                      0x00001F24
#define GIC_GICD_SPENDSGIRN_1_TYPE                                        UInt32
#define GIC_GICD_SPENDSGIRN_1_RESERVED_MASK                               0x00000000
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_SPENDSGIRN_1_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_SPENDSGIRN_2_OFFSET                                      0x00001F28
#define GIC_GICD_SPENDSGIRN_2_TYPE                                        UInt32
#define GIC_GICD_SPENDSGIRN_2_RESERVED_MASK                               0x00000000
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_SPENDSGIRN_2_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_SPENDSGIRN_3_OFFSET                                      0x00001F2C
#define GIC_GICD_SPENDSGIRN_3_TYPE                                        UInt32
#define GIC_GICD_SPENDSGIRN_3_RESERVED_MASK                               0x00000000
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_3_SHIFT                      24
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_3_MASK                       0xFF000000
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_2_SHIFT                      16
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_2_MASK                       0x00FF0000
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_1_SHIFT                      8
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_1_MASK                       0x0000FF00
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_0_SHIFT                      0
#define    GIC_GICD_SPENDSGIRN_3_CLR_PENDING_0_MASK                       0x000000FF

#define GIC_GICD_PIDR4_OFFSET                                             0x00001FD0
#define GIC_GICD_PIDR4_TYPE                                               UInt32
#define GIC_GICD_PIDR4_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR4_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR4_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR5_OFFSET                                             0x00001FD4
#define GIC_GICD_PIDR5_TYPE                                               UInt32
#define GIC_GICD_PIDR5_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR5_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR5_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR6_OFFSET                                             0x00001FD8
#define GIC_GICD_PIDR6_TYPE                                               UInt32
#define GIC_GICD_PIDR6_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR6_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR6_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR7_OFFSET                                             0x00001FDC
#define GIC_GICD_PIDR7_TYPE                                               UInt32
#define GIC_GICD_PIDR7_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR7_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR7_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR0_OFFSET                                             0x00001FE0
#define GIC_GICD_PIDR0_TYPE                                               UInt32
#define GIC_GICD_PIDR0_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR0_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR0_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR1_OFFSET                                             0x00001FE4
#define GIC_GICD_PIDR1_TYPE                                               UInt32
#define GIC_GICD_PIDR1_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR1_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR1_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR2_OFFSET                                             0x00001FE8
#define GIC_GICD_PIDR2_TYPE                                               UInt32
#define GIC_GICD_PIDR2_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR2_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR2_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_PIDR3_OFFSET                                             0x00001FEC
#define GIC_GICD_PIDR3_TYPE                                               UInt32
#define GIC_GICD_PIDR3_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_PIDR3_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_PIDR3_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_CIDR0_OFFSET                                             0x00001FF0
#define GIC_GICD_CIDR0_TYPE                                               UInt32
#define GIC_GICD_CIDR0_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_CIDR0_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_CIDR0_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_CIDR1_OFFSET                                             0x00001FF4
#define GIC_GICD_CIDR1_TYPE                                               UInt32
#define GIC_GICD_CIDR1_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_CIDR1_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_CIDR1_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_CIDR2_OFFSET                                             0x00001FF8
#define GIC_GICD_CIDR2_TYPE                                               UInt32
#define GIC_GICD_CIDR2_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_CIDR2_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_CIDR2_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICD_CIDR3_OFFSET                                             0x00001FFC
#define GIC_GICD_CIDR3_TYPE                                               UInt32
#define GIC_GICD_CIDR3_RESERVED_MASK                                      0xFFFFFF00
#define    GIC_GICD_CIDR3_DESIGNER_0_IMPLEMENT_SHIFT                      0
#define    GIC_GICD_CIDR3_DESIGNER_0_IMPLEMENT_MASK                       0x000000FF

#define GIC_GICC_CTLR_OFFSET                                              0x00002000
#define GIC_GICC_CTLR_TYPE                                                UInt32
#define GIC_GICC_CTLR_RESERVED_MASK                                       0xFFFFFD80
#define    GIC_GICC_CTLR_EOIMODENS_SHIFT                                  9
#define    GIC_GICC_CTLR_EOIMODENS_MASK                                   0x00000200
#define    GIC_GICC_CTLR_IRQBYPDISGRP1_SHIFT                              6
#define    GIC_GICC_CTLR_IRQBYPDISGRP1_MASK                               0x00000040
#define    GIC_GICC_CTLR_FIQBYPDISGRP1_SHIFT                              5
#define    GIC_GICC_CTLR_FIQBYPDISGRP1_MASK                               0x00000020
#define    GIC_GICC_CTLR_CBPR_SHIFT                                       4
#define    GIC_GICC_CTLR_CBPR_MASK                                        0x00000010
#define    GIC_GICC_CTLR_FIQEN_SHIFT                                      3
#define    GIC_GICC_CTLR_FIQEN_MASK                                       0x00000008
#define    GIC_GICC_CTLR_ACKCTL_SHIFT                                     2
#define    GIC_GICC_CTLR_ACKCTL_MASK                                      0x00000004
#define    GIC_GICC_CTLR_ENABLEGRP1_SHIFT                                 1
#define    GIC_GICC_CTLR_ENABLEGRP1_MASK                                  0x00000002
#define    GIC_GICC_CTLR_ENABLEGRP0_SHIFT                                 0
#define    GIC_GICC_CTLR_ENABLEGRP0_MASK                                  0x00000001

#define GIC_GICC_PMR_OFFSET                                               0x00002004
#define GIC_GICC_PMR_TYPE                                                 UInt32
#define GIC_GICC_PMR_RESERVED_MASK                                        0xFFFFFF00
#define    GIC_GICC_PMR_PRIORITY_SHIFT                                    0
#define    GIC_GICC_PMR_PRIORITY_MASK                                     0x000000FF

#define GIC_GICC_BPR_OFFSET                                               0x00002008
#define GIC_GICC_BPR_TYPE                                                 UInt32
#define GIC_GICC_BPR_RESERVED_MASK                                        0xFFFFFFF8
#define    GIC_GICC_BPR_BIN_PT_S_SHIFT                                    0
#define    GIC_GICC_BPR_BIN_PT_S_MASK                                     0x00000007

#define GIC_GICC_IAR_OFFSET                                               0x0000200C
#define GIC_GICC_IAR_TYPE                                                 UInt32
#define GIC_GICC_IAR_RESERVED_MASK                                        0xFFFFE000
#define    GIC_GICC_IAR_SOURCE_CPUID_SHIFT                                10
#define    GIC_GICC_IAR_SOURCE_CPUID_MASK                                 0x00001C00
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU0_STI                      0x00000000
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU1_STI                      0x00000001
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU2_STI                      0x00000002
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU3_STI                      0x00000003
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU4_STI                      0x00000004
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU5_STI                      0x00000005
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU6_STI                      0x00000006
#define       GIC_GICC_IAR_SOURCE_CPUID_CMD_CPU7_STI                      0x00000007
#define    GIC_GICC_IAR_ACK_INTID_SHIFT                                   0
#define    GIC_GICC_IAR_ACK_INTID_MASK                                    0x000003FF

#define GIC_GICC_EOIR_OFFSET                                              0x00002010
#define GIC_GICC_EOIR_TYPE                                                UInt32
#define GIC_GICC_EOIR_RESERVED_MASK                                       0xFFFFE000
#define    GIC_GICC_EOIR_SOURCECPUID_SHIFT                                10
#define    GIC_GICC_EOIR_SOURCECPUID_MASK                                 0x00001C00
#define    GIC_GICC_EOIR_EOI_INTID_SHIFT                                  0
#define    GIC_GICC_EOIR_EOI_INTID_MASK                                   0x000003FF

#define GIC_GICC_RPR_OFFSET                                               0x00002014
#define GIC_GICC_RPR_TYPE                                                 UInt32
#define GIC_GICC_RPR_RESERVED_MASK                                        0xFFFFFF00
#define    GIC_GICC_RPR_PRIORITY_SHIFT                                    0
#define    GIC_GICC_RPR_PRIORITY_MASK                                     0x000000FF

#define GIC_GICC_HPPIR_OFFSET                                             0x00002018
#define GIC_GICC_HPPIR_TYPE                                               UInt32
#define GIC_GICC_HPPIR_RESERVED_MASK                                      0xFFFFE000
#define    GIC_GICC_HPPIR_SOURCECPUID_SHIFT                               10
#define    GIC_GICC_HPPIR_SOURCECPUID_MASK                                0x00001C00
#define    GIC_GICC_HPPIR_PEND_INTID_SHIFT                                0
#define    GIC_GICC_HPPIR_PEND_INTID_MASK                                 0x000003FF

#define GIC_GICC_ABPR_OFFSET                                              0x0000201C
#define GIC_GICC_ABPR_TYPE                                                UInt32
#define GIC_GICC_ABPR_RESERVED_MASK                                       0xFFFFFFF8
#define    GIC_GICC_ABPR_BIN_PT_SHIFT                                     0
#define    GIC_GICC_ABPR_BIN_PT_MASK                                      0x00000007

#define GIC_GICC_AIAR_OFFSET                                              0x00002020
#define GIC_GICC_AIAR_TYPE                                                UInt32
#define GIC_GICC_AIAR_RESERVED_MASK                                       0xFFFFE000
#define    GIC_GICC_AIAR_SOURCECPUID_SHIFT                                10
#define    GIC_GICC_AIAR_SOURCECPUID_MASK                                 0x00001C00
#define    GIC_GICC_AIAR_PEND_INTID_SHIFT                                 0
#define    GIC_GICC_AIAR_PEND_INTID_MASK                                  0x000003FF

#define GIC_GICC_AEOIR_OFFSET                                             0x00002024
#define GIC_GICC_AEOIR_TYPE                                               UInt32
#define GIC_GICC_AEOIR_RESERVED_MASK                                      0xFFFFE000
#define    GIC_GICC_AEOIR_SOURCECPUID_SHIFT                               10
#define    GIC_GICC_AEOIR_SOURCECPUID_MASK                                0x00001C00
#define    GIC_GICC_AEOIR_EOI_INTID_SHIFT                                 0
#define    GIC_GICC_AEOIR_EOI_INTID_MASK                                  0x000003FF

#define GIC_GICC_AHPPIR_OFFSET                                            0x00002028
#define GIC_GICC_AHPPIR_TYPE                                              UInt32
#define GIC_GICC_AHPPIR_RESERVED_MASK                                     0xFFFFE000
#define    GIC_GICC_AHPPIR_SOURCECPUID_SHIFT                              10
#define    GIC_GICC_AHPPIR_SOURCECPUID_MASK                               0x00001C00
#define    GIC_GICC_AHPPIR_PEND_INTID_SHIFT                               0
#define    GIC_GICC_AHPPIR_PEND_INTID_MASK                                0x000003FF

#define GIC_GICC_APR0_OFFSET                                              0x000020D0
#define GIC_GICC_APR0_TYPE                                                UInt32
#define GIC_GICC_APR0_RESERVED_MASK                                       0x00000000
#define    GIC_GICC_APR0_VALUE_SHIFT                                      0
#define    GIC_GICC_APR0_VALUE_MASK                                       0xFFFFFFFF

#define GIC_GICC_NSAPR0_OFFSET                                            0x000020E0
#define GIC_GICC_NSAPR0_TYPE                                              UInt32
#define GIC_GICC_NSAPR0_RESERVED_MASK                                     0x00000000
#define    GIC_GICC_NSAPR0_VALUE_SHIFT                                    0
#define    GIC_GICC_NSAPR0_VALUE_MASK                                     0xFFFFFFFF

#define GIC_GICC_IIDR_OFFSET                                              0x000020FC
#define GIC_GICC_IIDR_TYPE                                                UInt32
#define GIC_GICC_IIDR_RESERVED_MASK                                       0x00000000
#define    GIC_GICC_IIDR_PRODUCTID_SHIFT                                  20
#define    GIC_GICC_IIDR_PRODUCTID_MASK                                   0xFFF00000
#define    GIC_GICC_IIDR_ARCHITECTURE_VERSION_SHIFT                       16
#define    GIC_GICC_IIDR_ARCHITECTURE_VERSION_MASK                        0x000F0000
#define    GIC_GICC_IIDR_REVISION_SHIFT                                   12
#define    GIC_GICC_IIDR_REVISION_MASK                                    0x0000F000
#define    GIC_GICC_IIDR_IMPLEMENTER_SHIFT                                0
#define    GIC_GICC_IIDR_IMPLEMENTER_MASK                                 0x00000FFF

#define GIC_GICC_DIR_OFFSET                                               0x00003000
#define GIC_GICC_DIR_TYPE                                                 UInt32
#define GIC_GICC_DIR_RESERVED_MASK                                        0xFFFFE000
#define    GIC_GICC_DIR_CPUID_SHIFT                                       10
#define    GIC_GICC_DIR_CPUID_MASK                                        0x00001C00
#define    GIC_GICC_DIR_INTERRUPT_ID_SHIFT                                0
#define    GIC_GICC_DIR_INTERRUPT_ID_MASK                                 0x000003FF

#define GIC_GICH_HCR_OFFSET                                               0x00004000
#define GIC_GICH_HCR_TYPE                                                 UInt32
#define GIC_GICH_HCR_RESERVED_MASK                                        0x07FFFF00
#define    GIC_GICH_HCR_EOICOUNT_SHIFT                                    27
#define    GIC_GICH_HCR_EOICOUNT_MASK                                     0xF8000000
#define    GIC_GICH_HCR_VGRP1DIE_SHIFT                                    7
#define    GIC_GICH_HCR_VGRP1DIE_MASK                                     0x00000080
#define    GIC_GICH_HCR_VGRP1EIE_SHIFT                                    6
#define    GIC_GICH_HCR_VGRP1EIE_MASK                                     0x00000040
#define    GIC_GICH_HCR_VGRP0DIE_SHIFT                                    5
#define    GIC_GICH_HCR_VGRP0DIE_MASK                                     0x00000020
#define    GIC_GICH_HCR_VGRP0EIE_SHIFT                                    4
#define    GIC_GICH_HCR_VGRP0EIE_MASK                                     0x00000010
#define    GIC_GICH_HCR_NPIE_SHIFT                                        3
#define    GIC_GICH_HCR_NPIE_MASK                                         0x00000008
#define    GIC_GICH_HCR_LRENPIE_SHIFT                                     2
#define    GIC_GICH_HCR_LRENPIE_MASK                                      0x00000004
#define    GIC_GICH_HCR_UIE_SHIFT                                         1
#define    GIC_GICH_HCR_UIE_MASK                                          0x00000002
#define    GIC_GICH_HCR_EN_SHIFT                                          0
#define    GIC_GICH_HCR_EN_MASK                                           0x00000001

#define GIC_GICH_VTR_OFFSET                                               0x00004004
#define GIC_GICH_VTR_TYPE                                                 UInt32
#define GIC_GICH_VTR_RESERVED_MASK                                        0x03FFFFC0
#define    GIC_GICH_VTR_PRIBITS_SHIFT                                     29
#define    GIC_GICH_VTR_PRIBITS_MASK                                      0xE0000000
#define    GIC_GICH_VTR_PREBITS_SHIFT                                     26
#define    GIC_GICH_VTR_PREBITS_MASK                                      0x1C000000
#define    GIC_GICH_VTR_LISTREGS_SHIFT                                    0
#define    GIC_GICH_VTR_LISTREGS_MASK                                     0x0000003F

#define GIC_GICH_VMCR_OFFSET                                              0x00004008
#define GIC_GICH_VMCR_TYPE                                                UInt32
#define GIC_GICH_VMCR_RESERVED_MASK                                       0x0703FDE0
#define    GIC_GICH_VMCR_VMPRIMASK_SHIFT                                  27
#define    GIC_GICH_VMCR_VMPRIMASK_MASK                                   0xF8000000
#define    GIC_GICH_VMCR_VMBP_SHIFT                                       21
#define    GIC_GICH_VMCR_VMBP_MASK                                        0x00E00000
#define    GIC_GICH_VMCR_VMABP_SHIFT                                      18
#define    GIC_GICH_VMCR_VMABP_MASK                                       0x001C0000
#define    GIC_GICH_VMCR_VEM_SHIFT                                        9
#define    GIC_GICH_VMCR_VEM_MASK                                         0x00000200
#define    GIC_GICH_VMCR_VMCBPR_SHIFT                                     4
#define    GIC_GICH_VMCR_VMCBPR_MASK                                      0x00000010
#define    GIC_GICH_VMCR_VMFIQEN_SHIFT                                    3
#define    GIC_GICH_VMCR_VMFIQEN_MASK                                     0x00000008
#define    GIC_GICH_VMCR_VMACKCTL_SHIFT                                   2
#define    GIC_GICH_VMCR_VMACKCTL_MASK                                    0x00000004
#define    GIC_GICH_VMCR_VMGRP1EN_SHIFT                                   1
#define    GIC_GICH_VMCR_VMGRP1EN_MASK                                    0x00000002
#define    GIC_GICH_VMCR_VMGRP0EN_SHIFT                                   0
#define    GIC_GICH_VMCR_VMGRP0EN_MASK                                    0x00000001

#define GIC_GICH_MISR_OFFSET                                              0x00004010
#define GIC_GICH_MISR_TYPE                                                UInt32
#define GIC_GICH_MISR_RESERVED_MASK                                       0xFFFFFF00
#define    GIC_GICH_MISR_VGRP1D_SHIFT                                     7
#define    GIC_GICH_MISR_VGRP1D_MASK                                      0x00000080
#define    GIC_GICH_MISR_VGRP1E_SHIFT                                     6
#define    GIC_GICH_MISR_VGRP1E_MASK                                      0x00000040
#define    GIC_GICH_MISR_VGRP0D_SHIFT                                     5
#define    GIC_GICH_MISR_VGRP0D_MASK                                      0x00000020
#define    GIC_GICH_MISR_VGRP0E_SHIFT                                     4
#define    GIC_GICH_MISR_VGRP0E_MASK                                      0x00000010
#define    GIC_GICH_MISR_NP_SHIFT                                         3
#define    GIC_GICH_MISR_NP_MASK                                          0x00000008
#define    GIC_GICH_MISR_LRENP_SHIFT                                      2
#define    GIC_GICH_MISR_LRENP_MASK                                       0x00000004
#define    GIC_GICH_MISR_U_SHIFT                                          1
#define    GIC_GICH_MISR_U_MASK                                           0x00000002
#define    GIC_GICH_MISR_EOI_SHIFT                                        0
#define    GIC_GICH_MISR_EOI_MASK                                         0x00000001

#define GIC_GICH_EISR0_OFFSET                                             0x00004020
#define GIC_GICH_EISR0_TYPE                                               UInt32
#define GIC_GICH_EISR0_RESERVED_MASK                                      0x00000000
#define    GIC_GICH_EISR0_EOI_STAT_SHIFT                                  0
#define    GIC_GICH_EISR0_EOI_STAT_MASK                                   0xFFFFFFFF

#define GIC_GICH_ELSR0_OFFSET                                             0x00004030
#define GIC_GICH_ELSR0_TYPE                                               UInt32
#define GIC_GICH_ELSR0_RESERVED_MASK                                      0x00000000
#define    GIC_GICH_ELSR0_STATUS_SHIFT                                    0
#define    GIC_GICH_ELSR0_STATUS_MASK                                     0xFFFFFFFF

#define GIC_GICH_APR0_OFFSET                                              0x000040F0
#define GIC_GICH_APR0_TYPE                                                UInt32
#define GIC_GICH_APR0_RESERVED_MASK                                       0x00000000
#define    GIC_GICH_APR0_ACT_PRIORITY_SHIFT                               0
#define    GIC_GICH_APR0_ACT_PRIORITY_MASK                                0xFFFFFFFF

#define GIC_GICH_LR0_OFFSET                                               0x00004100
#define GIC_GICH_LR0_TYPE                                                 UInt32
#define GIC_GICH_LR0_RESERVED_MASK                                        0x00700000
#define    GIC_GICH_LR0_HW_SHIFT                                          31
#define    GIC_GICH_LR0_HW_MASK                                           0x80000000
#define    GIC_GICH_LR0_GRP1_SHIFT                                        30
#define    GIC_GICH_LR0_GRP1_MASK                                         0x40000000
#define    GIC_GICH_LR0_STATE_SHIFT                                       28
#define    GIC_GICH_LR0_STATE_MASK                                        0x30000000
#define    GIC_GICH_LR0_PRIORITY_SHIFT                                    23
#define    GIC_GICH_LR0_PRIORITY_MASK                                     0x0F800000
#define    GIC_GICH_LR0_PHYSICALID_SHIFT                                  10
#define    GIC_GICH_LR0_PHYSICALID_MASK                                   0x000FFC00
#define    GIC_GICH_LR0_VIRTUALID_SHIFT                                   0
#define    GIC_GICH_LR0_VIRTUALID_MASK                                    0x000003FF

#define GIC_GICH_LR1_OFFSET                                               0x00004104
#define GIC_GICH_LR1_TYPE                                                 UInt32
#define GIC_GICH_LR1_RESERVED_MASK                                        0x00700000
#define    GIC_GICH_LR1_HW_SHIFT                                          31
#define    GIC_GICH_LR1_HW_MASK                                           0x80000000
#define    GIC_GICH_LR1_GRP1_SHIFT                                        30
#define    GIC_GICH_LR1_GRP1_MASK                                         0x40000000
#define    GIC_GICH_LR1_STATE_SHIFT                                       28
#define    GIC_GICH_LR1_STATE_MASK                                        0x30000000
#define    GIC_GICH_LR1_PRIORITY_SHIFT                                    23
#define    GIC_GICH_LR1_PRIORITY_MASK                                     0x0F800000
#define    GIC_GICH_LR1_PHYSICALID_SHIFT                                  10
#define    GIC_GICH_LR1_PHYSICALID_MASK                                   0x000FFC00
#define    GIC_GICH_LR1_VIRTUALID_SHIFT                                   0
#define    GIC_GICH_LR1_VIRTUALID_MASK                                    0x000003FF

#define GIC_GICH_LR2_OFFSET                                               0x00004108
#define GIC_GICH_LR2_TYPE                                                 UInt32
#define GIC_GICH_LR2_RESERVED_MASK                                        0x00700000
#define    GIC_GICH_LR2_HW_SHIFT                                          31
#define    GIC_GICH_LR2_HW_MASK                                           0x80000000
#define    GIC_GICH_LR2_GRP1_SHIFT                                        30
#define    GIC_GICH_LR2_GRP1_MASK                                         0x40000000
#define    GIC_GICH_LR2_STATE_SHIFT                                       28
#define    GIC_GICH_LR2_STATE_MASK                                        0x30000000
#define    GIC_GICH_LR2_PRIORITY_SHIFT                                    23
#define    GIC_GICH_LR2_PRIORITY_MASK                                     0x0F800000
#define    GIC_GICH_LR2_PHYSICALID_SHIFT                                  10
#define    GIC_GICH_LR2_PHYSICALID_MASK                                   0x000FFC00
#define    GIC_GICH_LR2_VIRTUALID_SHIFT                                   0
#define    GIC_GICH_LR2_VIRTUALID_MASK                                    0x000003FF

#define GIC_GICH_LR3_OFFSET                                               0x0000410C
#define GIC_GICH_LR3_TYPE                                                 UInt32
#define GIC_GICH_LR3_RESERVED_MASK                                        0x00700000
#define    GIC_GICH_LR3_HW_SHIFT                                          31
#define    GIC_GICH_LR3_HW_MASK                                           0x80000000
#define    GIC_GICH_LR3_GRP1_SHIFT                                        30
#define    GIC_GICH_LR3_GRP1_MASK                                         0x40000000
#define    GIC_GICH_LR3_STATE_SHIFT                                       28
#define    GIC_GICH_LR3_STATE_MASK                                        0x30000000
#define    GIC_GICH_LR3_PRIORITY_SHIFT                                    23
#define    GIC_GICH_LR3_PRIORITY_MASK                                     0x0F800000
#define    GIC_GICH_LR3_PHYSICALID_SHIFT                                  10
#define    GIC_GICH_LR3_PHYSICALID_MASK                                   0x000FFC00
#define    GIC_GICH_LR3_VIRTUALID_SHIFT                                   0
#define    GIC_GICH_LR3_VIRTUALID_MASK                                    0x000003FF

#define GIC_GICH_HCR_CPU0_OFFSET                                          0x00005000
#define GIC_GICH_HCR_CPU0_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU0_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU0_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU0_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU0_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU0_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU0_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU0_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU0_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU0_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU0_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU0_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU0_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU0_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU0_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU0_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU0_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU0_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU0_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU0_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU0_OFFSET                                          0x00005004
#define GIC_GICH_VTR_CPU0_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU0_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU0_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU0_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU0_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU0_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU0_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU0_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU0_OFFSET                                         0x00005008
#define GIC_GICH_VMCR_CPU0_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU0_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU0_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU0_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU0_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU0_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU0_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU0_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU0_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU0_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU0_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU0_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU0_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU0_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU0_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU0_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU0_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU0_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU0_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU0_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU0_OFFSET                                         0x00005010
#define GIC_GICH_MISR_CPU0_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU0_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU0_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU0_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU0_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU0_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU0_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU0_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU0_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU0_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU0_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU0_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU0_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU0_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU0_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU0_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU0_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU0_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU0_OFFSET                                        0x00005020
#define GIC_GICH_EISR0_CPU0_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU0_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU0_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU0_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU0_OFFSET                                        0x00005030
#define GIC_GICH_ELSR0_CPU0_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU0_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU0_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU0_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU0_OFFSET                                         0x000050F0
#define GIC_GICH_APR0_CPU0_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU0_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU0_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU0_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU0_OFFSET                                          0x00005100
#define GIC_GICH_LR0_CPU0_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU0_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU0_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU0_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU0_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU0_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU0_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU0_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU0_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU0_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU0_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU0_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU0_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU0_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU0_OFFSET                                          0x00005104
#define GIC_GICH_LR1_CPU0_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU0_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU0_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU0_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU0_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU0_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU0_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU0_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU0_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU0_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU0_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU0_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU0_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU0_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU0_OFFSET                                          0x00005108
#define GIC_GICH_LR2_CPU0_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU0_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU0_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU0_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU0_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU0_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU0_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU0_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU0_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU0_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU0_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU0_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU0_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU0_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU0_OFFSET                                          0x0000510C
#define GIC_GICH_LR3_CPU0_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU0_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU0_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU0_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU0_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU0_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU0_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU0_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU0_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU0_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU0_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU0_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU0_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU0_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU1_OFFSET                                          0x00005200
#define GIC_GICH_HCR_CPU1_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU1_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU1_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU1_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU1_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU1_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU1_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU1_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU1_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU1_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU1_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU1_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU1_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU1_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU1_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU1_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU1_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU1_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU1_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU1_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU1_OFFSET                                          0x00005204
#define GIC_GICH_VTR_CPU1_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU1_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU1_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU1_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU1_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU1_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU1_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU1_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU1_OFFSET                                         0x00005208
#define GIC_GICH_VMCR_CPU1_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU1_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU1_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU1_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU1_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU1_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU1_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU1_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU1_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU1_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU1_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU1_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU1_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU1_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU1_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU1_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU1_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU1_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU1_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU1_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU1_OFFSET                                         0x00005210
#define GIC_GICH_MISR_CPU1_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU1_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU1_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU1_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU1_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU1_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU1_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU1_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU1_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU1_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU1_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU1_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU1_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU1_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU1_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU1_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU1_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU1_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU1_OFFSET                                        0x00005220
#define GIC_GICH_EISR0_CPU1_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU1_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU1_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU1_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU1_OFFSET                                        0x00005230
#define GIC_GICH_ELSR0_CPU1_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU1_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU1_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU1_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU1_OFFSET                                         0x000052F0
#define GIC_GICH_APR0_CPU1_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU1_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU1_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU1_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU1_OFFSET                                          0x00005300
#define GIC_GICH_LR0_CPU1_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU1_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU1_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU1_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU1_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU1_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU1_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU1_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU1_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU1_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU1_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU1_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU1_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU1_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU1_OFFSET                                          0x00005304
#define GIC_GICH_LR1_CPU1_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU1_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU1_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU1_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU1_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU1_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU1_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU1_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU1_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU1_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU1_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU1_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU1_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU1_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU1_OFFSET                                          0x00005308
#define GIC_GICH_LR2_CPU1_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU1_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU1_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU1_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU1_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU1_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU1_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU1_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU1_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU1_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU1_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU1_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU1_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU1_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU1_OFFSET                                          0x0000530C
#define GIC_GICH_LR3_CPU1_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU1_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU1_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU1_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU1_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU1_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU1_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU1_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU1_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU1_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU1_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU1_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU1_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU1_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU2_OFFSET                                          0x00005400
#define GIC_GICH_HCR_CPU2_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU2_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU2_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU2_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU2_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU2_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU2_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU2_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU2_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU2_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU2_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU2_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU2_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU2_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU2_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU2_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU2_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU2_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU2_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU2_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU2_OFFSET                                          0x00005404
#define GIC_GICH_VTR_CPU2_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU2_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU2_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU2_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU2_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU2_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU2_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU2_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU2_OFFSET                                         0x00005408
#define GIC_GICH_VMCR_CPU2_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU2_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU2_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU2_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU2_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU2_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU2_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU2_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU2_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU2_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU2_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU2_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU2_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU2_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU2_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU2_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU2_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU2_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU2_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU2_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU2_OFFSET                                         0x00005410
#define GIC_GICH_MISR_CPU2_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU2_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU2_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU2_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU2_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU2_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU2_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU2_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU2_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU2_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU2_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU2_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU2_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU2_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU2_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU2_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU2_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU2_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU2_OFFSET                                        0x00005420
#define GIC_GICH_EISR0_CPU2_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU2_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU2_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU2_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU2_OFFSET                                        0x00005430
#define GIC_GICH_ELSR0_CPU2_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU2_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU2_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU2_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU2_OFFSET                                         0x000054F0
#define GIC_GICH_APR0_CPU2_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU2_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU2_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU2_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU2_OFFSET                                          0x00005500
#define GIC_GICH_LR0_CPU2_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU2_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU2_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU2_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU2_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU2_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU2_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU2_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU2_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU2_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU2_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU2_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU2_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU2_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU2_OFFSET                                          0x00005504
#define GIC_GICH_LR1_CPU2_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU2_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU2_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU2_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU2_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU2_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU2_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU2_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU2_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU2_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU2_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU2_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU2_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU2_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU2_OFFSET                                          0x00005508
#define GIC_GICH_LR2_CPU2_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU2_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU2_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU2_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU2_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU2_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU2_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU2_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU2_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU2_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU2_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU2_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU2_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU2_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU2_OFFSET                                          0x0000550C
#define GIC_GICH_LR3_CPU2_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU2_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU2_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU2_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU2_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU2_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU2_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU2_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU2_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU2_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU2_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU2_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU2_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU2_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU3_OFFSET                                          0x00005600
#define GIC_GICH_HCR_CPU3_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU3_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU3_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU3_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU3_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU3_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU3_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU3_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU3_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU3_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU3_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU3_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU3_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU3_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU3_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU3_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU3_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU3_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU3_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU3_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU3_OFFSET                                          0x00005604
#define GIC_GICH_VTR_CPU3_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU3_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU3_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU3_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU3_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU3_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU3_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU3_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU3_OFFSET                                         0x00005608
#define GIC_GICH_VMCR_CPU3_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU3_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU3_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU3_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU3_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU3_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU3_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU3_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU3_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU3_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU3_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU3_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU3_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU3_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU3_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU3_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU3_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU3_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU3_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU3_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU3_OFFSET                                         0x00005610
#define GIC_GICH_MISR_CPU3_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU3_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU3_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU3_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU3_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU3_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU3_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU3_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU3_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU3_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU3_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU3_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU3_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU3_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU3_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU3_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU3_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU3_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU3_OFFSET                                        0x00005620
#define GIC_GICH_EISR0_CPU3_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU3_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU3_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU3_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU3_OFFSET                                        0x00005630
#define GIC_GICH_ELSR0_CPU3_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU3_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU3_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU3_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU3_OFFSET                                         0x000056F0
#define GIC_GICH_APR0_CPU3_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU3_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU3_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU3_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU3_OFFSET                                          0x00005700
#define GIC_GICH_LR0_CPU3_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU3_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU3_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU3_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU3_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU3_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU3_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU3_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU3_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU3_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU3_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU3_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU3_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU3_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU3_OFFSET                                          0x00005704
#define GIC_GICH_LR1_CPU3_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU3_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU3_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU3_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU3_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU3_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU3_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU3_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU3_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU3_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU3_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU3_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU3_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU3_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU3_OFFSET                                          0x00005708
#define GIC_GICH_LR2_CPU3_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU3_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU3_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU3_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU3_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU3_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU3_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU3_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU3_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU3_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU3_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU3_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU3_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU3_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU3_OFFSET                                          0x0000570C
#define GIC_GICH_LR3_CPU3_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU3_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU3_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU3_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU3_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU3_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU3_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU3_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU3_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU3_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU3_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU3_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU3_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU3_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU4_OFFSET                                          0x00005800
#define GIC_GICH_HCR_CPU4_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU4_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU4_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU4_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU4_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU4_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU4_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU4_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU4_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU4_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU4_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU4_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU4_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU4_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU4_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU4_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU4_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU4_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU4_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU4_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU4_OFFSET                                          0x00005804
#define GIC_GICH_VTR_CPU4_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU4_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU4_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU4_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU4_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU4_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU4_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU4_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU4_OFFSET                                         0x00005808
#define GIC_GICH_VMCR_CPU4_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU4_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU4_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU4_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU4_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU4_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU4_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU4_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU4_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU4_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU4_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU4_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU4_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU4_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU4_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU4_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU4_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU4_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU4_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU4_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU4_OFFSET                                         0x00005810
#define GIC_GICH_MISR_CPU4_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU4_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU4_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU4_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU4_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU4_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU4_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU4_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU4_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU4_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU4_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU4_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU4_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU4_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU4_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU4_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU4_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU4_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU4_OFFSET                                        0x00005820
#define GIC_GICH_EISR0_CPU4_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU4_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU4_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU4_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU4_OFFSET                                        0x00005830
#define GIC_GICH_ELSR0_CPU4_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU4_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU4_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU4_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU4_OFFSET                                         0x000058F0
#define GIC_GICH_APR0_CPU4_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU4_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU4_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU4_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU4_OFFSET                                          0x00005900
#define GIC_GICH_LR0_CPU4_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU4_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU4_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU4_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU4_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU4_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU4_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU4_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU4_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU4_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU4_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU4_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU4_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU4_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU4_OFFSET                                          0x00005904
#define GIC_GICH_LR1_CPU4_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU4_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU4_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU4_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU4_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU4_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU4_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU4_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU4_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU4_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU4_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU4_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU4_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU4_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU4_OFFSET                                          0x00005908
#define GIC_GICH_LR2_CPU4_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU4_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU4_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU4_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU4_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU4_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU4_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU4_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU4_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU4_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU4_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU4_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU4_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU4_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU4_OFFSET                                          0x0000590C
#define GIC_GICH_LR3_CPU4_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU4_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU4_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU4_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU4_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU4_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU4_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU4_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU4_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU4_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU4_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU4_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU4_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU4_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU5_OFFSET                                          0x00005A00
#define GIC_GICH_HCR_CPU5_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU5_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU5_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU5_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU5_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU5_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU5_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU5_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU5_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU5_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU5_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU5_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU5_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU5_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU5_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU5_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU5_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU5_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU5_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU5_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU5_OFFSET                                          0x00005A04
#define GIC_GICH_VTR_CPU5_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU5_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU5_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU5_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU5_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU5_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU5_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU5_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU5_OFFSET                                         0x00005A08
#define GIC_GICH_VMCR_CPU5_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU5_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU5_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU5_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU5_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU5_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU5_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU5_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU5_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU5_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU5_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU5_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU5_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU5_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU5_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU5_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU5_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU5_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU5_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU5_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU5_OFFSET                                         0x00005A10
#define GIC_GICH_MISR_CPU5_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU5_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU5_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU5_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU5_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU5_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU5_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU5_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU5_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU5_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU5_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU5_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU5_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU5_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU5_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU5_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU5_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU5_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU5_OFFSET                                        0x00005A20
#define GIC_GICH_EISR0_CPU5_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU5_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU5_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU5_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU5_OFFSET                                        0x00005A30
#define GIC_GICH_ELSR0_CPU5_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU5_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU5_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU5_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU5_OFFSET                                         0x00005AF0
#define GIC_GICH_APR0_CPU5_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU5_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU5_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU5_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU5_OFFSET                                          0x00005B00
#define GIC_GICH_LR0_CPU5_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU5_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU5_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU5_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU5_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU5_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU5_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU5_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU5_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU5_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU5_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU5_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU5_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU5_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU5_OFFSET                                          0x00005B04
#define GIC_GICH_LR1_CPU5_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU5_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU5_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU5_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU5_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU5_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU5_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU5_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU5_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU5_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU5_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU5_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU5_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU5_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU5_OFFSET                                          0x00005B08
#define GIC_GICH_LR2_CPU5_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU5_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU5_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU5_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU5_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU5_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU5_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU5_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU5_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU5_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU5_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU5_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU5_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU5_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU5_OFFSET                                          0x00005B0C
#define GIC_GICH_LR3_CPU5_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU5_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU5_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU5_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU5_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU5_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU5_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU5_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU5_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU5_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU5_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU5_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU5_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU5_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU6_OFFSET                                          0x00005C00
#define GIC_GICH_HCR_CPU6_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU6_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU6_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU6_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU6_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU6_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU6_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU6_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU6_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU6_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU6_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU6_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU6_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU6_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU6_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU6_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU6_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU6_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU6_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU6_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU6_OFFSET                                          0x00005C04
#define GIC_GICH_VTR_CPU6_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU6_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU6_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU6_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU6_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU6_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU6_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU6_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU6_OFFSET                                         0x00005C08
#define GIC_GICH_VMCR_CPU6_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU6_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU6_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU6_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU6_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU6_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU6_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU6_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU6_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU6_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU6_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU6_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU6_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU6_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU6_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU6_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU6_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU6_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU6_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU6_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU6_OFFSET                                         0x00005C10
#define GIC_GICH_MISR_CPU6_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU6_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU6_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU6_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU6_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU6_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU6_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU6_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU6_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU6_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU6_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU6_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU6_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU6_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU6_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU6_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU6_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU6_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU6_OFFSET                                        0x00005C20
#define GIC_GICH_EISR0_CPU6_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU6_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU6_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU6_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU6_OFFSET                                        0x00005C30
#define GIC_GICH_ELSR0_CPU6_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU6_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU6_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU6_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU6_OFFSET                                         0x00005CF0
#define GIC_GICH_APR0_CPU6_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU6_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU6_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU6_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU6_OFFSET                                          0x00005D00
#define GIC_GICH_LR0_CPU6_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU6_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU6_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU6_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU6_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU6_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU6_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU6_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU6_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU6_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU6_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU6_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU6_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU6_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU6_OFFSET                                          0x00005D04
#define GIC_GICH_LR1_CPU6_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU6_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU6_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU6_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU6_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU6_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU6_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU6_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU6_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU6_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU6_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU6_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU6_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU6_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU6_OFFSET                                          0x00005D08
#define GIC_GICH_LR2_CPU6_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU6_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU6_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU6_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU6_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU6_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU6_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU6_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU6_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU6_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU6_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU6_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU6_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU6_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU6_OFFSET                                          0x00005D0C
#define GIC_GICH_LR3_CPU6_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU6_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU6_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU6_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU6_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU6_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU6_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU6_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU6_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU6_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU6_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU6_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU6_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU6_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_HCR_CPU7_OFFSET                                          0x00005E00
#define GIC_GICH_HCR_CPU7_TYPE                                            UInt32
#define GIC_GICH_HCR_CPU7_RESERVED_MASK                                   0x07FFFF00
#define    GIC_GICH_HCR_CPU7_EOICOUNT_SHIFT                               27
#define    GIC_GICH_HCR_CPU7_EOICOUNT_MASK                                0xF8000000
#define    GIC_GICH_HCR_CPU7_VGRP1DIE_SHIFT                               7
#define    GIC_GICH_HCR_CPU7_VGRP1DIE_MASK                                0x00000080
#define    GIC_GICH_HCR_CPU7_VGRP1EIE_SHIFT                               6
#define    GIC_GICH_HCR_CPU7_VGRP1EIE_MASK                                0x00000040
#define    GIC_GICH_HCR_CPU7_VGRP0DIE_SHIFT                               5
#define    GIC_GICH_HCR_CPU7_VGRP0DIE_MASK                                0x00000020
#define    GIC_GICH_HCR_CPU7_VGRP0EIE_SHIFT                               4
#define    GIC_GICH_HCR_CPU7_VGRP0EIE_MASK                                0x00000010
#define    GIC_GICH_HCR_CPU7_NPIE_SHIFT                                   3
#define    GIC_GICH_HCR_CPU7_NPIE_MASK                                    0x00000008
#define    GIC_GICH_HCR_CPU7_LRENPIE_SHIFT                                2
#define    GIC_GICH_HCR_CPU7_LRENPIE_MASK                                 0x00000004
#define    GIC_GICH_HCR_CPU7_UIE_SHIFT                                    1
#define    GIC_GICH_HCR_CPU7_UIE_MASK                                     0x00000002
#define    GIC_GICH_HCR_CPU7_EN_SHIFT                                     0
#define    GIC_GICH_HCR_CPU7_EN_MASK                                      0x00000001

#define GIC_GICH_VTR_CPU7_OFFSET                                          0x00005E04
#define GIC_GICH_VTR_CPU7_TYPE                                            UInt32
#define GIC_GICH_VTR_CPU7_RESERVED_MASK                                   0x03FFFFC0
#define    GIC_GICH_VTR_CPU7_PRIBITS_SHIFT                                29
#define    GIC_GICH_VTR_CPU7_PRIBITS_MASK                                 0xE0000000
#define    GIC_GICH_VTR_CPU7_PREBITS_SHIFT                                26
#define    GIC_GICH_VTR_CPU7_PREBITS_MASK                                 0x1C000000
#define    GIC_GICH_VTR_CPU7_LISTREGS_SHIFT                               0
#define    GIC_GICH_VTR_CPU7_LISTREGS_MASK                                0x0000003F

#define GIC_GICH_VMCR_CPU7_OFFSET                                         0x00005E08
#define GIC_GICH_VMCR_CPU7_TYPE                                           UInt32
#define GIC_GICH_VMCR_CPU7_RESERVED_MASK                                  0x0703FDE0
#define    GIC_GICH_VMCR_CPU7_VMPRIMASK_SHIFT                             27
#define    GIC_GICH_VMCR_CPU7_VMPRIMASK_MASK                              0xF8000000
#define    GIC_GICH_VMCR_CPU7_VMBP_SHIFT                                  21
#define    GIC_GICH_VMCR_CPU7_VMBP_MASK                                   0x00E00000
#define    GIC_GICH_VMCR_CPU7_VMABP_SHIFT                                 18
#define    GIC_GICH_VMCR_CPU7_VMABP_MASK                                  0x001C0000
#define    GIC_GICH_VMCR_CPU7_VEM_SHIFT                                   9
#define    GIC_GICH_VMCR_CPU7_VEM_MASK                                    0x00000200
#define    GIC_GICH_VMCR_CPU7_VMCBPR_SHIFT                                4
#define    GIC_GICH_VMCR_CPU7_VMCBPR_MASK                                 0x00000010
#define    GIC_GICH_VMCR_CPU7_VMFIQEN_SHIFT                               3
#define    GIC_GICH_VMCR_CPU7_VMFIQEN_MASK                                0x00000008
#define    GIC_GICH_VMCR_CPU7_VMACKCTL_SHIFT                              2
#define    GIC_GICH_VMCR_CPU7_VMACKCTL_MASK                               0x00000004
#define    GIC_GICH_VMCR_CPU7_VMGRP1EN_SHIFT                              1
#define    GIC_GICH_VMCR_CPU7_VMGRP1EN_MASK                               0x00000002
#define    GIC_GICH_VMCR_CPU7_VMGRP0EN_SHIFT                              0
#define    GIC_GICH_VMCR_CPU7_VMGRP0EN_MASK                               0x00000001

#define GIC_GICH_MISR_CPU7_OFFSET                                         0x00005E10
#define GIC_GICH_MISR_CPU7_TYPE                                           UInt32
#define GIC_GICH_MISR_CPU7_RESERVED_MASK                                  0xFFFFFF00
#define    GIC_GICH_MISR_CPU7_VGRP1D_SHIFT                                7
#define    GIC_GICH_MISR_CPU7_VGRP1D_MASK                                 0x00000080
#define    GIC_GICH_MISR_CPU7_VGRP1E_SHIFT                                6
#define    GIC_GICH_MISR_CPU7_VGRP1E_MASK                                 0x00000040
#define    GIC_GICH_MISR_CPU7_VGRP0D_SHIFT                                5
#define    GIC_GICH_MISR_CPU7_VGRP0D_MASK                                 0x00000020
#define    GIC_GICH_MISR_CPU7_VGRP0E_SHIFT                                4
#define    GIC_GICH_MISR_CPU7_VGRP0E_MASK                                 0x00000010
#define    GIC_GICH_MISR_CPU7_NP_SHIFT                                    3
#define    GIC_GICH_MISR_CPU7_NP_MASK                                     0x00000008
#define    GIC_GICH_MISR_CPU7_LRENP_SHIFT                                 2
#define    GIC_GICH_MISR_CPU7_LRENP_MASK                                  0x00000004
#define    GIC_GICH_MISR_CPU7_U_SHIFT                                     1
#define    GIC_GICH_MISR_CPU7_U_MASK                                      0x00000002
#define    GIC_GICH_MISR_CPU7_EOI_SHIFT                                   0
#define    GIC_GICH_MISR_CPU7_EOI_MASK                                    0x00000001

#define GIC_GICH_EISR0_CPU7_OFFSET                                        0x00005E20
#define GIC_GICH_EISR0_CPU7_TYPE                                          UInt32
#define GIC_GICH_EISR0_CPU7_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_EISR0_CPU7_EOI_STAT_SHIFT                             0
#define    GIC_GICH_EISR0_CPU7_EOI_STAT_MASK                              0xFFFFFFFF

#define GIC_GICH_ELSR0_CPU7_OFFSET                                        0x00005E30
#define GIC_GICH_ELSR0_CPU7_TYPE                                          UInt32
#define GIC_GICH_ELSR0_CPU7_RESERVED_MASK                                 0x00000000
#define    GIC_GICH_ELSR0_CPU7_STATUS_SHIFT                               0
#define    GIC_GICH_ELSR0_CPU7_STATUS_MASK                                0xFFFFFFFF

#define GIC_GICH_APR0_CPU7_OFFSET                                         0x00005EF0
#define GIC_GICH_APR0_CPU7_TYPE                                           UInt32
#define GIC_GICH_APR0_CPU7_RESERVED_MASK                                  0x00000000
#define    GIC_GICH_APR0_CPU7_ACT_PRIORITY_SHIFT                          0
#define    GIC_GICH_APR0_CPU7_ACT_PRIORITY_MASK                           0xFFFFFFFF

#define GIC_GICH_LR0_CPU7_OFFSET                                          0x00005F00
#define GIC_GICH_LR0_CPU7_TYPE                                            UInt32
#define GIC_GICH_LR0_CPU7_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR0_CPU7_HW_SHIFT                                     31
#define    GIC_GICH_LR0_CPU7_HW_MASK                                      0x80000000
#define    GIC_GICH_LR0_CPU7_GRP1_SHIFT                                   30
#define    GIC_GICH_LR0_CPU7_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR0_CPU7_STATE_SHIFT                                  28
#define    GIC_GICH_LR0_CPU7_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR0_CPU7_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR0_CPU7_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR0_CPU7_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR0_CPU7_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR0_CPU7_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR0_CPU7_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR1_CPU7_OFFSET                                          0x00005F04
#define GIC_GICH_LR1_CPU7_TYPE                                            UInt32
#define GIC_GICH_LR1_CPU7_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR1_CPU7_HW_SHIFT                                     31
#define    GIC_GICH_LR1_CPU7_HW_MASK                                      0x80000000
#define    GIC_GICH_LR1_CPU7_GRP1_SHIFT                                   30
#define    GIC_GICH_LR1_CPU7_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR1_CPU7_STATE_SHIFT                                  28
#define    GIC_GICH_LR1_CPU7_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR1_CPU7_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR1_CPU7_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR1_CPU7_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR1_CPU7_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR1_CPU7_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR1_CPU7_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR2_CPU7_OFFSET                                          0x00005F08
#define GIC_GICH_LR2_CPU7_TYPE                                            UInt32
#define GIC_GICH_LR2_CPU7_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR2_CPU7_HW_SHIFT                                     31
#define    GIC_GICH_LR2_CPU7_HW_MASK                                      0x80000000
#define    GIC_GICH_LR2_CPU7_GRP1_SHIFT                                   30
#define    GIC_GICH_LR2_CPU7_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR2_CPU7_STATE_SHIFT                                  28
#define    GIC_GICH_LR2_CPU7_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR2_CPU7_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR2_CPU7_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR2_CPU7_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR2_CPU7_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR2_CPU7_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR2_CPU7_VIRTUALID_MASK                               0x000003FF

#define GIC_GICH_LR3_CPU7_OFFSET                                          0x00005F0C
#define GIC_GICH_LR3_CPU7_TYPE                                            UInt32
#define GIC_GICH_LR3_CPU7_RESERVED_MASK                                   0x00700000
#define    GIC_GICH_LR3_CPU7_HW_SHIFT                                     31
#define    GIC_GICH_LR3_CPU7_HW_MASK                                      0x80000000
#define    GIC_GICH_LR3_CPU7_GRP1_SHIFT                                   30
#define    GIC_GICH_LR3_CPU7_GRP1_MASK                                    0x40000000
#define    GIC_GICH_LR3_CPU7_STATE_SHIFT                                  28
#define    GIC_GICH_LR3_CPU7_STATE_MASK                                   0x30000000
#define    GIC_GICH_LR3_CPU7_PRIORITY_SHIFT                               23
#define    GIC_GICH_LR3_CPU7_PRIORITY_MASK                                0x0F800000
#define    GIC_GICH_LR3_CPU7_PHYSICALID_SHIFT                             10
#define    GIC_GICH_LR3_CPU7_PHYSICALID_MASK                              0x000FFC00
#define    GIC_GICH_LR3_CPU7_VIRTUALID_SHIFT                              0
#define    GIC_GICH_LR3_CPU7_VIRTUALID_MASK                               0x000003FF

#define GIC_GICV_CTLR_OFFSET                                              0x00006000
#define GIC_GICV_CTLR_TYPE                                                UInt32
#define GIC_GICV_CTLR_RESERVED_MASK                                       0xFFFFFDE0
#define    GIC_GICV_CTLR_EOIMODE_SHIFT                                    9
#define    GIC_GICV_CTLR_EOIMODE_MASK                                     0x00000200
#define    GIC_GICV_CTLR_CBPR_SHIFT                                       4
#define    GIC_GICV_CTLR_CBPR_MASK                                        0x00000010
#define    GIC_GICV_CTLR_FIQEN_SHIFT                                      3
#define    GIC_GICV_CTLR_FIQEN_MASK                                       0x00000008
#define    GIC_GICV_CTLR_ACKCTL_SHIFT                                     2
#define    GIC_GICV_CTLR_ACKCTL_MASK                                      0x00000004
#define    GIC_GICV_CTLR_ENABLEGRP1_SHIFT                                 1
#define    GIC_GICV_CTLR_ENABLEGRP1_MASK                                  0x00000002
#define    GIC_GICV_CTLR_ENABLEGRP0_SHIFT                                 0
#define    GIC_GICV_CTLR_ENABLEGRP0_MASK                                  0x00000001

#define GIC_GICV_PMR_OFFSET                                               0x00006004
#define GIC_GICV_PMR_TYPE                                                 UInt32
#define GIC_GICV_PMR_RESERVED_MASK                                        0xFFFFFF07
#define    GIC_GICV_PMR_PRIORITY_SHIFT                                    3
#define    GIC_GICV_PMR_PRIORITY_MASK                                     0x000000F8

#define GIC_GICV_BPR_OFFSET                                               0x00006008
#define GIC_GICV_BPR_TYPE                                                 UInt32
#define GIC_GICV_BPR_RESERVED_MASK                                        0xFFFFFFF8
#define    GIC_GICV_BPR_BIN_PT_S_SHIFT                                    0
#define    GIC_GICV_BPR_BIN_PT_S_MASK                                     0x00000007

#define GIC_GICV_IAR_OFFSET                                               0x0000600C
#define GIC_GICV_IAR_TYPE                                                 UInt32
#define GIC_GICV_IAR_RESERVED_MASK                                        0xFFFFE000
#define    GIC_GICV_IAR_SOURCE_CPUID_SHIFT                                10
#define    GIC_GICV_IAR_SOURCE_CPUID_MASK                                 0x00001C00
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU0_STI                      0x00000000
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU1_STI                      0x00000001
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU2_STI                      0x00000002
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU3_STI                      0x00000003
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU4_STI                      0x00000004
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU5_STI                      0x00000005
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU6_STI                      0x00000006
#define       GIC_GICV_IAR_SOURCE_CPUID_CMD_CPU7_STI                      0x00000007
#define    GIC_GICV_IAR_ACK_INTID_SHIFT                                   0
#define    GIC_GICV_IAR_ACK_INTID_MASK                                    0x000003FF

#define GIC_GICV_EOIR_OFFSET                                              0x00006010
#define GIC_GICV_EOIR_TYPE                                                UInt32
#define GIC_GICV_EOIR_RESERVED_MASK                                       0xFFFFE000
#define    GIC_GICV_EOIR_SOURCECPUID_SHIFT                                10
#define    GIC_GICV_EOIR_SOURCECPUID_MASK                                 0x00001C00
#define    GIC_GICV_EOIR_EOI_INTID_SHIFT                                  0
#define    GIC_GICV_EOIR_EOI_INTID_MASK                                   0x000003FF

#define GIC_GICV_RPR_OFFSET                                               0x00006014
#define GIC_GICV_RPR_TYPE                                                 UInt32
#define GIC_GICV_RPR_RESERVED_MASK                                        0xFFFFFF00
#define    GIC_GICV_RPR_PRIORITY_SHIFT                                    0
#define    GIC_GICV_RPR_PRIORITY_MASK                                     0x000000FF

#define GIC_GICV_HPPIR_OFFSET                                             0x00006018
#define GIC_GICV_HPPIR_TYPE                                               UInt32
#define GIC_GICV_HPPIR_RESERVED_MASK                                      0xFFFFE000
#define    GIC_GICV_HPPIR_SOURCECPUID_SHIFT                               10
#define    GIC_GICV_HPPIR_SOURCECPUID_MASK                                0x00001C00
#define    GIC_GICV_HPPIR_PEND_INTID_SHIFT                                0
#define    GIC_GICV_HPPIR_PEND_INTID_MASK                                 0x000003FF

#define GIC_GICV_ABPR_OFFSET                                              0x0000601C
#define GIC_GICV_ABPR_TYPE                                                UInt32
#define GIC_GICV_ABPR_RESERVED_MASK                                       0xFFFFFFF8
#define    GIC_GICV_ABPR_BIN_PT_SHIFT                                     0
#define    GIC_GICV_ABPR_BIN_PT_MASK                                      0x00000007

#define GIC_GICV_AIAR_OFFSET                                              0x00006020
#define GIC_GICV_AIAR_TYPE                                                UInt32
#define GIC_GICV_AIAR_RESERVED_MASK                                       0xFFFFE000
#define    GIC_GICV_AIAR_SOURCECPUID_SHIFT                                10
#define    GIC_GICV_AIAR_SOURCECPUID_MASK                                 0x00001C00
#define    GIC_GICV_AIAR_PEND_INTID_SHIFT                                 0
#define    GIC_GICV_AIAR_PEND_INTID_MASK                                  0x000003FF

#define GIC_GICV_AEOIR_OFFSET                                             0x00006024
#define GIC_GICV_AEOIR_TYPE                                               UInt32
#define GIC_GICV_AEOIR_RESERVED_MASK                                      0xFFFFE000
#define    GIC_GICV_AEOIR_SOURCECPUID_SHIFT                               10
#define    GIC_GICV_AEOIR_SOURCECPUID_MASK                                0x00001C00
#define    GIC_GICV_AEOIR_EOI_INTID_SHIFT                                 0
#define    GIC_GICV_AEOIR_EOI_INTID_MASK                                  0x000003FF

#define GIC_GICV_AHPPIR_OFFSET                                            0x00006028
#define GIC_GICV_AHPPIR_TYPE                                              UInt32
#define GIC_GICV_AHPPIR_RESERVED_MASK                                     0xFFFFE000
#define    GIC_GICV_AHPPIR_SOURCECPUID_SHIFT                              10
#define    GIC_GICV_AHPPIR_SOURCECPUID_MASK                               0x00001C00
#define    GIC_GICV_AHPPIR_PEND_INTID_SHIFT                               0
#define    GIC_GICV_AHPPIR_PEND_INTID_MASK                                0x000003FF

#define GIC_GICV_APR0_OFFSET                                              0x000060D0
#define GIC_GICV_APR0_TYPE                                                UInt32
#define GIC_GICV_APR0_RESERVED_MASK                                       0x00000000
#define    GIC_GICV_APR0_VALUE_SHIFT                                      0
#define    GIC_GICV_APR0_VALUE_MASK                                       0xFFFFFFFF

#define GIC_GICV_IIDR_OFFSET                                              0x000060FC
#define GIC_GICV_IIDR_TYPE                                                UInt32
#define GIC_GICV_IIDR_RESERVED_MASK                                       0x00000000
#define    GIC_GICV_IIDR_PRODUCTID_SHIFT                                  20
#define    GIC_GICV_IIDR_PRODUCTID_MASK                                   0xFFF00000
#define    GIC_GICV_IIDR_ARCHITECTURE_VERSION_SHIFT                       16
#define    GIC_GICV_IIDR_ARCHITECTURE_VERSION_MASK                        0x000F0000
#define    GIC_GICV_IIDR_REVISION_SHIFT                                   12
#define    GIC_GICV_IIDR_REVISION_MASK                                    0x0000F000
#define    GIC_GICV_IIDR_IMPLEMENTER_SHIFT                                0
#define    GIC_GICV_IIDR_IMPLEMENTER_MASK                                 0x00000FFF

#define GIC_GICV_DIR_OFFSET                                               0x00007000
#define GIC_GICV_DIR_TYPE                                                 UInt32
#define GIC_GICV_DIR_RESERVED_MASK                                        0xFFFFE000
#define    GIC_GICV_DIR_CPUID_SHIFT                                       10
#define    GIC_GICV_DIR_CPUID_MASK                                        0x00001C00
#define    GIC_GICV_DIR_INTERRUPT_ID_SHIFT                                0
#define    GIC_GICV_DIR_INTERRUPT_ID_MASK                                 0x000003FF

#endif /* __BRCM_RDB_GIC_H__ */



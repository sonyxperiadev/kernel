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

#ifndef __BRCM_RDB_GICTR4_H__
#define __BRCM_RDB_GICTR4_H__

#define GICTR4_GIC_CONFIG_OFFSET                                          0x00000000
#define GICTR4_GIC_CONFIG_TYPE                                            UInt32
#define GICTR4_GIC_CONFIG_RESERVED_MASK                                   0xFFFFFBF6
#define    GICTR4_GIC_CONFIG_LATENCY_EN_SHIFT                             10
#define    GICTR4_GIC_CONFIG_LATENCY_EN_MASK                              0x00000400
#define    GICTR4_GIC_CONFIG_TRACE_EN_SHIFT                               3
#define    GICTR4_GIC_CONFIG_TRACE_EN_MASK                                0x00000008
#define    GICTR4_GIC_CONFIG_CTRL_SRC_SHIFT                               0
#define    GICTR4_GIC_CONFIG_CTRL_SRC_MASK                                0x00000001

#define GICTR4_GIC_STATUS_OFFSET                                          0x00000004
#define GICTR4_GIC_STATUS_TYPE                                            UInt32
#define GICTR4_GIC_STATUS_RESERVED_MASK                                   0xFFFFEFFF
#define    GICTR4_GIC_STATUS_STATE_LATENCY_EN_SHIFT                       12
#define    GICTR4_GIC_STATUS_STATE_LATENCY_EN_MASK                        0x00001000

#define GICTR4_GIC_OUTIDS_OFFSET                                          0x00000008
#define GICTR4_GIC_OUTIDS_TYPE                                            UInt32
#define GICTR4_GIC_OUTIDS_RESERVED_MASK                                   0xFFFFFF80
#define    GICTR4_GIC_OUTIDS_RATB_ID_SHIFT                                0
#define    GICTR4_GIC_OUTIDS_RATB_ID_MASK                                 0x0000007F

#define GICTR4_GIC_CMD_OFFSET                                             0x0000000C
#define GICTR4_GIC_CMD_TYPE                                               UInt32
#define GICTR4_GIC_CMD_RESERVED_MASK                                      0xFFFFFFFC
#define    GICTR4_GIC_CMD_LOCAL_CMD_SHIFT                                 0
#define    GICTR4_GIC_CMD_LOCAL_CMD_MASK                                  0x00000003

#define GICTR4_GIC_FIQ_LAT0_OFFSET                                        0x00000010
#define GICTR4_GIC_FIQ_LAT0_TYPE                                          UInt32
#define GICTR4_GIC_FIQ_LAT0_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_FIQ_LAT0_FIQ_LAT0_SHIFT                             0
#define    GICTR4_GIC_FIQ_LAT0_FIQ_LAT0_MASK                              0xFFFFFFFF

#define GICTR4_GIC_IRQ_LAT0_OFFSET                                        0x00000014
#define GICTR4_GIC_IRQ_LAT0_TYPE                                          UInt32
#define GICTR4_GIC_IRQ_LAT0_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_IRQ_LAT0_IRQ_LAT0_SHIFT                             0
#define    GICTR4_GIC_IRQ_LAT0_IRQ_LAT0_MASK                              0xFFFFFFFF

#define GICTR4_GIC_FIQ_LAT1_OFFSET                                        0x00000018
#define GICTR4_GIC_FIQ_LAT1_TYPE                                          UInt32
#define GICTR4_GIC_FIQ_LAT1_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_FIQ_LAT1_FIQ_LAT1_SHIFT                             0
#define    GICTR4_GIC_FIQ_LAT1_FIQ_LAT1_MASK                              0xFFFFFFFF

#define GICTR4_GIC_IRQ_LAT1_OFFSET                                        0x0000001C
#define GICTR4_GIC_IRQ_LAT1_TYPE                                          UInt32
#define GICTR4_GIC_IRQ_LAT1_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_IRQ_LAT1_IRQ_LAT1_SHIFT                             0
#define    GICTR4_GIC_IRQ_LAT1_IRQ_LAT1_MASK                              0xFFFFFFFF

#define GICTR4_GIC_FIQ_LAT2_OFFSET                                        0x00000020
#define GICTR4_GIC_FIQ_LAT2_TYPE                                          UInt32
#define GICTR4_GIC_FIQ_LAT2_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_FIQ_LAT2_FIQ_LAT0_SHIFT                             0
#define    GICTR4_GIC_FIQ_LAT2_FIQ_LAT0_MASK                              0xFFFFFFFF

#define GICTR4_GIC_IRQ_LAT2_OFFSET                                        0x00000024
#define GICTR4_GIC_IRQ_LAT2_TYPE                                          UInt32
#define GICTR4_GIC_IRQ_LAT2_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_IRQ_LAT2_IRQ_LAT0_SHIFT                             0
#define    GICTR4_GIC_IRQ_LAT2_IRQ_LAT0_MASK                              0xFFFFFFFF

#define GICTR4_GIC_FIQ_LAT3_OFFSET                                        0x00000028
#define GICTR4_GIC_FIQ_LAT3_TYPE                                          UInt32
#define GICTR4_GIC_FIQ_LAT3_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_FIQ_LAT3_FIQ_LAT1_SHIFT                             0
#define    GICTR4_GIC_FIQ_LAT3_FIQ_LAT1_MASK                              0xFFFFFFFF

#define GICTR4_GIC_IRQ_LAT3_OFFSET                                        0x0000002C
#define GICTR4_GIC_IRQ_LAT3_TYPE                                          UInt32
#define GICTR4_GIC_IRQ_LAT3_RESERVED_MASK                                 0x00000000
#define    GICTR4_GIC_IRQ_LAT3_IRQ_LAT1_SHIFT                             0
#define    GICTR4_GIC_IRQ_LAT3_IRQ_LAT1_MASK                              0xFFFFFFFF

#endif /* __BRCM_RDB_GICTR4_H__ */



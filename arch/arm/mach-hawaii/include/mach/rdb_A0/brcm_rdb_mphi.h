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

#ifndef __BRCM_RDB_MPHI_H__
#define __BRCM_RDB_MPHI_H__

#define MPHI_CMDR_CS0_OFFSET                                              0x00000000
#define MPHI_CMDR_CS0_TYPE                                                UInt32
#define MPHI_CMDR_CS0_RESERVED_MASK                                       0x00000000
#define    MPHI_CMDR_CS0_CMD_SHIFT                                        0
#define    MPHI_CMDR_CS0_CMD_MASK                                         0xFFFFFFFF

#define MPHI_DATR_CS0_OFFSET                                              0x00000004
#define MPHI_DATR_CS0_TYPE                                                UInt32
#define MPHI_DATR_CS0_RESERVED_MASK                                       0x00000000
#define    MPHI_DATR_CS0_DAT_SHIFT                                        0
#define    MPHI_DATR_CS0_DAT_MASK                                         0xFFFFFFFF

#define MPHI_RREQ_OFFSET                                                  0x00000008
#define MPHI_RREQ_TYPE                                                    UInt32
#define MPHI_RREQ_RESERVED_MASK                                           0xFFC00000
#define    MPHI_RREQ_RD_CNT_HIGH_SHIFT                                    12
#define    MPHI_RREQ_RD_CNT_HIGH_MASK                                     0x003FF000
#define    MPHI_RREQ_CS_CONTROL_SHIFT                                     11
#define    MPHI_RREQ_CS_CONTROL_MASK                                      0x00000800
#define    MPHI_RREQ_RD_CMD_SHIFT                                         10
#define    MPHI_RREQ_RD_CMD_MASK                                          0x00000400
#define    MPHI_RREQ_RD_CNT_LOW_SHIFT                                     0
#define    MPHI_RREQ_RD_CNT_LOW_MASK                                      0x000003FF

#define MPHI_SPECIAL_RREQ_OFFSET                                          0x0000000C
#define MPHI_SPECIAL_RREQ_TYPE                                            UInt32
#define MPHI_SPECIAL_RREQ_RESERVED_MASK                                   0xFF800000
#define    MPHI_SPECIAL_RREQ_RD_CNT2_HIGH_SHIFT                           13
#define    MPHI_SPECIAL_RREQ_RD_CNT2_HIGH_MASK                            0x007FE000
#define    MPHI_SPECIAL_RREQ_CS_CONTROL_SHIFT                             12
#define    MPHI_SPECIAL_RREQ_CS_CONTROL_MASK                              0x00001000
#define    MPHI_SPECIAL_RREQ_FIFO_REG_SHIFT                               11
#define    MPHI_SPECIAL_RREQ_FIFO_REG_MASK                                0x00000800
#define    MPHI_SPECIAL_RREQ_RD_CMD2_SHIFT                                10
#define    MPHI_SPECIAL_RREQ_RD_CMD2_MASK                                 0x00000400
#define    MPHI_SPECIAL_RREQ_RD_CNT2_LOW_SHIFT                            0
#define    MPHI_SPECIAL_RREQ_RD_CNT2_LOW_MASK                             0x000003FF

#define MPHI_CR_OFFSET                                                    0x00000010
#define MPHI_CR_TYPE                                                      UInt32
#define MPHI_CR_RESERVED_MASK                                             0x00000FFF
#define    MPHI_CR_MPHI_MODE_SHIFT                                        31
#define    MPHI_CR_MPHI_MODE_MASK                                         0x80000000
#define    MPHI_CR_READ_LOW_SHIFT                                         28
#define    MPHI_CR_READ_LOW_MASK                                          0x70000000
#define    MPHI_CR_READ_HIGH_SHIFT                                        25
#define    MPHI_CR_READ_HIGH_MASK                                         0x0E000000
#define    MPHI_CR_WRITE_LOW_SHIFT                                        22
#define    MPHI_CR_WRITE_LOW_MASK                                         0x01C00000
#define    MPHI_CR_WRITE_HIGH_SHIFT                                       19
#define    MPHI_CR_WRITE_HIGH_MASK                                        0x00380000
#define    MPHI_CR_PAUSE_SHIFT                                            18
#define    MPHI_CR_PAUSE_MASK                                             0x00040000
#define    MPHI_CR_RUN_EN_SHIFT                                           17
#define    MPHI_CR_RUN_EN_MASK                                            0x00020000
#define    MPHI_CR_HAT0_EN_SHIFT                                          16
#define    MPHI_CR_HAT0_EN_MASK                                           0x00010000
#define    MPHI_CR_HAT1_EN_SHIFT                                          15
#define    MPHI_CR_HAT1_EN_MASK                                           0x00008000
#define    MPHI_CR_HAT0_CLR_SHIFT                                         14
#define    MPHI_CR_HAT0_CLR_MASK                                          0x00004000
#define    MPHI_CR_HAT1_CLR_SHIFT                                         13
#define    MPHI_CR_HAT1_CLR_MASK                                          0x00002000
#define    MPHI_CR_MAXFREQ_EN_SHIFT                                       12
#define    MPHI_CR_MAXFREQ_EN_MASK                                        0x00001000

#define MPHI_SPECIAL_READ_OFFSET                                          0x00000014
#define MPHI_SPECIAL_READ_TYPE                                            UInt32
#define MPHI_SPECIAL_READ_RESERVED_MASK                                   0x00000000
#define    MPHI_SPECIAL_READ_READ_DATA_SHIFT                              0
#define    MPHI_SPECIAL_READ_READ_DATA_MASK                               0xFFFFFFFF

#define MPHI_STATUS_OFFSET                                                0x00000018
#define MPHI_STATUS_TYPE                                                  UInt32
#define MPHI_STATUS_RESERVED_MASK                                         0x00000FFF
#define    MPHI_STATUS_FEMPTY_SHIFT                                       31
#define    MPHI_STATUS_FEMPTY_MASK                                        0x80000000
#define    MPHI_STATUS_FFULL_SHIFT                                        30
#define    MPHI_STATUS_FFULL_MASK                                         0x40000000
#define    MPHI_STATUS_WR_FHALF_SHIFT                                     29
#define    MPHI_STATUS_WR_FHALF_MASK                                      0x20000000
#define    MPHI_STATUS_RD_FHALF_SHIFT                                     28
#define    MPHI_STATUS_RD_FHALF_MASK                                      0x10000000
#define    MPHI_STATUS_FWP_SHIFT                                          23
#define    MPHI_STATUS_FWP_MASK                                           0x0F800000
#define    MPHI_STATUS_FRP_SHIFT                                          18
#define    MPHI_STATUS_FRP_MASK                                           0x007C0000
#define    MPHI_STATUS_MPHI_BUSY_SHIFT                                    17
#define    MPHI_STATUS_MPHI_BUSY_MASK                                     0x00020000
#define    MPHI_STATUS_RREQ_SHIFT                                         16
#define    MPHI_STATUS_RREQ_MASK                                          0x00010000
#define    MPHI_STATUS_PAUSE_COMP_SHIFT                                   15
#define    MPHI_STATUS_PAUSE_COMP_MASK                                    0x00008000
#define    MPHI_STATUS_RREQ2_SHIFT                                        14
#define    MPHI_STATUS_RREQ2_MASK                                         0x00004000
#define    MPHI_STATUS_HAT0_SHIFT                                         13
#define    MPHI_STATUS_HAT0_MASK                                          0x00002000
#define    MPHI_STATUS_HAT1_SHIFT                                         12
#define    MPHI_STATUS_HAT1_MASK                                          0x00001000

#define MPHI_CMDR_CS1_OFFSET                                              0x00000020
#define MPHI_CMDR_CS1_TYPE                                                UInt32
#define MPHI_CMDR_CS1_RESERVED_MASK                                       0x00000000
#define    MPHI_CMDR_CS1_CMD_SHIFT                                        0
#define    MPHI_CMDR_CS1_CMD_MASK                                         0xFFFFFFFF

#define MPHI_DATR_CS1_OFFSET                                              0x00000024
#define MPHI_DATR_CS1_TYPE                                                UInt32
#define MPHI_DATR_CS1_RESERVED_MASK                                       0x00000000
#define    MPHI_DATR_CS1_DAT_SHIFT                                        0
#define    MPHI_DATR_CS1_DAT_MASK                                         0xFFFFFFFF

#endif /* __BRCM_RDB_MPHI_H__ */



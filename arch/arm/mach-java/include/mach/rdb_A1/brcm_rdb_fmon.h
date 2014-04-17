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

#ifndef __BRCM_RDB_FMON_H__
#define __BRCM_RDB_FMON_H__

#define FMON_CTRL_OFFSET                                                  0x00000000
#define FMON_CTRL_TYPE                                                    UInt32
#define FMON_CTRL_RESERVED_MASK                                           0xFFFF3FCC
#define    FMON_CTRL_CTRL_RESERVED_SHIFT                                  14
#define    FMON_CTRL_CTRL_RESERVED_MASK                                   0x0000C000
#define    FMON_CTRL_LOW_RST_EN_SHIFT                                     5
#define    FMON_CTRL_LOW_RST_EN_MASK                                      0x00000020
#define    FMON_CTRL_HIGH_RST_EN_SHIFT                                    4
#define    FMON_CTRL_HIGH_RST_EN_MASK                                     0x00000010
#define    FMON_CTRL_PWRDN_SHIFT                                          1
#define    FMON_CTRL_PWRDN_MASK                                           0x00000002
#define    FMON_CTRL_SOFT_RST_SHIFT                                       0
#define    FMON_CTRL_SOFT_RST_MASK                                        0x00000001

#define FMON_STS_OFFSET                                                   0x00000004
#define FMON_STS_TYPE                                                     UInt32
#define FMON_STS_RESERVED_MASK                                            0x00000000
#define    FMON_STS_STS_RESERVED_SHIFT                                    2
#define    FMON_STS_STS_RESERVED_MASK                                     0xFFFFFFFC
#define    FMON_STS_LOW_STS_SHIFT                                         1
#define    FMON_STS_LOW_STS_MASK                                          0x00000002
#define    FMON_STS_HIGH_STS_SHIFT                                        0
#define    FMON_STS_HIGH_STS_MASK                                         0x00000001

#define FMON_INTR_OFFSET                                                  0x00000008
#define FMON_INTR_TYPE                                                    UInt32
#define FMON_INTR_RESERVED_MASK                                           0x00000000
#define    FMON_INTR_INT_CLR_RESERVED_SHIFT                               2
#define    FMON_INTR_INT_CLR_RESERVED_MASK                                0xFFFFFFFC
#define    FMON_INTR_LOW_INTR_SHIFT                                       1
#define    FMON_INTR_LOW_INTR_MASK                                        0x00000002
#define    FMON_INTR_HIGH_INTR_SHIFT                                      0
#define    FMON_INTR_HIGH_INTR_MASK                                       0x00000001

#define FMON_INT_ENABLE_OFFSET                                            0x0000000C
#define FMON_INT_ENABLE_TYPE                                              UInt32
#define FMON_INT_ENABLE_RESERVED_MASK                                     0x00000000
#define    FMON_INT_ENABLE_INT_EN_RESERVED_SHIFT                          2
#define    FMON_INT_ENABLE_INT_EN_RESERVED_MASK                           0xFFFFFFFC
#define    FMON_INT_ENABLE_LOW_INT_EN_SHIFT                               1
#define    FMON_INT_ENABLE_LOW_INT_EN_MASK                                0x00000002
#define    FMON_INT_ENABLE_HIGH_INT_EN_SHIFT                              0
#define    FMON_INT_ENABLE_HIGH_INT_EN_MASK                               0x00000001

#define FMON_CALIBRATION_OFFSET                                           0x00000010
#define FMON_CALIBRATION_TYPE                                             UInt32
#define FMON_CALIBRATION_RESERVED_MASK                                    0x00800000
#define    FMON_CALIBRATION_LFO_STATUS_SHIFT                              24
#define    FMON_CALIBRATION_LFO_STATUS_MASK                               0xFF000000
#define    FMON_CALIBRATION_LFO_CAL_SHIFT                                 16
#define    FMON_CALIBRATION_LFO_CAL_MASK                                  0x007F0000
#define    FMON_CALIBRATION_LFO_CTRL_SHIFT                                0
#define    FMON_CALIBRATION_LFO_CTRL_MASK                                 0x0000FFFF

#endif /* __BRCM_RDB_FMON_H__ */



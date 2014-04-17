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

#ifndef __BRCM_RDB_TMON_H__
#define __BRCM_RDB_TMON_H__

#define TMON_CFG_ENBALE_OFFSET                                            0x00000000
#define TMON_CFG_ENBALE_TYPE                                              UInt32
#define TMON_CFG_ENBALE_RESERVED_MASK                                     0xFFFFFFF0
#define    TMON_CFG_ENBALE_ENABLE_SHIFT                                   0
#define    TMON_CFG_ENBALE_ENABLE_MASK                                    0x0000000F

#define TMON_CFG_CTRL_OFFSET                                              0x00000004
#define TMON_CFG_CTRL_TYPE                                                UInt32
#define TMON_CFG_CTRL_RESERVED_MASK                                       0xFFFF0000
#define    TMON_CFG_CTRL_CTRL_SHIFT                                       0
#define    TMON_CFG_CTRL_CTRL_MASK                                        0x0000FFFF

#define TMON_CFG_INT_THRESH_OFFSET                                        0x00000008
#define TMON_CFG_INT_THRESH_TYPE                                          UInt32
#define TMON_CFG_INT_THRESH_RESERVED_MASK                                 0xFFFFFC00
#define    TMON_CFG_INT_THRESH_INT_THRESH_SHIFT                           0
#define    TMON_CFG_INT_THRESH_INT_THRESH_MASK                            0x000003FF

#define TMON_CFG_RST_THRESH_OFFSET                                        0x0000000C
#define TMON_CFG_RST_THRESH_TYPE                                          UInt32
#define TMON_CFG_RST_THRESH_RESERVED_MASK                                 0xFFFFFC00
#define    TMON_CFG_RST_THRESH_RST_THRESH_SHIFT                           0
#define    TMON_CFG_RST_THRESH_RST_THRESH_MASK                            0x000003FF

#define TMON_CFG_INTERVAL_VAL_OFFSET                                      0x00000010
#define TMON_CFG_INTERVAL_VAL_TYPE                                        UInt32
#define TMON_CFG_INTERVAL_VAL_RESERVED_MASK                               0xFFFFFC00
#define    TMON_CFG_INTERVAL_VAL_INTERVAL_VAL_SHIFT                       0
#define    TMON_CFG_INTERVAL_VAL_INTERVAL_VAL_MASK                        0x000003FF

#define TMON_CFG_CLR_INT_OFFSET                                           0x00000014
#define TMON_CFG_CLR_INT_TYPE                                             UInt32
#define TMON_CFG_CLR_INT_RESERVED_MASK                                    0xFFFFFFFE
#define    TMON_CFG_CLR_INT_CLR_INT_SHIFT                                 0
#define    TMON_CFG_CLR_INT_CLR_INT_MASK                                  0x00000001

#define TMON_CFG_RST_EN_OFFSET                                            0x00000018
#define TMON_CFG_RST_EN_TYPE                                              UInt32
#define TMON_CFG_RST_EN_RESERVED_MASK                                     0xFFFFFFFE
#define    TMON_CFG_RST_EN_RST_ENABLE_SHIFT                               0
#define    TMON_CFG_RST_EN_RST_ENABLE_MASK                                0x00000001

#define TMON_TEMP_VAL_OFFSET                                              0x0000001C
#define TMON_TEMP_VAL_TYPE                                                UInt32
#define TMON_TEMP_VAL_RESERVED_MASK                                       0xFFFFFC00
#define    TMON_TEMP_VAL_TEMP_VAL_SHIFT                                   0
#define    TMON_TEMP_VAL_TEMP_VAL_MASK                                    0x000003FF

#define TMON_CFG_SUPP_MODE_OFFSET                                         0x00000020
#define TMON_CFG_SUPP_MODE_TYPE                                           UInt32
#define TMON_CFG_SUPP_MODE_RESERVED_MASK                                  0xFFFFFFFE
#define    TMON_CFG_SUPP_MODE_SUPPLYMODE_SHIFT                            0
#define    TMON_CFG_SUPP_MODE_SUPPLYMODE_MASK                             0x00000001

#define TMON_CFG_ANA_VTMON_CTRL_OFFSET                                    0x00000024
#define TMON_CFG_ANA_VTMON_CTRL_TYPE                                      UInt32
#define TMON_CFG_ANA_VTMON_CTRL_RESERVED_MASK                             0xFFFFFFC0
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_MUXSEL_SHIFT                 5
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_MUXSEL_MASK                  0x00000020
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_RESET_SHIFT                  4
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_RESET_MASK                   0x00000010
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_PWRUP_SHIFT                  3
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_PWRUP_MASK                   0x00000008
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_SEL_SHIFT                    0
#define    TMON_CFG_ANA_VTMON_CTRL_ANA_VTMON_SEL_MASK                     0x00000007

#endif /* __BRCM_RDB_TMON_H__ */



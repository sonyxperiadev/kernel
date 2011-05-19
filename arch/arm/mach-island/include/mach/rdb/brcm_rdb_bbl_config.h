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

#ifndef __BRCM_RDB_BBL_CONFIG_H__
#define __BRCM_RDB_BBL_CONFIG_H__

#define BBL_CONFIG_CLK_DISABLE_OFFSET                                     0x00000000
#define BBL_CONFIG_CLK_DISABLE_TYPE                                       UInt32
#define BBL_CONFIG_CLK_DISABLE_RESERVED_MASK                              0x00000000
#define    BBL_CONFIG_CLK_DISABLE_CLK_DISABLE_UNUSED_SHIFT                5
#define    BBL_CONFIG_CLK_DISABLE_CLK_DISABLE_UNUSED_MASK                 0xFFFFFFE0
#define    BBL_CONFIG_CLK_DISABLE_BBRAM_CLK_DIS_SHIFT                     4
#define    BBL_CONFIG_CLK_DISABLE_BBRAM_CLK_DIS_MASK                      0x00000010
#define    BBL_CONFIG_CLK_DISABLE_WDG_CLK_DIS_SHIFT                       3
#define    BBL_CONFIG_CLK_DISABLE_WDG_CLK_DIS_MASK                        0x00000008
#define    BBL_CONFIG_CLK_DISABLE_VFM_CLK_DIS_SHIFT                       2
#define    BBL_CONFIG_CLK_DISABLE_VFM_CLK_DIS_MASK                        0x00000004
#define    BBL_CONFIG_CLK_DISABLE_PWSEQ_CLK_DIS_SHIFT                     1
#define    BBL_CONFIG_CLK_DISABLE_PWSEQ_CLK_DIS_MASK                      0x00000002
#define    BBL_CONFIG_CLK_DISABLE_RTC_CLK_DIS_SHIFT                       0
#define    BBL_CONFIG_CLK_DISABLE_RTC_CLK_DIS_MASK                        0x00000001

#define BBL_CONFIG_SOFT_RST_OFFSET                                        0x00000004
#define BBL_CONFIG_SOFT_RST_TYPE                                          UInt32
#define BBL_CONFIG_SOFT_RST_RESERVED_MASK                                 0x00000000
#define    BBL_CONFIG_SOFT_RST_SOFT_RST_UNUSED_SHIFT                      5
#define    BBL_CONFIG_SOFT_RST_SOFT_RST_UNUSED_MASK                       0xFFFFFFE0
#define    BBL_CONFIG_SOFT_RST_BBRAM_SOFT_RST_SHIFT                       4
#define    BBL_CONFIG_SOFT_RST_BBRAM_SOFT_RST_MASK                        0x00000010
#define    BBL_CONFIG_SOFT_RST_WDG_SOFT_RST_SHIFT                         3
#define    BBL_CONFIG_SOFT_RST_WDG_SOFT_RST_MASK                          0x00000008
#define    BBL_CONFIG_SOFT_RST_VFM_SOFT_RST_SHIFT                         2
#define    BBL_CONFIG_SOFT_RST_VFM_SOFT_RST_MASK                          0x00000004
#define    BBL_CONFIG_SOFT_RST_PWSEQ_SOFT_RST_SHIFT                       1
#define    BBL_CONFIG_SOFT_RST_PWSEQ_SOFT_RST_MASK                        0x00000002
#define    BBL_CONFIG_SOFT_RST_RTC_SOFT_RST_SHIFT                         0
#define    BBL_CONFIG_SOFT_RST_RTC_SOFT_RST_MASK                          0x00000001

#define BBL_CONFIG_BBL_GP_REG1_OFFSET                                     0x00000008
#define BBL_CONFIG_BBL_GP_REG1_TYPE                                       UInt32
#define BBL_CONFIG_BBL_GP_REG1_RESERVED_MASK                              0x00000000
#define    BBL_CONFIG_BBL_GP_REG1_BBL_GP_REG1_DATA_SHIFT                  0
#define    BBL_CONFIG_BBL_GP_REG1_BBL_GP_REG1_DATA_MASK                   0xFFFFFFFF

#define BBL_CONFIG_BBL_GP_REG2_OFFSET                                     0x0000000C
#define BBL_CONFIG_BBL_GP_REG2_TYPE                                       UInt32
#define BBL_CONFIG_BBL_GP_REG2_RESERVED_MASK                              0x00000000
#define    BBL_CONFIG_BBL_GP_REG2_BBL_GP_REG2_DATA_SHIFT                  0
#define    BBL_CONFIG_BBL_GP_REG2_BBL_GP_REG2_DATA_MASK                   0xFFFFFFFF

#define BBL_CONFIG_BBL_GP_REG3_OFFSET                                     0x00000010
#define BBL_CONFIG_BBL_GP_REG3_TYPE                                       UInt32
#define BBL_CONFIG_BBL_GP_REG3_RESERVED_MASK                              0x00000000
#define    BBL_CONFIG_BBL_GP_REG3_BBL_GP_REG3_DATA_SHIFT                  0
#define    BBL_CONFIG_BBL_GP_REG3_BBL_GP_REG3_DATA_MASK                   0xFFFFFFFF

#define BBL_CONFIG_BBL_GP_REG4_OFFSET                                     0x00000014
#define BBL_CONFIG_BBL_GP_REG4_TYPE                                       UInt32
#define BBL_CONFIG_BBL_GP_REG4_RESERVED_MASK                              0x00000000
#define    BBL_CONFIG_BBL_GP_REG4_BBL_GP_REG4_DATA_SHIFT                  0
#define    BBL_CONFIG_BBL_GP_REG4_BBL_GP_REG4_DATA_MASK                   0xFFFFFFFF

#define BBL_CONFIG_BBL_INT_STS_OFFSET                                     0x00000018
#define BBL_CONFIG_BBL_INT_STS_TYPE                                       UInt32
#define BBL_CONFIG_BBL_INT_STS_RESERVED_MASK                              0xFFFFFFFE
#define    BBL_CONFIG_BBL_INT_STS_TAMPER_STS_SHIFT                        0
#define    BBL_CONFIG_BBL_INT_STS_TAMPER_STS_MASK                         0x00000001

#define BBL_CONFIG_BBL_INT_EN_OFFSET                                      0x0000001C
#define BBL_CONFIG_BBL_INT_EN_TYPE                                        UInt32
#define BBL_CONFIG_BBL_INT_EN_RESERVED_MASK                               0xFFFFFFFE
#define    BBL_CONFIG_BBL_INT_EN_TAMPER_INT_EN_SHIFT                      0
#define    BBL_CONFIG_BBL_INT_EN_TAMPER_INT_EN_MASK                       0x00000001

#define BBL_CONFIG_BBL_RESET_EN_OFFSET                                    0x00000020
#define BBL_CONFIG_BBL_RESET_EN_TYPE                                      UInt32
#define BBL_CONFIG_BBL_RESET_EN_RESERVED_MASK                             0xFFFFFFFE
#define    BBL_CONFIG_BBL_RESET_EN_TAMPER_RESET_EN_SHIFT                  0
#define    BBL_CONFIG_BBL_RESET_EN_TAMPER_RESET_EN_MASK                   0x00000001

#endif /* __BRCM_RDB_BBL_CONFIG_H__ */



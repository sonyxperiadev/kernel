/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/************************************************************************************************/

#ifndef __BRCM_RDB_BBL_VFM_H__
#define __BRCM_RDB_BBL_VFM_H__

#define BBL_VFM_CTRL_OFFSET                                               0x00000000
#define BBL_VFM_CTRL_TYPE                                                 UInt32
#define BBL_VFM_CTRL_RESERVED_MASK                                        0x00000000
#define    BBL_VFM_CTRL_BBLSI_CTRL_SHIFT                                  16
#define    BBL_VFM_CTRL_BBLSI_CTRL_MASK                                   0xFFFF0000
#define    BBL_VFM_CTRL_CTRL_UNUSED_SHIFT                                 12
#define    BBL_VFM_CTRL_CTRL_UNUSED_MASK                                  0x0000F000
#define    BBL_VFM_CTRL_TAMPER_RESET_MASK_SHIFT                           8
#define    BBL_VFM_CTRL_TAMPER_RESET_MASK_MASK                            0x00000F00
#define    BBL_VFM_CTRL_LFO_TEST_SHIFT                                    7
#define    BBL_VFM_CTRL_LFO_TEST_MASK                                     0x00000080
#define    BBL_VFM_CTRL_LFO_CAL_SHIFT                                     3
#define    BBL_VFM_CTRL_LFO_CAL_MASK                                      0x00000078
#define    BBL_VFM_CTRL_VMON_PWRDN_SHIFT                                  2
#define    BBL_VFM_CTRL_VMON_PWRDN_MASK                                   0x00000004
#define    BBL_VFM_CTRL_FMON_PWRDN_SHIFT                                  1
#define    BBL_VFM_CTRL_FMON_PWRDN_MASK                                   0x00000002
#define    BBL_VFM_CTRL_FMON_RESETB_SHIFT                                 0
#define    BBL_VFM_CTRL_FMON_RESETB_MASK                                  0x00000001

#define BBL_VFM_STS_OFFSET                                                0x00000004
#define BBL_VFM_STS_TYPE                                                  UInt32
#define BBL_VFM_STS_RESERVED_MASK                                         0x00000000
#define    BBL_VFM_STS_STS_UNUSED_SHIFT                                   4
#define    BBL_VFM_STS_STS_UNUSED_MASK                                    0xFFFFFFF0
#define    BBL_VFM_STS_VMON_LOWB_SHIFT                                    3
#define    BBL_VFM_STS_VMON_LOWB_MASK                                     0x00000008
#define    BBL_VFM_STS_VMON_HIGH_SHIFT                                    2
#define    BBL_VFM_STS_VMON_HIGH_MASK                                     0x00000004
#define    BBL_VFM_STS_FMON_LOW_SHIFT                                     1
#define    BBL_VFM_STS_FMON_LOW_MASK                                      0x00000002
#define    BBL_VFM_STS_FMON_HIGH_SHIFT                                    0
#define    BBL_VFM_STS_FMON_HIGH_MASK                                     0x00000001

#define BBL_VFM_INT_CLR_OFFSET                                            0x00000008
#define BBL_VFM_INT_CLR_TYPE                                              UInt32
#define BBL_VFM_INT_CLR_RESERVED_MASK                                     0x00000000
#define    BBL_VFM_INT_CLR_INT_CLR_UNUSED_SHIFT                           4
#define    BBL_VFM_INT_CLR_INT_CLR_UNUSED_MASK                            0xFFFFFFF0
#define    BBL_VFM_INT_CLR_BBL_FMON_LOW_CLR_SHIFT                         3
#define    BBL_VFM_INT_CLR_BBL_FMON_LOW_CLR_MASK                          0x00000008
#define    BBL_VFM_INT_CLR_BBL_FMON_HIGH_CLR_SHIFT                        2
#define    BBL_VFM_INT_CLR_BBL_FMON_HIGH_CLR_MASK                         0x00000004
#define    BBL_VFM_INT_CLR_BBL_VMON_LOWB_CLR_SHIFT                        1
#define    BBL_VFM_INT_CLR_BBL_VMON_LOWB_CLR_MASK                         0x00000002
#define    BBL_VFM_INT_CLR_BBL_VMON_HIGH_CLR_SHIFT                        0
#define    BBL_VFM_INT_CLR_BBL_VMON_HIGH_CLR_MASK                         0x00000001

#define BBL_VFM_INT_ENABLE_OFFSET                                         0x0000000C
#define BBL_VFM_INT_ENABLE_TYPE                                           UInt32
#define BBL_VFM_INT_ENABLE_RESERVED_MASK                                  0x00000000
#define    BBL_VFM_INT_ENABLE_INT_EN_UNUSED_SHIFT                         4
#define    BBL_VFM_INT_ENABLE_INT_EN_UNUSED_MASK                          0xFFFFFFF0
#define    BBL_VFM_INT_ENABLE_BBL_FMON_EN_SHIFT                           2
#define    BBL_VFM_INT_ENABLE_BBL_FMON_EN_MASK                            0x0000000C
#define    BBL_VFM_INT_ENABLE_BBL_VMON_EN_SHIFT                           0
#define    BBL_VFM_INT_ENABLE_BBL_VMON_EN_MASK                            0x00000003

#define BBL_VFM_VTH_CFG_OFFSET                                            0x00000010
#define BBL_VFM_VTH_CFG_TYPE                                              UInt32
#define BBL_VFM_VTH_CFG_RESERVED_MASK                                     0x00000000
#define    BBL_VFM_VTH_CFG_VTH_CFG_UNUSED_SHIFT                           8
#define    BBL_VFM_VTH_CFG_VTH_CFG_UNUSED_MASK                            0xFFFFFF00
#define    BBL_VFM_VTH_CFG_BBL_HIGH_VTH_CFG_SHIFT                         4
#define    BBL_VFM_VTH_CFG_BBL_HIGH_VTH_CFG_MASK                          0x000000F0
#define    BBL_VFM_VTH_CFG_BBL_LOW_VTH_CFG_SHIFT                          0
#define    BBL_VFM_VTH_CFG_BBL_LOW_VTH_CFG_MASK                           0x0000000F

#endif /* __BRCM_RDB_BBL_VFM_H__ */



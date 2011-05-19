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
/*     Date     : Generated on 5/15/2011 11:41:39                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_PMU_FG_H__
#define __BRCM_RDB_PMU_FG_H__

#define PMU_FG_FGCTRL1_OFFSET                                             0x00000000
#define PMU_FG_FGCTRL1_TYPE                                               UInt32
#define PMU_FG_FGCTRL1_RESERVED_MASK                                      0xFFFFFF0E
#define    PMU_FG_FGCTRL1_FGCTRL_3_0__SHIFT                               4
#define    PMU_FG_FGCTRL1_FGCTRL_3_0__MASK                                0x000000F0
#define    PMU_FG_FGCTRL1_FGHOSTEN_SHIFT                                  0
#define    PMU_FG_FGCTRL1_FGHOSTEN_MASK                                   0x00000001

#define PMU_FG_FGCTRL2_OFFSET                                             0x00000004
#define PMU_FG_FGCTRL2_TYPE                                               UInt32
#define PMU_FG_FGCTRL2_RESERVED_MASK                                      0xFFFFF800
#define    PMU_FG_FGCTRL2_FGHSMODE_SHIFT                                  10
#define    PMU_FG_FGCTRL2_FGHSMODE_MASK                                   0x00000400
#define    PMU_FG_FGCTRL2_FGSMPL_SEL_1_0__SHIFT                           8
#define    PMU_FG_FGCTRL2_FGSMPL_SEL_1_0__MASK                            0x00000300
#define    PMU_FG_FGCTRL2_FGFORCECAL_SHIFT                                7
#define    PMU_FG_FGCTRL2_FGFORCECAL_MASK                                 0x00000080
#define    PMU_FG_FGCTRL2_FGFRZSMPL_SHIFT                                 6
#define    PMU_FG_FGCTRL2_FGFRZSMPL_MASK                                  0x00000040
#define    PMU_FG_FGCTRL2_FGFRZREAD_SHIFT                                 5
#define    PMU_FG_FGCTRL2_FGFRZREAD_MASK                                  0x00000020
#define    PMU_FG_FGCTRL2_FGRESET_SHIFT                                   4
#define    PMU_FG_FGCTRL2_FGRESET_MASK                                    0x00000010
#define    PMU_FG_FGCTRL2_FG1PTCAL_SHIFT                                  3
#define    PMU_FG_FGCTRL2_FG1PTCAL_MASK                                   0x00000008
#define    PMU_FG_FGCTRL2_LONGCAL_SHIFT                                   2
#define    PMU_FG_FGCTRL2_LONGCAL_MASK                                    0x00000004
#define    PMU_FG_FGCTRL2_FGCAL_SHIFT                                     1
#define    PMU_FG_FGCTRL2_FGCAL_MASK                                      0x00000002
#define    PMU_FG_FGCTRL2_FGTRIM_SHIFT                                    0
#define    PMU_FG_FGCTRL2_FGTRIM_MASK                                     0x00000001

#define PMU_FG_FGCTRL3_OFFSET                                             0x00000008
#define PMU_FG_FGCTRL3_TYPE                                               UInt32
#define PMU_FG_FGCTRL3_RESERVED_MASK                                      0xFFFFFCC0
#define    PMU_FG_FGCTRL3_FG_COMB_RATE_1_0__SHIFT                         8
#define    PMU_FG_FGCTRL3_FG_COMB_RATE_1_0__MASK                          0x00000300
#define    PMU_FG_FGCTRL3_FGMODON_SHIFT                                   5
#define    PMU_FG_FGCTRL3_FGMODON_MASK                                    0x00000020
#define    PMU_FG_FGCTRL3_FGSYNCMODE_SHIFT                                4
#define    PMU_FG_FGCTRL3_FGSYNCMODE_MASK                                 0x00000010
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_3__SHIFT                            3
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_3__MASK                             0x00000008
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_2__SHIFT                            2
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_2__MASK                             0x00000004
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_1__SHIFT                            1
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_1__MASK                             0x00000002
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_0__SHIFT                            0
#define    PMU_FG_FGCTRL3_FGOPMODCTRL_0__MASK                             0x00000001

#define PMU_FG_FGCTRL4_OFFSET                                             0x0000000C
#define PMU_FG_FGCTRL4_TYPE                                               UInt32
#define PMU_FG_FGCTRL4_RESERVED_MASK                                      0xFFFFFF00
#define    PMU_FG_FGCTRL4_FGOFFSET_TRIM_7_0__SHIFT                        0
#define    PMU_FG_FGCTRL4_FGOFFSET_TRIM_7_0__MASK                         0x000000FF

#define PMU_FG_FGCTRL5_OFFSET                                             0x00000010
#define PMU_FG_FGCTRL5_TYPE                                               UInt32
#define PMU_FG_FGCTRL5_RESERVED_MASK                                      0xFFFFFF00
#define    PMU_FG_FGCTRL5_FGGAIN_TRIM_7_0__SHIFT                          0
#define    PMU_FG_FGCTRL5_FGGAIN_TRIM_7_0__MASK                           0x000000FF

#define PMU_FG_FGCTRL6_OFFSET                                             0x00000014
#define PMU_FG_FGCTRL6_TYPE                                               UInt32
#define PMU_FG_FGCTRL6_RESERVED_MASK                                      0x7C000000
#define    PMU_FG_FGCTRL6_FGRDVALID_SHIFT                                 31
#define    PMU_FG_FGCTRL6_FGRDVALID_MASK                                  0x80000000
#define    PMU_FG_FGCTRL6_FGACCM_25_0__SHIFT                              0
#define    PMU_FG_FGCTRL6_FGACCM_25_0__MASK                               0x03FFFFFF

#define PMU_FG_FGCTRL7_OFFSET                                             0x00000018
#define PMU_FG_FGCTRL7_TYPE                                               UInt32
#define PMU_FG_FGCTRL7_RESERVED_MASK                                      0xFFFFF000
#define    PMU_FG_FGCTRL7_FGCNT_11_0__SHIFT                               0
#define    PMU_FG_FGCTRL7_FGCNT_11_0__MASK                                0x00000FFF

#define PMU_FG_FGCTRL8_OFFSET                                             0x0000001C
#define PMU_FG_FGCTRL8_TYPE                                               UInt32
#define PMU_FG_FGCTRL8_RESERVED_MASK                                      0xFFFF0000
#define    PMU_FG_FGCTRL8_FGSLEEPCNT_15_0__SHIFT                          0
#define    PMU_FG_FGCTRL8_FGSLEEPCNT_15_0__MASK                           0x0000FFFF

#define PMU_FG_FGCTRL9_OFFSET                                             0x00000020
#define PMU_FG_FGCTRL9_TYPE                                               UInt32
#define PMU_FG_FGCTRL9_RESERVED_MASK                                      0xFFFF0000
#define    PMU_FG_FGCTRL9_FGSMPL_OUT_15_0__SHIFT                          0
#define    PMU_FG_FGCTRL9_FGSMPL_OUT_15_0__MASK                           0x0000FFFF

#define PMU_FG_FGCTRL10_OFFSET                                            0x00000024
#define PMU_FG_FGCTRL10_TYPE                                              UInt32
#define PMU_FG_FGCTRL10_RESERVED_MASK                                     0xFFFF0000
#define    PMU_FG_FGCTRL10_FGOFFSET_15_0__SHIFT                           0
#define    PMU_FG_FGCTRL10_FGOFFSET_15_0__MASK                            0x0000FFFF

#define PMU_FG_FGCTRL11_OFFSET                                            0x00000028
#define PMU_FG_FGCTRL11_TYPE                                              UInt32
#define PMU_FG_FGCTRL11_RESERVED_MASK                                     0xFFFFFC00
#define    PMU_FG_FGCTRL11_FGVMBAT_9_0__SHIFT                             0
#define    PMU_FG_FGCTRL11_FGVMBAT_9_0__MASK                              0x000003FF

#define PMU_FG_FGCTRL12_OFFSET                                            0x0000002C
#define PMU_FG_FGCTRL12_TYPE                                              UInt32
#define PMU_FG_FGCTRL12_RESERVED_MASK                                     0xFF000000
#define    PMU_FG_FGCTRL12_FGGNRL3_7_0__SHIFT                             16
#define    PMU_FG_FGCTRL12_FGGNRL3_7_0__MASK                              0x00FF0000
#define    PMU_FG_FGCTRL12_FGGNRL2_7_0__SHIFT                             8
#define    PMU_FG_FGCTRL12_FGGNRL2_7_0__MASK                              0x0000FF00
#define    PMU_FG_FGCTRL12_FGGNRL1_7_0__SHIFT                             0
#define    PMU_FG_FGCTRL12_FGGNRL1_7_0__MASK                              0x000000FF

#define PMU_FG_FGCTRL13_OFFSET                                            0x00000030
#define PMU_FG_FGCTRL13_TYPE                                              UInt32
#define PMU_FG_FGCTRL13_RESERVED_MASK                                     0xC000C000
#define    PMU_FG_FGCTRL13_FGGNTRM_PT2_SHIFT                              16
#define    PMU_FG_FGCTRL13_FGGNTRM_PT2_MASK                               0x3FFF0000
#define    PMU_FG_FGCTRL13_FGGNTRM_PT1_SHIFT                              0
#define    PMU_FG_FGCTRL13_FGGNTRM_PT1_MASK                               0x00003FFF

#define PMU_FG_FGCTRL14_OFFSET                                            0x00000034
#define PMU_FG_FGCTRL14_TYPE                                              UInt32
#define PMU_FG_FGCTRL14_RESERVED_MASK                                     0xFFFFE0C0
#define    PMU_FG_FGCTRL14_FGBAT_OVERI_DEB_4_0__SHIFT                     8
#define    PMU_FG_FGCTRL14_FGBAT_OVERI_DEB_4_0__MASK                      0x00001F00
#define    PMU_FG_FGCTRL14_FGBAT_OVERI_THRES_5_0__SHIFT                   0
#define    PMU_FG_FGCTRL14_FGBAT_OVERI_THRES_5_0__MASK                    0x0000003F

#endif /* __BRCM_RDB_PMU_FG_H__ */



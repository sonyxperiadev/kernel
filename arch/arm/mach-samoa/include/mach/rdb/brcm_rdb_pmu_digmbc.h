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
/*     Date     : Generated on 4/10/2011 22:27:55                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_PMU_DIGMBC_H__
#define __BRCM_RDB_PMU_DIGMBC_H__

#define PMU_DIGMBC_COLCNFG1_OFFSET                                        0x00000000
#define PMU_DIGMBC_COLCNFG1_TYPE                                          UInt32
#define PMU_DIGMBC_COLCNFG1_RESERVED_MASK                                 0xC0000000
#define    PMU_DIGMBC_COLCNFG1_LOWCOL_9_0__SHIFT                          20
#define    PMU_DIGMBC_COLCNFG1_LOWCOL_9_0__MASK                           0x3FF00000
#define    PMU_DIGMBC_COLCNFG1_OFFADAP_9_0__SHIFT                         10
#define    PMU_DIGMBC_COLCNFG1_OFFADAP_9_0__MASK                          0x000FFC00
#define    PMU_DIGMBC_COLCNFG1_LOWADAP_9_0__SHIFT                         0
#define    PMU_DIGMBC_COLCNFG1_LOWADAP_9_0__MASK                          0x000003FF

#define PMU_DIGMBC_COLCNFG2_OFFSET                                        0x00000004
#define PMU_DIGMBC_COLCNFG2_TYPE                                          UInt32
#define PMU_DIGMBC_COLCNFG2_RESERVED_MASK                                 0xC0000000
#define    PMU_DIGMBC_COLCNFG2_COL_AA1_9_0__SHIFT                         20
#define    PMU_DIGMBC_COLCNFG2_COL_AA1_9_0__MASK                          0x3FF00000
#define    PMU_DIGMBC_COLCNFG2_CCLIMCOL_9_0__SHIFT                        10
#define    PMU_DIGMBC_COLCNFG2_CCLIMCOL_9_0__MASK                         0x000FFC00
#define    PMU_DIGMBC_COLCNFG2_OFFCOL_9_0__SHIFT                          0
#define    PMU_DIGMBC_COLCNFG2_OFFCOL_9_0__MASK                           0x000003FF

#define PMU_DIGMBC_ILOOPCNFG1_OFFSET                                      0x00000008
#define PMU_DIGMBC_ILOOPCNFG1_TYPE                                        UInt32
#define PMU_DIGMBC_ILOOPCNFG1_RESERVED_MASK                               0xF0000000
#define    PMU_DIGMBC_ILOOPCNFG1_OVERIMAXERR_12_0__SHIFT                  14
#define    PMU_DIGMBC_ILOOPCNFG1_OVERIMAXERR_12_0__MASK                   0x0FFFC000
#define    PMU_DIGMBC_ILOOPCNFG1_MAXERR1_13_0__SHIFT                      0
#define    PMU_DIGMBC_ILOOPCNFG1_MAXERR1_13_0__MASK                       0x00003FFF

#define PMU_DIGMBC_ILOOPCNFG2_OFFSET                                      0x0000000C
#define PMU_DIGMBC_ILOOPCNFG2_TYPE                                        UInt32
#define PMU_DIGMBC_ILOOPCNFG2_RESERVED_MASK                               0xF0000000
#define    PMU_DIGMBC_ILOOPCNFG2_EN3DELAY_10_0__SHIFT                     17
#define    PMU_DIGMBC_ILOOPCNFG2_EN3DELAY_10_0__MASK                      0x0FFE0000
#define    PMU_DIGMBC_ILOOPCNFG2_PENDELAY_10_0__SHIFT                     6
#define    PMU_DIGMBC_ILOOPCNFG2_PENDELAY_10_0__MASK                      0x0001FFC0
#define    PMU_DIGMBC_ILOOPCNFG2_ISAMPTIME_5_0__SHIFT                     0
#define    PMU_DIGMBC_ILOOPCNFG2_ISAMPTIME_5_0__MASK                      0x0000003F

#define PMU_DIGMBC_ILOOPCNFG3_OFFSET                                      0x00000010
#define PMU_DIGMBC_ILOOPCNFG3_TYPE                                        UInt32
#define PMU_DIGMBC_ILOOPCNFG3_RESERVED_MASK                               0xFC000000
#define    PMU_DIGMBC_ILOOPCNFG3_METHOD_1_0__SHIFT                        24
#define    PMU_DIGMBC_ILOOPCNFG3_METHOD_1_0__MASK                         0x03000000
#define    PMU_DIGMBC_ILOOPCNFG3_ILOOP_BB1_9_0__SHIFT                     14
#define    PMU_DIGMBC_ILOOPCNFG3_ILOOP_BB1_9_0__MASK                      0x00FFC000
#define    PMU_DIGMBC_ILOOPCNFG3_BETASAMP_13_0__SHIFT                     0
#define    PMU_DIGMBC_ILOOPCNFG3_BETASAMP_13_0__MASK                      0x00003FFF

#define PMU_DIGMBC_VILOOPCNFG1_OFFSET                                     0x00000014
#define PMU_DIGMBC_VILOOPCNFG1_TYPE                                       UInt32
#define PMU_DIGMBC_VILOOPCNFG1_RESERVED_MASK                              0xFC000000
#define    PMU_DIGMBC_VILOOPCNFG1_VLOOP_BB0_5_0__SHIFT                    10
#define    PMU_DIGMBC_VILOOPCNFG1_VLOOP_BB0_5_0__MASK                     0x03FFFC00
#define    PMU_DIGMBC_VILOOPCNFG1_VLOOP_AA1_9_0__SHIFT                    0
#define    PMU_DIGMBC_VILOOPCNFG1_VLOOP_AA1_9_0__MASK                     0x000003FF

#define PMU_DIGMBC_VILOOPCNFG2_OFFSET                                     0x00000018
#define PMU_DIGMBC_VILOOPCNFG2_TYPE                                       UInt32
#define PMU_DIGMBC_VILOOPCNFG2_RESERVED_MASK                              0xFC000000
#define    PMU_DIGMBC_VILOOPCNFG2_VERR0_9_0__SHIFT                        16
#define    PMU_DIGMBC_VILOOPCNFG2_VERR0_9_0__MASK                         0x03FF0000
#define    PMU_DIGMBC_VILOOPCNFG2_VLOOP_BB1_15_0__SHIFT                   0
#define    PMU_DIGMBC_VILOOPCNFG2_VLOOP_BB1_15_0__MASK                    0x0000FFFF

#define PMU_DIGMBC_VILOOPCNFG3_OFFSET                                     0x0000001C
#define PMU_DIGMBC_VILOOPCNFG3_TYPE                                       UInt32
#define PMU_DIGMBC_VILOOPCNFG3_RESERVED_MASK                              0xFFFFFC00
#define    PMU_DIGMBC_VILOOPCNFG3_ISTEPMAX_9_0__SHIFT                     0
#define    PMU_DIGMBC_VILOOPCNFG3_ISTEPMAX_9_0__MASK                      0x000003FF

#define PMU_DIGMBC_DACCNFG1_OFFSET                                        0x00000020
#define PMU_DIGMBC_DACCNFG1_TYPE                                          UInt32
#define PMU_DIGMBC_DACCNFG1_RESERVED_MASK                                 0xFFE0FC00
#define    PMU_DIGMBC_DACCNFG1_DACCTRL_4_0__SHIFT                         16
#define    PMU_DIGMBC_DACCNFG1_DACCTRL_4_0__MASK                          0x001F0000
#define    PMU_DIGMBC_DACCNFG1_DACSTEP_9_0__SHIFT                         0
#define    PMU_DIGMBC_DACCNFG1_DACSTEP_9_0__MASK                          0x000003FF

#define PMU_DIGMBC_ILOOPCNFG4_OFFSET                                      0x00000024
#define PMU_DIGMBC_ILOOPCNFG4_TYPE                                        UInt32
#define PMU_DIGMBC_ILOOPCNFG4_RESERVED_MASK                               0xF8000000
#define    PMU_DIGMBC_ILOOPCNFG4_IGAIN_STARTUP_TIME_2_0__SHIFT            24
#define    PMU_DIGMBC_ILOOPCNFG4_IGAIN_STARTUP_TIME_2_0__MASK             0x07000000
#define    PMU_DIGMBC_ILOOPCNFG4_ILOOP_BB2_9_0__SHIFT                     14
#define    PMU_DIGMBC_ILOOPCNFG4_ILOOP_BB2_9_0__MASK                      0x00FFC000
#define    PMU_DIGMBC_ILOOPCNFG4_MAXERR2_13_0__SHIFT                      0
#define    PMU_DIGMBC_ILOOPCNFG4_MAXERR2_13_0__MASK                       0x00003FFF

#define PMU_DIGMBC_PLOOPCNFG1_OFFSET                                      0x00000028
#define PMU_DIGMBC_PLOOPCNFG1_TYPE                                        UInt32
#define PMU_DIGMBC_PLOOPCNFG1_RESERVED_MASK                               0xF800E000
#define    PMU_DIGMBC_PLOOPCNFG1_NSET_10_0__SHIFT                         16
#define    PMU_DIGMBC_PLOOPCNFG1_NSET_10_0__MASK                          0x07FF0000
#define    PMU_DIGMBC_PLOOPCNFG1_PDSET_12_0__SHIFT                        0
#define    PMU_DIGMBC_PLOOPCNFG1_PDSET_12_0__MASK                         0x00001FFF

#define PMU_DIGMBC_DIGMBCATE1_OFFSET                                      0x0000002C
#define PMU_DIGMBC_DIGMBCATE1_TYPE                                        UInt32
#define PMU_DIGMBC_DIGMBCATE1_RESERVED_MASK                               0xFFFFE000
#define    PMU_DIGMBC_DIGMBCATE1_DACCTRL_ATE_12_0__SHIFT                  0
#define    PMU_DIGMBC_DIGMBCATE1_DACCTRL_ATE_12_0__MASK                   0x00001FFF

#define PMU_DIGMBC_DIGMBCATE2_OFFSET                                      0x00000030
#define PMU_DIGMBC_DIGMBCATE2_TYPE                                        UInt32
#define PMU_DIGMBC_DIGMBCATE2_RESERVED_MASK                               0xFFFFC000
#define    PMU_DIGMBC_DIGMBCATE2_DIGMBC_ICHG_13_0__SHIFT                  0
#define    PMU_DIGMBC_DIGMBCATE2_DIGMBC_ICHG_13_0__MASK                   0x00003FFF

#endif /* __BRCM_RDB_PMU_DIGMBC_H__ */



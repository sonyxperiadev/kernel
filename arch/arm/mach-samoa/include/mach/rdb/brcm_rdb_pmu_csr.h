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

#ifndef __BRCM_RDB_PMU_CSR_H__
#define __BRCM_RDB_PMU_CSR_H__

#define PMU_CSR_CSRPMCTRL_OFFSET                                          0x00000000
#define PMU_CSR_CSRPMCTRL_TYPE                                            UInt32
#define PMU_CSR_CSRPMCTRL_RESERVED_MASK                                   0xFFFFF800
#define    PMU_CSR_CSRPMCTRL_CSRGRP_2_0__SHIFT                            8
#define    PMU_CSR_CSRPMCTRL_CSRGRP_2_0__MASK                             0x00000700
#define    PMU_CSR_CSRPMCTRL_CSR_PM3_1_0__SHIFT                           6
#define    PMU_CSR_CSRPMCTRL_CSR_PM3_1_0__MASK                            0x000000C0
#define    PMU_CSR_CSRPMCTRL_CSR_PM2_1_0__SHIFT                           4
#define    PMU_CSR_CSRPMCTRL_CSR_PM2_1_0__MASK                            0x00000030
#define    PMU_CSR_CSRPMCTRL_CSRL_PM1_1_0__SHIFT                          2
#define    PMU_CSR_CSRPMCTRL_CSRL_PM1_1_0__MASK                           0x0000000C
#define    PMU_CSR_CSRPMCTRL_CSR_PM0_1_0__SHIFT                           0
#define    PMU_CSR_CSRPMCTRL_CSR_PM0_1_0__MASK                            0x00000003

#define PMU_CSR_IOSRPMCTRL_OFFSET                                         0x00000004
#define PMU_CSR_IOSRPMCTRL_TYPE                                           UInt32
#define PMU_CSR_IOSRPMCTRL_RESERVED_MASK                                  0xFFFFF800
#define    PMU_CSR_IOSRPMCTRL_IOSRGRP_2_0__SHIFT                          8
#define    PMU_CSR_IOSRPMCTRL_IOSRGRP_2_0__MASK                           0x00000700
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM3_1_0__SHIFT                         6
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM3_1_0__MASK                          0x000000C0
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM2_1_0__SHIFT                         4
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM2_1_0__MASK                          0x00000030
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM1_1_0__SHIFT                         2
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM1_1_0__MASK                          0x0000000C
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM0_1_0__SHIFT                         0
#define    PMU_CSR_IOSRPMCTRL_IOSR_PM0_1_0__MASK                          0x00000003

#define PMU_CSR_CSRVOUT1_OFFSET                                           0x00000008
#define PMU_CSR_CSRVOUT1_TYPE                                             UInt32
#define PMU_CSR_CSRVOUT1_RESERVED_MASK                                    0xFFFFFFE0
#define    PMU_CSR_CSRVOUT1_CSR_VOUT_NM1_4_0__SHIFT                       0
#define    PMU_CSR_CSRVOUT1_CSR_VOUT_NM1_4_0__MASK                        0x0000001F

#define PMU_CSR_CSRVOUT2_OFFSET                                           0x0000000C
#define PMU_CSR_CSRVOUT2_TYPE                                             UInt32
#define PMU_CSR_CSRVOUT2_RESERVED_MASK                                    0xFFFFFFE0
#define    PMU_CSR_CSRVOUT2_CSR_VOUT_LPM_4_0__SHIFT                       0
#define    PMU_CSR_CSRVOUT2_CSR_VOUT_LPM_4_0__MASK                        0x0000001F

#define PMU_CSR_CSRVOUT3_OFFSET                                           0x00000010
#define PMU_CSR_CSRVOUT3_TYPE                                             UInt32
#define PMU_CSR_CSRVOUT3_RESERVED_MASK                                    0xFFFFFFE0
#define    PMU_CSR_CSRVOUT3_CSR_VOUT_NM2_4_0__SHIFT                       0
#define    PMU_CSR_CSRVOUT3_CSR_VOUT_NM2_4_0__MASK                        0x0000001F

#define PMU_CSR_IOSRVOUT1_OFFSET                                          0x00000014
#define PMU_CSR_IOSRVOUT1_TYPE                                            UInt32
#define PMU_CSR_IOSRVOUT1_RESERVED_MASK                                   0xFFFFFFF8
#define    PMU_CSR_IOSRVOUT1_IOSR_VOUT_NM1_2_0__SHIFT                     0
#define    PMU_CSR_IOSRVOUT1_IOSR_VOUT_NM1_2_0__MASK                      0x00000007

#define PMU_CSR_IOSRVOUT2_OFFSET                                          0x00000018
#define PMU_CSR_IOSRVOUT2_TYPE                                            UInt32
#define PMU_CSR_IOSRVOUT2_RESERVED_MASK                                   0xFFFFFFF8
#define    PMU_CSR_IOSRVOUT2_IOSR_VOUT_LPM_2_0__SHIFT                     0
#define    PMU_CSR_IOSRVOUT2_IOSR_VOUT_LPM_2_0__MASK                      0x00000007

#define PMU_CSR_IOSRVOUT3_OFFSET                                          0x0000001C
#define PMU_CSR_IOSRVOUT3_TYPE                                            UInt32
#define PMU_CSR_IOSRVOUT3_RESERVED_MASK                                   0xFFFFFFF8
#define    PMU_CSR_IOSRVOUT3_IOSR_VOUT_NM2_2_0__SHIFT                     0
#define    PMU_CSR_IOSRVOUT3_IOSR_VOUT_NM2_2_0__MASK                      0x00000007

#define PMU_CSR_CSR_VTRIM_OFFSET                                          0x00000020
#define PMU_CSR_CSR_VTRIM_TYPE                                            UInt32
#define PMU_CSR_CSR_VTRIM_RESERVED_MASK                                   0xFFFFFFF0
#define    PMU_CSR_CSR_VTRIM_CSR_VTRIM_3_0__SHIFT                         0
#define    PMU_CSR_CSR_VTRIM_CSR_VTRIM_3_0__MASK                          0x0000000F

#define PMU_CSR_CSRCTRL1_OFFSET                                           0x00000024
#define PMU_CSR_CSRCTRL1_TYPE                                             UInt32
#define PMU_CSR_CSRCTRL1_RESERVED_MASK                                    0x00000000
#define    PMU_CSR_CSRCTRL1_CSR_CNTL_31_0__SHIFT                          0
#define    PMU_CSR_CSRCTRL1_CSR_CNTL_31_0__MASK                           0xFFFFFFFF

#define PMU_CSR_CSRCTRL2_OFFSET                                           0x00000028
#define PMU_CSR_CSRCTRL2_TYPE                                             UInt32
#define PMU_CSR_CSRCTRL2_RESERVED_MASK                                    0xF8000000
#define    PMU_CSR_CSRCTRL2_CSR_CNTL_58_32__SHIFT                         0
#define    PMU_CSR_CSRCTRL2_CSR_CNTL_58_32__MASK                          0x07FFFFFF

#define PMU_CSR_IOSRCTRL1_OFFSET                                          0x0000002C
#define PMU_CSR_IOSRCTRL1_TYPE                                            UInt32
#define PMU_CSR_IOSRCTRL1_RESERVED_MASK                                   0x00000000
#define    PMU_CSR_IOSRCTRL1_IOSR_CNTL_31_0__SHIFT                        0
#define    PMU_CSR_IOSRCTRL1_IOSR_CNTL_31_0__MASK                         0xFFFFFFFF

#define PMU_CSR_IOSRCTRL2_OFFSET                                          0x00000030
#define PMU_CSR_IOSRCTRL2_TYPE                                            UInt32
#define PMU_CSR_IOSRCTRL2_RESERVED_MASK                                   0xFF000000
#define    PMU_CSR_IOSRCTRL2_IOSR_CNTL_55_32__SHIFT                       0
#define    PMU_CSR_IOSRCTRL2_IOSR_CNTL_55_32__MASK                        0x00FFFFFF

#define PMU_CSR_SRCTRL_OFFSET                                             0x00000034
#define PMU_CSR_SRCTRL_TYPE                                               UInt32
#define PMU_CSR_SRCTRL_RESERVED_MASK                                      0xFFFE0000
#define    PMU_CSR_SRCTRL_SR_PLLON_SHIFT                                  16
#define    PMU_CSR_SRCTRL_SR_PLLON_MASK                                   0x00010000
#define    PMU_CSR_SRCTRL_SR_CNTL_15_0__SHIFT                             0
#define    PMU_CSR_SRCTRL_SR_CNTL_15_0__MASK                              0x0000FFFF

#endif /* __BRCM_RDB_PMU_CSR_H__ */



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

#ifndef __BRCM_RDB_CORTEXA7_PMU_H__
#define __BRCM_RDB_CORTEXA7_PMU_H__

#define CORTEXA7_PMU_PMXEVCNTR0_OFFSET                                    0x00000000
#define CORTEXA7_PMU_PMXEVCNTR0_TYPE                                      UInt32
#define CORTEXA7_PMU_PMXEVCNTR0_RESERVED_MASK                             0x00000000
#define    CORTEXA7_PMU_PMXEVCNTR0_PMXEVCNTR0_SHIFT                       0
#define    CORTEXA7_PMU_PMXEVCNTR0_PMXEVCNTR0_MASK                        0xFFFFFFFF

#define CORTEXA7_PMU_PMXEVCNTR1_OFFSET                                    0x00000004
#define CORTEXA7_PMU_PMXEVCNTR1_TYPE                                      UInt32
#define CORTEXA7_PMU_PMXEVCNTR1_RESERVED_MASK                             0x00000000
#define    CORTEXA7_PMU_PMXEVCNTR1_PMXEVCNTR1_SHIFT                       0
#define    CORTEXA7_PMU_PMXEVCNTR1_PMXEVCNTR1_MASK                        0xFFFFFFFF

#define CORTEXA7_PMU_PMXEVCNTR2_OFFSET                                    0x00000008
#define CORTEXA7_PMU_PMXEVCNTR2_TYPE                                      UInt32
#define CORTEXA7_PMU_PMXEVCNTR2_RESERVED_MASK                             0x00000000
#define    CORTEXA7_PMU_PMXEVCNTR2_PMXEVCNTR2_SHIFT                       0
#define    CORTEXA7_PMU_PMXEVCNTR2_PMXEVCNTR2_MASK                        0xFFFFFFFF

#define CORTEXA7_PMU_PMXEVCNTR3_OFFSET                                    0x0000000C
#define CORTEXA7_PMU_PMXEVCNTR3_TYPE                                      UInt32
#define CORTEXA7_PMU_PMXEVCNTR3_RESERVED_MASK                             0x00000000
#define    CORTEXA7_PMU_PMXEVCNTR3_PMXEVCNTR3_SHIFT                       0
#define    CORTEXA7_PMU_PMXEVCNTR3_PMXEVCNTR3_MASK                        0xFFFFFFFF

#define CORTEXA7_PMU_PMCCNTR_OFFSET                                       0x0000007C
#define CORTEXA7_PMU_PMCCNTR_TYPE                                         UInt32
#define CORTEXA7_PMU_PMCCNTR_RESERVED_MASK                                0x00000000
#define    CORTEXA7_PMU_PMCCNTR_PMCCNTR_SHIFT                             0
#define    CORTEXA7_PMU_PMCCNTR_PMCCNTR_MASK                              0xFFFFFFFF

#define CORTEXA7_PMU_PMXEVTYPER0_OFFSET                                   0x00000400
#define CORTEXA7_PMU_PMXEVTYPER0_TYPE                                     UInt32
#define CORTEXA7_PMU_PMXEVTYPER0_RESERVED_MASK                            0xFFFFFF00
#define    CORTEXA7_PMU_PMXEVTYPER0_EVTCOUNT_SHIFT                        0
#define    CORTEXA7_PMU_PMXEVTYPER0_EVTCOUNT_MASK                         0x000000FF

#define CORTEXA7_PMU_PMXEVTYPER1_OFFSET                                   0x00000404
#define CORTEXA7_PMU_PMXEVTYPER1_TYPE                                     UInt32
#define CORTEXA7_PMU_PMXEVTYPER1_RESERVED_MASK                            0xFFFFFF00
#define    CORTEXA7_PMU_PMXEVTYPER1_EVTCOUNT_SHIFT                        0
#define    CORTEXA7_PMU_PMXEVTYPER1_EVTCOUNT_MASK                         0x000000FF

#define CORTEXA7_PMU_PMXEVTYPER2_OFFSET                                   0x00000408
#define CORTEXA7_PMU_PMXEVTYPER2_TYPE                                     UInt32
#define CORTEXA7_PMU_PMXEVTYPER2_RESERVED_MASK                            0xFFFFFF00
#define    CORTEXA7_PMU_PMXEVTYPER2_EVTCOUNT_SHIFT                        0
#define    CORTEXA7_PMU_PMXEVTYPER2_EVTCOUNT_MASK                         0x000000FF

#define CORTEXA7_PMU_PMXEVTYPER3_OFFSET                                   0x0000040C
#define CORTEXA7_PMU_PMXEVTYPER3_TYPE                                     UInt32
#define CORTEXA7_PMU_PMXEVTYPER3_RESERVED_MASK                            0xFFFFFF00
#define    CORTEXA7_PMU_PMXEVTYPER3_EVTCOUNT_SHIFT                        0
#define    CORTEXA7_PMU_PMXEVTYPER3_EVTCOUNT_MASK                         0x000000FF

#define CORTEXA7_PMU_PMXEVTYPER31_OFFSET                                  0x0000047C
#define CORTEXA7_PMU_PMXEVTYPER31_TYPE                                    UInt32
#define CORTEXA7_PMU_PMXEVTYPER31_RESERVED_MASK                           0xFFFFFF00
#define    CORTEXA7_PMU_PMXEVTYPER31_EVTCOUNT_SHIFT                       0
#define    CORTEXA7_PMU_PMXEVTYPER31_EVTCOUNT_MASK                        0x000000FF

#define CORTEXA7_PMU_PMCNTENSET_OFFSET                                    0x00000C00
#define CORTEXA7_PMU_PMCNTENSET_TYPE                                      UInt32
#define CORTEXA7_PMU_PMCNTENSET_RESERVED_MASK                             0x7FFFFFC0
#define    CORTEXA7_PMU_PMCNTENSET_C_SHIFT                                31
#define    CORTEXA7_PMU_PMCNTENSET_C_MASK                                 0x80000000
#define    CORTEXA7_PMU_PMCNTENSET_P5_SHIFT                               5
#define    CORTEXA7_PMU_PMCNTENSET_P5_MASK                                0x00000020
#define    CORTEXA7_PMU_PMCNTENSET_P4_SHIFT                               4
#define    CORTEXA7_PMU_PMCNTENSET_P4_MASK                                0x00000010
#define    CORTEXA7_PMU_PMCNTENSET_P3_SHIFT                               3
#define    CORTEXA7_PMU_PMCNTENSET_P3_MASK                                0x00000008
#define    CORTEXA7_PMU_PMCNTENSET_P2_SHIFT                               2
#define    CORTEXA7_PMU_PMCNTENSET_P2_MASK                                0x00000004
#define    CORTEXA7_PMU_PMCNTENSET_P1_SHIFT                               1
#define    CORTEXA7_PMU_PMCNTENSET_P1_MASK                                0x00000002
#define    CORTEXA7_PMU_PMCNTENSET_P0_SHIFT                               0
#define    CORTEXA7_PMU_PMCNTENSET_P0_MASK                                0x00000001

#define CORTEXA7_PMU_PMCNTENCLR_OFFSET                                    0x00000C20
#define CORTEXA7_PMU_PMCNTENCLR_TYPE                                      UInt32
#define CORTEXA7_PMU_PMCNTENCLR_RESERVED_MASK                             0x7FFFFFC0
#define    CORTEXA7_PMU_PMCNTENCLR_C_SHIFT                                31
#define    CORTEXA7_PMU_PMCNTENCLR_C_MASK                                 0x80000000
#define    CORTEXA7_PMU_PMCNTENCLR_P5_SHIFT                               5
#define    CORTEXA7_PMU_PMCNTENCLR_P5_MASK                                0x00000020
#define    CORTEXA7_PMU_PMCNTENCLR_P4_SHIFT                               4
#define    CORTEXA7_PMU_PMCNTENCLR_P4_MASK                                0x00000010
#define    CORTEXA7_PMU_PMCNTENCLR_P3_SHIFT                               3
#define    CORTEXA7_PMU_PMCNTENCLR_P3_MASK                                0x00000008
#define    CORTEXA7_PMU_PMCNTENCLR_P2_SHIFT                               2
#define    CORTEXA7_PMU_PMCNTENCLR_P2_MASK                                0x00000004
#define    CORTEXA7_PMU_PMCNTENCLR_P1_SHIFT                               1
#define    CORTEXA7_PMU_PMCNTENCLR_P1_MASK                                0x00000002
#define    CORTEXA7_PMU_PMCNTENCLR_P0_SHIFT                               0
#define    CORTEXA7_PMU_PMCNTENCLR_P0_MASK                                0x00000001

#define CORTEXA7_PMU_PMINTENSET_OFFSET                                    0x00000C40
#define CORTEXA7_PMU_PMINTENSET_TYPE                                      UInt32
#define CORTEXA7_PMU_PMINTENSET_RESERVED_MASK                             0x7FFFFFC0
#define    CORTEXA7_PMU_PMINTENSET_C_SHIFT                                31
#define    CORTEXA7_PMU_PMINTENSET_C_MASK                                 0x80000000
#define    CORTEXA7_PMU_PMINTENSET_P5_SHIFT                               5
#define    CORTEXA7_PMU_PMINTENSET_P5_MASK                                0x00000020
#define    CORTEXA7_PMU_PMINTENSET_P4_SHIFT                               4
#define    CORTEXA7_PMU_PMINTENSET_P4_MASK                                0x00000010
#define    CORTEXA7_PMU_PMINTENSET_P3_SHIFT                               3
#define    CORTEXA7_PMU_PMINTENSET_P3_MASK                                0x00000008
#define    CORTEXA7_PMU_PMINTENSET_P2_SHIFT                               2
#define    CORTEXA7_PMU_PMINTENSET_P2_MASK                                0x00000004
#define    CORTEXA7_PMU_PMINTENSET_P1_SHIFT                               1
#define    CORTEXA7_PMU_PMINTENSET_P1_MASK                                0x00000002
#define    CORTEXA7_PMU_PMINTENSET_P0_SHIFT                               0
#define    CORTEXA7_PMU_PMINTENSET_P0_MASK                                0x00000001

#define CORTEXA7_PMU_PMINTENCLR_OFFSET                                    0x00000C60
#define CORTEXA7_PMU_PMINTENCLR_TYPE                                      UInt32
#define CORTEXA7_PMU_PMINTENCLR_RESERVED_MASK                             0x7FFFFFC0
#define    CORTEXA7_PMU_PMINTENCLR_C_SHIFT                                31
#define    CORTEXA7_PMU_PMINTENCLR_C_MASK                                 0x80000000
#define    CORTEXA7_PMU_PMINTENCLR_P5_SHIFT                               5
#define    CORTEXA7_PMU_PMINTENCLR_P5_MASK                                0x00000020
#define    CORTEXA7_PMU_PMINTENCLR_P4_SHIFT                               4
#define    CORTEXA7_PMU_PMINTENCLR_P4_MASK                                0x00000010
#define    CORTEXA7_PMU_PMINTENCLR_P3_SHIFT                               3
#define    CORTEXA7_PMU_PMINTENCLR_P3_MASK                                0x00000008
#define    CORTEXA7_PMU_PMINTENCLR_P2_SHIFT                               2
#define    CORTEXA7_PMU_PMINTENCLR_P2_MASK                                0x00000004
#define    CORTEXA7_PMU_PMINTENCLR_P1_SHIFT                               1
#define    CORTEXA7_PMU_PMINTENCLR_P1_MASK                                0x00000002
#define    CORTEXA7_PMU_PMINTENCLR_P0_SHIFT                               0
#define    CORTEXA7_PMU_PMINTENCLR_P0_MASK                                0x00000001

#define CORTEXA7_PMU_PMOVSR_OFFSET                                        0x00000C80
#define CORTEXA7_PMU_PMOVSR_TYPE                                          UInt32
#define CORTEXA7_PMU_PMOVSR_RESERVED_MASK                                 0x7FFFFFC0
#define    CORTEXA7_PMU_PMOVSR_C_SHIFT                                    31
#define    CORTEXA7_PMU_PMOVSR_C_MASK                                     0x80000000
#define    CORTEXA7_PMU_PMOVSR_P5_SHIFT                                   5
#define    CORTEXA7_PMU_PMOVSR_P5_MASK                                    0x00000020
#define    CORTEXA7_PMU_PMOVSR_P4_SHIFT                                   4
#define    CORTEXA7_PMU_PMOVSR_P4_MASK                                    0x00000010
#define    CORTEXA7_PMU_PMOVSR_P3_SHIFT                                   3
#define    CORTEXA7_PMU_PMOVSR_P3_MASK                                    0x00000008
#define    CORTEXA7_PMU_PMOVSR_P2_SHIFT                                   2
#define    CORTEXA7_PMU_PMOVSR_P2_MASK                                    0x00000004
#define    CORTEXA7_PMU_PMOVSR_P1_SHIFT                                   1
#define    CORTEXA7_PMU_PMOVSR_P1_MASK                                    0x00000002
#define    CORTEXA7_PMU_PMOVSR_P0_SHIFT                                   0
#define    CORTEXA7_PMU_PMOVSR_P0_MASK                                    0x00000001

#define CORTEXA7_PMU_PMSWINC_OFFSET                                       0x00000CA0
#define CORTEXA7_PMU_PMSWINC_TYPE                                         UInt32
#define CORTEXA7_PMU_PMSWINC_RESERVED_MASK                                0xFFFFFFC0
#define    CORTEXA7_PMU_PMSWINC_P5_SHIFT                                  5
#define    CORTEXA7_PMU_PMSWINC_P5_MASK                                   0x00000020
#define    CORTEXA7_PMU_PMSWINC_P4_SHIFT                                  4
#define    CORTEXA7_PMU_PMSWINC_P4_MASK                                   0x00000010
#define    CORTEXA7_PMU_PMSWINC_P3_SHIFT                                  3
#define    CORTEXA7_PMU_PMSWINC_P3_MASK                                   0x00000008
#define    CORTEXA7_PMU_PMSWINC_P2_SHIFT                                  2
#define    CORTEXA7_PMU_PMSWINC_P2_MASK                                   0x00000004
#define    CORTEXA7_PMU_PMSWINC_P1_SHIFT                                  1
#define    CORTEXA7_PMU_PMSWINC_P1_MASK                                   0x00000002
#define    CORTEXA7_PMU_PMSWINC_P0_SHIFT                                  0
#define    CORTEXA7_PMU_PMSWINC_P0_MASK                                   0x00000001

#define CORTEXA7_PMU_PMCFGR_OFFSET                                        0x00000E00
#define CORTEXA7_PMU_PMCFGR_TYPE                                          UInt32
#define CORTEXA7_PMU_PMCFGR_RESERVED_MASK                                 0xFFF02700
#define    CORTEXA7_PMU_PMCFGR_UEN_SHIFT                                  19
#define    CORTEXA7_PMU_PMCFGR_UEN_MASK                                   0x00080000
#define    CORTEXA7_PMU_PMCFGR_WT_SHIFT                                   18
#define    CORTEXA7_PMU_PMCFGR_WT_MASK                                    0x00040000
#define    CORTEXA7_PMU_PMCFGR_NA_SHIFT                                   17
#define    CORTEXA7_PMU_PMCFGR_NA_MASK                                    0x00020000
#define    CORTEXA7_PMU_PMCFGR_EX_SHIFT                                   16
#define    CORTEXA7_PMU_PMCFGR_EX_MASK                                    0x00010000
#define    CORTEXA7_PMU_PMCFGR_CCD_SHIFT                                  15
#define    CORTEXA7_PMU_PMCFGR_CCD_MASK                                   0x00008000
#define    CORTEXA7_PMU_PMCFGR_CC_SHIFT                                   14
#define    CORTEXA7_PMU_PMCFGR_CC_MASK                                    0x00004000
#define    CORTEXA7_PMU_PMCFGR_SIZE_SHIFT                                 11
#define    CORTEXA7_PMU_PMCFGR_SIZE_MASK                                  0x00001800
#define    CORTEXA7_PMU_PMCFGR_N_SHIFT                                    0
#define    CORTEXA7_PMU_PMCFGR_N_MASK                                     0x000000FF

#define CORTEXA7_PMU_PMCR_OFFSET                                          0x00000E04
#define CORTEXA7_PMU_PMCR_TYPE                                            UInt32
#define CORTEXA7_PMU_PMCR_RESERVED_MASK                                   0x000007C0
#define    CORTEXA7_PMU_PMCR_IMP_SHIFT                                    24
#define    CORTEXA7_PMU_PMCR_IMP_MASK                                     0xFF000000
#define    CORTEXA7_PMU_PMCR_IDCODE_SHIFT                                 16
#define    CORTEXA7_PMU_PMCR_IDCODE_MASK                                  0x00FF0000
#define    CORTEXA7_PMU_PMCR_N_SHIFT                                      11
#define    CORTEXA7_PMU_PMCR_N_MASK                                       0x0000F800
#define    CORTEXA7_PMU_PMCR_DP_SHIFT                                     5
#define    CORTEXA7_PMU_PMCR_DP_MASK                                      0x00000020
#define    CORTEXA7_PMU_PMCR_X_SHIFT                                      4
#define    CORTEXA7_PMU_PMCR_X_MASK                                       0x00000010
#define    CORTEXA7_PMU_PMCR_D_SHIFT                                      3
#define    CORTEXA7_PMU_PMCR_D_MASK                                       0x00000008
#define    CORTEXA7_PMU_PMCR_C_SHIFT                                      2
#define    CORTEXA7_PMU_PMCR_C_MASK                                       0x00000004
#define    CORTEXA7_PMU_PMCR_P_SHIFT                                      1
#define    CORTEXA7_PMU_PMCR_P_MASK                                       0x00000002
#define    CORTEXA7_PMU_PMCR_E_SHIFT                                      0
#define    CORTEXA7_PMU_PMCR_E_MASK                                       0x00000001

#define CORTEXA7_PMU_PMUSERENR_OFFSET                                     0x00000E08
#define CORTEXA7_PMU_PMUSERENR_TYPE                                       UInt32
#define CORTEXA7_PMU_PMUSERENR_RESERVED_MASK                              0x00000000
#define    CORTEXA7_PMU_PMUSERENR_PMUSERENR_SHIFT                         0
#define    CORTEXA7_PMU_PMUSERENR_PMUSERENR_MASK                          0xFFFFFFFF

#define CORTEXA7_PMU_PMCEID0_OFFSET                                       0x00000E20
#define CORTEXA7_PMU_PMCEID0_TYPE                                         UInt32
#define CORTEXA7_PMU_PMCEID0_RESERVED_MASK                                0xC0000000
#define    CORTEXA7_PMU_PMCEID0_BUSCYCLE_SHIFT                            29
#define    CORTEXA7_PMU_PMCEID0_BUSCYCLE_MASK                             0x20000000
#define    CORTEXA7_PMU_PMCEID0_PASSWRITETOTTB_SHIFT                      28
#define    CORTEXA7_PMU_PMCEID0_PASSWRITETOTTB_MASK                       0x10000000
#define    CORTEXA7_PMU_PMCEID0_SPECEXC_SHIFT                             27
#define    CORTEXA7_PMU_PMCEID0_SPECEXC_MASK                              0x08000000
#define    CORTEXA7_PMU_PMCEID0_LOCALMEMERR_SHIFT                         26
#define    CORTEXA7_PMU_PMCEID0_LOCALMEMERR_MASK                          0x04000000
#define    CORTEXA7_PMU_PMCEID0_BUSACCESS_SHIFT                           25
#define    CORTEXA7_PMU_PMCEID0_BUSACCESS_MASK                            0x02000000
#define    CORTEXA7_PMU_PMCEID0_L2DCACHEWB_SHIFT                          24
#define    CORTEXA7_PMU_PMCEID0_L2DCACHEWB_MASK                           0x01000000
#define    CORTEXA7_PMU_PMCEID0_L2DCACHEREFILL_SHIFT                      23
#define    CORTEXA7_PMU_PMCEID0_L2DCACHEREFILL_MASK                       0x00800000
#define    CORTEXA7_PMU_PMCEID0_L2DCACHEACCESS_SHIFT                      22
#define    CORTEXA7_PMU_PMCEID0_L2DCACHEACCESS_MASK                       0x00400000
#define    CORTEXA7_PMU_PMCEID0_L1DCACHEWB_SHIFT                          21
#define    CORTEXA7_PMU_PMCEID0_L1DCACHEWB_MASK                           0x00200000
#define    CORTEXA7_PMU_PMCEID0_L1ICACHEACCESS_SHIFT                      20
#define    CORTEXA7_PMU_PMCEID0_L1ICACHEACCESS_MASK                       0x00100000
#define    CORTEXA7_PMU_PMCEID0_DATAMEMACCESS_SHIFT                       19
#define    CORTEXA7_PMU_PMCEID0_DATAMEMACCESS_MASK                        0x00080000
#define    CORTEXA7_PMU_PMCEID0_PREDBRNSPECEXC_SHIFT                      18
#define    CORTEXA7_PMU_PMCEID0_PREDBRNSPECEXC_MASK                       0x00040000
#define    CORTEXA7_PMU_PMCEID0_CYCLE_SHIFT                               17
#define    CORTEXA7_PMU_PMCEID0_CYCLE_MASK                                0x00020000
#define    CORTEXA7_PMU_PMCEID0_MISPREDBRNSPECEXC_SHIFT                   16
#define    CORTEXA7_PMU_PMCEID0_MISPREDBRNSPECEXC_MASK                    0x00010000
#define    CORTEXA7_PMU_PMCEID0_PASSUNALGNLS_SHIFT                        15
#define    CORTEXA7_PMU_PMCEID0_PASSUNALGNLS_MASK                         0x00008000
#define    CORTEXA7_PMU_PMCEID0_PASSPRCRETURN_SHIFT                       14
#define    CORTEXA7_PMU_PMCEID0_PASSPRCRETURN_MASK                        0x00004000
#define    CORTEXA7_PMU_PMCEID0_IMMBRN_SHIFT                              13
#define    CORTEXA7_PMU_PMCEID0_IMMBRN_MASK                               0x00002000
#define    CORTEXA7_PMU_PMCEID0_PASSSWCHANGEPC_SHIFT                      12
#define    CORTEXA7_PMU_PMCEID0_PASSSWCHANGEPC_MASK                       0x00001000
#define    CORTEXA7_PMU_PMCEID0_PASSWRITETOCONTEXTIDR_SHIFT               11
#define    CORTEXA7_PMU_PMCEID0_PASSWRITETOCONTEXTIDR_MASK                0x00000800
#define    CORTEXA7_PMU_PMCEID0_PASSEXCRETURN_SHIFT                       10
#define    CORTEXA7_PMU_PMCEID0_PASSEXCRETURN_MASK                        0x00000400
#define    CORTEXA7_PMU_PMCEID0_EXCTAKEN_SHIFT                            9
#define    CORTEXA7_PMU_PMCEID0_EXCTAKEN_MASK                             0x00000200
#define    CORTEXA7_PMU_PMCEID0_INSTARCHEXC_SHIFT                         8
#define    CORTEXA7_PMU_PMCEID0_INSTARCHEXC_MASK                          0x00000100
#define    CORTEXA7_PMU_PMCEID0_PASSSTORE_SHIFT                           7
#define    CORTEXA7_PMU_PMCEID0_PASSSTORE_MASK                            0x00000080
#define    CORTEXA7_PMU_PMCEID0_PASSLOAD_SHIFT                            6
#define    CORTEXA7_PMU_PMCEID0_PASSLOAD_MASK                             0x00000040
#define    CORTEXA7_PMU_PMCEID0_L1DATATLBREFILL_SHIFT                     5
#define    CORTEXA7_PMU_PMCEID0_L1DATATLBREFILL_MASK                      0x00000020
#define    CORTEXA7_PMU_PMCEID0_L1DCACHEACCESS_SHIFT                      4
#define    CORTEXA7_PMU_PMCEID0_L1DCACHEACCESS_MASK                       0x00000010
#define    CORTEXA7_PMU_PMCEID0_L1DCACHEREFILL_SHIFT                      3
#define    CORTEXA7_PMU_PMCEID0_L1DCACHEREFILL_MASK                       0x00000008
#define    CORTEXA7_PMU_PMCEID0_L1INSTTLBREFILL_SHIFT                     2
#define    CORTEXA7_PMU_PMCEID0_L1INSTTLBREFILL_MASK                      0x00000004
#define    CORTEXA7_PMU_PMCEID0_L1ICACHEREFILL_SHIFT                      1
#define    CORTEXA7_PMU_PMCEID0_L1ICACHEREFILL_MASK                       0x00000002
#define    CORTEXA7_PMU_PMCEID0_PASSSWINC_SHIFT                           0
#define    CORTEXA7_PMU_PMCEID0_PASSSWINC_MASK                            0x00000001

#define CORTEXA7_PMU_PMCEID1_OFFSET                                       0x00000E24
#define CORTEXA7_PMU_PMCEID1_TYPE                                         UInt32
#define CORTEXA7_PMU_PMCEID1_RESERVED_MASK                                0x00000000
#define    CORTEXA7_PMU_PMCEID1_PMCEID1_SHIFT                             0
#define    CORTEXA7_PMU_PMCEID1_PMCEID1_MASK                              0xFFFFFFFF

#define CORTEXA7_PMU_PMLAR_OFFSET                                         0x00000FB0
#define CORTEXA7_PMU_PMLAR_TYPE                                           UInt32
#define CORTEXA7_PMU_PMLAR_RESERVED_MASK                                  0x00000000
#define    CORTEXA7_PMU_PMLAR_LOCKACCESSCONTROL_SHIFT                     0
#define    CORTEXA7_PMU_PMLAR_LOCKACCESSCONTROL_MASK                      0xFFFFFFFF

#define CORTEXA7_PMU_PMLSR_OFFSET                                         0x00000FB4
#define CORTEXA7_PMU_PMLSR_TYPE                                           UInt32
#define CORTEXA7_PMU_PMLSR_RESERVED_MASK                                  0xFFFFFFF8
#define    CORTEXA7_PMU_PMLSR_NTT_SHIFT                                   2
#define    CORTEXA7_PMU_PMLSR_NTT_MASK                                    0x00000004
#define    CORTEXA7_PMU_PMLSR_SLK_SHIFT                                   1
#define    CORTEXA7_PMU_PMLSR_SLK_MASK                                    0x00000002
#define    CORTEXA7_PMU_PMLSR_SLI_SHIFT                                   0
#define    CORTEXA7_PMU_PMLSR_SLI_MASK                                    0x00000001

#define CORTEXA7_PMU_PMAUTHSTATUS_OFFSET                                  0x00000FB8
#define CORTEXA7_PMU_PMAUTHSTATUS_TYPE                                    UInt32
#define CORTEXA7_PMU_PMAUTHSTATUS_RESERVED_MASK                           0xFFFFFF00
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECNINVDBGFTIMP_SHIFT                7
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECNINVDBGFTIMP_MASK                 0x00000080
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECNINVDBGEN_SHIFT                   6
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECNINVDBGEN_MASK                    0x00000040
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECINVDBGFTIMP_SHIFT                 5
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECINVDBGFTIMP_MASK                  0x00000020
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECINVDBGEN_SHIFT                    4
#define    CORTEXA7_PMU_PMAUTHSTATUS_SECINVDBGEN_MASK                     0x00000010
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECNINVDBGFTIMP_SHIFT               3
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECNINVDBGFTIMP_MASK                0x00000008
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECNINVDBGEN_SHIFT                  2
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECNINVDBGEN_MASK                   0x00000004
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECINVDBGFTIMP_SHIFT                1
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECINVDBGFTIMP_MASK                 0x00000002
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECINVDBGEN_SHIFT                   0
#define    CORTEXA7_PMU_PMAUTHSTATUS_NSECINVDBGEN_MASK                    0x00000001

#define CORTEXA7_PMU_PMDEVTYPE_OFFSET                                     0x00000FCC
#define CORTEXA7_PMU_PMDEVTYPE_TYPE                                       UInt32
#define CORTEXA7_PMU_PMDEVTYPE_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_PMU_PMDEVTYPE_T_SHIFT                                 4
#define    CORTEXA7_PMU_PMDEVTYPE_T_MASK                                  0x000000F0
#define    CORTEXA7_PMU_PMDEVTYPE_C_SHIFT                                 0
#define    CORTEXA7_PMU_PMDEVTYPE_C_MASK                                  0x0000000F

#define CORTEXA7_PMU_PID4_OFFSET                                          0x00000FD0
#define CORTEXA7_PMU_PID4_TYPE                                            UInt32
#define CORTEXA7_PMU_PID4_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID4_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID4_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID5_OFFSET                                          0x00000FD4
#define CORTEXA7_PMU_PID5_TYPE                                            UInt32
#define CORTEXA7_PMU_PID5_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID5_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID5_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID6_OFFSET                                          0x00000FD8
#define CORTEXA7_PMU_PID6_TYPE                                            UInt32
#define CORTEXA7_PMU_PID6_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID6_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID6_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID7_OFFSET                                          0x00000FDC
#define CORTEXA7_PMU_PID7_TYPE                                            UInt32
#define CORTEXA7_PMU_PID7_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID7_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID7_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID0_OFFSET                                          0x00000FE0
#define CORTEXA7_PMU_PID0_TYPE                                            UInt32
#define CORTEXA7_PMU_PID0_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID0_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID0_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID1_OFFSET                                          0x00000FE4
#define CORTEXA7_PMU_PID1_TYPE                                            UInt32
#define CORTEXA7_PMU_PID1_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID1_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID1_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID2_OFFSET                                          0x00000FE8
#define CORTEXA7_PMU_PID2_TYPE                                            UInt32
#define CORTEXA7_PMU_PID2_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID2_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID2_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_PID3_OFFSET                                          0x00000FEC
#define CORTEXA7_PMU_PID3_TYPE                                            UInt32
#define CORTEXA7_PMU_PID3_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_PID3_PID_SHIFT                                    0
#define    CORTEXA7_PMU_PID3_PID_MASK                                     0x000000FF

#define CORTEXA7_PMU_CID0_OFFSET                                          0x00000FF0
#define CORTEXA7_PMU_CID0_TYPE                                            UInt32
#define CORTEXA7_PMU_CID0_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_CID0_CID_SHIFT                                    0
#define    CORTEXA7_PMU_CID0_CID_MASK                                     0x000000FF

#define CORTEXA7_PMU_CID1_OFFSET                                          0x00000FF4
#define CORTEXA7_PMU_CID1_TYPE                                            UInt32
#define CORTEXA7_PMU_CID1_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_CID1_CID_SHIFT                                    0
#define    CORTEXA7_PMU_CID1_CID_MASK                                     0x000000FF

#define CORTEXA7_PMU_CID2_OFFSET                                          0x00000FF8
#define CORTEXA7_PMU_CID2_TYPE                                            UInt32
#define CORTEXA7_PMU_CID2_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_CID2_CID_SHIFT                                    0
#define    CORTEXA7_PMU_CID2_CID_MASK                                     0x000000FF

#define CORTEXA7_PMU_CID3_OFFSET                                          0x00000FFC
#define CORTEXA7_PMU_CID3_TYPE                                            UInt32
#define CORTEXA7_PMU_CID3_RESERVED_MASK                                   0xFFFFFF00
#define    CORTEXA7_PMU_CID3_CID_SHIFT                                    0
#define    CORTEXA7_PMU_CID3_CID_MASK                                     0x000000FF

#endif /* __BRCM_RDB_CORTEXA7_PMU_H__ */



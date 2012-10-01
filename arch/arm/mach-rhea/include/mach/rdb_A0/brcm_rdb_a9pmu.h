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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_A9PMU_H__
#define __BRCM_RDB_A9PMU_H__

#define A9PMU_PMEVCNTR0_OFFSET                                            0x00000000
#define A9PMU_PMEVCNTR0_TYPE                                              UInt32
#define A9PMU_PMEVCNTR0_RESERVED_MASK                                     0x00000000
#define    A9PMU_PMEVCNTR0_EVENT_CNT_0_SHIFT                              0
#define    A9PMU_PMEVCNTR0_EVENT_CNT_0_MASK                               0xFFFFFFFF

#define A9PMU_PMEVCNTR1_OFFSET                                            0x00000004
#define A9PMU_PMEVCNTR1_TYPE                                              UInt32
#define A9PMU_PMEVCNTR1_RESERVED_MASK                                     0x00000000
#define    A9PMU_PMEVCNTR1_EVENT_CNT_1_SHIFT                              0
#define    A9PMU_PMEVCNTR1_EVENT_CNT_1_MASK                               0xFFFFFFFF

#define A9PMU_PMEVCNTR2_OFFSET                                            0x00000008
#define A9PMU_PMEVCNTR2_TYPE                                              UInt32
#define A9PMU_PMEVCNTR2_RESERVED_MASK                                     0x00000000
#define    A9PMU_PMEVCNTR2_EVENT_CNT_2_SHIFT                              0
#define    A9PMU_PMEVCNTR2_EVENT_CNT_2_MASK                               0xFFFFFFFF

#define A9PMU_PMEVCNTR3_OFFSET                                            0x0000000C
#define A9PMU_PMEVCNTR3_TYPE                                              UInt32
#define A9PMU_PMEVCNTR3_RESERVED_MASK                                     0x00000000
#define    A9PMU_PMEVCNTR3_EVENT_CNT_3_SHIFT                              0
#define    A9PMU_PMEVCNTR3_EVENT_CNT_3_MASK                               0xFFFFFFFF

#define A9PMU_PMEVCNTR4_OFFSET                                            0x00000010
#define A9PMU_PMEVCNTR4_TYPE                                              UInt32
#define A9PMU_PMEVCNTR4_RESERVED_MASK                                     0x00000000
#define    A9PMU_PMEVCNTR4_EVENT_CNT_4_SHIFT                              0
#define    A9PMU_PMEVCNTR4_EVENT_CNT_4_MASK                               0xFFFFFFFF

#define A9PMU_PMEVCNTR5_OFFSET                                            0x00000014
#define A9PMU_PMEVCNTR5_TYPE                                              UInt32
#define A9PMU_PMEVCNTR5_RESERVED_MASK                                     0x00000000
#define    A9PMU_PMEVCNTR5_EVENT_CNT_5_SHIFT                              0
#define    A9PMU_PMEVCNTR5_EVENT_CNT_5_MASK                               0xFFFFFFFF

#define A9PMU_PMCCNTR_OFFSET                                              0x0000007C
#define A9PMU_PMCCNTR_TYPE                                                UInt32
#define A9PMU_PMCCNTR_RESERVED_MASK                                       0x00000000
#define    A9PMU_PMCCNTR_CYCLE_CNT_SHIFT                                  0
#define    A9PMU_PMCCNTR_CYCLE_CNT_MASK                                   0xFFFFFFFF

#define A9PMU_PMXEVTYPER0_OFFSET                                          0x00000400
#define A9PMU_PMXEVTYPER0_TYPE                                            UInt32
#define A9PMU_PMXEVTYPER0_RESERVED_MASK                                   0xFFFFFF00
#define    A9PMU_PMXEVTYPER0_EVTSEL_0_SHIFT                               0
#define    A9PMU_PMXEVTYPER0_EVTSEL_0_MASK                                0x000000FF

#define A9PMU_PMXEVTYPER1_OFFSET                                          0x00000404
#define A9PMU_PMXEVTYPER1_TYPE                                            UInt32
#define A9PMU_PMXEVTYPER1_RESERVED_MASK                                   0xFFFFFF00
#define    A9PMU_PMXEVTYPER1_EVTSEL_1_SHIFT                               0
#define    A9PMU_PMXEVTYPER1_EVTSEL_1_MASK                                0x000000FF

#define A9PMU_PMXEVTYPER2_OFFSET                                          0x00000408
#define A9PMU_PMXEVTYPER2_TYPE                                            UInt32
#define A9PMU_PMXEVTYPER2_RESERVED_MASK                                   0xFFFFFF00
#define    A9PMU_PMXEVTYPER2_EVTSEL_2_SHIFT                               0
#define    A9PMU_PMXEVTYPER2_EVTSEL_2_MASK                                0x000000FF

#define A9PMU_PMXEVTYPER3_OFFSET                                          0x0000040C
#define A9PMU_PMXEVTYPER3_TYPE                                            UInt32
#define A9PMU_PMXEVTYPER3_RESERVED_MASK                                   0xFFFFFF00
#define    A9PMU_PMXEVTYPER3_EVTSEL_3_SHIFT                               0
#define    A9PMU_PMXEVTYPER3_EVTSEL_3_MASK                                0x000000FF

#define A9PMU_PMXEVTYPER4_OFFSET                                          0x00000410
#define A9PMU_PMXEVTYPER4_TYPE                                            UInt32
#define A9PMU_PMXEVTYPER4_RESERVED_MASK                                   0xFFFFFF00
#define    A9PMU_PMXEVTYPER4_EVTSEL_4_SHIFT                               0
#define    A9PMU_PMXEVTYPER4_EVTSEL_4_MASK                                0x000000FF

#define A9PMU_PMXEVTYPER5_OFFSET                                          0x00000414
#define A9PMU_PMXEVTYPER5_TYPE                                            UInt32
#define A9PMU_PMXEVTYPER5_RESERVED_MASK                                   0xFFFFFF00
#define    A9PMU_PMXEVTYPER5_EVTSEL_5_SHIFT                               0
#define    A9PMU_PMXEVTYPER5_EVTSEL_5_MASK                                0x000000FF

#define A9PMU_PMCNTENSET_OFFSET                                           0x00000C00
#define A9PMU_PMCNTENSET_TYPE                                             UInt32
#define A9PMU_PMCNTENSET_RESERVED_MASK                                    0x7FFFFFC0
#define    A9PMU_PMCNTENSET_CYCLE_CNT_ENSET_SHIFT                         31
#define    A9PMU_PMCNTENSET_CYCLE_CNT_ENSET_MASK                          0x80000000
#define    A9PMU_PMCNTENSET_COUNTER_ENSET_SHIFT                           0
#define    A9PMU_PMCNTENSET_COUNTER_ENSET_MASK                            0x0000003F

#define A9PMU_PMCNTENCLR_OFFSET                                           0x00000C20
#define A9PMU_PMCNTENCLR_TYPE                                             UInt32
#define A9PMU_PMCNTENCLR_RESERVED_MASK                                    0x7FFFFFC0
#define    A9PMU_PMCNTENCLR_CYCLE_CNT_ENCLR_SHIFT                         31
#define    A9PMU_PMCNTENCLR_CYCLE_CNT_ENCLR_MASK                          0x80000000
#define    A9PMU_PMCNTENCLR_COUNTER_ENCLR_SHIFT                           0
#define    A9PMU_PMCNTENCLR_COUNTER_ENCLR_MASK                            0x0000003F

#define A9PMU_PMINTENSET_OFFSET                                           0x00000C40
#define A9PMU_PMINTENSET_TYPE                                             UInt32
#define A9PMU_PMINTENSET_RESERVED_MASK                                    0x7FFFFFC0
#define    A9PMU_PMINTENSET_CYCLE_CNT_OVF_IENSET_SHIFT                    31
#define    A9PMU_PMINTENSET_CYCLE_CNT_OVF_IENSET_MASK                     0x80000000
#define    A9PMU_PMINTENSET_COUNTER_OVF_IENSET_SHIFT                      0
#define    A9PMU_PMINTENSET_COUNTER_OVF_IENSET_MASK                       0x0000003F

#define A9PMU_PMINTENCLR_OFFSET                                           0x00000C60
#define A9PMU_PMINTENCLR_TYPE                                             UInt32
#define A9PMU_PMINTENCLR_RESERVED_MASK                                    0x7FFFFFC0
#define    A9PMU_PMINTENCLR_CYCLE_CNT_OVF_IENCLR_SHIFT                    31
#define    A9PMU_PMINTENCLR_CYCLE_CNT_OVF_IENCLR_MASK                     0x80000000
#define    A9PMU_PMINTENCLR_COUNTER_OVF_IENCLR_SHIFT                      0
#define    A9PMU_PMINTENCLR_COUNTER_OVF_IENCLR_MASK                       0x0000003F

#define A9PMU_PMOVSR_OFFSET                                               0x00000C80
#define A9PMU_PMOVSR_TYPE                                                 UInt32
#define A9PMU_PMOVSR_RESERVED_MASK                                        0x7FFFFFC0
#define    A9PMU_PMOVSR_CYCLE_CNT_OVF_SHIFT                               31
#define    A9PMU_PMOVSR_CYCLE_CNT_OVF_MASK                                0x80000000
#define    A9PMU_PMOVSR_COUNTER_OVF_SHIFT                                 0
#define    A9PMU_PMOVSR_COUNTER_OVF_MASK                                  0x0000003F

#define A9PMU_PMSWINC_OFFSET                                              0x00000CA0
#define A9PMU_PMSWINC_TYPE                                                UInt32
#define A9PMU_PMSWINC_RESERVED_MASK                                       0xFFFFFFC0
#define    A9PMU_PMSWINC_COUNTER_INC_SHIFT                                0
#define    A9PMU_PMSWINC_COUNTER_INC_MASK                                 0x0000003F

#define A9PMU_PMCR_OFFSET                                                 0x00000E04
#define A9PMU_PMCR_TYPE                                                   UInt32
#define A9PMU_PMCR_RESERVED_MASK                                          0x000007C0
#define    A9PMU_PMCR_IMP_SHIFT                                           24
#define    A9PMU_PMCR_IMP_MASK                                            0xFF000000
#define    A9PMU_PMCR_IDCODE_SHIFT                                        16
#define    A9PMU_PMCR_IDCODE_MASK                                         0x00FF0000
#define    A9PMU_PMCR_NUM_CTRS_SHIFT                                      11
#define    A9PMU_PMCR_NUM_CTRS_MASK                                       0x0000F800
#define    A9PMU_PMCR_DIS_CYCLE_CTR_SHIFT                                 5
#define    A9PMU_PMCR_DIS_CYCLE_CTR_MASK                                  0x00000020
#define    A9PMU_PMCR_EN_EXPORT_EVENTS_SHIFT                              4
#define    A9PMU_PMCR_EN_EXPORT_EVENTS_MASK                               0x00000010
#define    A9PMU_PMCR_CYCLE_CNT_DIV_SHIFT                                 3
#define    A9PMU_PMCR_CYCLE_CNT_DIV_MASK                                  0x00000008
#define    A9PMU_PMCR_CYCLE_CNT_RESET_SHIFT                               2
#define    A9PMU_PMCR_CYCLE_CNT_RESET_MASK                                0x00000004
#define    A9PMU_PMCR_PERF_CNT_RESET_SHIFT                                1
#define    A9PMU_PMCR_PERF_CNT_RESET_MASK                                 0x00000002
#define    A9PMU_PMCR_EN_COUNTERS_SHIFT                                   0
#define    A9PMU_PMCR_EN_COUNTERS_MASK                                    0x00000001

#define A9PMU_PMUSERENR_OFFSET                                            0x00000E08
#define A9PMU_PMUSERENR_TYPE                                              UInt32
#define A9PMU_PMUSERENR_RESERVED_MASK                                     0xFFFFFFFE
#define    A9PMU_PMUSERENR_USER_EN_SHIFT                                  0
#define    A9PMU_PMUSERENR_USER_EN_MASK                                   0x00000001

#endif /* __BRCM_RDB_A9PMU_H__ */



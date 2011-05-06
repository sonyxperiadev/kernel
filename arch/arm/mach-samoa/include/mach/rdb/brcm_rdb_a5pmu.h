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

#ifndef __BRCM_RDB_A5PMU_H__
#define __BRCM_RDB_A5PMU_H__

#define A5PMU_PMEVCNTR0_OFFSET                                            0x00000000
#define A5PMU_PMEVCNTR0_TYPE                                              UInt32
#define A5PMU_PMEVCNTR0_RESERVED_MASK                                     0x00000000
#define    A5PMU_PMEVCNTR0_EVENT_CNT_0_SHIFT                              0
#define    A5PMU_PMEVCNTR0_EVENT_CNT_0_MASK                               0xFFFFFFFF

#define A5PMU_PMEVCNTR1_OFFSET                                            0x00000004
#define A5PMU_PMEVCNTR1_TYPE                                              UInt32
#define A5PMU_PMEVCNTR1_RESERVED_MASK                                     0x00000000
#define    A5PMU_PMEVCNTR1_EVENT_CNT_1_SHIFT                              0
#define    A5PMU_PMEVCNTR1_EVENT_CNT_1_MASK                               0xFFFFFFFF

#define A5PMU_PMCCNTR_OFFSET                                              0x0000007C
#define A5PMU_PMCCNTR_TYPE                                                UInt32
#define A5PMU_PMCCNTR_RESERVED_MASK                                       0x00000000
#define    A5PMU_PMCCNTR_CYCLE_CNT_SHIFT                                  0
#define    A5PMU_PMCCNTR_CYCLE_CNT_MASK                                   0xFFFFFFFF

#define A5PMU_PMXEVTYPER0_OFFSET                                          0x00000400
#define A5PMU_PMXEVTYPER0_TYPE                                            UInt32
#define A5PMU_PMXEVTYPER0_RESERVED_MASK                                   0xFFFFFF00
#define    A5PMU_PMXEVTYPER0_EVTSEL_0_SHIFT                               0
#define    A5PMU_PMXEVTYPER0_EVTSEL_0_MASK                                0x000000FF

#define A5PMU_PMXEVTYPER1_OFFSET                                          0x00000404
#define A5PMU_PMXEVTYPER1_TYPE                                            UInt32
#define A5PMU_PMXEVTYPER1_RESERVED_MASK                                   0xFFFFFF00
#define    A5PMU_PMXEVTYPER1_EVTSEL_1_SHIFT                               0
#define    A5PMU_PMXEVTYPER1_EVTSEL_1_MASK                                0x000000FF

#define A5PMU_PMCCFILTR_OFFSET                                            0x0000047C
#define A5PMU_PMCCFILTR_TYPE                                              UInt32
#define A5PMU_PMCCFILTR_RESERVED_MASK                                     0x0FFFFFFF
#define    A5PMU_PMCCFILTR_CC_P_SHIFT                                     31
#define    A5PMU_PMCCFILTR_CC_P_MASK                                      0x80000000
#define    A5PMU_PMCCFILTR_CC_U_SHIFT                                     30
#define    A5PMU_PMCCFILTR_CC_U_MASK                                      0x40000000
#define    A5PMU_PMCCFILTR_CC_NSK_SHIFT                                   29
#define    A5PMU_PMCCFILTR_CC_NSK_MASK                                    0x20000000
#define    A5PMU_PMCCFILTR_CC_NSU_SHIFT                                   28
#define    A5PMU_PMCCFILTR_CC_NSU_MASK                                    0x10000000

#define A5PMU_PMCNTENSET_OFFSET                                           0x00000C00
#define A5PMU_PMCNTENSET_TYPE                                             UInt32
#define A5PMU_PMCNTENSET_RESERVED_MASK                                    0x7FFFFFFC
#define    A5PMU_PMCNTENSET_CYCLE_CNT_ENSET_SHIFT                         31
#define    A5PMU_PMCNTENSET_CYCLE_CNT_ENSET_MASK                          0x80000000
#define    A5PMU_PMCNTENSET_COUNTER_ENSET_SHIFT                           0
#define    A5PMU_PMCNTENSET_COUNTER_ENSET_MASK                            0x00000003

#define A5PMU_PMCNTENCLR_OFFSET                                           0x00000C20
#define A5PMU_PMCNTENCLR_TYPE                                             UInt32
#define A5PMU_PMCNTENCLR_RESERVED_MASK                                    0x7FFFFFFC
#define    A5PMU_PMCNTENCLR_CYCLE_CNT_ENCLR_SHIFT                         31
#define    A5PMU_PMCNTENCLR_CYCLE_CNT_ENCLR_MASK                          0x80000000
#define    A5PMU_PMCNTENCLR_COUNTER_ENCLR_SHIFT                           0
#define    A5PMU_PMCNTENCLR_COUNTER_ENCLR_MASK                            0x00000003

#define A5PMU_PMINTENSET_OFFSET                                           0x00000C40
#define A5PMU_PMINTENSET_TYPE                                             UInt32
#define A5PMU_PMINTENSET_RESERVED_MASK                                    0x7FFFFFFC
#define    A5PMU_PMINTENSET_CYCLE_CNT_OVF_IENSET_SHIFT                    31
#define    A5PMU_PMINTENSET_CYCLE_CNT_OVF_IENSET_MASK                     0x80000000
#define    A5PMU_PMINTENSET_COUNTER_OVF_IENSET_SHIFT                      0
#define    A5PMU_PMINTENSET_COUNTER_OVF_IENSET_MASK                       0x00000003

#define A5PMU_PMINTENCLR_OFFSET                                           0x00000C60
#define A5PMU_PMINTENCLR_TYPE                                             UInt32
#define A5PMU_PMINTENCLR_RESERVED_MASK                                    0x7FFFFFFC
#define    A5PMU_PMINTENCLR_CYCLE_CNT_OVF_IENCLR_SHIFT                    31
#define    A5PMU_PMINTENCLR_CYCLE_CNT_OVF_IENCLR_MASK                     0x80000000
#define    A5PMU_PMINTENCLR_COUNTER_OVF_IENCLR_SHIFT                      0
#define    A5PMU_PMINTENCLR_COUNTER_OVF_IENCLR_MASK                       0x00000003

#define A5PMU_PMOVSR_OFFSET                                               0x00000C80
#define A5PMU_PMOVSR_TYPE                                                 UInt32
#define A5PMU_PMOVSR_RESERVED_MASK                                        0x7FFFFFFC
#define    A5PMU_PMOVSR_CYCLE_CNT_OVF_SHIFT                               31
#define    A5PMU_PMOVSR_CYCLE_CNT_OVF_MASK                                0x80000000
#define    A5PMU_PMOVSR_COUNTER_OVF_SHIFT                                 0
#define    A5PMU_PMOVSR_COUNTER_OVF_MASK                                  0x00000003

#define A5PMU_PMSWINC_OFFSET                                              0x00000CA0
#define A5PMU_PMSWINC_TYPE                                                UInt32
#define A5PMU_PMSWINC_RESERVED_MASK                                       0xFFFFFFFC
#define    A5PMU_PMSWINC_COUNTER_INC_SHIFT                                0
#define    A5PMU_PMSWINC_COUNTER_INC_MASK                                 0x00000003

#define A5PMU_PMCFGR_OFFSET                                               0x00000E00
#define A5PMU_PMCFGR_TYPE                                                 UInt32
#define A5PMU_PMCFGR_RESERVED_MASK                                        0xFFF627E0
#define    A5PMU_PMCFGR_UEN_SHIFT                                         19
#define    A5PMU_PMCFGR_UEN_MASK                                          0x00080000
#define    A5PMU_PMCFGR_EX_SHIFT                                          16
#define    A5PMU_PMCFGR_EX_MASK                                           0x00010000
#define    A5PMU_PMCFGR_CCD_SHIFT                                         15
#define    A5PMU_PMCFGR_CCD_MASK                                          0x00008000
#define    A5PMU_PMCFGR_CC_SHIFT                                          14
#define    A5PMU_PMCFGR_CC_MASK                                           0x00004000
#define    A5PMU_PMCFGR_SIZE_SHIFT                                        11
#define    A5PMU_PMCFGR_SIZE_MASK                                         0x00001800
#define    A5PMU_PMCFGR_N_SHIFT                                           0
#define    A5PMU_PMCFGR_N_MASK                                            0x0000001F

#define A5PMU_PMCR_OFFSET                                                 0x00000E04
#define A5PMU_PMCR_TYPE                                                   UInt32
#define A5PMU_PMCR_RESERVED_MASK                                          0x000007C0
#define    A5PMU_PMCR_IMP_SHIFT                                           24
#define    A5PMU_PMCR_IMP_MASK                                            0xFF000000
#define    A5PMU_PMCR_IDCODE_SHIFT                                        16
#define    A5PMU_PMCR_IDCODE_MASK                                         0x00FF0000
#define    A5PMU_PMCR_NUM_CTRS_SHIFT                                      11
#define    A5PMU_PMCR_NUM_CTRS_MASK                                       0x0000F800
#define    A5PMU_PMCR_DIS_CYCLE_CTR_SHIFT                                 5
#define    A5PMU_PMCR_DIS_CYCLE_CTR_MASK                                  0x00000020
#define    A5PMU_PMCR_EN_EXPORT_EVENTS_SHIFT                              4
#define    A5PMU_PMCR_EN_EXPORT_EVENTS_MASK                               0x00000010
#define    A5PMU_PMCR_CYCLE_CNT_DIV_SHIFT                                 3
#define    A5PMU_PMCR_CYCLE_CNT_DIV_MASK                                  0x00000008
#define    A5PMU_PMCR_CYCLE_CNT_RESET_SHIFT                               2
#define    A5PMU_PMCR_CYCLE_CNT_RESET_MASK                                0x00000004
#define    A5PMU_PMCR_PERF_CNT_RESET_SHIFT                                1
#define    A5PMU_PMCR_PERF_CNT_RESET_MASK                                 0x00000002
#define    A5PMU_PMCR_EN_COUNTERS_SHIFT                                   0
#define    A5PMU_PMCR_EN_COUNTERS_MASK                                    0x00000001

#define A5PMU_PMUSERENR_OFFSET                                            0x00000E08
#define A5PMU_PMUSERENR_TYPE                                              UInt32
#define A5PMU_PMUSERENR_RESERVED_MASK                                     0xFFFFFFFE
#define    A5PMU_PMUSERENR_USER_EN_SHIFT                                  0
#define    A5PMU_PMUSERENR_USER_EN_MASK                                   0x00000001

#define A5PMU_PMCEID0_OFFSET                                              0x00000E20
#define A5PMU_PMCEID0_TYPE                                                UInt32
#define A5PMU_PMCEID0_RESERVED_MASK                                       0xC0000000
#define    A5PMU_PMCEID0_BUS_CYCLE_SHIFT                                  29
#define    A5PMU_PMCEID0_BUS_CYCLE_MASK                                   0x20000000
#define    A5PMU_PMCEID0_WR_TRANS_SHIFT                                   28
#define    A5PMU_PMCEID0_WR_TRANS_MASK                                    0x10000000
#define    A5PMU_PMCEID0_INSTR_SPEC_SHIFT                                 27
#define    A5PMU_PMCEID0_INSTR_SPEC_MASK                                  0x08000000
#define    A5PMU_PMCEID0_LOCAL_MEMERR_SHIFT                               26
#define    A5PMU_PMCEID0_LOCAL_MEMERR_MASK                                0x04000000
#define    A5PMU_PMCEID0_BUS_ACCESS_SHIFT                                 25
#define    A5PMU_PMCEID0_BUS_ACCESS_MASK                                  0x02000000
#define    A5PMU_PMCEID0_L2_DATA_WB_SHIFT                                 24
#define    A5PMU_PMCEID0_L2_DATA_WB_MASK                                  0x01000000
#define    A5PMU_PMCEID0_L2_DATA_REFILL_SHIFT                             23
#define    A5PMU_PMCEID0_L2_DATA_REFILL_MASK                              0x00800000
#define    A5PMU_PMCEID0_L2_DATA_ACCESS_SHIFT                             22
#define    A5PMU_PMCEID0_L2_DATA_ACCESS_MASK                              0x00400000
#define    A5PMU_PMCEID0_L1_DATA_WB_SHIFT                                 21
#define    A5PMU_PMCEID0_L1_DATA_WB_MASK                                  0x00200000
#define    A5PMU_PMCEID0_L1_INSTR_ACC_SHIFT                               20
#define    A5PMU_PMCEID0_L1_INSTR_ACC_MASK                                0x00100000
#define    A5PMU_PMCEID0_DATA_ACCESS_SHIFT                                19
#define    A5PMU_PMCEID0_DATA_ACCESS_MASK                                 0x00080000
#define    A5PMU_PMCEID0_PRED_BR_SPEC_SHIFT                               18
#define    A5PMU_PMCEID0_PRED_BR_SPEC_MASK                                0x00040000
#define    A5PMU_PMCEID0_CYCLE_SHIFT                                      17
#define    A5PMU_PMCEID0_CYCLE_MASK                                       0x00020000
#define    A5PMU_PMCEID0_MISPRED_BR_SHIFT                                 16
#define    A5PMU_PMCEID0_MISPRED_BR_MASK                                  0x00010000
#define    A5PMU_PMCEID0_UNALIGNED_SHIFT                                  15
#define    A5PMU_PMCEID0_UNALIGNED_MASK                                   0x00008000
#define    A5PMU_PMCEID0_PROC_RET_SHIFT                                   14
#define    A5PMU_PMCEID0_PROC_RET_MASK                                    0x00004000
#define    A5PMU_PMCEID0_IMMED_BR_SHIFT                                   13
#define    A5PMU_PMCEID0_IMMED_BR_MASK                                    0x00002000
#define    A5PMU_PMCEID0_SW_CHG_PC_SHIFT                                  12
#define    A5PMU_PMCEID0_SW_CHG_PC_MASK                                   0x00001000
#define    A5PMU_PMCEID0_WR_CONTEXT_SHIFT                                 11
#define    A5PMU_PMCEID0_WR_CONTEXT_MASK                                  0x00000800
#define    A5PMU_PMCEID0_EXCEPT_RET_SHIFT                                 10
#define    A5PMU_PMCEID0_EXCEPT_RET_MASK                                  0x00000400
#define    A5PMU_PMCEID0_EXCEPT_TAKE_SHIFT                                9
#define    A5PMU_PMCEID0_EXCEPT_TAKE_MASK                                 0x00000200
#define    A5PMU_PMCEID0_INSTR_ARCH_EX_SHIFT                              8
#define    A5PMU_PMCEID0_INSTR_ARCH_EX_MASK                               0x00000100
#define    A5PMU_PMCEID0_STORE_SHIFT                                      7
#define    A5PMU_PMCEID0_STORE_MASK                                       0x00000080
#define    A5PMU_PMCEID0_LOAD_SHIFT                                       6
#define    A5PMU_PMCEID0_LOAD_MASK                                        0x00000040
#define    A5PMU_PMCEID0_L1_DATA_TLB_SHIFT                                5
#define    A5PMU_PMCEID0_L1_DATA_TLB_MASK                                 0x00000020
#define    A5PMU_PMCEID0_L1_DATA_ACC_SHIFT                                4
#define    A5PMU_PMCEID0_L1_DATA_ACC_MASK                                 0x00000010
#define    A5PMU_PMCEID0_L1_DATA_REF_SHIFT                                3
#define    A5PMU_PMCEID0_L1_DATA_REF_MASK                                 0x00000008
#define    A5PMU_PMCEID0_L1_INSTR_TLB_SHIFT                               2
#define    A5PMU_PMCEID0_L1_INSTR_TLB_MASK                                0x00000004
#define    A5PMU_PMCEID0_L1_INSTR_REF_SHIFT                               1
#define    A5PMU_PMCEID0_L1_INSTR_REF_MASK                                0x00000002
#define    A5PMU_PMCEID0_SW_INCR_SHIFT                                    0
#define    A5PMU_PMCEID0_SW_INCR_MASK                                     0x00000001

#define A5PMU_PMLAR_OFFSET                                                0x00000FB0
#define A5PMU_PMLAR_TYPE                                                  UInt32
#define A5PMU_PMLAR_RESERVED_MASK                                         0x00000000
#define    A5PMU_PMLAR_LOCK_ACCESS_SHIFT                                  0
#define    A5PMU_PMLAR_LOCK_ACCESS_MASK                                   0xFFFFFFFF

#define A5PMU_PMLSR_OFFSET                                                0x00000FB4
#define A5PMU_PMLSR_TYPE                                                  UInt32
#define A5PMU_PMLSR_RESERVED_MASK                                         0xFFFFFFF8
#define    A5PMU_PMLSR_ACCESS_32BIT_SHIFT                                 2
#define    A5PMU_PMLSR_ACCESS_32BIT_MASK                                  0x00000004
#define    A5PMU_PMLSR_LOCKED_SHIFT                                       1
#define    A5PMU_PMLSR_LOCKED_MASK                                        0x00000002
#define    A5PMU_PMLSR_LOCK_IMPLEMENTED_SHIFT                             0
#define    A5PMU_PMLSR_LOCK_IMPLEMENTED_MASK                              0x00000001

#define A5PMU_PMAUTHSTATUS_OFFSET                                         0x00000FB8
#define A5PMU_PMAUTHSTATUS_TYPE                                           UInt32
#define A5PMU_PMAUTHSTATUS_RESERVED_MASK                                  0xFFFFFF00
#define    A5PMU_PMAUTHSTATUS_SEC_NON_DBG_SHIFT                           6
#define    A5PMU_PMAUTHSTATUS_SEC_NON_DBG_MASK                            0x000000C0
#define    A5PMU_PMAUTHSTATUS_SEC_INV_DBG_SHIFT                           4
#define    A5PMU_PMAUTHSTATUS_SEC_INV_DBG_MASK                            0x00000030
#define    A5PMU_PMAUTHSTATUS_OPEN_NON_DBG_SHIFT                          2
#define    A5PMU_PMAUTHSTATUS_OPEN_NON_DBG_MASK                           0x0000000C
#define    A5PMU_PMAUTHSTATUS_OPEN_INV_DBG_SHIFT                          0
#define    A5PMU_PMAUTHSTATUS_OPEN_INV_DBG_MASK                           0x00000003

#define A5PMU_PMDEVTYPE_OFFSET                                            0x00000FCC
#define A5PMU_PMDEVTYPE_TYPE                                              UInt32
#define A5PMU_PMDEVTYPE_RESERVED_MASK                                     0xFFFFFF00
#define    A5PMU_PMDEVTYPE_SUB_TYPE_SHIFT                                 4
#define    A5PMU_PMDEVTYPE_SUB_TYPE_MASK                                  0x000000F0
#define    A5PMU_PMDEVTYPE_MAIN_CLASS_SHIFT                               0
#define    A5PMU_PMDEVTYPE_MAIN_CLASS_MASK                                0x0000000F

#define A5PMU_PERID4_OFFSET                                               0x00000FD0
#define A5PMU_PERID4_TYPE                                                 UInt32
#define A5PMU_PERID4_RESERVED_MASK                                        0xFFFFFF00
#define    A5PMU_PERID4_PID4_SHIFT                                        0
#define    A5PMU_PERID4_PID4_MASK                                         0x000000FF

#define A5PMU_PERID0_OFFSET                                               0x00000FE0
#define A5PMU_PERID0_TYPE                                                 UInt32
#define A5PMU_PERID0_RESERVED_MASK                                        0xFFFFFF00
#define    A5PMU_PERID0_PID0_SHIFT                                        0
#define    A5PMU_PERID0_PID0_MASK                                         0x000000FF

#define A5PMU_PERID1_OFFSET                                               0x00000FE4
#define A5PMU_PERID1_TYPE                                                 UInt32
#define A5PMU_PERID1_RESERVED_MASK                                        0xFFFFFF00
#define    A5PMU_PERID1_PID1_SHIFT                                        0
#define    A5PMU_PERID1_PID1_MASK                                         0x000000FF

#define A5PMU_PERID2_OFFSET                                               0x00000FE8
#define A5PMU_PERID2_TYPE                                                 UInt32
#define A5PMU_PERID2_RESERVED_MASK                                        0xFFFFFF00
#define    A5PMU_PERID2_PID2_SHIFT                                        0
#define    A5PMU_PERID2_PID2_MASK                                         0x000000FF

#define A5PMU_PERID3_OFFSET                                               0x00000FEC
#define A5PMU_PERID3_TYPE                                                 UInt32
#define A5PMU_PERID3_RESERVED_MASK                                        0xFFFFFF00
#define    A5PMU_PERID3_PID3_SHIFT                                        0
#define    A5PMU_PERID3_PID3_MASK                                         0x000000FF

#define A5PMU_COMPID0_OFFSET                                              0x00000FF0
#define A5PMU_COMPID0_TYPE                                                UInt32
#define A5PMU_COMPID0_RESERVED_MASK                                       0xFFFFFF00
#define    A5PMU_COMPID0_CID0_SHIFT                                       0
#define    A5PMU_COMPID0_CID0_MASK                                        0x000000FF

#define A5PMU_COMPID1_OFFSET                                              0x00000FF4
#define A5PMU_COMPID1_TYPE                                                UInt32
#define A5PMU_COMPID1_RESERVED_MASK                                       0xFFFFFF00
#define    A5PMU_COMPID1_CID1_SHIFT                                       0
#define    A5PMU_COMPID1_CID1_MASK                                        0x000000FF

#define A5PMU_COMPID2_OFFSET                                              0x00000FF8
#define A5PMU_COMPID2_TYPE                                                UInt32
#define A5PMU_COMPID2_RESERVED_MASK                                       0xFFFFFF00
#define    A5PMU_COMPID2_CID2_SHIFT                                       0
#define    A5PMU_COMPID2_CID2_MASK                                        0x000000FF

#define A5PMU_COMPID3_OFFSET                                              0x00000FFC
#define A5PMU_COMPID3_TYPE                                                UInt32
#define A5PMU_COMPID3_RESERVED_MASK                                       0xFFFFFF00
#define    A5PMU_COMPID3_CID3_SHIFT                                       0
#define    A5PMU_COMPID3_CID3_MASK                                        0x000000FF

#endif /* __BRCM_RDB_A5PMU_H__ */



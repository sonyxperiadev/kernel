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

#ifndef __BRCM_RDB_A9CPU_H__
#define __BRCM_RDB_A9CPU_H__

#define A9CPU_DIDR_OFFSET                                                 0x00000000
#define A9CPU_DIDR_TYPE                                                   UInt32
#define A9CPU_DIDR_RESERVED_MASK                                          0x00008F00
#define    A9CPU_DIDR_WRP_SHIFT                                           28
#define    A9CPU_DIDR_WRP_MASK                                            0xF0000000
#define    A9CPU_DIDR_BRP_SHIFT                                           24
#define    A9CPU_DIDR_BRP_MASK                                            0x0F000000
#define    A9CPU_DIDR_CONTEXT_SHIFT                                       20
#define    A9CPU_DIDR_CONTEXT_MASK                                        0x00F00000
#define    A9CPU_DIDR_DBG_ARCH_VER_SHIFT                                  16
#define    A9CPU_DIDR_DBG_ARCH_VER_MASK                                   0x000F0000
#define    A9CPU_DIDR_SECURE_HALT_SHIFT                                   14
#define    A9CPU_DIDR_SECURE_HALT_MASK                                    0x00004000
#define    A9CPU_DIDR_PC_SAMPLING_REG_SHIFT                               13
#define    A9CPU_DIDR_PC_SAMPLING_REG_MASK                                0x00002000
#define    A9CPU_DIDR_SECURITY_EXTS_SHIFT                                 12
#define    A9CPU_DIDR_SECURITY_EXTS_MASK                                  0x00001000
#define    A9CPU_DIDR_VARIANT_SHIFT                                       4
#define    A9CPU_DIDR_VARIANT_MASK                                        0x000000F0
#define    A9CPU_DIDR_REVISION_SHIFT                                      0
#define    A9CPU_DIDR_REVISION_MASK                                       0x0000000F

#define A9CPU_WFAR_OFFSET                                                 0x00000018
#define A9CPU_WFAR_TYPE                                                   UInt32
#define A9CPU_WFAR_RESERVED_MASK                                          0x00000001
#define    A9CPU_WFAR_VADDR_WPT_SHIFT                                     1
#define    A9CPU_WFAR_VADDR_WPT_MASK                                      0xFFFFFFFE

#define A9CPU_VCR_OFFSET                                                  0x0000001C
#define A9CPU_VCR_TYPE                                                    UInt32
#define A9CPU_VCR_RESERVED_MASK                                           0x21FF21FF
#define    A9CPU_VCR_FIQ_OPEN_SHIFT                                       31
#define    A9CPU_VCR_FIQ_OPEN_MASK                                        0x80000000
#define    A9CPU_VCR_IRQ_OPEN_SHIFT                                       30
#define    A9CPU_VCR_IRQ_OPEN_MASK                                        0x40000000
#define    A9CPU_VCR_DABORT_OPEN_SHIFT                                    28
#define    A9CPU_VCR_DABORT_OPEN_MASK                                     0x10000000
#define    A9CPU_VCR_PABORT_OPEN_SHIFT                                    27
#define    A9CPU_VCR_PABORT_OPEN_MASK                                     0x08000000
#define    A9CPU_VCR_SVC_OPEN_SHIFT                                       26
#define    A9CPU_VCR_SVC_OPEN_MASK                                        0x04000000
#define    A9CPU_VCR_UNDEF_OPEN_SHIFT                                     25
#define    A9CPU_VCR_UNDEF_OPEN_MASK                                      0x02000000
#define    A9CPU_VCR_FIQ_SEC_SHIFT                                        15
#define    A9CPU_VCR_FIQ_SEC_MASK                                         0x00008000
#define    A9CPU_VCR_IRQ_SEC_SHIFT                                        14
#define    A9CPU_VCR_IRQ_SEC_MASK                                         0x00004000
#define    A9CPU_VCR_DABORT_SEC_SHIFT                                     12
#define    A9CPU_VCR_DABORT_SEC_MASK                                      0x00001000
#define    A9CPU_VCR_PABORT_SEC_SHIFT                                     11
#define    A9CPU_VCR_PABORT_SEC_MASK                                      0x00000800
#define    A9CPU_VCR_SVC_SEC_SHIFT                                        10
#define    A9CPU_VCR_SVC_SEC_MASK                                         0x00000400
#define    A9CPU_VCR_UNDEF_SEC_SHIFT                                      9
#define    A9CPU_VCR_UNDEF_SEC_MASK                                       0x00000200

#define A9CPU_DTRRX_OFFSET                                                0x00000080
#define A9CPU_DTRRX_TYPE                                                  UInt32
#define A9CPU_DTRRX_RESERVED_MASK                                         0x00000000
#define    A9CPU_DTRRX_DATAA_SHIFT                                        0
#define    A9CPU_DTRRX_DATAA_MASK                                         0xFFFFFFFF

#define A9CPU_PCSR_OFFSET                                                 0x00000084
#define A9CPU_PCSR_TYPE                                                   UInt32
#define A9CPU_PCSR_RESERVED_MASK                                          0x00000000
#define    A9CPU_PCSR_PC_SAMPLE_SHIFT                                     2
#define    A9CPU_PCSR_PC_SAMPLE_MASK                                      0xFFFFFFFC
#define    A9CPU_PCSR_SAMPLE_MEANING_SHIFT                                0
#define    A9CPU_PCSR_SAMPLE_MEANING_MASK                                 0x00000003

#define A9CPU_DSCR_OFFSET                                                 0x00000088
#define A9CPU_DSCR_TYPE                                                   UInt32
#define A9CPU_DSCR_RESERVED_MASK                                          0x90C00200
#define    A9CPU_DSCR_DTRRXFULL_SHIFT                                     30
#define    A9CPU_DSCR_DTRRXFULL_MASK                                      0x40000000
#define    A9CPU_DSCR_DTRTXFULL_SHIFT                                     29
#define    A9CPU_DSCR_DTRTXFULL_MASK                                      0x20000000
#define    A9CPU_DSCR_DTRRXFULL_L_SHIFT                                   27
#define    A9CPU_DSCR_DTRRXFULL_L_MASK                                    0x08000000
#define    A9CPU_DSCR_DTRTXFULL_L_SHIFT                                   26
#define    A9CPU_DSCR_DTRTXFULL_L_MASK                                    0x04000000
#define    A9CPU_DSCR_SICKY_PIPE_SHIFT                                    25
#define    A9CPU_DSCR_SICKY_PIPE_MASK                                     0x02000000
#define    A9CPU_DSCR_INSTRCOMPL_L_SHIFT                                  24
#define    A9CPU_DSCR_INSTRCOMPL_L_MASK                                   0x01000000
#define    A9CPU_DSCR_DTR_MODE_SHIFT                                      20
#define    A9CPU_DSCR_DTR_MODE_MASK                                       0x00300000
#define    A9CPU_DSCR_DISCARD_IMP_SHIFT                                   19
#define    A9CPU_DSCR_DISCARD_IMP_MASK                                    0x00080000
#define    A9CPU_DSCR_OPEN_STATUS_SHIFT                                   18
#define    A9CPU_DSCR_OPEN_STATUS_MASK                                    0x00040000
#define    A9CPU_DSCR_SEC_PRIV_NON_DIS_SHIFT                              17
#define    A9CPU_DSCR_SEC_PRIV_NON_DIS_MASK                               0x00020000
#define    A9CPU_DSCR_SEC_PRIV_INV_DIS_SHIFT                              16
#define    A9CPU_DSCR_SEC_PRIV_INV_DIS_MASK                               0x00010000
#define    A9CPU_DSCR_MONITOR_MODE_SHIFT                                  15
#define    A9CPU_DSCR_MONITOR_MODE_MASK                                   0x00008000
#define    A9CPU_DSCR_HALTING_MODE_SHIFT                                  14
#define    A9CPU_DSCR_HALTING_MODE_MASK                                   0x00004000
#define    A9CPU_DSCR_EXEC_INSTR_EN_SHIFT                                 13
#define    A9CPU_DSCR_EXEC_INSTR_EN_MASK                                  0x00002000
#define    A9CPU_DSCR_CP14_USER_DIS_SHIFT                                 12
#define    A9CPU_DSCR_CP14_USER_DIS_MASK                                  0x00001000
#define    A9CPU_DSCR_INTRPT_DIS_SHIFT                                    11
#define    A9CPU_DSCR_INTRPT_DIS_MASK                                     0x00000800
#define    A9CPU_DSCR_DBGACK_SHIFT                                        10
#define    A9CPU_DSCR_DBGACK_MASK                                         0x00000400
#define    A9CPU_DSCR_STICKY_UNDEF_SHIFT                                  8
#define    A9CPU_DSCR_STICKY_UNDEF_MASK                                   0x00000100
#define    A9CPU_DSCR_STICKY_IMP_SHIFT                                    7
#define    A9CPU_DSCR_STICKY_IMP_MASK                                     0x00000080
#define    A9CPU_DSCR_STICKY_PRE_SHIFT                                    6
#define    A9CPU_DSCR_STICKY_PRE_MASK                                     0x00000040
#define    A9CPU_DSCR_ENTRY_SHIFT                                         2
#define    A9CPU_DSCR_ENTRY_MASK                                          0x0000003C
#define    A9CPU_DSCR_CORE_RESTART_SHIFT                                  1
#define    A9CPU_DSCR_CORE_RESTART_MASK                                   0x00000002
#define    A9CPU_DSCR_CORE_HALT_SHIFT                                     0
#define    A9CPU_DSCR_CORE_HALT_MASK                                      0x00000001

#define A9CPU_DTRTX_OFFSET                                                0x0000008C
#define A9CPU_DTRTX_TYPE                                                  UInt32
#define A9CPU_DTRTX_RESERVED_MASK                                         0x00000000
#define    A9CPU_DTRTX_DATAC_SHIFT                                        0
#define    A9CPU_DTRTX_DATAC_MASK                                         0xFFFFFFFF

#define A9CPU_DRCR_OFFSET                                                 0x00000090
#define A9CPU_DRCR_TYPE                                                   UInt32
#define A9CPU_DRCR_RESERVED_MASK                                          0xFFFFFFF0
#define    A9CPU_DRCR_CLR_STK_PIPE_SHIFT                                  3
#define    A9CPU_DRCR_CLR_STK_PIPE_MASK                                   0x00000008
#define    A9CPU_DRCR_CLR_STK_EXC_SHIFT                                   2
#define    A9CPU_DRCR_CLR_STK_EXC_MASK                                    0x00000004
#define    A9CPU_DRCR_REQ_REST_SHIFT                                      1
#define    A9CPU_DRCR_REQ_REST_MASK                                       0x00000002
#define    A9CPU_DRCR_REQ_HALT_SHIFT                                      0
#define    A9CPU_DRCR_REQ_HALT_MASK                                       0x00000001

#define A9CPU_BVR0_OFFSET                                                 0x00000100
#define A9CPU_BVR0_TYPE                                                   UInt32
#define A9CPU_BVR0_RESERVED_MASK                                          0x00000000
#define    A9CPU_BVR0_BKPT_VALUEA_SHIFT                                   0
#define    A9CPU_BVR0_BKPT_VALUEA_MASK                                    0xFFFFFFFF

#define A9CPU_BVR1_OFFSET                                                 0x00000104
#define A9CPU_BVR1_TYPE                                                   UInt32
#define A9CPU_BVR1_RESERVED_MASK                                          0x00000000
#define    A9CPU_BVR1_BKPT_VALUEB_SHIFT                                   0
#define    A9CPU_BVR1_BKPT_VALUEB_MASK                                    0xFFFFFFFF

#define A9CPU_BVR2_OFFSET                                                 0x00000108
#define A9CPU_BVR2_TYPE                                                   UInt32
#define A9CPU_BVR2_RESERVED_MASK                                          0x00000000
#define    A9CPU_BVR2_BKPT_VALUEC_SHIFT                                   0
#define    A9CPU_BVR2_BKPT_VALUEC_MASK                                    0xFFFFFFFF

#define A9CPU_BVR3_OFFSET                                                 0x0000010C
#define A9CPU_BVR3_TYPE                                                   UInt32
#define A9CPU_BVR3_RESERVED_MASK                                          0x00000000
#define    A9CPU_BVR3_BKPT_VALUED_SHIFT                                   0
#define    A9CPU_BVR3_BKPT_VALUED_MASK                                    0xFFFFFFFF

#define A9CPU_BVR4_OFFSET                                                 0x00000110
#define A9CPU_BVR4_TYPE                                                   UInt32
#define A9CPU_BVR4_RESERVED_MASK                                          0x00000000
#define    A9CPU_BVR4_BKPT_VALUEE_SHIFT                                   0
#define    A9CPU_BVR4_BKPT_VALUEE_MASK                                    0xFFFFFFFF

#define A9CPU_BVR5_OFFSET                                                 0x00000114
#define A9CPU_BVR5_TYPE                                                   UInt32
#define A9CPU_BVR5_RESERVED_MASK                                          0x00000000
#define    A9CPU_BVR5_BKPT_VALUEF_SHIFT                                   0
#define    A9CPU_BVR5_BKPT_VALUEF_MASK                                    0xFFFFFFFF

#define A9CPU_BCR0_OFFSET                                                 0x00000140
#define A9CPU_BCR0_TYPE                                                   UInt32
#define A9CPU_BCR0_RESERVED_MASK                                          0xE0803E18
#define    A9CPU_BCR0_BKPT_ADDR_MASKA_SHIFT                               24
#define    A9CPU_BCR0_BKPT_ADDR_MASKA_MASK                                0x1F000000
#define    A9CPU_BCR0_MA_SHIFT                                            20
#define    A9CPU_BCR0_MA_MASK                                             0x00700000
#define    A9CPU_BCR0_LINKED_BRPA_SHIFT                                   16
#define    A9CPU_BCR0_LINKED_BRPA_MASK                                    0x000F0000
#define    A9CPU_BCR0_SECURE_CTRLA_SHIFT                                  14
#define    A9CPU_BCR0_SECURE_CTRLA_MASK                                   0x0000C000
#define    A9CPU_BCR0_BYTE_ADDR_SEL_SHIFT                                 5
#define    A9CPU_BCR0_BYTE_ADDR_SEL_MASK                                  0x000001E0
#define    A9CPU_BCR0_SA_SHIFT                                            1
#define    A9CPU_BCR0_SA_MASK                                             0x00000006
#define    A9CPU_BCR0_BA_SHIFT                                            0
#define    A9CPU_BCR0_BA_MASK                                             0x00000001

#define A9CPU_BCR1_OFFSET                                                 0x00000144
#define A9CPU_BCR1_TYPE                                                   UInt32
#define A9CPU_BCR1_RESERVED_MASK                                          0xE0803E18
#define    A9CPU_BCR1_BKPT_ADDR_MASKB_SHIFT                               24
#define    A9CPU_BCR1_BKPT_ADDR_MASKB_MASK                                0x1F000000
#define    A9CPU_BCR1_MB_SHIFT                                            20
#define    A9CPU_BCR1_MB_MASK                                             0x00700000
#define    A9CPU_BCR1_LINKED_BRPB_SHIFT                                   16
#define    A9CPU_BCR1_LINKED_BRPB_MASK                                    0x000F0000
#define    A9CPU_BCR1_SECURE_CTRLB_SHIFT                                  14
#define    A9CPU_BCR1_SECURE_CTRLB_MASK                                   0x0000C000
#define    A9CPU_BCR1_BYTE_ADDR_SELB_SHIFT                                5
#define    A9CPU_BCR1_BYTE_ADDR_SELB_MASK                                 0x000001E0
#define    A9CPU_BCR1_SB_SHIFT                                            1
#define    A9CPU_BCR1_SB_MASK                                             0x00000006
#define    A9CPU_BCR1_BB_SHIFT                                            0
#define    A9CPU_BCR1_BB_MASK                                             0x00000001

#define A9CPU_BCR2_OFFSET                                                 0x00000148
#define A9CPU_BCR2_TYPE                                                   UInt32
#define A9CPU_BCR2_RESERVED_MASK                                          0xE0803E18
#define    A9CPU_BCR2_BKPT_ADDR_MASKC_SHIFT                               24
#define    A9CPU_BCR2_BKPT_ADDR_MASKC_MASK                                0x1F000000
#define    A9CPU_BCR2_MC_SHIFT                                            20
#define    A9CPU_BCR2_MC_MASK                                             0x00700000
#define    A9CPU_BCR2_LINKED_BRPC_SHIFT                                   16
#define    A9CPU_BCR2_LINKED_BRPC_MASK                                    0x000F0000
#define    A9CPU_BCR2_SECURE_CTRLC_SHIFT                                  14
#define    A9CPU_BCR2_SECURE_CTRLC_MASK                                   0x0000C000
#define    A9CPU_BCR2_BYTE_ADDR_SELC_SHIFT                                5
#define    A9CPU_BCR2_BYTE_ADDR_SELC_MASK                                 0x000001E0
#define    A9CPU_BCR2_SC_SHIFT                                            1
#define    A9CPU_BCR2_SC_MASK                                             0x00000006
#define    A9CPU_BCR2_BC_SHIFT                                            0
#define    A9CPU_BCR2_BC_MASK                                             0x00000001

#define A9CPU_BCR3_OFFSET                                                 0x0000014C
#define A9CPU_BCR3_TYPE                                                   UInt32
#define A9CPU_BCR3_RESERVED_MASK                                          0xE0803E18
#define    A9CPU_BCR3_BKPT_ADDR_MASKD_SHIFT                               24
#define    A9CPU_BCR3_BKPT_ADDR_MASKD_MASK                                0x1F000000
#define    A9CPU_BCR3_MD_SHIFT                                            20
#define    A9CPU_BCR3_MD_MASK                                             0x00700000
#define    A9CPU_BCR3_LINKED_BRPD_SHIFT                                   16
#define    A9CPU_BCR3_LINKED_BRPD_MASK                                    0x000F0000
#define    A9CPU_BCR3_SECURE_CTRLD_SHIFT                                  14
#define    A9CPU_BCR3_SECURE_CTRLD_MASK                                   0x0000C000
#define    A9CPU_BCR3_BYTE_ADDR_SELD_SHIFT                                5
#define    A9CPU_BCR3_BYTE_ADDR_SELD_MASK                                 0x000001E0
#define    A9CPU_BCR3_SD_SHIFT                                            1
#define    A9CPU_BCR3_SD_MASK                                             0x00000006
#define    A9CPU_BCR3_BD_SHIFT                                            0
#define    A9CPU_BCR3_BD_MASK                                             0x00000001

#define A9CPU_BCR4_OFFSET                                                 0x00000150
#define A9CPU_BCR4_TYPE                                                   UInt32
#define A9CPU_BCR4_RESERVED_MASK                                          0xE0803E18
#define    A9CPU_BCR4_BKPT_ADDR_MASKE_SHIFT                               24
#define    A9CPU_BCR4_BKPT_ADDR_MASKE_MASK                                0x1F000000
#define    A9CPU_BCR4_ME_SHIFT                                            20
#define    A9CPU_BCR4_ME_MASK                                             0x00700000
#define    A9CPU_BCR4_LINKED_BRPE_SHIFT                                   16
#define    A9CPU_BCR4_LINKED_BRPE_MASK                                    0x000F0000
#define    A9CPU_BCR4_SECURE_CTRLE_SHIFT                                  14
#define    A9CPU_BCR4_SECURE_CTRLE_MASK                                   0x0000C000
#define    A9CPU_BCR4_BYTE_ADDR_SELE_SHIFT                                5
#define    A9CPU_BCR4_BYTE_ADDR_SELE_MASK                                 0x000001E0
#define    A9CPU_BCR4_SE_SHIFT                                            1
#define    A9CPU_BCR4_SE_MASK                                             0x00000006
#define    A9CPU_BCR4_BE_SHIFT                                            0
#define    A9CPU_BCR4_BE_MASK                                             0x00000001

#define A9CPU_BCR5_OFFSET                                                 0x00000154
#define A9CPU_BCR5_TYPE                                                   UInt32
#define A9CPU_BCR5_RESERVED_MASK                                          0xE0803E18
#define    A9CPU_BCR5_BKPT_ADDR_MASKF_SHIFT                               24
#define    A9CPU_BCR5_BKPT_ADDR_MASKF_MASK                                0x1F000000
#define    A9CPU_BCR5_MF_SHIFT                                            20
#define    A9CPU_BCR5_MF_MASK                                             0x00700000
#define    A9CPU_BCR5_LINKED_BRPF_SHIFT                                   16
#define    A9CPU_BCR5_LINKED_BRPF_MASK                                    0x000F0000
#define    A9CPU_BCR5_SECURE_CTRLF_SHIFT                                  14
#define    A9CPU_BCR5_SECURE_CTRLF_MASK                                   0x0000C000
#define    A9CPU_BCR5_BYTE_ADDR_SELF_SHIFT                                5
#define    A9CPU_BCR5_BYTE_ADDR_SELF_MASK                                 0x000001E0
#define    A9CPU_BCR5_SF_SHIFT                                            1
#define    A9CPU_BCR5_SF_MASK                                             0x00000006
#define    A9CPU_BCR5_BF_SHIFT                                            0
#define    A9CPU_BCR5_BF_MASK                                             0x00000001

#define A9CPU_WVR0_OFFSET                                                 0x00000180
#define A9CPU_WVR0_TYPE                                                   UInt32
#define A9CPU_WVR0_RESERVED_MASK                                          0x00000003
#define    A9CPU_WVR0_ADDRA_SHIFT                                         2
#define    A9CPU_WVR0_ADDRA_MASK                                          0xFFFFFFFC

#define A9CPU_WVR1_OFFSET                                                 0x00000184
#define A9CPU_WVR1_TYPE                                                   UInt32
#define A9CPU_WVR1_RESERVED_MASK                                          0x00000003
#define    A9CPU_WVR1_ADDRB_SHIFT                                         2
#define    A9CPU_WVR1_ADDRB_MASK                                          0xFFFFFFFC

#define A9CPU_WVR2_OFFSET                                                 0x00000188
#define A9CPU_WVR2_TYPE                                                   UInt32
#define A9CPU_WVR2_RESERVED_MASK                                          0x00000003
#define    A9CPU_WVR2_ADDRC_SHIFT                                         2
#define    A9CPU_WVR2_ADDRC_MASK                                          0xFFFFFFFC

#define A9CPU_WVR3_OFFSET                                                 0x0000018C
#define A9CPU_WVR3_TYPE                                                   UInt32
#define A9CPU_WVR3_RESERVED_MASK                                          0x00000003
#define    A9CPU_WVR3_ADDRD_SHIFT                                         2
#define    A9CPU_WVR3_ADDRD_MASK                                          0xFFFFFFFC

#define A9CPU_WVR4_OFFSET                                                 0x00000190
#define A9CPU_WVR4_TYPE                                                   UInt32
#define A9CPU_WVR4_RESERVED_MASK                                          0x00000003
#define    A9CPU_WVR4_ADDRE_SHIFT                                         2
#define    A9CPU_WVR4_ADDRE_MASK                                          0xFFFFFFFC

#define A9CPU_WVR5_OFFSET                                                 0x00000194
#define A9CPU_WVR5_TYPE                                                   UInt32
#define A9CPU_WVR5_RESERVED_MASK                                          0x00000003
#define    A9CPU_WVR5_ADDRF_SHIFT                                         2
#define    A9CPU_WVR5_ADDRF_MASK                                          0xFFFFFFFC

#define A9CPU_WCR0_OFFSET                                                 0x000001C0
#define A9CPU_WCR0_TYPE                                                   UInt32
#define A9CPU_WCR0_RESERVED_MASK                                          0xE0E02000
#define    A9CPU_WCR0_WPT_ADDR_MASKG_SHIFT                                24
#define    A9CPU_WCR0_WPT_ADDR_MASKG_MASK                                 0x1F000000
#define    A9CPU_WCR0_EG_SHIFT                                            20
#define    A9CPU_WCR0_EG_MASK                                             0x00100000
#define    A9CPU_WCR0_LINKED_BRPG_SHIFT                                   16
#define    A9CPU_WCR0_LINKED_BRPG_MASK                                    0x000F0000
#define    A9CPU_WCR0_SECURE_CTRLG_SHIFT                                  14
#define    A9CPU_WCR0_SECURE_CTRLG_MASK                                   0x0000C000
#define    A9CPU_WCR0_BYTE_ADDR_SELG_SHIFT                                5
#define    A9CPU_WCR0_BYTE_ADDR_SELG_MASK                                 0x00001FE0
#define    A9CPU_WCR0_L_SG_SHIFT                                          3
#define    A9CPU_WCR0_L_SG_MASK                                           0x00000018
#define    A9CPU_WCR0_SPG_SHIFT                                           1
#define    A9CPU_WCR0_SPG_MASK                                            0x00000006
#define    A9CPU_WCR0_WG_SHIFT                                            0
#define    A9CPU_WCR0_WG_MASK                                             0x00000001

#define A9CPU_WCR1_OFFSET                                                 0x000001C4
#define A9CPU_WCR1_TYPE                                                   UInt32
#define A9CPU_WCR1_RESERVED_MASK                                          0xE0E02000
#define    A9CPU_WCR1_WPT_ADDR_MASKH_SHIFT                                24
#define    A9CPU_WCR1_WPT_ADDR_MASKH_MASK                                 0x1F000000
#define    A9CPU_WCR1_EH_SHIFT                                            20
#define    A9CPU_WCR1_EH_MASK                                             0x00100000
#define    A9CPU_WCR1_LINKED_BRPH_SHIFT                                   16
#define    A9CPU_WCR1_LINKED_BRPH_MASK                                    0x000F0000
#define    A9CPU_WCR1_SECURE_CTRLH_SHIFT                                  14
#define    A9CPU_WCR1_SECURE_CTRLH_MASK                                   0x0000C000
#define    A9CPU_WCR1_BYTE_ADDR_SELH_SHIFT                                5
#define    A9CPU_WCR1_BYTE_ADDR_SELH_MASK                                 0x00001FE0
#define    A9CPU_WCR1_L_SH_SHIFT                                          3
#define    A9CPU_WCR1_L_SH_MASK                                           0x00000018
#define    A9CPU_WCR1_SPH_SHIFT                                           1
#define    A9CPU_WCR1_SPH_MASK                                            0x00000006
#define    A9CPU_WCR1_WH_SHIFT                                            0
#define    A9CPU_WCR1_WH_MASK                                             0x00000001

#define A9CPU_WCR2_OFFSET                                                 0x000001C8
#define A9CPU_WCR2_TYPE                                                   UInt32
#define A9CPU_WCR2_RESERVED_MASK                                          0xE0E02000
#define    A9CPU_WCR2_WPT_ADDR_MASKG1_SHIFT                               24
#define    A9CPU_WCR2_WPT_ADDR_MASKG1_MASK                                0x1F000000
#define    A9CPU_WCR2_EG1_SHIFT                                           20
#define    A9CPU_WCR2_EG1_MASK                                            0x00100000
#define    A9CPU_WCR2_LINKED_BRPG1_SHIFT                                  16
#define    A9CPU_WCR2_LINKED_BRPG1_MASK                                   0x000F0000
#define    A9CPU_WCR2_SECURE_CTRLG1_SHIFT                                 14
#define    A9CPU_WCR2_SECURE_CTRLG1_MASK                                  0x0000C000
#define    A9CPU_WCR2_BYTE_ADDR_SELG1_SHIFT                               5
#define    A9CPU_WCR2_BYTE_ADDR_SELG1_MASK                                0x00001FE0
#define    A9CPU_WCR2_L_SG1_SHIFT                                         3
#define    A9CPU_WCR2_L_SG1_MASK                                          0x00000018
#define    A9CPU_WCR2_SPG1_SHIFT                                          1
#define    A9CPU_WCR2_SPG1_MASK                                           0x00000006
#define    A9CPU_WCR2_WG1_SHIFT                                           0
#define    A9CPU_WCR2_WG1_MASK                                            0x00000001

#define A9CPU_WCR3_OFFSET                                                 0x000001CC
#define A9CPU_WCR3_TYPE                                                   UInt32
#define A9CPU_WCR3_RESERVED_MASK                                          0xE0E02000
#define    A9CPU_WCR3_WPT_ADDR_MASKH1_SHIFT                               24
#define    A9CPU_WCR3_WPT_ADDR_MASKH1_MASK                                0x1F000000
#define    A9CPU_WCR3_EH1_SHIFT                                           20
#define    A9CPU_WCR3_EH1_MASK                                            0x00100000
#define    A9CPU_WCR3_LINKED_BRPH1_SHIFT                                  16
#define    A9CPU_WCR3_LINKED_BRPH1_MASK                                   0x000F0000
#define    A9CPU_WCR3_SECURE_CTRLH1_SHIFT                                 14
#define    A9CPU_WCR3_SECURE_CTRLH1_MASK                                  0x0000C000
#define    A9CPU_WCR3_BYTE_ADDR_SELH1_SHIFT                               5
#define    A9CPU_WCR3_BYTE_ADDR_SELH1_MASK                                0x00001FE0
#define    A9CPU_WCR3_L_SH1_SHIFT                                         3
#define    A9CPU_WCR3_L_SH1_MASK                                          0x00000018
#define    A9CPU_WCR3_SPH1_SHIFT                                          1
#define    A9CPU_WCR3_SPH1_MASK                                           0x00000006
#define    A9CPU_WCR3_WH1_SHIFT                                           0
#define    A9CPU_WCR3_WH1_MASK                                            0x00000001

#define A9CPU_WCR4_OFFSET                                                 0x000001D0
#define A9CPU_WCR4_TYPE                                                   UInt32
#define A9CPU_WCR4_RESERVED_MASK                                          0xE0E02000
#define    A9CPU_WCR4_WPT_ADDR_MASKG2_SHIFT                               24
#define    A9CPU_WCR4_WPT_ADDR_MASKG2_MASK                                0x1F000000
#define    A9CPU_WCR4_EG2_SHIFT                                           20
#define    A9CPU_WCR4_EG2_MASK                                            0x00100000
#define    A9CPU_WCR4_LINKED_BRPG2_SHIFT                                  16
#define    A9CPU_WCR4_LINKED_BRPG2_MASK                                   0x000F0000
#define    A9CPU_WCR4_SECURE_CTRLG2_SHIFT                                 14
#define    A9CPU_WCR4_SECURE_CTRLG2_MASK                                  0x0000C000
#define    A9CPU_WCR4_BYTE_ADDR_SELG2_SHIFT                               5
#define    A9CPU_WCR4_BYTE_ADDR_SELG2_MASK                                0x00001FE0
#define    A9CPU_WCR4_L_SG2_SHIFT                                         3
#define    A9CPU_WCR4_L_SG2_MASK                                          0x00000018
#define    A9CPU_WCR4_SPG2_SHIFT                                          1
#define    A9CPU_WCR4_SPG2_MASK                                           0x00000006
#define    A9CPU_WCR4_WG2_SHIFT                                           0
#define    A9CPU_WCR4_WG2_MASK                                            0x00000001

#define A9CPU_WCR5_OFFSET                                                 0x000001D4
#define A9CPU_WCR5_TYPE                                                   UInt32
#define A9CPU_WCR5_RESERVED_MASK                                          0xE0E02000
#define    A9CPU_WCR5_WPT_ADDR_MASKH2_SHIFT                               24
#define    A9CPU_WCR5_WPT_ADDR_MASKH2_MASK                                0x1F000000
#define    A9CPU_WCR5_EH2_SHIFT                                           20
#define    A9CPU_WCR5_EH2_MASK                                            0x00100000
#define    A9CPU_WCR5_LINKED_BRPH2_SHIFT                                  16
#define    A9CPU_WCR5_LINKED_BRPH2_MASK                                   0x000F0000
#define    A9CPU_WCR5_SECURE_CTRLH2_SHIFT                                 14
#define    A9CPU_WCR5_SECURE_CTRLH2_MASK                                  0x0000C000
#define    A9CPU_WCR5_BYTE_ADDR_SELH2_SHIFT                               5
#define    A9CPU_WCR5_BYTE_ADDR_SELH2_MASK                                0x00001FE0
#define    A9CPU_WCR5_L_SH2_SHIFT                                         3
#define    A9CPU_WCR5_L_SH2_MASK                                          0x00000018
#define    A9CPU_WCR5_SPH2_SHIFT                                          1
#define    A9CPU_WCR5_SPH2_MASK                                           0x00000006
#define    A9CPU_WCR5_WH2_SHIFT                                           0
#define    A9CPU_WCR5_WH2_MASK                                            0x00000001

#define A9CPU_PRCR_OFFSET                                                 0x00000310
#define A9CPU_PRCR_TYPE                                                   UInt32
#define A9CPU_PRCR_RESERVED_MASK                                          0xFFFFFFFE
#define    A9CPU_PRCR_NO_PWRDWN_SHIFT                                     0
#define    A9CPU_PRCR_NO_PWRDWN_MASK                                      0x00000001

#define A9CPU_PRSR_OFFSET                                                 0x00000314
#define A9CPU_PRSR_TYPE                                                   UInt32
#define A9CPU_PRSR_RESERVED_MASK                                          0xFFFFFFF8
#define    A9CPU_PRSR_RESET_SHIFT                                         2
#define    A9CPU_PRSR_RESET_MASK                                          0x00000004
#define    A9CPU_PRSR_STICKY_PWRDWN_SHIFT                                 1
#define    A9CPU_PRSR_STICKY_PWRDWN_MASK                                  0x00000002
#define    A9CPU_PRSR_PWRDWN_SHIFT                                        0
#define    A9CPU_PRSR_PWRDWN_MASK                                         0x00000001

#define A9CPU_ICTRL_OFFSET                                                0x00000F00
#define A9CPU_ICTRL_TYPE                                                  UInt32
#define A9CPU_ICTRL_RESERVED_MASK                                         0xFFFFFFFE
#define    A9CPU_ICTRL_INTEG_EN_SHIFT                                     0
#define    A9CPU_ICTRL_INTEG_EN_MASK                                      0x00000001

#define A9CPU_CLAIMSET_OFFSET                                             0x00000FA0
#define A9CPU_CLAIMSET_TYPE                                               UInt32
#define A9CPU_CLAIMSET_RESERVED_MASK                                      0xFFFFFF00
#define    A9CPU_CLAIMSET_CLAIM_TAGSA_SHIFT                               0
#define    A9CPU_CLAIMSET_CLAIM_TAGSA_MASK                                0x000000FF

#define A9CPU_CLAIMCLR_OFFSET                                             0x00000FA4
#define A9CPU_CLAIMCLR_TYPE                                               UInt32
#define A9CPU_CLAIMCLR_RESERVED_MASK                                      0xFFFFFF00
#define    A9CPU_CLAIMCLR_CLAIM_TAGSB_SHIFT                               0
#define    A9CPU_CLAIMCLR_CLAIM_TAGSB_MASK                                0x000000FF

#define A9CPU_LOCKACCESS_OFFSET                                           0x00000FB0
#define A9CPU_LOCKACCESS_TYPE                                             UInt32
#define A9CPU_LOCKACCESS_RESERVED_MASK                                    0x00000000
#define    A9CPU_LOCKACCESS_LOCK_ACCESS_SHIFT                             0
#define    A9CPU_LOCKACCESS_LOCK_ACCESS_MASK                              0xFFFFFFFF

#define A9CPU_LOCKSTATUS_OFFSET                                           0x00000FB4
#define A9CPU_LOCKSTATUS_TYPE                                             UInt32
#define A9CPU_LOCKSTATUS_RESERVED_MASK                                    0xFFFFFFF8
#define    A9CPU_LOCKSTATUS_ACCESS_32BIT_SHIFT                            2
#define    A9CPU_LOCKSTATUS_ACCESS_32BIT_MASK                             0x00000004
#define    A9CPU_LOCKSTATUS_LOCKED_SHIFT                                  1
#define    A9CPU_LOCKSTATUS_LOCKED_MASK                                   0x00000002
#define    A9CPU_LOCKSTATUS_LOCK_IMPLEMENTED_SHIFT                        0
#define    A9CPU_LOCKSTATUS_LOCK_IMPLEMENTED_MASK                         0x00000001

#define A9CPU_AUTHSTATUS_OFFSET                                           0x00000FB8
#define A9CPU_AUTHSTATUS_TYPE                                             UInt32
#define A9CPU_AUTHSTATUS_RESERVED_MASK                                    0xFFFFFF00
#define    A9CPU_AUTHSTATUS_SEC_NON_DBG_SHIFT                             6
#define    A9CPU_AUTHSTATUS_SEC_NON_DBG_MASK                              0x000000C0
#define    A9CPU_AUTHSTATUS_SEC_INV_DBG_SHIFT                             4
#define    A9CPU_AUTHSTATUS_SEC_INV_DBG_MASK                              0x00000030
#define    A9CPU_AUTHSTATUS_OPEN_NON_DBG_SHIFT                            2
#define    A9CPU_AUTHSTATUS_OPEN_NON_DBG_MASK                             0x0000000C
#define    A9CPU_AUTHSTATUS_OPEN_INV_DBG_SHIFT                            0
#define    A9CPU_AUTHSTATUS_OPEN_INV_DBG_MASK                             0x00000003

#define A9CPU_DEVTYPE_OFFSET                                              0x00000FCC
#define A9CPU_DEVTYPE_TYPE                                                UInt32
#define A9CPU_DEVTYPE_RESERVED_MASK                                       0xFFFFFF00
#define    A9CPU_DEVTYPE_SUB_TYPE_SHIFT                                   4
#define    A9CPU_DEVTYPE_SUB_TYPE_MASK                                    0x000000F0
#define    A9CPU_DEVTYPE_MAIN_CLASS_SHIFT                                 0
#define    A9CPU_DEVTYPE_MAIN_CLASS_MASK                                  0x0000000F

#define A9CPU_PERID4_OFFSET                                               0x00000FD0
#define A9CPU_PERID4_TYPE                                                 UInt32
#define A9CPU_PERID4_RESERVED_MASK                                        0xFFFFFF00
#define    A9CPU_PERID4_PID4_SHIFT                                        0
#define    A9CPU_PERID4_PID4_MASK                                         0x000000FF

#define A9CPU_PERID0_OFFSET                                               0x00000FE0
#define A9CPU_PERID0_TYPE                                                 UInt32
#define A9CPU_PERID0_RESERVED_MASK                                        0xFFFFFF00
#define    A9CPU_PERID0_PID0_SHIFT                                        0
#define    A9CPU_PERID0_PID0_MASK                                         0x000000FF

#define A9CPU_PERID1_OFFSET                                               0x00000FE4
#define A9CPU_PERID1_TYPE                                                 UInt32
#define A9CPU_PERID1_RESERVED_MASK                                        0xFFFFFF00
#define    A9CPU_PERID1_PID1_SHIFT                                        0
#define    A9CPU_PERID1_PID1_MASK                                         0x000000FF

#define A9CPU_PERID2_OFFSET                                               0x00000FE8
#define A9CPU_PERID2_TYPE                                                 UInt32
#define A9CPU_PERID2_RESERVED_MASK                                        0xFFFFFF00
#define    A9CPU_PERID2_PID2_SHIFT                                        0
#define    A9CPU_PERID2_PID2_MASK                                         0x000000FF

#define A9CPU_PERID3_OFFSET                                               0x00000FEC
#define A9CPU_PERID3_TYPE                                                 UInt32
#define A9CPU_PERID3_RESERVED_MASK                                        0xFFFFFF00
#define    A9CPU_PERID3_PID3_SHIFT                                        0
#define    A9CPU_PERID3_PID3_MASK                                         0x000000FF

#define A9CPU_COMPID0_OFFSET                                              0x00000FF0
#define A9CPU_COMPID0_TYPE                                                UInt32
#define A9CPU_COMPID0_RESERVED_MASK                                       0xFFFFFF00
#define    A9CPU_COMPID0_CID0_SHIFT                                       0
#define    A9CPU_COMPID0_CID0_MASK                                        0x000000FF

#define A9CPU_COMPID1_OFFSET                                              0x00000FF4
#define A9CPU_COMPID1_TYPE                                                UInt32
#define A9CPU_COMPID1_RESERVED_MASK                                       0xFFFFFF00
#define    A9CPU_COMPID1_CID1_SHIFT                                       0
#define    A9CPU_COMPID1_CID1_MASK                                        0x000000FF

#define A9CPU_COMPID2_OFFSET                                              0x00000FF8
#define A9CPU_COMPID2_TYPE                                                UInt32
#define A9CPU_COMPID2_RESERVED_MASK                                       0xFFFFFF00
#define    A9CPU_COMPID2_CID2_SHIFT                                       0
#define    A9CPU_COMPID2_CID2_MASK                                        0x000000FF

#define A9CPU_COMPID3_OFFSET                                              0x00000FFC
#define A9CPU_COMPID3_TYPE                                                UInt32
#define A9CPU_COMPID3_RESERVED_MASK                                       0xFFFFFF00
#define    A9CPU_COMPID3_CID3_SHIFT                                       0
#define    A9CPU_COMPID3_CID3_MASK                                        0x000000FF

#endif /* __BRCM_RDB_A9CPU_H__ */



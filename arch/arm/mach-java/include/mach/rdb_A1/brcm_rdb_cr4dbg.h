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

#ifndef __BRCM_RDB_CR4DBG_H__
#define __BRCM_RDB_CR4DBG_H__

#define CR4DBG_DIDR_OFFSET                                                0x00000000
#define CR4DBG_DIDR_TYPE                                                  UInt32
#define CR4DBG_DIDR_RESERVED_MASK                                         0x0000FF00
#define    CR4DBG_DIDR_WRP_SHIFT                                          28
#define    CR4DBG_DIDR_WRP_MASK                                           0xF0000000
#define    CR4DBG_DIDR_BRP_SHIFT                                          24
#define    CR4DBG_DIDR_BRP_MASK                                           0x0F000000
#define    CR4DBG_DIDR_CONTEXT_SHIFT                                      20
#define    CR4DBG_DIDR_CONTEXT_MASK                                       0x00F00000
#define    CR4DBG_DIDR_DBG_ARCH_VER_SHIFT                                 16
#define    CR4DBG_DIDR_DBG_ARCH_VER_MASK                                  0x000F0000
#define    CR4DBG_DIDR_VARIANT_SHIFT                                      4
#define    CR4DBG_DIDR_VARIANT_MASK                                       0x000000F0
#define    CR4DBG_DIDR_REVISION_SHIFT                                     0
#define    CR4DBG_DIDR_REVISION_MASK                                      0x0000000F

#define CR4DBG_WFAR_OFFSET                                                0x00000018
#define CR4DBG_WFAR_TYPE                                                  UInt32
#define CR4DBG_WFAR_RESERVED_MASK                                         0x00000001
#define    CR4DBG_WFAR_ADDRESS_SHIFT                                      1
#define    CR4DBG_WFAR_ADDRESS_MASK                                       0xFFFFFFFE

#define CR4DBG_VCR_OFFSET                                                 0x0000001C
#define CR4DBG_VCR_TYPE                                                   UInt32
#define CR4DBG_VCR_RESERVED_MASK                                          0xFFFFFF20
#define    CR4DBG_VCR_FIQ_SHIFT                                           7
#define    CR4DBG_VCR_FIQ_MASK                                            0x00000080
#define    CR4DBG_VCR_IRQ_SHIFT                                           6
#define    CR4DBG_VCR_IRQ_MASK                                            0x00000040
#define    CR4DBG_VCR_DATA_ABORT_SHIFT                                    4
#define    CR4DBG_VCR_DATA_ABORT_MASK                                     0x00000010
#define    CR4DBG_VCR_PREFETCH_ABORT_SHIFT                                3
#define    CR4DBG_VCR_PREFETCH_ABORT_MASK                                 0x00000008
#define    CR4DBG_VCR_SVC_SHIFT                                           2
#define    CR4DBG_VCR_SVC_MASK                                            0x00000004
#define    CR4DBG_VCR_UNDEF_SHIFT                                         1
#define    CR4DBG_VCR_UNDEF_MASK                                          0x00000002
#define    CR4DBG_VCR_RESET_SHIFT                                         0
#define    CR4DBG_VCR_RESET_MASK                                          0x00000001

#define CR4DBG_DSCCR_OFFSET                                               0x00000028
#define CR4DBG_DSCCR_TYPE                                                 UInt32
#define CR4DBG_DSCCR_RESERVED_MASK                                        0xFFFFFFF8
#define    CR4DBG_DSCCR_NWT_SHIFT                                         2
#define    CR4DBG_DSCCR_NWT_MASK                                          0x00000004
#define    CR4DBG_DSCCR_NIL_SHIFT                                         1
#define    CR4DBG_DSCCR_NIL_MASK                                          0x00000002
#define    CR4DBG_DSCCR_NDL_SHIFT                                         0
#define    CR4DBG_DSCCR_NDL_MASK                                          0x00000001

#define CR4DBG_DTRRX_OFFSET                                               0x00000080
#define CR4DBG_DTRRX_TYPE                                                 UInt32
#define CR4DBG_DTRRX_RESERVED_MASK                                        0x00000000
#define    CR4DBG_DTRRX_DATA_SHIFT                                        0
#define    CR4DBG_DTRRX_DATA_MASK                                         0xFFFFFFFF

#define CR4DBG_ITR_OFFSET                                                 0x00000084
#define CR4DBG_ITR_TYPE                                                   UInt32
#define CR4DBG_ITR_RESERVED_MASK                                          0x00000000
#define    CR4DBG_ITR_INSTR_SHIFT                                         0
#define    CR4DBG_ITR_INSTR_MASK                                          0xFFFFFFFF

#define CR4DBG_DSCR_OFFSET                                                0x00000088
#define CR4DBG_DSCR_TYPE                                                  UInt32
#define CR4DBG_DSCR_RESERVED_MASK                                         0x9CC70200
#define    CR4DBG_DSCR_DTRRXFULL_SHIFT                                    30
#define    CR4DBG_DSCR_DTRRXFULL_MASK                                     0x40000000
#define    CR4DBG_DSCR_DTRTXFULL_SHIFT                                    29
#define    CR4DBG_DSCR_DTRTXFULL_MASK                                     0x20000000
#define    CR4DBG_DSCR_PIPEADV_SHIFT                                      25
#define    CR4DBG_DSCR_PIPEADV_MASK                                       0x02000000
#define    CR4DBG_DSCR_INSTRCOMPL_SHIFT                                   24
#define    CR4DBG_DSCR_INSTRCOMPL_MASK                                    0x01000000
#define    CR4DBG_DSCR_DTRACCESS_SHIFT                                    20
#define    CR4DBG_DSCR_DTRACCESS_MASK                                     0x00300000
#define    CR4DBG_DSCR_DSCIMPABORT_SHIFT                                  19
#define    CR4DBG_DSCR_DSCIMPABORT_MASK                                   0x00080000
#define    CR4DBG_DSCR_MONITORMODE_SHIFT                                  15
#define    CR4DBG_DSCR_MONITORMODE_MASK                                   0x00008000
#define    CR4DBG_DSCR_HALTINGMODE_SHIFT                                  14
#define    CR4DBG_DSCR_HALTINGMODE_MASK                                   0x00004000
#define    CR4DBG_DSCR_ARM_SHIFT                                          13
#define    CR4DBG_DSCR_ARM_MASK                                           0x00002000
#define    CR4DBG_DSCR_COMMS_SHIFT                                        12
#define    CR4DBG_DSCR_COMMS_MASK                                         0x00001000
#define    CR4DBG_DSCR_INTDIS_SHIFT                                       11
#define    CR4DBG_DSCR_INTDIS_MASK                                        0x00000800
#define    CR4DBG_DSCR_DBGACK_SHIFT                                       10
#define    CR4DBG_DSCR_DBGACK_MASK                                        0x00000400
#define    CR4DBG_DSCR_STICKYUNDEF_SHIFT                                  8
#define    CR4DBG_DSCR_STICKYUNDEF_MASK                                   0x00000100
#define    CR4DBG_DSCR_STICKYIABORT_SHIFT                                 7
#define    CR4DBG_DSCR_STICKYIABORT_MASK                                  0x00000080
#define    CR4DBG_DSCR_STICKYPABORT_SHIFT                                 6
#define    CR4DBG_DSCR_STICKYPABORT_MASK                                  0x00000040
#define    CR4DBG_DSCR_MOE_SHIFT                                          2
#define    CR4DBG_DSCR_MOE_MASK                                           0x0000003C
#define    CR4DBG_DSCR_CORERESTARTED_SHIFT                                1
#define    CR4DBG_DSCR_CORERESTARTED_MASK                                 0x00000002
#define    CR4DBG_DSCR_COREHALTED_SHIFT                                   0
#define    CR4DBG_DSCR_COREHALTED_MASK                                    0x00000001

#define CR4DBG_DTRTX_OFFSET                                               0x0000008C
#define CR4DBG_DTRTX_TYPE                                                 UInt32
#define CR4DBG_DTRTX_RESERVED_MASK                                        0x00000000
#define    CR4DBG_DTRTX_DATA_SHIFT                                        0
#define    CR4DBG_DTRTX_DATA_MASK                                         0xFFFFFFFF

#define CR4DBG_DRCR_OFFSET                                                0x00000090
#define CR4DBG_DRCR_TYPE                                                  UInt32
#define CR4DBG_DRCR_RESERVED_MASK                                         0xFFFFFFE0
#define    CR4DBG_DRCR_CANCELMEMREQS_SHIFT                                4
#define    CR4DBG_DRCR_CANCELMEMREQS_MASK                                 0x00000010
#define    CR4DBG_DRCR_CLRSTICKYPADV_SHIFT                                3
#define    CR4DBG_DRCR_CLRSTICKYPADV_MASK                                 0x00000008
#define    CR4DBG_DRCR_CLRSTICKYEXCP_SHIFT                                2
#define    CR4DBG_DRCR_CLRSTICKYEXCP_MASK                                 0x00000004
#define    CR4DBG_DRCR_RESTARTREQ_SHIFT                                   1
#define    CR4DBG_DRCR_RESTARTREQ_MASK                                    0x00000002
#define    CR4DBG_DRCR_HALTREQ_SHIFT                                      0
#define    CR4DBG_DRCR_HALTREQ_MASK                                       0x00000001

#define CR4DBG_BVR0_OFFSET                                                0x00000100
#define CR4DBG_BVR0_TYPE                                                  UInt32
#define CR4DBG_BVR0_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR0_VALUE_SHIFT                                        2
#define    CR4DBG_BVR0_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR1_OFFSET                                                0x00000104
#define CR4DBG_BVR1_TYPE                                                  UInt32
#define CR4DBG_BVR1_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR1_VALUE_SHIFT                                        2
#define    CR4DBG_BVR1_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR2_OFFSET                                                0x00000108
#define CR4DBG_BVR2_TYPE                                                  UInt32
#define CR4DBG_BVR2_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR2_VALUE_SHIFT                                        2
#define    CR4DBG_BVR2_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR3_OFFSET                                                0x0000010C
#define CR4DBG_BVR3_TYPE                                                  UInt32
#define CR4DBG_BVR3_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR3_VALUE_SHIFT                                        2
#define    CR4DBG_BVR3_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR4_OFFSET                                                0x00000110
#define CR4DBG_BVR4_TYPE                                                  UInt32
#define CR4DBG_BVR4_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR4_VALUE_SHIFT                                        2
#define    CR4DBG_BVR4_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR5_OFFSET                                                0x00000114
#define CR4DBG_BVR5_TYPE                                                  UInt32
#define CR4DBG_BVR5_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR5_VALUE_SHIFT                                        2
#define    CR4DBG_BVR5_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR6_OFFSET                                                0x00000118
#define CR4DBG_BVR6_TYPE                                                  UInt32
#define CR4DBG_BVR6_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR6_VALUE_SHIFT                                        2
#define    CR4DBG_BVR6_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BVR7_OFFSET                                                0x0000011C
#define CR4DBG_BVR7_TYPE                                                  UInt32
#define CR4DBG_BVR7_RESERVED_MASK                                         0x00000003
#define    CR4DBG_BVR7_VALUE_SHIFT                                        2
#define    CR4DBG_BVR7_VALUE_MASK                                         0xFFFFFFFC

#define CR4DBG_BCR0_OFFSET                                                0x00000140
#define CR4DBG_BCR0_TYPE                                                  UInt32
#define CR4DBG_BCR0_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR0_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR0_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR0_M_SHIFT                                            20
#define    CR4DBG_BCR0_M_MASK                                             0x00700000
#define    CR4DBG_BCR0_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR0_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR0_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR0_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR0_S_SHIFT                                            1
#define    CR4DBG_BCR0_S_MASK                                             0x00000006
#define    CR4DBG_BCR0_B_SHIFT                                            0
#define    CR4DBG_BCR0_B_MASK                                             0x00000001

#define CR4DBG_BCR1_OFFSET                                                0x00000144
#define CR4DBG_BCR1_TYPE                                                  UInt32
#define CR4DBG_BCR1_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR1_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR1_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR1_M_SHIFT                                            20
#define    CR4DBG_BCR1_M_MASK                                             0x00700000
#define    CR4DBG_BCR1_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR1_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR1_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR1_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR1_S_SHIFT                                            1
#define    CR4DBG_BCR1_S_MASK                                             0x00000006
#define    CR4DBG_BCR1_B_SHIFT                                            0
#define    CR4DBG_BCR1_B_MASK                                             0x00000001

#define CR4DBG_BCR2_OFFSET                                                0x00000148
#define CR4DBG_BCR2_TYPE                                                  UInt32
#define CR4DBG_BCR2_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR2_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR2_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR2_M_SHIFT                                            20
#define    CR4DBG_BCR2_M_MASK                                             0x00700000
#define    CR4DBG_BCR2_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR2_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR2_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR2_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR2_S_SHIFT                                            1
#define    CR4DBG_BCR2_S_MASK                                             0x00000006
#define    CR4DBG_BCR2_B_SHIFT                                            0
#define    CR4DBG_BCR2_B_MASK                                             0x00000001

#define CR4DBG_BCR3_OFFSET                                                0x0000014C
#define CR4DBG_BCR3_TYPE                                                  UInt32
#define CR4DBG_BCR3_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR3_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR3_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR3_M_SHIFT                                            20
#define    CR4DBG_BCR3_M_MASK                                             0x00700000
#define    CR4DBG_BCR3_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR3_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR3_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR3_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR3_S_SHIFT                                            1
#define    CR4DBG_BCR3_S_MASK                                             0x00000006
#define    CR4DBG_BCR3_B_SHIFT                                            0
#define    CR4DBG_BCR3_B_MASK                                             0x00000001

#define CR4DBG_BCR4_OFFSET                                                0x00000150
#define CR4DBG_BCR4_TYPE                                                  UInt32
#define CR4DBG_BCR4_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR4_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR4_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR4_M_SHIFT                                            20
#define    CR4DBG_BCR4_M_MASK                                             0x00700000
#define    CR4DBG_BCR4_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR4_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR4_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR4_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR4_S_SHIFT                                            1
#define    CR4DBG_BCR4_S_MASK                                             0x00000006
#define    CR4DBG_BCR4_B_SHIFT                                            0
#define    CR4DBG_BCR4_B_MASK                                             0x00000001

#define CR4DBG_BCR5_OFFSET                                                0x00000154
#define CR4DBG_BCR5_TYPE                                                  UInt32
#define CR4DBG_BCR5_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR5_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR5_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR5_M_SHIFT                                            20
#define    CR4DBG_BCR5_M_MASK                                             0x00700000
#define    CR4DBG_BCR5_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR5_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR5_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR5_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR5_S_SHIFT                                            1
#define    CR4DBG_BCR5_S_MASK                                             0x00000006
#define    CR4DBG_BCR5_B_SHIFT                                            0
#define    CR4DBG_BCR5_B_MASK                                             0x00000001

#define CR4DBG_BCR6_OFFSET                                                0x00000158
#define CR4DBG_BCR6_TYPE                                                  UInt32
#define CR4DBG_BCR6_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR6_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR6_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR6_M_SHIFT                                            20
#define    CR4DBG_BCR6_M_MASK                                             0x00700000
#define    CR4DBG_BCR6_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR6_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR6_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR6_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR6_S_SHIFT                                            1
#define    CR4DBG_BCR6_S_MASK                                             0x00000006
#define    CR4DBG_BCR6_B_SHIFT                                            0
#define    CR4DBG_BCR6_B_MASK                                             0x00000001

#define CR4DBG_BCR7_OFFSET                                                0x0000015C
#define CR4DBG_BCR7_TYPE                                                  UInt32
#define CR4DBG_BCR7_RESERVED_MASK                                         0xE080FE18
#define    CR4DBG_BCR7_BPADDRMASK_SHIFT                                   24
#define    CR4DBG_BCR7_BPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_BCR7_M_SHIFT                                            20
#define    CR4DBG_BCR7_M_MASK                                             0x00700000
#define    CR4DBG_BCR7_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_BCR7_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_BCR7_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_BCR7_BYTEADDRSEL_MASK                                   0x000001E0
#define    CR4DBG_BCR7_S_SHIFT                                            1
#define    CR4DBG_BCR7_S_MASK                                             0x00000006
#define    CR4DBG_BCR7_B_SHIFT                                            0
#define    CR4DBG_BCR7_B_MASK                                             0x00000001

#define CR4DBG_WVR0_OFFSET                                                0x00000180
#define CR4DBG_WVR0_TYPE                                                  UInt32
#define CR4DBG_WVR0_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR0_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR0_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR1_OFFSET                                                0x00000184
#define CR4DBG_WVR1_TYPE                                                  UInt32
#define CR4DBG_WVR1_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR1_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR1_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR2_OFFSET                                                0x00000188
#define CR4DBG_WVR2_TYPE                                                  UInt32
#define CR4DBG_WVR2_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR2_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR2_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR3_OFFSET                                                0x0000018C
#define CR4DBG_WVR3_TYPE                                                  UInt32
#define CR4DBG_WVR3_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR3_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR3_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR4_OFFSET                                                0x00000190
#define CR4DBG_WVR4_TYPE                                                  UInt32
#define CR4DBG_WVR4_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR4_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR4_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR5_OFFSET                                                0x00000194
#define CR4DBG_WVR5_TYPE                                                  UInt32
#define CR4DBG_WVR5_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR5_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR5_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR6_OFFSET                                                0x00000198
#define CR4DBG_WVR6_TYPE                                                  UInt32
#define CR4DBG_WVR6_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR6_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR6_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WVR7_OFFSET                                                0x0000019C
#define CR4DBG_WVR7_TYPE                                                  UInt32
#define CR4DBG_WVR7_RESERVED_MASK                                         0x00000003
#define    CR4DBG_WVR7_WPADDR_SHIFT                                       2
#define    CR4DBG_WVR7_WPADDR_MASK                                        0xFFFFFFFC

#define CR4DBG_WCR0_OFFSET                                                0x000001C0
#define CR4DBG_WCR0_TYPE                                                  UInt32
#define CR4DBG_WCR0_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR0_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR0_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR0_E_SHIFT                                            20
#define    CR4DBG_WCR0_E_MASK                                             0x00100000
#define    CR4DBG_WCR0_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR0_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR0_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR0_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR0_LS_SHIFT                                           3
#define    CR4DBG_WCR0_LS_MASK                                            0x00000018
#define    CR4DBG_WCR0_S_SHIFT                                            1
#define    CR4DBG_WCR0_S_MASK                                             0x00000006
#define    CR4DBG_WCR0_W_SHIFT                                            0
#define    CR4DBG_WCR0_W_MASK                                             0x00000001

#define CR4DBG_WCR1_OFFSET                                                0x000001C4
#define CR4DBG_WCR1_TYPE                                                  UInt32
#define CR4DBG_WCR1_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR1_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR1_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR1_E_SHIFT                                            20
#define    CR4DBG_WCR1_E_MASK                                             0x00100000
#define    CR4DBG_WCR1_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR1_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR1_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR1_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR1_LS_SHIFT                                           3
#define    CR4DBG_WCR1_LS_MASK                                            0x00000018
#define    CR4DBG_WCR1_S_SHIFT                                            1
#define    CR4DBG_WCR1_S_MASK                                             0x00000006
#define    CR4DBG_WCR1_W_SHIFT                                            0
#define    CR4DBG_WCR1_W_MASK                                             0x00000001

#define CR4DBG_WCR2_OFFSET                                                0x000001C8
#define CR4DBG_WCR2_TYPE                                                  UInt32
#define CR4DBG_WCR2_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR2_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR2_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR2_E_SHIFT                                            20
#define    CR4DBG_WCR2_E_MASK                                             0x00100000
#define    CR4DBG_WCR2_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR2_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR2_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR2_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR2_LS_SHIFT                                           3
#define    CR4DBG_WCR2_LS_MASK                                            0x00000018
#define    CR4DBG_WCR2_S_SHIFT                                            1
#define    CR4DBG_WCR2_S_MASK                                             0x00000006
#define    CR4DBG_WCR2_W_SHIFT                                            0
#define    CR4DBG_WCR2_W_MASK                                             0x00000001

#define CR4DBG_WCR3_OFFSET                                                0x000001CC
#define CR4DBG_WCR3_TYPE                                                  UInt32
#define CR4DBG_WCR3_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR3_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR3_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR3_E_SHIFT                                            20
#define    CR4DBG_WCR3_E_MASK                                             0x00100000
#define    CR4DBG_WCR3_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR3_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR3_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR3_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR3_LS_SHIFT                                           3
#define    CR4DBG_WCR3_LS_MASK                                            0x00000018
#define    CR4DBG_WCR3_S_SHIFT                                            1
#define    CR4DBG_WCR3_S_MASK                                             0x00000006
#define    CR4DBG_WCR3_W_SHIFT                                            0
#define    CR4DBG_WCR3_W_MASK                                             0x00000001

#define CR4DBG_WCR4_OFFSET                                                0x000001D0
#define CR4DBG_WCR4_TYPE                                                  UInt32
#define CR4DBG_WCR4_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR4_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR4_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR4_E_SHIFT                                            20
#define    CR4DBG_WCR4_E_MASK                                             0x00100000
#define    CR4DBG_WCR4_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR4_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR4_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR4_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR4_LS_SHIFT                                           3
#define    CR4DBG_WCR4_LS_MASK                                            0x00000018
#define    CR4DBG_WCR4_S_SHIFT                                            1
#define    CR4DBG_WCR4_S_MASK                                             0x00000006
#define    CR4DBG_WCR4_W_SHIFT                                            0
#define    CR4DBG_WCR4_W_MASK                                             0x00000001

#define CR4DBG_WCR5_OFFSET                                                0x000001D4
#define CR4DBG_WCR5_TYPE                                                  UInt32
#define CR4DBG_WCR5_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR5_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR5_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR5_E_SHIFT                                            20
#define    CR4DBG_WCR5_E_MASK                                             0x00100000
#define    CR4DBG_WCR5_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR5_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR5_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR5_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR5_LS_SHIFT                                           3
#define    CR4DBG_WCR5_LS_MASK                                            0x00000018
#define    CR4DBG_WCR5_S_SHIFT                                            1
#define    CR4DBG_WCR5_S_MASK                                             0x00000006
#define    CR4DBG_WCR5_W_SHIFT                                            0
#define    CR4DBG_WCR5_W_MASK                                             0x00000001

#define CR4DBG_WCR6_OFFSET                                                0x000001D8
#define CR4DBG_WCR6_TYPE                                                  UInt32
#define CR4DBG_WCR6_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR6_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR6_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR6_E_SHIFT                                            20
#define    CR4DBG_WCR6_E_MASK                                             0x00100000
#define    CR4DBG_WCR6_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR6_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR6_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR6_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR6_LS_SHIFT                                           3
#define    CR4DBG_WCR6_LS_MASK                                            0x00000018
#define    CR4DBG_WCR6_S_SHIFT                                            1
#define    CR4DBG_WCR6_S_MASK                                             0x00000006
#define    CR4DBG_WCR6_W_SHIFT                                            0
#define    CR4DBG_WCR6_W_MASK                                             0x00000001

#define CR4DBG_WCR7_OFFSET                                                0x000001DC
#define CR4DBG_WCR7_TYPE                                                  UInt32
#define CR4DBG_WCR7_RESERVED_MASK                                         0xE0E0E000
#define    CR4DBG_WCR7_WPADDRMASK_SHIFT                                   24
#define    CR4DBG_WCR7_WPADDRMASK_MASK                                    0x1F000000
#define    CR4DBG_WCR7_E_SHIFT                                            20
#define    CR4DBG_WCR7_E_MASK                                             0x00100000
#define    CR4DBG_WCR7_LINKEDBRP_SHIFT                                    16
#define    CR4DBG_WCR7_LINKEDBRP_MASK                                     0x000F0000
#define    CR4DBG_WCR7_BYTEADDRSEL_SHIFT                                  5
#define    CR4DBG_WCR7_BYTEADDRSEL_MASK                                   0x00001FE0
#define    CR4DBG_WCR7_LS_SHIFT                                           3
#define    CR4DBG_WCR7_LS_MASK                                            0x00000018
#define    CR4DBG_WCR7_S_SHIFT                                            1
#define    CR4DBG_WCR7_S_MASK                                             0x00000006
#define    CR4DBG_WCR7_W_SHIFT                                            0
#define    CR4DBG_WCR7_W_MASK                                             0x00000001

#define CR4DBG_OSLSR_OFFSET                                               0x00000304
#define CR4DBG_OSLSR_TYPE                                                 UInt32
#define CR4DBG_OSLSR_RESERVED_MASK                                        0xFFFFFFFE
#define    CR4DBG_OSLSR_LOCKIMPLMNTD_SHIFT                                0
#define    CR4DBG_OSLSR_LOCKIMPLMNTD_MASK                                 0x00000001

#define CR4DBG_PRCR_OFFSET                                                0x00000310
#define CR4DBG_PRCR_TYPE                                                  UInt32
#define CR4DBG_PRCR_RESERVED_MASK                                         0xFFFFFFF8
#define    CR4DBG_PRCR_HOLDITLRESET_SHIFT                                 2
#define    CR4DBG_PRCR_HOLDITLRESET_MASK                                  0x00000004
#define    CR4DBG_PRCR_FORCEITLRESET_SHIFT                                1
#define    CR4DBG_PRCR_FORCEITLRESET_MASK                                 0x00000002
#define    CR4DBG_PRCR_NOPOWERDOWN_SHIFT                                  0
#define    CR4DBG_PRCR_NOPOWERDOWN_MASK                                   0x00000001

#define CR4DBG_PRSR_OFFSET                                                0x00000314
#define CR4DBG_PRSR_TYPE                                                  UInt32
#define CR4DBG_PRSR_RESERVED_MASK                                         0xFFFFFFF0
#define    CR4DBG_PRSR_STICKYRSTSTAT_SHIFT                                3
#define    CR4DBG_PRSR_STICKYRSTSTAT_MASK                                 0x00000008
#define    CR4DBG_PRSR_RSTSTAT_SHIFT                                      2
#define    CR4DBG_PRSR_RSTSTAT_MASK                                       0x00000004
#define    CR4DBG_PRSR_STICKYPDSTAT_SHIFT                                 1
#define    CR4DBG_PRSR_STICKYPDSTAT_MASK                                  0x00000002
#define    CR4DBG_PRSR_PDSTAT_SHIFT                                       0
#define    CR4DBG_PRSR_PDSTAT_MASK                                        0x00000001

#define CR4DBG_MIDR_OFFSET                                                0x00000D00
#define CR4DBG_MIDR_TYPE                                                  UInt32
#define CR4DBG_MIDR_RESERVED_MASK                                         0x00000000
#define    CR4DBG_MIDR_IMPLEMENTER_SHIFT                                  24
#define    CR4DBG_MIDR_IMPLEMENTER_MASK                                   0xFF000000
#define    CR4DBG_MIDR_VARIANT_SHIFT                                      20
#define    CR4DBG_MIDR_VARIANT_MASK                                       0x00F00000
#define    CR4DBG_MIDR_ARCHITECTURE_SHIFT                                 16
#define    CR4DBG_MIDR_ARCHITECTURE_MASK                                  0x000F0000
#define    CR4DBG_MIDR_PRIMARYPARTNO_SHIFT                                4
#define    CR4DBG_MIDR_PRIMARYPARTNO_MASK                                 0x0000FFF0
#define    CR4DBG_MIDR_REVISION_SHIFT                                     0
#define    CR4DBG_MIDR_REVISION_MASK                                      0x0000000F

#define CR4DBG_CTR_OFFSET                                                 0x00000D04
#define CR4DBG_CTR_TYPE                                                   UInt32
#define CR4DBG_CTR_RESERVED_MASK                                          0x00000000
#define    CR4DBG_CTR_ALWAYS8_SHIFT                                       28
#define    CR4DBG_CTR_ALWAYS8_MASK                                        0xF0000000
#define    CR4DBG_CTR_CWG_SHIFT                                           24
#define    CR4DBG_CTR_CWG_MASK                                            0x0F000000
#define    CR4DBG_CTR_ERG_SHIFT                                           20
#define    CR4DBG_CTR_ERG_MASK                                            0x00F00000
#define    CR4DBG_CTR_DMINLINE_SHIFT                                      16
#define    CR4DBG_CTR_DMINLINE_MASK                                       0x000F0000
#define    CR4DBG_CTR_ALWAYS3_SHIFT                                       14
#define    CR4DBG_CTR_ALWAYS3_MASK                                        0x0000C000
#define    CR4DBG_CTR_ALWAYS0_SHIFT                                       4
#define    CR4DBG_CTR_ALWAYS0_MASK                                        0x00003FF0
#define    CR4DBG_CTR_IMINLINE_SHIFT                                      0
#define    CR4DBG_CTR_IMINLINE_MASK                                       0x0000000F

#define CR4DBG_TCMTR_OFFSET                                               0x00000D08
#define CR4DBG_TCMTR_TYPE                                                 UInt32
#define CR4DBG_TCMTR_RESERVED_MASK                                        0x1FF8FFF8
#define    CR4DBG_TCMTR_ALWAYS0_SHIFT                                     29
#define    CR4DBG_TCMTR_ALWAYS0_MASK                                      0xE0000000
#define    CR4DBG_TCMTR_BTCM_SHIFT                                        16
#define    CR4DBG_TCMTR_BTCM_MASK                                         0x00070000
#define    CR4DBG_TCMTR_ATCM_SHIFT                                        0
#define    CR4DBG_TCMTR_ATCM_MASK                                         0x00000007

#define CR4DBG_MIDR_ALIAS1_OFFSET                                         0x00000D0C
#define CR4DBG_MIDR_ALIAS1_TYPE                                           UInt32
#define CR4DBG_MIDR_ALIAS1_RESERVED_MASK                                  0x00000000
#define    CR4DBG_MIDR_ALIAS1_IMPLEMENTER_SHIFT                           24
#define    CR4DBG_MIDR_ALIAS1_IMPLEMENTER_MASK                            0xFF000000
#define    CR4DBG_MIDR_ALIAS1_VARIANT_SHIFT                               20
#define    CR4DBG_MIDR_ALIAS1_VARIANT_MASK                                0x00F00000
#define    CR4DBG_MIDR_ALIAS1_ARCHITECTURE_SHIFT                          16
#define    CR4DBG_MIDR_ALIAS1_ARCHITECTURE_MASK                           0x000F0000
#define    CR4DBG_MIDR_ALIAS1_PRIMARYPARTNO_SHIFT                         4
#define    CR4DBG_MIDR_ALIAS1_PRIMARYPARTNO_MASK                          0x0000FFF0
#define    CR4DBG_MIDR_ALIAS1_REVISION_SHIFT                              0
#define    CR4DBG_MIDR_ALIAS1_REVISION_MASK                               0x0000000F

#define CR4DBG_MPUIR_OFFSET                                               0x00000D10
#define CR4DBG_MPUIR_TYPE                                                 UInt32
#define CR4DBG_MPUIR_RESERVED_MASK                                        0xFFFF00FE
#define    CR4DBG_MPUIR_DREGION_SHIFT                                     8
#define    CR4DBG_MPUIR_DREGION_MASK                                      0x0000FF00
#define    CR4DBG_MPUIR_S_SHIFT                                           0
#define    CR4DBG_MPUIR_S_MASK                                            0x00000001

#define CR4DBG_MPIDR_OFFSET                                               0x00000D14
#define CR4DBG_MPIDR_TYPE                                                 UInt32
#define CR4DBG_MPIDR_RESERVED_MASK                                        0xFF000000
#define    CR4DBG_MPIDR_AFINITYLVL2_SHIFT                                 16
#define    CR4DBG_MPIDR_AFINITYLVL2_MASK                                  0x00FF0000
#define    CR4DBG_MPIDR_AFINITYLVL1_SHIFT                                 8
#define    CR4DBG_MPIDR_AFINITYLVL1_MASK                                  0x0000FF00
#define    CR4DBG_MPIDR_AFINITYLVL0_SHIFT                                 0
#define    CR4DBG_MPIDR_AFINITYLVL0_MASK                                  0x000000FF

#define CR4DBG_MIDR_ALIAS2_OFFSET                                         0x00000D18
#define CR4DBG_MIDR_ALIAS2_TYPE                                           UInt32
#define CR4DBG_MIDR_ALIAS2_RESERVED_MASK                                  0x00000000
#define    CR4DBG_MIDR_ALIAS2_IMPLEMENTER_SHIFT                           24
#define    CR4DBG_MIDR_ALIAS2_IMPLEMENTER_MASK                            0xFF000000
#define    CR4DBG_MIDR_ALIAS2_VARIANT_SHIFT                               20
#define    CR4DBG_MIDR_ALIAS2_VARIANT_MASK                                0x00F00000
#define    CR4DBG_MIDR_ALIAS2_ARCHITECTURE_SHIFT                          16
#define    CR4DBG_MIDR_ALIAS2_ARCHITECTURE_MASK                           0x000F0000
#define    CR4DBG_MIDR_ALIAS2_PRIMARYPARTNO_SHIFT                         4
#define    CR4DBG_MIDR_ALIAS2_PRIMARYPARTNO_MASK                          0x0000FFF0
#define    CR4DBG_MIDR_ALIAS2_REVISION_SHIFT                              0
#define    CR4DBG_MIDR_ALIAS2_REVISION_MASK                               0x0000000F

#define CR4DBG_MIDR_ALIAS3_OFFSET                                         0x00000D1C
#define CR4DBG_MIDR_ALIAS3_TYPE                                           UInt32
#define CR4DBG_MIDR_ALIAS3_RESERVED_MASK                                  0x00000000
#define    CR4DBG_MIDR_ALIAS3_IMPLEMENTER_SHIFT                           24
#define    CR4DBG_MIDR_ALIAS3_IMPLEMENTER_MASK                            0xFF000000
#define    CR4DBG_MIDR_ALIAS3_VARIANT_SHIFT                               20
#define    CR4DBG_MIDR_ALIAS3_VARIANT_MASK                                0x00F00000
#define    CR4DBG_MIDR_ALIAS3_ARCHITECTURE_SHIFT                          16
#define    CR4DBG_MIDR_ALIAS3_ARCHITECTURE_MASK                           0x000F0000
#define    CR4DBG_MIDR_ALIAS3_PRIMARYPARTNO_SHIFT                         4
#define    CR4DBG_MIDR_ALIAS3_PRIMARYPARTNO_MASK                          0x0000FFF0
#define    CR4DBG_MIDR_ALIAS3_REVISION_SHIFT                              0
#define    CR4DBG_MIDR_ALIAS3_REVISION_MASK                               0x0000000F

#define CR4DBG_IDPFR0_OFFSET                                              0x00000D20
#define CR4DBG_IDPFR0_TYPE                                                UInt32
#define CR4DBG_IDPFR0_RESERVED_MASK                                       0xFFFF0000
#define    CR4DBG_IDPFR0_STATE3_SHIFT                                     12
#define    CR4DBG_IDPFR0_STATE3_MASK                                      0x0000F000
#define    CR4DBG_IDPFR0_STATE2_SHIFT                                     8
#define    CR4DBG_IDPFR0_STATE2_MASK                                      0x00000F00
#define    CR4DBG_IDPFR0_STATE1_SHIFT                                     4
#define    CR4DBG_IDPFR0_STATE1_MASK                                      0x000000F0
#define    CR4DBG_IDPFR0_STATE0_SHIFT                                     0
#define    CR4DBG_IDPFR0_STATE0_MASK                                      0x0000000F

#define CR4DBG_IDPFR1_OFFSET                                              0x00000D24
#define CR4DBG_IDPFR1_TYPE                                                UInt32
#define CR4DBG_IDPFR1_RESERVED_MASK                                       0xFFFFF000
#define    CR4DBG_IDPFR1_MCUPROGMDL_SHIFT                                 8
#define    CR4DBG_IDPFR1_MCUPROGMDL_MASK                                  0x00000F00
#define    CR4DBG_IDPFR1_SECEXTENSION_SHIFT                               4
#define    CR4DBG_IDPFR1_SECEXTENSION_MASK                                0x000000F0
#define    CR4DBG_IDPFR1_ARMV4PROGMDL_SHIFT                               0
#define    CR4DBG_IDPFR1_ARMV4PROGMDL_MASK                                0x0000000F

#define CR4DBG_IDDFR0_OFFSET                                              0x00000D28
#define CR4DBG_IDDFR0_TYPE                                                UInt32
#define CR4DBG_IDDFR0_RESERVED_MASK                                       0xFF000000
#define    CR4DBG_IDDFR0_MCUDBGMDLMM_SHIFT                                20
#define    CR4DBG_IDDFR0_MCUDBGMDLMM_MASK                                 0x00F00000
#define    CR4DBG_IDDFR0_TRACEDBGMDLMM_SHIFT                              16
#define    CR4DBG_IDDFR0_TRACEDBGMDLMM_MASK                               0x000F0000
#define    CR4DBG_IDDFR0_TRACEDBGMDLCP_SHIFT                              12
#define    CR4DBG_IDDFR0_TRACEDBGMDLCP_MASK                               0x0000F000
#define    CR4DBG_IDDFR0_COREDBGMDLMM_SHIFT                               8
#define    CR4DBG_IDDFR0_COREDBGMDLMM_MASK                                0x00000F00
#define    CR4DBG_IDDFR0_SECDBGMDL_SHIFT                                  4
#define    CR4DBG_IDDFR0_SECDBGMDL_MASK                                   0x000000F0
#define    CR4DBG_IDDFR0_COREDBGMDLCP_SHIFT                               0
#define    CR4DBG_IDDFR0_COREDBGMDLCP_MASK                                0x0000000F

#define CR4DBG_IDAFR0_OFFSET                                              0x00000D2C
#define CR4DBG_IDAFR0_TYPE                                                UInt32
#define CR4DBG_IDAFR0_RESERVED_MASK                                       0x00000000
#define    CR4DBG_IDAFR0_AFR0_SHIFT                                       0
#define    CR4DBG_IDAFR0_AFR0_MASK                                        0xFFFFFFFF

#define CR4DBG_IDMMFR0_OFFSET                                             0x00000D30
#define CR4DBG_IDMMFR0_TYPE                                               UInt32
#define CR4DBG_IDMMFR0_RESERVED_MASK                                      0xF0000000
#define    CR4DBG_IDMMFR0_FCSE_SHIFT                                      24
#define    CR4DBG_IDMMFR0_FCSE_MASK                                       0x0F000000
#define    CR4DBG_IDMMFR0_AUXCTRLREG_SHIFT                                20
#define    CR4DBG_IDMMFR0_AUXCTRLREG_MASK                                 0x00F00000
#define    CR4DBG_IDMMFR0_TCM_SHIFT                                       16
#define    CR4DBG_IDMMFR0_TCM_MASK                                        0x000F0000
#define    CR4DBG_IDMMFR0_OUTERSHARE_SHIFT                                12
#define    CR4DBG_IDMMFR0_OUTERSHARE_MASK                                 0x0000F000
#define    CR4DBG_IDMMFR0_CACHECOHERNCE_SHIFT                             8
#define    CR4DBG_IDMMFR0_CACHECOHERNCE_MASK                              0x00000F00
#define    CR4DBG_IDMMFR0_PMSA_SHIFT                                      4
#define    CR4DBG_IDMMFR0_PMSA_MASK                                       0x000000F0
#define    CR4DBG_IDMMFR0_VMSA_SHIFT                                      0
#define    CR4DBG_IDMMFR0_VMSA_MASK                                       0x0000000F

#define CR4DBG_IDMMFR1_OFFSET                                             0x00000D34
#define CR4DBG_IDMMFR1_TYPE                                               UInt32
#define CR4DBG_IDMMFR1_RESERVED_MASK                                      0x00000000
#define    CR4DBG_IDMMFR1_BRANCHPRED_SHIFT                                28
#define    CR4DBG_IDMMFR1_BRANCHPRED_MASK                                 0xF0000000
#define    CR4DBG_IDMMFR1_L1TESTCLN_SHIFT                                 24
#define    CR4DBG_IDMMFR1_L1TESTCLN_MASK                                  0x0F000000
#define    CR4DBG_IDMMFR1_L1CACHEMU_SHIFT                                 20
#define    CR4DBG_IDMMFR1_L1CACHEMU_MASK                                  0x00F00000
#define    CR4DBG_IDMMFR1_L1CACHEMH_SHIFT                                 16
#define    CR4DBG_IDMMFR1_L1CACHEMH_MASK                                  0x000F0000
#define    CR4DBG_IDMMFR1_L1CACHELMSWU_SHIFT                              12
#define    CR4DBG_IDMMFR1_L1CACHELMSWU_MASK                               0x0000F000
#define    CR4DBG_IDMMFR1_L1CACHELMSWH_SHIFT                              8
#define    CR4DBG_IDMMFR1_L1CACHELMSWH_MASK                               0x00000F00
#define    CR4DBG_IDMMFR1_L1CACHEMMVAU_SHIFT                              4
#define    CR4DBG_IDMMFR1_L1CACHEMMVAU_MASK                               0x000000F0
#define    CR4DBG_IDMMFR1_L1CACHEMMVAH_SHIFT                              0
#define    CR4DBG_IDMMFR1_L1CACHEMMVAH_MASK                               0x0000000F

#define CR4DBG_IDMMFR2_OFFSET                                             0x00000D38
#define CR4DBG_IDMMFR2_TYPE                                               UInt32
#define CR4DBG_IDMMFR2_RESERVED_MASK                                      0x00000000
#define    CR4DBG_IDMMFR2_HWACCFLAG_SHIFT                                 28
#define    CR4DBG_IDMMFR2_HWACCFLAG_MASK                                  0xF0000000
#define    CR4DBG_IDMMFR2_WFI_SHIFT                                       24
#define    CR4DBG_IDMMFR2_WFI_MASK                                        0x0F000000
#define    CR4DBG_IDMMFR2_MEMBARRIER_SHIFT                                20
#define    CR4DBG_IDMMFR2_MEMBARRIER_MASK                                 0x00F00000
#define    CR4DBG_IDMMFR2_TLBMOPU_SHIFT                                   16
#define    CR4DBG_IDMMFR2_TLBMOPU_MASK                                    0x000F0000
#define    CR4DBG_IDMMFR2_TLBMOPH_SHIFT                                   12
#define    CR4DBG_IDMMFR2_TLBMOPH_MASK                                    0x0000F000
#define    CR4DBG_IDMMFR2_L1MROPH_SHIFT                                   8
#define    CR4DBG_IDMMFR2_L1MROPH_MASK                                    0x00000F00
#define    CR4DBG_IDMMFR2_L1BGPREFOPH_SHIFT                               4
#define    CR4DBG_IDMMFR2_L1BGPREFOPH_MASK                                0x000000F0
#define    CR4DBG_IDMMFR2_L1FGPREFOPH_SHIFT                               0
#define    CR4DBG_IDMMFR2_L1FGPREFOPH_MASK                                0x0000000F

#define CR4DBG_IDMMFR3_OFFSET                                             0x00000D3C
#define CR4DBG_IDMMFR3_TYPE                                               UInt32
#define CR4DBG_IDMMFR3_RESERVED_MASK                                      0xFFFFF000
#define    CR4DBG_IDMMFR3_BPMOP_SHIFT                                     8
#define    CR4DBG_IDMMFR3_BPMOP_MASK                                      0x00000F00
#define    CR4DBG_IDMMFR3_HCMOPSW_SHIFT                                   4
#define    CR4DBG_IDMMFR3_HCMOPSW_MASK                                    0x000000F0
#define    CR4DBG_IDMMFR3_HCMOPMVA_SHIFT                                  0
#define    CR4DBG_IDMMFR3_HCMOPMVA_MASK                                   0x0000000F

#define CR4DBG_IDISAR0_OFFSET                                             0x00000D40
#define CR4DBG_IDISAR0_TYPE                                               UInt32
#define CR4DBG_IDISAR0_RESERVED_MASK                                      0xF0000000
#define    CR4DBG_IDISAR0_DIVINSTR_SHIFT                                  24
#define    CR4DBG_IDISAR0_DIVINSTR_MASK                                   0x0F000000
#define    CR4DBG_IDISAR0_DBGINSTR_SHIFT                                  20
#define    CR4DBG_IDISAR0_DBGINSTR_MASK                                   0x00F00000
#define    CR4DBG_IDISAR0_COPINSTR_SHIFT                                  16
#define    CR4DBG_IDISAR0_COPINSTR_MASK                                   0x000F0000
#define    CR4DBG_IDISAR0_CMPANDBRINSTR_SHIFT                             12
#define    CR4DBG_IDISAR0_CMPANDBRINSTR_MASK                              0x0000F000
#define    CR4DBG_IDISAR0_BITFIELDINSTR_SHIFT                             8
#define    CR4DBG_IDISAR0_BITFIELDINSTR_MASK                              0x00000F00
#define    CR4DBG_IDISAR0_BITCOUNTINSTR_SHIFT                             4
#define    CR4DBG_IDISAR0_BITCOUNTINSTR_MASK                              0x000000F0
#define    CR4DBG_IDISAR0_ATOMICINSTR_SHIFT                               0
#define    CR4DBG_IDISAR0_ATOMICINSTR_MASK                                0x0000000F

#define CR4DBG_IDISAR1_OFFSET                                             0x00000D44
#define CR4DBG_IDISAR1_TYPE                                               UInt32
#define CR4DBG_IDISAR1_RESERVED_MASK                                      0x00000000
#define    CR4DBG_IDISAR1_JAZINSTR_SHIFT                                  28
#define    CR4DBG_IDISAR1_JAZINSTR_MASK                                   0xF0000000
#define    CR4DBG_IDISAR1_IWINSTR_SHIFT                                   24
#define    CR4DBG_IDISAR1_IWINSTR_MASK                                    0x0F000000
#define    CR4DBG_IDISAR1_IMMINSTR_SHIFT                                  20
#define    CR4DBG_IDISAR1_IMMINSTR_MASK                                   0x00F00000
#define    CR4DBG_IDISAR1_ITEINSTR_SHIFT                                  16
#define    CR4DBG_IDISAR1_ITEINSTR_MASK                                   0x000F0000
#define    CR4DBG_IDISAR1_EXTENDINSTR_SHIFT                               12
#define    CR4DBG_IDISAR1_EXTENDINSTR_MASK                                0x0000F000
#define    CR4DBG_IDISAR1_EXCEPT2INSTR_SHIFT                              8
#define    CR4DBG_IDISAR1_EXCEPT2INSTR_MASK                               0x00000F00
#define    CR4DBG_IDISAR1_EXCEPT1INSTR_SHIFT                              4
#define    CR4DBG_IDISAR1_EXCEPT1INSTR_MASK                               0x000000F0
#define    CR4DBG_IDISAR1_ENDIANINSTR_SHIFT                               0
#define    CR4DBG_IDISAR1_ENDIANINSTR_MASK                                0x0000000F

#define CR4DBG_IDISAR2_OFFSET                                             0x00000D48
#define CR4DBG_IDISAR2_TYPE                                               UInt32
#define CR4DBG_IDISAR2_RESERVED_MASK                                      0x00000000
#define    CR4DBG_IDISAR2_REVERSALINSTR_SHIFT                             28
#define    CR4DBG_IDISAR2_REVERSALINSTR_MASK                              0xF0000000
#define    CR4DBG_IDISAR2_PSRINSTR_SHIFT                                  24
#define    CR4DBG_IDISAR2_PSRINSTR_MASK                                   0x0F000000
#define    CR4DBG_IDISAR2_UMULTINSTR_SHIFT                                20
#define    CR4DBG_IDISAR2_UMULTINSTR_MASK                                 0x00F00000
#define    CR4DBG_IDISAR2_SMULTINSTR_SHIFT                                16
#define    CR4DBG_IDISAR2_SMULTINSTR_MASK                                 0x000F0000
#define    CR4DBG_IDISAR2_MULINSTR_SHIFT                                  12
#define    CR4DBG_IDISAR2_MULINSTR_MASK                                   0x0000F000
#define    CR4DBG_IDISAR2_INTINSTR_SHIFT                                  8
#define    CR4DBG_IDISAR2_INTINSTR_MASK                                   0x00000F00
#define    CR4DBG_IDISAR2_MEMHINTINSTR_SHIFT                              4
#define    CR4DBG_IDISAR2_MEMHINTINSTR_MASK                               0x000000F0
#define    CR4DBG_IDISAR2_LDSTINSTR_SHIFT                                 0
#define    CR4DBG_IDISAR2_LDSTINSTR_MASK                                  0x0000000F

#define CR4DBG_IDISAR3_OFFSET                                             0x00000D4C
#define CR4DBG_IDISAR3_TYPE                                               UInt32
#define CR4DBG_IDISAR3_RESERVED_MASK                                      0x00000000
#define    CR4DBG_IDISAR3_THUMBEE_SHIFT                                   28
#define    CR4DBG_IDISAR3_THUMBEE_MASK                                    0xF0000000
#define    CR4DBG_IDISAR3_TRUENOPINSTR_SHIFT                              24
#define    CR4DBG_IDISAR3_TRUENOPINSTR_MASK                               0x0F000000
#define    CR4DBG_IDISAR3_THUMBCPINSTR_SHIFT                              20
#define    CR4DBG_IDISAR3_THUMBCPINSTR_MASK                               0x00F00000
#define    CR4DBG_IDISAR3_TBLBRINSTR_SHIFT                                16
#define    CR4DBG_IDISAR3_TBLBRINSTR_MASK                                 0x000F0000
#define    CR4DBG_IDISAR3_SYNCPRIMINSTR_SHIFT                             12
#define    CR4DBG_IDISAR3_SYNCPRIMINSTR_MASK                              0x0000F000
#define    CR4DBG_IDISAR3_SVCINSTR_SHIFT                                  8
#define    CR4DBG_IDISAR3_SVCINSTR_MASK                                   0x00000F00
#define    CR4DBG_IDISAR3_SIMDINSTR_SHIFT                                 4
#define    CR4DBG_IDISAR3_SIMDINSTR_MASK                                  0x000000F0
#define    CR4DBG_IDISAR3_SATINSTR_SHIFT                                  0
#define    CR4DBG_IDISAR3_SATINSTR_MASK                                   0x0000000F

#define CR4DBG_IDISAR4_OFFSET                                             0x00000D50
#define CR4DBG_IDISAR4_TYPE                                               UInt32
#define CR4DBG_IDISAR4_RESERVED_MASK                                      0xFF000000
#define    CR4DBG_IDISAR4_EXCLINSTR_SHIFT                                 20
#define    CR4DBG_IDISAR4_EXCLINSTR_MASK                                  0x00F00000
#define    CR4DBG_IDISAR4_BARRIERINSTR_SHIFT                              16
#define    CR4DBG_IDISAR4_BARRIERINSTR_MASK                               0x000F0000
#define    CR4DBG_IDISAR4_SMCINSTR_SHIFT                                  12
#define    CR4DBG_IDISAR4_SMCINSTR_MASK                                   0x0000F000
#define    CR4DBG_IDISAR4_WBINSTR_SHIFT                                   8
#define    CR4DBG_IDISAR4_WBINSTR_MASK                                    0x00000F00
#define    CR4DBG_IDISAR4_WITHSHFTINSTR_SHIFT                             4
#define    CR4DBG_IDISAR4_WITHSHFTINSTR_MASK                              0x000000F0
#define    CR4DBG_IDISAR4_UNPROVINSTR_SHIFT                               0
#define    CR4DBG_IDISAR4_UNPROVINSTR_MASK                                0x0000000F

#define CR4DBG_IDISAR5_OFFSET                                             0x00000D54
#define CR4DBG_IDISAR5_TYPE                                               UInt32
#define CR4DBG_IDISAR5_RESERVED_MASK                                      0x00000000
#define    CR4DBG_IDISAR5_ISAR5_SHIFT                                     0
#define    CR4DBG_IDISAR5_ISAR5_MASK                                      0xFFFFFFFF

#define CR4DBG_ITETMIF_OFFSET                                             0x00000ED8
#define CR4DBG_ITETMIF_TYPE                                               UInt32
#define CR4DBG_ITETMIF_RESERVED_MASK                                      0xFFFF8000
#define    CR4DBG_ITETMIF_EVNTBUS46_SHIFT                                 14
#define    CR4DBG_ITETMIF_EVNTBUS46_MASK                                  0x00004000
#define    CR4DBG_ITETMIF_EVNTBUS28_SHIFT                                 13
#define    CR4DBG_ITETMIF_EVNTBUS28_MASK                                  0x00002000
#define    CR4DBG_ITETMIF_EVNTBUS0_SHIFT                                  12
#define    CR4DBG_ITETMIF_EVNTBUS0_MASK                                   0x00001000
#define    CR4DBG_ITETMIF_ETMCID31_SHIFT                                  11
#define    CR4DBG_ITETMIF_ETMCID31_MASK                                   0x00000800
#define    CR4DBG_ITETMIF_ETMCID0_SHIFT                                   10
#define    CR4DBG_ITETMIF_ETMCID0_MASK                                    0x00000400
#define    CR4DBG_ITETMIF_ETMDD63_SHIFT                                   9
#define    CR4DBG_ITETMIF_ETMDD63_MASK                                    0x00000200
#define    CR4DBG_ITETMIF_ETMDD0_SHIFT                                    8
#define    CR4DBG_ITETMIF_ETMDD0_MASK                                     0x00000100
#define    CR4DBG_ITETMIF_ETMDA31_SHIFT                                   7
#define    CR4DBG_ITETMIF_ETMDA31_MASK                                    0x00000080
#define    CR4DBG_ITETMIF_ETMDA0_SHIFT                                    6
#define    CR4DBG_ITETMIF_ETMDA0_MASK                                     0x00000040
#define    CR4DBG_ITETMIF_ETMDCTL11_SHIFT                                 5
#define    CR4DBG_ITETMIF_ETMDCTL11_MASK                                  0x00000020
#define    CR4DBG_ITETMIF_ETMDCTL0_SHIFT                                  4
#define    CR4DBG_ITETMIF_ETMDCTL0_MASK                                   0x00000010
#define    CR4DBG_ITETMIF_ETMIA31_SHIFT                                   3
#define    CR4DBG_ITETMIF_ETMIA31_MASK                                    0x00000008
#define    CR4DBG_ITETMIF_ETMIA1_SHIFT                                    2
#define    CR4DBG_ITETMIF_ETMIA1_MASK                                     0x00000004
#define    CR4DBG_ITETMIF_ETMICTL13_SHIFT                                 1
#define    CR4DBG_ITETMIF_ETMICTL13_MASK                                  0x00000002
#define    CR4DBG_ITETMIF_ETMICTL0_SHIFT                                  0
#define    CR4DBG_ITETMIF_ETMICTL0_MASK                                   0x00000001

#define CR4DBG_ITMISCOUT_OFFSET                                           0x00000EF8
#define CR4DBG_ITMISCOUT_TYPE                                             UInt32
#define CR4DBG_ITMISCOUT_RESERVED_MASK                                    0xFFFFFCC8
#define    CR4DBG_ITMISCOUT_DBGRESTARTED_SHIFT                            9
#define    CR4DBG_ITMISCOUT_DBGRESTARTED_MASK                             0x00000200
#define    CR4DBG_ITMISCOUT_DBGTRIGGER_SHIFT                              8
#define    CR4DBG_ITMISCOUT_DBGTRIGGER_MASK                               0x00000100
#define    CR4DBG_ITMISCOUT_ETMWFIPENDING_SHIFT                           5
#define    CR4DBG_ITMISCOUT_ETMWFIPENDING_MASK                            0x00000020
#define    CR4DBG_ITMISCOUT_NPMUIRQ_SHIFT                                 4
#define    CR4DBG_ITMISCOUT_NPMUIRQ_MASK                                  0x00000010
#define    CR4DBG_ITMISCOUT_COMMTX_SHIFT                                  2
#define    CR4DBG_ITMISCOUT_COMMTX_MASK                                   0x00000004
#define    CR4DBG_ITMISCOUT_COMMRX_SHIFT                                  1
#define    CR4DBG_ITMISCOUT_COMMRX_MASK                                   0x00000002
#define    CR4DBG_ITMISCOUT_DBGACK_SHIFT                                  0
#define    CR4DBG_ITMISCOUT_DBGACK_MASK                                   0x00000001

#define CR4DBG_ITMISCIN_OFFSET                                            0x00000EFC
#define CR4DBG_ITMISCIN_TYPE                                              UInt32
#define CR4DBG_ITMISCIN_RESERVED_MASK                                     0xFFFFF4D8
#define    CR4DBG_ITMISCIN_DBGRESTART_SHIFT                               11
#define    CR4DBG_ITMISCIN_DBGRESTART_MASK                                0x00000800
#define    CR4DBG_ITMISCIN_ETMEXTOUT_SHIFT                                8
#define    CR4DBG_ITMISCIN_ETMEXTOUT_MASK                                 0x00000300
#define    CR4DBG_ITMISCIN_NETMWFIREADY_SHIFT                             5
#define    CR4DBG_ITMISCIN_NETMWFIREADY_MASK                              0x00000020
#define    CR4DBG_ITMISCIN_NFIQ_SHIFT                                     2
#define    CR4DBG_ITMISCIN_NFIQ_MASK                                      0x00000004
#define    CR4DBG_ITMISCIN_NIRQ_SHIFT                                     1
#define    CR4DBG_ITMISCIN_NIRQ_MASK                                      0x00000002
#define    CR4DBG_ITMISCIN_EDBGRQ_SHIFT                                   0
#define    CR4DBG_ITMISCIN_EDBGRQ_MASK                                    0x00000001

#define CR4DBG_ITCTRL_OFFSET                                              0x00000F00
#define CR4DBG_ITCTRL_TYPE                                                UInt32
#define CR4DBG_ITCTRL_RESERVED_MASK                                       0xFFFFFFFE
#define    CR4DBG_ITCTRL_INTMODE_SHIFT                                    0
#define    CR4DBG_ITCTRL_INTMODE_MASK                                     0x00000001

#define CR4DBG_CLAIMSET_OFFSET                                            0x00000FA0
#define CR4DBG_CLAIMSET_TYPE                                              UInt32
#define CR4DBG_CLAIMSET_RESERVED_MASK                                     0xFFFFFF00
#define    CR4DBG_CLAIMSET_CLAIMTAGSET_SHIFT                              0
#define    CR4DBG_CLAIMSET_CLAIMTAGSET_MASK                               0x000000FF

#define CR4DBG_CLAIMCLR_OFFSET                                            0x00000FA4
#define CR4DBG_CLAIMCLR_TYPE                                              UInt32
#define CR4DBG_CLAIMCLR_RESERVED_MASK                                     0xFFFFFF00
#define    CR4DBG_CLAIMCLR_CLAIMTAGCLEAR_SHIFT                            0
#define    CR4DBG_CLAIMCLR_CLAIMTAGCLEAR_MASK                             0x000000FF

#define CR4DBG_LOCKACCESS_OFFSET                                          0x00000FB0
#define CR4DBG_LOCKACCESS_TYPE                                            UInt32
#define CR4DBG_LOCKACCESS_RESERVED_MASK                                   0x00000000
#define    CR4DBG_LOCKACCESS_KEY_SHIFT                                    0
#define    CR4DBG_LOCKACCESS_KEY_MASK                                     0xFFFFFFFF

#define CR4DBG_LOCKSTATUS_OFFSET                                          0x00000FB4
#define CR4DBG_LOCKSTATUS_TYPE                                            UInt32
#define CR4DBG_LOCKSTATUS_RESERVED_MASK                                   0xFFFFFFFF

#define CR4DBG_AUTHSTATUS_OFFSET                                          0x00000FB8
#define CR4DBG_AUTHSTATUS_TYPE                                            UInt32
#define CR4DBG_AUTHSTATUS_RESERVED_MASK                                   0xFFFFFF00
#define    CR4DBG_AUTHSTATUS_SECNINVDFIMPL_SHIFT                          7
#define    CR4DBG_AUTHSTATUS_SECNINVDFIMPL_MASK                           0x00000080
#define    CR4DBG_AUTHSTATUS_SECNINVDFEN_SHIFT                            6
#define    CR4DBG_AUTHSTATUS_SECNINVDFEN_MASK                             0x00000040
#define    CR4DBG_AUTHSTATUS_SECINVDFIMPL_SHIFT                           5
#define    CR4DBG_AUTHSTATUS_SECINVDFIMPL_MASK                            0x00000020
#define    CR4DBG_AUTHSTATUS_SECINVDFEN_SHIFT                             4
#define    CR4DBG_AUTHSTATUS_SECINVDFEN_MASK                              0x00000010
#define    CR4DBG_AUTHSTATUS_NSECDBGF_SHIFT                               0
#define    CR4DBG_AUTHSTATUS_NSECDBGF_MASK                                0x0000000F

#define CR4DBG_DEVID_OFFSET                                               0x00000FC8
#define CR4DBG_DEVID_TYPE                                                 UInt32
#define CR4DBG_DEVID_RESERVED_MASK                                        0xFFFFFFFF

#define CR4DBG_DEVTYPE_OFFSET                                             0x00000FCC
#define CR4DBG_DEVTYPE_TYPE                                               UInt32
#define CR4DBG_DEVTYPE_RESERVED_MASK                                      0xFFFFFF00
#define    CR4DBG_DEVTYPE_SUBTYPE_SHIFT                                   4
#define    CR4DBG_DEVTYPE_SUBTYPE_MASK                                    0x000000F0
#define    CR4DBG_DEVTYPE_MAINCLASS_SHIFT                                 0
#define    CR4DBG_DEVTYPE_MAINCLASS_MASK                                  0x0000000F

#define CR4DBG_PERIPHID4_OFFSET                                           0x00000FD0
#define CR4DBG_PERIPHID4_TYPE                                             UInt32
#define CR4DBG_PERIPHID4_RESERVED_MASK                                    0xFFFFFF00
#define    CR4DBG_PERIPHID4_DEBUGBLOCKS_SHIFT                             4
#define    CR4DBG_PERIPHID4_DEBUGBLOCKS_MASK                              0x000000F0
#define    CR4DBG_PERIPHID4_JEP106CNTCODE_SHIFT                           0
#define    CR4DBG_PERIPHID4_JEP106CNTCODE_MASK                            0x0000000F

#define CR4DBG_PERIPHID0_OFFSET                                           0x00000FE0
#define CR4DBG_PERIPHID0_TYPE                                             UInt32
#define CR4DBG_PERIPHID0_RESERVED_MASK                                    0xFFFFFF00
#define    CR4DBG_PERIPHID0_PARTNUMBER_SHIFT                              0
#define    CR4DBG_PERIPHID0_PARTNUMBER_MASK                               0x000000FF

#define CR4DBG_PERIPHID1_OFFSET                                           0x00000FE4
#define CR4DBG_PERIPHID1_TYPE                                             UInt32
#define CR4DBG_PERIPHID1_RESERVED_MASK                                    0xFFFFFF00
#define    CR4DBG_PERIPHID1_JEP106IDCODE_SHIFT                            4
#define    CR4DBG_PERIPHID1_JEP106IDCODE_MASK                             0x000000F0
#define    CR4DBG_PERIPHID1_PARTNUMBER_SHIFT                              0
#define    CR4DBG_PERIPHID1_PARTNUMBER_MASK                               0x0000000F

#define CR4DBG_PERIPHID2_OFFSET                                           0x00000FE8
#define CR4DBG_PERIPHID2_TYPE                                             UInt32
#define CR4DBG_PERIPHID2_RESERVED_MASK                                    0xFFFFFF00
#define    CR4DBG_PERIPHID2_REVNUMBER_SHIFT                               4
#define    CR4DBG_PERIPHID2_REVNUMBER_MASK                                0x000000F0
#define    CR4DBG_PERIPHID2_JEP106FEATURE_SHIFT                           3
#define    CR4DBG_PERIPHID2_JEP106FEATURE_MASK                            0x00000008
#define    CR4DBG_PERIPHID2_JEP106IDCODE_SHIFT                            0
#define    CR4DBG_PERIPHID2_JEP106IDCODE_MASK                             0x00000007

#define CR4DBG_PERIPHID3_OFFSET                                           0x00000FEC
#define CR4DBG_PERIPHID3_TYPE                                             UInt32
#define CR4DBG_PERIPHID3_RESERVED_MASK                                    0xFFFFFF00
#define    CR4DBG_PERIPHID3_MFRREVNUMBER_SHIFT                            4
#define    CR4DBG_PERIPHID3_MFRREVNUMBER_MASK                             0x000000F0
#define    CR4DBG_PERIPHID3_CSTMRMOD_SHIFT                                0
#define    CR4DBG_PERIPHID3_CSTMRMOD_MASK                                 0x0000000F

#define CR4DBG_COMPONENTID0_OFFSET                                        0x00000FF0
#define CR4DBG_COMPONENTID0_TYPE                                          UInt32
#define CR4DBG_COMPONENTID0_RESERVED_MASK                                 0xFFFFFF00
#define    CR4DBG_COMPONENTID0_COMPID_SHIFT                               0
#define    CR4DBG_COMPONENTID0_COMPID_MASK                                0x000000FF

#define CR4DBG_COMPONENTID1_OFFSET                                        0x00000FF4
#define CR4DBG_COMPONENTID1_TYPE                                          UInt32
#define CR4DBG_COMPONENTID1_RESERVED_MASK                                 0xFFFFFF00
#define    CR4DBG_COMPONENTID1_COMPID_SHIFT                               0
#define    CR4DBG_COMPONENTID1_COMPID_MASK                                0x000000FF

#define CR4DBG_COMPONENTID2_OFFSET                                        0x00000FF8
#define CR4DBG_COMPONENTID2_TYPE                                          UInt32
#define CR4DBG_COMPONENTID2_RESERVED_MASK                                 0xFFFFFF00
#define    CR4DBG_COMPONENTID2_COMPID_SHIFT                               0
#define    CR4DBG_COMPONENTID2_COMPID_MASK                                0x000000FF

#define CR4DBG_COMPONENTID3_OFFSET                                        0x00000FFC
#define CR4DBG_COMPONENTID3_TYPE                                          UInt32
#define CR4DBG_COMPONENTID3_RESERVED_MASK                                 0xFFFFFF00
#define    CR4DBG_COMPONENTID3_COMPID_SHIFT                               0
#define    CR4DBG_COMPONENTID3_COMPID_MASK                                0x000000FF

#endif /* __BRCM_RDB_CR4DBG_H__ */



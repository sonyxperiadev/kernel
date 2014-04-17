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

#ifndef __BRCM_RDB_CORTEXA7_DEBUG_H__
#define __BRCM_RDB_CORTEXA7_DEBUG_H__

#define CORTEXA7_DEBUG_DBGDIDR_OFFSET                                     0x00000000
#define CORTEXA7_DEBUG_DBGDIDR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGDIDR_RESERVED_MASK                              0x00000F00
#define    CORTEXA7_DEBUG_DBGDIDR_WRPS_SHIFT                              28
#define    CORTEXA7_DEBUG_DBGDIDR_WRPS_MASK                               0xF0000000
#define    CORTEXA7_DEBUG_DBGDIDR_BRPS_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGDIDR_BRPS_MASK                               0x0F000000
#define    CORTEXA7_DEBUG_DBGDIDR_CTX_CMPS_SHIFT                          20
#define    CORTEXA7_DEBUG_DBGDIDR_CTX_CMPS_MASK                           0x00F00000
#define    CORTEXA7_DEBUG_DBGDIDR_VERSION_SHIFT                           16
#define    CORTEXA7_DEBUG_DBGDIDR_VERSION_MASK                            0x000F0000
#define    CORTEXA7_DEBUG_DBGDIDR_DEVID_IMP_SHIFT                         15
#define    CORTEXA7_DEBUG_DBGDIDR_DEVID_IMP_MASK                          0x00008000
#define    CORTEXA7_DEBUG_DBGDIDR_NSUHD_IMP_SHIFT                         14
#define    CORTEXA7_DEBUG_DBGDIDR_NSUHD_IMP_MASK                          0x00004000
#define    CORTEXA7_DEBUG_DBGDIDR_PCSR_IMP_SHIFT                          13
#define    CORTEXA7_DEBUG_DBGDIDR_PCSR_IMP_MASK                           0x00002000
#define    CORTEXA7_DEBUG_DBGDIDR_SE_IMP_SHIFT                            12
#define    CORTEXA7_DEBUG_DBGDIDR_SE_IMP_MASK                             0x00001000
#define    CORTEXA7_DEBUG_DBGDIDR_VARIANT_SHIFT                           4
#define    CORTEXA7_DEBUG_DBGDIDR_VARIANT_MASK                            0x000000F0
#define    CORTEXA7_DEBUG_DBGDIDR_REVISION_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGDIDR_REVISION_MASK                           0x0000000F

#define CORTEXA7_DEBUG_DBGDSCR_INT_OFFSET                                 0x00000004
#define CORTEXA7_DEBUG_DBGDSCR_INT_TYPE                                   UInt32
#define CORTEXA7_DEBUG_DBGDSCR_INT_RESERVED_MASK                          0x90C00000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_RXFULL_SHIFT                        30
#define    CORTEXA7_DEBUG_DBGDSCR_INT_RXFULL_MASK                         0x40000000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_TXFULL_SHIFT                        29
#define    CORTEXA7_DEBUG_DBGDSCR_INT_TXFULL_MASK                         0x20000000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_RXFULLL_SHIFT                       27
#define    CORTEXA7_DEBUG_DBGDSCR_INT_RXFULLL_MASK                        0x08000000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_TXFULLL_SHIFT                       26
#define    CORTEXA7_DEBUG_DBGDSCR_INT_TXFULLL_MASK                        0x04000000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_PIPEADV_SHIFT                       25
#define    CORTEXA7_DEBUG_DBGDSCR_INT_PIPEADV_MASK                        0x02000000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_INSTRCOMPLL_SHIFT                   24
#define    CORTEXA7_DEBUG_DBGDSCR_INT_INSTRCOMPLL_MASK                    0x01000000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_EXTDCCMODE_SHIFT                    20
#define    CORTEXA7_DEBUG_DBGDSCR_INT_EXTDCCMODE_MASK                     0x00300000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_ADADISCARD_SHIFT                    19
#define    CORTEXA7_DEBUG_DBGDSCR_INT_ADADISCARD_MASK                     0x00080000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_NS_SHIFT                            18
#define    CORTEXA7_DEBUG_DBGDSCR_INT_NS_MASK                             0x00040000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_SPNIDDIS_SHIFT                      17
#define    CORTEXA7_DEBUG_DBGDSCR_INT_SPNIDDIS_MASK                       0x00020000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_SPIDDIS_SHIFT                       16
#define    CORTEXA7_DEBUG_DBGDSCR_INT_SPIDDIS_MASK                        0x00010000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_MDBGEN_SHIFT                        15
#define    CORTEXA7_DEBUG_DBGDSCR_INT_MDBGEN_MASK                         0x00008000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_HDBGEN_SHIFT                        14
#define    CORTEXA7_DEBUG_DBGDSCR_INT_HDBGEN_MASK                         0x00004000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_ITREN_SHIFT                         13
#define    CORTEXA7_DEBUG_DBGDSCR_INT_ITREN_MASK                          0x00002000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_UDCCDIS_SHIFT                       12
#define    CORTEXA7_DEBUG_DBGDSCR_INT_UDCCDIS_MASK                        0x00001000
#define    CORTEXA7_DEBUG_DBGDSCR_INT_INTDIS_SHIFT                        11
#define    CORTEXA7_DEBUG_DBGDSCR_INT_INTDIS_MASK                         0x00000800
#define    CORTEXA7_DEBUG_DBGDSCR_INT_DBGACK_SHIFT                        10
#define    CORTEXA7_DEBUG_DBGDSCR_INT_DBGACK_MASK                         0x00000400
#define    CORTEXA7_DEBUG_DBGDSCR_INT_FS_SHIFT                            9
#define    CORTEXA7_DEBUG_DBGDSCR_INT_FS_MASK                             0x00000200
#define    CORTEXA7_DEBUG_DBGDSCR_INT_UNDL_SHIFT                          8
#define    CORTEXA7_DEBUG_DBGDSCR_INT_UNDL_MASK                           0x00000100
#define    CORTEXA7_DEBUG_DBGDSCR_INT_ADABORTL_SHIFT                      7
#define    CORTEXA7_DEBUG_DBGDSCR_INT_ADABORTL_MASK                       0x00000080
#define    CORTEXA7_DEBUG_DBGDSCR_INT_SDABORTL_SHIFT                      6
#define    CORTEXA7_DEBUG_DBGDSCR_INT_SDABORTL_MASK                       0x00000040
#define    CORTEXA7_DEBUG_DBGDSCR_INT_MOE_SHIFT                           2
#define    CORTEXA7_DEBUG_DBGDSCR_INT_MOE_MASK                            0x0000003C
#define    CORTEXA7_DEBUG_DBGDSCR_INT_RESTARTED_SHIFT                     1
#define    CORTEXA7_DEBUG_DBGDSCR_INT_RESTARTED_MASK                      0x00000002
#define    CORTEXA7_DEBUG_DBGDSCR_INT_HALTED_SHIFT                        0
#define    CORTEXA7_DEBUG_DBGDSCR_INT_HALTED_MASK                         0x00000001

#define CORTEXA7_DEBUG_DBGWFAR_OFFSET                                     0x00000018
#define CORTEXA7_DEBUG_DBGWFAR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWFAR_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGWFAR_IAPLUSOFFSET_SHIFT                      0
#define    CORTEXA7_DEBUG_DBGWFAR_IAPLUSOFFSET_MASK                       0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGVCR_OFFSET                                      0x0000001C
#define CORTEXA7_DEBUG_DBGVCR_TYPE                                        UInt32
#define CORTEXA7_DEBUG_DBGVCR_RESERVED_MASK                               0x21012320
#define    CORTEXA7_DEBUG_DBGVCR_NSF_SHIFT                                31
#define    CORTEXA7_DEBUG_DBGVCR_NSF_MASK                                 0x80000000
#define    CORTEXA7_DEBUG_DBGVCR_NSI_SHIFT                                30
#define    CORTEXA7_DEBUG_DBGVCR_NSI_MASK                                 0x40000000
#define    CORTEXA7_DEBUG_DBGVCR_NSD_SHIFT                                28
#define    CORTEXA7_DEBUG_DBGVCR_NSD_MASK                                 0x10000000
#define    CORTEXA7_DEBUG_DBGVCR_NSP_SHIFT                                27
#define    CORTEXA7_DEBUG_DBGVCR_NSP_MASK                                 0x08000000
#define    CORTEXA7_DEBUG_DBGVCR_NSS_SHIFT                                26
#define    CORTEXA7_DEBUG_DBGVCR_NSS_MASK                                 0x04000000
#define    CORTEXA7_DEBUG_DBGVCR_NSU_SHIFT                                25
#define    CORTEXA7_DEBUG_DBGVCR_NSU_MASK                                 0x02000000
#define    CORTEXA7_DEBUG_DBGVCR_NSHF_SHIFT                               23
#define    CORTEXA7_DEBUG_DBGVCR_NSHF_MASK                                0x00800000
#define    CORTEXA7_DEBUG_DBGVCR_NSHI_SHIFT                               22
#define    CORTEXA7_DEBUG_DBGVCR_NSHI_MASK                                0x00400000
#define    CORTEXA7_DEBUG_DBGVCR_NSHE_SHIFT                               21
#define    CORTEXA7_DEBUG_DBGVCR_NSHE_MASK                                0x00200000
#define    CORTEXA7_DEBUG_DBGVCR_NSHD_SHIFT                               20
#define    CORTEXA7_DEBUG_DBGVCR_NSHD_MASK                                0x00100000
#define    CORTEXA7_DEBUG_DBGVCR_NSHP_SHIFT                               19
#define    CORTEXA7_DEBUG_DBGVCR_NSHP_MASK                                0x00080000
#define    CORTEXA7_DEBUG_DBGVCR_NSHC_SHIFT                               18
#define    CORTEXA7_DEBUG_DBGVCR_NSHC_MASK                                0x00040000
#define    CORTEXA7_DEBUG_DBGVCR_NSHU_SHIFT                               17
#define    CORTEXA7_DEBUG_DBGVCR_NSHU_MASK                                0x00020000
#define    CORTEXA7_DEBUG_DBGVCR_MF_SHIFT                                 15
#define    CORTEXA7_DEBUG_DBGVCR_MF_MASK                                  0x00008000
#define    CORTEXA7_DEBUG_DBGVCR_MI_SHIFT                                 14
#define    CORTEXA7_DEBUG_DBGVCR_MI_MASK                                  0x00004000
#define    CORTEXA7_DEBUG_DBGVCR_MD_SHIFT                                 12
#define    CORTEXA7_DEBUG_DBGVCR_MD_MASK                                  0x00001000
#define    CORTEXA7_DEBUG_DBGVCR_MP_SHIFT                                 11
#define    CORTEXA7_DEBUG_DBGVCR_MP_MASK                                  0x00000800
#define    CORTEXA7_DEBUG_DBGVCR_MS_SHIFT                                 10
#define    CORTEXA7_DEBUG_DBGVCR_MS_MASK                                  0x00000400
#define    CORTEXA7_DEBUG_DBGVCR_SF_SHIFT                                 7
#define    CORTEXA7_DEBUG_DBGVCR_SF_MASK                                  0x00000080
#define    CORTEXA7_DEBUG_DBGVCR_SI_SHIFT                                 6
#define    CORTEXA7_DEBUG_DBGVCR_SI_MASK                                  0x00000040
#define    CORTEXA7_DEBUG_DBGVCR_SD_SHIFT                                 4
#define    CORTEXA7_DEBUG_DBGVCR_SD_MASK                                  0x00000010
#define    CORTEXA7_DEBUG_DBGVCR_SP_SHIFT                                 3
#define    CORTEXA7_DEBUG_DBGVCR_SP_MASK                                  0x00000008
#define    CORTEXA7_DEBUG_DBGVCR_SS_SHIFT                                 2
#define    CORTEXA7_DEBUG_DBGVCR_SS_MASK                                  0x00000004
#define    CORTEXA7_DEBUG_DBGVCR_SU_SHIFT                                 1
#define    CORTEXA7_DEBUG_DBGVCR_SU_MASK                                  0x00000002
#define    CORTEXA7_DEBUG_DBGVCR_R_SHIFT                                  0
#define    CORTEXA7_DEBUG_DBGVCR_R_MASK                                   0x00000001

#define CORTEXA7_DEBUG_DBGECR_OFFSET                                      0x00000024
#define CORTEXA7_DEBUG_DBGECR_TYPE                                        UInt32
#define CORTEXA7_DEBUG_DBGECR_RESERVED_MASK                               0xFFFFFFFE
#define    CORTEXA7_DEBUG_DBGECR_OSUCE_SHIFT                              0
#define    CORTEXA7_DEBUG_DBGECR_OSUCE_MASK                               0x00000001

#define CORTEXA7_DEBUG_DBGDTRRX_EXT_OFFSET                                0x00000080
#define CORTEXA7_DEBUG_DBGDTRRX_EXT_TYPE                                  UInt32
#define CORTEXA7_DEBUG_DBGDTRRX_EXT_RESERVED_MASK                         0x00000000
#define    CORTEXA7_DEBUG_DBGDTRRX_EXT_HOSTTOTARGETDATA_SHIFT             0
#define    CORTEXA7_DEBUG_DBGDTRRX_EXT_HOSTTOTARGETDATA_MASK              0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGITR_OFFSET                                      0x00000084
#define CORTEXA7_DEBUG_DBGITR_TYPE                                        UInt32
#define CORTEXA7_DEBUG_DBGITR_RESERVED_MASK                               0x00000000
#define    CORTEXA7_DEBUG_DBGITR_ARMINSTR_SHIFT                           0
#define    CORTEXA7_DEBUG_DBGITR_ARMINSTR_MASK                            0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGDSCR_EXT_OFFSET                                 0x00000088
#define CORTEXA7_DEBUG_DBGDSCR_EXT_TYPE                                   UInt32
#define CORTEXA7_DEBUG_DBGDSCR_EXT_RESERVED_MASK                          0x90C00000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_RXFULL_SHIFT                        30
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_RXFULL_MASK                         0x40000000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_TXFULL_SHIFT                        29
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_TXFULL_MASK                         0x20000000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_RXFULLL_SHIFT                       27
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_RXFULLL_MASK                        0x08000000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_TXFULLL_SHIFT                       26
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_TXFULLL_MASK                        0x04000000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_PIPEADV_SHIFT                       25
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_PIPEADV_MASK                        0x02000000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_INSTRCOMPLL_SHIFT                   24
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_INSTRCOMPLL_MASK                    0x01000000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_EXTDCCMODE_SHIFT                    20
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_EXTDCCMODE_MASK                     0x00300000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_ADADISCARD_SHIFT                    19
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_ADADISCARD_MASK                     0x00080000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_NS_SHIFT                            18
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_NS_MASK                             0x00040000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_SPNIDDIS_SHIFT                      17
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_SPNIDDIS_MASK                       0x00020000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_SPIDDIS_SHIFT                       16
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_SPIDDIS_MASK                        0x00010000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_MDBGEN_SHIFT                        15
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_MDBGEN_MASK                         0x00008000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_HDBGEN_SHIFT                        14
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_HDBGEN_MASK                         0x00004000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_ITREN_SHIFT                         13
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_ITREN_MASK                          0x00002000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_UDCCDIS_SHIFT                       12
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_UDCCDIS_MASK                        0x00001000
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_INTDIS_SHIFT                        11
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_INTDIS_MASK                         0x00000800
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_DBGACK_SHIFT                        10
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_DBGACK_MASK                         0x00000400
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_FS_SHIFT                            9
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_FS_MASK                             0x00000200
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_UNDL_SHIFT                          8
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_UNDL_MASK                           0x00000100
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_ADABORTL_SHIFT                      7
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_ADABORTL_MASK                       0x00000080
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_SDABORTL_SHIFT                      6
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_SDABORTL_MASK                       0x00000040
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_MOE_SHIFT                           2
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_MOE_MASK                            0x0000003C
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_RESTARTED_SHIFT                     1
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_RESTARTED_MASK                      0x00000002
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_HALTED_SHIFT                        0
#define    CORTEXA7_DEBUG_DBGDSCR_EXT_HALTED_MASK                         0x00000001

#define CORTEXA7_DEBUG_DBGDTRTX_OFFSET                                    0x0000008C
#define CORTEXA7_DEBUG_DBGDTRTX_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGDTRTX_RESERVED_MASK                             0x00000000
#define    CORTEXA7_DEBUG_DBGDTRTX_TARGETTOHOSTDATA_SHIFT                 0
#define    CORTEXA7_DEBUG_DBGDTRTX_TARGETTOHOSTDATA_MASK                  0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGDRCR_OFFSET                                     0x00000090
#define CORTEXA7_DEBUG_DBGDRCR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGDRCR_RESERVED_MASK                              0xFFFFFFE0
#define    CORTEXA7_DEBUG_DBGDRCR_CBRRQ_SHIFT                             4
#define    CORTEXA7_DEBUG_DBGDRCR_CBRRQ_MASK                              0x00000010
#define    CORTEXA7_DEBUG_DBGDRCR_CSPA_SHIFT                              3
#define    CORTEXA7_DEBUG_DBGDRCR_CSPA_MASK                               0x00000008
#define    CORTEXA7_DEBUG_DBGDRCR_CSE_SHIFT                               2
#define    CORTEXA7_DEBUG_DBGDRCR_CSE_MASK                                0x00000004
#define    CORTEXA7_DEBUG_DBGDRCR_RRQ_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGDRCR_RRQ_MASK                                0x00000002
#define    CORTEXA7_DEBUG_DBGDRCR_HRQ_SHIFT                               0
#define    CORTEXA7_DEBUG_DBGDRCR_HRQ_MASK                                0x00000001

#define CORTEXA7_DEBUG_DBGEACR_OFFSET                                     0x00000094
#define CORTEXA7_DEBUG_DBGEACR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGEACR_RESERVED_MASK                              0xFFFFFFF7
#define    CORTEXA7_DEBUG_DBGEACR_CORE_DEBUG_RESET_STATUS_SHIFT           3
#define    CORTEXA7_DEBUG_DBGEACR_CORE_DEBUG_RESET_STATUS_MASK            0x00000008

#define CORTEXA7_DEBUG_DBGPCSR_OFFSET                                     0x000000A0
#define CORTEXA7_DEBUG_DBGPCSR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPCSR_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGPCSR_PCS_SHIFT                               2
#define    CORTEXA7_DEBUG_DBGPCSR_PCS_MASK                                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGPCSR_IS_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGPCSR_IS_MASK                                 0x00000003

#define CORTEXA7_DEBUG_DBGCIDSR_OFFSET                                    0x000000A4
#define CORTEXA7_DEBUG_DBGCIDSR_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGCIDSR_RESERVED_MASK                             0x00000000
#define    CORTEXA7_DEBUG_DBGCIDSR_CONTEXTIDRSAMPVAL_SHIFT                0
#define    CORTEXA7_DEBUG_DBGCIDSR_CONTEXTIDRSAMPVAL_MASK                 0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGVIDSR_OFFSET                                    0x000000A8
#define CORTEXA7_DEBUG_DBGVIDSR_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGVIDSR_RESERVED_MASK                             0x3FFFFF00
#define    CORTEXA7_DEBUG_DBGVIDSR_NS_SHIFT                               31
#define    CORTEXA7_DEBUG_DBGVIDSR_NS_MASK                                0x80000000
#define    CORTEXA7_DEBUG_DBGVIDSR_H_SHIFT                                30
#define    CORTEXA7_DEBUG_DBGVIDSR_H_MASK                                 0x40000000
#define    CORTEXA7_DEBUG_DBGVIDSR_VMID_SHIFT                             0
#define    CORTEXA7_DEBUG_DBGVIDSR_VMID_MASK                              0x000000FF

#define CORTEXA7_DEBUG_DBGBVR0_OFFSET                                     0x00000100
#define CORTEXA7_DEBUG_DBGBVR0_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBVR0_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGBVR0_INSTRUCTION_ADDRESS_SHIFT               2
#define    CORTEXA7_DEBUG_DBGBVR0_INSTRUCTION_ADDRESS_MASK                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGBVR0_RESERVED_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGBVR0_RESERVED_MASK                           0x00000003

#define CORTEXA7_DEBUG_DBGBVR1_OFFSET                                     0x00000104
#define CORTEXA7_DEBUG_DBGBVR1_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBVR1_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGBVR1_INSTRUCTION_ADDRESS_SHIFT               2
#define    CORTEXA7_DEBUG_DBGBVR1_INSTRUCTION_ADDRESS_MASK                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGBVR1_RESERVED_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGBVR1_RESERVED_MASK                           0x00000003

#define CORTEXA7_DEBUG_DBGBVR2_OFFSET                                     0x00000108
#define CORTEXA7_DEBUG_DBGBVR2_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBVR2_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGBVR2_INSTRUCTION_ADDRESS_SHIFT               2
#define    CORTEXA7_DEBUG_DBGBVR2_INSTRUCTION_ADDRESS_MASK                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGBVR2_RESERVED_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGBVR2_RESERVED_MASK                           0x00000003

#define CORTEXA7_DEBUG_DBGBVR3_OFFSET                                     0x0000010C
#define CORTEXA7_DEBUG_DBGBVR3_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBVR3_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGBVR3_INSTRUCTION_ADDRESS_SHIFT               2
#define    CORTEXA7_DEBUG_DBGBVR3_INSTRUCTION_ADDRESS_MASK                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGBVR3_RESERVED_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGBVR3_RESERVED_MASK                           0x00000003

#define CORTEXA7_DEBUG_DBGBVR4_OFFSET                                     0x00000110
#define CORTEXA7_DEBUG_DBGBVR4_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBVR4_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGBVR4_INSTRUCTION_ADDRESS_SHIFT               2
#define    CORTEXA7_DEBUG_DBGBVR4_INSTRUCTION_ADDRESS_MASK                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGBVR4_RESERVED_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGBVR4_RESERVED_MASK                           0x00000003

#define CORTEXA7_DEBUG_DBGBVR5_OFFSET                                     0x00000114
#define CORTEXA7_DEBUG_DBGBVR5_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBVR5_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGBVR5_INSTRUCTION_ADDRESS_SHIFT               2
#define    CORTEXA7_DEBUG_DBGBVR5_INSTRUCTION_ADDRESS_MASK                0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGBVR5_RESERVED_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGBVR5_RESERVED_MASK                           0x00000003

#define CORTEXA7_DEBUG_DBGBCR0_OFFSET                                     0x00000140
#define CORTEXA7_DEBUG_DBGBCR0_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBCR0_RESERVED_MASK                              0xE0001E18
#define    CORTEXA7_DEBUG_DBGBCR0_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGBCR0_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGBCR0_BT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGBCR0_BT_MASK                                 0x00F00000
#define    CORTEXA7_DEBUG_DBGBCR0_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGBCR0_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGBCR0_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGBCR0_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGBCR0_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGBCR0_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGBCR0_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGBCR0_BAS_MASK                                0x000001E0
#define    CORTEXA7_DEBUG_DBGBCR0_PMC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGBCR0_PMC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGBCR0_BE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGBCR0_BE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGBCR1_OFFSET                                     0x00000144
#define CORTEXA7_DEBUG_DBGBCR1_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBCR1_RESERVED_MASK                              0xE0001E18
#define    CORTEXA7_DEBUG_DBGBCR1_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGBCR1_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGBCR1_BT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGBCR1_BT_MASK                                 0x00F00000
#define    CORTEXA7_DEBUG_DBGBCR1_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGBCR1_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGBCR1_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGBCR1_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGBCR1_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGBCR1_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGBCR1_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGBCR1_BAS_MASK                                0x000001E0
#define    CORTEXA7_DEBUG_DBGBCR1_PMC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGBCR1_PMC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGBCR1_BE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGBCR1_BE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGBCR2_OFFSET                                     0x00000148
#define CORTEXA7_DEBUG_DBGBCR2_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBCR2_RESERVED_MASK                              0xE0001E18
#define    CORTEXA7_DEBUG_DBGBCR2_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGBCR2_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGBCR2_BT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGBCR2_BT_MASK                                 0x00F00000
#define    CORTEXA7_DEBUG_DBGBCR2_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGBCR2_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGBCR2_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGBCR2_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGBCR2_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGBCR2_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGBCR2_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGBCR2_BAS_MASK                                0x000001E0
#define    CORTEXA7_DEBUG_DBGBCR2_PMC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGBCR2_PMC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGBCR2_BE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGBCR2_BE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGBCR3_OFFSET                                     0x0000014C
#define CORTEXA7_DEBUG_DBGBCR3_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBCR3_RESERVED_MASK                              0xE0001E18
#define    CORTEXA7_DEBUG_DBGBCR3_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGBCR3_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGBCR3_BT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGBCR3_BT_MASK                                 0x00F00000
#define    CORTEXA7_DEBUG_DBGBCR3_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGBCR3_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGBCR3_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGBCR3_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGBCR3_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGBCR3_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGBCR3_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGBCR3_BAS_MASK                                0x000001E0
#define    CORTEXA7_DEBUG_DBGBCR3_PMC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGBCR3_PMC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGBCR3_BE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGBCR3_BE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGBCR4_OFFSET                                     0x00000150
#define CORTEXA7_DEBUG_DBGBCR4_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBCR4_RESERVED_MASK                              0xE0001E18
#define    CORTEXA7_DEBUG_DBGBCR4_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGBCR4_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGBCR4_BT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGBCR4_BT_MASK                                 0x00F00000
#define    CORTEXA7_DEBUG_DBGBCR4_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGBCR4_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGBCR4_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGBCR4_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGBCR4_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGBCR4_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGBCR4_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGBCR4_BAS_MASK                                0x000001E0
#define    CORTEXA7_DEBUG_DBGBCR4_PMC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGBCR4_PMC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGBCR4_BE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGBCR4_BE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGBCR5_OFFSET                                     0x00000154
#define CORTEXA7_DEBUG_DBGBCR5_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGBCR5_RESERVED_MASK                              0xE0001E18
#define    CORTEXA7_DEBUG_DBGBCR5_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGBCR5_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGBCR5_BT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGBCR5_BT_MASK                                 0x00F00000
#define    CORTEXA7_DEBUG_DBGBCR5_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGBCR5_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGBCR5_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGBCR5_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGBCR5_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGBCR5_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGBCR5_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGBCR5_BAS_MASK                                0x000001E0
#define    CORTEXA7_DEBUG_DBGBCR5_PMC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGBCR5_PMC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGBCR5_BE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGBCR5_BE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGWVR0_OFFSET                                     0x00000180
#define CORTEXA7_DEBUG_DBGWVR0_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWVR0_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGWVR0_DATA_ADDRESS_31_2_SHIFT                 2
#define    CORTEXA7_DEBUG_DBGWVR0_DATA_ADDRESS_31_2_MASK                  0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGWVR0_NA_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWVR0_NA_MASK                                 0x00000003

#define CORTEXA7_DEBUG_DBGWVR1_OFFSET                                     0x00000184
#define CORTEXA7_DEBUG_DBGWVR1_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWVR1_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGWVR1_DATA_ADDRESS_31_2_SHIFT                 2
#define    CORTEXA7_DEBUG_DBGWVR1_DATA_ADDRESS_31_2_MASK                  0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGWVR1_NA_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWVR1_NA_MASK                                 0x00000003

#define CORTEXA7_DEBUG_DBGWVR2_OFFSET                                     0x00000188
#define CORTEXA7_DEBUG_DBGWVR2_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWVR2_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGWVR2_DATA_ADDRESS_31_2_SHIFT                 2
#define    CORTEXA7_DEBUG_DBGWVR2_DATA_ADDRESS_31_2_MASK                  0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGWVR2_NA_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWVR2_NA_MASK                                 0x00000003

#define CORTEXA7_DEBUG_DBGWVR3_OFFSET                                     0x0000018C
#define CORTEXA7_DEBUG_DBGWVR3_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWVR3_RESERVED_MASK                              0x00000000
#define    CORTEXA7_DEBUG_DBGWVR3_DATA_ADDRESS_31_2_SHIFT                 2
#define    CORTEXA7_DEBUG_DBGWVR3_DATA_ADDRESS_31_2_MASK                  0xFFFFFFFC
#define    CORTEXA7_DEBUG_DBGWVR3_NA_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWVR3_NA_MASK                                 0x00000003

#define CORTEXA7_DEBUG_DBGWCR0_OFFSET                                     0x000001C0
#define CORTEXA7_DEBUG_DBGWCR0_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWCR0_RESERVED_MASK                              0xE0E00000
#define    CORTEXA7_DEBUG_DBGWCR0_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGWCR0_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGWCR0_WT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGWCR0_WT_MASK                                 0x00100000
#define    CORTEXA7_DEBUG_DBGWCR0_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGWCR0_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGWCR0_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGWCR0_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGWCR0_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGWCR0_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGWCR0_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGWCR0_BAS_MASK                                0x00001FE0
#define    CORTEXA7_DEBUG_DBGWCR0_LSC_SHIFT                               3
#define    CORTEXA7_DEBUG_DBGWCR0_LSC_MASK                                0x00000018
#define    CORTEXA7_DEBUG_DBGWCR0_PAC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGWCR0_PAC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGWCR0_WE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWCR0_WE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGWCR1_OFFSET                                     0x000001C4
#define CORTEXA7_DEBUG_DBGWCR1_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWCR1_RESERVED_MASK                              0xE0E00000
#define    CORTEXA7_DEBUG_DBGWCR1_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGWCR1_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGWCR1_WT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGWCR1_WT_MASK                                 0x00100000
#define    CORTEXA7_DEBUG_DBGWCR1_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGWCR1_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGWCR1_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGWCR1_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGWCR1_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGWCR1_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGWCR1_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGWCR1_BAS_MASK                                0x00001FE0
#define    CORTEXA7_DEBUG_DBGWCR1_LSC_SHIFT                               3
#define    CORTEXA7_DEBUG_DBGWCR1_LSC_MASK                                0x00000018
#define    CORTEXA7_DEBUG_DBGWCR1_PAC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGWCR1_PAC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGWCR1_WE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWCR1_WE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGWCR2_OFFSET                                     0x000001C8
#define CORTEXA7_DEBUG_DBGWCR2_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWCR2_RESERVED_MASK                              0xE0E00000
#define    CORTEXA7_DEBUG_DBGWCR2_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGWCR2_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGWCR2_WT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGWCR2_WT_MASK                                 0x00100000
#define    CORTEXA7_DEBUG_DBGWCR2_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGWCR2_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGWCR2_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGWCR2_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGWCR2_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGWCR2_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGWCR2_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGWCR2_BAS_MASK                                0x00001FE0
#define    CORTEXA7_DEBUG_DBGWCR2_LSC_SHIFT                               3
#define    CORTEXA7_DEBUG_DBGWCR2_LSC_MASK                                0x00000018
#define    CORTEXA7_DEBUG_DBGWCR2_PAC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGWCR2_PAC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGWCR2_WE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWCR2_WE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGWCR3_OFFSET                                     0x000001CC
#define CORTEXA7_DEBUG_DBGWCR3_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGWCR3_RESERVED_MASK                              0xE0E00000
#define    CORTEXA7_DEBUG_DBGWCR3_MASK_SHIFT                              24
#define    CORTEXA7_DEBUG_DBGWCR3_MASK_MASK                               0x1F000000
#define    CORTEXA7_DEBUG_DBGWCR3_WT_SHIFT                                20
#define    CORTEXA7_DEBUG_DBGWCR3_WT_MASK                                 0x00100000
#define    CORTEXA7_DEBUG_DBGWCR3_LBN_SHIFT                               16
#define    CORTEXA7_DEBUG_DBGWCR3_LBN_MASK                                0x000F0000
#define    CORTEXA7_DEBUG_DBGWCR3_SSC_SHIFT                               14
#define    CORTEXA7_DEBUG_DBGWCR3_SSC_MASK                                0x0000C000
#define    CORTEXA7_DEBUG_DBGWCR3_HMC_SHIFT                               13
#define    CORTEXA7_DEBUG_DBGWCR3_HMC_MASK                                0x00002000
#define    CORTEXA7_DEBUG_DBGWCR3_BAS_SHIFT                               5
#define    CORTEXA7_DEBUG_DBGWCR3_BAS_MASK                                0x00001FE0
#define    CORTEXA7_DEBUG_DBGWCR3_LSC_SHIFT                               3
#define    CORTEXA7_DEBUG_DBGWCR3_LSC_MASK                                0x00000018
#define    CORTEXA7_DEBUG_DBGWCR3_PAC_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGWCR3_PAC_MASK                                0x00000006
#define    CORTEXA7_DEBUG_DBGWCR3_WE_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGWCR3_WE_MASK                                 0x00000001

#define CORTEXA7_DEBUG_DBGDRAR_OFFSET                                     0x00000200
#define CORTEXA7_DEBUG_DBGDRAR_TYPE                                       UInt64
#define CORTEXA7_DEBUG_DBGDRAR_RESERVED_MASK                              0xFFFFFF0000000FFC
#define    CORTEXA7_DEBUG_DBGDRAR_ROMADDR_39_32_SHIFT                     32
#define    CORTEXA7_DEBUG_DBGDRAR_ROMADDR_39_32_MASK                      0xFF00000000
#define    CORTEXA7_DEBUG_DBGDRAR_ROMADDR_31_12_SHIFT                     12
#define    CORTEXA7_DEBUG_DBGDRAR_ROMADDR_31_12_MASK                      0xFFFFF000
#define    CORTEXA7_DEBUG_DBGDRAR_VALID_SHIFT                             0
#define    CORTEXA7_DEBUG_DBGDRAR_VALID_MASK                              0x00000003

#define CORTEXA7_DEBUG_DBGBXVR0_OFFSET                                    0x00000250
#define CORTEXA7_DEBUG_DBGBXVR0_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGBXVR0_RESERVED_MASK                             0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGBXVR0_VMID_SHIFT                             0
#define    CORTEXA7_DEBUG_DBGBXVR0_VMID_MASK                              0x000000FF

#define CORTEXA7_DEBUG_DBGBXVR1_OFFSET                                    0x00000254
#define CORTEXA7_DEBUG_DBGBXVR1_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGBXVR1_RESERVED_MASK                             0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGBXVR1_VMID_SHIFT                             0
#define    CORTEXA7_DEBUG_DBGBXVR1_VMID_MASK                              0x000000FF

#define CORTEXA7_DEBUG_DBGOSLAR_OFFSET                                    0x00000300
#define CORTEXA7_DEBUG_DBGOSLAR_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGOSLAR_RESERVED_MASK                             0x00000000
#define    CORTEXA7_DEBUG_DBGOSLAR_OSLAR_SHIFT                            0
#define    CORTEXA7_DEBUG_DBGOSLAR_OSLAR_MASK                             0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGOSLSR_OFFSET                                    0x00000304
#define CORTEXA7_DEBUG_DBGOSLSR_TYPE                                      UInt32
#define CORTEXA7_DEBUG_DBGOSLSR_RESERVED_MASK                             0xFFFFFFF0
#define    CORTEXA7_DEBUG_DBGOSLSR_OSLM_1_SHIFT                           3
#define    CORTEXA7_DEBUG_DBGOSLSR_OSLM_1_MASK                            0x00000008
#define    CORTEXA7_DEBUG_DBGOSLSR_NTT_SHIFT                              2
#define    CORTEXA7_DEBUG_DBGOSLSR_NTT_MASK                               0x00000004
#define    CORTEXA7_DEBUG_DBGOSLSR_OSLK_SHIFT                             1
#define    CORTEXA7_DEBUG_DBGOSLSR_OSLK_MASK                              0x00000002
#define    CORTEXA7_DEBUG_DBGOSLSR_OSLM_0_SHIFT                           0
#define    CORTEXA7_DEBUG_DBGOSLSR_OSLM_0_MASK                            0x00000001

#define CORTEXA7_DEBUG_DBGPRCR_OFFSET                                     0x00000310
#define CORTEXA7_DEBUG_DBGPRCR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPRCR_RESERVED_MASK                              0xFFFFFFF0
#define    CORTEXA7_DEBUG_DBGPRCR_COREPURQ_SHIFT                          3
#define    CORTEXA7_DEBUG_DBGPRCR_COREPURQ_MASK                           0x00000008
#define    CORTEXA7_DEBUG_DBGPRCR_HCWR_SHIFT                              2
#define    CORTEXA7_DEBUG_DBGPRCR_HCWR_MASK                               0x00000004
#define    CORTEXA7_DEBUG_DBGPRCR_CWRR_SHIFT                              1
#define    CORTEXA7_DEBUG_DBGPRCR_CWRR_MASK                               0x00000002
#define    CORTEXA7_DEBUG_DBGPRCR_CORENPDRQ_SHIFT                         0
#define    CORTEXA7_DEBUG_DBGPRCR_CORENPDRQ_MASK                          0x00000001

#define CORTEXA7_DEBUG_DBGPRSR_OFFSET                                     0x00000314
#define CORTEXA7_DEBUG_DBGPRSR_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPRSR_RESERVED_MASK                              0xFFFFFF80
#define    CORTEXA7_DEBUG_DBGPRSR_DLK_SHIFT                               6
#define    CORTEXA7_DEBUG_DBGPRSR_DLK_MASK                                0x00000040
#define    CORTEXA7_DEBUG_DBGPRSR_OSLK_SHIFT                              5
#define    CORTEXA7_DEBUG_DBGPRSR_OSLK_MASK                               0x00000020
#define    CORTEXA7_DEBUG_DBGPRSR_HALTED_SHIFT                            4
#define    CORTEXA7_DEBUG_DBGPRSR_HALTED_MASK                             0x00000010
#define    CORTEXA7_DEBUG_DBGPRSR_SR_SHIFT                                3
#define    CORTEXA7_DEBUG_DBGPRSR_SR_MASK                                 0x00000008
#define    CORTEXA7_DEBUG_DBGPRSR_R_SHIFT                                 2
#define    CORTEXA7_DEBUG_DBGPRSR_R_MASK                                  0x00000004
#define    CORTEXA7_DEBUG_DBGPRSR_SPD_SHIFT                               1
#define    CORTEXA7_DEBUG_DBGPRSR_SPD_MASK                                0x00000002
#define    CORTEXA7_DEBUG_DBGPRSR_PU_SHIFT                                0
#define    CORTEXA7_DEBUG_DBGPRSR_PU_MASK                                 0x00000001

#define CORTEXA7_DEBUG_PIDR_OFFSET                                        0x00000D00
#define CORTEXA7_DEBUG_PIDR_TYPE                                          UInt32
#define CORTEXA7_DEBUG_PIDR_RESERVED_MASK                                 0x00000000
#define    CORTEXA7_DEBUG_PIDR_IMPLEMENTER_SHIFT                          24
#define    CORTEXA7_DEBUG_PIDR_IMPLEMENTER_MASK                           0xFF000000
#define    CORTEXA7_DEBUG_PIDR_VARIANT_SHIFT                              20
#define    CORTEXA7_DEBUG_PIDR_VARIANT_MASK                               0x00F00000
#define    CORTEXA7_DEBUG_PIDR_ARCHITECTURE_SHIFT                         16
#define    CORTEXA7_DEBUG_PIDR_ARCHITECTURE_MASK                          0x000F0000
#define    CORTEXA7_DEBUG_PIDR_PRIMARYPARTNUMBER_SHIFT                    4
#define    CORTEXA7_DEBUG_PIDR_PRIMARYPARTNUMBER_MASK                     0x0000FFF0
#define    CORTEXA7_DEBUG_PIDR_REVISION_SHIFT                             0
#define    CORTEXA7_DEBUG_PIDR_REVISION_MASK                              0x0000000F

#define CORTEXA7_DEBUG_DBGITMISCOUNT_OFFSET                               0x00000EF8
#define CORTEXA7_DEBUG_DBGITMISCOUNT_TYPE                                 UInt32
#define CORTEXA7_DEBUG_DBGITMISCOUNT_RESERVED_MASK                        0xFFFFFDEE
#define    CORTEXA7_DEBUG_DBGITMISCOUNT_DBGRESTARTED_SHIFT                9
#define    CORTEXA7_DEBUG_DBGITMISCOUNT_DBGRESTARTED_MASK                 0x00000200
#define    CORTEXA7_DEBUG_DBGITMISCOUNT_PMUIRQ_SHIFT                      4
#define    CORTEXA7_DEBUG_DBGITMISCOUNT_PMUIRQ_MASK                       0x00000010
#define    CORTEXA7_DEBUG_DBGITMISCOUNT_DBGACK_SHIFT                      0
#define    CORTEXA7_DEBUG_DBGITMISCOUNT_DBGACK_MASK                       0x00000001

#define CORTEXA7_DEBUG_DBGITMISCIN_OFFSET                                 0x00000EFC
#define CORTEXA7_DEBUG_DBGITMISCIN_TYPE                                   UInt32
#define CORTEXA7_DEBUG_DBGITMISCIN_RESERVED_MASK                          0xFFFFFFF8
#define    CORTEXA7_DEBUG_DBGITMISCIN_NFIQ_SHIFT                          2
#define    CORTEXA7_DEBUG_DBGITMISCIN_NFIQ_MASK                           0x00000004
#define    CORTEXA7_DEBUG_DBGITMISCIN_NIRQ_SHIFT                          1
#define    CORTEXA7_DEBUG_DBGITMISCIN_NIRQ_MASK                           0x00000002
#define    CORTEXA7_DEBUG_DBGITMISCIN_EDBGRQ_SHIFT                        0
#define    CORTEXA7_DEBUG_DBGITMISCIN_EDBGRQ_MASK                         0x00000001

#define CORTEXA7_DEBUG_DBGITCTRL_OFFSET                                   0x00000F00
#define CORTEXA7_DEBUG_DBGITCTRL_TYPE                                     UInt32
#define CORTEXA7_DEBUG_DBGITCTRL_RESERVED_MASK                            0xFFFFFFFE
#define    CORTEXA7_DEBUG_DBGITCTRL_INTEGRATION_MODE_ENABLE_SHIFT         0
#define    CORTEXA7_DEBUG_DBGITCTRL_INTEGRATION_MODE_ENABLE_MASK          0x00000001

#define CORTEXA7_DEBUG_DBGCLAIMSET_OFFSET                                 0x00000FA0
#define CORTEXA7_DEBUG_DBGCLAIMSET_TYPE                                   UInt32
#define CORTEXA7_DEBUG_DBGCLAIMSET_RESERVED_MASK                          0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGCLAIMSET_CLAIM_SHIFT                         0
#define    CORTEXA7_DEBUG_DBGCLAIMSET_CLAIM_MASK                          0x000000FF

#define CORTEXA7_DEBUG_DBGCLAIMCLR_OFFSET                                 0x00000FA4
#define CORTEXA7_DEBUG_DBGCLAIMCLR_TYPE                                   UInt32
#define CORTEXA7_DEBUG_DBGCLAIMCLR_RESERVED_MASK                          0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGCLAIMCLR_CLAIM_SHIFT                         0
#define    CORTEXA7_DEBUG_DBGCLAIMCLR_CLAIM_MASK                          0x000000FF

#define CORTEXA7_DEBUG_DBGLAR_OFFSET                                      0x00000FB0
#define CORTEXA7_DEBUG_DBGLAR_TYPE                                        UInt32
#define CORTEXA7_DEBUG_DBGLAR_RESERVED_MASK                               0x00000000
#define    CORTEXA7_DEBUG_DBGLAR_LOCKACCESSCONTROL_SHIFT                  0
#define    CORTEXA7_DEBUG_DBGLAR_LOCKACCESSCONTROL_MASK                   0xFFFFFFFF

#define CORTEXA7_DEBUG_DBGLSR_OFFSET                                      0x00000FB4
#define CORTEXA7_DEBUG_DBGLSR_TYPE                                        UInt32
#define CORTEXA7_DEBUG_DBGLSR_RESERVED_MASK                               0xFFFFFFF8
#define    CORTEXA7_DEBUG_DBGLSR_X32BITACCESS_SHIFT                       2
#define    CORTEXA7_DEBUG_DBGLSR_X32BITACCESS_MASK                        0x00000004
#define    CORTEXA7_DEBUG_DBGLSR_LOCKED_SHIFT                             1
#define    CORTEXA7_DEBUG_DBGLSR_LOCKED_MASK                              0x00000002
#define    CORTEXA7_DEBUG_DBGLSR_LOCKIMPLEMENTED_SHIFT                    0
#define    CORTEXA7_DEBUG_DBGLSR_LOCKIMPLEMENTED_MASK                     0x00000001

#define CORTEXA7_DEBUG_DBGAUTHSTATUS_OFFSET                               0x00000FB8
#define CORTEXA7_DEBUG_DBGAUTHSTATUS_TYPE                                 UInt32
#define CORTEXA7_DEBUG_DBGAUTHSTATUS_RESERVED_MASK                        0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECNONINVDBGFEATURESIMPL_SHIFT    7
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECNONINVDBGFEATURESIMPL_MASK     0x00000080
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECNONINVDBGEN_SHIFT              6
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECNONINVDBGEN_MASK               0x00000040
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECINVDBGFEATURESIMPL_SHIFT       5
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECINVDBGFEATURESIMPL_MASK        0x00000020
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECINVDBGEN_SHIFT                 4
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_SECINVDBGEN_MASK                  0x00000010
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECNONINVDBGFEATURSIMPL_SHIFT  3
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECNONINVDBGFEATURSIMPL_MASK   0x00000008
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECNONINVDBGEN_SHIFT           2
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECNONINVDBGEN_MASK            0x00000004
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECINVDBGFEATURESIMPL_SHIFT    1
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECINVDBGFEATURESIMPL_MASK     0x00000002
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECINVDBGEN_SHIFT              0
#define    CORTEXA7_DEBUG_DBGAUTHSTATUS_NONSECINVDBGEN_MASK               0x00000001

#define CORTEXA7_DEBUG_DBGDEVID2_OFFSET                                   0x00000FC0
#define CORTEXA7_DEBUG_DBGDEVID2_TYPE                                     UInt32
#define CORTEXA7_DEBUG_DBGDEVID2_RESERVED_MASK                            0xF0000000
#define    CORTEXA7_DEBUG_DBGDEVID2_AUXREGS_SHIFT                         24
#define    CORTEXA7_DEBUG_DBGDEVID2_AUXREGS_MASK                          0x0F000000
#define    CORTEXA7_DEBUG_DBGDEVID2_DOUBLELOCK_SHIFT                      20
#define    CORTEXA7_DEBUG_DBGDEVID2_DOUBLELOCK_MASK                       0x00F00000
#define    CORTEXA7_DEBUG_DBGDEVID2_VIRTEXTNS_SHIFT                       16
#define    CORTEXA7_DEBUG_DBGDEVID2_VIRTEXTNS_MASK                        0x000F0000
#define    CORTEXA7_DEBUG_DBGDEVID2_VECTORCATCH_SHIFT                     12
#define    CORTEXA7_DEBUG_DBGDEVID2_VECTORCATCH_MASK                      0x0000F000
#define    CORTEXA7_DEBUG_DBGDEVID2_BPADDRMASK_SHIFT                      8
#define    CORTEXA7_DEBUG_DBGDEVID2_BPADDRMASK_MASK                       0x00000F00
#define    CORTEXA7_DEBUG_DBGDEVID2_WPADDRMASK_SHIFT                      4
#define    CORTEXA7_DEBUG_DBGDEVID2_WPADDRMASK_MASK                       0x000000F0
#define    CORTEXA7_DEBUG_DBGDEVID2_PCSAMPLE_SHIFT                        0
#define    CORTEXA7_DEBUG_DBGDEVID2_PCSAMPLE_MASK                         0x0000000F

#define CORTEXA7_DEBUG_DBGDEVID1_OFFSET                                   0x00000FC4
#define CORTEXA7_DEBUG_DBGDEVID1_TYPE                                     UInt32
#define CORTEXA7_DEBUG_DBGDEVID1_RESERVED_MASK                            0xF0000000
#define    CORTEXA7_DEBUG_DBGDEVID1_AUXREGS_SHIFT                         24
#define    CORTEXA7_DEBUG_DBGDEVID1_AUXREGS_MASK                          0x0F000000
#define    CORTEXA7_DEBUG_DBGDEVID1_DOUBLELOCK_SHIFT                      20
#define    CORTEXA7_DEBUG_DBGDEVID1_DOUBLELOCK_MASK                       0x00F00000
#define    CORTEXA7_DEBUG_DBGDEVID1_VIRTEXTNS_SHIFT                       16
#define    CORTEXA7_DEBUG_DBGDEVID1_VIRTEXTNS_MASK                        0x000F0000
#define    CORTEXA7_DEBUG_DBGDEVID1_VECTORCATCH_SHIFT                     12
#define    CORTEXA7_DEBUG_DBGDEVID1_VECTORCATCH_MASK                      0x0000F000
#define    CORTEXA7_DEBUG_DBGDEVID1_BPADDRMASK_SHIFT                      8
#define    CORTEXA7_DEBUG_DBGDEVID1_BPADDRMASK_MASK                       0x00000F00
#define    CORTEXA7_DEBUG_DBGDEVID1_WPADDRMASK_SHIFT                      4
#define    CORTEXA7_DEBUG_DBGDEVID1_WPADDRMASK_MASK                       0x000000F0
#define    CORTEXA7_DEBUG_DBGDEVID1_PCSAMPLE_SHIFT                        0
#define    CORTEXA7_DEBUG_DBGDEVID1_PCSAMPLE_MASK                         0x0000000F

#define CORTEXA7_DEBUG_DBGDEVID0_OFFSET                                   0x00000FC8
#define CORTEXA7_DEBUG_DBGDEVID0_TYPE                                     UInt32
#define CORTEXA7_DEBUG_DBGDEVID0_RESERVED_MASK                            0xF0000000
#define    CORTEXA7_DEBUG_DBGDEVID0_AUXREGS_SHIFT                         24
#define    CORTEXA7_DEBUG_DBGDEVID0_AUXREGS_MASK                          0x0F000000
#define    CORTEXA7_DEBUG_DBGDEVID0_DOUBLELOCK_SHIFT                      20
#define    CORTEXA7_DEBUG_DBGDEVID0_DOUBLELOCK_MASK                       0x00F00000
#define    CORTEXA7_DEBUG_DBGDEVID0_VIRTEXTNS_SHIFT                       16
#define    CORTEXA7_DEBUG_DBGDEVID0_VIRTEXTNS_MASK                        0x000F0000
#define    CORTEXA7_DEBUG_DBGDEVID0_VECTORCATCH_SHIFT                     12
#define    CORTEXA7_DEBUG_DBGDEVID0_VECTORCATCH_MASK                      0x0000F000
#define    CORTEXA7_DEBUG_DBGDEVID0_BPADDRMASK_SHIFT                      8
#define    CORTEXA7_DEBUG_DBGDEVID0_BPADDRMASK_MASK                       0x00000F00
#define    CORTEXA7_DEBUG_DBGDEVID0_WPADDRMASK_SHIFT                      4
#define    CORTEXA7_DEBUG_DBGDEVID0_WPADDRMASK_MASK                       0x000000F0
#define    CORTEXA7_DEBUG_DBGDEVID0_PCSAMPLE_SHIFT                        0
#define    CORTEXA7_DEBUG_DBGDEVID0_PCSAMPLE_MASK                         0x0000000F

#define CORTEXA7_DEBUG_DBGDEVTYPE_OFFSET                                  0x00000FCC
#define CORTEXA7_DEBUG_DBGDEVTYPE_TYPE                                    UInt32
#define CORTEXA7_DEBUG_DBGDEVTYPE_RESERVED_MASK                           0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGDEVTYPE_SUBTYPE_SHIFT                        4
#define    CORTEXA7_DEBUG_DBGDEVTYPE_SUBTYPE_MASK                         0x000000F0
#define    CORTEXA7_DEBUG_DBGDEVTYPE_MAINCLASS_SHIFT                      0
#define    CORTEXA7_DEBUG_DBGDEVTYPE_MAINCLASS_MASK                       0x0000000F

#define CORTEXA7_DEBUG_DBGPID4_OFFSET                                     0x00000FD0
#define CORTEXA7_DEBUG_DBGPID4_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPID4_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGPID4_COUNT4KB_SHIFT                          4
#define    CORTEXA7_DEBUG_DBGPID4_COUNT4KB_MASK                           0x000000F0
#define    CORTEXA7_DEBUG_DBGPID4_JEP106CONTCODE_SHIFT                    0
#define    CORTEXA7_DEBUG_DBGPID4_JEP106CONTCODE_MASK                     0x0000000F

#define CORTEXA7_DEBUG_DBGPID0_OFFSET                                     0x00000FE0
#define CORTEXA7_DEBUG_DBGPID0_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPID0_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGPID0_COUNT4KB_SHIFT                          4
#define    CORTEXA7_DEBUG_DBGPID0_COUNT4KB_MASK                           0x000000F0
#define    CORTEXA7_DEBUG_DBGPID0_JEP106CONTCODE_SHIFT                    0
#define    CORTEXA7_DEBUG_DBGPID0_JEP106CONTCODE_MASK                     0x0000000F

#define CORTEXA7_DEBUG_DBGPID1_OFFSET                                     0x00000FE4
#define CORTEXA7_DEBUG_DBGPID1_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPID1_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGPID1_COUNT4KB_SHIFT                          4
#define    CORTEXA7_DEBUG_DBGPID1_COUNT4KB_MASK                           0x000000F0
#define    CORTEXA7_DEBUG_DBGPID1_JEP106CONTCODE_SHIFT                    0
#define    CORTEXA7_DEBUG_DBGPID1_JEP106CONTCODE_MASK                     0x0000000F

#define CORTEXA7_DEBUG_DBGPID2_OFFSET                                     0x00000FE8
#define CORTEXA7_DEBUG_DBGPID2_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPID2_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGPID2_COUNT4KB_SHIFT                          4
#define    CORTEXA7_DEBUG_DBGPID2_COUNT4KB_MASK                           0x000000F0
#define    CORTEXA7_DEBUG_DBGPID2_JEP106CONTCODE_SHIFT                    0
#define    CORTEXA7_DEBUG_DBGPID2_JEP106CONTCODE_MASK                     0x0000000F

#define CORTEXA7_DEBUG_DBGPID3_OFFSET                                     0x00000FEC
#define CORTEXA7_DEBUG_DBGPID3_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGPID3_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGPID3_COUNT4KB_SHIFT                          4
#define    CORTEXA7_DEBUG_DBGPID3_COUNT4KB_MASK                           0x000000F0
#define    CORTEXA7_DEBUG_DBGPID3_JEP106CONTCODE_SHIFT                    0
#define    CORTEXA7_DEBUG_DBGPID3_JEP106CONTCODE_MASK                     0x0000000F

#define CORTEXA7_DEBUG_DBGCID0_OFFSET                                     0x00000FF0
#define CORTEXA7_DEBUG_DBGCID0_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGCID0_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGCID0_PREAMBLE_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGCID0_PREAMBLE_MASK                           0x000000FF

#define CORTEXA7_DEBUG_DBGCID1_OFFSET                                     0x00000FF4
#define CORTEXA7_DEBUG_DBGCID1_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGCID1_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGCID1_COMPONENTCLASS_SHIFT                    4
#define    CORTEXA7_DEBUG_DBGCID1_COMPONENTCLASS_MASK                     0x000000F0
#define    CORTEXA7_DEBUG_DBGCID1_PREAMBLE_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGCID1_PREAMBLE_MASK                           0x0000000F

#define CORTEXA7_DEBUG_DBGCID2_OFFSET                                     0x00000FF8
#define CORTEXA7_DEBUG_DBGCID2_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGCID2_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGCID2_PREAMBLE_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGCID2_PREAMBLE_MASK                           0x000000FF

#define CORTEXA7_DEBUG_DBGCID3_OFFSET                                     0x00000FFC
#define CORTEXA7_DEBUG_DBGCID3_TYPE                                       UInt32
#define CORTEXA7_DEBUG_DBGCID3_RESERVED_MASK                              0xFFFFFF00
#define    CORTEXA7_DEBUG_DBGCID3_PREAMBLE_SHIFT                          0
#define    CORTEXA7_DEBUG_DBGCID3_PREAMBLE_MASK                           0x000000FF

#endif /* __BRCM_RDB_CORTEXA7_DEBUG_H__ */



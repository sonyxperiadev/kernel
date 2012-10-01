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

#ifndef __BRCM_RDB_V3D_H__
#define __BRCM_RDB_V3D_H__

#define V3D_IDENT0_OFFSET                                                 0x00000000
#define V3D_IDENT0_TYPE                                                   UInt32
#define V3D_IDENT0_RESERVED_MASK                                          0x00000000
#define    V3D_IDENT0_TVER_SHIFT                                          24
#define    V3D_IDENT0_TVER_MASK                                           0xFF000000
#define    V3D_IDENT0_IDSTR_SHIFT                                         0
#define    V3D_IDENT0_IDSTR_MASK                                          0x00FFFFFF

#define V3D_IDENT1_OFFSET                                                 0x00000004
#define V3D_IDENT1_TYPE                                                   UInt32
#define V3D_IDENT1_RESERVED_MASK                                          0x00000000
#define    V3D_IDENT1_VPMSZ_SHIFT                                         28
#define    V3D_IDENT1_VPMSZ_MASK                                          0xF0000000
#define    V3D_IDENT1_HDRT_SHIFT                                          24
#define    V3D_IDENT1_HDRT_MASK                                           0x0F000000
#define    V3D_IDENT1_NSEM_SHIFT                                          16
#define    V3D_IDENT1_NSEM_MASK                                           0x00FF0000
#define    V3D_IDENT1_TUPS_SHIFT                                          12
#define    V3D_IDENT1_TUPS_MASK                                           0x0000F000
#define    V3D_IDENT1_QUPS_SHIFT                                          8
#define    V3D_IDENT1_QUPS_MASK                                           0x00000F00
#define    V3D_IDENT1_NSLC_SHIFT                                          4
#define    V3D_IDENT1_NSLC_MASK                                           0x000000F0
#define    V3D_IDENT1_REV_SHIFT                                           0
#define    V3D_IDENT1_REV_MASK                                            0x0000000F

#define V3D_IDENT2_OFFSET                                                 0x00000008
#define V3D_IDENT2_TYPE                                                   UInt32
#define V3D_IDENT2_RESERVED_MASK                                          0xFF000000
#define    V3D_IDENT2_NOEARLYZ_SHIFT                                      23
#define    V3D_IDENT2_NOEARLYZ_MASK                                       0x00800000
#define    V3D_IDENT2_AXI_RW_REORDER_SHIFT                                22
#define    V3D_IDENT2_AXI_RW_REORDER_MASK                                 0x00400000
#define    V3D_IDENT2_ENDSWP_SHIFT                                        21
#define    V3D_IDENT2_ENDSWP_MASK                                         0x00200000
#define    V3D_IDENT2_BIGEND_SHIFT                                        20
#define    V3D_IDENT2_BIGEND_MASK                                         0x00100000
#define    V3D_IDENT2_QUCSZ_SHIFT                                         16
#define    V3D_IDENT2_QUCSZ_MASK                                          0x000F0000
#define    V3D_IDENT2_QICSZ_SHIFT                                         12
#define    V3D_IDENT2_QICSZ_MASK                                          0x0000F000
#define    V3D_IDENT2_TLBDB_SHIFT                                         8
#define    V3D_IDENT2_TLBDB_MASK                                          0x00000F00
#define    V3D_IDENT2_TLBSZ_SHIFT                                         4
#define    V3D_IDENT2_TLBSZ_MASK                                          0x000000F0
#define    V3D_IDENT2_VRISZ_SHIFT                                         0
#define    V3D_IDENT2_VRISZ_MASK                                          0x0000000F

#define V3D_IDENT3_OFFSET                                                 0x0000000C
#define V3D_IDENT3_TYPE                                                   UInt32
#define V3D_IDENT3_RESERVED_MASK                                          0xFFFF0000
#define    V3D_IDENT3_IPREV_SHIFT                                         8
#define    V3D_IDENT3_IPREV_MASK                                          0x0000FF00
#define    V3D_IDENT3_IPIDX_SHIFT                                         0
#define    V3D_IDENT3_IPIDX_MASK                                          0x000000FF

#define V3D_SCRATCH_OFFSET                                                0x00000010
#define V3D_SCRATCH_TYPE                                                  UInt32
#define V3D_SCRATCH_RESERVED_MASK                                         0x00000000
#define    V3D_SCRATCH_SCRATCH_SHIFT                                      0
#define    V3D_SCRATCH_SCRATCH_MASK                                       0xFFFFFFFF

#define V3D_L2CACTL_OFFSET                                                0x00000020
#define V3D_L2CACTL_TYPE                                                  UInt32
#define V3D_L2CACTL_RESERVED_MASK                                         0xFFFFFFF8
#define    V3D_L2CACTL_L2CCLR_SHIFT                                       2
#define    V3D_L2CACTL_L2CCLR_MASK                                        0x00000004
#define    V3D_L2CACTL_L2CDIS_SHIFT                                       1
#define    V3D_L2CACTL_L2CDIS_MASK                                        0x00000002
#define    V3D_L2CACTL_L2CENA_SHIFT                                       0
#define    V3D_L2CACTL_L2CENA_MASK                                        0x00000001

#define V3D_SLCACTL_OFFSET                                                0x00000024
#define V3D_SLCACTL_TYPE                                                  UInt32
#define V3D_SLCACTL_RESERVED_MASK                                         0xF0F0F0F0
#define    V3D_SLCACTL_T1CCS0_TO_T1CCS3_SHIFT                             24
#define    V3D_SLCACTL_T1CCS0_TO_T1CCS3_MASK                              0x0F000000
#define    V3D_SLCACTL_T0CCS0_TO_T0CCS3_SHIFT                             16
#define    V3D_SLCACTL_T0CCS0_TO_T0CCS3_MASK                              0x000F0000
#define    V3D_SLCACTL_UCCS0_TO_UCCS3_SHIFT                               8
#define    V3D_SLCACTL_UCCS0_TO_UCCS3_MASK                                0x00000F00
#define    V3D_SLCACTL_ICCS0_TO_ICCS3_SHIFT                               0
#define    V3D_SLCACTL_ICCS0_TO_ICCS3_MASK                                0x0000000F

#define V3D_INTCTL_OFFSET                                                 0x00000030
#define V3D_INTCTL_TYPE                                                   UInt32
#define V3D_INTCTL_RESERVED_MASK                                          0xFFFFFFF0
#define    V3D_INTCTL_INT_SPILLUSE_SHIFT                                  3
#define    V3D_INTCTL_INT_SPILLUSE_MASK                                   0x00000008
#define    V3D_INTCTL_INT_OUTOMEM_SHIFT                                   2
#define    V3D_INTCTL_INT_OUTOMEM_MASK                                    0x00000004
#define    V3D_INTCTL_INT_FLDONE_SHIFT                                    1
#define    V3D_INTCTL_INT_FLDONE_MASK                                     0x00000002
#define    V3D_INTCTL_INT_FRDONE_SHIFT                                    0
#define    V3D_INTCTL_INT_FRDONE_MASK                                     0x00000001

#define V3D_INTENA_OFFSET                                                 0x00000034
#define V3D_INTENA_TYPE                                                   UInt32
#define V3D_INTENA_RESERVED_MASK                                          0xFFFFFFF0
#define    V3D_INTENA_EI_SPILLUSE_SHIFT                                   3
#define    V3D_INTENA_EI_SPILLUSE_MASK                                    0x00000008
#define    V3D_INTENA_EI_OUTOMEM_SHIFT                                    2
#define    V3D_INTENA_EI_OUTOMEM_MASK                                     0x00000004
#define    V3D_INTENA_EI_FLDONE_SHIFT                                     1
#define    V3D_INTENA_EI_FLDONE_MASK                                      0x00000002
#define    V3D_INTENA_EI_FRDONE_SHIFT                                     0
#define    V3D_INTENA_EI_FRDONE_MASK                                      0x00000001

#define V3D_INTDIS_OFFSET                                                 0x00000038
#define V3D_INTDIS_TYPE                                                   UInt32
#define V3D_INTDIS_RESERVED_MASK                                          0xFFFFFFF0
#define    V3D_INTDIS_DI_SPILLUSE_SHIFT                                   3
#define    V3D_INTDIS_DI_SPILLUSE_MASK                                    0x00000008
#define    V3D_INTDIS_DI_OUTOMEM_SHIFT                                    2
#define    V3D_INTDIS_DI_OUTOMEM_MASK                                     0x00000004
#define    V3D_INTDIS_DI_FLDONE_SHIFT                                     1
#define    V3D_INTDIS_DI_FLDONE_MASK                                      0x00000002
#define    V3D_INTDIS_DI_FRDONE_SHIFT                                     0
#define    V3D_INTDIS_DI_FRDONE_MASK                                      0x00000001

#define V3D_ENDSWP_OFFSET                                                 0x00000040
#define V3D_ENDSWP_TYPE                                                   UInt32
#define V3D_ENDSWP_RESERVED_MASK                                          0xFFFFFFFE
#define    V3D_ENDSWP_ENDSWP_SHIFT                                        0
#define    V3D_ENDSWP_ENDSWP_MASK                                         0x00000001

#define V3D_CT0CS_OFFSET                                                  0x00000100
#define V3D_CT0CS_TYPE                                                    UInt32
#define V3D_CT0CS_RESERVED_MASK                                           0xFFFF0CC6
#define    V3D_CT0CS_CTRSTA_SHIFT                                         15
#define    V3D_CT0CS_CTRSTA_MASK                                          0x00008000
#define    V3D_CT0CS_CTSEMA_SHIFT                                         12
#define    V3D_CT0CS_CTSEMA_MASK                                          0x00007000
#define    V3D_CT0CS_CTRTSD_SHIFT                                         8
#define    V3D_CT0CS_CTRTSD_MASK                                          0x00000300
#define    V3D_CT0CS_CTRUN_SHIFT                                          5
#define    V3D_CT0CS_CTRUN_MASK                                           0x00000020
#define    V3D_CT0CS_CTSUBS_SHIFT                                         4
#define    V3D_CT0CS_CTSUBS_MASK                                          0x00000010
#define    V3D_CT0CS_CTERR_SHIFT                                          3
#define    V3D_CT0CS_CTERR_MASK                                           0x00000008
#define    V3D_CT0CS_CTMODE_SHIFT                                         0
#define    V3D_CT0CS_CTMODE_MASK                                          0x00000001

#define V3D_CT1CS_OFFSET                                                  0x00000104
#define V3D_CT1CS_TYPE                                                    UInt32
#define V3D_CT1CS_RESERVED_MASK                                           0xFFFF0CC6
#define    V3D_CT1CS_CTRSTA_SHIFT                                         15
#define    V3D_CT1CS_CTRSTA_MASK                                          0x00008000
#define    V3D_CT1CS_CTSEMA_SHIFT                                         12
#define    V3D_CT1CS_CTSEMA_MASK                                          0x00007000
#define    V3D_CT1CS_CTRTSD_SHIFT                                         8
#define    V3D_CT1CS_CTRTSD_MASK                                          0x00000300
#define    V3D_CT1CS_CTRUN_SHIFT                                          5
#define    V3D_CT1CS_CTRUN_MASK                                           0x00000020
#define    V3D_CT1CS_CTSUBS_SHIFT                                         4
#define    V3D_CT1CS_CTSUBS_MASK                                          0x00000010
#define    V3D_CT1CS_CTERR_SHIFT                                          3
#define    V3D_CT1CS_CTERR_MASK                                           0x00000008
#define    V3D_CT1CS_CTMODE_SHIFT                                         0
#define    V3D_CT1CS_CTMODE_MASK                                          0x00000001

#define V3D_CT0EA_OFFSET                                                  0x00000108
#define V3D_CT0EA_TYPE                                                    UInt32
#define V3D_CT0EA_RESERVED_MASK                                           0x00000000
#define    V3D_CT0EA_CTLEA_SHIFT                                          0
#define    V3D_CT0EA_CTLEA_MASK                                           0xFFFFFFFF

#define V3D_CT1EA_OFFSET                                                  0x0000010C
#define V3D_CT1EA_TYPE                                                    UInt32
#define V3D_CT1EA_RESERVED_MASK                                           0x00000000
#define    V3D_CT1EA_CTLEA_SHIFT                                          0
#define    V3D_CT1EA_CTLEA_MASK                                           0xFFFFFFFF

#define V3D_CT0CA_OFFSET                                                  0x00000110
#define V3D_CT0CA_TYPE                                                    UInt32
#define V3D_CT0CA_RESERVED_MASK                                           0x00000000
#define    V3D_CT0CA_CTLCA_SHIFT                                          0
#define    V3D_CT0CA_CTLCA_MASK                                           0xFFFFFFFF

#define V3D_CT1CA_OFFSET                                                  0x00000114
#define V3D_CT1CA_TYPE                                                    UInt32
#define V3D_CT1CA_RESERVED_MASK                                           0x00000000
#define    V3D_CT1CA_CTLCA_SHIFT                                          0
#define    V3D_CT1CA_CTLCA_MASK                                           0xFFFFFFFF

#define V3D_CT00RA0_OFFSET                                                0x00000118
#define V3D_CT00RA0_TYPE                                                  UInt32
#define V3D_CT00RA0_RESERVED_MASK                                         0x00000000
#define    V3D_CT00RA0_CTLRA_SHIFT                                        0
#define    V3D_CT00RA0_CTLRA_MASK                                         0xFFFFFFFF

#define V3D_CT01RA0_OFFSET                                                0x0000011C
#define V3D_CT01RA0_TYPE                                                  UInt32
#define V3D_CT01RA0_RESERVED_MASK                                         0x00000000
#define    V3D_CT01RA0_CTLRA_SHIFT                                        0
#define    V3D_CT01RA0_CTLRA_MASK                                         0xFFFFFFFF

#define V3D_CT0LC_OFFSET                                                  0x00000120
#define V3D_CT0LC_TYPE                                                    UInt32
#define V3D_CT0LC_RESERVED_MASK                                           0x00000000
#define    V3D_CT0LC_CTLLCM_SHIFT                                         16
#define    V3D_CT0LC_CTLLCM_MASK                                          0xFFFF0000
#define    V3D_CT0LC_CTLSLCS_SHIFT                                        0
#define    V3D_CT0LC_CTLSLCS_MASK                                         0x0000FFFF

#define V3D_CT1LC_OFFSET                                                  0x00000124
#define V3D_CT1LC_TYPE                                                    UInt32
#define V3D_CT1LC_RESERVED_MASK                                           0x00000000
#define    V3D_CT1LC_CTLLCM_SHIFT                                         16
#define    V3D_CT1LC_CTLLCM_MASK                                          0xFFFF0000
#define    V3D_CT1LC_CTLSLCS_SHIFT                                        0
#define    V3D_CT1LC_CTLSLCS_MASK                                         0x0000FFFF

#define V3D_CT0PC_OFFSET                                                  0x00000128
#define V3D_CT0PC_TYPE                                                    UInt32
#define V3D_CT0PC_RESERVED_MASK                                           0x00000000
#define    V3D_CT0PC_CTLPC_SHIFT                                          0
#define    V3D_CT0PC_CTLPC_MASK                                           0xFFFFFFFF

#define V3D_CT1PC_OFFSET                                                  0x0000012C
#define V3D_CT1PC_TYPE                                                    UInt32
#define V3D_CT1PC_RESERVED_MASK                                           0x00000000
#define    V3D_CT1PC_CTLPC_SHIFT                                          0
#define    V3D_CT1PC_CTLPC_MASK                                           0xFFFFFFFF

#define V3D_PCS_OFFSET                                                    0x00000130
#define V3D_PCS_TYPE                                                      UInt32
#define V3D_PCS_RESERVED_MASK                                             0xFFFFFEF0
#define    V3D_PCS_BMOOM_SHIFT                                            8
#define    V3D_PCS_BMOOM_MASK                                             0x00000100
#define    V3D_PCS_RMBUSY_SHIFT                                           3
#define    V3D_PCS_RMBUSY_MASK                                            0x00000008
#define    V3D_PCS_RMACTIVE_SHIFT                                         2
#define    V3D_PCS_RMACTIVE_MASK                                          0x00000004
#define    V3D_PCS_BMBUSY_SHIFT                                           1
#define    V3D_PCS_BMBUSY_MASK                                            0x00000002
#define    V3D_PCS_BMACTIVE_SHIFT                                         0
#define    V3D_PCS_BMACTIVE_MASK                                          0x00000001

#define V3D_BFC_OFFSET                                                    0x00000134
#define V3D_BFC_TYPE                                                      UInt32
#define V3D_BFC_RESERVED_MASK                                             0xFFFFFF00
#define    V3D_BFC_BMFCT_SHIFT                                            0
#define    V3D_BFC_BMFCT_MASK                                             0x000000FF

#define V3D_RFC_OFFSET                                                    0x00000138
#define V3D_RFC_TYPE                                                      UInt32
#define V3D_RFC_RESERVED_MASK                                             0xFFFFFF00
#define    V3D_RFC_RMFCT_SHIFT                                            0
#define    V3D_RFC_RMFCT_MASK                                             0x000000FF

#define V3D_BPCA_OFFSET                                                   0x00000300
#define V3D_BPCA_TYPE                                                     UInt32
#define V3D_BPCA_RESERVED_MASK                                            0x00000000
#define    V3D_BPCA_BMPCA_SHIFT                                           0
#define    V3D_BPCA_BMPCA_MASK                                            0xFFFFFFFF

#define V3D_BPCS_OFFSET                                                   0x00000304
#define V3D_BPCS_TYPE                                                     UInt32
#define V3D_BPCS_RESERVED_MASK                                            0x00000000
#define    V3D_BPCS_BMPRS_SHIFT                                           0
#define    V3D_BPCS_BMPRS_MASK                                            0xFFFFFFFF

#define V3D_BPOA_OFFSET                                                   0x00000308
#define V3D_BPOA_TYPE                                                     UInt32
#define V3D_BPOA_RESERVED_MASK                                            0x00000000
#define    V3D_BPOA_BMPOA_SHIFT                                           0
#define    V3D_BPOA_BMPOA_MASK                                            0xFFFFFFFF

#define V3D_BPOS_OFFSET                                                   0x0000030C
#define V3D_BPOS_TYPE                                                     UInt32
#define V3D_BPOS_RESERVED_MASK                                            0x00000000
#define    V3D_BPOS_BMPOS_SHIFT                                           0
#define    V3D_BPOS_BMPOS_MASK                                            0xFFFFFFFF

#define V3D_BXCF_OFFSET                                                   0x00000310
#define V3D_BXCF_TYPE                                                     UInt32
#define V3D_BXCF_RESERVED_MASK                                            0xFFFFFFFC
#define    V3D_BXCF_CLIPDISA_SHIFT                                        1
#define    V3D_BXCF_CLIPDISA_MASK                                         0x00000002
#define    V3D_BXCF_FWDDISA_SHIFT                                         0
#define    V3D_BXCF_FWDDISA_MASK                                          0x00000001

#define V3D_SQRSV0_OFFSET                                                 0x00000410
#define V3D_SQRSV0_TYPE                                                   UInt32
#define V3D_SQRSV0_RESERVED_MASK                                          0x00000000
#define    V3D_SQRSV0_QPURSV7_SHIFT                                       28
#define    V3D_SQRSV0_QPURSV7_MASK                                        0xF0000000
#define    V3D_SQRSV0_QPURSV6_SHIFT                                       24
#define    V3D_SQRSV0_QPURSV6_MASK                                        0x0F000000
#define    V3D_SQRSV0_QPURSV5_SHIFT                                       20
#define    V3D_SQRSV0_QPURSV5_MASK                                        0x00F00000
#define    V3D_SQRSV0_QPURSV4_SHIFT                                       16
#define    V3D_SQRSV0_QPURSV4_MASK                                        0x000F0000
#define    V3D_SQRSV0_QPURSV3_SHIFT                                       12
#define    V3D_SQRSV0_QPURSV3_MASK                                        0x0000F000
#define    V3D_SQRSV0_QPURSV2_SHIFT                                       8
#define    V3D_SQRSV0_QPURSV2_MASK                                        0x00000F00
#define    V3D_SQRSV0_QPURSV1_SHIFT                                       4
#define    V3D_SQRSV0_QPURSV1_MASK                                        0x000000F0
#define    V3D_SQRSV0_QPURSV0_SHIFT                                       0
#define    V3D_SQRSV0_QPURSV0_MASK                                        0x0000000F

#define V3D_SQRSV1_OFFSET                                                 0x00000414
#define V3D_SQRSV1_TYPE                                                   UInt32
#define V3D_SQRSV1_RESERVED_MASK                                          0x00000000
#define    V3D_SQRSV1_QPURSV15_SHIFT                                      28
#define    V3D_SQRSV1_QPURSV15_MASK                                       0xF0000000
#define    V3D_SQRSV1_QPURSV14_SHIFT                                      24
#define    V3D_SQRSV1_QPURSV14_MASK                                       0x0F000000
#define    V3D_SQRSV1_QPURSV13_SHIFT                                      20
#define    V3D_SQRSV1_QPURSV13_MASK                                       0x00F00000
#define    V3D_SQRSV1_QPURSV12_SHIFT                                      16
#define    V3D_SQRSV1_QPURSV12_MASK                                       0x000F0000
#define    V3D_SQRSV1_QPURSV11_SHIFT                                      12
#define    V3D_SQRSV1_QPURSV11_MASK                                       0x0000F000
#define    V3D_SQRSV1_QPURSV10_SHIFT                                      8
#define    V3D_SQRSV1_QPURSV10_MASK                                       0x00000F00
#define    V3D_SQRSV1_QPURSV9_SHIFT                                       4
#define    V3D_SQRSV1_QPURSV9_MASK                                        0x000000F0
#define    V3D_SQRSV1_QPURSV8_SHIFT                                       0
#define    V3D_SQRSV1_QPURSV8_MASK                                        0x0000000F

#define V3D_SQCNTL_OFFSET                                                 0x00000418
#define V3D_SQCNTL_TYPE                                                   UInt32
#define V3D_SQCNTL_RESERVED_MASK                                          0xFFFFFFF0
#define    V3D_SQCNTL_CSRBL_SHIFT                                         2
#define    V3D_SQCNTL_CSRBL_MASK                                          0x0000000C
#define    V3D_SQCNTL_VSRBL_SHIFT                                         0
#define    V3D_SQCNTL_VSRBL_MASK                                          0x00000003

#define V3D_SRQPC_OFFSET                                                  0x00000430
#define V3D_SRQPC_TYPE                                                    UInt32
#define V3D_SRQPC_RESERVED_MASK                                           0x00000000
#define    V3D_SRQPC_QPURQPC_SHIFT                                        0
#define    V3D_SRQPC_QPURQPC_MASK                                         0xFFFFFFFF

#define V3D_SRQUA_OFFSET                                                  0x00000434
#define V3D_SRQUA_TYPE                                                    UInt32
#define V3D_SRQUA_RESERVED_MASK                                           0x00000000
#define    V3D_SRQUA_QPURQUA_SHIFT                                        0
#define    V3D_SRQUA_QPURQUA_MASK                                         0xFFFFFFFF

#define V3D_SRQUL_OFFSET                                                  0x00000438
#define V3D_SRQUL_TYPE                                                    UInt32
#define V3D_SRQUL_RESERVED_MASK                                           0xFFFFF000
#define    V3D_SRQUL_QPURQUL_SHIFT                                        0
#define    V3D_SRQUL_QPURQUL_MASK                                         0x00000FFF

#define V3D_SRQCS_OFFSET                                                  0x0000043C
#define V3D_SRQCS_TYPE                                                    UInt32
#define V3D_SRQCS_RESERVED_MASK                                           0xFF000040
#define    V3D_SRQCS_QPURQCC_SHIFT                                        16
#define    V3D_SRQCS_QPURQCC_MASK                                         0x00FF0000
#define    V3D_SRQCS_QPURQCM_SHIFT                                        8
#define    V3D_SRQCS_QPURQCM_MASK                                         0x0000FF00
#define    V3D_SRQCS_QPURQERR_SHIFT                                       7
#define    V3D_SRQCS_QPURQERR_MASK                                        0x00000080
#define    V3D_SRQCS_QPURQL_SHIFT                                         0
#define    V3D_SRQCS_QPURQL_MASK                                          0x0000003F

#define V3D_VPACNTL_OFFSET                                                0x00000500
#define V3D_VPACNTL_TYPE                                                  UInt32
#define V3D_VPACNTL_RESERVED_MASK                                         0xFFFFC000
#define    V3D_VPACNTL_VPATOEN_SHIFT                                      13
#define    V3D_VPACNTL_VPATOEN_MASK                                       0x00002000
#define    V3D_VPACNTL_VPALIMEN_SHIFT                                     12
#define    V3D_VPACNTL_VPALIMEN_MASK                                      0x00001000
#define    V3D_VPACNTL_VPABATO_SHIFT                                      9
#define    V3D_VPACNTL_VPABATO_MASK                                       0x00000E00
#define    V3D_VPACNTL_VPARATO_SHIFT                                      6
#define    V3D_VPACNTL_VPARATO_MASK                                       0x000001C0
#define    V3D_VPACNTL_VPABALIM_SHIFT                                     3
#define    V3D_VPACNTL_VPABALIM_MASK                                      0x00000038
#define    V3D_VPACNTL_VPARALIM_SHIFT                                     0
#define    V3D_VPACNTL_VPARALIM_MASK                                      0x00000007

#define V3D_VPMBASE_OFFSET                                                0x00000504
#define V3D_VPMBASE_TYPE                                                  UInt32
#define V3D_VPMBASE_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_VPMBASE_VPMURSV_SHIFT                                      0
#define    V3D_VPMBASE_VPMURSV_MASK                                       0x0000001F

#define V3D_PCTRC_OFFSET                                                  0x00000670
#define V3D_PCTRC_TYPE                                                    UInt32
#define V3D_PCTRC_RESERVED_MASK                                           0xFFFF0000
#define    V3D_PCTRC_CTCLR0_CTCLR15_SHIFT                                 0
#define    V3D_PCTRC_CTCLR0_CTCLR15_MASK                                  0x0000FFFF

#define V3D_PCTRE_OFFSET                                                  0x00000674
#define V3D_PCTRE_TYPE                                                    UInt32
#define V3D_PCTRE_RESERVED_MASK                                           0xFFFF0000
#define    V3D_PCTRE_CTEN0_CTEN15_SHIFT                                   0
#define    V3D_PCTRE_CTEN0_CTEN15_MASK                                    0x0000FFFF

#define V3D_PCTR0_OFFSET                                                  0x00000680
#define V3D_PCTR0_TYPE                                                    UInt32
#define V3D_PCTR0_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR0_PCTR_SHIFT                                           0
#define    V3D_PCTR0_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS0_OFFSET                                                 0x00000684
#define V3D_PCTRS0_TYPE                                                   UInt32
#define V3D_PCTRS0_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS0_PCTRS_SHIFT                                         0
#define    V3D_PCTRS0_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR1_OFFSET                                                  0x00000688
#define V3D_PCTR1_TYPE                                                    UInt32
#define V3D_PCTR1_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR1_PCTR_SHIFT                                           0
#define    V3D_PCTR1_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS1_OFFSET                                                 0x0000068C
#define V3D_PCTRS1_TYPE                                                   UInt32
#define V3D_PCTRS1_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS1_PCTRS_SHIFT                                         0
#define    V3D_PCTRS1_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR2_OFFSET                                                  0x00000690
#define V3D_PCTR2_TYPE                                                    UInt32
#define V3D_PCTR2_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR2_PCTR_SHIFT                                           0
#define    V3D_PCTR2_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS2_OFFSET                                                 0x00000694
#define V3D_PCTRS2_TYPE                                                   UInt32
#define V3D_PCTRS2_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS2_PCTRS_SHIFT                                         0
#define    V3D_PCTRS2_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR3_OFFSET                                                  0x00000698
#define V3D_PCTR3_TYPE                                                    UInt32
#define V3D_PCTR3_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR3_PCTR_SHIFT                                           0
#define    V3D_PCTR3_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS3_OFFSET                                                 0x0000069C
#define V3D_PCTRS3_TYPE                                                   UInt32
#define V3D_PCTRS3_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS3_PCTRS_SHIFT                                         0
#define    V3D_PCTRS3_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR4_OFFSET                                                  0x000006A0
#define V3D_PCTR4_TYPE                                                    UInt32
#define V3D_PCTR4_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR4_PCTR_SHIFT                                           0
#define    V3D_PCTR4_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS4_OFFSET                                                 0x000006A4
#define V3D_PCTRS4_TYPE                                                   UInt32
#define V3D_PCTRS4_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS4_PCTRS_SHIFT                                         0
#define    V3D_PCTRS4_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR5_OFFSET                                                  0x000006A8
#define V3D_PCTR5_TYPE                                                    UInt32
#define V3D_PCTR5_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR5_PCTR_SHIFT                                           0
#define    V3D_PCTR5_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS5_OFFSET                                                 0x000006AC
#define V3D_PCTRS5_TYPE                                                   UInt32
#define V3D_PCTRS5_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS5_PCTRS_SHIFT                                         0
#define    V3D_PCTRS5_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR6_OFFSET                                                  0x000006B0
#define V3D_PCTR6_TYPE                                                    UInt32
#define V3D_PCTR6_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR6_PCTR_SHIFT                                           0
#define    V3D_PCTR6_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS6_OFFSET                                                 0x000006B4
#define V3D_PCTRS6_TYPE                                                   UInt32
#define V3D_PCTRS6_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS6_PCTRS_SHIFT                                         0
#define    V3D_PCTRS6_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR7_OFFSET                                                  0x000006B8
#define V3D_PCTR7_TYPE                                                    UInt32
#define V3D_PCTR7_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR7_PCTR_SHIFT                                           0
#define    V3D_PCTR7_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS7_OFFSET                                                 0x000006BC
#define V3D_PCTRS7_TYPE                                                   UInt32
#define V3D_PCTRS7_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS7_PCTRS_SHIFT                                         0
#define    V3D_PCTRS7_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR8_OFFSET                                                  0x000006C0
#define V3D_PCTR8_TYPE                                                    UInt32
#define V3D_PCTR8_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR8_PCTR_SHIFT                                           0
#define    V3D_PCTR8_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS8_OFFSET                                                 0x000006C4
#define V3D_PCTRS8_TYPE                                                   UInt32
#define V3D_PCTRS8_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS8_PCTRS_SHIFT                                         0
#define    V3D_PCTRS8_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR9_OFFSET                                                  0x000006C8
#define V3D_PCTR9_TYPE                                                    UInt32
#define V3D_PCTR9_RESERVED_MASK                                           0x00000000
#define    V3D_PCTR9_PCTR_SHIFT                                           0
#define    V3D_PCTR9_PCTR_MASK                                            0xFFFFFFFF

#define V3D_PCTRS9_OFFSET                                                 0x000006CC
#define V3D_PCTRS9_TYPE                                                   UInt32
#define V3D_PCTRS9_RESERVED_MASK                                          0xFFFFFFE0
#define    V3D_PCTRS9_PCTRS_SHIFT                                         0
#define    V3D_PCTRS9_PCTRS_MASK                                          0x0000001F

#define V3D_PCTR10_OFFSET                                                 0x000006D0
#define V3D_PCTR10_TYPE                                                   UInt32
#define V3D_PCTR10_RESERVED_MASK                                          0x00000000
#define    V3D_PCTR10_PCTR_SHIFT                                          0
#define    V3D_PCTR10_PCTR_MASK                                           0xFFFFFFFF

#define V3D_PCTRS10_OFFSET                                                0x000006D4
#define V3D_PCTRS10_TYPE                                                  UInt32
#define V3D_PCTRS10_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_PCTRS10_PCTRS_SHIFT                                        0
#define    V3D_PCTRS10_PCTRS_MASK                                         0x0000001F

#define V3D_PCTR11_OFFSET                                                 0x000006D8
#define V3D_PCTR11_TYPE                                                   UInt32
#define V3D_PCTR11_RESERVED_MASK                                          0x00000000
#define    V3D_PCTR11_PCTR_SHIFT                                          0
#define    V3D_PCTR11_PCTR_MASK                                           0xFFFFFFFF

#define V3D_PCTRS11_OFFSET                                                0x000006DC
#define V3D_PCTRS11_TYPE                                                  UInt32
#define V3D_PCTRS11_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_PCTRS11_PCTRS_SHIFT                                        0
#define    V3D_PCTRS11_PCTRS_MASK                                         0x0000001F

#define V3D_PCTR12_OFFSET                                                 0x000006E0
#define V3D_PCTR12_TYPE                                                   UInt32
#define V3D_PCTR12_RESERVED_MASK                                          0x00000000
#define    V3D_PCTR12_PCTR_SHIFT                                          0
#define    V3D_PCTR12_PCTR_MASK                                           0xFFFFFFFF

#define V3D_PCTRS12_OFFSET                                                0x000006E4
#define V3D_PCTRS12_TYPE                                                  UInt32
#define V3D_PCTRS12_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_PCTRS12_PCTRS_SHIFT                                        0
#define    V3D_PCTRS12_PCTRS_MASK                                         0x0000001F

#define V3D_PCTR13_OFFSET                                                 0x000006E8
#define V3D_PCTR13_TYPE                                                   UInt32
#define V3D_PCTR13_RESERVED_MASK                                          0x00000000
#define    V3D_PCTR13_PCTR_SHIFT                                          0
#define    V3D_PCTR13_PCTR_MASK                                           0xFFFFFFFF

#define V3D_PCTRS13_OFFSET                                                0x000006EC
#define V3D_PCTRS13_TYPE                                                  UInt32
#define V3D_PCTRS13_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_PCTRS13_PCTRS_SHIFT                                        0
#define    V3D_PCTRS13_PCTRS_MASK                                         0x0000001F

#define V3D_PCTR14_OFFSET                                                 0x000006F0
#define V3D_PCTR14_TYPE                                                   UInt32
#define V3D_PCTR14_RESERVED_MASK                                          0x00000000
#define    V3D_PCTR14_PCTR_SHIFT                                          0
#define    V3D_PCTR14_PCTR_MASK                                           0xFFFFFFFF

#define V3D_PCTRS14_OFFSET                                                0x000006F4
#define V3D_PCTRS14_TYPE                                                  UInt32
#define V3D_PCTRS14_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_PCTRS14_PCTRS_SHIFT                                        0
#define    V3D_PCTRS14_PCTRS_MASK                                         0x0000001F

#define V3D_PCTR15_OFFSET                                                 0x000006F8
#define V3D_PCTR15_TYPE                                                   UInt32
#define V3D_PCTR15_RESERVED_MASK                                          0x00000000
#define    V3D_PCTR15_PCTR_SHIFT                                          0
#define    V3D_PCTR15_PCTR_MASK                                           0xFFFFFFFF

#define V3D_PCTRS15_OFFSET                                                0x000006FC
#define V3D_PCTRS15_TYPE                                                  UInt32
#define V3D_PCTRS15_RESERVED_MASK                                         0xFFFFFFE0
#define    V3D_PCTRS15_PCTRS_SHIFT                                        0
#define    V3D_PCTRS15_PCTRS_MASK                                         0x0000001F

#define V3D_DBCFG_OFFSET                                                  0x00000E00
#define V3D_DBCFG_TYPE                                                    UInt32
#define V3D_DBCFG_RESERVED_MASK                                           0xFFFFFFFE
#define    V3D_DBCFG_DBGEN_SHIFT                                          0
#define    V3D_DBCFG_DBGEN_MASK                                           0x00000001

#define V3D_DBSCS_OFFSET                                                  0x00000E04
#define V3D_DBSCS_TYPE                                                    UInt32
#define V3D_DBSCS_RESERVED_MASK                                           0xFFFF80E0
#define    V3D_DBSCS_LENGTH_SHIFT                                         8
#define    V3D_DBSCS_LENGTH_MASK                                          0x00007F00
#define    V3D_DBSCS_STORE_SHIFT                                          4
#define    V3D_DBSCS_STORE_MASK                                           0x00000010
#define    V3D_DBSCS_LOAD_SHIFT                                           3
#define    V3D_DBSCS_LOAD_MASK                                            0x00000008
#define    V3D_DBSCS_SHIFTDR_SHIFT                                        2
#define    V3D_DBSCS_SHIFTDR_MASK                                         0x00000004
#define    V3D_DBSCS_SHIFTSR_SHIFT                                        1
#define    V3D_DBSCS_SHIFTSR_MASK                                         0x00000002
#define    V3D_DBSCS_IDLE_SHIFT                                           0
#define    V3D_DBSCS_IDLE_MASK                                            0x00000001

#define V3D_DBSCFG_OFFSET                                                 0x00000E08
#define V3D_DBSCFG_TYPE                                                   UInt32
#define V3D_DBSCFG_RESERVED_MASK                                          0xFFFFFFC0
#define    V3D_DBSCFG_CLIENT_SHIFT                                        0
#define    V3D_DBSCFG_CLIENT_MASK                                         0x0000003F

#define V3D_DBSSR_OFFSET                                                  0x00000E0C
#define V3D_DBSSR_TYPE                                                    UInt32
#define V3D_DBSSR_RESERVED_MASK                                           0xFFFF0000
#define    V3D_DBSSR_MADR_SHIFT                                           8
#define    V3D_DBSSR_MADR_MASK                                            0x0000FF00
#define    V3D_DBSSR_SUXTRA_SHIFT                                         6
#define    V3D_DBSSR_SUXTRA_MASK                                          0x000000C0
#define    V3D_DBSSR_DEVID_SHIFT                                          0
#define    V3D_DBSSR_DEVID_MASK                                           0x0000003F

#define V3D_DBSDR0_OFFSET                                                 0x00000E10
#define V3D_DBSDR0_TYPE                                                   UInt32
#define V3D_DBSDR0_RESERVED_MASK                                          0x00000000
#define    V3D_DBSDR0_DR0_SHIFT                                           0
#define    V3D_DBSDR0_DR0_MASK                                            0xFFFFFFFF

#define V3D_DBSDR1_OFFSET                                                 0x00000E14
#define V3D_DBSDR1_TYPE                                                   UInt32
#define V3D_DBSDR1_RESERVED_MASK                                          0x00000000
#define    V3D_DBSDR1_DR1_SHIFT                                           0
#define    V3D_DBSDR1_DR1_MASK                                            0xFFFFFFFF

#define V3D_DBSDR2_OFFSET                                                 0x00000E18
#define V3D_DBSDR2_TYPE                                                   UInt32
#define V3D_DBSDR2_RESERVED_MASK                                          0x00000000
#define    V3D_DBSDR2_DR2_SHIFT                                           0
#define    V3D_DBSDR2_DR2_MASK                                            0xFFFFFFFF

#define V3D_DBSDR3_OFFSET                                                 0x00000E1C
#define V3D_DBSDR3_TYPE                                                   UInt32
#define V3D_DBSDR3_RESERVED_MASK                                          0x00000000
#define    V3D_DBSDR3_DR3_SHIFT                                           0
#define    V3D_DBSDR3_DR3_MASK                                            0xFFFFFFFF

#define V3D_DBQRUN_OFFSET                                                 0x00000E20
#define V3D_DBQRUN_TYPE                                                   UInt32
#define V3D_DBQRUN_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQRUN_RUN_QPU0_RUN_QPU15_SHIFT                            0
#define    V3D_DBQRUN_RUN_QPU0_RUN_QPU15_MASK                             0x0000FFFF

#define V3D_DBQHLT_OFFSET                                                 0x00000E24
#define V3D_DBQHLT_TYPE                                                   UInt32
#define V3D_DBQHLT_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQHLT_HALT_QPU0_HALT_QPU15_SHIFT                          0
#define    V3D_DBQHLT_HALT_QPU0_HALT_QPU15_MASK                           0x0000FFFF

#define V3D_DBQSTP_OFFSET                                                 0x00000E28
#define V3D_DBQSTP_TYPE                                                   UInt32
#define V3D_DBQSTP_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQSTP_STEP_QPU0_STEP_QPU15_SHIFT                          0
#define    V3D_DBQSTP_STEP_QPU0_STEP_QPU15_MASK                           0x0000FFFF

#define V3D_DBQITE_OFFSET                                                 0x00000E2C
#define V3D_DBQITE_TYPE                                                   UInt32
#define V3D_DBQITE_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQITE_IE_QPU0_TO_IE_QPU15_SHIFT                           0
#define    V3D_DBQITE_IE_QPU0_TO_IE_QPU15_MASK                            0x0000FFFF

#define V3D_DBQITC_OFFSET                                                 0x00000E30
#define V3D_DBQITC_TYPE                                                   UInt32
#define V3D_DBQITC_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQITC_IC_QPU0_TO_IC_QPU15_SHIFT                           0
#define    V3D_DBQITC_IC_QPU0_TO_IC_QPU15_MASK                            0x0000FFFF

#define V3D_DBQGHC_OFFSET                                                 0x00000E34
#define V3D_DBQGHC_TYPE                                                   UInt32
#define V3D_DBQGHC_RESERVED_MASK                                          0xFFFFFFFC
#define    V3D_DBQGHC_HALT_SHIFT                                          1
#define    V3D_DBQGHC_HALT_MASK                                           0x00000002
#define    V3D_DBQGHC_HALTEN_SHIFT                                        0
#define    V3D_DBQGHC_HALTEN_MASK                                         0x00000001

#define V3D_DBQGHG_OFFSET                                                 0x00000E38
#define V3D_DBQGHG_TYPE                                                   UInt32
#define V3D_DBQGHG_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQGHG_GHG_QPU0_GHG_QPU15_SHIFT                            0
#define    V3D_DBQGHG_GHG_QPU0_GHG_QPU15_MASK                             0x0000FFFF

#define V3D_DBQGHH_OFFSET                                                 0x00000E3C
#define V3D_DBQGHH_TYPE                                                   UInt32
#define V3D_DBQGHH_RESERVED_MASK                                          0xFFFF0000
#define    V3D_DBQGHH_GHH_QPU0_GHH_QPU15_SHIFT                            0
#define    V3D_DBQGHH_GHH_QPU0_GHH_QPU15_MASK                             0x0000FFFF

#define V3D_DBGE_OFFSET                                                   0x00000F00
#define V3D_DBGE_TYPE                                                     UInt32
#define V3D_DBGE_RESERVED_MASK                                            0xFFE0FFF9
#define    V3D_DBGE_IPD2_FPDUSED_SHIFT                                    20
#define    V3D_DBGE_IPD2_FPDUSED_MASK                                     0x00100000
#define    V3D_DBGE_IPD2_VALID_SHIFT                                      19
#define    V3D_DBGE_IPD2_VALID_MASK                                       0x00080000
#define    V3D_DBGE_MULIP2_SHIFT                                          18
#define    V3D_DBGE_MULIP2_MASK                                           0x00040000
#define    V3D_DBGE_MULIP1_SHIFT                                          17
#define    V3D_DBGE_MULIP1_MASK                                           0x00020000
#define    V3D_DBGE_MULIP0_SHIFT                                          16
#define    V3D_DBGE_MULIP0_MASK                                           0x00010000
#define    V3D_DBGE_VR1_B_SHIFT                                           2
#define    V3D_DBGE_VR1_B_MASK                                            0x00000004
#define    V3D_DBGE_VR1_A_SHIFT                                           1
#define    V3D_DBGE_VR1_A_MASK                                            0x00000002

#define V3D_FDBG0_OFFSET                                                  0x00000F04
#define V3D_FDBG0_TYPE                                                    UInt32
#define V3D_FDBG0_RESERVED_MASK                                           0xFFFD0301
#define    V3D_FDBG0_EZREQ_FIFO_ORUN_SHIFT                                17
#define    V3D_FDBG0_EZREQ_FIFO_ORUN_MASK                                 0x00020000
#define    V3D_FDBG0_EZVAL_FIFO_ORUN_SHIFT                                15
#define    V3D_FDBG0_EZVAL_FIFO_ORUN_MASK                                 0x00008000
#define    V3D_FDBG0_DEPTHO_ORUN_SHIFT                                    14
#define    V3D_FDBG0_DEPTHO_ORUN_MASK                                     0x00004000
#define    V3D_FDBG0_DEPTHO_FIFO_ORUN_SHIFT                               13
#define    V3D_FDBG0_DEPTHO_FIFO_ORUN_MASK                                0x00002000
#define    V3D_FDBG0_REFXY_FIFO_ORUN_SHIFT                                12
#define    V3D_FDBG0_REFXY_FIFO_ORUN_MASK                                 0x00001000
#define    V3D_FDBG0_ZCOEFF_FIFO_FULL_SHIFT                               11
#define    V3D_FDBG0_ZCOEFF_FIFO_FULL_MASK                                0x00000800
#define    V3D_FDBG0_XYRELW_FIFO_ORUN_SHIFT                               10
#define    V3D_FDBG0_XYRELW_FIFO_ORUN_MASK                                0x00000400
#define    V3D_FDBG0_XYRELO_FIFO_ORUN_SHIFT                               7
#define    V3D_FDBG0_XYRELO_FIFO_ORUN_MASK                                0x00000080
#define    V3D_FDBG0_FIXZ_ORUN_SHIFT                                      6
#define    V3D_FDBG0_FIXZ_ORUN_MASK                                       0x00000040
#define    V3D_FDBG0_XYFO_FIFO_ORUN_SHIFT                                 5
#define    V3D_FDBG0_XYFO_FIFO_ORUN_MASK                                  0x00000020
#define    V3D_FDBG0_QBSZ_FIFO_ORUN_SHIFT                                 4
#define    V3D_FDBG0_QBSZ_FIFO_ORUN_MASK                                  0x00000010
#define    V3D_FDBG0_QBFR_FIFO_ORUN_SHIFT                                 3
#define    V3D_FDBG0_QBFR_FIFO_ORUN_MASK                                  0x00000008
#define    V3D_FDBG0_XYRELZ_FIFO_FULL_SHIFT                               2
#define    V3D_FDBG0_XYRELZ_FIFO_FULL_MASK                                0x00000004
#define    V3D_FDBG0_WCOEFF_FIFO_FULL_SHIFT                               1
#define    V3D_FDBG0_WCOEFF_FIFO_FULL_MASK                                0x00000002

#define V3D_FDBGB_OFFSET                                                  0x00000F08
#define V3D_FDBGB_TYPE                                                    UInt32
#define V3D_FDBGB_RESERVED_MASK                                           0xE17FFF00
#define    V3D_FDBGB_XYFO_FIFO_OP_READY_SHIFT                             28
#define    V3D_FDBGB_XYFO_FIFO_OP_READY_MASK                              0x10000000
#define    V3D_FDBGB_QXYF_FIFO_OP_READY_SHIFT                             27
#define    V3D_FDBGB_QXYF_FIFO_OP_READY_MASK                              0x08000000
#define    V3D_FDBGB_RAST_BUSY_SHIFT                                      26
#define    V3D_FDBGB_RAST_BUSY_MASK                                       0x04000000
#define    V3D_FDBGB_EZ_XY_READY_SHIFT                                    25
#define    V3D_FDBGB_EZ_XY_READY_MASK                                     0x02000000
#define    V3D_FDBGB_EZ_DATA_READY_SHIFT                                  23
#define    V3D_FDBGB_EZ_DATA_READY_MASK                                   0x00800000
#define    V3D_FDBGB_ZRWPE_READY_SHIFT                                    7
#define    V3D_FDBGB_ZRWPE_READY_MASK                                     0x00000080
#define    V3D_FDBGB_ZRWPE_STALL_SHIFT                                    6
#define    V3D_FDBGB_ZRWPE_STALL_MASK                                     0x00000040
#define    V3D_FDBGB_EDGES_CTRLID_SHIFT                                   3
#define    V3D_FDBGB_EDGES_CTRLID_MASK                                    0x00000038
#define    V3D_FDBGB_EDGES_ISCTRL_SHIFT                                   2
#define    V3D_FDBGB_EDGES_ISCTRL_MASK                                    0x00000004
#define    V3D_FDBGB_EDGES_READY_SHIFT                                    1
#define    V3D_FDBGB_EDGES_READY_MASK                                     0x00000002
#define    V3D_FDBGB_EDGES_STALL_SHIFT                                    0
#define    V3D_FDBGB_EDGES_STALL_MASK                                     0x00000001

#define V3D_FDBGR_OFFSET                                                  0x00000F0C
#define V3D_FDBGR_TYPE                                                    UInt32
#define V3D_FDBGR_RESERVED_MASK                                           0xA6009700
#define    V3D_FDBGR_FIXZ_READY_SHIFT                                     30
#define    V3D_FDBGR_FIXZ_READY_MASK                                      0x40000000
#define    V3D_FDBGR_RECIPW_READY_SHIFT                                   28
#define    V3D_FDBGR_RECIPW_READY_MASK                                    0x10000000
#define    V3D_FDBGR_INTERPRW_READY_SHIFT                                 27
#define    V3D_FDBGR_INTERPRW_READY_MASK                                  0x08000000
#define    V3D_FDBGR_INTERPZ_READY_SHIFT                                  24
#define    V3D_FDBGR_INTERPZ_READY_MASK                                   0x01000000
#define    V3D_FDBGR_XYRELZ_FIFO_LAST_SHIFT                               23
#define    V3D_FDBGR_XYRELZ_FIFO_LAST_MASK                                0x00800000
#define    V3D_FDBGR_XYRELZ_FIFO_READY_SHIFT                              22
#define    V3D_FDBGR_XYRELZ_FIFO_READY_MASK                               0x00400000
#define    V3D_FDBGR_XYNRM_LAST_SHIFT                                     21
#define    V3D_FDBGR_XYNRM_LAST_MASK                                      0x00200000
#define    V3D_FDBGR_XYNRM_READY_SHIFT                                    20
#define    V3D_FDBGR_XYNRM_READY_MASK                                     0x00100000
#define    V3D_FDBGR_EZLIM_READY_SHIFT                                    19
#define    V3D_FDBGR_EZLIM_READY_MASK                                     0x00080000
#define    V3D_FDBGR_DEPTHO_READY_SHIFT                                   18
#define    V3D_FDBGR_DEPTHO_READY_MASK                                    0x00040000
#define    V3D_FDBGR_RAST_LAST_SHIFT                                      17
#define    V3D_FDBGR_RAST_LAST_MASK                                       0x00020000
#define    V3D_FDBGR_RAST_READY_SHIFT                                     16
#define    V3D_FDBGR_RAST_READY_MASK                                      0x00010000
#define    V3D_FDBGR_XYFO_FIFO_READY_SHIFT                                14
#define    V3D_FDBGR_XYFO_FIFO_READY_MASK                                 0x00004000
#define    V3D_FDBGR_ZO_FIFO_READY_SHIFT                                  13
#define    V3D_FDBGR_ZO_FIFO_READY_MASK                                   0x00002000
#define    V3D_FDBGR_XYRELO_FIFO_READY_SHIFT                              11
#define    V3D_FDBGR_XYRELO_FIFO_READY_MASK                               0x00000800
#define    V3D_FDBGR_WCOEFF_FIFO_READY_SHIFT                              7
#define    V3D_FDBGR_WCOEFF_FIFO_READY_MASK                               0x00000080
#define    V3D_FDBGR_XYRELW_FIFO_READY_SHIFT                              6
#define    V3D_FDBGR_XYRELW_FIFO_READY_MASK                               0x00000040
#define    V3D_FDBGR_ZCOEFF_FIFO_READY_SHIFT                              5
#define    V3D_FDBGR_ZCOEFF_FIFO_READY_MASK                               0x00000020
#define    V3D_FDBGR_REFXY_FIFO_READY_SHIFT                               4
#define    V3D_FDBGR_REFXY_FIFO_READY_MASK                                0x00000010
#define    V3D_FDBGR_DEPTHO_FIFO_READY_SHIFT                              3
#define    V3D_FDBGR_DEPTHO_FIFO_READY_MASK                               0x00000008
#define    V3D_FDBGR_EZVAL_FIFO_READY_SHIFT                               2
#define    V3D_FDBGR_EZVAL_FIFO_READY_MASK                                0x00000004
#define    V3D_FDBGR_EZREQ_FIFO_READY_SHIFT                               1
#define    V3D_FDBGR_EZREQ_FIFO_READY_MASK                                0x00000002
#define    V3D_FDBGR_QXYF_FIFO_READY_SHIFT                                0
#define    V3D_FDBGR_QXYF_FIFO_READY_MASK                                 0x00000001

#define V3D_FDBGS_OFFSET                                                  0x00000F10
#define V3D_FDBGS_TYPE                                                    UInt32
#define V3D_FDBGS_RESERVED_MASK                                           0xEDB80000
#define    V3D_FDBGS_ZO_FIFO_IP_STALL_SHIFT                               28
#define    V3D_FDBGS_ZO_FIFO_IP_STALL_MASK                                0x10000000
#define    V3D_FDBGS_RECIPW_IP_STALL_SHIFT                                25
#define    V3D_FDBGS_RECIPW_IP_STALL_MASK                                 0x02000000
#define    V3D_FDBGS_INTERPW_IP_STALL_SHIFT                               22
#define    V3D_FDBGS_INTERPW_IP_STALL_MASK                                0x00400000
#define    V3D_FDBGS_XYRELZ_FIFO_IP_STALL_SHIFT                           18
#define    V3D_FDBGS_XYRELZ_FIFO_IP_STALL_MASK                            0x00040000
#define    V3D_FDBGS_INTERPZ_IP_STALL_SHIFT                               17
#define    V3D_FDBGS_INTERPZ_IP_STALL_MASK                                0x00020000
#define    V3D_FDBGS_DEPTHO_FIFO_IP_STALL_SHIFT                           16
#define    V3D_FDBGS_DEPTHO_FIFO_IP_STALL_MASK                            0x00010000
#define    V3D_FDBGS_EZLIM_IP_STALL_SHIFT                                 15
#define    V3D_FDBGS_EZLIM_IP_STALL_MASK                                  0x00008000
#define    V3D_FDBGS_XYNRM_IP_STALL_SHIFT                                 14
#define    V3D_FDBGS_XYNRM_IP_STALL_MASK                                  0x00004000
#define    V3D_FDBGS_EZREQ_FIFO_OP_VALID_SHIFT                            13
#define    V3D_FDBGS_EZREQ_FIFO_OP_VALID_MASK                             0x00002000
#define    V3D_FDBGS_QXYF_FIFO_OP_VALID_SHIFT                             12
#define    V3D_FDBGS_QXYF_FIFO_OP_VALID_MASK                              0x00001000
#define    V3D_FDBGS_QXYF_FIFO_OP_LAST_SHIFT                              11
#define    V3D_FDBGS_QXYF_FIFO_OP_LAST_MASK                               0x00000800
#define    V3D_FDBGS_QXYF_FIFO_OP1_DUMMY_SHIFT                            10
#define    V3D_FDBGS_QXYF_FIFO_OP1_DUMMY_MASK                             0x00000400
#define    V3D_FDBGS_QXYF_FIFO_OP1_LAST_SHIFT                             9
#define    V3D_FDBGS_QXYF_FIFO_OP1_LAST_MASK                              0x00000200
#define    V3D_FDBGS_QXYF_FIFO_OP1_VALID_SHIFT                            8
#define    V3D_FDBGS_QXYF_FIFO_OP1_VALID_MASK                             0x00000100
#define    V3D_FDBGS_EZTEST_ANYQVALID_SHIFT                               7
#define    V3D_FDBGS_EZTEST_ANYQVALID_MASK                                0x00000080
#define    V3D_FDBGS_EZTEST_ANYQF_SHIFT                                   6
#define    V3D_FDBGS_EZTEST_ANYQF_MASK                                    0x00000040
#define    V3D_FDBGS_EZTEST_QREADY_SHIFT                                  5
#define    V3D_FDBGS_EZTEST_QREADY_MASK                                   0x00000020
#define    V3D_FDBGS_EZTEST_VLF_OKNOVALID_SHIFT                           4
#define    V3D_FDBGS_EZTEST_VLF_OKNOVALID_MASK                            0x00000010
#define    V3D_FDBGS_EZTEST_STALL_SHIFT                                   3
#define    V3D_FDBGS_EZTEST_STALL_MASK                                    0x00000008
#define    V3D_FDBGS_EZTEST_IP_VLFSTALL_SHIFT                             2
#define    V3D_FDBGS_EZTEST_IP_VLFSTALL_MASK                              0x00000004
#define    V3D_FDBGS_EZTEST_IP_PRSTALL_SHIFT                              1
#define    V3D_FDBGS_EZTEST_IP_PRSTALL_MASK                               0x00000002
#define    V3D_FDBGS_EZTEST_IP_QSTALL_SHIFT                               0
#define    V3D_FDBGS_EZTEST_IP_QSTALL_MASK                                0x00000001

#define V3D_ERRSTAT_OFFSET                                                0x00000F20
#define V3D_ERRSTAT_TYPE                                                  UInt32
#define V3D_ERRSTAT_RESERVED_MASK                                         0xFFFF0000
#define    V3D_ERRSTAT_L2CARE_SHIFT                                       15
#define    V3D_ERRSTAT_L2CARE_MASK                                        0x00008000
#define    V3D_ERRSTAT_VCMBE_SHIFT                                        14
#define    V3D_ERRSTAT_VCMBE_MASK                                         0x00004000
#define    V3D_ERRSTAT_VCMRE_SHIFT                                        13
#define    V3D_ERRSTAT_VCMRE_MASK                                         0x00002000
#define    V3D_ERRSTAT_VCDI_SHIFT                                         12
#define    V3D_ERRSTAT_VCDI_MASK                                          0x00001000
#define    V3D_ERRSTAT_VCDE_SHIFT                                         11
#define    V3D_ERRSTAT_VCDE_MASK                                          0x00000800
#define    V3D_ERRSTAT_VDWE_SHIFT                                         10
#define    V3D_ERRSTAT_VDWE_MASK                                          0x00000400
#define    V3D_ERRSTAT_VPMEAS_SHIFT                                       9
#define    V3D_ERRSTAT_VPMEAS_MASK                                        0x00000200
#define    V3D_ERRSTAT_VPMEFNA_SHIFT                                      8
#define    V3D_ERRSTAT_VPMEFNA_MASK                                       0x00000100
#define    V3D_ERRSTAT_VPMEWNA_SHIFT                                      7
#define    V3D_ERRSTAT_VPMEWNA_MASK                                       0x00000080
#define    V3D_ERRSTAT_VPMERNA_SHIFT                                      6
#define    V3D_ERRSTAT_VPMERNA_MASK                                       0x00000040
#define    V3D_ERRSTAT_VPMERR_SHIFT                                       5
#define    V3D_ERRSTAT_VPMERR_MASK                                        0x00000020
#define    V3D_ERRSTAT_VPMEWR_SHIFT                                       4
#define    V3D_ERRSTAT_VPMEWR_MASK                                        0x00000010
#define    V3D_ERRSTAT_VPAERRGL_SHIFT                                     3
#define    V3D_ERRSTAT_VPAERRGL_MASK                                      0x00000008
#define    V3D_ERRSTAT_VPAEBRGL_SHIFT                                     2
#define    V3D_ERRSTAT_VPAEBRGL_MASK                                      0x00000004
#define    V3D_ERRSTAT_VPAERGS_SHIFT                                      1
#define    V3D_ERRSTAT_VPAERGS_MASK                                       0x00000002
#define    V3D_ERRSTAT_VPAEABB_SHIFT                                      0
#define    V3D_ERRSTAT_VPAEABB_MASK                                       0x00000001

#endif /* __BRCM_RDB_V3D_H__ */



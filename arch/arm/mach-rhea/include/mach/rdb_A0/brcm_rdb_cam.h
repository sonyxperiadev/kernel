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

#ifndef __BRCM_RDB_CAM_H__
#define __BRCM_RDB_CAM_H__

#define CAM_CTL_OFFSET                                                    0x00000000
#define CAM_CTL_TYPE                                                      UInt32
#define CAM_CTL_RESERVED_MASK                                             0xFFF000C0
#define    CAM_CTL_OET_SHIFT                                              12
#define    CAM_CTL_OET_MASK                                               0x000FF000
#define    CAM_CTL_PFT_SHIFT                                              8
#define    CAM_CTL_PFT_MASK                                               0x00000F00
#define    CAM_CTL_DCM_SHIFT                                              5
#define    CAM_CTL_DCM_MASK                                               0x00000020
#define    CAM_CTL_SOE_SHIFT                                              4
#define    CAM_CTL_SOE_MASK                                               0x00000010
#define    CAM_CTL_CPM_SHIFT                                              3
#define    CAM_CTL_CPM_MASK                                               0x00000008
#define    CAM_CTL_CPR_SHIFT                                              2
#define    CAM_CTL_CPR_MASK                                               0x00000004
#define    CAM_CTL_MEN_SHIFT                                              1
#define    CAM_CTL_MEN_MASK                                               0x00000002
#define    CAM_CTL_CPE_SHIFT                                              0
#define    CAM_CTL_CPE_MASK                                               0x00000001

#define CAM_STA_OFFSET                                                    0x00000004
#define CAM_STA_TYPE                                                      UInt32
#define CAM_STA_RESERVED_MASK                                             0xFF0E0000
#define    CAM_STA_BUF1_NO_SHIFT                                          23
#define    CAM_STA_BUF1_NO_MASK                                           0x00800000
#define    CAM_STA_BUF1_RDY_SHIFT                                         22
#define    CAM_STA_BUF1_RDY_MASK                                          0x00400000
#define    CAM_STA_BUF0_NO_SHIFT                                          21
#define    CAM_STA_BUF0_NO_MASK                                           0x00200000
#define    CAM_STA_BUF0_RDY_SHIFT                                         20
#define    CAM_STA_BUF0_RDY_MASK                                          0x00100000
#define    CAM_STA_PI1_SHIFT                                              16
#define    CAM_STA_PI1_MASK                                               0x00010000
#define    CAM_STA_PI0_SHIFT                                              15
#define    CAM_STA_PI0_MASK                                               0x00008000
#define    CAM_STA_IS_SHIFT                                               14
#define    CAM_STA_IS_MASK                                                0x00004000
#define    CAM_STA_PS_SHIFT                                               13
#define    CAM_STA_PS_MASK                                                0x00002000
#define    CAM_STA_DL_SHIFT                                               12
#define    CAM_STA_DL_MASK                                                0x00001000
#define    CAM_STA_BFO_SHIFT                                              11
#define    CAM_STA_BFO_MASK                                               0x00000800
#define    CAM_STA_OFO_SHIFT                                              10
#define    CAM_STA_OFO_MASK                                               0x00000400
#define    CAM_STA_IFO_SHIFT                                              9
#define    CAM_STA_IFO_MASK                                               0x00000200
#define    CAM_STA_OES_SHIFT                                              8
#define    CAM_STA_OES_MASK                                               0x00000100
#define    CAM_STA_CRCE_SHIFT                                             7
#define    CAM_STA_CRCE_MASK                                              0x00000080
#define    CAM_STA_SSC_SHIFT                                              6
#define    CAM_STA_SSC_MASK                                               0x00000040
#define    CAM_STA_PLE_SHIFT                                              5
#define    CAM_STA_PLE_MASK                                               0x00000020
#define    CAM_STA_HOE_SHIFT                                              4
#define    CAM_STA_HOE_MASK                                               0x00000010
#define    CAM_STA_PBE_SHIFT                                              3
#define    CAM_STA_PBE_MASK                                               0x00000008
#define    CAM_STA_SBE_SHIFT                                              2
#define    CAM_STA_SBE_MASK                                               0x00000004
#define    CAM_STA_CS_SHIFT                                               1
#define    CAM_STA_CS_MASK                                                0x00000002
#define    CAM_STA_SYN_SHIFT                                              0
#define    CAM_STA_SYN_MASK                                               0x00000001

#define CAM_ANA_OFFSET                                                    0x00000008
#define CAM_ANA_TYPE                                                      UInt32
#define CAM_ANA_RESERVED_MASK                                             0xFFFFF000
#define    CAM_ANA_PTATADJ_SHIFT                                          8
#define    CAM_ANA_PTATADJ_MASK                                           0x00000F00
#define    CAM_ANA_CTATADJ_SHIFT                                          4
#define    CAM_ANA_CTATADJ_MASK                                           0x000000F0
#define    CAM_ANA_DDL_SHIFT                                              3
#define    CAM_ANA_DDL_MASK                                               0x00000008
#define    CAM_ANA_AR_SHIFT                                               2
#define    CAM_ANA_AR_MASK                                                0x00000004
#define    CAM_ANA_BPD_SHIFT                                              1
#define    CAM_ANA_BPD_MASK                                               0x00000002
#define    CAM_ANA_APD_SHIFT                                              0
#define    CAM_ANA_APD_MASK                                               0x00000001

#define CAM_PRI_OFFSET                                                    0x0000000C
#define CAM_PRI_TYPE                                                      UInt32
#define CAM_PRI_RESERVED_MASK                                             0xFFFC0009
#define    CAM_PRI_BL_SHIFT                                               16
#define    CAM_PRI_BL_MASK                                                0x00030000
#define    CAM_PRI_BS_SHIFT                                               12
#define    CAM_PRI_BS_MASK                                                0x0000F000
#define    CAM_PRI_PP_SHIFT                                               8
#define    CAM_PRI_PP_MASK                                                0x00000F00
#define    CAM_PRI_NP_SHIFT                                               4
#define    CAM_PRI_NP_MASK                                                0x000000F0
#define    CAM_PRI_PT_SHIFT                                               1
#define    CAM_PRI_PT_MASK                                                0x00000006

#define CAM_CLK_OFFSET                                                    0x00000010
#define CAM_CLK_TYPE                                                      UInt32
#define CAM_CLK_RESERVED_MASK                                             0x10FFF0E0
#define    CAM_CLK_CLDN_SHIFT                                             31
#define    CAM_CLK_CLDN_MASK                                              0x80000000
#define    CAM_CLK_CLDP_SHIFT                                             30
#define    CAM_CLK_CLDP_MASK                                              0x40000000
#define    CAM_CLK_CLSTE_SHIFT                                            29
#define    CAM_CLK_CLSTE_MASK                                             0x20000000
#define    CAM_CLK_CLS_SHIFT                                              24
#define    CAM_CLK_CLS_MASK                                               0x0F000000
#define    CAM_CLK_CLAC_SHIFT                                             8
#define    CAM_CLK_CLAC_MASK                                              0x00000F00
#define    CAM_CLK_CLTRE_SHIFT                                            4
#define    CAM_CLK_CLTRE_MASK                                             0x00000010
#define    CAM_CLK_CLHSE_SHIFT                                            3
#define    CAM_CLK_CLHSE_MASK                                             0x00000008
#define    CAM_CLK_CLLPE_SHIFT                                            2
#define    CAM_CLK_CLLPE_MASK                                             0x00000004
#define    CAM_CLK_CLPD_SHIFT                                             1
#define    CAM_CLK_CLPD_MASK                                              0x00000002
#define    CAM_CLK_CLE_SHIFT                                              0
#define    CAM_CLK_CLE_MASK                                               0x00000001

#define CAM_CLT_OFFSET                                                    0x00000014
#define CAM_CLT_TYPE                                                      UInt32
#define CAM_CLT_RESERVED_MASK                                             0xFFFF0000
#define    CAM_CLT_CLT2_SHIFT                                             8
#define    CAM_CLT_CLT2_MASK                                              0x0000FF00
#define    CAM_CLT_CLT1_SHIFT                                             0
#define    CAM_CLT_CLT1_MASK                                              0x000000FF

#define CAM_DAT0_OFFSET                                                   0x00000018
#define CAM_DAT0_TYPE                                                     UInt32
#define CAM_DAT0_RESERVED_MASK                                            0x007FF0C0
#define    CAM_DAT0_DLDNN_SHIFT                                           31
#define    CAM_DAT0_DLDNN_MASK                                            0x80000000
#define    CAM_DAT0_DLDPN_SHIFT                                           30
#define    CAM_DAT0_DLDPN_MASK                                            0x40000000
#define    CAM_DAT0_DLSTEN_SHIFT                                          29
#define    CAM_DAT0_DLSTEN_MASK                                           0x20000000
#define    CAM_DAT0_DLFON_SHIFT                                           28
#define    CAM_DAT0_DLFON_MASK                                            0x10000000
#define    CAM_DAT0_DLSN_SHIFT                                            24
#define    CAM_DAT0_DLSN_MASK                                             0x0F000000
#define    CAM_DAT0_DLSEN_SHIFT                                           23
#define    CAM_DAT0_DLSEN_MASK                                            0x00800000
#define    CAM_DAT0_DLACN_SHIFT                                           8
#define    CAM_DAT0_DLACN_MASK                                            0x00000F00
#define    CAM_DAT0_DLSMN_SHIFT                                           5
#define    CAM_DAT0_DLSMN_MASK                                            0x00000020
#define    CAM_DAT0_DLTREN_SHIFT                                          4
#define    CAM_DAT0_DLTREN_MASK                                           0x00000010
#define    CAM_DAT0_DLHSEN_SHIFT                                          3
#define    CAM_DAT0_DLHSEN_MASK                                           0x00000008
#define    CAM_DAT0_DLLPEN_SHIFT                                          2
#define    CAM_DAT0_DLLPEN_MASK                                           0x00000004
#define    CAM_DAT0_DLPDN_SHIFT                                           1
#define    CAM_DAT0_DLPDN_MASK                                            0x00000002
#define    CAM_DAT0_DLEN_SHIFT                                            0
#define    CAM_DAT0_DLEN_MASK                                             0x00000001

#define CAM_DAT1_OFFSET                                                   0x0000001C
#define CAM_DAT1_TYPE                                                     UInt32
#define CAM_DAT1_RESERVED_MASK                                            0x007FF0C0
#define    CAM_DAT1_DLDNN_SHIFT                                           31
#define    CAM_DAT1_DLDNN_MASK                                            0x80000000
#define    CAM_DAT1_DLDPN_SHIFT                                           30
#define    CAM_DAT1_DLDPN_MASK                                            0x40000000
#define    CAM_DAT1_DLSTEN_SHIFT                                          29
#define    CAM_DAT1_DLSTEN_MASK                                           0x20000000
#define    CAM_DAT1_DLFON_SHIFT                                           28
#define    CAM_DAT1_DLFON_MASK                                            0x10000000
#define    CAM_DAT1_DLSN_SHIFT                                            24
#define    CAM_DAT1_DLSN_MASK                                             0x0F000000
#define    CAM_DAT1_DLSEN_SHIFT                                           23
#define    CAM_DAT1_DLSEN_MASK                                            0x00800000
#define    CAM_DAT1_DLACN_SHIFT                                           8
#define    CAM_DAT1_DLACN_MASK                                            0x00000F00
#define    CAM_DAT1_DLSMN_SHIFT                                           5
#define    CAM_DAT1_DLSMN_MASK                                            0x00000020
#define    CAM_DAT1_DLTREN_SHIFT                                          4
#define    CAM_DAT1_DLTREN_MASK                                           0x00000010
#define    CAM_DAT1_DLHSEN_SHIFT                                          3
#define    CAM_DAT1_DLHSEN_MASK                                           0x00000008
#define    CAM_DAT1_DLLPEN_SHIFT                                          2
#define    CAM_DAT1_DLLPEN_MASK                                           0x00000004
#define    CAM_DAT1_DLPDN_SHIFT                                           1
#define    CAM_DAT1_DLPDN_MASK                                            0x00000002
#define    CAM_DAT1_DLEN_SHIFT                                            0
#define    CAM_DAT1_DLEN_MASK                                             0x00000001

#define CAM_DLT_OFFSET                                                    0x00000028
#define CAM_DLT_TYPE                                                      UInt32
#define CAM_DLT_RESERVED_MASK                                             0xFF000000
#define    CAM_DLT_DLT3_SHIFT                                             16
#define    CAM_DLT_DLT3_MASK                                              0x00FF0000
#define    CAM_DLT_DLT2_SHIFT                                             8
#define    CAM_DLT_DLT2_MASK                                              0x0000FF00
#define    CAM_DLT_DLT1_SHIFT                                             0
#define    CAM_DLT_DLT1_MASK                                              0x000000FF

#define CAM_CMP0_OFFSET                                                   0x0000002C
#define CAM_CMP0_TYPE                                                     UInt32
#define CAM_CMP0_RESERVED_MASK                                            0x7FFFFC00
#define    CAM_CMP0_PCEN_SHIFT                                            31
#define    CAM_CMP0_PCEN_MASK                                             0x80000000
#define    CAM_CMP0_GIN_SHIFT                                             9
#define    CAM_CMP0_GIN_MASK                                              0x00000200
#define    CAM_CMP0_CPHN_SHIFT                                            8
#define    CAM_CMP0_CPHN_MASK                                             0x00000100
#define    CAM_CMP0_PCVCN_SHIFT                                           6
#define    CAM_CMP0_PCVCN_MASK                                            0x000000C0
#define    CAM_CMP0_PCDTN_SHIFT                                           0
#define    CAM_CMP0_PCDTN_MASK                                            0x0000003F

#define CAM_CMP1_OFFSET                                                   0x00000030
#define CAM_CMP1_TYPE                                                     UInt32
#define CAM_CMP1_RESERVED_MASK                                            0x7FFFFC00
#define    CAM_CMP1_PCEN_SHIFT                                            31
#define    CAM_CMP1_PCEN_MASK                                             0x80000000
#define    CAM_CMP1_GIN_SHIFT                                             9
#define    CAM_CMP1_GIN_MASK                                              0x00000200
#define    CAM_CMP1_CPHN_SHIFT                                            8
#define    CAM_CMP1_CPHN_MASK                                             0x00000100
#define    CAM_CMP1_PCVCN_SHIFT                                           6
#define    CAM_CMP1_PCVCN_MASK                                            0x000000C0
#define    CAM_CMP1_PCDTN_SHIFT                                           0
#define    CAM_CMP1_PCDTN_MASK                                            0x0000003F

#define CAM_CAP0_OFFSET                                                   0x00000034
#define CAM_CAP0_TYPE                                                     UInt32
#define CAM_CAP0_RESERVED_MASK                                            0x40000000
#define    CAM_CAP0_CPHV_SHIFT                                            31
#define    CAM_CAP0_CPHV_MASK                                             0x80000000
#define    CAM_CAP0_CECCN_SHIFT                                           24
#define    CAM_CAP0_CECCN_MASK                                            0x3F000000
#define    CAM_CAP0_CWCN_SHIFT                                            8
#define    CAM_CAP0_CWCN_MASK                                             0x00FFFF00
#define    CAM_CAP0_CVCN_SHIFT                                            6
#define    CAM_CAP0_CVCN_MASK                                             0x000000C0
#define    CAM_CAP0_CDTN_SHIFT                                            0
#define    CAM_CAP0_CDTN_MASK                                             0x0000003F

#define CAM_CAP1_OFFSET                                                   0x00000038
#define CAM_CAP1_TYPE                                                     UInt32
#define CAM_CAP1_RESERVED_MASK                                            0x40000000
#define    CAM_CAP1_CPHV_SHIFT                                            31
#define    CAM_CAP1_CPHV_MASK                                             0x80000000
#define    CAM_CAP1_CECCN_SHIFT                                           24
#define    CAM_CAP1_CECCN_MASK                                            0x3F000000
#define    CAM_CAP1_CWCN_SHIFT                                            8
#define    CAM_CAP1_CWCN_MASK                                             0x00FFFF00
#define    CAM_CAP1_CVCN_SHIFT                                            6
#define    CAM_CAP1_CVCN_MASK                                             0x000000C0
#define    CAM_CAP1_CDTN_SHIFT                                            0
#define    CAM_CAP1_CDTN_MASK                                             0x0000003F

#define CAM_DBG0_OFFSET                                                   0x000000F0
#define CAM_DBG0_TYPE                                                     UInt32
#define CAM_DBG0_RESERVED_MASK                                            0xFFF00000
#define    CAM_DBG0_L3PK_SHIFT                                            15
#define    CAM_DBG0_L3PK_MASK                                             0x000F8000
#define    CAM_DBG0_L2PK_SHIFT                                            10
#define    CAM_DBG0_L2PK_MASK                                             0x00007C00
#define    CAM_DBG0_L1PK_SHIFT                                            5
#define    CAM_DBG0_L1PK_MASK                                             0x000003E0
#define    CAM_DBG0_L0PK_SHIFT                                            0
#define    CAM_DBG0_L0PK_MASK                                             0x0000001F

#define CAM_DBG1_OFFSET                                                   0x000000F4
#define CAM_DBG1_TYPE                                                     UInt32
#define CAM_DBG1_RESERVED_MASK                                            0x00000000
#define    CAM_DBG1_BFPK_SHIFT                                            16
#define    CAM_DBG1_BFPK_MASK                                             0xFFFF0000
#define    CAM_DBG1_DFPK_SHIFT                                            0
#define    CAM_DBG1_DFPK_MASK                                             0x0000FFFF

#define CAM_DBG2_OFFSET                                                   0x000000F8
#define CAM_DBG2_TYPE                                                     UInt32
#define CAM_DBG2_RESERVED_MASK                                            0x00000000
#define    CAM_DBG2_BFC_SHIFT                                             16
#define    CAM_DBG2_BFC_MASK                                              0xFFFF0000
#define    CAM_DBG2_DFC_SHIFT                                             0
#define    CAM_DBG2_DFC_MASK                                              0x0000FFFF

#define CAM_DBG3_OFFSET                                                   0x000000FC
#define CAM_DBG3_TYPE                                                     UInt32
#define CAM_DBG3_RESERVED_MASK                                            0x0000FF80
#define    CAM_DBG3_IFPK_SHIFT                                            16
#define    CAM_DBG3_IFPK_MASK                                             0xFFFF0000
#define    CAM_DBG3_PFS_SHIFT                                             4
#define    CAM_DBG3_PFS_MASK                                              0x00000070
#define    CAM_DBG3_HSSYNC_SHIFT                                          0
#define    CAM_DBG3_HSSYNC_MASK                                           0x0000000F

#define CAM_ICTL_OFFSET                                                   0x00000100
#define CAM_ICTL_TYPE                                                     UInt32
#define CAM_ICTL_RESERVED_MASK                                            0xE000FF80
#define    CAM_ICTL_LCIE_SHIFT                                            16
#define    CAM_ICTL_LCIE_MASK                                             0x1FFF0000
#define    CAM_ICTL_LIP_SHIFT                                             5
#define    CAM_ICTL_LIP_MASK                                              0x00000060
#define    CAM_ICTL_TFC_SHIFT                                             4
#define    CAM_ICTL_TFC_MASK                                              0x00000010
#define    CAM_ICTL_FCM_SHIFT                                             3
#define    CAM_ICTL_FCM_MASK                                              0x00000008
#define    CAM_ICTL_IBOB_SHIFT                                            2
#define    CAM_ICTL_IBOB_MASK                                             0x00000004
#define    CAM_ICTL_FEIE_SHIFT                                            1
#define    CAM_ICTL_FEIE_MASK                                             0x00000002
#define    CAM_ICTL_FSIE_SHIFT                                            0
#define    CAM_ICTL_FSIE_MASK                                             0x00000001

#define CAM_ISTA_OFFSET                                                   0x00000104
#define CAM_ISTA_TYPE                                                     UInt32
#define CAM_ISTA_RESERVED_MASK                                            0xFFFFFFF8
#define    CAM_ISTA_LCI_SHIFT                                             2
#define    CAM_ISTA_LCI_MASK                                              0x00000004
#define    CAM_ISTA_FEI_SHIFT                                             1
#define    CAM_ISTA_FEI_MASK                                              0x00000002
#define    CAM_ISTA_FSI_SHIFT                                             0
#define    CAM_ISTA_FSI_MASK                                              0x00000001

#define CAM_IDI_OFFSET                                                    0x00000108
#define CAM_IDI_TYPE                                                      UInt32
#define CAM_IDI_RESERVED_MASK                                             0x00000000
#define    CAM_IDI_IDI3_SHIFT                                             24
#define    CAM_IDI_IDI3_MASK                                              0xFF000000
#define    CAM_IDI_IDI2_SHIFT                                             16
#define    CAM_IDI_IDI2_MASK                                              0x00FF0000
#define    CAM_IDI_IDI1_SHIFT                                             8
#define    CAM_IDI_IDI1_MASK                                              0x0000FF00
#define    CAM_IDI_IDI0_SHIFT                                             0
#define    CAM_IDI_IDI0_MASK                                              0x000000FF

#define CAM_IPIPE_OFFSET                                                  0x0000010C
#define CAM_IPIPE_TYPE                                                    UInt32
#define CAM_IPIPE_RESERVED_MASK                                           0xFFF80000
#define    CAM_IPIPE_IDM_SHIFT                                            17
#define    CAM_IPIPE_IDM_MASK                                             0x00060000
#define    CAM_IPIPE_ICM_SHIFT                                            15
#define    CAM_IPIPE_ICM_MASK                                             0x00018000
#define    CAM_IPIPE_DEBL_SHIFT                                           12
#define    CAM_IPIPE_DEBL_MASK                                            0x00007000
#define    CAM_IPIPE_DEM_SHIFT                                            10
#define    CAM_IPIPE_DEM_MASK                                             0x00000C00
#define    CAM_IPIPE_PPM_SHIFT                                            7
#define    CAM_IPIPE_PPM_MASK                                             0x00000380
#define    CAM_IPIPE_DDM_SHIFT                                            3
#define    CAM_IPIPE_DDM_MASK                                             0x00000078
#define    CAM_IPIPE_PUM_SHIFT                                            0
#define    CAM_IPIPE_PUM_MASK                                             0x00000007

#define CAM_IBSA_OFFSET                                                   0x00000110
#define CAM_IBSA_TYPE                                                     UInt32
#define CAM_IBSA_RESERVED_MASK                                            0x00000000
#define    CAM_IBSA_IBSA_SHIFT                                            0
#define    CAM_IBSA_IBSA_MASK                                             0xFFFFFFFF

#define CAM_IBEA_OFFSET                                                   0x00000114
#define CAM_IBEA_TYPE                                                     UInt32
#define CAM_IBEA_RESERVED_MASK                                            0x00000000
#define    CAM_IBEA_IBEA_SHIFT                                            0
#define    CAM_IBEA_IBEA_MASK                                             0xFFFFFFFF

#define CAM_IBLS_OFFSET                                                   0x00000118
#define CAM_IBLS_TYPE                                                     UInt32
#define CAM_IBLS_RESERVED_MASK                                            0xFFFF0000
#define    CAM_IBLS_IBLS_SHIFT                                            0
#define    CAM_IBLS_IBLS_MASK                                             0x0000FFFF

#define CAM_IBWP_OFFSET                                                   0x0000011C
#define CAM_IBWP_TYPE                                                     UInt32
#define CAM_IBWP_RESERVED_MASK                                            0x00000000
#define    CAM_IBWP_IBWP_SHIFT                                            0
#define    CAM_IBWP_IBWP_MASK                                             0xFFFFFFFF

#define CAM_IHWIN_OFFSET                                                  0x00000120
#define CAM_IHWIN_TYPE                                                    UInt32
#define CAM_IHWIN_RESERVED_MASK                                           0x00000000
#define    CAM_IHWIN_HWEP_SHIFT                                           16
#define    CAM_IHWIN_HWEP_MASK                                            0xFFFF0000
#define    CAM_IHWIN_HWSP_SHIFT                                           0
#define    CAM_IHWIN_HWSP_MASK                                            0x0000FFFF

#define CAM_IHSTA_OFFSET                                                  0x00000124
#define CAM_IHSTA_TYPE                                                    UInt32
#define CAM_IHSTA_RESERVED_MASK                                           0xFFFF0000
#define    CAM_IHSTA_PPL_SHIFT                                            0
#define    CAM_IHSTA_PPL_MASK                                             0x0000FFFF

#define CAM_IVWIN_OFFSET                                                  0x00000128
#define CAM_IVWIN_TYPE                                                    UInt32
#define CAM_IVWIN_RESERVED_MASK                                           0x00000000
#define    CAM_IVWIN_VWEL_SHIFT                                           16
#define    CAM_IVWIN_VWEL_MASK                                            0xFFFF0000
#define    CAM_IVWIN_VWSL_SHIFT                                           0
#define    CAM_IVWIN_VWSL_MASK                                            0x0000FFFF

#define CAM_IVSTA_OFFSET                                                  0x0000012C
#define CAM_IVSTA_TYPE                                                    UInt32
#define CAM_IVSTA_RESERVED_MASK                                           0xFFFF0000
#define    CAM_IVSTA_LPF_SHIFT                                            0
#define    CAM_IVSTA_LPF_MASK                                             0x0000FFFF

#define CAM_ICC_OFFSET                                                    0x00000130
#define CAM_ICC_TYPE                                                      UInt32
#define CAM_ICC_RESERVED_MASK                                             0x00000000
#define    CAM_ICC_ICLL_SHIFT                                             16
#define    CAM_ICC_ICLL_MASK                                              0xFFFF0000
#define    CAM_ICC_ICLT_SHIFT                                             13
#define    CAM_ICC_ICLT_MASK                                              0x0000E000
#define    CAM_ICC_ICST_SHIFT                                             10
#define    CAM_ICC_ICST_MASK                                              0x00001C00
#define    CAM_ICC_ICFH_SHIFT                                             5
#define    CAM_ICC_ICFH_MASK                                              0x000003E0
#define    CAM_ICC_ICFL_SHIFT                                             0
#define    CAM_ICC_ICFL_MASK                                              0x0000001F

#define CAM_ICS_OFFSET                                                    0x00000134
#define CAM_ICS_TYPE                                                      UInt32
#define CAM_ICS_RESERVED_MASK                                             0x00000000
#define    CAM_ICS_ICOPC_SHIFT                                            16
#define    CAM_ICS_ICOPC_MASK                                             0xFFFF0000
#define    CAM_ICS_ICIPC_SHIFT                                            0
#define    CAM_ICS_ICIPC_MASK                                             0x0000FFFF

#define CAM_IDC_OFFSET                                                    0x00000138
#define CAM_IDC_TYPE                                                      UInt32
#define CAM_IDC_RESERVED_MASK                                             0x00000000
#define    CAM_IDC_IDLL_SHIFT                                             16
#define    CAM_IDC_IDLL_MASK                                              0xFFFF0000
#define    CAM_IDC_IDSF_SHIFT                                             0
#define    CAM_IDC_IDSF_MASK                                              0x0000FFFF

#define CAM_IDPO_OFFSET                                                   0x0000013C
#define CAM_IDPO_TYPE                                                     UInt32
#define CAM_IDPO_RESERVED_MASK                                            0x00000000
#define    CAM_IDPO_IDOPO_SHIFT                                           16
#define    CAM_IDPO_IDOPO_MASK                                            0xFFFF0000
#define    CAM_IDPO_IDSO_SHIFT                                            0
#define    CAM_IDPO_IDSO_MASK                                             0x0000FFFF

#define CAM_IDCA_OFFSET                                                   0x00000140
#define CAM_IDCA_TYPE                                                     UInt32
#define CAM_IDCA_RESERVED_MASK                                            0xFFFFFF00
#define    CAM_IDCA_IDCA_SHIFT                                            0
#define    CAM_IDCA_IDCA_MASK                                             0x000000FF

#define CAM_IDCD_OFFSET                                                   0x00000144
#define CAM_IDCD_TYPE                                                     UInt32
#define CAM_IDCD_RESERVED_MASK                                            0xE000E000
#define    CAM_IDCD_IDCDB_SHIFT                                           16
#define    CAM_IDCD_IDCDB_MASK                                            0x1FFF0000
#define    CAM_IDCD_IDCDA_SHIFT                                           0
#define    CAM_IDCD_IDCDA_MASK                                            0x00001FFF

#define CAM_IDS_OFFSET                                                    0x00000148
#define CAM_IDS_TYPE                                                      UInt32
#define CAM_IDS_RESERVED_MASK                                             0x00000000
#define    CAM_IDS_IDOPC_SHIFT                                            16
#define    CAM_IDS_IDOPC_MASK                                             0xFFFF0000
#define    CAM_IDS_IDIPC_SHIFT                                            0
#define    CAM_IDS_IDIPC_MASK                                             0x0000FFFF

#define CAM_DCS_OFFSET                                                    0x00000200
#define CAM_DCS_TYPE                                                      UInt32
#define CAM_DCS_RESERVED_MASK                                             0xFFFF00C0
#define    CAM_DCS_EDL_SHIFT                                              8
#define    CAM_DCS_EDL_MASK                                               0x0000FF00
#define    CAM_DCS_LDP_SHIFT                                              5
#define    CAM_DCS_LDP_MASK                                               0x00000020
#define    CAM_DCS_FDE_SHIFT                                              4
#define    CAM_DCS_FDE_MASK                                               0x00000010
#define    CAM_DCS_DBOB_SHIFT                                             3
#define    CAM_DCS_DBOB_MASK                                              0x00000008
#define    CAM_DCS_DI_SHIFT                                               2
#define    CAM_DCS_DI_MASK                                                0x00000004
#define    CAM_DCS_DIM_SHIFT                                              1
#define    CAM_DCS_DIM_MASK                                               0x00000002
#define    CAM_DCS_DIE_SHIFT                                              0
#define    CAM_DCS_DIE_MASK                                               0x00000001

#define CAM_DBSA_OFFSET                                                   0x00000204
#define CAM_DBSA_TYPE                                                     UInt32
#define CAM_DBSA_RESERVED_MASK                                            0x00000000
#define    CAM_DBSA_DBSA_SHIFT                                            0
#define    CAM_DBSA_DBSA_MASK                                             0xFFFFFFFF

#define CAM_DBEA_OFFSET                                                   0x00000208
#define CAM_DBEA_TYPE                                                     UInt32
#define CAM_DBEA_RESERVED_MASK                                            0x00000000
#define    CAM_DBEA_DBEA_SHIFT                                            0
#define    CAM_DBEA_DBEA_MASK                                             0xFFFFFFFF

#define CAM_DBWP_OFFSET                                                   0x0000020C
#define CAM_DBWP_TYPE                                                     UInt32
#define CAM_DBWP_RESERVED_MASK                                            0x00000000
#define    CAM_DBWP_DBWP_SHIFT                                            0
#define    CAM_DBWP_DBWP_MASK                                             0xFFFFFFFF

#define CAM_DBCTL_OFFSET                                                  0x00000300
#define CAM_DBCTL_TYPE                                                    UInt32
#define CAM_DBCTL_RESERVED_MASK                                           0xFFFFFFF8
#define    CAM_DBCTL_BUF1_IE_SHIFT                                        2
#define    CAM_DBCTL_BUF1_IE_MASK                                         0x00000004
#define    CAM_DBCTL_BUF0_IE_SHIFT                                        1
#define    CAM_DBCTL_BUF0_IE_MASK                                         0x00000002
#define    CAM_DBCTL_DB_EN_SHIFT                                          0
#define    CAM_DBCTL_DB_EN_MASK                                           0x00000001

#define CAM_IBSA1_OFFSET                                                  0x00000304
#define CAM_IBSA1_TYPE                                                    UInt32
#define CAM_IBSA1_RESERVED_MASK                                           0x00000000
#define    CAM_IBSA1_IBSA1_SHIFT                                          0
#define    CAM_IBSA1_IBSA1_MASK                                           0xFFFFFFFF

#define CAM_IBEA1_OFFSET                                                  0x00000308
#define CAM_IBEA1_TYPE                                                    UInt32
#define CAM_IBEA1_RESERVED_MASK                                           0x00000000
#define    CAM_IBEA1_IBEA1_SHIFT                                          0
#define    CAM_IBEA1_IBEA1_MASK                                           0xFFFFFFFF

#define CAM_IDI1_OFFSET                                                   0x0000030C
#define CAM_IDI1_TYPE                                                     UInt32
#define CAM_IDI1_RESERVED_MASK                                            0x00000000
#define    CAM_IDI1_IDI7_SHIFT                                            24
#define    CAM_IDI1_IDI7_MASK                                             0xFF000000
#define    CAM_IDI1_IDI6_SHIFT                                            16
#define    CAM_IDI1_IDI6_MASK                                             0x00FF0000
#define    CAM_IDI1_IDI5_SHIFT                                            8
#define    CAM_IDI1_IDI5_MASK                                             0x0000FF00
#define    CAM_IDI1_IDI4_SHIFT                                            0
#define    CAM_IDI1_IDI4_MASK                                             0x000000FF

#define CAM_DBSA1_OFFSET                                                  0x00000310
#define CAM_DBSA1_TYPE                                                    UInt32
#define CAM_DBSA1_RESERVED_MASK                                           0x00000000
#define    CAM_DBSA1_DBSA1_SHIFT                                          0
#define    CAM_DBSA1_DBSA1_MASK                                           0xFFFFFFFF

#define CAM_DBEA1_OFFSET                                                  0x00000314
#define CAM_DBEA1_TYPE                                                    UInt32
#define CAM_DBEA1_RESERVED_MASK                                           0x00000000
#define    CAM_DBEA1_DBEA1_SHIFT                                          0
#define    CAM_DBEA1_DBEA1_MASK                                           0xFFFFFFFF

#define CAM_MISC_OFFSET                                                   0x00000400
#define CAM_MISC_TYPE                                                     UInt32
#define CAM_MISC_RESERVED_MASK                                            0xFFFC0008
#define    CAM_MISC_CPI_SELECT_SHIFT                                      17
#define    CAM_MISC_CPI_SELECT_MASK                                       0x00020000
#define    CAM_MISC_AXI_BURST_CTRL_SHIFT                                  15
#define    CAM_MISC_AXI_BURST_CTRL_MASK                                   0x00018000
#define    CAM_MISC_FSP_FLUSH_SHIFT                                       10
#define    CAM_MISC_FSP_FLUSH_MASK                                        0x00007C00
#define    CAM_MISC_FIFO_FLUSH_SHIFT                                      5
#define    CAM_MISC_FIFO_FLUSH_MASK                                       0x000003E0
#define    CAM_MISC_DIS_DB_IE_SHIFT                                       4
#define    CAM_MISC_DIS_DB_IE_MASK                                        0x00000010
#define    CAM_MISC_PASS_LS_SHIFT                                         2
#define    CAM_MISC_PASS_LS_MASK                                          0x00000004
#define    CAM_MISC_PACK_FLUSH_SHIFT                                      0
#define    CAM_MISC_PACK_FLUSH_MASK                                       0x00000003

#endif /* __BRCM_RDB_CAM_H__ */



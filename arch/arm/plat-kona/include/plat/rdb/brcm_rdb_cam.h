/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2009  Broadcom Corporation                                                        */
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
/************************************************************************************************/

#ifndef __BRCM_RDB_CAMINTF_H__
#define __BRCM_RDB_CAMINTF_H__

#define CAMINTF_CPIS_OFFSET                                               0x00000C00
#define CAMINTF_CPIS_TYPE                                                 UInt32
#define CAMINTF_CPIS_RESERVED_MASK                                        0x3FFF3FC0
#define    CAMINTF_CPIS_REGF_SHIFT                                        31
#define    CAMINTF_CPIS_REGF_MASK                                         0x80000000
#define    CAMINTF_CPIS_HERR_SHIFT                                        30
#define    CAMINTF_CPIS_HERR_MASK                                         0x40000000
#define    CAMINTF_CPIS_REGM_SHIFT                                        15
#define    CAMINTF_CPIS_REGM_MASK                                         0x00008000
#define    CAMINTF_CPIS_HERRM_SHIFT                                       14
#define    CAMINTF_CPIS_HERRM_MASK                                        0x00004000
#define    CAMINTF_CPIS_SRST_SHIFT                                        5
#define    CAMINTF_CPIS_SRST_MASK                                         0x00000020
#define    CAMINTF_CPIS_DACT_SHIFT                                        4
#define    CAMINTF_CPIS_DACT_MASK                                         0x00000010
#define    CAMINTF_CPIS_SHOT_SHIFT                                        3
#define    CAMINTF_CPIS_SHOT_MASK                                         0x00000008
#define    CAMINTF_CPIS_CAPT_SHIFT                                        2
#define    CAMINTF_CPIS_CAPT_MASK                                         0x00000004
#define    CAMINTF_CPIS_ACT_SHIFT                                         1
#define    CAMINTF_CPIS_ACT_MASK                                          0x00000002
#define    CAMINTF_CPIS_ENB_SHIFT                                         0
#define    CAMINTF_CPIS_ENB_MASK                                          0x00000001

#define CAMINTF_CPIR_OFFSET                                               0x00000C04
#define CAMINTF_CPIR_TYPE                                                 UInt32
#define CAMINTF_CPIR_RESERVED_MASK                                        0xFFFFE000
#define    CAMINTF_CPIR_BITSHIFT_SHIFT                                    9
#define    CAMINTF_CPIR_BITSHIFT_MASK                                     0x00001E00
#define    CAMINTF_CPIR_BITWIDTH_SHIFT                                    7
#define    CAMINTF_CPIR_BITWIDTH_MASK                                     0x00000180
#define    CAMINTF_CPIR_HSYNC_SHIFT                                       6
#define    CAMINTF_CPIR_HSYNC_MASK                                        0x00000040
#define    CAMINTF_CPIR_VSYNC_SHIFT                                       5
#define    CAMINTF_CPIR_VSYNC_MASK                                        0x00000020
#define    CAMINTF_CPIR_DSRM_SHIFT                                        4
#define    CAMINTF_CPIR_DSRM_MASK                                         0x00000010
#define    CAMINTF_CPIR_HSRM_SHIFT                                        3
#define    CAMINTF_CPIR_HSRM_MASK                                         0x00000008
#define    CAMINTF_CPIR_VSRM_SHIFT                                        2
#define    CAMINTF_CPIR_VSRM_MASK                                         0x00000004
#define    CAMINTF_CPIR_HSAL_SHIFT                                        1
#define    CAMINTF_CPIR_HSAL_MASK                                         0x00000002
#define    CAMINTF_CPIR_VSAL_SHIFT                                        0
#define    CAMINTF_CPIR_VSAL_MASK                                         0x00000001

#define CAMINTF_CPIF_OFFSET                                               0x00000C08
#define CAMINTF_CPIF_TYPE                                                 UInt32
#define CAMINTF_CPIF_RESERVED_MASK                                        0xFFFFF800
#define    CAMINTF_CPIF_FDETECT_SHIFT                                     10
#define    CAMINTF_CPIF_FDETECT_MASK                                      0x00000400
#define    CAMINTF_CPIF_STATE_SHIFT                                       7
#define    CAMINTF_CPIF_STATE_MASK                                        0x00000380
#define    CAMINTF_CPIF_FMODE_SHIFT                                       5
#define    CAMINTF_CPIF_FMODE_MASK                                        0x00000060
#define    CAMINTF_CPIF_HMODE_SHIFT                                       3
#define    CAMINTF_CPIF_HMODE_MASK                                        0x00000018
#define    CAMINTF_CPIF_VMODE_SHIFT                                       1
#define    CAMINTF_CPIF_VMODE_MASK                                        0x00000006
#define    CAMINTF_CPIF_SMODE_SHIFT                                       0
#define    CAMINTF_CPIF_SMODE_MASK                                        0x00000001

#define CAMINTF_CPIW_OFFSET                                               0x00000C0C
#define CAMINTF_CPIW_TYPE                                                 UInt32
#define CAMINTF_CPIW_RESERVED_MASK                                        0xFFFFFFF8
#define    CAMINTF_CPIW_FGATE_SHIFT                                       1
#define    CAMINTF_CPIW_FGATE_MASK                                        0x00000006
#define    CAMINTF_CPIW_ENB_SHIFT                                         0
#define    CAMINTF_CPIW_ENB_MASK                                          0x00000001

#define CAMINTF_CPIWVC_OFFSET                                             0x00000C10
#define CAMINTF_CPIWVC_TYPE                                               UInt32
#define CAMINTF_CPIWVC_RESERVED_MASK                                      0x00000000
#define    CAMINTF_CPIWVC_VACTIVE_SHIFT                                   16
#define    CAMINTF_CPIWVC_VACTIVE_MASK                                    0xFFFF0000
#define    CAMINTF_CPIWVC_VFRONT_SHIFT                                    0
#define    CAMINTF_CPIWVC_VFRONT_MASK                                     0x0000FFFF

#define CAMINTF_CPIWVS_OFFSET                                             0x00000C14
#define CAMINTF_CPIWVS_TYPE                                               UInt32
#define CAMINTF_CPIWVS_RESERVED_MASK                                      0x00000000
#define    CAMINTF_CPIWVS_VLINE_SHIFT                                     16
#define    CAMINTF_CPIWVS_VLINE_MASK                                      0xFFFF0000
#define    CAMINTF_CPIWVS_VBACK_SHIFT                                     0
#define    CAMINTF_CPIWVS_VBACK_MASK                                      0x0000FFFF

#define CAMINTF_CPIWHC_OFFSET                                             0x00000C18
#define CAMINTF_CPIWHC_TYPE                                               UInt32
#define CAMINTF_CPIWHC_RESERVED_MASK                                      0x00000000
#define    CAMINTF_CPIWHC_HACTIVE_SHIFT                                   16
#define    CAMINTF_CPIWHC_HACTIVE_MASK                                    0xFFFF0000
#define    CAMINTF_CPIWHC_HFRONT_SHIFT                                    0
#define    CAMINTF_CPIWHC_HFRONT_MASK                                     0x0000FFFF

#define CAMINTF_CPIWHS_OFFSET                                             0x00000C1C
#define CAMINTF_CPIWHS_TYPE                                               UInt32
#define CAMINTF_CPIWHS_RESERVED_MASK                                      0x00000000
#define    CAMINTF_CPIWHS_HLINE_SHIFT                                     16
#define    CAMINTF_CPIWHS_HLINE_MASK                                      0xFFFF0000
#define    CAMINTF_CPIWHS_HBACK_SHIFT                                     0
#define    CAMINTF_CPIWHS_HBACK_MASK                                      0x0000FFFF

#define CAMINTF_CPIPIB_OFFSET                                             0x00000C24
#define CAMINTF_CPIPIB_TYPE                                               UInt32
#define CAMINTF_CPIPIB_RESERVED_MASK                                      0xFFFFFFF0
#define    CAMINTF_CPIPIB_CHANC_SHIFT                                     3
#define    CAMINTF_CPIPIB_CHANC_MASK                                      0x00000008
#define    CAMINTF_CPIPIB_LINEC_SHIFT                                     0
#define    CAMINTF_CPIPIB_LINEC_MASK                                      0x00000007

#define CAMINTF_CCP2RC_OFFSET                                             0x00000800
#define CAMINTF_CCP2RC_TYPE                                               UInt32
#define CAMINTF_CCP2RC_RESERVED_MASK                                      0x00000040
#define    CAMINTF_CCP2RC_CTATADJ_SHIFT                                   28
#define    CAMINTF_CCP2RC_CTATADJ_MASK                                    0xF0000000
#define    CAMINTF_CCP2RC_PTATADJ_SHIFT                                   24
#define    CAMINTF_CCP2RC_PTATADJ_MASK                                    0x0F000000
#define    CAMINTF_CCP2RC_BPD_SHIFT                                       23
#define    CAMINTF_CCP2RC_BPD_MASK                                        0x00800000
#define    CAMINTF_CCP2RC_APD_SHIFT                                       22
#define    CAMINTF_CCP2RC_APD_MASK                                        0x00400000
#define    CAMINTF_CCP2RC_ARST_SHIFT                                      21
#define    CAMINTF_CCP2RC_ARST_MASK                                       0x00200000
#define    CAMINTF_CCP2RC_CLAC_SHIFT                                      17
#define    CAMINTF_CCP2RC_CLAC_MASK                                       0x001E0000
#define    CAMINTF_CCP2RC_DLAC_SHIFT                                      13
#define    CAMINTF_CCP2RC_DLAC_MASK                                       0x0001E000
#define    CAMINTF_CCP2RC_CLK_IDR_SHIFT                                   10
#define    CAMINTF_CCP2RC_CLK_IDR_MASK                                    0x00001C00
#define    CAMINTF_CCP2RC_DATA_IDR_SHIFT                                  7
#define    CAMINTF_CCP2RC_DATA_IDR_MASK                                   0x00000380
#define    CAMINTF_CCP2RC_SWR_SHIFT                                       5
#define    CAMINTF_CCP2RC_SWR_MASK                                        0x00000020
#define    CAMINTF_CCP2RC_RSYN_SHIFT                                      4
#define    CAMINTF_CCP2RC_RSYN_MASK                                       0x00000010
#define    CAMINTF_CCP2RC_CLKM_SHIFT                                      3
#define    CAMINTF_CCP2RC_CLKM_MASK                                       0x00000008
#define    CAMINTF_CCP2RC_INMP_SHIFT                                      2
#define    CAMINTF_CCP2RC_INMP_MASK                                       0x00000004
#define    CAMINTF_CCP2RC_TREN_SHIFT                                      1
#define    CAMINTF_CCP2RC_TREN_MASK                                       0x00000002
#define    CAMINTF_CCP2RC_RXEN_SHIFT                                      0
#define    CAMINTF_CCP2RC_RXEN_MASK                                       0x00000001

#define CAMINTF_CCP2RS_OFFSET                                             0x00000804
#define CAMINTF_CCP2RS_TYPE                                               UInt32
#define CAMINTF_CCP2RS_RESERVED_MASK                                      0xFFFCFE00
#define    CAMINTF_CCP2RS_IS1_SHIFT                                       17
#define    CAMINTF_CCP2RS_IS1_MASK                                        0x00020000
#define    CAMINTF_CCP2RS_IS0_SHIFT                                       16
#define    CAMINTF_CCP2RS_IS0_MASK                                        0x00010000
#define    CAMINTF_CCP2RS_IDSO_SHIFT                                      8
#define    CAMINTF_CCP2RS_IDSO_MASK                                       0x00000100
#define    CAMINTF_CCP2RS_IFO_SHIFT                                       7
#define    CAMINTF_CCP2RS_IFO_MASK                                        0x00000080
#define    CAMINTF_CCP2RS_OFO_SHIFT                                       6
#define    CAMINTF_CCP2RS_OFO_MASK                                        0x00000040
#define    CAMINTF_CCP2RS_OFP_SHIFT                                       5
#define    CAMINTF_CCP2RS_OFP_MASK                                        0x00000020
#define    CAMINTF_CCP2RS_FSC_SHIFT                                       4
#define    CAMINTF_CCP2RS_FSC_MASK                                        0x00000010
#define    CAMINTF_CCP2RS_SSC_SHIFT                                       3
#define    CAMINTF_CCP2RS_SSC_MASK                                        0x00000008
#define    CAMINTF_CCP2RS_OEB_SHIFT                                       2
#define    CAMINTF_CCP2RS_OEB_MASK                                        0x00000004
#define    CAMINTF_CCP2RS_SYN_SHIFT                                       1
#define    CAMINTF_CCP2RS_SYN_MASK                                        0x00000002
#define    CAMINTF_CCP2RS_GEF_SHIFT                                       0
#define    CAMINTF_CCP2RS_GEF_MASK                                        0x00000001

#define CAMINTF_CCP2RP_OFFSET                                             0x00000808
#define CAMINTF_CCP2RP_TYPE                                               UInt32
#define CAMINTF_CCP2RP_RESERVED_MASK                                      0xFFFFF000
#define    CAMINTF_CCP2RP_RPT_SHIFT                                       8
#define    CAMINTF_CCP2RP_RPT_MASK                                        0x00000F00
#define    CAMINTF_CCP2RP_RRP_SHIFT                                       4
#define    CAMINTF_CCP2RP_RRP_MASK                                        0x000000F0
#define    CAMINTF_CCP2RP_RNP_SHIFT                                       0
#define    CAMINTF_CCP2RP_RNP_MASK                                        0x0000000F

#define CAMINTF_CCP2RDR1_OFFSET                                           0x00000880
#define CAMINTF_CCP2RDR1_TYPE                                             UInt32
#define CAMINTF_CCP2RDR1_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2RDR1_ITT_SHIFT                                     29
#define    CAMINTF_CCP2RDR1_ITT_MASK                                      0x20000000
#define    CAMINTF_CCP2RDR1_LNUM0_SHIFT                                   16
#define    CAMINTF_CCP2RDR1_LNUM0_MASK                                    0x1FFF0000
#define    CAMINTF_CCP2RDR1_SFS_SHIFT                                     13
#define    CAMINTF_CCP2RDR1_SFS_MASK                                      0x0000E000
#define    CAMINTF_CCP2RDR1_CDMAC_SHIFT                                   9
#define    CAMINTF_CCP2RDR1_CDMAC_MASK                                    0x00001E00
#define    CAMINTF_CCP2RDR1_CLC_SHIFT                                     5
#define    CAMINTF_CCP2RDR1_CLC_MASK                                      0x000001E0
#define    CAMINTF_CCP2RDR1_BA_SHIFT                                      2
#define    CAMINTF_CCP2RDR1_BA_MASK                                       0x0000001C
#define    CAMINTF_CCP2RDR1_BAS_SHIFT                                     0
#define    CAMINTF_CCP2RDR1_BAS_MASK                                      0x00000003

#define CAMINTF_CCP2RDR2_OFFSET                                           0x00000884
#define CAMINTF_CCP2RDR2_TYPE                                             UInt32
#define CAMINTF_CCP2RDR2_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CCP2RDR2_ICV0_SHIFT                                    31
#define    CAMINTF_CCP2RDR2_ICV0_MASK                                     0x80000000
#define    CAMINTF_CCP2RDR2_ICS0_SHIFT                                    28
#define    CAMINTF_CCP2RDR2_ICS0_MASK                                     0x70000000
#define    CAMINTF_CCP2RDR2_BTPS0_SHIFT                                   26
#define    CAMINTF_CCP2RDR2_BTPS0_MASK                                    0x0C000000
#define    CAMINTF_CCP2RDR2_IFEL0_SHIFT                                   13
#define    CAMINTF_CCP2RDR2_IFEL0_MASK                                    0x03FFE000
#define    CAMINTF_CCP2RDR2_IFSL0_SHIFT                                   0
#define    CAMINTF_CCP2RDR2_IFSL0_MASK                                    0x00001FFF

#define CAMINTF_CCP2RDR3_OFFSET                                           0x00000888
#define CAMINTF_CCP2RDR3_TYPE                                             UInt32
#define CAMINTF_CCP2RDR3_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CCP2RDR3_OEBFC_SHIFT                                   26
#define    CAMINTF_CCP2RDR3_OEBFC_MASK                                    0xFC000000
#define    CAMINTF_CCP2RDR3_OEDFC_SHIFT                                   18
#define    CAMINTF_CCP2RDR3_OEDFC_MASK                                    0x03FC0000
#define    CAMINTF_CCP2RDR3_OEWS_SHIFT                                    16
#define    CAMINTF_CCP2RDR3_OEWS_MASK                                     0x00030000
#define    CAMINTF_CCP2RDR3_FFDS0_SHIFT                                   12
#define    CAMINTF_CCP2RDR3_FFDS0_MASK                                    0x0000F000
#define    CAMINTF_CCP2RDR3_IC0_SHIFT                                     0
#define    CAMINTF_CCP2RDR3_IC0_MASK                                      0x00000FFF

#define CAMINTF_CCP2RDR4_OFFSET                                           0x0000088C
#define CAMINTF_CCP2RDR4_TYPE                                             UInt32
#define CAMINTF_CCP2RDR4_RESERVED_MASK                                    0xFFFFC000
#define    CAMINTF_CCP2RDR4_OEBFP_SHIFT                                   8
#define    CAMINTF_CCP2RDR4_OEBFP_MASK                                    0x00003F00
#define    CAMINTF_CCP2RDR4_OEDFP_SHIFT                                   0
#define    CAMINTF_CCP2RDR4_OEDFP_MASK                                    0x000000FF

#define CAMINTF_CCP2RC0_OFFSET                                            0x00000900
#define CAMINTF_CCP2RC0_TYPE                                              UInt32
#define CAMINTF_CCP2RC0_RESERVED_MASK                                     0x2000F080
#define    CAMINTF_CCP2RC0_TRIG_SHIFT                                     31
#define    CAMINTF_CCP2RC0_TRIG_MASK                                      0x80000000
#define    CAMINTF_CCP2RC0_FCM_SHIFT                                      30
#define    CAMINTF_CCP2RC0_FCM_MASK                                       0x40000000
#define    CAMINTF_CCP2RC0_LCIE_SHIFT                                     16
#define    CAMINTF_CCP2RC0_LCIE_MASK                                      0x1FFF0000
#define    CAMINTF_CCP2RC0_LEIE_SHIFT                                     11
#define    CAMINTF_CCP2RC0_LEIE_MASK                                      0x00000800
#define    CAMINTF_CCP2RC0_LSIE_SHIFT                                     10
#define    CAMINTF_CCP2RC0_LSIE_MASK                                      0x00000400
#define    CAMINTF_CCP2RC0_FEIE_SHIFT                                     9
#define    CAMINTF_CCP2RC0_FEIE_MASK                                      0x00000200
#define    CAMINTF_CCP2RC0_FSIE_SHIFT                                     8
#define    CAMINTF_CCP2RC0_FSIE_MASK                                      0x00000100
#define    CAMINTF_CCP2RC0_EDT_SHIFT                                      4
#define    CAMINTF_CCP2RC0_EDT_MASK                                       0x00000070
#define    CAMINTF_CCP2RC0_LCN_SHIFT                                      1
#define    CAMINTF_CCP2RC0_LCN_MASK                                       0x0000000E
#define    CAMINTF_CCP2RC0_CHEN_SHIFT                                     0
#define    CAMINTF_CCP2RC0_CHEN_MASK                                      0x00000001

#define CAMINTF_CCP2RPC0_OFFSET                                           0x00000904
#define CAMINTF_CCP2RPC0_TYPE                                             UInt32
#define CAMINTF_CCP2RPC0_RESERVED_MASK                                    0xFE00E8E0
#define    CAMINTF_CCP2RPC0_EBL_SHIFT                                     16
#define    CAMINTF_CCP2RPC0_EBL_MASK                                      0x01FF0000
#define    CAMINTF_CCP2RPC0_EAP_SHIFT                                     12
#define    CAMINTF_CCP2RPC0_EAP_MASK                                      0x00001000
#define    CAMINTF_CCP2RPC0_EP_SHIFT                                      8
#define    CAMINTF_CCP2RPC0_EP_MASK                                       0x00000700
#define    CAMINTF_CCP2RPC0_DAP_SHIFT                                     4
#define    CAMINTF_CCP2RPC0_DAP_MASK                                      0x00000010
#define    CAMINTF_CCP2RPC0_DP_SHIFT                                      0
#define    CAMINTF_CCP2RPC0_DP_MASK                                       0x0000000F

#define CAMINTF_CCP2RS0_OFFSET                                            0x00000908
#define CAMINTF_CCP2RS0_TYPE                                              UInt32
#define CAMINTF_CCP2RS0_RESERVED_MASK                                     0x00FFE0C0
#define    CAMINTF_CCP2RS0_FNUM_SHIFT                                     24
#define    CAMINTF_CCP2RS0_FNUM_MASK                                      0xFF000000
#define    CAMINTF_CCP2RS0_UMT_SHIFT                                      12
#define    CAMINTF_CCP2RS0_UMT_MASK                                       0x00001000
#define    CAMINTF_CCP2RS0_UDF_SHIFT                                      11
#define    CAMINTF_CCP2RS0_UDF_MASK                                       0x00000800
#define    CAMINTF_CCP2RS0_DBO_SHIFT                                      10
#define    CAMINTF_CCP2RS0_DBO_MASK                                       0x00000400
#define    CAMINTF_CCP2RS0_IBO_SHIFT                                      9
#define    CAMINTF_CCP2RS0_IBO_MASK                                       0x00000200
#define    CAMINTF_CCP2RS0_CRCE_SHIFT                                     8
#define    CAMINTF_CCP2RS0_CRCE_MASK                                      0x00000100
#define    CAMINTF_CCP2RS0_LCI_SHIFT                                      5
#define    CAMINTF_CCP2RS0_LCI_MASK                                       0x00000020
#define    CAMINTF_CCP2RS0_LEI_SHIFT                                      4
#define    CAMINTF_CCP2RS0_LEI_MASK                                       0x00000010
#define    CAMINTF_CCP2RS0_LSI_SHIFT                                      3
#define    CAMINTF_CCP2RS0_LSI_MASK                                       0x00000008
#define    CAMINTF_CCP2RS0_FEI_SHIFT                                      2
#define    CAMINTF_CCP2RS0_FEI_MASK                                       0x00000004
#define    CAMINTF_CCP2RS0_FSI_SHIFT                                      1
#define    CAMINTF_CCP2RS0_FSI_MASK                                       0x00000002
#define    CAMINTF_CCP2RS0_CHB_SHIFT                                      0
#define    CAMINTF_CCP2RS0_CHB_MASK                                       0x00000001

#define CAMINTF_CCP2RSA0_OFFSET                                           0x0000090C
#define CAMINTF_CCP2RSA0_TYPE                                             UInt32
#define CAMINTF_CCP2RSA0_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CCP2RSA0_SA_SHIFT                                      0
#define    CAMINTF_CCP2RSA0_SA_MASK                                       0xFFFFFFFF

#define CAMINTF_CCP2REA0_OFFSET                                           0x00000910
#define CAMINTF_CCP2REA0_TYPE                                             UInt32
#define CAMINTF_CCP2REA0_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CCP2REA0_EA_SHIFT                                      0
#define    CAMINTF_CCP2REA0_EA_MASK                                       0xFFFFFFFF

#define CAMINTF_CCP2RWP0_OFFSET                                           0x00000914
#define CAMINTF_CCP2RWP0_TYPE                                             UInt32
#define CAMINTF_CCP2RWP0_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2RWP0_WPTR_SHIFT                                    0
#define    CAMINTF_CCP2RWP0_WPTR_MASK                                     0x3FFFFFFF

#define CAMINTF_CCP2RBC0_OFFSET                                           0x00000918
#define CAMINTF_CCP2RBC0_TYPE                                             UInt32
#define CAMINTF_CCP2RBC0_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2RBC0_BCTR_SHIFT                                    0
#define    CAMINTF_CCP2RBC0_BCTR_MASK                                     0x3FFFFFFF

#define CAMINTF_CCP2RLS0_OFFSET                                           0x0000091C
#define CAMINTF_CCP2RLS0_TYPE                                             UInt32
#define CAMINTF_CCP2RLS0_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CCP2RLS0_LSTR_SHIFT                                    0
#define    CAMINTF_CCP2RLS0_LSTR_MASK                                     0x0000FFFF

#define CAMINTF_CCP2RDSA0_OFFSET                                          0x00000920
#define CAMINTF_CCP2RDSA0_TYPE                                            UInt32
#define CAMINTF_CCP2RDSA0_RESERVED_MASK                                   0x00000000
#define    CAMINTF_CCP2RDSA0_DSA_SHIFT                                    0
#define    CAMINTF_CCP2RDSA0_DSA_MASK                                     0xFFFFFFFF

#define CAMINTF_CCP2RDEA0_OFFSET                                          0x00000924
#define CAMINTF_CCP2RDEA0_TYPE                                            UInt32
#define CAMINTF_CCP2RDEA0_RESERVED_MASK                                   0x00000000
#define    CAMINTF_CCP2RDEA0_DEA_SHIFT                                    0
#define    CAMINTF_CCP2RDEA0_DEA_MASK                                     0xFFFFFFFF

#define CAMINTF_CCP2RDS0_OFFSET                                           0x00000928
#define CAMINTF_CCP2RDS0_TYPE                                             UInt32
#define CAMINTF_CCP2RDS0_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CCP2RDS0_DSTR_SHIFT                                    0
#define    CAMINTF_CCP2RDS0_DSTR_MASK                                     0x0000FFFF

#define CAMINTF_CCP2RC1_OFFSET                                            0x00000A00
#define CAMINTF_CCP2RC1_TYPE                                              UInt32
#define CAMINTF_CCP2RC1_RESERVED_MASK                                     0x2000F080
#define    CAMINTF_CCP2RC1_TRIG_SHIFT                                     31
#define    CAMINTF_CCP2RC1_TRIG_MASK                                      0x80000000
#define    CAMINTF_CCP2RC1_FCM_SHIFT                                      30
#define    CAMINTF_CCP2RC1_FCM_MASK                                       0x40000000
#define    CAMINTF_CCP2RC1_LCIE_SHIFT                                     16
#define    CAMINTF_CCP2RC1_LCIE_MASK                                      0x1FFF0000
#define    CAMINTF_CCP2RC1_LEIE_SHIFT                                     11
#define    CAMINTF_CCP2RC1_LEIE_MASK                                      0x00000800
#define    CAMINTF_CCP2RC1_LSIE_SHIFT                                     10
#define    CAMINTF_CCP2RC1_LSIE_MASK                                      0x00000400
#define    CAMINTF_CCP2RC1_FEIE_SHIFT                                     9
#define    CAMINTF_CCP2RC1_FEIE_MASK                                      0x00000200
#define    CAMINTF_CCP2RC1_FSIE_SHIFT                                     8
#define    CAMINTF_CCP2RC1_FSIE_MASK                                      0x00000100
#define    CAMINTF_CCP2RC1_EDT_SHIFT                                      4
#define    CAMINTF_CCP2RC1_EDT_MASK                                       0x00000070
#define    CAMINTF_CCP2RC1_LCN_SHIFT                                      1
#define    CAMINTF_CCP2RC1_LCN_MASK                                       0x0000000E
#define    CAMINTF_CCP2RC1_CHEN_SHIFT                                     0
#define    CAMINTF_CCP2RC1_CHEN_MASK                                      0x00000001

#define CAMINTF_CCP2RPC1_OFFSET                                           0x00000A04
#define CAMINTF_CCP2RPC1_TYPE                                             UInt32
#define CAMINTF_CCP2RPC1_RESERVED_MASK                                    0xFE00E8E0
#define    CAMINTF_CCP2RPC1_EBL_SHIFT                                     16
#define    CAMINTF_CCP2RPC1_EBL_MASK                                      0x01FF0000
#define    CAMINTF_CCP2RPC1_EAP_SHIFT                                     12
#define    CAMINTF_CCP2RPC1_EAP_MASK                                      0x00001000
#define    CAMINTF_CCP2RPC1_EP_SHIFT                                      8
#define    CAMINTF_CCP2RPC1_EP_MASK                                       0x00000700
#define    CAMINTF_CCP2RPC1_DAP_SHIFT                                     4
#define    CAMINTF_CCP2RPC1_DAP_MASK                                      0x00000010
#define    CAMINTF_CCP2RPC1_DP_SHIFT                                      0
#define    CAMINTF_CCP2RPC1_DP_MASK                                       0x0000000F

#define CAMINTF_CCP2RS1_OFFSET                                            0x00000A08
#define CAMINTF_CCP2RS1_TYPE                                              UInt32
#define CAMINTF_CCP2RS1_RESERVED_MASK                                     0x00FFE0C0
#define    CAMINTF_CCP2RS1_FNUM_SHIFT                                     24
#define    CAMINTF_CCP2RS1_FNUM_MASK                                      0xFF000000
#define    CAMINTF_CCP2RS1_UMT_SHIFT                                      12
#define    CAMINTF_CCP2RS1_UMT_MASK                                       0x00001000
#define    CAMINTF_CCP2RS1_UDF_SHIFT                                      11
#define    CAMINTF_CCP2RS1_UDF_MASK                                       0x00000800
#define    CAMINTF_CCP2RS1_DBO_SHIFT                                      10
#define    CAMINTF_CCP2RS1_DBO_MASK                                       0x00000400
#define    CAMINTF_CCP2RS1_IBO_SHIFT                                      9
#define    CAMINTF_CCP2RS1_IBO_MASK                                       0x00000200
#define    CAMINTF_CCP2RS1_CRCE_SHIFT                                     8
#define    CAMINTF_CCP2RS1_CRCE_MASK                                      0x00000100
#define    CAMINTF_CCP2RS1_LCI_SHIFT                                      5
#define    CAMINTF_CCP2RS1_LCI_MASK                                       0x00000020
#define    CAMINTF_CCP2RS1_LEI_SHIFT                                      4
#define    CAMINTF_CCP2RS1_LEI_MASK                                       0x00000010
#define    CAMINTF_CCP2RS1_LSI_SHIFT                                      3
#define    CAMINTF_CCP2RS1_LSI_MASK                                       0x00000008
#define    CAMINTF_CCP2RS1_FEI_SHIFT                                      2
#define    CAMINTF_CCP2RS1_FEI_MASK                                       0x00000004
#define    CAMINTF_CCP2RS1_FSI_SHIFT                                      1
#define    CAMINTF_CCP2RS1_FSI_MASK                                       0x00000002
#define    CAMINTF_CCP2RS1_CHB_SHIFT                                      0
#define    CAMINTF_CCP2RS1_CHB_MASK                                       0x00000001

#define CAMINTF_CCP2RSA1_OFFSET                                           0x00000A0C
#define CAMINTF_CCP2RSA1_TYPE                                             UInt32
#define CAMINTF_CCP2RSA1_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2RSA1_SA_SHIFT                                      0
#define    CAMINTF_CCP2RSA1_SA_MASK                                       0x3FFFFFFF

#define CAMINTF_CCP2REA1_OFFSET                                           0x00000A10
#define CAMINTF_CCP2REA1_TYPE                                             UInt32
#define CAMINTF_CCP2REA1_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2REA1_EA_SHIFT                                      0
#define    CAMINTF_CCP2REA1_EA_MASK                                       0x3FFFFFFF

#define CAMINTF_CCP2RWP1_OFFSET                                           0x00000A14
#define CAMINTF_CCP2RWP1_TYPE                                             UInt32
#define CAMINTF_CCP2RWP1_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2RWP1_WPTR_SHIFT                                    0
#define    CAMINTF_CCP2RWP1_WPTR_MASK                                     0x3FFFFFFF

#define CAMINTF_CCP2RBC1_OFFSET                                           0x00000A18
#define CAMINTF_CCP2RBC1_TYPE                                             UInt32
#define CAMINTF_CCP2RBC1_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CCP2RBC1_BCTR_SHIFT                                    0
#define    CAMINTF_CCP2RBC1_BCTR_MASK                                     0x3FFFFFFF

#define CAMINTF_CCP2RLS1_OFFSET                                           0x00000A1C
#define CAMINTF_CCP2RLS1_TYPE                                             UInt32
#define CAMINTF_CCP2RLS1_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CCP2RLS1_LSTR_SHIFT                                    0
#define    CAMINTF_CCP2RLS1_LSTR_MASK                                     0x0000FFFF

#define CAMINTF_CCP2RDSA1_OFFSET                                          0x00000A20
#define CAMINTF_CCP2RDSA1_TYPE                                            UInt32
#define CAMINTF_CCP2RDSA1_RESERVED_MASK                                   0xC0000000
#define    CAMINTF_CCP2RDSA1_DSA_SHIFT                                    0
#define    CAMINTF_CCP2RDSA1_DSA_MASK                                     0x3FFFFFFF

#define CAMINTF_CCP2RDEA1_OFFSET                                          0x00000A24
#define CAMINTF_CCP2RDEA1_TYPE                                            UInt32
#define CAMINTF_CCP2RDEA1_RESERVED_MASK                                   0xC0000000
#define    CAMINTF_CCP2RDEA1_DEA_SHIFT                                    0
#define    CAMINTF_CCP2RDEA1_DEA_MASK                                     0x3FFFFFFF

#define CAMINTF_CCP2RDS1_OFFSET                                           0x00000A28
#define CAMINTF_CCP2RDS1_TYPE                                             UInt32
#define CAMINTF_CCP2RDS1_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CCP2RDS1_DSTR_SHIFT                                    0
#define    CAMINTF_CCP2RDS1_DSTR_MASK                                     0x0000FFFF

#define CAMINTF_CSI2RC_OFFSET                                             0x00000000
#define CAMINTF_CSI2RC_TYPE                                               UInt32
#define CAMINTF_CSI2RC_RESERVED_MASK                                      0x00003F80
#define    CAMINTF_CSI2RC_CTATADJ_SHIFT                                   28
#define    CAMINTF_CSI2RC_CTATADJ_MASK                                    0xF0000000
#define    CAMINTF_CSI2RC_PTATADJ_SHIFT                                   24
#define    CAMINTF_CSI2RC_PTATADJ_MASK                                    0x0F000000
#define    CAMINTF_CSI2RC_RPP_SHIFT                                       20
#define    CAMINTF_CSI2RC_RPP_MASK                                        0x00F00000
#define    CAMINTF_CSI2RC_RNP_SHIFT                                       19
#define    CAMINTF_CSI2RC_RNP_MASK                                        0x00080000
#define    CAMINTF_CSI2RC_BPD_SHIFT                                       18
#define    CAMINTF_CSI2RC_BPD_MASK                                        0x00040000
#define    CAMINTF_CSI2RC_APD_SHIFT                                       17
#define    CAMINTF_CSI2RC_APD_MASK                                        0x00020000
#define    CAMINTF_CSI2RC_FOE_SHIFT                                       16
#define    CAMINTF_CSI2RC_FOE_MASK                                        0x00010000
#define    CAMINTF_CSI2RC_FOF_SHIFT                                       15
#define    CAMINTF_CSI2RC_FOF_MASK                                        0x00008000
#define    CAMINTF_CSI2RC_RSYN_SHIFT                                      14
#define    CAMINTF_CSI2RC_RSYN_MASK                                       0x00004000
#define    CAMINTF_CSI2RC_F16B_SHIFT                                      6
#define    CAMINTF_CSI2RC_F16B_MASK                                       0x00000040
#define    CAMINTF_CSI2RC_GEN_SHIFT                                       5
#define    CAMINTF_CSI2RC_GEN_MASK                                        0x00000020
#define    CAMINTF_CSI2RC_LENC_SHIFT                                      4
#define    CAMINTF_CSI2RC_LENC_MASK                                       0x00000010
#define    CAMINTF_CSI2RC_LEN4_SHIFT                                      3
#define    CAMINTF_CSI2RC_LEN4_MASK                                       0x00000008
#define    CAMINTF_CSI2RC_LEN3_SHIFT                                      2
#define    CAMINTF_CSI2RC_LEN3_MASK                                       0x00000004
#define    CAMINTF_CSI2RC_LEN2_SHIFT                                      1
#define    CAMINTF_CSI2RC_LEN2_MASK                                       0x00000002
#define    CAMINTF_CSI2RC_LEN1_SHIFT                                      0
#define    CAMINTF_CSI2RC_LEN1_MASK                                       0x00000001

#define CAMINTF_CSI2RS_OFFSET                                             0x00000004
#define CAMINTF_CSI2RS_TYPE                                               UInt32
#define CAMINTF_CSI2RS_RESERVED_MASK                                      0xFFFFFCC0
#define    CAMINTF_CSI2RS_IS1_SHIFT                                       9
#define    CAMINTF_CSI2RS_IS1_MASK                                        0x00000200
#define    CAMINTF_CSI2RS_IS0_SHIFT                                       8
#define    CAMINTF_CSI2RS_IS0_MASK                                        0x00000100
#define    CAMINTF_CSI2RS_OFF_SHIFT                                       5
#define    CAMINTF_CSI2RS_OFF_MASK                                        0x00000020
#define    CAMINTF_CSI2RS_OFP_SHIFT                                       4
#define    CAMINTF_CSI2RS_OFP_MASK                                        0x00000010
#define    CAMINTF_CSI2RS_PEC_SHIFT                                       3
#define    CAMINTF_CSI2RS_PEC_MASK                                        0x00000008
#define    CAMINTF_CSI2RS_PED_SHIFT                                       2
#define    CAMINTF_CSI2RS_PED_MASK                                        0x00000004
#define    CAMINTF_CSI2RS_OEB_SHIFT                                       1
#define    CAMINTF_CSI2RS_OEB_MASK                                        0x00000002
#define    CAMINTF_CSI2RS_GEF_SHIFT                                       0
#define    CAMINTF_CSI2RS_GEF_MASK                                        0x00000001

#define CAMINTF_CSI2RDLS_OFFSET                                           0x00000008
#define CAMINTF_CSI2RDLS_TYPE                                             UInt32
#define CAMINTF_CSI2RDLS_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CSI2RDLS_CEC_SHIFT                                     29
#define    CAMINTF_CSI2RDLS_CEC_MASK                                      0x20000000
#define    CAMINTF_CSI2RDLS_ESEC_SHIFT                                    28
#define    CAMINTF_CSI2RDLS_ESEC_MASK                                     0x10000000
#define    CAMINTF_CSI2RDLS_EEEC_SHIFT                                    27
#define    CAMINTF_CSI2RDLS_EEEC_MASK                                     0x08000000
#define    CAMINTF_CSI2RDLS_SOTSEC_SHIFT                                  26
#define    CAMINTF_CSI2RDLS_SOTSEC_MASK                                   0x04000000
#define    CAMINTF_CSI2RDLS_SOTEC_SHIFT                                   25
#define    CAMINTF_CSI2RDLS_SOTEC_MASK                                    0x02000000
#define    CAMINTF_CSI2RDLS_ULPSC_SHIFT                                   24
#define    CAMINTF_CSI2RDLS_ULPSC_MASK                                    0x01000000
#define    CAMINTF_CSI2RDLS_CE4_SHIFT                                     23
#define    CAMINTF_CSI2RDLS_CE4_MASK                                      0x00800000
#define    CAMINTF_CSI2RDLS_ESE4_SHIFT                                    22
#define    CAMINTF_CSI2RDLS_ESE4_MASK                                     0x00400000
#define    CAMINTF_CSI2RDLS_EEE4_SHIFT                                    21
#define    CAMINTF_CSI2RDLS_EEE4_MASK                                     0x00200000
#define    CAMINTF_CSI2RDLS_SOTSE4_SHIFT                                  20
#define    CAMINTF_CSI2RDLS_SOTSE4_MASK                                   0x00100000
#define    CAMINTF_CSI2RDLS_SOTE4_SHIFT                                   19
#define    CAMINTF_CSI2RDLS_SOTE4_MASK                                    0x00080000
#define    CAMINTF_CSI2RDLS_ULPS4_SHIFT                                   18
#define    CAMINTF_CSI2RDLS_ULPS4_MASK                                    0x00040000
#define    CAMINTF_CSI2RDLS_CE3_SHIFT                                     17
#define    CAMINTF_CSI2RDLS_CE3_MASK                                      0x00020000
#define    CAMINTF_CSI2RDLS_ESE3_SHIFT                                    16
#define    CAMINTF_CSI2RDLS_ESE3_MASK                                     0x00010000
#define    CAMINTF_CSI2RDLS_EEE3_SHIFT                                    15
#define    CAMINTF_CSI2RDLS_EEE3_MASK                                     0x00008000
#define    CAMINTF_CSI2RDLS_SOTSE3_SHIFT                                  14
#define    CAMINTF_CSI2RDLS_SOTSE3_MASK                                   0x00004000
#define    CAMINTF_CSI2RDLS_SOTE3_SHIFT                                   13
#define    CAMINTF_CSI2RDLS_SOTE3_MASK                                    0x00002000
#define    CAMINTF_CSI2RDLS_ULPS3_SHIFT                                   12
#define    CAMINTF_CSI2RDLS_ULPS3_MASK                                    0x00001000
#define    CAMINTF_CSI2RDLS_CE2_SHIFT                                     11
#define    CAMINTF_CSI2RDLS_CE2_MASK                                      0x00000800
#define    CAMINTF_CSI2RDLS_ESE2_SHIFT                                    10
#define    CAMINTF_CSI2RDLS_ESE2_MASK                                     0x00000400
#define    CAMINTF_CSI2RDLS_EEE2_SHIFT                                    9
#define    CAMINTF_CSI2RDLS_EEE2_MASK                                     0x00000200
#define    CAMINTF_CSI2RDLS_SOTSE2_SHIFT                                  8
#define    CAMINTF_CSI2RDLS_SOTSE2_MASK                                   0x00000100
#define    CAMINTF_CSI2RDLS_SOTE2_SHIFT                                   7
#define    CAMINTF_CSI2RDLS_SOTE2_MASK                                    0x00000080
#define    CAMINTF_CSI2RDLS_ULPS2_SHIFT                                   6
#define    CAMINTF_CSI2RDLS_ULPS2_MASK                                    0x00000040
#define    CAMINTF_CSI2RDLS_CE1_SHIFT                                     5
#define    CAMINTF_CSI2RDLS_CE1_MASK                                      0x00000020
#define    CAMINTF_CSI2RDLS_ESE1_SHIFT                                    4
#define    CAMINTF_CSI2RDLS_ESE1_MASK                                     0x00000010
#define    CAMINTF_CSI2RDLS_EEE1_SHIFT                                    3
#define    CAMINTF_CSI2RDLS_EEE1_MASK                                     0x00000008
#define    CAMINTF_CSI2RDLS_SOTSE1_SHIFT                                  2
#define    CAMINTF_CSI2RDLS_SOTSE1_MASK                                   0x00000004
#define    CAMINTF_CSI2RDLS_SOTE1_SHIFT                                   1
#define    CAMINTF_CSI2RDLS_SOTE1_MASK                                    0x00000002
#define    CAMINTF_CSI2RDLS_ULPS1_SHIFT                                   0
#define    CAMINTF_CSI2RDLS_ULPS1_MASK                                    0x00000001

#define CAMINTF_CSI2RGSP_OFFSET                                           0x0000000C
#define CAMINTF_CSI2RGSP_TYPE                                             UInt32
#define CAMINTF_CSI2RGSP_RESERVED_MASK                                    0xFF000000
#define    CAMINTF_CSI2RGSP_DATA_SHIFT                                    8
#define    CAMINTF_CSI2RGSP_DATA_MASK                                     0x00FFFF00
#define    CAMINTF_CSI2RGSP_VC_SHIFT                                      6
#define    CAMINTF_CSI2RGSP_VC_MASK                                       0x000000C0
#define    CAMINTF_CSI2RGSP_DT_SHIFT                                      0
#define    CAMINTF_CSI2RGSP_DT_MASK                                       0x0000003F

#define CAMINTF_CSI2TREN_OFFSET                                           0x00000010
#define CAMINTF_CSI2TREN_TYPE                                             UInt32
#define CAMINTF_CSI2TREN_RESERVED_MASK                                    0xFFFFFC00
#define    CAMINTF_CSI2TREN_TROVC_SHIFT                                   9
#define    CAMINTF_CSI2TREN_TROVC_MASK                                    0x00000200
#define    CAMINTF_CSI2TREN_TROV4_SHIFT                                   8
#define    CAMINTF_CSI2TREN_TROV4_MASK                                    0x00000100
#define    CAMINTF_CSI2TREN_TROV3_SHIFT                                   7
#define    CAMINTF_CSI2TREN_TROV3_MASK                                    0x00000080
#define    CAMINTF_CSI2TREN_TROV2_SHIFT                                   6
#define    CAMINTF_CSI2TREN_TROV2_MASK                                    0x00000040
#define    CAMINTF_CSI2TREN_TROV1_SHIFT                                   5
#define    CAMINTF_CSI2TREN_TROV1_MASK                                    0x00000020
#define    CAMINTF_CSI2TREN_TRENC_SHIFT                                   4
#define    CAMINTF_CSI2TREN_TRENC_MASK                                    0x00000010
#define    CAMINTF_CSI2TREN_TREN4_SHIFT                                   3
#define    CAMINTF_CSI2TREN_TREN4_MASK                                    0x00000008
#define    CAMINTF_CSI2TREN_TREN3_SHIFT                                   2
#define    CAMINTF_CSI2TREN_TREN3_MASK                                    0x00000004
#define    CAMINTF_CSI2TREN_TREN2_SHIFT                                   1
#define    CAMINTF_CSI2TREN_TREN2_MASK                                    0x00000002
#define    CAMINTF_CSI2TREN_TREN1_SHIFT                                   0
#define    CAMINTF_CSI2TREN_TREN1_MASK                                    0x00000001

#define CAMINTF_CSI2THSSTO_OFFSET                                         0x00000014
#define CAMINTF_CSI2THSSTO_TYPE                                           UInt32
#define CAMINTF_CSI2THSSTO_RESERVED_MASK                                  0xFFFF0000
#define    CAMINTF_CSI2THSSTO_THSSTO_SHIFT                                0
#define    CAMINTF_CSI2THSSTO_THSSTO_MASK                                 0x0000FFFF

#define CAMINTF_CSI2THSSET_OFFSET                                         0x00000018
#define CAMINTF_CSI2THSSET_TYPE                                           UInt32
#define CAMINTF_CSI2THSSET_RESERVED_MASK                                  0xFFFFF000
#define    CAMINTF_CSI2THSSET_TD_SHIFT                                    8
#define    CAMINTF_CSI2THSSET_TD_MASK                                     0x00000F00
#define    CAMINTF_CSI2THSSET_HSMC_SHIFT                                  0
#define    CAMINTF_CSI2THSSET_HSMC_MASK                                   0x000000FF

#define CAMINTF_CSI2THSCKTO_OFFSET                                        0x0000001C
#define CAMINTF_CSI2THSCKTO_TYPE                                          UInt32
#define CAMINTF_CSI2THSCKTO_RESERVED_MASK                                 0xFFFF0000
#define    CAMINTF_CSI2THSCKTO_THSCKTO_SHIFT                              0
#define    CAMINTF_CSI2THSCKTO_THSCKTO_MASK                               0x0000FFFF

#define CAMINTF_CSI2DBGDPHY_OFFSET                                        0x00000080
#define CAMINTF_CSI2DBGDPHY_TYPE                                          UInt32
#define CAMINTF_CSI2DBGDPHY_RESERVED_MASK                                 0xFFF00000
#define    CAMINTF_CSI2DBGDPHY_DBGDPHY_SHIFT                              0
#define    CAMINTF_CSI2DBGDPHY_DBGDPHY_MASK                               0x000FFFFF

#define CAMINTF_CSI2DBGMISC_OFFSET                                        0x00000084
#define CAMINTF_CSI2DBGMISC_TYPE                                          UInt32
#define CAMINTF_CSI2DBGMISC_RESERVED_MASK                                 0xFFFFFE00
#define    CAMINTF_CSI2DBGMISC_DBGMISC_SHIFT                              0
#define    CAMINTF_CSI2DBGMISC_DBGMISC_MASK                               0x000001FF

#define CAMINTF_CSI2TRIG_OFFSET                                           0x00000088
#define CAMINTF_CSI2TRIG_TYPE                                             UInt32
#define CAMINTF_CSI2TRIG_RESERVED_MASK                                    0xFFF00000
#define    CAMINTF_CSI2TRIG_TRIG_SHIFT                                    0
#define    CAMINTF_CSI2TRIG_TRIG_MASK                                     0x000FFFFF

#define CAMINTF_CSI2SRST_OFFSET                                           0x00000090
#define CAMINTF_CSI2SRST_TYPE                                             UInt32
#define CAMINTF_CSI2SRST_RESERVED_MASK                                    0xFFFFFFFC
#define    CAMINTF_CSI2SRST_ARST_SHIFT                                    1
#define    CAMINTF_CSI2SRST_ARST_MASK                                     0x00000002
#define    CAMINTF_CSI2SRST_SRST_SHIFT                                    0
#define    CAMINTF_CSI2SRST_SRST_MASK                                     0x00000001

#define CAMINTF_CSI2RDR3_OFFSET                                           0x00000094
#define CAMINTF_CSI2RDR3_TYPE                                             UInt32
#define CAMINTF_CSI2RDR3_RESERVED_MASK                                    0xFFFFE000
#define    CAMINTF_CSI2RDR3_RDR3_SHIFT                                    0
#define    CAMINTF_CSI2RDR3_RDR3_MASK                                     0x00001FFF

#define CAMINTF_CSI2MDBG_OFFSET                                           0x00000098
#define CAMINTF_CSI2MDBG_TYPE                                             UInt32
#define CAMINTF_CSI2MDBG_RESERVED_MASK                                    0xFFFFFFFE
#define    CAMINTF_CSI2MDBG_DPHY_ASYNC_SHIFT                              0
#define    CAMINTF_CSI2MDBG_DPHY_ASYNC_MASK                               0x00000001

#define CAMINTF_CSI2LPDS_OFFSET                                           0x0000009C
#define CAMINTF_CSI2LPDS_TYPE                                             UInt32
#define CAMINTF_CSI2LPDS_RESERVED_MASK                                    0xFFFFFFC0
#define    CAMINTF_CSI2LPDS_D1_LP_DP_SHIFT                                5
#define    CAMINTF_CSI2LPDS_D1_LP_DP_MASK                                 0x00000020
#define    CAMINTF_CSI2LPDS_D1_LP_DM_SHIFT                                4
#define    CAMINTF_CSI2LPDS_D1_LP_DM_MASK                                 0x00000010
#define    CAMINTF_CSI2LPDS_D0_LP_DP_SHIFT                                3
#define    CAMINTF_CSI2LPDS_D0_LP_DP_MASK                                 0x00000008
#define    CAMINTF_CSI2LPDS_D0_LP_DM_SHIFT                                2
#define    CAMINTF_CSI2LPDS_D0_LP_DM_MASK                                 0x00000004
#define    CAMINTF_CSI2LPDS_CLK_LP_DP_SHIFT                               1
#define    CAMINTF_CSI2LPDS_CLK_LP_DP_MASK                                0x00000002
#define    CAMINTF_CSI2LPDS_CLK_LP_DM_SHIFT                               0
#define    CAMINTF_CSI2LPDS_CLK_LP_DM_MASK                                0x00000001

#define CAMINTF_CSI2PCP_OFFSET                                            0x000000A0
#define CAMINTF_CSI2PCP_TYPE                                              UInt32
#define CAMINTF_CSI2PCP_RESERVED_MASK                                     0xF888F000
#define    CAMINTF_CSI2PCP_CLK_IDR_SHIFT                                  24
#define    CAMINTF_CSI2PCP_CLK_IDR_MASK                                   0x07000000
#define    CAMINTF_CSI2PCP_D1_IDR_SHIFT                                   20
#define    CAMINTF_CSI2PCP_D1_IDR_MASK                                    0x00700000
#define    CAMINTF_CSI2PCP_D0_IDR_SHIFT                                   16
#define    CAMINTF_CSI2PCP_D0_IDR_MASK                                    0x00070000
#define    CAMINTF_CSI2PCP_CLK_ACTRL_SHIFT                                8
#define    CAMINTF_CSI2PCP_CLK_ACTRL_MASK                                 0x00000F00
#define    CAMINTF_CSI2PCP_D1_ACTRL_SHIFT                                 4
#define    CAMINTF_CSI2PCP_D1_ACTRL_MASK                                  0x000000F0
#define    CAMINTF_CSI2PCP_D0_ACTRL_SHIFT                                 0
#define    CAMINTF_CSI2PCP_D0_ACTRL_MASK                                  0x0000000F

#define CAMINTF_CSI2RC0_OFFSET                                            0x00000100
#define CAMINTF_CSI2RC0_TYPE                                              UInt32
#define CAMINTF_CSI2RC0_RESERVED_MASK                                     0xE000E0F8
#define    CAMINTF_CSI2RC0_LCIE_SHIFT                                     16
#define    CAMINTF_CSI2RC0_LCIE_MASK                                      0x1FFF0000
#define    CAMINTF_CSI2RC0_GSPIE_SHIFT                                    12
#define    CAMINTF_CSI2RC0_GSPIE_MASK                                     0x00001000
#define    CAMINTF_CSI2RC0_LEIE_SHIFT                                     11
#define    CAMINTF_CSI2RC0_LEIE_MASK                                      0x00000800
#define    CAMINTF_CSI2RC0_LSIE_SHIFT                                     10
#define    CAMINTF_CSI2RC0_LSIE_MASK                                      0x00000400
#define    CAMINTF_CSI2RC0_FEIE_SHIFT                                     9
#define    CAMINTF_CSI2RC0_FEIE_MASK                                      0x00000200
#define    CAMINTF_CSI2RC0_FSIE_SHIFT                                     8
#define    CAMINTF_CSI2RC0_FSIE_MASK                                      0x00000100
#define    CAMINTF_CSI2RC0_VC_SHIFT                                       1
#define    CAMINTF_CSI2RC0_VC_MASK                                        0x00000006
#define    CAMINTF_CSI2RC0_CHEN_SHIFT                                     0
#define    CAMINTF_CSI2RC0_CHEN_MASK                                      0x00000001

#define CAMINTF_CSI2RC1_OFFSET                                            0x00000200
#define CAMINTF_CSI2RC1_TYPE                                              UInt32
#define CAMINTF_CSI2RC1_RESERVED_MASK                                     0xE000E0F8
#define    CAMINTF_CSI2RC1_LCIE_SHIFT                                     16
#define    CAMINTF_CSI2RC1_LCIE_MASK                                      0x1FFF0000
#define    CAMINTF_CSI2RC1_GSPIE_SHIFT                                    12
#define    CAMINTF_CSI2RC1_GSPIE_MASK                                     0x00001000
#define    CAMINTF_CSI2RC1_LEIE_SHIFT                                     11
#define    CAMINTF_CSI2RC1_LEIE_MASK                                      0x00000800
#define    CAMINTF_CSI2RC1_LSIE_SHIFT                                     10
#define    CAMINTF_CSI2RC1_LSIE_MASK                                      0x00000400
#define    CAMINTF_CSI2RC1_FEIE_SHIFT                                     9
#define    CAMINTF_CSI2RC1_FEIE_MASK                                      0x00000200
#define    CAMINTF_CSI2RC1_FSIE_SHIFT                                     8
#define    CAMINTF_CSI2RC1_FSIE_MASK                                      0x00000100
#define    CAMINTF_CSI2RC1_VC_SHIFT                                       1
#define    CAMINTF_CSI2RC1_VC_MASK                                        0x00000006
#define    CAMINTF_CSI2RC1_CHEN_SHIFT                                     0
#define    CAMINTF_CSI2RC1_CHEN_MASK                                      0x00000001

#define CAMINTF_CSI2RPC0_OFFSET                                           0x00000104
#define CAMINTF_CSI2RPC0_TYPE                                             UInt32
#define CAMINTF_CSI2RPC0_RESERVED_MASK                                    0xFE00E0E0
#define    CAMINTF_CSI2RPC0_EBL_SHIFT                                     16
#define    CAMINTF_CSI2RPC0_EBL_MASK                                      0x01FF0000
#define    CAMINTF_CSI2RPC0_EAP_SHIFT                                     12
#define    CAMINTF_CSI2RPC0_EAP_MASK                                      0x00001000
#define    CAMINTF_CSI2RPC0_EP_SHIFT                                      8
#define    CAMINTF_CSI2RPC0_EP_MASK                                       0x00000F00
#define    CAMINTF_CSI2RPC0_DAP_SHIFT                                     4
#define    CAMINTF_CSI2RPC0_DAP_MASK                                      0x00000010
#define    CAMINTF_CSI2RPC0_DP_SHIFT                                      0
#define    CAMINTF_CSI2RPC0_DP_MASK                                       0x0000000F

#define CAMINTF_CSI2RPC1_OFFSET                                           0x00000204
#define CAMINTF_CSI2RPC1_TYPE                                             UInt32
#define CAMINTF_CSI2RPC1_RESERVED_MASK                                    0xFE00E0E0
#define    CAMINTF_CSI2RPC1_EBL_SHIFT                                     16
#define    CAMINTF_CSI2RPC1_EBL_MASK                                      0x01FF0000
#define    CAMINTF_CSI2RPC1_EAP_SHIFT                                     12
#define    CAMINTF_CSI2RPC1_EAP_MASK                                      0x00001000
#define    CAMINTF_CSI2RPC1_EP_SHIFT                                      8
#define    CAMINTF_CSI2RPC1_EP_MASK                                       0x00000F00
#define    CAMINTF_CSI2RPC1_DAP_SHIFT                                     4
#define    CAMINTF_CSI2RPC1_DAP_MASK                                      0x00000010
#define    CAMINTF_CSI2RPC1_DP_SHIFT                                      0
#define    CAMINTF_CSI2RPC1_DP_MASK                                       0x0000000F

#define CAMINTF_CSI2RS0_OFFSET                                            0x00000108
#define CAMINTF_CSI2RS0_TYPE                                              UInt32
#define CAMINTF_CSI2RS0_RESERVED_MASK                                     0xFE000000
#define    CAMINTF_CSI2RS0_DBO_SHIFT                                      24
#define    CAMINTF_CSI2RS0_DBO_MASK                                       0x01000000
#define    CAMINTF_CSI2RS0_IBO_SHIFT                                      23
#define    CAMINTF_CSI2RS0_IBO_MASK                                       0x00800000
#define    CAMINTF_CSI2RS0_CRCE_SHIFT                                     22
#define    CAMINTF_CSI2RS0_CRCE_MASK                                      0x00400000
#define    CAMINTF_CSI2RS0_LCI_SHIFT                                      21
#define    CAMINTF_CSI2RS0_LCI_MASK                                       0x00200000
#define    CAMINTF_CSI2RS0_GSPI_SHIFT                                     20
#define    CAMINTF_CSI2RS0_GSPI_MASK                                      0x00100000
#define    CAMINTF_CSI2RS0_LEI_SHIFT                                      19
#define    CAMINTF_CSI2RS0_LEI_MASK                                       0x00080000
#define    CAMINTF_CSI2RS0_LSI_SHIFT                                      18
#define    CAMINTF_CSI2RS0_LSI_MASK                                       0x00040000
#define    CAMINTF_CSI2RS0_FEI_SHIFT                                      17
#define    CAMINTF_CSI2RS0_FEI_MASK                                       0x00020000
#define    CAMINTF_CSI2RS0_FSI_SHIFT                                      16
#define    CAMINTF_CSI2RS0_FSI_MASK                                       0x00010000
#define    CAMINTF_CSI2RS0_FNUM_SHIFT                                     0
#define    CAMINTF_CSI2RS0_FNUM_MASK                                      0x0000FFFF

#define CAMINTF_CSI2RS1_OFFSET                                            0x00000208
#define CAMINTF_CSI2RS1_TYPE                                              UInt32
#define CAMINTF_CSI2RS1_RESERVED_MASK                                     0xFE000000
#define    CAMINTF_CSI2RS1_DBO_SHIFT                                      24
#define    CAMINTF_CSI2RS1_DBO_MASK                                       0x01000000
#define    CAMINTF_CSI2RS1_IBO_SHIFT                                      23
#define    CAMINTF_CSI2RS1_IBO_MASK                                       0x00800000
#define    CAMINTF_CSI2RS1_CRCE_SHIFT                                     22
#define    CAMINTF_CSI2RS1_CRCE_MASK                                      0x00400000
#define    CAMINTF_CSI2RS1_LCI_SHIFT                                      21
#define    CAMINTF_CSI2RS1_LCI_MASK                                       0x00200000
#define    CAMINTF_CSI2RS1_GSPI_SHIFT                                     20
#define    CAMINTF_CSI2RS1_GSPI_MASK                                      0x00100000
#define    CAMINTF_CSI2RS1_LEI_SHIFT                                      19
#define    CAMINTF_CSI2RS1_LEI_MASK                                       0x00080000
#define    CAMINTF_CSI2RS1_LSI_SHIFT                                      18
#define    CAMINTF_CSI2RS1_LSI_MASK                                       0x00040000
#define    CAMINTF_CSI2RS1_FEI_SHIFT                                      17
#define    CAMINTF_CSI2RS1_FEI_MASK                                       0x00020000
#define    CAMINTF_CSI2RS1_FSI_SHIFT                                      16
#define    CAMINTF_CSI2RS1_FSI_MASK                                       0x00010000
#define    CAMINTF_CSI2RS1_FNUM_SHIFT                                     0
#define    CAMINTF_CSI2RS1_FNUM_MASK                                      0x0000FFFF

#define CAMINTF_CSI2RSA0_OFFSET                                           0x0000010C
#define CAMINTF_CSI2RSA0_TYPE                                             UInt32
#define CAMINTF_CSI2RSA0_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CSI2RSA0_RSA_SHIFT                                     0
#define    CAMINTF_CSI2RSA0_RSA_MASK                                      0xFFFFFFFF

#define CAMINTF_CSI2RSA1_OFFSET                                           0x0000020C
#define CAMINTF_CSI2RSA1_TYPE                                             UInt32
#define CAMINTF_CSI2RSA1_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CSI2RSA1_RSA_SHIFT                                     0
#define    CAMINTF_CSI2RSA1_RSA_MASK                                      0xFFFFFFFF

#define CAMINTF_CSI2REA0_OFFSET                                           0x00000110
#define CAMINTF_CSI2REA0_TYPE                                             UInt32
#define CAMINTF_CSI2REA0_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CSI2REA0_REA_SHIFT                                     0
#define    CAMINTF_CSI2REA0_REA_MASK                                      0xFFFFFFFF

#define CAMINTF_CSI2REA1_OFFSET                                           0x00000210
#define CAMINTF_CSI2REA1_TYPE                                             UInt32
#define CAMINTF_CSI2REA1_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CSI2REA1_REA_SHIFT                                     0
#define    CAMINTF_CSI2REA1_REA_MASK                                      0xFFFFFFFF

#define CAMINTF_CSI2RWP0_OFFSET                                           0x00000114
#define CAMINTF_CSI2RWP0_TYPE                                             UInt32
#define CAMINTF_CSI2RWP0_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CSI2RWP0_RWP_SHIFT                                     0
#define    CAMINTF_CSI2RWP0_RWP_MASK                                      0x3FFFFFFF

#define CAMINTF_CSI2RWP1_OFFSET                                           0x00000214
#define CAMINTF_CSI2RWP1_TYPE                                             UInt32
#define CAMINTF_CSI2RWP1_RESERVED_MASK                                    0xC0000000
#define    CAMINTF_CSI2RWP1_RWP_SHIFT                                     0
#define    CAMINTF_CSI2RWP1_RWP_MASK                                      0x3FFFFFFF

#define CAMINTF_CSI2RBC0_OFFSET                                           0x00000118
#define CAMINTF_CSI2RBC0_TYPE                                             UInt32
#define CAMINTF_CSI2RBC0_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CSI2RBC0_RBC_SHIFT                                     0
#define    CAMINTF_CSI2RBC0_RBC_MASK                                      0xFFFFFFFF

#define CAMINTF_CSI2RBC1_OFFSET                                           0x00000218
#define CAMINTF_CSI2RBC1_TYPE                                             UInt32
#define CAMINTF_CSI2RBC1_RESERVED_MASK                                    0x00000000
#define    CAMINTF_CSI2RBC1_RBC_SHIFT                                     0
#define    CAMINTF_CSI2RBC1_RBC_MASK                                      0xFFFFFFFF

#define CAMINTF_CSI2RLS0_OFFSET                                           0x0000011C
#define CAMINTF_CSI2RLS0_TYPE                                             UInt32
#define CAMINTF_CSI2RLS0_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CSI2RLS0_RLS_SHIFT                                     0
#define    CAMINTF_CSI2RLS0_RLS_MASK                                      0x0000FFFF

#define CAMINTF_CSI2RLS1_OFFSET                                           0x0000021C
#define CAMINTF_CSI2RLS1_TYPE                                             UInt32
#define CAMINTF_CSI2RLS1_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CSI2RLS1_RLS_SHIFT                                     0
#define    CAMINTF_CSI2RLS1_RLS_MASK                                      0x0000FFFF

#define CAMINTF_CSI2RDSA0_OFFSET                                          0x00000120
#define CAMINTF_CSI2RDSA0_TYPE                                            UInt32
#define CAMINTF_CSI2RDSA0_RESERVED_MASK                                   0x00000000
#define    CAMINTF_CSI2RDSA0_RDSA_SHIFT                                   0
#define    CAMINTF_CSI2RDSA0_RDSA_MASK                                    0xFFFFFFFF

#define CAMINTF_CSI2RDSA1_OFFSET                                          0x00000220
#define CAMINTF_CSI2RDSA1_TYPE                                            UInt32
#define CAMINTF_CSI2RDSA1_RESERVED_MASK                                   0x00000000
#define    CAMINTF_CSI2RDSA1_RDSA_SHIFT                                   0
#define    CAMINTF_CSI2RDSA1_RDSA_MASK                                    0xFFFFFFFF

#define CAMINTF_CSI2RDEA0_OFFSET                                          0x00000124
#define CAMINTF_CSI2RDEA0_TYPE                                            UInt32
#define CAMINTF_CSI2RDEA0_RESERVED_MASK                                   0x00000000
#define    CAMINTF_CSI2RDEA0_RDEA_SHIFT                                   0
#define    CAMINTF_CSI2RDEA0_RDEA_MASK                                    0xFFFFFFFF

#define CAMINTF_CSI2RDEA1_OFFSET                                          0x00000224
#define CAMINTF_CSI2RDEA1_TYPE                                            UInt32
#define CAMINTF_CSI2RDEA1_RESERVED_MASK                                   0x00000000
#define    CAMINTF_CSI2RDEA1_RDEA_SHIFT                                   0
#define    CAMINTF_CSI2RDEA1_RDEA_MASK                                    0xFFFFFFFF

#define CAMINTF_CSI2RDS0_OFFSET                                           0x00000128
#define CAMINTF_CSI2RDS0_TYPE                                             UInt32
#define CAMINTF_CSI2RDS0_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CSI2RDS0_RDS_SHIFT                                     0
#define    CAMINTF_CSI2RDS0_RDS_MASK                                      0x0000FFFF

#define CAMINTF_CSI2RDS1_OFFSET                                           0x00000228
#define CAMINTF_CSI2RDS1_TYPE                                             UInt32
#define CAMINTF_CSI2RDS1_RESERVED_MASK                                    0xFFFF0000
#define    CAMINTF_CSI2RDS1_RDS_SHIFT                                     0
#define    CAMINTF_CSI2RDS1_RDS_MASK                                      0x0000FFFF

#define CAMINTF_CSI2DTOV0_OFFSET                                          0x0000012C
#define CAMINTF_CSI2DTOV0_TYPE                                            UInt32
#define CAMINTF_CSI2DTOV0_RESERVED_MASK                                   0xFFFF8080
#define    CAMINTF_CSI2DTOV0_IMEN_SHIFT                                   14
#define    CAMINTF_CSI2DTOV0_IMEN_MASK                                    0x00004000
#define    CAMINTF_CSI2DTOV0_IMDT_SHIFT                                   8
#define    CAMINTF_CSI2DTOV0_IMDT_MASK                                    0x00003F00
#define    CAMINTF_CSI2DTOV0_EMEN_SHIFT                                   6
#define    CAMINTF_CSI2DTOV0_EMEN_MASK                                    0x00000040
#define    CAMINTF_CSI2DTOV0_EMDT_SHIFT                                   0
#define    CAMINTF_CSI2DTOV0_EMDT_MASK                                    0x0000003F

#define CAMINTF_CSI2DTOV1_OFFSET                                          0x0000022C
#define CAMINTF_CSI2DTOV1_TYPE                                            UInt32
#define CAMINTF_CSI2DTOV1_RESERVED_MASK                                   0xFFFF8080
#define    CAMINTF_CSI2DTOV1_IMEN_SHIFT                                   14
#define    CAMINTF_CSI2DTOV1_IMEN_MASK                                    0x00004000
#define    CAMINTF_CSI2DTOV1_IMDT_SHIFT                                   8
#define    CAMINTF_CSI2DTOV1_IMDT_MASK                                    0x00003F00
#define    CAMINTF_CSI2DTOV1_EMEN_SHIFT                                   6
#define    CAMINTF_CSI2DTOV1_EMEN_MASK                                    0x00000040
#define    CAMINTF_CSI2DTOV1_EMDT_SHIFT                                   0
#define    CAMINTF_CSI2DTOV1_EMDT_MASK                                    0x0000003F

#endif /* __BRCM_RDB_CAMINTF_H__ */



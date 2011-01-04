/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:16:58                                             */
/*     RDB file : //HERA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_MDM_CLK_MGR_REG_H__
#define __BRCM_RDB_MDM_CLK_MGR_REG_H__

#define MDM_CLK_MGR_REG_WR_ACCESS_OFFSET                                  0x00000000
#define MDM_CLK_MGR_REG_WR_ACCESS_TYPE                                    UInt32
#define MDM_CLK_MGR_REG_WR_ACCESS_RESERVED_MASK                           0x7F0000FE
#define    MDM_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT               31
#define    MDM_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                0x80000000
#define    MDM_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                       8
#define    MDM_CLK_MGR_REG_WR_ACCESS_PASSWORD_MASK                        0x00FFFF00
#define    MDM_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT                     0
#define    MDM_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK                      0x00000001

#define MDM_CLK_MGR_REG_POLICY_FREQ_OFFSET                                0x00000008
#define MDM_CLK_MGR_REG_POLICY_FREQ_TYPE                                  UInt32
#define MDM_CLK_MGR_REG_POLICY_FREQ_RESERVED_MASK                         0x7FFFF000
#define    MDM_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_SHIFT             31
#define    MDM_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT                 9
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_MASK                  0x00000E00
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT                 6
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_MASK                  0x000001C0
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT                 3
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_MASK                  0x00000038
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT                 0
#define    MDM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK                  0x00000007

#define MDM_CLK_MGR_REG_POLICY_CTL_OFFSET                                 0x0000000C
#define MDM_CLK_MGR_REG_POLICY_CTL_TYPE                                   UInt32
#define MDM_CLK_MGR_REG_POLICY_CTL_RESERVED_MASK                          0x7FFFFFF0
#define    MDM_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_SHIFT              31
#define    MDM_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MDM_CLK_MGR_REG_POLICY_CTL_TGT_VLD_SHIFT                       3
#define    MDM_CLK_MGR_REG_POLICY_CTL_TGT_VLD_MASK                        0x00000008
#define    MDM_CLK_MGR_REG_POLICY_CTL_GO_ATL_SHIFT                        2
#define    MDM_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK                         0x00000004
#define    MDM_CLK_MGR_REG_POLICY_CTL_GO_AC_SHIFT                         1
#define    MDM_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK                          0x00000002
#define    MDM_CLK_MGR_REG_POLICY_CTL_GO_SHIFT                            0
#define    MDM_CLK_MGR_REG_POLICY_CTL_GO_MASK                             0x00000001

#define MDM_CLK_MGR_REG_INTEN_OFFSET                                      0x00000020
#define MDM_CLK_MGR_REG_INTEN_TYPE                                        UInt32
#define MDM_CLK_MGR_REG_INTEN_RESERVED_MASK                               0x7FFFFFF8
#define    MDM_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_SHIFT                   31
#define    MDM_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MDM_CLK_MGR_REG_INTEN_INTEN_RST_INT_EN_SHIFT                   2
#define    MDM_CLK_MGR_REG_INTEN_INTEN_RST_INT_EN_MASK                    0x00000004
#define    MDM_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_SHIFT                   1
#define    MDM_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_MASK                    0x00000002
#define    MDM_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_SHIFT                   0
#define    MDM_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_MASK                    0x00000001

#define MDM_CLK_MGR_REG_INTSTAT_OFFSET                                    0x00000024
#define MDM_CLK_MGR_REG_INTSTAT_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_INTSTAT_RESERVED_MASK                             0x7FFFFFF8
#define    MDM_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_INTSTAT_INTSTAT_RST_INT_SHIFT                  2
#define    MDM_CLK_MGR_REG_INTSTAT_INTSTAT_RST_INT_MASK                   0x00000004
#define    MDM_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_SHIFT                  1
#define    MDM_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_MASK                   0x00000002
#define    MDM_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_SHIFT                  0
#define    MDM_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_MASK                   0x00000001

#define MDM_CLK_MGR_REG_VLT_PERI_OFFSET                                   0x00000030
#define MDM_CLK_MGR_REG_VLT_PERI_TYPE                                     UInt32
#define MDM_CLK_MGR_REG_VLT_PERI_RESERVED_MASK                            0x7FFFFF00
#define    MDM_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_SHIFT                31
#define    MDM_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MDM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT                   4
#define    MDM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK                    0x000000F0
#define    MDM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT                 0
#define    MDM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK                  0x0000000F

#define MDM_CLK_MGR_REG_LVM_EN_OFFSET                                     0x00000034
#define MDM_CLK_MGR_REG_LVM_EN_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_LVM_EN_RESERVED_MASK                              0x7FFFFFFE
#define    MDM_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_SHIFT                  0
#define    MDM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK                   0x00000001

#define MDM_CLK_MGR_REG_LVM0_3_OFFSET                                     0x00000038
#define MDM_CLK_MGR_REG_LVM0_3_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_LVM0_3_RESERVED_MASK                              0x7FFF0000
#define    MDM_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_SHIFT                      12
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_MASK                       0x0000F000
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_SHIFT                      8
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_MASK                       0x00000F00
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_SHIFT                      4
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_MASK                       0x000000F0
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_SHIFT                      0
#define    MDM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK                       0x0000000F

#define MDM_CLK_MGR_REG_LVM4_7_OFFSET                                     0x0000003C
#define MDM_CLK_MGR_REG_LVM4_7_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_LVM4_7_RESERVED_MASK                              0x7FFF0000
#define    MDM_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_SHIFT                      12
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_MASK                       0x0000F000
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_SHIFT                      8
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_MASK                       0x00000F00
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_SHIFT                      4
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_MASK                       0x000000F0
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_SHIFT                      0
#define    MDM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_MASK                       0x0000000F

#define MDM_CLK_MGR_REG_VLT0_3_OFFSET                                     0x00000040
#define MDM_CLK_MGR_REG_VLT0_3_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_VLT0_3_RESERVED_MASK                              0x7FFF0000
#define    MDM_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT                      12
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_MASK                       0x0000F000
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT                      8
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_MASK                       0x00000F00
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT                      4
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_MASK                       0x000000F0
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT                      0
#define    MDM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK                       0x0000000F

#define MDM_CLK_MGR_REG_VLT4_7_OFFSET                                     0x00000044
#define MDM_CLK_MGR_REG_VLT4_7_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_VLT4_7_RESERVED_MASK                              0x7FFF0000
#define    MDM_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT                      12
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_MASK                       0x0000F000
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT                      8
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_MASK                       0x00000F00
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT                      4
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_MASK                       0x000000F0
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT                      0
#define    MDM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_MASK                       0x0000000F

#define MDM_CLK_MGR_REG_LVM_MODE_OFFSET                                   0x00000058
#define MDM_CLK_MGR_REG_LVM_MODE_TYPE                                     UInt32
#define MDM_CLK_MGR_REG_LVM_MODE_RESERVED_MASK                            0x7FFFFFF0
#define    MDM_CLK_MGR_REG_LVM_MODE_PRIV_ACCESS_MODE_SHIFT                31
#define    MDM_CLK_MGR_REG_LVM_MODE_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MDM_CLK_MGR_REG_LVM_MODE_WGM_LVM_MODE_SHIFT                    0
#define    MDM_CLK_MGR_REG_LVM_MODE_WGM_LVM_MODE_MASK                     0x0000000F

#define MDM_CLK_MGR_REG_APB_CLKGATE_OFFSET                                0x00000400
#define MDM_CLK_MGR_REG_APB_CLKGATE_TYPE                                  UInt32
#define MDM_CLK_MGR_REG_APB_CLKGATE_RESERVED_MASK                         0x7FFAF0F5
#define    MDM_CLK_MGR_REG_APB_CLKGATE_PRIV_ACCESS_MODE_SHIFT             31
#define    MDM_CLK_MGR_REG_APB_CLKGATE_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_STPRSTS_SHIFT              18
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_STPRSTS_MASK               0x00040000
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_STPRSTS_SHIFT                  16
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_STPRSTS_MASK                   0x00010000
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_HYST_VAL_SHIFT             11
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_HYST_VAL_MASK              0x00000800
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_HYST_EN_SHIFT              10
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_HYST_EN_MASK               0x00000400
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_HYST_VAL_SHIFT                 9
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_HYST_VAL_MASK                  0x00000200
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_HYST_EN_SHIFT                  8
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_HYST_EN_MASK                   0x00000100
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_HW_SW_GATING_SEL_SHIFT     3
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_AON_HW_SW_GATING_SEL_MASK      0x00000008
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_HW_SW_GATING_SEL_SHIFT         1
#define    MDM_CLK_MGR_REG_APB_CLKGATE_APB_HW_SW_GATING_SEL_MASK          0x00000002

#define MDM_CLK_MGR_REG_WGM_HF_CLKGATE_OFFSET                             0x00000600
#define MDM_CLK_MGR_REG_WGM_HF_CLKGATE_TYPE                               UInt32
#define MDM_CLK_MGR_REG_WGM_HF_CLKGATE_RESERVED_MASK                      0x7FFEFCFC
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_PRIV_ACCESS_MODE_SHIFT          31
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_STPRSTS_SHIFT            16
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_STPRSTS_MASK             0x00010000
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_HYST_VAL_SHIFT           9
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_HYST_VAL_MASK            0x00000200
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_HYST_EN_SHIFT            8
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_HYST_EN_MASK             0x00000100
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_HW_SW_GATING_SEL_SHIFT   1
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_HW_SW_GATING_SEL_MASK    0x00000002
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_CLK_EN_SHIFT             0
#define    MDM_CLK_MGR_REG_WGM_HF_CLKGATE_WGM_HF_CLK_EN_MASK              0x00000001

#define MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_OFFSET                          0x00000604
#define MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_TYPE                            UInt32
#define MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_RESERVED_MASK                   0x7FFEFCFC
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_PRIV_ACCESS_MODE_SHIFT       31
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_PRIV_ACCESS_MODE_MASK        0x80000000
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_STPRSTS_SHIFT      16
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_STPRSTS_MASK       0x00010000
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_HYST_VAL_SHIFT     9
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_HYST_VAL_MASK      0x00000200
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_HYST_EN_SHIFT      8
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_HYST_EN_MASK       0x00000100
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_HW_SW_GATING_SEL_SHIFT 1
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_HW_SW_GATING_SEL_MASK 0x00000002
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_CLK_EN_SHIFT       0
#define    MDM_CLK_MGR_REG_WGM_RFSYS_CLKGATE_WGM_RFSYS_CLK_EN_MASK        0x00000001

#define MDM_CLK_MGR_REG_CX40_CLKGATE_OFFSET                               0x00000618
#define MDM_CLK_MGR_REG_CX40_CLKGATE_TYPE                                 UInt32
#define MDM_CLK_MGR_REG_CX40_CLKGATE_RESERVED_MASK                        0x7FFEFCFC
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_PRIV_ACCESS_MODE_SHIFT            31
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_STPRSTS_SHIFT                16
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_STPRSTS_MASK                 0x00010000
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_HYST_VAL_SHIFT               9
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_HYST_VAL_MASK                0x00000200
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_HYST_EN_SHIFT                8
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_HYST_EN_MASK                 0x00000100
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_HW_SW_GATING_SEL_SHIFT       1
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_HW_SW_GATING_SEL_MASK        0x00000002
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_CLK_EN_SHIFT                 0
#define    MDM_CLK_MGR_REG_CX40_CLKGATE_CX40_CLK_EN_MASK                  0x00000001

#define MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_OFFSET                           0x00000624
#define MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_TYPE                             UInt32
#define MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RESERVED_MASK                    0x7FFEFCFC
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_PRIV_ACCESS_MODE_SHIFT        31
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_PRIV_ACCESS_MODE_MASK         0x80000000
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_STPRSTS_SHIFT       16
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_STPRSTS_MASK        0x00010000
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_HYST_VAL_SHIFT      9
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_HYST_VAL_MASK       0x00000200
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_HYST_EN_SHIFT       8
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_HYST_EN_MASK        0x00000100
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_HW_SW_GATING_SEL_SHIFT 1
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_HW_SW_GATING_SEL_MASK 0x00000002
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_CLK_EN_SHIFT        0
#define    MDM_CLK_MGR_REG_RFBB_26M_CLKGATE_RF_BB_26M_CLK_EN_MASK         0x00000001

#define MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_OFFSET                           0x0000062C
#define MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_TYPE                             UInt32
#define MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RESERVED_MASK                    0x7FFEFCFC
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_PRIV_ACCESS_MODE_SHIFT        31
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_PRIV_ACCESS_MODE_MASK         0x80000000
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_STPRSTS_SHIFT       16
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_STPRSTS_MASK        0x00010000
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_HYST_VAL_SHIFT      9
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_HYST_VAL_MASK       0x00000200
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_HYST_EN_SHIFT       8
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_HYST_EN_MASK        0x00000100
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_HW_SW_GATING_SEL_SHIFT 1
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_HW_SW_GATING_SEL_MASK 0x00000002
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_CLK_EN_SHIFT        0
#define    MDM_CLK_MGR_REG_RFBB_SPI_CLKGATE_RF_BB_SPI_CLK_EN_MASK         0x00000001

#define MDM_CLK_MGR_REG_ATB_CLKGATE_OFFSET                                0x00000634
#define MDM_CLK_MGR_REG_ATB_CLKGATE_TYPE                                  UInt32
#define MDM_CLK_MGR_REG_ATB_CLKGATE_RESERVED_MASK                         0x7FFEFCFC
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_PRIV_ACCESS_MODE_SHIFT             31
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_STPRSTS_SHIFT                  16
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_STPRSTS_MASK                   0x00010000
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_HYST_VAL_SHIFT                 9
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_HYST_VAL_MASK                  0x00000200
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_HYST_EN_SHIFT                  8
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_HYST_EN_MASK                   0x00000100
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_HW_SW_GATING_SEL_SHIFT         1
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_HW_SW_GATING_SEL_MASK          0x00000002
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_CLK_EN_SHIFT                   0
#define    MDM_CLK_MGR_REG_ATB_CLKGATE_ATB_CLK_EN_MASK                    0x00000001

#define MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_OFFSET                       0x0000063C
#define MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_TYPE                         UInt32
#define MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_RESERVED_MASK                0x7FFEFCFC
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PRIV_ACCESS_MODE_SHIFT    31
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PRIV_ACCESS_MODE_MASK     0x80000000
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_STPRSTS_SHIFT 16
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_STPRSTS_MASK 0x00010000
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_HYST_VAL_SHIFT 9
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_HYST_VAL_MASK 0x00000200
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_HYST_EN_SHIFT 8
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_HYST_EN_MASK 0x00000100
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_HW_SW_GATING_SEL_SHIFT 1
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_HW_SW_GATING_SEL_MASK 0x00000002
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_CLK_EN_SHIFT 0
#define    MDM_CLK_MGR_REG_PMU_SHIM_26M_CLKGATE_PMU_SHIM_26M_CLK_EN_MASK  0x00000001

#define MDM_CLK_MGR_REG_APB_FREE_DIV_OFFSET                               0x00000A00
#define MDM_CLK_MGR_REG_APB_FREE_DIV_TYPE                                 UInt32
#define MDM_CLK_MGR_REG_APB_FREE_DIV_RESERVED_MASK                        0x7FFFFCFC
#define    MDM_CLK_MGR_REG_APB_FREE_DIV_PRIV_ACCESS_MODE_SHIFT            31
#define    MDM_CLK_MGR_REG_APB_FREE_DIV_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MDM_CLK_MGR_REG_APB_FREE_DIV_APB_FREE_DIV_SHIFT                8
#define    MDM_CLK_MGR_REG_APB_FREE_DIV_APB_FREE_DIV_MASK                 0x00000300
#define    MDM_CLK_MGR_REG_APB_FREE_DIV_APB_FREE_PLL_SELECT_SHIFT         0
#define    MDM_CLK_MGR_REG_APB_FREE_DIV_APB_FREE_PLL_SELECT_MASK          0x00000003
#define       MDM_CLK_MGR_REG_APB_FREE_DIV_APB_FREE_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000000
#define       MDM_CLK_MGR_REG_APB_FREE_DIV_APB_FREE_PLL_SELECT_CMD_VAR_312M_CLK 0x00000001

#define MDM_CLK_MGR_REG_RFBB_SPI_DIV_OFFSET                               0x00000A04
#define MDM_CLK_MGR_REG_RFBB_SPI_DIV_TYPE                                 UInt32
#define MDM_CLK_MGR_REG_RFBB_SPI_DIV_RESERVED_MASK                        0x7FFFF8FC
#define    MDM_CLK_MGR_REG_RFBB_SPI_DIV_PRIV_ACCESS_MODE_SHIFT            31
#define    MDM_CLK_MGR_REG_RFBB_SPI_DIV_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MDM_CLK_MGR_REG_RFBB_SPI_DIV_RF_BB_SPI_DIV_SHIFT               8
#define    MDM_CLK_MGR_REG_RFBB_SPI_DIV_RF_BB_SPI_DIV_MASK                0x00000700
#define    MDM_CLK_MGR_REG_RFBB_SPI_DIV_RF_BB_SPI_PLL_SELECT_SHIFT        0
#define    MDM_CLK_MGR_REG_RFBB_SPI_DIV_RF_BB_SPI_PLL_SELECT_MASK         0x00000003
#define       MDM_CLK_MGR_REG_RFBB_SPI_DIV_RF_BB_SPI_PLL_SELECT_CMD_REF_CX40_CLK 0x00000000
#define       MDM_CLK_MGR_REG_RFBB_SPI_DIV_RF_BB_SPI_PLL_SELECT_CMD_VAR_312M_CLK 0x00000001

#define MDM_CLK_MGR_REG_ATB_DIV_OFFSET                                    0x00000A08
#define MDM_CLK_MGR_REG_ATB_DIV_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_ATB_DIV_RESERVED_MASK                             0x7FFFFCFC
#define    MDM_CLK_MGR_REG_ATB_DIV_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_ATB_DIV_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_ATB_DIV_ATB_DIV_SHIFT                          8
#define    MDM_CLK_MGR_REG_ATB_DIV_ATB_DIV_MASK                           0x00000300
#define    MDM_CLK_MGR_REG_ATB_DIV_ATB_PLL_SELECT_SHIFT                   0
#define    MDM_CLK_MGR_REG_ATB_DIV_ATB_PLL_SELECT_MASK                    0x00000003
#define       MDM_CLK_MGR_REG_ATB_DIV_ATB_PLL_SELECT_CMD_REF_CRYSTAL_CLK  0x00000000
#define       MDM_CLK_MGR_REG_ATB_DIV_ATB_PLL_SELECT_CMD_PLLMDM_CLK       0x00000001

#define MDM_CLK_MGR_REG_DIV_TRIG_OFFSET                                   0x00000AFC
#define MDM_CLK_MGR_REG_DIV_TRIG_TYPE                                     UInt32
#define MDM_CLK_MGR_REG_DIV_TRIG_RESERVED_MASK                            0x7FFFFFF8
#define    MDM_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_SHIFT                31
#define    MDM_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MDM_CLK_MGR_REG_DIV_TRIG_ATB_TRIGGER_SHIFT                     2
#define    MDM_CLK_MGR_REG_DIV_TRIG_ATB_TRIGGER_MASK                      0x00000004
#define    MDM_CLK_MGR_REG_DIV_TRIG_RF_BB_SPI_TRIGGER_SHIFT               1
#define    MDM_CLK_MGR_REG_DIV_TRIG_RF_BB_SPI_TRIGGER_MASK                0x00000002
#define    MDM_CLK_MGR_REG_DIV_TRIG_APB_FREE_TRIGGER_SHIFT                0
#define    MDM_CLK_MGR_REG_DIV_TRIG_APB_FREE_TRIGGER_MASK                 0x00000001

#define MDM_CLK_MGR_REG_PLLMDMA_OFFSET                                    0x00000C00
#define MDM_CLK_MGR_REG_PLLMDMA_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMA_RESERVED_MASK                             0x68FC0004
#define    MDM_CLK_MGR_REG_PLLMDMA_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMA_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_LOCK_SHIFT                      28
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_LOCK_MASK                       0x10000000
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_PDIV_SHIFT                      24
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_PDIV_MASK                       0x07000000
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_NDIV_INT_SHIFT                  8
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_NDIV_INT_MASK                   0x0003FF00
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_LOCK_RAW_SHIFT                  7
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_LOCK_RAW_MASK                   0x00000080
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_LOCK_LATCH_SHIFT                6
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_LOCK_LATCH_MASK                 0x00000040
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_FLOCK_SHIFT                     5
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_FLOCK_MASK                      0x00000020
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_IDLE_PWRDWN_SW_OVRRIDE_SHIFT    4
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_IDLE_PWRDWN_SW_OVRRIDE_MASK     0x00000010
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_PWRDWN_SHIFT                    3
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_PWRDWN_MASK                     0x00000008
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_SOFT_POST_RESETB_SHIFT          1
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_SOFT_POST_RESETB_MASK           0x00000002
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_SOFT_RESETB_SHIFT               0
#define    MDM_CLK_MGR_REG_PLLMDMA_PLLMDM_SOFT_RESETB_MASK                0x00000001

#define MDM_CLK_MGR_REG_PLLMDMB_OFFSET                                    0x00000C04
#define MDM_CLK_MGR_REG_PLLMDMB_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMB_RESERVED_MASK                             0x7FF00000
#define    MDM_CLK_MGR_REG_PLLMDMB_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMB_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMB_PLLMDM_NDIV_FRAC_SHIFT                 0
#define    MDM_CLK_MGR_REG_PLLMDMB_PLLMDM_NDIV_FRAC_MASK                  0x000FFFFF

#define MDM_CLK_MGR_REG_PLLMDMC_OFFSET                                    0x00000C08
#define MDM_CLK_MGR_REG_PLLMDMC_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMC_RESERVED_MASK                             0x7FFF8000
#define    MDM_CLK_MGR_REG_PLLMDMC_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMC_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_MDEL_SHIFT                      12
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_MDEL_MASK                       0x00007000
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_LOAD_EN_SHIFT                   11
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_LOAD_EN_MASK                    0x00000800
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_HOLD_SHIFT                      10
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_HOLD_MASK                       0x00000400
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_ENB_CLKOUT_SHIFT                9
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_ENB_CLKOUT_MASK                 0x00000200
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_BYPCLK_EN_SHIFT                 8
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_BYPCLK_EN_MASK                  0x00000100
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_MDIV_SHIFT                      0
#define    MDM_CLK_MGR_REG_PLLMDMC_PLLMDM_MDIV_MASK                       0x000000FF

#define MDM_CLK_MGR_REG_PLLMDMD_OFFSET                                    0x00000C0C
#define MDM_CLK_MGR_REG_PLLMDMD_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMD_RESERVED_MASK                             0x7F000000
#define    MDM_CLK_MGR_REG_PLLMDMD_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMD_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_8PHASE_EN_SHIFT                 23
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_8PHASE_EN_MASK                  0x00800000
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_KP_SHIFT                        19
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_KP_MASK                         0x00780000
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_KI_SHIFT                        16
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_KI_MASK                         0x00070000
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_KA_SHIFT                        13
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_KA_MASK                         0x0000E000
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_FB_PHASE_EN_SHIFT               12
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_FB_PHASE_EN_MASK                0x00001000
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_FB_OFFSET_SHIFT                 0
#define    MDM_CLK_MGR_REG_PLLMDMD_PLLMDM_FB_OFFSET_MASK                  0x00000FFF

#define MDM_CLK_MGR_REG_PLLMDME_OFFSET                                    0x00000C10
#define MDM_CLK_MGR_REG_PLLMDME_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDME_RESERVED_MASK                             0x7FFE0000
#define    MDM_CLK_MGR_REG_PLLMDME_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDME_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDME_PLLMDM_SSC_MODE_SHIFT                  16
#define    MDM_CLK_MGR_REG_PLLMDME_PLLMDM_SSC_MODE_MASK                   0x00010000
#define    MDM_CLK_MGR_REG_PLLMDME_PLLMDM_SSC_STEP_SHIFT                  0
#define    MDM_CLK_MGR_REG_PLLMDME_PLLMDM_SSC_STEP_MASK                   0x0000FFFF

#define MDM_CLK_MGR_REG_PLLMDMF_OFFSET                                    0x00000C14
#define MDM_CLK_MGR_REG_PLLMDMF_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMF_RESERVED_MASK                             0x7C000000
#define    MDM_CLK_MGR_REG_PLLMDMF_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMF_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMF_PLLMDM_SSC_LIMIT_SHIFT                 0
#define    MDM_CLK_MGR_REG_PLLMDMF_PLLMDM_SSC_LIMIT_MASK                  0x03FFFFFF

#define MDM_CLK_MGR_REG_PLLMDMG_OFFSET                                    0x00000C18
#define MDM_CLK_MGR_REG_PLLMDMG_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMG_RESERVED_MASK                             0x70000000
#define    MDM_CLK_MGR_REG_PLLMDMG_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMG_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMG_PLLMDM_PLL_CONFIG_CTRL_SHIFT           0
#define    MDM_CLK_MGR_REG_PLLMDMG_PLLMDM_PLL_CONFIG_CTRL_MASK            0x0FFFFFFF

#define MDM_CLK_MGR_REG_PLLMDMH_OFFSET                                    0x00000C1C
#define MDM_CLK_MGR_REG_PLLMDMH_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMH_RESERVED_MASK                             0x7FFFF000
#define    MDM_CLK_MGR_REG_PLLMDMH_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMH_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMH_PLLMDM_STAT_OUT_SHIFT                  0
#define    MDM_CLK_MGR_REG_PLLMDMH_PLLMDM_STAT_OUT_MASK                   0x00000FFF

#define MDM_CLK_MGR_REG_PLLMDMJ_OFFSET                                    0x00000C24
#define MDM_CLK_MGR_REG_PLLMDMJ_TYPE                                      UInt32
#define MDM_CLK_MGR_REG_PLLMDMJ_RESERVED_MASK                             0x4FF00000
#define    MDM_CLK_MGR_REG_PLLMDMJ_PRIV_ACCESS_MODE_SHIFT                 31
#define    MDM_CLK_MGR_REG_PLLMDMJ_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MDM_CLK_MGR_REG_PLLMDMJ_PLLMDM_OFFSET_SW_CTL_SHIFT             29
#define    MDM_CLK_MGR_REG_PLLMDMJ_PLLMDM_OFFSET_SW_CTL_MASK              0x20000000
#define    MDM_CLK_MGR_REG_PLLMDMJ_PLLMDM_OFFSET_MODE_SHIFT               28
#define    MDM_CLK_MGR_REG_PLLMDMJ_PLLMDM_OFFSET_MODE_MASK                0x10000000
#define    MDM_CLK_MGR_REG_PLLMDMJ_PLLMDM_NDIV_FRAC_OFFSET_SHIFT          0
#define    MDM_CLK_MGR_REG_PLLMDMJ_PLLMDM_NDIV_FRAC_OFFSET_MASK           0x000FFFFF

#define MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_OFFSET                            0x00000C30
#define MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_TYPE                              UInt32
#define MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_RESERVED_MASK                     0x7FFFFFFE
#define    MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_PRIV_ACCESS_MODE_SHIFT         31
#define    MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_PRIV_ACCESS_MODE_MASK          0x80000000
#define    MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_WGM_HF_CLK_DIV2_SHIFT          0
#define    MDM_CLK_MGR_REG_WGM_HF_CLK_DIV2_WGM_HF_CLK_DIV2_MASK           0x00000001

#define MDM_CLK_MGR_REG_DEBUG0_OFFSET                                     0x00000E10
#define MDM_CLK_MGR_REG_DEBUG0_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_DEBUG0_RESERVED_MASK                              0x7FFFFFFC
#define    MDM_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_DEBUG0_PLLMDM_LOCK_LOST_SHIFT                  1
#define    MDM_CLK_MGR_REG_DEBUG0_PLLMDM_LOCK_LOST_MASK                   0x00000002
#define    MDM_CLK_MGR_REG_DEBUG0_PLLMDM_LOCK_IGNORE_SHIFT                0
#define    MDM_CLK_MGR_REG_DEBUG0_PLLMDM_LOCK_IGNORE_MASK                 0x00000001

#define MDM_CLK_MGR_REG_DEBUG1_OFFSET                                     0x00000E24
#define MDM_CLK_MGR_REG_DEBUG1_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_DEBUG1_RESERVED_MASK                              0x7FFFFFFA
#define    MDM_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_DEBUG1_APB_AON_CLK_EN_SHIFT                    2
#define    MDM_CLK_MGR_REG_DEBUG1_APB_AON_CLK_EN_MASK                     0x00000004
#define    MDM_CLK_MGR_REG_DEBUG1_APB_CLK_EN_SHIFT                        0
#define    MDM_CLK_MGR_REG_DEBUG1_APB_CLK_EN_MASK                         0x00000001

#define MDM_CLK_MGR_REG_CLKMON_OFFSET                                     0x00000E54
#define MDM_CLK_MGR_REG_CLKMON_TYPE                                       UInt32
#define MDM_CLK_MGR_REG_CLKMON_RESERVED_MASK                              0x7FFFFC00
#define    MDM_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_SHIFT                  31
#define    MDM_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MDM_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT                        8
#define    MDM_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK                         0x00000300
#define    MDM_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT                        0
#define    MDM_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK                         0x000000FF

#endif /* __BRCM_RDB_MDM_CLK_MGR_REG_H__ */



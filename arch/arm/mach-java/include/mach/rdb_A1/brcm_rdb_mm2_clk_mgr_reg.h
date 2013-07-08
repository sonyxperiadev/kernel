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

#ifndef __BRCM_RDB_MM2_CLK_MGR_REG_H__
#define __BRCM_RDB_MM2_CLK_MGR_REG_H__

#define MM2_CLK_MGR_REG_WR_ACCESS_OFFSET                                  0x00000000
#define MM2_CLK_MGR_REG_WR_ACCESS_TYPE                                    UInt32
#define MM2_CLK_MGR_REG_WR_ACCESS_RESERVED_MASK                           0x7F0000FE
#define    MM2_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT               31
#define    MM2_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                0x80000000
#define    MM2_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                       8
#define    MM2_CLK_MGR_REG_WR_ACCESS_PASSWORD_MASK                        0x00FFFF00
#define    MM2_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT                     0
#define    MM2_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK                      0x00000001

#define MM2_CLK_MGR_REG_POLICY_FREQ_OFFSET                                0x00000008
#define MM2_CLK_MGR_REG_POLICY_FREQ_TYPE                                  UInt32
#define MM2_CLK_MGR_REG_POLICY_FREQ_RESERVED_MASK                         0x7FFEF000
#define    MM2_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_SHIFT             31
#define    MM2_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY_BASE_312_SHIFT              16
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY_BASE_312_MASK               0x00010000
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT                 9
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_MASK                  0x00000E00
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT                 6
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_MASK                  0x000001C0
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT                 3
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_MASK                  0x00000038
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT                 0
#define    MM2_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK                  0x00000007

#define MM2_CLK_MGR_REG_POLICY_CTL_OFFSET                                 0x0000000C
#define MM2_CLK_MGR_REG_POLICY_CTL_TYPE                                   UInt32
#define MM2_CLK_MGR_REG_POLICY_CTL_RESERVED_MASK                          0x7FFFFFF0
#define    MM2_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_SHIFT              31
#define    MM2_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM2_CLK_MGR_REG_POLICY_CTL_TGT_VLD_SHIFT                       3
#define    MM2_CLK_MGR_REG_POLICY_CTL_TGT_VLD_MASK                        0x00000008
#define    MM2_CLK_MGR_REG_POLICY_CTL_GO_ATL_SHIFT                        2
#define    MM2_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK                         0x00000004
#define    MM2_CLK_MGR_REG_POLICY_CTL_GO_AC_SHIFT                         1
#define    MM2_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK                          0x00000002
#define    MM2_CLK_MGR_REG_POLICY_CTL_GO_SHIFT                            0
#define    MM2_CLK_MGR_REG_POLICY_CTL_GO_MASK                             0x00000001

#define MM2_CLK_MGR_REG_POLICY0_MASK_OFFSET                               0x00000010
#define MM2_CLK_MGR_REG_POLICY0_MASK_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_POLICY0_MASK_RESERVED_MASK                        0x7FFFBE7E
#define    MM2_CLK_MGR_REG_POLICY0_MASK_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_POLICY0_MASK_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_POLICY0_MASK_ATB_POLICY0_MASK_SHIFT            14
#define    MM2_CLK_MGR_REG_POLICY0_MASK_ATB_POLICY0_MASK_MASK             0x00004000
#define    MM2_CLK_MGR_REG_POLICY0_MASK_ISP2_POLICY0_MASK_SHIFT           8
#define    MM2_CLK_MGR_REG_POLICY0_MASK_ISP2_POLICY0_MASK_MASK            0x00000100
#define    MM2_CLK_MGR_REG_POLICY0_MASK_JPEG_POLICY0_MASK_SHIFT           7
#define    MM2_CLK_MGR_REG_POLICY0_MASK_JPEG_POLICY0_MASK_MASK            0x00000080
#define    MM2_CLK_MGR_REG_POLICY0_MASK_MM2_SWITCH_POLICY0_MASK_SHIFT     0
#define    MM2_CLK_MGR_REG_POLICY0_MASK_MM2_SWITCH_POLICY0_MASK_MASK      0x00000001

#define MM2_CLK_MGR_REG_POLICY1_MASK_OFFSET                               0x00000014
#define MM2_CLK_MGR_REG_POLICY1_MASK_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_POLICY1_MASK_RESERVED_MASK                        0x7FFFBE7E
#define    MM2_CLK_MGR_REG_POLICY1_MASK_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_POLICY1_MASK_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_POLICY1_MASK_ATB_POLICY1_MASK_SHIFT            14
#define    MM2_CLK_MGR_REG_POLICY1_MASK_ATB_POLICY1_MASK_MASK             0x00004000
#define    MM2_CLK_MGR_REG_POLICY1_MASK_ISP2_POLICY1_MASK_SHIFT           8
#define    MM2_CLK_MGR_REG_POLICY1_MASK_ISP2_POLICY1_MASK_MASK            0x00000100
#define    MM2_CLK_MGR_REG_POLICY1_MASK_JPEG_POLICY1_MASK_SHIFT           7
#define    MM2_CLK_MGR_REG_POLICY1_MASK_JPEG_POLICY1_MASK_MASK            0x00000080
#define    MM2_CLK_MGR_REG_POLICY1_MASK_MM2_SWITCH_POLICY1_MASK_SHIFT     0
#define    MM2_CLK_MGR_REG_POLICY1_MASK_MM2_SWITCH_POLICY1_MASK_MASK      0x00000001

#define MM2_CLK_MGR_REG_POLICY2_MASK_OFFSET                               0x00000018
#define MM2_CLK_MGR_REG_POLICY2_MASK_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_POLICY2_MASK_RESERVED_MASK                        0x7FFFBE7E
#define    MM2_CLK_MGR_REG_POLICY2_MASK_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_POLICY2_MASK_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_POLICY2_MASK_ATB_POLICY2_MASK_SHIFT            14
#define    MM2_CLK_MGR_REG_POLICY2_MASK_ATB_POLICY2_MASK_MASK             0x00004000
#define    MM2_CLK_MGR_REG_POLICY2_MASK_ISP2_POLICY2_MASK_SHIFT           8
#define    MM2_CLK_MGR_REG_POLICY2_MASK_ISP2_POLICY2_MASK_MASK            0x00000100
#define    MM2_CLK_MGR_REG_POLICY2_MASK_JPEG_POLICY2_MASK_SHIFT           7
#define    MM2_CLK_MGR_REG_POLICY2_MASK_JPEG_POLICY2_MASK_MASK            0x00000080
#define    MM2_CLK_MGR_REG_POLICY2_MASK_MM2_SWITCH_POLICY2_MASK_SHIFT     0
#define    MM2_CLK_MGR_REG_POLICY2_MASK_MM2_SWITCH_POLICY2_MASK_MASK      0x00000001

#define MM2_CLK_MGR_REG_POLICY3_MASK_OFFSET                               0x0000001C
#define MM2_CLK_MGR_REG_POLICY3_MASK_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_POLICY3_MASK_RESERVED_MASK                        0x7FFFBE7E
#define    MM2_CLK_MGR_REG_POLICY3_MASK_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_POLICY3_MASK_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_POLICY3_MASK_ATB_POLICY3_MASK_SHIFT            14
#define    MM2_CLK_MGR_REG_POLICY3_MASK_ATB_POLICY3_MASK_MASK             0x00004000
#define    MM2_CLK_MGR_REG_POLICY3_MASK_ISP2_POLICY3_MASK_SHIFT           8
#define    MM2_CLK_MGR_REG_POLICY3_MASK_ISP2_POLICY3_MASK_MASK            0x00000100
#define    MM2_CLK_MGR_REG_POLICY3_MASK_JPEG_POLICY3_MASK_SHIFT           7
#define    MM2_CLK_MGR_REG_POLICY3_MASK_JPEG_POLICY3_MASK_MASK            0x00000080
#define    MM2_CLK_MGR_REG_POLICY3_MASK_MM2_SWITCH_POLICY3_MASK_SHIFT     0
#define    MM2_CLK_MGR_REG_POLICY3_MASK_MM2_SWITCH_POLICY3_MASK_MASK      0x00000001

#define MM2_CLK_MGR_REG_INTEN_OFFSET                                      0x00000020
#define MM2_CLK_MGR_REG_INTEN_TYPE                                        UInt32
#define MM2_CLK_MGR_REG_INTEN_RESERVED_MASK                               0x7FFFFFFC
#define    MM2_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM2_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM2_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_SHIFT                   1
#define    MM2_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_MASK                    0x00000002
#define    MM2_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_SHIFT                   0
#define    MM2_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_MASK                    0x00000001

#define MM2_CLK_MGR_REG_INTSTAT_OFFSET                                    0x00000024
#define MM2_CLK_MGR_REG_INTSTAT_TYPE                                      UInt32
#define MM2_CLK_MGR_REG_INTSTAT_RESERVED_MASK                             0x7FFFFFFC
#define    MM2_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_SHIFT                 31
#define    MM2_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MM2_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_SHIFT                  1
#define    MM2_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_MASK                   0x00000002
#define    MM2_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_SHIFT                  0
#define    MM2_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_MASK                   0x00000001

#define MM2_CLK_MGR_REG_VLT_PERI_OFFSET                                   0x00000030
#define MM2_CLK_MGR_REG_VLT_PERI_TYPE                                     UInt32
#define MM2_CLK_MGR_REG_VLT_PERI_RESERVED_MASK                            0x7FFFFF00
#define    MM2_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_SHIFT                31
#define    MM2_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MM2_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT                   4
#define    MM2_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK                    0x000000F0
#define    MM2_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT                 0
#define    MM2_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK                  0x0000000F

#define MM2_CLK_MGR_REG_LVM_EN_OFFSET                                     0x00000034
#define MM2_CLK_MGR_REG_LVM_EN_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_LVM_EN_RESERVED_MASK                              0x7FFFFFFE
#define    MM2_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_SHIFT                  0
#define    MM2_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK                   0x00000001

#define MM2_CLK_MGR_REG_LVM0_3_OFFSET                                     0x00000038
#define MM2_CLK_MGR_REG_LVM0_3_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_LVM0_3_RESERVED_MASK                              0x7FFF0000
#define    MM2_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_SHIFT                      12
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_MASK                       0x0000F000
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_SHIFT                      8
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_MASK                       0x00000F00
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_SHIFT                      4
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_MASK                       0x000000F0
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_SHIFT                      0
#define    MM2_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK                       0x0000000F

#define MM2_CLK_MGR_REG_LVM4_7_OFFSET                                     0x0000003C
#define MM2_CLK_MGR_REG_LVM4_7_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_LVM4_7_RESERVED_MASK                              0x7FFF0000
#define    MM2_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_SHIFT                      12
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_MASK                       0x0000F000
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_SHIFT                      8
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_MASK                       0x00000F00
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_SHIFT                      4
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_MASK                       0x000000F0
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_SHIFT                      0
#define    MM2_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_MASK                       0x0000000F

#define MM2_CLK_MGR_REG_VLT0_3_OFFSET                                     0x00000040
#define MM2_CLK_MGR_REG_VLT0_3_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_VLT0_3_RESERVED_MASK                              0x7FFF0000
#define    MM2_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT                      12
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_MASK                       0x0000F000
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT                      8
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_MASK                       0x00000F00
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT                      4
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_MASK                       0x000000F0
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT                      0
#define    MM2_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK                       0x0000000F

#define MM2_CLK_MGR_REG_VLT4_7_OFFSET                                     0x00000044
#define MM2_CLK_MGR_REG_VLT4_7_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_VLT4_7_RESERVED_MASK                              0x7FFF0000
#define    MM2_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT                      12
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_MASK                       0x0000F000
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT                      8
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_MASK                       0x00000F00
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT                      4
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_MASK                       0x000000F0
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT                      0
#define    MM2_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_MASK                       0x0000000F

#define MM2_CLK_MGR_REG_BUS_QUIESC_OFFSET                                 0x00000100
#define MM2_CLK_MGR_REG_BUS_QUIESC_TYPE                                   UInt32
#define MM2_CLK_MGR_REG_BUS_QUIESC_RESERVED_MASK                          0x7FFFBFFF
#define    MM2_CLK_MGR_REG_BUS_QUIESC_PRIV_ACCESS_MODE_SHIFT              31
#define    MM2_CLK_MGR_REG_BUS_QUIESC_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM2_CLK_MGR_REG_BUS_QUIESC_MM2_SWITCH_AXI_SWITCH_REQGNT_INH_SHIFT 14
#define    MM2_CLK_MGR_REG_BUS_QUIESC_MM2_SWITCH_AXI_SWITCH_REQGNT_INH_MASK 0x00004000

#define MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_OFFSET                     0x00000200
#define MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_TYPE                       UInt32
#define MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_RESERVED_MASK              0x6FFEFCFD
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_PRIV_ACCESS_MODE_SHIFT  31
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_PRIV_ACCESS_MODE_MASK   0x80000000
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_VOLTAGE_LEVEL_SHIFT 28
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_VOLTAGE_LEVEL_MASK 0x10000000
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_STPRSTS_SHIFT 16
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_STPRSTS_MASK 0x00010000
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HYST_VAL_SHIFT 9
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HYST_VAL_MASK 0x00000200
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HYST_EN_SHIFT 8
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HYST_EN_MASK 0x00000100
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HW_SW_GATING_SEL_SHIFT 1
#define    MM2_CLK_MGR_REG_MM2_AXI_SWITCH_CLKGATE_MM2_SWITCH_AXI_HW_SW_GATING_SEL_MASK 0x00000002

#define MM2_CLK_MGR_REG_ISP2_CLKGATE_OFFSET                               0x00000258
#define MM2_CLK_MGR_REG_ISP2_CLKGATE_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_ISP2_CLKGATE_RESERVED_MASK                        0x6FFEFCFC
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_VOLTAGE_LEVEL_SHIFT          28
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_VOLTAGE_LEVEL_MASK           0x10000000
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_STPRSTS_SHIFT            16
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_STPRSTS_MASK             0x00010000
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HYST_VAL_SHIFT           9
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HYST_VAL_MASK            0x00000200
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HYST_EN_SHIFT            8
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HYST_EN_MASK             0x00000100
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HW_SW_GATING_SEL_SHIFT   1
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_HW_SW_GATING_SEL_MASK    0x00000002
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_CLK_EN_SHIFT             0
#define    MM2_CLK_MGR_REG_ISP2_CLKGATE_ISP2_AXI_CLK_EN_MASK              0x00000001

#define MM2_CLK_MGR_REG_JPEG_CLKGATE_OFFSET                               0x00000270
#define MM2_CLK_MGR_REG_JPEG_CLKGATE_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_JPEG_CLKGATE_RESERVED_MASK                        0x6FFEFCFC
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_VOLTAGE_LEVEL_SHIFT          28
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_VOLTAGE_LEVEL_MASK           0x10000000
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_STPRSTS_SHIFT            16
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_STPRSTS_MASK             0x00010000
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HYST_VAL_SHIFT           9
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HYST_VAL_MASK            0x00000200
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HYST_EN_SHIFT            8
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HYST_EN_MASK             0x00000100
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HW_SW_GATING_SEL_SHIFT   1
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_HW_SW_GATING_SEL_MASK    0x00000002
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_CLK_EN_SHIFT             0
#define    MM2_CLK_MGR_REG_JPEG_CLKGATE_JPEG_AXI_CLK_EN_MASK              0x00000001

#define MM2_CLK_MGR_REG_ATB_CLKGATE_OFFSET                                0x00000298
#define MM2_CLK_MGR_REG_ATB_CLKGATE_TYPE                                  UInt32
#define MM2_CLK_MGR_REG_ATB_CLKGATE_RESERVED_MASK                         0x6FFEFCFC
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_PRIV_ACCESS_MODE_SHIFT             31
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_VOLTAGE_LEVEL_SHIFT            28
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_VOLTAGE_LEVEL_MASK             0x10000000
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_STPRSTS_SHIFT              16
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_STPRSTS_MASK               0x00010000
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HYST_VAL_SHIFT             9
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HYST_VAL_MASK              0x00000200
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HYST_EN_SHIFT              8
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HYST_EN_MASK               0x00000100
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HW_SW_GATING_SEL_SHIFT     1
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_HW_SW_GATING_SEL_MASK      0x00000002
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_CLK_EN_SHIFT               0
#define    MM2_CLK_MGR_REG_ATB_CLKGATE_ATB_AXI_CLK_EN_MASK                0x00000001

#define MM2_CLK_MGR_REG_AXI_DIV_OFFSET                                    0x00000A00
#define MM2_CLK_MGR_REG_AXI_DIV_TYPE                                      UInt32
#define MM2_CLK_MGR_REG_AXI_DIV_RESERVED_MASK                             0x7CEFE0FC
#define    MM2_CLK_MGR_REG_AXI_DIV_PRIV_ACCESS_MODE_SHIFT                 31
#define    MM2_CLK_MGR_REG_AXI_DIV_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_DIV_OVERRIDE_SHIFT      25
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_DIV_OVERRIDE_MASK       0x02000000
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_OVERRIDE_SHIFT 24
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_OVERRIDE_MASK 0x01000000
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_TRIGGER_OVERRIDE_SHIFT  20
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_TRIGGER_OVERRIDE_MASK   0x00100000
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_DIV_SHIFT               8
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_DIV_MASK                0x00001F00
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_SHIFT        0
#define    MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_MASK         0x00000003
#define       MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000000
#define       MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_CMD_VAR_500M_CLK 0x00000001
#define       MM2_CLK_MGR_REG_AXI_DIV_MM2_SWITCH_AXI_PLL_SELECT_CMD_VAR_312M_CLK 0x00000002

#define MM2_CLK_MGR_REG_DIV_TRIG_OFFSET                                   0x00000AFC
#define MM2_CLK_MGR_REG_DIV_TRIG_TYPE                                     UInt32
#define MM2_CLK_MGR_REG_DIV_TRIG_RESERVED_MASK                            0x7FFFFFFE
#define    MM2_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_SHIFT                31
#define    MM2_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MM2_CLK_MGR_REG_DIV_TRIG_MM2_SWITCH_AXI_TRIGGER_SHIFT          0
#define    MM2_CLK_MGR_REG_DIV_TRIG_MM2_SWITCH_AXI_TRIGGER_MASK           0x00000001

#define MM2_CLK_MGR_REG_DEBUG0_OFFSET                                     0x00000E00
#define MM2_CLK_MGR_REG_DEBUG0_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_DEBUG0_RESERVED_MASK                              0x7FFFFFFE
#define    MM2_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_DEBUG0_MM2_SWITCH_AXI_POLICY_OVERRIDE_SHIFT    0
#define    MM2_CLK_MGR_REG_DEBUG0_MM2_SWITCH_AXI_POLICY_OVERRIDE_MASK     0x00000001

#define MM2_CLK_MGR_REG_DEBUG1_OFFSET                                     0x00000E04
#define MM2_CLK_MGR_REG_DEBUG1_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_DEBUG1_RESERVED_MASK                              0x6FFEDFFF
#define    MM2_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_DEBUG1_ATB_AXI_POLICY_OVERRIDE_SHIFT           28
#define    MM2_CLK_MGR_REG_DEBUG1_ATB_AXI_POLICY_OVERRIDE_MASK            0x10000000
#define    MM2_CLK_MGR_REG_DEBUG1_JPEG_AXI_POLICY_OVERRIDE_SHIFT          16
#define    MM2_CLK_MGR_REG_DEBUG1_JPEG_AXI_POLICY_OVERRIDE_MASK           0x00010000
#define    MM2_CLK_MGR_REG_DEBUG1_ISP2_AXI_POLICY_OVERRIDE_SHIFT          13
#define    MM2_CLK_MGR_REG_DEBUG1_ISP2_AXI_POLICY_OVERRIDE_MASK           0x00002000

#define MM2_CLK_MGR_REG_DEBUG2_OFFSET                                     0x00000E08
#define MM2_CLK_MGR_REG_DEBUG2_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_DEBUG2_RESERVED_MASK                              0x7FFFFFFE
#define    MM2_CLK_MGR_REG_DEBUG2_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_DEBUG2_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_DEBUG2_MM2_SWITCH_AXI_POLICY_OVERRIDE_VALUE_SHIFT 0
#define    MM2_CLK_MGR_REG_DEBUG2_MM2_SWITCH_AXI_POLICY_OVERRIDE_VALUE_MASK 0x00000001

#define MM2_CLK_MGR_REG_DEBUG3_OFFSET                                     0x00000E0C
#define MM2_CLK_MGR_REG_DEBUG3_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_DEBUG3_RESERVED_MASK                              0x6FFEDFFF
#define    MM2_CLK_MGR_REG_DEBUG3_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_DEBUG3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_DEBUG3_ATB_AXI_POLICY_OVERRIDE_VALUE_SHIFT     28
#define    MM2_CLK_MGR_REG_DEBUG3_ATB_AXI_POLICY_OVERRIDE_VALUE_MASK      0x10000000
#define    MM2_CLK_MGR_REG_DEBUG3_JPEG_AXI_POLICY_OVERRIDE_VALUE_SHIFT    16
#define    MM2_CLK_MGR_REG_DEBUG3_JPEG_AXI_POLICY_OVERRIDE_VALUE_MASK     0x00010000
#define    MM2_CLK_MGR_REG_DEBUG3_ISP2_AXI_POLICY_OVERRIDE_VALUE_SHIFT    13
#define    MM2_CLK_MGR_REG_DEBUG3_ISP2_AXI_POLICY_OVERRIDE_VALUE_MASK     0x00002000

#define MM2_CLK_MGR_REG_DEBUG4_OFFSET                                     0x00000E10
#define MM2_CLK_MGR_REG_DEBUG4_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_DEBUG4_RESERVED_MASK                              0x7FFFFFFE
#define    MM2_CLK_MGR_REG_DEBUG4_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_DEBUG4_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_DEBUG4_MM2_SWITCH_AXI_CLK_EN_SHIFT             0
#define    MM2_CLK_MGR_REG_DEBUG4_MM2_SWITCH_AXI_CLK_EN_MASK              0x00000001

#define MM2_CLK_MGR_REG_SCAN_DIV_OFFSET                                   0x00000E34
#define MM2_CLK_MGR_REG_SCAN_DIV_TYPE                                     UInt32
#define MM2_CLK_MGR_REG_SCAN_DIV_RESERVED_MASK                            0x7FFFFEF8
#define    MM2_CLK_MGR_REG_SCAN_DIV_PRIV_ACCESS_MODE_SHIFT                31
#define    MM2_CLK_MGR_REG_SCAN_DIV_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MM2_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_TRIGGER_SHIFT               8
#define    MM2_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_TRIGGER_MASK                0x00000100
#define    MM2_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_DIV_SHIFT                   0
#define    MM2_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_DIV_MASK                    0x00000007

#define MM2_CLK_MGR_REG_ACTIVITY_MON1_OFFSET                              0x00000E40
#define MM2_CLK_MGR_REG_ACTIVITY_MON1_TYPE                                UInt32
#define MM2_CLK_MGR_REG_ACTIVITY_MON1_RESERVED_MASK                       0x7DFFF9FE
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_PRIV_ACCESS_MODE_SHIFT           31
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_PRIV_ACCESS_MODE_MASK            0x80000000
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_ATB_AXI_ACTVSTS_SHIFT            25
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_ATB_AXI_ACTVSTS_MASK             0x02000000
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_ISP2_AXI_ACTVSTS_SHIFT           10
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_ISP2_AXI_ACTVSTS_MASK            0x00000400
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_JPEG_AXI_ACTVSTS_SHIFT           9
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_JPEG_AXI_ACTVSTS_MASK            0x00000200
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_MM2_SWITCH_AXI_ACTVSTS_SHIFT     0
#define    MM2_CLK_MGR_REG_ACTIVITY_MON1_MM2_SWITCH_AXI_ACTVSTS_MASK      0x00000001

#define MM2_CLK_MGR_REG_ACTIVITY_MON2_OFFSET                              0x00000E44
#define MM2_CLK_MGR_REG_ACTIVITY_MON2_TYPE                                UInt32
#define MM2_CLK_MGR_REG_ACTIVITY_MON2_RESERVED_MASK                       0x7FFFFFC0
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_PRIV_ACCESS_MODE_SHIFT           31
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_PRIV_ACCESS_MODE_MASK            0x80000000
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN3_ACTVSTS_SHIFT          5
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN3_ACTVSTS_MASK           0x00000020
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN2_ACTVSTS_SHIFT          4
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN2_ACTVSTS_MASK           0x00000010
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN1_ACTVSTS_SHIFT          3
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN1_ACTVSTS_MASK           0x00000008
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN4_ACTVSTS_SHIFT          2
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN4_ACTVSTS_MASK           0x00000004
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN5_ACTVSTS_SHIFT          1
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN5_ACTVSTS_MASK           0x00000002
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN0_ACTVSTS_SHIFT          0
#define    MM2_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN0_ACTVSTS_MASK           0x00000001

#define MM2_CLK_MGR_REG_CLKMON_OFFSET                                     0x00000E54
#define MM2_CLK_MGR_REG_CLKMON_TYPE                                       UInt32
#define MM2_CLK_MGR_REG_CLKMON_RESERVED_MASK                              0x7FFFFC00
#define    MM2_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM2_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM2_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT                        8
#define    MM2_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK                         0x00000300
#define    MM2_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT                        0
#define    MM2_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK                         0x000000FF

#define MM2_CLK_MGR_REG_POLICY_DBG_OFFSET                                 0x00000EC0
#define MM2_CLK_MGR_REG_POLICY_DBG_TYPE                                   UInt32
#define MM2_CLK_MGR_REG_POLICY_DBG_RESERVED_MASK                          0x7FFFFC00
#define    MM2_CLK_MGR_REG_POLICY_DBG_PRIV_ACCESS_MODE_SHIFT              31
#define    MM2_CLK_MGR_REG_POLICY_DBG_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM2_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT                      7
#define    MM2_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_MASK                       0x00000380
#define    MM2_CLK_MGR_REG_POLICY_DBG_TGT_FREQ_SHIFT                      4
#define    MM2_CLK_MGR_REG_POLICY_DBG_TGT_FREQ_MASK                       0x00000070
#define    MM2_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT                    2
#define    MM2_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK                     0x0000000C
#define    MM2_CLK_MGR_REG_POLICY_DBG_TGT_POLICY_SHIFT                    0
#define    MM2_CLK_MGR_REG_POLICY_DBG_TGT_POLICY_MASK                     0x00000003

#define MM2_CLK_MGR_REG_TGTMASK_DBG1_OFFSET                               0x00000EC4
#define MM2_CLK_MGR_REG_TGTMASK_DBG1_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_TGTMASK_DBG1_RESERVED_MASK                        0x7FFFECFE
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_ATB_TARGET_MASK_SHIFT             12
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_ATB_TARGET_MASK_MASK              0x00001000
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_ISP2_TARGET_MASK_SHIFT            9
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_ISP2_TARGET_MASK_MASK             0x00000200
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_JPEG_TARGET_MASK_SHIFT            8
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_JPEG_TARGET_MASK_MASK             0x00000100
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_MM2_SWITCH_TARGET_MASK_SHIFT      0
#define    MM2_CLK_MGR_REG_TGTMASK_DBG1_MM2_SWITCH_TARGET_MASK_MASK       0x00000001

#define MM2_CLK_MGR_REG_ACTMASK_DBG1_OFFSET                               0x00000ED4
#define MM2_CLK_MGR_REG_ACTMASK_DBG1_TYPE                                 UInt32
#define MM2_CLK_MGR_REG_ACTMASK_DBG1_RESERVED_MASK                        0x7FFFECFE
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_PRIV_ACCESS_MODE_SHIFT            31
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_ATB_ACTIVE_MASK_SHIFT             12
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_ATB_ACTIVE_MASK_MASK              0x00001000
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_ISP2_ACTIVE_MASK_SHIFT            9
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_ISP2_ACTIVE_MASK_MASK             0x00000200
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_JPEG_ACTIVE_MASK_SHIFT            8
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_JPEG_ACTIVE_MASK_MASK             0x00000100
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_MM2_SWITCH_ACTIVE_MASK_SHIFT      0
#define    MM2_CLK_MGR_REG_ACTMASK_DBG1_MM2_SWITCH_ACTIVE_MASK_MASK       0x00000001

#endif /* __BRCM_RDB_MM2_CLK_MGR_REG_H__ */



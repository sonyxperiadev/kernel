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

#ifndef __BRCM_RDB_MM_CLK_MGR_REG_H__
#define __BRCM_RDB_MM_CLK_MGR_REG_H__

#define MM_CLK_MGR_REG_WR_ACCESS_OFFSET                                   0x00000000
#define MM_CLK_MGR_REG_WR_ACCESS_TYPE                                     UInt32
#define MM_CLK_MGR_REG_WR_ACCESS_RESERVED_MASK                            0x7F0000FE
#define    MM_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT                31
#define    MM_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    MM_CLK_MGR_REG_WR_ACCESS_PASSWORD_SHIFT                        8
#define    MM_CLK_MGR_REG_WR_ACCESS_PASSWORD_MASK                         0x00FFFF00
#define    MM_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT                      0
#define    MM_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK                       0x00000001

#define MM_CLK_MGR_REG_POLICY_FREQ_OFFSET                                 0x00000008
#define MM_CLK_MGR_REG_POLICY_FREQ_TYPE                                   UInt32
#define MM_CLK_MGR_REG_POLICY_FREQ_RESERVED_MASK                          0x7FFFF000
#define    MM_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_SHIFT              31
#define    MM_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT                  9
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_MASK                   0x00000E00
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT                  6
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_MASK                   0x000001C0
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT                  3
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_MASK                   0x00000038
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT                  0
#define    MM_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK                   0x00000007

#define MM_CLK_MGR_REG_POLICY_CTL_OFFSET                                  0x0000000C
#define MM_CLK_MGR_REG_POLICY_CTL_TYPE                                    UInt32
#define MM_CLK_MGR_REG_POLICY_CTL_RESERVED_MASK                           0x7FFFFFF0
#define    MM_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_SHIFT               31
#define    MM_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_MASK                0x80000000
#define    MM_CLK_MGR_REG_POLICY_CTL_TGT_VLD_SHIFT                        3
#define    MM_CLK_MGR_REG_POLICY_CTL_TGT_VLD_MASK                         0x00000008
#define    MM_CLK_MGR_REG_POLICY_CTL_GO_ATL_SHIFT                         2
#define    MM_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK                          0x00000004
#define    MM_CLK_MGR_REG_POLICY_CTL_GO_AC_SHIFT                          1
#define    MM_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK                           0x00000002
#define    MM_CLK_MGR_REG_POLICY_CTL_GO_SHIFT                             0
#define    MM_CLK_MGR_REG_POLICY_CTL_GO_MASK                              0x00000001

#define MM_CLK_MGR_REG_POLICY0_MASK_OFFSET                                0x00000010
#define MM_CLK_MGR_REG_POLICY0_MASK_TYPE                                  UInt32
#define MM_CLK_MGR_REG_POLICY0_MASK_RESERVED_MASK                         0x7FFFEA26
#define    MM_CLK_MGR_REG_POLICY0_MASK_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_POLICY0_MASK_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_POLICY0_MASK_MM_APB_POLICY0_MASK_SHIFT          12
#define    MM_CLK_MGR_REG_POLICY0_MASK_MM_APB_POLICY0_MASK_MASK           0x00001000
#define    MM_CLK_MGR_REG_POLICY0_MASK_MM_DMA_POLICY0_MASK_SHIFT          10
#define    MM_CLK_MGR_REG_POLICY0_MASK_MM_DMA_POLICY0_MASK_MASK           0x00000400
#define    MM_CLK_MGR_REG_POLICY0_MASK_ISP_POLICY0_MASK_SHIFT             8
#define    MM_CLK_MGR_REG_POLICY0_MASK_ISP_POLICY0_MASK_MASK              0x00000100
#define    MM_CLK_MGR_REG_POLICY0_MASK_V3D_POLICY0_MASK_SHIFT             7
#define    MM_CLK_MGR_REG_POLICY0_MASK_V3D_POLICY0_MASK_MASK              0x00000080
#define    MM_CLK_MGR_REG_POLICY0_MASK_SMI_POLICY0_MASK_SHIFT             6
#define    MM_CLK_MGR_REG_POLICY0_MASK_SMI_POLICY0_MASK_MASK              0x00000040
#define    MM_CLK_MGR_REG_POLICY0_MASK_CSI0_POLICY0_MASK_SHIFT            4
#define    MM_CLK_MGR_REG_POLICY0_MASK_CSI0_POLICY0_MASK_MASK             0x00000010
#define    MM_CLK_MGR_REG_POLICY0_MASK_SPI_POLICY0_MASK_SHIFT             3
#define    MM_CLK_MGR_REG_POLICY0_MASK_SPI_POLICY0_MASK_MASK              0x00000008
#define    MM_CLK_MGR_REG_POLICY0_MASK_MM_SWITCH_POLICY0_MASK_SHIFT       0
#define    MM_CLK_MGR_REG_POLICY0_MASK_MM_SWITCH_POLICY0_MASK_MASK        0x00000001

#define MM_CLK_MGR_REG_POLICY1_MASK_OFFSET                                0x00000014
#define MM_CLK_MGR_REG_POLICY1_MASK_TYPE                                  UInt32
#define MM_CLK_MGR_REG_POLICY1_MASK_RESERVED_MASK                         0x7FFFEA26
#define    MM_CLK_MGR_REG_POLICY1_MASK_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_POLICY1_MASK_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_POLICY1_MASK_MM_APB_POLICY1_MASK_SHIFT          12
#define    MM_CLK_MGR_REG_POLICY1_MASK_MM_APB_POLICY1_MASK_MASK           0x00001000
#define    MM_CLK_MGR_REG_POLICY1_MASK_MM_DMA_POLICY1_MASK_SHIFT          10
#define    MM_CLK_MGR_REG_POLICY1_MASK_MM_DMA_POLICY1_MASK_MASK           0x00000400
#define    MM_CLK_MGR_REG_POLICY1_MASK_ISP_POLICY1_MASK_SHIFT             8
#define    MM_CLK_MGR_REG_POLICY1_MASK_ISP_POLICY1_MASK_MASK              0x00000100
#define    MM_CLK_MGR_REG_POLICY1_MASK_V3D_POLICY1_MASK_SHIFT             7
#define    MM_CLK_MGR_REG_POLICY1_MASK_V3D_POLICY1_MASK_MASK              0x00000080
#define    MM_CLK_MGR_REG_POLICY1_MASK_SMI_POLICY1_MASK_SHIFT             6
#define    MM_CLK_MGR_REG_POLICY1_MASK_SMI_POLICY1_MASK_MASK              0x00000040
#define    MM_CLK_MGR_REG_POLICY1_MASK_CSI0_POLICY1_MASK_SHIFT            4
#define    MM_CLK_MGR_REG_POLICY1_MASK_CSI0_POLICY1_MASK_MASK             0x00000010
#define    MM_CLK_MGR_REG_POLICY1_MASK_SPI_POLICY1_MASK_SHIFT             3
#define    MM_CLK_MGR_REG_POLICY1_MASK_SPI_POLICY1_MASK_MASK              0x00000008
#define    MM_CLK_MGR_REG_POLICY1_MASK_MM_SWITCH_POLICY1_MASK_SHIFT       0
#define    MM_CLK_MGR_REG_POLICY1_MASK_MM_SWITCH_POLICY1_MASK_MASK        0x00000001

#define MM_CLK_MGR_REG_POLICY2_MASK_OFFSET                                0x00000018
#define MM_CLK_MGR_REG_POLICY2_MASK_TYPE                                  UInt32
#define MM_CLK_MGR_REG_POLICY2_MASK_RESERVED_MASK                         0x7FFFEA26
#define    MM_CLK_MGR_REG_POLICY2_MASK_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_POLICY2_MASK_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_POLICY2_MASK_MM_APB_POLICY2_MASK_SHIFT          12
#define    MM_CLK_MGR_REG_POLICY2_MASK_MM_APB_POLICY2_MASK_MASK           0x00001000
#define    MM_CLK_MGR_REG_POLICY2_MASK_MM_DMA_POLICY2_MASK_SHIFT          10
#define    MM_CLK_MGR_REG_POLICY2_MASK_MM_DMA_POLICY2_MASK_MASK           0x00000400
#define    MM_CLK_MGR_REG_POLICY2_MASK_ISP_POLICY2_MASK_SHIFT             8
#define    MM_CLK_MGR_REG_POLICY2_MASK_ISP_POLICY2_MASK_MASK              0x00000100
#define    MM_CLK_MGR_REG_POLICY2_MASK_V3D_POLICY2_MASK_SHIFT             7
#define    MM_CLK_MGR_REG_POLICY2_MASK_V3D_POLICY2_MASK_MASK              0x00000080
#define    MM_CLK_MGR_REG_POLICY2_MASK_SMI_POLICY2_MASK_SHIFT             6
#define    MM_CLK_MGR_REG_POLICY2_MASK_SMI_POLICY2_MASK_MASK              0x00000040
#define    MM_CLK_MGR_REG_POLICY2_MASK_CSI0_POLICY2_MASK_SHIFT            4
#define    MM_CLK_MGR_REG_POLICY2_MASK_CSI0_POLICY2_MASK_MASK             0x00000010
#define    MM_CLK_MGR_REG_POLICY2_MASK_SPI_POLICY2_MASK_SHIFT             3
#define    MM_CLK_MGR_REG_POLICY2_MASK_SPI_POLICY2_MASK_MASK              0x00000008
#define    MM_CLK_MGR_REG_POLICY2_MASK_MM_SWITCH_POLICY2_MASK_SHIFT       0
#define    MM_CLK_MGR_REG_POLICY2_MASK_MM_SWITCH_POLICY2_MASK_MASK        0x00000001

#define MM_CLK_MGR_REG_POLICY3_MASK_OFFSET                                0x0000001C
#define MM_CLK_MGR_REG_POLICY3_MASK_TYPE                                  UInt32
#define MM_CLK_MGR_REG_POLICY3_MASK_RESERVED_MASK                         0x7FFFEA26
#define    MM_CLK_MGR_REG_POLICY3_MASK_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_POLICY3_MASK_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_POLICY3_MASK_MM_APB_POLICY3_MASK_SHIFT          12
#define    MM_CLK_MGR_REG_POLICY3_MASK_MM_APB_POLICY3_MASK_MASK           0x00001000
#define    MM_CLK_MGR_REG_POLICY3_MASK_MM_DMA_POLICY3_MASK_SHIFT          10
#define    MM_CLK_MGR_REG_POLICY3_MASK_MM_DMA_POLICY3_MASK_MASK           0x00000400
#define    MM_CLK_MGR_REG_POLICY3_MASK_ISP_POLICY3_MASK_SHIFT             8
#define    MM_CLK_MGR_REG_POLICY3_MASK_ISP_POLICY3_MASK_MASK              0x00000100
#define    MM_CLK_MGR_REG_POLICY3_MASK_V3D_POLICY3_MASK_SHIFT             7
#define    MM_CLK_MGR_REG_POLICY3_MASK_V3D_POLICY3_MASK_MASK              0x00000080
#define    MM_CLK_MGR_REG_POLICY3_MASK_SMI_POLICY3_MASK_SHIFT             6
#define    MM_CLK_MGR_REG_POLICY3_MASK_SMI_POLICY3_MASK_MASK              0x00000040
#define    MM_CLK_MGR_REG_POLICY3_MASK_CSI0_POLICY3_MASK_SHIFT            4
#define    MM_CLK_MGR_REG_POLICY3_MASK_CSI0_POLICY3_MASK_MASK             0x00000010
#define    MM_CLK_MGR_REG_POLICY3_MASK_SPI_POLICY3_MASK_SHIFT             3
#define    MM_CLK_MGR_REG_POLICY3_MASK_SPI_POLICY3_MASK_MASK              0x00000008
#define    MM_CLK_MGR_REG_POLICY3_MASK_MM_SWITCH_POLICY3_MASK_SHIFT       0
#define    MM_CLK_MGR_REG_POLICY3_MASK_MM_SWITCH_POLICY3_MASK_MASK        0x00000001

#define MM_CLK_MGR_REG_INTEN_OFFSET                                       0x00000020
#define MM_CLK_MGR_REG_INTEN_TYPE                                         UInt32
#define MM_CLK_MGR_REG_INTEN_RESERVED_MASK                                0x7FFFFFFC
#define    MM_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_SHIFT                    31
#define    MM_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_MASK                     0x80000000
#define    MM_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_SHIFT                    1
#define    MM_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_MASK                     0x00000002
#define    MM_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_SHIFT                    0
#define    MM_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_MASK                     0x00000001

#define MM_CLK_MGR_REG_INTSTAT_OFFSET                                     0x00000024
#define MM_CLK_MGR_REG_INTSTAT_TYPE                                       UInt32
#define MM_CLK_MGR_REG_INTSTAT_RESERVED_MASK                              0x7FFFFFFC
#define    MM_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_SHIFT                   1
#define    MM_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_MASK                    0x00000002
#define    MM_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_SHIFT                   0
#define    MM_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_MASK                    0x00000001

#define MM_CLK_MGR_REG_VLT_PERI_OFFSET                                    0x00000030
#define MM_CLK_MGR_REG_VLT_PERI_TYPE                                      UInt32
#define MM_CLK_MGR_REG_VLT_PERI_RESERVED_MASK                             0x7FFFFF00
#define    MM_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_SHIFT                 31
#define    MM_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT                    4
#define    MM_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK                     0x000000F0
#define    MM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT                  0
#define    MM_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK                   0x0000000F

#define MM_CLK_MGR_REG_LVM_EN_OFFSET                                      0x00000034
#define MM_CLK_MGR_REG_LVM_EN_TYPE                                        UInt32
#define MM_CLK_MGR_REG_LVM_EN_RESERVED_MASK                               0x7FFFFFFE
#define    MM_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_SHIFT                   0
#define    MM_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK                    0x00000001

#define MM_CLK_MGR_REG_LVM0_3_OFFSET                                      0x00000038
#define MM_CLK_MGR_REG_LVM0_3_TYPE                                        UInt32
#define MM_CLK_MGR_REG_LVM0_3_RESERVED_MASK                               0x7FFF0000
#define    MM_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_SHIFT                       12
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_MASK                        0x0000F000
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_SHIFT                       8
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_MASK                        0x00000F00
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_SHIFT                       4
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_MASK                        0x000000F0
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_SHIFT                       0
#define    MM_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK                        0x0000000F

#define MM_CLK_MGR_REG_LVM4_7_OFFSET                                      0x0000003C
#define MM_CLK_MGR_REG_LVM4_7_TYPE                                        UInt32
#define MM_CLK_MGR_REG_LVM4_7_RESERVED_MASK                               0x7FFF0000
#define    MM_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_SHIFT                       12
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_MASK                        0x0000F000
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_SHIFT                       8
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_MASK                        0x00000F00
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_SHIFT                       4
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_MASK                        0x000000F0
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_SHIFT                       0
#define    MM_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_MASK                        0x0000000F

#define MM_CLK_MGR_REG_VLT0_3_OFFSET                                      0x00000040
#define MM_CLK_MGR_REG_VLT0_3_TYPE                                        UInt32
#define MM_CLK_MGR_REG_VLT0_3_RESERVED_MASK                               0x7FFF0000
#define    MM_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT                       12
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_MASK                        0x0000F000
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT                       8
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_MASK                        0x00000F00
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT                       4
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_MASK                        0x000000F0
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT                       0
#define    MM_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK                        0x0000000F

#define MM_CLK_MGR_REG_VLT4_7_OFFSET                                      0x00000044
#define MM_CLK_MGR_REG_VLT4_7_TYPE                                        UInt32
#define MM_CLK_MGR_REG_VLT4_7_RESERVED_MASK                               0x7FFF0000
#define    MM_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT                       12
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_MASK                        0x0000F000
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT                       8
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_MASK                        0x00000F00
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT                       4
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_MASK                        0x000000F0
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT                       0
#define    MM_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_MASK                        0x0000000F

#define MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_OFFSET                       0x00000200
#define MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_TYPE                         UInt32
#define MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_RESERVED_MASK                0x6FFEFCFD
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_PRIV_ACCESS_MODE_SHIFT    31
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_PRIV_ACCESS_MODE_MASK     0x80000000
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_VOLTAGE_LEVEL_SHIFT 28
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_VOLTAGE_LEVEL_MASK 0x10000000
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_STPRSTS_SHIFT 16
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_STPRSTS_MASK 0x00010000
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HYST_VAL_SHIFT 9
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HYST_VAL_MASK 0x00000200
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HYST_EN_SHIFT 8
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HYST_EN_MASK 0x00000100
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HW_SW_GATING_SEL_SHIFT 1
#define    MM_CLK_MGR_REG_MM_AXI_SWITCH_CLKGATE_MM_SWITCH_AXI_HW_SW_GATING_SEL_MASK 0x00000002

#define MM_CLK_MGR_REG_MM_DMA_CLKGATE_OFFSET                              0x00000210
#define MM_CLK_MGR_REG_MM_DMA_CLKGATE_TYPE                                UInt32
#define MM_CLK_MGR_REG_MM_DMA_CLKGATE_RESERVED_MASK                       0x6FFEFCFC
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_PRIV_ACCESS_MODE_SHIFT           31
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_PRIV_ACCESS_MODE_MASK            0x80000000
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_VOLTAGE_LEVEL_SHIFT       28
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_VOLTAGE_LEVEL_MASK        0x10000000
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_STPRSTS_SHIFT         16
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_STPRSTS_MASK          0x00010000
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HYST_VAL_SHIFT        9
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HYST_VAL_MASK         0x00000200
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HYST_EN_SHIFT         8
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HYST_EN_MASK          0x00000100
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HW_SW_GATING_SEL_SHIFT 1
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_HW_SW_GATING_SEL_MASK 0x00000002
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_CLK_EN_SHIFT          0
#define    MM_CLK_MGR_REG_MM_DMA_CLKGATE_MM_DMA_AXI_CLK_EN_MASK           0x00000001

#define MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_OFFSET                            0x00000228
#define MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_TYPE                              UInt32
#define MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_RESERVED_MASK                     0x7FFEFCFC
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_PRIV_ACCESS_MODE_SHIFT         31
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_PRIV_ACCESS_MODE_MASK          0x80000000
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_STPRSTS_SHIFT         16
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_STPRSTS_MASK          0x00010000
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HYST_VAL_SHIFT        9
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HYST_VAL_MASK         0x00000200
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HYST_EN_SHIFT         8
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HYST_EN_MASK          0x00000100
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HW_SW_GATING_SEL_SHIFT 1
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_HW_SW_GATING_SEL_MASK 0x00000002
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_CLK_EN_SHIFT          0
#define    MM_CLK_MGR_REG_CSI0_AXI_CLKGATE_CSI0_AXI_CLK_EN_MASK           0x00000001

#define MM_CLK_MGR_REG_CSI0_LP_CLKGATE_OFFSET                             0x00000230
#define MM_CLK_MGR_REG_CSI0_LP_CLKGATE_TYPE                               UInt32
#define MM_CLK_MGR_REG_CSI0_LP_CLKGATE_RESERVED_MASK                      0x6FFEFFFC
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_VOLTAGE_LEVEL_SHIFT        28
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_VOLTAGE_LEVEL_MASK         0x10000000
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_STPRSTS_SHIFT           16
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_STPRSTS_MASK            0x00010000
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_HW_SW_GATING_SEL_SHIFT  1
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_HW_SW_GATING_SEL_MASK   0x00000002
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_CLK_EN_SHIFT            0
#define    MM_CLK_MGR_REG_CSI0_LP_CLKGATE_CSI0_LP_CLK_EN_MASK             0x00000001

#define MM_CLK_MGR_REG_ISP_CLKGATE_OFFSET                                 0x00000258
#define MM_CLK_MGR_REG_ISP_CLKGATE_TYPE                                   UInt32
#define MM_CLK_MGR_REG_ISP_CLKGATE_RESERVED_MASK                          0x6FFEFCFC
#define    MM_CLK_MGR_REG_ISP_CLKGATE_PRIV_ACCESS_MODE_SHIFT              31
#define    MM_CLK_MGR_REG_ISP_CLKGATE_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_VOLTAGE_LEVEL_SHIFT             28
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_VOLTAGE_LEVEL_MASK              0x10000000
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_STPRSTS_SHIFT               16
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_STPRSTS_MASK                0x00010000
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HYST_VAL_SHIFT              9
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HYST_VAL_MASK               0x00000200
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HYST_EN_SHIFT               8
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HYST_EN_MASK                0x00000100
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HW_SW_GATING_SEL_SHIFT      1
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_HW_SW_GATING_SEL_MASK       0x00000002
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_CLK_EN_SHIFT                0
#define    MM_CLK_MGR_REG_ISP_CLKGATE_ISP_AXI_CLK_EN_MASK                 0x00000001

#define MM_CLK_MGR_REG_SMI_AXI_CLKGATE_OFFSET                             0x00000260
#define MM_CLK_MGR_REG_SMI_AXI_CLKGATE_TYPE                               UInt32
#define MM_CLK_MGR_REG_SMI_AXI_CLKGATE_RESERVED_MASK                      0x7FFEFCFC
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_STPRSTS_SHIFT           16
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_STPRSTS_MASK            0x00010000
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HYST_VAL_SHIFT          9
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HYST_VAL_MASK           0x00000200
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HYST_EN_SHIFT           8
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HYST_EN_MASK            0x00000100
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HW_SW_GATING_SEL_SHIFT  1
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_HW_SW_GATING_SEL_MASK   0x00000002
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_CLK_EN_SHIFT            0
#define    MM_CLK_MGR_REG_SMI_AXI_CLKGATE_SMI_AXI_CLK_EN_MASK             0x00000001

#define MM_CLK_MGR_REG_SMI_CLKGATE_OFFSET                                 0x00000264
#define MM_CLK_MGR_REG_SMI_CLKGATE_TYPE                                   UInt32
#define MM_CLK_MGR_REG_SMI_CLKGATE_RESERVED_MASK                          0x6FFEFFFC
#define    MM_CLK_MGR_REG_SMI_CLKGATE_PRIV_ACCESS_MODE_SHIFT              31
#define    MM_CLK_MGR_REG_SMI_CLKGATE_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_VOLTAGE_LEVEL_SHIFT             28
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_VOLTAGE_LEVEL_MASK              0x10000000
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_STPRSTS_SHIFT                   16
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_STPRSTS_MASK                    0x00010000
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_HW_SW_GATING_SEL_SHIFT          1
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_HW_SW_GATING_SEL_MASK           0x00000002
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_CLK_EN_SHIFT                    0
#define    MM_CLK_MGR_REG_SMI_CLKGATE_SMI_CLK_EN_MASK                     0x00000001

#define MM_CLK_MGR_REG_V3D_CLKGATE_OFFSET                                 0x00000270
#define MM_CLK_MGR_REG_V3D_CLKGATE_TYPE                                   UInt32
#define MM_CLK_MGR_REG_V3D_CLKGATE_RESERVED_MASK                          0x6FFEFCFC
#define    MM_CLK_MGR_REG_V3D_CLKGATE_PRIV_ACCESS_MODE_SHIFT              31
#define    MM_CLK_MGR_REG_V3D_CLKGATE_PRIV_ACCESS_MODE_MASK               0x80000000
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_VOLTAGE_LEVEL_SHIFT             28
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_VOLTAGE_LEVEL_MASK              0x10000000
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_STPRSTS_SHIFT               16
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_STPRSTS_MASK                0x00010000
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HYST_VAL_SHIFT              9
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HYST_VAL_MASK               0x00000200
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HYST_EN_SHIFT               8
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HYST_EN_MASK                0x00000100
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HW_SW_GATING_SEL_SHIFT      1
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_HW_SW_GATING_SEL_MASK       0x00000002
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_CLK_EN_SHIFT                0
#define    MM_CLK_MGR_REG_V3D_CLKGATE_V3D_AXI_CLK_EN_MASK                 0x00000001

#define MM_CLK_MGR_REG_MM_APB_CLKGATE_OFFSET                              0x00000400
#define MM_CLK_MGR_REG_MM_APB_CLKGATE_TYPE                                UInt32
#define MM_CLK_MGR_REG_MM_APB_CLKGATE_RESERVED_MASK                       0x6FFEFCFA
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_PRIV_ACCESS_MODE_SHIFT           31
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_PRIV_ACCESS_MODE_MASK            0x80000000
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_VOLTAGE_LEVEL_SHIFT       28
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_VOLTAGE_LEVEL_MASK        0x10000000
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_STPRSTS_SHIFT             16
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_STPRSTS_MASK              0x00010000
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HYST_VAL_SHIFT            9
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HYST_VAL_MASK             0x00000200
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HYST_EN_SHIFT             8
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HYST_EN_MASK              0x00000100
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HW_SW_GATING_SEL_SHIFT    2
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_HW_SW_GATING_SEL_MASK     0x00000004
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_CLK_EN_SHIFT              0
#define    MM_CLK_MGR_REG_MM_APB_CLKGATE_MM_APB_CLK_EN_MASK               0x00000001

#define MM_CLK_MGR_REG_SPI_APB_CLKGATE_OFFSET                             0x00000418
#define MM_CLK_MGR_REG_SPI_APB_CLKGATE_TYPE                               UInt32
#define MM_CLK_MGR_REG_SPI_APB_CLKGATE_RESERVED_MASK                      0x6FFEFCFC
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_VOLTAGE_LEVEL_SHIFT         28
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_VOLTAGE_LEVEL_MASK          0x10000000
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_STPRSTS_SHIFT           16
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_STPRSTS_MASK            0x00010000
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HYST_VAL_SHIFT          9
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HYST_VAL_MASK           0x00000200
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HYST_EN_SHIFT           8
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HYST_EN_MASK            0x00000100
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HW_SW_GATING_SEL_SHIFT  1
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_HW_SW_GATING_SEL_MASK   0x00000002
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_CLK_EN_SHIFT            0
#define    MM_CLK_MGR_REG_SPI_APB_CLKGATE_SPI_APB_CLK_EN_MASK             0x00000001

#define MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_OFFSET                           0x00000608
#define MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TYPE                             UInt32
#define MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_RESERVED_MASK                    0x7FFAFFFA
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_PRIV_ACCESS_MODE_SHIFT        31
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_PRIV_ACCESS_MODE_MASK         0x80000000
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_DIV2_STPRSTS_SHIFT  18
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_DIV2_STPRSTS_MASK   0x00040000
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_STPRSTS_SHIFT       16
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_STPRSTS_MASK        0x00010000
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_DIV2_CLK_EN_SHIFT   2
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_DIV2_CLK_EN_MASK    0x00000004
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_CLK_EN_SHIFT        0
#define    MM_CLK_MGR_REG_TESTDEBUG_CLKGATE_TESTDEBUG_CLK_EN_MASK         0x00000001

#define MM_CLK_MGR_REG_AXI_DIV_OFFSET                                     0x00000A00
#define MM_CLK_MGR_REG_AXI_DIV_TYPE                                       UInt32
#define MM_CLK_MGR_REG_AXI_DIV_RESERVED_MASK                              0x74E3E1FC
#define    MM_CLK_MGR_REG_AXI_DIV_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM_CLK_MGR_REG_AXI_DIV_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM_CLK_MGR_REG_AXI_DIV_MM_APB_DIV_OVERRIDE_SHIFT               27
#define    MM_CLK_MGR_REG_AXI_DIV_MM_APB_DIV_OVERRIDE_MASK                0x08000000
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_DIV_OVERRIDE_SHIFT        25
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_DIV_OVERRIDE_MASK         0x02000000
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_OVERRIDE_SHIFT 24
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_OVERRIDE_MASK  0x01000000
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_TRIGGER_OVERRIDE_SHIFT    20
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_TRIGGER_OVERRIDE_MASK     0x00100000
#define    MM_CLK_MGR_REG_AXI_DIV_MM_APB_DIV_SHIFT                        18
#define    MM_CLK_MGR_REG_AXI_DIV_MM_APB_DIV_MASK                         0x000C0000
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_DIV_SHIFT                 9
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_DIV_MASK                  0x00001E00
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_SHIFT          0
#define    MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_MASK           0x00000003
#define       MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000000
#define       MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_CMD_VAR_312M_CLK 0x00000001
#define       MM_CLK_MGR_REG_AXI_DIV_MM_SWITCH_AXI_PLL_SELECT_CMD_VAR_208M_CLK 0x00000002

#define MM_CLK_MGR_REG_CSI0_PHY_DIV_OFFSET                                0x00000A04
#define MM_CLK_MGR_REG_CSI0_PHY_DIV_TYPE                                  UInt32
#define MM_CLK_MGR_REG_CSI0_PHY_DIV_RESERVED_MASK                         0x7FFFF000
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_SHIFT       8
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_MASK        0x00000F00
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_CSI0_RX0_BCLKHS_N 0x00000009
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_CSI0_RX0_BCLKHS 0x00000008
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_CSI0_RX1_BCLKHS_N 0x00000007
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_CSI0_RX1_BCLKHS 0x00000006
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_TESTDEBUG_CLK 0x00000005
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_TESTDEBUG_DIV2_CLK 0x00000004
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_CAMPIX_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000003
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_SHIFT        4
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_MASK         0x000000F0
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_CSI0_RX0_BCLKHS_N 0x00000009
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_CSI0_RX0_BCLKHS 0x00000008
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_CSI0_RX1_BCLKHS_N 0x00000007
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_CSI0_RX1_BCLKHS 0x00000006
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_TESTDEBUG_CLK 0x00000005
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_TESTDEBUG_DIV2_CLK 0x00000004
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE1_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000003
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_SHIFT        0
#define    MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_MASK         0x0000000F
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_CSI0_RX0_BCLKHS_N 0x00000009
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_CSI0_RX0_BCLKHS 0x00000008
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_CSI0_RX1_BCLKHS_N 0x00000007
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_CSI0_RX1_BCLKHS 0x00000006
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_TESTDEBUG_CLK 0x00000005
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_TESTDEBUG_DIV2_CLK 0x00000004
#define       MM_CLK_MGR_REG_CSI0_PHY_DIV_CSI0_BYTE0_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000003

#define MM_CLK_MGR_REG_CSI0_DIV_OFFSET                                    0x00000A08
#define MM_CLK_MGR_REG_CSI0_DIV_TYPE                                      UInt32
#define MM_CLK_MGR_REG_CSI0_DIV_RESERVED_MASK                             0x7FFFFF8C
#define    MM_CLK_MGR_REG_CSI0_DIV_PRIV_ACCESS_MODE_SHIFT                 31
#define    MM_CLK_MGR_REG_CSI0_DIV_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_DIV_SHIFT                      4
#define    MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_DIV_MASK                       0x00000070
#define    MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_SHIFT               0
#define    MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_MASK                0x00000003
#define       MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_CMD_VAR_312M_CLK 0x00000000
#define       MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_CMD_VAR_208M_CLK 0x00000001
#define       MM_CLK_MGR_REG_CSI0_DIV_CSI0_LP_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000002

#define MM_CLK_MGR_REG_SMI_DIV_OFFSET                                     0x00000A18
#define MM_CLK_MGR_REG_SMI_DIV_TYPE                                       UInt32
#define MM_CLK_MGR_REG_SMI_DIV_RESERVED_MASK                              0x7FFFFE1C
#define    MM_CLK_MGR_REG_SMI_DIV_PRIV_ACCESS_MODE_SHIFT                  31
#define    MM_CLK_MGR_REG_SMI_DIV_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    MM_CLK_MGR_REG_SMI_DIV_SMI_DIV_SHIFT                           5
#define    MM_CLK_MGR_REG_SMI_DIV_SMI_DIV_MASK                            0x000001E0
#define    MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_SHIFT                    0
#define    MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_MASK                     0x00000003
#define       MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_CMD_VAR_312M_CLK      0x00000000
#define       MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_CMD_VAR_208M_CLK      0x00000001
#define       MM_CLK_MGR_REG_SMI_DIV_SMI_PLL_SELECT_CMD_REF_CRYSTAL_CLK   0x00000002

#define MM_CLK_MGR_REG_TESTDEBUG_DIV_OFFSET                               0x00000A28
#define MM_CLK_MGR_REG_TESTDEBUG_DIV_TYPE                                 UInt32
#define MM_CLK_MGR_REG_TESTDEBUG_DIV_RESERVED_MASK                        0x7FFFFCE0
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_PRIV_ACCESS_MODE_SHIFT            31
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_PLL_SELECT_SHIFT        8
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_PLL_SELECT_MASK         0x00000300
#define       MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_PLL_SELECT_CMD_REF_CRYSTAL_CLK 0x00000000
#define       MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_PLL_SELECT_CMD_VAR_312M_CLK 0x00000001
#define       MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_PLL_SELECT_CMD_VAR_208M_CLK 0x00000002
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_DIV2_DIV_SHIFT          4
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_DIV2_DIV_MASK           0x00000010
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_DIV_SHIFT               0
#define    MM_CLK_MGR_REG_TESTDEBUG_DIV_TESTDEBUG_DIV_MASK                0x0000000F

#define MM_CLK_MGR_REG_DIV_TRIG_OFFSET                                    0x00000AFC
#define MM_CLK_MGR_REG_DIV_TRIG_TYPE                                      UInt32
#define MM_CLK_MGR_REG_DIV_TRIG_RESERVED_MASK                             0x7FEFEEFE
#define    MM_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_SHIFT                 31
#define    MM_CLK_MGR_REG_DIV_TRIG_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MM_CLK_MGR_REG_DIV_TRIG_TESTDEBUG_TRIGGER_SHIFT                20
#define    MM_CLK_MGR_REG_DIV_TRIG_TESTDEBUG_TRIGGER_MASK                 0x00100000
#define    MM_CLK_MGR_REG_DIV_TRIG_SMI_TRIGGER_SHIFT                      12
#define    MM_CLK_MGR_REG_DIV_TRIG_SMI_TRIGGER_MASK                       0x00001000
#define    MM_CLK_MGR_REG_DIV_TRIG_CSI0_LP_TRIGGER_SHIFT                  8
#define    MM_CLK_MGR_REG_DIV_TRIG_CSI0_LP_TRIGGER_MASK                   0x00000100
#define    MM_CLK_MGR_REG_DIV_TRIG_MM_SWITCH_AXI_TRIGGER_SHIFT            0
#define    MM_CLK_MGR_REG_DIV_TRIG_MM_SWITCH_AXI_TRIGGER_MASK             0x00000001

#define MM_CLK_MGR_REG_DEBUG0_OFFSET                                      0x00000E00
#define MM_CLK_MGR_REG_DEBUG0_TYPE                                        UInt32
#define MM_CLK_MGR_REG_DEBUG0_RESERVED_MASK                               0x7FFFFFFE
#define    MM_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_DEBUG0_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_DEBUG0_MM_SWITCH_AXI_POLICY_OVERRIDE_SHIFT      0
#define    MM_CLK_MGR_REG_DEBUG0_MM_SWITCH_AXI_POLICY_OVERRIDE_MASK       0x00000001

#define MM_CLK_MGR_REG_DEBUG1_OFFSET                                      0x00000E04
#define MM_CLK_MGR_REG_DEBUG1_TYPE                                        UInt32
#define MM_CLK_MGR_REG_DEBUG1_RESERVED_MASK                               0x7E7E1F76
#define    MM_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_DEBUG1_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_DEBUG1_SPI_APB_POLICY_OVERRIDE_SHIFT            24
#define    MM_CLK_MGR_REG_DEBUG1_SPI_APB_POLICY_OVERRIDE_MASK             0x01000000
#define    MM_CLK_MGR_REG_DEBUG1_MM_APB_POLICY_OVERRIDE_SHIFT             23
#define    MM_CLK_MGR_REG_DEBUG1_MM_APB_POLICY_OVERRIDE_MASK              0x00800000
#define    MM_CLK_MGR_REG_DEBUG1_V3D_AXI_POLICY_OVERRIDE_SHIFT            16
#define    MM_CLK_MGR_REG_DEBUG1_V3D_AXI_POLICY_OVERRIDE_MASK             0x00010000
#define    MM_CLK_MGR_REG_DEBUG1_SMI_POLICY_OVERRIDE_SHIFT                15
#define    MM_CLK_MGR_REG_DEBUG1_SMI_POLICY_OVERRIDE_MASK                 0x00008000
#define    MM_CLK_MGR_REG_DEBUG1_SMI_AXI_POLICY_OVERRIDE_SHIFT            14
#define    MM_CLK_MGR_REG_DEBUG1_SMI_AXI_POLICY_OVERRIDE_MASK             0x00004000
#define    MM_CLK_MGR_REG_DEBUG1_ISP_AXI_POLICY_OVERRIDE_SHIFT            13
#define    MM_CLK_MGR_REG_DEBUG1_ISP_AXI_POLICY_OVERRIDE_MASK             0x00002000
#define    MM_CLK_MGR_REG_DEBUG1_CSI0_LP_POLICY_OVERRIDE_SHIFT            7
#define    MM_CLK_MGR_REG_DEBUG1_CSI0_LP_POLICY_OVERRIDE_MASK             0x00000080
#define    MM_CLK_MGR_REG_DEBUG1_CSI0_AXI_POLICY_OVERRIDE_SHIFT           3
#define    MM_CLK_MGR_REG_DEBUG1_CSI0_AXI_POLICY_OVERRIDE_MASK            0x00000008
#define    MM_CLK_MGR_REG_DEBUG1_MM_DMA_AXI_POLICY_OVERRIDE_SHIFT         0
#define    MM_CLK_MGR_REG_DEBUG1_MM_DMA_AXI_POLICY_OVERRIDE_MASK          0x00000001

#define MM_CLK_MGR_REG_DEBUG2_OFFSET                                      0x00000E08
#define MM_CLK_MGR_REG_DEBUG2_TYPE                                        UInt32
#define MM_CLK_MGR_REG_DEBUG2_RESERVED_MASK                               0x7FFFFFFE
#define    MM_CLK_MGR_REG_DEBUG2_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_DEBUG2_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_DEBUG2_MM_SWITCH_AXI_POLICY_OVERRIDE_VALUE_SHIFT 0
#define    MM_CLK_MGR_REG_DEBUG2_MM_SWITCH_AXI_POLICY_OVERRIDE_VALUE_MASK 0x00000001

#define MM_CLK_MGR_REG_DEBUG3_OFFSET                                      0x00000E0C
#define MM_CLK_MGR_REG_DEBUG3_TYPE                                        UInt32
#define MM_CLK_MGR_REG_DEBUG3_RESERVED_MASK                               0x7E7E1F76
#define    MM_CLK_MGR_REG_DEBUG3_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_DEBUG3_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_DEBUG3_SPI_APB_POLICY_OVERRIDE_VALUE_SHIFT      24
#define    MM_CLK_MGR_REG_DEBUG3_SPI_APB_POLICY_OVERRIDE_VALUE_MASK       0x01000000
#define    MM_CLK_MGR_REG_DEBUG3_MM_APB_POLICY_OVERRIDE_VALUE_SHIFT       23
#define    MM_CLK_MGR_REG_DEBUG3_MM_APB_POLICY_OVERRIDE_VALUE_MASK        0x00800000
#define    MM_CLK_MGR_REG_DEBUG3_V3D_AXI_POLICY_OVERRIDE_VALUE_SHIFT      16
#define    MM_CLK_MGR_REG_DEBUG3_V3D_AXI_POLICY_OVERRIDE_VALUE_MASK       0x00010000
#define    MM_CLK_MGR_REG_DEBUG3_SMI_POLICY_OVERRIDE_VALUE_SHIFT          15
#define    MM_CLK_MGR_REG_DEBUG3_SMI_POLICY_OVERRIDE_VALUE_MASK           0x00008000
#define    MM_CLK_MGR_REG_DEBUG3_SMI_AXI_POLICY_OVERRIDE_VALUE_SHIFT      14
#define    MM_CLK_MGR_REG_DEBUG3_SMI_AXI_POLICY_OVERRIDE_VALUE_MASK       0x00004000
#define    MM_CLK_MGR_REG_DEBUG3_ISP_AXI_POLICY_OVERRIDE_VALUE_SHIFT      13
#define    MM_CLK_MGR_REG_DEBUG3_ISP_AXI_POLICY_OVERRIDE_VALUE_MASK       0x00002000
#define    MM_CLK_MGR_REG_DEBUG3_CSI0_LP_POLICY_OVERRIDE_VALUE_SHIFT      7
#define    MM_CLK_MGR_REG_DEBUG3_CSI0_LP_POLICY_OVERRIDE_VALUE_MASK       0x00000080
#define    MM_CLK_MGR_REG_DEBUG3_CSI0_AXI_POLICY_OVERRIDE_VALUE_SHIFT     3
#define    MM_CLK_MGR_REG_DEBUG3_CSI0_AXI_POLICY_OVERRIDE_VALUE_MASK      0x00000008
#define    MM_CLK_MGR_REG_DEBUG3_MM_DMA_AXI_POLICY_OVERRIDE_VALUE_SHIFT   0
#define    MM_CLK_MGR_REG_DEBUG3_MM_DMA_AXI_POLICY_OVERRIDE_VALUE_MASK    0x00000001

#define MM_CLK_MGR_REG_DEBUG4_OFFSET                                      0x00000E10
#define MM_CLK_MGR_REG_DEBUG4_TYPE                                        UInt32
#define MM_CLK_MGR_REG_DEBUG4_RESERVED_MASK                               0x7FFFFFFE
#define    MM_CLK_MGR_REG_DEBUG4_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_DEBUG4_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_DEBUG4_MM_SWITCH_AXI_CLK_EN_SHIFT               0
#define    MM_CLK_MGR_REG_DEBUG4_MM_SWITCH_AXI_CLK_EN_MASK                0x00000001

#define MM_CLK_MGR_REG_SCAN_DIV_OFFSET                                    0x00000E34
#define MM_CLK_MGR_REG_SCAN_DIV_TYPE                                      UInt32
#define MM_CLK_MGR_REG_SCAN_DIV_RESERVED_MASK                             0x7FFFFEF8
#define    MM_CLK_MGR_REG_SCAN_DIV_PRIV_ACCESS_MODE_SHIFT                 31
#define    MM_CLK_MGR_REG_SCAN_DIV_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    MM_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_TRIGGER_SHIFT                8
#define    MM_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_TRIGGER_MASK                 0x00000100
#define    MM_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_DIV_SHIFT                    0
#define    MM_CLK_MGR_REG_SCAN_DIV_DFT_SCAN0_DIV_MASK                     0x00000007

#define MM_CLK_MGR_REG_ACTIVITY_MON1_OFFSET                               0x00000E40
#define MM_CLK_MGR_REG_ACTIVITY_MON1_TYPE                                 UInt32
#define MM_CLK_MGR_REG_ACTIVITY_MON1_RESERVED_MASK                        0x7F9D68E2
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_PRIV_ACCESS_MODE_SHIFT            31
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_TESTDEBUG_DIV2_ACTVSTS_SHIFT      22
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_TESTDEBUG_DIV2_ACTVSTS_MASK       0x00400000
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_TESTDEBUG_ACTVSTS_SHIFT           21
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_TESTDEBUG_ACTVSTS_MASK            0x00200000
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_CSI0_LP_ACTVSTS_SHIFT             17
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_CSI0_LP_ACTVSTS_MASK              0x00020000
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_SMI_ACTVSTS_SHIFT                 15
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_SMI_ACTVSTS_MASK                  0x00008000
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_MM_DMA_AXI_ACTVSTS_SHIFT          12
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_MM_DMA_AXI_ACTVSTS_MASK           0x00001000
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_ISP_AXI_ACTVSTS_SHIFT             10
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_ISP_AXI_ACTVSTS_MASK              0x00000400
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_V3D_AXI_ACTVSTS_SHIFT             9
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_V3D_AXI_ACTVSTS_MASK              0x00000200
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_SMI_AXI_ACTVSTS_SHIFT             8
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_SMI_AXI_ACTVSTS_MASK              0x00000100
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_CSI0_AXI_ACTVSTS_SHIFT            4
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_CSI0_AXI_ACTVSTS_MASK             0x00000010
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_MM_APB_ACTVSTS_SHIFT              3
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_MM_APB_ACTVSTS_MASK               0x00000008
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_SPI_APB_ACTVSTS_SHIFT             2
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_SPI_APB_ACTVSTS_MASK              0x00000004
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_MM_SWITCH_AXI_ACTVSTS_SHIFT       0
#define    MM_CLK_MGR_REG_ACTIVITY_MON1_MM_SWITCH_AXI_ACTVSTS_MASK        0x00000001

#define MM_CLK_MGR_REG_ACTIVITY_MON2_OFFSET                               0x00000E44
#define MM_CLK_MGR_REG_ACTIVITY_MON2_TYPE                                 UInt32
#define MM_CLK_MGR_REG_ACTIVITY_MON2_RESERVED_MASK                        0x7FFFFFC0
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_PRIV_ACCESS_MODE_SHIFT            31
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_PRIV_ACCESS_MODE_MASK             0x80000000
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN3_ACTVSTS_SHIFT           5
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN3_ACTVSTS_MASK            0x00000020
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN2_ACTVSTS_SHIFT           4
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN2_ACTVSTS_MASK            0x00000010
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN1_ACTVSTS_SHIFT           3
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN1_ACTVSTS_MASK            0x00000008
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN4_ACTVSTS_SHIFT           2
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN4_ACTVSTS_MASK            0x00000004
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN5_ACTVSTS_SHIFT           1
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN5_ACTVSTS_MASK            0x00000002
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN0_ACTVSTS_SHIFT           0
#define    MM_CLK_MGR_REG_ACTIVITY_MON2_DFT_SCAN0_ACTVSTS_MASK            0x00000001

#define MM_CLK_MGR_REG_CLKMON_OFFSET                                      0x00000E54
#define MM_CLK_MGR_REG_CLKMON_TYPE                                        UInt32
#define MM_CLK_MGR_REG_CLKMON_RESERVED_MASK                               0x7FFFFC00
#define    MM_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_SHIFT                   31
#define    MM_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    MM_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT                         8
#define    MM_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK                          0x00000300
#define    MM_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT                         0
#define    MM_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK                          0x000000FF

#define MM_CLK_MGR_REG_MM_CCU_PROF_CTL_OFFSET                             0x00000E90
#define MM_CLK_MGR_REG_MM_CCU_PROF_CTL_TYPE                               UInt32
#define MM_CLK_MGR_REG_MM_CCU_PROF_CTL_RESERVED_MASK                      0x7FFFFF8E
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CTL_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CTL_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CTL_MM_CCU_PROF_CNT_CTRL_SHIFT      4
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CTL_MM_CCU_PROF_CNT_CTRL_MASK       0x00000070
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CTL_MM_CCU_PROF_CNT_START_SHIFT     0
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CTL_MM_CCU_PROF_CNT_START_MASK      0x00000001

#define MM_CLK_MGR_REG_MM_CCU_PROF_SEL_OFFSET                             0x00000E94
#define MM_CLK_MGR_REG_MM_CCU_PROF_SEL_TYPE                               UInt32
#define MM_CLK_MGR_REG_MM_CCU_PROF_SEL_RESERVED_MASK                      0x7000F0F0
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_MM_CCU_PROF_CLK_REQ_SEL_SHIFT   16
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_MM_CCU_PROF_CLK_REQ_SEL_MASK    0x0FFF0000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_MM_CCU_PROF_POLICY_SEL_SHIFT    8
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_MM_CCU_PROF_POLICY_SEL_MASK     0x00000F00
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_MM_CCU_PROF_AUTOGATING_SEL_SHIFT 0
#define    MM_CLK_MGR_REG_MM_CCU_PROF_SEL_MM_CCU_PROF_AUTOGATING_SEL_MASK 0x0000000F

#define MM_CLK_MGR_REG_MM_CCU_PROF_CNT_OFFSET                             0x00000E98
#define MM_CLK_MGR_REG_MM_CCU_PROF_CNT_TYPE                               UInt32
#define MM_CLK_MGR_REG_MM_CCU_PROF_CNT_RESERVED_MASK                      0x00000000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CNT_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CNT_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CNT_MM_CCU_PROF_CNT_SHIFT           0
#define    MM_CLK_MGR_REG_MM_CCU_PROF_CNT_MM_CCU_PROF_CNT_MASK            0x7FFFFFFF

#define MM_CLK_MGR_REG_MM_CCU_PROF_DBG_OFFSET                             0x00000E9C
#define MM_CLK_MGR_REG_MM_CCU_PROF_DBG_TYPE                               UInt32
#define MM_CLK_MGR_REG_MM_CCU_PROF_DBG_RESERVED_MASK                      0x7FC00000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_DBG_PRIV_ACCESS_MODE_SHIFT          31
#define    MM_CLK_MGR_REG_MM_CCU_PROF_DBG_PRIV_ACCESS_MODE_MASK           0x80000000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_DBG_MM_CCU_PROF_DEBUG_BUS_SEL_SHIFT 16
#define    MM_CLK_MGR_REG_MM_CCU_PROF_DBG_MM_CCU_PROF_DEBUG_BUS_SEL_MASK  0x003F0000
#define    MM_CLK_MGR_REG_MM_CCU_PROF_DBG_MM_CCU_PROF_DEBUG_BUS_SHIFT     0
#define    MM_CLK_MGR_REG_MM_CCU_PROF_DBG_MM_CCU_PROF_DEBUG_BUS_MASK      0x0000FFFF

#define MM_CLK_MGR_REG_POLICY_DBG_OFFSET                                  0x00000EC0
#define MM_CLK_MGR_REG_POLICY_DBG_TYPE                                    UInt32
#define MM_CLK_MGR_REG_POLICY_DBG_RESERVED_MASK                           0x7FFFFC00
#define    MM_CLK_MGR_REG_POLICY_DBG_PRIV_ACCESS_MODE_SHIFT               31
#define    MM_CLK_MGR_REG_POLICY_DBG_PRIV_ACCESS_MODE_MASK                0x80000000
#define    MM_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT                       7
#define    MM_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_MASK                        0x00000380
#define    MM_CLK_MGR_REG_POLICY_DBG_TGT_FREQ_SHIFT                       4
#define    MM_CLK_MGR_REG_POLICY_DBG_TGT_FREQ_MASK                        0x00000070
#define    MM_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT                     2
#define    MM_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK                      0x0000000C
#define    MM_CLK_MGR_REG_POLICY_DBG_TGT_POLICY_SHIFT                     0
#define    MM_CLK_MGR_REG_POLICY_DBG_TGT_POLICY_MASK                      0x00000003

#define MM_CLK_MGR_REG_TGTMASK_DBG1_OFFSET                                0x00000EC4
#define MM_CLK_MGR_REG_TGTMASK_DBG1_TYPE                                  UInt32
#define MM_CLK_MGR_REG_TGTMASK_DBG1_RESERVED_MASK                         0x7FFFF470
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_MM_DMA_TARGET_MASK_SHIFT           11
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_MM_DMA_TARGET_MASK_MASK            0x00000800
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_ISP_TARGET_MASK_SHIFT              9
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_ISP_TARGET_MASK_MASK               0x00000200
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_V3D_TARGET_MASK_SHIFT              8
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_V3D_TARGET_MASK_MASK               0x00000100
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_SMI_TARGET_MASK_SHIFT              7
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_SMI_TARGET_MASK_MASK               0x00000080
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_CSI0_TARGET_MASK_SHIFT             3
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_CSI0_TARGET_MASK_MASK              0x00000008
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_MM_APB_TARGET_MASK_SHIFT           2
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_MM_APB_TARGET_MASK_MASK            0x00000004
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_SPI_TARGET_MASK_SHIFT              1
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_SPI_TARGET_MASK_MASK               0x00000002
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_MM_SWITCH_TARGET_MASK_SHIFT        0
#define    MM_CLK_MGR_REG_TGTMASK_DBG1_MM_SWITCH_TARGET_MASK_MASK         0x00000001

#define MM_CLK_MGR_REG_ACTMASK_DBG1_OFFSET                                0x00000ED4
#define MM_CLK_MGR_REG_ACTMASK_DBG1_TYPE                                  UInt32
#define MM_CLK_MGR_REG_ACTMASK_DBG1_RESERVED_MASK                         0x7FFFF470
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_PRIV_ACCESS_MODE_SHIFT             31
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_PRIV_ACCESS_MODE_MASK              0x80000000
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_MM_DMA_ACTIVE_MASK_SHIFT           11
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_MM_DMA_ACTIVE_MASK_MASK            0x00000800
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_ISP_ACTIVE_MASK_SHIFT              9
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_ISP_ACTIVE_MASK_MASK               0x00000200
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_V3D_ACTIVE_MASK_SHIFT              8
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_V3D_ACTIVE_MASK_MASK               0x00000100
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_SMI_ACTIVE_MASK_SHIFT              7
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_SMI_ACTIVE_MASK_MASK               0x00000080
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_CSI0_ACTIVE_MASK_SHIFT             3
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_CSI0_ACTIVE_MASK_MASK              0x00000008
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_MM_APB_ACTIVE_MASK_SHIFT           2
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_MM_APB_ACTIVE_MASK_MASK            0x00000004
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_SPI_ACTIVE_MASK_SHIFT              1
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_SPI_ACTIVE_MASK_MASK               0x00000002
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_MM_SWITCH_ACTIVE_MASK_SHIFT        0
#define    MM_CLK_MGR_REG_ACTMASK_DBG1_MM_SWITCH_ACTIVE_MASK_MASK         0x00000001

#endif /* __BRCM_RDB_MM_CLK_MGR_REG_H__ */



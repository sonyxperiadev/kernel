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

#ifndef __BRCM_RDB_DSP_CLK_MGR_REG_H__
#define __BRCM_RDB_DSP_CLK_MGR_REG_H__

#define DSP_CLK_MGR_REG_WR_ACCESS_OFFSET                                  0x00000000
#define DSP_CLK_MGR_REG_WR_ACCESS_TYPE                                    UInt32
#define DSP_CLK_MGR_REG_WR_ACCESS_RESERVED_MASK                           0x7FFFFFFE
#define    DSP_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_SHIFT               31
#define    DSP_CLK_MGR_REG_WR_ACCESS_PRIV_ACCESS_MODE_MASK                0x80000000
#define    DSP_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_SHIFT                     0
#define    DSP_CLK_MGR_REG_WR_ACCESS_CLKMGR_ACC_MASK                      0x00000001

#define DSP_CLK_MGR_REG_POLICY_FREQ_OFFSET                                0x00000008
#define DSP_CLK_MGR_REG_POLICY_FREQ_TYPE                                  UInt32
#define DSP_CLK_MGR_REG_POLICY_FREQ_RESERVED_MASK                         0x78F8F8F8
#define    DSP_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_SHIFT             31
#define    DSP_CLK_MGR_REG_POLICY_FREQ_PRIV_ACCESS_MODE_MASK              0x80000000
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_SHIFT                 24
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY3_FREQ_MASK                  0x07000000
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_SHIFT                 16
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY2_FREQ_MASK                  0x00070000
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_SHIFT                 8
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY1_FREQ_MASK                  0x00000700
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_SHIFT                 0
#define    DSP_CLK_MGR_REG_POLICY_FREQ_POLICY0_FREQ_MASK                  0x00000007

#define DSP_CLK_MGR_REG_POLICY_CTL_OFFSET                                 0x0000000C
#define DSP_CLK_MGR_REG_POLICY_CTL_TYPE                                   UInt32
#define DSP_CLK_MGR_REG_POLICY_CTL_RESERVED_MASK                          0x7FFFFFF0
#define    DSP_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_SHIFT              31
#define    DSP_CLK_MGR_REG_POLICY_CTL_PRIV_ACCESS_MODE_MASK               0x80000000
#define    DSP_CLK_MGR_REG_POLICY_CTL_TGT_VLD_SHIFT                       3
#define    DSP_CLK_MGR_REG_POLICY_CTL_TGT_VLD_MASK                        0x00000008
#define    DSP_CLK_MGR_REG_POLICY_CTL_GO_ATL_SHIFT                        2
#define    DSP_CLK_MGR_REG_POLICY_CTL_GO_ATL_MASK                         0x00000004
#define    DSP_CLK_MGR_REG_POLICY_CTL_GO_AC_SHIFT                         1
#define    DSP_CLK_MGR_REG_POLICY_CTL_GO_AC_MASK                          0x00000002
#define    DSP_CLK_MGR_REG_POLICY_CTL_GO_SHIFT                            0
#define    DSP_CLK_MGR_REG_POLICY_CTL_GO_MASK                             0x00000001

#define DSP_CLK_MGR_REG_INTEN_OFFSET                                      0x00000020
#define DSP_CLK_MGR_REG_INTEN_TYPE                                        UInt32
#define DSP_CLK_MGR_REG_INTEN_RESERVED_MASK                               0x7FFFFFFC
#define    DSP_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_SHIFT                   31
#define    DSP_CLK_MGR_REG_INTEN_PRIV_ACCESS_MODE_MASK                    0x80000000
#define    DSP_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_SHIFT                   1
#define    DSP_CLK_MGR_REG_INTEN_INTEN_ACT_INT_EN_MASK                    0x00000002
#define    DSP_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_SHIFT                   0
#define    DSP_CLK_MGR_REG_INTEN_INTEN_TGT_INT_EN_MASK                    0x00000001

#define DSP_CLK_MGR_REG_INTSTAT_OFFSET                                    0x00000024
#define DSP_CLK_MGR_REG_INTSTAT_TYPE                                      UInt32
#define DSP_CLK_MGR_REG_INTSTAT_RESERVED_MASK                             0x7FFFFFFC
#define    DSP_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_SHIFT                 31
#define    DSP_CLK_MGR_REG_INTSTAT_PRIV_ACCESS_MODE_MASK                  0x80000000
#define    DSP_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_SHIFT                  1
#define    DSP_CLK_MGR_REG_INTSTAT_INTSTAT_ACT_INT_MASK                   0x00000002
#define    DSP_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_SHIFT                  0
#define    DSP_CLK_MGR_REG_INTSTAT_INTSTAT_TGT_INT_MASK                   0x00000001

#define DSP_CLK_MGR_REG_VLT_PERI_OFFSET                                   0x00000030
#define DSP_CLK_MGR_REG_VLT_PERI_TYPE                                     UInt32
#define DSP_CLK_MGR_REG_VLT_PERI_RESERVED_MASK                            0x7FFFF0F0
#define    DSP_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_SHIFT                31
#define    DSP_CLK_MGR_REG_VLT_PERI_PRIV_ACCESS_MODE_MASK                 0x80000000
#define    DSP_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_SHIFT                   8
#define    DSP_CLK_MGR_REG_VLT_PERI_VLT_HIGH_PERI_MASK                    0x00000F00
#define    DSP_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_SHIFT                 0
#define    DSP_CLK_MGR_REG_VLT_PERI_VLT_NORMAL_PERI_MASK                  0x0000000F

#define DSP_CLK_MGR_REG_LVM_EN_OFFSET                                     0x00000034
#define DSP_CLK_MGR_REG_LVM_EN_TYPE                                       UInt32
#define DSP_CLK_MGR_REG_LVM_EN_RESERVED_MASK                              0x7FFFFFFE
#define    DSP_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_SHIFT                  31
#define    DSP_CLK_MGR_REG_LVM_EN_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    DSP_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_SHIFT                  0
#define    DSP_CLK_MGR_REG_LVM_EN_POLICY_CONFIG_EN_MASK                   0x00000001

#define DSP_CLK_MGR_REG_LVM0_3_OFFSET                                     0x00000038
#define DSP_CLK_MGR_REG_LVM0_3_TYPE                                       UInt32
#define DSP_CLK_MGR_REG_LVM0_3_RESERVED_MASK                              0x7FFF0000
#define    DSP_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_SHIFT                  31
#define    DSP_CLK_MGR_REG_LVM0_3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_SHIFT                      12
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_03_MASK                       0x0000F000
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_SHIFT                      8
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_02_MASK                       0x00000F00
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_SHIFT                      4
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_01_MASK                       0x000000F0
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_SHIFT                      0
#define    DSP_CLK_MGR_REG_LVM0_3_LVM0_3_MD_00_MASK                       0x0000000F

#define DSP_CLK_MGR_REG_LVM4_7_OFFSET                                     0x0000003C
#define DSP_CLK_MGR_REG_LVM4_7_TYPE                                       UInt32
#define DSP_CLK_MGR_REG_LVM4_7_RESERVED_MASK                              0x7FFF0000
#define    DSP_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_SHIFT                  31
#define    DSP_CLK_MGR_REG_LVM4_7_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_SHIFT                      12
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_07_MASK                       0x0000F000
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_SHIFT                      8
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_06_MASK                       0x00000F00
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_SHIFT                      4
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_05_MASK                       0x000000F0
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_SHIFT                      0
#define    DSP_CLK_MGR_REG_LVM4_7_LVM4_7_MD_04_MASK                       0x0000000F

#define DSP_CLK_MGR_REG_VLT0_3_OFFSET                                     0x00000040
#define DSP_CLK_MGR_REG_VLT0_3_TYPE                                       UInt32
#define DSP_CLK_MGR_REG_VLT0_3_RESERVED_MASK                              0x70F0F0F0
#define    DSP_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_SHIFT                  31
#define    DSP_CLK_MGR_REG_VLT0_3_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_SHIFT                      24
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_03_MASK                       0x0F000000
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_SHIFT                      16
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_02_MASK                       0x000F0000
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_SHIFT                      8
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_01_MASK                       0x00000F00
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_SHIFT                      0
#define    DSP_CLK_MGR_REG_VLT0_3_VLT0_3_VV_00_MASK                       0x0000000F

#define DSP_CLK_MGR_REG_VLT4_7_OFFSET                                     0x00000044
#define DSP_CLK_MGR_REG_VLT4_7_TYPE                                       UInt32
#define DSP_CLK_MGR_REG_VLT4_7_RESERVED_MASK                              0x70F0F0F0
#define    DSP_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_SHIFT                  31
#define    DSP_CLK_MGR_REG_VLT4_7_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_SHIFT                      24
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_07_MASK                       0x0F000000
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_SHIFT                      16
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_06_MASK                       0x000F0000
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_SHIFT                      8
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_05_MASK                       0x00000F00
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_SHIFT                      0
#define    DSP_CLK_MGR_REG_VLT4_7_VLT4_7_VV_04_MASK                       0x0000000F

#define DSP_CLK_MGR_REG_DSP_CLKGATE_OFFSET                                0x00000200
#define DSP_CLK_MGR_REG_DSP_CLKGATE_TYPE                                  UInt32
#define DSP_CLK_MGR_REG_DSP_CLKGATE_RESERVED_MASK                         0x7FFEFCFC
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_PRIV_ACCESS_MODE_SHIFT             31
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_PRIV_ACCESS_MODE_MASK              0x80000000
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_STPRSTS_SHIFT                  16
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_STPRSTS_MASK                   0x00010000
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_HYST_EN_SHIFT                  9
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_HYST_EN_MASK                   0x00000200
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_HYST_VAL_SHIFT                 8
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_HYST_VAL_MASK                  0x00000100
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_HW_SW_GATING_SEL_SHIFT         1
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_HW_SW_GATING_SEL_MASK          0x00000002
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_CLK_EN_SHIFT                   0
#define    DSP_CLK_MGR_REG_DSP_CLKGATE_DSP_CLK_EN_MASK                    0x00000001

#define DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_OFFSET                    0x00000380
#define DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TYPE                      UInt32
#define DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_RESERVED_MASK             0x7FFEFCFC
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_PRIV_ACCESS_MODE_SHIFT 31
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_PRIV_ACCESS_MODE_MASK  0x80000000
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_STPRSTS_SHIFT 16
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_STPRSTS_MASK 0x00010000
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_HYST_EN_SHIFT 9
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_HYST_EN_MASK 0x00000200
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_HYST_VAL_SHIFT 8
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_HYST_VAL_MASK 0x00000100
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_HW_SW_GATING_SEL_SHIFT 1
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_HW_SW_GATING_SEL_MASK 0x00000002
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_CLK_EN_SHIFT 0
#define    DSP_CLK_MGR_REG_TRANSCEIVER_26M_CLKGATE_TRANSCEIVER_26M_CLK_EN_MASK 0x00000001

#define DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_OFFSET                    0x00000384
#define DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TYPE                      UInt32
#define DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_RESERVED_MASK             0x7FFEFCFC
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_PRIV_ACCESS_MODE_SHIFT 31
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_PRIV_ACCESS_MODE_MASK  0x80000000
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_STPRSTS_SHIFT 16
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_STPRSTS_MASK 0x00010000
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_HYST_EN_SHIFT 9
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_HYST_EN_MASK 0x00000200
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_HYST_VAL_SHIFT 8
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_HYST_VAL_MASK 0x00000100
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_HW_SW_GATING_SEL_SHIFT 1
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_HW_SW_GATING_SEL_MASK 0x00000002
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_CLK_EN_SHIFT 0
#define    DSP_CLK_MGR_REG_TRANSCEIVER_52M_CLKGATE_TRANSCEIVER_52M_CLK_EN_MASK 0x00000001

#define DSP_CLK_MGR_REG_DSP_DEBUG_OFFSET                                  0x00000E00
#define DSP_CLK_MGR_REG_DSP_DEBUG_TYPE                                    UInt32
#define DSP_CLK_MGR_REG_DSP_DEBUG_RESERVED_MASK                           0x7FFCFFF8
#define    DSP_CLK_MGR_REG_DSP_DEBUG_PRIV_ACCESS_MODE_SHIFT               31
#define    DSP_CLK_MGR_REG_DSP_DEBUG_PRIV_ACCESS_MODE_MASK                0x80000000
#define    DSP_CLK_MGR_REG_DSP_DEBUG_DSP_POLICY_OVERRIDE_VALUE_SHIFT      17
#define    DSP_CLK_MGR_REG_DSP_DEBUG_DSP_POLICY_OVERRIDE_VALUE_MASK       0x00020000
#define    DSP_CLK_MGR_REG_DSP_DEBUG_DSP_POLICY_OVERRIDE_SHIFT            16
#define    DSP_CLK_MGR_REG_DSP_DEBUG_DSP_POLICY_OVERRIDE_MASK             0x00010000
#define    DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_SHIFT                 0
#define    DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_MASK                  0x00000007
#define       DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_CMD_VAR_312M_CLK   0x00000000
#define       DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_CMD_VAR_208M_CLK   0x00000001
#define       DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_CMD_VAR_156M_CLK   0x00000002
#define       DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_CMD_VAR_104M_CLK   0x00000003
#define       DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_CMD_VAR_52M_CLK    0x00000004
#define       DSP_CLK_MGR_REG_DSP_DEBUG_DSP_PLL_SELECT_CMD_REF_26M_CLK    0x00000005

#define DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_OFFSET                          0x00000E10
#define DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_TYPE                            UInt32
#define DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_RESERVED_MASK                   0x7FFFFFFE
#define    DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_PRIV_ACCESS_MODE_SHIFT       31
#define    DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_PRIV_ACCESS_MODE_MASK        0x80000000
#define    DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_DSP_TRIGGER_SHIFT            0
#define    DSP_CLK_MGR_REG_DSP_TRIGGER_DEBUG_DSP_TRIGGER_MASK             0x00000001

#define DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_OFFSET                           0x00000E18
#define DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_TYPE                             UInt32
#define DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_RESERVED_MASK                    0x7FFFFE08
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_PRIV_ACCESS_MODE_SHIFT        31
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_PRIV_ACCESS_MODE_MASK         0x80000000
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_BMDM_DSP_POLICY_LINK_DISABLE_SHIFT 8
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_BMDM_DSP_POLICY_LINK_DISABLE_MASK 0x00000100
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_ACT_VOLTAGE_SHIFT             4
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_ACT_VOLTAGE_MASK              0x000000F0
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_CURR_FREQ_SHIFT               0
#define    DSP_CLK_MGR_REG_DSP_POLICY_DEBUG_CURR_FREQ_MASK                0x00000007

#define DSP_CLK_MGR_REG_ACTIVITY_MON1_OFFSET                              0x00000E40
#define DSP_CLK_MGR_REG_ACTIVITY_MON1_TYPE                                UInt32
#define DSP_CLK_MGR_REG_ACTIVITY_MON1_RESERVED_MASK                       0x7FFFFFF8
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_PRIV_ACCESS_MODE_SHIFT           31
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_PRIV_ACCESS_MODE_MASK            0x80000000
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_TRANSCEIVER_26M_ACTVSTS_SHIFT    2
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_TRANSCEIVER_26M_ACTVSTS_MASK     0x00000004
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_TRANSCEIVER_52M_ACTVSTS_SHIFT    1
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_TRANSCEIVER_52M_ACTVSTS_MASK     0x00000002
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_DSP_ACTVSTS_SHIFT                0
#define    DSP_CLK_MGR_REG_ACTIVITY_MON1_DSP_ACTVSTS_MASK                 0x00000001

#define DSP_CLK_MGR_REG_CLKMON_OFFSET                                     0x00000E58
#define DSP_CLK_MGR_REG_CLKMON_TYPE                                       UInt32
#define DSP_CLK_MGR_REG_CLKMON_RESERVED_MASK                              0x7FFFFC00
#define    DSP_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_SHIFT                  31
#define    DSP_CLK_MGR_REG_CLKMON_PRIV_ACCESS_MODE_MASK                   0x80000000
#define    DSP_CLK_MGR_REG_CLKMON_CLKMON_CTL_SHIFT                        8
#define    DSP_CLK_MGR_REG_CLKMON_CLKMON_CTL_MASK                         0x00000300
#define    DSP_CLK_MGR_REG_CLKMON_CLKMON_SEL_SHIFT                        0
#define    DSP_CLK_MGR_REG_CLKMON_CLKMON_SEL_MASK                         0x000000FF

#define DSP_CLK_MGR_REG_POLICY_DBG_OFFSET                                 0x00000EC0
#define DSP_CLK_MGR_REG_POLICY_DBG_TYPE                                   UInt32
#define DSP_CLK_MGR_REG_POLICY_DBG_RESERVED_MASK                          0x7FFF88CC
#define    DSP_CLK_MGR_REG_POLICY_DBG_PRIV_ACCESS_MODE_SHIFT              31
#define    DSP_CLK_MGR_REG_POLICY_DBG_PRIV_ACCESS_MODE_MASK               0x80000000
#define    DSP_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_SHIFT                      12
#define    DSP_CLK_MGR_REG_POLICY_DBG_ACT_FREQ_MASK                       0x00007000
#define    DSP_CLK_MGR_REG_POLICY_DBG_TGT_FREQ_SHIFT                      8
#define    DSP_CLK_MGR_REG_POLICY_DBG_TGT_FREQ_MASK                       0x00000700
#define    DSP_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_SHIFT                    4
#define    DSP_CLK_MGR_REG_POLICY_DBG_ACT_POLICY_MASK                     0x00000030
#define    DSP_CLK_MGR_REG_POLICY_DBG_TGT_POLICY_SHIFT                    0
#define    DSP_CLK_MGR_REG_POLICY_DBG_TGT_POLICY_MASK                     0x00000003

#endif /* __BRCM_RDB_DSP_CLK_MGR_REG_H__ */



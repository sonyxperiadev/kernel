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
/*     Date     : Generated on 5/15/2011 11:41:39                                             */
/*     RDB file : /projects/SAMOA/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_PMU_MBC_H__
#define __BRCM_RDB_PMU_MBC_H__

#define PMU_MBC_MBCCTRL1_OFFSET                                           0x00000000
#define PMU_MBC_MBCCTRL1_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL1_RESERVED_MASK                                    0xFFF0F000
#define    PMU_MBC_MBCCTRL1_EN_DLY_SEL_1_0__SHIFT                         18
#define    PMU_MBC_MBCCTRL1_EN_DLY_SEL_1_0__MASK                          0x000C0000
#define    PMU_MBC_MBCCTRL1_BRK_PER_SEL_1_0__SHIFT                        16
#define    PMU_MBC_MBCCTRL1_BRK_PER_SEL_1_0__MASK                         0x00030000
#define    PMU_MBC_MBCCTRL1_BTEMPHWL_EN_SHIFT                             11
#define    PMU_MBC_MBCCTRL1_BTEMPHWL_EN_MASK                              0x00000800
#define    PMU_MBC_MBCCTRL1_BTEMPHWH_EN_SHIFT                             10
#define    PMU_MBC_MBCCTRL1_BTEMPHWH_EN_MASK                              0x00000400
#define    PMU_MBC_MBCCTRL1_BTEMPSWL_EN_SHIFT                             9
#define    PMU_MBC_MBCCTRL1_BTEMPSWL_EN_MASK                              0x00000200
#define    PMU_MBC_MBCCTRL1_BTEMPSWH_EN_SHIFT                             8
#define    PMU_MBC_MBCCTRL1_BTEMPSWH_EN_MASK                              0x00000100
#define    PMU_MBC_MBCCTRL1_CV_EXP_SEL_2_0__SHIFT                         5
#define    PMU_MBC_MBCCTRL1_CV_EXP_SEL_2_0__MASK                          0x000000E0
#define    PMU_MBC_MBCCTRL1_SW_TMR_CLR_SHIFT                              4
#define    PMU_MBC_MBCCTRL1_SW_TMR_CLR_MASK                               0x00000010
#define    PMU_MBC_MBCCTRL1_HW_TMR_EN_SHIFT                               3
#define    PMU_MBC_MBCCTRL1_HW_TMR_EN_MASK                                0x00000008
#define    PMU_MBC_MBCCTRL1_SW_TMR_EN_SHIFT                               2
#define    PMU_MBC_MBCCTRL1_SW_TMR_EN_MASK                                0x00000004
#define    PMU_MBC_MBCCTRL1_SW_EXP_SEL_SHIFT                              0
#define    PMU_MBC_MBCCTRL1_SW_EXP_SEL_MASK                               0x00000003

#define PMU_MBC_MBCCTRL2_OFFSET                                           0x00000004
#define PMU_MBC_MBCCTRL2_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL2_RESERVED_MASK                                    0x8C008CC0
#define    PMU_MBC_MBCCTRL2_PRIOB_CONFIG_SHIFT                            30
#define    PMU_MBC_MBCCTRL2_PRIOB_CONFIG_MASK                             0x40000000
#define    PMU_MBC_MBCCTRL2_TYPE2_CTRL_SHIFT                              29
#define    PMU_MBC_MBCCTRL2_TYPE2_CTRL_MASK                               0x20000000
#define    PMU_MBC_MBCCTRL2_TYPE1_CTRL_SHIFT                              28
#define    PMU_MBC_MBCCTRL2_TYPE1_CTRL_MASK                               0x10000000
#define    PMU_MBC_MBCCTRL2_CHP_TYP_5_0__SHIFT                            20
#define    PMU_MBC_MBCCTRL2_CHP_TYP_5_0__MASK                             0x03F00000
#define    PMU_MBC_MBCCTRL2_BC_DET_RETRY_EN_SHIFT                         19
#define    PMU_MBC_MBCCTRL2_BC_DET_RETRY_EN_MASK                          0x00080000
#define    PMU_MBC_MBCCTRL2_USB_DET_LDO_EN_SHIFT                          18
#define    PMU_MBC_MBCCTRL2_USB_DET_LDO_EN_MASK                           0x00040000
#define    PMU_MBC_MBCCTRL2_BCDLDO_AON_SHIFT                              17
#define    PMU_MBC_MBCCTRL2_BCDLDO_AON_MASK                               0x00020000
#define    PMU_MBC_MBCCTRL2_BC11_EN_SHIFT                                 16
#define    PMU_MBC_MBCCTRL2_BC11_EN_MASK                                  0x00010000
#define    PMU_MBC_MBCCTRL2_DSS_TYPE_SHIFT                                14
#define    PMU_MBC_MBCCTRL2_DSS_TYPE_MASK                                 0x00004000
#define    PMU_MBC_MBCCTRL2_DSS_TIME_1_0__SHIFT                           12
#define    PMU_MBC_MBCCTRL2_DSS_TIME_1_0__MASK                            0x00003000
#define    PMU_MBC_MBCCTRL2_SW_EOC_SHIFT                                  9
#define    PMU_MBC_MBCCTRL2_SW_EOC_MASK                                   0x00000200
#define    PMU_MBC_MBCCTRL2_SW_CTRL_SWITCH_SHIFT                          8
#define    PMU_MBC_MBCCTRL2_SW_CTRL_SWITCH_MASK                           0x00000100
#define    PMU_MBC_MBCCTRL2_DBP_SHIFT                                     5
#define    PMU_MBC_MBCCTRL2_DBP_MASK                                      0x00000020
#define    PMU_MBC_MBCCTRL2_SWUP_SHIFT                                    4
#define    PMU_MBC_MBCCTRL2_SWUP_MASK                                     0x00000010
#define    PMU_MBC_MBCCTRL2_MBOV_BLOCK_EN_SHIFT                           3
#define    PMU_MBC_MBCCTRL2_MBOV_BLOCK_EN_MASK                            0x00000008
#define    PMU_MBC_MBCCTRL2_ADP_PRI_SHIFT                                 2
#define    PMU_MBC_MBCCTRL2_ADP_PRI_MASK                                  0x00000004
#define    PMU_MBC_MBCCTRL2_USB_HOSTEN_SHIFT                              1
#define    PMU_MBC_MBCCTRL2_USB_HOSTEN_MASK                               0x00000002
#define    PMU_MBC_MBCCTRL2_WAC_HOSTEN_SHIFT                              0
#define    PMU_MBC_MBCCTRL2_WAC_HOSTEN_MASK                               0x00000001

#define PMU_MBC_MBCCTRL3_OFFSET                                           0x00000008
#define PMU_MBC_MBCCTRL3_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL3_RESERVED_MASK                                    0xFFFFFFE0
#define    PMU_MBC_MBCCTRL3_VFLOATMAX_LOCK_SHIFT                          4
#define    PMU_MBC_MBCCTRL3_VFLOATMAX_LOCK_MASK                           0x00000010
#define    PMU_MBC_MBCCTRL3_VFLOATMAX_3_0__SHIFT                          0
#define    PMU_MBC_MBCCTRL3_VFLOATMAX_3_0__MASK                           0x0000000F

#define PMU_MBC_MBCCTRL4_OFFSET                                           0x0000000C
#define PMU_MBC_MBCCTRL4_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL4_RESERVED_MASK                                    0xFFFFFFF0
#define    PMU_MBC_MBCCTRL4_ICCMAX_LOCK_SHIFT                             3
#define    PMU_MBC_MBCCTRL4_ICCMAX_LOCK_MASK                              0x00000008
#define    PMU_MBC_MBCCTRL4_ICCMAX_2_0__SHIFT                             0
#define    PMU_MBC_MBCCTRL4_ICCMAX_2_0__MASK                              0x00000007

#define PMU_MBC_MBCCTRL5_OFFSET                                           0x00000010
#define PMU_MBC_MBCCTRL5_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL5_RESERVED_MASK                                    0xFFFFF088
#define    PMU_MBC_MBCCTRL5_VFLOAT_3_0__SHIFT                             8
#define    PMU_MBC_MBCCTRL5_VFLOAT_3_0__MASK                              0x00000F00
#define    PMU_MBC_MBCCTRL5_USB_FC_CC_2_0__SHIFT                          4
#define    PMU_MBC_MBCCTRL5_USB_FC_CC_2_0__MASK                           0x00000070
#define    PMU_MBC_MBCCTRL5_WAC_FC_CC_2_0__SHIFT                          0
#define    PMU_MBC_MBCCTRL5_WAC_FC_CC_2_0__MASK                           0x00000007

#define PMU_MBC_MBCCTRL6_OFFSET                                           0x00000014
#define PMU_MBC_MBCCTRL6_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL6_RESERVED_MASK                                    0x00000000
#define    PMU_MBC_MBCCTRL6_MBCCTRL6_RESERVED_SHIFT                       0
#define    PMU_MBC_MBCCTRL6_MBCCTRL6_RESERVED_MASK                        0xFFFFFFFF

#define PMU_MBC_MBCCTRL7_OFFSET                                           0x00000018
#define PMU_MBC_MBCCTRL7_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL7_RESERVED_MASK                                    0xC0080088
#define    PMU_MBC_MBCCTRL7_MBC_CV_REF_ATE_9_0__SHIFT                     20
#define    PMU_MBC_MBCCTRL7_MBC_CV_REF_ATE_9_0__MASK                      0x3FF00000
#define    PMU_MBC_MBCCTRL7_MBC_CC_REF_ATE_10_0__SHIFT                    8
#define    PMU_MBC_MBCCTRL7_MBC_CC_REF_ATE_10_0__MASK                     0x0007FF00
#define    PMU_MBC_MBCCTRL7_MBC_ATE_MODE1_SHIFT                           6
#define    PMU_MBC_MBCCTRL7_MBC_ATE_MODE1_MASK                            0x00000040
#define    PMU_MBC_MBCCTRL7_USB_PRIO_B_ATE_SHIFT                          5
#define    PMU_MBC_MBCCTRL7_USB_PRIO_B_ATE_MASK                           0x00000020
#define    PMU_MBC_MBCCTRL7_WALL_PRIO_B_ATE_SHIFT                         4
#define    PMU_MBC_MBCCTRL7_WALL_PRIO_B_ATE_MASK                          0x00000010
#define    PMU_MBC_MBCCTRL7_BCDLDOEN_ATE_SHIFT                            2
#define    PMU_MBC_MBCCTRL7_BCDLDOEN_ATE_MASK                             0x00000004
#define    PMU_MBC_MBCCTRL7_MBC_EN_ATE_SHIFT                              1
#define    PMU_MBC_MBCCTRL7_MBC_EN_ATE_MASK                               0x00000002
#define    PMU_MBC_MBCCTRL7_MBC_ATE_MODE_SHIFT                            0
#define    PMU_MBC_MBCCTRL7_MBC_ATE_MODE_MASK                             0x00000001

#define PMU_MBC_MBCCTRL8_OFFSET                                           0x0000001C
#define PMU_MBC_MBCCTRL8_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL8_RESERVED_MASK                                    0xFFFFFF00
#define    PMU_MBC_MBCCTRL8_MBCTEST_3_0__SHIFT                            4
#define    PMU_MBC_MBCCTRL8_MBCTEST_3_0__MASK                             0x000000F0
#define    PMU_MBC_MBCCTRL8_CHGDET_IN_SEL_SHIFT                           3
#define    PMU_MBC_MBCCTRL8_CHGDET_IN_SEL_MASK                            0x00000008
#define    PMU_MBC_MBCCTRL8_P_PHY_MODE_2_0__SHIFT                         0
#define    PMU_MBC_MBCCTRL8_P_PHY_MODE_2_0__MASK                          0x00000007

#define PMU_MBC_MBCCTRL9_OFFSET                                           0x00000020
#define PMU_MBC_MBCCTRL9_TYPE                                             UInt32
#define PMU_MBC_MBCCTRL9_RESERVED_MASK                                    0xF800F800
#define    PMU_MBC_MBCCTRL9_MBC_CC_WALL_100_10_0__SHIFT                   16
#define    PMU_MBC_MBCCTRL9_MBC_CC_WALL_100_10_0__MASK                    0x07FF0000
#define    PMU_MBC_MBCCTRL9_MBC_CC_USB_100_10_0__SHIFT                    0
#define    PMU_MBC_MBCCTRL9_MBC_CC_USB_100_10_0__MASK                     0x000007FF

#define PMU_MBC_MBCCTRL10_OFFSET                                          0x00000024
#define PMU_MBC_MBCCTRL10_TYPE                                            UInt32
#define PMU_MBC_MBCCTRL10_RESERVED_MASK                                   0xF800F800
#define    PMU_MBC_MBCCTRL10_MBC_CC_USB_450_10_0__SHIFT                   16
#define    PMU_MBC_MBCCTRL10_MBC_CC_USB_450_10_0__MASK                    0x07FF0000
#define    PMU_MBC_MBCCTRL10_MBC_CC_350_10_0__SHIFT                       0
#define    PMU_MBC_MBCCTRL10_MBC_CC_350_10_0__MASK                        0x000007FF

#define PMU_MBC_MBCCTRL11_OFFSET                                          0x00000028
#define PMU_MBC_MBCCTRL11_TYPE                                            UInt32
#define PMU_MBC_MBCCTRL11_RESERVED_MASK                                   0xF800F800
#define    PMU_MBC_MBCCTRL11_MBC_CC_WALL_500_10_0__SHIFT                  16
#define    PMU_MBC_MBCCTRL11_MBC_CC_WALL_500_10_0__MASK                   0x07FF0000
#define    PMU_MBC_MBCCTRL11_MBC_CC_USB_500_10_0__SHIFT                   0
#define    PMU_MBC_MBCCTRL11_MBC_CC_USB_500_10_0__MASK                    0x000007FF

#define PMU_MBC_MBCCTRL12_OFFSET                                          0x0000002C
#define PMU_MBC_MBCCTRL12_TYPE                                            UInt32
#define PMU_MBC_MBCCTRL12_RESERVED_MASK                                   0xF800F800
#define    PMU_MBC_MBCCTRL12_MBC_CC_800_10_0__SHIFT                       16
#define    PMU_MBC_MBCCTRL12_MBC_CC_800_10_0__MASK                        0x07FF0000
#define    PMU_MBC_MBCCTRL12_MBC_CC_600_10_0__SHIFT                       0
#define    PMU_MBC_MBCCTRL12_MBC_CC_600_10_0__MASK                        0x000007FF

#define PMU_MBC_MBCCTRL13_OFFSET                                          0x00000030
#define PMU_MBC_MBCCTRL13_TYPE                                            UInt32
#define PMU_MBC_MBCCTRL13_RESERVED_MASK                                   0xF800F800
#define    PMU_MBC_MBCCTRL13_MBC_CC_WALL_1000_10_0__SHIFT                 16
#define    PMU_MBC_MBCCTRL13_MBC_CC_WALL_1000_10_0__MASK                  0x07FF0000
#define    PMU_MBC_MBCCTRL13_MBC_CC_1000_10_0__SHIFT                      0
#define    PMU_MBC_MBCCTRL13_MBC_CC_1000_10_0__MASK                       0x000007FF

#define PMU_MBC_MBCCTRL14_OFFSET                                          0x00000034
#define PMU_MBC_MBCCTRL14_TYPE                                            UInt32
#define PMU_MBC_MBCCTRL14_RESERVED_MASK                                   0x00000000
#define    PMU_MBC_MBCCTRL14_MBCCTRL14_RESERVED_21_0__SHIFT               11
#define    PMU_MBC_MBCCTRL14_MBCCTRL14_RESERVED_21_0__MASK                0xFFFFF800
#define    PMU_MBC_MBCCTRL14_MBCCTRL14_OTP_RESERVED_10_0__SHIFT           0
#define    PMU_MBC_MBCCTRL14_MBCCTRL14_OTP_RESERVED_10_0__MASK            0x000007FF

#define PMU_MBC_MBCCTRL15_OFFSET                                          0x00000038
#define PMU_MBC_MBCCTRL15_TYPE                                            UInt32
#define PMU_MBC_MBCCTRL15_RESERVED_MASK                                   0x00000000
#define    PMU_MBC_MBCCTRL15_MBCCTRL15_RESERVED_21_0__SHIFT               10
#define    PMU_MBC_MBCCTRL15_MBCCTRL15_RESERVED_21_0__MASK                0xFFFFFC00
#define    PMU_MBC_MBCCTRL15_MBC_FLOAT_REF_10_0__SHIFT                    0
#define    PMU_MBC_MBCCTRL15_MBC_FLOAT_REF_10_0__MASK                     0x000003FF

#endif /* __BRCM_RDB_PMU_MBC_H__ */



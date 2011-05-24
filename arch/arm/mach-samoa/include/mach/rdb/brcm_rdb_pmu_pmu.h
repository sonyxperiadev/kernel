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

#ifndef __BRCM_RDB_PMU_PMU_H__
#define __BRCM_RDB_PMU_PMU_H__

#define PMU_PMU_HOSTCTRL1_OFFSET                                          0x00000000
#define PMU_PMU_HOSTCTRL1_TYPE                                            UInt32
#define PMU_PMU_HOSTCTRL1_RESERVED_MASK                                   0x000000A0
#define    PMU_PMU_HOSTCTRL1_BYPASS_OTP_ECC_SHIFT                         31
#define    PMU_PMU_HOSTCTRL1_BYPASS_OTP_ECC_MASK                          0x80000000
#define    PMU_PMU_HOSTCTRL1_HOSTON_WO_BSI_SHIFT                          30
#define    PMU_PMU_HOSTCTRL1_HOSTON_WO_BSI_MASK                           0x40000000
#define    PMU_PMU_HOSTCTRL1_AUXON_CTRL_1_0__SHIFT                        28
#define    PMU_PMU_HOSTCTRL1_AUXON_CTRL_1_0__MASK                         0x30000000
#define    PMU_PMU_HOSTCTRL1_DVS_SHDWN_EN_SHIFT                           27
#define    PMU_PMU_HOSTCTRL1_DVS_SHDWN_EN_MASK                            0x08000000
#define    PMU_PMU_HOSTCTRL1_SR_SHDWN_EN_SHIFT                            26
#define    PMU_PMU_HOSTCTRL1_SR_SHDWN_EN_MASK                             0x04000000
#define    PMU_PMU_HOSTCTRL1_SMPL_WKUP_SHIFT                              25
#define    PMU_PMU_HOSTCTRL1_SMPL_WKUP_MASK                               0x02000000
#define    PMU_PMU_HOSTCTRL1_GBAT_WKUP_SHIFT                              24
#define    PMU_PMU_HOSTCTRL1_GBAT_WKUP_MASK                               0x01000000
#define    PMU_PMU_HOSTCTRL1_MBWV_SHDWN_SRT_DB_SHIFT                      23
#define    PMU_PMU_HOSTCTRL1_MBWV_SHDWN_SRT_DB_MASK                       0x00800000
#define    PMU_PMU_HOSTCTRL1_BATRMTEST_SHIFT                              22
#define    PMU_PMU_HOSTCTRL1_BATRMTEST_MASK                               0x00400000
#define    PMU_PMU_HOSTCTRL1_SELF_TEST_SHIFT                              21
#define    PMU_PMU_HOSTCTRL1_SELF_TEST_MASK                               0x00200000
#define    PMU_PMU_HOSTCTRL1_MBPD_F_DB_4_0__SHIFT                         16
#define    PMU_PMU_HOSTCTRL1_MBPD_F_DB_4_0__MASK                          0x001F0000
#define    PMU_PMU_HOSTCTRL1_PMU_WV_SEL_SHIFT                             15
#define    PMU_PMU_HOSTCTRL1_PMU_WV_SEL_MASK                              0x00008000
#define    PMU_PMU_HOSTCTRL1_SYS_WDT_TIME_6_0__SHIFT                      8
#define    PMU_PMU_HOSTCTRL1_SYS_WDT_TIME_6_0__MASK                       0x00007F00
#define    PMU_PMU_HOSTCTRL1_MBPD_R_DB_SHIFT                              6
#define    PMU_PMU_HOSTCTRL1_MBPD_R_DB_MASK                               0x00000040
#define    PMU_PMU_HOSTCTRL1_CLK32K_ONOFF_SHIFT                           4
#define    PMU_PMU_HOSTCTRL1_CLK32K_ONOFF_MASK                            0x00000010
#define    PMU_PMU_HOSTCTRL1_BLK_MBWV_SHDWN_SHIFT                         3
#define    PMU_PMU_HOSTCTRL1_BLK_MBWV_SHDWN_MASK                          0x00000008
#define    PMU_PMU_HOSTCTRL1_SW_SHDWN_SHIFT                               2
#define    PMU_PMU_HOSTCTRL1_SW_SHDWN_MASK                                0x00000004
#define    PMU_PMU_HOSTCTRL1_SYS_WDT_CLR_SHIFT                            1
#define    PMU_PMU_HOSTCTRL1_SYS_WDT_CLR_MASK                             0x00000002
#define    PMU_PMU_HOSTCTRL1_SYS_WDT_EN_SHIFT                             0
#define    PMU_PMU_HOSTCTRL1_SYS_WDT_EN_MASK                              0x00000001

#define PMU_PMU_WRLOCKKEY_OFFSET                                          0x00000018
#define PMU_PMU_WRLOCKKEY_TYPE                                            UInt32
#define PMU_PMU_WRLOCKKEY_RESERVED_MASK                                   0xFFFFFF00
#define    PMU_PMU_WRLOCKKEY_WRLOCKKEY_7_0__SHIFT                         0
#define    PMU_PMU_WRLOCKKEY_WRLOCKKEY_7_0__MASK                          0x000000FF

#define PMU_PMU_WRPROEN_OFFSET                                            0x0000001C
#define PMU_PMU_WRPROEN_TYPE                                              UInt32
#define PMU_PMU_WRPROEN_RESERVED_MASK                                     0xFFFFFFF0
#define    PMU_PMU_WRPROEN_VSRC_SWITCH_EN_SHIFT                           3
#define    PMU_PMU_WRPROEN_VSRC_SWITCH_EN_MASK                            0x00000008
#define    PMU_PMU_WRPROEN_LOCK_EN_SHIFT                                  2
#define    PMU_PMU_WRPROEN_LOCK_EN_MASK                                   0x00000004
#define    PMU_PMU_WRPROEN_PMU_UNLOCK_SHIFT                               1
#define    PMU_PMU_WRPROEN_PMU_UNLOCK_MASK                                0x00000002
#define    PMU_PMU_WRPROEN_WR_PRO_EN_SHIFT                                0
#define    PMU_PMU_WRPROEN_WR_PRO_EN_MASK                                 0x00000001

#define PMU_PMU_PMUGID_OFFSET                                             0x00000024
#define PMU_PMU_PMUGID_TYPE                                               UInt32
#define PMU_PMU_PMUGID_RESERVED_MASK                                      0xFFFFFF00
#define    PMU_PMU_PMUGID_PMUGID_7_0__SHIFT                               0
#define    PMU_PMU_PMUGID_PMUGID_7_0__MASK                                0x000000FF

#define PMU_PMU_PLLPMCTRL_OFFSET                                          0x00000028
#define PMU_PMU_PLLPMCTRL_TYPE                                            UInt32
#define PMU_PMU_PLLPMCTRL_RESERVED_MASK                                   0xFFFFFFF0
#define    PMU_PMU_PLLPMCTRL_PLL_PM3_SHIFT                                3
#define    PMU_PMU_PLLPMCTRL_PLL_PM3_MASK                                 0x00000008
#define    PMU_PMU_PLLPMCTRL_PLL_PM2_SHIFT                                2
#define    PMU_PMU_PLLPMCTRL_PLL_PM2_MASK                                 0x00000004
#define    PMU_PMU_PLLPMCTRL_PLL_PM1_SHIFT                                1
#define    PMU_PMU_PLLPMCTRL_PLL_PM1_MASK                                 0x00000002
#define    PMU_PMU_PLLPMCTRL_PLL_PM0_SHIFT                                0
#define    PMU_PMU_PLLPMCTRL_PLL_PM0_MASK                                 0x00000001

#define PMU_PMU_PLLCTRL_OFFSET                                            0x0000002C
#define PMU_PMU_PLLCTRL_TYPE                                              UInt32
#define PMU_PMU_PLLCTRL_RESERVED_MASK                                     0x3FFC0000
#define    PMU_PMU_PLLCTRL_PLLTESTEN_SHIFT                                31
#define    PMU_PMU_PLLCTRL_PLLTESTEN_MASK                                 0x80000000
#define    PMU_PMU_PLLCTRL_PLL_ATE_MODE_SHIFT                             30
#define    PMU_PMU_PLLCTRL_PLL_ATE_MODE_MASK                              0x40000000
#define    PMU_PMU_PLLCTRL_PLLEN_SR_SHIFT                                 17
#define    PMU_PMU_PLLCTRL_PLLEN_SR_MASK                                  0x00020000
#define    PMU_PMU_PLLCTRL_PLLEN_DIG_SHIFT                                16
#define    PMU_PMU_PLLCTRL_PLLEN_DIG_MASK                                 0x00010000
#define    PMU_PMU_PLLCTRL_PLLEN_CHG_SHIFT                                15
#define    PMU_PMU_PLLCTRL_PLLEN_CHG_MASK                                 0x00008000
#define    PMU_PMU_PLLCTRL_PLLEN_BB_SHIFT                                 14
#define    PMU_PMU_PLLCTRL_PLLEN_BB_MASK                                  0x00004000
#define    PMU_PMU_PLLCTRL_PLLEN_ADC_SHIFT                                13
#define    PMU_PMU_PLLCTRL_PLLEN_ADC_MASK                                 0x00002000
#define    PMU_PMU_PLLCTRL_PLLEN_FG_SHIFT                                 12
#define    PMU_PMU_PLLCTRL_PLLEN_FG_MASK                                  0x00001000
#define    PMU_PMU_PLLCTRL_PLLEN_SHIFT                                    11
#define    PMU_PMU_PLLCTRL_PLLEN_MASK                                     0x00000800
#define    PMU_PMU_PLLCTRL_PLLBYP_SHIFT                                   10
#define    PMU_PMU_PLLCTRL_PLLBYP_MASK                                    0x00000400
#define    PMU_PMU_PLLCTRL_PLLDIVSEL_3_0__SHIFT                           6
#define    PMU_PMU_PLLCTRL_PLLDIVSEL_3_0__MASK                            0x000003C0
#define    PMU_PMU_PLLCTRL_PLLCTRL_5_0__SHIFT                             0
#define    PMU_PMU_PLLCTRL_PLLCTRL_5_0__MASK                              0x0000003F

#define PMU_PMU_ACCESSORYCTRL_OFFSET                                      0x00000030
#define PMU_PMU_ACCESSORYCTRL_TYPE                                        UInt32
#define PMU_PMU_ACCESSORYCTRL_RESERVED_MASK                               0xFFFF0000
#define    PMU_PMU_ACCESSORYCTRL_SHORT_POK_SHTDWN_SHIFT                   15
#define    PMU_PMU_ACCESSORYCTRL_SHORT_POK_SHTDWN_MASK                    0x00008000
#define    PMU_PMU_ACCESSORYCTRL_KEY_PAD_LOCK_SHIFT                       14
#define    PMU_PMU_ACCESSORYCTRL_KEY_PAD_LOCK_MASK                        0x00004000
#define    PMU_PMU_ACCESSORYCTRL_PONCTRL_EN_SHIFT                         13
#define    PMU_PMU_ACCESSORYCTRL_PONCTRL_EN_MASK                          0x00002000
#define    PMU_PMU_ACCESSORYCTRL_PON_RELEASE_DEB_2_0__SHIFT               10
#define    PMU_PMU_ACCESSORYCTRL_PON_RELEASE_DEB_2_0__MASK                0x00001C00
#define    PMU_PMU_ACCESSORYCTRL_PON_PRESS_DEB_2_0__SHIFT                 7
#define    PMU_PMU_ACCESSORYCTRL_PON_PRESS_DEB_2_0__MASK                  0x00000380
#define    PMU_PMU_ACCESSORYCTRL_PON_VLD_TMR_4_0__SHIFT                   2
#define    PMU_PMU_ACCESSORYCTRL_PON_VLD_TMR_4_0__MASK                    0x0000007C
#define    PMU_PMU_ACCESSORYCTRL_AUXON_DEB_1_0__SHIFT                     0
#define    PMU_PMU_ACCESSORYCTRL_AUXON_DEB_1_0__MASK                      0x00000003

#endif /* __BRCM_RDB_PMU_PMU_H__ */



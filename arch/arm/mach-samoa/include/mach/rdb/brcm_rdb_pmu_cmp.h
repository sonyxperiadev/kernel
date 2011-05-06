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

#ifndef __BRCM_RDB_PMU_CMP_H__
#define __BRCM_RDB_PMU_CMP_H__

#define PMU_CMP_CMPCTRL1_OFFSET                                           0x00000000
#define PMU_CMP_CMPCTRL1_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL1_RESERVED_MASK                                    0xFFFFE080
#define    PMU_CMP_CMPCTRL1_MBWV_DB_SEL_1_0__SHIFT                        11
#define    PMU_CMP_CMPCTRL1_MBWV_DB_SEL_1_0__MASK                         0x00001800
#define    PMU_CMP_CMPCTRL1_MBUVS_2_0__SHIFT                              8
#define    PMU_CMP_CMPCTRL1_MBUVS_2_0__MASK                               0x00000700
#define    PMU_CMP_CMPCTRL1_MBWV_HYST_1_0__SHIFT                          5
#define    PMU_CMP_CMPCTRL1_MBWV_HYST_1_0__MASK                           0x00000060
#define    PMU_CMP_CMPCTRL1_MBWV_GAP_SHIFT                                4
#define    PMU_CMP_CMPCTRL1_MBWV_GAP_MASK                                 0x00000010
#define    PMU_CMP_CMPCTRL1_MBWVS_3_0__SHIFT                              0
#define    PMU_CMP_CMPCTRL1_MBWVS_3_0__MASK                               0x0000000F

#define PMU_CMP_CMPCTRL2_OFFSET                                           0x00000004
#define PMU_CMP_CMPCTRL2_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL2_RESERVED_MASK                                    0xFFF08082
#define    PMU_CMP_CMPCTRL2_USBOV_HYST_SHIFT                              19
#define    PMU_CMP_CMPCTRL2_USBOV_HYST_MASK                               0x00080000
#define    PMU_CMP_CMPCTRL2_USBOVS_SHIFT                                  18
#define    PMU_CMP_CMPCTRL2_USBOVS_MASK                                   0x00040000
#define    PMU_CMP_CMPCTRL2_WALLOV_HYST_SHIFT                             17
#define    PMU_CMP_CMPCTRL2_WALLOV_HYST_MASK                              0x00020000
#define    PMU_CMP_CMPCTRL2_WALLOVS_SHIFT                                 16
#define    PMU_CMP_CMPCTRL2_WALLOVS_MASK                                  0x00010000
#define    PMU_CMP_CMPCTRL2_UBPD_HYST_SHIFT                               14
#define    PMU_CMP_CMPCTRL2_UBPD_HYST_MASK                                0x00004000
#define    PMU_CMP_CMPCTRL2_UBPDVS_1_0__SHIFT                             12
#define    PMU_CMP_CMPCTRL2_UBPDVS_1_0__MASK                              0x00003000
#define    PMU_CMP_CMPCTRL2_CGPD_HYST_SHIFT                               11
#define    PMU_CMP_CMPCTRL2_CGPD_HYST_MASK                                0x00000800
#define    PMU_CMP_CMPCTRL2_CGPDVS_2_0__SHIFT                             8
#define    PMU_CMP_CMPCTRL2_CGPDVS_2_0__MASK                              0x00000700
#define    PMU_CMP_CMPCTRL2_ERCVS_2_0__SHIFT                              4
#define    PMU_CMP_CMPCTRL2_ERCVS_2_0__MASK                               0x00000070
#define    PMU_CMP_CMPCTRL2_FGCVS_1_0__SHIFT                              2
#define    PMU_CMP_CMPCTRL2_FGCVS_1_0__MASK                               0x0000000C
#define    PMU_CMP_CMPCTRL2_MBMC_GAP_SHIFT                                0
#define    PMU_CMP_CMPCTRL2_MBMC_GAP_MASK                                 0x00000001

#define PMU_CMP_CMPCTRL3_OFFSET                                           0x00000008
#define PMU_CMP_CMPCTRL3_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL3_RESERVED_MASK                                    0xFC00FC00
#define    PMU_CMP_CMPCTRL3_NTCHT_RISE_9_0__SHIFT                         16
#define    PMU_CMP_CMPCTRL3_NTCHT_RISE_9_0__MASK                          0x03FF0000
#define    PMU_CMP_CMPCTRL3_NTCHT_FALL_9_0__SHIFT                         0
#define    PMU_CMP_CMPCTRL3_NTCHT_FALL_9_0__MASK                          0x000003FF

#define PMU_CMP_CMPCTRL4_OFFSET                                           0x0000000C
#define PMU_CMP_CMPCTRL4_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL4_RESERVED_MASK                                    0xFC00FC00
#define    PMU_CMP_CMPCTRL4_NTCCT_RISE_9_0__SHIFT                         16
#define    PMU_CMP_CMPCTRL4_NTCCT_RISE_9_0__MASK                          0x03FF0000
#define    PMU_CMP_CMPCTRL4_NTCCT_FALL_9_0__SHIFT                         0
#define    PMU_CMP_CMPCTRL4_NTCCT_FALL_9_0__MASK                          0x000003FF

#define PMU_CMP_CMPCTRL5_OFFSET                                           0x00000010
#define PMU_CMP_CMPCTRL5_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL5_RESERVED_MASK                                    0xFF00F8E0
#define    PMU_CMP_CMPCTRL5_ICHG_USB_TRIM_3_0__SHIFT                      20
#define    PMU_CMP_CMPCTRL5_ICHG_USB_TRIM_3_0__MASK                       0x00F00000
#define    PMU_CMP_CMPCTRL5_ICHG_WALL_TRIM_3_0__SHIFT                     16
#define    PMU_CMP_CMPCTRL5_ICHG_WALL_TRIM_3_0__MASK                      0x000F0000
#define    PMU_CMP_CMPCTRL5_PTAT_PROG_2_0__SHIFT                          8
#define    PMU_CMP_CMPCTRL5_PTAT_PROG_2_0__MASK                           0x00000700
#define    PMU_CMP_CMPCTRL5_VBG_RES_TRIM_4_0__SHIFT                       0
#define    PMU_CMP_CMPCTRL5_VBG_RES_TRIM_4_0__MASK                        0x0000001F

#define PMU_CMP_CMPCTRL6_OFFSET                                           0x00000014
#define PMU_CMP_CMPCTRL6_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL6_RESERVED_MASK                                    0xFFFFF0F0
#define    PMU_CMP_CMPCTRL6_CMP_TEST_EN_SHIFT                             11
#define    PMU_CMP_CMPCTRL6_CMP_TEST_EN_MASK                              0x00000800
#define    PMU_CMP_CMPCTRL6_BAT_TYP_DET_EN_SHIFT                          10
#define    PMU_CMP_CMPCTRL6_BAT_TYP_DET_EN_MASK                           0x00000400
#define    PMU_CMP_CMPCTRL6_MBOV_HOST_EN_SHIFT                            9
#define    PMU_CMP_CMPCTRL6_MBOV_HOST_EN_MASK                             0x00000200
#define    PMU_CMP_CMPCTRL6_RT_CMP_AON_SHIFT                              8
#define    PMU_CMP_CMPCTRL6_RT_CMP_AON_MASK                               0x00000100
#define    PMU_CMP_CMPCTRL6_HOTTEMP_DET_EN_SHIFT                          3
#define    PMU_CMP_CMPCTRL6_HOTTEMP_DET_EN_MASK                           0x00000008
#define    PMU_CMP_CMPCTRL6_ACP7_P2P_VSEL_1_0__SHIFT                      1
#define    PMU_CMP_CMPCTRL6_ACP7_P2P_VSEL_1_0__MASK                       0x00000006
#define    PMU_CMP_CMPCTRL6_BAT_BDET_EN_SHIFT                             0
#define    PMU_CMP_CMPCTRL6_BAT_BDET_EN_MASK                              0x00000001

#define PMU_CMP_CMPCTRL7_OFFSET                                           0x00000018
#define PMU_CMP_CMPCTRL7_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL7_RESERVED_MASK                                    0xFFFFFF00
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM3_SHIFT                              7
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM3_MASK                               0x00000080
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM2_SHIFT                              6
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM2_MASK                               0x00000040
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM1_SHIFT                              5
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM1_MASK                               0x00000020
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM0_SHIFT                              4
#define    PMU_CMP_CMPCTRL7_NTC_EN_PM0_MASK                               0x00000010
#define    PMU_CMP_CMPCTRL7_NTC_EN_DLY_1_0__SHIFT                         2
#define    PMU_CMP_CMPCTRL7_NTC_EN_DLY_1_0__MASK                          0x0000000C
#define    PMU_CMP_CMPCTRL7_NTC_SYNC_MODE_SHIFT                           1
#define    PMU_CMP_CMPCTRL7_NTC_SYNC_MODE_MASK                            0x00000002
#define    PMU_CMP_CMPCTRL7_NTCON_SHIFT                                   0
#define    PMU_CMP_CMPCTRL7_NTCON_MASK                                    0x00000001

#define PMU_CMP_CMPCTRL8_OFFSET                                           0x0000001C
#define PMU_CMP_CMPCTRL8_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL8_RESERVED_MASK                                    0xFFE00000
#define    PMU_CMP_CMPCTRL8_BAT_TYPE3_MBUVS_2_0__SHIFT                    18
#define    PMU_CMP_CMPCTRL8_BAT_TYPE3_MBUVS_2_0__MASK                     0x001C0000
#define    PMU_CMP_CMPCTRL8_BAT_TYPE3_MBWVS_3_0__SHIFT                    14
#define    PMU_CMP_CMPCTRL8_BAT_TYPE3_MBWVS_3_0__MASK                     0x0003C000
#define    PMU_CMP_CMPCTRL8_BAT_TYPE2_MBUVS_2_0__SHIFT                    11
#define    PMU_CMP_CMPCTRL8_BAT_TYPE2_MBUVS_2_0__MASK                     0x00003800
#define    PMU_CMP_CMPCTRL8_BAT_TYPE2_MBWVS_3_0__SHIFT                    7
#define    PMU_CMP_CMPCTRL8_BAT_TYPE2_MBWVS_3_0__MASK                     0x00000780
#define    PMU_CMP_CMPCTRL8_BAT_TYPE1_MBUVS_2_0__SHIFT                    4
#define    PMU_CMP_CMPCTRL8_BAT_TYPE1_MBUVS_2_0__MASK                     0x00000070
#define    PMU_CMP_CMPCTRL8_BAT_TYPE1_MBWVS_3_0__SHIFT                    0
#define    PMU_CMP_CMPCTRL8_BAT_TYPE1_MBWVS_3_0__MASK                     0x0000000F

#define PMU_CMP_CMPCTRL9_OFFSET                                           0x00000020
#define PMU_CMP_CMPCTRL9_TYPE                                             UInt32
#define PMU_CMP_CMPCTRL9_RESERVED_MASK                                    0xFC00FC00
#define    PMU_CMP_CMPCTRL9_OT1_RISE_9_0__SHIFT                           16
#define    PMU_CMP_CMPCTRL9_OT1_RISE_9_0__MASK                            0x03FF0000
#define    PMU_CMP_CMPCTRL9_OT1_FALL_9_0__SHIFT                           0
#define    PMU_CMP_CMPCTRL9_OT1_FALL_9_0__MASK                            0x000003FF

#define PMU_CMP_CMPCTRL10_OFFSET                                          0x00000024
#define PMU_CMP_CMPCTRL10_TYPE                                            UInt32
#define PMU_CMP_CMPCTRL10_RESERVED_MASK                                   0xFC00FC00
#define    PMU_CMP_CMPCTRL10_OT2_RISE_9_0__SHIFT                          16
#define    PMU_CMP_CMPCTRL10_OT2_RISE_9_0__MASK                           0x03FF0000
#define    PMU_CMP_CMPCTRL10_OT2_FALL_9_0__SHIFT                          0
#define    PMU_CMP_CMPCTRL10_OT2_FALL_9_0__MASK                           0x000003FF

#define PMU_CMP_CMPCTRL11_OFFSET                                          0x00000028
#define PMU_CMP_CMPCTRL11_TYPE                                            UInt32
#define PMU_CMP_CMPCTRL11_RESERVED_MASK                                   0xFC00FC00
#define    PMU_CMP_CMPCTRL11_HOTTEMP_RISE_9_0__SHIFT                      16
#define    PMU_CMP_CMPCTRL11_HOTTEMP_RISE_9_0__MASK                       0x03FF0000
#define    PMU_CMP_CMPCTRL11_HOTTEMP_FALL_9_0__SHIFT                      0
#define    PMU_CMP_CMPCTRL11_HOTTEMP_FALL_9_0__MASK                       0x000003FF

#define PMU_CMP_CMPCTRL12_OFFSET                                          0x0000002C
#define PMU_CMP_CMPCTRL12_TYPE                                            UInt32
#define PMU_CMP_CMPCTRL12_RESERVED_MASK                                   0xC0000000
#define    PMU_CMP_CMPCTRL12_BOM_BAT_TYP_TH3_9_0__SHIFT                   20
#define    PMU_CMP_CMPCTRL12_BOM_BAT_TYP_TH3_9_0__MASK                    0x3FF00000
#define    PMU_CMP_CMPCTRL12_BOM_BAT_TYP_TH2_9_0__SHIFT                   10
#define    PMU_CMP_CMPCTRL12_BOM_BAT_TYP_TH2_9_0__MASK                    0x000FFC00
#define    PMU_CMP_CMPCTRL12_BOM_BAT_TYP_TH1_9_0__SHIFT                   0
#define    PMU_CMP_CMPCTRL12_BOM_BAT_TYP_TH1_9_0__MASK                    0x000003FF

#endif /* __BRCM_RDB_PMU_CMP_H__ */



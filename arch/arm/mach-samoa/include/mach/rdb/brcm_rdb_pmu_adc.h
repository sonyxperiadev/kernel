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

#ifndef __BRCM_RDB_PMU_ADC_H__
#define __BRCM_RDB_PMU_ADC_H__

#define PMU_ADC_ADCCTRL1_OFFSET                                           0x00000000
#define PMU_ADC_ADCCTRL1_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL1_RESERVED_MASK                                    0xFFFFC000
#define    PMU_ADC_ADCCTRL1_GSM_DEBOUNCE_SHIFT                            13
#define    PMU_ADC_ADCCTRL1_GSM_DEBOUNCE_MASK                             0x00002000
#define    PMU_ADC_ADCCTRL1_RTM_DELAY_4_0__SHIFT                          8
#define    PMU_ADC_ADCCTRL1_RTM_DELAY_4_0__MASK                           0x00001F00
#define    PMU_ADC_ADCCTRL1_RTM_SELECT_3_0__SHIFT                         4
#define    PMU_ADC_ADCCTRL1_RTM_SELECT_3_0__MASK                          0x000000F0
#define    PMU_ADC_ADCCTRL1_RTM_MASK_SHIFT                                3
#define    PMU_ADC_ADCCTRL1_RTM_MASK_MASK                                 0x00000008
#define    PMU_ADC_ADCCTRL1_RTM_START_SHIFT                               2
#define    PMU_ADC_ADCCTRL1_RTM_START_MASK                                0x00000004
#define    PMU_ADC_ADCCTRL1_MAX_RESET_COUNT_1_0__SHIFT                    0
#define    PMU_ADC_ADCCTRL1_MAX_RESET_COUNT_1_0__MASK                     0x00000003

#define PMU_ADC_ADCCTRL2_OFFSET                                           0x00000004
#define PMU_ADC_ADCCTRL2_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL2_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL2_VMBAT_DATA_INVALID_HKADC_SHIFT                10
#define    PMU_ADC_ADCCTRL2_VMBAT_DATA_INVALID_HKADC_MASK                 0x00000400
#define    PMU_ADC_ADCCTRL2_HKADC_DATA_VMBAT_9_0__SHIFT                   0
#define    PMU_ADC_ADCCTRL2_HKADC_DATA_VMBAT_9_0__MASK                    0x000003FF

#define PMU_ADC_ADCCTRL3_OFFSET                                           0x00000008
#define PMU_ADC_ADCCTRL3_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL3_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL3_VBBAT_DATA_INVALID_SHIFT                      10
#define    PMU_ADC_ADCCTRL3_VBBAT_DATA_INVALID_MASK                       0x00000400
#define    PMU_ADC_ADCCTRL3_HKADC_DATA_VBBAT_9_0__SHIFT                   0
#define    PMU_ADC_ADCCTRL3_HKADC_DATA_VBBAT_9_0__MASK                    0x000003FF

#define PMU_ADC_ADCCTRL4_OFFSET                                           0x0000000C
#define PMU_ADC_ADCCTRL4_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL4_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL4_VWALL_DATA_INVALID_SHIFT                      10
#define    PMU_ADC_ADCCTRL4_VWALL_DATA_INVALID_MASK                       0x00000400
#define    PMU_ADC_ADCCTRL4_HKADC_DATA_VWALL_9_0__SHIFT                   0
#define    PMU_ADC_ADCCTRL4_HKADC_DATA_VWALL_9_0__MASK                    0x000003FF

#define PMU_ADC_ADCCTRL5_OFFSET                                           0x00000010
#define PMU_ADC_ADCCTRL5_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL5_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL5_VUSB_DATA_INVALID_SHIFT                       10
#define    PMU_ADC_ADCCTRL5_VUSB_DATA_INVALID_MASK                        0x00000400
#define    PMU_ADC_ADCCTRL5_HKADC_DATA_VUSB_9_0__SHIFT                    0
#define    PMU_ADC_ADCCTRL5_HKADC_DATA_VUSB_9_0__MASK                     0x000003FF

#define PMU_ADC_ADCCTRL6_OFFSET                                           0x00000014
#define PMU_ADC_ADCCTRL6_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL6_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL6_NTC_DATA_INVALID_SHIFT                        10
#define    PMU_ADC_ADCCTRL6_NTC_DATA_INVALID_MASK                         0x00000400
#define    PMU_ADC_ADCCTRL6_HKADC_DATA_NTC_9_0__SHIFT                     0
#define    PMU_ADC_ADCCTRL6_HKADC_DATA_NTC_9_0__MASK                      0x000003FF

#define PMU_ADC_ADCCTRL7_OFFSET                                           0x00000018
#define PMU_ADC_ADCCTRL7_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL7_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL7_BSI_DATA_INVALID_SHIFT                        10
#define    PMU_ADC_ADCCTRL7_BSI_DATA_INVALID_MASK                         0x00000400
#define    PMU_ADC_ADCCTRL7_HKADC_DATA_BSI_9_0__SHIFT                     0
#define    PMU_ADC_ADCCTRL7_HKADC_DATA_BSI_9_0__MASK                      0x000003FF

#define PMU_ADC_ADCCTRL8_OFFSET                                           0x0000001C
#define PMU_ADC_ADCCTRL8_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL8_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL8_PATEMP_DATA_INVALID_SHIFT                     10
#define    PMU_ADC_ADCCTRL8_PATEMP_DATA_INVALID_MASK                      0x00000400
#define    PMU_ADC_ADCCTRL8_HKADC_DATA_PATEMP_9_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL8_HKADC_DATA_PATEMP_9_0__MASK                   0x000003FF

#define PMU_ADC_ADCCTRL9_OFFSET                                           0x00000020
#define PMU_ADC_ADCCTRL9_TYPE                                             UInt32
#define PMU_ADC_ADCCTRL9_RESERVED_MASK                                    0xFFFFF800
#define    PMU_ADC_ADCCTRL9_BOM_DATA_INVALID_SHIFT                        10
#define    PMU_ADC_ADCCTRL9_BOM_DATA_INVALID_MASK                         0x00000400
#define    PMU_ADC_ADCCTRL9_HKADC_DATA_BOM_9_0__SHIFT                     0
#define    PMU_ADC_ADCCTRL9_HKADC_DATA_BOM_9_0__MASK                      0x000003FF

#define PMU_ADC_ADCCTRL10_OFFSET                                          0x00000024
#define PMU_ADC_ADCCTRL10_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL10_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL10_VTEMP_DATA_INVALID_SHIFT                     10
#define    PMU_ADC_ADCCTRL10_VTEMP_DATA_INVALID_MASK                      0x00000400
#define    PMU_ADC_ADCCTRL10_HKADC_DATA_VTEMP_9_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL10_HKADC_DATA_VTEMP_9_0__MASK                   0x000003FF

#define PMU_ADC_ADCCTRL11_OFFSET                                          0x00000028
#define PMU_ADC_ADCCTRL11_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL11_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL11_VIWALL_DATA_INVALID_SHIFT                    10
#define    PMU_ADC_ADCCTRL11_VIWALL_DATA_INVALID_MASK                     0x00000400
#define    PMU_ADC_ADCCTRL11_HKADC_DATA_VIWALL_9_0__SHIFT                 0
#define    PMU_ADC_ADCCTRL11_HKADC_DATA_VIWALL_9_0__MASK                  0x000003FF

#define PMU_ADC_ADCCTRL12_OFFSET                                          0x0000002C
#define PMU_ADC_ADCCTRL12_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL12_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL12_VIUSB_DATA_INVALID_SHIFT                     10
#define    PMU_ADC_ADCCTRL12_VIUSB_DATA_INVALID_MASK                      0x00000400
#define    PMU_ADC_ADCCTRL12_HKADC_DATA_VIUSB_9_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL12_HKADC_DATA_VIUSB_9_0__MASK                   0x000003FF

#define PMU_ADC_ADCCTRL13_OFFSET                                          0x00000030
#define PMU_ADC_ADCCTRL13_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL13_RESERVED_MASK                                   0xFFFFE000
#define    PMU_ADC_ADCCTRL13_ADP_VWLL_DATA_INVALID_SHIFT                  12
#define    PMU_ADC_ADCCTRL13_ADP_VWLL_DATA_INVALID_MASK                   0x00001000
#define    PMU_ADC_ADCCTRL13_ADP_VWALL_11_0__SHIFT                        0
#define    PMU_ADC_ADCCTRL13_ADP_VWALL_11_0__MASK                         0x00000FFF

#define PMU_ADC_ADCCTRL14_OFFSET                                          0x00000034
#define PMU_ADC_ADCCTRL14_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL14_RESERVED_MASK                                   0xFFFFE000
#define    PMU_ADC_ADCCTRL14_ADP_VBUS_DATA_INVALID_SHIFT                  12
#define    PMU_ADC_ADCCTRL14_ADP_VBUS_DATA_INVALID_MASK                   0x00001000
#define    PMU_ADC_ADCCTRL14_ADP_VBUS_11_0__SHIFT                         0
#define    PMU_ADC_ADCCTRL14_ADP_VBUS_11_0__MASK                          0x00000FFF

#define PMU_ADC_ADCCTRL15_OFFSET                                          0x00000038
#define PMU_ADC_ADCCTRL15_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL15_RESERVED_MASK                                   0xFFFFFC00
#define    PMU_ADC_ADCCTRL15_HKADC_DATA_RTM_9_0__SHIFT                    0
#define    PMU_ADC_ADCCTRL15_HKADC_DATA_RTM_9_0__MASK                     0x000003FF

#define PMU_ADC_ADCCTRL16_OFFSET                                          0x0000003C
#define PMU_ADC_ADCCTRL16_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL16_RESERVED_MASK                                   0xFFFFF080
#define    PMU_ADC_ADCCTRL16_HKADC_OFFSET_TRIM1_3_0__SHIFT                8
#define    PMU_ADC_ADCCTRL16_HKADC_OFFSET_TRIM1_3_0__MASK                 0x00000F00
#define    PMU_ADC_ADCCTRL16_HKADC_GAIN_TRIM1_6_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL16_HKADC_GAIN_TRIM1_6_0__MASK                   0x0000007F

#define PMU_ADC_ADCCTRL17_OFFSET                                          0x00000040
#define PMU_ADC_ADCCTRL17_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL17_RESERVED_MASK                                   0xFFFFF080
#define    PMU_ADC_ADCCTRL17_HKADC_OFFSET_TRIM2_3_0__SHIFT                8
#define    PMU_ADC_ADCCTRL17_HKADC_OFFSET_TRIM2_3_0__MASK                 0x00000F00
#define    PMU_ADC_ADCCTRL17_HKADC_GAIN_TRIM2_6_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL17_HKADC_GAIN_TRIM2_6_0__MASK                   0x0000007F

#define PMU_ADC_ADCCTRL18_OFFSET                                          0x00000044
#define PMU_ADC_ADCCTRL18_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL18_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL18_AUXON_DATA_INVALID_SHIFT                     10
#define    PMU_ADC_ADCCTRL18_AUXON_DATA_INVALID_MASK                      0x00000400
#define    PMU_ADC_ADCCTRL18_HKADC_DATA_AUXON_9_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL18_HKADC_DATA_AUXON_9_0__MASK                   0x000003FF

#define PMU_ADC_ADCCTRL20_OFFSET                                          0x00000048
#define PMU_ADC_ADCCTRL20_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL20_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL20_VMBATCV_DATA_INVALID_SHIFT                   10
#define    PMU_ADC_ADCCTRL20_VMBATCV_DATA_INVALID_MASK                    0x00000400
#define    PMU_ADC_ADCCTRL20_CHGADC_DATA_VMBATCV_9_0__SHIFT               0
#define    PMU_ADC_ADCCTRL20_CHGADC_DATA_VMBATCV_9_0__MASK                0x000003FF

#define PMU_ADC_ADCCTRL21_OFFSET                                          0x0000004C
#define PMU_ADC_ADCCTRL21_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL21_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL21_VMBAT_DATA_INVALID_CHGADC_SHIFT              10
#define    PMU_ADC_ADCCTRL21_VMBAT_DATA_INVALID_CHGADC_MASK               0x00000400
#define    PMU_ADC_ADCCTRL21_CHGADC_DATA_VMBAT_9_0__SHIFT                 0
#define    PMU_ADC_ADCCTRL21_CHGADC_DATA_VMBAT_9_0__MASK                  0x000003FF

#define PMU_ADC_ADCCTRL22_OFFSET                                          0x00000050
#define PMU_ADC_ADCCTRL22_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL22_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL22_VCHG_DATA_INVALID_SHIFT                      10
#define    PMU_ADC_ADCCTRL22_VCHG_DATA_INVALID_MASK                       0x00000400
#define    PMU_ADC_ADCCTRL22_CHGADC_DATA_VCHG_9_0__SHIFT                  0
#define    PMU_ADC_ADCCTRL22_CHGADC_DATA_VCHG_9_0__MASK                   0x000003FF

#define PMU_ADC_ADCCTRL23_OFFSET                                          0x00000054
#define PMU_ADC_ADCCTRL23_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL23_RESERVED_MASK                                   0xFFFFF800
#define    PMU_ADC_ADCCTRL23_VICHG_DATA_INVALID_SHIFT                     10
#define    PMU_ADC_ADCCTRL23_VICHG_DATA_INVALID_MASK                      0x00000400
#define    PMU_ADC_ADCCTRL23_CHGADC_DATA_VICHG_9_0__SHIFT                 0
#define    PMU_ADC_ADCCTRL23_CHGADC_DATA_VICHG_9_0__MASK                  0x000003FF

#define PMU_ADC_ADCCTRL24_OFFSET                                          0x00000058
#define PMU_ADC_ADCCTRL24_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL24_RESERVED_MASK                                   0xFFFFE000
#define    PMU_ADC_ADCCTRL24_VADP_DATA_INVALID_SHIFT                      12
#define    PMU_ADC_ADCCTRL24_VADP_DATA_INVALID_MASK                       0x00001000
#define    PMU_ADC_ADCCTRL24_VADP_11_0__SHIFT                             0
#define    PMU_ADC_ADCCTRL24_VADP_11_0__MASK                              0x00000FFF

#define PMU_ADC_ADCCTRL25_OFFSET                                          0x0000005C
#define PMU_ADC_ADCCTRL25_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL25_RESERVED_MASK                                   0xFFFFF080
#define    PMU_ADC_ADCCTRL25_CHGADC_HZ_OFFSET_TRIM_3_0__SHIFT             8
#define    PMU_ADC_ADCCTRL25_CHGADC_HZ_OFFSET_TRIM_3_0__MASK              0x00000F00
#define    PMU_ADC_ADCCTRL25_CHGADC_HZ_GAIN_TRIM_6_0__SHIFT               0
#define    PMU_ADC_ADCCTRL25_CHGADC_HZ_GAIN_TRIM_6_0__MASK                0x0000007F

#define PMU_ADC_ADCCTRL26_OFFSET                                          0x00000060
#define PMU_ADC_ADCCTRL26_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL26_RESERVED_MASK                                   0xFFFFF080
#define    PMU_ADC_ADCCTRL26_CHGADC_LZ_OFFSET_TRIM_3_0__SHIFT             8
#define    PMU_ADC_ADCCTRL26_CHGADC_LZ_OFFSET_TRIM_3_0__MASK              0x00000F00
#define    PMU_ADC_ADCCTRL26_CHGADC_LZ_GAIN_TRIM_6_0__SHIFT               0
#define    PMU_ADC_ADCCTRL26_CHGADC_LZ_GAIN_TRIM_6_0__MASK                0x0000007F

#define PMU_ADC_ADCCTRL27_OFFSET                                          0x00000064
#define PMU_ADC_ADCCTRL27_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL27_RESERVED_MASK                                   0x00000000
#define    PMU_ADC_ADCCTRL27_USB_SHUNT_RES_RATIO_15_0__SHIFT              16
#define    PMU_ADC_ADCCTRL27_USB_SHUNT_RES_RATIO_15_0__MASK               0xFFFF0000
#define    PMU_ADC_ADCCTRL27_WALL_SHUNT_RES_RATIO_15_0__SHIFT             0
#define    PMU_ADC_ADCCTRL27_WALL_SHUNT_RES_RATIO_15_0__MASK              0x0000FFFF

#define PMU_ADC_ADCCTRL28_OFFSET                                          0x00000068
#define PMU_ADC_ADCCTRL28_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL28_RESERVED_MASK                                   0xFFFFF8C0
#define    PMU_ADC_ADCCTRL28_DIS_ADPMBC_SHIFT                             10
#define    PMU_ADC_ADCCTRL28_DIS_ADPMBC_MASK                              0x00000400
#define    PMU_ADC_ADCCTRL28_CHGADC_SHUNT_THRES_SEL_1_0__SHIFT            8
#define    PMU_ADC_ADCCTRL28_CHGADC_SHUNT_THRES_SEL_1_0__MASK             0x00000300
#define    PMU_ADC_ADCCTRL28_CHG_TRIM_5_0__SHIFT                          0
#define    PMU_ADC_ADCCTRL28_CHG_TRIM_5_0__MASK                           0x0000003F

#define PMU_ADC_ADCCTRL29_OFFSET                                          0x0000006C
#define PMU_ADC_ADCCTRL29_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL29_RESERVED_MASK                                   0xFFFFC0C0
#define    PMU_ADC_ADCCTRL29_ADPMBC_HYS_HI_5_0__SHIFT                     8
#define    PMU_ADC_ADCCTRL29_ADPMBC_HYS_HI_5_0__MASK                      0x00003F00
#define    PMU_ADC_ADCCTRL29_ADPMBC_HYS_LO_5_0__SHIFT                     0
#define    PMU_ADC_ADCCTRL29_ADPMBC_HYS_LO_5_0__MASK                      0x0000003F

#define PMU_ADC_ADCCTRL30_OFFSET                                          0x00000070
#define PMU_ADC_ADCCTRL30_TYPE                                            UInt32
#define PMU_ADC_ADCCTRL30_RESERVED_MASK                                   0xFE00FC00
#define    PMU_ADC_ADCCTRL30_ADPMBC_BLK_TM_8_0__SHIFT                     16
#define    PMU_ADC_ADCCTRL30_ADPMBC_BLK_TM_8_0__MASK                      0x01FF0000
#define    PMU_ADC_ADCCTRL30_ADPMBC_DEB_TM_9_0__SHIFT                     0
#define    PMU_ADC_ADCCTRL30_ADPMBC_DEB_TM_9_0__MASK                      0x000003FF

#endif /* __BRCM_RDB_PMU_ADC_H__ */



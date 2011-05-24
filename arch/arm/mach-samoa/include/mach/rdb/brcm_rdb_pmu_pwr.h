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

#ifndef __BRCM_RDB_PMU_PWR_H__
#define __BRCM_RDB_PMU_PWR_H__

#define PMU_PWR_PWR_GRP_DLY_OFFSET                                        0x00000000
#define PMU_PWR_PWR_GRP_DLY_TYPE                                          UInt32
#define PMU_PWR_PWR_GRP_DLY_RESERVED_MASK                                 0xFFFFDFF8
#define    PMU_PWR_PWR_GRP_DLY_SIM2_FUNC_DIS_SHIFT                        13
#define    PMU_PWR_PWR_GRP_DLY_SIM2_FUNC_DIS_MASK                         0x00002000
#define    PMU_PWR_PWR_GRP_DLY_LDO_SHDWN_EN_SHIFT                         2
#define    PMU_PWR_PWR_GRP_DLY_LDO_SHDWN_EN_MASK                          0x00000004
#define    PMU_PWR_PWR_GRP_DLY_PWR_GRP_DLY_1_0__SHIFT                     0
#define    PMU_PWR_PWR_GRP_DLY_PWR_GRP_DLY_1_0__MASK                      0x00000003

#define PMU_PWR_RFLDOPMCTRL_OFFSET                                        0x00000004
#define PMU_PWR_RFLDOPMCTRL_TYPE                                          UInt32
#define PMU_PWR_RFLDOPMCTRL_RESERVED_MASK                                 0x003FC000
#define    PMU_PWR_RFLDOPMCTRL_LDO_RF_CNTL_9_0__SHIFT                     22
#define    PMU_PWR_RFLDOPMCTRL_LDO_RF_CNTL_9_0__MASK                      0xFFC00000
#define    PMU_PWR_RFLDOPMCTRL_LDO_RF_VSEL_2_0__SHIFT                     11
#define    PMU_PWR_RFLDOPMCTRL_LDO_RF_VSEL_2_0__MASK                      0x00003800
#define    PMU_PWR_RFLDOPMCTRL_RFGRP_2_0__SHIFT                           8
#define    PMU_PWR_RFLDOPMCTRL_RFGRP_2_0__MASK                            0x00000700
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM3_1_0__SHIFT                       6
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM3_1_0__MASK                        0x000000C0
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM2_1_0__SHIFT                       4
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM2_1_0__MASK                        0x00000030
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM1_1_0__SHIFT                       2
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM1_1_0__MASK                        0x0000000C
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM0_1_0__SHIFT                       0
#define    PMU_PWR_RFLDOPMCTRL_RFLDO_PM0_1_0__MASK                        0x00000003

#define PMU_PWR_CAMLDOPMCTRL_OFFSET                                       0x00000008
#define PMU_PWR_CAMLDOPMCTRL_TYPE                                         UInt32
#define PMU_PWR_CAMLDOPMCTRL_RESERVED_MASK                                0x007F8000
#define    PMU_PWR_CAMLDOPMCTRL_LDO_CAM_CNTL_8_0__SHIFT                   23
#define    PMU_PWR_CAMLDOPMCTRL_LDO_CAM_CNTL_8_0__MASK                    0xFF800000
#define    PMU_PWR_CAMLDOPMCTRL_LDO_CAM_VSEL_3_0__SHIFT                   11
#define    PMU_PWR_CAMLDOPMCTRL_LDO_CAM_VSEL_3_0__MASK                    0x00007800
#define    PMU_PWR_CAMLDOPMCTRL_CAMGRP_2_0__SHIFT                         8
#define    PMU_PWR_CAMLDOPMCTRL_CAMGRP_2_0__MASK                          0x00000700
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM3_1_0__SHIFT                     6
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM3_1_0__MASK                      0x000000C0
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM2_1_0__SHIFT                     4
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM2_1_0__MASK                      0x00000030
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM1_1_0__SHIFT                     2
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM1_1_0__MASK                      0x0000000C
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM0_1_0__SHIFT                     0
#define    PMU_PWR_CAMLDOPMCTRL_CAMLDO_PM0_1_0__MASK                      0x00000003

#define PMU_PWR_HVLDO1PMCTRL_OFFSET                                       0x0000000C
#define PMU_PWR_HVLDO1PMCTRL_TYPE                                         UInt32
#define PMU_PWR_HVLDO1PMCTRL_RESERVED_MASK                                0x007FC000
#define    PMU_PWR_HVLDO1PMCTRL_LDO_HV1_CNTL_8_0__SHIFT                   23
#define    PMU_PWR_HVLDO1PMCTRL_LDO_HV1_CNTL_8_0__MASK                    0xFF800000
#define    PMU_PWR_HVLDO1PMCTRL_LDO_HV1_VSEL_2_0__SHIFT                   11
#define    PMU_PWR_HVLDO1PMCTRL_LDO_HV1_VSEL_2_0__MASK                    0x00003800
#define    PMU_PWR_HVLDO1PMCTRL_HV1GRP_2_0__SHIFT                         8
#define    PMU_PWR_HVLDO1PMCTRL_HV1GRP_2_0__MASK                          0x00000700
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM3_1_0__SHIFT                     6
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM3_1_0__MASK                      0x000000C0
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM2_1_0__SHIFT                     4
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM2_1_0__MASK                      0x00000030
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM1_1_0__SHIFT                     2
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM1_1_0__MASK                      0x0000000C
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM0_1_0__SHIFT                     0
#define    PMU_PWR_HVLDO1PMCTRL_HVLDO1_PM0_1_0__MASK                      0x00000003

#define PMU_PWR_HVLDO2PMCTRL_OFFSET                                       0x00000010
#define PMU_PWR_HVLDO2PMCTRL_TYPE                                         UInt32
#define PMU_PWR_HVLDO2PMCTRL_RESERVED_MASK                                0x007FC000
#define    PMU_PWR_HVLDO2PMCTRL_LDO_HV2_CNTL_8_0__SHIFT                   23
#define    PMU_PWR_HVLDO2PMCTRL_LDO_HV2_CNTL_8_0__MASK                    0xFF800000
#define    PMU_PWR_HVLDO2PMCTRL_LDO_HV2_VSEL_2_0__SHIFT                   11
#define    PMU_PWR_HVLDO2PMCTRL_LDO_HV2_VSEL_2_0__MASK                    0x00003800
#define    PMU_PWR_HVLDO2PMCTRL_HV2GRP_2_0__SHIFT                         8
#define    PMU_PWR_HVLDO2PMCTRL_HV2GRP_2_0__MASK                          0x00000700
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM3_1_0__SHIFT                     6
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM3_1_0__MASK                      0x000000C0
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM2_1_0__SHIFT                     4
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM2_1_0__MASK                      0x00000030
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM1_1_0__SHIFT                     2
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM1_1_0__MASK                      0x0000000C
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM0_1_0__SHIFT                     0
#define    PMU_PWR_HVLDO2PMCTRL_HVLDO2_PM0_1_0__MASK                      0x00000003

#define PMU_PWR_SIMLDO1PMCTRL_OFFSET                                      0x00000014
#define PMU_PWR_SIMLDO1PMCTRL_TYPE                                        UInt32
#define PMU_PWR_SIMLDO1PMCTRL_RESERVED_MASK                               0x007F8000
#define    PMU_PWR_SIMLDO1PMCTRL_LDO_SIM1_CNTL_8_0__SHIFT                 23
#define    PMU_PWR_SIMLDO1PMCTRL_LDO_SIM1_CNTL_8_0__MASK                  0xFF800000
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO0OFF_EN_SHIFT                      14
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO0OFF_EN_MASK                       0x00004000
#define    PMU_PWR_SIMLDO1PMCTRL_LDO_SIM1_VSEL_2_0__SHIFT                 11
#define    PMU_PWR_SIMLDO1PMCTRL_LDO_SIM1_VSEL_2_0__MASK                  0x00003800
#define    PMU_PWR_SIMLDO1PMCTRL_SIM1GRP_2_0__SHIFT                       8
#define    PMU_PWR_SIMLDO1PMCTRL_SIM1GRP_2_0__MASK                        0x00000700
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM3_1_0__SHIFT                   6
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM3_1_0__MASK                    0x000000C0
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM2_1_0__SHIFT                   4
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM2_1_0__MASK                    0x00000030
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM1_1_0__SHIFT                   2
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM1_1_0__MASK                    0x0000000C
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM0_1_0__SHIFT                   0
#define    PMU_PWR_SIMLDO1PMCTRL_SIMLDO1_PM0_1_0__MASK                    0x00000003

#define PMU_PWR_SIMLDO2PMCTRL_OFFSET                                      0x00000018
#define PMU_PWR_SIMLDO2PMCTRL_TYPE                                        UInt32
#define PMU_PWR_SIMLDO2PMCTRL_RESERVED_MASK                               0x007F8000
#define    PMU_PWR_SIMLDO2PMCTRL_LDO_SIM2_CNTL_8_0__SHIFT                 23
#define    PMU_PWR_SIMLDO2PMCTRL_LDO_SIM2_CNTL_8_0__MASK                  0xFF800000
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO1OFF_EN_SHIFT                      14
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO1OFF_EN_MASK                       0x00004000
#define    PMU_PWR_SIMLDO2PMCTRL_LDO_SIM2_VSEL_2_0__SHIFT                 11
#define    PMU_PWR_SIMLDO2PMCTRL_LDO_SIM2_VSEL_2_0__MASK                  0x00003800
#define    PMU_PWR_SIMLDO2PMCTRL_SIM2GRP_2_0__SHIFT                       8
#define    PMU_PWR_SIMLDO2PMCTRL_SIM2GRP_2_0__MASK                        0x00000700
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM3_1_0__SHIFT                   6
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM3_1_0__MASK                    0x000000C0
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM2_1_0__SHIFT                   4
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM2_1_0__MASK                    0x00000030
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM1_1_0__SHIFT                   2
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM1_1_0__MASK                    0x0000000C
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM0_1_0__SHIFT                   0
#define    PMU_PWR_SIMLDO2PMCTRL_SIMLDO2_PM0_1_0__MASK                    0x00000003

#define PMU_PWR_USBLDOPMCTRL_OFFSET                                       0x0000001C
#define PMU_PWR_USBLDOPMCTRL_TYPE                                         UInt32
#define PMU_PWR_USBLDOPMCTRL_RESERVED_MASK                                0x00000000
#define    PMU_PWR_USBLDOPMCTRL_LDO_USB_CNTL_11_0__SHIFT                  20
#define    PMU_PWR_USBLDOPMCTRL_LDO_USB_CNTL_11_0__MASK                   0xFFF00000
#define    PMU_PWR_USBLDOPMCTRL_LDO_BCD_CNTL_2_0__SHIFT                   17
#define    PMU_PWR_USBLDOPMCTRL_LDO_BCD_CNTL_2_0__MASK                    0x000E0000
#define    PMU_PWR_USBLDOPMCTRL_LDO_BCD_VSEL_2_0__SHIFT                   14
#define    PMU_PWR_USBLDOPMCTRL_LDO_BCD_VSEL_2_0__MASK                    0x0001C000
#define    PMU_PWR_USBLDOPMCTRL_LDO_USB_VSEL_2_0__SHIFT                   11
#define    PMU_PWR_USBLDOPMCTRL_LDO_USB_VSEL_2_0__MASK                    0x00003800
#define    PMU_PWR_USBLDOPMCTRL_USBLDOGRP_2_0__SHIFT                      8
#define    PMU_PWR_USBLDOPMCTRL_USBLDOGRP_2_0__MASK                       0x00000700
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM3_1_0__SHIFT                     6
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM3_1_0__MASK                      0x000000C0
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM2_1_0__SHIFT                     4
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM2_1_0__MASK                      0x00000030
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM1_1_0__SHIFT                     2
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM1_1_0__MASK                      0x0000000C
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM0_1_0__SHIFT                     0
#define    PMU_PWR_USBLDOPMCTRL_USBLDO_PM0_1_0__MASK                      0x00000003

#define PMU_PWR_DVSLDO1PMCTRL_OFFSET                                      0x00000020
#define PMU_PWR_DVSLDO1PMCTRL_TYPE                                        UInt32
#define PMU_PWR_DVSLDO1PMCTRL_RESERVED_MASK                               0xFFFC0000
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO_OVRI_DEB_2_0__SHIFT               15
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO_OVRI_DEB_2_0__MASK                0x00038000
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_PULLDN_SHIFT                    14
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_PULLDN_MASK                     0x00004000
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_PDILIM_SHIFT                    13
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_PDILIM_MASK                     0x00002000
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_MINDROP_SHIFT                   12
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_MINDROP_MASK                    0x00001000
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_ILIMTST_SHIFT                   11
#define    PMU_PWR_DVSLDO1PMCTRL_LDO_DVS1_ILIMTST_MASK                    0x00000800
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1GRP_2_0__SHIFT                    8
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1GRP_2_0__MASK                     0x00000700
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM3_1_0__SHIFT                   6
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM3_1_0__MASK                    0x000000C0
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM2_1_0__SHIFT                   4
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM2_1_0__MASK                    0x00000030
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM1_1_0__SHIFT                   2
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM1_1_0__MASK                    0x0000000C
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM0_1_0__SHIFT                   0
#define    PMU_PWR_DVSLDO1PMCTRL_DVSLDO1_PM0_1_0__MASK                    0x00000003

#define PMU_PWR_DVSLDO2PMCTRL_OFFSET                                      0x00000024
#define PMU_PWR_DVSLDO2PMCTRL_TYPE                                        UInt32
#define PMU_PWR_DVSLDO2PMCTRL_RESERVED_MASK                               0xFFFE0000
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PD_SHIFT                         16
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PD_MASK                          0x00010000
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO1_PD_SHIFT                         15
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO1_PD_MASK                          0x00008000
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_PULLDN_SHIFT                    14
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_PULLDN_MASK                     0x00004000
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_PDILIM_SHIFT                    13
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_PDILIM_MASK                     0x00002000
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_MINDROP_SHIFT                   12
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_MINDROP_MASK                    0x00001000
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_ILIMTST_SHIFT                   11
#define    PMU_PWR_DVSLDO2PMCTRL_LDO_DVS2_ILIMTST_MASK                    0x00000800
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2GRP_2_0__SHIFT                    8
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2GRP_2_0__MASK                     0x00000700
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM3_1_0__SHIFT                   6
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM3_1_0__MASK                    0x000000C0
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM2_1_0__SHIFT                   4
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM2_1_0__MASK                    0x00000030
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM1_1_0__SHIFT                   2
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM1_1_0__MASK                    0x0000000C
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM0_1_0__SHIFT                   0
#define    PMU_PWR_DVSLDO2PMCTRL_DVSLDO2_PM0_1_0__MASK                    0x00000003

#define PMU_PWR_DVSLDO1VSEL1_OFFSET                                       0x00000028
#define PMU_PWR_DVSLDO1VSEL1_TYPE                                         UInt32
#define PMU_PWR_DVSLDO1VSEL1_RESERVED_MASK                                0xFFFFFFE0
#define    PMU_PWR_DVSLDO1VSEL1_DVSLDO1VSEL_NM1_4_0__SHIFT                0
#define    PMU_PWR_DVSLDO1VSEL1_DVSLDO1VSEL_NM1_4_0__MASK                 0x0000001F

#define PMU_PWR_DVSLDO1VSEL2_OFFSET                                       0x0000002C
#define PMU_PWR_DVSLDO1VSEL2_TYPE                                         UInt32
#define PMU_PWR_DVSLDO1VSEL2_RESERVED_MASK                                0xFFFFFFE0
#define    PMU_PWR_DVSLDO1VSEL2_DVSLDO1VSEL_LPM_4_0__SHIFT                0
#define    PMU_PWR_DVSLDO1VSEL2_DVSLDO1VSEL_LPM_4_0__MASK                 0x0000001F

#define PMU_PWR_DVSLDO1VSEL3_OFFSET                                       0x00000030
#define PMU_PWR_DVSLDO1VSEL3_TYPE                                         UInt32
#define PMU_PWR_DVSLDO1VSEL3_RESERVED_MASK                                0xFFFFFFE0
#define    PMU_PWR_DVSLDO1VSEL3_DVSLDO1VSEL_NM2_4_0__SHIFT                0
#define    PMU_PWR_DVSLDO1VSEL3_DVSLDO1VSEL_NM2_4_0__MASK                 0x0000001F

#define PMU_PWR_DVSLDO2VSEL1_OFFSET                                       0x00000034
#define PMU_PWR_DVSLDO2VSEL1_TYPE                                         UInt32
#define PMU_PWR_DVSLDO2VSEL1_RESERVED_MASK                                0xFFFFFFE0
#define    PMU_PWR_DVSLDO2VSEL1_DVSLDO2_VSEL_NM1_4_0__SHIFT               0
#define    PMU_PWR_DVSLDO2VSEL1_DVSLDO2_VSEL_NM1_4_0__MASK                0x0000001F

#define PMU_PWR_DVSLDO2VSEL2_OFFSET                                       0x00000038
#define PMU_PWR_DVSLDO2VSEL2_TYPE                                         UInt32
#define PMU_PWR_DVSLDO2VSEL2_RESERVED_MASK                                0xFFFFFFE0
#define    PMU_PWR_DVSLDO2VSEL2_DVSLDO2_VSEL_LPM_4_0__SHIFT               0
#define    PMU_PWR_DVSLDO2VSEL2_DVSLDO2_VSEL_LPM_4_0__MASK                0x0000001F

#define PMU_PWR_DVSLDO2VSEL3_OFFSET                                       0x0000003C
#define PMU_PWR_DVSLDO2VSEL3_TYPE                                         UInt32
#define PMU_PWR_DVSLDO2VSEL3_RESERVED_MASK                                0xFFFFFFE0
#define    PMU_PWR_DVSLDO2VSEL3_DVSLDO2_VSEL_NM2_4_0__SHIFT               0
#define    PMU_PWR_DVSLDO2VSEL3_DVSLDO2_VSEL_NM2_4_0__MASK                0x0000001F

#define PMU_PWR_PWRTESTCTRL_OFFSET                                        0x00000040
#define PMU_PWR_PWRTESTCTRL_TYPE                                          UInt32
#define PMU_PWR_PWRTESTCTRL_RESERVED_MASK                                 0xFFFF0000
#define    PMU_PWR_PWRTESTCTRL_GRP2_BIAS_ATE_EN_SHIFT                     15
#define    PMU_PWR_PWRTESTCTRL_GRP2_BIAS_ATE_EN_MASK                      0x00008000
#define    PMU_PWR_PWRTESTCTRL_GRP1_BIAS_ATE_EN_SHIFT                     14
#define    PMU_PWR_PWRTESTCTRL_GRP1_BIAS_ATE_EN_MASK                      0x00004000
#define    PMU_PWR_PWRTESTCTRL_PC_RF_APB_SHIFT                            13
#define    PMU_PWR_PWRTESTCTRL_PC_RF_APB_MASK                             0x00002000
#define    PMU_PWR_PWRTESTCTRL_PC_APB_1_0__SHIFT                          11
#define    PMU_PWR_PWRTESTCTRL_PC_APB_1_0__MASK                           0x00001800
#define    PMU_PWR_PWRTESTCTRL_PC_APB_EN_SHIFT                            10
#define    PMU_PWR_PWRTESTCTRL_PC_APB_EN_MASK                             0x00000400
#define    PMU_PWR_PWRTESTCTRL_PWR_ATE_MODE_SHIFT                         9
#define    PMU_PWR_PWRTESTCTRL_PWR_ATE_MODE_MASK                          0x00000200
#define    PMU_PWR_PWRTESTCTRL_BYPASS_OVERI_DEB_SHIFT                     8
#define    PMU_PWR_PWRTESTCTRL_BYPASS_OVERI_DEB_MASK                      0x00000100
#define    PMU_PWR_PWRTESTCTRL_DIS_USB_LDO_PULLDB_EN_SHIFT                7
#define    PMU_PWR_PWRTESTCTRL_DIS_USB_LDO_PULLDB_EN_MASK                 0x00000080
#define    PMU_PWR_PWRTESTCTRL_LDO_GRP2_TEST_1_0__SHIFT                   5
#define    PMU_PWR_PWRTESTCTRL_LDO_GRP2_TEST_1_0__MASK                    0x00000060
#define    PMU_PWR_PWRTESTCTRL_LDO_GRP1_TEST_2_0__SHIFT                   2
#define    PMU_PWR_PWRTESTCTRL_LDO_GRP1_TEST_2_0__MASK                    0x0000001C
#define    PMU_PWR_PWRTESTCTRL_LDO_DVS_TEST_1_0__SHIFT                    0
#define    PMU_PWR_PWRTESTCTRL_LDO_DVS_TEST_1_0__MASK                     0x00000003

#endif /* __BRCM_RDB_PMU_PWR_H__ */



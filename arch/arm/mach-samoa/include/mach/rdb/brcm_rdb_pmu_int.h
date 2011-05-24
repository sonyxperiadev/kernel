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

#ifndef __BRCM_RDB_PMU_INT_H__
#define __BRCM_RDB_PMU_INT_H__

#define PMU_INT_INT1_OFFSET                                               0x00000000
#define PMU_INT_INT1_TYPE                                                 UInt32
#define PMU_INT_INT1_RESERVED_MASK                                        0xE0000000
#define    PMU_INT_INT1_SMPL_INT_SHIFT                                    28
#define    PMU_INT_INT1_SMPL_INT_MASK                                     0x10000000
#define    PMU_INT_INT1_VBUS_4P5V_F_SHIFT                                 27
#define    PMU_INT_INT1_VBUS_4P5V_F_MASK                                  0x08000000
#define    PMU_INT_INT1_VBUS_1P5V_F_SHIFT                                 26
#define    PMU_INT_INT1_VBUS_1P5V_F_MASK                                  0x04000000
#define    PMU_INT_INT1_VBUS_4P5V_R_SHIFT                                 25
#define    PMU_INT_INT1_VBUS_4P5V_R_MASK                                  0x02000000
#define    PMU_INT_INT1_VBUS_1P5V_R_SHIFT                                 24
#define    PMU_INT_INT1_VBUS_1P5V_R_MASK                                  0x01000000
#define    PMU_INT_INT1_ACP7_INT_SHIFT                                    23
#define    PMU_INT_INT1_ACP7_INT_MASK                                     0x00800000
#define    PMU_INT_INT1_CHGERRDIS_SHIFT                                   22
#define    PMU_INT_INT1_CHGERRDIS_MASK                                    0x00400000
#define    PMU_INT_INT1_UBPD_CHG_F_SHIFT                                  21
#define    PMU_INT_INT1_UBPD_CHG_F_MASK                                   0x00200000
#define    PMU_INT_INT1_CGPD_CHG_F_SHIFT                                  20
#define    PMU_INT_INT1_CGPD_CHG_F_MASK                                   0x00100000
#define    PMU_INT_INT1_CV_TMR_EXP_SHIFT                                  19
#define    PMU_INT_INT1_CV_TMR_EXP_MASK                                   0x00080000
#define    PMU_INT_INT1_MBC_CV_LOOP_SHIFT                                 18
#define    PMU_INT_INT1_MBC_CV_LOOP_MASK                                  0x00040000
#define    PMU_INT_INT1_USB_REVERSE_POLARITY_SHIFT                        17
#define    PMU_INT_INT1_USB_REVERSE_POLARITY_MASK                         0x00020000
#define    PMU_INT_INT1_WAC_REVERSE_POLARITY_SHIFT                        16
#define    PMU_INT_INT1_WAC_REVERSE_POLARITY_MASK                         0x00010000
#define    PMU_INT_INT1_USB_OVERI_SHIFT                                   15
#define    PMU_INT_INT1_USB_OVERI_MASK                                    0x00008000
#define    PMU_INT_INT1_UBPD_CHP_DIS_SHIFT                                14
#define    PMU_INT_INT1_UBPD_CHP_DIS_MASK                                 0x00004000
#define    PMU_INT_INT1_USBRM_SHIFT                                       13
#define    PMU_INT_INT1_USBRM_MASK                                        0x00002000
#define    PMU_INT_INT1_USBINS_SHIFT                                      12
#define    PMU_INT_INT1_USBINS_MASK                                       0x00001000
#define    PMU_INT_INT1_WAC_OVERI_SHIFT                                   11
#define    PMU_INT_INT1_WAC_OVERI_MASK                                    0x00000800
#define    PMU_INT_INT1_CHGOV_DIS_SHIFT                                   10
#define    PMU_INT_INT1_CHGOV_DIS_MASK                                    0x00000400
#define    PMU_INT_INT1_CHGRM_SHIFT                                       9
#define    PMU_INT_INT1_CHGRM_MASK                                        0x00000200
#define    PMU_INT_INT1_CHGINS_SHIFT                                      8
#define    PMU_INT_INT1_CHGINS_MASK                                       0x00000100
#define    PMU_INT_INT1_GBAT_PLUG_IN_SHIFT                                7
#define    PMU_INT_INT1_GBAT_PLUG_IN_MASK                                 0x00000080
#define    PMU_INT_INT1_AUX_RM_SHIFT                                      6
#define    PMU_INT_INT1_AUX_RM_MASK                                       0x00000040
#define    PMU_INT_INT1_AUX_INS_SHIFT                                     5
#define    PMU_INT_INT1_AUX_INS_MASK                                      0x00000020
#define    PMU_INT_INT1_POK_SHDWN_SHIFT                                   4
#define    PMU_INT_INT1_POK_SHDWN_MASK                                    0x00000010
#define    PMU_INT_INT1_POK_BIT_VLD_SHIFT                                 3
#define    PMU_INT_INT1_POK_BIT_VLD_MASK                                  0x00000008
#define    PMU_INT_INT1_POK_WAKEUP_SHIFT                                  2
#define    PMU_INT_INT1_POK_WAKEUP_MASK                                   0x00000004
#define    PMU_INT_INT1_POK_RELEASED_SHIFT                                1
#define    PMU_INT_INT1_POK_RELEASED_MASK                                 0x00000002
#define    PMU_INT_INT1_POK_PRESSED_SHIFT                                 0
#define    PMU_INT_INT1_POK_PRESSED_MASK                                  0x00000001

#define PMU_INT_INT2_OFFSET                                               0x00000004
#define PMU_INT_INT2_TYPE                                                 UInt32
#define PMU_INT_INT2_RESERVED_MASK                                        0xF0000008
#define    PMU_INT_INT2_RTCADJ_SHIFT                                      27
#define    PMU_INT_INT2_RTCADJ_MASK                                       0x08000000
#define    PMU_INT_INT2_RTC_MIN_SHIFT                                     26
#define    PMU_INT_INT2_RTC_MIN_MASK                                      0x04000000
#define    PMU_INT_INT2_RTC_SEC_SHIFT                                     25
#define    PMU_INT_INT2_RTC_SEC_MASK                                      0x02000000
#define    PMU_INT_INT2_RTC_ALARM_SHIFT                                   24
#define    PMU_INT_INT2_RTC_ALARM_MASK                                    0x01000000
#define    PMU_INT_INT2_FGC_SHIFT                                         23
#define    PMU_INT_INT2_FGC_MASK                                          0x00800000
#define    PMU_INT_INT2_BBLOW_SHIFT                                       22
#define    PMU_INT_INT2_BBLOW_MASK                                        0x00400000
#define    PMU_INT_INT2_MBOV_SHIFT                                        21
#define    PMU_INT_INT2_MBOV_MASK                                         0x00200000
#define    PMU_INT_INT2_MBWV_R_10S_WAIT_SHIFT                             20
#define    PMU_INT_INT2_MBWV_R_10S_WAIT_MASK                              0x00100000
#define    PMU_INT_INT2_MBWV_F_SHIFT                                      19
#define    PMU_INT_INT2_MBWV_F_MASK                                       0x00080000
#define    PMU_INT_INT2_OTP_LOAD_ERR_SHIFT                                18
#define    PMU_INT_INT2_OTP_LOAD_ERR_MASK                                 0x00040000
#define    PMU_INT_INT2_CHG_HW_TMR_EXP_SHIFT                              17
#define    PMU_INT_INT2_CHG_HW_TMR_EXP_MASK                               0x00020000
#define    PMU_INT_INT2_CHG_SW_TMR_EXP_SHIFT                              16
#define    PMU_INT_INT2_CHG_SW_TMR_EXP_MASK                               0x00010000
#define    PMU_INT_INT2_USBOV_DIS_SHIFT                                   15
#define    PMU_INT_INT2_USBOV_DIS_MASK                                    0x00008000
#define    PMU_INT_INT2_MBOV_DIS_SHIFT                                    14
#define    PMU_INT_INT2_MBOV_DIS_MASK                                     0x00004000
#define    PMU_INT_INT2_USBOV_SHIFT                                       13
#define    PMU_INT_INT2_USBOV_MASK                                        0x00002000
#define    PMU_INT_INT2_CHGOV_SHIFT                                       12
#define    PMU_INT_INT2_CHGOV_MASK                                        0x00001000
#define    PMU_INT_INT2_MBTEMPHIGH_SHIFT                                  11
#define    PMU_INT_INT2_MBTEMPHIGH_MASK                                   0x00000800
#define    PMU_INT_INT2_MBTEMPLOW_SHIFT                                   10
#define    PMU_INT_INT2_MBTEMPLOW_MASK                                    0x00000400
#define    PMU_INT_INT2_BATRM_SHIFT                                       9
#define    PMU_INT_INT2_BATRM_MASK                                        0x00000200
#define    PMU_INT_INT2_BATINS_SHIFT                                      8
#define    PMU_INT_INT2_BATINS_MASK                                       0x00000100
#define    PMU_INT_INT2_RESUME_VWALL_SHIFT                                7
#define    PMU_INT_INT2_RESUME_VWALL_MASK                                 0x00000080
#define    PMU_INT_INT2_RESUME_VBUS_SHIFT                                 6
#define    PMU_INT_INT2_RESUME_VBUS_MASK                                  0x00000040
#define    PMU_INT_INT2_DIE_OT2_SHIFT                                     5
#define    PMU_INT_INT2_DIE_OT2_MASK                                      0x00000020
#define    PMU_INT_INT2_DIE_OT1_SHIFT                                     4
#define    PMU_INT_INT2_DIE_OT1_MASK                                      0x00000010
#define    PMU_INT_INT2_CHGDET_TO_SHIFT                                   2
#define    PMU_INT_INT2_CHGDET_TO_MASK                                    0x00000004
#define    PMU_INT_INT2_CHGDET_LATCH_SHIFT                                1
#define    PMU_INT_INT2_CHGDET_LATCH_MASK                                 0x00000002
#define    PMU_INT_INT2_POWLIM_LOOP_SHIFT                                 0
#define    PMU_INT_INT2_POWLIM_LOOP_MASK                                  0x00000001

#define PMU_INT_INT3_OFFSET                                               0x00000008
#define PMU_INT_INT3_TYPE                                                 UInt32
#define PMU_INT_INT3_RESERVED_MASK                                        0xFC0000C8
#define    PMU_INT_INT3_USBLDO_SHD_SHIFT                                  25
#define    PMU_INT_INT3_USBLDO_SHD_MASK                                   0x02000000
#define    PMU_INT_INT3_SIMLDO2_SHD_SHIFT                                 24
#define    PMU_INT_INT3_SIMLDO2_SHD_MASK                                  0x01000000
#define    PMU_INT_INT3_SIMLDO1_SHD_SHIFT                                 23
#define    PMU_INT_INT3_SIMLDO1_SHD_MASK                                  0x00800000
#define    PMU_INT_INT3_HVLDO2_SHD_SHIFT                                  22
#define    PMU_INT_INT3_HVLDO2_SHD_MASK                                   0x00400000
#define    PMU_INT_INT3_HVLDO1_SHD_SHIFT                                  21
#define    PMU_INT_INT3_HVLDO1_SHD_MASK                                   0x00200000
#define    PMU_INT_INT3_RFLDO_SHD_SHIFT                                   20
#define    PMU_INT_INT3_RFLDO_SHD_MASK                                    0x00100000
#define    PMU_INT_INT3_CAMLDO_SHD_SHIFT                                  19
#define    PMU_INT_INT3_CAMLDO_SHD_MASK                                   0x00080000
#define    PMU_INT_INT3_DVSLDO2OVRI_SHIFT                                 18
#define    PMU_INT_INT3_DVSLDO2OVRI_MASK                                  0x00040000
#define    PMU_INT_INT3_DVSLDO1OVRI_SHIFT                                 17
#define    PMU_INT_INT3_DVSLDO1OVRI_MASK                                  0x00020000
#define    PMU_INT_INT3_USBLDOOVRI_SHIFT                                  16
#define    PMU_INT_INT3_USBLDOOVRI_MASK                                   0x00010000
#define    PMU_INT_INT3_SIMLDO2OVRI_SHIFT                                 15
#define    PMU_INT_INT3_SIMLDO2OVRI_MASK                                  0x00008000
#define    PMU_INT_INT3_SIMLDO1OVRI_SHIFT                                 14
#define    PMU_INT_INT3_SIMLDO1OVRI_MASK                                  0x00004000
#define    PMU_INT_INT3_HVLDO2OVRI_SHIFT                                  13
#define    PMU_INT_INT3_HVLDO2OVRI_MASK                                   0x00002000
#define    PMU_INT_INT3_HVLDO1OVRI_SHIFT                                  12
#define    PMU_INT_INT3_HVLDO1OVRI_MASK                                   0x00001000
#define    PMU_INT_INT3_RFLDOOVRI_SHIFT                                   11
#define    PMU_INT_INT3_RFLDOOVRI_MASK                                    0x00000800
#define    PMU_INT_INT3_CAMLDOOVRI_SHIFT                                  10
#define    PMU_INT_INT3_CAMLDOOVRI_MASK                                   0x00000400
#define    PMU_INT_INT3_IOSROVRI_SHIFT                                    9
#define    PMU_INT_INT3_IOSROVRI_MASK                                     0x00000200
#define    PMU_INT_INT3_CSROVRI_SHIFT                                     8
#define    PMU_INT_INT3_CSROVRI_MASK                                      0x00000100
#define    PMU_INT_INT3_XTAL_FAILURE_SHIFT                                5
#define    PMU_INT_INT3_XTAL_FAILURE_MASK                                 0x00000020
#define    PMU_INT_INT3_RTM_OVERRIDDEN_SHIFT                              4
#define    PMU_INT_INT3_RTM_OVERRIDDEN_MASK                               0x00000010
#define    PMU_INT_INT3_RTM_UPPER_BOUND_SHIFT                             2
#define    PMU_INT_INT3_RTM_UPPER_BOUND_MASK                              0x00000004
#define    PMU_INT_INT3_RTM_DURING_CON_MEAS_SHIFT                         1
#define    PMU_INT_INT3_RTM_DURING_CON_MEAS_MASK                          0x00000002
#define    PMU_INT_INT3_RTM_DATA_RDY_SHIFT                                0
#define    PMU_INT_INT3_RTM_DATA_RDY_MASK                                 0x00000001

#define PMU_INT_INT1MSK_OFFSET                                            0x00000040
#define PMU_INT_INT1MSK_TYPE                                              UInt32
#define PMU_INT_INT1MSK_RESERVED_MASK                                     0xE0000000
#define    PMU_INT_INT1MSK_SMPL_INT_MSK_SHIFT                             28
#define    PMU_INT_INT1MSK_SMPL_INT_MSK_MASK                              0x10000000
#define    PMU_INT_INT1MSK_VBUS_4P5V_F_MSK_SHIFT                          27
#define    PMU_INT_INT1MSK_VBUS_4P5V_F_MSK_MASK                           0x08000000
#define    PMU_INT_INT1MSK_VBUS_1P5V_F_MSK_SHIFT                          26
#define    PMU_INT_INT1MSK_VBUS_1P5V_F_MSK_MASK                           0x04000000
#define    PMU_INT_INT1MSK_VBUS_4P5V_R_MSK_SHIFT                          25
#define    PMU_INT_INT1MSK_VBUS_4P5V_R_MSK_MASK                           0x02000000
#define    PMU_INT_INT1MSK_VBUS_1P5V_R_MSK_SHIFT                          24
#define    PMU_INT_INT1MSK_VBUS_1P5V_R_MSK_MASK                           0x01000000
#define    PMU_INT_INT1MSK_ACP7_INT_MSK_SHIFT                             23
#define    PMU_INT_INT1MSK_ACP7_INT_MSK_MASK                              0x00800000
#define    PMU_INT_INT1MSK_CHGERRDIS_MSK_SHIFT                            22
#define    PMU_INT_INT1MSK_CHGERRDIS_MSK_MASK                             0x00400000
#define    PMU_INT_INT1MSK_UBPD_CHG_F_MSK_SHIFT                           21
#define    PMU_INT_INT1MSK_UBPD_CHG_F_MSK_MASK                            0x00200000
#define    PMU_INT_INT1MSK_CGPD_CHG_F_MSK_SHIFT                           20
#define    PMU_INT_INT1MSK_CGPD_CHG_F_MSK_MASK                            0x00100000
#define    PMU_INT_INT1MSK_CV_TMR_EXP_MSK_SHIFT                           19
#define    PMU_INT_INT1MSK_CV_TMR_EXP_MSK_MASK                            0x00080000
#define    PMU_INT_INT1MSK_MBC_CV_LOOP_MSK_SHIFT                          18
#define    PMU_INT_INT1MSK_MBC_CV_LOOP_MSK_MASK                           0x00040000
#define    PMU_INT_INT1MSK_USB_REVERSE_POLARITY_MSK_SHIFT                 17
#define    PMU_INT_INT1MSK_USB_REVERSE_POLARITY_MSK_MASK                  0x00020000
#define    PMU_INT_INT1MSK_WAC_REVERSE_POLARITY_MSK_SHIFT                 16
#define    PMU_INT_INT1MSK_WAC_REVERSE_POLARITY_MSK_MASK                  0x00010000
#define    PMU_INT_INT1MSK_USB_OVERI_MSK_SHIFT                            15
#define    PMU_INT_INT1MSK_USB_OVERI_MSK_MASK                             0x00008000
#define    PMU_INT_INT1MSK_UBPD_CHP_DIS_MSK_SHIFT                         14
#define    PMU_INT_INT1MSK_UBPD_CHP_DIS_MSK_MASK                          0x00004000
#define    PMU_INT_INT1MSK_USBRM_MSK_SHIFT                                13
#define    PMU_INT_INT1MSK_USBRM_MSK_MASK                                 0x00002000
#define    PMU_INT_INT1MSK_USBINS_MSK_SHIFT                               12
#define    PMU_INT_INT1MSK_USBINS_MSK_MASK                                0x00001000
#define    PMU_INT_INT1MSK_WAC_OVERI_MSK_SHIFT                            11
#define    PMU_INT_INT1MSK_WAC_OVERI_MSK_MASK                             0x00000800
#define    PMU_INT_INT1MSK_CHGOV_DIS_MSK_SHIFT                            10
#define    PMU_INT_INT1MSK_CHGOV_DIS_MSK_MASK                             0x00000400
#define    PMU_INT_INT1MSK_CHGRM_MSK_SHIFT                                9
#define    PMU_INT_INT1MSK_CHGRM_MSK_MASK                                 0x00000200
#define    PMU_INT_INT1MSK_CHGINS_MSK_SHIFT                               8
#define    PMU_INT_INT1MSK_CHGINS_MSK_MASK                                0x00000100
#define    PMU_INT_INT1MSK_GBAT_PLUG_IN_MSK_SHIFT                         7
#define    PMU_INT_INT1MSK_GBAT_PLUG_IN_MSK_MASK                          0x00000080
#define    PMU_INT_INT1MSK_AUX_RM_MSK_SHIFT                               6
#define    PMU_INT_INT1MSK_AUX_RM_MSK_MASK                                0x00000040
#define    PMU_INT_INT1MSK_AUX_INS_MSK_SHIFT                              5
#define    PMU_INT_INT1MSK_AUX_INS_MSK_MASK                               0x00000020
#define    PMU_INT_INT1MSK_POK_SHDWN_MSK_SHIFT                            4
#define    PMU_INT_INT1MSK_POK_SHDWN_MSK_MASK                             0x00000010
#define    PMU_INT_INT1MSK_POK_BIT_VLD_MSK_SHIFT                          3
#define    PMU_INT_INT1MSK_POK_BIT_VLD_MSK_MASK                           0x00000008
#define    PMU_INT_INT1MSK_POK_WAKEUP_MSK_SHIFT                           2
#define    PMU_INT_INT1MSK_POK_WAKEUP_MSK_MASK                            0x00000004
#define    PMU_INT_INT1MSK_POK_RELEASED_MSK_SHIFT                         1
#define    PMU_INT_INT1MSK_POK_RELEASED_MSK_MASK                          0x00000002
#define    PMU_INT_INT1MSK_POK_PRESSED_MSK_SHIFT                          0
#define    PMU_INT_INT1MSK_POK_PRESSED_MSK_MASK                           0x00000001

#define PMU_INT_INT2MSK_OFFSET                                            0x00000044
#define PMU_INT_INT2MSK_TYPE                                              UInt32
#define PMU_INT_INT2MSK_RESERVED_MASK                                     0xF0000008
#define    PMU_INT_INT2MSK_RTCADJ_MSK_SHIFT                               27
#define    PMU_INT_INT2MSK_RTCADJ_MSK_MASK                                0x08000000
#define    PMU_INT_INT2MSK_RTC_MIN_MSK_SHIFT                              26
#define    PMU_INT_INT2MSK_RTC_MIN_MSK_MASK                               0x04000000
#define    PMU_INT_INT2MSK_RTC_SEC_MSK_SHIFT                              25
#define    PMU_INT_INT2MSK_RTC_SEC_MSK_MASK                               0x02000000
#define    PMU_INT_INT2MSK_RTC_ALARM_MSK_SHIFT                            24
#define    PMU_INT_INT2MSK_RTC_ALARM_MSK_MASK                             0x01000000
#define    PMU_INT_INT2MSK_FGC_MSK_SHIFT                                  23
#define    PMU_INT_INT2MSK_FGC_MSK_MASK                                   0x00800000
#define    PMU_INT_INT2MSK_BBLOW_MSK_SHIFT                                22
#define    PMU_INT_INT2MSK_BBLOW_MSK_MASK                                 0x00400000
#define    PMU_INT_INT2MSK_MBOV_MSK_SHIFT                                 21
#define    PMU_INT_INT2MSK_MBOV_MSK_MASK                                  0x00200000
#define    PMU_INT_INT2MSK_MBWV_R_10S_WAIT_MSK_SHIFT                      20
#define    PMU_INT_INT2MSK_MBWV_R_10S_WAIT_MSK_MASK                       0x00100000
#define    PMU_INT_INT2MSK_MBWV_F_MSK_SHIFT                               19
#define    PMU_INT_INT2MSK_MBWV_F_MSK_MASK                                0x00080000
#define    PMU_INT_INT2MSK_OTP_LOAD_ERR_MSK_SHIFT                         18
#define    PMU_INT_INT2MSK_OTP_LOAD_ERR_MSK_MASK                          0x00040000
#define    PMU_INT_INT2MSK_CHG_HW_TMR_EXP_MSK_SHIFT                       17
#define    PMU_INT_INT2MSK_CHG_HW_TMR_EXP_MSK_MASK                        0x00020000
#define    PMU_INT_INT2MSK_CHG_SW_TMR_EXP_MSK_SHIFT                       16
#define    PMU_INT_INT2MSK_CHG_SW_TMR_EXP_MSK_MASK                        0x00010000
#define    PMU_INT_INT2MSK_USBOV_DIS_MSK_SHIFT                            15
#define    PMU_INT_INT2MSK_USBOV_DIS_MSK_MASK                             0x00008000
#define    PMU_INT_INT2MSK_MBOV_DIS_MSK_SHIFT                             14
#define    PMU_INT_INT2MSK_MBOV_DIS_MSK_MASK                              0x00004000
#define    PMU_INT_INT2MSK_USBOV_MSK_SHIFT                                13
#define    PMU_INT_INT2MSK_USBOV_MSK_MASK                                 0x00002000
#define    PMU_INT_INT2MSK_CHGOV_MSK_SHIFT                                12
#define    PMU_INT_INT2MSK_CHGOV_MSK_MASK                                 0x00001000
#define    PMU_INT_INT2MSK_MBTEMPHIGH_MSK_SHIFT                           11
#define    PMU_INT_INT2MSK_MBTEMPHIGH_MSK_MASK                            0x00000800
#define    PMU_INT_INT2MSK_MBTEMPLOW_MSK_SHIFT                            10
#define    PMU_INT_INT2MSK_MBTEMPLOW_MSK_MASK                             0x00000400
#define    PMU_INT_INT2MSK_BATRM_MSK_SHIFT                                9
#define    PMU_INT_INT2MSK_BATRM_MSK_MASK                                 0x00000200
#define    PMU_INT_INT2MSK_BATINS_MSK_SHIFT                               8
#define    PMU_INT_INT2MSK_BATINS_MSK_MASK                                0x00000100
#define    PMU_INT_INT2MSK_RESUME_VWALL_MSK_SHIFT                         7
#define    PMU_INT_INT2MSK_RESUME_VWALL_MSK_MASK                          0x00000080
#define    PMU_INT_INT2MSK_RESUME_VBUS_MSK_SHIFT                          6
#define    PMU_INT_INT2MSK_RESUME_VBUS_MSK_MASK                           0x00000040
#define    PMU_INT_INT2MSK_DIE_OT2_MSK_SHIFT                              5
#define    PMU_INT_INT2MSK_DIE_OT2_MSK_MASK                               0x00000020
#define    PMU_INT_INT2MSK_DIE_OT1_MSK_SHIFT                              4
#define    PMU_INT_INT2MSK_DIE_OT1_MSK_MASK                               0x00000010
#define    PMU_INT_INT2MSK_CHGDET_TO_MSK_SHIFT                            2
#define    PMU_INT_INT2MSK_CHGDET_TO_MSK_MASK                             0x00000004
#define    PMU_INT_INT2MSK_CHGDET_LATCH_MSK_SHIFT                         1
#define    PMU_INT_INT2MSK_CHGDET_LATCH_MSK_MASK                          0x00000002
#define    PMU_INT_INT2MSK_POWLIM_LOOP_MSK_SHIFT                          0
#define    PMU_INT_INT2MSK_POWLIM_LOOP_MSK_MASK                           0x00000001

#define PMU_INT_INT3MSK_OFFSET                                            0x00000048
#define PMU_INT_INT3MSK_TYPE                                              UInt32
#define PMU_INT_INT3MSK_RESERVED_MASK                                     0xFC0000C8
#define    PMU_INT_INT3MSK_USBLDO_SHD_MSK_SHIFT                           25
#define    PMU_INT_INT3MSK_USBLDO_SHD_MSK_MASK                            0x02000000
#define    PMU_INT_INT3MSK_SIMLDO2_SHD_MSK_SHIFT                          24
#define    PMU_INT_INT3MSK_SIMLDO2_SHD_MSK_MASK                           0x01000000
#define    PMU_INT_INT3MSK_SIMLDO1_SHD_MSK_SHIFT                          23
#define    PMU_INT_INT3MSK_SIMLDO1_SHD_MSK_MASK                           0x00800000
#define    PMU_INT_INT3MSK_HVLDO2_SHD_MSK_SHIFT                           22
#define    PMU_INT_INT3MSK_HVLDO2_SHD_MSK_MASK                            0x00400000
#define    PMU_INT_INT3MSK_HVLDO1_SHD_MSK_SHIFT                           21
#define    PMU_INT_INT3MSK_HVLDO1_SHD_MSK_MASK                            0x00200000
#define    PMU_INT_INT3MSK_RFLDO_SHD_MSK_SHIFT                            20
#define    PMU_INT_INT3MSK_RFLDO_SHD_MSK_MASK                             0x00100000
#define    PMU_INT_INT3MSK_CAMLDO_SHD_MSK_SHIFT                           19
#define    PMU_INT_INT3MSK_CAMLDO_SHD_MSK_MASK                            0x00080000
#define    PMU_INT_INT3MSK_DVSLDO2OVRI_MSK_SHIFT                          18
#define    PMU_INT_INT3MSK_DVSLDO2OVRI_MSK_MASK                           0x00040000
#define    PMU_INT_INT3MSK_DVSLDO1OVRI_MSK_SHIFT                          17
#define    PMU_INT_INT3MSK_DVSLDO1OVRI_MSK_MASK                           0x00020000
#define    PMU_INT_INT3MSK_USBLDOOVRI_MSK_SHIFT                           16
#define    PMU_INT_INT3MSK_USBLDOOVRI_MSK_MASK                            0x00010000
#define    PMU_INT_INT3MSK_SIMLDO2OVRI_MSK_SHIFT                          15
#define    PMU_INT_INT3MSK_SIMLDO2OVRI_MSK_MASK                           0x00008000
#define    PMU_INT_INT3MSK_SIMLDO1OVRI_MSK_SHIFT                          14
#define    PMU_INT_INT3MSK_SIMLDO1OVRI_MSK_MASK                           0x00004000
#define    PMU_INT_INT3MSK_HVLDO2OVRI_MSK_SHIFT                           13
#define    PMU_INT_INT3MSK_HVLDO2OVRI_MSK_MASK                            0x00002000
#define    PMU_INT_INT3MSK_HVLDO1OVRI_MSK_SHIFT                           12
#define    PMU_INT_INT3MSK_HVLDO1OVRI_MSK_MASK                            0x00001000
#define    PMU_INT_INT3MSK_RFLDOOVRI_MSK_SHIFT                            11
#define    PMU_INT_INT3MSK_RFLDOOVRI_MSK_MASK                             0x00000800
#define    PMU_INT_INT3MSK_CAMLDOOVRI_MSK_SHIFT                           10
#define    PMU_INT_INT3MSK_CAMLDOOVRI_MSK_MASK                            0x00000400
#define    PMU_INT_INT3MSK_IOSROVRI_MSK_SHIFT                             9
#define    PMU_INT_INT3MSK_IOSROVRI_MSK_MASK                              0x00000200
#define    PMU_INT_INT3MSK_CSROVRI_MSK_SHIFT                              8
#define    PMU_INT_INT3MSK_CSROVRI_MSK_MASK                               0x00000100
#define    PMU_INT_INT3MSK_XTAL_FAILURE_MSK_SHIFT                         5
#define    PMU_INT_INT3MSK_XTAL_FAILURE_MSK_MASK                          0x00000020
#define    PMU_INT_INT3MSK_RTM_OVERRIDDEN_MSK_SHIFT                       4
#define    PMU_INT_INT3MSK_RTM_OVERRIDDEN_MSK_MASK                        0x00000010
#define    PMU_INT_INT3MSK_RTM_UPPER_BOUND_MSK_SHIFT                      2
#define    PMU_INT_INT3MSK_RTM_UPPER_BOUND_MSK_MASK                       0x00000004
#define    PMU_INT_INT3MSK_RTM_DURING_CON_MEAS_MSK_SHIFT                  1
#define    PMU_INT_INT3MSK_RTM_DURING_CON_MEAS_MSK_MASK                   0x00000002
#define    PMU_INT_INT3MSK_RTM_DATA_RDY_MSK_SHIFT                         0
#define    PMU_INT_INT3MSK_RTM_DATA_RDY_MSK_MASK                          0x00000001

#endif /* __BRCM_RDB_PMU_INT_H__ */



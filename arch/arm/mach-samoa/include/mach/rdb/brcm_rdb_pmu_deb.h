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

#ifndef __BRCM_RDB_PMU_DEB_H__
#define __BRCM_RDB_PMU_DEB_H__

#define PMU_DEB_ENV1_OFFSET                                               0x00000000
#define PMU_DEB_ENV1_TYPE                                                 UInt32
#define PMU_DEB_ENV1_RESERVED_MASK                                        0x7F000000
#define    PMU_DEB_ENV1_P_FGC_SHIFT                                       31
#define    PMU_DEB_ENV1_P_FGC_MASK                                        0x80000000
#define    PMU_DEB_ENV1_USB_PORT_DISABLE_SHIFT                            23
#define    PMU_DEB_ENV1_USB_PORT_DISABLE_MASK                             0x00800000
#define    PMU_DEB_ENV1_ACP7_DET_SHIFT                                    22
#define    PMU_DEB_ENV1_ACP7_DET_MASK                                     0x00400000
#define    PMU_DEB_ENV1_P_USBOV_SHIFT                                     21
#define    PMU_DEB_ENV1_P_USBOV_MASK                                      0x00200000
#define    PMU_DEB_ENV1_P_CHGOV_SHIFT                                     20
#define    PMU_DEB_ENV1_P_CHGOV_MASK                                      0x00100000
#define    PMU_DEB_ENV1_P_UBMBC_SHIFT                                     19
#define    PMU_DEB_ENV1_P_UBMBC_MASK                                      0x00080000
#define    PMU_DEB_ENV1_P_CGMBC_SHIFT                                     18
#define    PMU_DEB_ENV1_P_CGMBC_MASK                                      0x00040000
#define    PMU_DEB_ENV1_P_UBPD_CHG_SHIFT                                  17
#define    PMU_DEB_ENV1_P_UBPD_CHG_MASK                                   0x00020000
#define    PMU_DEB_ENV1_P_CGPD_CHG_SHIFT                                  16
#define    PMU_DEB_ENV1_P_CGPD_CHG_MASK                                   0x00010000
#define    PMU_DEB_ENV1_USB_VALID_SHIFT                                   15
#define    PMU_DEB_ENV1_USB_VALID_MASK                                    0x00008000
#define    PMU_DEB_ENV1_WAC_VALID_SHIFT                                   14
#define    PMU_DEB_ENV1_WAC_VALID_MASK                                    0x00004000
#define    PMU_DEB_ENV1_P_UBPD_PRI_SHIFT                                  13
#define    PMU_DEB_ENV1_P_UBPD_PRI_MASK                                   0x00002000
#define    PMU_DEB_ENV1_P_CGPD_PRI_SHIFT                                  12
#define    PMU_DEB_ENV1_P_CGPD_PRI_MASK                                   0x00001000
#define    PMU_DEB_ENV1_P_UBPD_INT_SHIFT                                  11
#define    PMU_DEB_ENV1_P_UBPD_INT_MASK                                   0x00000800
#define    PMU_DEB_ENV1_P_UBPD_USBDET_SHIFT                               10
#define    PMU_DEB_ENV1_P_UBPD_USBDET_MASK                                0x00000400
#define    PMU_DEB_ENV1_P_UBPD_ENV_SHIFT                                  9
#define    PMU_DEB_ENV1_P_UBPD_ENV_MASK                                   0x00000200
#define    PMU_DEB_ENV1_P_CGPD_ENV_SHIFT                                  8
#define    PMU_DEB_ENV1_P_CGPD_ENV_MASK                                   0x00000100
#define    PMU_DEB_ENV1_P_BBLOWB_SHIFT                                    7
#define    PMU_DEB_ENV1_P_BBLOWB_MASK                                     0x00000080
#define    PMU_DEB_ENV1_P_BSIWV_SHIFT                                     6
#define    PMU_DEB_ENV1_P_BSIWV_MASK                                      0x00000040
#define    PMU_DEB_ENV1_P_MBUV_SHIFT                                      5
#define    PMU_DEB_ENV1_P_MBUV_MASK                                       0x00000020
#define    PMU_DEB_ENV1_P_MBOV_SHIFT                                      4
#define    PMU_DEB_ENV1_P_MBOV_MASK                                       0x00000010
#define    PMU_DEB_ENV1_P_MBMC_SHIFT                                      3
#define    PMU_DEB_ENV1_P_MBMC_MASK                                       0x00000008
#define    PMU_DEB_ENV1_P_ERC_SHIFT                                       2
#define    PMU_DEB_ENV1_P_ERC_MASK                                        0x00000004
#define    PMU_DEB_ENV1_P_MBWV_DELTA_SHIFT                                1
#define    PMU_DEB_ENV1_P_MBWV_DELTA_MASK                                 0x00000002
#define    PMU_DEB_ENV1_P_MBWV_SHIFT                                      0
#define    PMU_DEB_ENV1_P_MBWV_MASK                                       0x00000001

#define PMU_DEB_ENV2_OFFSET                                               0x00000004
#define PMU_DEB_ENV2_TYPE                                                 UInt32
#define PMU_DEB_ENV2_RESERVED_MASK                                        0xFF2CFE40
#define    PMU_DEB_ENV2_STAT2_ENV_SHIFT                                   23
#define    PMU_DEB_ENV2_STAT2_ENV_MASK                                    0x00800000
#define    PMU_DEB_ENV2_STAT1_ENV_SHIFT                                   22
#define    PMU_DEB_ENV2_STAT1_ENV_MASK                                    0x00400000
#define    PMU_DEB_ENV2_TRUE_TF_SHIFT                                     20
#define    PMU_DEB_ENV2_TRUE_TF_MASK                                      0x00100000
#define    PMU_DEB_ENV2_P_USB_RVP_SHIFT                                   17
#define    PMU_DEB_ENV2_P_USB_RVP_MASK                                    0x00020000
#define    PMU_DEB_ENV2_P_WAC_RVP_SHIFT                                   16
#define    PMU_DEB_ENV2_P_WAC_RVP_MASK                                    0x00010000
#define    PMU_DEB_ENV2_MBC_ERR_SHIFT                                     8
#define    PMU_DEB_ENV2_MBC_ERR_MASK                                      0x00000100
#define    PMU_DEB_ENV2_P_MBPD_LONG_SHIFT                                 7
#define    PMU_DEB_ENV2_P_MBPD_LONG_MASK                                  0x00000080
#define    PMU_DEB_ENV2_P_THSD_SHIFT                                      5
#define    PMU_DEB_ENV2_P_THSD_MASK                                       0x00000020
#define    PMU_DEB_ENV2_P_AUXON_N_SHIFT                                   4
#define    PMU_DEB_ENV2_P_AUXON_N_MASK                                    0x00000010
#define    PMU_DEB_ENV2_P_PONKEY_N_SHIFT                                  3
#define    PMU_DEB_ENV2_P_PONKEY_N_MASK                                   0x00000008
#define    PMU_DEB_ENV2_P_MBTEMPHIGH_SHIFT                                2
#define    PMU_DEB_ENV2_P_MBTEMPHIGH_MASK                                 0x00000004
#define    PMU_DEB_ENV2_P_MBTEMPLOW_SHIFT                                 1
#define    PMU_DEB_ENV2_P_MBTEMPLOW_MASK                                  0x00000002
#define    PMU_DEB_ENV2_P_MBPD_SHIFT                                      0
#define    PMU_DEB_ENV2_P_MBPD_MASK                                       0x00000001

#define PMU_DEB_ENV3_OFFSET                                               0x00000008
#define PMU_DEB_ENV3_TYPE                                                 UInt32
#define PMU_DEB_ENV3_RESERVED_MASK                                        0xFC00FE00
#define    PMU_DEB_ENV3_OTP_LOAD_ERROR_SHIFT                              25
#define    PMU_DEB_ENV3_OTP_LOAD_ERROR_MASK                               0x02000000
#define    PMU_DEB_ENV3_DIS_CHG_XTAL_FAIL_SHIFT                           24
#define    PMU_DEB_ENV3_DIS_CHG_XTAL_FAIL_MASK                            0x01000000
#define    PMU_DEB_ENV3_CANT_WAKUP_BY_CHG_SHIFT                           23
#define    PMU_DEB_ENV3_CANT_WAKUP_BY_CHG_MASK                            0x00800000
#define    PMU_DEB_ENV3_WAKE_UP_EVENT_6_0__SHIFT                          16
#define    PMU_DEB_ENV3_WAKE_UP_EVENT_6_0__MASK                           0x007F0000
#define    PMU_DEB_ENV3_XTAL_FAIL_SHDWN_SHIFT                             8
#define    PMU_DEB_ENV3_XTAL_FAIL_SHDWN_MASK                              0x00000100
#define    PMU_DEB_ENV3_DVS_OVERI_SHDWN_SHIFT                             7
#define    PMU_DEB_ENV3_DVS_OVERI_SHDWN_MASK                              0x00000080
#define    PMU_DEB_ENV3_SR_OVERI_SHDWN_SHIFT                              6
#define    PMU_DEB_ENV3_SR_OVERI_SHDWN_MASK                               0x00000040
#define    PMU_DEB_ENV3_LONG_POK_SHDWN_SHIFT                              5
#define    PMU_DEB_ENV3_LONG_POK_SHDWN_MASK                               0x00000020
#define    PMU_DEB_ENV3_SW_SHDWN_SHIFT                                    4
#define    PMU_DEB_ENV3_SW_SHDWN_MASK                                     0x00000010
#define    PMU_DEB_ENV3_SYS_WDT_EXP_SHDWN_SHIFT                           3
#define    PMU_DEB_ENV3_SYS_WDT_EXP_SHDWN_MASK                            0x00000008
#define    PMU_DEB_ENV3_MBWV_SHDWN_SHIFT                                  2
#define    PMU_DEB_ENV3_MBWV_SHDWN_MASK                                   0x00000004
#define    PMU_DEB_ENV3_THSD_SHDWN_SHIFT                                  1
#define    PMU_DEB_ENV3_THSD_SHDWN_MASK                                   0x00000002
#define    PMU_DEB_ENV3_MBUV_SHDWN_SHIFT                                  0
#define    PMU_DEB_ENV3_MBUV_SHDWN_MASK                                   0x00000001

#endif /* __BRCM_RDB_PMU_DEB_H__ */



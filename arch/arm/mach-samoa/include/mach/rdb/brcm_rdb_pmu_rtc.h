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

#ifndef __BRCM_RDB_PMU_RTC_H__
#define __BRCM_RDB_PMU_RTC_H__

#define PMU_RTC_RTC_TIME_OFFSET                                           0x00000000
#define PMU_RTC_RTC_TIME_TYPE                                             UInt32
#define PMU_RTC_RTC_TIME_RESERVED_MASK                                    0x08E0C0C0
#define    PMU_RTC_RTC_TIME_BYTE_WRITE3_TIME_SHIFT                        31
#define    PMU_RTC_RTC_TIME_BYTE_WRITE3_TIME_MASK                         0x80000000
#define    PMU_RTC_RTC_TIME_BYTE_WRITE2_TIME_SHIFT                        30
#define    PMU_RTC_RTC_TIME_BYTE_WRITE2_TIME_MASK                         0x40000000
#define    PMU_RTC_RTC_TIME_BYTE_WRITE1_TIME_SHIFT                        29
#define    PMU_RTC_RTC_TIME_BYTE_WRITE1_TIME_MASK                         0x20000000
#define    PMU_RTC_RTC_TIME_BYTE_WRITE0_TIME_SHIFT                        28
#define    PMU_RTC_RTC_TIME_BYTE_WRITE0_TIME_MASK                         0x10000000
#define    PMU_RTC_RTC_TIME_WEEKDAY_2_0__SHIFT                            24
#define    PMU_RTC_RTC_TIME_WEEKDAY_2_0__MASK                             0x07000000
#define    PMU_RTC_RTC_TIME_HOUR_4_0__SHIFT                               16
#define    PMU_RTC_RTC_TIME_HOUR_4_0__MASK                                0x001F0000
#define    PMU_RTC_RTC_TIME_MIN_5_0__SHIFT                                8
#define    PMU_RTC_RTC_TIME_MIN_5_0__MASK                                 0x00003F00
#define    PMU_RTC_RTC_TIME_SEC_5_0__SHIFT                                0
#define    PMU_RTC_RTC_TIME_SEC_5_0__MASK                                 0x0000003F

#define PMU_RTC_RTC_DATE_OFFSET                                           0x00000004
#define PMU_RTC_RTC_DATE_TYPE                                             UInt32
#define PMU_RTC_RTC_DATE_RESERVED_MASK                                    0x8F00F0E0
#define    PMU_RTC_RTC_DATE_BYTE_WRITE2_DATE_SHIFT                        30
#define    PMU_RTC_RTC_DATE_BYTE_WRITE2_DATE_MASK                         0x40000000
#define    PMU_RTC_RTC_DATE_BYTE_WRITE1_DATE_SHIFT                        29
#define    PMU_RTC_RTC_DATE_BYTE_WRITE1_DATE_MASK                         0x20000000
#define    PMU_RTC_RTC_DATE_BYTE_WRITE0_DATE_SHIFT                        28
#define    PMU_RTC_RTC_DATE_BYTE_WRITE0_DATE_MASK                         0x10000000
#define    PMU_RTC_RTC_DATE_YEAR_7_0__SHIFT                               16
#define    PMU_RTC_RTC_DATE_YEAR_7_0__MASK                                0x00FF0000
#define    PMU_RTC_RTC_DATE_MONTH_3_0__SHIFT                              8
#define    PMU_RTC_RTC_DATE_MONTH_3_0__MASK                               0x00000F00
#define    PMU_RTC_RTC_DATE_DAY_4_0__SHIFT                                0
#define    PMU_RTC_RTC_DATE_DAY_4_0__MASK                                 0x0000001F

#define PMU_RTC_RTC_A1TIME_OFFSET                                         0x00000008
#define PMU_RTC_RTC_A1TIME_TYPE                                           UInt32
#define PMU_RTC_RTC_A1TIME_RESERVED_MASK                                  0x8FE0C0C0
#define    PMU_RTC_RTC_A1TIME_BYTE_WRITE2_A1TIME_SHIFT                    30
#define    PMU_RTC_RTC_A1TIME_BYTE_WRITE2_A1TIME_MASK                     0x40000000
#define    PMU_RTC_RTC_A1TIME_BYTE_WRITE1_A1TIME_SHIFT                    29
#define    PMU_RTC_RTC_A1TIME_BYTE_WRITE1_A1TIME_MASK                     0x20000000
#define    PMU_RTC_RTC_A1TIME_BYTE_WRITE0_A1TIME_SHIFT                    28
#define    PMU_RTC_RTC_A1TIME_BYTE_WRITE0_A1TIME_MASK                     0x10000000
#define    PMU_RTC_RTC_A1TIME_HOUR_A1_4_0__SHIFT                          16
#define    PMU_RTC_RTC_A1TIME_HOUR_A1_4_0__MASK                           0x001F0000
#define    PMU_RTC_RTC_A1TIME_MIN_A1_5_0__SHIFT                           8
#define    PMU_RTC_RTC_A1TIME_MIN_A1_5_0__MASK                            0x00003F00
#define    PMU_RTC_RTC_A1TIME_SEC_A1_5_0__SHIFT                           0
#define    PMU_RTC_RTC_A1TIME_SEC_A1_5_0__MASK                            0x0000003F

#define PMU_RTC_RTC_A1DATE_OFFSET                                         0x0000000C
#define PMU_RTC_RTC_A1DATE_TYPE                                           UInt32
#define PMU_RTC_RTC_A1DATE_RESERVED_MASK                                  0x8F00F0E0
#define    PMU_RTC_RTC_A1DATE_BYTE_WRITE2_A1DATE_SHIFT                    30
#define    PMU_RTC_RTC_A1DATE_BYTE_WRITE2_A1DATE_MASK                     0x40000000
#define    PMU_RTC_RTC_A1DATE_BYTE_WRITE1_A1DATE_SHIFT                    29
#define    PMU_RTC_RTC_A1DATE_BYTE_WRITE1_A1DATE_MASK                     0x20000000
#define    PMU_RTC_RTC_A1DATE_BYTE_WRITE0_A1DATE_SHIFT                    28
#define    PMU_RTC_RTC_A1DATE_BYTE_WRITE0_A1DATE_MASK                     0x10000000
#define    PMU_RTC_RTC_A1DATE_YEAR_A1_7_0__SHIFT                          16
#define    PMU_RTC_RTC_A1DATE_YEAR_A1_7_0__MASK                           0x00FF0000
#define    PMU_RTC_RTC_A1DATE_MONTH_A1_3_0__SHIFT                         8
#define    PMU_RTC_RTC_A1DATE_MONTH_A1_3_0__MASK                          0x00000F00
#define    PMU_RTC_RTC_A1DATE_DAY_A1_4_0__SHIFT                           0
#define    PMU_RTC_RTC_A1DATE_DAY_A1_4_0__MASK                            0x0000001F

#define PMU_RTC_RTC_A1ENAB_OFFSET                                         0x00000010
#define PMU_RTC_RTC_A1ENAB_TYPE                                           UInt32
#define PMU_RTC_RTC_A1ENAB_RESERVED_MASK                                  0xCFFF0080
#define    PMU_RTC_RTC_A1ENAB_BYTE_WRITE1_A1ENAB_SHIFT                    29
#define    PMU_RTC_RTC_A1ENAB_BYTE_WRITE1_A1ENAB_MASK                     0x20000000
#define    PMU_RTC_RTC_A1ENAB_BYTE_WRITE0_A1ENAB_SHIFT                    28
#define    PMU_RTC_RTC_A1ENAB_BYTE_WRITE0_A1ENAB_MASK                     0x10000000
#define    PMU_RTC_RTC_A1ENAB_WEEKDAY_A1_6_0__SHIFT                       8
#define    PMU_RTC_RTC_A1ENAB_WEEKDAY_A1_6_0__MASK                        0x0000FF00
#define    PMU_RTC_RTC_A1ENAB_ENABLES_A1_6_0__SHIFT                       0
#define    PMU_RTC_RTC_A1ENAB_ENABLES_A1_6_0__MASK                        0x0000007F

#define PMU_RTC_RTC_A2TIME_OFFSET                                         0x00000014
#define PMU_RTC_RTC_A2TIME_TYPE                                           UInt32
#define PMU_RTC_RTC_A2TIME_RESERVED_MASK                                  0x8FE0C0C0
#define    PMU_RTC_RTC_A2TIME_BYTE_WRITE2_A2TIME_SHIFT                    30
#define    PMU_RTC_RTC_A2TIME_BYTE_WRITE2_A2TIME_MASK                     0x40000000
#define    PMU_RTC_RTC_A2TIME_BYTE_WRITE1_A2TIME_SHIFT                    29
#define    PMU_RTC_RTC_A2TIME_BYTE_WRITE1_A2TIME_MASK                     0x20000000
#define    PMU_RTC_RTC_A2TIME_BYTE_WRITE0_A2TIME_SHIFT                    28
#define    PMU_RTC_RTC_A2TIME_BYTE_WRITE0_A2TIME_MASK                     0x10000000
#define    PMU_RTC_RTC_A2TIME_HOUR_A2_4_0__SHIFT                          16
#define    PMU_RTC_RTC_A2TIME_HOUR_A2_4_0__MASK                           0x001F0000
#define    PMU_RTC_RTC_A2TIME_MIN_A2_5_0__SHIFT                           8
#define    PMU_RTC_RTC_A2TIME_MIN_A2_5_0__MASK                            0x00003F00
#define    PMU_RTC_RTC_A2TIME_SEC_A2_5_0__SHIFT                           0
#define    PMU_RTC_RTC_A2TIME_SEC_A2_5_0__MASK                            0x0000003F

#define PMU_RTC_RTC_A2DATE_OFFSET                                         0x00000018
#define PMU_RTC_RTC_A2DATE_TYPE                                           UInt32
#define PMU_RTC_RTC_A2DATE_RESERVED_MASK                                  0x8F00F0E0
#define    PMU_RTC_RTC_A2DATE_BYTE_WRITE2_A2DATE_SHIFT                    30
#define    PMU_RTC_RTC_A2DATE_BYTE_WRITE2_A2DATE_MASK                     0x40000000
#define    PMU_RTC_RTC_A2DATE_BYTE_WRITE1_A2DATE_SHIFT                    29
#define    PMU_RTC_RTC_A2DATE_BYTE_WRITE1_A2DATE_MASK                     0x20000000
#define    PMU_RTC_RTC_A2DATE_BYTE_WRITE0_A2DATE_SHIFT                    28
#define    PMU_RTC_RTC_A2DATE_BYTE_WRITE0_A2DATE_MASK                     0x10000000
#define    PMU_RTC_RTC_A2DATE_YEAR_A2_7_0__SHIFT                          16
#define    PMU_RTC_RTC_A2DATE_YEAR_A2_7_0__MASK                           0x00FF0000
#define    PMU_RTC_RTC_A2DATE_MONTH_A2_3_0__SHIFT                         8
#define    PMU_RTC_RTC_A2DATE_MONTH_A2_3_0__MASK                          0x00000F00
#define    PMU_RTC_RTC_A2DATE_DAY_A2_3_0__SHIFT                           0
#define    PMU_RTC_RTC_A2DATE_DAY_A2_3_0__MASK                            0x0000001F

#define PMU_RTC_RTC_A2ENAB_OFFSET                                         0x0000001C
#define PMU_RTC_RTC_A2ENAB_TYPE                                           UInt32
#define PMU_RTC_RTC_A2ENAB_RESERVED_MASK                                  0xCFFF0080
#define    PMU_RTC_RTC_A2ENAB_BYTE_WRITE1_A2ENAB_SHIFT                    29
#define    PMU_RTC_RTC_A2ENAB_BYTE_WRITE1_A2ENAB_MASK                     0x20000000
#define    PMU_RTC_RTC_A2ENAB_BYTE_WRITE0_A2ENAB_SHIFT                    28
#define    PMU_RTC_RTC_A2ENAB_BYTE_WRITE0_A2ENAB_MASK                     0x10000000
#define    PMU_RTC_RTC_A2ENAB_WEEKDAY_A2_6_0__SHIFT                       8
#define    PMU_RTC_RTC_A2ENAB_WEEKDAY_A2_6_0__MASK                        0x0000FF00
#define    PMU_RTC_RTC_A2ENAB_ENABLES_A2_6_0__SHIFT                       0
#define    PMU_RTC_RTC_A2ENAB_ENABLES_A2_6_0__MASK                        0x0000007F

#define PMU_RTC_RTC_FREQ_OFFSET                                           0x00000020
#define PMU_RTC_RTC_FREQ_TYPE                                             UInt32
#define PMU_RTC_RTC_FREQ_RESERVED_MASK                                    0xFFFFF000
#define    PMU_RTC_RTC_FREQ_RTC_FREQOFFSET_11_0__SHIFT                    0
#define    PMU_RTC_RTC_FREQ_RTC_FREQOFFSET_11_0__MASK                     0x00000FFF

#define PMU_RTC_RTC_EXSAVE_OFFSET                                         0x00000024
#define PMU_RTC_RTC_EXSAVE_TYPE                                           UInt32
#define PMU_RTC_RTC_EXSAVE_RESERVED_MASK                                  0xFFFFFF00
#define    PMU_RTC_RTC_EXSAVE_RTC_EXSAVE_2_0__SHIFT                       4
#define    PMU_RTC_RTC_EXSAVE_RTC_EXSAVE_2_0__MASK                        0x000000F0
#define    PMU_RTC_RTC_EXSAVE_FREETSTCLK_SHIFT                            3
#define    PMU_RTC_RTC_EXSAVE_FREETSTCLK_MASK                             0x00000008
#define    PMU_RTC_RTC_EXSAVE_POR_GLITCH_SHIFT                            2
#define    PMU_RTC_RTC_EXSAVE_POR_GLITCH_MASK                             0x00000004
#define    PMU_RTC_RTC_EXSAVE_POR_ROSE_SHIFT                              1
#define    PMU_RTC_RTC_EXSAVE_POR_ROSE_MASK                               0x00000002
#define    PMU_RTC_RTC_EXSAVE_POR_FELL_SHIFT                              0
#define    PMU_RTC_RTC_EXSAVE_POR_FELL_MASK                               0x00000001

#define PMU_RTC_RTC_RTCALIB_OFFSET                                        0x00000028
#define PMU_RTC_RTC_RTCALIB_TYPE                                          UInt32
#define PMU_RTC_RTC_RTCALIB_RESERVED_MASK                                 0x3FE00000
#define    PMU_RTC_RTC_RTCALIB_RTC_CALIB_ENABLE_SHIFT                     31
#define    PMU_RTC_RTC_RTCALIB_RTC_CALIB_ENABLE_MASK                      0x80000000
#define    PMU_RTC_RTC_RTCALIB_RTC_CALIB_DIRECTION_SHIFT                  30
#define    PMU_RTC_RTC_RTCALIB_RTC_CALIB_DIRECTION_MASK                   0x40000000
#define    PMU_RTC_RTC_RTCALIB_RTC_CALIB_20_0__SHIFT                      0
#define    PMU_RTC_RTC_RTCALIB_RTC_CALIB_20_0__MASK                       0x001FFFFF

#define PMU_RTC_BBCCTRL_OFFSET                                            0x0000002C
#define PMU_RTC_BBCCTRL_TYPE                                              UInt32
#define PMU_RTC_BBCCTRL_RESERVED_MASK                                     0xFFFFFFE0
#define    PMU_RTC_BBCCTRL_BBCVS_1_0__SHIFT                               3
#define    PMU_RTC_BBCCTRL_BBCVS_1_0__MASK                                0x00000018
#define    PMU_RTC_BBCCTRL_BBCCS_1_0__SHIFT                               1
#define    PMU_RTC_BBCCTRL_BBCCS_1_0__MASK                                0x00000006
#define    PMU_RTC_BBCCTRL_BBCHOSTEN_SHIFT                                0
#define    PMU_RTC_BBCCTRL_BBCHOSTEN_MASK                                 0x00000001

#define PMU_RTC_SMPLSET_OFFSET                                            0x00000030
#define PMU_RTC_SMPLSET_TYPE                                              UInt32
#define PMU_RTC_SMPLSET_RESERVED_MASK                                     0xFFFFFFF0
#define    PMU_RTC_SMPLSET_SMPL_TIME_2_0__SHIFT                           1
#define    PMU_RTC_SMPLSET_SMPL_TIME_2_0__MASK                            0x0000000E
#define    PMU_RTC_SMPLSET_SMPL_ON_SHIFT                                  0
#define    PMU_RTC_SMPLSET_SMPL_ON_MASK                                   0x00000001

#define PMU_RTC_C2C1_TRIM_OFFSET                                          0x00000034
#define PMU_RTC_C2C1_TRIM_TYPE                                            UInt32
#define PMU_RTC_C2C1_TRIM_RESERVED_MASK                                   0xFFFFFF00
#define    PMU_RTC_C2C1_TRIM_CLK32K_C2_2_0__SHIFT                         4
#define    PMU_RTC_C2C1_TRIM_CLK32K_C2_2_0__MASK                          0x000000F0
#define    PMU_RTC_C2C1_TRIM_CLK32K_C1_3_0__SHIFT                         0
#define    PMU_RTC_C2C1_TRIM_CLK32K_C1_3_0__MASK                          0x0000000F

#define PMU_RTC_XTAL_PROTECT_OFFSET                                       0x00000038
#define PMU_RTC_XTAL_PROTECT_TYPE                                         UInt32
#define PMU_RTC_XTAL_PROTECT_RESERVED_MASK                                0xFFFFFFC0
#define    PMU_RTC_XTAL_PROTECT_SW_XTAL_FAIL_COUNT_1_0__SHIFT             4
#define    PMU_RTC_XTAL_PROTECT_SW_XTAL_FAIL_COUNT_1_0__MASK              0x00000030
#define    PMU_RTC_XTAL_PROTECT_XTAL_RETRY_CNT_1_0__SHIFT                 2
#define    PMU_RTC_XTAL_PROTECT_XTAL_RETRY_CNT_1_0__MASK                  0x0000000C
#define    PMU_RTC_XTAL_PROTECT_BYPASS_XTAL_DETECT_SHIFT                  1
#define    PMU_RTC_XTAL_PROTECT_BYPASS_XTAL_DETECT_MASK                   0x00000002
#define    PMU_RTC_XTAL_PROTECT_XTAL_PROTECTION_EN_SHIFT                  0
#define    PMU_RTC_XTAL_PROTECT_XTAL_PROTECTION_EN_MASK                   0x00000001

#define PMU_RTC_HOSTCTRL2_OFFSET                                          0x0000003C
#define PMU_RTC_HOSTCTRL2_TYPE                                            UInt32
#define PMU_RTC_HOSTCTRL2_RESERVED_MASK                                   0xFFFFFFFA
#define    PMU_RTC_HOSTCTRL2_AONCLK32_EN_RESERVED_SHIFT                   2
#define    PMU_RTC_HOSTCTRL2_AONCLK32_EN_RESERVED_MASK                    0x00000004
#define    PMU_RTC_HOSTCTRL2_SYS_DBS_SEL_SHIFT                            0
#define    PMU_RTC_HOSTCTRL2_SYS_DBS_SEL_MASK                             0x00000001

#endif /* __BRCM_RDB_PMU_RTC_H__ */



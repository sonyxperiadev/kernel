/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2013  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_GPTIMERS_H__
#define __BRCM_RDB_GPTIMERS_H__

#define GPTIMERS_GPTISR_OFFSET                                            0x00000000
#define GPTIMERS_GPTISR_TYPE                                              UInt32
#define GPTIMERS_GPTISR_RESERVED_MASK                                     0xFFC0C0C0
#define    GPTIMERS_GPTISR_GPT5_INT_ASSIGN_SHIFT                          21
#define    GPTIMERS_GPTISR_GPT5_INT_ASSIGN_MASK                           0x00200000
#define    GPTIMERS_GPTISR_GPT4_INT_ASSIGN_SHIFT                          20
#define    GPTIMERS_GPTISR_GPT4_INT_ASSIGN_MASK                           0x00100000
#define    GPTIMERS_GPTISR_GPT3_INT_ASSIGN_SHIFT                          19
#define    GPTIMERS_GPTISR_GPT3_INT_ASSIGN_MASK                           0x00080000
#define    GPTIMERS_GPTISR_GPT2_INT_ASSIGN_SHIFT                          18
#define    GPTIMERS_GPTISR_GPT2_INT_ASSIGN_MASK                           0x00040000
#define    GPTIMERS_GPTISR_GPT1_INT_ASSIGN_SHIFT                          17
#define    GPTIMERS_GPTISR_GPT1_INT_ASSIGN_MASK                           0x00020000
#define    GPTIMERS_GPTISR_GPT0_INT_ASSIGN_SHIFT                          16
#define    GPTIMERS_GPTISR_GPT0_INT_ASSIGN_MASK                           0x00010000
#define    GPTIMERS_GPTISR_GPT5_INT_EN_SHIFT                              13
#define    GPTIMERS_GPTISR_GPT5_INT_EN_MASK                               0x00002000
#define    GPTIMERS_GPTISR_GPT4_INT_EN_SHIFT                              12
#define    GPTIMERS_GPTISR_GPT4_INT_EN_MASK                               0x00001000
#define    GPTIMERS_GPTISR_GPT3_INT_EN_SHIFT                              11
#define    GPTIMERS_GPTISR_GPT3_INT_EN_MASK                               0x00000800
#define    GPTIMERS_GPTISR_GPT2_INT_EN_SHIFT                              10
#define    GPTIMERS_GPTISR_GPT2_INT_EN_MASK                               0x00000400
#define    GPTIMERS_GPTISR_GPT1_INT_EN_SHIFT                              9
#define    GPTIMERS_GPTISR_GPT1_INT_EN_MASK                               0x00000200
#define    GPTIMERS_GPTISR_GPT0_INT_EN_SHIFT                              8
#define    GPTIMERS_GPTISR_GPT0_INT_EN_MASK                               0x00000100
#define    GPTIMERS_GPTISR_GPT5_INT_FLAG_SHIFT                            5
#define    GPTIMERS_GPTISR_GPT5_INT_FLAG_MASK                             0x00000020
#define    GPTIMERS_GPTISR_GPT4_INT_FLAG_SHIFT                            4
#define    GPTIMERS_GPTISR_GPT4_INT_FLAG_MASK                             0x00000010
#define    GPTIMERS_GPTISR_GPT3_INT_FLAG_SHIFT                            3
#define    GPTIMERS_GPTISR_GPT3_INT_FLAG_MASK                             0x00000008
#define    GPTIMERS_GPTISR_GPT2_INT_FLAG_SHIFT                            2
#define    GPTIMERS_GPTISR_GPT2_INT_FLAG_MASK                             0x00000004
#define    GPTIMERS_GPTISR_GPT1_INT_FLAG_SHIFT                            1
#define    GPTIMERS_GPTISR_GPT1_INT_FLAG_MASK                             0x00000002
#define    GPTIMERS_GPTISR_GPT0_INT_FLAG_SHIFT                            0
#define    GPTIMERS_GPTISR_GPT0_INT_FLAG_MASK                             0x00000001

#define GPTIMERS_GPT0CSR_OFFSET                                           0x00000004
#define GPTIMERS_GPT0CSR_TYPE                                             UInt32
#define GPTIMERS_GPT0CSR_RESERVED_MASK                                    0x001FFCFE
#define    GPTIMERS_GPT0CSR_EN_SHIFT                                      31
#define    GPTIMERS_GPT0CSR_EN_MASK                                       0x80000000
#define    GPTIMERS_GPT0CSR_CLKSEL_SHIFT                                  30
#define    GPTIMERS_GPT0CSR_CLKSEL_MASK                                   0x40000000
#define    GPTIMERS_GPT0CSR_SW_SLPMODE_SHIFT                              29
#define    GPTIMERS_GPT0CSR_SW_SLPMODE_MASK                               0x20000000
#define    GPTIMERS_GPT0CSR_HW_SLEEP_MODE_DIS_SHIFT                       28
#define    GPTIMERS_GPT0CSR_HW_SLEEP_MODE_DIS_MASK                        0x10000000
#define    GPTIMERS_GPT0CSR_INT_EN_SHIFT                                  27
#define    GPTIMERS_GPT0CSR_INT_EN_MASK                                   0x08000000
#define    GPTIMERS_GPT0CSR_INT2_ASSIGN_SHIFT                             26
#define    GPTIMERS_GPT0CSR_INT2_ASSIGN_MASK                              0x04000000
#define    GPTIMERS_GPT0CSR_CLKSEL1_SHIFT                                 25
#define    GPTIMERS_GPT0CSR_CLKSEL1_MASK                                  0x02000000
#define    GPTIMERS_GPT0CSR_TIMER_PWRON_SHIFT                             24
#define    GPTIMERS_GPT0CSR_TIMER_PWRON_MASK                              0x01000000
#define    GPTIMERS_GPT0CSR_SW_PEDEMODE_SHIFT                             23
#define    GPTIMERS_GPT0CSR_SW_PEDEMODE_MASK                              0x00800000
#define    GPTIMERS_GPT0CSR_HW_PEDEMODE_DIS_SHIFT                         22
#define    GPTIMERS_GPT0CSR_HW_PEDEMODE_DIS_MASK                          0x00400000
#define    GPTIMERS_GPT0CSR_APB_READ_SEL_SHIFT                            21
#define    GPTIMERS_GPT0CSR_APB_READ_SEL_MASK                             0x00200000
#define    GPTIMERS_GPT0CSR_PEDESTAL_STATE_SHIFT                          9
#define    GPTIMERS_GPT0CSR_PEDESTAL_STATE_MASK                           0x00000200
#define    GPTIMERS_GPT0CSR_SLEEP_STATE_SHIFT                             8
#define    GPTIMERS_GPT0CSR_SLEEP_STATE_MASK                              0x00000100
#define    GPTIMERS_GPT0CSR_INT_FLAG_SHIFT                                0
#define    GPTIMERS_GPT0CSR_INT_FLAG_MASK                                 0x00000001

#define GPTIMERS_GPT0RR_OFFSET                                            0x00000008
#define GPTIMERS_GPT0RR_TYPE                                              UInt32
#define GPTIMERS_GPT0RR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT0RR_RLOAD_VALUE_SHIFT                              0
#define    GPTIMERS_GPT0RR_RLOAD_VALUE_MASK                               0xFFFFFFFF

#define GPTIMERS_GPT0VR_OFFSET                                            0x0000000C
#define GPTIMERS_GPT0VR_TYPE                                              UInt32
#define GPTIMERS_GPT0VR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT0VR_CNT_VALUE_SHIFT                                0
#define    GPTIMERS_GPT0VR_CNT_VALUE_MASK                                 0xFFFFFFFF

#define GPTIMERS_GPT1CSR_OFFSET                                           0x00000014
#define GPTIMERS_GPT1CSR_TYPE                                             UInt32
#define GPTIMERS_GPT1CSR_RESERVED_MASK                                    0x001FFCFE
#define    GPTIMERS_GPT1CSR_EN_SHIFT                                      31
#define    GPTIMERS_GPT1CSR_EN_MASK                                       0x80000000
#define    GPTIMERS_GPT1CSR_CLKSEL_SHIFT                                  30
#define    GPTIMERS_GPT1CSR_CLKSEL_MASK                                   0x40000000
#define    GPTIMERS_GPT1CSR_SW_SLPMODE_SHIFT                              29
#define    GPTIMERS_GPT1CSR_SW_SLPMODE_MASK                               0x20000000
#define    GPTIMERS_GPT1CSR_HW_SLEEP_MODE_DIS_SHIFT                       28
#define    GPTIMERS_GPT1CSR_HW_SLEEP_MODE_DIS_MASK                        0x10000000
#define    GPTIMERS_GPT1CSR_INT_EN_SHIFT                                  27
#define    GPTIMERS_GPT1CSR_INT_EN_MASK                                   0x08000000
#define    GPTIMERS_GPT1CSR_INT2_ASSIGN_SHIFT                             26
#define    GPTIMERS_GPT1CSR_INT2_ASSIGN_MASK                              0x04000000
#define    GPTIMERS_GPT1CSR_CLKSEL1_SHIFT                                 25
#define    GPTIMERS_GPT1CSR_CLKSEL1_MASK                                  0x02000000
#define    GPTIMERS_GPT1CSR_TIMER_PWRON_SHIFT                             24
#define    GPTIMERS_GPT1CSR_TIMER_PWRON_MASK                              0x01000000
#define    GPTIMERS_GPT1CSR_SW_PEDEMODE_SHIFT                             23
#define    GPTIMERS_GPT1CSR_SW_PEDEMODE_MASK                              0x00800000
#define    GPTIMERS_GPT1CSR_HW_PEDEMODE_DIS_SHIFT                         22
#define    GPTIMERS_GPT1CSR_HW_PEDEMODE_DIS_MASK                          0x00400000
#define    GPTIMERS_GPT1CSR_APB_READ_SEL_SHIFT                            21
#define    GPTIMERS_GPT1CSR_APB_READ_SEL_MASK                             0x00200000
#define    GPTIMERS_GPT1CSR_PEDESTAL_STATE_SHIFT                          9
#define    GPTIMERS_GPT1CSR_PEDESTAL_STATE_MASK                           0x00000200
#define    GPTIMERS_GPT1CSR_SLEEP_STATE_SHIFT                             8
#define    GPTIMERS_GPT1CSR_SLEEP_STATE_MASK                              0x00000100
#define    GPTIMERS_GPT1CSR_INT_FLAG_SHIFT                                0
#define    GPTIMERS_GPT1CSR_INT_FLAG_MASK                                 0x00000001

#define GPTIMERS_GPT1RR_OFFSET                                            0x00000018
#define GPTIMERS_GPT1RR_TYPE                                              UInt32
#define GPTIMERS_GPT1RR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT1RR_RLOAD_VALUE_SHIFT                              0
#define    GPTIMERS_GPT1RR_RLOAD_VALUE_MASK                               0xFFFFFFFF

#define GPTIMERS_GPT1VR_OFFSET                                            0x0000001C
#define GPTIMERS_GPT1VR_TYPE                                              UInt32
#define GPTIMERS_GPT1VR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT1VR_CNT_VALUE_SHIFT                                0
#define    GPTIMERS_GPT1VR_CNT_VALUE_MASK                                 0xFFFFFFFF

#define GPTIMERS_GPT2CSR_OFFSET                                           0x00000024
#define GPTIMERS_GPT2CSR_TYPE                                             UInt32
#define GPTIMERS_GPT2CSR_RESERVED_MASK                                    0x001FFCFE
#define    GPTIMERS_GPT2CSR_EN_SHIFT                                      31
#define    GPTIMERS_GPT2CSR_EN_MASK                                       0x80000000
#define    GPTIMERS_GPT2CSR_CLKSEL_SHIFT                                  30
#define    GPTIMERS_GPT2CSR_CLKSEL_MASK                                   0x40000000
#define    GPTIMERS_GPT2CSR_SW_SLPMODE_SHIFT                              29
#define    GPTIMERS_GPT2CSR_SW_SLPMODE_MASK                               0x20000000
#define    GPTIMERS_GPT2CSR_HW_SLEEP_MODE_DIS_SHIFT                       28
#define    GPTIMERS_GPT2CSR_HW_SLEEP_MODE_DIS_MASK                        0x10000000
#define    GPTIMERS_GPT2CSR_INT_EN_SHIFT                                  27
#define    GPTIMERS_GPT2CSR_INT_EN_MASK                                   0x08000000
#define    GPTIMERS_GPT2CSR_INT2_ASSIGN_SHIFT                             26
#define    GPTIMERS_GPT2CSR_INT2_ASSIGN_MASK                              0x04000000
#define    GPTIMERS_GPT2CSR_CLKSEL1_SHIFT                                 25
#define    GPTIMERS_GPT2CSR_CLKSEL1_MASK                                  0x02000000
#define    GPTIMERS_GPT2CSR_TIMER_PWRON_SHIFT                             24
#define    GPTIMERS_GPT2CSR_TIMER_PWRON_MASK                              0x01000000
#define    GPTIMERS_GPT2CSR_SW_PEDEMODE_SHIFT                             23
#define    GPTIMERS_GPT2CSR_SW_PEDEMODE_MASK                              0x00800000
#define    GPTIMERS_GPT2CSR_HW_PEDEMODE_DIS_SHIFT                         22
#define    GPTIMERS_GPT2CSR_HW_PEDEMODE_DIS_MASK                          0x00400000
#define    GPTIMERS_GPT2CSR_APB_READ_SEL_SHIFT                            21
#define    GPTIMERS_GPT2CSR_APB_READ_SEL_MASK                             0x00200000
#define    GPTIMERS_GPT2CSR_PEDESTAL_STATE_SHIFT                          9
#define    GPTIMERS_GPT2CSR_PEDESTAL_STATE_MASK                           0x00000200
#define    GPTIMERS_GPT2CSR_SLEEP_STATE_SHIFT                             8
#define    GPTIMERS_GPT2CSR_SLEEP_STATE_MASK                              0x00000100
#define    GPTIMERS_GPT2CSR_INT_FLAG_SHIFT                                0
#define    GPTIMERS_GPT2CSR_INT_FLAG_MASK                                 0x00000001

#define GPTIMERS_GPT2RR_OFFSET                                            0x00000028
#define GPTIMERS_GPT2RR_TYPE                                              UInt32
#define GPTIMERS_GPT2RR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT2RR_RLOAD_VALUE_SHIFT                              0
#define    GPTIMERS_GPT2RR_RLOAD_VALUE_MASK                               0xFFFFFFFF

#define GPTIMERS_GPT2VR_OFFSET                                            0x0000002C
#define GPTIMERS_GPT2VR_TYPE                                              UInt32
#define GPTIMERS_GPT2VR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT2VR_CNT_VALUE_SHIFT                                0
#define    GPTIMERS_GPT2VR_CNT_VALUE_MASK                                 0xFFFFFFFF

#define GPTIMERS_GPT3CSR_OFFSET                                           0x00000034
#define GPTIMERS_GPT3CSR_TYPE                                             UInt32
#define GPTIMERS_GPT3CSR_RESERVED_MASK                                    0x001FFCFE
#define    GPTIMERS_GPT3CSR_EN_SHIFT                                      31
#define    GPTIMERS_GPT3CSR_EN_MASK                                       0x80000000
#define    GPTIMERS_GPT3CSR_CLKSEL_SHIFT                                  30
#define    GPTIMERS_GPT3CSR_CLKSEL_MASK                                   0x40000000
#define    GPTIMERS_GPT3CSR_SW_SLPMODE_SHIFT                              29
#define    GPTIMERS_GPT3CSR_SW_SLPMODE_MASK                               0x20000000
#define    GPTIMERS_GPT3CSR_HW_SLEEP_MODE_DIS_SHIFT                       28
#define    GPTIMERS_GPT3CSR_HW_SLEEP_MODE_DIS_MASK                        0x10000000
#define    GPTIMERS_GPT3CSR_INT_EN_SHIFT                                  27
#define    GPTIMERS_GPT3CSR_INT_EN_MASK                                   0x08000000
#define    GPTIMERS_GPT3CSR_INT2_ASSIGN_SHIFT                             26
#define    GPTIMERS_GPT3CSR_INT2_ASSIGN_MASK                              0x04000000
#define    GPTIMERS_GPT3CSR_CLKSEL1_SHIFT                                 25
#define    GPTIMERS_GPT3CSR_CLKSEL1_MASK                                  0x02000000
#define    GPTIMERS_GPT3CSR_TIMER_PWRON_SHIFT                             24
#define    GPTIMERS_GPT3CSR_TIMER_PWRON_MASK                              0x01000000
#define    GPTIMERS_GPT3CSR_SW_PEDEMODE_SHIFT                             23
#define    GPTIMERS_GPT3CSR_SW_PEDEMODE_MASK                              0x00800000
#define    GPTIMERS_GPT3CSR_HW_PEDEMODE_DIS_SHIFT                         22
#define    GPTIMERS_GPT3CSR_HW_PEDEMODE_DIS_MASK                          0x00400000
#define    GPTIMERS_GPT3CSR_APB_READ_SEL_SHIFT                            21
#define    GPTIMERS_GPT3CSR_APB_READ_SEL_MASK                             0x00200000
#define    GPTIMERS_GPT3CSR_PEDESTAL_STATE_SHIFT                          9
#define    GPTIMERS_GPT3CSR_PEDESTAL_STATE_MASK                           0x00000200
#define    GPTIMERS_GPT3CSR_SLEEP_STATE_SHIFT                             8
#define    GPTIMERS_GPT3CSR_SLEEP_STATE_MASK                              0x00000100
#define    GPTIMERS_GPT3CSR_INT_FLAG_SHIFT                                0
#define    GPTIMERS_GPT3CSR_INT_FLAG_MASK                                 0x00000001

#define GPTIMERS_GPT3RR_OFFSET                                            0x00000038
#define GPTIMERS_GPT3RR_TYPE                                              UInt32
#define GPTIMERS_GPT3RR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT3RR_RLOAD_VALUE_SHIFT                              0
#define    GPTIMERS_GPT3RR_RLOAD_VALUE_MASK                               0xFFFFFFFF

#define GPTIMERS_GPT3VR_OFFSET                                            0x0000003C
#define GPTIMERS_GPT3VR_TYPE                                              UInt32
#define GPTIMERS_GPT3VR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT3VR_CNT_VALUE_SHIFT                                0
#define    GPTIMERS_GPT3VR_CNT_VALUE_MASK                                 0xFFFFFFFF

#define GPTIMERS_GPT4CSR_OFFSET                                           0x00000044
#define GPTIMERS_GPT4CSR_TYPE                                             UInt32
#define GPTIMERS_GPT4CSR_RESERVED_MASK                                    0x001FFCFE
#define    GPTIMERS_GPT4CSR_EN_SHIFT                                      31
#define    GPTIMERS_GPT4CSR_EN_MASK                                       0x80000000
#define    GPTIMERS_GPT4CSR_CLKSEL_SHIFT                                  30
#define    GPTIMERS_GPT4CSR_CLKSEL_MASK                                   0x40000000
#define    GPTIMERS_GPT4CSR_SW_SLPMODE_SHIFT                              29
#define    GPTIMERS_GPT4CSR_SW_SLPMODE_MASK                               0x20000000
#define    GPTIMERS_GPT4CSR_HW_SLEEP_MODE_DIS_SHIFT                       28
#define    GPTIMERS_GPT4CSR_HW_SLEEP_MODE_DIS_MASK                        0x10000000
#define    GPTIMERS_GPT4CSR_INT_EN_SHIFT                                  27
#define    GPTIMERS_GPT4CSR_INT_EN_MASK                                   0x08000000
#define    GPTIMERS_GPT4CSR_INT2_ASSIGN_SHIFT                             26
#define    GPTIMERS_GPT4CSR_INT2_ASSIGN_MASK                              0x04000000
#define    GPTIMERS_GPT4CSR_CLKSEL1_SHIFT                                 25
#define    GPTIMERS_GPT4CSR_CLKSEL1_MASK                                  0x02000000
#define    GPTIMERS_GPT4CSR_TIMER_PWRON_SHIFT                             24
#define    GPTIMERS_GPT4CSR_TIMER_PWRON_MASK                              0x01000000
#define    GPTIMERS_GPT4CSR_SW_PEDEMODE_SHIFT                             23
#define    GPTIMERS_GPT4CSR_SW_PEDEMODE_MASK                              0x00800000
#define    GPTIMERS_GPT4CSR_HW_PEDEMODE_DIS_SHIFT                         22
#define    GPTIMERS_GPT4CSR_HW_PEDEMODE_DIS_MASK                          0x00400000
#define    GPTIMERS_GPT4CSR_APB_READ_SEL_SHIFT                            21
#define    GPTIMERS_GPT4CSR_APB_READ_SEL_MASK                             0x00200000
#define    GPTIMERS_GPT4CSR_PEDESTAL_STATE_SHIFT                          9
#define    GPTIMERS_GPT4CSR_PEDESTAL_STATE_MASK                           0x00000200
#define    GPTIMERS_GPT4CSR_SLEEP_STATE_SHIFT                             8
#define    GPTIMERS_GPT4CSR_SLEEP_STATE_MASK                              0x00000100
#define    GPTIMERS_GPT4CSR_INT_FLAG_SHIFT                                0
#define    GPTIMERS_GPT4CSR_INT_FLAG_MASK                                 0x00000001

#define GPTIMERS_GPT4RR_OFFSET                                            0x00000048
#define GPTIMERS_GPT4RR_TYPE                                              UInt32
#define GPTIMERS_GPT4RR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT4RR_RLOAD_VALUE_SHIFT                              0
#define    GPTIMERS_GPT4RR_RLOAD_VALUE_MASK                               0xFFFFFFFF

#define GPTIMERS_GPT4VR_OFFSET                                            0x0000004C
#define GPTIMERS_GPT4VR_TYPE                                              UInt32
#define GPTIMERS_GPT4VR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT4VR_CNT_VALUE_SHIFT                                0
#define    GPTIMERS_GPT4VR_CNT_VALUE_MASK                                 0xFFFFFFFF

#define GPTIMERS_GPT5CSR_OFFSET                                           0x00000054
#define GPTIMERS_GPT5CSR_TYPE                                             UInt32
#define GPTIMERS_GPT5CSR_RESERVED_MASK                                    0x001FFCFE
#define    GPTIMERS_GPT5CSR_EN_SHIFT                                      31
#define    GPTIMERS_GPT5CSR_EN_MASK                                       0x80000000
#define    GPTIMERS_GPT5CSR_CLKSEL_SHIFT                                  30
#define    GPTIMERS_GPT5CSR_CLKSEL_MASK                                   0x40000000
#define    GPTIMERS_GPT5CSR_SW_SLPMODE_SHIFT                              29
#define    GPTIMERS_GPT5CSR_SW_SLPMODE_MASK                               0x20000000
#define    GPTIMERS_GPT5CSR_HW_SLEEP_MODE_DIS_SHIFT                       28
#define    GPTIMERS_GPT5CSR_HW_SLEEP_MODE_DIS_MASK                        0x10000000
#define    GPTIMERS_GPT5CSR_INT_EN_SHIFT                                  27
#define    GPTIMERS_GPT5CSR_INT_EN_MASK                                   0x08000000
#define    GPTIMERS_GPT5CSR_INT2_ASSIGN_SHIFT                             26
#define    GPTIMERS_GPT5CSR_INT2_ASSIGN_MASK                              0x04000000
#define    GPTIMERS_GPT5CSR_CLKSEL1_SHIFT                                 25
#define    GPTIMERS_GPT5CSR_CLKSEL1_MASK                                  0x02000000
#define    GPTIMERS_GPT5CSR_TIMER_PWRON_SHIFT                             24
#define    GPTIMERS_GPT5CSR_TIMER_PWRON_MASK                              0x01000000
#define    GPTIMERS_GPT5CSR_SW_PEDEMODE_SHIFT                             23
#define    GPTIMERS_GPT5CSR_SW_PEDEMODE_MASK                              0x00800000
#define    GPTIMERS_GPT5CSR_HW_PEDEMODE_DIS_SHIFT                         22
#define    GPTIMERS_GPT5CSR_HW_PEDEMODE_DIS_MASK                          0x00400000
#define    GPTIMERS_GPT5CSR_APB_READ_SEL_SHIFT                            21
#define    GPTIMERS_GPT5CSR_APB_READ_SEL_MASK                             0x00200000
#define    GPTIMERS_GPT5CSR_PEDESTAL_STATE_SHIFT                          9
#define    GPTIMERS_GPT5CSR_PEDESTAL_STATE_MASK                           0x00000200
#define    GPTIMERS_GPT5CSR_SLEEP_STATE_SHIFT                             8
#define    GPTIMERS_GPT5CSR_SLEEP_STATE_MASK                              0x00000100
#define    GPTIMERS_GPT5CSR_INT_FLAG_SHIFT                                0
#define    GPTIMERS_GPT5CSR_INT_FLAG_MASK                                 0x00000001

#define GPTIMERS_GPT5RR_OFFSET                                            0x00000058
#define GPTIMERS_GPT5RR_TYPE                                              UInt32
#define GPTIMERS_GPT5RR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT5RR_RLOAD_VALUE_SHIFT                              0
#define    GPTIMERS_GPT5RR_RLOAD_VALUE_MASK                               0xFFFFFFFF

#define GPTIMERS_GPT5VR_OFFSET                                            0x0000005C
#define GPTIMERS_GPT5VR_TYPE                                              UInt32
#define GPTIMERS_GPT5VR_RESERVED_MASK                                     0x00000000
#define    GPTIMERS_GPT5VR_CNT_VALUE_SHIFT                                0
#define    GPTIMERS_GPT5VR_CNT_VALUE_MASK                                 0xFFFFFFFF

#define GPTIMERS_GPTBMDM_CTL_OFFSET                                       0x00000060
#define GPTIMERS_GPTBMDM_CTL_TYPE                                         UInt32
#define GPTIMERS_GPTBMDM_CTL_RESERVED_MASK                                0xFFFFFFFC
#define    GPTIMERS_GPTBMDM_CTL_BMDM_GO_ENABLE_SHIFT                      1
#define    GPTIMERS_GPTBMDM_CTL_BMDM_GO_ENABLE_MASK                       0x00000002
#define    GPTIMERS_GPTBMDM_CTL_BMDM_GO_SHIFT                             0
#define    GPTIMERS_GPTBMDM_CTL_BMDM_GO_MASK                              0x00000001

#endif /* __BRCM_RDB_GPTIMERS_H__ */



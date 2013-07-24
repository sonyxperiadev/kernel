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

#ifndef __BRCM_RDB_PWM_TOP_H__
#define __BRCM_RDB_PWM_TOP_H__

#define PWM_TOP_PWM_CONTROL_OFFSET                                        0x00000000
#define PWM_TOP_PWM_CONTROL_TYPE                                          UInt32
#define PWM_TOP_PWM_CONTROL_RESERVED_MASK                                 0xC0C0C0C0
#define    PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_SHIFT                          24
#define    PWM_TOP_PWM_CONTROL_SMOOTH_TYPE_MASK                           0x3F000000
#define    PWM_TOP_PWM_CONTROL_PWMOUT_TYPE_SHIFT                          16
#define    PWM_TOP_PWM_CONTROL_PWMOUT_TYPE_MASK                           0x003F0000
#define    PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_SHIFT                      8
#define    PWM_TOP_PWM_CONTROL_PWMOUT_POLARITY_MASK                       0x00003F00
#define    PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_SHIFT                        0
#define    PWM_TOP_PWM_CONTROL_PWMOUT_ENABLE_MASK                         0x0000003F

#define PWM_TOP_PRESCALE_CONTROL_OFFSET                                   0x00000004
#define PWM_TOP_PRESCALE_CONTROL_TYPE                                     UInt32
#define PWM_TOP_PRESCALE_CONTROL_RESERVED_MASK                            0xFF888888
#define    PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_SHIFT                   20
#define    PWM_TOP_PRESCALE_CONTROL_PWM5_PRESCALE_MASK                    0x00700000
#define    PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_SHIFT                   16
#define    PWM_TOP_PRESCALE_CONTROL_PWM4_PRESCALE_MASK                    0x00070000
#define    PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_SHIFT                   12
#define    PWM_TOP_PRESCALE_CONTROL_PWM3_PRESCALE_MASK                    0x00007000
#define    PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_SHIFT                   8
#define    PWM_TOP_PRESCALE_CONTROL_PWM2_PRESCALE_MASK                    0x00000700
#define    PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_SHIFT                   4
#define    PWM_TOP_PRESCALE_CONTROL_PWM1_PRESCALE_MASK                    0x00000070
#define    PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_SHIFT                   0
#define    PWM_TOP_PRESCALE_CONTROL_PWM0_PRESCALE_MASK                    0x00000007

#define PWM_TOP_PWM0_PERIOD_COUNT_OFFSET                                  0x00000008
#define PWM_TOP_PWM0_PERIOD_COUNT_TYPE                                    UInt32
#define PWM_TOP_PWM0_PERIOD_COUNT_RESERVED_MASK                           0xFF000000
#define    PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_SHIFT                       0
#define    PWM_TOP_PWM0_PERIOD_COUNT_PWM0_CNT_MASK                        0x00FFFFFF

#define PWM_TOP_PWM0_DUTY_CYCLE_HIGH_OFFSET                               0x0000000C
#define PWM_TOP_PWM0_DUTY_CYCLE_HIGH_TYPE                                 UInt32
#define PWM_TOP_PWM0_DUTY_CYCLE_HIGH_RESERVED_MASK                        0xFF000000
#define    PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_SHIFT                   0
#define    PWM_TOP_PWM0_DUTY_CYCLE_HIGH_PWM0_HIGH_MASK                    0x00FFFFFF

#define PWM_TOP_PWM1_PERIOD_COUNT_OFFSET                                  0x00000010
#define PWM_TOP_PWM1_PERIOD_COUNT_TYPE                                    UInt32
#define PWM_TOP_PWM1_PERIOD_COUNT_RESERVED_MASK                           0xFF000000
#define    PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_SHIFT                       0
#define    PWM_TOP_PWM1_PERIOD_COUNT_PWM1_CNT_MASK                        0x00FFFFFF

#define PWM_TOP_PWM1_DUTY_CYCLE_HIGH_OFFSET                               0x00000014
#define PWM_TOP_PWM1_DUTY_CYCLE_HIGH_TYPE                                 UInt32
#define PWM_TOP_PWM1_DUTY_CYCLE_HIGH_RESERVED_MASK                        0xFF000000
#define    PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_SHIFT                   0
#define    PWM_TOP_PWM1_DUTY_CYCLE_HIGH_PWM1_HIGH_MASK                    0x00FFFFFF

#define PWM_TOP_PWM2_PERIOD_COUNT_OFFSET                                  0x00000018
#define PWM_TOP_PWM2_PERIOD_COUNT_TYPE                                    UInt32
#define PWM_TOP_PWM2_PERIOD_COUNT_RESERVED_MASK                           0xFF000000
#define    PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_SHIFT                       0
#define    PWM_TOP_PWM2_PERIOD_COUNT_PWM2_CNT_MASK                        0x00FFFFFF

#define PWM_TOP_PWM2_DUTY_CYCLE_HIGH_OFFSET                               0x0000001C
#define PWM_TOP_PWM2_DUTY_CYCLE_HIGH_TYPE                                 UInt32
#define PWM_TOP_PWM2_DUTY_CYCLE_HIGH_RESERVED_MASK                        0xFF000000
#define    PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_SHIFT                   0
#define    PWM_TOP_PWM2_DUTY_CYCLE_HIGH_PWM2_HIGH_MASK                    0x00FFFFFF

#define PWM_TOP_PWM3_PERIOD_COUNT_OFFSET                                  0x00000020
#define PWM_TOP_PWM3_PERIOD_COUNT_TYPE                                    UInt32
#define PWM_TOP_PWM3_PERIOD_COUNT_RESERVED_MASK                           0xFF000000
#define    PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_SHIFT                       0
#define    PWM_TOP_PWM3_PERIOD_COUNT_PWM3_CNT_MASK                        0x00FFFFFF

#define PWM_TOP_PWM3_DUTY_CYCLE_HIGH_OFFSET                               0x00000024
#define PWM_TOP_PWM3_DUTY_CYCLE_HIGH_TYPE                                 UInt32
#define PWM_TOP_PWM3_DUTY_CYCLE_HIGH_RESERVED_MASK                        0xFF000000
#define    PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_SHIFT                   0
#define    PWM_TOP_PWM3_DUTY_CYCLE_HIGH_PWM3_HIGH_MASK                    0x00FFFFFF

#define PWM_TOP_PWM4_PERIOD_COUNT_OFFSET                                  0x00000028
#define PWM_TOP_PWM4_PERIOD_COUNT_TYPE                                    UInt32
#define PWM_TOP_PWM4_PERIOD_COUNT_RESERVED_MASK                           0xFF000000
#define    PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_SHIFT                       0
#define    PWM_TOP_PWM4_PERIOD_COUNT_PWM4_CNT_MASK                        0x00FFFFFF

#define PWM_TOP_PWM4_DUTY_CYCLE_HIGH_OFFSET                               0x0000002C
#define PWM_TOP_PWM4_DUTY_CYCLE_HIGH_TYPE                                 UInt32
#define PWM_TOP_PWM4_DUTY_CYCLE_HIGH_RESERVED_MASK                        0xFF000000
#define    PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_SHIFT                   0
#define    PWM_TOP_PWM4_DUTY_CYCLE_HIGH_PWM4_HIGH_MASK                    0x00FFFFFF

#define PWM_TOP_PWM5_PERIOD_COUNT_OFFSET                                  0x00000030
#define PWM_TOP_PWM5_PERIOD_COUNT_TYPE                                    UInt32
#define PWM_TOP_PWM5_PERIOD_COUNT_RESERVED_MASK                           0xFF000000
#define    PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_SHIFT                       0
#define    PWM_TOP_PWM5_PERIOD_COUNT_PWM5_CNT_MASK                        0x00FFFFFF

#define PWM_TOP_PWM5_DUTY_CYCLE_HIGH_OFFSET                               0x00000034
#define PWM_TOP_PWM5_DUTY_CYCLE_HIGH_TYPE                                 UInt32
#define PWM_TOP_PWM5_DUTY_CYCLE_HIGH_RESERVED_MASK                        0xFF000000
#define    PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_SHIFT                   0
#define    PWM_TOP_PWM5_DUTY_CYCLE_HIGH_PWM5_HIGH_MASK                    0x00FFFFFF

#define PWM_TOP_PWM_VIBRA_OFFSET                                          0x00000080
#define PWM_TOP_PWM_VIBRA_TYPE                                            UInt32
#define PWM_TOP_PWM_VIBRA_RESERVED_MASK                                   0xFFFF88F8
#define    PWM_TOP_PWM_VIBRA_VFM_INV_PWM5_PWM4_SHIFT                      14
#define    PWM_TOP_PWM_VIBRA_VFM_INV_PWM5_PWM4_MASK                       0x00004000
#define    PWM_TOP_PWM_VIBRA_VFM_INV_PWM3_PWM2_SHIFT                      13
#define    PWM_TOP_PWM_VIBRA_VFM_INV_PWM3_PWM2_MASK                       0x00002000
#define    PWM_TOP_PWM_VIBRA_VFM_INV_PWM1_PWM0_SHIFT                      12
#define    PWM_TOP_PWM_VIBRA_VFM_INV_PWM1_PWM0_MASK                       0x00001000
#define    PWM_TOP_PWM_VIBRA_VFM_PWM5_PWM4_SHIFT                          10
#define    PWM_TOP_PWM_VIBRA_VFM_PWM5_PWM4_MASK                           0x00000400
#define    PWM_TOP_PWM_VIBRA_VFM_PWM3_PWM2_SHIFT                          9
#define    PWM_TOP_PWM_VIBRA_VFM_PWM3_PWM2_MASK                           0x00000200
#define    PWM_TOP_PWM_VIBRA_VFM_PWM1_PWM0_SHIFT                          8
#define    PWM_TOP_PWM_VIBRA_VFM_PWM1_PWM0_MASK                           0x00000100
#define    PWM_TOP_PWM_VIBRA_VIBRA_PWM5_PWM4_SHIFT                        2
#define    PWM_TOP_PWM_VIBRA_VIBRA_PWM5_PWM4_MASK                         0x00000004
#define    PWM_TOP_PWM_VIBRA_VIBRA_PWM3_PWM2_SHIFT                        1
#define    PWM_TOP_PWM_VIBRA_VIBRA_PWM3_PWM2_MASK                         0x00000002
#define    PWM_TOP_PWM_VIBRA_VIBRA_PWM1_PWM0_SHIFT                        0
#define    PWM_TOP_PWM_VIBRA_VIBRA_PWM1_PWM0_MASK                         0x00000001

#define PWM_TOP_PWM_DMA_CONTROL_OFFSET                                    0x00000084
#define PWM_TOP_PWM_DMA_CONTROL_TYPE                                      UInt32
#define PWM_TOP_PWM_DMA_CONTROL_RESERVED_MASK                             0xFFFFFFC0
#define    PWM_TOP_PWM_DMA_CONTROL_PWM_DMA_EN_SHIFT                       0
#define    PWM_TOP_PWM_DMA_CONTROL_PWM_DMA_EN_MASK                        0x0000003F

#define PWM_TOP_PWM_DMA_INTERVAL_CTL0_OFFSET                              0x00000088
#define PWM_TOP_PWM_DMA_INTERVAL_CTL0_TYPE                                UInt32
#define PWM_TOP_PWM_DMA_INTERVAL_CTL0_RESERVED_MASK                       0x00000000
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM3_DMA_INTERVAL_SHIFT          24
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM3_DMA_INTERVAL_MASK           0xFF000000
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM2_DMA_INTERVAL_SHIFT          16
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM2_DMA_INTERVAL_MASK           0x00FF0000
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM1_DMA_INTERVAL_SHIFT          8
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM1_DMA_INTERVAL_MASK           0x0000FF00
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM0_DMA_INTERVAL_SHIFT          0
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL0_PWM0_DMA_INTERVAL_MASK           0x000000FF

#define PWM_TOP_PWM_DMA_INTERVAL_CTL1_OFFSET                              0x0000008C
#define PWM_TOP_PWM_DMA_INTERVAL_CTL1_TYPE                                UInt32
#define PWM_TOP_PWM_DMA_INTERVAL_CTL1_RESERVED_MASK                       0xFFFF0000
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL1_PWM5_DMA_INTERVAL_SHIFT          8
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL1_PWM5_DMA_INTERVAL_MASK           0x0000FF00
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL1_PWM4_DMA_INTERVAL_SHIFT          0
#define    PWM_TOP_PWM_DMA_INTERVAL_CTL1_PWM4_DMA_INTERVAL_MASK           0x000000FF

#endif /* __BRCM_RDB_PWM_TOP_H__ */



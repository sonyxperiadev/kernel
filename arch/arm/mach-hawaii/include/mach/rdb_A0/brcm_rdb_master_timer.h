/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2012  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_MASTER_TIMER_H__
#define __BRCM_RDB_MASTER_TIMER_H__

#define MASTER_TIMER_TX_SLOT_START_SAMP_R1_OFFSET                         0x00000000
#define MASTER_TIMER_TX_SLOT_START_SAMP_R1_TYPE                           UInt32
#define MASTER_TIMER_TX_SLOT_START_SAMP_R1_RESERVED_MASK                  0xFFFFC000
#define    MASTER_TIMER_TX_SLOT_START_SAMP_R1_TX_SLOT_START_SAMP_R1_SHIFT 0
#define    MASTER_TIMER_TX_SLOT_START_SAMP_R1_TX_SLOT_START_SAMP_R1_MASK  0x00003FFF

#define MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_OFFSET                           0x00000004
#define MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TYPE                             UInt32
#define MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_RESERVED_MASK                    0x00004000
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_FRAME_SHIFT       20
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_FRAME_MASK        0xFFF00000
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_SLOT_SHIFT        16
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_SLOT_MASK         0x000F0000
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_MODE_SHIFT        15
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_MODE_MASK         0x00008000
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_SAMP_R1_SHIFT     0
#define    MASTER_TIMER_TX_PWR_CTRL_SAMP_R1_TX_PWR_CTRL_SAMP_R1_MASK      0x00003FFF

#define MASTER_TIMER_RX_SLOT_START_SAMP_R1_OFFSET                         0x00000008
#define MASTER_TIMER_RX_SLOT_START_SAMP_R1_TYPE                           UInt32
#define MASTER_TIMER_RX_SLOT_START_SAMP_R1_RESERVED_MASK                  0xFFFFC000
#define    MASTER_TIMER_RX_SLOT_START_SAMP_R1_RX_SLOT_START_SAMP_R1_SHIFT 0
#define    MASTER_TIMER_RX_SLOT_START_SAMP_R1_RX_SLOT_START_SAMP_R1_MASK  0x00003FFF

#define MASTER_TIMER_GP_INT1_OFFSET                                       0x0000000C
#define MASTER_TIMER_GP_INT1_TYPE                                         UInt32
#define MASTER_TIMER_GP_INT1_RESERVED_MASK                                0x00004000
#define    MASTER_TIMER_GP_INT1_GP_INT1_FRAME_SHIFT                       20
#define    MASTER_TIMER_GP_INT1_GP_INT1_FRAME_MASK                        0xFFF00000
#define    MASTER_TIMER_GP_INT1_GP_INT1_SLOT_SHIFT                        16
#define    MASTER_TIMER_GP_INT1_GP_INT1_SLOT_MASK                         0x000F0000
#define    MASTER_TIMER_GP_INT1_GP_INT1_MODE_SHIFT                        15
#define    MASTER_TIMER_GP_INT1_GP_INT1_MODE_MASK                         0x00008000
#define    MASTER_TIMER_GP_INT1_GP_INT1_SAMP_R1_SHIFT                     0
#define    MASTER_TIMER_GP_INT1_GP_INT1_SAMP_R1_MASK                      0x00003FFF

#define MASTER_TIMER_GP_INT2_OFFSET                                       0x00000010
#define MASTER_TIMER_GP_INT2_TYPE                                         UInt32
#define MASTER_TIMER_GP_INT2_RESERVED_MASK                                0x00004000
#define    MASTER_TIMER_GP_INT2_GP_INT2_FRAME_SHIFT                       20
#define    MASTER_TIMER_GP_INT2_GP_INT2_FRAME_MASK                        0xFFF00000
#define    MASTER_TIMER_GP_INT2_GP_INT2_SLOT_SHIFT                        16
#define    MASTER_TIMER_GP_INT2_GP_INT2_SLOT_MASK                         0x000F0000
#define    MASTER_TIMER_GP_INT2_GP_INT2_MODE_SHIFT                        15
#define    MASTER_TIMER_GP_INT2_GP_INT2_MODE_MASK                         0x00008000
#define    MASTER_TIMER_GP_INT2_GP_INT2_SAMP_R1_SHIFT                     0
#define    MASTER_TIMER_GP_INT2_GP_INT2_SAMP_R1_MASK                      0x00003FFF

#define MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_OFFSET                           0x00000014
#define MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_TYPE                             UInt32
#define MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_RESERVED_MASK                    0xFFFEC000
#define    MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_MT_SLOT_START_RATE_SHIFT      16
#define    MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_MT_SLOT_START_RATE_MASK       0x00010000
#define    MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_RF_LOAD_CMD_SAMP_R1_SHIFT     0
#define    MASTER_TIMER_RF_LOAD_CMD_SAMP_R1_RF_LOAD_CMD_SAMP_R1_MASK      0x00003FFF

#define MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_OFFSET                         0x00000018
#define MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_TYPE                           UInt32
#define MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_RESERVED_MASK                  0x00004000
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_FRAME_SHIFT    20
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_FRAME_MASK     0xFFF00000
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_SLOT_SHIFT     16
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_SLOT_MASK      0x000F0000
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_MODE_SHIFT     15
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_MODE_MASK      0x00008000
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_SAMP_R1_SHIFT  0
#define    MASTER_TIMER_SEARCHER_SLOT_SAMP_R1_SEARCHER_INT_SAMP_R1_MASK   0x00003FFF

#define MASTER_TIMER_SLEEP_START_SLOT_R1_S1_OFFSET                        0x0000001C
#define MASTER_TIMER_SLEEP_START_SLOT_R1_S1_TYPE                          UInt32
#define MASTER_TIMER_SLEEP_START_SLOT_R1_S1_RESERVED_MASK                 0xC000FFE0
#define    MASTER_TIMER_SLEEP_START_SLOT_R1_S1_SLEEP_START_SAMPLE_R1_S1_SHIFT 16
#define    MASTER_TIMER_SLEEP_START_SLOT_R1_S1_SLEEP_START_SAMPLE_R1_S1_MASK 0x3FFF0000
#define    MASTER_TIMER_SLEEP_START_SLOT_R1_S1_SLEEP_START_SLOT_R1_S1_SHIFT 0
#define    MASTER_TIMER_SLEEP_START_SLOT_R1_S1_SLEEP_START_SLOT_R1_S1_MASK 0x0000001F

#define MASTER_TIMER_GP_DAC_SAMP_R1_OFFSET                                0x00000020
#define MASTER_TIMER_GP_DAC_SAMP_R1_TYPE                                  UInt32
#define MASTER_TIMER_GP_DAC_SAMP_R1_RESERVED_MASK                         0xC000C000
#define    MASTER_TIMER_GP_DAC_SAMP_R1_TWIF_SAMP_R1_SHIFT                 16
#define    MASTER_TIMER_GP_DAC_SAMP_R1_TWIF_SAMP_R1_MASK                  0x3FFF0000
#define    MASTER_TIMER_GP_DAC_SAMP_R1_GP_DAC_SAMP_R1_SHIFT               0
#define    MASTER_TIMER_GP_DAC_SAMP_R1_GP_DAC_SAMP_R1_MASK                0x00003FFF

#define MASTER_TIMER_CALIBRATION_MODE_OFFSET                              0x00000024
#define MASTER_TIMER_CALIBRATION_MODE_TYPE                                UInt32
#define MASTER_TIMER_CALIBRATION_MODE_RESERVED_MASK                       0xFFFFFFFE
#define    MASTER_TIMER_CALIBRATION_MODE_CALIBRATION_MODE_SHIFT           0
#define    MASTER_TIMER_CALIBRATION_MODE_CALIBRATION_MODE_MASK            0x00000001

#define MASTER_TIMER_OSC_PWD_SLOW_CMP_R1_OFFSET                           0x00000028
#define MASTER_TIMER_OSC_PWD_SLOW_CMP_R1_TYPE                             UInt32
#define MASTER_TIMER_OSC_PWD_SLOW_CMP_R1_RESERVED_MASK                    0xFFFC0000
#define    MASTER_TIMER_OSC_PWD_SLOW_CMP_R1_OSC_PWD_SLOW_CMP_R1_SHIFT     0
#define    MASTER_TIMER_OSC_PWD_SLOW_CMP_R1_OSC_PWD_SLOW_CMP_R1_MASK      0x0003FFFF

#define MASTER_TIMER_TX_HS_SLOT_START_SAMP_R1_OFFSET                      0x0000002C
#define MASTER_TIMER_TX_HS_SLOT_START_SAMP_R1_TYPE                        UInt32
#define MASTER_TIMER_TX_HS_SLOT_START_SAMP_R1_RESERVED_MASK               0xC000FFFF
#define    MASTER_TIMER_TX_HS_SLOT_START_SAMP_R1_TX_HS_SLOT_START_SAMP_R1_SHIFT 16
#define    MASTER_TIMER_TX_HS_SLOT_START_SAMP_R1_TX_HS_SLOT_START_SAMP_R1_MASK 0x3FFF0000

#define MASTER_TIMER_TX_HSDPA_INT_OFFSET                                  0x00000030
#define MASTER_TIMER_TX_HSDPA_INT_TYPE                                    UInt32
#define MASTER_TIMER_TX_HSDPA_INT_RESERVED_MASK                           0x00004000
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_FRAME_SHIFT             20
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_FRAME_MASK              0xFFF00000
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_SLOT_SHIFT              16
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_SLOT_MASK               0x000F0000
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_MODE_SHIFT              15
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_MODE_MASK               0x00008000
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_SAMP_R1_SHIFT           0
#define    MASTER_TIMER_TX_HSDPA_INT_TX_HSDPA_INT_SAMP_R1_MASK            0x00003FFF

#define MASTER_TIMER_FAST_SLEEP_PERIOD_R1_OFFSET                          0x00000034
#define MASTER_TIMER_FAST_SLEEP_PERIOD_R1_TYPE                            UInt32
#define MASTER_TIMER_FAST_SLEEP_PERIOD_R1_RESERVED_MASK                   0xFFFFFC00
#define    MASTER_TIMER_FAST_SLEEP_PERIOD_R1_FAST_SLEEP_PERIOD_R1_SHIFT   0
#define    MASTER_TIMER_FAST_SLEEP_PERIOD_R1_FAST_SLEEP_PERIOD_R1_MASK    0x000003FF

#define MASTER_TIMER_SLOW_SLEEP_PERIOD_R1_OFFSET                          0x00000038
#define MASTER_TIMER_SLOW_SLEEP_PERIOD_R1_TYPE                            UInt32
#define MASTER_TIMER_SLOW_SLEEP_PERIOD_R1_RESERVED_MASK                   0xFFFC0000
#define    MASTER_TIMER_SLOW_SLEEP_PERIOD_R1_SLOW_SLEEP_PERIOD_R1_SHIFT   0
#define    MASTER_TIMER_SLOW_SLEEP_PERIOD_R1_SLOW_SLEEP_PERIOD_R1_MASK    0x0003FFFF

#define MASTER_TIMER_ENABLE_SLEEP_ON_OFFSET                               0x0000003C
#define MASTER_TIMER_ENABLE_SLEEP_ON_TYPE                                 UInt32
#define MASTER_TIMER_ENABLE_SLEEP_ON_RESERVED_MASK                        0x1FFFEEED
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEP_STATE_DBG_SHIFT             29
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEP_STATE_DBG_MASK              0xE0000000
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEPTIMER3G_SFN_EN_SHIFT         12
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEPTIMER3G_SFN_EN_MASK          0x00001000
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEPTIMER3G_EN_SHIFT             8
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEPTIMER3G_EN_MASK              0x00000100
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEP_MODE_SHIFT                  4
#define    MASTER_TIMER_ENABLE_SLEEP_ON_SLEEP_MODE_MASK                   0x00000010
#define    MASTER_TIMER_ENABLE_SLEEP_ON_ENABLE_SLEEP_ON_SHIFT             1
#define    MASTER_TIMER_ENABLE_SLEEP_ON_ENABLE_SLEEP_ON_MASK              0x00000002

#define MASTER_TIMER_CNT_OFFSET                                           0x00000040
#define MASTER_TIMER_CNT_TYPE                                             UInt32
#define MASTER_TIMER_CNT_RESERVED_MASK                                    0x0000C000
#define    MASTER_TIMER_CNT_FRAME_CNT_SHIFT                               20
#define    MASTER_TIMER_CNT_FRAME_CNT_MASK                                0xFFF00000
#define    MASTER_TIMER_CNT_SLOT_CNT_RH_E_SHIFT                           16
#define    MASTER_TIMER_CNT_SLOT_CNT_RH_E_MASK                            0x000F0000
#define    MASTER_TIMER_CNT_SAMPLE_CNT_SHIFT                              0
#define    MASTER_TIMER_CNT_SAMPLE_CNT_MASK                               0x00003FFF

#define MASTER_TIMER_CNT_CAP_OFFSET                                       0x00000044
#define MASTER_TIMER_CNT_CAP_TYPE                                         UInt32
#define MASTER_TIMER_CNT_CAP_RESERVED_MASK                                0x0000C000
#define    MASTER_TIMER_CNT_CAP_FRAME_CNT_CAP_SHIFT                       20
#define    MASTER_TIMER_CNT_CAP_FRAME_CNT_CAP_MASK                        0xFFF00000
#define    MASTER_TIMER_CNT_CAP_SLOT_CNT_CAP_SHIFT                        16
#define    MASTER_TIMER_CNT_CAP_SLOT_CNT_CAP_MASK                         0x000F0000
#define    MASTER_TIMER_CNT_CAP_SAMPLE_CNT_CAP_SHIFT                      0
#define    MASTER_TIMER_CNT_CAP_SAMPLE_CNT_CAP_MASK                       0x00003FFF

#define MASTER_TIMER_CNT_LOAD_VAL_OFFSET                                  0x00000048
#define MASTER_TIMER_CNT_LOAD_VAL_TYPE                                    UInt32
#define MASTER_TIMER_CNT_LOAD_VAL_RESERVED_MASK                           0xC0000000
#define    MASTER_TIMER_CNT_LOAD_VAL_SAMPLE_CNT_LOAD_VAL_SHIFT            16
#define    MASTER_TIMER_CNT_LOAD_VAL_SAMPLE_CNT_LOAD_VAL_MASK             0x3FFF0000
#define    MASTER_TIMER_CNT_LOAD_VAL_FRAME_CNT_LOAD_VAL_SHIFT             4
#define    MASTER_TIMER_CNT_LOAD_VAL_FRAME_CNT_LOAD_VAL_MASK              0x0000FFF0
#define    MASTER_TIMER_CNT_LOAD_VAL_SLOT_CNT_LOAD_VAL_SHIFT              0
#define    MASTER_TIMER_CNT_LOAD_VAL_SLOT_CNT_LOAD_VAL_MASK               0x0000000F

#define MASTER_TIMER_SLEEP_CNT_OFFSET                                     0x0000004C
#define MASTER_TIMER_SLEEP_CNT_TYPE                                       UInt32
#define MASTER_TIMER_SLEEP_CNT_RESERVED_MASK                              0xC00C0000
#define    MASTER_TIMER_SLEEP_CNT_SLEEP_FAST_CNT_SHIFT                    20
#define    MASTER_TIMER_SLEEP_CNT_SLEEP_FAST_CNT_MASK                     0x3FF00000
#define    MASTER_TIMER_SLEEP_CNT_SLEEP_SLOW_CNT_SHIFT                    0
#define    MASTER_TIMER_SLEEP_CNT_SLEEP_SLOW_CNT_MASK                     0x0003FFFF

#define MASTER_TIMER_SFN_CTRL_OFFSET                                      0x00000050
#define MASTER_TIMER_SFN_CTRL_TYPE                                        UInt32
#define MASTER_TIMER_SFN_CTRL_RESERVED_MASK                               0x1FFFFEEE
#define    MASTER_TIMER_SFN_CTRL_SFN_STATE_DBG_SHIFT                      29
#define    MASTER_TIMER_SFN_CTRL_SFN_STATE_DBG_MASK                       0xE0000000
#define    MASTER_TIMER_SFN_CTRL_SFN_START_SHIFT                          8
#define    MASTER_TIMER_SFN_CTRL_SFN_START_MASK                           0x00000100
#define    MASTER_TIMER_SFN_CTRL_SFN_STOP_SHIFT                           4
#define    MASTER_TIMER_SFN_CTRL_SFN_STOP_MASK                            0x00000010
#define    MASTER_TIMER_SFN_CTRL_SFN_FREE_RUN_SHIFT                       0
#define    MASTER_TIMER_SFN_CTRL_SFN_FREE_RUN_MASK                        0x00000001

#define MASTER_TIMER_SFN_CNT_MAX_R1_OFFSET                                0x00000054
#define MASTER_TIMER_SFN_CNT_MAX_R1_TYPE                                  UInt32
#define MASTER_TIMER_SFN_CNT_MAX_R1_RESERVED_MASK                         0xFFFFF000
#define    MASTER_TIMER_SFN_CNT_MAX_R1_SFN_CNT_MAX_R1_SHIFT               0
#define    MASTER_TIMER_SFN_CNT_MAX_R1_SFN_CNT_MAX_R1_MASK                0x00000FFF

#define MASTER_TIMER_SFN_CNT_LOAD_OFFSET                                  0x00000058
#define MASTER_TIMER_SFN_CNT_LOAD_TYPE                                    UInt32
#define MASTER_TIMER_SFN_CNT_LOAD_RESERVED_MASK                           0xFFFFE000
#define    MASTER_TIMER_SFN_CNT_LOAD_SFN_CNT_LOAD_PULSE_SHIFT             12
#define    MASTER_TIMER_SFN_CNT_LOAD_SFN_CNT_LOAD_PULSE_MASK              0x00001000
#define    MASTER_TIMER_SFN_CNT_LOAD_SFN_CNT_LOAD_VAL_SHIFT               0
#define    MASTER_TIMER_SFN_CNT_LOAD_SFN_CNT_LOAD_VAL_MASK                0x00000FFF

#define MASTER_TIMER_SFN_ACC_INIT_VAL_OFFSET                              0x0000005C
#define MASTER_TIMER_SFN_ACC_INIT_VAL_TYPE                                UInt32
#define MASTER_TIMER_SFN_ACC_INIT_VAL_RESERVED_MASK                       0xFFFFFE00
#define    MASTER_TIMER_SFN_ACC_INIT_VAL_SFN_ACC_INIT_VAL_SHIFT           0
#define    MASTER_TIMER_SFN_ACC_INIT_VAL_SFN_ACC_INIT_VAL_MASK            0x000001FF

#define MASTER_TIMER_SFN_ACC_Y_R1_OFFSET                                  0x00000060
#define MASTER_TIMER_SFN_ACC_Y_R1_TYPE                                    UInt32
#define MASTER_TIMER_SFN_ACC_Y_R1_RESERVED_MASK                           0xFFFC0000
#define    MASTER_TIMER_SFN_ACC_Y_R1_SFN_ACC_Y_R1_SHIFT                   0
#define    MASTER_TIMER_SFN_ACC_Y_R1_SFN_ACC_Y_R1_MASK                    0x0003FFFF

#define MASTER_TIMER_RF_AWAKE_PULSE1_R1_OFFSET                            0x00000064
#define MASTER_TIMER_RF_AWAKE_PULSE1_R1_TYPE                              UInt32
#define MASTER_TIMER_RF_AWAKE_PULSE1_R1_RESERVED_MASK                     0xFFFFC000
#define    MASTER_TIMER_RF_AWAKE_PULSE1_R1_RF_AWAKE_PULSE1_SAMP_R1_SHIFT  0
#define    MASTER_TIMER_RF_AWAKE_PULSE1_R1_RF_AWAKE_PULSE1_SAMP_R1_MASK   0x00003FFF

#define MASTER_TIMER_RF_AWAKE_PULSE2_R1_OFFSET                            0x00000068
#define MASTER_TIMER_RF_AWAKE_PULSE2_R1_TYPE                              UInt32
#define MASTER_TIMER_RF_AWAKE_PULSE2_R1_RESERVED_MASK                     0xFFFFC000
#define    MASTER_TIMER_RF_AWAKE_PULSE2_R1_RF_AWAKE_PULSE2_SAMP_R1_SHIFT  0
#define    MASTER_TIMER_RF_AWAKE_PULSE2_R1_RF_AWAKE_PULSE2_SAMP_R1_MASK   0x00003FFF

#define MASTER_TIMER_RF_AWAKE_PULSE3_R1_OFFSET                            0x0000006C
#define MASTER_TIMER_RF_AWAKE_PULSE3_R1_TYPE                              UInt32
#define MASTER_TIMER_RF_AWAKE_PULSE3_R1_RESERVED_MASK                     0xFFF0C000
#define    MASTER_TIMER_RF_AWAKE_PULSE3_R1_RF_AWAKE_PULSE3_SLOT_R1_SHIFT  16
#define    MASTER_TIMER_RF_AWAKE_PULSE3_R1_RF_AWAKE_PULSE3_SLOT_R1_MASK   0x000F0000
#define    MASTER_TIMER_RF_AWAKE_PULSE3_R1_RF_AWAKE_PULSE3_SAMP_R1_SHIFT  0
#define    MASTER_TIMER_RF_AWAKE_PULSE3_R1_RF_AWAKE_PULSE3_SAMP_R1_MASK   0x00003FFF

#define MASTER_TIMER_RF_SLEEP1_SLOW_CMP_R1_OFFSET                         0x00000070
#define MASTER_TIMER_RF_SLEEP1_SLOW_CMP_R1_TYPE                           UInt32
#define MASTER_TIMER_RF_SLEEP1_SLOW_CMP_R1_RESERVED_MASK                  0xFFFC0000
#define    MASTER_TIMER_RF_SLEEP1_SLOW_CMP_R1_RF_SLEEP1_SLOW_CMP_R1_SHIFT 0
#define    MASTER_TIMER_RF_SLEEP1_SLOW_CMP_R1_RF_SLEEP1_SLOW_CMP_R1_MASK  0x0003FFFF

#define MASTER_TIMER_RF_SLEEP2_SLOW_CMP_R1_OFFSET                         0x00000074
#define MASTER_TIMER_RF_SLEEP2_SLOW_CMP_R1_TYPE                           UInt32
#define MASTER_TIMER_RF_SLEEP2_SLOW_CMP_R1_RESERVED_MASK                  0xFFFC0000
#define    MASTER_TIMER_RF_SLEEP2_SLOW_CMP_R1_RF_SLEEP2_SLOW_CMP_R1_SHIFT 0
#define    MASTER_TIMER_RF_SLEEP2_SLOW_CMP_R1_RF_SLEEP2_SLOW_CMP_R1_MASK  0x0003FFFF

#define MASTER_TIMER_RF_SLEEP3_SLOW_CMP_R1_OFFSET                         0x00000078
#define MASTER_TIMER_RF_SLEEP3_SLOW_CMP_R1_TYPE                           UInt32
#define MASTER_TIMER_RF_SLEEP3_SLOW_CMP_R1_RESERVED_MASK                  0xFFFC0000
#define    MASTER_TIMER_RF_SLEEP3_SLOW_CMP_R1_RF_SLEEP3_SLOW_CMP_R1_SHIFT 0
#define    MASTER_TIMER_RF_SLEEP3_SLOW_CMP_R1_RF_SLEEP3_SLOW_CMP_R1_MASK  0x0003FFFF

#define MASTER_TIMER_CL1_INT_OFFSET                                       0x0000007C
#define MASTER_TIMER_CL1_INT_TYPE                                         UInt32
#define MASTER_TIMER_CL1_INT_RESERVED_MASK                                0x00004000
#define    MASTER_TIMER_CL1_INT_CL1_INT_FRAME_SHIFT                       20
#define    MASTER_TIMER_CL1_INT_CL1_INT_FRAME_MASK                        0xFFF00000
#define    MASTER_TIMER_CL1_INT_CL1_INT_SLOT_SHIFT                        16
#define    MASTER_TIMER_CL1_INT_CL1_INT_SLOT_MASK                         0x000F0000
#define    MASTER_TIMER_CL1_INT_CL1_INT_MODE_SHIFT                        15
#define    MASTER_TIMER_CL1_INT_CL1_INT_MODE_MASK                         0x00008000
#define    MASTER_TIMER_CL1_INT_CL1_INT_SAMP_R1_SHIFT                     0
#define    MASTER_TIMER_CL1_INT_CL1_INT_SAMP_R1_MASK                      0x00003FFF

#endif /* __BRCM_RDB_MASTER_TIMER_H__ */



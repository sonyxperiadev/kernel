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

#ifndef __BRCM_RDB_FNG_CORE_TOP_H__
#define __BRCM_RDB_FNG_CORE_TOP_H__

#define FNG_CORE_TOP_CTRL_OFFSET                                          0x00000000
#define FNG_CORE_TOP_CTRL_TYPE                                            UInt32
#define FNG_CORE_TOP_CTRL_RESERVED_MASK                                   0x208EEECC
#define    FNG_CORE_TOP_CTRL_CHEST_IIR_CLEAR_SHIFT                        31
#define    FNG_CORE_TOP_CTRL_CHEST_IIR_CLEAR_MASK                         0x80000000
#define    FNG_CORE_TOP_CTRL_RSCP_IIR_CLEAR_SHIFT                         30
#define    FNG_CORE_TOP_CTRL_RSCP_IIR_CLEAR_MASK                          0x40000000
#define    FNG_CORE_TOP_CTRL_RSCP_PILOT_SEL_SHIFT                         28
#define    FNG_CORE_TOP_CTRL_RSCP_PILOT_SEL_MASK                          0x10000000
#define    FNG_CORE_TOP_CTRL_RSCP_IIR_COEF_RB_001_SHIFT                   25
#define    FNG_CORE_TOP_CTRL_RSCP_IIR_COEF_RB_001_MASK                    0x0E000000
#define    FNG_CORE_TOP_CTRL_CHEST_HOLD_SHIFT                             24
#define    FNG_CORE_TOP_CTRL_CHEST_HOLD_MASK                              0x01000000
#define    FNG_CORE_TOP_CTRL_CHEST_IIR_COEF_RB_001_SHIFT                  20
#define    FNG_CORE_TOP_CTRL_CHEST_IIR_COEF_RB_001_MASK                   0x00700000
#define    FNG_CORE_TOP_CTRL_SW_CLK_EN_SHIFT                              16
#define    FNG_CORE_TOP_CTRL_SW_CLK_EN_MASK                               0x00010000
#define    FNG_CORE_TOP_CTRL_SW_CPI_STTD_SHIFT                            12
#define    FNG_CORE_TOP_CTRL_SW_CPI_STTD_MASK                             0x00001000
#define    FNG_CORE_TOP_CTRL_TX_DIV_MODE_SHIFT                            8
#define    FNG_CORE_TOP_CTRL_TX_DIV_MODE_MASK                             0x00000100
#define    FNG_CORE_TOP_CTRL_IQ_MSTTIMERGATE_SHIFT                        5
#define    FNG_CORE_TOP_CTRL_IQ_MSTTIMERGATE_MASK                         0x00000020
#define    FNG_CORE_TOP_CTRL_SW_FNG_EN_SHIFT                              4
#define    FNG_CORE_TOP_CTRL_SW_FNG_EN_MASK                               0x00000010
#define    FNG_CORE_TOP_CTRL_FNG_AUTO_LOAD_SHIFT                          1
#define    FNG_CORE_TOP_CTRL_FNG_AUTO_LOAD_MASK                           0x00000002
#define    FNG_CORE_TOP_CTRL_FNG_PARA_LOAD_SHIFT                          0
#define    FNG_CORE_TOP_CTRL_FNG_PARA_LOAD_MASK                           0x00000001

#define FNG_CORE_TOP_CH0_LOAD_OFFSET                                      0x00000004
#define FNG_CORE_TOP_CH0_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH0_LOAD_RESERVED_MASK                               0xFFFFEEEE
#define    FNG_CORE_TOP_CH0_LOAD_CH0_DATA_TYPE_SHIFT                      12
#define    FNG_CORE_TOP_CH0_LOAD_CH0_DATA_TYPE_MASK                       0x00001000
#define    FNG_CORE_TOP_CH0_LOAD_CH0_WAKE_LOAD_SHIFT                      8
#define    FNG_CORE_TOP_CH0_LOAD_CH0_WAKE_LOAD_MASK                       0x00000100
#define    FNG_CORE_TOP_CH0_LOAD_CH0_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH0_LOAD_CH0_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH0_LOAD_CH0_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH0_LOAD_CH0_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_CH1_LOAD_OFFSET                                      0x00000008
#define FNG_CORE_TOP_CH1_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH1_LOAD_RESERVED_MASK                               0xFFFFECEE
#define    FNG_CORE_TOP_CH1_LOAD_CH1_DATA_TYPE_SHIFT                      12
#define    FNG_CORE_TOP_CH1_LOAD_CH1_DATA_TYPE_MASK                       0x00001000
#define    FNG_CORE_TOP_CH1_LOAD_CH1_WAKE_LOAD_SHIFT                      8
#define    FNG_CORE_TOP_CH1_LOAD_CH1_WAKE_LOAD_MASK                       0x00000300
#define    FNG_CORE_TOP_CH1_LOAD_CH1_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH1_LOAD_CH1_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH1_LOAD_CH1_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH1_LOAD_CH1_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_CH2_LOAD_OFFSET                                      0x0000000C
#define FNG_CORE_TOP_CH2_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH2_LOAD_RESERVED_MASK                               0xFFFFECEE
#define    FNG_CORE_TOP_CH2_LOAD_CH2_DATA_TYPE_SHIFT                      12
#define    FNG_CORE_TOP_CH2_LOAD_CH2_DATA_TYPE_MASK                       0x00001000
#define    FNG_CORE_TOP_CH2_LOAD_CH2_WAKE_LOAD_SHIFT                      8
#define    FNG_CORE_TOP_CH2_LOAD_CH2_WAKE_LOAD_MASK                       0x00000300
#define    FNG_CORE_TOP_CH2_LOAD_CH2_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH2_LOAD_CH2_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH2_LOAD_CH2_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH2_LOAD_CH2_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_CURRENT_POS_OFFSET                                   0x00000010
#define FNG_CORE_TOP_CURRENT_POS_TYPE                                     UInt32
#define FNG_CORE_TOP_CURRENT_POS_RESERVED_MASK                            0x6EF08000
#define    FNG_CORE_TOP_CURRENT_POS_PENDING_ADJUST_SHIFT                  31
#define    FNG_CORE_TOP_CURRENT_POS_PENDING_ADJUST_MASK                   0x80000000
#define    FNG_CORE_TOP_CURRENT_POS_CLEAR_ERROR_FLAG_SHIFT                28
#define    FNG_CORE_TOP_CURRENT_POS_CLEAR_ERROR_FLAG_MASK                 0x10000000
#define    FNG_CORE_TOP_CURRENT_POS_ERROR_FLAG_SHIFT                      24
#define    FNG_CORE_TOP_CURRENT_POS_ERROR_FLAG_MASK                       0x01000000
#define    FNG_CORE_TOP_CURRENT_POS_CURRENT_SLOTPOS_SHIFT                 16
#define    FNG_CORE_TOP_CURRENT_POS_CURRENT_SLOTPOS_MASK                  0x000F0000
#define    FNG_CORE_TOP_CURRENT_POS_CURRENT_SAMPLEPOS_SHIFT               0
#define    FNG_CORE_TOP_CURRENT_POS_CURRENT_SAMPLEPOS_MASK                0x00007FFF

#define FNG_CORE_TOP_WAKEUP_PHASE_OFFSET                                  0x00000014
#define FNG_CORE_TOP_WAKEUP_PHASE_TYPE                                    UInt32
#define FNG_CORE_TOP_WAKEUP_PHASE_RESERVED_MASK                           0xFFFF0000
#define    FNG_CORE_TOP_WAKEUP_PHASE_WAKEUP_PHASE_SHIFT                   0
#define    FNG_CORE_TOP_WAKEUP_PHASE_WAKEUP_PHASE_MASK                    0x0000FFFF

#define FNG_CORE_TOP_WAKEUP_POS_OFFSET                                    0x00000018
#define FNG_CORE_TOP_WAKEUP_POS_TYPE                                      UInt32
#define FNG_CORE_TOP_WAKEUP_POS_RESERVED_MASK                             0xFFF0C000
#define    FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SLOTPOS_SHIFT                   16
#define    FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SLOTPOS_MASK                    0x000F0000
#define    FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SAMPLEPOS_SHIFT                 0
#define    FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SAMPLEPOS_MASK                  0x00003FFF

#define FNG_CORE_TOP_PPI_CTRL_OFFSET                                      0x0000001C
#define FNG_CORE_TOP_PPI_CTRL_TYPE                                        UInt32
#define FNG_CORE_TOP_PPI_CTRL_RESERVED_MASK                               0x0000000E
#define    FNG_CORE_TOP_PPI_CTRL_ANT2_PPI_RSCP_SHIFT                      24
#define    FNG_CORE_TOP_PPI_CTRL_ANT2_PPI_RSCP_MASK                       0xFF000000
#define    FNG_CORE_TOP_PPI_CTRL_ANT1_PPI_RSCP_SHIFT                      16
#define    FNG_CORE_TOP_PPI_CTRL_ANT1_PPI_RSCP_MASK                       0x00FF0000
#define    FNG_CORE_TOP_PPI_CTRL_PPI_OVSF_SEED_SHIFT                      8
#define    FNG_CORE_TOP_PPI_CTRL_PPI_OVSF_SEED_MASK                       0x0000FF00
#define    FNG_CORE_TOP_PPI_CTRL_PPI_ACCLEN_SHIFT                         4
#define    FNG_CORE_TOP_PPI_CTRL_PPI_ACCLEN_MASK                          0x000000F0
#define    FNG_CORE_TOP_PPI_CTRL_SW_PPI_EN_SHIFT                          0
#define    FNG_CORE_TOP_PPI_CTRL_SW_PPI_EN_MASK                           0x00000001

#define FNG_CORE_TOP_PPI_WAKE_X_SEED_OFFSET                               0x00000020
#define FNG_CORE_TOP_PPI_WAKE_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_PPI_WAKE_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_PPI_WAKE_X_SEED_PPI_WAKE_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_PPI_WAKE_X_SEED_PPI_WAKE_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_PPI_WAKE_Y_SEED_OFFSET                               0x00000024
#define FNG_CORE_TOP_PPI_WAKE_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_PPI_WAKE_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_PPI_WAKE_Y_SEED_PPI_WAKE_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_PPI_WAKE_Y_SEED_PPI_WAKE_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_PPI_INIT_X_SEED_OFFSET                               0x00000028
#define FNG_CORE_TOP_PPI_INIT_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_PPI_INIT_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_PPI_INIT_X_SEED_PPI_INIT_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_PPI_INIT_X_SEED_PPI_INIT_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_PPI_INIT_Y_SEED_OFFSET                               0x0000002C
#define FNG_CORE_TOP_PPI_INIT_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_PPI_INIT_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_PPI_INIT_Y_SEED_PPI_INIT_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_PPI_INIT_Y_SEED_PPI_INIT_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_SPI_CTRL_OFFSET                                      0x00000034
#define FNG_CORE_TOP_SPI_CTRL_TYPE                                        UInt32
#define FNG_CORE_TOP_SPI_CTRL_RESERVED_MASK                               0x0000000E
#define    FNG_CORE_TOP_SPI_CTRL_ANT2_SPI_RSCP_SHIFT                      24
#define    FNG_CORE_TOP_SPI_CTRL_ANT2_SPI_RSCP_MASK                       0xFF000000
#define    FNG_CORE_TOP_SPI_CTRL_ANT1_SPI_RSCP_SHIFT                      16
#define    FNG_CORE_TOP_SPI_CTRL_ANT1_SPI_RSCP_MASK                       0x00FF0000
#define    FNG_CORE_TOP_SPI_CTRL_SPI_OVSF_SEED_SHIFT                      8
#define    FNG_CORE_TOP_SPI_CTRL_SPI_OVSF_SEED_MASK                       0x0000FF00
#define    FNG_CORE_TOP_SPI_CTRL_SPI_ACCLEN_SHIFT                         4
#define    FNG_CORE_TOP_SPI_CTRL_SPI_ACCLEN_MASK                          0x000000F0
#define    FNG_CORE_TOP_SPI_CTRL_SW_SPI_EN_SHIFT                          0
#define    FNG_CORE_TOP_SPI_CTRL_SW_SPI_EN_MASK                           0x00000001

#define FNG_CORE_TOP_SPI_WAKE_X_SEED_OFFSET                               0x00000038
#define FNG_CORE_TOP_SPI_WAKE_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_SPI_WAKE_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_SPI_WAKE_X_SEED_SPI_WAKE_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_SPI_WAKE_X_SEED_SPI_WAKE_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_SPI_WAKE_Y_SEED_OFFSET                               0x0000003C
#define FNG_CORE_TOP_SPI_WAKE_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_SPI_WAKE_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_SPI_WAKE_Y_SEED_SPI_WAKE_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_SPI_WAKE_Y_SEED_SPI_WAKE_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_SPI_INIT_X_SEED_OFFSET                               0x00000040
#define FNG_CORE_TOP_SPI_INIT_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_SPI_INIT_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_SPI_INIT_X_SEED_SPI_INIT_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_SPI_INIT_X_SEED_SPI_INIT_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_SPI_INIT_Y_SEED_OFFSET                               0x00000044
#define FNG_CORE_TOP_SPI_INIT_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_SPI_INIT_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_SPI_INIT_Y_SEED_SPI_INIT_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_SPI_INIT_Y_SEED_SPI_INIT_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH0_CTRL1_OFFSET                                     0x00000048
#define FNG_CORE_TOP_CH0_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH0_CTRL1_RESERVED_MASK                              0x00000EEE
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_OVF_SYMCNT_SHIFT                    24
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_OVF_SYMCNT_MASK                     0xFF000000
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_OVF_SYMLEN_SHIFT                    20
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_OVF_SYMLEN_MASK                     0x00F00000
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_TD_VPGAIN_SHIFT                     16
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_TD_VPGAIN_MASK                      0x000F0000
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_CL_VPGAIN_SHIFT                     12
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_CL_VPGAIN_MASK                      0x0000F000
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_PILOT_SEL_SHIFT                     8
#define    FNG_CORE_TOP_CH0_CTRL1_CH0_PILOT_SEL_MASK                      0x00000100
#define    FNG_CORE_TOP_CH0_CTRL1_SW_CH0_STTD_SHIFT                       4
#define    FNG_CORE_TOP_CH0_CTRL1_SW_CH0_STTD_MASK                        0x00000010
#define    FNG_CORE_TOP_CH0_CTRL1_SW_CH0_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH0_CTRL1_SW_CH0_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH0_CTRL2_OFFSET                                     0x0000004C
#define FNG_CORE_TOP_CH0_CTRL2_TYPE                                       UInt32
#define FNG_CORE_TOP_CH0_CTRL2_RESERVED_MASK                              0x00F08F00
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_OFFSET_SHIFT                        24
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_OFFSET_MASK                         0xFF000000
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_VPGAIN_SHIFT                        16
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_VPGAIN_MASK                         0x000F0000
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_ACCLEN_SHIFT                        12
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_ACCLEN_MASK                         0x00007000
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_OVSF_SEED_SHIFT                     0
#define    FNG_CORE_TOP_CH0_CTRL2_CH0_OVSF_SEED_MASK                      0x000000FF

#define FNG_CORE_TOP_CH0_WAKE_X_SEED_OFFSET                               0x00000050
#define FNG_CORE_TOP_CH0_WAKE_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH0_WAKE_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH0_WAKE_X_SEED_CH0_WAKE_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH0_WAKE_X_SEED_CH0_WAKE_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH0_WAKE_Y_SEED_OFFSET                               0x00000054
#define FNG_CORE_TOP_CH0_WAKE_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH0_WAKE_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH0_WAKE_Y_SEED_CH0_WAKE_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH0_WAKE_Y_SEED_CH0_WAKE_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH0_INIT_X_SEED_OFFSET                               0x00000058
#define FNG_CORE_TOP_CH0_INIT_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH0_INIT_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH0_INIT_X_SEED_CH0_INIT_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH0_INIT_X_SEED_CH0_INIT_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH0_INIT_Y_SEED_OFFSET                               0x0000005C
#define FNG_CORE_TOP_CH0_INIT_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH0_INIT_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH0_INIT_Y_SEED_CH0_INIT_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH0_INIT_Y_SEED_CH0_INIT_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH1_CTRL1_OFFSET                                     0x00000060
#define FNG_CORE_TOP_CH1_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH1_CTRL1_RESERVED_MASK                              0x00000EEE
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_OVF_SYMCNT_SHIFT                    24
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_OVF_SYMCNT_MASK                     0xFF000000
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_OVF_SYMLEN_SHIFT                    20
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_OVF_SYMLEN_MASK                     0x00F00000
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_TD_VPGAIN_SHIFT                     16
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_TD_VPGAIN_MASK                      0x000F0000
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_CL_VPGAIN_SHIFT                     12
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_CL_VPGAIN_MASK                      0x0000F000
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_PILOT_SEL_SHIFT                     8
#define    FNG_CORE_TOP_CH1_CTRL1_CH1_PILOT_SEL_MASK                      0x00000100
#define    FNG_CORE_TOP_CH1_CTRL1_SW_CH1_STTD_SHIFT                       4
#define    FNG_CORE_TOP_CH1_CTRL1_SW_CH1_STTD_MASK                        0x00000010
#define    FNG_CORE_TOP_CH1_CTRL1_SW_CH1_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH1_CTRL1_SW_CH1_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH1_CTRL2_OFFSET                                     0x00000064
#define FNG_CORE_TOP_CH1_CTRL2_TYPE                                       UInt32
#define FNG_CORE_TOP_CH1_CTRL2_RESERVED_MASK                              0x00F08F00
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_OFFSET_SHIFT                        24
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_OFFSET_MASK                         0xFF000000
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_VPGAIN_SHIFT                        16
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_VPGAIN_MASK                         0x000F0000
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_ACCLEN_SHIFT                        12
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_ACCLEN_MASK                         0x00007000
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_OVSF_SEED_SHIFT                     0
#define    FNG_CORE_TOP_CH1_CTRL2_CH1_OVSF_SEED_MASK                      0x000000FF

#define FNG_CORE_TOP_CH1_WAKE_X_SEED_OFFSET                               0x00000068
#define FNG_CORE_TOP_CH1_WAKE_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH1_WAKE_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH1_WAKE_X_SEED_CH1_WAKE_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH1_WAKE_X_SEED_CH1_WAKE_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH1_WAKE_Y_SEED_OFFSET                               0x0000006C
#define FNG_CORE_TOP_CH1_WAKE_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH1_WAKE_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH1_WAKE_Y_SEED_CH1_WAKE_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH1_WAKE_Y_SEED_CH1_WAKE_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH1_INIT_X_SEED_OFFSET                               0x00000070
#define FNG_CORE_TOP_CH1_INIT_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH1_INIT_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH1_INIT_X_SEED_CH1_INIT_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH1_INIT_X_SEED_CH1_INIT_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH1_INIT_Y_SEED_OFFSET                               0x00000074
#define FNG_CORE_TOP_CH1_INIT_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH1_INIT_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH1_INIT_Y_SEED_CH1_INIT_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH1_INIT_Y_SEED_CH1_INIT_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH2_CTRL1_OFFSET                                     0x00000078
#define FNG_CORE_TOP_CH2_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH2_CTRL1_RESERVED_MASK                              0x00000ECE
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_OVF_SYMCNT_SHIFT                    24
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_OVF_SYMCNT_MASK                     0xFF000000
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_OVF_SYMLEN_SHIFT                    20
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_OVF_SYMLEN_MASK                     0x00F00000
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_TD_VPGAIN_SHIFT                     16
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_TD_VPGAIN_MASK                      0x000F0000
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_CL_VPGAIN_SHIFT                     12
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_CL_VPGAIN_MASK                      0x0000F000
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_PILOT_SEL_SHIFT                     8
#define    FNG_CORE_TOP_CH2_CTRL1_CH2_PILOT_SEL_MASK                      0x00000100
#define    FNG_CORE_TOP_CH2_CTRL1_SW_CH2_MODE_SHIFT                       5
#define    FNG_CORE_TOP_CH2_CTRL1_SW_CH2_MODE_MASK                        0x00000020
#define    FNG_CORE_TOP_CH2_CTRL1_SW_CH2_STTD_SHIFT                       4
#define    FNG_CORE_TOP_CH2_CTRL1_SW_CH2_STTD_MASK                        0x00000010
#define    FNG_CORE_TOP_CH2_CTRL1_SW_CH2_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH2_CTRL1_SW_CH2_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH2_CTRL2_OFFSET                                     0x0000007C
#define FNG_CORE_TOP_CH2_CTRL2_TYPE                                       UInt32
#define FNG_CORE_TOP_CH2_CTRL2_RESERVED_MASK                              0x00F08F00
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_OFFSET_SHIFT                        24
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_OFFSET_MASK                         0xFF000000
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_VPGAIN_SHIFT                        16
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_VPGAIN_MASK                         0x000F0000
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_ACCLEN_SHIFT                        12
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_ACCLEN_MASK                         0x00007000
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_OVSF_SEED_SHIFT                     0
#define    FNG_CORE_TOP_CH2_CTRL2_CH2_OVSF_SEED_MASK                      0x000000FF

#define FNG_CORE_TOP_CH2_WAKE_X_SEED_OFFSET                               0x00000080
#define FNG_CORE_TOP_CH2_WAKE_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH2_WAKE_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH2_WAKE_X_SEED_CH2_WAKE_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH2_WAKE_X_SEED_CH2_WAKE_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH2_WAKE_Y_SEED_OFFSET                               0x00000084
#define FNG_CORE_TOP_CH2_WAKE_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH2_WAKE_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH2_WAKE_Y_SEED_CH2_WAKE_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH2_WAKE_Y_SEED_CH2_WAKE_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH2_INIT_X_SEED_OFFSET                               0x00000088
#define FNG_CORE_TOP_CH2_INIT_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH2_INIT_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH2_INIT_X_SEED_CH2_INIT_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH2_INIT_X_SEED_CH2_INIT_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH2_INIT_Y_SEED_OFFSET                               0x0000008C
#define FNG_CORE_TOP_CH2_INIT_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH2_INIT_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH2_INIT_Y_SEED_CH2_INIT_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH2_INIT_Y_SEED_CH2_INIT_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_DLL_CTRL1_OFFSET                                     0x00000090
#define FNG_CORE_TOP_DLL_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_DLL_CTRL1_RESERVED_MASK                              0xEE06800E
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_SW_RET_EN_SHIFT                     28
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_SW_RET_EN_MASK                      0x10000000
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_SW_ADV_EN_SHIFT                     24
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_SW_ADV_EN_MASK                      0x01000000
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_SW_CMD_NO_SHIFT                     20
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_SW_CMD_NO_MASK                      0x00F00000
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_UNFREEZE_BIT_SHIFT                  19
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_UNFREEZE_BIT_MASK                   0x00080000
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_HW_FREEZE_SHIFT                     16
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_HW_FREEZE_MASK                      0x00010000
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_MASTER_SHIFT                        12
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_MASTER_MASK                         0x00007000
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_VPGAIN_SHIFT                        8
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_VPGAIN_MASK                         0x00000F00
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_ACCLEN_SHIFT                        4
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_ACCLEN_MASK                         0x000000F0
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_PILOT_SEL_SHIFT                     0
#define    FNG_CORE_TOP_DLL_CTRL1_DLL_PILOT_SEL_MASK                      0x00000001

#define FNG_CORE_TOP_DLL_CTRL2_OFFSET                                     0x00000094
#define FNG_CORE_TOP_DLL_CTRL2_TYPE                                       UInt32
#define FNG_CORE_TOP_DLL_CTRL2_RESERVED_MASK                              0xFFFFEEEE
#define    FNG_CORE_TOP_DLL_CTRL2_DLL_DECLEN_SHIFT                        12
#define    FNG_CORE_TOP_DLL_CTRL2_DLL_DECLEN_MASK                         0x00001000
#define    FNG_CORE_TOP_DLL_CTRL2_DLL_NEW_EN_SHIFT                        8
#define    FNG_CORE_TOP_DLL_CTRL2_DLL_NEW_EN_MASK                         0x00000100
#define    FNG_CORE_TOP_DLL_CTRL2_DLL_HOLD_SHIFT                          4
#define    FNG_CORE_TOP_DLL_CTRL2_DLL_HOLD_MASK                           0x00000010
#define    FNG_CORE_TOP_DLL_CTRL2_SW_DLL_EN_SHIFT                         0
#define    FNG_CORE_TOP_DLL_CTRL2_SW_DLL_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CPI_ACC_RSCP1_OFFSET                                 0x00000098
#define FNG_CORE_TOP_CPI_ACC_RSCP1_TYPE                                   UInt32
#define FNG_CORE_TOP_CPI_ACC_RSCP1_RESERVED_MASK                          0x00000000
#define    FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC1_RSCP_SHIFT                 16
#define    FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC1_RSCP_MASK                  0xFFFF0000
#define    FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC0_RSCP_SHIFT                 0
#define    FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC0_RSCP_MASK                  0x0000FFFF

#define FNG_CORE_TOP_CPI_ACC_RSCP2_OFFSET                                 0x0000009C
#define FNG_CORE_TOP_CPI_ACC_RSCP2_TYPE                                   UInt32
#define FNG_CORE_TOP_CPI_ACC_RSCP2_RESERVED_MASK                          0x00000000
#define    FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC3_RSCP_SHIFT                 16
#define    FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC3_RSCP_MASK                  0xFFFF0000
#define    FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC2_RSCP_SHIFT                 0
#define    FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC2_RSCP_MASK                  0x0000FFFF

#define FNG_CORE_TOP_CH_DEMOD_RSCP_OFFSET                                 0x000000A4
#define FNG_CORE_TOP_CH_DEMOD_RSCP_TYPE                                   UInt32
#define FNG_CORE_TOP_CH_DEMOD_RSCP_RESERVED_MASK                          0x00000000
#define    FNG_CORE_TOP_CH_DEMOD_RSCP_CH1_DEMOD_RSCP_SHIFT                16
#define    FNG_CORE_TOP_CH_DEMOD_RSCP_CH1_DEMOD_RSCP_MASK                 0xFFFF0000
#define    FNG_CORE_TOP_CH_DEMOD_RSCP_CH0_DEMOD_RSCP_SHIFT                0
#define    FNG_CORE_TOP_CH_DEMOD_RSCP_CH0_DEMOD_RSCP_MASK                 0x0000FFFF

#define FNG_CORE_TOP_CH2_DEMOD_RSCP_OFFSET                                0x000000A8
#define FNG_CORE_TOP_CH2_DEMOD_RSCP_TYPE                                  UInt32
#define FNG_CORE_TOP_CH2_DEMOD_RSCP_RESERVED_MASK                         0x00000000
#define    FNG_CORE_TOP_CH2_DEMOD_RSCP_CH3_DEMOD_RSCP_SHIFT               16
#define    FNG_CORE_TOP_CH2_DEMOD_RSCP_CH3_DEMOD_RSCP_MASK                0xFFFF0000
#define    FNG_CORE_TOP_CH2_DEMOD_RSCP_CH2_DEMOD_RSCP_SHIFT               0
#define    FNG_CORE_TOP_CH2_DEMOD_RSCP_CH2_DEMOD_RSCP_MASK                0x0000FFFF

#define FNG_CORE_TOP_ANT_CHEST_PDATA_OFFSET                               0x000000AC
#define FNG_CORE_TOP_ANT_CHEST_PDATA_TYPE                                 UInt32
#define FNG_CORE_TOP_ANT_CHEST_PDATA_RESERVED_MASK                        0x00000000
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT2_CHEST_Q_PDMCH_SHIFT          24
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT2_CHEST_Q_PDMCH_MASK           0xFF000000
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT2_CHEST_I_PDMCH_SHIFT          16
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT2_CHEST_I_PDMCH_MASK           0x00FF0000
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT1_CHEST_Q_PDMCH_SHIFT          8
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT1_CHEST_Q_PDMCH_MASK           0x0000FF00
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT1_CHEST_I_PDMCH_SHIFT          0
#define    FNG_CORE_TOP_ANT_CHEST_PDATA_ANT1_CHEST_I_PDMCH_MASK           0x000000FF

#define FNG_CORE_TOP_ANT_CHEST_SDATA_OFFSET                               0x000000B0
#define FNG_CORE_TOP_ANT_CHEST_SDATA_TYPE                                 UInt32
#define FNG_CORE_TOP_ANT_CHEST_SDATA_RESERVED_MASK                        0x00000000
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT2_CHEST_Q_SDMCH_SHIFT          24
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT2_CHEST_Q_SDMCH_MASK           0xFF000000
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT2_CHEST_I_SDMCH_SHIFT          16
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT2_CHEST_I_SDMCH_MASK           0x00FF0000
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT1_CHEST_Q_SDMCH_SHIFT          8
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT1_CHEST_Q_SDMCH_MASK           0x0000FF00
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT1_CHEST_I_SDMCH_SHIFT          0
#define    FNG_CORE_TOP_ANT_CHEST_SDATA_ANT1_CHEST_I_SDMCH_MASK           0x000000FF

#define FNG_CORE_TOP_ANT_CLOOP_GAIN_OFFSET                                0x000000B4
#define FNG_CORE_TOP_ANT_CLOOP_GAIN_TYPE                                  UInt32
#define FNG_CORE_TOP_ANT_CLOOP_GAIN_RESERVED_MASK                         0x00000000
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT2_CLOOP_Q_GAIN_SHIFT            24
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT2_CLOOP_Q_GAIN_MASK             0xFF000000
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT2_CLOOP_I_GAIN_SHIFT            16
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT2_CLOOP_I_GAIN_MASK             0x00FF0000
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT1_CLOOP_Q_GAIN_SHIFT            8
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT1_CLOOP_Q_GAIN_MASK             0x0000FF00
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT1_CLOOP_I_GAIN_SHIFT            0
#define    FNG_CORE_TOP_ANT_CLOOP_GAIN_ANT1_CLOOP_I_GAIN_MASK             0x000000FF

#define FNG_CORE_TOP_DLL_PWR_DIFF_OFFSET                                  0x000000B8
#define FNG_CORE_TOP_DLL_PWR_DIFF_TYPE                                    UInt32
#define FNG_CORE_TOP_DLL_PWR_DIFF_RESERVED_MASK                           0x00000000
#define    FNG_CORE_TOP_DLL_PWR_DIFF_DLL_FAT_FNG_COMB_SHIFT               24
#define    FNG_CORE_TOP_DLL_PWR_DIFF_DLL_FAT_FNG_COMB_MASK                0xFF000000
#define    FNG_CORE_TOP_DLL_PWR_DIFF_DLL_PWR_DIFF_SHIFT                   0
#define    FNG_CORE_TOP_DLL_PWR_DIFF_DLL_PWR_DIFF_MASK                    0x00FFFFFF

#define FNG_CORE_TOP_CH3_LOAD_OFFSET                                      0x000000C0
#define FNG_CORE_TOP_CH3_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH3_LOAD_RESERVED_MASK                               0xFFFFFCEE
#define    FNG_CORE_TOP_CH3_LOAD_CH3_WAKE_LOAD_SHIFT                      8
#define    FNG_CORE_TOP_CH3_LOAD_CH3_WAKE_LOAD_MASK                       0x00000300
#define    FNG_CORE_TOP_CH3_LOAD_CH3_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH3_LOAD_CH3_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH3_LOAD_CH3_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH3_LOAD_CH3_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_CH3_CTRL1_OFFSET                                     0x000000C4
#define FNG_CORE_TOP_CH3_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH3_CTRL1_RESERVED_MASK                              0x00000EEE
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_OVF_SYMCNT_SHIFT                    24
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_OVF_SYMCNT_MASK                     0xFF000000
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_OVF_SYMLEN_SHIFT                    20
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_OVF_SYMLEN_MASK                     0x00F00000
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_TD_VPGAIN_SHIFT                     16
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_TD_VPGAIN_MASK                      0x000F0000
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_CL_VPGAIN_SHIFT                     12
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_CL_VPGAIN_MASK                      0x0000F000
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_PILOT_SEL_SHIFT                     8
#define    FNG_CORE_TOP_CH3_CTRL1_CH3_PILOT_SEL_MASK                      0x00000100
#define    FNG_CORE_TOP_CH3_CTRL1_SW_CH3_STTD_SHIFT                       4
#define    FNG_CORE_TOP_CH3_CTRL1_SW_CH3_STTD_MASK                        0x00000010
#define    FNG_CORE_TOP_CH3_CTRL1_SW_CH3_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH3_CTRL1_SW_CH3_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH3_CTRL2_OFFSET                                     0x000000C8
#define FNG_CORE_TOP_CH3_CTRL2_TYPE                                       UInt32
#define FNG_CORE_TOP_CH3_CTRL2_RESERVED_MASK                              0x00C08E00
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_OFFSET_SHIFT                        24
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_OFFSET_MASK                         0xFF000000
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_STTD_SEL_SHIFT                      20
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_STTD_SEL_MASK                       0x00300000
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_VPGAIN_SHIFT                        16
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_VPGAIN_MASK                         0x000F0000
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_ACCLEN_SHIFT                        12
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_ACCLEN_MASK                         0x00007000
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_FDPCH_EN_SHIFT                      8
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_FDPCH_EN_MASK                       0x00000100
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_OVSF_SEED_SHIFT                     0
#define    FNG_CORE_TOP_CH3_CTRL2_CH3_OVSF_SEED_MASK                      0x000000FF

#define FNG_CORE_TOP_CH3_WAKE_X_SEED_OFFSET                               0x000000CC
#define FNG_CORE_TOP_CH3_WAKE_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH3_WAKE_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH3_WAKE_X_SEED_CH3_WAKE_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH3_WAKE_X_SEED_CH3_WAKE_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH3_WAKE_Y_SEED_OFFSET                               0x000000D0
#define FNG_CORE_TOP_CH3_WAKE_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH3_WAKE_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH3_WAKE_Y_SEED_CH3_WAKE_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH3_WAKE_Y_SEED_CH3_WAKE_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH3_INIT_X_SEED_OFFSET                               0x000000D4
#define FNG_CORE_TOP_CH3_INIT_X_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH3_INIT_X_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH3_INIT_X_SEED_CH3_INIT_X_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH3_INIT_X_SEED_CH3_INIT_X_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_CH3_INIT_Y_SEED_OFFSET                               0x000000D8
#define FNG_CORE_TOP_CH3_INIT_Y_SEED_TYPE                                 UInt32
#define FNG_CORE_TOP_CH3_INIT_Y_SEED_RESERVED_MASK                        0xFFFC0000
#define    FNG_CORE_TOP_CH3_INIT_Y_SEED_CH3_INIT_Y_SEED_SHIFT             0
#define    FNG_CORE_TOP_CH3_INIT_Y_SEED_CH3_INIT_Y_SEED_MASK              0x0003FFFF

#define FNG_CORE_TOP_ANT1_CHEST_PNODM_OFFSET                              0x000000DC
#define FNG_CORE_TOP_ANT1_CHEST_PNODM_TYPE                                UInt32
#define FNG_CORE_TOP_ANT1_CHEST_PNODM_RESERVED_MASK                       0x00000000
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_Q_PNODM_SHIFT         24
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_Q_PNODM_MASK          0xFF000000
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_I_PNODM_SHIFT         16
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_I_PNODM_MASK          0x00FF0000
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_Q_PNODM_SHIFT         8
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_Q_PNODM_MASK          0x0000FF00
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_I_PNODM_SHIFT         0
#define    FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_I_PNODM_MASK          0x000000FF

#define FNG_CORE_TOP_ANT1_CHEST_SNODM_OFFSET                              0x000000E0
#define FNG_CORE_TOP_ANT1_CHEST_SNODM_TYPE                                UInt32
#define FNG_CORE_TOP_ANT1_CHEST_SNODM_RESERVED_MASK                       0x00000000
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_Q_SNODM_SHIFT         24
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_Q_SNODM_MASK          0xFF000000
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_I_SNODM_SHIFT         16
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_I_SNODM_MASK          0x00FF0000
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_Q_SNODM_SHIFT         8
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_Q_SNODM_MASK          0x0000FF00
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_I_SNODM_SHIFT         0
#define    FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_I_SNODM_MASK          0x000000FF

#define FNG_CORE_TOP_LPF_OFFSET                                           0x000000E4
#define FNG_CORE_TOP_LPF_TYPE                                             UInt32
#define FNG_CORE_TOP_LPF_RESERVED_MASK                                    0xFFFFFCCE
#define    FNG_CORE_TOP_LPF_LPF_NODM_SEL_SHIFT                            8
#define    FNG_CORE_TOP_LPF_LPF_NODM_SEL_MASK                             0x00000300
#define    FNG_CORE_TOP_LPF_LPF_DMCH_SEL_SHIFT                            4
#define    FNG_CORE_TOP_LPF_LPF_DMCH_SEL_MASK                             0x00000030
#define    FNG_CORE_TOP_LPF_SW_DM_ENA_SHIFT                               0
#define    FNG_CORE_TOP_LPF_SW_DM_ENA_MASK                                0x00000001

#define FNG_CORE_TOP_CH4_LOAD_OFFSET                                      0x000000EC
#define FNG_CORE_TOP_CH4_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH4_LOAD_RESERVED_MASK                               0xFFFFFEEE
#define    FNG_CORE_TOP_CH4_LOAD_CH4_WAKE_LOAD_SHIFT                      8
#define    FNG_CORE_TOP_CH4_LOAD_CH4_WAKE_LOAD_MASK                       0x00000100
#define    FNG_CORE_TOP_CH4_LOAD_CH4_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH4_LOAD_CH4_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH4_LOAD_CH4_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH4_LOAD_CH4_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_CH5_LOAD_OFFSET                                      0x000000F0
#define FNG_CORE_TOP_CH5_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH5_LOAD_RESERVED_MASK                               0xFFFFFEEE
#define    FNG_CORE_TOP_CH5_LOAD_CH5_LOAD_MASK_SHIFT                      8
#define    FNG_CORE_TOP_CH5_LOAD_CH5_LOAD_MASK_MASK                       0x00000100
#define    FNG_CORE_TOP_CH5_LOAD_CH5_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH5_LOAD_CH5_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH5_LOAD_CH5_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH5_LOAD_CH5_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_CH6_LOAD_OFFSET                                      0x000000F4
#define FNG_CORE_TOP_CH6_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH6_LOAD_RESERVED_MASK                               0xFFFFFEEE
#define    FNG_CORE_TOP_CH6_LOAD_CH6_LOAD_MASK_SHIFT                      8
#define    FNG_CORE_TOP_CH6_LOAD_CH6_LOAD_MASK_MASK                       0x00000100
#define    FNG_CORE_TOP_CH6_LOAD_CH6_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH6_LOAD_CH6_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH6_LOAD_CH6_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH6_LOAD_CH6_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_HSUPA_CTRL_OFFSET                                    0x000000F8
#define FNG_CORE_TOP_HSUPA_CTRL_TYPE                                      UInt32
#define FNG_CORE_TOP_HSUPA_CTRL_RESERVED_MASK                             0x80000EEE
#define    FNG_CORE_TOP_HSUPA_CTRL_NOISE_GAIN_SEL_SHIFT                   30
#define    FNG_CORE_TOP_HSUPA_CTRL_NOISE_GAIN_SEL_MASK                    0x40000000
#define    FNG_CORE_TOP_HSUPA_CTRL_CHEST_GAIN_SEL_SHIFT                   29
#define    FNG_CORE_TOP_HSUPA_CTRL_CHEST_GAIN_SEL_MASK                    0x20000000
#define    FNG_CORE_TOP_HSUPA_CTRL_SW_CPM_EN_SHIFT                        28
#define    FNG_CORE_TOP_HSUPA_CTRL_SW_CPM_EN_MASK                         0x10000000
#define    FNG_CORE_TOP_HSUPA_CTRL_NPE_VPGAIN_SHIFT                       24
#define    FNG_CORE_TOP_HSUPA_CTRL_NPE_VPGAIN_MASK                        0x0F000000
#define    FNG_CORE_TOP_HSUPA_CTRL_ECH_OVSF_SEED_SHIFT                    16
#define    FNG_CORE_TOP_HSUPA_CTRL_ECH_OVSF_SEED_MASK                     0x00FF0000
#define    FNG_CORE_TOP_HSUPA_CTRL_ECH_VPGAIN_SHIFT                       12
#define    FNG_CORE_TOP_HSUPA_CTRL_ECH_VPGAIN_MASK                        0x0000F000
#define    FNG_CORE_TOP_HSUPA_CTRL_ECH_PILOT_SEL_SHIFT                    8
#define    FNG_CORE_TOP_HSUPA_CTRL_ECH_PILOT_SEL_MASK                     0x00000100
#define    FNG_CORE_TOP_HSUPA_CTRL_SW_ECH_STTD_SHIFT                      4
#define    FNG_CORE_TOP_HSUPA_CTRL_SW_ECH_STTD_MASK                       0x00000010
#define    FNG_CORE_TOP_HSUPA_CTRL_SW_NPE_EN_SHIFT                        0
#define    FNG_CORE_TOP_HSUPA_CTRL_SW_NPE_EN_MASK                         0x00000001

#define FNG_CORE_TOP_CH4_CTRL1_OFFSET                                     0x000000FC
#define FNG_CORE_TOP_CH4_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH4_CTRL1_RESERVED_MASK                              0xF0000EEE
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_OVSF_SEED_SHIFT                     20
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_OVSF_SEED_MASK                      0x0FF00000
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_TD_VPGAIN_SHIFT                     16
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_TD_VPGAIN_MASK                      0x000F0000
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_VPGAIN_SHIFT                        12
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_VPGAIN_MASK                         0x0000F000
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_PILOT_SEL_SHIFT                     8
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_PILOT_SEL_MASK                      0x00000100
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_TTI_SIZE_SHIFT                      4
#define    FNG_CORE_TOP_CH4_CTRL1_CH4_TTI_SIZE_MASK                       0x00000010
#define    FNG_CORE_TOP_CH4_CTRL1_SW_CH4_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH4_CTRL1_SW_CH4_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH5_CTRL1_OFFSET                                     0x00000100
#define FNG_CORE_TOP_CH5_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH5_CTRL1_RESERVED_MASK                              0xFF00C0CE
#define    FNG_CORE_TOP_CH5_CTRL1_CH5_OFFSET_SHIFT                        16
#define    FNG_CORE_TOP_CH5_CTRL1_CH5_OFFSET_MASK                         0x00FF0000
#define    FNG_CORE_TOP_CH5_CTRL1_CH5_SEQ_INDEX_SHIFT                     8
#define    FNG_CORE_TOP_CH5_CTRL1_CH5_SEQ_INDEX_MASK                      0x00003F00
#define    FNG_CORE_TOP_CH5_CTRL1_CH5_TTI_SIZE_SHIFT                      4
#define    FNG_CORE_TOP_CH5_CTRL1_CH5_TTI_SIZE_MASK                       0x00000030
#define    FNG_CORE_TOP_CH5_CTRL1_SW_CH5_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH5_CTRL1_SW_CH5_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH6_CTRL1_OFFSET                                     0x00000104
#define FNG_CORE_TOP_CH6_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH6_CTRL1_RESERVED_MASK                              0xFF00C0EE
#define    FNG_CORE_TOP_CH6_CTRL1_CH6_OFFSET_SHIFT                        16
#define    FNG_CORE_TOP_CH6_CTRL1_CH6_OFFSET_MASK                         0x00FF0000
#define    FNG_CORE_TOP_CH6_CTRL1_CH6_SEQ_INDEX_SHIFT                     8
#define    FNG_CORE_TOP_CH6_CTRL1_CH6_SEQ_INDEX_MASK                      0x00003F00
#define    FNG_CORE_TOP_CH6_CTRL1_CH6_TTI_SIZE_SHIFT                      4
#define    FNG_CORE_TOP_CH6_CTRL1_CH6_TTI_SIZE_MASK                       0x00000010
#define    FNG_CORE_TOP_CH6_CTRL1_SW_CH6_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH6_CTRL1_SW_CH6_EN_MASK                          0x00000001

#define FNG_CORE_TOP_ERGCH_SD_OFFSET                                      0x00000108
#define FNG_CORE_TOP_ERGCH_SD_TYPE                                        UInt32
#define FNG_CORE_TOP_ERGCH_SD_RESERVED_MASK                               0x80000000
#define    FNG_CORE_TOP_ERGCH_SD_ERGCH_NOISE_PWR_SHIFT                    16
#define    FNG_CORE_TOP_ERGCH_SD_ERGCH_NOISE_PWR_MASK                     0x7FFF0000
#define    FNG_CORE_TOP_ERGCH_SD_ERGCH_SOFT_DATA_SHIFT                    0
#define    FNG_CORE_TOP_ERGCH_SD_ERGCH_SOFT_DATA_MASK                     0x0000FFFF

#define FNG_CORE_TOP_EHICH_SD_OFFSET                                      0x0000010C
#define FNG_CORE_TOP_EHICH_SD_TYPE                                        UInt32
#define FNG_CORE_TOP_EHICH_SD_RESERVED_MASK                               0x80000000
#define    FNG_CORE_TOP_EHICH_SD_EHICH_NOISE_PWR_SHIFT                    16
#define    FNG_CORE_TOP_EHICH_SD_EHICH_NOISE_PWR_MASK                     0x7FFF0000
#define    FNG_CORE_TOP_EHICH_SD_EHICH_SOFT_DATA_SHIFT                    0
#define    FNG_CORE_TOP_EHICH_SD_EHICH_SOFT_DATA_MASK                     0x0000FFFF

#define FNG_CORE_TOP_SCH_NP_SD_OFFSET                                     0x00000110
#define FNG_CORE_TOP_SCH_NP_SD_TYPE                                       UInt32
#define FNG_CORE_TOP_SCH_NP_SD_RESERVED_MASK                              0x00000000
#define    FNG_CORE_TOP_SCH_NP_SD_SCH_NOISE_PWR2_SHIFT                    16
#define    FNG_CORE_TOP_SCH_NP_SD_SCH_NOISE_PWR2_MASK                     0xFFFF0000
#define    FNG_CORE_TOP_SCH_NP_SD_SCH_NOISE_PWR1_SHIFT                    0
#define    FNG_CORE_TOP_SCH_NP_SD_SCH_NOISE_PWR1_MASK                     0x0000FFFF

#define FNG_CORE_TOP_DPILOT_START_CNT_OFFSET                              0x00000114
#define FNG_CORE_TOP_DPILOT_START_CNT_TYPE                                UInt32
#define FNG_CORE_TOP_DPILOT_START_CNT_RESERVED_MASK                       0xFFFFFC00
#define    FNG_CORE_TOP_DPILOT_START_CNT_DPILOT_START_CNT_SHIFT           0
#define    FNG_CORE_TOP_DPILOT_START_CNT_DPILOT_START_CNT_MASK            0x000003FF

#define FNG_CORE_TOP_CPM_START_OFFSET                                     0x00000118
#define FNG_CORE_TOP_CPM_START_TYPE                                       UInt32
#define FNG_CORE_TOP_CPM_START_RESERVED_MASK                              0x0000C000
#define    FNG_CORE_TOP_CPM_START_CPM_START_FRAMEPOS_SHIFT                20
#define    FNG_CORE_TOP_CPM_START_CPM_START_FRAMEPOS_MASK                 0xFFF00000
#define    FNG_CORE_TOP_CPM_START_CPM_START_SLOTPOS_SHIFT                 16
#define    FNG_CORE_TOP_CPM_START_CPM_START_SLOTPOS_MASK                  0x000F0000
#define    FNG_CORE_TOP_CPM_START_CPM_START_SAMPLEPOS_SHIFT               0
#define    FNG_CORE_TOP_CPM_START_CPM_START_SAMPLEPOS_MASK                0x00003FFF

#define FNG_CORE_TOP_CPM_STOP_OFFSET                                      0x0000011C
#define FNG_CORE_TOP_CPM_STOP_TYPE                                        UInt32
#define FNG_CORE_TOP_CPM_STOP_RESERVED_MASK                               0x0000C000
#define    FNG_CORE_TOP_CPM_STOP_CPM_STOP_FRAMEPOS_SHIFT                  20
#define    FNG_CORE_TOP_CPM_STOP_CPM_STOP_FRAMEPOS_MASK                   0xFFF00000
#define    FNG_CORE_TOP_CPM_STOP_CPM_STOP_SLOTPOS_SHIFT                   16
#define    FNG_CORE_TOP_CPM_STOP_CPM_STOP_SLOTPOS_MASK                    0x000F0000
#define    FNG_CORE_TOP_CPM_STOP_CPM_STOP_SAMPLEPOS_SHIFT                 0
#define    FNG_CORE_TOP_CPM_STOP_CPM_STOP_SAMPLEPOS_MASK                  0x00003FFF

#define FNG_CORE_TOP_DLL_THRESHOLD_OFFSET                                 0x00000120
#define FNG_CORE_TOP_DLL_THRESHOLD_TYPE                                   UInt32
#define FNG_CORE_TOP_DLL_THRESHOLD_RESERVED_MASK                          0xEF000000
#define    FNG_CORE_TOP_DLL_THRESHOLD_DLL_ACC_CLEAR_SHIFT                 28
#define    FNG_CORE_TOP_DLL_THRESHOLD_DLL_ACC_CLEAR_MASK                  0x10000000
#define    FNG_CORE_TOP_DLL_THRESHOLD_DLL_THRESHOLD_SHIFT                 0
#define    FNG_CORE_TOP_DLL_THRESHOLD_DLL_THRESHOLD_MASK                  0x00FFFFFF

#define FNG_CORE_TOP_EST_PPI_ANT0_OFFSET                                  0x00000124
#define FNG_CORE_TOP_EST_PPI_ANT0_TYPE                                    UInt32
#define FNG_CORE_TOP_EST_PPI_ANT0_RESERVED_MASK                           0x00000000
#define    FNG_CORE_TOP_EST_PPI_ANT0_EST_AMP_SQ_PPI_ANT0_SHIFT            16
#define    FNG_CORE_TOP_EST_PPI_ANT0_EST_AMP_SQ_PPI_ANT0_MASK             0xFFFF0000
#define    FNG_CORE_TOP_EST_PPI_ANT0_EST_AMP_VAR_PPI_ANT0_SHIFT           0
#define    FNG_CORE_TOP_EST_PPI_ANT0_EST_AMP_VAR_PPI_ANT0_MASK            0x0000FFFF

#define FNG_CORE_TOP_EST_SPI_ANT0_OFFSET                                  0x00000140
#define FNG_CORE_TOP_EST_SPI_ANT0_TYPE                                    UInt32
#define FNG_CORE_TOP_EST_SPI_ANT0_RESERVED_MASK                           0x00000000
#define    FNG_CORE_TOP_EST_SPI_ANT0_EST_AMP_SQ_SPI_ANT0_SHIFT            16
#define    FNG_CORE_TOP_EST_SPI_ANT0_EST_AMP_SQ_SPI_ANT0_MASK             0xFFFF0000
#define    FNG_CORE_TOP_EST_SPI_ANT0_EST_AMP_VAR_SPI_ANT0_SHIFT           0
#define    FNG_CORE_TOP_EST_SPI_ANT0_EST_AMP_VAR_SPI_ANT0_MASK            0x0000FFFF

#define FNG_CORE_TOP_EST_IIR_OFFSET                                       0x00000144
#define FNG_CORE_TOP_EST_IIR_TYPE                                         UInt32
#define FNG_CORE_TOP_EST_IIR_RESERVED_MASK                                0xFFFFEE88
#define    FNG_CORE_TOP_EST_IIR_VAR_EST_IIR_CLEAR_SHIFT                   12
#define    FNG_CORE_TOP_EST_IIR_VAR_EST_IIR_CLEAR_MASK                    0x00001000
#define    FNG_CORE_TOP_EST_IIR_AMP_EST_IIR_CLEAR_SHIFT                   8
#define    FNG_CORE_TOP_EST_IIR_AMP_EST_IIR_CLEAR_MASK                    0x00000100
#define    FNG_CORE_TOP_EST_IIR_AMP_EST_IIR_COEF_SHIFT                    4
#define    FNG_CORE_TOP_EST_IIR_AMP_EST_IIR_COEF_MASK                     0x00000070
#define    FNG_CORE_TOP_EST_IIR_VAR_EST_IIR_COEF_SHIFT                    0
#define    FNG_CORE_TOP_EST_IIR_VAR_EST_IIR_COEF_MASK                     0x00000007

#define FNG_CORE_TOP_EST_PPI_ANT1_OFFSET                                  0x00000148
#define FNG_CORE_TOP_EST_PPI_ANT1_TYPE                                    UInt32
#define FNG_CORE_TOP_EST_PPI_ANT1_RESERVED_MASK                           0x00000000
#define    FNG_CORE_TOP_EST_PPI_ANT1_EST_AMP_SQ_PPI_ANT1_SHIFT            16
#define    FNG_CORE_TOP_EST_PPI_ANT1_EST_AMP_SQ_PPI_ANT1_MASK             0xFFFF0000
#define    FNG_CORE_TOP_EST_PPI_ANT1_EST_AMP_VAR_PPI_ANT1_SHIFT           0
#define    FNG_CORE_TOP_EST_PPI_ANT1_EST_AMP_VAR_PPI_ANT1_MASK            0x0000FFFF

#define FNG_CORE_TOP_EST_SPI_ANT1_OFFSET                                  0x0000014C
#define FNG_CORE_TOP_EST_SPI_ANT1_TYPE                                    UInt32
#define FNG_CORE_TOP_EST_SPI_ANT1_RESERVED_MASK                           0x00000000
#define    FNG_CORE_TOP_EST_SPI_ANT1_EST_AMP_SQ_SPI_ANT1_SHIFT            16
#define    FNG_CORE_TOP_EST_SPI_ANT1_EST_AMP_SQ_SPI_ANT1_MASK             0xFFFF0000
#define    FNG_CORE_TOP_EST_SPI_ANT1_EST_AMP_VAR_SPI_ANT1_SHIFT           0
#define    FNG_CORE_TOP_EST_SPI_ANT1_EST_AMP_VAR_SPI_ANT1_MASK            0x0000FFFF

#define FNG_CORE_TOP_AGCH_WAKE_X_SEED_OFFSET                              0x00000150
#define FNG_CORE_TOP_AGCH_WAKE_X_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_AGCH_WAKE_X_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_AGCH_WAKE_X_SEED_AGCH_WAKE_X_SEED_SHIFT           0
#define    FNG_CORE_TOP_AGCH_WAKE_X_SEED_AGCH_WAKE_X_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_AGCH_WAKE_Y_SEED_OFFSET                              0x00000154
#define FNG_CORE_TOP_AGCH_WAKE_Y_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_AGCH_WAKE_Y_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_AGCH_WAKE_Y_SEED_AGCH_WAKE_Y_SEED_SHIFT           0
#define    FNG_CORE_TOP_AGCH_WAKE_Y_SEED_AGCH_WAKE_Y_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_AGCH_INIT_X_SEED_OFFSET                              0x00000158
#define FNG_CORE_TOP_AGCH_INIT_X_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_AGCH_INIT_X_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_AGCH_INIT_X_SEED_AGCH_INIT_X_SEED_SHIFT           0
#define    FNG_CORE_TOP_AGCH_INIT_X_SEED_AGCH_INIT_X_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_AGCH_INIT_Y_SEED_OFFSET                              0x0000015C
#define FNG_CORE_TOP_AGCH_INIT_Y_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_AGCH_INIT_Y_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_AGCH_INIT_Y_SEED_AGCH_INIT_Y_SEED_SHIFT           0
#define    FNG_CORE_TOP_AGCH_INIT_Y_SEED_AGCH_INIT_Y_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_SCCH_WAKE_X_SEED_OFFSET                              0x00000160
#define FNG_CORE_TOP_SCCH_WAKE_X_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_SCCH_WAKE_X_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_SCCH_WAKE_X_SEED_SCCH_WAKE_X_SEED_SHIFT           0
#define    FNG_CORE_TOP_SCCH_WAKE_X_SEED_SCCH_WAKE_X_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_SCCH_WAKE_Y_SEED_OFFSET                              0x00000164
#define FNG_CORE_TOP_SCCH_WAKE_Y_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_SCCH_WAKE_Y_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_SCCH_WAKE_Y_SEED_SCCH_WAKE_Y_SEED_SHIFT           0
#define    FNG_CORE_TOP_SCCH_WAKE_Y_SEED_SCCH_WAKE_Y_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_SCCH_INIT_X_SEED_OFFSET                              0x00000168
#define FNG_CORE_TOP_SCCH_INIT_X_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_SCCH_INIT_X_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_SCCH_INIT_X_SEED_SCCH_INIT_X_SEED_SHIFT           0
#define    FNG_CORE_TOP_SCCH_INIT_X_SEED_SCCH_INIT_X_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_SCCH_INIT_Y_SEED_OFFSET                              0x0000016C
#define FNG_CORE_TOP_SCCH_INIT_Y_SEED_TYPE                                UInt32
#define FNG_CORE_TOP_SCCH_INIT_Y_SEED_RESERVED_MASK                       0xFFFC0000
#define    FNG_CORE_TOP_SCCH_INIT_Y_SEED_SCCH_INIT_Y_SEED_SHIFT           0
#define    FNG_CORE_TOP_SCCH_INIT_Y_SEED_SCCH_INIT_Y_SEED_MASK            0x0003FFFF

#define FNG_CORE_TOP_CH7_CTRL1_OFFSET                                     0x00000170
#define FNG_CORE_TOP_CH7_CTRL1_TYPE                                       UInt32
#define FNG_CORE_TOP_CH7_CTRL1_RESERVED_MASK                              0x0000FEEE
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMCNT_SHIFT                    24
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMCNT_MASK                     0xFF000000
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMLEN_SHIFT                    20
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMLEN_MASK                     0x00F00000
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_TD_VPGAIN_SHIFT                     16
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_TD_VPGAIN_MASK                      0x000F0000
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_PILOT_SEL_SHIFT                     8
#define    FNG_CORE_TOP_CH7_CTRL1_CH7_PILOT_SEL_MASK                      0x00000100
#define    FNG_CORE_TOP_CH7_CTRL1_SW_CH7_STTD_SHIFT                       4
#define    FNG_CORE_TOP_CH7_CTRL1_SW_CH7_STTD_MASK                        0x00000010
#define    FNG_CORE_TOP_CH7_CTRL1_SW_CH7_EN_SHIFT                         0
#define    FNG_CORE_TOP_CH7_CTRL1_SW_CH7_EN_MASK                          0x00000001

#define FNG_CORE_TOP_CH7_CTRL2_OFFSET                                     0x00000174
#define FNG_CORE_TOP_CH7_CTRL2_TYPE                                       UInt32
#define FNG_CORE_TOP_CH7_CTRL2_RESERVED_MASK                              0x00F08F00
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_OFFSET_SHIFT                        24
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_OFFSET_MASK                         0xFF000000
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_VPGAIN_SHIFT                        16
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_VPGAIN_MASK                         0x000F0000
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_ACCLEN_SHIFT                        12
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_ACCLEN_MASK                         0x00007000
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_OVSF_SEED_SHIFT                     0
#define    FNG_CORE_TOP_CH7_CTRL2_CH7_OVSF_SEED_MASK                      0x000000FF

#define FNG_CORE_TOP_CH7_LOAD_OFFSET                                      0x00000178
#define FNG_CORE_TOP_CH7_LOAD_TYPE                                        UInt32
#define FNG_CORE_TOP_CH7_LOAD_RESERVED_MASK                               0xFFFFEEEE
#define    FNG_CORE_TOP_CH7_LOAD_CH7_DATA_TYPE_SHIFT                      12
#define    FNG_CORE_TOP_CH7_LOAD_CH7_DATA_TYPE_MASK                       0x00001000
#define    FNG_CORE_TOP_CH7_LOAD_CH7_WAKE_LOAD_SHIFT                      8
#define    FNG_CORE_TOP_CH7_LOAD_CH7_WAKE_LOAD_MASK                       0x00000100
#define    FNG_CORE_TOP_CH7_LOAD_CH7_AUTO_LOAD_SHIFT                      4
#define    FNG_CORE_TOP_CH7_LOAD_CH7_AUTO_LOAD_MASK                       0x00000010
#define    FNG_CORE_TOP_CH7_LOAD_CH7_PARA_LOAD_SHIFT                      0
#define    FNG_CORE_TOP_CH7_LOAD_CH7_PARA_LOAD_MASK                       0x00000001

#define FNG_CORE_TOP_AICH_CTRL_OFFSET                                     0x0000017C
#define FNG_CORE_TOP_AICH_CTRL_TYPE                                       UInt32
#define FNG_CORE_TOP_AICH_CTRL_RESERVED_MASK                              0xFFFFEE00
#define    FNG_CORE_TOP_AICH_CTRL_AICH_PILOT_SEL_SHIFT                    12
#define    FNG_CORE_TOP_AICH_CTRL_AICH_PILOT_SEL_MASK                     0x00001000
#define    FNG_CORE_TOP_AICH_CTRL_AICH_PWR_EST_EN_SHIFT                   8
#define    FNG_CORE_TOP_AICH_CTRL_AICH_PWR_EST_EN_MASK                    0x00000100
#define    FNG_CORE_TOP_AICH_CTRL_AICH_START_SHIFT                        0
#define    FNG_CORE_TOP_AICH_CTRL_AICH_START_MASK                         0x000000FF

#define FNG_CORE_TOP_AICH_PWR_EST_OFFSET                                  0x00000180
#define FNG_CORE_TOP_AICH_PWR_EST_TYPE                                    UInt32
#define FNG_CORE_TOP_AICH_PWR_EST_RESERVED_MASK                           0xFFF00000
#define    FNG_CORE_TOP_AICH_PWR_EST_AICH_PWR_EST_SHIFT                   0
#define    FNG_CORE_TOP_AICH_PWR_EST_AICH_PWR_EST_MASK                    0x000FFFFF

#define FNG_CORE_TOP_SBNE_CTRL_OFFSET                                     0x00000184
#define FNG_CORE_TOP_SBNE_CTRL_TYPE                                       UInt32
#define FNG_CORE_TOP_SBNE_CTRL_RESERVED_MASK                              0xFFEE000E
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_SPI_EN_SHIFT                       20
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_SPI_EN_MASK                        0x00100000
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_PPI_EN_SHIFT                       16
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_PPI_EN_MASK                        0x00010000
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_VP6_SHIFT                          12
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_VP6_MASK                           0x0000F000
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_VP5_SHIFT                          8
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_VP5_MASK                           0x00000F00
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_VP4_SHIFT                          4
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_VP4_MASK                           0x000000F0
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_EN_SHIFT                           0
#define    FNG_CORE_TOP_SBNE_CTRL_SBNE_EN_MASK                            0x00000001

#define FNG_CORE_TOP_SBNE_NOISE_OUT_OFFSET                                0x00000188
#define FNG_CORE_TOP_SBNE_NOISE_OUT_TYPE                                  UInt32
#define FNG_CORE_TOP_SBNE_NOISE_OUT_RESERVED_MASK                         0x0FFFC000
#define    FNG_CORE_TOP_SBNE_NOISE_OUT_SBNE_NOISE_SLOT_SHIFT              28
#define    FNG_CORE_TOP_SBNE_NOISE_OUT_SBNE_NOISE_SLOT_MASK               0xF0000000
#define    FNG_CORE_TOP_SBNE_NOISE_OUT_SBNE_NOISE_OUT_SHIFT               0
#define    FNG_CORE_TOP_SBNE_NOISE_OUT_SBNE_NOISE_OUT_MASK                0x00003FFF

#define FNG_CORE_TOP_SBNE_CHEST_OUT_OFFSET                                0x0000018C
#define FNG_CORE_TOP_SBNE_CHEST_OUT_TYPE                                  UInt32
#define FNG_CORE_TOP_SBNE_CHEST_OUT_RESERVED_MASK                         0x0FFF0000
#define    FNG_CORE_TOP_SBNE_CHEST_OUT_SBNE_CHEST_SLOT_SHIFT              28
#define    FNG_CORE_TOP_SBNE_CHEST_OUT_SBNE_CHEST_SLOT_MASK               0xF0000000
#define    FNG_CORE_TOP_SBNE_CHEST_OUT_SBNE_CHANNEL_OUT_SHIFT             0
#define    FNG_CORE_TOP_SBNE_CHEST_OUT_SBNE_CHANNEL_OUT_MASK              0x0000FFFF

#endif /* __BRCM_RDB_FNG_CORE_TOP_H__ */



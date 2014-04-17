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
/*     Date     : Generated on 3/4/2013 11:52:5                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_EDCH_FNG_CORE_TOP_H__
#define __BRCM_RDB_EDCH_FNG_CORE_TOP_H__

#define EDCH_FNG_CORE_TOP_CTRL_OFFSET                                     0x00000000
#define EDCH_FNG_CORE_TOP_CTRL_TYPE                                       UInt32
#define EDCH_FNG_CORE_TOP_CTRL_RESERVED_MASK                              0x208EEFCC
#define    EDCH_FNG_CORE_TOP_CTRL_CHEST_IIR_CLEAR_SHIFT                   31
#define    EDCH_FNG_CORE_TOP_CTRL_CHEST_IIR_CLEAR_MASK                    0x80000000
#define    EDCH_FNG_CORE_TOP_CTRL_RSCP_IIR_CLEAR_SHIFT                    30
#define    EDCH_FNG_CORE_TOP_CTRL_RSCP_IIR_CLEAR_MASK                     0x40000000
#define    EDCH_FNG_CORE_TOP_CTRL_RSCP_PILOT_SEL_SHIFT                    28
#define    EDCH_FNG_CORE_TOP_CTRL_RSCP_PILOT_SEL_MASK                     0x10000000
#define    EDCH_FNG_CORE_TOP_CTRL_RSCP_IIR_COEF_RB_001_SHIFT              25
#define    EDCH_FNG_CORE_TOP_CTRL_RSCP_IIR_COEF_RB_001_MASK               0x0E000000
#define    EDCH_FNG_CORE_TOP_CTRL_CHEST_HOLD_SHIFT                        24
#define    EDCH_FNG_CORE_TOP_CTRL_CHEST_HOLD_MASK                         0x01000000
#define    EDCH_FNG_CORE_TOP_CTRL_CHEST_IIR_COEF_RB_001_SHIFT             20
#define    EDCH_FNG_CORE_TOP_CTRL_CHEST_IIR_COEF_RB_001_MASK              0x00700000
#define    EDCH_FNG_CORE_TOP_CTRL_SW_CLK_EN_SHIFT                         16
#define    EDCH_FNG_CORE_TOP_CTRL_SW_CLK_EN_MASK                          0x00010000
#define    EDCH_FNG_CORE_TOP_CTRL_SW_CPI_STTD_SHIFT                       12
#define    EDCH_FNG_CORE_TOP_CTRL_SW_CPI_STTD_MASK                        0x00001000
#define    EDCH_FNG_CORE_TOP_CTRL_IQ_MSTTIMERGATE_SHIFT                   5
#define    EDCH_FNG_CORE_TOP_CTRL_IQ_MSTTIMERGATE_MASK                    0x00000020
#define    EDCH_FNG_CORE_TOP_CTRL_SW_FNG_EN_SHIFT                         4
#define    EDCH_FNG_CORE_TOP_CTRL_SW_FNG_EN_MASK                          0x00000010
#define    EDCH_FNG_CORE_TOP_CTRL_FNG_AUTO_LOAD_SHIFT                     1
#define    EDCH_FNG_CORE_TOP_CTRL_FNG_AUTO_LOAD_MASK                      0x00000002
#define    EDCH_FNG_CORE_TOP_CTRL_FNG_PARA_LOAD_SHIFT                     0
#define    EDCH_FNG_CORE_TOP_CTRL_FNG_PARA_LOAD_MASK                      0x00000001

#define EDCH_FNG_CORE_TOP_CURRENT_POS_OFFSET                              0x00000010
#define EDCH_FNG_CORE_TOP_CURRENT_POS_TYPE                                UInt32
#define EDCH_FNG_CORE_TOP_CURRENT_POS_RESERVED_MASK                       0xEEF08000
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_CLEAR_ERROR_FLAG_SHIFT           28
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_CLEAR_ERROR_FLAG_MASK            0x10000000
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_ERROR_FLAG_SHIFT                 24
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_ERROR_FLAG_MASK                  0x01000000
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_CURRENT_SLOTPOS_SHIFT            16
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_CURRENT_SLOTPOS_MASK             0x000F0000
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_CURRENT_SAMPLEPOS_SHIFT          0
#define    EDCH_FNG_CORE_TOP_CURRENT_POS_CURRENT_SAMPLEPOS_MASK           0x00007FFF

#define EDCH_FNG_CORE_TOP_WAKEUP_PHASE_OFFSET                             0x00000014
#define EDCH_FNG_CORE_TOP_WAKEUP_PHASE_TYPE                               UInt32
#define EDCH_FNG_CORE_TOP_WAKEUP_PHASE_RESERVED_MASK                      0xFFFF0000
#define    EDCH_FNG_CORE_TOP_WAKEUP_PHASE_WAKEUP_PHASE_SHIFT              0
#define    EDCH_FNG_CORE_TOP_WAKEUP_PHASE_WAKEUP_PHASE_MASK               0x0000FFFF

#define EDCH_FNG_CORE_TOP_WAKEUP_POS_OFFSET                               0x00000018
#define EDCH_FNG_CORE_TOP_WAKEUP_POS_TYPE                                 UInt32
#define EDCH_FNG_CORE_TOP_WAKEUP_POS_RESERVED_MASK                        0xFFF0C000
#define    EDCH_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SLOTPOS_SHIFT              16
#define    EDCH_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SLOTPOS_MASK               0x000F0000
#define    EDCH_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SAMPLEPOS_SHIFT            0
#define    EDCH_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SAMPLEPOS_MASK             0x00003FFF

#define EDCH_FNG_CORE_TOP_PPI_CTRL_OFFSET                                 0x0000001C
#define EDCH_FNG_CORE_TOP_PPI_CTRL_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_PPI_CTRL_RESERVED_MASK                          0x0000000E
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_ANT2_PPI_RSCP_SHIFT                 24
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_ANT2_PPI_RSCP_MASK                  0xFF000000
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_ANT1_PPI_RSCP_SHIFT                 16
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_ANT1_PPI_RSCP_MASK                  0x00FF0000
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_PPI_OVSF_SEED_SHIFT                 8
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_PPI_OVSF_SEED_MASK                  0x0000FF00
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_PPI_ACCLEN_SHIFT                    4
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_PPI_ACCLEN_MASK                     0x000000F0
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_SW_PPI_EN_SHIFT                     0
#define    EDCH_FNG_CORE_TOP_PPI_CTRL_SW_PPI_EN_MASK                      0x00000001

#define EDCH_FNG_CORE_TOP_PPI_WAKE_X_SEED_OFFSET                          0x00000020
#define EDCH_FNG_CORE_TOP_PPI_WAKE_X_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_PPI_WAKE_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_PPI_WAKE_X_SEED_PPI_WAKE_X_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_PPI_WAKE_X_SEED_PPI_WAKE_X_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_PPI_WAKE_Y_SEED_OFFSET                          0x00000024
#define EDCH_FNG_CORE_TOP_PPI_WAKE_Y_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_PPI_WAKE_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_PPI_WAKE_Y_SEED_PPI_WAKE_Y_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_PPI_WAKE_Y_SEED_PPI_WAKE_Y_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_PPI_INIT_X_SEED_OFFSET                          0x00000028
#define EDCH_FNG_CORE_TOP_PPI_INIT_X_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_PPI_INIT_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_PPI_INIT_X_SEED_PPI_INIT_X_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_PPI_INIT_X_SEED_PPI_INIT_X_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_PPI_INIT_Y_SEED_OFFSET                          0x0000002C
#define EDCH_FNG_CORE_TOP_PPI_INIT_Y_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_PPI_INIT_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_PPI_INIT_Y_SEED_PPI_INIT_Y_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_PPI_INIT_Y_SEED_PPI_INIT_Y_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_SPI_CTRL_OFFSET                                 0x00000034
#define EDCH_FNG_CORE_TOP_SPI_CTRL_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_SPI_CTRL_RESERVED_MASK                          0x0000000E
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_ANT2_SPI_RSCP_SHIFT                 24
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_ANT2_SPI_RSCP_MASK                  0xFF000000
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_ANT1_SPI_RSCP_SHIFT                 16
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_ANT1_SPI_RSCP_MASK                  0x00FF0000
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_SPI_OVSF_SEED_SHIFT                 8
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_SPI_OVSF_SEED_MASK                  0x0000FF00
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_SPI_ACCLEN_SHIFT                    4
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_SPI_ACCLEN_MASK                     0x000000F0
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_SW_SPI_EN_SHIFT                     0
#define    EDCH_FNG_CORE_TOP_SPI_CTRL_SW_SPI_EN_MASK                      0x00000001

#define EDCH_FNG_CORE_TOP_SPI_WAKE_X_SEED_OFFSET                          0x00000038
#define EDCH_FNG_CORE_TOP_SPI_WAKE_X_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_SPI_WAKE_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SPI_WAKE_X_SEED_SPI_WAKE_X_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_SPI_WAKE_X_SEED_SPI_WAKE_X_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_SPI_WAKE_Y_SEED_OFFSET                          0x0000003C
#define EDCH_FNG_CORE_TOP_SPI_WAKE_Y_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_SPI_WAKE_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SPI_WAKE_Y_SEED_SPI_WAKE_Y_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_SPI_WAKE_Y_SEED_SPI_WAKE_Y_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_SPI_INIT_X_SEED_OFFSET                          0x00000040
#define EDCH_FNG_CORE_TOP_SPI_INIT_X_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_SPI_INIT_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SPI_INIT_X_SEED_SPI_INIT_X_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_SPI_INIT_X_SEED_SPI_INIT_X_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_SPI_INIT_Y_SEED_OFFSET                          0x00000044
#define EDCH_FNG_CORE_TOP_SPI_INIT_Y_SEED_TYPE                            UInt32
#define EDCH_FNG_CORE_TOP_SPI_INIT_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SPI_INIT_Y_SEED_SPI_INIT_Y_SEED_SHIFT        0
#define    EDCH_FNG_CORE_TOP_SPI_INIT_Y_SEED_SPI_INIT_Y_SEED_MASK         0x0003FFFF

#define EDCH_FNG_CORE_TOP_DLL_CTRL1_OFFSET                                0x00000090
#define EDCH_FNG_CORE_TOP_DLL_CTRL1_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_DLL_CTRL1_RESERVED_MASK                         0xEE06800E
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_RET_EN_SHIFT                28
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_RET_EN_MASK                 0x10000000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_ADV_EN_SHIFT                24
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_ADV_EN_MASK                 0x01000000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_CMD_NO_SHIFT                20
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_CMD_NO_MASK                 0x00F00000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_UNFREEZE_BIT_SHIFT             19
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_UNFREEZE_BIT_MASK              0x00080000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_HW_FREEZE_SHIFT                16
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_HW_FREEZE_MASK                 0x00010000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_MASTER_SHIFT                   12
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_MASTER_MASK                    0x00007000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_VPGAIN_SHIFT                   8
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_VPGAIN_MASK                    0x00000F00
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_ACCLEN_SHIFT                   4
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_ACCLEN_MASK                    0x000000F0
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_PILOT_SEL_SHIFT                0
#define    EDCH_FNG_CORE_TOP_DLL_CTRL1_DLL_PILOT_SEL_MASK                 0x00000001

#define EDCH_FNG_CORE_TOP_DLL_CTRL2_OFFSET                                0x00000094
#define EDCH_FNG_CORE_TOP_DLL_CTRL2_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_DLL_CTRL2_RESERVED_MASK                         0xFFFFEEEE
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_DLL_DECLEN_SHIFT                   12
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_DLL_DECLEN_MASK                    0x00001000
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_DLL_NEW_EN_SHIFT                   8
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_DLL_NEW_EN_MASK                    0x00000100
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_DLL_HOLD_SHIFT                     4
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_DLL_HOLD_MASK                      0x00000010
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_SW_DLL_EN_SHIFT                    0
#define    EDCH_FNG_CORE_TOP_DLL_CTRL2_SW_DLL_EN_MASK                     0x00000001

#define EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_OFFSET                            0x00000098
#define EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_TYPE                              UInt32
#define EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_RESERVED_MASK                     0x00000000
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC1_RSCP_SHIFT            16
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC1_RSCP_MASK             0xFFFF0000
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC0_RSCP_SHIFT            0
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC0_RSCP_MASK             0x0000FFFF

#define EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_OFFSET                            0x0000009C
#define EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_TYPE                              UInt32
#define EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_RESERVED_MASK                     0x00000000
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC3_RSCP_SHIFT            16
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC3_RSCP_MASK             0xFFFF0000
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC2_RSCP_SHIFT            0
#define    EDCH_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC2_RSCP_MASK             0x0000FFFF

#define EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_OFFSET                             0x000000B8
#define EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_TYPE                               UInt32
#define EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_RESERVED_MASK                      0x00000000
#define    EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_DLL_FAT_FNG_COMB_SHIFT          24
#define    EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_DLL_FAT_FNG_COMB_MASK           0xFF000000
#define    EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_DLL_PWR_DIFF_SHIFT              0
#define    EDCH_FNG_CORE_TOP_DLL_PWR_DIFF_DLL_PWR_DIFF_MASK               0x00FFFFFF

#define EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_OFFSET                         0x000000DC
#define EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_RESERVED_MASK                  0x00000000
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_Q_PNODM_SHIFT    24
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_Q_PNODM_MASK     0xFF000000
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_I_PNODM_SHIFT    16
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT2_CHEST_I_PNODM_MASK     0x00FF0000
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_Q_PNODM_SHIFT    8
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_Q_PNODM_MASK     0x0000FF00
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_I_PNODM_SHIFT    0
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_PNODM_ANT1_CHEST_I_PNODM_MASK     0x000000FF

#define EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_OFFSET                         0x000000E0
#define EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_RESERVED_MASK                  0x00000000
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_Q_SNODM_SHIFT    24
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_Q_SNODM_MASK     0xFF000000
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_I_SNODM_SHIFT    16
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT2_CHEST_I_SNODM_MASK     0x00FF0000
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_Q_SNODM_SHIFT    8
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_Q_SNODM_MASK     0x0000FF00
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_I_SNODM_SHIFT    0
#define    EDCH_FNG_CORE_TOP_ANT1_CHEST_SNODM_ANT1_CHEST_I_SNODM_MASK     0x000000FF

#define EDCH_FNG_CORE_TOP_LPF_OFFSET                                      0x000000E4
#define EDCH_FNG_CORE_TOP_LPF_TYPE                                        UInt32
#define EDCH_FNG_CORE_TOP_LPF_RESERVED_MASK                               0xFFFFFCCE
#define    EDCH_FNG_CORE_TOP_LPF_LPF_NODM_SEL_SHIFT                       8
#define    EDCH_FNG_CORE_TOP_LPF_LPF_NODM_SEL_MASK                        0x00000300
#define    EDCH_FNG_CORE_TOP_LPF_LPF_DMCH_SEL_SHIFT                       4
#define    EDCH_FNG_CORE_TOP_LPF_LPF_DMCH_SEL_MASK                        0x00000030
#define    EDCH_FNG_CORE_TOP_LPF_SW_DM_ENA_SHIFT                          0
#define    EDCH_FNG_CORE_TOP_LPF_SW_DM_ENA_MASK                           0x00000001

#define EDCH_FNG_CORE_TOP_CH4_LOAD_OFFSET                                 0x000000EC
#define EDCH_FNG_CORE_TOP_CH4_LOAD_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_CH4_LOAD_RESERVED_MASK                          0xFFFFFEEE
#define    EDCH_FNG_CORE_TOP_CH4_LOAD_CH4_WAKE_LOAD_SHIFT                 8
#define    EDCH_FNG_CORE_TOP_CH4_LOAD_CH4_WAKE_LOAD_MASK                  0x00000100
#define    EDCH_FNG_CORE_TOP_CH4_LOAD_CH4_AUTO_LOAD_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH4_LOAD_CH4_AUTO_LOAD_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_CH4_LOAD_CH4_PARA_LOAD_SHIFT                 0
#define    EDCH_FNG_CORE_TOP_CH4_LOAD_CH4_PARA_LOAD_MASK                  0x00000001

#define EDCH_FNG_CORE_TOP_CH5_LOAD_OFFSET                                 0x000000F0
#define EDCH_FNG_CORE_TOP_CH5_LOAD_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_CH5_LOAD_RESERVED_MASK                          0xFFFFFEEE
#define    EDCH_FNG_CORE_TOP_CH5_LOAD_CH5_LOAD_MASK_SHIFT                 8
#define    EDCH_FNG_CORE_TOP_CH5_LOAD_CH5_LOAD_MASK_MASK                  0x00000100
#define    EDCH_FNG_CORE_TOP_CH5_LOAD_CH5_AUTO_LOAD_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH5_LOAD_CH5_AUTO_LOAD_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_CH5_LOAD_CH5_PARA_LOAD_SHIFT                 0
#define    EDCH_FNG_CORE_TOP_CH5_LOAD_CH5_PARA_LOAD_MASK                  0x00000001

#define EDCH_FNG_CORE_TOP_CH6_LOAD_OFFSET                                 0x000000F4
#define EDCH_FNG_CORE_TOP_CH6_LOAD_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_CH6_LOAD_RESERVED_MASK                          0xFFFFFEEE
#define    EDCH_FNG_CORE_TOP_CH6_LOAD_CH6_LOAD_MASK_SHIFT                 8
#define    EDCH_FNG_CORE_TOP_CH6_LOAD_CH6_LOAD_MASK_MASK                  0x00000100
#define    EDCH_FNG_CORE_TOP_CH6_LOAD_CH6_AUTO_LOAD_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH6_LOAD_CH6_AUTO_LOAD_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_CH6_LOAD_CH6_PARA_LOAD_SHIFT                 0
#define    EDCH_FNG_CORE_TOP_CH6_LOAD_CH6_PARA_LOAD_MASK                  0x00000001

#define EDCH_FNG_CORE_TOP_HSUPA_CTRL_OFFSET                               0x000000F8
#define EDCH_FNG_CORE_TOP_HSUPA_CTRL_TYPE                                 UInt32
#define EDCH_FNG_CORE_TOP_HSUPA_CTRL_RESERVED_MASK                        0x80000EEE
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_NOISE_GAIN_SEL_SHIFT              30
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_NOISE_GAIN_SEL_MASK               0x40000000
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_CHEST_GAIN_SEL_SHIFT              29
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_CHEST_GAIN_SEL_MASK               0x20000000
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_SW_CPM_EN_SHIFT                   28
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_SW_CPM_EN_MASK                    0x10000000
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_NPE_VPGAIN_SHIFT                  24
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_NPE_VPGAIN_MASK                   0x0F000000
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_ECH_OVSF_SEED_SHIFT               16
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_ECH_OVSF_SEED_MASK                0x00FF0000
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_ECH_VPGAIN_SHIFT                  12
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_ECH_VPGAIN_MASK                   0x0000F000
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_ECH_PILOT_SEL_SHIFT               8
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_ECH_PILOT_SEL_MASK                0x00000100
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_SW_ECH_STTD_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_SW_ECH_STTD_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_SW_NPE_EN_SHIFT                   0
#define    EDCH_FNG_CORE_TOP_HSUPA_CTRL_SW_NPE_EN_MASK                    0x00000001

#define EDCH_FNG_CORE_TOP_CH4_CTRL1_OFFSET                                0x000000FC
#define EDCH_FNG_CORE_TOP_CH4_CTRL1_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_CH4_CTRL1_RESERVED_MASK                         0xF0000EEE
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_OVSF_SEED_SHIFT                20
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_OVSF_SEED_MASK                 0x0FF00000
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_TD_VPGAIN_SHIFT                16
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_TD_VPGAIN_MASK                 0x000F0000
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_VPGAIN_SHIFT                   12
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_VPGAIN_MASK                    0x0000F000
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_PILOT_SEL_SHIFT                8
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_PILOT_SEL_MASK                 0x00000100
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_TTI_SIZE_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_CH4_TTI_SIZE_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_SW_CH4_EN_SHIFT                    0
#define    EDCH_FNG_CORE_TOP_CH4_CTRL1_SW_CH4_EN_MASK                     0x00000001

#define EDCH_FNG_CORE_TOP_CH5_CTRL1_OFFSET                                0x00000100
#define EDCH_FNG_CORE_TOP_CH5_CTRL1_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_CH5_CTRL1_RESERVED_MASK                         0xFF00C0CE
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_CH5_OFFSET_SHIFT                   16
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_CH5_OFFSET_MASK                    0x00FF0000
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_CH5_SEQ_INDEX_SHIFT                8
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_CH5_SEQ_INDEX_MASK                 0x00003F00
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_CH5_TTI_SIZE_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_CH5_TTI_SIZE_MASK                  0x00000030
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_SW_CH5_EN_SHIFT                    0
#define    EDCH_FNG_CORE_TOP_CH5_CTRL1_SW_CH5_EN_MASK                     0x00000001

#define EDCH_FNG_CORE_TOP_CH6_CTRL1_OFFSET                                0x00000104
#define EDCH_FNG_CORE_TOP_CH6_CTRL1_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_CH6_CTRL1_RESERVED_MASK                         0xFF00C0EE
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_CH6_OFFSET_SHIFT                   16
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_CH6_OFFSET_MASK                    0x00FF0000
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_CH6_SEQ_INDEX_SHIFT                8
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_CH6_SEQ_INDEX_MASK                 0x00003F00
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_CH6_TTI_SIZE_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_CH6_TTI_SIZE_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_SW_CH6_EN_SHIFT                    0
#define    EDCH_FNG_CORE_TOP_CH6_CTRL1_SW_CH6_EN_MASK                     0x00000001

#define EDCH_FNG_CORE_TOP_ERGCH_SD_OFFSET                                 0x00000108
#define EDCH_FNG_CORE_TOP_ERGCH_SD_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_ERGCH_SD_RESERVED_MASK                          0x80000000
#define    EDCH_FNG_CORE_TOP_ERGCH_SD_ERGCH_NOISE_PWR_SHIFT               16
#define    EDCH_FNG_CORE_TOP_ERGCH_SD_ERGCH_NOISE_PWR_MASK                0x7FFF0000
#define    EDCH_FNG_CORE_TOP_ERGCH_SD_ERGCH_SOFT_DATA_SHIFT               0
#define    EDCH_FNG_CORE_TOP_ERGCH_SD_ERGCH_SOFT_DATA_MASK                0x0000FFFF

#define EDCH_FNG_CORE_TOP_EHICH_SD_OFFSET                                 0x0000010C
#define EDCH_FNG_CORE_TOP_EHICH_SD_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_EHICH_SD_RESERVED_MASK                          0x80000000
#define    EDCH_FNG_CORE_TOP_EHICH_SD_EHICH_NOISE_PWR_SHIFT               16
#define    EDCH_FNG_CORE_TOP_EHICH_SD_EHICH_NOISE_PWR_MASK                0x7FFF0000
#define    EDCH_FNG_CORE_TOP_EHICH_SD_EHICH_SOFT_DATA_SHIFT               0
#define    EDCH_FNG_CORE_TOP_EHICH_SD_EHICH_SOFT_DATA_MASK                0x0000FFFF

#define EDCH_FNG_CORE_TOP_CPM_START_OFFSET                                0x00000118
#define EDCH_FNG_CORE_TOP_CPM_START_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_CPM_START_RESERVED_MASK                         0x0000C000
#define    EDCH_FNG_CORE_TOP_CPM_START_CPM_START_FRAMEPOS_SHIFT           20
#define    EDCH_FNG_CORE_TOP_CPM_START_CPM_START_FRAMEPOS_MASK            0xFFF00000
#define    EDCH_FNG_CORE_TOP_CPM_START_CPM_START_SLOTPOS_SHIFT            16
#define    EDCH_FNG_CORE_TOP_CPM_START_CPM_START_SLOTPOS_MASK             0x000F0000
#define    EDCH_FNG_CORE_TOP_CPM_START_CPM_START_SAMPLEPOS_SHIFT          0
#define    EDCH_FNG_CORE_TOP_CPM_START_CPM_START_SAMPLEPOS_MASK           0x00003FFF

#define EDCH_FNG_CORE_TOP_CPM_STOP_OFFSET                                 0x0000011C
#define EDCH_FNG_CORE_TOP_CPM_STOP_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_CPM_STOP_RESERVED_MASK                          0x0000C000
#define    EDCH_FNG_CORE_TOP_CPM_STOP_CPM_STOP_FRAMEPOS_SHIFT             20
#define    EDCH_FNG_CORE_TOP_CPM_STOP_CPM_STOP_FRAMEPOS_MASK              0xFFF00000
#define    EDCH_FNG_CORE_TOP_CPM_STOP_CPM_STOP_SLOTPOS_SHIFT              16
#define    EDCH_FNG_CORE_TOP_CPM_STOP_CPM_STOP_SLOTPOS_MASK               0x000F0000
#define    EDCH_FNG_CORE_TOP_CPM_STOP_CPM_STOP_SAMPLEPOS_SHIFT            0
#define    EDCH_FNG_CORE_TOP_CPM_STOP_CPM_STOP_SAMPLEPOS_MASK             0x00003FFF

#define EDCH_FNG_CORE_TOP_DLL_THRESHOLD_OFFSET                            0x00000120
#define EDCH_FNG_CORE_TOP_DLL_THRESHOLD_TYPE                              UInt32
#define EDCH_FNG_CORE_TOP_DLL_THRESHOLD_RESERVED_MASK                     0xEF000000
#define    EDCH_FNG_CORE_TOP_DLL_THRESHOLD_DLL_ACC_CLEAR_SHIFT            28
#define    EDCH_FNG_CORE_TOP_DLL_THRESHOLD_DLL_ACC_CLEAR_MASK             0x10000000
#define    EDCH_FNG_CORE_TOP_DLL_THRESHOLD_DLL_THRESHOLD_SHIFT            0
#define    EDCH_FNG_CORE_TOP_DLL_THRESHOLD_DLL_THRESHOLD_MASK             0x00FFFFFF

#define EDCH_FNG_CORE_TOP_AGCH_WAKE_X_SEED_OFFSET                         0x00000150
#define EDCH_FNG_CORE_TOP_AGCH_WAKE_X_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_AGCH_WAKE_X_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_AGCH_WAKE_X_SEED_AGCH_WAKE_X_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_AGCH_WAKE_X_SEED_AGCH_WAKE_X_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_AGCH_WAKE_Y_SEED_OFFSET                         0x00000154
#define EDCH_FNG_CORE_TOP_AGCH_WAKE_Y_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_AGCH_WAKE_Y_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_AGCH_WAKE_Y_SEED_AGCH_WAKE_Y_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_AGCH_WAKE_Y_SEED_AGCH_WAKE_Y_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_AGCH_INIT_X_SEED_OFFSET                         0x00000158
#define EDCH_FNG_CORE_TOP_AGCH_INIT_X_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_AGCH_INIT_X_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_AGCH_INIT_X_SEED_AGCH_INIT_X_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_AGCH_INIT_X_SEED_AGCH_INIT_X_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_AGCH_INIT_Y_SEED_OFFSET                         0x0000015C
#define EDCH_FNG_CORE_TOP_AGCH_INIT_Y_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_AGCH_INIT_Y_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_AGCH_INIT_Y_SEED_AGCH_INIT_Y_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_AGCH_INIT_Y_SEED_AGCH_INIT_Y_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_SCCH_WAKE_X_SEED_OFFSET                         0x00000160
#define EDCH_FNG_CORE_TOP_SCCH_WAKE_X_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_SCCH_WAKE_X_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SCCH_WAKE_X_SEED_SCCH_WAKE_X_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_SCCH_WAKE_X_SEED_SCCH_WAKE_X_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_SCCH_WAKE_Y_SEED_OFFSET                         0x00000164
#define EDCH_FNG_CORE_TOP_SCCH_WAKE_Y_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_SCCH_WAKE_Y_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SCCH_WAKE_Y_SEED_SCCH_WAKE_Y_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_SCCH_WAKE_Y_SEED_SCCH_WAKE_Y_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_SCCH_INIT_X_SEED_OFFSET                         0x00000168
#define EDCH_FNG_CORE_TOP_SCCH_INIT_X_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_SCCH_INIT_X_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SCCH_INIT_X_SEED_SCCH_INIT_X_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_SCCH_INIT_X_SEED_SCCH_INIT_X_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_SCCH_INIT_Y_SEED_OFFSET                         0x0000016C
#define EDCH_FNG_CORE_TOP_SCCH_INIT_Y_SEED_TYPE                           UInt32
#define EDCH_FNG_CORE_TOP_SCCH_INIT_Y_SEED_RESERVED_MASK                  0xFFFC0000
#define    EDCH_FNG_CORE_TOP_SCCH_INIT_Y_SEED_SCCH_INIT_Y_SEED_SHIFT      0
#define    EDCH_FNG_CORE_TOP_SCCH_INIT_Y_SEED_SCCH_INIT_Y_SEED_MASK       0x0003FFFF

#define EDCH_FNG_CORE_TOP_CH7_CTRL1_OFFSET                                0x00000170
#define EDCH_FNG_CORE_TOP_CH7_CTRL1_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_CH7_CTRL1_RESERVED_MASK                         0x0000FEEE
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMCNT_SHIFT               24
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMCNT_MASK                0xFF000000
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMLEN_SHIFT               20
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_OVF_SYMLEN_MASK                0x00F00000
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_TD_VPGAIN_SHIFT                16
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_TD_VPGAIN_MASK                 0x000F0000
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_PILOT_SEL_SHIFT                8
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_CH7_PILOT_SEL_MASK                 0x00000100
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_SW_CH7_STTD_SHIFT                  4
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_SW_CH7_STTD_MASK                   0x00000010
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_SW_CH7_EN_SHIFT                    0
#define    EDCH_FNG_CORE_TOP_CH7_CTRL1_SW_CH7_EN_MASK                     0x00000001

#define EDCH_FNG_CORE_TOP_CH7_CTRL2_OFFSET                                0x00000174
#define EDCH_FNG_CORE_TOP_CH7_CTRL2_TYPE                                  UInt32
#define EDCH_FNG_CORE_TOP_CH7_CTRL2_RESERVED_MASK                         0x00F08F00
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_OFFSET_SHIFT                   24
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_OFFSET_MASK                    0xFF000000
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_VPGAIN_SHIFT                   16
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_VPGAIN_MASK                    0x000F0000
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_ACCLEN_SHIFT                   12
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_ACCLEN_MASK                    0x00007000
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_OVSF_SEED_SHIFT                0
#define    EDCH_FNG_CORE_TOP_CH7_CTRL2_CH7_OVSF_SEED_MASK                 0x000000FF

#define EDCH_FNG_CORE_TOP_CH7_LOAD_OFFSET                                 0x00000178
#define EDCH_FNG_CORE_TOP_CH7_LOAD_TYPE                                   UInt32
#define EDCH_FNG_CORE_TOP_CH7_LOAD_RESERVED_MASK                          0xFFFFEEEE
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_DATA_TYPE_SHIFT                 12
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_DATA_TYPE_MASK                  0x00001000
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_WAKE_LOAD_SHIFT                 8
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_WAKE_LOAD_MASK                  0x00000100
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_AUTO_LOAD_SHIFT                 4
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_AUTO_LOAD_MASK                  0x00000010
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_PARA_LOAD_SHIFT                 0
#define    EDCH_FNG_CORE_TOP_CH7_LOAD_CH7_PARA_LOAD_MASK                  0x00000001

#endif /* __BRCM_RDB_EDCH_FNG_CORE_TOP_H__ */



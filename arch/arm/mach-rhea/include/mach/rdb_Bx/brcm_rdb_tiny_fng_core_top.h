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
/*     Date     : Generated on 2/10/2012 15:44:22                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_TINY_FNG_CORE_TOP_H__
#define __BRCM_RDB_TINY_FNG_CORE_TOP_H__

#define TINY_FNG_CORE_TOP_CTRL_OFFSET                                     0x00000000
#define TINY_FNG_CORE_TOP_CTRL_TYPE                                       UInt32
#define TINY_FNG_CORE_TOP_CTRL_RESERVED_MASK                              0x0EEEEEEE
#define    TINY_FNG_CORE_TOP_CTRL_IIR_CLEAR_SHIFT                         31
#define    TINY_FNG_CORE_TOP_CTRL_IIR_CLEAR_MASK                          0x80000000
#define    TINY_FNG_CORE_TOP_CTRL_IIR_PROG_COEF_RB_001_SHIFT              28
#define    TINY_FNG_CORE_TOP_CTRL_IIR_PROG_COEF_RB_001_MASK               0x70000000
#define    TINY_FNG_CORE_TOP_CTRL_ADV_GATE_CLK_SHIFT                      24
#define    TINY_FNG_CORE_TOP_CTRL_ADV_GATE_CLK_MASK                       0x01000000
#define    TINY_FNG_CORE_TOP_CTRL_FNG_AUTO_LOAD_SHIFT                     20
#define    TINY_FNG_CORE_TOP_CTRL_FNG_AUTO_LOAD_MASK                      0x00100000
#define    TINY_FNG_CORE_TOP_CTRL_SW_CLK_EN_SHIFT                         16
#define    TINY_FNG_CORE_TOP_CTRL_SW_CLK_EN_MASK                          0x00010000
#define    TINY_FNG_CORE_TOP_CTRL_SW_CPI_STTD_SHIFT                       12
#define    TINY_FNG_CORE_TOP_CTRL_SW_CPI_STTD_MASK                        0x00001000
#define    TINY_FNG_CORE_TOP_CTRL_CCPCH_SEL_SHIFT                         8
#define    TINY_FNG_CORE_TOP_CTRL_CCPCH_SEL_MASK                          0x00000100
#define    TINY_FNG_CORE_TOP_CTRL_SW_FNG_EN_SHIFT                         4
#define    TINY_FNG_CORE_TOP_CTRL_SW_FNG_EN_MASK                          0x00000010
#define    TINY_FNG_CORE_TOP_CTRL_FNG_PARA_LOAD_SHIFT                     0
#define    TINY_FNG_CORE_TOP_CTRL_FNG_PARA_LOAD_MASK                      0x00000001

#define TINY_FNG_CORE_TOP_CCP_LOAD_OFFSET                                 0x00000004
#define TINY_FNG_CORE_TOP_CCP_LOAD_TYPE                                   UInt32
#define TINY_FNG_CORE_TOP_CCP_LOAD_RESERVED_MASK                          0xFFFFFFEE
#define    TINY_FNG_CORE_TOP_CCP_LOAD_CCP_AUTO_LOAD_SHIFT                 4
#define    TINY_FNG_CORE_TOP_CCP_LOAD_CCP_AUTO_LOAD_MASK                  0x00000010
#define    TINY_FNG_CORE_TOP_CCP_LOAD_CCP_PARA_LOAD_SHIFT                 0
#define    TINY_FNG_CORE_TOP_CCP_LOAD_CCP_PARA_LOAD_MASK                  0x00000001

#define TINY_FNG_CORE_TOP_CURRENT_POS_OFFSET                              0x00000008
#define TINY_FNG_CORE_TOP_CURRENT_POS_TYPE                                UInt32
#define TINY_FNG_CORE_TOP_CURRENT_POS_RESERVED_MASK                       0xFFF08000
#define    TINY_FNG_CORE_TOP_CURRENT_POS_CURRENT_SLOTPOS_SHIFT            16
#define    TINY_FNG_CORE_TOP_CURRENT_POS_CURRENT_SLOTPOS_MASK             0x000F0000
#define    TINY_FNG_CORE_TOP_CURRENT_POS_CURRENT_SAMPLEPOS_SHIFT          0
#define    TINY_FNG_CORE_TOP_CURRENT_POS_CURRENT_SAMPLEPOS_MASK           0x00007FFF

#define TINY_FNG_CORE_TOP_WAKEUP_PHASE_OFFSET                             0x0000000C
#define TINY_FNG_CORE_TOP_WAKEUP_PHASE_TYPE                               UInt32
#define TINY_FNG_CORE_TOP_WAKEUP_PHASE_RESERVED_MASK                      0xFFFF0000
#define    TINY_FNG_CORE_TOP_WAKEUP_PHASE_WAKEUP_PHASE_SHIFT              0
#define    TINY_FNG_CORE_TOP_WAKEUP_PHASE_WAKEUP_PHASE_MASK               0x0000FFFF

#define TINY_FNG_CORE_TOP_WAKEUP_POS_OFFSET                               0x00000010
#define TINY_FNG_CORE_TOP_WAKEUP_POS_TYPE                                 UInt32
#define TINY_FNG_CORE_TOP_WAKEUP_POS_RESERVED_MASK                        0xFFF0C000
#define    TINY_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SLOTPOS_SHIFT              16
#define    TINY_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SLOTPOS_MASK               0x000F0000
#define    TINY_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SAMPLEPOS_SHIFT            0
#define    TINY_FNG_CORE_TOP_WAKEUP_POS_WAKEUP_SAMPLEPOS_MASK             0x00003FFF

#define TINY_FNG_CORE_TOP_PPI_CTRL_OFFSET                                 0x00000014
#define TINY_FNG_CORE_TOP_PPI_CTRL_TYPE                                   UInt32
#define TINY_FNG_CORE_TOP_PPI_CTRL_RESERVED_MASK                          0x0000FE0E
#define    TINY_FNG_CORE_TOP_PPI_CTRL_A2_PILOT_RSCP_SHIFT                 24
#define    TINY_FNG_CORE_TOP_PPI_CTRL_A2_PILOT_RSCP_MASK                  0xFF000000
#define    TINY_FNG_CORE_TOP_PPI_CTRL_A1_PILOT_RSCP_SHIFT                 16
#define    TINY_FNG_CORE_TOP_PPI_CTRL_A1_PILOT_RSCP_MASK                  0x00FF0000
#define    TINY_FNG_CORE_TOP_PPI_CTRL_BYPASS_IIR_SHIFT                    8
#define    TINY_FNG_CORE_TOP_PPI_CTRL_BYPASS_IIR_MASK                     0x00000100
#define    TINY_FNG_CORE_TOP_PPI_CTRL_PPI_ACCLEN_SHIFT                    4
#define    TINY_FNG_CORE_TOP_PPI_CTRL_PPI_ACCLEN_MASK                     0x000000F0
#define    TINY_FNG_CORE_TOP_PPI_CTRL_SW_PPI_EN_SHIFT                     0
#define    TINY_FNG_CORE_TOP_PPI_CTRL_SW_PPI_EN_MASK                      0x00000001

#define TINY_FNG_CORE_TOP_PPI_WAKE_X_SEED_OFFSET                          0x00000018
#define TINY_FNG_CORE_TOP_PPI_WAKE_X_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_PPI_WAKE_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_PPI_WAKE_X_SEED_PPI_WAKE_X_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_PPI_WAKE_X_SEED_PPI_WAKE_X_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_PPI_WAKE_Y_SEED_OFFSET                          0x0000001C
#define TINY_FNG_CORE_TOP_PPI_WAKE_Y_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_PPI_WAKE_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_PPI_WAKE_Y_SEED_PPI_WAKE_Y_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_PPI_WAKE_Y_SEED_PPI_WAKE_Y_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_PPI_INIT_X_SEED_OFFSET                          0x00000020
#define TINY_FNG_CORE_TOP_PPI_INIT_X_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_PPI_INIT_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_PPI_INIT_X_SEED_PPI_INIT_X_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_PPI_INIT_X_SEED_PPI_INIT_X_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_PPI_INIT_Y_SEED_OFFSET                          0x00000024
#define TINY_FNG_CORE_TOP_PPI_INIT_Y_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_PPI_INIT_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_PPI_INIT_Y_SEED_PPI_INIT_Y_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_PPI_INIT_Y_SEED_PPI_INIT_Y_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_CHEST_DATA_OFFSET                               0x00000028
#define TINY_FNG_CORE_TOP_CHEST_DATA_TYPE                                 UInt32
#define TINY_FNG_CORE_TOP_CHEST_DATA_RESERVED_MASK                        0x00000000
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A2_CHEST_Q_DATA_SHIFT             24
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A2_CHEST_Q_DATA_MASK              0xFF000000
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A2_CHEST_I_DATA_SHIFT             16
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A2_CHEST_I_DATA_MASK              0x00FF0000
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A1_CHEST_Q_DATA_SHIFT             8
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A1_CHEST_Q_DATA_MASK              0x0000FF00
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A1_CHEST_I_DATA_SHIFT             0
#define    TINY_FNG_CORE_TOP_CHEST_DATA_A1_CHEST_I_DATA_MASK              0x000000FF

#define TINY_FNG_CORE_TOP_CCP_CTRL1_OFFSET                                0x0000002C
#define TINY_FNG_CORE_TOP_CCP_CTRL1_TYPE                                  UInt32
#define TINY_FNG_CORE_TOP_CCP_CTRL1_RESERVED_MASK                         0x0000FFEE
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_CCP_OVF_SYMCNT_SHIFT               24
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_CCP_OVF_SYMCNT_MASK                0xFF000000
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_CCP_OVF_SYMLEN_SHIFT               20
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_CCP_OVF_SYMLEN_MASK                0x00F00000
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_CCP_TD_VPGAIN_SHIFT                16
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_CCP_TD_VPGAIN_MASK                 0x000F0000
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_SW_CCP_STTD_SHIFT                  4
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_SW_CCP_STTD_MASK                   0x00000010
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_SW_CCP_EN_SHIFT                    0
#define    TINY_FNG_CORE_TOP_CCP_CTRL1_SW_CCP_EN_MASK                     0x00000001

#define TINY_FNG_CORE_TOP_CCP_CTRL2_OFFSET                                0x00000030
#define TINY_FNG_CORE_TOP_CCP_CTRL2_TYPE                                  UInt32
#define TINY_FNG_CORE_TOP_CCP_CTRL2_RESERVED_MASK                         0x00F08F00
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_OFFSET_SHIFT                   24
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_OFFSET_MASK                    0xFF000000
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_VPGAIN_SHIFT                   16
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_VPGAIN_MASK                    0x000F0000
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_ACCLEN_SHIFT                   12
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_ACCLEN_MASK                    0x00007000
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_OVSF_SEED_SHIFT                0
#define    TINY_FNG_CORE_TOP_CCP_CTRL2_CCP_OVSF_SEED_MASK                 0x000000FF

#define TINY_FNG_CORE_TOP_CCP_WAKE_X_SEED_OFFSET                          0x00000034
#define TINY_FNG_CORE_TOP_CCP_WAKE_X_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_CCP_WAKE_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_CCP_WAKE_X_SEED_CCP_WAKE_X_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_CCP_WAKE_X_SEED_CCP_WAKE_X_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_CCP_WAKE_Y_SEED_OFFSET                          0x00000038
#define TINY_FNG_CORE_TOP_CCP_WAKE_Y_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_CCP_WAKE_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_CCP_WAKE_Y_SEED_CCP_WAKE_Y_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_CCP_WAKE_Y_SEED_CCP_WAKE_Y_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_CCP_INIT_X_SEED_OFFSET                          0x0000003C
#define TINY_FNG_CORE_TOP_CCP_INIT_X_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_CCP_INIT_X_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_CCP_INIT_X_SEED_CCP_INIT_X_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_CCP_INIT_X_SEED_CCP_INIT_X_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_CCP_INIT_Y_SEED_OFFSET                          0x00000040
#define TINY_FNG_CORE_TOP_CCP_INIT_Y_SEED_TYPE                            UInt32
#define TINY_FNG_CORE_TOP_CCP_INIT_Y_SEED_RESERVED_MASK                   0xFFFC0000
#define    TINY_FNG_CORE_TOP_CCP_INIT_Y_SEED_CCP_INIT_Y_SEED_SHIFT        0
#define    TINY_FNG_CORE_TOP_CCP_INIT_Y_SEED_CCP_INIT_Y_SEED_MASK         0x0003FFFF

#define TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_OFFSET                            0x00000044
#define TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_TYPE                              UInt32
#define TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_RESERVED_MASK                     0x00000000
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC1_RSCP_SHIFT            16
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC1_RSCP_MASK             0xFFFF0000
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC0_RSCP_SHIFT            0
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP1_CPI_ACC0_RSCP_MASK             0x0000FFFF

#define TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_OFFSET                            0x00000048
#define TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_TYPE                              UInt32
#define TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_RESERVED_MASK                     0x00000000
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC3_RSCP_SHIFT            16
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC3_RSCP_MASK             0xFFFF0000
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC2_RSCP_SHIFT            0
#define    TINY_FNG_CORE_TOP_CPI_ACC_RSCP2_CPI_ACC2_RSCP_MASK             0x0000FFFF

#define TINY_FNG_CORE_TOP_CCP_DEMOD_RSCP_OFFSET                           0x0000004C
#define TINY_FNG_CORE_TOP_CCP_DEMOD_RSCP_TYPE                             UInt32
#define TINY_FNG_CORE_TOP_CCP_DEMOD_RSCP_RESERVED_MASK                    0xFFFF0000
#define    TINY_FNG_CORE_TOP_CCP_DEMOD_RSCP_CCP_DEMOD_RSCP_SHIFT          0
#define    TINY_FNG_CORE_TOP_CCP_DEMOD_RSCP_CCP_DEMOD_RSCP_MASK           0x0000FFFF

#define TINY_FNG_CORE_TOP_DLL_CTRL1_OFFSET                                0x00000050
#define TINY_FNG_CORE_TOP_DLL_CTRL1_TYPE                                  UInt32
#define TINY_FNG_CORE_TOP_DLL_CTRL1_RESERVED_MASK                         0xEE0EC00F
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_RET_EN_SHIFT                28
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_RET_EN_MASK                 0x10000000
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_ADV_EN_SHIFT                24
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_ADV_EN_MASK                 0x01000000
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_CMD_NO_SHIFT                20
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_SW_CMD_NO_MASK                 0x00F00000
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_HW_FREEZE_SHIFT                16
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_HW_FREEZE_MASK                 0x00010000
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_MASTER_SHIFT                   12
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_MASTER_MASK                    0x00003000
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_VPGAIN_SHIFT                   8
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_VPGAIN_MASK                    0x00000F00
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_ACCLEN_SHIFT                   4
#define    TINY_FNG_CORE_TOP_DLL_CTRL1_DLL_ACCLEN_MASK                    0x000000F0

#define TINY_FNG_CORE_TOP_DLL_CTRL2_OFFSET                                0x00000054
#define TINY_FNG_CORE_TOP_DLL_CTRL2_TYPE                                  UInt32
#define TINY_FNG_CORE_TOP_DLL_CTRL2_RESERVED_MASK                         0x0000EEFE
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_DLL_ARTH_SHIFT                     16
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_DLL_ARTH_MASK                      0xFFFF0000
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_DLL_DECLEN_SHIFT                   12
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_DLL_DECLEN_MASK                    0x00001000
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_DLL_NEW_EN_SHIFT                   8
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_DLL_NEW_EN_MASK                    0x00000100
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_SW_DLL_EN_SHIFT                    0
#define    TINY_FNG_CORE_TOP_DLL_CTRL2_SW_DLL_EN_MASK                     0x00000001

#define TINY_FNG_CORE_TOP_DLL_PWR_DIFF_OFFSET                             0x00000058
#define TINY_FNG_CORE_TOP_DLL_PWR_DIFF_TYPE                               UInt32
#define TINY_FNG_CORE_TOP_DLL_PWR_DIFF_RESERVED_MASK                      0xFFFF0000
#define    TINY_FNG_CORE_TOP_DLL_PWR_DIFF_DLL_PWR_DIFF_SHIFT              0
#define    TINY_FNG_CORE_TOP_DLL_PWR_DIFF_DLL_PWR_DIFF_MASK               0x0000FFFF

#endif /* __BRCM_RDB_TINY_FNG_CORE_TOP_H__ */



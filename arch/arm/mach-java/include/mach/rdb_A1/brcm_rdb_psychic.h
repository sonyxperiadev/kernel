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

#ifndef __BRCM_RDB_PSYCHIC_H__
#define __BRCM_RDB_PSYCHIC_H__

#define PSYCHIC_PSYCHIC_CTRL1_OFFSET                                      0x00000000
#define PSYCHIC_PSYCHIC_CTRL1_TYPE                                        UInt32
#define PSYCHIC_PSYCHIC_CTRL1_RESERVED_MASK                               0x00000000
#define    PSYCHIC_PSYCHIC_CTRL1_CHEST_SEL_SHIFT                          31
#define    PSYCHIC_PSYCHIC_CTRL1_CHEST_SEL_MASK                           0x80000000
#define    PSYCHIC_PSYCHIC_CTRL1_CPICH_SEL_SHIFT                          30
#define    PSYCHIC_PSYCHIC_CTRL1_CPICH_SEL_MASK                           0x40000000
#define    PSYCHIC_PSYCHIC_CTRL1_RECON_W_SHIFT                            25
#define    PSYCHIC_PSYCHIC_CTRL1_RECON_W_MASK                             0x3E000000
#define    PSYCHIC_PSYCHIC_CTRL1_PWR_MES_SKIP_SHIFT                       24
#define    PSYCHIC_PSYCHIC_CTRL1_PWR_MES_SKIP_MASK                        0x01000000
#define    PSYCHIC_PSYCHIC_CTRL1_COMP_GAP_MODE_SHIFT                      23
#define    PSYCHIC_PSYCHIC_CTRL1_COMP_GAP_MODE_MASK                       0x00800000
#define    PSYCHIC_PSYCHIC_CTRL1_CELL_ID_CHANGE_MODE_SHIFT                22
#define    PSYCHIC_PSYCHIC_CTRL1_CELL_ID_CHANGE_MODE_MASK                 0x00400000
#define    PSYCHIC_PSYCHIC_CTRL1_FNGR_CHANGE_MODE_SHIFT                   21
#define    PSYCHIC_PSYCHIC_CTRL1_FNGR_CHANGE_MODE_MASK                    0x00200000
#define    PSYCHIC_PSYCHIC_CTRL1_CELL_ID_CHANGE_SHIFT                     20
#define    PSYCHIC_PSYCHIC_CTRL1_CELL_ID_CHANGE_MASK                      0x00100000
#define    PSYCHIC_PSYCHIC_CTRL1_SC_GROUP_ID_SHIFT                        14
#define    PSYCHIC_PSYCHIC_CTRL1_SC_GROUP_ID_MASK                         0x000FC000
#define    PSYCHIC_PSYCHIC_CTRL1_PWR_MES_APB_RST_SHIFT                    13
#define    PSYCHIC_PSYCHIC_CTRL1_PWR_MES_APB_RST_MASK                     0x00002000
#define    PSYCHIC_PSYCHIC_CTRL1_PWR_MES_LEN_SHIFT                        5
#define    PSYCHIC_PSYCHIC_CTRL1_PWR_MES_LEN_MASK                         0x00001FE0
#define    PSYCHIC_PSYCHIC_CTRL1_CAN_FNGR_NUM_SHIFT                       1
#define    PSYCHIC_PSYCHIC_CTRL1_CAN_FNGR_NUM_MASK                        0x0000001E
#define    PSYCHIC_PSYCHIC_CTRL1_PSYCHIC_EN_SHIFT                         0
#define    PSYCHIC_PSYCHIC_CTRL1_PSYCHIC_EN_MASK                          0x00000001

#define PSYCHIC_PSYCHIC_CTRL2_OFFSET                                      0x00000004
#define PSYCHIC_PSYCHIC_CTRL2_TYPE                                        UInt32
#define PSYCHIC_PSYCHIC_CTRL2_RESERVED_MASK                               0xF0000000
#define    PSYCHIC_PSYCHIC_CTRL2_SOFT_RESET_SHIFT                         27
#define    PSYCHIC_PSYCHIC_CTRL2_SOFT_RESET_MASK                          0x08000000
#define    PSYCHIC_PSYCHIC_CTRL2_APB_AUTO_RESET_SHIFT                     26
#define    PSYCHIC_PSYCHIC_CTRL2_APB_AUTO_RESET_MASK                      0x04000000
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_NO_RND_SHIFT                     25
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_NO_RND_MASK                      0x02000000
#define    PSYCHIC_PSYCHIC_CTRL2_PSYCHIC_OUT_MODE_SHIFT                   24
#define    PSYCHIC_PSYCHIC_CTRL2_PSYCHIC_OUT_MODE_MASK                    0x01000000
#define    PSYCHIC_PSYCHIC_CTRL2_TSTD_SHIFT                               23
#define    PSYCHIC_PSYCHIC_CTRL2_TSTD_MASK                                0x00800000
#define    PSYCHIC_PSYCHIC_CTRL2_STTD_SHIFT                               22
#define    PSYCHIC_PSYCHIC_CTRL2_STTD_MASK                                0x00400000
#define    PSYCHIC_PSYCHIC_CTRL2_SSCH_PWR_EST_FROM_SW_SHIFT               14
#define    PSYCHIC_PSYCHIC_CTRL2_SSCH_PWR_EST_FROM_SW_MASK                0x003FC000
#define    PSYCHIC_PSYCHIC_CTRL2_SSCH_PWR_EST_SRC_SHIFT                   13
#define    PSYCHIC_PSYCHIC_CTRL2_SSCH_PWR_EST_SRC_MASK                    0x00002000
#define    PSYCHIC_PSYCHIC_CTRL2_PSCH_PWR_EST_FROM_SW_SHIFT               5
#define    PSYCHIC_PSYCHIC_CTRL2_PSCH_PWR_EST_FROM_SW_MASK                0x00001FE0
#define    PSYCHIC_PSYCHIC_CTRL2_PSCH_PWR_EST_SRC_SHIFT                   4
#define    PSYCHIC_PSYCHIC_CTRL2_PSCH_PWR_EST_SRC_MASK                    0x00000010
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_MPD_SHIFT                     3
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_MPD_MASK                      0x00000008
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_SCH_SHIFT                     2
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_SCH_MASK                      0x00000004
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_WCDMA_SHIFT                   1
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_WCDMA_MASK                    0x00000002
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_HSDPA_SHIFT                   0
#define    PSYCHIC_PSYCHIC_CTRL2_SCH_CAN_EN_HSDPA_MASK                    0x00000001

#define PSYCHIC_PSYCHIC_CTRL3_OFFSET                                      0x00000008
#define PSYCHIC_PSYCHIC_CTRL3_TYPE                                        UInt32
#define PSYCHIC_PSYCHIC_CTRL3_RESERVED_MASK                               0xF800000C
#define    PSYCHIC_PSYCHIC_CTRL3_CX16_INTERP_APB_FRZ_SHIFT                26
#define    PSYCHIC_PSYCHIC_CTRL3_CX16_INTERP_APB_FRZ_MASK                 0x04000000
#define    PSYCHIC_PSYCHIC_CTRL3_CX1_RECON_APB_FRZ_SHIFT                  25
#define    PSYCHIC_PSYCHIC_CTRL3_CX1_RECON_APB_FRZ_MASK                   0x02000000
#define    PSYCHIC_PSYCHIC_CTRL3_HSQR_APB_FRZ_SHIFT                       24
#define    PSYCHIC_PSYCHIC_CTRL3_HSQR_APB_FRZ_MASK                        0x01000000
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORRH_APB_FRZ_SHIFT                 23
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORRH_APB_FRZ_MASK                  0x00800000
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORRH_APB_FRZ_SHIFT                 22
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORRH_APB_FRZ_MASK                  0x00400000
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORR_APB_FRZ_SHIFT                  21
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORR_APB_FRZ_MASK                   0x00200000
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORR_APB_FRZ_SHIFT                  20
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORR_APB_FRZ_MASK                   0x00100000
#define    PSYCHIC_PSYCHIC_CTRL3_CX16_INTERP_APB_RST_SHIFT                19
#define    PSYCHIC_PSYCHIC_CTRL3_CX16_INTERP_APB_RST_MASK                 0x00080000
#define    PSYCHIC_PSYCHIC_CTRL3_CX1_RECON_APB_RST_SHIFT                  18
#define    PSYCHIC_PSYCHIC_CTRL3_CX1_RECON_APB_RST_MASK                   0x00040000
#define    PSYCHIC_PSYCHIC_CTRL3_HSQR_APB_RST_SHIFT                       17
#define    PSYCHIC_PSYCHIC_CTRL3_HSQR_APB_RST_MASK                        0x00020000
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORRH_APB_RST_SHIFT                 16
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORRH_APB_RST_MASK                  0x00010000
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORRH_APB_RST_SHIFT                 15
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORRH_APB_RST_MASK                  0x00008000
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORR_APB_RST_SHIFT                  14
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORR_APB_RST_MASK                   0x00004000
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORR_APB_RST_SHIFT                  13
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORR_APB_RST_MASK                   0x00002000
#define    PSYCHIC_PSYCHIC_CTRL3_HSQR_IIR_N_SHIFT                         10
#define    PSYCHIC_PSYCHIC_CTRL3_HSQR_IIR_N_MASK                          0x00001C00
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORRH_IIR_N_SHIFT                   7
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORRH_IIR_N_MASK                    0x00000380
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORRH_IIR_N_SHIFT                   4
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORRH_IIR_N_MASK                    0x00000070
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORR_VP_GAIN_SHIFT                  1
#define    PSYCHIC_PSYCHIC_CTRL3_SSCH_CORR_VP_GAIN_MASK                   0x00000002
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORR_VP_GAIN_SHIFT                  0
#define    PSYCHIC_PSYCHIC_CTRL3_PSCH_CORR_VP_GAIN_MASK                   0x00000001

#define PSYCHIC_PSYCHIC_STATE_OBS1_OFFSET                                 0x0000000C
#define PSYCHIC_PSYCHIC_STATE_OBS1_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_STATE_OBS1_RESERVED_MASK                          0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_OBS1_PSCH_CORRH_D_SHIFT                  0
#define    PSYCHIC_PSYCHIC_STATE_OBS1_PSCH_CORRH_D_MASK                   0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_OBS2_OFFSET                                 0x00000010
#define PSYCHIC_PSYCHIC_STATE_OBS2_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_STATE_OBS2_RESERVED_MASK                          0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_OBS2_SSCH_CORRH_D_SHIFT                  0
#define    PSYCHIC_PSYCHIC_STATE_OBS2_SSCH_CORRH_D_MASK                   0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_OBS3_OFFSET                                 0x00000014
#define PSYCHIC_PSYCHIC_STATE_OBS3_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_STATE_OBS3_RESERVED_MASK                          0xFFFF0000
#define    PSYCHIC_PSYCHIC_STATE_OBS3_PWR_MES_DONE_SHIFT                  15
#define    PSYCHIC_PSYCHIC_STATE_OBS3_PWR_MES_DONE_MASK                   0x00008000
#define    PSYCHIC_PSYCHIC_STATE_OBS3_HSQR_D_SHIFT                        0
#define    PSYCHIC_PSYCHIC_STATE_OBS3_HSQR_D_MASK                         0x00007FFF

#define PSYCHIC_PSYCHIC_STATE_RST1_OFFSET                                 0x00000018
#define PSYCHIC_PSYCHIC_STATE_RST1_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_STATE_RST1_RESERVED_MASK                          0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_RST1_PSCH_CORRH_D_SW_SHIFT               0
#define    PSYCHIC_PSYCHIC_STATE_RST1_PSCH_CORRH_D_SW_MASK                0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_RST2_OFFSET                                 0x0000001C
#define PSYCHIC_PSYCHIC_STATE_RST2_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_STATE_RST2_RESERVED_MASK                          0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_RST2_SSCH_CORRH_D_SW_SHIFT               0
#define    PSYCHIC_PSYCHIC_STATE_RST2_SSCH_CORRH_D_SW_MASK                0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_RST3_OFFSET                                 0x00000020
#define PSYCHIC_PSYCHIC_STATE_RST3_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_STATE_RST3_RESERVED_MASK                          0xFFFF8000
#define    PSYCHIC_PSYCHIC_STATE_RST3_HSQR_D_SW_SHIFT                     0
#define    PSYCHIC_PSYCHIC_STATE_RST3_HSQR_D_SW_MASK                      0x00007FFF

#define PSYCHIC_PSYCHIC_DEBUG1_OFFSET                                     0x00000024
#define PSYCHIC_PSYCHIC_DEBUG1_TYPE                                       UInt32
#define PSYCHIC_PSYCHIC_DEBUG1_RESERVED_MASK                              0xFC00FC00
#define    PSYCHIC_PSYCHIC_DEBUG1_PSCH_CORR_Q_VAL_SHIFT                   16
#define    PSYCHIC_PSYCHIC_DEBUG1_PSCH_CORR_Q_VAL_MASK                    0x03FF0000
#define    PSYCHIC_PSYCHIC_DEBUG1_PSCH_CORR_I_VAL_SHIFT                   0
#define    PSYCHIC_PSYCHIC_DEBUG1_PSCH_CORR_I_VAL_MASK                    0x000003FF

#define PSYCHIC_PSYCHIC_DEBUG2_OFFSET                                     0x00000028
#define PSYCHIC_PSYCHIC_DEBUG2_TYPE                                       UInt32
#define PSYCHIC_PSYCHIC_DEBUG2_RESERVED_MASK                              0xFC00FC00
#define    PSYCHIC_PSYCHIC_DEBUG2_SSCH_CORR_Q_VAL_SHIFT                   16
#define    PSYCHIC_PSYCHIC_DEBUG2_SSCH_CORR_Q_VAL_MASK                    0x03FF0000
#define    PSYCHIC_PSYCHIC_DEBUG2_SSCH_CORR_I_VAL_SHIFT                   0
#define    PSYCHIC_PSYCHIC_DEBUG2_SSCH_CORR_I_VAL_MASK                    0x000003FF

#define PSYCHIC_PSYCHIC_DEBUG3_OFFSET                                     0x0000002C
#define PSYCHIC_PSYCHIC_DEBUG3_TYPE                                       UInt32
#define PSYCHIC_PSYCHIC_DEBUG3_RESERVED_MASK                              0xC0000000
#define    PSYCHIC_PSYCHIC_DEBUG3_HSQR_SHIFT                              22
#define    PSYCHIC_PSYCHIC_DEBUG3_HSQR_MASK                               0x3FC00000
#define    PSYCHIC_PSYCHIC_DEBUG3_SSCH_CORRH_SHIFT                        11
#define    PSYCHIC_PSYCHIC_DEBUG3_SSCH_CORRH_MASK                         0x003FF800
#define    PSYCHIC_PSYCHIC_DEBUG3_PSCH_CORRH_SHIFT                        0
#define    PSYCHIC_PSYCHIC_DEBUG3_PSCH_CORRH_MASK                         0x000007FF

#define PSYCHIC_PSYCHIC_DEBUG4_OFFSET                                     0x00000030
#define PSYCHIC_PSYCHIC_DEBUG4_TYPE                                       UInt32
#define PSYCHIC_PSYCHIC_DEBUG4_RESERVED_MASK                              0xFFF80000
#define    PSYCHIC_PSYCHIC_DEBUG4_FSM_STATE_SHIFT                         16
#define    PSYCHIC_PSYCHIC_DEBUG4_FSM_STATE_MASK                          0x00070000
#define    PSYCHIC_PSYCHIC_DEBUG4_SSCH_GAMMA_SHIFT                        8
#define    PSYCHIC_PSYCHIC_DEBUG4_SSCH_GAMMA_MASK                         0x0000FF00
#define    PSYCHIC_PSYCHIC_DEBUG4_PSCH_GAMMA_SHIFT                        0
#define    PSYCHIC_PSYCHIC_DEBUG4_PSCH_GAMMA_MASK                         0x000000FF

#define PSYCHIC_PSYCHIC_DELAY_OFFSET                                      0x00000034
#define PSYCHIC_PSYCHIC_DELAY_TYPE                                        UInt32
#define PSYCHIC_PSYCHIC_DELAY_RESERVED_MASK                               0xFF00C0C0
#define    PSYCHIC_PSYCHIC_DELAY_CHIP_OFFSET_SHIFT                        16
#define    PSYCHIC_PSYCHIC_DELAY_CHIP_OFFSET_MASK                         0x00FF0000
#define    PSYCHIC_PSYCHIC_DELAY_CLEAN_DELAY_SHIFT                        8
#define    PSYCHIC_PSYCHIC_DELAY_CLEAN_DELAY_MASK                         0x00003F00
#define    PSYCHIC_PSYCHIC_DELAY_INPUT_DELAY_SHIFT                        0
#define    PSYCHIC_PSYCHIC_DELAY_INPUT_DELAY_MASK                         0x0000003F

#define PSYCHIC_PSYCHIC_RXD_OFFSET                                        0x00000038
#define PSYCHIC_PSYCHIC_RXD_TYPE                                          UInt32
#define PSYCHIC_PSYCHIC_RXD_RESERVED_MASK                                 0xFEE0FEE0
#define    PSYCHIC_PSYCHIC_RXD_SSCH_GAMMA_SEL_SHIFT                       24
#define    PSYCHIC_PSYCHIC_RXD_SSCH_GAMMA_SEL_MASK                        0x01000000
#define    PSYCHIC_PSYCHIC_RXD_SSCH_GAMMA_WT_SHIFT                        16
#define    PSYCHIC_PSYCHIC_RXD_SSCH_GAMMA_WT_MASK                         0x001F0000
#define    PSYCHIC_PSYCHIC_RXD_PSCH_GAMMA_SEL_SHIFT                       8
#define    PSYCHIC_PSYCHIC_RXD_PSCH_GAMMA_SEL_MASK                        0x00000100
#define    PSYCHIC_PSYCHIC_RXD_PSCH_GAMMA_WT_SHIFT                        0
#define    PSYCHIC_PSYCHIC_RXD_PSCH_GAMMA_WT_MASK                         0x0000001F

#define PSYCHIC_PSYCHIC_CTRL1_RA1_OFFSET                                  0x00000040
#define PSYCHIC_PSYCHIC_CTRL1_RA1_TYPE                                    UInt32
#define PSYCHIC_PSYCHIC_CTRL1_RA1_RESERVED_MASK                           0x00000000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CHEST_SEL_RA1_SHIFT                  31
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CHEST_SEL_RA1_MASK                   0x80000000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CPICH_SEL_RA1_SHIFT                  30
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CPICH_SEL_RA1_MASK                   0x40000000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_RECON_W_RA1_SHIFT                    25
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_RECON_W_RA1_MASK                     0x3E000000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PWR_MES_SKIP_RA1_SHIFT               24
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PWR_MES_SKIP_RA1_MASK                0x01000000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_COMP_GAP_MODE_RA1_SHIFT              23
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_COMP_GAP_MODE_RA1_MASK               0x00800000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CELL_ID_CHANGE_MODE_RA1_SHIFT        22
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CELL_ID_CHANGE_MODE_RA1_MASK         0x00400000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_FNGR_CHANGE_MODE_RA1_SHIFT           21
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_FNGR_CHANGE_MODE_RA1_MASK            0x00200000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CELL_ID_CHANGE_RA1_SHIFT             20
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CELL_ID_CHANGE_RA1_MASK              0x00100000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_SC_GROUP_ID_RA1_SHIFT                14
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_SC_GROUP_ID_RA1_MASK                 0x000FC000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PWR_MES_APB_RST_RA1_SHIFT            13
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PWR_MES_APB_RST_RA1_MASK             0x00002000
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PWR_MES_LEN_RA1_SHIFT                5
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PWR_MES_LEN_RA1_MASK                 0x00001FE0
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CAN_FNGR_NUM_RA1_SHIFT               1
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_CAN_FNGR_NUM_RA1_MASK                0x0000001E
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PSYCHIC_EN_RA1_SHIFT                 0
#define    PSYCHIC_PSYCHIC_CTRL1_RA1_PSYCHIC_EN_RA1_MASK                  0x00000001

#define PSYCHIC_PSYCHIC_CTRL2_RA1_OFFSET                                  0x00000044
#define PSYCHIC_PSYCHIC_CTRL2_RA1_TYPE                                    UInt32
#define PSYCHIC_PSYCHIC_CTRL2_RA1_RESERVED_MASK                           0xF0000000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SOFT_RESET_RA1_SHIFT                 27
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SOFT_RESET_RA1_MASK                  0x08000000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_APB_AUTO_RESET_RA1_SHIFT             26
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_APB_AUTO_RESET_RA1_MASK              0x04000000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_NO_RND_RA1_SHIFT             25
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_NO_RND_RA1_MASK              0x02000000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_PSYCHIC_OUT_MODE_RA1_SHIFT           24
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_PSYCHIC_OUT_MODE_RA1_MASK            0x01000000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_TSTD_RA1_SHIFT                       23
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_TSTD_RA1_MASK                        0x00800000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_STTD_RA1_SHIFT                       22
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_STTD_RA1_MASK                        0x00400000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SSCH_PWR_EST_FROM_SW_RA1_SHIFT       14
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SSCH_PWR_EST_FROM_SW_RA1_MASK        0x003FC000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SSCH_PWR_EST_SRC_RA1_SHIFT           13
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SSCH_PWR_EST_SRC_RA1_MASK            0x00002000
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_PSCH_PWR_EST_FROM_SW_RA1_SHIFT       5
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_PSCH_PWR_EST_FROM_SW_RA1_MASK        0x00001FE0
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_PSCH_PWR_EST_SRC_RA1_SHIFT           4
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_PSCH_PWR_EST_SRC_RA1_MASK            0x00000010
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_MPD_RA1_SHIFT             3
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_MPD_RA1_MASK              0x00000008
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_SCH_RA1_SHIFT             2
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_SCH_RA1_MASK              0x00000004
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_WCDMA_RA1_SHIFT           1
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_WCDMA_RA1_MASK            0x00000002
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_HSDPA_RA1_SHIFT           0
#define    PSYCHIC_PSYCHIC_CTRL2_RA1_SCH_CAN_EN_HSDPA_RA1_MASK            0x00000001

#define PSYCHIC_PSYCHIC_CTRL3_RA1_OFFSET                                  0x00000048
#define PSYCHIC_PSYCHIC_CTRL3_RA1_TYPE                                    UInt32
#define PSYCHIC_PSYCHIC_CTRL3_RA1_RESERVED_MASK                           0xF800000C
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX16_INTERP_APB_FRZ_RA1_SHIFT        26
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX16_INTERP_APB_FRZ_RA1_MASK         0x04000000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX1_RECON_APB_FRZ_RA1_SHIFT          25
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX1_RECON_APB_FRZ_RA1_MASK           0x02000000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_HSQR_APB_FRZ_RA1_SHIFT               24
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_HSQR_APB_FRZ_RA1_MASK                0x01000000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORRH_APB_FRZ_RA1_SHIFT         23
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORRH_APB_FRZ_RA1_MASK          0x00800000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORRH_APB_FRZ_RA1_SHIFT         22
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORRH_APB_FRZ_RA1_MASK          0x00400000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORR_APB_FRZ_RA1_SHIFT          21
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORR_APB_FRZ_RA1_MASK           0x00200000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORR_APB_FRZ_RA1_SHIFT          20
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORR_APB_FRZ_RA1_MASK           0x00100000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX16_INTERP_APB_RST_RA1_SHIFT        19
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX16_INTERP_APB_RST_RA1_MASK         0x00080000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX1_RECON_APB_RST_RA1_SHIFT          18
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_CX1_RECON_APB_RST_RA1_MASK           0x00040000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_HSQR_APB_RST_RA1_SHIFT               17
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_HSQR_APB_RST_RA1_MASK                0x00020000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORRH_APB_RST_RA1_SHIFT         16
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORRH_APB_RST_RA1_MASK          0x00010000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORRH_APB_RST_RA1_SHIFT         15
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORRH_APB_RST_RA1_MASK          0x00008000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORR_APB_RST_RA1_SHIFT          14
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORR_APB_RST_RA1_MASK           0x00004000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORR_APB_RST_RA1_SHIFT          13
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORR_APB_RST_RA1_MASK           0x00002000
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_HSQR_IIR_N_RA1_SHIFT                 10
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_HSQR_IIR_N_RA1_MASK                  0x00001C00
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORRH_IIR_N_RA1_SHIFT           7
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORRH_IIR_N_RA1_MASK            0x00000380
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORRH_IIR_N_RA1_SHIFT           4
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORRH_IIR_N_RA1_MASK            0x00000070
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORR_VP_GAIN_RA1_SHIFT          1
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_SSCH_CORR_VP_GAIN_RA1_MASK           0x00000002
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORR_VP_GAIN_RA1_SHIFT          0
#define    PSYCHIC_PSYCHIC_CTRL3_RA1_PSCH_CORR_VP_GAIN_RA1_MASK           0x00000001

#define PSYCHIC_PSYCHIC_STATE_OBS1_RA1_OFFSET                             0x0000004C
#define PSYCHIC_PSYCHIC_STATE_OBS1_RA1_TYPE                               UInt32
#define PSYCHIC_PSYCHIC_STATE_OBS1_RA1_RESERVED_MASK                      0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_OBS1_RA1_PSCH_CORRH_D_RA1_SHIFT          0
#define    PSYCHIC_PSYCHIC_STATE_OBS1_RA1_PSCH_CORRH_D_RA1_MASK           0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_OBS2_RA1_OFFSET                             0x00000050
#define PSYCHIC_PSYCHIC_STATE_OBS2_RA1_TYPE                               UInt32
#define PSYCHIC_PSYCHIC_STATE_OBS2_RA1_RESERVED_MASK                      0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_OBS2_RA1_SSCH_CORRH_D_RA1_SHIFT          0
#define    PSYCHIC_PSYCHIC_STATE_OBS2_RA1_SSCH_CORRH_D_RA1_MASK           0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_OBS3_RA1_OFFSET                             0x00000054
#define PSYCHIC_PSYCHIC_STATE_OBS3_RA1_TYPE                               UInt32
#define PSYCHIC_PSYCHIC_STATE_OBS3_RA1_RESERVED_MASK                      0xFFFF0000
#define    PSYCHIC_PSYCHIC_STATE_OBS3_RA1_PWR_MES_DONE_RA1_SHIFT          15
#define    PSYCHIC_PSYCHIC_STATE_OBS3_RA1_PWR_MES_DONE_RA1_MASK           0x00008000
#define    PSYCHIC_PSYCHIC_STATE_OBS3_RA1_HSQR_D_RA1_SHIFT                0
#define    PSYCHIC_PSYCHIC_STATE_OBS3_RA1_HSQR_D_RA1_MASK                 0x00007FFF

#define PSYCHIC_PSYCHIC_STATE_RST1_RA1_OFFSET                             0x00000058
#define PSYCHIC_PSYCHIC_STATE_RST1_RA1_TYPE                               UInt32
#define PSYCHIC_PSYCHIC_STATE_RST1_RA1_RESERVED_MASK                      0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_RST1_RA1_PSCH_CORRH_D_SW_RA1_SHIFT       0
#define    PSYCHIC_PSYCHIC_STATE_RST1_RA1_PSCH_CORRH_D_SW_RA1_MASK        0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_RST2_RA1_OFFSET                             0x0000005C
#define PSYCHIC_PSYCHIC_STATE_RST2_RA1_TYPE                               UInt32
#define PSYCHIC_PSYCHIC_STATE_RST2_RA1_RESERVED_MASK                      0xFFFC0000
#define    PSYCHIC_PSYCHIC_STATE_RST2_RA1_SSCH_CORRH_D_SW_RA1_SHIFT       0
#define    PSYCHIC_PSYCHIC_STATE_RST2_RA1_SSCH_CORRH_D_SW_RA1_MASK        0x0003FFFF

#define PSYCHIC_PSYCHIC_STATE_RST3_RA1_OFFSET                             0x00000060
#define PSYCHIC_PSYCHIC_STATE_RST3_RA1_TYPE                               UInt32
#define PSYCHIC_PSYCHIC_STATE_RST3_RA1_RESERVED_MASK                      0xFFFF8000
#define    PSYCHIC_PSYCHIC_STATE_RST3_RA1_HSQR_D_SW_RA1_SHIFT             0
#define    PSYCHIC_PSYCHIC_STATE_RST3_RA1_HSQR_D_SW_RA1_MASK              0x00007FFF

#define PSYCHIC_PSYCHIC_DEBUG1_RA1_OFFSET                                 0x00000064
#define PSYCHIC_PSYCHIC_DEBUG1_RA1_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_DEBUG1_RA1_RESERVED_MASK                          0xFC00FC00
#define    PSYCHIC_PSYCHIC_DEBUG1_RA1_PSCH_CORR_Q_VAL_RA1_SHIFT           16
#define    PSYCHIC_PSYCHIC_DEBUG1_RA1_PSCH_CORR_Q_VAL_RA1_MASK            0x03FF0000
#define    PSYCHIC_PSYCHIC_DEBUG1_RA1_PSCH_CORR_I_VAL_RA1_SHIFT           0
#define    PSYCHIC_PSYCHIC_DEBUG1_RA1_PSCH_CORR_I_VAL_RA1_MASK            0x000003FF

#define PSYCHIC_PSYCHIC_DEBUG2_RA1_OFFSET                                 0x00000068
#define PSYCHIC_PSYCHIC_DEBUG2_RA1_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_DEBUG2_RA1_RESERVED_MASK                          0xFC00FC00
#define    PSYCHIC_PSYCHIC_DEBUG2_RA1_SSCH_CORR_Q_VAL_RA1_SHIFT           16
#define    PSYCHIC_PSYCHIC_DEBUG2_RA1_SSCH_CORR_Q_VAL_RA1_MASK            0x03FF0000
#define    PSYCHIC_PSYCHIC_DEBUG2_RA1_SSCH_CORR_I_VAL_RA1_SHIFT           0
#define    PSYCHIC_PSYCHIC_DEBUG2_RA1_SSCH_CORR_I_VAL_RA1_MASK            0x000003FF

#define PSYCHIC_PSYCHIC_DEBUG3_RA1_OFFSET                                 0x0000006C
#define PSYCHIC_PSYCHIC_DEBUG3_RA1_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_DEBUG3_RA1_RESERVED_MASK                          0xC0000000
#define    PSYCHIC_PSYCHIC_DEBUG3_RA1_HSQR_RA1_SHIFT                      22
#define    PSYCHIC_PSYCHIC_DEBUG3_RA1_HSQR_RA1_MASK                       0x3FC00000
#define    PSYCHIC_PSYCHIC_DEBUG3_RA1_SSCH_CORRH_RA1_SHIFT                11
#define    PSYCHIC_PSYCHIC_DEBUG3_RA1_SSCH_CORRH_RA1_MASK                 0x003FF800
#define    PSYCHIC_PSYCHIC_DEBUG3_RA1_PSCH_CORRH_RA1_SHIFT                0
#define    PSYCHIC_PSYCHIC_DEBUG3_RA1_PSCH_CORRH_RA1_MASK                 0x000007FF

#define PSYCHIC_PSYCHIC_DEBUG4_RA1_OFFSET                                 0x00000070
#define PSYCHIC_PSYCHIC_DEBUG4_RA1_TYPE                                   UInt32
#define PSYCHIC_PSYCHIC_DEBUG4_RA1_RESERVED_MASK                          0xFFF80000
#define    PSYCHIC_PSYCHIC_DEBUG4_RA1_FSM_STATE_RA1_SHIFT                 16
#define    PSYCHIC_PSYCHIC_DEBUG4_RA1_FSM_STATE_RA1_MASK                  0x00070000
#define    PSYCHIC_PSYCHIC_DEBUG4_RA1_SSCH_GAMMA_RA1_SHIFT                8
#define    PSYCHIC_PSYCHIC_DEBUG4_RA1_SSCH_GAMMA_RA1_MASK                 0x0000FF00
#define    PSYCHIC_PSYCHIC_DEBUG4_RA1_PSCH_GAMMA_RA1_SHIFT                0
#define    PSYCHIC_PSYCHIC_DEBUG4_RA1_PSCH_GAMMA_RA1_MASK                 0x000000FF

#define PSYCHIC_PSYCHIC_DELAY_RA1_OFFSET                                  0x00000074
#define PSYCHIC_PSYCHIC_DELAY_RA1_TYPE                                    UInt32
#define PSYCHIC_PSYCHIC_DELAY_RA1_RESERVED_MASK                           0xFF00C0C0
#define    PSYCHIC_PSYCHIC_DELAY_RA1_CHIP_OFFSET_RA1_SHIFT                16
#define    PSYCHIC_PSYCHIC_DELAY_RA1_CHIP_OFFSET_RA1_MASK                 0x00FF0000
#define    PSYCHIC_PSYCHIC_DELAY_RA1_CLEAN_DELAY_RA1_SHIFT                8
#define    PSYCHIC_PSYCHIC_DELAY_RA1_CLEAN_DELAY_RA1_MASK                 0x00003F00
#define    PSYCHIC_PSYCHIC_DELAY_RA1_INPUT_DELAY_RA1_SHIFT                0
#define    PSYCHIC_PSYCHIC_DELAY_RA1_INPUT_DELAY_RA1_MASK                 0x0000003F

#define PSYCHIC_PSYCHIC_RXD_RA1_OFFSET                                    0x00000078
#define PSYCHIC_PSYCHIC_RXD_RA1_TYPE                                      UInt32
#define PSYCHIC_PSYCHIC_RXD_RA1_RESERVED_MASK                             0xFEE0FEE0
#define    PSYCHIC_PSYCHIC_RXD_RA1_SSCH_GAMMA_SEL_RA1_SHIFT               24
#define    PSYCHIC_PSYCHIC_RXD_RA1_SSCH_GAMMA_SEL_RA1_MASK                0x01000000
#define    PSYCHIC_PSYCHIC_RXD_RA1_SSCH_GAMMA_WT_RA1_SHIFT                16
#define    PSYCHIC_PSYCHIC_RXD_RA1_SSCH_GAMMA_WT_RA1_MASK                 0x001F0000
#define    PSYCHIC_PSYCHIC_RXD_RA1_PSCH_GAMMA_SEL_RA1_SHIFT               8
#define    PSYCHIC_PSYCHIC_RXD_RA1_PSCH_GAMMA_SEL_RA1_MASK                0x00000100
#define    PSYCHIC_PSYCHIC_RXD_RA1_PSCH_GAMMA_WT_RA1_SHIFT                0
#define    PSYCHIC_PSYCHIC_RXD_RA1_PSCH_GAMMA_WT_RA1_MASK                 0x0000001F

#endif /* __BRCM_RDB_PSYCHIC_H__ */



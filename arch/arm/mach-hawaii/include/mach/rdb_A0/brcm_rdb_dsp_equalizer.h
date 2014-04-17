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

#ifndef __BRCM_RDB_DSP_EQUALIZER_H__
#define __BRCM_RDB_DSP_EQUALIZER_H__

#define DSP_EQUALIZER_EQ_CFG_OFFSET                                       0x0000E400
#define DSP_EQUALIZER_EQ_CFG_TYPE                                         UInt16
#define DSP_EQUALIZER_EQ_CFG_RESERVED_MASK                                0x0000BFC0
#define    DSP_EQUALIZER_EQ_CFG_VITSTAT_SHIFT                             14
#define    DSP_EQUALIZER_EQ_CFG_VITSTAT_MASK                              0x00004000
#define    DSP_EQUALIZER_EQ_CFG_CH_UPDATE_EN_SHIFT                        4
#define    DSP_EQUALIZER_EQ_CFG_CH_UPDATE_EN_MASK                         0x00000030
#define    DSP_EQUALIZER_EQ_CFG_SYMSWAP_SHIFT                             3
#define    DSP_EQUALIZER_EQ_CFG_SYMSWAP_MASK                              0x00000008
#define    DSP_EQUALIZER_EQ_CFG_GMSK32S_SHIFT                             2
#define    DSP_EQUALIZER_EQ_CFG_GMSK32S_MASK                              0x00000004
#define       DSP_EQUALIZER_EQ_CFG_GMSK32S_CMD_STATE16                    0x00000000
#define       DSP_EQUALIZER_EQ_CFG_GMSK32S_CMD_STATE32                    0x00000001
#define    DSP_EQUALIZER_EQ_CFG_EQCFG_SHIFT                               0
#define    DSP_EQUALIZER_EQ_CFG_EQCFG_MASK                                0x00000003
#define       DSP_EQUALIZER_EQ_CFG_EQCFG_CMD_STOP                         0x00000000
#define       DSP_EQUALIZER_EQ_CFG_EQCFG_CMD_GSMT                         0x00000001
#define       DSP_EQUALIZER_EQ_CFG_EQCFG_CMD_GSMS                         0x00000002
#define       DSP_EQUALIZER_EQ_CFG_EQCFG_CMD_EDGE                         0x00000003

#define DSP_EQUALIZER_EQSEL_OFFSET                                        0x0000E401
#define DSP_EQUALIZER_EQSEL_TYPE                                          UInt16
#define DSP_EQUALIZER_EQSEL_RESERVED_MASK                                 0x000060E0
#define    DSP_EQUALIZER_EQSEL_MTST_ENABLE_SHIFT                          15
#define    DSP_EQUALIZER_EQSEL_MTST_ENABLE_MASK                           0x00008000
#define    DSP_EQUALIZER_EQSEL_RD_SEL_ID_SHIFT                            8
#define    DSP_EQUALIZER_EQSEL_RD_SEL_ID_MASK                             0x00001F00
#define    DSP_EQUALIZER_EQSEL_WR_SEL_ID_SHIFT                            0
#define    DSP_EQUALIZER_EQSEL_WR_SEL_ID_MASK                             0x0000001F

#define DSP_EQUALIZER_EQAPR_OFFSET                                        0x0000E402
#define DSP_EQUALIZER_EQAPR_TYPE                                          UInt16
#define DSP_EQUALIZER_EQAPR_RESERVED_MASK                                 0x0000F000
#define    DSP_EQUALIZER_EQAPR_OFF_ADDR_SHIFT                             0
#define    DSP_EQUALIZER_EQAPR_OFF_ADDR_MASK                              0x00000FFF

#define DSP_EQUALIZER_EQBUF_OFFSET                                        0x0000E403
#define DSP_EQUALIZER_EQBUF_TYPE                                          UInt16
#define DSP_EQUALIZER_EQBUF_RESERVED_MASK                                 0x00000000
#define    DSP_EQUALIZER_EQBUF_VITLD_SHIFT                                0
#define    DSP_EQUALIZER_EQBUF_VITLD_MASK                                 0x0000FFFF

#define DSP_EQUALIZER_EQBUF32_OFFSET                                      0x0000E404
#define DSP_EQUALIZER_EQBUF32_TYPE                                        UInt32
#define DSP_EQUALIZER_EQBUF32_RESERVED_MASK                               0x00000000
#define    DSP_EQUALIZER_EQBUF32_VITLD_SHIFT                              0
#define    DSP_EQUALIZER_EQBUF32_VITLD_MASK                               0xFFFFFFFF

#define DSP_EQUALIZER_VITRLL_I0_OFFSET                                    0x0000E481
#define DSP_EQUALIZER_VITRLL_I0_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_I0_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I0_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_I0_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_I1_OFFSET                                    0x0000E482
#define DSP_EQUALIZER_VITRLL_I1_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_I1_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I1_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_I1_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_I2_OFFSET                                    0x0000E483
#define DSP_EQUALIZER_VITRLL_I2_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_I2_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I2_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_I2_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_I3_OFFSET                                    0x0000E484
#define DSP_EQUALIZER_VITRLL_I3_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_I3_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I3_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_I3_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_I4_OFFSET                                    0x0000E485
#define DSP_EQUALIZER_VITRLL_I4_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_I4_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I4_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_I4_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_I5_OFFSET                                    0x0000E486
#define DSP_EQUALIZER_VITRLL_I5_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_I5_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I5_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_I5_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_Q0_OFFSET                                    0x0000E488
#define DSP_EQUALIZER_VITRLL_Q0_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_Q0_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q0_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_Q0_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_Q1_OFFSET                                    0x0000E489
#define DSP_EQUALIZER_VITRLL_Q1_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_Q1_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q1_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_Q1_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_Q2_OFFSET                                    0x0000E48A
#define DSP_EQUALIZER_VITRLL_Q2_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_Q2_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q2_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_Q2_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_Q3_OFFSET                                    0x0000E48B
#define DSP_EQUALIZER_VITRLL_Q3_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_Q3_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q3_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_Q3_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_Q4_OFFSET                                    0x0000E48C
#define DSP_EQUALIZER_VITRLL_Q4_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_Q4_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q4_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_Q4_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_VITRLL_Q5_OFFSET                                    0x0000E48D
#define DSP_EQUALIZER_VITRLL_Q5_TYPE                                      UInt16
#define DSP_EQUALIZER_VITRLL_Q5_RESERVED_MASK                             0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q5_VITD_SHIFT                             0
#define    DSP_EQUALIZER_VITRLL_Q5_VITD_MASK                              0x000001FF

#define DSP_EQUALIZER_LHINIT_OFFSET                                       0x0000E490
#define DSP_EQUALIZER_LHINIT_TYPE                                         UInt16
#define DSP_EQUALIZER_LHINIT_RESERVED_MASK                                0x00000000
#define    DSP_EQUALIZER_LHINIT_MODE32_INTM_SHIFT                         8
#define    DSP_EQUALIZER_LHINIT_MODE32_INTM_MASK                          0x00001F00
#define    DSP_EQUALIZER_LHINIT_MODE32_INTS_SHIFT                         0
#define    DSP_EQUALIZER_LHINIT_MODE32_INTS_MASK                          0x0000001F
#define    DSP_EQUALIZER_LHINIT_MODE16_FINM_SHIFT                         12
#define    DSP_EQUALIZER_LHINIT_MODE16_FINM_MASK                          0x0000F000
#define    DSP_EQUALIZER_LHINIT_MODE16_FINS_SHIFT                         8
#define    DSP_EQUALIZER_LHINIT_MODE16_FINS_MASK                          0x00000F00
#define    DSP_EQUALIZER_LHINIT_MODE16_INTM_SHIFT                         4
#define    DSP_EQUALIZER_LHINIT_MODE16_INTM_MASK                          0x000000F0
#define    DSP_EQUALIZER_LHINIT_MODE16_INTS_SHIFT                         0
#define    DSP_EQUALIZER_LHINIT_MODE16_INTS_MASK                          0x0000000F
#define    DSP_EQUALIZER_LHINIT_EDGEMODE_INTS_SHIFT                       0
#define    DSP_EQUALIZER_LHINIT_EDGEMODE_INTS_MASK                        0x0000003F

#define DSP_EQUALIZER_RHINIT_OFFSET                                       0x0000E491
#define DSP_EQUALIZER_RHINIT_TYPE                                         UInt16
#define DSP_EQUALIZER_RHINIT_RESERVED_MASK                                0x00000000
#define    DSP_EQUALIZER_RHINIT_GSM32_INTM_SHIFT                          8
#define    DSP_EQUALIZER_RHINIT_GSM32_INTM_MASK                           0x00001F00
#define    DSP_EQUALIZER_RHINIT_GSM32_INTS_SHIFT                          0
#define    DSP_EQUALIZER_RHINIT_GSM32_INTS_MASK                           0x0000001F
#define    DSP_EQUALIZER_RHINIT_GSM16_FINM_SHIFT                          12
#define    DSP_EQUALIZER_RHINIT_GSM16_FINM_MASK                           0x0000F000
#define    DSP_EQUALIZER_RHINIT_GSM16_FINS_SHIFT                          8
#define    DSP_EQUALIZER_RHINIT_GSM16_FINS_MASK                           0x00000F00
#define    DSP_EQUALIZER_RHINIT_GSM16_INTM_SHIFT                          4
#define    DSP_EQUALIZER_RHINIT_GSM16_INTM_MASK                           0x000000F0
#define    DSP_EQUALIZER_RHINIT_GSM16_INTS_SHIFT                          0
#define    DSP_EQUALIZER_RHINIT_GSM16_INTS_MASK                           0x0000000F
#define    DSP_EQUALIZER_RHINIT_EDGEMODE_INTS_SHIFT                       0
#define    DSP_EQUALIZER_RHINIT_EDGEMODE_INTS_MASK                        0x0000003F

#define DSP_EQUALIZER_LHFIN_OFFSET                                        0x0000E492
#define DSP_EQUALIZER_LHFIN_TYPE                                          UInt16
#define DSP_EQUALIZER_LHFIN_RESERVED_MASK                                 0x0000E0E0
#define    DSP_EQUALIZER_LHFIN_FINM_SHIFT                                 8
#define    DSP_EQUALIZER_LHFIN_FINM_MASK                                  0x00001F00
#define    DSP_EQUALIZER_LHFIN_FINS_SHIFT                                 0
#define    DSP_EQUALIZER_LHFIN_FINS_MASK                                  0x0000001F

#define DSP_EQUALIZER_LHFB_OFFSET                                         0x0000E492
#define DSP_EQUALIZER_LHFB_TYPE                                           UInt16
#define DSP_EQUALIZER_LHFB_RESERVED_MASK                                  0x00000000
#define    DSP_EQUALIZER_LHFB_IND0_SHIFT                                  15
#define    DSP_EQUALIZER_LHFB_IND0_MASK                                   0x00008000
#define    DSP_EQUALIZER_LHFB_IL_FB4_SHIFT                                12
#define    DSP_EQUALIZER_LHFB_IL_FB4_MASK                                 0x00007000
#define    DSP_EQUALIZER_LHFB_IL_FB3_SHIFT                                9
#define    DSP_EQUALIZER_LHFB_IL_FB3_MASK                                 0x00000E00
#define    DSP_EQUALIZER_LHFB_IL_FB2_SHIFT                                6
#define    DSP_EQUALIZER_LHFB_IL_FB2_MASK                                 0x000001C0
#define    DSP_EQUALIZER_LHFB_IL_FB1_SHIFT                                3
#define    DSP_EQUALIZER_LHFB_IL_FB1_MASK                                 0x00000038
#define    DSP_EQUALIZER_LHFB_IL_FB0_SHIFT                                0
#define    DSP_EQUALIZER_LHFB_IL_FB0_MASK                                 0x00000007

#define DSP_EQUALIZER_RHFIN_OFFSET                                        0x0000E494
#define DSP_EQUALIZER_RHFIN_TYPE                                          UInt16
#define DSP_EQUALIZER_RHFIN_RESERVED_MASK                                 0x0000E0E0
#define    DSP_EQUALIZER_RHFIN_FINM_SHIFT                                 8
#define    DSP_EQUALIZER_RHFIN_FINM_MASK                                  0x00001F00
#define    DSP_EQUALIZER_RHFIN_FINS_SHIFT                                 0
#define    DSP_EQUALIZER_RHFIN_FINS_MASK                                  0x0000001F

#define DSP_EQUALIZER_RHFB_OFFSET                                         0x0000E494
#define DSP_EQUALIZER_RHFB_TYPE                                           UInt16
#define DSP_EQUALIZER_RHFB_RESERVED_MASK                                  0x00000000
#define    DSP_EQUALIZER_RHFB_IND1_SHIFT                                  15
#define    DSP_EQUALIZER_RHFB_IND1_MASK                                   0x00008000
#define    DSP_EQUALIZER_RHFB_IR_FB4_SHIFT                                12
#define    DSP_EQUALIZER_RHFB_IR_FB4_MASK                                 0x00007000
#define    DSP_EQUALIZER_RHFB_IR_FB3_SHIFT                                9
#define    DSP_EQUALIZER_RHFB_IR_FB3_MASK                                 0x00000E00
#define    DSP_EQUALIZER_RHFB_IR_FB2_SHIFT                                6
#define    DSP_EQUALIZER_RHFB_IR_FB2_MASK                                 0x000001C0
#define    DSP_EQUALIZER_RHFB_IR_FB1_SHIFT                                3
#define    DSP_EQUALIZER_RHFB_IR_FB1_MASK                                 0x00000038
#define    DSP_EQUALIZER_RHFB_IR_FB0_SHIFT                                0
#define    DSP_EQUALIZER_RHFB_IR_FB0_MASK                                 0x00000007

#define DSP_EQUALIZER_VITNSF_OFFSET                                       0x0000E497
#define DSP_EQUALIZER_VITNSF_TYPE                                         UInt16
#define DSP_EQUALIZER_VITNSF_RESERVED_MASK                                0x0000FFE0
#define    DSP_EQUALIZER_VITNSF_NSF_SHIFT                                 1
#define    DSP_EQUALIZER_VITNSF_NSF_MASK                                  0x0000001E
#define    DSP_EQUALIZER_VITNSF_DELTA_SHIFT                               0
#define    DSP_EQUALIZER_VITNSF_DELTA_MASK                                0x00000001

#define DSP_EQUALIZER_EQ_CFG_R_OFFSET                                     0x00000800
#define DSP_EQUALIZER_EQ_CFG_R_TYPE                                       UInt16
#define DSP_EQUALIZER_EQ_CFG_R_RESERVED_MASK                              0x0000BFC0
#define    DSP_EQUALIZER_EQ_CFG_R_VITSTAT_SHIFT                           14
#define    DSP_EQUALIZER_EQ_CFG_R_VITSTAT_MASK                            0x00004000
#define    DSP_EQUALIZER_EQ_CFG_R_CH_UPDATE_EN_SHIFT                      4
#define    DSP_EQUALIZER_EQ_CFG_R_CH_UPDATE_EN_MASK                       0x00000030
#define    DSP_EQUALIZER_EQ_CFG_R_SYMSWAP_SHIFT                           3
#define    DSP_EQUALIZER_EQ_CFG_R_SYMSWAP_MASK                            0x00000008
#define    DSP_EQUALIZER_EQ_CFG_R_GMSK32S_SHIFT                           2
#define    DSP_EQUALIZER_EQ_CFG_R_GMSK32S_MASK                            0x00000004
#define       DSP_EQUALIZER_EQ_CFG_R_GMSK32S_CMD_STATE16                  0x00000000
#define       DSP_EQUALIZER_EQ_CFG_R_GMSK32S_CMD_STATE32                  0x00000001
#define    DSP_EQUALIZER_EQ_CFG_R_EQCFG_SHIFT                             0
#define    DSP_EQUALIZER_EQ_CFG_R_EQCFG_MASK                              0x00000003
#define       DSP_EQUALIZER_EQ_CFG_R_EQCFG_CMD_STOP                       0x00000000
#define       DSP_EQUALIZER_EQ_CFG_R_EQCFG_CMD_GSMT                       0x00000001
#define       DSP_EQUALIZER_EQ_CFG_R_EQCFG_CMD_GSMS                       0x00000002
#define       DSP_EQUALIZER_EQ_CFG_R_EQCFG_CMD_EDGE                       0x00000003

#define DSP_EQUALIZER_EQSEL_R_OFFSET                                      0x00000802
#define DSP_EQUALIZER_EQSEL_R_TYPE                                        UInt16
#define DSP_EQUALIZER_EQSEL_R_RESERVED_MASK                               0x000060E0
#define    DSP_EQUALIZER_EQSEL_R_MTST_ENABLE_SHIFT                        15
#define    DSP_EQUALIZER_EQSEL_R_MTST_ENABLE_MASK                         0x00008000
#define    DSP_EQUALIZER_EQSEL_R_RD_SEL_ID_SHIFT                          8
#define    DSP_EQUALIZER_EQSEL_R_RD_SEL_ID_MASK                           0x00001F00
#define    DSP_EQUALIZER_EQSEL_R_WR_SEL_ID_SHIFT                          0
#define    DSP_EQUALIZER_EQSEL_R_WR_SEL_ID_MASK                           0x0000001F

#define DSP_EQUALIZER_EQAPR_R_OFFSET                                      0x00000804
#define DSP_EQUALIZER_EQAPR_R_TYPE                                        UInt16
#define DSP_EQUALIZER_EQAPR_R_RESERVED_MASK                               0x0000F000
#define    DSP_EQUALIZER_EQAPR_R_OFF_ADDR_SHIFT                           0
#define    DSP_EQUALIZER_EQAPR_R_OFF_ADDR_MASK                            0x00000FFF

#define DSP_EQUALIZER_EQBUF_R_OFFSET                                      0x00000806
#define DSP_EQUALIZER_EQBUF_R_TYPE                                        UInt16
#define DSP_EQUALIZER_EQBUF_R_RESERVED_MASK                               0x00000000
#define    DSP_EQUALIZER_EQBUF_R_VITLD_SHIFT                              0
#define    DSP_EQUALIZER_EQBUF_R_VITLD_MASK                               0x0000FFFF

#define DSP_EQUALIZER_EQBUF32_R_OFFSET                                    0x00000808
#define DSP_EQUALIZER_EQBUF32_R_TYPE                                      UInt32
#define DSP_EQUALIZER_EQBUF32_R_RESERVED_MASK                             0x00000000
#define    DSP_EQUALIZER_EQBUF32_R_VITLD_SHIFT                            0
#define    DSP_EQUALIZER_EQBUF32_R_VITLD_MASK                             0xFFFFFFFF

#define DSP_EQUALIZER_VITRLL_I_R0_OFFSET                                  0x00000902
#define DSP_EQUALIZER_VITRLL_I_R0_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_I_R0_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I_R0_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_I_R0_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_I_R1_OFFSET                                  0x00000904
#define DSP_EQUALIZER_VITRLL_I_R1_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_I_R1_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I_R1_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_I_R1_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_I_R2_OFFSET                                  0x00000906
#define DSP_EQUALIZER_VITRLL_I_R2_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_I_R2_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I_R2_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_I_R2_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_I_R3_OFFSET                                  0x00000908
#define DSP_EQUALIZER_VITRLL_I_R3_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_I_R3_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I_R3_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_I_R3_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_I_R4_OFFSET                                  0x0000090A
#define DSP_EQUALIZER_VITRLL_I_R4_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_I_R4_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I_R4_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_I_R4_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_I_R5_OFFSET                                  0x0000090C
#define DSP_EQUALIZER_VITRLL_I_R5_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_I_R5_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_I_R5_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_I_R5_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_Q_R0_OFFSET                                  0x00000910
#define DSP_EQUALIZER_VITRLL_Q_R0_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_Q_R0_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q_R0_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_Q_R0_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_Q_R1_OFFSET                                  0x00000912
#define DSP_EQUALIZER_VITRLL_Q_R1_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_Q_R1_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q_R1_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_Q_R1_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_Q_R2_OFFSET                                  0x00000914
#define DSP_EQUALIZER_VITRLL_Q_R2_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_Q_R2_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q_R2_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_Q_R2_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_Q_R3_OFFSET                                  0x00000916
#define DSP_EQUALIZER_VITRLL_Q_R3_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_Q_R3_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q_R3_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_Q_R3_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_Q_R4_OFFSET                                  0x00000918
#define DSP_EQUALIZER_VITRLL_Q_R4_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_Q_R4_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q_R4_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_Q_R4_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_VITRLL_Q_R5_OFFSET                                  0x0000091A
#define DSP_EQUALIZER_VITRLL_Q_R5_TYPE                                    UInt16
#define DSP_EQUALIZER_VITRLL_Q_R5_RESERVED_MASK                           0x0000FE00
#define    DSP_EQUALIZER_VITRLL_Q_R5_VITD_SHIFT                           0
#define    DSP_EQUALIZER_VITRLL_Q_R5_VITD_MASK                            0x000001FF

#define DSP_EQUALIZER_LHINIT_R_OFFSET                                     0x00000920
#define DSP_EQUALIZER_LHINIT_R_TYPE                                       UInt16
#define DSP_EQUALIZER_LHINIT_R_RESERVED_MASK                              0x00000000
#define    DSP_EQUALIZER_LHINIT_R_MODE32_INTM_SHIFT                       8
#define    DSP_EQUALIZER_LHINIT_R_MODE32_INTM_MASK                        0x00001F00
#define    DSP_EQUALIZER_LHINIT_R_MODE32_INTS_SHIFT                       0
#define    DSP_EQUALIZER_LHINIT_R_MODE32_INTS_MASK                        0x0000001F
#define    DSP_EQUALIZER_LHINIT_R_MODE16_FINM_SHIFT                       12
#define    DSP_EQUALIZER_LHINIT_R_MODE16_FINM_MASK                        0x0000F000
#define    DSP_EQUALIZER_LHINIT_R_MODE16_FINS_SHIFT                       8
#define    DSP_EQUALIZER_LHINIT_R_MODE16_FINS_MASK                        0x00000F00
#define    DSP_EQUALIZER_LHINIT_R_MODE16_INTM_SHIFT                       4
#define    DSP_EQUALIZER_LHINIT_R_MODE16_INTM_MASK                        0x000000F0
#define    DSP_EQUALIZER_LHINIT_R_MODE16_INTS_SHIFT                       0
#define    DSP_EQUALIZER_LHINIT_R_MODE16_INTS_MASK                        0x0000000F
#define    DSP_EQUALIZER_LHINIT_R_EDGEMODE_INTS_SHIFT                     0
#define    DSP_EQUALIZER_LHINIT_R_EDGEMODE_INTS_MASK                      0x0000003F

#define DSP_EQUALIZER_RHINIT_R_OFFSET                                     0x00000922
#define DSP_EQUALIZER_RHINIT_R_TYPE                                       UInt16
#define DSP_EQUALIZER_RHINIT_R_RESERVED_MASK                              0x00000000
#define    DSP_EQUALIZER_RHINIT_R_GSM32_INTM_SHIFT                        8
#define    DSP_EQUALIZER_RHINIT_R_GSM32_INTM_MASK                         0x00001F00
#define    DSP_EQUALIZER_RHINIT_R_GSM32_INTS_SHIFT                        0
#define    DSP_EQUALIZER_RHINIT_R_GSM32_INTS_MASK                         0x0000001F
#define    DSP_EQUALIZER_RHINIT_R_GSM16_FINM_SHIFT                        12
#define    DSP_EQUALIZER_RHINIT_R_GSM16_FINM_MASK                         0x0000F000
#define    DSP_EQUALIZER_RHINIT_R_GSM16_FINS_SHIFT                        8
#define    DSP_EQUALIZER_RHINIT_R_GSM16_FINS_MASK                         0x00000F00
#define    DSP_EQUALIZER_RHINIT_R_GSM16_INTM_SHIFT                        4
#define    DSP_EQUALIZER_RHINIT_R_GSM16_INTM_MASK                         0x000000F0
#define    DSP_EQUALIZER_RHINIT_R_GSM16_INTS_SHIFT                        0
#define    DSP_EQUALIZER_RHINIT_R_GSM16_INTS_MASK                         0x0000000F
#define    DSP_EQUALIZER_RHINIT_R_EDGEMODE_INTS_SHIFT                     0
#define    DSP_EQUALIZER_RHINIT_R_EDGEMODE_INTS_MASK                      0x0000003F

#define DSP_EQUALIZER_LHFIN_R_OFFSET                                      0x00000924
#define DSP_EQUALIZER_LHFIN_R_TYPE                                        UInt16
#define DSP_EQUALIZER_LHFIN_R_RESERVED_MASK                               0x0000E0E0
#define    DSP_EQUALIZER_LHFIN_R_FINM_SHIFT                               8
#define    DSP_EQUALIZER_LHFIN_R_FINM_MASK                                0x00001F00
#define    DSP_EQUALIZER_LHFIN_R_FINS_SHIFT                               0
#define    DSP_EQUALIZER_LHFIN_R_FINS_MASK                                0x0000001F

#define DSP_EQUALIZER_LHFB_R_OFFSET                                       0x00000924
#define DSP_EQUALIZER_LHFB_R_TYPE                                         UInt16
#define DSP_EQUALIZER_LHFB_R_RESERVED_MASK                                0x00000000
#define    DSP_EQUALIZER_LHFB_R_IND0_SHIFT                                15
#define    DSP_EQUALIZER_LHFB_R_IND0_MASK                                 0x00008000
#define    DSP_EQUALIZER_LHFB_R_IL_FB4_SHIFT                              12
#define    DSP_EQUALIZER_LHFB_R_IL_FB4_MASK                               0x00007000
#define    DSP_EQUALIZER_LHFB_R_IL_FB3_SHIFT                              9
#define    DSP_EQUALIZER_LHFB_R_IL_FB3_MASK                               0x00000E00
#define    DSP_EQUALIZER_LHFB_R_IL_FB2_SHIFT                              6
#define    DSP_EQUALIZER_LHFB_R_IL_FB2_MASK                               0x000001C0
#define    DSP_EQUALIZER_LHFB_R_IL_FB1_SHIFT                              3
#define    DSP_EQUALIZER_LHFB_R_IL_FB1_MASK                               0x00000038
#define    DSP_EQUALIZER_LHFB_R_IL_FB0_SHIFT                              0
#define    DSP_EQUALIZER_LHFB_R_IL_FB0_MASK                               0x00000007

#define DSP_EQUALIZER_RHFIN_R_OFFSET                                      0x00000928
#define DSP_EQUALIZER_RHFIN_R_TYPE                                        UInt16
#define DSP_EQUALIZER_RHFIN_R_RESERVED_MASK                               0x0000E0E0
#define    DSP_EQUALIZER_RHFIN_R_FINM_SHIFT                               8
#define    DSP_EQUALIZER_RHFIN_R_FINM_MASK                                0x00001F00
#define    DSP_EQUALIZER_RHFIN_R_FINS_SHIFT                               0
#define    DSP_EQUALIZER_RHFIN_R_FINS_MASK                                0x0000001F

#define DSP_EQUALIZER_RHFB_R_OFFSET                                       0x00000928
#define DSP_EQUALIZER_RHFB_R_TYPE                                         UInt16
#define DSP_EQUALIZER_RHFB_R_RESERVED_MASK                                0x00000000
#define    DSP_EQUALIZER_RHFB_R_IND1_SHIFT                                15
#define    DSP_EQUALIZER_RHFB_R_IND1_MASK                                 0x00008000
#define    DSP_EQUALIZER_RHFB_R_IR_FB4_SHIFT                              12
#define    DSP_EQUALIZER_RHFB_R_IR_FB4_MASK                               0x00007000
#define    DSP_EQUALIZER_RHFB_R_IR_FB3_SHIFT                              9
#define    DSP_EQUALIZER_RHFB_R_IR_FB3_MASK                               0x00000E00
#define    DSP_EQUALIZER_RHFB_R_IR_FB2_SHIFT                              6
#define    DSP_EQUALIZER_RHFB_R_IR_FB2_MASK                               0x000001C0
#define    DSP_EQUALIZER_RHFB_R_IR_FB1_SHIFT                              3
#define    DSP_EQUALIZER_RHFB_R_IR_FB1_MASK                               0x00000038
#define    DSP_EQUALIZER_RHFB_R_IR_FB0_SHIFT                              0
#define    DSP_EQUALIZER_RHFB_R_IR_FB0_MASK                               0x00000007

#define DSP_EQUALIZER_VITNSF_R_OFFSET                                     0x0000092E
#define DSP_EQUALIZER_VITNSF_R_TYPE                                       UInt16
#define DSP_EQUALIZER_VITNSF_R_RESERVED_MASK                              0x0000FFE0
#define    DSP_EQUALIZER_VITNSF_R_NSF_SHIFT                               1
#define    DSP_EQUALIZER_VITNSF_R_NSF_MASK                                0x0000001E
#define    DSP_EQUALIZER_VITNSF_R_DELTA_SHIFT                             0
#define    DSP_EQUALIZER_VITNSF_R_DELTA_MASK                              0x00000001

#endif /* __BRCM_RDB_DSP_EQUALIZER_H__ */



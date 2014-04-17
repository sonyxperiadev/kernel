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

#ifndef __BRCM_RDB_TX_H__
#define __BRCM_RDB_TX_H__

#define TX_HSUPA_TRBLOCKLENGTH_OFFSET                                     0x00000000
#define TX_HSUPA_TRBLOCKLENGTH_TYPE                                       UInt32
#define TX_HSUPA_TRBLOCKLENGTH_RESERVED_MASK                              0xF0008000
#define    TX_HSUPA_TRBLOCKLENGTH_HSUPA_HARQ_BASEADDR_SHIFT               16
#define    TX_HSUPA_TRBLOCKLENGTH_HSUPA_HARQ_BASEADDR_MASK                0x0FFF0000
#define    TX_HSUPA_TRBLOCKLENGTH_HSUPA_TRBLOCKLENGTH_SHIFT               0
#define    TX_HSUPA_TRBLOCKLENGTH_HSUPA_TRBLOCKLENGTH_MASK                0x00007FFF

#define TX_HSUPA_CODEBLOCKSEGM_OFFSET                                     0x00000004
#define TX_HSUPA_CODEBLOCKSEGM_TYPE                                       UInt32
#define TX_HSUPA_CODEBLOCKSEGM_RESERVED_MASK                              0xFFFCE000
#define    TX_HSUPA_CODEBLOCKSEGM_HSUPA_NUMFILLERBITS_SHIFT               16
#define    TX_HSUPA_CODEBLOCKSEGM_HSUPA_NUMFILLERBITS_MASK                0x00030000
#define    TX_HSUPA_CODEBLOCKSEGM_HSUPA_CODEBLOCKLENGTH_SHIFT             0
#define    TX_HSUPA_CODEBLOCKSEGM_HSUPA_CODEBLOCKLENGTH_MASK              0x00001FFF

#define TX_HSUPA_TURBOSETUP0_OFFSET                                       0x00000008
#define TX_HSUPA_TURBOSETUP0_TYPE                                         UInt32
#define TX_HSUPA_TURBOSETUP0_RESERVED_MASK                                0x8080E0E0
#define    TX_HSUPA_TURBOSETUP0_HSUPA_SKIP2_SHIFT                         24
#define    TX_HSUPA_TURBOSETUP0_HSUPA_SKIP2_MASK                          0x7F000000
#define    TX_HSUPA_TURBOSETUP0_HSUPA_SKIP1_SHIFT                         16
#define    TX_HSUPA_TURBOSETUP0_HSUPA_SKIP1_MASK                          0x007F0000
#define    TX_HSUPA_TURBOSETUP0_HSUPA_V_SHIFT                             8
#define    TX_HSUPA_TURBOSETUP0_HSUPA_V_MASK                              0x00001F00
#define    TX_HSUPA_TURBOSETUP0_HSUPA_R_SHIFT                             0
#define    TX_HSUPA_TURBOSETUP0_HSUPA_R_MASK                              0x0000001F

#define TX_HSUPA_TURBOSETUP1_OFFSET                                       0x0000000C
#define TX_HSUPA_TURBOSETUP1_TYPE                                         UInt32
#define TX_HSUPA_TURBOSETUP1_RESERVED_MASK                                0x8E00FE00
#define    TX_HSUPA_TURBOSETUP1_HSUPA_KRC_SHIFT                           30
#define    TX_HSUPA_TURBOSETUP1_HSUPA_KRC_MASK                            0x40000000
#define    TX_HSUPA_TURBOSETUP1_HSUPA_TSEL_SHIFT                          28
#define    TX_HSUPA_TURBOSETUP1_HSUPA_TSEL_MASK                           0x30000000
#define    TX_HSUPA_TURBOSETUP1_HSUPA_PRIME_SHIFT                         16
#define    TX_HSUPA_TURBOSETUP1_HSUPA_PRIME_MASK                          0x01FF0000
#define    TX_HSUPA_TURBOSETUP1_HSUPA_C_SHIFT                             0
#define    TX_HSUPA_TURBOSETUP1_HSUPA_C_MASK                              0x000001FF

#define TX_HSUPA_RM_S_EINI_OFFSET                                         0x00000010
#define TX_HSUPA_RM_S_EINI_TYPE                                           UInt32
#define TX_HSUPA_RM_S_EINI_RESERVED_MASK                                  0xFFF80000
#define    TX_HSUPA_RM_S_EINI_HSUPA_S_EINI_SHIFT                          0
#define    TX_HSUPA_RM_S_EINI_HSUPA_S_EINI_MASK                           0x0007FFFF

#define TX_HSUPA_RM_S_EPLUS_OFFSET                                        0x00000014
#define TX_HSUPA_RM_S_EPLUS_TYPE                                          UInt32
#define TX_HSUPA_RM_S_EPLUS_RESERVED_MASK                                 0xFFF80000
#define    TX_HSUPA_RM_S_EPLUS_HSUPA_S_EPLUS_SHIFT                        0
#define    TX_HSUPA_RM_S_EPLUS_HSUPA_S_EPLUS_MASK                         0x0007FFFF

#define TX_HSUPA_RM_S_EMINUS_OFFSET                                       0x00000018
#define TX_HSUPA_RM_S_EMINUS_TYPE                                         UInt32
#define TX_HSUPA_RM_S_EMINUS_RESERVED_MASK                                0xFFF80000
#define    TX_HSUPA_RM_S_EMINUS_HSUPA_S_EMINUS_SHIFT                      0
#define    TX_HSUPA_RM_S_EMINUS_HSUPA_S_EMINUS_MASK                       0x0007FFFF

#define TX_HSUPA_RM_P1_EINI_OFFSET                                        0x0000001C
#define TX_HSUPA_RM_P1_EINI_TYPE                                          UInt32
#define TX_HSUPA_RM_P1_EINI_RESERVED_MASK                                 0xFFF80000
#define    TX_HSUPA_RM_P1_EINI_HSUPA_P1_EINI_SHIFT                        0
#define    TX_HSUPA_RM_P1_EINI_HSUPA_P1_EINI_MASK                         0x0007FFFF

#define TX_HSUPA_RM_P1_EPLUS_OFFSET                                       0x00000020
#define TX_HSUPA_RM_P1_EPLUS_TYPE                                         UInt32
#define TX_HSUPA_RM_P1_EPLUS_RESERVED_MASK                                0xFFF80000
#define    TX_HSUPA_RM_P1_EPLUS_HSUPA_P1_EPLUS_SHIFT                      0
#define    TX_HSUPA_RM_P1_EPLUS_HSUPA_P1_EPLUS_MASK                       0x0007FFFF

#define TX_HSUPA_RM_P1_EMINUS_OFFSET                                      0x00000024
#define TX_HSUPA_RM_P1_EMINUS_TYPE                                        UInt32
#define TX_HSUPA_RM_P1_EMINUS_RESERVED_MASK                               0xFFF80000
#define    TX_HSUPA_RM_P1_EMINUS_HSUPA_P1_EMINUS_SHIFT                    0
#define    TX_HSUPA_RM_P1_EMINUS_HSUPA_P1_EMINUS_MASK                     0x0007FFFF

#define TX_HSUPA_RM_P2_EINI_OFFSET                                        0x00000028
#define TX_HSUPA_RM_P2_EINI_TYPE                                          UInt32
#define TX_HSUPA_RM_P2_EINI_RESERVED_MASK                                 0xFFF80000
#define    TX_HSUPA_RM_P2_EINI_HSUPA_P2_EINI_SHIFT                        0
#define    TX_HSUPA_RM_P2_EINI_HSUPA_P2_EINI_MASK                         0x0007FFFF

#define TX_HSUPA_RM_P2_EPLUS_OFFSET                                       0x0000002C
#define TX_HSUPA_RM_P2_EPLUS_TYPE                                         UInt32
#define TX_HSUPA_RM_P2_EPLUS_RESERVED_MASK                                0xFFF80000
#define    TX_HSUPA_RM_P2_EPLUS_HSUPA_P2_EPLUS_SHIFT                      0
#define    TX_HSUPA_RM_P2_EPLUS_HSUPA_P2_EPLUS_MASK                       0x0007FFFF

#define TX_HSUPA_RM_P2_EMINUS_OFFSET                                      0x00000030
#define TX_HSUPA_RM_P2_EMINUS_TYPE                                        UInt32
#define TX_HSUPA_RM_P2_EMINUS_RESERVED_MASK                               0xFFF80000
#define    TX_HSUPA_RM_P2_EMINUS_HSUPA_P2_EMINUS_SHIFT                    0
#define    TX_HSUPA_RM_P2_EMINUS_HSUPA_P2_EMINUS_MASK                     0x0007FFFF

#define TX_HSUPA_RMSETUP_OFFSET                                           0x00000034
#define TX_HSUPA_RMSETUP_TYPE                                             UInt32
#define TX_HSUPA_RMSETUP_RESERVED_MASK                                    0xFE0E0000
#define    TX_HSUPA_RMSETUP_HSUPA_PHCH_MAP_SHIFT                          20
#define    TX_HSUPA_RMSETUP_HSUPA_PHCH_MAP_MASK                           0x01F00000
#define    TX_HSUPA_RMSETUP_HSUPA_RMPUNCTURE_SHIFT                        16
#define    TX_HSUPA_RMSETUP_HSUPA_RMPUNCTURE_MASK                         0x00010000
#define    TX_HSUPA_RMSETUP_HSUPA_RM_LIMIT_SHIFT                          0
#define    TX_HSUPA_RMSETUP_HSUPA_RM_LIMIT_MASK                           0x0000FFFF

#define TX_HSUPA_TRCHSTART_OFFSET                                         0x00000038
#define TX_HSUPA_TRCHSTART_TYPE                                           UInt32
#define TX_HSUPA_TRCHSTART_RESERVED_MASK                                  0xFFFFFFEE
#define    TX_HSUPA_TRCHSTART_HSUPA_INPROG_SHIFT                          4
#define    TX_HSUPA_TRCHSTART_HSUPA_INPROG_MASK                           0x00000010
#define    TX_HSUPA_TRCHSTART_HSUPA_TRIGGER_TRCH_SHIFT                    0
#define    TX_HSUPA_TRCHSTART_HSUPA_TRIGGER_TRCH_MASK                     0x00000001

#define TX_HSUPA_HARQ_REQOUT_CNT_OFFSET                                   0x0000003C
#define TX_HSUPA_HARQ_REQOUT_CNT_TYPE                                     UInt32
#define TX_HSUPA_HARQ_REQOUT_CNT_RESERVED_MASK                            0x8000FC00
#define    TX_HSUPA_HARQ_REQOUT_CNT_HSUPA_HARQ_OUTCNT_SHIFT               16
#define    TX_HSUPA_HARQ_REQOUT_CNT_HSUPA_HARQ_OUTCNT_MASK                0x7FFF0000
#define    TX_HSUPA_HARQ_REQOUT_CNT_HSUPA_HARQ_REQCNT_SHIFT               0
#define    TX_HSUPA_HARQ_REQOUT_CNT_HSUPA_HARQ_REQCNT_MASK                0x000003FF

#define TX_HSUPA_TC_RM_INCNT_OFFSET                                       0x00000040
#define TX_HSUPA_TC_RM_INCNT_TYPE                                         UInt32
#define TX_HSUPA_TC_RM_INCNT_RESERVED_MASK                                0x00000000
#define    TX_HSUPA_TC_RM_INCNT_HSUPA_RM_INCNT_SHIFT                      16
#define    TX_HSUPA_TC_RM_INCNT_HSUPA_RM_INCNT_MASK                       0xFFFF0000
#define    TX_HSUPA_TC_RM_INCNT_HSUPA_TC_INCNT_SHIFT                      0
#define    TX_HSUPA_TC_RM_INCNT_HSUPA_TC_INCNT_MASK                       0x0000FFFF

#define TX_HSUPA_RMOUT_S_CNT_OFFSET                                       0x00000044
#define TX_HSUPA_RMOUT_S_CNT_TYPE                                         UInt32
#define TX_HSUPA_RMOUT_S_CNT_RESERVED_MASK                                0x00000000
#define    TX_HSUPA_RMOUT_S_CNT_HSUPA_S_CNT_SHIFT                         16
#define    TX_HSUPA_RMOUT_S_CNT_HSUPA_S_CNT_MASK                          0xFFFF0000
#define    TX_HSUPA_RMOUT_S_CNT_HSUPA_RM_OUTCNT_SHIFT                     0
#define    TX_HSUPA_RMOUT_S_CNT_HSUPA_RM_OUTCNT_MASK                      0x0000FFFF

#define TX_HSUPA_P1P2_CNT_OFFSET                                          0x00000048
#define TX_HSUPA_P1P2_CNT_TYPE                                            UInt32
#define TX_HSUPA_P1P2_CNT_RESERVED_MASK                                   0x00000000
#define    TX_HSUPA_P1P2_CNT_HSUPA_P2_CNT_SHIFT                           16
#define    TX_HSUPA_P1P2_CNT_HSUPA_P2_CNT_MASK                            0xFFFF0000
#define    TX_HSUPA_P1P2_CNT_HSUPA_P1_CNT_SHIFT                           0
#define    TX_HSUPA_P1P2_CNT_HSUPA_P1_CNT_MASK                            0x0000FFFF

#define TX_HSUPA_CLKCNT_OFFSET                                            0x0000004C
#define TX_HSUPA_CLKCNT_TYPE                                              UInt32
#define TX_HSUPA_CLKCNT_RESERVED_MASK                                     0xFFFE0000
#define    TX_HSUPA_CLKCNT_HSUPA_CLK_CNT_SHIFT                            0
#define    TX_HSUPA_CLKCNT_HSUPA_CLK_CNT_MASK                             0x0001FFFF

#define TX_HSUPA_FRBUFCTRL_OFFSET                                         0x00000050
#define TX_HSUPA_FRBUFCTRL_TYPE                                           UInt32
#define TX_HSUPA_FRBUFCTRL_RESERVED_MASK                                  0xFEEE0000
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_MODE_SHIFT                       24
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_MODE_MASK                        0x01000000
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_RD_EN_SHIFT                      20
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_RD_EN_MASK                       0x00100000
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_WR_EN_SHIFT                      16
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_WR_EN_MASK                       0x00010000
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_ADDR_SHIFT                       0
#define    TX_HSUPA_FRBUFCTRL_HSUPA_PHCH_ADDR_MASK                        0x0000FFFF

#define TX_HSUPA_FRBUFWRDATA_OFFSET                                       0x00000054
#define TX_HSUPA_FRBUFWRDATA_TYPE                                         UInt32
#define TX_HSUPA_FRBUFWRDATA_RESERVED_MASK                                0xFFFF0000
#define    TX_HSUPA_FRBUFWRDATA_HSUPA_PHCH_WR_DATA_SHIFT                  0
#define    TX_HSUPA_FRBUFWRDATA_HSUPA_PHCH_WR_DATA_MASK                   0x0000FFFF

#define TX_HSUPA_FRBUFRDDATA_OFFSET                                       0x00000058
#define TX_HSUPA_FRBUFRDDATA_TYPE                                         UInt32
#define TX_HSUPA_FRBUFRDDATA_RESERVED_MASK                                0xFFEF0000
#define    TX_HSUPA_FRBUFRDDATA_HSUPA_PHCH_BUFF_PTR_SHIFT                 20
#define    TX_HSUPA_FRBUFRDDATA_HSUPA_PHCH_BUFF_PTR_MASK                  0x00100000
#define    TX_HSUPA_FRBUFRDDATA_HSUPA_PHCH_RD_DATA_SHIFT                  0
#define    TX_HSUPA_FRBUFRDDATA_HSUPA_PHCH_RD_DATA_MASK                   0x0000FFFF

#define TX_WCDMA_BETA_OFFSET                                              0x0000005C
#define TX_WCDMA_BETA_TYPE                                                UInt32
#define TX_WCDMA_BETA_RESERVED_MASK                                       0xFFFFF000
#define    TX_WCDMA_BETA_PHCHPARAM_BETA_C_SHIFT                           8
#define    TX_WCDMA_BETA_PHCHPARAM_BETA_C_MASK                            0x00000F00
#define    TX_WCDMA_BETA_PHCHPARAM_BETA_D_X15_SHIFT                       0
#define    TX_WCDMA_BETA_PHCHPARAM_BETA_D_X15_MASK                        0x000000FF

#define TX_WCDMA_CTRLSFOVSF_OFFSET                                        0x00000060
#define TX_WCDMA_CTRLSFOVSF_TYPE                                          UInt32
#define TX_WCDMA_CTRLSFOVSF_RESERVED_MASK                                 0xFFFF0003
#define    TX_WCDMA_CTRLSFOVSF_PHCHPARAM_DPCCH_OVSF_SHIFT                 8
#define    TX_WCDMA_CTRLSFOVSF_PHCHPARAM_DPCCH_OVSF_MASK                  0x0000FF00
#define    TX_WCDMA_CTRLSFOVSF_PHCHPARAM_DPCCH_SF_SHIFT                   2
#define    TX_WCDMA_CTRLSFOVSF_PHCHPARAM_DPCCH_SF_MASK                    0x000000FC

#define TX_WCDMA_DATASFOVSF_OFFSET                                        0x00000064
#define TX_WCDMA_DATASFOVSF_TYPE                                          UInt32
#define TX_WCDMA_DATASFOVSF_RESERVED_MASK                                 0xC0000003
#define    TX_WCDMA_DATASFOVSF_PHCHPARAM_DPDCH_PHCHSIZE_SHIFT             16
#define    TX_WCDMA_DATASFOVSF_PHCHPARAM_DPDCH_PHCHSIZE_MASK              0x3FFF0000
#define    TX_WCDMA_DATASFOVSF_PHCHPARAM_DPDCH_OVSF_SHIFT                 8
#define    TX_WCDMA_DATASFOVSF_PHCHPARAM_DPDCH_OVSF_MASK                  0x0000FF00
#define    TX_WCDMA_DATASFOVSF_PHCHPARAM_DPDCH_SF_SHIFT                   2
#define    TX_WCDMA_DATASFOVSF_PHCHPARAM_DPDCH_SF_MASK                    0x000000FC

#define TX_WCDMA_SLOTSETUP_OFFSET                                         0x00000068
#define TX_WCDMA_SLOTSETUP_TYPE                                           UInt32
#define TX_WCDMA_SLOTSETUP_RESERVED_MASK                                  0xFFEE0000
#define    TX_WCDMA_SLOTSETUP_PHCHPARAM_DPDCH_ENABLE_SHIFT                20
#define    TX_WCDMA_SLOTSETUP_PHCHPARAM_DPDCH_ENABLE_MASK                 0x00100000
#define    TX_WCDMA_SLOTSETUP_PHCHPARAM_DPCCH_ENABLE_SHIFT                16
#define    TX_WCDMA_SLOTSETUP_PHCHPARAM_DPCCH_ENABLE_MASK                 0x00010000
#define    TX_WCDMA_SLOTSETUP_PHCHPARAM_DPCCH_DATA_SHIFT                  0
#define    TX_WCDMA_SLOTSETUP_PHCHPARAM_DPCCH_DATA_MASK                   0x0000FFFF

#define TX_WCDMA_SCRXINIT_OFFSET                                          0x0000006C
#define TX_WCDMA_SCRXINIT_TYPE                                            UInt32
#define TX_WCDMA_SCRXINIT_RESERVED_MASK                                   0xEE000000
#define    TX_WCDMA_SCRXINIT_PHCHPARAM_SCRLONG_SHIFT                      28
#define    TX_WCDMA_SCRXINIT_PHCHPARAM_SCRLONG_MASK                       0x10000000
#define    TX_WCDMA_SCRXINIT_PHCHPARAM_SCRXINIT_SHIFT                     0
#define    TX_WCDMA_SCRXINIT_PHCHPARAM_SCRXINIT_MASK                      0x01FFFFFF

#define TX_WCDMA_SCRYINIT_OFFSET                                          0x00000070
#define TX_WCDMA_SCRYINIT_TYPE                                            UInt32
#define TX_WCDMA_SCRYINIT_RESERVED_MASK                                   0xFE000000
#define    TX_WCDMA_SCRYINIT_PHCHPARAM_SCRYINIT_SHIFT                     0
#define    TX_WCDMA_SCRYINIT_PHCHPARAM_SCRYINIT_MASK                      0x01FFFFFF

#define TX_WCDMA_SCRAMBLE_X_READ_OFFSET                                   0x00000074
#define TX_WCDMA_SCRAMBLE_X_READ_TYPE                                     UInt32
#define TX_WCDMA_SCRAMBLE_X_READ_RESERVED_MASK                            0xFE000000
#define    TX_WCDMA_SCRAMBLE_X_READ_SCRAMBLE_X_READ_SHIFT                 0
#define    TX_WCDMA_SCRAMBLE_X_READ_SCRAMBLE_X_READ_MASK                  0x01FFFFFF

#define TX_WCDMA_SCRAMBLE_Y_READ_OFFSET                                   0x00000078
#define TX_WCDMA_SCRAMBLE_Y_READ_TYPE                                     UInt32
#define TX_WCDMA_SCRAMBLE_Y_READ_RESERVED_MASK                            0xFE000000
#define    TX_WCDMA_SCRAMBLE_Y_READ_SCRAMBLE_Y_READ_SHIFT                 0
#define    TX_WCDMA_SCRAMBLE_Y_READ_SCRAMBLE_Y_READ_MASK                  0x01FFFFFF

#define TX_WCDMA_CHIPFLAGS_OFFSET                                         0x0000007C
#define TX_WCDMA_CHIPFLAGS_TYPE                                           UInt32
#define TX_WCDMA_CHIPFLAGS_RESERVED_MASK                                  0xFFFEEEEE
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_LOADBCBD_SHIFT                    16
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_LOADBCBD_MASK                     0x00010000
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_SCRENABLE_SHIFT                   12
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_SCRENABLE_MASK                    0x00001000
#define    TX_WCDMA_CHIPFLAGS_HSUPA_RFS_SLOT_SHIFT                        8
#define    TX_WCDMA_CHIPFLAGS_HSUPA_RFS_SLOT_MASK                         0x00000100
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_PREAMBLE_SLOT_SHIFT               4
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_PREAMBLE_SLOT_MASK                0x00000010
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_RFS_SLOT_SHIFT                    0
#define    TX_WCDMA_CHIPFLAGS_PHCHPARAM_RFS_SLOT_MASK                     0x00000001

#define TX_WCDMA_TRBLOCKSETPTR_OFFSET                                     0x00000080
#define TX_WCDMA_TRBLOCKSETPTR_TYPE                                       UInt32
#define TX_WCDMA_TRBLOCKSETPTR_RESERVED_MASK                              0x00000003
#define    TX_WCDMA_TRBLOCKSETPTR_TRBLOCKSETPTR_SHIFT                     2
#define    TX_WCDMA_TRBLOCKSETPTR_TRBLOCKSETPTR_MASK                      0xFFFFFFFC

#define TX_WCDMA_TRBLOCKSETUP_OFFSET                                      0x00000084
#define TX_WCDMA_TRBLOCKSETUP_TYPE                                        UInt32
#define TX_WCDMA_TRBLOCKSETUP_RESERVED_MASK                               0xFFC0E000
#define    TX_WCDMA_TRBLOCKSETUP_NUMTRBLOCKS_SHIFT                        16
#define    TX_WCDMA_TRBLOCKSETUP_NUMTRBLOCKS_MASK                         0x003F0000
#define    TX_WCDMA_TRBLOCKSETUP_TRBLOCKLENGTH_SHIFT                      0
#define    TX_WCDMA_TRBLOCKSETUP_TRBLOCKLENGTH_MASK                       0x00001FFF

#define TX_WCDMA_TRCHSETUP_OFFSET                                         0x00000088
#define TX_WCDMA_TRCHSETUP_TYPE                                           UInt32
#define TX_WCDMA_TRCHSETUP_RESERVED_MASK                                  0xE000C08C
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_CODEBLOCKLENGTH_SHIFT             16
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_CODEBLOCKLENGTH_MASK              0x1FFF0000
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_NUMFILLERBITS_SHIFT               8
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_NUMFILLERBITS_MASK                0x00003F00
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_CRCTYPE_SHIFT                     4
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_CRCTYPE_MASK                      0x00000070
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_CODINGTYPE_SHIFT                  0
#define    TX_WCDMA_TRCHSETUP_TRCHPARAM_CODINGTYPE_MASK                   0x00000003

#define TX_WCDMA_TURBOSETUP0_OFFSET                                       0x0000008C
#define TX_WCDMA_TURBOSETUP0_TYPE                                         UInt32
#define TX_WCDMA_TURBOSETUP0_RESERVED_MASK                                0x8080E0E0
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_SKIP2_SHIFT                     24
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_SKIP2_MASK                      0x7F000000
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_SKIP1_SHIFT                     16
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_SKIP1_MASK                      0x007F0000
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_V_SHIFT                         8
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_V_MASK                          0x00001F00
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_R_SHIFT                         0
#define    TX_WCDMA_TURBOSETUP0_TRCHPARAM_R_MASK                          0x0000001F

#define TX_WCDMA_TURBOSETUP1_OFFSET                                       0x00000090
#define TX_WCDMA_TURBOSETUP1_TYPE                                         UInt32
#define TX_WCDMA_TURBOSETUP1_RESERVED_MASK                                0x8E00FE00
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_KRC_SHIFT                       30
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_KRC_MASK                        0x40000000
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_TSEL_SHIFT                      28
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_TSEL_MASK                       0x30000000
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_PRIME_SHIFT                     16
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_PRIME_MASK                      0x01FF0000
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_C_SHIFT                         0
#define    TX_WCDMA_TURBOSETUP1_TRCHPARAM_C_MASK                          0x000001FF

#define TX_WCDMA_TTIRMSETUP_OFFSET                                        0x00000094
#define TX_WCDMA_TTIRMSETUP_TYPE                                          UInt32
#define TX_WCDMA_TTIRMSETUP_RESERVED_MASK                                 0x4E000008
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_RMPUNCTURE_SHIFT                 31
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_RMPUNCTURE_MASK                  0x80000000
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_RMSEQUENCE_SHIFT                 28
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_RMSEQUENCE_MASK                  0x30000000
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_NBITS2RM_SHIFT                   8
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_NBITS2RM_MASK                    0x01FFFF00
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_FI_SHIFT                         4
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_FI_MASK                          0x000000F0
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_P1FNI_SHIFT                      0
#define    TX_WCDMA_TTIRMSETUP_TRCHPARAM_P1FNI_MASK                       0x00000007

#define TX_WCDMA_PARITY_A_EINIT_OFFSET                                    0x00000098
#define TX_WCDMA_PARITY_A_EINIT_TYPE                                      UInt32
#define TX_WCDMA_PARITY_A_EINIT_RESERVED_MASK                             0xFFF80000
#define    TX_WCDMA_PARITY_A_EINIT_TRCHPARAM_PARITY_A_EINIT_SHIFT         0
#define    TX_WCDMA_PARITY_A_EINIT_TRCHPARAM_PARITY_A_EINIT_MASK          0x0007FFFF

#define TX_WCDMA_PARITY_A_EPLUS_OFFSET                                    0x0000009C
#define TX_WCDMA_PARITY_A_EPLUS_TYPE                                      UInt32
#define TX_WCDMA_PARITY_A_EPLUS_RESERVED_MASK                             0xFFF80000
#define    TX_WCDMA_PARITY_A_EPLUS_TRCHPARAM_PARITY_A_EPLUS_SHIFT         0
#define    TX_WCDMA_PARITY_A_EPLUS_TRCHPARAM_PARITY_A_EPLUS_MASK          0x0007FFFF

#define TX_WCDMA_PARITY_A_EMINUS_OFFSET                                   0x000000A0
#define TX_WCDMA_PARITY_A_EMINUS_TYPE                                     UInt32
#define TX_WCDMA_PARITY_A_EMINUS_RESERVED_MASK                            0xFFF80000
#define    TX_WCDMA_PARITY_A_EMINUS_TRCHPARAM_PARITY_A_EMINUS_SHIFT       0
#define    TX_WCDMA_PARITY_A_EMINUS_TRCHPARAM_PARITY_A_EMINUS_MASK        0x0007FFFF

#define TX_WCDMA_PARITY_B_EINIT_OFFSET                                    0x000000A4
#define TX_WCDMA_PARITY_B_EINIT_TYPE                                      UInt32
#define TX_WCDMA_PARITY_B_EINIT_RESERVED_MASK                             0xFFF80000
#define    TX_WCDMA_PARITY_B_EINIT_TRCHPARAM_PARITY_B_EINIT_SHIFT         0
#define    TX_WCDMA_PARITY_B_EINIT_TRCHPARAM_PARITY_B_EINIT_MASK          0x0007FFFF

#define TX_WCDMA_PARITY_B_EPLUS_OFFSET                                    0x000000A8
#define TX_WCDMA_PARITY_B_EPLUS_TYPE                                      UInt32
#define TX_WCDMA_PARITY_B_EPLUS_RESERVED_MASK                             0xFFF80000
#define    TX_WCDMA_PARITY_B_EPLUS_TRCHPARAM_PARITY_B_EPLUS_SHIFT         0
#define    TX_WCDMA_PARITY_B_EPLUS_TRCHPARAM_PARITY_B_EPLUS_MASK          0x0007FFFF

#define TX_WCDMA_PARITY_B_EMINUS_OFFSET                                   0x000000AC
#define TX_WCDMA_PARITY_B_EMINUS_TYPE                                     UInt32
#define TX_WCDMA_PARITY_B_EMINUS_RESERVED_MASK                            0xFFF80000
#define    TX_WCDMA_PARITY_B_EMINUS_TRCHPARAM_PARITY_B_EMINUS_SHIFT       0
#define    TX_WCDMA_PARITY_B_EMINUS_TRCHPARAM_PARITY_B_EMINUS_MASK        0x0007FFFF

#define TX_WCDMA_TRIGGER_TRCH_OFFSET                                      0x000000B0
#define TX_WCDMA_TRIGGER_TRCH_TYPE                                        UInt32
#define TX_WCDMA_TRIGGER_TRCH_RESERVED_MASK                               0xFFFFFFFE
#define    TX_WCDMA_TRIGGER_TRCH_TRIGGER_TRCH_SHIFT                       0
#define    TX_WCDMA_TRIGGER_TRCH_TRIGGER_TRCH_MASK                        0x00000001

#define TX_WCDMA_CFGBITS_OFFSET                                           0x000000B4
#define TX_WCDMA_CFGBITS_TYPE                                             UInt32
#define TX_WCDMA_CFGBITS_RESERVED_MASK                                    0xFFFE88EE
#define    TX_WCDMA_CFGBITS_BB_IQ_TEST_SHIFT                              16
#define    TX_WCDMA_CFGBITS_BB_IQ_TEST_MASK                               0x00010000
#define    TX_WCDMA_CFGBITS_D2090_TRAIL_CHIP_EN_SHIFT                     14
#define    TX_WCDMA_CFGBITS_D2090_TRAIL_CHIP_EN_MASK                      0x00004000
#define    TX_WCDMA_CFGBITS_D2090_CM_CHIP_EN_SHIFT                        13
#define    TX_WCDMA_CFGBITS_D2090_CM_CHIP_EN_MASK                         0x00002000
#define    TX_WCDMA_CFGBITS_D2090_CM_SLOT_EN_SHIFT                        12
#define    TX_WCDMA_CFGBITS_D2090_CM_SLOT_EN_MASK                         0x00001000
#define    TX_WCDMA_CFGBITS_DIGRF_TRAIL_CHIP_EN_SHIFT                     10
#define    TX_WCDMA_CFGBITS_DIGRF_TRAIL_CHIP_EN_MASK                      0x00000400
#define    TX_WCDMA_CFGBITS_DIGRF_CM_CHIP_EN_SHIFT                        9
#define    TX_WCDMA_CFGBITS_DIGRF_CM_CHIP_EN_MASK                         0x00000200
#define    TX_WCDMA_CFGBITS_DIGRF_CM_SLOT_EN_SHIFT                        8
#define    TX_WCDMA_CFGBITS_DIGRF_CM_SLOT_EN_MASK                         0x00000100
#define    TX_WCDMA_CFGBITS_CHIPPOLARITY_SHIFT                            4
#define    TX_WCDMA_CFGBITS_CHIPPOLARITY_MASK                             0x00000010
#define    TX_WCDMA_CFGBITS_CFGBIGEND_SHIFT                               0
#define    TX_WCDMA_CFGBITS_CFGBIGEND_MASK                                0x00000001

#define TX_WCDMA_UNBUFFERED_CTRLCH_DATA_OFFSET                            0x000000B8
#define TX_WCDMA_UNBUFFERED_CTRLCH_DATA_TYPE                              UInt32
#define TX_WCDMA_UNBUFFERED_CTRLCH_DATA_RESERVED_MASK                     0xFFFFFC03
#define    TX_WCDMA_UNBUFFERED_CTRLCH_DATA_PHCHPARAM_UNBUFFERED_DPCCH_DATA_SHIFT 2
#define    TX_WCDMA_UNBUFFERED_CTRLCH_DATA_PHCHPARAM_UNBUFFERED_DPCCH_DATA_MASK 0x000003FC

#define TX_HSDPA_HSCTRLSETUP_OFFSET                                       0x000000BC
#define TX_HSDPA_HSCTRLSETUP_TYPE                                         UInt32
#define TX_HSDPA_HSCTRLSETUP_RESERVED_MASK                                0xAC0000C0
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_HS_DPCCH_ENABLE_SHIFT           30
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_HS_DPCCH_ENABLE_MASK            0x40000000
#define    TX_HSDPA_HSCTRLSETUP_TRCHDEBUG_SHIFT                           28
#define    TX_HSDPA_HSCTRLSETUP_TRCHDEBUG_MASK                            0x10000000
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_HS_DPCCH_DATA_SHIFT             16
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_HS_DPCCH_DATA_MASK              0x03FF0000
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_HS_DPCCH_OVSF_SHIFT             8
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_HS_DPCCH_OVSF_MASK              0x0000FF00
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_AHS_X15_SHIFT                   0
#define    TX_HSDPA_HSCTRLSETUP_PHCHPARAM_AHS_X15_MASK                    0x0000003F

#define TX_HSUPA_EDPCCH_OFFSET                                            0x000000C0
#define TX_HSUPA_EDPCCH_TYPE                                              UInt32
#define TX_HSUPA_EDPCCH_RESERVED_MASK                                     0x400E0000
#define    TX_HSUPA_EDPCCH_PHCHPARAM_E_DPCCH_ENABLE_SHIFT                 31
#define    TX_HSUPA_EDPCCH_PHCHPARAM_E_DPCCH_ENABLE_MASK                  0x80000000
#define    TX_HSUPA_EDPCCH_PHCHPARAM_E_DPCCH_DATA_SHIFT                   20
#define    TX_HSUPA_EDPCCH_PHCHPARAM_E_DPCCH_DATA_MASK                    0x3FF00000
#define    TX_HSUPA_EDPCCH_PHCHPARAM_AEC_X15_SHIFT                        8
#define    TX_HSUPA_EDPCCH_PHCHPARAM_AEC_X15_MASK                         0x0001FF00
#define    TX_HSUPA_EDPCCH_PHCHPARAM_E_DPCCH_OVSF_SHIFT                   0
#define    TX_HSUPA_EDPCCH_PHCHPARAM_E_DPCCH_OVSF_MASK                    0x000000FF

#define TX_HSUPA_EDPDCH1_OFFSET                                           0x000000C4
#define TX_HSUPA_EDPDCH1_TYPE                                             UInt32
#define TX_HSUPA_EDPDCH1_RESERVED_MASK                                    0x0E000001
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_SWAP_IQ_SHIFT             31
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_SWAP_IQ_MASK              0x80000000
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_INVERT_Q_SHIFT            30
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_INVERT_Q_MASK             0x40000000
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_INVERT_I_SHIFT            29
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_INVERT_I_MASK             0x20000000
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_ENABLE_SHIFT              28
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_ENABLE_MASK               0x10000000
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_AED_1_X15_SHIFT                     16
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_AED_1_X15_MASK                      0x01FF0000
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_OVSF_SHIFT                8
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_OVSF_MASK                 0x0000FF00
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_SF_SHIFT                  1
#define    TX_HSUPA_EDPDCH1_PHCHPARAM_E_DPDCH_1_SF_MASK                   0x000000FE

#define TX_HSUPA_EDPDCH2_OFFSET                                           0x000000C8
#define TX_HSUPA_EDPDCH2_TYPE                                             UInt32
#define TX_HSUPA_EDPDCH2_RESERVED_MASK                                    0x0E000001
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_SWAP_IQ_SHIFT             31
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_SWAP_IQ_MASK              0x80000000
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_INVERT_Q_SHIFT            30
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_INVERT_Q_MASK             0x40000000
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_INVERT_I_SHIFT            29
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_INVERT_I_MASK             0x20000000
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_ENABLE_SHIFT              28
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_ENABLE_MASK               0x10000000
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_AED_2_X15_SHIFT                     16
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_AED_2_X15_MASK                      0x01FF0000
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_OVSF_SHIFT                8
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_OVSF_MASK                 0x0000FF00
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_SF_SHIFT                  1
#define    TX_HSUPA_EDPDCH2_PHCHPARAM_E_DPDCH_2_SF_MASK                   0x000000FE

#define TX_HSUPA_EDPDCH3_OFFSET                                           0x000000CC
#define TX_HSUPA_EDPDCH3_TYPE                                             UInt32
#define TX_HSUPA_EDPDCH3_RESERVED_MASK                                    0x0E000001
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_SWAP_IQ_SHIFT             31
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_SWAP_IQ_MASK              0x80000000
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_INVERT_Q_SHIFT            30
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_INVERT_Q_MASK             0x40000000
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_INVERT_I_SHIFT            29
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_INVERT_I_MASK             0x20000000
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_ENABLE_SHIFT              28
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_ENABLE_MASK               0x10000000
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_AED_3_X15_SHIFT                     16
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_AED_3_X15_MASK                      0x01FF0000
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_OVSF_SHIFT                8
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_OVSF_MASK                 0x0000FF00
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_SF_SHIFT                  1
#define    TX_HSUPA_EDPDCH3_PHCHPARAM_E_DPDCH_3_SF_MASK                   0x000000FE

#define TX_HSUPA_EDPDCH4_OFFSET                                           0x000000D0
#define TX_HSUPA_EDPDCH4_TYPE                                             UInt32
#define TX_HSUPA_EDPDCH4_RESERVED_MASK                                    0x0E000001
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_SWAP_IQ_SHIFT             31
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_SWAP_IQ_MASK              0x80000000
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_INVERT_Q_SHIFT            30
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_INVERT_Q_MASK             0x40000000
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_INVERT_I_SHIFT            29
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_INVERT_I_MASK             0x20000000
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_ENABLE_SHIFT              28
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_ENABLE_MASK               0x10000000
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_AED_4_X15_SHIFT                     16
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_AED_4_X15_MASK                      0x01FF0000
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_OVSF_SHIFT                8
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_OVSF_MASK                 0x0000FF00
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_SF_SHIFT                  1
#define    TX_HSUPA_EDPDCH4_PHCHPARAM_E_DPDCH_4_SF_MASK                   0x000000FE

#define TX_HSUPA_EDPDCH12_SIZE_OFFSET                                     0x000000D4
#define TX_HSUPA_EDPDCH12_SIZE_TYPE                                       UInt32
#define TX_HSUPA_EDPDCH12_SIZE_RESERVED_MASK                              0x00000000
#define    TX_HSUPA_EDPDCH12_SIZE_E_DPDCH_2_PHCHSIZE_SHIFT                16
#define    TX_HSUPA_EDPDCH12_SIZE_E_DPDCH_2_PHCHSIZE_MASK                 0xFFFF0000
#define    TX_HSUPA_EDPDCH12_SIZE_E_DPDCH_1_PHCHSIZE_SHIFT                0
#define    TX_HSUPA_EDPDCH12_SIZE_E_DPDCH_1_PHCHSIZE_MASK                 0x0000FFFF

#define TX_HSUPA_EDPDCH34_SIZE_OFFSET                                     0x000000D8
#define TX_HSUPA_EDPDCH34_SIZE_TYPE                                       UInt32
#define TX_HSUPA_EDPDCH34_SIZE_RESERVED_MASK                              0x00000000
#define    TX_HSUPA_EDPDCH34_SIZE_E_DPDCH_4_PHCHSIZE_SHIFT                16
#define    TX_HSUPA_EDPDCH34_SIZE_E_DPDCH_4_PHCHSIZE_MASK                 0xFFFF0000
#define    TX_HSUPA_EDPDCH34_SIZE_E_DPDCH_3_PHCHSIZE_SHIFT                0
#define    TX_HSUPA_EDPDCH34_SIZE_E_DPDCH_3_PHCHSIZE_MASK                 0x0000FFFF

#endif /* __BRCM_RDB_TX_H__ */



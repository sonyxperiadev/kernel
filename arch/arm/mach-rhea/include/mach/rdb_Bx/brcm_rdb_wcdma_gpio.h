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

#ifndef __BRCM_RDB_WCDMA_GPIO_H__
#define __BRCM_RDB_WCDMA_GPIO_H__

#define WCDMA_GPIO_OUT_RW_OFFSET                                          0x00000000
#define WCDMA_GPIO_OUT_RW_TYPE                                            UInt32
#define WCDMA_GPIO_OUT_RW_RESERVED_MASK                                   0xFFFFC0FF
#define    WCDMA_GPIO_OUT_RW_RFGPIO_OUT_SHIFT                             8
#define    WCDMA_GPIO_OUT_RW_RFGPIO_OUT_MASK                              0x00003F00

#define WCDMA_GPIO_OUT_RW_CLR_OFFSET                                      0x00000004
#define WCDMA_GPIO_OUT_RW_CLR_TYPE                                        UInt32
#define WCDMA_GPIO_OUT_RW_CLR_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_OUT_RW_CLR_CLEAR_SHIFT                              0
#define    WCDMA_GPIO_OUT_RW_CLR_CLEAR_MASK                               0xFFFFFFFF

#define WCDMA_GPIO_OUT_RW_SET_OFFSET                                      0x00000008
#define WCDMA_GPIO_OUT_RW_SET_TYPE                                        UInt32
#define WCDMA_GPIO_OUT_RW_SET_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_OUT_RW_SET_SET_SHIFT                                0
#define    WCDMA_GPIO_OUT_RW_SET_SET_MASK                                 0xFFFFFFFF

#define WCDMA_GPIO_IN_R_OFFSET                                            0x0000000C
#define WCDMA_GPIO_IN_R_TYPE                                              UInt32
#define WCDMA_GPIO_IN_R_RESERVED_MASK                                     0xFFFFC0FF
#define    WCDMA_GPIO_IN_R_RFGPIO_IN_SHIFT                                8
#define    WCDMA_GPIO_IN_R_RFGPIO_IN_MASK                                 0x00003F00

#define WCDMA_GPIO_OEN_RW_OFFSET                                          0x00000010
#define WCDMA_GPIO_OEN_RW_TYPE                                            UInt32
#define WCDMA_GPIO_OEN_RW_RESERVED_MASK                                   0xFFFFC0FF
#define    WCDMA_GPIO_OEN_RW_RFGPIO_OEN_RB_111111_SHIFT                   8
#define    WCDMA_GPIO_OEN_RW_RFGPIO_OEN_RB_111111_MASK                    0x00003F00

#define WCDMA_GPIO_OEN_RW_CLR_OFFSET                                      0x00000014
#define WCDMA_GPIO_OEN_RW_CLR_TYPE                                        UInt32
#define WCDMA_GPIO_OEN_RW_CLR_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_OEN_RW_CLR_CLEAR_SHIFT                              0
#define    WCDMA_GPIO_OEN_RW_CLR_CLEAR_MASK                               0xFFFFFFFF

#define WCDMA_GPIO_OEN_RW_SET_OFFSET                                      0x00000018
#define WCDMA_GPIO_OEN_RW_SET_TYPE                                        UInt32
#define WCDMA_GPIO_OEN_RW_SET_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_OEN_RW_SET_SET_SHIFT                                0
#define    WCDMA_GPIO_OEN_RW_SET_SET_MASK                                 0xFFFFFFFF

#define WCDMA_GPIO_GPO_RW_OFFSET                                          0x00000020
#define WCDMA_GPIO_GPO_RW_TYPE                                            UInt32
#define WCDMA_GPIO_GPO_RW_RESERVED_MASK                                   0xFFFF0000
#define    WCDMA_GPIO_GPO_RW_GPO_SHIFT                                    0
#define    WCDMA_GPIO_GPO_RW_GPO_MASK                                     0x0000FFFF

#define WCDMA_GPIO_GPO_RW_CLR_OFFSET                                      0x00000024
#define WCDMA_GPIO_GPO_RW_CLR_TYPE                                        UInt32
#define WCDMA_GPIO_GPO_RW_CLR_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_GPO_RW_CLR_CLEAR_SHIFT                              0
#define    WCDMA_GPIO_GPO_RW_CLR_CLEAR_MASK                               0xFFFFFFFF

#define WCDMA_GPIO_GPO_RW_SET_OFFSET                                      0x00000028
#define WCDMA_GPIO_GPO_RW_SET_TYPE                                        UInt32
#define WCDMA_GPIO_GPO_RW_SET_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_GPO_RW_SET_SET_SHIFT                                0
#define    WCDMA_GPIO_GPO_RW_SET_SET_MASK                                 0xFFFFFFFF

#define WCDMA_GPIO_GPWEN_RW_OFFSET                                        0x00000040
#define WCDMA_GPIO_GPWEN_RW_TYPE                                          UInt32
#define WCDMA_GPIO_GPWEN_RW_RESERVED_MASK                                 0xFFFF0000
#define    WCDMA_GPIO_GPWEN_RW_GPWEN_SHIFT                                0
#define    WCDMA_GPIO_GPWEN_RW_GPWEN_MASK                                 0x0000FFFF

#define WCDMA_GPIO_GPWEN_RW_CLR_OFFSET                                    0x00000044
#define WCDMA_GPIO_GPWEN_RW_CLR_TYPE                                      UInt32
#define WCDMA_GPIO_GPWEN_RW_CLR_RESERVED_MASK                             0x00000000
#define    WCDMA_GPIO_GPWEN_RW_CLR_CLEAR_SHIFT                            0
#define    WCDMA_GPIO_GPWEN_RW_CLR_CLEAR_MASK                             0xFFFFFFFF

#define WCDMA_GPIO_GPWEN_RW_SET_OFFSET                                    0x00000048
#define WCDMA_GPIO_GPWEN_RW_SET_TYPE                                      UInt32
#define WCDMA_GPIO_GPWEN_RW_SET_RESERVED_MASK                             0x00000000
#define    WCDMA_GPIO_GPWEN_RW_SET_SET_SHIFT                              0
#define    WCDMA_GPIO_GPWEN_RW_SET_SET_MASK                               0xFFFFFFFF

#define WCDMA_GPIO_CTRL_RFGPIO_RW_OFFSET                                  0x00000050
#define WCDMA_GPIO_CTRL_RFGPIO_RW_TYPE                                    UInt32
#define WCDMA_GPIO_CTRL_RFGPIO_RW_RESERVED_MASK                           0xFFCCCCCC
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO5_SHIFT                   20
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO5_MASK                    0x00300000
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO4_SHIFT                   16
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO4_MASK                    0x00030000
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO3_SHIFT                   12
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO3_MASK                    0x00003000
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO2_SHIFT                   8
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO2_MASK                    0x00000300
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO1_SHIFT                   4
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO1_MASK                    0x00000030
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO0_SHIFT                   0
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CTRL_RFGPIO0_MASK                    0x00000003

#define WCDMA_GPIO_CTRL_RFGPIO_RW_CLR_OFFSET                              0x00000054
#define WCDMA_GPIO_CTRL_RFGPIO_RW_CLR_TYPE                                UInt32
#define WCDMA_GPIO_CTRL_RFGPIO_RW_CLR_RESERVED_MASK                       0x00000000
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CLR_CLEAR_SHIFT                      0
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_CLR_CLEAR_MASK                       0xFFFFFFFF

#define WCDMA_GPIO_CTRL_RFGPIO_RW_SET_OFFSET                              0x00000058
#define WCDMA_GPIO_CTRL_RFGPIO_RW_SET_TYPE                                UInt32
#define WCDMA_GPIO_CTRL_RFGPIO_RW_SET_RESERVED_MASK                       0x00000000
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_SET_SET_SHIFT                        0
#define    WCDMA_GPIO_CTRL_RFGPIO_RW_SET_SET_MASK                         0xFFFFFFFF

#define WCDMA_GPIO_CFG_DBG_OFFSET                                         0x0000005C
#define WCDMA_GPIO_CFG_DBG_TYPE                                           UInt32
#define WCDMA_GPIO_CFG_DBG_RESERVED_MASK                                  0xFFFFFFC8
#define    WCDMA_GPIO_CFG_DBG_DEBUG_MODE_SHIFT                            4
#define    WCDMA_GPIO_CFG_DBG_DEBUG_MODE_MASK                             0x00000030
#define    WCDMA_GPIO_CFG_DBG_CFG_DBG_SHIFT                               0
#define    WCDMA_GPIO_CFG_DBG_CFG_DBG_MASK                                0x00000007

#define WCDMA_GPIO_DBG0SELECT_OFFSET                                      0x00000060
#define WCDMA_GPIO_DBG0SELECT_TYPE                                        UInt32
#define WCDMA_GPIO_DBG0SELECT_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT3_SHIFT                        24
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT3_MASK                         0xFF000000
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT2_SHIFT                        16
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT2_MASK                         0x00FF0000
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT1_SHIFT                        8
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT1_MASK                         0x0000FF00
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT0_SHIFT                        0
#define    WCDMA_GPIO_DBG0SELECT_DBG0SELECT0_MASK                         0x000000FF

#define WCDMA_GPIO_DBG1SELECT_OFFSET                                      0x00000064
#define WCDMA_GPIO_DBG1SELECT_TYPE                                        UInt32
#define WCDMA_GPIO_DBG1SELECT_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT3_SHIFT                        24
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT3_MASK                         0xFF000000
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT2_SHIFT                        16
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT2_MASK                         0x00FF0000
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT1_SHIFT                        8
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT1_MASK                         0x0000FF00
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT0_SHIFT                        0
#define    WCDMA_GPIO_DBG1SELECT_DBG1SELECT0_MASK                         0x000000FF

#define WCDMA_GPIO_DBG2SELECT_OFFSET                                      0x00000068
#define WCDMA_GPIO_DBG2SELECT_TYPE                                        UInt32
#define WCDMA_GPIO_DBG2SELECT_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT3_SHIFT                        24
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT3_MASK                         0xFF000000
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT2_SHIFT                        16
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT2_MASK                         0x00FF0000
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT1_SHIFT                        8
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT1_MASK                         0x0000FF00
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT0_SHIFT                        0
#define    WCDMA_GPIO_DBG2SELECT_DBG2SELECT0_MASK                         0x000000FF

#define WCDMA_GPIO_DBG3SELECT_OFFSET                                      0x0000006C
#define WCDMA_GPIO_DBG3SELECT_TYPE                                        UInt32
#define WCDMA_GPIO_DBG3SELECT_RESERVED_MASK                               0x00000000
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT3_SHIFT                        24
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT3_MASK                         0xFF000000
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT2_SHIFT                        16
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT2_MASK                         0x00FF0000
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT1_SHIFT                        8
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT1_MASK                         0x0000FF00
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT0_SHIFT                        0
#define    WCDMA_GPIO_DBG3SELECT_DBG3SELECT0_MASK                         0x000000FF

#define WCDMA_GPIO_DBGRFGPIOSLCT_OFFSET                                   0x00000070
#define WCDMA_GPIO_DBGRFGPIOSLCT_TYPE                                     UInt32
#define WCDMA_GPIO_DBGRFGPIOSLCT_RESERVED_MASK                            0xFFFFCCCC
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO3SLCT_SHIFT                  12
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO3SLCT_MASK                   0x00003000
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO2SLCT_SHIFT                  8
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO2SLCT_MASK                   0x00000300
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO1SLCT_SHIFT                  4
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO1SLCT_MASK                   0x00000030
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO0SLCT_SHIFT                  0
#define    WCDMA_GPIO_DBGRFGPIOSLCT_DBGRFGPIO0SLCT_MASK                   0x00000003

#define WCDMA_GPIO_DBGPORTSELECTCNT_OFFSET                                0x00000074
#define WCDMA_GPIO_DBGPORTSELECTCNT_TYPE                                  UInt32
#define WCDMA_GPIO_DBGPORTSELECTCNT_RESERVED_MASK                         0xFFFFFFEC
#define    WCDMA_GPIO_DBGPORTSELECTCNT_DBGPORTSELECTCNTENB_SHIFT          4
#define    WCDMA_GPIO_DBGPORTSELECTCNT_DBGPORTSELECTCNTENB_MASK           0x00000010
#define    WCDMA_GPIO_DBGPORTSELECTCNT_DBGPORTSELECTCNTINIT_SHIFT         0
#define    WCDMA_GPIO_DBGPORTSELECTCNT_DBGPORTSELECTCNTINIT_MASK          0x00000003

#define WCDMA_GPIO_CTL3_2_OFFSET                                          0x00000078
#define WCDMA_GPIO_CTL3_2_TYPE                                            UInt32
#define WCDMA_GPIO_CTL3_2_RESERVED_MASK                                   0xE000E000
#define    WCDMA_GPIO_CTL3_2_CTL3_CTRL_SHIFT                              16
#define    WCDMA_GPIO_CTL3_2_CTL3_CTRL_MASK                               0x1FFF0000
#define    WCDMA_GPIO_CTL3_2_CTL2_CTRL_SHIFT                              0
#define    WCDMA_GPIO_CTL3_2_CTL2_CTRL_MASK                               0x00001FFF

#define WCDMA_GPIO_CTL1_0_OFFSET                                          0x0000007C
#define WCDMA_GPIO_CTL1_0_TYPE                                            UInt32
#define WCDMA_GPIO_CTL1_0_RESERVED_MASK                                   0xE000E000
#define    WCDMA_GPIO_CTL1_0_CTL1_CTRL_SHIFT                              16
#define    WCDMA_GPIO_CTL1_0_CTL1_CTRL_MASK                               0x1FFF0000
#define    WCDMA_GPIO_CTL1_0_CTL0_CTRL_SHIFT                              0
#define    WCDMA_GPIO_CTL1_0_CTL0_CTRL_MASK                               0x00001FFF

#define WCDMA_GPIO_CTL_R_OFFSET                                           0x00000080
#define WCDMA_GPIO_CTL_R_TYPE                                             UInt32
#define WCDMA_GPIO_CTL_R_RESERVED_MASK                                    0xF0F00000
#define    WCDMA_GPIO_CTL_R_CTL_SHIFT                                     24
#define    WCDMA_GPIO_CTL_R_CTL_MASK                                      0x0F000000
#define    WCDMA_GPIO_CTL_R_DBGRFGPIO_SHIFT                               16
#define    WCDMA_GPIO_CTL_R_DBGRFGPIO_MASK                                0x000F0000
#define    WCDMA_GPIO_CTL_R_DBGPORT_SHIFT                                 0
#define    WCDMA_GPIO_CTL_R_DBGPORT_MASK                                  0x0000FFFF

#define WCDMA_GPIO_SW_DEBUGWORD_OFFSET                                    0x00000084
#define WCDMA_GPIO_SW_DEBUGWORD_TYPE                                      UInt32
#define WCDMA_GPIO_SW_DEBUGWORD_RESERVED_MASK                             0x00000000
#define    WCDMA_GPIO_SW_DEBUGWORD_SW_DEBUGWORD_SHIFT                     0
#define    WCDMA_GPIO_SW_DEBUGWORD_SW_DEBUGWORD_MASK                      0xFFFFFFFF

#define WCDMA_GPIO_EXTINT_OFFSET                                          0x00000088
#define WCDMA_GPIO_EXTINT_TYPE                                            UInt32
#define WCDMA_GPIO_EXTINT_RESERVED_MASK                                   0xFFFFFF78
#define    WCDMA_GPIO_EXTINT_EXTINT_INVERT_SHIFT                          7
#define    WCDMA_GPIO_EXTINT_EXTINT_INVERT_MASK                           0x00000080
#define    WCDMA_GPIO_EXTINT_EXTINT_GPIO_SLCT_SHIFT                       0
#define    WCDMA_GPIO_EXTINT_EXTINT_GPIO_SLCT_MASK                        0x00000007

#define WCDMA_GPIO_RFGPIO_PADS_HI_OFFSET                                  0x00000094
#define WCDMA_GPIO_RFGPIO_PADS_HI_TYPE                                    UInt32
#define WCDMA_GPIO_RFGPIO_PADS_HI_RESERVED_MASK                           0xFFFF8888
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_SEL_RB_010_SHIFT             12
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_SEL_RB_010_MASK              0x00007000
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_SLEW_R1_SHIFT                10
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_SLEW_R1_MASK                 0x00000400
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_PDN_R1_SHIFT                 9
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_PDN_R1_MASK                  0x00000200
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_PUP_SHIFT                    8
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO5_PUP_MASK                     0x00000100
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_SEL_RB_010_SHIFT             4
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_SEL_RB_010_MASK              0x00000070
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_SLEW_R1_SHIFT                2
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_SLEW_R1_MASK                 0x00000004
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_PDN_R1_SHIFT                 1
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_PDN_R1_MASK                  0x00000002
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_PUP_SHIFT                    0
#define    WCDMA_GPIO_RFGPIO_PADS_HI_RFGPIO4_PUP_MASK                     0x00000001

#define WCDMA_GPIO_RFGPIO_PADS_OFFSET                                     0x00000098
#define WCDMA_GPIO_RFGPIO_PADS_TYPE                                       UInt32
#define WCDMA_GPIO_RFGPIO_PADS_RESERVED_MASK                              0x88888888
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_SEL_RB_010_SHIFT                28
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_SEL_RB_010_MASK                 0x70000000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_SLEW_R1_SHIFT                   26
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_SLEW_R1_MASK                    0x04000000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_PDN_R1_SHIFT                    25
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_PDN_R1_MASK                     0x02000000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_PUP_SHIFT                       24
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO3_PUP_MASK                        0x01000000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_SEL_RB_010_SHIFT                20
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_SEL_RB_010_MASK                 0x00700000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_SLEW_R1_SHIFT                   18
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_SLEW_R1_MASK                    0x00040000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_PDN_R1_SHIFT                    17
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_PDN_R1_MASK                     0x00020000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_PUP_SHIFT                       16
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO2_PUP_MASK                        0x00010000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_SEL_RB_010_SHIFT                12
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_SEL_RB_010_MASK                 0x00007000
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_SLEW_R1_SHIFT                   10
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_SLEW_R1_MASK                    0x00000400
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_PDN_R1_SHIFT                    9
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_PDN_R1_MASK                     0x00000200
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_PUP_SHIFT                       8
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO1_PUP_MASK                        0x00000100
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_SEL_RB_010_SHIFT                4
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_SEL_RB_010_MASK                 0x00000070
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_SLEW_R1_SHIFT                   2
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_SLEW_R1_MASK                    0x00000004
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_PDN_R1_SHIFT                    1
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_PDN_R1_MASK                     0x00000002
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_PUP_SHIFT                       0
#define    WCDMA_GPIO_RFGPIO_PADS_RFGPIO0_PUP_MASK                        0x00000001

#define WCDMA_GPIO_MISC_PADS_OFFSET                                       0x0000009C
#define WCDMA_GPIO_MISC_PADS_TYPE                                         UInt32
#define WCDMA_GPIO_MISC_PADS_RESERVED_MASK                                0xFF8888FF
#define    WCDMA_GPIO_MISC_PADS_TWIF_SEL_RB_010_SHIFT                     20
#define    WCDMA_GPIO_MISC_PADS_TWIF_SEL_RB_010_MASK                      0x00700000
#define    WCDMA_GPIO_MISC_PADS_TWIF_SLEW_R1_SHIFT                        18
#define    WCDMA_GPIO_MISC_PADS_TWIF_SLEW_R1_MASK                         0x00040000
#define    WCDMA_GPIO_MISC_PADS_TWIFDATA_PDN_R1_SHIFT                     17
#define    WCDMA_GPIO_MISC_PADS_TWIFDATA_PDN_R1_MASK                      0x00020000
#define    WCDMA_GPIO_MISC_PADS_TWIFDATA_PUP_SHIFT                        16
#define    WCDMA_GPIO_MISC_PADS_TWIFDATA_PUP_MASK                         0x00010000
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_SEL_RB_010_SHIFT                   12
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_SEL_RB_010_MASK                    0x00007000
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_SLEW_SHIFT                         10
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_SLEW_MASK                          0x00000400
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_PDN_SHIFT                          9
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_PDN_MASK                           0x00000200
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_PUP_SHIFT                          8
#define    WCDMA_GPIO_MISC_PADS_AFCPDM_PUP_MASK                           0x00000100

#define WCDMA_GPIO_RFIC_SCHEDULER_OFFSET                                  0x000000A0
#define WCDMA_GPIO_RFIC_SCHEDULER_TYPE                                    UInt32
#define WCDMA_GPIO_RFIC_SCHEDULER_RESERVED_MASK                           0xFFFFEEEE
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_SPI2_SHIFT               12
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_SPI2_MASK                0x00001000
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_SPI1_SHIFT               8
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_SPI1_MASK                0x00000100
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_SPI0_SHIFT               4
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_SPI0_MASK                0x00000010
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_GPEN_SHIFT               0
#define    WCDMA_GPIO_RFIC_SCHEDULER_RFSCHED_MUX_GPEN_MASK                0x00000001

#endif /* __BRCM_RDB_WCDMA_GPIO_H__ */



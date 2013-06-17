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

#ifndef __BRCM_RDB_DSP_SYS_TIMER_H__
#define __BRCM_RDB_DSP_SYS_TIMER_H__

#define DSP_SYS_TIMER_QBCR_R_OFFSET                                       0x00000A00
#define DSP_SYS_TIMER_QBCR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_QBCR_R_RESERVED_MASK                                0x0000C000
#define    DSP_SYS_TIMER_QBCR_R_QBCR_SHIFT                                0
#define    DSP_SYS_TIMER_QBCR_R_QBCR_MASK                                 0x00003FFF

#define DSP_SYS_TIMER_CHAR_R_OFFSET                                       0x00000A02
#define DSP_SYS_TIMER_CHAR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_CHAR_R_RESERVED_MASK                                0x0000C000
#define    DSP_SYS_TIMER_CHAR_R_CHAR_SHIFT                                0
#define    DSP_SYS_TIMER_CHAR_R_CHAR_MASK                                 0x00003FFF

#define DSP_SYS_TIMER_CHBR_R_OFFSET                                       0x00000A04
#define DSP_SYS_TIMER_CHBR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_CHBR_R_RESERVED_MASK                                0x0000C000
#define    DSP_SYS_TIMER_CHBR_R_CHBR_SHIFT                                0
#define    DSP_SYS_TIMER_CHBR_R_CHBR_MASK                                 0x00003FFF

#define DSP_SYS_TIMER_FCR_R_OFFSET                                        0x00000A06
#define DSP_SYS_TIMER_FCR_R_TYPE                                          UInt16
#define DSP_SYS_TIMER_FCR_R_RESERVED_MASK                                 0x0000FC00
#define    DSP_SYS_TIMER_FCR_R_FCR_SHIFT                                  0
#define    DSP_SYS_TIMER_FCR_R_FCR_MASK                                   0x000003FF

#define DSP_SYS_TIMER_FCAR_R_OFFSET                                       0x00000A08
#define DSP_SYS_TIMER_FCAR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_FCAR_R_RESERVED_MASK                                0x0000FC00
#define    DSP_SYS_TIMER_FCAR_R_FCAR_SHIFT                                0
#define    DSP_SYS_TIMER_FCAR_R_FCAR_MASK                                 0x000003FF

#define DSP_SYS_TIMER_FCBR_R_OFFSET                                       0x00000A0A
#define DSP_SYS_TIMER_FCBR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_FCBR_R_RESERVED_MASK                                0x0000F000
#define    DSP_SYS_TIMER_FCBR_R_FCBR_LOAD_SHIFT                           11
#define    DSP_SYS_TIMER_FCBR_R_FCBR_LOAD_MASK                            0x00000800
#define    DSP_SYS_TIMER_FCBR_R_FCBR_LOADED_SHIFT                         10
#define    DSP_SYS_TIMER_FCBR_R_FCBR_LOADED_MASK                          0x00000400
#define    DSP_SYS_TIMER_FCBR_R_FCBR_SHIFT                                0
#define    DSP_SYS_TIMER_FCBR_R_FCBR_MASK                                 0x000003FF

#define DSP_SYS_TIMER_BCR_R_OFFSET                                        0x00000A0E
#define DSP_SYS_TIMER_BCR_R_TYPE                                          UInt16
#define DSP_SYS_TIMER_BCR_R_RESERVED_MASK                                 0x000080F0
#define    DSP_SYS_TIMER_BCR_R_BGEN_SHIFT                                 14
#define    DSP_SYS_TIMER_BCR_R_BGEN_MASK                                  0x00004000
#define    DSP_SYS_TIMER_BCR_R_BCLKSEL_SHIFT                              13
#define    DSP_SYS_TIMER_BCR_R_BCLKSEL_MASK                               0x00002000
#define    DSP_SYS_TIMER_BCR_R_IQTST_SHIFT                                12
#define    DSP_SYS_TIMER_BCR_R_IQTST_MASK                                 0x00001000
#define    DSP_SYS_TIMER_BCR_R_BRDTST_SHIFT                               11
#define    DSP_SYS_TIMER_BCR_R_BRDTST_MASK                                0x00000800
#define    DSP_SYS_TIMER_BCR_R_BRATST_SHIFT                               10
#define    DSP_SYS_TIMER_BCR_R_BRATST_MASK                                0x00000400
#define    DSP_SYS_TIMER_BCR_R_BTDTST_SHIFT                               9
#define    DSP_SYS_TIMER_BCR_R_BTDTST_MASK                                0x00000200
#define    DSP_SYS_TIMER_BCR_R_BTATST_SHIFT                               8
#define    DSP_SYS_TIMER_BCR_R_BTATST_MASK                                0x00000100
#define    DSP_SYS_TIMER_BCR_R_TXREFS_SHIFT                               3
#define    DSP_SYS_TIMER_BCR_R_TXREFS_MASK                                0x00000008
#define    DSP_SYS_TIMER_BCR_R_FNENB_SHIFT                                2
#define    DSP_SYS_TIMER_BCR_R_FNENB_MASK                                 0x00000004
#define    DSP_SYS_TIMER_BCR_R_STOPCNT_SHIFT                              1
#define    DSP_SYS_TIMER_BCR_R_STOPCNT_MASK                               0x00000002
#define    DSP_SYS_TIMER_BCR_R_STARCNT_SHIFT                              0
#define    DSP_SYS_TIMER_BCR_R_STARCNT_MASK                               0x00000001

#define DSP_SYS_TIMER_SFGIR_R_OFFSET                                      0x00000A10
#define DSP_SYS_TIMER_SFGIR_R_TYPE                                        UInt16
#define DSP_SYS_TIMER_SFGIR_R_RESERVED_MASK                               0x0000FF00
#define    DSP_SYS_TIMER_SFGIR_R_SFGIR_SHIFT                              0
#define    DSP_SYS_TIMER_SFGIR_R_SFGIR_MASK                               0x000000FF

#define DSP_SYS_TIMER_SFGFR_R_OFFSET                                      0x00000A12
#define DSP_SYS_TIMER_SFGFR_R_TYPE                                        UInt16
#define DSP_SYS_TIMER_SFGFR_R_RESERVED_MASK                               0x0000F000
#define    DSP_SYS_TIMER_SFGFR_R_SFGFR_SHIFT                              0
#define    DSP_SYS_TIMER_SFGFR_R_SFGFR_MASK                               0x00000FFF

#define DSP_SYS_TIMER_GMXR_R_OFFSET                                       0x00000A14
#define DSP_SYS_TIMER_GMXR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_GMXR_R_RESERVED_MASK                                0x00000000
#define    DSP_SYS_TIMER_GMXR_R_EN_SHIFT                                  15
#define    DSP_SYS_TIMER_GMXR_R_EN_MASK                                   0x00008000
#define    DSP_SYS_TIMER_GMXR_R_CONT_EN_SHIFT                             14
#define    DSP_SYS_TIMER_GMXR_R_CONT_EN_MASK                              0x00004000
#define    DSP_SYS_TIMER_GMXR_R_QEXP_SHIFT                                0
#define    DSP_SYS_TIMER_GMXR_R_QEXP_MASK                                 0x00003FFF

#define DSP_SYS_TIMER_GMCR_R_OFFSET                                       0x00000A16
#define DSP_SYS_TIMER_GMCR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_GMCR_R_RESERVED_MASK                                0x00000000
#define    DSP_SYS_TIMER_GMCR_R_POL_SHIFT                                 15
#define    DSP_SYS_TIMER_GMCR_R_POL_MASK                                  0x00008000
#define    DSP_SYS_TIMER_GMCR_R_FRMTM_SEL_SHIFT                           14
#define    DSP_SYS_TIMER_GMCR_R_FRMTM_SEL_MASK                            0x00004000
#define    DSP_SYS_TIMER_GMCR_R_GPEN11_SEL_SHIFT                          13
#define    DSP_SYS_TIMER_GMCR_R_GPEN11_SEL_MASK                           0x00002000
#define    DSP_SYS_TIMER_GMCR_R_QEXP_SHIFT                                0
#define    DSP_SYS_TIMER_GMCR_R_QEXP_MASK                                 0x00001FFF

#define DSP_SYS_TIMER_SCKRL_R_OFFSET                                      0x00000A18
#define DSP_SYS_TIMER_SCKRL_R_TYPE                                        UInt16
#define DSP_SYS_TIMER_SCKRL_R_RESERVED_MASK                               0x00000000
#define    DSP_SYS_TIMER_SCKRL_R_SCKRL_SHIFT                              0
#define    DSP_SYS_TIMER_SCKRL_R_SCKRL_MASK                               0x0000FFFF

#define DSP_SYS_TIMER_SCKRH_R_OFFSET                                      0x00000A1A
#define DSP_SYS_TIMER_SCKRH_R_TYPE                                        UInt16
#define DSP_SYS_TIMER_SCKRH_R_RESERVED_MASK                               0x0000FFFC
#define    DSP_SYS_TIMER_SCKRH_R_SCKRH_SHIFT                              0
#define    DSP_SYS_TIMER_SCKRH_R_SCKRH_MASK                               0x00000003

#define DSP_SYS_TIMER_SFCR_R_OFFSET                                       0x00000A1C
#define DSP_SYS_TIMER_SFCR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_SFCR_R_RESERVED_MASK                                0x0000FC00
#define    DSP_SYS_TIMER_SFCR_R_SFCR_SHIFT                                0
#define    DSP_SYS_TIMER_SFCR_R_SFCR_MASK                                 0x000003FF

#define DSP_SYS_TIMER_FCWR_R_OFFSET                                       0x00000A1E
#define DSP_SYS_TIMER_FCWR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_FCWR_R_RESERVED_MASK                                0x0000FC00
#define    DSP_SYS_TIMER_FCWR_R_FCWR_SHIFT                                0
#define    DSP_SYS_TIMER_FCWR_R_FCWR_MASK                                 0x000003FF

#define DSP_SYS_TIMER_QMXR_R_OFFSET                                       0x00000A20
#define DSP_SYS_TIMER_QMXR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_QMXR_R_RESERVED_MASK                                0x0000FFFF

#define DSP_SYS_TIMER_QMCR_R_OFFSET                                       0x00000A22
#define DSP_SYS_TIMER_QMCR_R_TYPE                                         UInt16
#define DSP_SYS_TIMER_QMCR_R_RESERVED_MASK                                0x0000DFFF
#define    DSP_SYS_TIMER_QMCR_R_DSLP_ENTRY_MODE_SHIFT                     13
#define    DSP_SYS_TIMER_QMCR_R_DSLP_ENTRY_MODE_MASK                      0x00002000

#define DSP_SYS_TIMER_GMXR2_R_OFFSET                                      0x00000A28
#define DSP_SYS_TIMER_GMXR2_R_TYPE                                        UInt16
#define DSP_SYS_TIMER_GMXR2_R_RESERVED_MASK                               0x00000000
#define    DSP_SYS_TIMER_GMXR2_R_EN_SHIFT                                 15
#define    DSP_SYS_TIMER_GMXR2_R_EN_MASK                                  0x00008000
#define    DSP_SYS_TIMER_GMXR2_R_CONT_EN_SHIFT                            14
#define    DSP_SYS_TIMER_GMXR2_R_CONT_EN_MASK                             0x00004000
#define    DSP_SYS_TIMER_GMXR2_R_QEXP_SHIFT                               0
#define    DSP_SYS_TIMER_GMXR2_R_QEXP_MASK                                0x00003FFF

#define DSP_SYS_TIMER_GMCR2_R_OFFSET                                      0x00000A2A
#define DSP_SYS_TIMER_GMCR2_R_TYPE                                        UInt16
#define DSP_SYS_TIMER_GMCR2_R_RESERVED_MASK                               0x00000000
#define    DSP_SYS_TIMER_GMCR2_R_POL_SHIFT                                15
#define    DSP_SYS_TIMER_GMCR2_R_POL_MASK                                 0x00008000
#define    DSP_SYS_TIMER_GMCR2_R_FRMTM_SEL_SHIFT                          14
#define    DSP_SYS_TIMER_GMCR2_R_FRMTM_SEL_MASK                           0x00004000
#define    DSP_SYS_TIMER_GMCR2_R_GPEN11_SEL_SHIFT                         13
#define    DSP_SYS_TIMER_GMCR2_R_GPEN11_SEL_MASK                          0x00002000
#define    DSP_SYS_TIMER_GMCR2_R_QEXP_SHIFT                               0
#define    DSP_SYS_TIMER_GMCR2_R_QEXP_MASK                                0x00001FFF

#define DSP_SYS_TIMER_QBCR_OFFSET                                         0x0000E500
#define DSP_SYS_TIMER_QBCR_TYPE                                           UInt16
#define DSP_SYS_TIMER_QBCR_RESERVED_MASK                                  0x0000C000
#define    DSP_SYS_TIMER_QBCR_QBCR_SHIFT                                  0
#define    DSP_SYS_TIMER_QBCR_QBCR_MASK                                   0x00003FFF

#define DSP_SYS_TIMER_CHAR_OFFSET                                         0x0000E501
#define DSP_SYS_TIMER_CHAR_TYPE                                           UInt16
#define DSP_SYS_TIMER_CHAR_RESERVED_MASK                                  0x0000C000
#define    DSP_SYS_TIMER_CHAR_CHAR_SHIFT                                  0
#define    DSP_SYS_TIMER_CHAR_CHAR_MASK                                   0x00003FFF

#define DSP_SYS_TIMER_CHBR_OFFSET                                         0x0000E502
#define DSP_SYS_TIMER_CHBR_TYPE                                           UInt16
#define DSP_SYS_TIMER_CHBR_RESERVED_MASK                                  0x0000C000
#define    DSP_SYS_TIMER_CHBR_CHBR_SHIFT                                  0
#define    DSP_SYS_TIMER_CHBR_CHBR_MASK                                   0x00003FFF

#define DSP_SYS_TIMER_FCR_OFFSET                                          0x0000E503
#define DSP_SYS_TIMER_FCR_TYPE                                            UInt16
#define DSP_SYS_TIMER_FCR_RESERVED_MASK                                   0x0000FC00
#define    DSP_SYS_TIMER_FCR_FCR_SHIFT                                    0
#define    DSP_SYS_TIMER_FCR_FCR_MASK                                     0x000003FF

#define DSP_SYS_TIMER_FCAR_OFFSET                                         0x0000E504
#define DSP_SYS_TIMER_FCAR_TYPE                                           UInt16
#define DSP_SYS_TIMER_FCAR_RESERVED_MASK                                  0x0000FC00
#define    DSP_SYS_TIMER_FCAR_FCAR_SHIFT                                  0
#define    DSP_SYS_TIMER_FCAR_FCAR_MASK                                   0x000003FF

#define DSP_SYS_TIMER_FCBR_OFFSET                                         0x0000E505
#define DSP_SYS_TIMER_FCBR_TYPE                                           UInt16
#define DSP_SYS_TIMER_FCBR_RESERVED_MASK                                  0x0000F000
#define    DSP_SYS_TIMER_FCBR_FCBR_LOAD_SHIFT                             11
#define    DSP_SYS_TIMER_FCBR_FCBR_LOAD_MASK                              0x00000800
#define    DSP_SYS_TIMER_FCBR_FCBR_LOADED_SHIFT                           10
#define    DSP_SYS_TIMER_FCBR_FCBR_LOADED_MASK                            0x00000400
#define    DSP_SYS_TIMER_FCBR_FCBR_SHIFT                                  0
#define    DSP_SYS_TIMER_FCBR_FCBR_MASK                                   0x000003FF

#define DSP_SYS_TIMER_BCR_OFFSET                                          0x0000E507
#define DSP_SYS_TIMER_BCR_TYPE                                            UInt16
#define DSP_SYS_TIMER_BCR_RESERVED_MASK                                   0x000080F0
#define    DSP_SYS_TIMER_BCR_BGEN_SHIFT                                   14
#define    DSP_SYS_TIMER_BCR_BGEN_MASK                                    0x00004000
#define    DSP_SYS_TIMER_BCR_BCLKSEL_SHIFT                                13
#define    DSP_SYS_TIMER_BCR_BCLKSEL_MASK                                 0x00002000
#define    DSP_SYS_TIMER_BCR_IQTST_SHIFT                                  12
#define    DSP_SYS_TIMER_BCR_IQTST_MASK                                   0x00001000
#define    DSP_SYS_TIMER_BCR_BRDTST_SHIFT                                 11
#define    DSP_SYS_TIMER_BCR_BRDTST_MASK                                  0x00000800
#define    DSP_SYS_TIMER_BCR_BRATST_SHIFT                                 10
#define    DSP_SYS_TIMER_BCR_BRATST_MASK                                  0x00000400
#define    DSP_SYS_TIMER_BCR_BTDTST_SHIFT                                 9
#define    DSP_SYS_TIMER_BCR_BTDTST_MASK                                  0x00000200
#define    DSP_SYS_TIMER_BCR_BTATST_SHIFT                                 8
#define    DSP_SYS_TIMER_BCR_BTATST_MASK                                  0x00000100
#define    DSP_SYS_TIMER_BCR_TXREFS_SHIFT                                 3
#define    DSP_SYS_TIMER_BCR_TXREFS_MASK                                  0x00000008
#define    DSP_SYS_TIMER_BCR_FNENB_SHIFT                                  2
#define    DSP_SYS_TIMER_BCR_FNENB_MASK                                   0x00000004
#define    DSP_SYS_TIMER_BCR_STOPCNT_SHIFT                                1
#define    DSP_SYS_TIMER_BCR_STOPCNT_MASK                                 0x00000002
#define    DSP_SYS_TIMER_BCR_STARCNT_SHIFT                                0
#define    DSP_SYS_TIMER_BCR_STARCNT_MASK                                 0x00000001

#define DSP_SYS_TIMER_SFGIR_OFFSET                                        0x0000E508
#define DSP_SYS_TIMER_SFGIR_TYPE                                          UInt16
#define DSP_SYS_TIMER_SFGIR_RESERVED_MASK                                 0x0000FF00
#define    DSP_SYS_TIMER_SFGIR_SFGIR_SHIFT                                0
#define    DSP_SYS_TIMER_SFGIR_SFGIR_MASK                                 0x000000FF

#define DSP_SYS_TIMER_SFGFR_OFFSET                                        0x0000E509
#define DSP_SYS_TIMER_SFGFR_TYPE                                          UInt16
#define DSP_SYS_TIMER_SFGFR_RESERVED_MASK                                 0x0000F000
#define    DSP_SYS_TIMER_SFGFR_SFGFR_SHIFT                                0
#define    DSP_SYS_TIMER_SFGFR_SFGFR_MASK                                 0x00000FFF

#define DSP_SYS_TIMER_GMXR_OFFSET                                         0x0000E50A
#define DSP_SYS_TIMER_GMXR_TYPE                                           UInt16
#define DSP_SYS_TIMER_GMXR_RESERVED_MASK                                  0x00000000
#define    DSP_SYS_TIMER_GMXR_EN_SHIFT                                    15
#define    DSP_SYS_TIMER_GMXR_EN_MASK                                     0x00008000
#define    DSP_SYS_TIMER_GMXR_CONT_EN_SHIFT                               14
#define    DSP_SYS_TIMER_GMXR_CONT_EN_MASK                                0x00004000
#define    DSP_SYS_TIMER_GMXR_QEXP_SHIFT                                  0
#define    DSP_SYS_TIMER_GMXR_QEXP_MASK                                   0x00003FFF

#define DSP_SYS_TIMER_GMCR_OFFSET                                         0x0000E50B
#define DSP_SYS_TIMER_GMCR_TYPE                                           UInt16
#define DSP_SYS_TIMER_GMCR_RESERVED_MASK                                  0x00000000
#define    DSP_SYS_TIMER_GMCR_POL_SHIFT                                   15
#define    DSP_SYS_TIMER_GMCR_POL_MASK                                    0x00008000
#define    DSP_SYS_TIMER_GMCR_FRMTM_SEL_SHIFT                             14
#define    DSP_SYS_TIMER_GMCR_FRMTM_SEL_MASK                              0x00004000
#define    DSP_SYS_TIMER_GMCR_GPEN11_SEL_SHIFT                            13
#define    DSP_SYS_TIMER_GMCR_GPEN11_SEL_MASK                             0x00002000
#define    DSP_SYS_TIMER_GMCR_QEXP_SHIFT                                  0
#define    DSP_SYS_TIMER_GMCR_QEXP_MASK                                   0x00001FFF

#define DSP_SYS_TIMER_SCKRL_OFFSET                                        0x0000E50C
#define DSP_SYS_TIMER_SCKRL_TYPE                                          UInt16
#define DSP_SYS_TIMER_SCKRL_RESERVED_MASK                                 0x00000000
#define    DSP_SYS_TIMER_SCKRL_SCKRL_SHIFT                                0
#define    DSP_SYS_TIMER_SCKRL_SCKRL_MASK                                 0x0000FFFF

#define DSP_SYS_TIMER_SCKRH_OFFSET                                        0x0000E50D
#define DSP_SYS_TIMER_SCKRH_TYPE                                          UInt16
#define DSP_SYS_TIMER_SCKRH_RESERVED_MASK                                 0x0000FFFC
#define    DSP_SYS_TIMER_SCKRH_SCKRH_SHIFT                                0
#define    DSP_SYS_TIMER_SCKRH_SCKRH_MASK                                 0x00000003

#define DSP_SYS_TIMER_SFCR_OFFSET                                         0x0000E50E
#define DSP_SYS_TIMER_SFCR_TYPE                                           UInt16
#define DSP_SYS_TIMER_SFCR_RESERVED_MASK                                  0x0000FC00
#define    DSP_SYS_TIMER_SFCR_SFCR_SHIFT                                  0
#define    DSP_SYS_TIMER_SFCR_SFCR_MASK                                   0x000003FF

#define DSP_SYS_TIMER_FCWR_OFFSET                                         0x0000E50F
#define DSP_SYS_TIMER_FCWR_TYPE                                           UInt16
#define DSP_SYS_TIMER_FCWR_RESERVED_MASK                                  0x0000FC00
#define    DSP_SYS_TIMER_FCWR_FCWR_SHIFT                                  0
#define    DSP_SYS_TIMER_FCWR_FCWR_MASK                                   0x000003FF

#define DSP_SYS_TIMER_QMXR_OFFSET                                         0x0000E510
#define DSP_SYS_TIMER_QMXR_TYPE                                           UInt16
#define DSP_SYS_TIMER_QMXR_RESERVED_MASK                                  0x0000FFFF

#define DSP_SYS_TIMER_QMCR_OFFSET                                         0x0000E511
#define DSP_SYS_TIMER_QMCR_TYPE                                           UInt16
#define DSP_SYS_TIMER_QMCR_RESERVED_MASK                                  0x0000DFFF
#define    DSP_SYS_TIMER_QMCR_DSLP_ENTRY_MODE_SHIFT                       13
#define    DSP_SYS_TIMER_QMCR_DSLP_ENTRY_MODE_MASK                        0x00002000

#define DSP_SYS_TIMER_GMXR2_OFFSET                                        0x0000E514
#define DSP_SYS_TIMER_GMXR2_TYPE                                          UInt16
#define DSP_SYS_TIMER_GMXR2_RESERVED_MASK                                 0x00000000
#define    DSP_SYS_TIMER_GMXR2_EN_SHIFT                                   15
#define    DSP_SYS_TIMER_GMXR2_EN_MASK                                    0x00008000
#define    DSP_SYS_TIMER_GMXR2_CONT_EN_SHIFT                              14
#define    DSP_SYS_TIMER_GMXR2_CONT_EN_MASK                               0x00004000
#define    DSP_SYS_TIMER_GMXR2_QEXP_SHIFT                                 0
#define    DSP_SYS_TIMER_GMXR2_QEXP_MASK                                  0x00003FFF

#define DSP_SYS_TIMER_GMCR2_OFFSET                                        0x0000E515
#define DSP_SYS_TIMER_GMCR2_TYPE                                          UInt16
#define DSP_SYS_TIMER_GMCR2_RESERVED_MASK                                 0x00000000
#define    DSP_SYS_TIMER_GMCR2_POL_SHIFT                                  15
#define    DSP_SYS_TIMER_GMCR2_POL_MASK                                   0x00008000
#define    DSP_SYS_TIMER_GMCR2_FRMTM_SEL_SHIFT                            14
#define    DSP_SYS_TIMER_GMCR2_FRMTM_SEL_MASK                             0x00004000
#define    DSP_SYS_TIMER_GMCR2_GPEN10_SEL_SHIFT                           13
#define    DSP_SYS_TIMER_GMCR2_GPEN10_SEL_MASK                            0x00002000
#define    DSP_SYS_TIMER_GMCR2_QEXP_SHIFT                                 0
#define    DSP_SYS_TIMER_GMCR2_QEXP_MASK                                  0x00001FFF

#endif /* __BRCM_RDB_DSP_SYS_TIMER_H__ */



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

#ifndef __BRCM_RDB_DSP_TX_CONTROL_H__
#define __BRCM_RDB_DSP_TX_CONTROL_H__

#define DSP_TX_CONTROL_TRCR_OFFSET                                        0x0000E580
#define DSP_TX_CONTROL_TRCR_TYPE                                          UInt16
#define DSP_TX_CONTROL_TRCR_RESERVED_MASK                                 0x00000000
#define    DSP_TX_CONTROL_TRCR_RXSTAT_SHIFT                               15
#define    DSP_TX_CONTROL_TRCR_RXSTAT_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRCR_RCON_SHIFT                                 13
#define    DSP_TX_CONTROL_TRCR_RCON_MASK                                  0x00006000
#define    DSP_TX_CONTROL_TRCR_STOPRX_SHIFT                               12
#define    DSP_TX_CONTROL_TRCR_STOPRX_MASK                                0x00001000
#define    DSP_TX_CONTROL_TRCR_STARTRX_SHIFT                              11
#define    DSP_TX_CONTROL_TRCR_STARTRX_MASK                               0x00000800
#define    DSP_TX_CONTROL_TRCR_RBC_SHIFT                                  8
#define    DSP_TX_CONTROL_TRCR_RBC_MASK                                   0x00000700
#define    DSP_TX_CONTROL_TRCR_TXSTAT_SHIFT                               7
#define    DSP_TX_CONTROL_TRCR_TXSTAT_MASK                                0x00000080
#define    DSP_TX_CONTROL_TRCR_TCON_SHIFT                                 5
#define    DSP_TX_CONTROL_TRCR_TCON_MASK                                  0x00000060
#define    DSP_TX_CONTROL_TRCR_STOPTX_SHIFT                               4
#define    DSP_TX_CONTROL_TRCR_STOPTX_MASK                                0x00000010
#define    DSP_TX_CONTROL_TRCR_STARTTX_SHIFT                              3
#define    DSP_TX_CONTROL_TRCR_STARTTX_MASK                               0x00000008
#define    DSP_TX_CONTROL_TRCR_TBC_SHIFT                                  0
#define    DSP_TX_CONTROL_TRCR_TBC_MASK                                   0x00000007

#define DSP_TX_CONTROL_TCOR_OFFSET                                        0x0000E581
#define DSP_TX_CONTROL_TCOR_TYPE                                          UInt16
#define DSP_TX_CONTROL_TCOR_RESERVED_MASK                                 0x0000C0C0
#define    DSP_TX_CONTROL_TCOR_TCQ_SHIFT                                  8
#define    DSP_TX_CONTROL_TCOR_TCQ_MASK                                   0x00003F00
#define    DSP_TX_CONTROL_TCOR_TCI_SHIFT                                  0
#define    DSP_TX_CONTROL_TCOR_TCI_MASK                                   0x0000003F

#define DSP_TX_CONTROL_TCDR_OFFSET                                        0x0000E582
#define DSP_TX_CONTROL_TCDR_TYPE                                          UInt16
#define DSP_TX_CONTROL_TCDR_RESERVED_MASK                                 0x00008800
#define    DSP_TX_CONTROL_TCDR_TXCT_SHIFT                                 12
#define    DSP_TX_CONTROL_TCDR_TXCT_MASK                                  0x00007000
#define    DSP_TX_CONTROL_TCDR_TXIS_SHIFT                                 10
#define    DSP_TX_CONTROL_TCDR_TXIS_MASK                                  0x00000400
#define    DSP_TX_CONTROL_TCDR_DEL_SHIFT                                  0
#define    DSP_TX_CONTROL_TCDR_DEL_MASK                                   0x000003FF

#define DSP_TX_CONTROL_RFTCOR_OFFSET                                      0x0000E583
#define DSP_TX_CONTROL_RFTCOR_TYPE                                        UInt16
#define DSP_TX_CONTROL_RFTCOR_RESERVED_MASK                               0x0000C0C0
#define    DSP_TX_CONTROL_RFTCOR_RTCQ_SHIFT                               8
#define    DSP_TX_CONTROL_RFTCOR_RTCQ_MASK                                0x00003F00
#define    DSP_TX_CONTROL_RFTCOR_RTCI_SHIFT                               0
#define    DSP_TX_CONTROL_RFTCOR_RTCI_MASK                                0x0000003F

#define DSP_TX_CONTROL_TSDR_OFFSET                                        0x0000E584
#define DSP_TX_CONTROL_TSDR_TYPE                                          UInt16
#define DSP_TX_CONTROL_TSDR_RESERVED_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TSDR_MSDEL_SHIFT                                12
#define    DSP_TX_CONTROL_TSDR_MSDEL_MASK                                 0x00007000
#define    DSP_TX_CONTROL_TSDR_QIVPOL_SHIFT                               11
#define    DSP_TX_CONTROL_TSDR_QIVPOL_MASK                                0x00000800
#define    DSP_TX_CONTROL_TSDR_QIVEN_SHIFT                                10
#define    DSP_TX_CONTROL_TSDR_QIVEN_MASK                                 0x00000400
#define    DSP_TX_CONTROL_TSDR_DEL_SHIFT                                  0
#define    DSP_TX_CONTROL_TSDR_DEL_MASK                                   0x000003FF

#define DSP_TX_CONTROL_TDRFR_OFFSET                                       0x0000E585
#define DSP_TX_CONTROL_TDRFR_TYPE                                         UInt16
#define DSP_TX_CONTROL_TDRFR_RESERVED_MASK                                0x0000FFFE
#define    DSP_TX_CONTROL_TDRFR_MULTI_ENABLE_SHIFT                        0
#define    DSP_TX_CONTROL_TDRFR_MULTI_ENABLE_MASK                         0x00000001

#define DSP_TX_CONTROL_T86R_OFFSET                                        0x0000E586
#define DSP_TX_CONTROL_T86R_TYPE                                          UInt16
#define DSP_TX_CONTROL_T86R_RESERVED_MASK                                 0x0000FFFF

#define DSP_TX_CONTROL_MFCRG_OFFSET                                       0x0000E587
#define DSP_TX_CONTROL_MFCRG_TYPE                                         UInt16
#define DSP_TX_CONTROL_MFCRG_RESERVED_MASK                                0x0000F000
#define    DSP_TX_CONTROL_MFCRG_END_SHIFT                                 0
#define    DSP_TX_CONTROL_MFCRG_END_MASK                                  0x00000FFF

#define DSP_TX_CONTROL_MFCRP_OFFSET                                       0x0000E588
#define DSP_TX_CONTROL_MFCRP_TYPE                                         UInt16
#define DSP_TX_CONTROL_MFCRP_RESERVED_MASK                                0x0000F000
#define    DSP_TX_CONTROL_MFCRP_END_SHIFT                                 0
#define    DSP_TX_CONTROL_MFCRP_END_MASK                                  0x00000FFF

#define DSP_TX_CONTROL_TRDR_0_OFFSET                                      0x0000E58A
#define DSP_TX_CONTROL_TRDR_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_0_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_0_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_0_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_0_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_0_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_TRDR_1_OFFSET                                      0x0000E58B
#define DSP_TX_CONTROL_TRDR_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_1_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_1_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_1_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_1_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_1_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_TRDR_2_OFFSET                                      0x0000E58C
#define DSP_TX_CONTROL_TRDR_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_2_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_2_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_2_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_2_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_2_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_TRDR_3_OFFSET                                      0x0000E58D
#define DSP_TX_CONTROL_TRDR_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_3_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_3_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_3_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_3_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_3_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_TRDR_4_OFFSET                                      0x0000E58E
#define DSP_TX_CONTROL_TRDR_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_4_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_4_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_4_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_4_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_4_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_TRDR_5_OFFSET                                      0x0000E58F
#define DSP_TX_CONTROL_TRDR_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_5_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_5_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_5_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_5_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_5_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_TRDR_6_OFFSET                                      0x0000E590
#define DSP_TX_CONTROL_TRDR_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRDR_6_RESERVED_MASK                               0x00007000
#define    DSP_TX_CONTROL_TRDR_6_END_SHIFT                                15
#define    DSP_TX_CONTROL_TRDR_6_END_MASK                                 0x00008000
#define    DSP_TX_CONTROL_TRDR_6_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TRDR_6_DEL_MASK                                 0x00000FFF

#define DSP_TX_CONTROL_MSWTR_0_OFFSET                                     0x0000E591
#define DSP_TX_CONTROL_MSWTR_0_TYPE                                       UInt16
#define DSP_TX_CONTROL_MSWTR_0_RESERVED_MASK                              0x0000F000
#define    DSP_TX_CONTROL_MSWTR_0_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_MSWTR_0_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_MSWTR_1_OFFSET                                     0x0000E592
#define DSP_TX_CONTROL_MSWTR_1_TYPE                                       UInt16
#define DSP_TX_CONTROL_MSWTR_1_RESERVED_MASK                              0x0000F000
#define    DSP_TX_CONTROL_MSWTR_1_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_MSWTR_1_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_MSWTR_2_OFFSET                                     0x0000E593
#define DSP_TX_CONTROL_MSWTR_2_TYPE                                       UInt16
#define DSP_TX_CONTROL_MSWTR_2_RESERVED_MASK                              0x0000F000
#define    DSP_TX_CONTROL_MSWTR_2_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_MSWTR_2_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_MSWTR_3_OFFSET                                     0x0000E594
#define DSP_TX_CONTROL_MSWTR_3_TYPE                                       UInt16
#define DSP_TX_CONTROL_MSWTR_3_RESERVED_MASK                              0x0000F000
#define    DSP_TX_CONTROL_MSWTR_3_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_MSWTR_3_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_APCSWG_0_OFFSET                                    0x0000E595
#define DSP_TX_CONTROL_APCSWG_0_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWG_0_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWG_0_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWG_0_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWG_1_OFFSET                                    0x0000E596
#define DSP_TX_CONTROL_APCSWG_1_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWG_1_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWG_1_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWG_1_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWG_2_OFFSET                                    0x0000E597
#define DSP_TX_CONTROL_APCSWG_2_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWG_2_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWG_2_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWG_2_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWG_3_OFFSET                                    0x0000E598
#define DSP_TX_CONTROL_APCSWG_3_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWG_3_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWG_3_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWG_3_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWP_0_OFFSET                                    0x0000E599
#define DSP_TX_CONTROL_APCSWP_0_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWP_0_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWP_0_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWP_0_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWP_1_OFFSET                                    0x0000E59A
#define DSP_TX_CONTROL_APCSWP_1_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWP_1_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWP_1_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWP_1_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWP_2_OFFSET                                    0x0000E59B
#define DSP_TX_CONTROL_APCSWP_2_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWP_2_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWP_2_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWP_2_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWP_3_OFFSET                                    0x0000E59C
#define DSP_TX_CONTROL_APCSWP_3_TYPE                                      UInt16
#define DSP_TX_CONTROL_APCSWP_3_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_APCSWP_3_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_APCSWP_3_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCGDFR_OFFSET                                     0x0000E59D
#define DSP_TX_CONTROL_APCGDFR_TYPE                                       UInt16
#define DSP_TX_CONTROL_APCGDFR_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_APCGDFR_APCGDFR_SHIFT                           0
#define    DSP_TX_CONTROL_APCGDFR_APCGDFR_MASK                            0x000003FF

#define DSP_TX_CONTROL_APCPDFR_OFFSET                                     0x0000E59E
#define DSP_TX_CONTROL_APCPDFR_TYPE                                       UInt16
#define DSP_TX_CONTROL_APCPDFR_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_APCPDFR_APCPDFR_SHIFT                           0
#define    DSP_TX_CONTROL_APCPDFR_APCPDFR_MASK                            0x000003FF

#define DSP_TX_CONTROL_MTXCR_OFFSET                                       0x0000E59F
#define DSP_TX_CONTROL_MTXCR_TYPE                                         UInt16
#define DSP_TX_CONTROL_MTXCR_RESERVED_MASK                                0x00007F00
#define    DSP_TX_CONTROL_MTXCR_MIX_SHIFT                                 15
#define    DSP_TX_CONTROL_MTXCR_MIX_MASK                                  0x00008000
#define    DSP_TX_CONTROL_MTXCR_SLOT3_SHIFT                               6
#define    DSP_TX_CONTROL_MTXCR_SLOT3_MASK                                0x000000C0
#define       DSP_TX_CONTROL_MTXCR_SLOT3_CMD_G_MODE                       0x00000000
#define       DSP_TX_CONTROL_MTXCR_SLOT3_CMD_P_MODE                       0x00000002
#define    DSP_TX_CONTROL_MTXCR_SLOT2_SHIFT                               4
#define    DSP_TX_CONTROL_MTXCR_SLOT2_MASK                                0x00000030
#define       DSP_TX_CONTROL_MTXCR_SLOT2_CMD_G_MODE                       0x00000000
#define       DSP_TX_CONTROL_MTXCR_SLOT2_CMD_P_MODE                       0x00000002
#define    DSP_TX_CONTROL_MTXCR_SLOT1_SHIFT                               2
#define    DSP_TX_CONTROL_MTXCR_SLOT1_MASK                                0x0000000C
#define       DSP_TX_CONTROL_MTXCR_SLOT1_CMD_G_MODE                       0x00000000
#define       DSP_TX_CONTROL_MTXCR_SLOT1_CMD_P_MODE                       0x00000002
#define    DSP_TX_CONTROL_MTXCR_SLOT0_SHIFT                               0
#define    DSP_TX_CONTROL_MTXCR_SLOT0_MASK                                0x00000003
#define       DSP_TX_CONTROL_MTXCR_SLOT0_CMD_G_MODE                       0x00000000
#define       DSP_TX_CONTROL_MTXCR_SLOT0_CMD_P_MODE                       0x00000002

#define DSP_TX_CONTROL_TXCR_OFFSET                                        0x0000E5B2
#define DSP_TX_CONTROL_TXCR_TYPE                                          UInt16
#define DSP_TX_CONTROL_TXCR_RESERVED_MASK                                 0x0000FC00
#define    DSP_TX_CONTROL_TXCR_RAMP_DIGRF2_SHIFT                          9
#define    DSP_TX_CONTROL_TXCR_RAMP_DIGRF2_MASK                           0x00000200
#define    DSP_TX_CONTROL_TXCR_TXSTAT_SHIFT                               8
#define    DSP_TX_CONTROL_TXCR_TXSTAT_MASK                                0x00000100
#define    DSP_TX_CONTROL_TXCR_TCON_SHIFT                                 5
#define    DSP_TX_CONTROL_TXCR_TCON_MASK                                  0x000000E0
#define    DSP_TX_CONTROL_TXCR_STOPTX_SHIFT                               4
#define    DSP_TX_CONTROL_TXCR_STOPTX_MASK                                0x00000010
#define    DSP_TX_CONTROL_TXCR_STARTTX_SHIFT                              3
#define    DSP_TX_CONTROL_TXCR_STARTTX_MASK                               0x00000008
#define    DSP_TX_CONTROL_TXCR_TBC_SHIFT                                  0
#define    DSP_TX_CONTROL_TXCR_TBC_MASK                                   0x00000007

#define DSP_TX_CONTROL_MODADR_OFFSET                                      0x0000E700
#define DSP_TX_CONTROL_MODADR_TYPE                                        UInt16
#define DSP_TX_CONTROL_MODADR_RESERVED_MASK                               0x0000FF80
#define    DSP_TX_CONTROL_MODADR_BUFFER_SHIFT                             5
#define    DSP_TX_CONTROL_MODADR_BUFFER_MASK                              0x00000060
#define    DSP_TX_CONTROL_MODADR_ADDRESS_SHIFT                            0
#define    DSP_TX_CONTROL_MODADR_ADDRESS_MASK                             0x0000001F

#define DSP_TX_CONTROL_MODDAT_OFFSET                                      0x0000E701
#define DSP_TX_CONTROL_MODDAT_TYPE                                        UInt16
#define DSP_TX_CONTROL_MODDAT_RESERVED_MASK                               0x00000000
#define    DSP_TX_CONTROL_MODDAT_GSMMODE0_MODDAT_SHIFT                    0
#define    DSP_TX_CONTROL_MODDAT_GSMMODE0_MODDAT_MASK                     0x0000FFFF
#define    DSP_TX_CONTROL_MODDAT_PSKMODE0_MODDAT_SHIFT                    0
#define    DSP_TX_CONTROL_MODDAT_PSKMODE0_MODDAT_MASK                     0x00007FFF

#define DSP_TX_CONTROL_MODDAT2_OFFSET                                     0x0000E702
#define DSP_TX_CONTROL_MODDAT2_TYPE                                       UInt32
#define DSP_TX_CONTROL_MODDAT2_RESERVED_MASK                              0x00000000
#define    DSP_TX_CONTROL_MODDAT2_GSMMODE0_MODDAT_SHIFT                   0
#define    DSP_TX_CONTROL_MODDAT2_GSMMODE0_MODDAT_MASK                    0xFFFFFFFF
#define    DSP_TX_CONTROL_MODDAT2_PSKMODE0_MODDAT_U_SHIFT                 16
#define    DSP_TX_CONTROL_MODDAT2_PSKMODE0_MODDAT_U_MASK                  0x7FFF0000
#define    DSP_TX_CONTROL_MODDAT2_PSKMODE0_MODDAT_L_SHIFT                 0
#define    DSP_TX_CONTROL_MODDAT2_PSKMODE0_MODDAT_L_MASK                  0x00007FFF

#define DSP_TX_CONTROL_TPR0_0_OFFSET                                      0x0000E730
#define DSP_TX_CONTROL_TPR0_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_1_OFFSET                                      0x0000E731
#define DSP_TX_CONTROL_TPR0_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_2_OFFSET                                      0x0000E732
#define DSP_TX_CONTROL_TPR0_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_3_OFFSET                                      0x0000E733
#define DSP_TX_CONTROL_TPR0_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_4_OFFSET                                      0x0000E734
#define DSP_TX_CONTROL_TPR0_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_5_OFFSET                                      0x0000E735
#define DSP_TX_CONTROL_TPR0_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_6_OFFSET                                      0x0000E736
#define DSP_TX_CONTROL_TPR0_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_7_OFFSET                                      0x0000E737
#define DSP_TX_CONTROL_TPR0_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_8_OFFSET                                      0x0000E738
#define DSP_TX_CONTROL_TPR0_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_9_OFFSET                                      0x0000E739
#define DSP_TX_CONTROL_TPR0_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR0_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR0_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR0_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR0_10_OFFSET                                     0x0000E73A
#define DSP_TX_CONTROL_TPR0_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_11_OFFSET                                     0x0000E73B
#define DSP_TX_CONTROL_TPR0_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_12_OFFSET                                     0x0000E73C
#define DSP_TX_CONTROL_TPR0_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_13_OFFSET                                     0x0000E73D
#define DSP_TX_CONTROL_TPR0_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_14_OFFSET                                     0x0000E73E
#define DSP_TX_CONTROL_TPR0_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_15_OFFSET                                     0x0000E73F
#define DSP_TX_CONTROL_TPR0_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_0_OFFSET                                      0x0000E740
#define DSP_TX_CONTROL_TPR1_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_1_OFFSET                                      0x0000E741
#define DSP_TX_CONTROL_TPR1_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_2_OFFSET                                      0x0000E742
#define DSP_TX_CONTROL_TPR1_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_3_OFFSET                                      0x0000E743
#define DSP_TX_CONTROL_TPR1_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_4_OFFSET                                      0x0000E744
#define DSP_TX_CONTROL_TPR1_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_5_OFFSET                                      0x0000E745
#define DSP_TX_CONTROL_TPR1_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_6_OFFSET                                      0x0000E746
#define DSP_TX_CONTROL_TPR1_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_7_OFFSET                                      0x0000E747
#define DSP_TX_CONTROL_TPR1_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_8_OFFSET                                      0x0000E748
#define DSP_TX_CONTROL_TPR1_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_9_OFFSET                                      0x0000E749
#define DSP_TX_CONTROL_TPR1_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR1_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR1_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR1_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR1_10_OFFSET                                     0x0000E74A
#define DSP_TX_CONTROL_TPR1_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_11_OFFSET                                     0x0000E74B
#define DSP_TX_CONTROL_TPR1_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_12_OFFSET                                     0x0000E74C
#define DSP_TX_CONTROL_TPR1_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_13_OFFSET                                     0x0000E74D
#define DSP_TX_CONTROL_TPR1_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_14_OFFSET                                     0x0000E74E
#define DSP_TX_CONTROL_TPR1_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_15_OFFSET                                     0x0000E74F
#define DSP_TX_CONTROL_TPR1_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_0_OFFSET                                      0x0000E750
#define DSP_TX_CONTROL_TPR2_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_1_OFFSET                                      0x0000E751
#define DSP_TX_CONTROL_TPR2_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_2_OFFSET                                      0x0000E752
#define DSP_TX_CONTROL_TPR2_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_3_OFFSET                                      0x0000E753
#define DSP_TX_CONTROL_TPR2_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_4_OFFSET                                      0x0000E754
#define DSP_TX_CONTROL_TPR2_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_5_OFFSET                                      0x0000E755
#define DSP_TX_CONTROL_TPR2_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_6_OFFSET                                      0x0000E756
#define DSP_TX_CONTROL_TPR2_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_7_OFFSET                                      0x0000E757
#define DSP_TX_CONTROL_TPR2_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_8_OFFSET                                      0x0000E758
#define DSP_TX_CONTROL_TPR2_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_9_OFFSET                                      0x0000E759
#define DSP_TX_CONTROL_TPR2_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR2_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR2_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR2_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR2_10_OFFSET                                     0x0000E75A
#define DSP_TX_CONTROL_TPR2_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_11_OFFSET                                     0x0000E75B
#define DSP_TX_CONTROL_TPR2_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_12_OFFSET                                     0x0000E75C
#define DSP_TX_CONTROL_TPR2_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_13_OFFSET                                     0x0000E75D
#define DSP_TX_CONTROL_TPR2_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_14_OFFSET                                     0x0000E75E
#define DSP_TX_CONTROL_TPR2_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_15_OFFSET                                     0x0000E75F
#define DSP_TX_CONTROL_TPR2_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_0_OFFSET                                      0x0000E760
#define DSP_TX_CONTROL_TPR3_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_1_OFFSET                                      0x0000E761
#define DSP_TX_CONTROL_TPR3_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_2_OFFSET                                      0x0000E762
#define DSP_TX_CONTROL_TPR3_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_3_OFFSET                                      0x0000E763
#define DSP_TX_CONTROL_TPR3_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_4_OFFSET                                      0x0000E764
#define DSP_TX_CONTROL_TPR3_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_5_OFFSET                                      0x0000E765
#define DSP_TX_CONTROL_TPR3_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_6_OFFSET                                      0x0000E766
#define DSP_TX_CONTROL_TPR3_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_7_OFFSET                                      0x0000E767
#define DSP_TX_CONTROL_TPR3_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_8_OFFSET                                      0x0000E768
#define DSP_TX_CONTROL_TPR3_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_9_OFFSET                                      0x0000E769
#define DSP_TX_CONTROL_TPR3_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR3_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR3_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR3_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR3_10_OFFSET                                     0x0000E76A
#define DSP_TX_CONTROL_TPR3_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_11_OFFSET                                     0x0000E76B
#define DSP_TX_CONTROL_TPR3_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_12_OFFSET                                     0x0000E76C
#define DSP_TX_CONTROL_TPR3_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_13_OFFSET                                     0x0000E76D
#define DSP_TX_CONTROL_TPR3_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_14_OFFSET                                     0x0000E76E
#define DSP_TX_CONTROL_TPR3_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_15_OFFSET                                     0x0000E76F
#define DSP_TX_CONTROL_TPR3_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_0_OFFSET                                      0x0000E770
#define DSP_TX_CONTROL_TPR4_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_1_OFFSET                                      0x0000E771
#define DSP_TX_CONTROL_TPR4_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_2_OFFSET                                      0x0000E772
#define DSP_TX_CONTROL_TPR4_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_3_OFFSET                                      0x0000E773
#define DSP_TX_CONTROL_TPR4_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_4_OFFSET                                      0x0000E774
#define DSP_TX_CONTROL_TPR4_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_5_OFFSET                                      0x0000E775
#define DSP_TX_CONTROL_TPR4_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_6_OFFSET                                      0x0000E776
#define DSP_TX_CONTROL_TPR4_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_7_OFFSET                                      0x0000E777
#define DSP_TX_CONTROL_TPR4_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_8_OFFSET                                      0x0000E778
#define DSP_TX_CONTROL_TPR4_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_9_OFFSET                                      0x0000E779
#define DSP_TX_CONTROL_TPR4_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR4_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR4_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR4_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR4_10_OFFSET                                     0x0000E77A
#define DSP_TX_CONTROL_TPR4_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_11_OFFSET                                     0x0000E77B
#define DSP_TX_CONTROL_TPR4_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_12_OFFSET                                     0x0000E77C
#define DSP_TX_CONTROL_TPR4_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_13_OFFSET                                     0x0000E77D
#define DSP_TX_CONTROL_TPR4_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_14_OFFSET                                     0x0000E77E
#define DSP_TX_CONTROL_TPR4_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_15_OFFSET                                     0x0000E77F
#define DSP_TX_CONTROL_TPR4_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_0_OFFSET                                      0x0000E780
#define DSP_TX_CONTROL_TPR5_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_1_OFFSET                                      0x0000E781
#define DSP_TX_CONTROL_TPR5_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_2_OFFSET                                      0x0000E782
#define DSP_TX_CONTROL_TPR5_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_3_OFFSET                                      0x0000E783
#define DSP_TX_CONTROL_TPR5_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_4_OFFSET                                      0x0000E784
#define DSP_TX_CONTROL_TPR5_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_5_OFFSET                                      0x0000E785
#define DSP_TX_CONTROL_TPR5_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_6_OFFSET                                      0x0000E786
#define DSP_TX_CONTROL_TPR5_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_7_OFFSET                                      0x0000E787
#define DSP_TX_CONTROL_TPR5_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_8_OFFSET                                      0x0000E788
#define DSP_TX_CONTROL_TPR5_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_9_OFFSET                                      0x0000E789
#define DSP_TX_CONTROL_TPR5_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR5_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR5_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR5_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR5_10_OFFSET                                     0x0000E78A
#define DSP_TX_CONTROL_TPR5_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_11_OFFSET                                     0x0000E78B
#define DSP_TX_CONTROL_TPR5_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_12_OFFSET                                     0x0000E78C
#define DSP_TX_CONTROL_TPR5_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_13_OFFSET                                     0x0000E78D
#define DSP_TX_CONTROL_TPR5_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_14_OFFSET                                     0x0000E78E
#define DSP_TX_CONTROL_TPR5_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_15_OFFSET                                     0x0000E78F
#define DSP_TX_CONTROL_TPR5_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_0_OFFSET                                      0x0000E790
#define DSP_TX_CONTROL_TPR6_0_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_0_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_0_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_0_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_1_OFFSET                                      0x0000E791
#define DSP_TX_CONTROL_TPR6_1_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_1_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_1_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_1_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_2_OFFSET                                      0x0000E792
#define DSP_TX_CONTROL_TPR6_2_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_2_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_2_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_2_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_3_OFFSET                                      0x0000E793
#define DSP_TX_CONTROL_TPR6_3_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_3_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_3_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_3_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_4_OFFSET                                      0x0000E794
#define DSP_TX_CONTROL_TPR6_4_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_4_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_4_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_4_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_5_OFFSET                                      0x0000E795
#define DSP_TX_CONTROL_TPR6_5_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_5_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_5_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_5_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_6_OFFSET                                      0x0000E796
#define DSP_TX_CONTROL_TPR6_6_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_6_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_6_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_6_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_7_OFFSET                                      0x0000E797
#define DSP_TX_CONTROL_TPR6_7_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_7_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_7_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_7_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_8_OFFSET                                      0x0000E798
#define DSP_TX_CONTROL_TPR6_8_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_8_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_8_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_8_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_9_OFFSET                                      0x0000E799
#define DSP_TX_CONTROL_TPR6_9_TYPE                                        UInt16
#define DSP_TX_CONTROL_TPR6_9_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TPR6_9_TPR_SHIFT                                0
#define    DSP_TX_CONTROL_TPR6_9_TPR_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TPR6_10_OFFSET                                     0x0000E79A
#define DSP_TX_CONTROL_TPR6_10_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_10_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_10_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_10_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_11_OFFSET                                     0x0000E79B
#define DSP_TX_CONTROL_TPR6_11_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_11_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_11_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_11_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_12_OFFSET                                     0x0000E79C
#define DSP_TX_CONTROL_TPR6_12_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_12_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_12_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_12_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_13_OFFSET                                     0x0000E79D
#define DSP_TX_CONTROL_TPR6_13_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_13_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_13_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_13_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_14_OFFSET                                     0x0000E79E
#define DSP_TX_CONTROL_TPR6_14_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_14_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_14_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_14_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_15_OFFSET                                     0x0000E79F
#define DSP_TX_CONTROL_TPR6_15_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_15_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_15_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_15_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TRCR_R_OFFSET                                      0x00000B00
#define DSP_TX_CONTROL_TRCR_R_TYPE                                        UInt16
#define DSP_TX_CONTROL_TRCR_R_RESERVED_MASK                               0x00000000
#define    DSP_TX_CONTROL_TRCR_R_RXSTAT_SHIFT                             15
#define    DSP_TX_CONTROL_TRCR_R_RXSTAT_MASK                              0x00008000
#define    DSP_TX_CONTROL_TRCR_R_RCON_SHIFT                               13
#define    DSP_TX_CONTROL_TRCR_R_RCON_MASK                                0x00006000
#define    DSP_TX_CONTROL_TRCR_R_STOPRX_SHIFT                             12
#define    DSP_TX_CONTROL_TRCR_R_STOPRX_MASK                              0x00001000
#define    DSP_TX_CONTROL_TRCR_R_STARTRX_SHIFT                            11
#define    DSP_TX_CONTROL_TRCR_R_STARTRX_MASK                             0x00000800
#define    DSP_TX_CONTROL_TRCR_R_RBC_SHIFT                                8
#define    DSP_TX_CONTROL_TRCR_R_RBC_MASK                                 0x00000700
#define    DSP_TX_CONTROL_TRCR_R_TXSTAT_SHIFT                             7
#define    DSP_TX_CONTROL_TRCR_R_TXSTAT_MASK                              0x00000080
#define    DSP_TX_CONTROL_TRCR_R_TCON_SHIFT                               5
#define    DSP_TX_CONTROL_TRCR_R_TCON_MASK                                0x00000060
#define    DSP_TX_CONTROL_TRCR_R_STOPTX_SHIFT                             4
#define    DSP_TX_CONTROL_TRCR_R_STOPTX_MASK                              0x00000010
#define    DSP_TX_CONTROL_TRCR_R_STARTTX_SHIFT                            3
#define    DSP_TX_CONTROL_TRCR_R_STARTTX_MASK                             0x00000008
#define    DSP_TX_CONTROL_TRCR_R_TBC_SHIFT                                0
#define    DSP_TX_CONTROL_TRCR_R_TBC_MASK                                 0x00000007

#define DSP_TX_CONTROL_TCOR_R_OFFSET                                      0x00000B02
#define DSP_TX_CONTROL_TCOR_R_TYPE                                        UInt16
#define DSP_TX_CONTROL_TCOR_R_RESERVED_MASK                               0x0000C0C0
#define    DSP_TX_CONTROL_TCOR_R_TCQ_SHIFT                                8
#define    DSP_TX_CONTROL_TCOR_R_TCQ_MASK                                 0x00003F00
#define    DSP_TX_CONTROL_TCOR_R_TCI_SHIFT                                0
#define    DSP_TX_CONTROL_TCOR_R_TCI_MASK                                 0x0000003F

#define DSP_TX_CONTROL_TCDR_R_OFFSET                                      0x00000B04
#define DSP_TX_CONTROL_TCDR_R_TYPE                                        UInt16
#define DSP_TX_CONTROL_TCDR_R_RESERVED_MASK                               0x00008800
#define    DSP_TX_CONTROL_TCDR_R_TXCT_SHIFT                               12
#define    DSP_TX_CONTROL_TCDR_R_TXCT_MASK                                0x00007000
#define    DSP_TX_CONTROL_TCDR_R_TXIS_SHIFT                               10
#define    DSP_TX_CONTROL_TCDR_R_TXIS_MASK                                0x00000400
#define    DSP_TX_CONTROL_TCDR_R_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TCDR_R_DEL_MASK                                 0x000003FF

#define DSP_TX_CONTROL_RFTCOR_R_OFFSET                                    0x00000B06
#define DSP_TX_CONTROL_RFTCOR_R_TYPE                                      UInt16
#define DSP_TX_CONTROL_RFTCOR_R_RESERVED_MASK                             0x0000C0C0
#define    DSP_TX_CONTROL_RFTCOR_R_RTCQ_SHIFT                             8
#define    DSP_TX_CONTROL_RFTCOR_R_RTCQ_MASK                              0x00003F00
#define    DSP_TX_CONTROL_RFTCOR_R_RTCI_SHIFT                             0
#define    DSP_TX_CONTROL_RFTCOR_R_RTCI_MASK                              0x0000003F

#define DSP_TX_CONTROL_TSDR_R_OFFSET                                      0x00000B08
#define DSP_TX_CONTROL_TSDR_R_TYPE                                        UInt16
#define DSP_TX_CONTROL_TSDR_R_RESERVED_MASK                               0x00008000
#define    DSP_TX_CONTROL_TSDR_R_MSDEL_SHIFT                              12
#define    DSP_TX_CONTROL_TSDR_R_MSDEL_MASK                               0x00007000
#define    DSP_TX_CONTROL_TSDR_R_QIVPOL_SHIFT                             11
#define    DSP_TX_CONTROL_TSDR_R_QIVPOL_MASK                              0x00000800
#define    DSP_TX_CONTROL_TSDR_R_QIVEN_SHIFT                              10
#define    DSP_TX_CONTROL_TSDR_R_QIVEN_MASK                               0x00000400
#define    DSP_TX_CONTROL_TSDR_R_DEL_SHIFT                                0
#define    DSP_TX_CONTROL_TSDR_R_DEL_MASK                                 0x000003FF

#define DSP_TX_CONTROL_TDRFR_R_OFFSET                                     0x00000B0A
#define DSP_TX_CONTROL_TDRFR_R_TYPE                                       UInt16
#define DSP_TX_CONTROL_TDRFR_R_RESERVED_MASK                              0x0000FFFE
#define    DSP_TX_CONTROL_TDRFR_R_MULTI_ENABLE_SHIFT                      0
#define    DSP_TX_CONTROL_TDRFR_R_MULTI_ENABLE_MASK                       0x00000001

#define DSP_TX_CONTROL_T86R_R_OFFSET                                      0x00000B0C
#define DSP_TX_CONTROL_T86R_R_TYPE                                        UInt16
#define DSP_TX_CONTROL_T86R_R_RESERVED_MASK                               0x0000FFFF

#define DSP_TX_CONTROL_MFCRG_R_OFFSET                                     0x00000B0E
#define DSP_TX_CONTROL_MFCRG_R_TYPE                                       UInt16
#define DSP_TX_CONTROL_MFCRG_R_RESERVED_MASK                              0x0000F000
#define    DSP_TX_CONTROL_MFCRG_R_END_SHIFT                               0
#define    DSP_TX_CONTROL_MFCRG_R_END_MASK                                0x00000FFF

#define DSP_TX_CONTROL_MFCRP_R_OFFSET                                     0x00000B10
#define DSP_TX_CONTROL_MFCRP_R_TYPE                                       UInt16
#define DSP_TX_CONTROL_MFCRP_R_RESERVED_MASK                              0x0000F000
#define    DSP_TX_CONTROL_MFCRP_R_END_SHIFT                               0
#define    DSP_TX_CONTROL_MFCRP_R_END_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R0_OFFSET                                     0x00000B14
#define DSP_TX_CONTROL_TRDR_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R0_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R0_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R0_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R0_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R0_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R1_OFFSET                                     0x00000B16
#define DSP_TX_CONTROL_TRDR_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R1_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R1_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R1_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R1_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R1_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R2_OFFSET                                     0x00000B18
#define DSP_TX_CONTROL_TRDR_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R2_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R2_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R2_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R2_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R2_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R3_OFFSET                                     0x00000B1A
#define DSP_TX_CONTROL_TRDR_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R3_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R3_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R3_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R3_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R3_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R4_OFFSET                                     0x00000B1C
#define DSP_TX_CONTROL_TRDR_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R4_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R4_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R4_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R4_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R4_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R5_OFFSET                                     0x00000B1E
#define DSP_TX_CONTROL_TRDR_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R5_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R5_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R5_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R5_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R5_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_TRDR_R6_OFFSET                                     0x00000B20
#define DSP_TX_CONTROL_TRDR_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TRDR_R6_RESERVED_MASK                              0x00007000
#define    DSP_TX_CONTROL_TRDR_R6_END_SHIFT                               15
#define    DSP_TX_CONTROL_TRDR_R6_END_MASK                                0x00008000
#define    DSP_TX_CONTROL_TRDR_R6_DEL_SHIFT                               0
#define    DSP_TX_CONTROL_TRDR_R6_DEL_MASK                                0x00000FFF

#define DSP_TX_CONTROL_MSWTR_R0_OFFSET                                    0x00000B22
#define DSP_TX_CONTROL_MSWTR_R0_TYPE                                      UInt16
#define DSP_TX_CONTROL_MSWTR_R0_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_MSWTR_R0_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_MSWTR_R0_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_MSWTR_R1_OFFSET                                    0x00000B24
#define DSP_TX_CONTROL_MSWTR_R1_TYPE                                      UInt16
#define DSP_TX_CONTROL_MSWTR_R1_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_MSWTR_R1_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_MSWTR_R1_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_MSWTR_R2_OFFSET                                    0x00000B26
#define DSP_TX_CONTROL_MSWTR_R2_TYPE                                      UInt16
#define DSP_TX_CONTROL_MSWTR_R2_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_MSWTR_R2_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_MSWTR_R2_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_MSWTR_R3_OFFSET                                    0x00000B28
#define DSP_TX_CONTROL_MSWTR_R3_TYPE                                      UInt16
#define DSP_TX_CONTROL_MSWTR_R3_RESERVED_MASK                             0x0000F000
#define    DSP_TX_CONTROL_MSWTR_R3_DEL_SHIFT                              0
#define    DSP_TX_CONTROL_MSWTR_R3_DEL_MASK                               0x00000FFF

#define DSP_TX_CONTROL_APCSWG_R0_OFFSET                                   0x00000B2A
#define DSP_TX_CONTROL_APCSWG_R0_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWG_R0_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWG_R0_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWG_R0_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWG_R1_OFFSET                                   0x00000B2C
#define DSP_TX_CONTROL_APCSWG_R1_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWG_R1_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWG_R1_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWG_R1_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWG_R2_OFFSET                                   0x00000B2E
#define DSP_TX_CONTROL_APCSWG_R2_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWG_R2_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWG_R2_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWG_R2_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWG_R3_OFFSET                                   0x00000B30
#define DSP_TX_CONTROL_APCSWG_R3_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWG_R3_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWG_R3_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWG_R3_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWP_R0_OFFSET                                   0x00000B32
#define DSP_TX_CONTROL_APCSWP_R0_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWP_R0_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWP_R0_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWP_R0_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWP_R1_OFFSET                                   0x00000B34
#define DSP_TX_CONTROL_APCSWP_R1_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWP_R1_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWP_R1_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWP_R1_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWP_R2_OFFSET                                   0x00000B36
#define DSP_TX_CONTROL_APCSWP_R2_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWP_R2_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWP_R2_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWP_R2_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCSWP_R3_OFFSET                                   0x00000B38
#define DSP_TX_CONTROL_APCSWP_R3_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCSWP_R3_RESERVED_MASK                            0x0000F000
#define    DSP_TX_CONTROL_APCSWP_R3_DEL_SHIFT                             0
#define    DSP_TX_CONTROL_APCSWP_R3_DEL_MASK                              0x00000FFF

#define DSP_TX_CONTROL_APCGDFR_R_OFFSET                                   0x00000B3A
#define DSP_TX_CONTROL_APCGDFR_R_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCGDFR_R_RESERVED_MASK                            0x0000FC00
#define    DSP_TX_CONTROL_APCGDFR_R_APCGDFR_SHIFT                         0
#define    DSP_TX_CONTROL_APCGDFR_R_APCGDFR_MASK                          0x000003FF

#define DSP_TX_CONTROL_APCPDFR_R_OFFSET                                   0x00000B3C
#define DSP_TX_CONTROL_APCPDFR_R_TYPE                                     UInt16
#define DSP_TX_CONTROL_APCPDFR_R_RESERVED_MASK                            0x0000FC00
#define    DSP_TX_CONTROL_APCPDFR_R_APCPDFR_SHIFT                         0
#define    DSP_TX_CONTROL_APCPDFR_R_APCPDFR_MASK                          0x000003FF

#define DSP_TX_CONTROL_MTXCR_R_OFFSET                                     0x00000B3E
#define DSP_TX_CONTROL_MTXCR_R_TYPE                                       UInt16
#define DSP_TX_CONTROL_MTXCR_R_RESERVED_MASK                              0x00007F00
#define    DSP_TX_CONTROL_MTXCR_R_MIX_SHIFT                               15
#define    DSP_TX_CONTROL_MTXCR_R_MIX_MASK                                0x00008000
#define    DSP_TX_CONTROL_MTXCR_R_SLOT3_SHIFT                             6
#define    DSP_TX_CONTROL_MTXCR_R_SLOT3_MASK                              0x000000C0
#define       DSP_TX_CONTROL_MTXCR_R_SLOT3_CMD_G_MODE                     0x00000000
#define       DSP_TX_CONTROL_MTXCR_R_SLOT3_CMD_P_MODE                     0x00000002
#define    DSP_TX_CONTROL_MTXCR_R_SLOT2_SHIFT                             4
#define    DSP_TX_CONTROL_MTXCR_R_SLOT2_MASK                              0x00000030
#define       DSP_TX_CONTROL_MTXCR_R_SLOT2_CMD_G_MODE                     0x00000000
#define       DSP_TX_CONTROL_MTXCR_R_SLOT2_CMD_P_MODE                     0x00000002
#define    DSP_TX_CONTROL_MTXCR_R_SLOT1_SHIFT                             2
#define    DSP_TX_CONTROL_MTXCR_R_SLOT1_MASK                              0x0000000C
#define       DSP_TX_CONTROL_MTXCR_R_SLOT1_CMD_G_MODE                     0x00000000
#define       DSP_TX_CONTROL_MTXCR_R_SLOT1_CMD_P_MODE                     0x00000002
#define    DSP_TX_CONTROL_MTXCR_R_SLOT0_SHIFT                             0
#define    DSP_TX_CONTROL_MTXCR_R_SLOT0_MASK                              0x00000003
#define       DSP_TX_CONTROL_MTXCR_R_SLOT0_CMD_G_MODE                     0x00000000
#define       DSP_TX_CONTROL_MTXCR_R_SLOT0_CMD_P_MODE                     0x00000002

#define DSP_TX_CONTROL_TXCR_R_OFFSET                                      0x00000B64
#define DSP_TX_CONTROL_TXCR_R_TYPE                                        UInt16
#define DSP_TX_CONTROL_TXCR_R_RESERVED_MASK                               0x0000FC00
#define    DSP_TX_CONTROL_TXCR_R_RAMP_DIGRF2_SHIFT                        9
#define    DSP_TX_CONTROL_TXCR_R_RAMP_DIGRF2_MASK                         0x00000200
#define    DSP_TX_CONTROL_TXCR_R_TXSTAT_SHIFT                             8
#define    DSP_TX_CONTROL_TXCR_R_TXSTAT_MASK                              0x00000100
#define    DSP_TX_CONTROL_TXCR_R_TCON_SHIFT                               5
#define    DSP_TX_CONTROL_TXCR_R_TCON_MASK                                0x000000E0
#define    DSP_TX_CONTROL_TXCR_R_STOPTX_SHIFT                             4
#define    DSP_TX_CONTROL_TXCR_R_STOPTX_MASK                              0x00000010
#define    DSP_TX_CONTROL_TXCR_R_STARTTX_SHIFT                            3
#define    DSP_TX_CONTROL_TXCR_R_STARTTX_MASK                             0x00000008
#define    DSP_TX_CONTROL_TXCR_R_TBC_SHIFT                                0
#define    DSP_TX_CONTROL_TXCR_R_TBC_MASK                                 0x00000007

#define DSP_TX_CONTROL_MODADR_R_OFFSET                                    0x00000E00
#define DSP_TX_CONTROL_MODADR_R_TYPE                                      UInt16
#define DSP_TX_CONTROL_MODADR_R_RESERVED_MASK                             0x0000FF80
#define    DSP_TX_CONTROL_MODADR_R_BUFFER_SHIFT                           5
#define    DSP_TX_CONTROL_MODADR_R_BUFFER_MASK                            0x00000060
#define    DSP_TX_CONTROL_MODADR_R_ADDRESS_SHIFT                          0
#define    DSP_TX_CONTROL_MODADR_R_ADDRESS_MASK                           0x0000001F

#define DSP_TX_CONTROL_MODDAT_R_OFFSET                                    0x00000E02
#define DSP_TX_CONTROL_MODDAT_R_TYPE                                      UInt16
#define DSP_TX_CONTROL_MODDAT_R_RESERVED_MASK                             0x00000000
#define    DSP_TX_CONTROL_MODDAT_R_GSMMODE0_MODDAT_SHIFT                  0
#define    DSP_TX_CONTROL_MODDAT_R_GSMMODE0_MODDAT_MASK                   0x0000FFFF
#define    DSP_TX_CONTROL_MODDAT_R_PSKMODE0_MODDAT_SHIFT                  0
#define    DSP_TX_CONTROL_MODDAT_R_PSKMODE0_MODDAT_MASK                   0x00007FFF

#define DSP_TX_CONTROL_MODDAT2_R_OFFSET                                   0x00000E04
#define DSP_TX_CONTROL_MODDAT2_R_TYPE                                     UInt32
#define DSP_TX_CONTROL_MODDAT2_R_RESERVED_MASK                            0x00000000
#define    DSP_TX_CONTROL_MODDAT2_R_GSMMODE0_MODDAT_SHIFT                 0
#define    DSP_TX_CONTROL_MODDAT2_R_GSMMODE0_MODDAT_MASK                  0xFFFFFFFF
#define    DSP_TX_CONTROL_MODDAT2_R_PSKMODE0_MODDAT_U_SHIFT               16
#define    DSP_TX_CONTROL_MODDAT2_R_PSKMODE0_MODDAT_U_MASK                0x7FFF0000
#define    DSP_TX_CONTROL_MODDAT2_R_PSKMODE0_MODDAT_L_SHIFT               0
#define    DSP_TX_CONTROL_MODDAT2_R_PSKMODE0_MODDAT_L_MASK                0x00007FFF

#define DSP_TX_CONTROL_TPR0_R0_OFFSET                                     0x00000E60
#define DSP_TX_CONTROL_TPR0_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R1_OFFSET                                     0x00000E62
#define DSP_TX_CONTROL_TPR0_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R2_OFFSET                                     0x00000E64
#define DSP_TX_CONTROL_TPR0_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R3_OFFSET                                     0x00000E66
#define DSP_TX_CONTROL_TPR0_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R4_OFFSET                                     0x00000E68
#define DSP_TX_CONTROL_TPR0_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R5_OFFSET                                     0x00000E6A
#define DSP_TX_CONTROL_TPR0_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R6_OFFSET                                     0x00000E6C
#define DSP_TX_CONTROL_TPR0_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R7_OFFSET                                     0x00000E6E
#define DSP_TX_CONTROL_TPR0_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R8_OFFSET                                     0x00000E70
#define DSP_TX_CONTROL_TPR0_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R9_OFFSET                                     0x00000E72
#define DSP_TX_CONTROL_TPR0_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR0_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR0_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR0_R10_OFFSET                                    0x00000E74
#define DSP_TX_CONTROL_TPR0_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR0_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR0_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR0_R11_OFFSET                                    0x00000E76
#define DSP_TX_CONTROL_TPR0_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR0_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR0_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR0_R12_OFFSET                                    0x00000E78
#define DSP_TX_CONTROL_TPR0_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR0_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR0_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR0_R13_OFFSET                                    0x00000E7A
#define DSP_TX_CONTROL_TPR0_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR0_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR0_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR0_R14_OFFSET                                    0x00000E7C
#define DSP_TX_CONTROL_TPR0_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR0_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR0_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR0_R15_OFFSET                                    0x00000E7E
#define DSP_TX_CONTROL_TPR0_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR0_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR0_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR0_R15_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR1_R0_OFFSET                                     0x00000E80
#define DSP_TX_CONTROL_TPR1_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R1_OFFSET                                     0x00000E82
#define DSP_TX_CONTROL_TPR1_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R2_OFFSET                                     0x00000E84
#define DSP_TX_CONTROL_TPR1_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R3_OFFSET                                     0x00000E86
#define DSP_TX_CONTROL_TPR1_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R4_OFFSET                                     0x00000E88
#define DSP_TX_CONTROL_TPR1_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R5_OFFSET                                     0x00000E8A
#define DSP_TX_CONTROL_TPR1_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R6_OFFSET                                     0x00000E8C
#define DSP_TX_CONTROL_TPR1_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R7_OFFSET                                     0x00000E8E
#define DSP_TX_CONTROL_TPR1_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R8_OFFSET                                     0x00000E90
#define DSP_TX_CONTROL_TPR1_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R9_OFFSET                                     0x00000E92
#define DSP_TX_CONTROL_TPR1_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR1_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR1_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR1_R10_OFFSET                                    0x00000E94
#define DSP_TX_CONTROL_TPR1_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR1_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR1_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR1_R11_OFFSET                                    0x00000E96
#define DSP_TX_CONTROL_TPR1_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR1_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR1_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR1_R12_OFFSET                                    0x00000E98
#define DSP_TX_CONTROL_TPR1_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR1_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR1_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR1_R13_OFFSET                                    0x00000E9A
#define DSP_TX_CONTROL_TPR1_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR1_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR1_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR1_R14_OFFSET                                    0x00000E9C
#define DSP_TX_CONTROL_TPR1_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR1_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR1_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR1_R15_OFFSET                                    0x00000E9E
#define DSP_TX_CONTROL_TPR1_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR1_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR1_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR1_R15_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR2_R0_OFFSET                                     0x00000EA0
#define DSP_TX_CONTROL_TPR2_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R1_OFFSET                                     0x00000EA2
#define DSP_TX_CONTROL_TPR2_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R2_OFFSET                                     0x00000EA4
#define DSP_TX_CONTROL_TPR2_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R3_OFFSET                                     0x00000EA6
#define DSP_TX_CONTROL_TPR2_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R4_OFFSET                                     0x00000EA8
#define DSP_TX_CONTROL_TPR2_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R5_OFFSET                                     0x00000EAA
#define DSP_TX_CONTROL_TPR2_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R6_OFFSET                                     0x00000EAC
#define DSP_TX_CONTROL_TPR2_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R7_OFFSET                                     0x00000EAE
#define DSP_TX_CONTROL_TPR2_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R8_OFFSET                                     0x00000EB0
#define DSP_TX_CONTROL_TPR2_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R9_OFFSET                                     0x00000EB2
#define DSP_TX_CONTROL_TPR2_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR2_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR2_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR2_R10_OFFSET                                    0x00000EB4
#define DSP_TX_CONTROL_TPR2_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR2_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR2_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR2_R11_OFFSET                                    0x00000EB6
#define DSP_TX_CONTROL_TPR2_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR2_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR2_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR2_R12_OFFSET                                    0x00000EB8
#define DSP_TX_CONTROL_TPR2_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR2_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR2_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR2_R13_OFFSET                                    0x00000EBA
#define DSP_TX_CONTROL_TPR2_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR2_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR2_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR2_R14_OFFSET                                    0x00000EBC
#define DSP_TX_CONTROL_TPR2_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR2_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR2_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR2_R15_OFFSET                                    0x00000EBE
#define DSP_TX_CONTROL_TPR2_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR2_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR2_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR2_R15_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR3_R0_OFFSET                                     0x00000EC0
#define DSP_TX_CONTROL_TPR3_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R1_OFFSET                                     0x00000EC2
#define DSP_TX_CONTROL_TPR3_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R2_OFFSET                                     0x00000EC4
#define DSP_TX_CONTROL_TPR3_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R3_OFFSET                                     0x00000EC6
#define DSP_TX_CONTROL_TPR3_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R4_OFFSET                                     0x00000EC8
#define DSP_TX_CONTROL_TPR3_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R5_OFFSET                                     0x00000ECA
#define DSP_TX_CONTROL_TPR3_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R6_OFFSET                                     0x00000ECC
#define DSP_TX_CONTROL_TPR3_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R7_OFFSET                                     0x00000ECE
#define DSP_TX_CONTROL_TPR3_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R8_OFFSET                                     0x00000ED0
#define DSP_TX_CONTROL_TPR3_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R9_OFFSET                                     0x00000ED2
#define DSP_TX_CONTROL_TPR3_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR3_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR3_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR3_R10_OFFSET                                    0x00000ED4
#define DSP_TX_CONTROL_TPR3_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR3_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR3_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR3_R11_OFFSET                                    0x00000ED6
#define DSP_TX_CONTROL_TPR3_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR3_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR3_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR3_R12_OFFSET                                    0x00000ED8
#define DSP_TX_CONTROL_TPR3_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR3_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR3_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR3_R13_OFFSET                                    0x00000EDA
#define DSP_TX_CONTROL_TPR3_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR3_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR3_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR3_R14_OFFSET                                    0x00000EDC
#define DSP_TX_CONTROL_TPR3_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR3_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR3_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR3_R15_OFFSET                                    0x00000EDE
#define DSP_TX_CONTROL_TPR3_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR3_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR3_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR3_R15_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR4_R0_OFFSET                                     0x00000EE0
#define DSP_TX_CONTROL_TPR4_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R1_OFFSET                                     0x00000EE2
#define DSP_TX_CONTROL_TPR4_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R2_OFFSET                                     0x00000EE4
#define DSP_TX_CONTROL_TPR4_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R3_OFFSET                                     0x00000EE6
#define DSP_TX_CONTROL_TPR4_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R4_OFFSET                                     0x00000EE8
#define DSP_TX_CONTROL_TPR4_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R5_OFFSET                                     0x00000EEA
#define DSP_TX_CONTROL_TPR4_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R6_OFFSET                                     0x00000EEC
#define DSP_TX_CONTROL_TPR4_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R7_OFFSET                                     0x00000EEE
#define DSP_TX_CONTROL_TPR4_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R8_OFFSET                                     0x00000EF0
#define DSP_TX_CONTROL_TPR4_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R9_OFFSET                                     0x00000EF2
#define DSP_TX_CONTROL_TPR4_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR4_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR4_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR4_R10_OFFSET                                    0x00000EF4
#define DSP_TX_CONTROL_TPR4_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR4_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR4_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR4_R11_OFFSET                                    0x00000EF6
#define DSP_TX_CONTROL_TPR4_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR4_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR4_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR4_R12_OFFSET                                    0x00000EF8
#define DSP_TX_CONTROL_TPR4_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR4_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR4_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR4_R13_OFFSET                                    0x00000EFA
#define DSP_TX_CONTROL_TPR4_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR4_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR4_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR4_R14_OFFSET                                    0x00000EFC
#define DSP_TX_CONTROL_TPR4_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR4_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR4_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR4_R15_OFFSET                                    0x00000EFE
#define DSP_TX_CONTROL_TPR4_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR4_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR4_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR4_R15_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR5_R0_OFFSET                                     0x00000F00
#define DSP_TX_CONTROL_TPR5_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R1_OFFSET                                     0x00000F02
#define DSP_TX_CONTROL_TPR5_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R2_OFFSET                                     0x00000F04
#define DSP_TX_CONTROL_TPR5_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R3_OFFSET                                     0x00000F06
#define DSP_TX_CONTROL_TPR5_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R4_OFFSET                                     0x00000F08
#define DSP_TX_CONTROL_TPR5_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R5_OFFSET                                     0x00000F0A
#define DSP_TX_CONTROL_TPR5_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R6_OFFSET                                     0x00000F0C
#define DSP_TX_CONTROL_TPR5_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R7_OFFSET                                     0x00000F0E
#define DSP_TX_CONTROL_TPR5_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R8_OFFSET                                     0x00000F10
#define DSP_TX_CONTROL_TPR5_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R9_OFFSET                                     0x00000F12
#define DSP_TX_CONTROL_TPR5_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR5_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR5_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR5_R10_OFFSET                                    0x00000F14
#define DSP_TX_CONTROL_TPR5_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR5_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR5_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR5_R11_OFFSET                                    0x00000F16
#define DSP_TX_CONTROL_TPR5_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR5_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR5_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR5_R12_OFFSET                                    0x00000F18
#define DSP_TX_CONTROL_TPR5_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR5_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR5_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR5_R13_OFFSET                                    0x00000F1A
#define DSP_TX_CONTROL_TPR5_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR5_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR5_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR5_R14_OFFSET                                    0x00000F1C
#define DSP_TX_CONTROL_TPR5_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR5_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR5_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR5_R15_OFFSET                                    0x00000F1E
#define DSP_TX_CONTROL_TPR5_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR5_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR5_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR5_R15_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR6_R0_OFFSET                                     0x00000F20
#define DSP_TX_CONTROL_TPR6_R0_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R0_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R0_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R0_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R1_OFFSET                                     0x00000F22
#define DSP_TX_CONTROL_TPR6_R1_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R1_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R1_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R1_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R2_OFFSET                                     0x00000F24
#define DSP_TX_CONTROL_TPR6_R2_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R2_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R2_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R2_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R3_OFFSET                                     0x00000F26
#define DSP_TX_CONTROL_TPR6_R3_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R3_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R3_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R3_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R4_OFFSET                                     0x00000F28
#define DSP_TX_CONTROL_TPR6_R4_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R4_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R4_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R4_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R5_OFFSET                                     0x00000F2A
#define DSP_TX_CONTROL_TPR6_R5_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R5_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R5_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R5_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R6_OFFSET                                     0x00000F2C
#define DSP_TX_CONTROL_TPR6_R6_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R6_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R6_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R6_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R7_OFFSET                                     0x00000F2E
#define DSP_TX_CONTROL_TPR6_R7_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R7_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R7_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R7_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R8_OFFSET                                     0x00000F30
#define DSP_TX_CONTROL_TPR6_R8_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R8_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R8_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R8_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R9_OFFSET                                     0x00000F32
#define DSP_TX_CONTROL_TPR6_R9_TYPE                                       UInt16
#define DSP_TX_CONTROL_TPR6_R9_RESERVED_MASK                              0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R9_TPR_SHIFT                               0
#define    DSP_TX_CONTROL_TPR6_R9_TPR_MASK                                0x000003FF

#define DSP_TX_CONTROL_TPR6_R10_OFFSET                                    0x00000F34
#define DSP_TX_CONTROL_TPR6_R10_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR6_R10_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R10_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR6_R10_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR6_R11_OFFSET                                    0x00000F36
#define DSP_TX_CONTROL_TPR6_R11_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR6_R11_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R11_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR6_R11_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR6_R12_OFFSET                                    0x00000F38
#define DSP_TX_CONTROL_TPR6_R12_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR6_R12_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R12_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR6_R12_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR6_R13_OFFSET                                    0x00000F3A
#define DSP_TX_CONTROL_TPR6_R13_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR6_R13_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R13_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR6_R13_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR6_R14_OFFSET                                    0x00000F3C
#define DSP_TX_CONTROL_TPR6_R14_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR6_R14_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R14_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR6_R14_TPR_MASK                               0x000003FF

#define DSP_TX_CONTROL_TPR6_R15_OFFSET                                    0x00000F3E
#define DSP_TX_CONTROL_TPR6_R15_TYPE                                      UInt16
#define DSP_TX_CONTROL_TPR6_R15_RESERVED_MASK                             0x0000FC00
#define    DSP_TX_CONTROL_TPR6_R15_TPR_SHIFT                              0
#define    DSP_TX_CONTROL_TPR6_R15_TPR_MASK                               0x000003FF

#endif /* __BRCM_RDB_DSP_TX_CONTROL_H__ */



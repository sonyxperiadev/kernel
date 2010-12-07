/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:17:6                                             */
/*     RDB file : //R4/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_RFBB_TX2G_H__
#define __BRCM_RDB_RFBB_TX2G_H__

#define RFBB_TX2G_MODADR_OFFSET                                           0x00000000
#define RFBB_TX2G_MODADR_TYPE                                             UInt32
#define RFBB_TX2G_MODADR_RESERVED_MASK                                    0xFFFFFF80
#define    RFBB_TX2G_MODADR_BUFFER_SHIFT                                  5
#define    RFBB_TX2G_MODADR_BUFFER_MASK                                   0x00000060
#define    RFBB_TX2G_MODADR_ADDRESS_SHIFT                                 0
#define    RFBB_TX2G_MODADR_ADDRESS_MASK                                  0x0000001F

#define RFBB_TX2G_MODDAT_OFFSET                                           0x00000004
#define RFBB_TX2G_MODDAT_TYPE                                             UInt32
#define RFBB_TX2G_MODDAT_RESERVED_MASK                                    0x00000000
#define    RFBB_TX2G_MODDAT_MODDAT_SHIFT                                  0
#define    RFBB_TX2G_MODDAT_MODDAT_MASK                                   0xFFFFFFFF

#define RFBB_TX2G_TXCR_OFFSET                                             0x00000008
#define RFBB_TX2G_TXCR_TYPE                                               UInt32
#define RFBB_TX2G_TXCR_RESERVED_MASK                                      0xFF00FC00
#define    RFBB_TX2G_TXCR_SLOT3_SHIFT                                     22
#define    RFBB_TX2G_TXCR_SLOT3_MASK                                      0x00C00000
#define       RFBB_TX2G_TXCR_SLOT3_CMD_G_MODE                             0x00000000
#define       RFBB_TX2G_TXCR_SLOT3_CMD_P_MODE                             0x00000002
#define    RFBB_TX2G_TXCR_SLOT2_SHIFT                                     20
#define    RFBB_TX2G_TXCR_SLOT2_MASK                                      0x00300000
#define       RFBB_TX2G_TXCR_SLOT2_CMD_G_MODE                             0x00000000
#define       RFBB_TX2G_TXCR_SLOT2_CMD_P_MODE                             0x00000002
#define    RFBB_TX2G_TXCR_SLOT1_SHIFT                                     18
#define    RFBB_TX2G_TXCR_SLOT1_MASK                                      0x000C0000
#define       RFBB_TX2G_TXCR_SLOT1_CMD_G_MODE                             0x00000000
#define       RFBB_TX2G_TXCR_SLOT1_CMD_P_MODE                             0x00000002
#define    RFBB_TX2G_TXCR_SLOT0_SHIFT                                     16
#define    RFBB_TX2G_TXCR_SLOT0_MASK                                      0x00030000
#define       RFBB_TX2G_TXCR_SLOT0_CMD_G_MODE                             0x00000000
#define       RFBB_TX2G_TXCR_SLOT0_CMD_P_MODE                             0x00000002
#define    RFBB_TX2G_TXCR_TXIE_SHIFT                                      9
#define    RFBB_TX2G_TXCR_TXIE_MASK                                       0x00000200
#define    RFBB_TX2G_TXCR_TXSTAT_SHIFT                                    8
#define    RFBB_TX2G_TXCR_TXSTAT_MASK                                     0x00000100
#define    RFBB_TX2G_TXCR_TCON_SHIFT                                      5
#define    RFBB_TX2G_TXCR_TCON_MASK                                       0x000000E0
#define    RFBB_TX2G_TXCR_STOPTX_SHIFT                                    4
#define    RFBB_TX2G_TXCR_STOPTX_MASK                                     0x00000010
#define    RFBB_TX2G_TXCR_TXOP_SHIFT                                      0
#define    RFBB_TX2G_TXCR_TXOP_MASK                                       0x0000000F

#define RFBB_TX2G_TXT0_OFFSET                                             0x0000000C
#define RFBB_TX2G_TXT0_TYPE                                               UInt32
#define RFBB_TX2G_TXT0_RESERVED_MASK                                      0x00000000
#define    RFBB_TX2G_TXT0_TXT0_SHIFT                                      0
#define    RFBB_TX2G_TXT0_TXT0_MASK                                       0xFFFFFFFF

#define RFBB_TX2G_TSDR_OFFSET                                             0x00000010
#define RFBB_TX2G_TSDR_TYPE                                               UInt32
#define RFBB_TX2G_TSDR_RESERVED_MASK                                      0xFFFF8C00
#define    RFBB_TX2G_TSDR_MSDEL_SHIFT                                     12
#define    RFBB_TX2G_TSDR_MSDEL_MASK                                      0x00007000
#define    RFBB_TX2G_TSDR_DEL_SHIFT                                       0
#define    RFBB_TX2G_TSDR_DEL_MASK                                        0x000003FF

#define RFBB_TX2G_MSWTR_0_OFFSET                                          0x00000014
#define RFBB_TX2G_MSWTR_0_TYPE                                            UInt32
#define RFBB_TX2G_MSWTR_0_RESERVED_MASK                                   0xFC00FC00
#define    RFBB_TX2G_MSWTR_0_DEL1AND3_SHIFT                               16
#define    RFBB_TX2G_MSWTR_0_DEL1AND3_MASK                                0x03FF0000
#define    RFBB_TX2G_MSWTR_0_DEL0AND2_SHIFT                               0
#define    RFBB_TX2G_MSWTR_0_DEL0AND2_MASK                                0x000003FF

#define RFBB_TX2G_MSWTR_1_OFFSET                                          0x00000018
#define RFBB_TX2G_MSWTR_1_TYPE                                            UInt32
#define RFBB_TX2G_MSWTR_1_RESERVED_MASK                                   0xFC00FC00
#define    RFBB_TX2G_MSWTR_1_DEL1AND3_SHIFT                               16
#define    RFBB_TX2G_MSWTR_1_DEL1AND3_MASK                                0x03FF0000
#define    RFBB_TX2G_MSWTR_1_DEL0AND2_SHIFT                               0
#define    RFBB_TX2G_MSWTR_1_DEL0AND2_MASK                                0x000003FF

#define RFBB_TX2G_TXSLBUFSZ_OFFSET                                        0x0000001C
#define RFBB_TX2G_TXSLBUFSZ_TYPE                                          UInt32
#define RFBB_TX2G_TXSLBUFSZ_RESERVED_MASK                                 0x00000000
#define    RFBB_TX2G_TXSLBUFSZ_SL3BUFSIZE_SHIFT                           24
#define    RFBB_TX2G_TXSLBUFSZ_SL3BUFSIZE_MASK                            0xFF000000
#define    RFBB_TX2G_TXSLBUFSZ_SL2BUFSIZE_SHIFT                           16
#define    RFBB_TX2G_TXSLBUFSZ_SL2BUFSIZE_MASK                            0x00FF0000
#define    RFBB_TX2G_TXSLBUFSZ_SL1BUFSIZE_SHIFT                           8
#define    RFBB_TX2G_TXSLBUFSZ_SL1BUFSIZE_MASK                            0x0000FF00
#define    RFBB_TX2G_TXSLBUFSZ_SL0BUFSIZE_SHIFT                           0
#define    RFBB_TX2G_TXSLBUFSZ_SL0BUFSIZE_MASK                            0x000000FF

#define RFBB_TX2G_TDRFCR_OFFSET                                           0x00000040
#define RFBB_TX2G_TDRFCR_TYPE                                             UInt32
#define RFBB_TX2G_TDRFCR_RESERVED_MASK                                    0xFFFF3FD6
#define    RFBB_TX2G_TDRFCR_ENTSDR_SHIFT                                  15
#define    RFBB_TX2G_TDRFCR_ENTSDR_MASK                                   0x00008000
#define    RFBB_TX2G_TDRFCR_ENLASTGUARD_SHIFT                             14
#define    RFBB_TX2G_TDRFCR_ENLASTGUARD_MASK                              0x00004000
#define    RFBB_TX2G_TDRFCR_SKIPGUARD_SHIFT                               5
#define    RFBB_TX2G_TDRFCR_SKIPGUARD_MASK                                0x00000020
#define    RFBB_TX2G_TDRFCR_POLATX_SHIFT                                  3
#define    RFBB_TX2G_TDRFCR_POLATX_MASK                                   0x00000008
#define    RFBB_TX2G_TDRFCR_TXMODE_SHIFT                                  0
#define    RFBB_TX2G_TDRFCR_TXMODE_MASK                                   0x00000001

#define RFBB_TX2G_DRFLPM_OFFSET                                           0x00000044
#define RFBB_TX2G_DRFLPM_TYPE                                             UInt32
#define RFBB_TX2G_DRFLPM_RESERVED_MASK                                    0xFFFF8080
#define    RFBB_TX2G_DRFLPM_LPOS_SHIFT                                    8
#define    RFBB_TX2G_DRFLPM_LPOS_MASK                                     0x00007F00
#define    RFBB_TX2G_DRFLPM_LPRE_SHIFT                                    0
#define    RFBB_TX2G_DRFLPM_LPRE_MASK                                     0x0000007F

#define RFBB_TX2G_DRFPRE_0_OFFSET                                         0x00000048
#define RFBB_TX2G_DRFPRE_0_TYPE                                           UInt32
#define RFBB_TX2G_DRFPRE_0_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPRE_0_DRFPREH_SHIFT                               0
#define    RFBB_TX2G_DRFPRE_0_DRFPREH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPRE_1_OFFSET                                         0x0000004C
#define RFBB_TX2G_DRFPRE_1_TYPE                                           UInt32
#define RFBB_TX2G_DRFPRE_1_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPRE_1_DRFPREH_SHIFT                               0
#define    RFBB_TX2G_DRFPRE_1_DRFPREH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPRE_2_OFFSET                                         0x00000050
#define RFBB_TX2G_DRFPRE_2_TYPE                                           UInt32
#define RFBB_TX2G_DRFPRE_2_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPRE_2_DRFPREH_SHIFT                               0
#define    RFBB_TX2G_DRFPRE_2_DRFPREH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPRE_3_OFFSET                                         0x00000054
#define RFBB_TX2G_DRFPRE_3_TYPE                                           UInt32
#define RFBB_TX2G_DRFPRE_3_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPRE_3_DRFPREH_SHIFT                               0
#define    RFBB_TX2G_DRFPRE_3_DRFPREH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPOS_0_OFFSET                                         0x00000058
#define RFBB_TX2G_DRFPOS_0_TYPE                                           UInt32
#define RFBB_TX2G_DRFPOS_0_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPOS_0_DRFPOSH_SHIFT                               0
#define    RFBB_TX2G_DRFPOS_0_DRFPOSH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPOS_1_OFFSET                                         0x0000005C
#define RFBB_TX2G_DRFPOS_1_TYPE                                           UInt32
#define RFBB_TX2G_DRFPOS_1_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPOS_1_DRFPOSH_SHIFT                               0
#define    RFBB_TX2G_DRFPOS_1_DRFPOSH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPOS_2_OFFSET                                         0x00000060
#define RFBB_TX2G_DRFPOS_2_TYPE                                           UInt32
#define RFBB_TX2G_DRFPOS_2_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPOS_2_DRFPOSH_SHIFT                               0
#define    RFBB_TX2G_DRFPOS_2_DRFPOSH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_DRFPOS_3_OFFSET                                         0x00000064
#define RFBB_TX2G_DRFPOS_3_TYPE                                           UInt32
#define RFBB_TX2G_DRFPOS_3_RESERVED_MASK                                  0x00000000
#define    RFBB_TX2G_DRFPOS_3_DRFPOSH_SHIFT                               0
#define    RFBB_TX2G_DRFPOS_3_DRFPOSH_MASK                                0xFFFFFFFF

#define RFBB_TX2G_TRDR_0_OFFSET                                           0x00000080
#define RFBB_TX2G_TRDR_0_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_0_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_0_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_0_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_0_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_0_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_TRDR_1_OFFSET                                           0x00000084
#define RFBB_TX2G_TRDR_1_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_1_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_1_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_1_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_1_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_1_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_TRDR_2_OFFSET                                           0x00000088
#define RFBB_TX2G_TRDR_2_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_2_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_2_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_2_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_2_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_2_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_TRDR_3_OFFSET                                           0x0000008C
#define RFBB_TX2G_TRDR_3_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_3_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_3_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_3_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_3_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_3_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_TRDR_4_OFFSET                                           0x00000090
#define RFBB_TX2G_TRDR_4_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_4_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_4_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_4_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_4_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_4_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_TRDR_5_OFFSET                                           0x00000094
#define RFBB_TX2G_TRDR_5_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_5_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_5_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_5_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_5_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_5_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_TRDR_6_OFFSET                                           0x00000098
#define RFBB_TX2G_TRDR_6_TYPE                                             UInt32
#define RFBB_TX2G_TRDR_6_RESERVED_MASK                                    0xFFFF7000
#define    RFBB_TX2G_TRDR_6_END_SHIFT                                     15
#define    RFBB_TX2G_TRDR_6_END_MASK                                      0x00008000
#define    RFBB_TX2G_TRDR_6_DEL_SHIFT                                     0
#define    RFBB_TX2G_TRDR_6_DEL_MASK                                      0x00000FFF

#define RFBB_TX2G_AMPCR_OFFSET                                            0x000000BC
#define RFBB_TX2G_AMPCR_TYPE                                              UInt32
#define RFBB_TX2G_AMPCR_RESERVED_MASK                                     0xFFFFFFFC
#define    RFBB_TX2G_AMPCR_PWDAPCG_SHIFT                                  0
#define    RFBB_TX2G_AMPCR_PWDAPCG_MASK                                   0x00000003

#define RFBB_TX2G_TPR0_0_OFFSET                                           0x000000C0
#define RFBB_TX2G_TPR0_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_1_OFFSET                                           0x000000C4
#define RFBB_TX2G_TPR0_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_2_OFFSET                                           0x000000C8
#define RFBB_TX2G_TPR0_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_3_OFFSET                                           0x000000CC
#define RFBB_TX2G_TPR0_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_4_OFFSET                                           0x000000D0
#define RFBB_TX2G_TPR0_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_5_OFFSET                                           0x000000D4
#define RFBB_TX2G_TPR0_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_6_OFFSET                                           0x000000D8
#define RFBB_TX2G_TPR0_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR0_7_OFFSET                                           0x000000DC
#define RFBB_TX2G_TPR0_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR0_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR0_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR0_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR0_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR0_7_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_0_OFFSET                                           0x000000E0
#define RFBB_TX2G_TPR1_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_1_OFFSET                                           0x000000E4
#define RFBB_TX2G_TPR1_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_2_OFFSET                                           0x000000E8
#define RFBB_TX2G_TPR1_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_3_OFFSET                                           0x000000EC
#define RFBB_TX2G_TPR1_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_4_OFFSET                                           0x000000F0
#define RFBB_TX2G_TPR1_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_5_OFFSET                                           0x000000F4
#define RFBB_TX2G_TPR1_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_6_OFFSET                                           0x000000F8
#define RFBB_TX2G_TPR1_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR1_7_OFFSET                                           0x000000FC
#define RFBB_TX2G_TPR1_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR1_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR1_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR1_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR1_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR1_7_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_0_OFFSET                                           0x00000100
#define RFBB_TX2G_TPR2_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_1_OFFSET                                           0x00000104
#define RFBB_TX2G_TPR2_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_2_OFFSET                                           0x00000108
#define RFBB_TX2G_TPR2_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_3_OFFSET                                           0x0000010C
#define RFBB_TX2G_TPR2_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_4_OFFSET                                           0x00000110
#define RFBB_TX2G_TPR2_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_5_OFFSET                                           0x00000114
#define RFBB_TX2G_TPR2_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_6_OFFSET                                           0x00000118
#define RFBB_TX2G_TPR2_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR2_7_OFFSET                                           0x0000011C
#define RFBB_TX2G_TPR2_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR2_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR2_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR2_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR2_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR2_7_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_0_OFFSET                                           0x00000120
#define RFBB_TX2G_TPR3_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_1_OFFSET                                           0x00000124
#define RFBB_TX2G_TPR3_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_2_OFFSET                                           0x00000128
#define RFBB_TX2G_TPR3_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_3_OFFSET                                           0x0000012C
#define RFBB_TX2G_TPR3_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_4_OFFSET                                           0x00000130
#define RFBB_TX2G_TPR3_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_5_OFFSET                                           0x00000134
#define RFBB_TX2G_TPR3_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_6_OFFSET                                           0x00000138
#define RFBB_TX2G_TPR3_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR3_7_OFFSET                                           0x0000013C
#define RFBB_TX2G_TPR3_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR3_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR3_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR3_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR3_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR3_7_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_0_OFFSET                                           0x00000140
#define RFBB_TX2G_TPR4_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_1_OFFSET                                           0x00000144
#define RFBB_TX2G_TPR4_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_2_OFFSET                                           0x00000148
#define RFBB_TX2G_TPR4_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_3_OFFSET                                           0x0000014C
#define RFBB_TX2G_TPR4_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_4_OFFSET                                           0x00000150
#define RFBB_TX2G_TPR4_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_5_OFFSET                                           0x00000154
#define RFBB_TX2G_TPR4_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_6_OFFSET                                           0x00000158
#define RFBB_TX2G_TPR4_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR4_7_OFFSET                                           0x0000015C
#define RFBB_TX2G_TPR4_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR4_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR4_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR4_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR4_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR4_7_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_0_OFFSET                                           0x00000160
#define RFBB_TX2G_TPR5_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_1_OFFSET                                           0x00000164
#define RFBB_TX2G_TPR5_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_2_OFFSET                                           0x00000168
#define RFBB_TX2G_TPR5_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_3_OFFSET                                           0x0000016C
#define RFBB_TX2G_TPR5_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_4_OFFSET                                           0x00000170
#define RFBB_TX2G_TPR5_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_5_OFFSET                                           0x00000174
#define RFBB_TX2G_TPR5_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_6_OFFSET                                           0x00000178
#define RFBB_TX2G_TPR5_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR5_7_OFFSET                                           0x0000017C
#define RFBB_TX2G_TPR5_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR5_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR5_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR5_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR5_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR5_7_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_0_OFFSET                                           0x00000180
#define RFBB_TX2G_TPR6_0_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_0_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_0_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_0_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_0_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_0_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_1_OFFSET                                           0x00000184
#define RFBB_TX2G_TPR6_1_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_1_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_1_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_1_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_1_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_1_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_2_OFFSET                                           0x00000188
#define RFBB_TX2G_TPR6_2_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_2_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_2_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_2_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_2_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_2_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_3_OFFSET                                           0x0000018C
#define RFBB_TX2G_TPR6_3_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_3_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_3_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_3_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_3_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_3_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_4_OFFSET                                           0x00000190
#define RFBB_TX2G_TPR6_4_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_4_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_4_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_4_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_4_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_4_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_5_OFFSET                                           0x00000194
#define RFBB_TX2G_TPR6_5_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_5_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_5_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_5_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_5_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_5_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_6_OFFSET                                           0x00000198
#define RFBB_TX2G_TPR6_6_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_6_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_6_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_6_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_6_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_6_TPR0_MASK                                     0x000003FF

#define RFBB_TX2G_TPR6_7_OFFSET                                           0x0000019C
#define RFBB_TX2G_TPR6_7_TYPE                                             UInt32
#define RFBB_TX2G_TPR6_7_RESERVED_MASK                                    0xFC00FC00
#define    RFBB_TX2G_TPR6_7_TPR1_SHIFT                                    16
#define    RFBB_TX2G_TPR6_7_TPR1_MASK                                     0x03FF0000
#define    RFBB_TX2G_TPR6_7_TPR0_SHIFT                                    0
#define    RFBB_TX2G_TPR6_7_TPR0_MASK                                     0x000003FF

typedef volatile struct {
   UInt32 m_MODADR;                  // 0x0000
   UInt32 m_MODDAT;                  // 0x0004
   UInt32 m_TXCR;                    // 0x0008
   UInt32 m_TXT0;                    // 0x000C
   UInt32 m_TSDR;                    // 0x0010
   UInt32 m_MSWTR_0;                 // 0x0014
   UInt32 m_MSWTR_1;                 // 0x0018
   UInt32 m_TXSLBUFSZ;               // 0x001C
   UInt32 RESERVED_40_1C[8];
   UInt32 m_TDRFCR;                  // 0x0040
   UInt32 m_DRFLPM;                  // 0x0044
   UInt32 m_DRFPRE_0;                // 0x0048
   UInt32 m_DRFPRE_1;                // 0x004C
   UInt32 m_DRFPRE_2;                // 0x0050
   UInt32 m_DRFPRE_3;                // 0x0054
   UInt32 m_DRFPOS_0;                // 0x0058
   UInt32 m_DRFPOS_1;                // 0x005C
   UInt32 m_DRFPOS_2;                // 0x0060
   UInt32 m_DRFPOS_3;                // 0x0064
   UInt32 RESERVED_80_64[6];
   UInt32 m_TRDR_0;                  // 0x0080
   UInt32 m_TRDR_1;                  // 0x0084
   UInt32 m_TRDR_2;                  // 0x0088
   UInt32 m_TRDR_3;                  // 0x008C
   UInt32 m_TRDR_4;                  // 0x0090
   UInt32 m_TRDR_5;                  // 0x0094
   UInt32 m_TRDR_6;                  // 0x0098
   UInt32 RESERVED_BC_98[8];
   UInt32 m_AMPCR;                   // 0x00BC
   UInt32 m_TPR0_0;                  // 0x00C0
   UInt32 m_TPR0_1;                  // 0x00C4
   UInt32 m_TPR0_2;                  // 0x00C8
   UInt32 m_TPR0_3;                  // 0x00CC
   UInt32 m_TPR0_4;                  // 0x00D0
   UInt32 m_TPR0_5;                  // 0x00D4
   UInt32 m_TPR0_6;                  // 0x00D8
   UInt32 m_TPR0_7;                  // 0x00DC
   UInt32 m_TPR1_0;                  // 0x00E0
   UInt32 m_TPR1_1;                  // 0x00E4
   UInt32 m_TPR1_2;                  // 0x00E8
   UInt32 m_TPR1_3;                  // 0x00EC
   UInt32 m_TPR1_4;                  // 0x00F0
   UInt32 m_TPR1_5;                  // 0x00F4
   UInt32 m_TPR1_6;                  // 0x00F8
   UInt32 m_TPR1_7;                  // 0x00FC
   UInt32 m_TPR2_0;                  // 0x0100
   UInt32 m_TPR2_1;                  // 0x0104
   UInt32 m_TPR2_2;                  // 0x0108
   UInt32 m_TPR2_3;                  // 0x010C
   UInt32 m_TPR2_4;                  // 0x0110
   UInt32 m_TPR2_5;                  // 0x0114
   UInt32 m_TPR2_6;                  // 0x0118
   UInt32 m_TPR2_7;                  // 0x011C
   UInt32 m_TPR3_0;                  // 0x0120
   UInt32 m_TPR3_1;                  // 0x0124
   UInt32 m_TPR3_2;                  // 0x0128
   UInt32 m_TPR3_3;                  // 0x012C
   UInt32 m_TPR3_4;                  // 0x0130
   UInt32 m_TPR3_5;                  // 0x0134
   UInt32 m_TPR3_6;                  // 0x0138
   UInt32 m_TPR3_7;                  // 0x013C
   UInt32 m_TPR4_0;                  // 0x0140
   UInt32 m_TPR4_1;                  // 0x0144
   UInt32 m_TPR4_2;                  // 0x0148
   UInt32 m_TPR4_3;                  // 0x014C
   UInt32 m_TPR4_4;                  // 0x0150
   UInt32 m_TPR4_5;                  // 0x0154
   UInt32 m_TPR4_6;                  // 0x0158
   UInt32 m_TPR4_7;                  // 0x015C
   UInt32 m_TPR5_0;                  // 0x0160
   UInt32 m_TPR5_1;                  // 0x0164
   UInt32 m_TPR5_2;                  // 0x0168
   UInt32 m_TPR5_3;                  // 0x016C
   UInt32 m_TPR5_4;                  // 0x0170
   UInt32 m_TPR5_5;                  // 0x0174
   UInt32 m_TPR5_6;                  // 0x0178
   UInt32 m_TPR5_7;                  // 0x017C
   UInt32 m_TPR6_0;                  // 0x0180
   UInt32 m_TPR6_1;                  // 0x0184
   UInt32 m_TPR6_2;                  // 0x0188
   UInt32 m_TPR6_3;                  // 0x018C
   UInt32 m_TPR6_4;                  // 0x0190
   UInt32 m_TPR6_5;                  // 0x0194
   UInt32 m_TPR6_6;                  // 0x0198
   UInt32 m_TPR6_7;                  // 0x019C
} BRCM_RFBB_TX2G_REGS;


#endif /* __BRCM_RDB_RFBB_TX2G_H__ */



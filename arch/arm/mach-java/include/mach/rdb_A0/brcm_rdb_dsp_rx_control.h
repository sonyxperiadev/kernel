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

#ifndef __BRCM_RDB_DSP_RX_CONTROL_H__
#define __BRCM_RDB_DSP_RX_CONTROL_H__

#define DSP_RX_CONTROL_RXCR_OFFSET                                        0x0000E5B3
#define DSP_RX_CONTROL_RXCR_TYPE                                          UInt16
#define DSP_RX_CONTROL_RXCR_RESERVED_MASK                                 0x0000C000
#define    DSP_RX_CONTROL_RXCR_SPI_RX_INDEX_EN_SHIFT                      13
#define    DSP_RX_CONTROL_RXCR_SPI_RX_INDEX_EN_MASK                       0x00002000
#define    DSP_RX_CONTROL_RXCR_GPEN_RX_INDEX_EN_SHIFT                     12
#define    DSP_RX_CONTROL_RXCR_GPEN_RX_INDEX_EN_MASK                      0x00001000
#define    DSP_RX_CONTROL_RXCR_MON_EN_SHIFT                               11
#define    DSP_RX_CONTROL_RXCR_MON_EN_MASK                                0x00000800
#define    DSP_RX_CONTROL_RXCR_BYPASSFILT_SHIFT                           10
#define    DSP_RX_CONTROL_RXCR_BYPASSFILT_MASK                            0x00000400
#define    DSP_RX_CONTROL_RXCR_CLASS33_SHIFT                              9
#define    DSP_RX_CONTROL_RXCR_CLASS33_MASK                               0x00000200
#define    DSP_RX_CONTROL_RXCR_RXSTAT_SHIFT                               8
#define    DSP_RX_CONTROL_RXCR_RXSTAT_MASK                                0x00000100
#define    DSP_RX_CONTROL_RXCR_RCON_SHIFT                                 5
#define    DSP_RX_CONTROL_RXCR_RCON_MASK                                  0x000000E0
#define    DSP_RX_CONTROL_RXCR_STOPRX_SHIFT                               4
#define    DSP_RX_CONTROL_RXCR_STOPRX_MASK                                0x00000010
#define    DSP_RX_CONTROL_RXCR_STARRX_SHIFT                               3
#define    DSP_RX_CONTROL_RXCR_STARRX_MASK                                0x00000008
#define    DSP_RX_CONTROL_RXCR_RBC_SHIFT                                  0
#define    DSP_RX_CONTROL_RXCR_RBC_MASK                                   0x00000007

#define DSP_RX_CONTROL_RXSAMP_OFFSET                                      0x0000E5B4
#define DSP_RX_CONTROL_RXSAMP_TYPE                                        UInt16
#define DSP_RX_CONTROL_RXSAMP_RESERVED_MASK                               0x0000FF00
#define    DSP_RX_CONTROL_RXSAMP_RXNUM_SHIFT                              0
#define    DSP_RX_CONTROL_RXSAMP_RXNUM_MASK                               0x000000FF

#define DSP_RX_CONTROL_RCOR_OFFSET                                        0x0000E5C0
#define DSP_RX_CONTROL_RCOR_TYPE                                          UInt16
#define DSP_RX_CONTROL_RCOR_RESERVED_MASK                                 0x00000000
#define    DSP_RX_CONTROL_RCOR_RCQ_SHIFT                                  8
#define    DSP_RX_CONTROL_RCOR_RCQ_MASK                                   0x0000FF00
#define    DSP_RX_CONTROL_RCOR_RCI_SHIFT                                  0
#define    DSP_RX_CONTROL_RCOR_RCI_MASK                                   0x000000FF

#define DSP_RX_CONTROL_RSDR_OFFSET                                        0x0000E5C2
#define DSP_RX_CONTROL_RSDR_TYPE                                          UInt16
#define DSP_RX_CONTROL_RSDR_RESERVED_MASK                                 0x0000FC00
#define    DSP_RX_CONTROL_RSDR_DEL_SHIFT                                  0
#define    DSP_RX_CONTROL_RSDR_DEL_MASK                                   0x000003FF

#define DSP_RX_CONTROL_RSDR_MON_OFFSET                                    0x0000E5C3
#define DSP_RX_CONTROL_RSDR_MON_TYPE                                      UInt16
#define DSP_RX_CONTROL_RSDR_MON_RESERVED_MASK                             0x0000FC00
#define    DSP_RX_CONTROL_RSDR_MON_DEL_SHIFT                              0
#define    DSP_RX_CONTROL_RSDR_MON_DEL_MASK                               0x000003FF

#define DSP_RX_CONTROL_RFRCOR_OFFSET                                      0x0000E5C4
#define DSP_RX_CONTROL_RFRCOR_TYPE                                        UInt16
#define DSP_RX_CONTROL_RFRCOR_RESERVED_MASK                               0x00000000
#define    DSP_RX_CONTROL_RFRCOR_RFRCQ_SHIFT                              8
#define    DSP_RX_CONTROL_RFRCOR_RFRCQ_MASK                               0x0000FF00
#define    DSP_RX_CONTROL_RFRCOR_RFRCI_SHIFT                              0
#define    DSP_RX_CONTROL_RFRCOR_RFRCI_MASK                               0x000000FF

#define DSP_RX_CONTROL_REC4ADR_OFFSET                                     0x0000E5C8
#define DSP_RX_CONTROL_REC4ADR_TYPE                                       UInt16
#define DSP_RX_CONTROL_REC4ADR_RESERVED_MASK                              0x0000FFF0
#define    DSP_RX_CONTROL_REC4ADR_ADDRESS_SHIFT                           0
#define    DSP_RX_CONTROL_REC4ADR_ADDRESS_MASK                            0x0000000F

#define DSP_RX_CONTROL_REC4DAT_OFFSET                                     0x0000E5C9
#define DSP_RX_CONTROL_REC4DAT_TYPE                                       UInt16
#define DSP_RX_CONTROL_REC4DAT_RESERVED_MASK                              0x0000E000
#define    DSP_RX_CONTROL_REC4DAT_DATA13BIT_SHIFT                         0
#define    DSP_RX_CONTROL_REC4DAT_DATA13BIT_MASK                          0x00001FFF

#define DSP_RX_CONTROL_RXMATCH_OFFSET                                     0x0000E5CC
#define DSP_RX_CONTROL_RXMATCH_TYPE                                       UInt16
#define DSP_RX_CONTROL_RXMATCH_RESERVED_MASK                              0x0000FFC0
#define    DSP_RX_CONTROL_RXMATCH_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXMATCH_DATA_MASK                               0x0000003F

#define DSP_RX_CONTROL_RXBUF_0_OFFSET                                     0x0000E800
#define DSP_RX_CONTROL_RXBUF_0_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_0_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_0_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_0_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_1_OFFSET                                     0x0000E802
#define DSP_RX_CONTROL_RXBUF_1_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_1_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_1_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_1_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_2_OFFSET                                     0x0000E804
#define DSP_RX_CONTROL_RXBUF_2_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_2_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_2_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_2_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_3_OFFSET                                     0x0000E806
#define DSP_RX_CONTROL_RXBUF_3_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_3_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_3_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_3_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_4_OFFSET                                     0x0000E808
#define DSP_RX_CONTROL_RXBUF_4_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_4_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_4_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_4_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_5_OFFSET                                     0x0000E80A
#define DSP_RX_CONTROL_RXBUF_5_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_5_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_5_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_5_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_6_OFFSET                                     0x0000E80C
#define DSP_RX_CONTROL_RXBUF_6_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_6_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_6_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_6_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_7_OFFSET                                     0x0000E80E
#define DSP_RX_CONTROL_RXBUF_7_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_7_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_7_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_7_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_8_OFFSET                                     0x0000E810
#define DSP_RX_CONTROL_RXBUF_8_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_8_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_8_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_8_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_9_OFFSET                                     0x0000E812
#define DSP_RX_CONTROL_RXBUF_9_TYPE                                       UInt32
#define DSP_RX_CONTROL_RXBUF_9_RESERVED_MASK                              0x00000000
#define    DSP_RX_CONTROL_RXBUF_9_DATA_SHIFT                              0
#define    DSP_RX_CONTROL_RXBUF_9_DATA_MASK                               0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_10_OFFSET                                    0x0000E814
#define DSP_RX_CONTROL_RXBUF_10_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_10_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_10_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_10_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_11_OFFSET                                    0x0000E816
#define DSP_RX_CONTROL_RXBUF_11_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_11_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_11_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_11_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_12_OFFSET                                    0x0000E818
#define DSP_RX_CONTROL_RXBUF_12_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_12_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_12_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_12_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_13_OFFSET                                    0x0000E81A
#define DSP_RX_CONTROL_RXBUF_13_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_13_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_13_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_13_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_14_OFFSET                                    0x0000E81C
#define DSP_RX_CONTROL_RXBUF_14_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_14_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_14_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_14_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_15_OFFSET                                    0x0000E81E
#define DSP_RX_CONTROL_RXBUF_15_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_15_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_15_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_15_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_16_OFFSET                                    0x0000E820
#define DSP_RX_CONTROL_RXBUF_16_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_16_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_16_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_16_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_17_OFFSET                                    0x0000E822
#define DSP_RX_CONTROL_RXBUF_17_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_17_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_17_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_17_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_18_OFFSET                                    0x0000E824
#define DSP_RX_CONTROL_RXBUF_18_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_18_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_18_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_18_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_19_OFFSET                                    0x0000E826
#define DSP_RX_CONTROL_RXBUF_19_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_19_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_19_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_19_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_20_OFFSET                                    0x0000E828
#define DSP_RX_CONTROL_RXBUF_20_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_20_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_20_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_20_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_21_OFFSET                                    0x0000E82A
#define DSP_RX_CONTROL_RXBUF_21_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_21_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_21_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_21_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_22_OFFSET                                    0x0000E82C
#define DSP_RX_CONTROL_RXBUF_22_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_22_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_22_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_22_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_23_OFFSET                                    0x0000E82E
#define DSP_RX_CONTROL_RXBUF_23_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_23_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_23_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_23_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_24_OFFSET                                    0x0000E830
#define DSP_RX_CONTROL_RXBUF_24_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_24_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_24_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_24_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_25_OFFSET                                    0x0000E832
#define DSP_RX_CONTROL_RXBUF_25_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_25_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_25_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_25_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_26_OFFSET                                    0x0000E834
#define DSP_RX_CONTROL_RXBUF_26_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_26_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_26_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_26_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_27_OFFSET                                    0x0000E836
#define DSP_RX_CONTROL_RXBUF_27_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_27_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_27_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_27_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_28_OFFSET                                    0x0000E838
#define DSP_RX_CONTROL_RXBUF_28_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_28_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_28_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_28_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_29_OFFSET                                    0x0000E83A
#define DSP_RX_CONTROL_RXBUF_29_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_29_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_29_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_29_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_30_OFFSET                                    0x0000E83C
#define DSP_RX_CONTROL_RXBUF_30_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_30_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_30_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_30_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_31_OFFSET                                    0x0000E83E
#define DSP_RX_CONTROL_RXBUF_31_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_31_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_31_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_31_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_32_OFFSET                                    0x0000E840
#define DSP_RX_CONTROL_RXBUF_32_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_32_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_32_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_32_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_33_OFFSET                                    0x0000E842
#define DSP_RX_CONTROL_RXBUF_33_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_33_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_33_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_33_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_34_OFFSET                                    0x0000E844
#define DSP_RX_CONTROL_RXBUF_34_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_34_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_34_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_34_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_35_OFFSET                                    0x0000E846
#define DSP_RX_CONTROL_RXBUF_35_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_35_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_35_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_35_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_36_OFFSET                                    0x0000E848
#define DSP_RX_CONTROL_RXBUF_36_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_36_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_36_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_36_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_37_OFFSET                                    0x0000E84A
#define DSP_RX_CONTROL_RXBUF_37_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_37_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_37_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_37_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_38_OFFSET                                    0x0000E84C
#define DSP_RX_CONTROL_RXBUF_38_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_38_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_38_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_38_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_39_OFFSET                                    0x0000E84E
#define DSP_RX_CONTROL_RXBUF_39_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_39_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_39_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_39_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_40_OFFSET                                    0x0000E850
#define DSP_RX_CONTROL_RXBUF_40_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_40_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_40_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_40_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_41_OFFSET                                    0x0000E852
#define DSP_RX_CONTROL_RXBUF_41_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_41_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_41_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_41_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_42_OFFSET                                    0x0000E854
#define DSP_RX_CONTROL_RXBUF_42_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_42_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_42_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_42_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_43_OFFSET                                    0x0000E856
#define DSP_RX_CONTROL_RXBUF_43_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_43_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_43_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_43_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_44_OFFSET                                    0x0000E858
#define DSP_RX_CONTROL_RXBUF_44_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_44_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_44_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_44_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_45_OFFSET                                    0x0000E85A
#define DSP_RX_CONTROL_RXBUF_45_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_45_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_45_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_45_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_46_OFFSET                                    0x0000E85C
#define DSP_RX_CONTROL_RXBUF_46_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_46_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_46_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_46_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_47_OFFSET                                    0x0000E85E
#define DSP_RX_CONTROL_RXBUF_47_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_47_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_47_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_47_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_48_OFFSET                                    0x0000E860
#define DSP_RX_CONTROL_RXBUF_48_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_48_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_48_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_48_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_49_OFFSET                                    0x0000E862
#define DSP_RX_CONTROL_RXBUF_49_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_49_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_49_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_49_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_50_OFFSET                                    0x0000E864
#define DSP_RX_CONTROL_RXBUF_50_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_50_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_50_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_50_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_51_OFFSET                                    0x0000E866
#define DSP_RX_CONTROL_RXBUF_51_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_51_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_51_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_51_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_52_OFFSET                                    0x0000E868
#define DSP_RX_CONTROL_RXBUF_52_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_52_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_52_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_52_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_53_OFFSET                                    0x0000E86A
#define DSP_RX_CONTROL_RXBUF_53_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_53_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_53_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_53_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_54_OFFSET                                    0x0000E86C
#define DSP_RX_CONTROL_RXBUF_54_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_54_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_54_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_54_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_55_OFFSET                                    0x0000E86E
#define DSP_RX_CONTROL_RXBUF_55_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_55_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_55_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_55_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_56_OFFSET                                    0x0000E870
#define DSP_RX_CONTROL_RXBUF_56_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_56_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_56_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_56_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_57_OFFSET                                    0x0000E872
#define DSP_RX_CONTROL_RXBUF_57_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_57_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_57_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_57_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_58_OFFSET                                    0x0000E874
#define DSP_RX_CONTROL_RXBUF_58_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_58_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_58_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_58_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_59_OFFSET                                    0x0000E876
#define DSP_RX_CONTROL_RXBUF_59_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_59_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_59_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_59_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_60_OFFSET                                    0x0000E878
#define DSP_RX_CONTROL_RXBUF_60_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_60_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_60_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_60_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_61_OFFSET                                    0x0000E87A
#define DSP_RX_CONTROL_RXBUF_61_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_61_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_61_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_61_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_62_OFFSET                                    0x0000E87C
#define DSP_RX_CONTROL_RXBUF_62_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_62_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_62_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_62_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_63_OFFSET                                    0x0000E87E
#define DSP_RX_CONTROL_RXBUF_63_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_63_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_63_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_63_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_64_OFFSET                                    0x0000E880
#define DSP_RX_CONTROL_RXBUF_64_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_64_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_64_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_64_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_65_OFFSET                                    0x0000E882
#define DSP_RX_CONTROL_RXBUF_65_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_65_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_65_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_65_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_66_OFFSET                                    0x0000E884
#define DSP_RX_CONTROL_RXBUF_66_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_66_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_66_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_66_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_67_OFFSET                                    0x0000E886
#define DSP_RX_CONTROL_RXBUF_67_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_67_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_67_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_67_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_68_OFFSET                                    0x0000E888
#define DSP_RX_CONTROL_RXBUF_68_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_68_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_68_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_68_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_69_OFFSET                                    0x0000E88A
#define DSP_RX_CONTROL_RXBUF_69_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_69_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_69_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_69_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_70_OFFSET                                    0x0000E88C
#define DSP_RX_CONTROL_RXBUF_70_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_70_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_70_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_70_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_71_OFFSET                                    0x0000E88E
#define DSP_RX_CONTROL_RXBUF_71_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_71_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_71_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_71_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_72_OFFSET                                    0x0000E890
#define DSP_RX_CONTROL_RXBUF_72_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_72_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_72_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_72_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_73_OFFSET                                    0x0000E892
#define DSP_RX_CONTROL_RXBUF_73_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_73_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_73_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_73_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_74_OFFSET                                    0x0000E894
#define DSP_RX_CONTROL_RXBUF_74_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_74_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_74_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_74_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_75_OFFSET                                    0x0000E896
#define DSP_RX_CONTROL_RXBUF_75_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_75_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_75_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_75_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_76_OFFSET                                    0x0000E898
#define DSP_RX_CONTROL_RXBUF_76_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_76_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_76_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_76_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_77_OFFSET                                    0x0000E89A
#define DSP_RX_CONTROL_RXBUF_77_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_77_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_77_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_77_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_78_OFFSET                                    0x0000E89C
#define DSP_RX_CONTROL_RXBUF_78_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_78_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_78_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_78_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_79_OFFSET                                    0x0000E89E
#define DSP_RX_CONTROL_RXBUF_79_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_79_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_79_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_79_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_80_OFFSET                                    0x0000E8A0
#define DSP_RX_CONTROL_RXBUF_80_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_80_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_80_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_80_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_81_OFFSET                                    0x0000E8A2
#define DSP_RX_CONTROL_RXBUF_81_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_81_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_81_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_81_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_82_OFFSET                                    0x0000E8A4
#define DSP_RX_CONTROL_RXBUF_82_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_82_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_82_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_82_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_83_OFFSET                                    0x0000E8A6
#define DSP_RX_CONTROL_RXBUF_83_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_83_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_83_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_83_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_84_OFFSET                                    0x0000E8A8
#define DSP_RX_CONTROL_RXBUF_84_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_84_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_84_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_84_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_85_OFFSET                                    0x0000E8AA
#define DSP_RX_CONTROL_RXBUF_85_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_85_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_85_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_85_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_86_OFFSET                                    0x0000E8AC
#define DSP_RX_CONTROL_RXBUF_86_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_86_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_86_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_86_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_87_OFFSET                                    0x0000E8AE
#define DSP_RX_CONTROL_RXBUF_87_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_87_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_87_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_87_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_88_OFFSET                                    0x0000E8B0
#define DSP_RX_CONTROL_RXBUF_88_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_88_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_88_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_88_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_89_OFFSET                                    0x0000E8B2
#define DSP_RX_CONTROL_RXBUF_89_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_89_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_89_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_89_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_90_OFFSET                                    0x0000E8B4
#define DSP_RX_CONTROL_RXBUF_90_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_90_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_90_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_90_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_91_OFFSET                                    0x0000E8B6
#define DSP_RX_CONTROL_RXBUF_91_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_91_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_91_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_91_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_92_OFFSET                                    0x0000E8B8
#define DSP_RX_CONTROL_RXBUF_92_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_92_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_92_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_92_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_93_OFFSET                                    0x0000E8BA
#define DSP_RX_CONTROL_RXBUF_93_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_93_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_93_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_93_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_94_OFFSET                                    0x0000E8BC
#define DSP_RX_CONTROL_RXBUF_94_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_94_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_94_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_94_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_95_OFFSET                                    0x0000E8BE
#define DSP_RX_CONTROL_RXBUF_95_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_95_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_95_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_95_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_96_OFFSET                                    0x0000E8C0
#define DSP_RX_CONTROL_RXBUF_96_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_96_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_96_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_96_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_97_OFFSET                                    0x0000E8C2
#define DSP_RX_CONTROL_RXBUF_97_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_97_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_97_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_97_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_98_OFFSET                                    0x0000E8C4
#define DSP_RX_CONTROL_RXBUF_98_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_98_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_98_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_98_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_99_OFFSET                                    0x0000E8C6
#define DSP_RX_CONTROL_RXBUF_99_TYPE                                      UInt32
#define DSP_RX_CONTROL_RXBUF_99_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RXBUF_99_DATA_SHIFT                             0
#define    DSP_RX_CONTROL_RXBUF_99_DATA_MASK                              0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_100_OFFSET                                   0x0000E8C8
#define DSP_RX_CONTROL_RXBUF_100_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_100_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_100_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_100_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_101_OFFSET                                   0x0000E8CA
#define DSP_RX_CONTROL_RXBUF_101_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_101_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_101_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_101_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_102_OFFSET                                   0x0000E8CC
#define DSP_RX_CONTROL_RXBUF_102_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_102_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_102_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_102_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_103_OFFSET                                   0x0000E8CE
#define DSP_RX_CONTROL_RXBUF_103_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_103_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_103_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_103_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_104_OFFSET                                   0x0000E8D0
#define DSP_RX_CONTROL_RXBUF_104_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_104_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_104_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_104_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_105_OFFSET                                   0x0000E8D2
#define DSP_RX_CONTROL_RXBUF_105_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_105_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_105_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_105_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_106_OFFSET                                   0x0000E8D4
#define DSP_RX_CONTROL_RXBUF_106_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_106_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_106_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_106_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_107_OFFSET                                   0x0000E8D6
#define DSP_RX_CONTROL_RXBUF_107_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_107_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_107_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_107_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_108_OFFSET                                   0x0000E8D8
#define DSP_RX_CONTROL_RXBUF_108_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_108_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_108_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_108_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_109_OFFSET                                   0x0000E8DA
#define DSP_RX_CONTROL_RXBUF_109_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_109_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_109_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_109_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_110_OFFSET                                   0x0000E8DC
#define DSP_RX_CONTROL_RXBUF_110_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_110_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_110_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_110_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_111_OFFSET                                   0x0000E8DE
#define DSP_RX_CONTROL_RXBUF_111_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_111_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_111_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_111_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_112_OFFSET                                   0x0000E8E0
#define DSP_RX_CONTROL_RXBUF_112_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_112_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_112_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_112_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_113_OFFSET                                   0x0000E8E2
#define DSP_RX_CONTROL_RXBUF_113_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_113_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_113_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_113_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_114_OFFSET                                   0x0000E8E4
#define DSP_RX_CONTROL_RXBUF_114_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_114_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_114_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_114_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_115_OFFSET                                   0x0000E8E6
#define DSP_RX_CONTROL_RXBUF_115_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_115_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_115_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_115_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_116_OFFSET                                   0x0000E8E8
#define DSP_RX_CONTROL_RXBUF_116_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_116_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_116_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_116_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_117_OFFSET                                   0x0000E8EA
#define DSP_RX_CONTROL_RXBUF_117_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_117_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_117_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_117_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_118_OFFSET                                   0x0000E8EC
#define DSP_RX_CONTROL_RXBUF_118_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_118_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_118_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_118_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_119_OFFSET                                   0x0000E8EE
#define DSP_RX_CONTROL_RXBUF_119_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_119_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_119_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_119_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_120_OFFSET                                   0x0000E8F0
#define DSP_RX_CONTROL_RXBUF_120_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_120_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_120_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_120_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_121_OFFSET                                   0x0000E8F2
#define DSP_RX_CONTROL_RXBUF_121_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_121_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_121_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_121_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_122_OFFSET                                   0x0000E8F4
#define DSP_RX_CONTROL_RXBUF_122_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_122_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_122_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_122_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_123_OFFSET                                   0x0000E8F6
#define DSP_RX_CONTROL_RXBUF_123_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_123_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_123_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_123_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_124_OFFSET                                   0x0000E8F8
#define DSP_RX_CONTROL_RXBUF_124_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_124_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_124_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_124_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_125_OFFSET                                   0x0000E8FA
#define DSP_RX_CONTROL_RXBUF_125_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_125_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_125_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_125_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_126_OFFSET                                   0x0000E8FC
#define DSP_RX_CONTROL_RXBUF_126_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_126_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_126_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_126_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_127_OFFSET                                   0x0000E8FE
#define DSP_RX_CONTROL_RXBUF_127_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_127_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_127_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_127_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_128_OFFSET                                   0x0000E900
#define DSP_RX_CONTROL_RXBUF_128_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_128_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_128_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_128_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_129_OFFSET                                   0x0000E902
#define DSP_RX_CONTROL_RXBUF_129_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_129_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_129_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_129_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_130_OFFSET                                   0x0000E904
#define DSP_RX_CONTROL_RXBUF_130_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_130_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_130_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_130_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_131_OFFSET                                   0x0000E906
#define DSP_RX_CONTROL_RXBUF_131_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_131_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_131_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_131_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_132_OFFSET                                   0x0000E908
#define DSP_RX_CONTROL_RXBUF_132_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_132_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_132_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_132_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_133_OFFSET                                   0x0000E90A
#define DSP_RX_CONTROL_RXBUF_133_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_133_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_133_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_133_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_134_OFFSET                                   0x0000E90C
#define DSP_RX_CONTROL_RXBUF_134_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_134_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_134_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_134_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_135_OFFSET                                   0x0000E90E
#define DSP_RX_CONTROL_RXBUF_135_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_135_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_135_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_135_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_136_OFFSET                                   0x0000E910
#define DSP_RX_CONTROL_RXBUF_136_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_136_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_136_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_136_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_137_OFFSET                                   0x0000E912
#define DSP_RX_CONTROL_RXBUF_137_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_137_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_137_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_137_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_138_OFFSET                                   0x0000E914
#define DSP_RX_CONTROL_RXBUF_138_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_138_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_138_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_138_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_139_OFFSET                                   0x0000E916
#define DSP_RX_CONTROL_RXBUF_139_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_139_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_139_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_139_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_140_OFFSET                                   0x0000E918
#define DSP_RX_CONTROL_RXBUF_140_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_140_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_140_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_140_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_141_OFFSET                                   0x0000E91A
#define DSP_RX_CONTROL_RXBUF_141_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_141_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_141_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_141_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_142_OFFSET                                   0x0000E91C
#define DSP_RX_CONTROL_RXBUF_142_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_142_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_142_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_142_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_143_OFFSET                                   0x0000E91E
#define DSP_RX_CONTROL_RXBUF_143_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_143_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_143_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_143_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_144_OFFSET                                   0x0000E920
#define DSP_RX_CONTROL_RXBUF_144_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_144_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_144_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_144_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_145_OFFSET                                   0x0000E922
#define DSP_RX_CONTROL_RXBUF_145_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_145_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_145_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_145_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_146_OFFSET                                   0x0000E924
#define DSP_RX_CONTROL_RXBUF_146_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_146_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_146_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_146_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_147_OFFSET                                   0x0000E926
#define DSP_RX_CONTROL_RXBUF_147_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_147_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_147_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_147_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_148_OFFSET                                   0x0000E928
#define DSP_RX_CONTROL_RXBUF_148_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_148_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_148_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_148_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_149_OFFSET                                   0x0000E92A
#define DSP_RX_CONTROL_RXBUF_149_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_149_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_149_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_149_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_150_OFFSET                                   0x0000E92C
#define DSP_RX_CONTROL_RXBUF_150_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_150_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_150_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_150_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_151_OFFSET                                   0x0000E92E
#define DSP_RX_CONTROL_RXBUF_151_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_151_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_151_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_151_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_152_OFFSET                                   0x0000E930
#define DSP_RX_CONTROL_RXBUF_152_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_152_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_152_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_152_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_153_OFFSET                                   0x0000E932
#define DSP_RX_CONTROL_RXBUF_153_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_153_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_153_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_153_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_154_OFFSET                                   0x0000E934
#define DSP_RX_CONTROL_RXBUF_154_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_154_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_154_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_154_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_155_OFFSET                                   0x0000E936
#define DSP_RX_CONTROL_RXBUF_155_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_155_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_155_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_155_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_156_OFFSET                                   0x0000E938
#define DSP_RX_CONTROL_RXBUF_156_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_156_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_156_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_156_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_157_OFFSET                                   0x0000E93A
#define DSP_RX_CONTROL_RXBUF_157_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_157_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_157_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_157_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_158_OFFSET                                   0x0000E93C
#define DSP_RX_CONTROL_RXBUF_158_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_158_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_158_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_158_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_159_OFFSET                                   0x0000E93E
#define DSP_RX_CONTROL_RXBUF_159_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_159_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_159_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_159_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_160_OFFSET                                   0x0000E940
#define DSP_RX_CONTROL_RXBUF_160_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_160_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_160_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_160_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_161_OFFSET                                   0x0000E942
#define DSP_RX_CONTROL_RXBUF_161_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_161_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_161_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_161_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_162_OFFSET                                   0x0000E944
#define DSP_RX_CONTROL_RXBUF_162_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_162_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_162_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_162_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_163_OFFSET                                   0x0000E946
#define DSP_RX_CONTROL_RXBUF_163_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_163_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_163_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_163_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_164_OFFSET                                   0x0000E948
#define DSP_RX_CONTROL_RXBUF_164_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_164_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_164_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_164_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_165_OFFSET                                   0x0000E94A
#define DSP_RX_CONTROL_RXBUF_165_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_165_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_165_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_165_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_166_OFFSET                                   0x0000E94C
#define DSP_RX_CONTROL_RXBUF_166_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_166_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_166_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_166_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_167_OFFSET                                   0x0000E94E
#define DSP_RX_CONTROL_RXBUF_167_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_167_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_167_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_167_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_168_OFFSET                                   0x0000E950
#define DSP_RX_CONTROL_RXBUF_168_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_168_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_168_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_168_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_169_OFFSET                                   0x0000E952
#define DSP_RX_CONTROL_RXBUF_169_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_169_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_169_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_169_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_170_OFFSET                                   0x0000E954
#define DSP_RX_CONTROL_RXBUF_170_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_170_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_170_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_170_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_171_OFFSET                                   0x0000E956
#define DSP_RX_CONTROL_RXBUF_171_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_171_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_171_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_171_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_172_OFFSET                                   0x0000E958
#define DSP_RX_CONTROL_RXBUF_172_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_172_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_172_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_172_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_173_OFFSET                                   0x0000E95A
#define DSP_RX_CONTROL_RXBUF_173_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_173_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_173_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_173_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_174_OFFSET                                   0x0000E95C
#define DSP_RX_CONTROL_RXBUF_174_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_174_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_174_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_174_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_175_OFFSET                                   0x0000E95E
#define DSP_RX_CONTROL_RXBUF_175_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_175_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_175_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_175_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_176_OFFSET                                   0x0000E960
#define DSP_RX_CONTROL_RXBUF_176_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_176_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_176_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_176_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_177_OFFSET                                   0x0000E962
#define DSP_RX_CONTROL_RXBUF_177_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_177_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_177_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_177_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_178_OFFSET                                   0x0000E964
#define DSP_RX_CONTROL_RXBUF_178_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_178_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_178_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_178_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_179_OFFSET                                   0x0000E966
#define DSP_RX_CONTROL_RXBUF_179_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_179_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_179_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_179_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_180_OFFSET                                   0x0000E968
#define DSP_RX_CONTROL_RXBUF_180_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_180_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_180_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_180_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_181_OFFSET                                   0x0000E96A
#define DSP_RX_CONTROL_RXBUF_181_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_181_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_181_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_181_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_182_OFFSET                                   0x0000E96C
#define DSP_RX_CONTROL_RXBUF_182_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_182_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_182_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_182_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_183_OFFSET                                   0x0000E96E
#define DSP_RX_CONTROL_RXBUF_183_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_183_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_183_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_183_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_184_OFFSET                                   0x0000E970
#define DSP_RX_CONTROL_RXBUF_184_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_184_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_184_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_184_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_185_OFFSET                                   0x0000E972
#define DSP_RX_CONTROL_RXBUF_185_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_185_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_185_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_185_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_186_OFFSET                                   0x0000E974
#define DSP_RX_CONTROL_RXBUF_186_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_186_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_186_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_186_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_187_OFFSET                                   0x0000E976
#define DSP_RX_CONTROL_RXBUF_187_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_187_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_187_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_187_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_188_OFFSET                                   0x0000E978
#define DSP_RX_CONTROL_RXBUF_188_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_188_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_188_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_188_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_189_OFFSET                                   0x0000E97A
#define DSP_RX_CONTROL_RXBUF_189_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_189_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_189_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_189_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_190_OFFSET                                   0x0000E97C
#define DSP_RX_CONTROL_RXBUF_190_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_190_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_190_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_190_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_191_OFFSET                                   0x0000E97E
#define DSP_RX_CONTROL_RXBUF_191_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_191_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_191_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_191_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_192_OFFSET                                   0x0000E980
#define DSP_RX_CONTROL_RXBUF_192_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_192_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_192_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_192_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_193_OFFSET                                   0x0000E982
#define DSP_RX_CONTROL_RXBUF_193_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_193_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_193_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_193_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_194_OFFSET                                   0x0000E984
#define DSP_RX_CONTROL_RXBUF_194_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_194_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_194_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_194_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_195_OFFSET                                   0x0000E986
#define DSP_RX_CONTROL_RXBUF_195_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_195_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_195_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_195_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_196_OFFSET                                   0x0000E988
#define DSP_RX_CONTROL_RXBUF_196_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_196_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_196_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_196_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_197_OFFSET                                   0x0000E98A
#define DSP_RX_CONTROL_RXBUF_197_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_197_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_197_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_197_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_198_OFFSET                                   0x0000E98C
#define DSP_RX_CONTROL_RXBUF_198_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_198_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_198_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_198_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_199_OFFSET                                   0x0000E98E
#define DSP_RX_CONTROL_RXBUF_199_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_199_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_199_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_199_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_200_OFFSET                                   0x0000E990
#define DSP_RX_CONTROL_RXBUF_200_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_200_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_200_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_200_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_201_OFFSET                                   0x0000E992
#define DSP_RX_CONTROL_RXBUF_201_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_201_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_201_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_201_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_202_OFFSET                                   0x0000E994
#define DSP_RX_CONTROL_RXBUF_202_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_202_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_202_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_202_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_203_OFFSET                                   0x0000E996
#define DSP_RX_CONTROL_RXBUF_203_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_203_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_203_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_203_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_204_OFFSET                                   0x0000E998
#define DSP_RX_CONTROL_RXBUF_204_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_204_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_204_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_204_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_205_OFFSET                                   0x0000E99A
#define DSP_RX_CONTROL_RXBUF_205_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_205_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_205_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_205_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_206_OFFSET                                   0x0000E99C
#define DSP_RX_CONTROL_RXBUF_206_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_206_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_206_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_206_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_207_OFFSET                                   0x0000E99E
#define DSP_RX_CONTROL_RXBUF_207_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_207_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_207_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_207_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_208_OFFSET                                   0x0000E9A0
#define DSP_RX_CONTROL_RXBUF_208_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_208_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_208_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_208_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_209_OFFSET                                   0x0000E9A2
#define DSP_RX_CONTROL_RXBUF_209_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_209_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_209_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_209_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_210_OFFSET                                   0x0000E9A4
#define DSP_RX_CONTROL_RXBUF_210_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_210_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_210_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_210_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_211_OFFSET                                   0x0000E9A6
#define DSP_RX_CONTROL_RXBUF_211_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_211_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_211_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_211_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_212_OFFSET                                   0x0000E9A8
#define DSP_RX_CONTROL_RXBUF_212_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_212_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_212_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_212_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_213_OFFSET                                   0x0000E9AA
#define DSP_RX_CONTROL_RXBUF_213_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_213_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_213_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_213_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_214_OFFSET                                   0x0000E9AC
#define DSP_RX_CONTROL_RXBUF_214_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_214_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_214_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_214_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_215_OFFSET                                   0x0000E9AE
#define DSP_RX_CONTROL_RXBUF_215_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_215_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_215_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_215_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_216_OFFSET                                   0x0000E9B0
#define DSP_RX_CONTROL_RXBUF_216_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_216_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_216_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_216_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_217_OFFSET                                   0x0000E9B2
#define DSP_RX_CONTROL_RXBUF_217_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_217_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_217_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_217_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_218_OFFSET                                   0x0000E9B4
#define DSP_RX_CONTROL_RXBUF_218_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_218_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_218_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_218_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_219_OFFSET                                   0x0000E9B6
#define DSP_RX_CONTROL_RXBUF_219_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_219_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_219_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_219_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_220_OFFSET                                   0x0000E9B8
#define DSP_RX_CONTROL_RXBUF_220_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_220_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_220_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_220_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_221_OFFSET                                   0x0000E9BA
#define DSP_RX_CONTROL_RXBUF_221_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_221_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_221_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_221_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_222_OFFSET                                   0x0000E9BC
#define DSP_RX_CONTROL_RXBUF_222_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_222_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_222_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_222_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_223_OFFSET                                   0x0000E9BE
#define DSP_RX_CONTROL_RXBUF_223_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_223_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_223_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_223_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_224_OFFSET                                   0x0000E9C0
#define DSP_RX_CONTROL_RXBUF_224_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_224_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_224_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_224_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_225_OFFSET                                   0x0000E9C2
#define DSP_RX_CONTROL_RXBUF_225_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_225_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_225_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_225_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_226_OFFSET                                   0x0000E9C4
#define DSP_RX_CONTROL_RXBUF_226_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_226_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_226_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_226_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_227_OFFSET                                   0x0000E9C6
#define DSP_RX_CONTROL_RXBUF_227_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_227_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_227_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_227_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_228_OFFSET                                   0x0000E9C8
#define DSP_RX_CONTROL_RXBUF_228_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_228_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_228_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_228_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_229_OFFSET                                   0x0000E9CA
#define DSP_RX_CONTROL_RXBUF_229_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_229_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_229_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_229_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_230_OFFSET                                   0x0000E9CC
#define DSP_RX_CONTROL_RXBUF_230_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_230_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_230_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_230_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_231_OFFSET                                   0x0000E9CE
#define DSP_RX_CONTROL_RXBUF_231_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_231_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_231_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_231_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_232_OFFSET                                   0x0000E9D0
#define DSP_RX_CONTROL_RXBUF_232_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_232_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_232_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_232_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_233_OFFSET                                   0x0000E9D2
#define DSP_RX_CONTROL_RXBUF_233_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_233_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_233_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_233_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_234_OFFSET                                   0x0000E9D4
#define DSP_RX_CONTROL_RXBUF_234_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_234_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_234_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_234_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_235_OFFSET                                   0x0000E9D6
#define DSP_RX_CONTROL_RXBUF_235_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_235_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_235_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_235_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_236_OFFSET                                   0x0000E9D8
#define DSP_RX_CONTROL_RXBUF_236_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_236_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_236_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_236_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_237_OFFSET                                   0x0000E9DA
#define DSP_RX_CONTROL_RXBUF_237_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_237_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_237_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_237_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_238_OFFSET                                   0x0000E9DC
#define DSP_RX_CONTROL_RXBUF_238_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_238_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_238_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_238_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_239_OFFSET                                   0x0000E9DE
#define DSP_RX_CONTROL_RXBUF_239_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_239_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_239_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_239_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_240_OFFSET                                   0x0000E9E0
#define DSP_RX_CONTROL_RXBUF_240_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_240_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_240_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_240_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_241_OFFSET                                   0x0000E9E2
#define DSP_RX_CONTROL_RXBUF_241_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_241_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_241_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_241_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_242_OFFSET                                   0x0000E9E4
#define DSP_RX_CONTROL_RXBUF_242_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_242_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_242_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_242_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_243_OFFSET                                   0x0000E9E6
#define DSP_RX_CONTROL_RXBUF_243_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_243_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_243_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_243_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_244_OFFSET                                   0x0000E9E8
#define DSP_RX_CONTROL_RXBUF_244_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_244_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_244_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_244_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_245_OFFSET                                   0x0000E9EA
#define DSP_RX_CONTROL_RXBUF_245_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_245_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_245_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_245_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_246_OFFSET                                   0x0000E9EC
#define DSP_RX_CONTROL_RXBUF_246_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_246_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_246_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_246_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_247_OFFSET                                   0x0000E9EE
#define DSP_RX_CONTROL_RXBUF_247_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_247_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_247_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_247_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_248_OFFSET                                   0x0000E9F0
#define DSP_RX_CONTROL_RXBUF_248_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_248_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_248_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_248_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_249_OFFSET                                   0x0000E9F2
#define DSP_RX_CONTROL_RXBUF_249_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_249_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_249_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_249_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_250_OFFSET                                   0x0000E9F4
#define DSP_RX_CONTROL_RXBUF_250_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_250_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_250_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_250_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_251_OFFSET                                   0x0000E9F6
#define DSP_RX_CONTROL_RXBUF_251_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_251_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_251_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_251_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_252_OFFSET                                   0x0000E9F8
#define DSP_RX_CONTROL_RXBUF_252_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_252_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_252_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_252_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_253_OFFSET                                   0x0000E9FA
#define DSP_RX_CONTROL_RXBUF_253_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_253_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_253_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_253_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_254_OFFSET                                   0x0000E9FC
#define DSP_RX_CONTROL_RXBUF_254_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_254_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_254_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_254_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_255_OFFSET                                   0x0000E9FE
#define DSP_RX_CONTROL_RXBUF_255_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_255_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_255_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_255_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_256_OFFSET                                   0x0000EA00
#define DSP_RX_CONTROL_RXBUF_256_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_256_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_256_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_256_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_257_OFFSET                                   0x0000EA02
#define DSP_RX_CONTROL_RXBUF_257_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_257_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_257_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_257_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_258_OFFSET                                   0x0000EA04
#define DSP_RX_CONTROL_RXBUF_258_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_258_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_258_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_258_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_259_OFFSET                                   0x0000EA06
#define DSP_RX_CONTROL_RXBUF_259_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_259_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_259_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_259_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_260_OFFSET                                   0x0000EA08
#define DSP_RX_CONTROL_RXBUF_260_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_260_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_260_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_260_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_261_OFFSET                                   0x0000EA0A
#define DSP_RX_CONTROL_RXBUF_261_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_261_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_261_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_261_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_262_OFFSET                                   0x0000EA0C
#define DSP_RX_CONTROL_RXBUF_262_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_262_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_262_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_262_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_263_OFFSET                                   0x0000EA0E
#define DSP_RX_CONTROL_RXBUF_263_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_263_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_263_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_263_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_264_OFFSET                                   0x0000EA10
#define DSP_RX_CONTROL_RXBUF_264_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_264_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_264_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_264_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_265_OFFSET                                   0x0000EA12
#define DSP_RX_CONTROL_RXBUF_265_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_265_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_265_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_265_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_266_OFFSET                                   0x0000EA14
#define DSP_RX_CONTROL_RXBUF_266_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_266_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_266_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_266_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_267_OFFSET                                   0x0000EA16
#define DSP_RX_CONTROL_RXBUF_267_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_267_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_267_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_267_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_268_OFFSET                                   0x0000EA18
#define DSP_RX_CONTROL_RXBUF_268_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_268_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_268_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_268_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_269_OFFSET                                   0x0000EA1A
#define DSP_RX_CONTROL_RXBUF_269_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_269_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_269_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_269_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_270_OFFSET                                   0x0000EA1C
#define DSP_RX_CONTROL_RXBUF_270_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_270_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_270_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_270_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_271_OFFSET                                   0x0000EA1E
#define DSP_RX_CONTROL_RXBUF_271_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_271_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_271_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_271_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_272_OFFSET                                   0x0000EA20
#define DSP_RX_CONTROL_RXBUF_272_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_272_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_272_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_272_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_273_OFFSET                                   0x0000EA22
#define DSP_RX_CONTROL_RXBUF_273_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_273_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_273_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_273_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_274_OFFSET                                   0x0000EA24
#define DSP_RX_CONTROL_RXBUF_274_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_274_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_274_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_274_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_275_OFFSET                                   0x0000EA26
#define DSP_RX_CONTROL_RXBUF_275_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_275_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_275_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_275_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_276_OFFSET                                   0x0000EA28
#define DSP_RX_CONTROL_RXBUF_276_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_276_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_276_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_276_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_277_OFFSET                                   0x0000EA2A
#define DSP_RX_CONTROL_RXBUF_277_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_277_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_277_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_277_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_278_OFFSET                                   0x0000EA2C
#define DSP_RX_CONTROL_RXBUF_278_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_278_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_278_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_278_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_279_OFFSET                                   0x0000EA2E
#define DSP_RX_CONTROL_RXBUF_279_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_279_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_279_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_279_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_280_OFFSET                                   0x0000EA30
#define DSP_RX_CONTROL_RXBUF_280_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_280_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_280_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_280_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_281_OFFSET                                   0x0000EA32
#define DSP_RX_CONTROL_RXBUF_281_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_281_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_281_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_281_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_282_OFFSET                                   0x0000EA34
#define DSP_RX_CONTROL_RXBUF_282_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_282_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_282_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_282_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_283_OFFSET                                   0x0000EA36
#define DSP_RX_CONTROL_RXBUF_283_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_283_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_283_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_283_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_284_OFFSET                                   0x0000EA38
#define DSP_RX_CONTROL_RXBUF_284_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_284_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_284_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_284_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_285_OFFSET                                   0x0000EA3A
#define DSP_RX_CONTROL_RXBUF_285_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_285_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_285_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_285_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_286_OFFSET                                   0x0000EA3C
#define DSP_RX_CONTROL_RXBUF_286_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_286_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_286_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_286_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_287_OFFSET                                   0x0000EA3E
#define DSP_RX_CONTROL_RXBUF_287_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_287_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_287_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_287_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_288_OFFSET                                   0x0000EA40
#define DSP_RX_CONTROL_RXBUF_288_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_288_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_288_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_288_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_289_OFFSET                                   0x0000EA42
#define DSP_RX_CONTROL_RXBUF_289_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_289_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_289_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_289_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_290_OFFSET                                   0x0000EA44
#define DSP_RX_CONTROL_RXBUF_290_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_290_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_290_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_290_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_291_OFFSET                                   0x0000EA46
#define DSP_RX_CONTROL_RXBUF_291_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_291_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_291_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_291_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_292_OFFSET                                   0x0000EA48
#define DSP_RX_CONTROL_RXBUF_292_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_292_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_292_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_292_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_293_OFFSET                                   0x0000EA4A
#define DSP_RX_CONTROL_RXBUF_293_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_293_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_293_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_293_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_294_OFFSET                                   0x0000EA4C
#define DSP_RX_CONTROL_RXBUF_294_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_294_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_294_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_294_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_295_OFFSET                                   0x0000EA4E
#define DSP_RX_CONTROL_RXBUF_295_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_295_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_295_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_295_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_296_OFFSET                                   0x0000EA50
#define DSP_RX_CONTROL_RXBUF_296_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_296_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_296_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_296_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_297_OFFSET                                   0x0000EA52
#define DSP_RX_CONTROL_RXBUF_297_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_297_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_297_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_297_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_298_OFFSET                                   0x0000EA54
#define DSP_RX_CONTROL_RXBUF_298_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_298_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_298_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_298_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_299_OFFSET                                   0x0000EA56
#define DSP_RX_CONTROL_RXBUF_299_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_299_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_299_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_299_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_300_OFFSET                                   0x0000EA58
#define DSP_RX_CONTROL_RXBUF_300_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_300_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_300_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_300_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_301_OFFSET                                   0x0000EA5A
#define DSP_RX_CONTROL_RXBUF_301_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_301_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_301_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_301_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_302_OFFSET                                   0x0000EA5C
#define DSP_RX_CONTROL_RXBUF_302_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_302_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_302_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_302_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_303_OFFSET                                   0x0000EA5E
#define DSP_RX_CONTROL_RXBUF_303_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_303_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_303_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_303_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_304_OFFSET                                   0x0000EA60
#define DSP_RX_CONTROL_RXBUF_304_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_304_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_304_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_304_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_305_OFFSET                                   0x0000EA62
#define DSP_RX_CONTROL_RXBUF_305_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_305_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_305_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_305_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_306_OFFSET                                   0x0000EA64
#define DSP_RX_CONTROL_RXBUF_306_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_306_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_306_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_306_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_307_OFFSET                                   0x0000EA66
#define DSP_RX_CONTROL_RXBUF_307_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_307_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_307_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_307_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_308_OFFSET                                   0x0000EA68
#define DSP_RX_CONTROL_RXBUF_308_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_308_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_308_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_308_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_309_OFFSET                                   0x0000EA6A
#define DSP_RX_CONTROL_RXBUF_309_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_309_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_309_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_309_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_310_OFFSET                                   0x0000EA6C
#define DSP_RX_CONTROL_RXBUF_310_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_310_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_310_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_310_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXBUF_311_OFFSET                                   0x0000EA6E
#define DSP_RX_CONTROL_RXBUF_311_TYPE                                     UInt32
#define DSP_RX_CONTROL_RXBUF_311_RESERVED_MASK                            0x00000000
#define    DSP_RX_CONTROL_RXBUF_311_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXBUF_311_DATA_MASK                             0xFFFFFFFF

#define DSP_RX_CONTROL_RXCR_R_OFFSET                                      0x00000B66
#define DSP_RX_CONTROL_RXCR_R_TYPE                                        UInt16
#define DSP_RX_CONTROL_RXCR_R_RESERVED_MASK                               0x0000C000
#define    DSP_RX_CONTROL_RXCR_R_SPI_RX_INDEX_EN_SHIFT                    13
#define    DSP_RX_CONTROL_RXCR_R_SPI_RX_INDEX_EN_MASK                     0x00002000
#define    DSP_RX_CONTROL_RXCR_R_GPEN_RX_INDEX_EN_SHIFT                   12
#define    DSP_RX_CONTROL_RXCR_R_GPEN_RX_INDEX_EN_MASK                    0x00001000
#define    DSP_RX_CONTROL_RXCR_R_MON_EN_SHIFT                             11
#define    DSP_RX_CONTROL_RXCR_R_MON_EN_MASK                              0x00000800
#define    DSP_RX_CONTROL_RXCR_R_BYPASSFILT_SHIFT                         10
#define    DSP_RX_CONTROL_RXCR_R_BYPASSFILT_MASK                          0x00000400
#define    DSP_RX_CONTROL_RXCR_R_CLASS33_SHIFT                            9
#define    DSP_RX_CONTROL_RXCR_R_CLASS33_MASK                             0x00000200
#define    DSP_RX_CONTROL_RXCR_R_RXSTAT_SHIFT                             8
#define    DSP_RX_CONTROL_RXCR_R_RXSTAT_MASK                              0x00000100
#define    DSP_RX_CONTROL_RXCR_R_RCON_SHIFT                               5
#define    DSP_RX_CONTROL_RXCR_R_RCON_MASK                                0x000000E0
#define    DSP_RX_CONTROL_RXCR_R_STOPRX_SHIFT                             4
#define    DSP_RX_CONTROL_RXCR_R_STOPRX_MASK                              0x00000010
#define    DSP_RX_CONTROL_RXCR_R_STARRX_SHIFT                             3
#define    DSP_RX_CONTROL_RXCR_R_STARRX_MASK                              0x00000008
#define    DSP_RX_CONTROL_RXCR_R_RBC_SHIFT                                0
#define    DSP_RX_CONTROL_RXCR_R_RBC_MASK                                 0x00000007

#define DSP_RX_CONTROL_RXSAMP_R_OFFSET                                    0x00000B68
#define DSP_RX_CONTROL_RXSAMP_R_TYPE                                      UInt16
#define DSP_RX_CONTROL_RXSAMP_R_RESERVED_MASK                             0x0000FF00
#define    DSP_RX_CONTROL_RXSAMP_R_RXNUM_SHIFT                            0
#define    DSP_RX_CONTROL_RXSAMP_R_RXNUM_MASK                             0x000000FF

#define DSP_RX_CONTROL_RCOR_R_OFFSET                                      0x00000B80
#define DSP_RX_CONTROL_RCOR_R_TYPE                                        UInt16
#define DSP_RX_CONTROL_RCOR_R_RESERVED_MASK                               0x00000000
#define    DSP_RX_CONTROL_RCOR_R_RCQ_SHIFT                                8
#define    DSP_RX_CONTROL_RCOR_R_RCQ_MASK                                 0x0000FF00
#define    DSP_RX_CONTROL_RCOR_R_RCI_SHIFT                                0
#define    DSP_RX_CONTROL_RCOR_R_RCI_MASK                                 0x000000FF

#define DSP_RX_CONTROL_RCDR_R_OFFSET                                      0x00000B82
#define DSP_RX_CONTROL_RCDR_R_TYPE                                        UInt16
#define DSP_RX_CONTROL_RCDR_R_RESERVED_MASK                               0x00000C00
#define    DSP_RX_CONTROL_RCDR_R_CSHORT_SHIFT                             15
#define    DSP_RX_CONTROL_RCDR_R_CSHORT_MASK                              0x00008000
#define    DSP_RX_CONTROL_RCDR_R_RXCT_SHIFT                               12
#define    DSP_RX_CONTROL_RCDR_R_RXCT_MASK                                0x00007000
#define    DSP_RX_CONTROL_RCDR_R_DEL_SHIFT                                0
#define    DSP_RX_CONTROL_RCDR_R_DEL_MASK                                 0x000003FF

#define DSP_RX_CONTROL_RSDR_R_OFFSET                                      0x00000B84
#define DSP_RX_CONTROL_RSDR_R_TYPE                                        UInt16
#define DSP_RX_CONTROL_RSDR_R_RESERVED_MASK                               0x0000FC00
#define    DSP_RX_CONTROL_RSDR_R_DEL_SHIFT                                0
#define    DSP_RX_CONTROL_RSDR_R_DEL_MASK                                 0x000003FF

#define DSP_RX_CONTROL_RSDR_MON_R_OFFSET                                  0x00000B86
#define DSP_RX_CONTROL_RSDR_MON_R_TYPE                                    UInt16
#define DSP_RX_CONTROL_RSDR_MON_R_RESERVED_MASK                           0x0000FC00
#define    DSP_RX_CONTROL_RSDR_MON_R_DEL_SHIFT                            0
#define    DSP_RX_CONTROL_RSDR_MON_R_DEL_MASK                             0x000003FF

#define DSP_RX_CONTROL_RFRCOR_R_OFFSET                                    0x00000B88
#define DSP_RX_CONTROL_RFRCOR_R_TYPE                                      UInt16
#define DSP_RX_CONTROL_RFRCOR_R_RESERVED_MASK                             0x00000000
#define    DSP_RX_CONTROL_RFRCOR_R_RFRCQ_SHIFT                            8
#define    DSP_RX_CONTROL_RFRCOR_R_RFRCQ_MASK                             0x0000FF00
#define    DSP_RX_CONTROL_RFRCOR_R_RFRCI_SHIFT                            0
#define    DSP_RX_CONTROL_RFRCOR_R_RFRCI_MASK                             0x000000FF

#define DSP_RX_CONTROL_REC4ADR_R_OFFSET                                   0x00000B90
#define DSP_RX_CONTROL_REC4ADR_R_TYPE                                     UInt16
#define DSP_RX_CONTROL_REC4ADR_R_RESERVED_MASK                            0x0000FFF0
#define    DSP_RX_CONTROL_REC4ADR_R_ADDRESS_SHIFT                         0
#define    DSP_RX_CONTROL_REC4ADR_R_ADDRESS_MASK                          0x0000000F

#define DSP_RX_CONTROL_REC4DAT_R_OFFSET                                   0x00000B92
#define DSP_RX_CONTROL_REC4DAT_R_TYPE                                     UInt16
#define DSP_RX_CONTROL_REC4DAT_R_RESERVED_MASK                            0x0000E000
#define    DSP_RX_CONTROL_REC4DAT_R_DATA13BIT_SHIFT                       0
#define    DSP_RX_CONTROL_REC4DAT_R_DATA13BIT_MASK                        0x00001FFF

#define DSP_RX_CONTROL_RXMATCH_R_OFFSET                                   0x00000B98
#define DSP_RX_CONTROL_RXMATCH_R_TYPE                                     UInt16
#define DSP_RX_CONTROL_RXMATCH_R_RESERVED_MASK                            0x0000FFC0
#define    DSP_RX_CONTROL_RXMATCH_R_DATA_SHIFT                            0
#define    DSP_RX_CONTROL_RXMATCH_R_DATA_MASK                             0x0000003F

#define DSP_RX_CONTROL_RXBUF_R0TO619_OFFSET                               0x00001000
#define DSP_RX_CONTROL_RXBUF_R0TO619_TYPE                                 UInt32
#define DSP_RX_CONTROL_RXBUF_R0TO619_RESERVED_MASK                        0x00000000
#define    DSP_RX_CONTROL_RXBUF_R0TO619_DATA_SHIFT                        0
#define    DSP_RX_CONTROL_RXBUF_R0TO619_DATA_MASK                         0xFFFFFFFF

#endif /* __BRCM_RDB_DSP_RX_CONTROL_H__ */



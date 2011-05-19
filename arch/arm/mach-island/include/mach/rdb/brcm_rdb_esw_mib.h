/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2011  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_ESW_MIB_H__
#define __BRCM_RDB_ESW_MIB_H__

#define ESW_MIB_TXOCTETS_OFFSET                                           0x00000000
#define ESW_MIB_TXOCTETS_TYPE                                             UInt64
#define ESW_MIB_TXOCTETS_RESERVED_MASK                                    0x00000000
#define    ESW_MIB_TXOCTETS_TXOCTETS_SHIFT                                0
#define    ESW_MIB_TXOCTETS_TXOCTETS_MASK                                 0xFFFFFFFFFFFFFFFF

#define ESW_MIB_TXDROPPKTS_OFFSET                                         0x00000040
#define ESW_MIB_TXDROPPKTS_TYPE                                           UInt32
#define ESW_MIB_TXDROPPKTS_RESERVED_MASK                                  0x00000000
#define    ESW_MIB_TXDROPPKTS_TXDROPPKTS_SHIFT                            0
#define    ESW_MIB_TXDROPPKTS_TXDROPPKTS_MASK                             0xFFFFFFFF

#define ESW_MIB_TXQ0PKT_OFFSET                                            0x00000060
#define ESW_MIB_TXQ0PKT_TYPE                                              UInt32
#define ESW_MIB_TXQ0PKT_RESERVED_MASK                                     0x00000000
#define    ESW_MIB_TXQ0PKT_TXQ0PKT_SHIFT                                  0
#define    ESW_MIB_TXQ0PKT_TXQ0PKT_MASK                                   0xFFFFFFFF

#define ESW_MIB_TXBROADCASTPKTS_OFFSET                                    0x00000080
#define ESW_MIB_TXBROADCASTPKTS_TYPE                                      UInt32
#define ESW_MIB_TXBROADCASTPKTS_RESERVED_MASK                             0x00000000
#define    ESW_MIB_TXBROADCASTPKTS_TXBROADCASTPKTS_SHIFT                  0
#define    ESW_MIB_TXBROADCASTPKTS_TXBROADCASTPKTS_MASK                   0xFFFFFFFF

#define ESW_MIB_TXMULTICASTPKTS_OFFSET                                    0x000000A0
#define ESW_MIB_TXMULTICASTPKTS_TYPE                                      UInt32
#define ESW_MIB_TXMULTICASTPKTS_RESERVED_MASK                             0x00000000
#define    ESW_MIB_TXMULTICASTPKTS_TXMULTICASTPKTS_SHIFT                  0
#define    ESW_MIB_TXMULTICASTPKTS_TXMULTICASTPKTS_MASK                   0xFFFFFFFF

#define ESW_MIB_TXUNICASTPKTS_OFFSET                                      0x000000C0
#define ESW_MIB_TXUNICASTPKTS_TYPE                                        UInt32
#define ESW_MIB_TXUNICASTPKTS_RESERVED_MASK                               0x00000000
#define    ESW_MIB_TXUNICASTPKTS_TXUNICASTPKTS_SHIFT                      0
#define    ESW_MIB_TXUNICASTPKTS_TXUNICASTPKTS_MASK                       0xFFFFFFFF

#define ESW_MIB_TXCOLLISIONS_OFFSET                                       0x000000E0
#define ESW_MIB_TXCOLLISIONS_TYPE                                         UInt32
#define ESW_MIB_TXCOLLISIONS_RESERVED_MASK                                0x00000000
#define    ESW_MIB_TXCOLLISIONS_TXCOLLISIONS_SHIFT                        0
#define    ESW_MIB_TXCOLLISIONS_TXCOLLISIONS_MASK                         0xFFFFFFFF

#define ESW_MIB_TXSINGLECOLLISION_OFFSET                                  0x00000100
#define ESW_MIB_TXSINGLECOLLISION_TYPE                                    UInt32
#define ESW_MIB_TXSINGLECOLLISION_RESERVED_MASK                           0x00000000
#define    ESW_MIB_TXSINGLECOLLISION_TXSINGLECOLLISION_SHIFT              0
#define    ESW_MIB_TXSINGLECOLLISION_TXSINGLECOLLISION_MASK               0xFFFFFFFF

#define ESW_MIB_TXMULTIPLE_COLLISION_OFFSET                               0x00000120
#define ESW_MIB_TXMULTIPLE_COLLISION_TYPE                                 UInt32
#define ESW_MIB_TXMULTIPLE_COLLISION_RESERVED_MASK                        0x00000000
#define    ESW_MIB_TXMULTIPLE_COLLISION_TXMULTIPLE_COLLISION_SHIFT        0
#define    ESW_MIB_TXMULTIPLE_COLLISION_TXMULTIPLE_COLLISION_MASK         0xFFFFFFFF

#define ESW_MIB_TXDEFERREDTRANSMIT_OFFSET                                 0x00000140
#define ESW_MIB_TXDEFERREDTRANSMIT_TYPE                                   UInt32
#define ESW_MIB_TXDEFERREDTRANSMIT_RESERVED_MASK                          0x00000000
#define    ESW_MIB_TXDEFERREDTRANSMIT_TXDEFERREDTRANSMIT_SHIFT            0
#define    ESW_MIB_TXDEFERREDTRANSMIT_TXDEFERREDTRANSMIT_MASK             0xFFFFFFFF

#define ESW_MIB_TXLATECOLLISION_OFFSET                                    0x00000160
#define ESW_MIB_TXLATECOLLISION_TYPE                                      UInt32
#define ESW_MIB_TXLATECOLLISION_RESERVED_MASK                             0x00000000
#define    ESW_MIB_TXLATECOLLISION_TXLATECOLLISION_SHIFT                  0
#define    ESW_MIB_TXLATECOLLISION_TXLATECOLLISION_MASK                   0xFFFFFFFF

#define ESW_MIB_TXEXCESSIVECOLLISION_OFFSET                               0x00000180
#define ESW_MIB_TXEXCESSIVECOLLISION_TYPE                                 UInt32
#define ESW_MIB_TXEXCESSIVECOLLISION_RESERVED_MASK                        0x00000000
#define    ESW_MIB_TXEXCESSIVECOLLISION_TXEXCESSIVECOLLISION_SHIFT        0
#define    ESW_MIB_TXEXCESSIVECOLLISION_TXEXCESSIVECOLLISION_MASK         0xFFFFFFFF

#define ESW_MIB_TXFRAMEINDISC_OFFSET                                      0x000001A0
#define ESW_MIB_TXFRAMEINDISC_TYPE                                        UInt32
#define ESW_MIB_TXFRAMEINDISC_RESERVED_MASK                               0x00000000
#define    ESW_MIB_TXFRAMEINDISC_TXFRAMEINDISC_SHIFT                      0
#define    ESW_MIB_TXFRAMEINDISC_TXFRAMEINDISC_MASK                       0xFFFFFFFF

#define ESW_MIB_TXPAUSEPKTS_OFFSET                                        0x000001C0
#define ESW_MIB_TXPAUSEPKTS_TYPE                                          UInt32
#define ESW_MIB_TXPAUSEPKTS_RESERVED_MASK                                 0x00000000
#define    ESW_MIB_TXPAUSEPKTS_TXPAUSEPKTS_SHIFT                          0
#define    ESW_MIB_TXPAUSEPKTS_TXPAUSEPKTS_MASK                           0xFFFFFFFF

#define ESW_MIB_TXQ1PKT_OFFSET                                            0x000001E0
#define ESW_MIB_TXQ1PKT_TYPE                                              UInt32
#define ESW_MIB_TXQ1PKT_RESERVED_MASK                                     0x00000000
#define    ESW_MIB_TXQ1PKT_TXQ1PKT_SHIFT                                  0
#define    ESW_MIB_TXQ1PKT_TXQ1PKT_MASK                                   0xFFFFFFFF

#define ESW_MIB_TXQ2PKT_OFFSET                                            0x00000200
#define ESW_MIB_TXQ2PKT_TYPE                                              UInt32
#define ESW_MIB_TXQ2PKT_RESERVED_MASK                                     0x00000000
#define    ESW_MIB_TXQ2PKT_TXQ2PKT_SHIFT                                  0
#define    ESW_MIB_TXQ2PKT_TXQ2PKT_MASK                                   0xFFFFFFFF

#define ESW_MIB_TXQ3PKT_OFFSET                                            0x00000220
#define ESW_MIB_TXQ3PKT_TYPE                                              UInt32
#define ESW_MIB_TXQ3PKT_RESERVED_MASK                                     0x00000000
#define    ESW_MIB_TXQ3PKT_TXQ3PKT_SHIFT                                  0
#define    ESW_MIB_TXQ3PKT_TXQ3PKT_MASK                                   0xFFFFFFFF

#define ESW_MIB_TXQ4PKT_OFFSET                                            0x00000240
#define ESW_MIB_TXQ4PKT_TYPE                                              UInt32
#define ESW_MIB_TXQ4PKT_RESERVED_MASK                                     0x00000000
#define    ESW_MIB_TXQ4PKT_TXQ4PKT_SHIFT                                  0
#define    ESW_MIB_TXQ4PKT_TXQ4PKT_MASK                                   0xFFFFFFFF

#define ESW_MIB_TXQ5PKT_OFFSET                                            0x00000260
#define ESW_MIB_TXQ5PKT_TYPE                                              UInt32
#define ESW_MIB_TXQ5PKT_RESERVED_MASK                                     0x00000000
#define    ESW_MIB_TXQ5PKT_TXQ5PKT_SHIFT                                  0
#define    ESW_MIB_TXQ5PKT_TXQ5PKT_MASK                                   0xFFFFFFFF

#define ESW_MIB_RXOCTETS_OFFSET                                           0x00000280
#define ESW_MIB_RXOCTETS_TYPE                                             UInt64
#define ESW_MIB_RXOCTETS_RESERVED_MASK                                    0x00000000
#define    ESW_MIB_RXOCTETS_RXOCTETS_SHIFT                                0
#define    ESW_MIB_RXOCTETS_RXOCTETS_MASK                                 0xFFFFFFFFFFFFFFFF

#define ESW_MIB_RXUNDERSIZEPKTS_OFFSET                                    0x000002C0
#define ESW_MIB_RXUNDERSIZEPKTS_TYPE                                      UInt32
#define ESW_MIB_RXUNDERSIZEPKTS_RESERVED_MASK                             0x00000000
#define    ESW_MIB_RXUNDERSIZEPKTS_RXUNDERSIZEPKTS_SHIFT                  0
#define    ESW_MIB_RXUNDERSIZEPKTS_RXUNDERSIZEPKTS_MASK                   0xFFFFFFFF

#define ESW_MIB_RXPAUSEPKTS_OFFSET                                        0x000002E0
#define ESW_MIB_RXPAUSEPKTS_TYPE                                          UInt32
#define ESW_MIB_RXPAUSEPKTS_RESERVED_MASK                                 0x00000000
#define    ESW_MIB_RXPAUSEPKTS_RXPAUSEPKTS_SHIFT                          0
#define    ESW_MIB_RXPAUSEPKTS_RXPAUSEPKTS_MASK                           0xFFFFFFFF

#define ESW_MIB_PKTS64OCTETS_OFFSET                                       0x00000300
#define ESW_MIB_PKTS64OCTETS_TYPE                                         UInt32
#define ESW_MIB_PKTS64OCTETS_RESERVED_MASK                                0x00000000
#define    ESW_MIB_PKTS64OCTETS_PKTS64OCTETS_SHIFT                        0
#define    ESW_MIB_PKTS64OCTETS_PKTS64OCTETS_MASK                         0xFFFFFFFF

#define ESW_MIB_PKTS65TO127OCTETS_OFFSET                                  0x00000320
#define ESW_MIB_PKTS65TO127OCTETS_TYPE                                    UInt32
#define ESW_MIB_PKTS65TO127OCTETS_RESERVED_MASK                           0x00000000
#define    ESW_MIB_PKTS65TO127OCTETS_PKTS65TO127OCTETS_SHIFT              0
#define    ESW_MIB_PKTS65TO127OCTETS_PKTS65TO127OCTETS_MASK               0xFFFFFFFF

#define ESW_MIB_PKTS128TO255OCTETS_OFFSET                                 0x00000340
#define ESW_MIB_PKTS128TO255OCTETS_TYPE                                   UInt32
#define ESW_MIB_PKTS128TO255OCTETS_RESERVED_MASK                          0x00000000
#define    ESW_MIB_PKTS128TO255OCTETS_PKTS128TO255OCTETS_SHIFT            0
#define    ESW_MIB_PKTS128TO255OCTETS_PKTS128TO255OCTETS_MASK             0xFFFFFFFF

#define ESW_MIB_PKTS256TO511OCTETS_OFFSET                                 0x00000360
#define ESW_MIB_PKTS256TO511OCTETS_TYPE                                   UInt32
#define ESW_MIB_PKTS256TO511OCTETS_RESERVED_MASK                          0x00000000
#define    ESW_MIB_PKTS256TO511OCTETS_PKTS256TO511OCTETS_SHIFT            0
#define    ESW_MIB_PKTS256TO511OCTETS_PKTS256TO511OCTETS_MASK             0xFFFFFFFF

#define ESW_MIB_PKTS512TO1023OCTETS_OFFSET                                0x00000380
#define ESW_MIB_PKTS512TO1023OCTETS_TYPE                                  UInt32
#define ESW_MIB_PKTS512TO1023OCTETS_RESERVED_MASK                         0x00000000
#define    ESW_MIB_PKTS512TO1023OCTETS_PKTS512TO1023OCTETS_SHIFT          0
#define    ESW_MIB_PKTS512TO1023OCTETS_PKTS512TO1023OCTETS_MASK           0xFFFFFFFF

#define ESW_MIB_PKTS1024TOMAXPKTOCTETS_OFFSET                             0x000003A0
#define ESW_MIB_PKTS1024TOMAXPKTOCTETS_TYPE                               UInt32
#define ESW_MIB_PKTS1024TOMAXPKTOCTETS_RESERVED_MASK                      0x00000000
#define    ESW_MIB_PKTS1024TOMAXPKTOCTETS_PKTS1024TOMAXPKTOCTETS_SHIFT    0
#define    ESW_MIB_PKTS1024TOMAXPKTOCTETS_PKTS1024TOMAXPKTOCTETS_MASK     0xFFFFFFFF

#define ESW_MIB_RXOVERSIZEPKTS_OFFSET                                     0x000003C0
#define ESW_MIB_RXOVERSIZEPKTS_TYPE                                       UInt32
#define ESW_MIB_RXOVERSIZEPKTS_RESERVED_MASK                              0x00000000
#define    ESW_MIB_RXOVERSIZEPKTS_RXOVERSIZEPKTS_SHIFT                    0
#define    ESW_MIB_RXOVERSIZEPKTS_RXOVERSIZEPKTS_MASK                     0xFFFFFFFF

#define ESW_MIB_RXJABBERS_OFFSET                                          0x000003E0
#define ESW_MIB_RXJABBERS_TYPE                                            UInt32
#define ESW_MIB_RXJABBERS_RESERVED_MASK                                   0x00000000
#define    ESW_MIB_RXJABBERS_RXJABBERS_SHIFT                              0
#define    ESW_MIB_RXJABBERS_RXJABBERS_MASK                               0xFFFFFFFF

#define ESW_MIB_RXALIGNMENTERRORS_OFFSET                                  0x00000400
#define ESW_MIB_RXALIGNMENTERRORS_TYPE                                    UInt32
#define ESW_MIB_RXALIGNMENTERRORS_RESERVED_MASK                           0x00000000
#define    ESW_MIB_RXALIGNMENTERRORS_RXALIGNMENTERRORS_SHIFT              0
#define    ESW_MIB_RXALIGNMENTERRORS_RXALIGNMENTERRORS_MASK               0xFFFFFFFF

#define ESW_MIB_RXFCSERRORS_OFFSET                                        0x00000420
#define ESW_MIB_RXFCSERRORS_TYPE                                          UInt32
#define ESW_MIB_RXFCSERRORS_RESERVED_MASK                                 0x00000000
#define    ESW_MIB_RXFCSERRORS_RXFCSERRORS_SHIFT                          0
#define    ESW_MIB_RXFCSERRORS_RXFCSERRORS_MASK                           0xFFFFFFFF

#define ESW_MIB_RXGOODOCTETS_OFFSET                                       0x00000440
#define ESW_MIB_RXGOODOCTETS_TYPE                                         UInt64
#define ESW_MIB_RXGOODOCTETS_RESERVED_MASK                                0x00000000
#define    ESW_MIB_RXGOODOCTETS_RXGOODOCTETS_SHIFT                        0
#define    ESW_MIB_RXGOODOCTETS_RXGOODOCTETS_MASK                         0xFFFFFFFFFFFFFFFF

#define ESW_MIB_RXDROPPKTS_OFFSET                                         0x00000480
#define ESW_MIB_RXDROPPKTS_TYPE                                           UInt32
#define ESW_MIB_RXDROPPKTS_RESERVED_MASK                                  0x00000000
#define    ESW_MIB_RXDROPPKTS_RXDROPPKTS_SHIFT                            0
#define    ESW_MIB_RXDROPPKTS_RXDROPPKTS_MASK                             0xFFFFFFFF

#define ESW_MIB_RXUNICASTPKTS_OFFSET                                      0x000004A0
#define ESW_MIB_RXUNICASTPKTS_TYPE                                        UInt32
#define ESW_MIB_RXUNICASTPKTS_RESERVED_MASK                               0x00000000
#define    ESW_MIB_RXUNICASTPKTS_RXUNICASTPKTS_SHIFT                      0
#define    ESW_MIB_RXUNICASTPKTS_RXUNICASTPKTS_MASK                       0xFFFFFFFF

#define ESW_MIB_RXMULTICASTPKTS_OFFSET                                    0x000004C0
#define ESW_MIB_RXMULTICASTPKTS_TYPE                                      UInt32
#define ESW_MIB_RXMULTICASTPKTS_RESERVED_MASK                             0x00000000
#define    ESW_MIB_RXMULTICASTPKTS_RXMULTICASTPKTS_SHIFT                  0
#define    ESW_MIB_RXMULTICASTPKTS_RXMULTICASTPKTS_MASK                   0xFFFFFFFF

#define ESW_MIB_RXBROADCASTPKT_OFFSET                                     0x000004E0
#define ESW_MIB_RXBROADCASTPKT_TYPE                                       UInt32
#define ESW_MIB_RXBROADCASTPKT_RESERVED_MASK                              0x00000000
#define    ESW_MIB_RXBROADCASTPKT_RXBROADCASTPKT_SHIFT                    0
#define    ESW_MIB_RXBROADCASTPKT_RXBROADCASTPKT_MASK                     0xFFFFFFFF

#define ESW_MIB_RXSACHANGES_OFFSET                                        0x00000500
#define ESW_MIB_RXSACHANGES_TYPE                                          UInt32
#define ESW_MIB_RXSACHANGES_RESERVED_MASK                                 0x00000000
#define    ESW_MIB_RXSACHANGES_RXSACHANGES_SHIFT                          0
#define    ESW_MIB_RXSACHANGES_RXSACHANGES_MASK                           0xFFFFFFFF

#define ESW_MIB_RXFRAGMENTS_OFFSET                                        0x00000520
#define ESW_MIB_RXFRAGMENTS_TYPE                                          UInt32
#define ESW_MIB_RXFRAGMENTS_RESERVED_MASK                                 0x00000000
#define    ESW_MIB_RXFRAGMENTS_RXFRAGMENTS_SHIFT                          0
#define    ESW_MIB_RXFRAGMENTS_RXFRAGMENTS_MASK                           0xFFFFFFFF

#define ESW_MIB_JUMBOPKTCOUNT_OFFSET                                      0x00000540
#define ESW_MIB_JUMBOPKTCOUNT_TYPE                                        UInt32
#define ESW_MIB_JUMBOPKTCOUNT_RESERVED_MASK                               0x00000000
#define    ESW_MIB_JUMBOPKTCOUNT_JUMBOPKTCOUNT_SHIFT                      0
#define    ESW_MIB_JUMBOPKTCOUNT_JUMBOPKTCOUNT_MASK                       0xFFFFFFFF

#define ESW_MIB_RXSYMBOLERROR_OFFSET                                      0x00000560
#define ESW_MIB_RXSYMBOLERROR_TYPE                                        UInt32
#define ESW_MIB_RXSYMBOLERROR_RESERVED_MASK                               0x00000000
#define    ESW_MIB_RXSYMBOLERROR_RXSYMBOLERROR_SHIFT                      0
#define    ESW_MIB_RXSYMBOLERROR_RXSYMBOLERROR_MASK                       0xFFFFFFFF

#define ESW_MIB_INRANGEERRORCOUNT_OFFSET                                  0x00000580
#define ESW_MIB_INRANGEERRORCOUNT_TYPE                                    UInt32
#define ESW_MIB_INRANGEERRORCOUNT_RESERVED_MASK                           0x00000000
#define    ESW_MIB_INRANGEERRORCOUNT_INRANGEERRORCOUNT_SHIFT              0
#define    ESW_MIB_INRANGEERRORCOUNT_INRANGEERRORCOUNT_MASK               0xFFFFFFFF

#define ESW_MIB_OUTRANGEERRORCOUNT_OFFSET                                 0x000005A0
#define ESW_MIB_OUTRANGEERRORCOUNT_TYPE                                   UInt32
#define ESW_MIB_OUTRANGEERRORCOUNT_RESERVED_MASK                          0x00000000
#define    ESW_MIB_OUTRANGEERRORCOUNT_OUTRANGEERRORCOUNT_SHIFT            0
#define    ESW_MIB_OUTRANGEERRORCOUNT_OUTRANGEERRORCOUNT_MASK             0xFFFFFFFF

#define ESW_MIB_RXDISCARD_OFFSET                                          0x00000600
#define ESW_MIB_RXDISCARD_TYPE                                            UInt32
#define ESW_MIB_RXDISCARD_RESERVED_MASK                                   0x00000000
#define    ESW_MIB_RXDISCARD_RXDISCARD_SHIFT                              0
#define    ESW_MIB_RXDISCARD_RXDISCARD_MASK                               0xFFFFFFFF

#define ESW_MIB_DOSASSERTCOUNT_OFFSET                                     0x00000620
#define ESW_MIB_DOSASSERTCOUNT_TYPE                                       UInt32
#define ESW_MIB_DOSASSERTCOUNT_RESERVED_MASK                              0x00000000
#define    ESW_MIB_DOSASSERTCOUNT_DOSASSERTCOUNT_SHIFT                    0
#define    ESW_MIB_DOSASSERTCOUNT_DOSASSERTCOUNT_MASK                     0xFFFFFFFF

#define ESW_MIB_DOSDROPCOUNT_OFFSET                                       0x00000640
#define ESW_MIB_DOSDROPCOUNT_TYPE                                         UInt32
#define ESW_MIB_DOSDROPCOUNT_RESERVED_MASK                                0x00000000
#define    ESW_MIB_DOSDROPCOUNT_DOSDROPCOUNT_SHIFT                        0
#define    ESW_MIB_DOSDROPCOUNT_DOSDROPCOUNT_MASK                         0xFFFFFFFF

#define ESW_MIB_INGRESSRATEASSERTCOUNT_OFFSET                             0x00000660
#define ESW_MIB_INGRESSRATEASSERTCOUNT_TYPE                               UInt32
#define ESW_MIB_INGRESSRATEASSERTCOUNT_RESERVED_MASK                      0x00000000
#define    ESW_MIB_INGRESSRATEASSERTCOUNT_INGRESSRATEASSERTCOUNT_SHIFT    0
#define    ESW_MIB_INGRESSRATEASSERTCOUNT_INGRESSRATEASSERTCOUNT_MASK     0xFFFFFFFF

#define ESW_MIB_INGRESSRATEDROPCOUNT_OFFSET                               0x00000680
#define ESW_MIB_INGRESSRATEDROPCOUNT_TYPE                                 UInt32
#define ESW_MIB_INGRESSRATEDROPCOUNT_RESERVED_MASK                        0x00000000
#define    ESW_MIB_INGRESSRATEDROPCOUNT_INGRESSRATEDROPCOUNT_SHIFT        0
#define    ESW_MIB_INGRESSRATEDROPCOUNT_INGRESSRATEDROPCOUNT_MASK         0xFFFFFFFF

#define ESW_MIB_EGRESSRATEASSERTCOUNT_OFFSET                              0x000006A0
#define ESW_MIB_EGRESSRATEASSERTCOUNT_TYPE                                UInt32
#define ESW_MIB_EGRESSRATEASSERTCOUNT_RESERVED_MASK                       0x00000000
#define    ESW_MIB_EGRESSRATEASSERTCOUNT_EGRESSRATEASSERTCOUNT_SHIFT      0
#define    ESW_MIB_EGRESSRATEASSERTCOUNT_EGRESSRATEASSERTCOUNT_MASK       0xFFFFFFFF

#endif /* __BRCM_RDB_ESW_MIB_H__ */



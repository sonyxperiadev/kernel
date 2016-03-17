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

#ifndef __BRCM_RDB_MODEM_CLOCKS_PART_1_H__
#define __BRCM_RDB_MODEM_CLOCKS_PART_1_H__

#define MODEM_CLOCKS_PART_1_BUSGATE_OFFSET                                0x00000000
#define MODEM_CLOCKS_PART_1_BUSGATE_TYPE                                  UInt32
#define MODEM_CLOCKS_PART_1_BUSGATE_RESERVED_MASK                         0xFFFFFFFC
#define    MODEM_CLOCKS_PART_1_BUSGATE_AUTOGATEAPB_SHIFT                  1
#define    MODEM_CLOCKS_PART_1_BUSGATE_AUTOGATEAPB_MASK                   0x00000002
#define    MODEM_CLOCKS_PART_1_BUSGATE_AUTOGATEAHB_SHIFT                  0
#define    MODEM_CLOCKS_PART_1_BUSGATE_AUTOGATEAHB_MASK                   0x00000001

#define MODEM_CLOCKS_PART_1_TX_OFFSET                                     0x00000004
#define MODEM_CLOCKS_PART_1_TX_TYPE                                       UInt32
#define MODEM_CLOCKS_PART_1_TX_RESERVED_MASK                              0xFFFF7333
#define    MODEM_CLOCKS_PART_1_TX_ENABLEHSUPACLK_SHIFT                    15
#define    MODEM_CLOCKS_PART_1_TX_ENABLEHSUPACLK_MASK                     0x00008000
#define    MODEM_CLOCKS_PART_1_TX_ENABLETXTURBOCLK_SHIFT                  11
#define    MODEM_CLOCKS_PART_1_TX_ENABLETXTURBOCLK_MASK                   0x00000800
#define    MODEM_CLOCKS_PART_1_TX_AUTOGATETXTURBOCLK_SHIFT                10
#define    MODEM_CLOCKS_PART_1_TX_AUTOGATETXTURBOCLK_MASK                 0x00000400
#define    MODEM_CLOCKS_PART_1_TX_ENABLETXCLCLK_SHIFT                     7
#define    MODEM_CLOCKS_PART_1_TX_ENABLETXCLCLK_MASK                      0x00000080
#define    MODEM_CLOCKS_PART_1_TX_AUTOGATETXCLCLK_SHIFT                   6
#define    MODEM_CLOCKS_PART_1_TX_AUTOGATETXCLCLK_MASK                    0x00000040
#define    MODEM_CLOCKS_PART_1_TX_ENABLETXBLCLK_SHIFT                     3
#define    MODEM_CLOCKS_PART_1_TX_ENABLETXBLCLK_MASK                      0x00000008
#define    MODEM_CLOCKS_PART_1_TX_AUTOGATETXBLCLK_SHIFT                   2
#define    MODEM_CLOCKS_PART_1_TX_AUTOGATETXBLCLK_MASK                    0x00000004

#define MODEM_CLOCKS_PART_1_SEARCHER_OFFSET                               0x00000008
#define MODEM_CLOCKS_PART_1_SEARCHER_TYPE                                 UInt32
#define MODEM_CLOCKS_PART_1_SEARCHER_RESERVED_MASK                        0xC00F0030
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLERFICMST_SHIFT               29
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLERFICMST_MASK                0x20000000
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATERFICMST_SHIFT             28
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATERFICMST_MASK              0x10000000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLE3GDIGIRF_SHIFT              27
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLE3GDIGIRF_MASK               0x08000000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLECX4EXT_SHIFT                26
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLECX4EXT_MASK                 0x04000000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLESCHED_SHIFT                 25
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLESCHED_MASK                  0x02000000
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATESCHED_SHIFT               24
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATESCHED_MASK                0x01000000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLETWIF_SHIFT                  23
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLETWIF_MASK                   0x00800000
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATETWIF_SHIFT                22
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATETWIF_MASK                 0x00400000
#define    MODEM_CLOCKS_PART_1_SEARCHER_TWIFDIVIDER_RB_10_SHIFT           20
#define    MODEM_CLOCKS_PART_1_SEARCHER_TWIFDIVIDER_RB_10_MASK            0x00300000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLESTTDCLK_SHIFT               15
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLESTTDCLK_MASK                0x00008000
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATESTTDCLK_SHIFT             14
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATESTTDCLK_MASK              0x00004000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD1_30CLK_SHIFT            13
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD1_30CLK_MASK             0x00002000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD1_15CLK_SHIFT            12
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD1_15CLK_MASK             0x00001000
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD0_30CLK_SHIFT            11
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD0_30CLK_MASK             0x00000800
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATEMPD30CLK_SHIFT            10
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATEMPD30CLK_MASK             0x00000400
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD0_15CLK_SHIFT            9
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEMPD0_15CLK_MASK             0x00000200
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATEMPD15CLK_SHIFT            8
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATEMPD15CLK_MASK             0x00000100
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLESSYNCCLK_SHIFT              7
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLESSYNCCLK_MASK               0x00000080
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATESSYNCCLK_SHIFT            6
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATESSYNCCLK_MASK             0x00000040
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEPSYNCCLK_SHIFT              3
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEPSYNCCLK_MASK               0x00000008
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATEPSYNCCLK_SHIFT            2
#define    MODEM_CLOCKS_PART_1_SEARCHER_AUTOGATEPSYNCCLK_MASK             0x00000004
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEEWSCLK_SHIFT                1
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEEWSCLK_MASK                 0x00000002
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEE2SSCLK_SHIFT               0
#define    MODEM_CLOCKS_PART_1_SEARCHER_ENABLEE2SSCLK_MASK                0x00000001

#define MODEM_CLOCKS_PART_1_DMA_OFFSET                                    0x0000000C
#define MODEM_CLOCKS_PART_1_DMA_TYPE                                      UInt32
#define MODEM_CLOCKS_PART_1_DMA_RESERVED_MASK                             0xFFFFFFC0
#define    MODEM_CLOCKS_PART_1_DMA_DMA_TEST_ERROR_SHIFT                   5
#define    MODEM_CLOCKS_PART_1_DMA_DMA_TEST_ERROR_MASK                    0x00000020
#define    MODEM_CLOCKS_PART_1_DMA_DMA_TEST_RUN_SHIFT                     4
#define    MODEM_CLOCKS_PART_1_DMA_DMA_TEST_RUN_MASK                      0x00000010
#define    MODEM_CLOCKS_PART_1_DMA_HSDPATURBODMAERROR_S1_SHIFT            3
#define    MODEM_CLOCKS_PART_1_DMA_HSDPATURBODMAERROR_S1_MASK             0x00000008
#define    MODEM_CLOCKS_PART_1_DMA_NOBURST_SHIFT                          2
#define    MODEM_CLOCKS_PART_1_DMA_NOBURST_MASK                           0x00000004
#define    MODEM_CLOCKS_PART_1_DMA_RSPDMAONLY_SHIFT                       1
#define    MODEM_CLOCKS_PART_1_DMA_RSPDMAONLY_MASK                        0x00000002
#define    MODEM_CLOCKS_PART_1_DMA_ENABLERSPDMA_SHIFT                     0
#define    MODEM_CLOCKS_PART_1_DMA_ENABLERSPDMA_MASK                      0x00000001

#define MODEM_CLOCKS_PART_1_HSDPA_OFFSET                                  0x00000010
#define MODEM_CLOCKS_PART_1_HSDPA_TYPE                                    UInt32
#define MODEM_CLOCKS_PART_1_HSDPA_RESERVED_MASK                           0xFFFFFFC0
#define    MODEM_CLOCKS_PART_1_HSDPA_ENABLESCCH_SHIFT                     5
#define    MODEM_CLOCKS_PART_1_HSDPA_ENABLESCCH_MASK                      0x00000020
#define    MODEM_CLOCKS_PART_1_HSDPA_AUTOGATESCCH_SHIFT                   4
#define    MODEM_CLOCKS_PART_1_HSDPA_AUTOGATESCCH_MASK                    0x00000010
#define    MODEM_CLOCKS_PART_1_HSDPA_ENABLEHTDM_SHIFT                     3
#define    MODEM_CLOCKS_PART_1_HSDPA_ENABLEHTDM_MASK                      0x00000008
#define    MODEM_CLOCKS_PART_1_HSDPA_HSDPACLKDIV_SHIFT                    2
#define    MODEM_CLOCKS_PART_1_HSDPA_HSDPACLKDIV_MASK                     0x00000004
#define    MODEM_CLOCKS_PART_1_HSDPA_ENABLECPP_SHIFT                      1
#define    MODEM_CLOCKS_PART_1_HSDPA_ENABLECPP_MASK                       0x00000002
#define    MODEM_CLOCKS_PART_1_HSDPA_AUTOGATECPP_SHIFT                    0
#define    MODEM_CLOCKS_PART_1_HSDPA_AUTOGATECPP_MASK                     0x00000001

#define MODEM_CLOCKS_PART_1_AHBDIVIDER_OFFSET                             0x00000014
#define MODEM_CLOCKS_PART_1_AHBDIVIDER_TYPE                               UInt32
#define MODEM_CLOCKS_PART_1_AHBDIVIDER_RESERVED_MASK                      0xFFFFFFFC
#define    MODEM_CLOCKS_PART_1_AHBDIVIDER_AHBDIVIDER1_R1_SHIFT            1
#define    MODEM_CLOCKS_PART_1_AHBDIVIDER_AHBDIVIDER1_R1_MASK             0x00000002
#define    MODEM_CLOCKS_PART_1_AHBDIVIDER_AHBDIVIDER0_R1_SHIFT            0
#define    MODEM_CLOCKS_PART_1_AHBDIVIDER_AHBDIVIDER0_R1_MASK             0x00000001

#endif /* __BRCM_RDB_MODEM_CLOCKS_PART_1_H__ */



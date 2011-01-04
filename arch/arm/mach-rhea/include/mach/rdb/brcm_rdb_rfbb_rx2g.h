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

#ifndef __BRCM_RDB_RFBB_RX2G_H__
#define __BRCM_RDB_RFBB_RX2G_H__

#define RFBB_RX2G_CR_OFFSET                                               0x00000000
#define RFBB_RX2G_CR_TYPE                                                 UInt32
#define RFBB_RX2G_CR_RESERVED_MASK                                        0x00000000
#define    RFBB_RX2G_CR_CLK_EN_SHIFT                                      31
#define    RFBB_RX2G_CR_CLK_EN_MASK                                       0x80000000
#define    RFBB_RX2G_CR_RESERVED_30TO26_SHIFT                             26
#define    RFBB_RX2G_CR_RESERVED_30TO26_MASK                              0x7C000000
#define    RFBB_RX2G_CR_NS_SHIFT                                          16
#define    RFBB_RX2G_CR_NS_MASK                                           0x03FF0000
#define    RFBB_RX2G_CR_RESERVED_15TO11_SHIFT                             11
#define    RFBB_RX2G_CR_RESERVED_15TO11_MASK                              0x0000F800
#define    RFBB_RX2G_CR_VPGAIN_SHIFT                                      8
#define    RFBB_RX2G_CR_VPGAIN_MASK                                       0x00000700
#define    RFBB_RX2G_CR_POLARX_SHIFT                                      7
#define    RFBB_RX2G_CR_POLARX_MASK                                       0x00000080
#define    RFBB_RX2G_CR_RXORDER_SHIFT                                     6
#define    RFBB_RX2G_CR_RXORDER_MASK                                      0x00000040
#define    RFBB_RX2G_CR_RXCLK_GATING_SHIFT                                5
#define    RFBB_RX2G_CR_RXCLK_GATING_MASK                                 0x00000020
#define    RFBB_RX2G_CR_RXINT_SHIFT                                       4
#define    RFBB_RX2G_CR_RXINT_MASK                                        0x00000010
#define    RFBB_RX2G_CR_RESERVED_3TO0_SHIFT                               0
#define    RFBB_RX2G_CR_RESERVED_3TO0_MASK                                0x0000000F

#define RFBB_RX2G_STR_OFFSET                                              0x00000004
#define RFBB_RX2G_STR_TYPE                                                UInt32
#define RFBB_RX2G_STR_RESERVED_MASK                                       0x00000000
#define    RFBB_RX2G_STR_T0_SHIFT                                         0
#define    RFBB_RX2G_STR_T0_MASK                                          0xFFFFFFFF

#define RFBB_RX2G_TR_OFFSET                                               0x00000008
#define RFBB_RX2G_TR_TYPE                                                 UInt32
#define RFBB_RX2G_TR_RESERVED_MASK                                        0x00000000
#define    RFBB_RX2G_TR_RESERVED_31TO27_SHIFT                             27
#define    RFBB_RX2G_TR_RESERVED_31TO27_MASK                              0xF8000000
#define    RFBB_RX2G_TR_RXT0_SHIFT                                        16
#define    RFBB_RX2G_TR_RXT0_MASK                                         0x07FF0000
#define    RFBB_RX2G_TR_RESERVED_15TO10_SHIFT                             10
#define    RFBB_RX2G_TR_RESERVED_15TO10_MASK                              0x0000FC00
#define    RFBB_RX2G_TR_RXT1_SHIFT                                        0
#define    RFBB_RX2G_TR_RXT1_MASK                                         0x000003FF

#define RFBB_RX2G_MR_OFFSET                                               0x0000000C
#define RFBB_RX2G_MR_TYPE                                                 UInt32
#define RFBB_RX2G_MR_RESERVED_MASK                                        0x00000000
#define    RFBB_RX2G_MR_RESERVED_31TO4_SHIFT                              4
#define    RFBB_RX2G_MR_RESERVED_31TO4_MASK                               0xFFFFFFF0
#define    RFBB_RX2G_MR_RC_SHIFT                                          0
#define    RFBB_RX2G_MR_RC_MASK                                           0x0000000F

typedef volatile struct {
   UInt32 m_CR;                      // 0x0000
   UInt32 m_STR;                     // 0x0004
   UInt32 m_TR;                      // 0x0008
   UInt32 m_MR;                      // 0x000C
} BRCM_RFBB_RX2G_REGS;


#endif /* __BRCM_RDB_RFBB_RX2G_H__ */



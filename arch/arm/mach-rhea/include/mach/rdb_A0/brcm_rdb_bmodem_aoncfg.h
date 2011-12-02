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
/*     Date     : Generated on 5/17/2011 0:56:25                                             */
/*     RDB file : //RHEA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_BMODEM_AONCFG_H__
#define __BRCM_RDB_BMODEM_AONCFG_H__

#define BMODEM_AONCFG_CR_OFFSET                                           0x00000000
#define BMODEM_AONCFG_CR_TYPE                                             UInt32
#define BMODEM_AONCFG_CR_RESERVED_MASK                                    0xFFFFFFFC
#define    BMODEM_AONCFG_CR_AUXDAC_PWDOWN_3G_SHIFT                        1
#define    BMODEM_AONCFG_CR_AUXDAC_PWDOWN_3G_MASK                         0x00000002
#define    BMODEM_AONCFG_CR_AUXDAC_CLK_CTRL_3G_SHIFT                      0
#define    BMODEM_AONCFG_CR_AUXDAC_CLK_CTRL_3G_MASK                       0x00000001

#define BMODEM_AONCFG_AUXDAC_OFFSET                                       0x00000004
#define BMODEM_AONCFG_AUXDAC_TYPE                                         UInt32
#define BMODEM_AONCFG_AUXDAC_RESERVED_MASK                                0xFFFC0000
#define    BMODEM_AONCFG_AUXDAC_ISOLATE_OUT_SHIFT                         17
#define    BMODEM_AONCFG_AUXDAC_ISOLATE_OUT_MASK                          0x00020000
#define    BMODEM_AONCFG_AUXDAC_POWERDOWN_SHIFT                           16
#define    BMODEM_AONCFG_AUXDAC_POWERDOWN_MASK                            0x00010000
#define    BMODEM_AONCFG_AUXDAC_AUXDACCTRL_SHIFT                          0
#define    BMODEM_AONCFG_AUXDAC_AUXDACCTRL_MASK                           0x0000FFFF

#define BMODEM_AONCFG_IRDROP0_OFFSET                                      0x00000010
#define BMODEM_AONCFG_IRDROP0_TYPE                                        UInt32
#define BMODEM_AONCFG_IRDROP0_RESERVED_MASK                               0xCC00CC00
#define    BMODEM_AONCFG_IRDROP0_IRD_OSC_OUT_ENABLE_DSP_SHIFT             29
#define    BMODEM_AONCFG_IRDROP0_IRD_OSC_OUT_ENABLE_DSP_MASK              0x20000000
#define    BMODEM_AONCFG_IRDROP0_IRD_MON_ENABLE_DSP_SHIFT                 28
#define    BMODEM_AONCFG_IRDROP0_IRD_MON_ENABLE_DSP_MASK                  0x10000000
#define    BMODEM_AONCFG_IRDROP0_IRD_COUNTER_OUT_DSP_SHIFT                16
#define    BMODEM_AONCFG_IRDROP0_IRD_COUNTER_OUT_DSP_MASK                 0x03FF0000
#define    BMODEM_AONCFG_IRDROP0_IRD_OSC_OUT_ENABLE_R4_SHIFT              13
#define    BMODEM_AONCFG_IRDROP0_IRD_OSC_OUT_ENABLE_R4_MASK               0x00002000
#define    BMODEM_AONCFG_IRDROP0_IRD_MON_ENABLE_R4_SHIFT                  12
#define    BMODEM_AONCFG_IRDROP0_IRD_MON_ENABLE_R4_MASK                   0x00001000
#define    BMODEM_AONCFG_IRDROP0_IRD_COUNTER_OUT_R4_SHIFT                 0
#define    BMODEM_AONCFG_IRDROP0_IRD_COUNTER_OUT_R4_MASK                  0x000003FF

#define BMODEM_AONCFG_IRDROP1_OFFSET                                      0x00000014
#define BMODEM_AONCFG_IRDROP1_TYPE                                        UInt32
#define BMODEM_AONCFG_IRDROP1_RESERVED_MASK                               0xCC00CC00
#define    BMODEM_AONCFG_IRDROP1_IRD_OSC_OUT_ENABLE_WCDMA_2_SHIFT         29
#define    BMODEM_AONCFG_IRDROP1_IRD_OSC_OUT_ENABLE_WCDMA_2_MASK          0x20000000
#define    BMODEM_AONCFG_IRDROP1_IRD_MON_ENABLE_WCDMA_2_SHIFT             28
#define    BMODEM_AONCFG_IRDROP1_IRD_MON_ENABLE_WCDMA_2_MASK              0x10000000
#define    BMODEM_AONCFG_IRDROP1_IRD_COUNTER_OUT_WCDMA_2_SHIFT            16
#define    BMODEM_AONCFG_IRDROP1_IRD_COUNTER_OUT_WCDMA_2_MASK             0x03FF0000
#define    BMODEM_AONCFG_IRDROP1_IRD_OSC_OUT_ENABLE_WCDMA_1_SHIFT         13
#define    BMODEM_AONCFG_IRDROP1_IRD_OSC_OUT_ENABLE_WCDMA_1_MASK          0x00002000
#define    BMODEM_AONCFG_IRDROP1_IRD_MON_ENABLE_WCDMA_1_SHIFT             12
#define    BMODEM_AONCFG_IRDROP1_IRD_MON_ENABLE_WCDMA_1_MASK              0x00001000
#define    BMODEM_AONCFG_IRDROP1_IRD_COUNTER_OUT_WCDMA_1_SHIFT            0
#define    BMODEM_AONCFG_IRDROP1_IRD_COUNTER_OUT_WCDMA_1_MASK             0x000003FF

#endif /* __BRCM_RDB_BMODEM_AONCFG_H__ */



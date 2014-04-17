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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_BBRX_H__
#define __BRCM_RDB_BBRX_H__

#define BBRX_IQ_DELAY_OFFSET                                              0x00000000
#define BBRX_IQ_DELAY_TYPE                                                UInt32
#define BBRX_IQ_DELAY_RESERVED_MASK                                       0xFFFFCDDC
#define    BBRX_IQ_DELAY_HSDPA_VPGAIN_SHIFT                               12
#define    BBRX_IQ_DELAY_HSDPA_VPGAIN_MASK                                0x00003000
#define    BBRX_IQ_DELAY_VP_SEL_SHIFT                                     9
#define    BBRX_IQ_DELAY_VP_SEL_MASK                                      0x00000200
#define    BBRX_IQ_DELAY_BYPASS_CMF_SHIFT                                 5
#define    BBRX_IQ_DELAY_BYPASS_CMF_MASK                                  0x00000020
#define    BBRX_IQ_DELAY_IQ_DELAY_SHIFT                                   0
#define    BBRX_IQ_DELAY_IQ_DELAY_MASK                                    0x00000003

#define BBRX_CMF_RESP_EN_OFFSET                                           0x00000004
#define BBRX_CMF_RESP_EN_TYPE                                             UInt32
#define BBRX_CMF_RESP_EN_RESERVED_MASK                                    0xFFFFFFFE
#define    BBRX_CMF_RESP_EN_CMF_RESP_EN_SHIFT                             0
#define    BBRX_CMF_RESP_EN_CMF_RESP_EN_MASK                              0x00000001

#define BBRX_NOTCHCTRL0_OFFSET                                            0x00000008
#define BBRX_NOTCHCTRL0_TYPE                                              UInt32
#define BBRX_NOTCHCTRL0_RESERVED_MASK                                     0xFFFFFFEC
#define    BBRX_NOTCHCTRL0_SW_RESET_SHIFT                                 4
#define    BBRX_NOTCHCTRL0_SW_RESET_MASK                                  0x00000010
#define    BBRX_NOTCHCTRL0_NOTCH_OFF_SHIFT                                1
#define    BBRX_NOTCHCTRL0_NOTCH_OFF_MASK                                 0x00000002
#define    BBRX_NOTCHCTRL0_NOTCH_EN_SHIFT                                 0
#define    BBRX_NOTCHCTRL0_NOTCH_EN_MASK                                  0x00000001

#define BBRX_NOTCHCTRL1_OFFSET                                            0x0000000C
#define BBRX_NOTCHCTRL1_TYPE                                              UInt32
#define BBRX_NOTCHCTRL1_RESERVED_MASK                                     0x80008000
#define    BBRX_NOTCHCTRL1_RX_SHIFT                                       16
#define    BBRX_NOTCHCTRL1_RX_MASK                                        0x7FFF0000
#define    BBRX_NOTCHCTRL1_RY_SHIFT                                       0
#define    BBRX_NOTCHCTRL1_RY_MASK                                        0x00007FFF

#define BBRX_NOTCHCTRL2_OFFSET                                            0x00000010
#define BBRX_NOTCHCTRL2_TYPE                                              UInt32
#define BBRX_NOTCHCTRL2_RESERVED_MASK                                     0x08080000
#define    BBRX_NOTCHCTRL2_NS_BW_SHIFT                                    28
#define    BBRX_NOTCHCTRL2_NS_BW_MASK                                     0xF0000000
#define    BBRX_NOTCHCTRL2_BW_SHIFT                                       24
#define    BBRX_NOTCHCTRL2_BW_MASK                                        0x07000000
#define    BBRX_NOTCHCTRL2_MAX_CNT_SHIFT                                  20
#define    BBRX_NOTCHCTRL2_MAX_CNT_MASK                                   0x00F00000
#define    BBRX_NOTCHCTRL2_THRESHOLD_SHIFT                                0
#define    BBRX_NOTCHCTRL2_THRESHOLD_MASK                                 0x0007FFFF

#define BBRX_NOTCHSTATUS_OFFSET                                           0x00000014
#define BBRX_NOTCHSTATUS_TYPE                                             UInt32
#define BBRX_NOTCHSTATUS_RESERVED_MASK                                    0x7FF80000
#define    BBRX_NOTCHSTATUS_NOTCH_ON_SHIFT                                31
#define    BBRX_NOTCHSTATUS_NOTCH_ON_MASK                                 0x80000000
#define    BBRX_NOTCHSTATUS_NS_IIR_REG_SHIFT                              0
#define    BBRX_NOTCHSTATUS_NS_IIR_REG_MASK                               0x0007FFFF

#endif /* __BRCM_RDB_BBRX_H__ */



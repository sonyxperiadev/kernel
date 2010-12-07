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

#ifndef __BRCM_RDB_RFBB_RX3G_H__
#define __BRCM_RDB_RFBB_RX3G_H__

#define RFBB_RX3G_NOTCHCTRL0_OFFSET                                       0x00000000
#define RFBB_RX3G_NOTCHCTRL0_TYPE                                         UInt32
#define RFBB_RX3G_NOTCHCTRL0_RESERVED_MASK                                0xFFFFFEEC
#define    RFBB_RX3G_NOTCHCTRL0_RX_3G_CLK_EN_SHIFT                        8
#define    RFBB_RX3G_NOTCHCTRL0_RX_3G_CLK_EN_MASK                         0x00000100
#define    RFBB_RX3G_NOTCHCTRL0_SW_RESET_SHIFT                            4
#define    RFBB_RX3G_NOTCHCTRL0_SW_RESET_MASK                             0x00000010
#define    RFBB_RX3G_NOTCHCTRL0_NOTCH_OFF_SHIFT                           1
#define    RFBB_RX3G_NOTCHCTRL0_NOTCH_OFF_MASK                            0x00000002
#define    RFBB_RX3G_NOTCHCTRL0_NOTCH_EN_SHIFT                            0
#define    RFBB_RX3G_NOTCHCTRL0_NOTCH_EN_MASK                             0x00000001

#define RFBB_RX3G_NOTCHCTRL1_OFFSET                                       0x00000004
#define RFBB_RX3G_NOTCHCTRL1_TYPE                                         UInt32
#define RFBB_RX3G_NOTCHCTRL1_RESERVED_MASK                                0x80008000
#define    RFBB_RX3G_NOTCHCTRL1_RX_SHIFT                                  16
#define    RFBB_RX3G_NOTCHCTRL1_RX_MASK                                   0x7FFF0000
#define    RFBB_RX3G_NOTCHCTRL1_RY_SHIFT                                  0
#define    RFBB_RX3G_NOTCHCTRL1_RY_MASK                                   0x00007FFF

#define RFBB_RX3G_NOTCHCTRL2_OFFSET                                       0x00000008
#define RFBB_RX3G_NOTCHCTRL2_TYPE                                         UInt32
#define RFBB_RX3G_NOTCHCTRL2_RESERVED_MASK                                0x08080000
#define    RFBB_RX3G_NOTCHCTRL2_NS_BW_SHIFT                               28
#define    RFBB_RX3G_NOTCHCTRL2_NS_BW_MASK                                0xF0000000
#define    RFBB_RX3G_NOTCHCTRL2_BW_SHIFT                                  24
#define    RFBB_RX3G_NOTCHCTRL2_BW_MASK                                   0x07000000
#define    RFBB_RX3G_NOTCHCTRL2_MAX_CNT_SHIFT                             20
#define    RFBB_RX3G_NOTCHCTRL2_MAX_CNT_MASK                              0x00F00000
#define    RFBB_RX3G_NOTCHCTRL2_THRESHOLD_SHIFT                           0
#define    RFBB_RX3G_NOTCHCTRL2_THRESHOLD_MASK                            0x0007FFFF

#define RFBB_RX3G_NOTCHSTATUS_OFFSET                                      0x0000000C
#define RFBB_RX3G_NOTCHSTATUS_TYPE                                        UInt32
#define RFBB_RX3G_NOTCHSTATUS_RESERVED_MASK                               0x7FF80000
#define    RFBB_RX3G_NOTCHSTATUS_NOTCH_ON_SHIFT                           31
#define    RFBB_RX3G_NOTCHSTATUS_NOTCH_ON_MASK                            0x80000000
#define    RFBB_RX3G_NOTCHSTATUS_NS_IIR_REG_SHIFT                         0
#define    RFBB_RX3G_NOTCHSTATUS_NS_IIR_REG_MASK                          0x0007FFFF

#define RFBB_RX3G_DCCFG1_OFFSET                                           0x00000010
#define RFBB_RX3G_DCCFG1_TYPE                                             UInt32
#define RFBB_RX3G_DCCFG1_RESERVED_MASK                                    0xFFFFF0EE
#define    RFBB_RX3G_DCCFG1_DCOFFSETAVGBW_SHIFT                           8
#define    RFBB_RX3G_DCCFG1_DCOFFSETAVGBW_MASK                            0x00000F00
#define    RFBB_RX3G_DCCFG1_DCOFFSETMODE_SHIFT                            4
#define    RFBB_RX3G_DCCFG1_DCOFFSETMODE_MASK                             0x00000010
#define    RFBB_RX3G_DCCFG1_DCOFFSETEN_SHIFT                              0
#define    RFBB_RX3G_DCCFG1_DCOFFSETEN_MASK                               0x00000001

#define RFBB_RX3G_DCCFG2_OFFSET                                           0x00000014
#define RFBB_RX3G_DCCFG2_TYPE                                             UInt32
#define RFBB_RX3G_DCCFG2_RESERVED_MASK                                    0xFFFFFF0E
#define    RFBB_RX3G_DCCFG2_DCOFFSETGAIN_SHIFT                            4
#define    RFBB_RX3G_DCCFG2_DCOFFSETGAIN_MASK                             0x000000F0
#define    RFBB_RX3G_DCCFG2_DCOFFSETLOOP_SHIFT                            0
#define    RFBB_RX3G_DCCFG2_DCOFFSETLOOP_MASK                             0x00000001

#define RFBB_RX3G_DCCFG3_OFFSET                                           0x00000018
#define RFBB_RX3G_DCCFG3_TYPE                                             UInt32
#define RFBB_RX3G_DCCFG3_RESERVED_MASK                                    0xFF800800
#define    RFBB_RX3G_DCCFG3_DCOFFSETINITQ_SHIFT                           12
#define    RFBB_RX3G_DCCFG3_DCOFFSETINITQ_MASK                            0x007FF000
#define    RFBB_RX3G_DCCFG3_DCOFFSETINITI_SHIFT                           0
#define    RFBB_RX3G_DCCFG3_DCOFFSETINITI_MASK                            0x000007FF

#define RFBB_RX3G_DCCSTATUS1_OFFSET                                       0x0000001C
#define RFBB_RX3G_DCCSTATUS1_TYPE                                         UInt32
#define RFBB_RX3G_DCCSTATUS1_RESERVED_MASK                                0xC0000800
#define    RFBB_RX3G_DCCSTATUS1_DCOFFSETAVGI_SHIFT                        12
#define    RFBB_RX3G_DCCSTATUS1_DCOFFSETAVGI_MASK                         0x3FFFF000
#define    RFBB_RX3G_DCCSTATUS1_DCOFFSETESTI_SHIFT                        0
#define    RFBB_RX3G_DCCSTATUS1_DCOFFSETESTI_MASK                         0x000007FF

#define RFBB_RX3G_DCCSTATUS2_OFFSET                                       0x00000020
#define RFBB_RX3G_DCCSTATUS2_TYPE                                         UInt32
#define RFBB_RX3G_DCCSTATUS2_RESERVED_MASK                                0xC0000800
#define    RFBB_RX3G_DCCSTATUS2_DCOFFSETAVGQ_SHIFT                        12
#define    RFBB_RX3G_DCCSTATUS2_DCOFFSETAVGQ_MASK                         0x3FFFF000
#define    RFBB_RX3G_DCCSTATUS2_DCOFFSETESTQ_SHIFT                        0
#define    RFBB_RX3G_DCCSTATUS2_DCOFFSETESTQ_MASK                         0x000007FF

#define RFBB_RX3G_INTERPCTRL_OFFSET                                       0x00000024
#define RFBB_RX3G_INTERPCTRL_TYPE                                         UInt32
#define RFBB_RX3G_INTERPCTRL_RESERVED_MASK                                0xFFFFFFFE
#define    RFBB_RX3G_INTERPCTRL_CX10_INTERP_EN_SHIFT                      0
#define    RFBB_RX3G_INTERPCTRL_CX10_INTERP_EN_MASK                       0x00000001

typedef volatile struct {
   UInt32 m_NOTCHCTRL0;              // 0x0000
   UInt32 m_NOTCHCTRL1;              // 0x0004
   UInt32 m_NOTCHCTRL2;              // 0x0008
   UInt32 m_NOTCHSTATUS;             // 0x000C
   UInt32 m_DCCFG1;                  // 0x0010
   UInt32 m_DCCFG2;                  // 0x0014
   UInt32 m_DCCFG3;                  // 0x0018
   UInt32 m_DCCSTATUS1;              // 0x001C
   UInt32 m_DCCSTATUS2;              // 0x0020
   UInt32 m_INTERPCTRL;              // 0x0024
} BRCM_RFBB_RX3G_REGS;


#endif /* __BRCM_RDB_RFBB_RX3G_H__ */



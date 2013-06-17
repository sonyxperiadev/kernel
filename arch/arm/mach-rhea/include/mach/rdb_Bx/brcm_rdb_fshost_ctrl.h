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

#ifndef __BRCM_RDB_FSHOST_CTRL_H__
#define __BRCM_RDB_FSHOST_CTRL_H__

#define FSHOST_CTRL_USBSUSPEND_OFFSET                                     0x00000000
#define FSHOST_CTRL_USBSUSPEND_TYPE                                       UInt32
#define FSHOST_CTRL_USBSUSPEND_RESERVED_MASK                              0xCD7C01F0
#define    FSHOST_CTRL_USBSUSPEND_SUSPEND_SOFTMODE_SHIFT                  29
#define    FSHOST_CTRL_USBSUSPEND_SUSPEND_SOFTMODE_MASK                   0x20000000
#define    FSHOST_CTRL_USBSUSPEND_SUSPEND_SOFT_SHIFT                      28
#define    FSHOST_CTRL_USBSUSPEND_SUSPEND_SOFT_MASK                       0x10000000
#define    FSHOST_CTRL_USBSUSPEND_RESRVD_SHIFT                            25
#define    FSHOST_CTRL_USBSUSPEND_RESRVD_MASK                             0x02000000
#define    FSHOST_CTRL_USBSUSPEND_WAKEUP_SHIFT                            23
#define    FSHOST_CTRL_USBSUSPEND_WAKEUP_MASK                             0x00800000
#define    FSHOST_CTRL_USBSUSPEND_USB_HCLK_EN_DIRECT_SHIFT                17
#define    FSHOST_CTRL_USBSUSPEND_USB_HCLK_EN_DIRECT_MASK                 0x00020000
#define    FSHOST_CTRL_USBSUSPEND_USB_ON_IS_HCLK_EN_SHIFT                 16
#define    FSHOST_CTRL_USBSUSPEND_USB_ON_IS_HCLK_EN_MASK                  0x00010000
#define    FSHOST_CTRL_USBSUSPEND_USB_ON_SHIFT                            15
#define    FSHOST_CTRL_USBSUSPEND_USB_ON_MASK                             0x00008000
#define    FSHOST_CTRL_USBSUSPEND_RSRVD_SHIFT                             14
#define    FSHOST_CTRL_USBSUSPEND_RSRVD_MASK                              0x00004000
#define    FSHOST_CTRL_USBSUSPEND_AHBM_ENDIAN_SHIFT                       13
#define    FSHOST_CTRL_USBSUSPEND_AHBM_ENDIAN_MASK                        0x00002000
#define    FSHOST_CTRL_USBSUSPEND_AHBS_ENDIAN_SHIFT                       12
#define    FSHOST_CTRL_USBSUSPEND_AHBS_ENDIAN_MASK                        0x00001000
#define    FSHOST_CTRL_USBSUSPEND_PRST_N_SW_SHIFT                         11
#define    FSHOST_CTRL_USBSUSPEND_PRST_N_SW_MASK                          0x00000800
#define    FSHOST_CTRL_USBSUSPEND_HRESET_N_SW_SHIFT                       10
#define    FSHOST_CTRL_USBSUSPEND_HRESET_N_SW_MASK                        0x00000400
#define    FSHOST_CTRL_USBSUSPEND_RX_SE0_SHIFT                            9
#define    FSHOST_CTRL_USBSUSPEND_RX_SE0_MASK                             0x00000200
#define    FSHOST_CTRL_USBSUSPEND_RSVD_SHIFT                              3
#define    FSHOST_CTRL_USBSUSPEND_RSVD_MASK                               0x00000008
#define    FSHOST_CTRL_USBSUSPEND_RESVD_SHIFT                             2
#define    FSHOST_CTRL_USBSUSPEND_RESVD_MASK                              0x00000004
#define    FSHOST_CTRL_USBSUSPEND_WAKEUP_INT_INV_SHIFT                    1
#define    FSHOST_CTRL_USBSUSPEND_WAKEUP_INT_INV_MASK                     0x00000002
#define    FSHOST_CTRL_USBSUSPEND_ICMODE_SHIFT                            0
#define    FSHOST_CTRL_USBSUSPEND_ICMODE_MASK                             0x00000001

#define FSHOST_CTRL_USBPOWERCTRL_OFFSET                                   0x00000004
#define FSHOST_CTRL_USBPOWERCTRL_TYPE                                     UInt32
#define FSHOST_CTRL_USBPOWERCTRL_RESERVED_MASK                            0xFFFFFFFF

#define FSHOST_CTRL_USBREG3_OFFSET                                        0x0000000C
#define FSHOST_CTRL_USBREG3_TYPE                                          UInt32
#define FSHOST_CTRL_USBREG3_RESERVED_MASK                                 0xFFFF33F3
#define    FSHOST_CTRL_USBREG3_RESERV0_SHIFT                              15
#define    FSHOST_CTRL_USBREG3_RESERV0_MASK                               0x00008000
#define    FSHOST_CTRL_USBREG3_RESERV1_SHIFT                              14
#define    FSHOST_CTRL_USBREG3_RESERV1_MASK                               0x00004000
#define    FSHOST_CTRL_USBREG3_RESERV2_SHIFT                              11
#define    FSHOST_CTRL_USBREG3_RESERV2_MASK                               0x00000800
#define    FSHOST_CTRL_USBREG3_RESERV3_SHIFT                              10
#define    FSHOST_CTRL_USBREG3_RESERV3_MASK                               0x00000400
#define    FSHOST_CTRL_USBREG3_RESERV4_SHIFT                              3
#define    FSHOST_CTRL_USBREG3_RESERV4_MASK                               0x00000008
#define    FSHOST_CTRL_USBREG3_RESERV5_SHIFT                              2
#define    FSHOST_CTRL_USBREG3_RESERV5_MASK                               0x00000004

#define FSHOST_CTRL_USBREG_OFFSET                                         0x00000020
#define FSHOST_CTRL_USBREG_TYPE                                           UInt32
#define FSHOST_CTRL_USBREG_RESERVED_MASK                                  0xFFFFFFFC
#define    FSHOST_CTRL_USBREG_SS_SCALEDOWN_MODE_SHIFT                     0
#define    FSHOST_CTRL_USBREG_SS_SCALEDOWN_MODE_MASK                      0x00000003

#endif /* __BRCM_RDB_FSHOST_CTRL_H__ */



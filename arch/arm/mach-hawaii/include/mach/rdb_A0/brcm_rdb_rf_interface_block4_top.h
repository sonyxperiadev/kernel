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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_RF_INTERFACE_BLOCK4_TOP_H__
#define __BRCM_RDB_RF_INTERFACE_BLOCK4_TOP_H__

#define RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_OFFSET                       0x00000000
#define RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_TYPE                         UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_RESERVED_MASK                0xFFFFFFEE
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_TXDACTCLKEN_SHIFT         4
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_TXDACTCLKEN_MASK          0x00000010
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_TXDACCLKEN_SHIFT          0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCLKCTRL_TXDACCLKEN_MASK           0x00000001

#define RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_OFFSET                        0x00000004
#define RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TYPE                          UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_RESERVED_MASK                 0xFFFFEECC
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACFORMAT_SHIFT          12
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACFORMAT_MASK           0x00001000
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACSWAP_SHIFT            8
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACSWAP_MASK             0x00000100
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACTEST_SHIFT            4
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACTEST_MASK             0x00000030
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACMODE_SHIFT            0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACCONFIG_TXDACMODE_MASK             0x00000003

#define RF_INTERFACE_BLOCK4_TOP_TXDACAPB_OFFSET                           0x00000008
#define RF_INTERFACE_BLOCK4_TOP_TXDACAPB_TYPE                             UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACAPB_RESERVED_MASK                    0xFC00FC00
#define    RF_INTERFACE_BLOCK4_TOP_TXDACAPB_TXDACTESTDATAQ_SHIFT          16
#define    RF_INTERFACE_BLOCK4_TOP_TXDACAPB_TXDACTESTDATAQ_MASK           0x03FF0000
#define    RF_INTERFACE_BLOCK4_TOP_TXDACAPB_TXDACTESTDATAI_SHIFT          0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACAPB_TXDACTESTDATAI_MASK           0x000003FF

#define RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_OFFSET                      0x0000000C
#define RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_TYPE                        UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_RESERVED_MASK               0xFC00FC00
#define    RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_TXDACDCOFFSETQ_SHIFT     16
#define    RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_TXDACDCOFFSETQ_MASK      0x03FF0000
#define    RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_TXDACDCOFFSETI_SHIFT     0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACDCOFFSET_TXDACDCOFFSETI_MASK      0x000003FF

#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_OFFSET                         0x00000010
#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_TYPE                           UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_RESERVED_MASK                  0xFFFFF00E
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_TXDACOUTPUTSCL_SHIFT        4
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_TXDACOUTPUTSCL_MASK         0x00000FF0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_TXDAC2TONEEN_SHIFT          0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST1_TXDAC2TONEEN_MASK           0x00000001

#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST2_OFFSET                         0x00000014
#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST2_TYPE                           UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST2_RESERVED_MASK                  0x00000000
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST2_TXDACFCTRLWORD1_SHIFT       0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST2_TXDACFCTRLWORD1_MASK        0xFFFFFFFF

#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST3_OFFSET                         0x00000018
#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST3_TYPE                           UInt32
#define RF_INTERFACE_BLOCK4_TOP_TXDACTEST3_RESERVED_MASK                  0x00000000
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST3_TXDACFCTRLWORD2_SHIFT       0
#define    RF_INTERFACE_BLOCK4_TOP_TXDACTEST3_TXDACFCTRLWORD2_MASK        0xFFFFFFFF

#endif /* __BRCM_RDB_RF_INTERFACE_BLOCK4_TOP_H__ */



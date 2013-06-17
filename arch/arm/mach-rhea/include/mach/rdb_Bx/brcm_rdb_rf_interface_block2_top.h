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

#ifndef __BRCM_RDB_RF_INTERFACE_BLOCK2_TOP_H__
#define __BRCM_RDB_RF_INTERFACE_BLOCK2_TOP_H__

#define RF_INTERFACE_BLOCK2_TOP_CTRL0_OFFSET                              0x00000000
#define RF_INTERFACE_BLOCK2_TOP_CTRL0_TYPE                                UInt32
#define RF_INTERFACE_BLOCK2_TOP_CTRL0_RESERVED_MASK                       0xCCEECFFE
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELDATASEL_SHIFT              28
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELDATASEL_MASK               0x30000000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELOCLKSEL_SHIFT              24
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELOCLKSEL_MASK               0x03000000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELNCLKEN_SHIFT               20
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELNCLKEN_MASK                0x00100000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELPCLKEN_SHIFT               16
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_SIGDELPCLKEN_MASK                0x00010000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_RXADCDATASEL_SHIFT               12
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_RXADCDATASEL_MASK                0x00003000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_RXADCPCLKEN_SHIFT                0
#define    RF_INTERFACE_BLOCK2_TOP_CTRL0_RXADCPCLKEN_MASK                 0x00000001

#define RF_INTERFACE_BLOCK2_TOP_CTRL1_OFFSET                              0x00000004
#define RF_INTERFACE_BLOCK2_TOP_CTRL1_TYPE                                UInt32
#define RF_INTERFACE_BLOCK2_TOP_CTRL1_RESERVED_MASK                       0xFFCEFFCE
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_SIGDELDATAPKMODE_SHIFT           20
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_SIGDELDATAPKMODE_MASK            0x00300000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_SIGDELDATAPKEN_SHIFT             16
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_SIGDELDATAPKEN_MASK              0x00010000
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_RXADCDATAPKMODE_SHIFT            4
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_RXADCDATAPKMODE_MASK             0x00000030
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_RXADCDATAPKEN_SHIFT              0
#define    RF_INTERFACE_BLOCK2_TOP_CTRL1_RXADCDATAPKEN_MASK               0x00000001

#define RF_INTERFACE_BLOCK2_TOP_PACKDATA_OFFSET                           0x00000008
#define RF_INTERFACE_BLOCK2_TOP_PACKDATA_TYPE                             UInt32
#define RF_INTERFACE_BLOCK2_TOP_PACKDATA_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK2_TOP_PACKDATA_RXADCPACKDATA_SHIFT           0
#define    RF_INTERFACE_BLOCK2_TOP_PACKDATA_RXADCPACKDATA_MASK            0xFFFFFFFF

#define RF_INTERFACE_BLOCK2_TOP_SIGDELPACKDATA_OFFSET                     0x0000000C
#define RF_INTERFACE_BLOCK2_TOP_SIGDELPACKDATA_TYPE                       UInt32
#define RF_INTERFACE_BLOCK2_TOP_SIGDELPACKDATA_RESERVED_MASK              0x00000000
#define    RF_INTERFACE_BLOCK2_TOP_SIGDELPACKDATA_SIGDELPACKDATA_SHIFT    0
#define    RF_INTERFACE_BLOCK2_TOP_SIGDELPACKDATA_SIGDELPACKDATA_MASK     0xFFFFFFFF

#endif /* __BRCM_RDB_RF_INTERFACE_BLOCK2_TOP_H__ */



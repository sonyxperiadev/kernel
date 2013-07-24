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

#ifndef __BRCM_RDB_RF_INTERFACE_BLOCK3_TOP_H__
#define __BRCM_RDB_RF_INTERFACE_BLOCK3_TOP_H__

#define RF_INTERFACE_BLOCK3_TOP_DACCFG_OFFSET                             0x00000000
#define RF_INTERFACE_BLOCK3_TOP_DACCFG_TYPE                               UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACCFG_RESERVED_MASK                      0xFFFFCEEE
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACMODE_SHIFT                 12
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACMODE_MASK                  0x00003000
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACTWOS_SHIFT                 8
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACTWOS_MASK                  0x00000100
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACTESTSEL_SHIFT              4
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACTESTSEL_MASK               0x00000010
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACTESTEN_SHIFT               0
#define    RF_INTERFACE_BLOCK3_TOP_DACCFG_GPDACTESTEN_MASK                0x00000001

#define RF_INTERFACE_BLOCK3_TOP_DACDATA_OFFSET                            0x00000004
#define RF_INTERFACE_BLOCK3_TOP_DACDATA_TYPE                              UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACDATA_RESERVED_MASK                     0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACDATA_GPDACDATA_SHIFT                0
#define    RF_INTERFACE_BLOCK3_TOP_DACDATA_GPDACDATA_MASK                 0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACRDDATA_OFFSET                          0x00000008
#define RF_INTERFACE_BLOCK3_TOP_DACRDDATA_TYPE                            UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACRDDATA_RESERVED_MASK                   0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACRDDATA_GPDACRDDATA_SHIFT            0
#define    RF_INTERFACE_BLOCK3_TOP_DACRDDATA_GPDACRDDATA_MASK             0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_OFFSET                         0x00000010
#define RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_TYPE                           UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_RESERVED_MASK                  0xFF80FCCE
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCCYCLE_SHIFT         16
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCCYCLE_MASK          0x007F0000
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCUPDOWN_SHIFT        9
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCUPDOWN_MASK         0x00000200
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCUPDNSEL_SHIFT       8
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCUPDNSEL_MASK        0x00000100
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCSTROBE_SHIFT        5
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCSTROBE_MASK         0x00000020
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCSTBSEL_SHIFT        4
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCSTBSEL_MASK         0x00000010
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCMODE_SHIFT          0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCCTRL_GPDACATCMODE_MASK           0x00000001

#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA0_OFFSET                        0x00000014
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA0_TYPE                          UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA0_RESERVED_MASK                 0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA0_GPDACATCDATA0_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA0_GPDACATCDATA0_MASK         0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA1_OFFSET                        0x00000018
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA1_TYPE                          UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA1_RESERVED_MASK                 0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA1_GPDACATCDATA1_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA1_GPDACATCDATA1_MASK         0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA2_OFFSET                        0x0000001C
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA2_TYPE                          UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA2_RESERVED_MASK                 0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA2_GPDACATCDATA2_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA2_GPDACATCDATA2_MASK         0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA3_OFFSET                        0x00000020
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA3_TYPE                          UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA3_RESERVED_MASK                 0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA3_GPDACATCDATA3_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA3_GPDACATCDATA3_MASK         0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA4_OFFSET                        0x00000024
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA4_TYPE                          UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA4_RESERVED_MASK                 0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA4_GPDACATCDATA4_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA4_GPDACATCDATA4_MASK         0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA5_OFFSET                        0x00000028
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA5_TYPE                          UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCDATA5_RESERVED_MASK                 0xFFFFFC00
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA5_GPDACATCDATA5_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCDATA5_GPDACATCDATA5_MASK         0x000003FF

#define RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_OFFSET                       0x0000002C
#define RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_TYPE                         UInt32
#define RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_RESERVED_MASK                0xFFFFFF8E
#define    RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_GPDACATCDATANO_SHIFT      4
#define    RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_GPDACATCDATANO_MASK       0x00000070
#define    RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_GPDACATCDONE_SHIFT        0
#define    RF_INTERFACE_BLOCK3_TOP_DACATCSTATUS_GPDACATCDONE_MASK         0x00000001

#endif /* __BRCM_RDB_RF_INTERFACE_BLOCK3_TOP_H__ */



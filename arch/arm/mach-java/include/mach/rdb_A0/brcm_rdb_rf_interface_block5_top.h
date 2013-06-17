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

#ifndef __BRCM_RDB_RF_INTERFACE_BLOCK5_TOP_H__
#define __BRCM_RDB_RF_INTERFACE_BLOCK5_TOP_H__

#define RF_INTERFACE_BLOCK5_TOP_CLKCFG_OFFSET                             0x00000000
#define RF_INTERFACE_BLOCK5_TOP_CLKCFG_TYPE                               UInt32
#define RF_INTERFACE_BLOCK5_TOP_CLKCFG_RESERVED_MASK                      0x80800000
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCONFIGCNT_SHIFT             24
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCONFIGCNT_MASK              0x7F000000
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKENDCNT_SHIFT             16
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKENDCNT_MASK              0x007F0000
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKSTARTCNT_SHIFT           9
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKSTARTCNT_MASK            0x0000FE00
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKDIVCNT_SHIFT             6
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKDIVCNT_MASK              0x000001C0
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFSTROBESEL_SHIFT             5
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFSTROBESEL_MASK              0x00000020
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKENDPHASE_SHIFT           4
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKENDPHASE_MASK            0x00000010
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKPHASE_SHIFT              2
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKPHASE_MASK               0x0000000C
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKMODE_SHIFT               1
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFCLKMODE_MASK                0x00000002
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFENABLEIN_SHIFT              0
#define    RF_INTERFACE_BLOCK5_TOP_CLKCFG_TWIFENABLEIN_MASK               0x00000001

#define RF_INTERFACE_BLOCK5_TOP_SWSTROBE_OFFSET                           0x00000004
#define RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_SWSTROBE_RESERVED_MASK                    0xFFFEE00E
#define    RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TWIF3GDIGIRF_SHIFT            16
#define    RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TWIF3GDIGIRF_MASK             0x00010000
#define    RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TWIFATCCOUNT_SHIFT            4
#define    RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TWIFATCCOUNT_MASK             0x00001FF0
#define    RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TWIFSWSTROBE_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_SWSTROBE_TWIFSWSTROBE_MASK             0x00000001

#define RF_INTERFACE_BLOCK5_TOP_ENACFG_OFFSET                             0x00000008
#define RF_INTERFACE_BLOCK5_TOP_ENACFG_TYPE                               UInt32
#define RF_INTERFACE_BLOCK5_TOP_ENACFG_RESERVED_MASK                      0x008080FE
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAENDMODE_SHIFT            31
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAENDMODE_MASK             0x80000000
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAENDCNTRD_SHIFT           24
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAENDCNTRD_MASK            0x7F000000
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAENDCNT_SHIFT             16
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAENDCNT_MASK              0x007F0000
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENASTARTCNT_SHIFT           8
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENASTARTCNT_MASK            0x00007F00
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAPOLARITY_SHIFT           0
#define    RF_INTERFACE_BLOCK5_TOP_ENACFG_TWIFENAPOLARITY_MASK            0x00000001

#define RF_INTERFACE_BLOCK5_TOP_RENACFG_OFFSET                            0x0000000C
#define RF_INTERFACE_BLOCK5_TOP_RENACFG_TYPE                              UInt32
#define RF_INTERFACE_BLOCK5_TOP_RENACFG_RESERVED_MASK                     0xFF8080EE
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENAENDCNT_SHIFT           16
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENAENDCNT_MASK            0x007F0000
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENASTARTCNT_SHIFT         8
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENASTARTCNT_MASK          0x00007F00
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENACTRL_SHIFT             4
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENACTRL_MASK              0x00000010
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENAPOLARITY_SHIFT         0
#define    RF_INTERFACE_BLOCK5_TOP_RENACFG_TWIFRENAPOLARITY_MASK          0x00000001

#define RF_INTERFACE_BLOCK5_TOP_DATACFG_OFFSET                            0x00000010
#define RF_INTERFACE_BLOCK5_TOP_DATACFG_TYPE                              UInt32
#define RF_INTERFACE_BLOCK5_TOP_DATACFG_RESERVED_MASK                     0xFFC0F0EC
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATALENGTH_SHIFT           16
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATALENGTH_MASK            0x003F0000
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFCMDNUMBER_SHIFT            8
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFCMDNUMBER_MASK             0x00000F00
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATAREADMODE_SHIFT         4
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATAREADMODE_MASK          0x00000010
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATAENDPHASE_SHIFT         1
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATAENDPHASE_MASK          0x00000002
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATAORDER_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_DATACFG_TWIFDATAORDER_MASK             0x00000001

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA1_OFFSET                           0x00000014
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA1_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA1_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA1_TWIFCMDDATA1_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA1_TWIFCMDDATA1_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA2_OFFSET                           0x00000018
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA2_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA2_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA2_TWIFCMDDATA2_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA2_TWIFCMDDATA2_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA3_OFFSET                           0x0000001C
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA3_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA3_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA3_TWIFCMDDATA3_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA3_TWIFCMDDATA3_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA4_OFFSET                           0x00000020
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA4_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA4_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA4_TWIFCMDDATA4_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA4_TWIFCMDDATA4_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA5_OFFSET                           0x00000024
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA5_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA5_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA5_TWIFCMDDATA5_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA5_TWIFCMDDATA5_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA6_OFFSET                           0x00000028
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA6_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA6_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA6_TWIFCMDDATA6_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA6_TWIFCMDDATA6_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA7_OFFSET                           0x0000002C
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA7_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA7_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA7_TWIFCMDDATA7_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA7_TWIFCMDDATA7_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CMDDATA8_OFFSET                           0x00000030
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA8_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_CMDDATA8_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA8_TWIFCMDDATA8_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_CMDDATA8_TWIFCMDDATA8_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_CONFIGDONE_OFFSET                         0x00000034
#define RF_INTERFACE_BLOCK5_TOP_CONFIGDONE_TYPE                           UInt32
#define RF_INTERFACE_BLOCK5_TOP_CONFIGDONE_RESERVED_MASK                  0xFFFFFFFE
#define    RF_INTERFACE_BLOCK5_TOP_CONFIGDONE_TWIFCONFIGDONE_SHIFT        0
#define    RF_INTERFACE_BLOCK5_TOP_CONFIGDONE_TWIFCONFIGDONE_MASK         0x00000001

#define RF_INTERFACE_BLOCK5_TOP_READDATA_OFFSET                           0x00000038
#define RF_INTERFACE_BLOCK5_TOP_READDATA_TYPE                             UInt32
#define RF_INTERFACE_BLOCK5_TOP_READDATA_RESERVED_MASK                    0x00000000
#define    RF_INTERFACE_BLOCK5_TOP_READDATA_TWIFREADDATA_SHIFT            0
#define    RF_INTERFACE_BLOCK5_TOP_READDATA_TWIFREADDATA_MASK             0xFFFFFFFF

#define RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_OFFSET                      0x0000003C
#define RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_TYPE                        UInt32
#define RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RESERVED_MASK               0xFFEEEEEE
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPAUTO2_SHIFT       20
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPAUTO2_MASK        0x00100000
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPPOL2_SHIFT        16
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPPOL2_MASK         0x00010000
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPENB2_SHIFT        12
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPENB2_MASK         0x00001000
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPAUTO1_SHIFT       8
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPAUTO1_MASK        0x00000100
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPPOL1_SHIFT        4
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPPOL1_MASK         0x00000010
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPENB1_SHIFT        0
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL1_RFSLEEPENB1_MASK         0x00000001

#define RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_OFFSET                      0x00000040
#define RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_TYPE                        UInt32
#define RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RESERVED_MASK               0xFFEEEEEE
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFAWAKEIN3_SHIFT         20
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFAWAKEIN3_MASK          0x00100000
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFAWAKEIN2_SHIFT         16
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFAWAKEIN2_MASK          0x00010000
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFAWAKEIN1_SHIFT         12
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFAWAKEIN1_MASK          0x00001000
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFSLEEPAUTO3_SHIFT       8
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFSLEEPAUTO3_MASK        0x00000100
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFSLEEPPOL3_SHIFT        4
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFSLEEPPOL3_MASK         0x00000010
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFSLEEPENB3_SHIFT        0
#define    RF_INTERFACE_BLOCK5_TOP_RFSLEEPCNTRL2_RFSLEEPENB3_MASK         0x00000001

#define RF_INTERFACE_BLOCK5_TOP_RFAWAKE_OFFSET                            0x00000044
#define RF_INTERFACE_BLOCK5_TOP_RFAWAKE_TYPE                              UInt32
#define RF_INTERFACE_BLOCK5_TOP_RFAWAKE_RESERVED_MASK                     0xFEFFFFFF
#define    RF_INTERFACE_BLOCK5_TOP_RFAWAKE_RFAWAKE_SHIFT                  24
#define    RF_INTERFACE_BLOCK5_TOP_RFAWAKE_RFAWAKE_MASK                   0x01000000

#endif /* __BRCM_RDB_RF_INTERFACE_BLOCK5_TOP_H__ */



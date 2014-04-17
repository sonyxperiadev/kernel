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

#ifndef __BRCM_RDB_MP_H__
#define __BRCM_RDB_MP_H__

#define MP_MPCNFG_OFFSET                                                  0x00000000
#define MP_MPCNFG_TYPE                                                    UInt32
#define MP_MPCNFG_RESERVED_MASK                                           0x20000000
#define    MP_MPCNFG_ENCDEC_SHIFT                                         31
#define    MP_MPCNFG_ENCDEC_MASK                                          0x80000000
#define    MP_MPCNFG_CODEWORDSPEC_SHIFT                                   30
#define    MP_MPCNFG_CODEWORDSPEC_MASK                                    0x40000000
#define    MP_MPCNFG_COMBINERSLCT_SHIFT                                   28
#define    MP_MPCNFG_COMBINERSLCT_MASK                                    0x10000000
#define    MP_MPCNFG_DECDESC_SHIFT                                        24
#define    MP_MPCNFG_DECDESC_MASK                                         0x0F000000
#define    MP_MPCNFG_IR2_SHIFT                                            18
#define    MP_MPCNFG_IR2_MASK                                             0x00FC0000
#define    MP_MPCNFG_IR1_SHIFT                                            12
#define    MP_MPCNFG_IR1_MASK                                             0x0003F000
#define    MP_MPCNFG_IR0_SHIFT                                            6
#define    MP_MPCNFG_IR0_MASK                                             0x00000FC0
#define    MP_MPCNFG_CURDATADESC_SHIFT                                    0
#define    MP_MPCNFG_CURDATADESC_MASK                                     0x0000003F

#define MP_BFRSETUP_OFFSET                                                0x00000004
#define MP_BFRSETUP_TYPE                                                  UInt32
#define MP_BFRSETUP_RESERVED_MASK                                         0xFFFE0000
#define    MP_BFRSETUP_BFRADDRPTR_SHIFT                                   4
#define    MP_BFRSETUP_BFRADDRPTR_MASK                                    0x0001FFF0
#define    MP_BFRSETUP_BFRSLCT_SHIFT                                      0
#define    MP_BFRSETUP_BFRSLCT_MASK                                       0x0000000F

#define MP_BFRACCS_OFFSET                                                 0x00000008
#define MP_BFRACCS_TYPE                                                   UInt32
#define MP_BFRACCS_RESERVED_MASK                                          0x00000000
#define    MP_BFRACCS_BFRIO_SHIFT                                         0
#define    MP_BFRACCS_BFRIO_MASK                                          0xFFFFFFFF

#define MP_BFRCLKCTRL_OFFSET                                              0x0000000C
#define MP_BFRCLKCTRL_TYPE                                                UInt32
#define MP_BFRCLKCTRL_RESERVED_MASK                                       0xFFFFFFFE
#define    MP_BFRCLKCTRL_MPMEMENABLE_SHIFT                                0
#define    MP_BFRCLKCTRL_MPMEMENABLE_MASK                                 0x00000001

#define MP_MPSTATUS_OFFSET                                                0x00000010
#define MP_MPSTATUS_TYPE                                                  UInt32
#define MP_MPSTATUS_RESERVED_MASK                                         0xFFFF8008
#define    MP_MPSTATUS_SYMERRORCOUNT_SHIFT                                4
#define    MP_MPSTATUS_SYMERRORCOUNT_MASK                                 0x00007FF0
#define    MP_MPSTATUS_ENCBUSY_SHIFT                                      2
#define    MP_MPSTATUS_ENCBUSY_MASK                                       0x00000004
#define    MP_MPSTATUS_DECBUSY_SHIFT                                      1
#define    MP_MPSTATUS_DECBUSY_MASK                                       0x00000002
#define    MP_MPSTATUS_CRCERRORSTATUS_SHIFT                               0
#define    MP_MPSTATUS_CRCERRORSTATUS_MASK                                0x00000001

#define MP_MPRESET_OFFSET                                                 0x00000014
#define MP_MPRESET_TYPE                                                   UInt32
#define MP_MPRESET_RESERVED_MASK                                          0xFFFFFFFF

#define MP_CBNRSCLFACTOR_OFFSET                                           0x00000018
#define MP_CBNRSCLFACTOR_TYPE                                             UInt32
#define MP_CBNRSCLFACTOR_RESERVED_MASK                                    0xC0000000
#define    MP_CBNRSCLFACTOR_IRDATA4C_SHIFT                                25
#define    MP_CBNRSCLFACTOR_IRDATA4C_MASK                                 0x3E000000
#define    MP_CBNRSCLFACTOR_CURDATA4C_SHIFT                               20
#define    MP_CBNRSCLFACTOR_CURDATA4C_MASK                                0x01F00000
#define    MP_CBNRSCLFACTOR_IR2DATA5C_SHIFT                               15
#define    MP_CBNRSCLFACTOR_IR2DATA5C_MASK                                0x000F8000
#define    MP_CBNRSCLFACTOR_IR1DATA5C_SHIFT                               10
#define    MP_CBNRSCLFACTOR_IR1DATA5C_MASK                                0x00007C00
#define    MP_CBNRSCLFACTOR_IR0DATA5C_SHIFT                               5
#define    MP_CBNRSCLFACTOR_IR0DATA5C_MASK                                0x000003E0
#define    MP_CBNRSCLFACTOR_CURDATA5C_SHIFT                               0
#define    MP_CBNRSCLFACTOR_CURDATA5C_MASK                                0x0000001F

#endif /* __BRCM_RDB_MP_H__ */



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

#ifndef __BRCM_RDB_SC_XY_STATE_DERIVE_H__
#define __BRCM_RDB_SC_XY_STATE_DERIVE_H__

#define SC_XY_STATE_DERIVE_SCRAM_XY_IN_OFFSET                             0x00000000
#define SC_XY_STATE_DERIVE_SCRAM_XY_IN_TYPE                               UInt32
#define SC_XY_STATE_DERIVE_SCRAM_XY_IN_RESERVED_MASK                      0xFF800000
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_SYMBOLOFFSET_SHIFT              15
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_SYMBOLOFFSET_MASK               0x007F8000
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_ALTERNATERSEL_SHIFT             14
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_ALTERNATERSEL_MASK              0x00004000
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_ALTERNATELSEL_SHIFT             13
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_ALTERNATELSEL_MASK              0x00002000
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_SCSET_SHIFT                     4
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_SCSET_MASK                      0x00001FF0
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_SCPRIMSECSEL_SHIFT              0
#define    SC_XY_STATE_DERIVE_SCRAM_XY_IN_SCPRIMSECSEL_MASK               0x0000000F

#define SC_XY_STATE_DERIVE_SCRAM_XY_OUTY_OFFSET                           0x00000004
#define SC_XY_STATE_DERIVE_SCRAM_XY_OUTY_TYPE                             UInt32
#define SC_XY_STATE_DERIVE_SCRAM_XY_OUTY_RESERVED_MASK                    0xFFFC0000
#define    SC_XY_STATE_DERIVE_SCRAM_XY_OUTY_SCYSTATEOUT_SHIFT             0
#define    SC_XY_STATE_DERIVE_SCRAM_XY_OUTY_SCYSTATEOUT_MASK              0x0003FFFF

#define SC_XY_STATE_DERIVE_SCRAM_XY_OUTX_OFFSET                           0x00000008
#define SC_XY_STATE_DERIVE_SCRAM_XY_OUTX_TYPE                             UInt32
#define SC_XY_STATE_DERIVE_SCRAM_XY_OUTX_RESERVED_MASK                    0xFFFC0000
#define    SC_XY_STATE_DERIVE_SCRAM_XY_OUTX_SCXSTATEOUT_SHIFT             0
#define    SC_XY_STATE_DERIVE_SCRAM_XY_OUTX_SCXSTATEOUT_MASK              0x0003FFFF

#endif /* __BRCM_RDB_SC_XY_STATE_DERIVE_H__ */



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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_IPCOPEN_H__
#define __BRCM_RDB_IPCOPEN_H__

#define IPCOPEN_IPCASTATUS_OFFSET                                         0x00000000
#define IPCOPEN_IPCASTATUS_TYPE                                           UInt32
#define IPCOPEN_IPCASTATUS_RESERVED_MASK                                  0x00000000
#define    IPCOPEN_IPCASTATUS_ARMINT_SHIFT                                0
#define    IPCOPEN_IPCASTATUS_ARMINT_MASK                                 0xFFFFFFFF

#define IPCOPEN_IPCACLR_OFFSET                                            0x00000004
#define IPCOPEN_IPCACLR_TYPE                                              UInt32
#define IPCOPEN_IPCACLR_RESERVED_MASK                                     0x00000000
#define    IPCOPEN_IPCACLR_ARMINTCLR_SHIFT                                0
#define    IPCOPEN_IPCACLR_ARMINTCLR_MASK                                 0xFFFFFFFF

#define IPCOPEN_IPCASET_OFFSET                                            0x00000008
#define IPCOPEN_IPCASET_TYPE                                              UInt32
#define IPCOPEN_IPCASET_RESERVED_MASK                                     0x00000000
#define    IPCOPEN_IPCASET_VCINTSET_SHIFT                                 0
#define    IPCOPEN_IPCASET_VCINTSET_MASK                                  0xFFFFFFFF

#define IPCOPEN_IPCERR_OFFSET                                             0x00000014
#define IPCOPEN_IPCERR_TYPE                                               UInt32
#define IPCOPEN_IPCERR_RESERVED_MASK                                      0xFFFFFFFC
#define    IPCOPEN_IPCERR_VC_RST_MINI_SHIFT                               1
#define    IPCOPEN_IPCERR_VC_RST_MINI_MASK                                0x00000002
#define    IPCOPEN_IPCERR_VC_RST_SHIFT                                    0
#define    IPCOPEN_IPCERR_VC_RST_MASK                                     0x00000001

#define IPCOPEN_IPCVSTATUS_OFFSET                                         0x00000080
#define IPCOPEN_IPCVSTATUS_TYPE                                           UInt32
#define IPCOPEN_IPCVSTATUS_RESERVED_MASK                                  0x00000000
#define    IPCOPEN_IPCVSTATUS_VCINT_SHIFT                                 0
#define    IPCOPEN_IPCVSTATUS_VCINT_MASK                                  0xFFFFFFFF

#define IPCOPEN_IPCVCLR_OFFSET                                            0x00000084
#define IPCOPEN_IPCVCLR_TYPE                                              UInt32
#define IPCOPEN_IPCVCLR_RESERVED_MASK                                     0x00000000
#define    IPCOPEN_IPCVCLR_VCINTCLR_SHIFT                                 0
#define    IPCOPEN_IPCVCLR_VCINTCLR_MASK                                  0xFFFFFFFF

#define IPCOPEN_IPCVSET_OFFSET                                            0x00000088
#define IPCOPEN_IPCVSET_TYPE                                              UInt32
#define IPCOPEN_IPCVSET_RESERVED_MASK                                     0x00000000
#define    IPCOPEN_IPCVSET_ARMINTSET_SHIFT                                0
#define    IPCOPEN_IPCVSET_ARMINTSET_MASK                                 0xFFFFFFFF

#endif /* __BRCM_RDB_IPCOPEN_H__ */



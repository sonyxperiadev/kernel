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

#ifndef __BRCM_RDB_IPCSEC_H__
#define __BRCM_RDB_IPCSEC_H__

#define IPCSEC_IPCASECURE_OFFSET                                          0x0000000C
#define IPCSEC_IPCASECURE_TYPE                                            UInt32
#define IPCSEC_IPCASECURE_RESERVED_MASK                                   0x00000000
#define    IPCSEC_IPCASECURE_INTSECURE_SHIFT                              0
#define    IPCSEC_IPCASECURE_INTSECURE_MASK                               0xFFFFFFFF

#define IPCSEC_IPCAWAKE_OFFSET                                            0x00000010
#define IPCSEC_IPCAWAKE_TYPE                                              UInt32
#define IPCSEC_IPCAWAKE_RESERVED_MASK                                     0x00000000
#define    IPCSEC_IPCAWAKE_WAKEADDR_SHIFT                                 1
#define    IPCSEC_IPCAWAKE_WAKEADDR_MASK                                  0xFFFFFFFE
#define    IPCSEC_IPCAWAKE_WAKEUP_SHIFT                                   0
#define    IPCSEC_IPCAWAKE_WAKEUP_MASK                                    0x00000001

#define IPCSEC_IPCMAIL0_OFFSET                                            0x00000100
#define IPCSEC_IPCMAIL0_TYPE                                              UInt32
#define IPCSEC_IPCMAIL0_RESERVED_MASK                                     0x00000000
#define    IPCSEC_IPCMAIL0_MAILBOX0_SHIFT                                 0
#define    IPCSEC_IPCMAIL0_MAILBOX0_MASK                                  0xFFFFFFFF

#define IPCSEC_IPCMAIL1_OFFSET                                            0x00000104
#define IPCSEC_IPCMAIL1_TYPE                                              UInt32
#define IPCSEC_IPCMAIL1_RESERVED_MASK                                     0x00000000
#define    IPCSEC_IPCMAIL1_MAILBOX1_SHIFT                                 0
#define    IPCSEC_IPCMAIL1_MAILBOX1_MASK                                  0xFFFFFFFF

#define IPCSEC_IPCMAIL2_OFFSET                                            0x00000108
#define IPCSEC_IPCMAIL2_TYPE                                              UInt32
#define IPCSEC_IPCMAIL2_RESERVED_MASK                                     0x00000000
#define    IPCSEC_IPCMAIL2_MAILBOX2_SHIFT                                 0
#define    IPCSEC_IPCMAIL2_MAILBOX2_MASK                                  0xFFFFFFFF

#define IPCSEC_IPCMAIL3_OFFSET                                            0x0000010C
#define IPCSEC_IPCMAIL3_TYPE                                              UInt32
#define IPCSEC_IPCMAIL3_RESERVED_MASK                                     0x00000000
#define    IPCSEC_IPCMAIL3_MAILBOX3_SHIFT                                 0
#define    IPCSEC_IPCMAIL3_MAILBOX3_MASK                                  0xFFFFFFFF

#endif /* __BRCM_RDB_IPCSEC_H__ */



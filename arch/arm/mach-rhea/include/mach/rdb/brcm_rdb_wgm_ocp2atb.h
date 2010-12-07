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

#ifndef __BRCM_RDB_WGM_OCP2ATB_H__
#define __BRCM_RDB_WGM_OCP2ATB_H__

#define WGM_OCP2ATB_ATBID_REG_OFFSET                                      0x00000000
#define WGM_OCP2ATB_ATBID_REG_TYPE                                        UInt32
#define WGM_OCP2ATB_ATBID_REG_RESERVED_MASK                               0x00000000
#define    WGM_OCP2ATB_ATBID_REG_RESERVED_31TO7_SHIFT                     7
#define    WGM_OCP2ATB_ATBID_REG_RESERVED_31TO7_MASK                      0xFFFFFF80
#define    WGM_OCP2ATB_ATBID_REG_ATBID_SHIFT                              0
#define    WGM_OCP2ATB_ATBID_REG_ATBID_MASK                               0x0000007F

typedef volatile struct {
   UInt32 m_ATBID_REG;               // 0x0000
} BRCM_WGM_OCP2ATB_REGS;


#endif /* __BRCM_RDB_WGM_OCP2ATB_H__ */



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

#ifndef __BRCM_RDB_SPUM_APB_H__
#define __BRCM_RDB_SPUM_APB_H__

#define SPUM_APB_CTRL_OFFSET                                              0x00000000
#define SPUM_APB_CTRL_TYPE                                                UInt32
#define SPUM_APB_CTRL_RESERVED_MASK                                       0x3FFFE7FD
#define    SPUM_APB_CTRL_PEN_MODE_SHIFT                                   30
#define    SPUM_APB_CTRL_PEN_MODE_MASK                                    0xC0000000
#define    SPUM_APB_CTRL_OUT_ENDIAN_SHIFT                                 12
#define    SPUM_APB_CTRL_OUT_ENDIAN_MASK                                  0x00001000
#define    SPUM_APB_CTRL_IN_ENDIAN_SHIFT                                  11
#define    SPUM_APB_CTRL_IN_ENDIAN_MASK                                   0x00000800
#define    SPUM_APB_CTRL_SOFT_RST_SHIFT                                   1
#define    SPUM_APB_CTRL_SOFT_RST_MASK                                    0x00000002

#define SPUM_APB_KEK_CS_OFFSET                                            0x00000010
#define SPUM_APB_KEK_CS_TYPE                                              UInt32
#define SPUM_APB_KEK_CS_RESERVED_MASK                                     0x7F3FFFFF
#define    SPUM_APB_KEK_CS_KEY_CACHE_ERR_SHIFT                            31
#define    SPUM_APB_KEK_CS_KEY_CACHE_ERR_MASK                             0x80000000
#define    SPUM_APB_KEK_CS_KEY_CACHE_EN_SHIFT                             23
#define    SPUM_APB_KEK_CS_KEY_CACHE_EN_MASK                              0x00800000
#define    SPUM_APB_KEK_CS_DISABLE_DOUT_SHIFT                             22
#define    SPUM_APB_KEK_CS_DISABLE_DOUT_MASK                              0x00400000

#endif /* __BRCM_RDB_SPUM_APB_H__ */



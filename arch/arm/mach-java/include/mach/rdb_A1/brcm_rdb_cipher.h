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

#ifndef __BRCM_RDB_CIPHER_H__
#define __BRCM_RDB_CIPHER_H__

#define CIPHER_CONFIG_OFFSET                                              0x00000000
#define CIPHER_CONFIG_TYPE                                                UInt32
#define CIPHER_CONFIG_RESERVED_MASK                                       0xEEECEEEC
#define    CIPHER_CONFIG_BURST_OFF_SHIFT                                  28
#define    CIPHER_CONFIG_BURST_OFF_MASK                                   0x10000000
#define    CIPHER_CONFIG_ENH_CIPHER_MODE_SHIFT                            24
#define    CIPHER_CONFIG_ENH_CIPHER_MODE_MASK                             0x01000000
#define    CIPHER_CONFIG_SNOW_EN_SHIFT                                    20
#define    CIPHER_CONFIG_SNOW_EN_MASK                                     0x00100000
#define    CIPHER_CONFIG_CIPHER_MODE_SHIFT                                16
#define    CIPHER_CONFIG_CIPHER_MODE_MASK                                 0x00030000
#define    CIPHER_CONFIG_SW_RESET_SHIFT                                   12
#define    CIPHER_CONFIG_SW_RESET_MASK                                    0x00001000
#define    CIPHER_CONFIG_CIPHER_INT_CLEAR_SHIFT                           8
#define    CIPHER_CONFIG_CIPHER_INT_CLEAR_MASK                            0x00000100
#define    CIPHER_CONFIG_CIPHER_INT_SHIFT                                 4
#define    CIPHER_CONFIG_CIPHER_INT_MASK                                  0x00000010
#define    CIPHER_CONFIG_ENDIAN_SHIFT                                     1
#define    CIPHER_CONFIG_ENDIAN_MASK                                      0x00000002
#define    CIPHER_CONFIG_START_BIT_SHIFT                                  0
#define    CIPHER_CONFIG_START_BIT_MASK                                   0x00000001

#define CIPHER_PARAM_POINTER_OFFSET                                       0x00000004
#define CIPHER_PARAM_POINTER_TYPE                                         UInt32
#define CIPHER_PARAM_POINTER_RESERVED_MASK                                0x00000000
#define    CIPHER_PARAM_POINTER_PARAM_POINTER_SHIFT                       0
#define    CIPHER_PARAM_POINTER_PARAM_POINTER_MASK                        0xFFFFFFFF

#define CIPHER_KEY_POINTER_OFFSET                                         0x00000008
#define CIPHER_KEY_POINTER_TYPE                                           UInt32
#define CIPHER_KEY_POINTER_RESERVED_MASK                                  0x00000000
#define    CIPHER_KEY_POINTER_CIPHER_KEY_POINTER_SHIFT                    0
#define    CIPHER_KEY_POINTER_CIPHER_KEY_POINTER_MASK                     0xFFFFFFFF

#define CIPHER_MAC_I_OFFSET                                               0x0000000C
#define CIPHER_MAC_I_TYPE                                                 UInt32
#define CIPHER_MAC_I_RESERVED_MASK                                        0x00000000
#define    CIPHER_MAC_I_MAC_I_SHIFT                                       0
#define    CIPHER_MAC_I_MAC_I_MASK                                        0xFFFFFFFF

#endif /* __BRCM_RDB_CIPHER_H__ */



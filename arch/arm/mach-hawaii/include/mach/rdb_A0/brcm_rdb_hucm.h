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
/*     Date     : Generated on 9/25/2012 19:15:19                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_HUCM_H__
#define __BRCM_RDB_HUCM_H__

#define HUCM_DP_CONFIG_OFFSET                                             0x00000000
#define HUCM_DP_CONFIG_TYPE                                               UInt32
#define HUCM_DP_CONFIG_RESERVED_MASK                                      0x0000EFEE
#define    HUCM_DP_CONFIG_DP_NUMPDU_SHIFT                                 16
#define    HUCM_DP_CONFIG_DP_NUMPDU_MASK                                  0xFFFF0000
#define    HUCM_DP_CONFIG_DP_ENDIAN_SHIFT                                 12
#define    HUCM_DP_CONFIG_DP_ENDIAN_MASK                                  0x00001000
#define    HUCM_DP_CONFIG_DP_MODE_SHIFT                                   4
#define    HUCM_DP_CONFIG_DP_MODE_MASK                                    0x00000010
#define    HUCM_DP_CONFIG_DP_START_SHIFT                                  0
#define    HUCM_DP_CONFIG_DP_START_MASK                                   0x00000001

#define HUCM_DP_LIST_OFFSET                                               0x00000004
#define HUCM_DP_LIST_TYPE                                                 UInt32
#define HUCM_DP_LIST_RESERVED_MASK                                        0x00000000
#define    HUCM_DP_LIST_DP_LIST_ADDR_SHIFT                                0
#define    HUCM_DP_LIST_DP_LIST_ADDR_MASK                                 0xFFFFFFFF

#define HUCM_CM_CONFIG_OFFSET                                             0x00000010
#define HUCM_CM_CONFIG_TYPE                                               UInt32
#define HUCM_CM_CONFIG_RESERVED_MASK                                      0x002A0EE0
#define    HUCM_CM_CONFIG_CM_NDDI_SHIFT                                   24
#define    HUCM_CM_CONFIG_CM_NDDI_MASK                                    0xFF000000
#define    HUCM_CM_CONFIG_CM_REL8_SHIFT                                   23
#define    HUCM_CM_CONFIG_CM_REL8_MASK                                    0x00800000
#define    HUCM_CM_CONFIG_CM_HDR0_SHIFT                                   22
#define    HUCM_CM_CONFIG_CM_HDR0_MASK                                    0x00400000
#define    HUCM_CM_CONFIG_CM_START_SHIFT                                  20
#define    HUCM_CM_CONFIG_CM_START_MASK                                   0x00100000
#define    HUCM_CM_CONFIG_CM_LI_ENDIAN_SHIFT                              18
#define    HUCM_CM_CONFIG_CM_LI_ENDIAN_MASK                               0x00040000
#define    HUCM_CM_CONFIG_CM_ENDIAN_SHIFT                                 16
#define    HUCM_CM_CONFIG_CM_ENDIAN_MASK                                  0x00010000
#define    HUCM_CM_CONFIG_INT_MODE_SHIFT                                  15
#define    HUCM_CM_CONFIG_INT_MODE_MASK                                   0x00008000
#define    HUCM_CM_CONFIG_HARQ_MODE_SHIFT                                 14
#define    HUCM_CM_CONFIG_HARQ_MODE_MASK                                  0x00004000
#define    HUCM_CM_CONFIG_CM_MODE_SHIFT                                   12
#define    HUCM_CM_CONFIG_CM_MODE_MASK                                    0x00003000
#define    HUCM_CM_CONFIG_CM_SI_SHIFT                                     8
#define    HUCM_CM_CONFIG_CM_SI_MASK                                      0x00000100
#define    HUCM_CM_CONFIG_CM_NMACES_SHIFT                                 0
#define    HUCM_CM_CONFIG_CM_NMACES_MASK                                  0x0000001F

#define HUCM_CM_LIST_OFFSET                                               0x00000014
#define HUCM_CM_LIST_TYPE                                                 UInt32
#define HUCM_CM_LIST_RESERVED_MASK                                        0x00000000
#define    HUCM_CM_LIST_CM_LIST_ADDR_SHIFT                                0
#define    HUCM_CM_LIST_CM_LIST_ADDR_MASK                                 0xFFFFFFFF

#define HUCM_CM_DEST_OFFSET                                               0x00000018
#define HUCM_CM_DEST_TYPE                                                 UInt32
#define HUCM_CM_DEST_RESERVED_MASK                                        0xFFFFE000
#define    HUCM_CM_DEST_CM_HARQ_ADDR_SHIFT                                0
#define    HUCM_CM_DEST_CM_HARQ_ADDR_MASK                                 0x00001FFF

#define HUCM_CM_DDI_OFFSET                                                0x0000001C
#define HUCM_CM_DDI_TYPE                                                  UInt32
#define HUCM_CM_DDI_RESERVED_MASK                                         0x00000000
#define    HUCM_CM_DDI_CM_DDI_ADDR_SHIFT                                  0
#define    HUCM_CM_DDI_CM_DDI_ADDR_MASK                                   0xFFFFFFFF

#define HUCM_CM_KEY_OFFSET                                                0x00000020
#define HUCM_CM_KEY_TYPE                                                  UInt32
#define HUCM_CM_KEY_RESERVED_MASK                                         0x00000000
#define    HUCM_CM_KEY_CM_CIPHER_ADDR_SHIFT                               0
#define    HUCM_CM_KEY_CM_CIPHER_ADDR_MASK                                0xFFFFFFFF

#define HUCM_CM_STATUS_OFFSET                                             0x00000024
#define HUCM_CM_STATUS_TYPE                                               UInt32
#define HUCM_CM_STATUS_RESERVED_MASK                                      0xFFE00000
#define    HUCM_CM_STATUS_CM_PRESENT_STATE_SHIFT                          16
#define    HUCM_CM_STATUS_CM_PRESENT_STATE_MASK                           0x001F0000
#define    HUCM_CM_STATUS_CM_BIT_CNT_SHIFT                                0
#define    HUCM_CM_STATUS_CM_BIT_CNT_MASK                                 0x0000FFFF

#define HUCM_CM_SI_DATA_OFFSET                                            0x00000028
#define HUCM_CM_SI_DATA_TYPE                                              UInt32
#define HUCM_CM_SI_DATA_RESERVED_MASK                                     0xFFFC0000
#define    HUCM_CM_SI_DATA_CM_SI_DATA_SHIFT                               0
#define    HUCM_CM_SI_DATA_CM_SI_DATA_MASK                                0x0003FFFF

#define HUCM_CM_SI_PAD_OFFSET                                             0x0000002C
#define HUCM_CM_SI_PAD_TYPE                                               UInt32
#define HUCM_CM_SI_PAD_RESERVED_MASK                                      0xFFFFF000
#define    HUCM_CM_SI_PAD_CM_SIPAD_SHIFT                                  0
#define    HUCM_CM_SI_PAD_CM_SIPAD_MASK                                   0x00000FFF

#define HUCM_CM_ERNTI_OFFSET                                              0x00000030
#define HUCM_CM_ERNTI_TYPE                                                UInt32
#define HUCM_CM_ERNTI_RESERVED_MASK                                       0x00000000
#define    HUCM_CM_ERNTI_CM_ERNTI_SHIFT                                   0
#define    HUCM_CM_ERNTI_CM_ERNTI_MASK                                    0xFFFFFFFF

#endif /* __BRCM_RDB_HUCM_H__ */



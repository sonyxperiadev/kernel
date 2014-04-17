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

#ifndef __BRCM_RDB_DATAPACKER_H__
#define __BRCM_RDB_DATAPACKER_H__

#define DATAPACKER_DATA_PACK_CONFIG_OFFSET                                0x00000000
#define DATAPACKER_DATA_PACK_CONFIG_TYPE                                  UInt32
#define DATAPACKER_DATA_PACK_CONFIG_RESERVED_MASK                         0xFE00FFFE
#define    DATAPACKER_DATA_PACK_CONFIG_INTERRUPT_STATUS_SHIFT             24
#define    DATAPACKER_DATA_PACK_CONFIG_INTERRUPT_STATUS_MASK              0x01000000
#define    DATAPACKER_DATA_PACK_CONFIG_NG_SHIFT                           16
#define    DATAPACKER_DATA_PACK_CONFIG_NG_MASK                            0x00FF0000
#define    DATAPACKER_DATA_PACK_CONFIG_START_BIT_SHIFT                    0
#define    DATAPACKER_DATA_PACK_CONFIG_START_BIT_MASK                     0x00000001

#define DATAPACKER_SOURCE_ADDRESS_OFFSET                                  0x00000004
#define DATAPACKER_SOURCE_ADDRESS_TYPE                                    UInt32
#define DATAPACKER_SOURCE_ADDRESS_RESERVED_MASK                           0x00000000
#define    DATAPACKER_SOURCE_ADDRESS_SOURCE_ADDRESS_SHIFT                 0
#define    DATAPACKER_SOURCE_ADDRESS_SOURCE_ADDRESS_MASK                  0xFFFFFFFF

#define DATAPACKER_DESTINATION_ADDRESS_OFFSET                             0x00000008
#define DATAPACKER_DESTINATION_ADDRESS_TYPE                               UInt32
#define DATAPACKER_DESTINATION_ADDRESS_RESERVED_MASK                      0x00000000
#define    DATAPACKER_DESTINATION_ADDRESS_DESTINATION_ADDRESS_SHIFT       0
#define    DATAPACKER_DESTINATION_ADDRESS_DESTINATION_ADDRESS_MASK        0xFFFFFFFF

#endif /* __BRCM_RDB_DATAPACKER_H__ */



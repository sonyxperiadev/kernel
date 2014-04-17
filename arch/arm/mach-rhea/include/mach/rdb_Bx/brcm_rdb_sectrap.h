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

#ifndef __BRCM_RDB_SECTRAP_H__
#define __BRCM_RDB_SECTRAP_H__

#define SECTRAP_TRAP_CONFIG_OFFSET                                        0x00000000
#define SECTRAP_TRAP_CONFIG_TYPE                                          UInt32
#define SECTRAP_TRAP_CONFIG_RESERVED_MASK                                 0xFFFFFFF8
#define    SECTRAP_TRAP_CONFIG_VIO_WROVF_INTR_EN_SHIFT                    2
#define    SECTRAP_TRAP_CONFIG_VIO_WROVF_INTR_EN_MASK                     0x00000004
#define    SECTRAP_TRAP_CONFIG_VIO_RDOVF_INTR_EN_SHIFT                    1
#define    SECTRAP_TRAP_CONFIG_VIO_RDOVF_INTR_EN_MASK                     0x00000002
#define    SECTRAP_TRAP_CONFIG_VIO_ERR_INTR_EN_SHIFT                      0
#define    SECTRAP_TRAP_CONFIG_VIO_ERR_INTR_EN_MASK                       0x00000001

#define SECTRAP_TRAP_STATUS_OFFSET                                        0x00000004
#define SECTRAP_TRAP_STATUS_TYPE                                          UInt32
#define SECTRAP_TRAP_STATUS_RESERVED_MASK                                 0x00F0FFF8
#define    SECTRAP_TRAP_STATUS_VIO_ID_SHIFT                               24
#define    SECTRAP_TRAP_STATUS_VIO_ID_MASK                                0xFF000000
#define    SECTRAP_TRAP_STATUS_VIO_PROT_SHIFT                             17
#define    SECTRAP_TRAP_STATUS_VIO_PROT_MASK                              0x000E0000
#define    SECTRAP_TRAP_STATUS_VIO_WRITE_SHIFT                            16
#define    SECTRAP_TRAP_STATUS_VIO_WRITE_MASK                             0x00010000
#define    SECTRAP_TRAP_STATUS_VIO_WROVF_INTR_SHIFT                       2
#define    SECTRAP_TRAP_STATUS_VIO_WROVF_INTR_MASK                        0x00000004
#define    SECTRAP_TRAP_STATUS_VIO_RDOVF_INTR_SHIFT                       1
#define    SECTRAP_TRAP_STATUS_VIO_RDOVF_INTR_MASK                        0x00000002
#define    SECTRAP_TRAP_STATUS_VIO_ERR_INTR_SHIFT                         0
#define    SECTRAP_TRAP_STATUS_VIO_ERR_INTR_MASK                          0x00000001

#define SECTRAP_TRAP_ADDRESS_OFFSET                                       0x00000008
#define SECTRAP_TRAP_ADDRESS_TYPE                                         UInt32
#define SECTRAP_TRAP_ADDRESS_RESERVED_MASK                                0x00000000
#define    SECTRAP_TRAP_ADDRESS_VIO_ADDR_SHIFT                            0
#define    SECTRAP_TRAP_ADDRESS_VIO_ADDR_MASK                             0xFFFFFFFF

#endif /* __BRCM_RDB_SECTRAP_H__ */



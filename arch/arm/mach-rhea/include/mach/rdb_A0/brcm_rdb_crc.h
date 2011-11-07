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

#ifndef __BRCM_RDB_CRC_H__
#define __BRCM_RDB_CRC_H__

#define CRC_CS_OFFSET                                                     0x00000000
#define CRC_CS_TYPE                                                       UInt32
#define CRC_CS_RESERVED_MASK                                              0xFFFCFF00
#define    CRC_CS_CRCE_SHIFT                                              17
#define    CRC_CS_CRCE_MASK                                               0x00020000
#define    CRC_CS_CRCD_SHIFT                                              16
#define    CRC_CS_CRCD_MASK                                               0x00010000
#define    CRC_CS_CHKI_SHIFT                                              7
#define    CRC_CS_CHKI_MASK                                               0x00000080
#define    CRC_CS_CHKT_SHIFT                                              6
#define    CRC_CS_CHKT_MASK                                               0x00000040
#define    CRC_CS_CHKW_SHIFT                                              5
#define    CRC_CS_CHKW_MASK                                               0x00000020
#define    CRC_CS_BITOD_SHIFT                                             4
#define    CRC_CS_BITOD_MASK                                              0x00000010
#define    CRC_CS_BYTOD_SHIFT                                             3
#define    CRC_CS_BYTOD_MASK                                              0x00000008
#define    CRC_CS_CHKM_SHIFT                                              2
#define    CRC_CS_CHKM_MASK                                               0x00000004
#define    CRC_CS_CRCM_SHIFT                                              1
#define    CRC_CS_CRCM_MASK                                               0x00000002
#define    CRC_CS_START_SHIFT                                             0
#define    CRC_CS_START_MASK                                              0x00000001

#define CRC_POLY_OFFSET                                                   0x00000004
#define CRC_POLY_TYPE                                                     UInt32
#define CRC_POLY_RESERVED_MASK                                            0x00000000
#define    CRC_POLY_CRCPOLY_SHIFT                                         0
#define    CRC_POLY_CRCPOLY_MASK                                          0xFFFFFFFF

#define CRC_INIT_OFFSET                                                   0x00000008
#define CRC_INIT_TYPE                                                     UInt32
#define CRC_INIT_RESERVED_MASK                                            0x00000000
#define    CRC_INIT_CRCINIT_SHIFT                                         0
#define    CRC_INIT_CRCINIT_MASK                                          0xFFFFFFFF

#define CRC_RESULT_OFFSET                                                 0x0000000C
#define CRC_RESULT_TYPE                                                   UInt32
#define CRC_RESULT_RESERVED_MASK                                          0x00000000
#define    CRC_RESULT_CRC_SHIFT                                           0
#define    CRC_RESULT_CRC_MASK                                            0xFFFFFFFF

#define CRC_DPORT_OFFSET                                                  0x00000010
#define CRC_DPORT_TYPE                                                    UInt32
#define CRC_DPORT_RESERVED_MASK                                           0x00000000
#define    CRC_DPORT_DPORT_SHIFT                                          0
#define    CRC_DPORT_DPORT_MASK                                           0xFFFFFFFF

#endif /* __BRCM_RDB_CRC_H__ */



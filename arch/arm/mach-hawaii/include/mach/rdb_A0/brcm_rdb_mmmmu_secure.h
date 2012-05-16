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
/*     Date     : Generated on 4/25/2012 23:31:7                                             */
/*     RDB file : //HAWAII/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_MMMMU_SECURE_H__
#define __BRCM_RDB_MMMMU_SECURE_H__

#define MMMMU_SECURE_CR_OFFSET                                            0x00000000
#define MMMMU_SECURE_CR_TYPE                                              UInt32
#define MMMMU_SECURE_CR_RESERVED_MASK                                     0xFFFFF888
#define    MMMMU_SECURE_CR_NONSEC_PT_PDRD_PROT_SHIFT                      8
#define    MMMMU_SECURE_CR_NONSEC_PT_PDRD_PROT_MASK                       0x00000700
#define    MMMMU_SECURE_CR_SEC_PT_PDRD_PROT_SHIFT                         4
#define    MMMMU_SECURE_CR_SEC_PT_PDRD_PROT_MASK                          0x00000070
#define    MMMMU_SECURE_CR_SEC_OVLY_ENABLE_SHIFT                          2
#define    MMMMU_SECURE_CR_SEC_OVLY_ENABLE_MASK                           0x00000004
#define    MMMMU_SECURE_CR_HUM_ENABLE_SHIFT                               1
#define    MMMMU_SECURE_CR_HUM_ENABLE_MASK                                0x00000002
#define    MMMMU_SECURE_CR_ENABLE_SHIFT                                   0
#define    MMMMU_SECURE_CR_ENABLE_MASK                                    0x00000001

#define MMMMU_SECURE_ISR_OFFSET                                           0x00000010
#define MMMMU_SECURE_ISR_TYPE                                             UInt32
#define MMMMU_SECURE_ISR_RESERVED_MASK                                    0xFFFFFFFE
#define    MMMMU_SECURE_ISR_EXFIFO_NOT_EMPTY_SHIFT                        0
#define    MMMMU_SECURE_ISR_EXFIFO_NOT_EMPTY_MASK                         0x00000001

#define MMMMU_SECURE_IMR_OFFSET                                           0x00000014
#define MMMMU_SECURE_IMR_TYPE                                             UInt32
#define MMMMU_SECURE_IMR_RESERVED_MASK                                    0xFFFFFFFE
#define    MMMMU_SECURE_IMR_EXFIFO_NOT_EMPTY_SHIFT                        0
#define    MMMMU_SECURE_IMR_EXFIFO_NOT_EMPTY_MASK                         0x00000001

#define MMMMU_SECURE_TBR_OFFSET                                           0x00000020
#define MMMMU_SECURE_TBR_TYPE                                             UInt32
#define MMMMU_SECURE_TBR_RESERVED_MASK                                    0x0000001F
#define    MMMMU_SECURE_TBR_ADDRESS_SHIFT                                 5
#define    MMMMU_SECURE_TBR_ADDRESS_MASK                                  0xFFFFFFE0

#define MMMMU_SECURE_SOSR_OFFSET                                          0x00000024
#define MMMMU_SECURE_SOSR_TYPE                                            UInt32
#define MMMMU_SECURE_SOSR_RESERVED_MASK                                   0x00000FFF
#define    MMMMU_SECURE_SOSR_ADDRESS_SHIFT                                12
#define    MMMMU_SECURE_SOSR_ADDRESS_MASK                                 0xFFFFF000

#define MMMMU_SECURE_SOER_OFFSET                                          0x00000028
#define MMMMU_SECURE_SOER_TYPE                                            UInt32
#define MMMMU_SECURE_SOER_RESERVED_MASK                                   0x00000FFF
#define    MMMMU_SECURE_SOER_ADDRESS_SHIFT                                12
#define    MMMMU_SECURE_SOER_ADDRESS_MASK                                 0xFFFFF000

#define MMMMU_SECURE_XFIFO_OFFSET                                         0x00000050
#define MMMMU_SECURE_XFIFO_TYPE                                           UInt32
#define MMMMU_SECURE_XFIFO_RESERVED_MASK                                  0x00000000
#define    MMMMU_SECURE_XFIFO_ADDRESS_SHIFT                               12
#define    MMMMU_SECURE_XFIFO_ADDRESS_MASK                                0xFFFFF000
#define    MMMMU_SECURE_XFIFO_ID_SHIFT                                    4
#define    MMMMU_SECURE_XFIFO_ID_MASK                                     0x00000FF0
#define    MMMMU_SECURE_XFIFO_RW_SHIFT                                    3
#define    MMMMU_SECURE_XFIFO_RW_MASK                                     0x00000008
#define    MMMMU_SECURE_XFIFO_REASON_SHIFT                                2
#define    MMMMU_SECURE_XFIFO_REASON_MASK                                 0x00000004
#define    MMMMU_SECURE_XFIFO_OVFL_SHIFT                                  1
#define    MMMMU_SECURE_XFIFO_OVFL_MASK                                   0x00000002
#define    MMMMU_SECURE_XFIFO_VALID_SHIFT                                 0
#define    MMMMU_SECURE_XFIFO_VALID_MASK                                  0x00000001

#define MMMMU_SECURE_CAMA_OFFSET                                          0x00000100
#define MMMMU_SECURE_CAMA_TYPE                                            UInt32
#define MMMMU_SECURE_CAMA_RESERVED_MASK                                   0x7FFFFF80
#define    MMMMU_SECURE_CAMA_WRITE_SHIFT                                  31
#define    MMMMU_SECURE_CAMA_WRITE_MASK                                   0x80000000
#define    MMMMU_SECURE_CAMA_ADDRESS_SHIFT                                0
#define    MMMMU_SECURE_CAMA_ADDRESS_MASK                                 0x0000007F

#define MMMMU_SECURE_CAMD_OFFSET                                          0x00000104
#define MMMMU_SECURE_CAMD_TYPE                                            UInt32
#define MMMMU_SECURE_CAMD_RESERVED_MASK                                   0x00000000
#define    MMMMU_SECURE_CAMD_DATA_SHIFT                                   0
#define    MMMMU_SECURE_CAMD_DATA_MASK                                    0xFFFFFFFF

#define MMMMU_SECURE_RAMA_OFFSET                                          0x00000110
#define MMMMU_SECURE_RAMA_TYPE                                            UInt32
#define MMMMU_SECURE_RAMA_RESERVED_MASK                                   0x7FFFFC00
#define    MMMMU_SECURE_RAMA_WRITE_SHIFT                                  31
#define    MMMMU_SECURE_RAMA_WRITE_MASK                                   0x80000000
#define    MMMMU_SECURE_RAMA_ADDRESS_SHIFT                                0
#define    MMMMU_SECURE_RAMA_ADDRESS_MASK                                 0x000003FF

#define MMMMU_SECURE_RAMD_OFFSET                                          0x00000114
#define MMMMU_SECURE_RAMD_TYPE                                            UInt32
#define MMMMU_SECURE_RAMD_RESERVED_MASK                                   0x00000000
#define    MMMMU_SECURE_RAMD_DATA_SHIFT                                   0
#define    MMMMU_SECURE_RAMD_DATA_MASK                                    0xFFFFFFFF

#endif /* __BRCM_RDB_MMMMU_SECURE_H__ */



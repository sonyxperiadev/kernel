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

#ifndef __BRCM_RDB_MMMMU_SECURE_H__
#define __BRCM_RDB_MMMMU_SECURE_H__

#define MMMMU_SECURE_CR_OFFSET                                            0x00000000
#define MMMMU_SECURE_CR_TYPE                                              UInt32
#define MMMMU_SECURE_CR_RESERVED_MASK                                     0xFFFFC888
#define    MMMMU_SECURE_CR_HALT_ON_ERROR_ENABLE_SHIFT                     13
#define    MMMMU_SECURE_CR_HALT_ON_ERROR_ENABLE_MASK                      0x00002000
#define    MMMMU_SECURE_CR_HALT_NOW_ENABLE_SHIFT                          12
#define    MMMMU_SECURE_CR_HALT_NOW_ENABLE_MASK                           0x00001000
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
#define MMMMU_SECURE_SOSR_RESERVED_MASK                                   0x00007FFF
#define    MMMMU_SECURE_SOSR_ADDRESS_SHIFT                                15
#define    MMMMU_SECURE_SOSR_ADDRESS_MASK                                 0xFFFF8000

#define MMMMU_SECURE_SOER_OFFSET                                          0x00000028
#define MMMMU_SECURE_SOER_TYPE                                            UInt32
#define MMMMU_SECURE_SOER_RESERVED_MASK                                   0x00007FFF
#define    MMMMU_SECURE_SOER_ADDRESS_SHIFT                                15
#define    MMMMU_SECURE_SOER_ADDRESS_MASK                                 0xFFFF8000

#define MMMMU_SECURE_XFIFO_OFFSET                                         0x00000050
#define MMMMU_SECURE_XFIFO_TYPE                                           UInt32
#define MMMMU_SECURE_XFIFO_RESERVED_MASK                                  0x00000000
#define    MMMMU_SECURE_XFIFO_ADDRESS_SHIFT                               12
#define    MMMMU_SECURE_XFIFO_ADDRESS_MASK                                0xFFFFF000
#define    MMMMU_SECURE_XFIFO_ID_SHIFT                                    6
#define    MMMMU_SECURE_XFIFO_ID_MASK                                     0x00000FC0
#define    MMMMU_SECURE_XFIFO_RW_SHIFT                                    5
#define    MMMMU_SECURE_XFIFO_RW_MASK                                     0x00000020
#define    MMMMU_SECURE_XFIFO_REASON_SHIFT                                2
#define    MMMMU_SECURE_XFIFO_REASON_MASK                                 0x0000001C
#define    MMMMU_SECURE_XFIFO_OVFL_SHIFT                                  1
#define    MMMMU_SECURE_XFIFO_OVFL_MASK                                   0x00000002
#define    MMMMU_SECURE_XFIFO_VALID_SHIFT                                 0
#define    MMMMU_SECURE_XFIFO_VALID_MASK                                  0x00000001

#define MMMMU_SECURE_ACL0_OFFSET                                          0x00000080
#define MMMMU_SECURE_ACL0_TYPE                                            UInt32
#define MMMMU_SECURE_ACL0_RESERVED_MASK                                   0x00000000
#define    MMMMU_SECURE_ACL0_ACL_ID_F_SHIFT                               30
#define    MMMMU_SECURE_ACL0_ACL_ID_F_MASK                                0xC0000000
#define    MMMMU_SECURE_ACL0_ACL_ID_E_SHIFT                               28
#define    MMMMU_SECURE_ACL0_ACL_ID_E_MASK                                0x30000000
#define    MMMMU_SECURE_ACL0_ACL_ID_D_SHIFT                               26
#define    MMMMU_SECURE_ACL0_ACL_ID_D_MASK                                0x0C000000
#define    MMMMU_SECURE_ACL0_ACL_ID_C_SHIFT                               24
#define    MMMMU_SECURE_ACL0_ACL_ID_C_MASK                                0x03000000
#define    MMMMU_SECURE_ACL0_ACL_ID_B_SHIFT                               22
#define    MMMMU_SECURE_ACL0_ACL_ID_B_MASK                                0x00C00000
#define    MMMMU_SECURE_ACL0_ACL_ID_A_SHIFT                               20
#define    MMMMU_SECURE_ACL0_ACL_ID_A_MASK                                0x00300000
#define    MMMMU_SECURE_ACL0_ACL_ID_9_SHIFT                               18
#define    MMMMU_SECURE_ACL0_ACL_ID_9_MASK                                0x000C0000
#define    MMMMU_SECURE_ACL0_ACL_ID_8_SHIFT                               16
#define    MMMMU_SECURE_ACL0_ACL_ID_8_MASK                                0x00030000
#define    MMMMU_SECURE_ACL0_ACL_ID_7_SHIFT                               14
#define    MMMMU_SECURE_ACL0_ACL_ID_7_MASK                                0x0000C000
#define    MMMMU_SECURE_ACL0_ACL_ID_6_SHIFT                               12
#define    MMMMU_SECURE_ACL0_ACL_ID_6_MASK                                0x00003000
#define    MMMMU_SECURE_ACL0_ACL_ID_5_SHIFT                               10
#define    MMMMU_SECURE_ACL0_ACL_ID_5_MASK                                0x00000C00
#define    MMMMU_SECURE_ACL0_ACL_ID_4_SHIFT                               8
#define    MMMMU_SECURE_ACL0_ACL_ID_4_MASK                                0x00000300
#define    MMMMU_SECURE_ACL0_ACL_ID_3_SHIFT                               6
#define    MMMMU_SECURE_ACL0_ACL_ID_3_MASK                                0x000000C0
#define    MMMMU_SECURE_ACL0_ACL_ID_2_SHIFT                               4
#define    MMMMU_SECURE_ACL0_ACL_ID_2_MASK                                0x00000030
#define    MMMMU_SECURE_ACL0_ACL_ID_1_SHIFT                               2
#define    MMMMU_SECURE_ACL0_ACL_ID_1_MASK                                0x0000000C
#define    MMMMU_SECURE_ACL0_ACL_ID_0_SHIFT                               0
#define    MMMMU_SECURE_ACL0_ACL_ID_0_MASK                                0x00000003

#define MMMMU_SECURE_ACL1_OFFSET                                          0x00000084
#define MMMMU_SECURE_ACL1_TYPE                                            UInt32
#define MMMMU_SECURE_ACL1_RESERVED_MASK                                   0x00000000
#define    MMMMU_SECURE_ACL1_ACL_ID_1F_SHIFT                              30
#define    MMMMU_SECURE_ACL1_ACL_ID_1F_MASK                               0xC0000000
#define    MMMMU_SECURE_ACL1_ACL_ID_1E_SHIFT                              28
#define    MMMMU_SECURE_ACL1_ACL_ID_1E_MASK                               0x30000000
#define    MMMMU_SECURE_ACL1_ACL_ID_1D_SHIFT                              26
#define    MMMMU_SECURE_ACL1_ACL_ID_1D_MASK                               0x0C000000
#define    MMMMU_SECURE_ACL1_ACL_ID_1C_SHIFT                              24
#define    MMMMU_SECURE_ACL1_ACL_ID_1C_MASK                               0x03000000
#define    MMMMU_SECURE_ACL1_ACL_ID_1B_SHIFT                              22
#define    MMMMU_SECURE_ACL1_ACL_ID_1B_MASK                               0x00C00000
#define    MMMMU_SECURE_ACL1_ACL_ID_1A_SHIFT                              20
#define    MMMMU_SECURE_ACL1_ACL_ID_1A_MASK                               0x00300000
#define    MMMMU_SECURE_ACL1_ACL_ID_19_SHIFT                              18
#define    MMMMU_SECURE_ACL1_ACL_ID_19_MASK                               0x000C0000
#define    MMMMU_SECURE_ACL1_ACL_ID_18_SHIFT                              16
#define    MMMMU_SECURE_ACL1_ACL_ID_18_MASK                               0x00030000
#define    MMMMU_SECURE_ACL1_ACL_ID_17_SHIFT                              14
#define    MMMMU_SECURE_ACL1_ACL_ID_17_MASK                               0x0000C000
#define    MMMMU_SECURE_ACL1_ACL_ID_16_SHIFT                              12
#define    MMMMU_SECURE_ACL1_ACL_ID_16_MASK                               0x00003000
#define    MMMMU_SECURE_ACL1_ACL_ID_15_SHIFT                              10
#define    MMMMU_SECURE_ACL1_ACL_ID_15_MASK                               0x00000C00
#define    MMMMU_SECURE_ACL1_ACL_ID_14_SHIFT                              8
#define    MMMMU_SECURE_ACL1_ACL_ID_14_MASK                               0x00000300
#define    MMMMU_SECURE_ACL1_ACL_ID_13_SHIFT                              6
#define    MMMMU_SECURE_ACL1_ACL_ID_13_MASK                               0x000000C0
#define    MMMMU_SECURE_ACL1_ACL_ID_12_SHIFT                              4
#define    MMMMU_SECURE_ACL1_ACL_ID_12_MASK                               0x00000030
#define    MMMMU_SECURE_ACL1_ACL_ID_11_SHIFT                              2
#define    MMMMU_SECURE_ACL1_ACL_ID_11_MASK                               0x0000000C
#define    MMMMU_SECURE_ACL1_ACL_ID_10_SHIFT                              0
#define    MMMMU_SECURE_ACL1_ACL_ID_10_MASK                               0x00000003

#define MMMMU_SECURE_QWACL0_OFFSET                                        0x00000090
#define MMMMU_SECURE_QWACL0_TYPE                                          UInt32
#define MMMMU_SECURE_QWACL0_RESERVED_MASK                                 0x0000FFFC
#define    MMMMU_SECURE_QWACL0_IDEN_F_DOWNTO_0_SHIFT                      16
#define    MMMMU_SECURE_QWACL0_IDEN_F_DOWNTO_0_MASK                       0xFFFF0000
#define    MMMMU_SECURE_QWACL0_IDVAL_SHIFT                                0
#define    MMMMU_SECURE_QWACL0_IDVAL_MASK                                 0x00000003

#define MMMMU_SECURE_QWACL1_OFFSET                                        0x00000094
#define MMMMU_SECURE_QWACL1_TYPE                                          UInt32
#define MMMMU_SECURE_QWACL1_RESERVED_MASK                                 0x0000FFFC
#define    MMMMU_SECURE_QWACL1_IDEN_1F_DOWNTO_10_SHIFT                    16
#define    MMMMU_SECURE_QWACL1_IDEN_1F_DOWNTO_10_MASK                     0xFFFF0000
#define    MMMMU_SECURE_QWACL1_IDVAL_SHIFT                                0
#define    MMMMU_SECURE_QWACL1_IDVAL_MASK                                 0x00000003

#define MMMMU_SECURE_ACLMASK_OFFSET                                       0x000000A0
#define MMMMU_SECURE_ACLMASK_TYPE                                         UInt32
#define MMMMU_SECURE_ACLMASK_RESERVED_MASK                                0xFFFFFFE0
#define    MMMMU_SECURE_ACLMASK_ACLMASK_SHIFT                             0
#define    MMMMU_SECURE_ACLMASK_ACLMASK_MASK                              0x0000001F

#define MMMMU_SECURE_RAMCAMD_OFFSET                                       0x00000100
#define MMMMU_SECURE_RAMCAMD_TYPE                                         UInt32
#define MMMMU_SECURE_RAMCAMD_RESERVED_MASK                                0x00000000
#define    MMMMU_SECURE_RAMCAMD_DATA_SHIFT                                0
#define    MMMMU_SECURE_RAMCAMD_DATA_MASK                                 0xFFFFFFFF

#define MMMMU_SECURE_CAMA_OFFSET                                          0x00000110
#define MMMMU_SECURE_CAMA_TYPE                                            UInt32
#define MMMMU_SECURE_CAMA_RESERVED_MASK                                   0x7FFFFF80
#define    MMMMU_SECURE_CAMA_WRITE_SHIFT                                  31
#define    MMMMU_SECURE_CAMA_WRITE_MASK                                   0x80000000
#define    MMMMU_SECURE_CAMA_ADDRESS_SHIFT                                0
#define    MMMMU_SECURE_CAMA_ADDRESS_MASK                                 0x0000007F

#define MMMMU_SECURE_RAMA_OFFSET                                          0x00000120
#define MMMMU_SECURE_RAMA_TYPE                                            UInt32
#define MMMMU_SECURE_RAMA_RESERVED_MASK                                   0x7FFFFC00
#define    MMMMU_SECURE_RAMA_WRITE_SHIFT                                  31
#define    MMMMU_SECURE_RAMA_WRITE_MASK                                   0x80000000
#define    MMMMU_SECURE_RAMA_ADDRESS_SHIFT                                0
#define    MMMMU_SECURE_RAMA_ADDRESS_MASK                                 0x000003FF

#endif /* __BRCM_RDB_MMMMU_SECURE_H__ */



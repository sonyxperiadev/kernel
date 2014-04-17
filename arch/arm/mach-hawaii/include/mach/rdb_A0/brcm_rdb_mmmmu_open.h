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

#ifndef __BRCM_RDB_MMMMU_OPEN_H__
#define __BRCM_RDB_MMMMU_OPEN_H__

#define MMMMU_OPEN_CR_OFFSET                                              0x00000000
#define MMMMU_OPEN_CR_TYPE                                                UInt32
#define MMMMU_OPEN_CR_RESERVED_MASK                                       0xFFFFFFFF

#define MMMMU_OPEN_ISR_OFFSET                                             0x00000010
#define MMMMU_OPEN_ISR_TYPE                                               UInt32
#define MMMMU_OPEN_ISR_RESERVED_MASK                                      0xFFFFFFF8
#define    MMMMU_OPEN_ISR_PERFCOUNT2_OVERFLOW_SHIFT                       2
#define    MMMMU_OPEN_ISR_PERFCOUNT2_OVERFLOW_MASK                        0x00000004
#define    MMMMU_OPEN_ISR_PERFCOUNT1_OVERFLOW_SHIFT                       1
#define    MMMMU_OPEN_ISR_PERFCOUNT1_OVERFLOW_MASK                        0x00000002
#define    MMMMU_OPEN_ISR_EXFIFO_NOT_EMPTY_SHIFT                          0
#define    MMMMU_OPEN_ISR_EXFIFO_NOT_EMPTY_MASK                           0x00000001

#define MMMMU_OPEN_IMR_OFFSET                                             0x00000014
#define MMMMU_OPEN_IMR_TYPE                                               UInt32
#define MMMMU_OPEN_IMR_RESERVED_MASK                                      0xFFFFFFF8
#define    MMMMU_OPEN_IMR_PERFCOUNT2_OVERFLOW_SHIFT                       2
#define    MMMMU_OPEN_IMR_PERFCOUNT2_OVERFLOW_MASK                        0x00000004
#define    MMMMU_OPEN_IMR_PERFCOUNT1_OVERFLOW_SHIFT                       1
#define    MMMMU_OPEN_IMR_PERFCOUNT1_OVERFLOW_MASK                        0x00000002
#define    MMMMU_OPEN_IMR_EXFIFO_NOT_EMPTY_SHIFT                          0
#define    MMMMU_OPEN_IMR_EXFIFO_NOT_EMPTY_MASK                           0x00000001

#define MMMMU_OPEN_TBR_OFFSET                                             0x00000020
#define MMMMU_OPEN_TBR_TYPE                                               UInt32
#define MMMMU_OPEN_TBR_RESERVED_MASK                                      0x0000001F
#define    MMMMU_OPEN_TBR_ADDRESS_SHIFT                                   5
#define    MMMMU_OPEN_TBR_ADDRESS_MASK                                    0xFFFFFFE0

#define MMMMU_OPEN_LR_OFFSET                                              0x00000024
#define MMMMU_OPEN_LR_TYPE                                                UInt32
#define MMMMU_OPEN_LR_RESERVED_MASK                                       0x00000FFF
#define    MMMMU_OPEN_LR_LIMIT_SHIFT                                      12
#define    MMMMU_OPEN_LR_LIMIT_MASK                                       0xFFFFF000

#define MMMMU_OPEN_LDR_OFFSET                                             0x00000028
#define MMMMU_OPEN_LDR_TYPE                                               UInt32
#define MMMMU_OPEN_LDR_RESERVED_MASK                                      0x000000FE
#define    MMMMU_OPEN_LDR_PPP_SHIFT                                       12
#define    MMMMU_OPEN_LDR_PPP_MASK                                        0xFFFFF000
#define    MMMMU_OPEN_LDR_PPS_SHIFT                                       8
#define    MMMMU_OPEN_LDR_PPS_MASK                                        0x00000F00
#define    MMMMU_OPEN_LDR_VALID_SHIFT                                     0
#define    MMMMU_OPEN_LDR_VALID_MASK                                      0x00000001

#define MMMMU_OPEN_EFL_OFFSET                                             0x00000040
#define MMMMU_OPEN_EFL_TYPE                                               UInt32
#define MMMMU_OPEN_EFL_RESERVED_MASK                                      0x00007FE0
#define    MMMMU_OPEN_EFL_ADDRESS_SHIFT                                   15
#define    MMMMU_OPEN_EFL_ADDRESS_MASK                                    0xFFFF8000
#define    MMMMU_OPEN_EFL_RANGE_SHIFT                                     0
#define    MMMMU_OPEN_EFL_RANGE_MASK                                      0x0000001F

#define MMMMU_OPEN_ELOCK_OFFSET                                           0x00000044
#define MMMMU_OPEN_ELOCK_TYPE                                             UInt32
#define MMMMU_OPEN_ELOCK_RESERVED_MASK                                    0x00007FFF
#define    MMMMU_OPEN_ELOCK_ADDRESS_SHIFT                                 15
#define    MMMMU_OPEN_ELOCK_ADDRESS_MASK                                  0xFFFF8000

#define MMMMU_OPEN_EUNLOCK_OFFSET                                         0x00000048
#define MMMMU_OPEN_EUNLOCK_TYPE                                           UInt32
#define MMMMU_OPEN_EUNLOCK_RESERVED_MASK                                  0x00007FFF
#define    MMMMU_OPEN_EUNLOCK_ADDRESS_SHIFT                               15
#define    MMMMU_OPEN_EUNLOCK_ADDRESS_MASK                                0xFFFF8000

#define MMMMU_OPEN_XFIFO_OFFSET                                           0x00000050
#define MMMMU_OPEN_XFIFO_TYPE                                             UInt32
#define MMMMU_OPEN_XFIFO_RESERVED_MASK                                    0x00000000
#define    MMMMU_OPEN_XFIFO_ADDRESS_SHIFT                                 12
#define    MMMMU_OPEN_XFIFO_ADDRESS_MASK                                  0xFFFFF000
#define    MMMMU_OPEN_XFIFO_ID_SHIFT                                      6
#define    MMMMU_OPEN_XFIFO_ID_MASK                                       0x00000FC0
#define    MMMMU_OPEN_XFIFO_RW_SHIFT                                      5
#define    MMMMU_OPEN_XFIFO_RW_MASK                                       0x00000020
#define    MMMMU_OPEN_XFIFO_REASON_SHIFT                                  2
#define    MMMMU_OPEN_XFIFO_REASON_MASK                                   0x0000001C
#define    MMMMU_OPEN_XFIFO_OVFL_SHIFT                                    1
#define    MMMMU_OPEN_XFIFO_OVFL_MASK                                     0x00000002
#define    MMMMU_OPEN_XFIFO_VALID_SHIFT                                   0
#define    MMMMU_OPEN_XFIFO_VALID_MASK                                    0x00000001

#define MMMMU_OPEN_PCCR_OFFSET                                            0x00000060
#define MMMMU_OPEN_PCCR_TYPE                                              UInt32
#define MMMMU_OPEN_PCCR_RESERVED_MASK                                     0x00400040
#define    MMMMU_OPEN_PCCR_ID2_SHIFT                                      24
#define    MMMMU_OPEN_PCCR_ID2_MASK                                       0xFF000000
#define    MMMMU_OPEN_PCCR_IDEN2_SHIFT                                    23
#define    MMMMU_OPEN_PCCR_IDEN2_MASK                                     0x00800000
#define    MMMMU_OPEN_PCCR_ACL2_SHIFT                                     20
#define    MMMMU_OPEN_PCCR_ACL2_MASK                                      0x00300000
#define    MMMMU_OPEN_PCCR_ACLEN2_SHIFT                                   19
#define    MMMMU_OPEN_PCCR_ACLEN2_MASK                                    0x00080000
#define    MMMMU_OPEN_PCCR_MODE2_SHIFT                                    16
#define    MMMMU_OPEN_PCCR_MODE2_MASK                                     0x00070000
#define    MMMMU_OPEN_PCCR_ID1_SHIFT                                      8
#define    MMMMU_OPEN_PCCR_ID1_MASK                                       0x0000FF00
#define    MMMMU_OPEN_PCCR_IDEN1_SHIFT                                    7
#define    MMMMU_OPEN_PCCR_IDEN1_MASK                                     0x00000080
#define    MMMMU_OPEN_PCCR_ACL1_SHIFT                                     4
#define    MMMMU_OPEN_PCCR_ACL1_MASK                                      0x00000030
#define    MMMMU_OPEN_PCCR_ACLEN1_SHIFT                                   3
#define    MMMMU_OPEN_PCCR_ACLEN1_MASK                                    0x00000008
#define    MMMMU_OPEN_PCCR_MODE1_SHIFT                                    0
#define    MMMMU_OPEN_PCCR_MODE1_MASK                                     0x00000007

#define MMMMU_OPEN_PCR1_OFFSET                                            0x00000068
#define MMMMU_OPEN_PCR1_TYPE                                              UInt32
#define MMMMU_OPEN_PCR1_RESERVED_MASK                                     0x00000000
#define    MMMMU_OPEN_PCR1_COUNT_SHIFT                                    0
#define    MMMMU_OPEN_PCR1_COUNT_MASK                                     0xFFFFFFFF

#define MMMMU_OPEN_PCR2_OFFSET                                            0x0000006C
#define MMMMU_OPEN_PCR2_TYPE                                              UInt32
#define MMMMU_OPEN_PCR2_RESERVED_MASK                                     0x00000000
#define    MMMMU_OPEN_PCR2_COUNT_SHIFT                                    0
#define    MMMMU_OPEN_PCR2_COUNT_MASK                                     0xFFFFFFFF

#endif /* __BRCM_RDB_MMMMU_OPEN_H__ */



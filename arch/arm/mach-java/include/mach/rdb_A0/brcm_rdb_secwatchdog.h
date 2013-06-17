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

#ifndef __BRCM_RDB_SECWATCHDOG_H__
#define __BRCM_RDB_SECWATCHDOG_H__

#define SECWATCHDOG_SDOGCR_OFFSET                                         0x00000000
#define SECWATCHDOG_SDOGCR_TYPE                                           UInt32
#define SECWATCHDOG_SDOGCR_RESERVED_MASK                                  0xE2000000
#define    SECWATCHDOG_SDOGCR_WD_LOAD_FLAG_SHIFT                          28
#define    SECWATCHDOG_SDOGCR_WD_LOAD_FLAG_MASK                           0x10000000
#define    SECWATCHDOG_SDOGCR_EN_SHIFT                                    27
#define    SECWATCHDOG_SDOGCR_EN_MASK                                     0x08000000
#define    SECWATCHDOG_SDOGCR_SRSTEN_SHIFT                                26
#define    SECWATCHDOG_SDOGCR_SRSTEN_MASK                                 0x04000000
#define    SECWATCHDOG_SDOGCR_DSLPCEN_SHIFT                               24
#define    SECWATCHDOG_SDOGCR_DSLPCEN_MASK                                0x01000000
#define    SECWATCHDOG_SDOGCR_CLKS_SHIFT                                  20
#define    SECWATCHDOG_SDOGCR_CLKS_MASK                                   0x00F00000
#define    SECWATCHDOG_SDOGCR_LD_SHIFT                                    0
#define    SECWATCHDOG_SDOGCR_LD_MASK                                     0x000FFFFF

#define SECWATCHDOG_SDOGR_OFFSET                                          0x00000004
#define SECWATCHDOG_SDOGR_TYPE                                            UInt32
#define SECWATCHDOG_SDOGR_RESERVED_MASK                                   0xFFF00000
#define    SECWATCHDOG_SDOGR_CNT_SHIFT                                    0
#define    SECWATCHDOG_SDOGR_CNT_MASK                                     0x000FFFFF

#define SECWATCHDOG_STIMERCR_OFFSET                                       0x00000008
#define SECWATCHDOG_STIMERCR_TYPE                                         UInt32
#define SECWATCHDOG_STIMERCR_RESERVED_MASK                                0xE4000000
#define    SECWATCHDOG_STIMERCR_STIMER_LOAD_FLAG_SHIFT                    28
#define    SECWATCHDOG_STIMERCR_STIMER_LOAD_FLAG_MASK                     0x10000000
#define    SECWATCHDOG_STIMERCR_EN_SHIFT                                  27
#define    SECWATCHDOG_STIMERCR_EN_MASK                                   0x08000000
#define    SECWATCHDOG_STIMERCR_INTEN_SHIFT                               25
#define    SECWATCHDOG_STIMERCR_INTEN_MASK                                0x02000000
#define    SECWATCHDOG_STIMERCR_DSLPCEN_SHIFT                             24
#define    SECWATCHDOG_STIMERCR_DSLPCEN_MASK                              0x01000000
#define    SECWATCHDOG_STIMERCR_CLKS_SHIFT                                20
#define    SECWATCHDOG_STIMERCR_CLKS_MASK                                 0x00F00000
#define    SECWATCHDOG_STIMERCR_STIMER_SHIFT                              0
#define    SECWATCHDOG_STIMERCR_STIMER_MASK                               0x000FFFFF

#define SECWATCHDOG_STMR_OFFSET                                           0x0000000C
#define SECWATCHDOG_STMR_TYPE                                             UInt32
#define SECWATCHDOG_STMR_RESERVED_MASK                                    0xFFF00000
#define    SECWATCHDOG_STMR_CNT_SHIFT                                     0
#define    SECWATCHDOG_STMR_CNT_MASK                                      0x000FFFFF

#define SECWATCHDOG_FREE_32KHZ_CNTR_OFFSET                                0x00000010
#define SECWATCHDOG_FREE_32KHZ_CNTR_TYPE                                  UInt32
#define SECWATCHDOG_FREE_32KHZ_CNTR_RESERVED_MASK                         0xFFFF8000
#define    SECWATCHDOG_FREE_32KHZ_CNTR_CNT_32KHZ_SHIFT                    0
#define    SECWATCHDOG_FREE_32KHZ_CNTR_CNT_32KHZ_MASK                     0x00007FFF

#define SECWATCHDOG_UTC_LDR_OFFSET                                        0x00000014
#define SECWATCHDOG_UTC_LDR_TYPE                                          UInt32
#define SECWATCHDOG_UTC_LDR_RESERVED_MASK                                 0x00000000
#define    SECWATCHDOG_UTC_LDR_UT_LD_SHIFT                                0
#define    SECWATCHDOG_UTC_LDR_UT_LD_MASK                                 0xFFFFFFFF

#define SECWATCHDOG_UTC_CNTR_OFFSET                                       0x00000018
#define SECWATCHDOG_UTC_CNTR_TYPE                                         UInt32
#define SECWATCHDOG_UTC_CNTR_RESERVED_MASK                                0x00000000
#define    SECWATCHDOG_UTC_CNTR_UT_CNT_SHIFT                              0
#define    SECWATCHDOG_UTC_CNTR_UT_CNT_MASK                               0xFFFFFFFF

#define SECWATCHDOG_UTC_INTR_OFFSET                                       0x0000001C
#define SECWATCHDOG_UTC_INTR_TYPE                                         UInt32
#define SECWATCHDOG_UTC_INTR_RESERVED_MASK                                0x00000000
#define    SECWATCHDOG_UTC_INTR_UTC_INT_SHIFT                             0
#define    SECWATCHDOG_UTC_INTR_UTC_INT_MASK                              0xFFFFFFFF

#define SECWATCHDOG_UTC_CR_OFFSET                                         0x00000020
#define SECWATCHDOG_UTC_CR_TYPE                                           UInt32
#define SECWATCHDOG_UTC_CR_RESERVED_MASK                                  0xFFFFF0F0
#define    SECWATCHDOG_UTC_CR_UTC_LOAD_FLAG_SHIFT                         11
#define    SECWATCHDOG_UTC_CR_UTC_LOAD_FLAG_MASK                          0x00000800
#define    SECWATCHDOG_UTC_CR_DSLPCEN_SHIFT                               10
#define    SECWATCHDOG_UTC_CR_DSLPCEN_MASK                                0x00000400
#define    SECWATCHDOG_UTC_CR_UTCINT_EN_SHIFT                             9
#define    SECWATCHDOG_UTC_CR_UTCINT_EN_MASK                              0x00000200
#define    SECWATCHDOG_UTC_CR_UTC_EN_SHIFT                                8
#define    SECWATCHDOG_UTC_CR_UTC_EN_MASK                                 0x00000100
#define    SECWATCHDOG_UTC_CR_UTC_CLKS_SHIFT                              0
#define    SECWATCHDOG_UTC_CR_UTC_CLKS_MASK                               0x0000000F

#define SECWATCHDOG_CHKCR_OFFSET                                          0x00000024
#define SECWATCHDOG_CHKCR_TYPE                                            UInt32
#define SECWATCHDOG_CHKCR_RESERVED_MASK                                   0xF0FFFFFF
#define    SECWATCHDOG_CHKCR_CHK_LOAD_FLAG_SHIFT                          27
#define    SECWATCHDOG_CHKCR_CHK_LOAD_FLAG_MASK                           0x08000000
#define    SECWATCHDOG_CHKCR_CHK_EN_SHIFT                                 26
#define    SECWATCHDOG_CHKCR_CHK_EN_MASK                                  0x04000000
#define    SECWATCHDOG_CHKCR_CHK_SRSTEN_SHIFT                             25
#define    SECWATCHDOG_CHKCR_CHK_SRSTEN_MASK                              0x02000000
#define    SECWATCHDOG_CHKCR_CHK_INTEN_SHIFT                              24
#define    SECWATCHDOG_CHKCR_CHK_INTEN_MASK                               0x01000000

#define SECWATCHDOG_CHKLMCR_OFFSET                                        0x00000028
#define SECWATCHDOG_CHKLMCR_TYPE                                          UInt32
#define SECWATCHDOG_CHKLMCR_RESERVED_MASK                                 0x00000000
#define    SECWATCHDOG_CHKLMCR_CHK_HIGH_SHIFT                             16
#define    SECWATCHDOG_CHKLMCR_CHK_HIGH_MASK                              0xFFFF0000
#define    SECWATCHDOG_CHKLMCR_CHK_LOW_SHIFT                              0
#define    SECWATCHDOG_CHKLMCR_CHK_LOW_MASK                               0x0000FFFF

#define SECWATCHDOG_CHKCNTR_OFFSET                                        0x0000002C
#define SECWATCHDOG_CHKCNTR_TYPE                                          UInt32
#define SECWATCHDOG_CHKCNTR_RESERVED_MASK                                 0xFFFF0000
#define    SECWATCHDOG_CHKCNTR_CHK_CNT_SHIFT                              0
#define    SECWATCHDOG_CHKCNTR_CHK_CNT_MASK                               0x0000FFFF

#define SECWATCHDOG_INTSR_OFFSET                                          0x00000030
#define SECWATCHDOG_INTSR_TYPE                                            UInt32
#define SECWATCHDOG_INTSR_RESERVED_MASK                                   0xFFFFFFF0
#define    SECWATCHDOG_INTSR_UTC_OVERFLOW_FLAG_SHIFT                      3
#define    SECWATCHDOG_INTSR_UTC_OVERFLOW_FLAG_MASK                       0x00000008
#define    SECWATCHDOG_INTSR_CHKINT_FLAG_SHIFT                            2
#define    SECWATCHDOG_INTSR_CHKINT_FLAG_MASK                             0x00000004
#define    SECWATCHDOG_INTSR_UTCINT_FLAG_SHIFT                            1
#define    SECWATCHDOG_INTSR_UTCINT_FLAG_MASK                             0x00000002
#define    SECWATCHDOG_INTSR_STINT_FLAG_SHIFT                             0
#define    SECWATCHDOG_INTSR_STINT_FLAG_MASK                              0x00000001

#define SECWATCHDOG_SDOGCR_LCR_OFFSET                                     0x00000100
#define SECWATCHDOG_SDOGCR_LCR_TYPE                                       UInt32
#define SECWATCHDOG_SDOGCR_LCR_RESERVED_MASK                              0xE2EFFFFE
#define    SECWATCHDOG_SDOGCR_LCR_FREEZE_SHIFT                            28
#define    SECWATCHDOG_SDOGCR_LCR_FREEZE_MASK                             0x10000000
#define    SECWATCHDOG_SDOGCR_LCR_EN_LOCK_SHIFT                           27
#define    SECWATCHDOG_SDOGCR_LCR_EN_LOCK_MASK                            0x08000000
#define    SECWATCHDOG_SDOGCR_LCR_SRSTEN_LOCK_SHIFT                       26
#define    SECWATCHDOG_SDOGCR_LCR_SRSTEN_LOCK_MASK                        0x04000000
#define    SECWATCHDOG_SDOGCR_LCR_DSLPCEN_LOCK_SHIFT                      24
#define    SECWATCHDOG_SDOGCR_LCR_DSLPCEN_LOCK_MASK                       0x01000000
#define    SECWATCHDOG_SDOGCR_LCR_CLKS_LOCK_SHIFT                         20
#define    SECWATCHDOG_SDOGCR_LCR_CLKS_LOCK_MASK                          0x00100000
#define    SECWATCHDOG_SDOGCR_LCR_LD_LOCK_SHIFT                           0
#define    SECWATCHDOG_SDOGCR_LCR_LD_LOCK_MASK                            0x00000001

#define SECWATCHDOG_STIMERCR_LCR_OFFSET                                   0x00000108
#define SECWATCHDOG_STIMERCR_LCR_TYPE                                     UInt32
#define SECWATCHDOG_STIMERCR_LCR_RESERVED_MASK                            0xF4EFFFFE
#define    SECWATCHDOG_STIMERCR_LCR_EN_LOCK_SHIFT                         27
#define    SECWATCHDOG_STIMERCR_LCR_EN_LOCK_MASK                          0x08000000
#define    SECWATCHDOG_STIMERCR_LCR_INTEN_LOCK_SHIFT                      25
#define    SECWATCHDOG_STIMERCR_LCR_INTEN_LOCK_MASK                       0x02000000
#define    SECWATCHDOG_STIMERCR_LCR_DSLPCEN_LOCK_SHIFT                    24
#define    SECWATCHDOG_STIMERCR_LCR_DSLPCEN_LOCK_MASK                     0x01000000
#define    SECWATCHDOG_STIMERCR_LCR_CLKS_LOCK_SHIFT                       20
#define    SECWATCHDOG_STIMERCR_LCR_CLKS_LOCK_MASK                        0x00100000
#define    SECWATCHDOG_STIMERCR_LCR_STIMER_LOCK_SHIFT                     0
#define    SECWATCHDOG_STIMERCR_LCR_STIMER_LOCK_MASK                      0x00000001

#define SECWATCHDOG_UTC_LDR_LCR_OFFSET                                    0x00000114
#define SECWATCHDOG_UTC_LDR_LCR_TYPE                                      UInt32
#define SECWATCHDOG_UTC_LDR_LCR_RESERVED_MASK                             0xFFFFFFFE
#define    SECWATCHDOG_UTC_LDR_LCR_UT_LD_LOCK_SHIFT                       0
#define    SECWATCHDOG_UTC_LDR_LCR_UT_LD_LOCK_MASK                        0x00000001

#define SECWATCHDOG_UTC_INTR_LCR_OFFSET                                   0x0000011C
#define SECWATCHDOG_UTC_INTR_LCR_TYPE                                     UInt32
#define SECWATCHDOG_UTC_INTR_LCR_RESERVED_MASK                            0xFFFFFFFE
#define    SECWATCHDOG_UTC_INTR_LCR_UT_INT_LOCK_SHIFT                     0
#define    SECWATCHDOG_UTC_INTR_LCR_UT_INT_LOCK_MASK                      0x00000001

#define SECWATCHDOG_UTC_CR_LCR_OFFSET                                     0x00000120
#define SECWATCHDOG_UTC_CR_LCR_TYPE                                       UInt32
#define SECWATCHDOG_UTC_CR_LCR_RESERVED_MASK                              0xFFFFF8FE
#define    SECWATCHDOG_UTC_CR_LCR_UTC_DSLPCEN_LOCK_SHIFT                  10
#define    SECWATCHDOG_UTC_CR_LCR_UTC_DSLPCEN_LOCK_MASK                   0x00000400
#define    SECWATCHDOG_UTC_CR_LCR_UTCINT_EN_LOCK_SHIFT                    9
#define    SECWATCHDOG_UTC_CR_LCR_UTCINT_EN_LOCK_MASK                     0x00000200
#define    SECWATCHDOG_UTC_CR_LCR_UTC_EN_LOCK_SHIFT                       8
#define    SECWATCHDOG_UTC_CR_LCR_UTC_EN_LOCK_MASK                        0x00000100
#define    SECWATCHDOG_UTC_CR_LCR_UTC_CLKS_LOCK_SHIFT                     0
#define    SECWATCHDOG_UTC_CR_LCR_UTC_CLKS_LOCK_MASK                      0x00000001

#define SECWATCHDOG_CHKCR_LCR_OFFSET                                      0x00000124
#define SECWATCHDOG_CHKCR_LCR_TYPE                                        UInt32
#define SECWATCHDOG_CHKCR_LCR_RESERVED_MASK                               0xF8FFFFFF
#define    SECWATCHDOG_CHKCR_LCR_CHK_EN_LOCK_SHIFT                        26
#define    SECWATCHDOG_CHKCR_LCR_CHK_EN_LOCK_MASK                         0x04000000
#define    SECWATCHDOG_CHKCR_LCR_CHK_SRSTEN_LOCK_SHIFT                    25
#define    SECWATCHDOG_CHKCR_LCR_CHK_SRSTEN_LOCK_MASK                     0x02000000
#define    SECWATCHDOG_CHKCR_LCR_CHK_INTEN_LOCK_SHIFT                     24
#define    SECWATCHDOG_CHKCR_LCR_CHK_INTEN_LOCK_MASK                      0x01000000

#define SECWATCHDOG_CHKLMCR_LCR_OFFSET                                    0x00000128
#define SECWATCHDOG_CHKLMCR_LCR_TYPE                                      UInt32
#define SECWATCHDOG_CHKLMCR_LCR_RESERVED_MASK                             0xFFFEFFFE
#define    SECWATCHDOG_CHKLMCR_LCR_CHK_HIGH_LOCK_SHIFT                    16
#define    SECWATCHDOG_CHKLMCR_LCR_CHK_HIGH_LOCK_MASK                     0x00010000
#define    SECWATCHDOG_CHKLMCR_LCR_CHK_LOW_LOCK_SHIFT                     0
#define    SECWATCHDOG_CHKLMCR_LCR_CHK_LOW_LOCK_MASK                      0x00000001

#endif /* __BRCM_RDB_SECWATCHDOG_H__ */



/************************************************************************************************/
/*                                                                                              */
/*  Copyright 2010  Broadcom Corporation                                                        */
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
/*     Date     : Generated on 11/9/2010 1:17:6                                             */
/*     RDB file : //R4/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_NON_DMAC_H__
#define __BRCM_RDB_NON_DMAC_H__

#define NON_DMAC_DS_OFFSET                                                0x00000000
#define NON_DMAC_DS_TYPE                                                  UInt32
#define NON_DMAC_DS_RESERVED_MASK                                         0xFFFFFC00
#define    NON_DMAC_DS_DNS_SHIFT                                          9
#define    NON_DMAC_DS_DNS_MASK                                           0x00000200
#define    NON_DMAC_DS_WAKEUP_EVENT_SHIFT                                 4
#define    NON_DMAC_DS_WAKEUP_EVENT_MASK                                  0x000001F0
#define    NON_DMAC_DS_DMA_STATUS_SHIFT                                   0
#define    NON_DMAC_DS_DMA_STATUS_MASK                                    0x0000000F

#define NON_DMAC_DPC_OFFSET                                               0x00000004
#define NON_DMAC_DPC_TYPE                                                 UInt32
#define NON_DMAC_DPC_RESERVED_MASK                                        0x00000000
#define    NON_DMAC_DPC_PC_MGR_SHIFT                                      0
#define    NON_DMAC_DPC_PC_MGR_MASK                                       0xFFFFFFFF

#define NON_DMAC_INTEN_OFFSET                                             0x00000020
#define NON_DMAC_INTEN_TYPE                                               UInt32
#define NON_DMAC_INTEN_RESERVED_MASK                                      0x00000000
#define    NON_DMAC_INTEN_EVENT_IRQ_SELECT_SHIFT                          0
#define    NON_DMAC_INTEN_EVENT_IRQ_SELECT_MASK                           0xFFFFFFFF

#define NON_DMAC_ES_OFFSET                                                0x00000024
#define NON_DMAC_ES_TYPE                                                  UInt32
#define NON_DMAC_ES_RESERVED_MASK                                         0x00000000
#define    NON_DMAC_ES_DMASEV_ACTIVE_SHIFT                                0
#define    NON_DMAC_ES_DMASEV_ACTIVE_MASK                                 0xFFFFFFFF

#define NON_DMAC_INTSTATUS_OFFSET                                         0x00000028
#define NON_DMAC_INTSTATUS_TYPE                                           UInt32
#define NON_DMAC_INTSTATUS_RESERVED_MASK                                  0x00000000
#define    NON_DMAC_INTSTATUS_IRQ_STATUS_SHIFT                            0
#define    NON_DMAC_INTSTATUS_IRQ_STATUS_MASK                             0xFFFFFFFF

#define NON_DMAC_INTCLR_OFFSET                                            0x0000002C
#define NON_DMAC_INTCLR_TYPE                                              UInt32
#define NON_DMAC_INTCLR_RESERVED_MASK                                     0x00000000
#define    NON_DMAC_INTCLR_IRQ_CLR_SHIFT                                  0
#define    NON_DMAC_INTCLR_IRQ_CLR_MASK                                   0xFFFFFFFF

#define NON_DMAC_FSM_OFFSET                                               0x00000030
#define NON_DMAC_FSM_TYPE                                                 UInt32
#define NON_DMAC_FSM_RESERVED_MASK                                        0xFFFFFFFE
#define    NON_DMAC_FSM_FS_MGR_SHIFT                                      0
#define    NON_DMAC_FSM_FS_MGR_MASK                                       0x00000001

#define NON_DMAC_FSC_OFFSET                                               0x00000034
#define NON_DMAC_FSC_TYPE                                                 UInt32
#define NON_DMAC_FSC_RESERVED_MASK                                        0xFFFFFF00
#define    NON_DMAC_FSC_FAULT_STATUS_SHIFT                                0
#define    NON_DMAC_FSC_FAULT_STATUS_MASK                                 0x000000FF

#define NON_DMAC_FTM_OFFSET                                               0x00000038
#define NON_DMAC_FTM_TYPE                                                 UInt32
#define NON_DMAC_FTM_RESERVED_MASK                                        0xBFFEFFCC
#define    NON_DMAC_FTM_DBG_INSTR_SHIFT                                   30
#define    NON_DMAC_FTM_DBG_INSTR_MASK                                    0x40000000
#define    NON_DMAC_FTM_INSTR_FETCH_ERR_SHIFT                             16
#define    NON_DMAC_FTM_INSTR_FETCH_ERR_MASK                              0x00010000
#define    NON_DMAC_FTM_MGR_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTM_MGR_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTM_DMAGO_ERR_SHIFT                                   4
#define    NON_DMAC_FTM_DMAGO_ERR_MASK                                    0x00000010
#define    NON_DMAC_FTM_OPERAND_INVALID_SHIFT                             1
#define    NON_DMAC_FTM_OPERAND_INVALID_MASK                              0x00000002
#define    NON_DMAC_FTM_UNDEF_INSTR_SHIFT                                 0
#define    NON_DMAC_FTM_UNDEF_INSTR_MASK                                  0x00000001

#define NON_DMAC_FTC0_OFFSET                                              0x00000040
#define NON_DMAC_FTC0_TYPE                                                UInt32
#define NON_DMAC_FTC0_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC0_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC0_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC0_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC0_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC0_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC0_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC0_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC0_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC0_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC0_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC0_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC0_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC0_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC0_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC0_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC0_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC0_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC0_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC0_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC0_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC0_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC0_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC1_OFFSET                                              0x00000044
#define NON_DMAC_FTC1_TYPE                                                UInt32
#define NON_DMAC_FTC1_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC1_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC1_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC1_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC1_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC1_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC1_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC1_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC1_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC1_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC1_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC1_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC1_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC1_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC1_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC1_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC1_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC1_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC1_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC1_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC1_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC1_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC1_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC2_OFFSET                                              0x00000048
#define NON_DMAC_FTC2_TYPE                                                UInt32
#define NON_DMAC_FTC2_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC2_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC2_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC2_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC2_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC2_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC2_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC2_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC2_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC2_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC2_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC2_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC2_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC2_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC2_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC2_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC2_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC2_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC2_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC2_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC2_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC2_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC2_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC3_OFFSET                                              0x0000004C
#define NON_DMAC_FTC3_TYPE                                                UInt32
#define NON_DMAC_FTC3_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC3_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC3_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC3_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC3_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC3_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC3_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC3_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC3_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC3_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC3_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC3_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC3_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC3_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC3_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC3_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC3_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC3_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC3_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC3_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC3_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC3_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC3_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC4_OFFSET                                              0x00000050
#define NON_DMAC_FTC4_TYPE                                                UInt32
#define NON_DMAC_FTC4_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC4_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC4_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC4_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC4_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC4_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC4_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC4_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC4_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC4_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC4_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC4_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC4_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC4_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC4_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC4_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC4_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC4_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC4_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC4_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC4_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC4_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC4_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC5_OFFSET                                              0x00000054
#define NON_DMAC_FTC5_TYPE                                                UInt32
#define NON_DMAC_FTC5_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC5_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC5_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC5_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC5_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC5_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC5_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC5_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC5_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC5_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC5_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC5_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC5_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC5_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC5_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC5_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC5_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC5_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC5_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC5_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC5_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC5_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC5_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC6_OFFSET                                              0x00000058
#define NON_DMAC_FTC6_TYPE                                                UInt32
#define NON_DMAC_FTC6_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC6_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC6_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC6_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC6_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC6_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC6_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC6_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC6_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC6_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC6_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC6_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC6_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC6_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC6_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC6_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC6_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC6_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC6_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC6_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC6_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC6_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC6_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_FTC7_OFFSET                                              0x0000005C
#define NON_DMAC_FTC7_TYPE                                                UInt32
#define NON_DMAC_FTC7_RESERVED_MASK                                       0x3FF8EF1C
#define    NON_DMAC_FTC7_LOCKUP_ERR_SHIFT                                 31
#define    NON_DMAC_FTC7_LOCKUP_ERR_MASK                                  0x80000000
#define    NON_DMAC_FTC7_DBG_INSTR_SHIFT                                  30
#define    NON_DMAC_FTC7_DBG_INSTR_MASK                                   0x40000000
#define    NON_DMAC_FTC7_DATA_READ_ERR_SHIFT                              18
#define    NON_DMAC_FTC7_DATA_READ_ERR_MASK                               0x00040000
#define    NON_DMAC_FTC7_DATA_WRITE_ERR_SHIFT                             17
#define    NON_DMAC_FTC7_DATA_WRITE_ERR_MASK                              0x00020000
#define    NON_DMAC_FTC7_INSTR_FETCH_ERR_SHIFT                            16
#define    NON_DMAC_FTC7_INSTR_FETCH_ERR_MASK                             0x00010000
#define    NON_DMAC_FTC7_MFIFO_ERR_SHIFT                                  12
#define    NON_DMAC_FTC7_MFIFO_ERR_MASK                                   0x00001000
#define    NON_DMAC_FTC7_CH_RDWR_ERR_SHIFT                                7
#define    NON_DMAC_FTC7_CH_RDWR_ERR_MASK                                 0x00000080
#define    NON_DMAC_FTC7_CH_PERIPH_ERR_SHIFT                              6
#define    NON_DMAC_FTC7_CH_PERIPH_ERR_MASK                               0x00000040
#define    NON_DMAC_FTC7_CH_EVNT_ERR_SHIFT                                5
#define    NON_DMAC_FTC7_CH_EVNT_ERR_MASK                                 0x00000020
#define    NON_DMAC_FTC7_OPERAND_INVALID_SHIFT                            1
#define    NON_DMAC_FTC7_OPERAND_INVALID_MASK                             0x00000002
#define    NON_DMAC_FTC7_UNDEF_INSTR_SHIFT                                0
#define    NON_DMAC_FTC7_UNDEF_INSTR_MASK                                 0x00000001

#define NON_DMAC_CS0_OFFSET                                               0x00000100
#define NON_DMAC_CS0_TYPE                                                 UInt32
#define NON_DMAC_CS0_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS0_CNS_SHIFT                                         21
#define    NON_DMAC_CS0_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS0_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS0_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS0_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS0_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS0_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS0_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS0_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS0_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC0_OFFSET                                              0x00000104
#define NON_DMAC_CPC0_TYPE                                                UInt32
#define NON_DMAC_CPC0_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC0_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC0_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS1_OFFSET                                               0x00000108
#define NON_DMAC_CS1_TYPE                                                 UInt32
#define NON_DMAC_CS1_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS1_CNS_SHIFT                                         21
#define    NON_DMAC_CS1_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS1_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS1_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS1_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS1_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS1_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS1_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS1_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS1_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC1_OFFSET                                              0x0000010C
#define NON_DMAC_CPC1_TYPE                                                UInt32
#define NON_DMAC_CPC1_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC1_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC1_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS2_OFFSET                                               0x00000110
#define NON_DMAC_CS2_TYPE                                                 UInt32
#define NON_DMAC_CS2_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS2_CNS_SHIFT                                         21
#define    NON_DMAC_CS2_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS2_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS2_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS2_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS2_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS2_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS2_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS2_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS2_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC2_OFFSET                                              0x00000114
#define NON_DMAC_CPC2_TYPE                                                UInt32
#define NON_DMAC_CPC2_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC2_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC2_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS3_OFFSET                                               0x00000118
#define NON_DMAC_CS3_TYPE                                                 UInt32
#define NON_DMAC_CS3_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS3_CNS_SHIFT                                         21
#define    NON_DMAC_CS3_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS3_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS3_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS3_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS3_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS3_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS3_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS3_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS3_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC3_OFFSET                                              0x0000011C
#define NON_DMAC_CPC3_TYPE                                                UInt32
#define NON_DMAC_CPC3_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC3_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC3_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS4_OFFSET                                               0x00000120
#define NON_DMAC_CS4_TYPE                                                 UInt32
#define NON_DMAC_CS4_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS4_CNS_SHIFT                                         21
#define    NON_DMAC_CS4_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS4_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS4_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS4_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS4_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS4_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS4_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS4_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS4_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC4_OFFSET                                              0x00000124
#define NON_DMAC_CPC4_TYPE                                                UInt32
#define NON_DMAC_CPC4_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC4_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC4_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS5_OFFSET                                               0x00000128
#define NON_DMAC_CS5_TYPE                                                 UInt32
#define NON_DMAC_CS5_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS5_CNS_SHIFT                                         21
#define    NON_DMAC_CS5_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS5_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS5_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS5_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS5_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS5_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS5_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS5_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS5_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC5_OFFSET                                              0x0000012C
#define NON_DMAC_CPC5_TYPE                                                UInt32
#define NON_DMAC_CPC5_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC5_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC5_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS6_OFFSET                                               0x00000130
#define NON_DMAC_CS6_TYPE                                                 UInt32
#define NON_DMAC_CS6_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS6_CNS_SHIFT                                         21
#define    NON_DMAC_CS6_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS6_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS6_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS6_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS6_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS6_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS6_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS6_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS6_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC6_OFFSET                                              0x00000134
#define NON_DMAC_CPC6_TYPE                                                UInt32
#define NON_DMAC_CPC6_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC6_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC6_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_CS7_OFFSET                                               0x00000138
#define NON_DMAC_CS7_TYPE                                                 UInt32
#define NON_DMAC_CS7_RESERVED_MASK                                        0xFFDF3E00
#define    NON_DMAC_CS7_CNS_SHIFT                                         21
#define    NON_DMAC_CS7_CNS_MASK                                          0x00200000
#define    NON_DMAC_CS7_DMAWFP_PERIPH_SHIFT                               15
#define    NON_DMAC_CS7_DMAWFP_PERIPH_MASK                                0x00008000
#define    NON_DMAC_CS7_DMAWFP_B_NS_SHIFT                                 14
#define    NON_DMAC_CS7_DMAWFP_B_NS_MASK                                  0x00004000
#define    NON_DMAC_CS7_WAKEUP_NUMBER_SHIFT                               4
#define    NON_DMAC_CS7_WAKEUP_NUMBER_MASK                                0x000001F0
#define    NON_DMAC_CS7_CHANNEL_STATUS_SHIFT                              0
#define    NON_DMAC_CS7_CHANNEL_STATUS_MASK                               0x0000000F

#define NON_DMAC_CPC7_OFFSET                                              0x0000013C
#define NON_DMAC_CPC7_TYPE                                                UInt32
#define NON_DMAC_CPC7_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CPC7_PC_CHNL_SHIFT                                    0
#define    NON_DMAC_CPC7_PC_CHNL_MASK                                     0xFFFFFFFF

#define NON_DMAC_SA_0_OFFSET                                              0x00000400
#define NON_DMAC_SA_0_TYPE                                                UInt32
#define NON_DMAC_SA_0_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_0_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_0_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_0_OFFSET                                              0x00000404
#define NON_DMAC_DA_0_TYPE                                                UInt32
#define NON_DMAC_DA_0_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_0_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_0_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_0_OFFSET                                              0x00000408
#define NON_DMAC_CC_0_TYPE                                                UInt32
#define NON_DMAC_CC_0_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_0_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_0_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_0_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_0_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_0_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_0_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_0_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_0_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_0_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_0_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_0_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_0_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_0_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_0_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_0_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_0_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_0_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_0_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_0_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_0_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_0_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_0_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_0_OFFSET                                             0x0000040C
#define NON_DMAC_LC0_0_TYPE                                               UInt32
#define NON_DMAC_LC0_0_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_0_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_0_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_0_OFFSET                                             0x00000410
#define NON_DMAC_LC1_0_TYPE                                               UInt32
#define NON_DMAC_LC1_0_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_0_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_0_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_1_OFFSET                                              0x00000420
#define NON_DMAC_SA_1_TYPE                                                UInt32
#define NON_DMAC_SA_1_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_1_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_1_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_1_OFFSET                                              0x00000424
#define NON_DMAC_DA_1_TYPE                                                UInt32
#define NON_DMAC_DA_1_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_1_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_1_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_1_OFFSET                                              0x00000428
#define NON_DMAC_CC_1_TYPE                                                UInt32
#define NON_DMAC_CC_1_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_1_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_1_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_1_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_1_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_1_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_1_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_1_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_1_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_1_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_1_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_1_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_1_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_1_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_1_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_1_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_1_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_1_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_1_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_1_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_1_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_1_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_1_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_1_OFFSET                                             0x0000042C
#define NON_DMAC_LC0_1_TYPE                                               UInt32
#define NON_DMAC_LC0_1_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_1_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_1_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_1_OFFSET                                             0x00000430
#define NON_DMAC_LC1_1_TYPE                                               UInt32
#define NON_DMAC_LC1_1_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_1_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_1_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_2_OFFSET                                              0x00000440
#define NON_DMAC_SA_2_TYPE                                                UInt32
#define NON_DMAC_SA_2_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_2_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_2_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_2_OFFSET                                              0x00000444
#define NON_DMAC_DA_2_TYPE                                                UInt32
#define NON_DMAC_DA_2_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_2_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_2_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_2_OFFSET                                              0x00000448
#define NON_DMAC_CC_2_TYPE                                                UInt32
#define NON_DMAC_CC_2_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_2_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_2_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_2_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_2_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_2_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_2_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_2_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_2_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_2_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_2_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_2_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_2_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_2_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_2_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_2_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_2_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_2_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_2_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_2_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_2_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_2_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_2_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_2_OFFSET                                             0x0000044C
#define NON_DMAC_LC0_2_TYPE                                               UInt32
#define NON_DMAC_LC0_2_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_2_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_2_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_2_OFFSET                                             0x00000450
#define NON_DMAC_LC1_2_TYPE                                               UInt32
#define NON_DMAC_LC1_2_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_2_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_2_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_3_OFFSET                                              0x00000460
#define NON_DMAC_SA_3_TYPE                                                UInt32
#define NON_DMAC_SA_3_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_3_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_3_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_3_OFFSET                                              0x00000464
#define NON_DMAC_DA_3_TYPE                                                UInt32
#define NON_DMAC_DA_3_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_3_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_3_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_3_OFFSET                                              0x00000468
#define NON_DMAC_CC_3_TYPE                                                UInt32
#define NON_DMAC_CC_3_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_3_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_3_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_3_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_3_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_3_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_3_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_3_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_3_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_3_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_3_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_3_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_3_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_3_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_3_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_3_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_3_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_3_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_3_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_3_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_3_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_3_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_3_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_3_OFFSET                                             0x0000046C
#define NON_DMAC_LC0_3_TYPE                                               UInt32
#define NON_DMAC_LC0_3_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_3_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_3_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_3_OFFSET                                             0x00000470
#define NON_DMAC_LC1_3_TYPE                                               UInt32
#define NON_DMAC_LC1_3_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_3_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_3_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_4_OFFSET                                              0x00000480
#define NON_DMAC_SA_4_TYPE                                                UInt32
#define NON_DMAC_SA_4_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_4_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_4_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_4_OFFSET                                              0x00000484
#define NON_DMAC_DA_4_TYPE                                                UInt32
#define NON_DMAC_DA_4_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_4_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_4_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_4_OFFSET                                              0x00000488
#define NON_DMAC_CC_4_TYPE                                                UInt32
#define NON_DMAC_CC_4_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_4_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_4_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_4_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_4_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_4_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_4_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_4_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_4_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_4_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_4_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_4_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_4_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_4_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_4_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_4_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_4_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_4_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_4_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_4_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_4_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_4_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_4_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_4_OFFSET                                             0x0000048C
#define NON_DMAC_LC0_4_TYPE                                               UInt32
#define NON_DMAC_LC0_4_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_4_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_4_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_4_OFFSET                                             0x00000490
#define NON_DMAC_LC1_4_TYPE                                               UInt32
#define NON_DMAC_LC1_4_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_4_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_4_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_5_OFFSET                                              0x000004A0
#define NON_DMAC_SA_5_TYPE                                                UInt32
#define NON_DMAC_SA_5_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_5_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_5_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_5_OFFSET                                              0x000004A4
#define NON_DMAC_DA_5_TYPE                                                UInt32
#define NON_DMAC_DA_5_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_5_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_5_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_5_OFFSET                                              0x000004A8
#define NON_DMAC_CC_5_TYPE                                                UInt32
#define NON_DMAC_CC_5_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_5_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_5_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_5_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_5_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_5_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_5_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_5_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_5_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_5_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_5_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_5_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_5_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_5_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_5_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_5_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_5_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_5_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_5_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_5_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_5_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_5_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_5_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_5_OFFSET                                             0x000004AC
#define NON_DMAC_LC0_5_TYPE                                               UInt32
#define NON_DMAC_LC0_5_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_5_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_5_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_5_OFFSET                                             0x000004B0
#define NON_DMAC_LC1_5_TYPE                                               UInt32
#define NON_DMAC_LC1_5_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_5_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_5_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_6_OFFSET                                              0x000004C0
#define NON_DMAC_SA_6_TYPE                                                UInt32
#define NON_DMAC_SA_6_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_6_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_6_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_6_OFFSET                                              0x000004C4
#define NON_DMAC_DA_6_TYPE                                                UInt32
#define NON_DMAC_DA_6_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_6_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_6_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_6_OFFSET                                              0x000004C8
#define NON_DMAC_CC_6_TYPE                                                UInt32
#define NON_DMAC_CC_6_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_6_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_6_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_6_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_6_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_6_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_6_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_6_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_6_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_6_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_6_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_6_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_6_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_6_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_6_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_6_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_6_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_6_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_6_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_6_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_6_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_6_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_6_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_6_OFFSET                                             0x000004CC
#define NON_DMAC_LC0_6_TYPE                                               UInt32
#define NON_DMAC_LC0_6_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_6_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_6_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_6_OFFSET                                             0x000004D0
#define NON_DMAC_LC1_6_TYPE                                               UInt32
#define NON_DMAC_LC1_6_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_6_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_6_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_SA_7_OFFSET                                              0x000004E0
#define NON_DMAC_SA_7_TYPE                                                UInt32
#define NON_DMAC_SA_7_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_SA_7_SRC_ADDR_SHIFT                                   0
#define    NON_DMAC_SA_7_SRC_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_DA_7_OFFSET                                              0x000004E4
#define NON_DMAC_DA_7_TYPE                                                UInt32
#define NON_DMAC_DA_7_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_DA_7_DST_ADDR_SHIFT                                   0
#define    NON_DMAC_DA_7_DST_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CC_7_OFFSET                                              0x000004E8
#define NON_DMAC_CC_7_TYPE                                                UInt32
#define NON_DMAC_CC_7_RESERVED_MASK                                       0x00000000
#define    NON_DMAC_CC_7_ENDIAN_SWAP_SIZE_SHIFT                           28
#define    NON_DMAC_CC_7_ENDIAN_SWAP_SIZE_MASK                            0xF0000000
#define    NON_DMAC_CC_7_DST_CACHE_CTRL_SHIFT                             25
#define    NON_DMAC_CC_7_DST_CACHE_CTRL_MASK                              0x0E000000
#define    NON_DMAC_CC_7_DST_PROT_CTRL_SHIFT                              22
#define    NON_DMAC_CC_7_DST_PROT_CTRL_MASK                               0x01C00000
#define    NON_DMAC_CC_7_DST_BURST_LEN_SHIFT                              18
#define    NON_DMAC_CC_7_DST_BURST_LEN_MASK                               0x003C0000
#define    NON_DMAC_CC_7_DST_BURST_SIZE_SHIFT                             15
#define    NON_DMAC_CC_7_DST_BURST_SIZE_MASK                              0x00038000
#define    NON_DMAC_CC_7_DST_INC_SHIFT                                    14
#define    NON_DMAC_CC_7_DST_INC_MASK                                     0x00004000
#define    NON_DMAC_CC_7_SRC_CACHE_CTRL_SHIFT                             11
#define    NON_DMAC_CC_7_SRC_CACHE_CTRL_MASK                              0x00003800
#define    NON_DMAC_CC_7_SRC_PROT_CTRL_SHIFT                              8
#define    NON_DMAC_CC_7_SRC_PROT_CTRL_MASK                               0x00000700
#define    NON_DMAC_CC_7_SRC_BURST_LEN_SHIFT                              4
#define    NON_DMAC_CC_7_SRC_BURST_LEN_MASK                               0x000000F0
#define    NON_DMAC_CC_7_SRC_BURST_SIZE_SHIFT                             1
#define    NON_DMAC_CC_7_SRC_BURST_SIZE_MASK                              0x0000000E
#define    NON_DMAC_CC_7_SRC_INC_SHIFT                                    0
#define    NON_DMAC_CC_7_SRC_INC_MASK                                     0x00000001

#define NON_DMAC_LC0_7_OFFSET                                             0x000004EC
#define NON_DMAC_LC0_7_TYPE                                               UInt32
#define NON_DMAC_LC0_7_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC0_7_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC0_7_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_LC1_7_OFFSET                                             0x000004F0
#define NON_DMAC_LC1_7_TYPE                                               UInt32
#define NON_DMAC_LC1_7_RESERVED_MASK                                      0xFFFFFF00
#define    NON_DMAC_LC1_7_LOOP_COUNTER_ITERATIONS_SHIFT                   0
#define    NON_DMAC_LC1_7_LOOP_COUNTER_ITERATIONS_MASK                    0x000000FF

#define NON_DMAC_DBGSTATUS_OFFSET                                         0x00000D00
#define NON_DMAC_DBGSTATUS_TYPE                                           UInt32
#define NON_DMAC_DBGSTATUS_RESERVED_MASK                                  0xFFFFFFFE
#define    NON_DMAC_DBGSTATUS_DBGSTATUS_SHIFT                             0
#define    NON_DMAC_DBGSTATUS_DBGSTATUS_MASK                              0x00000001

#define NON_DMAC_DBGCMD_OFFSET                                            0x00000D04
#define NON_DMAC_DBGCMD_TYPE                                              UInt32
#define NON_DMAC_DBGCMD_RESERVED_MASK                                     0xFFFFFFFC
#define    NON_DMAC_DBGCMD_DBGCMD_SHIFT                                   0
#define    NON_DMAC_DBGCMD_DBGCMD_MASK                                    0x00000003

#define NON_DMAC_DBGINST0_OFFSET                                          0x00000D08
#define NON_DMAC_DBGINST0_TYPE                                            UInt32
#define NON_DMAC_DBGINST0_RESERVED_MASK                                   0x0000F8FE
#define    NON_DMAC_DBGINST0_INSTR1_SHIFT                                 24
#define    NON_DMAC_DBGINST0_INSTR1_MASK                                  0xFF000000
#define    NON_DMAC_DBGINST0_INSTR0_SHIFT                                 16
#define    NON_DMAC_DBGINST0_INSTR0_MASK                                  0x00FF0000
#define    NON_DMAC_DBGINST0_CHANNEL_NUM_SHIFT                            8
#define    NON_DMAC_DBGINST0_CHANNEL_NUM_MASK                             0x00000700
#define    NON_DMAC_DBGINST0_DEBUG_THREAD_SHIFT                           0
#define    NON_DMAC_DBGINST0_DEBUG_THREAD_MASK                            0x00000001

#define NON_DMAC_DBGINST1_OFFSET                                          0x00000D0C
#define NON_DMAC_DBGINST1_TYPE                                            UInt32
#define NON_DMAC_DBGINST1_RESERVED_MASK                                   0x00000000
#define    NON_DMAC_DBGINST1_INSTR5_SHIFT                                 24
#define    NON_DMAC_DBGINST1_INSTR5_MASK                                  0xFF000000
#define    NON_DMAC_DBGINST1_INSTR4_SHIFT                                 16
#define    NON_DMAC_DBGINST1_INSTR4_MASK                                  0x00FF0000
#define    NON_DMAC_DBGINST1_INSTR3_SHIFT                                 8
#define    NON_DMAC_DBGINST1_INSTR3_MASK                                  0x0000FF00
#define    NON_DMAC_DBGINST1_INSTR2_SHIFT                                 0
#define    NON_DMAC_DBGINST1_INSTR2_MASK                                  0x000000FF

#define NON_DMAC_CR0_OFFSET                                               0x00000E00
#define NON_DMAC_CR0_TYPE                                                 UInt32
#define NON_DMAC_CR0_RESERVED_MASK                                        0xFFC00F88
#define    NON_DMAC_CR0_NUM_EVENTS_SHIFT                                  17
#define    NON_DMAC_CR0_NUM_EVENTS_MASK                                   0x003E0000
#define    NON_DMAC_CR0_NUM_PERIPH_REQ_SHIFT                              12
#define    NON_DMAC_CR0_NUM_PERIPH_REQ_MASK                               0x0001F000
#define    NON_DMAC_CR0_NUM_CHNLS_SHIFT                                   4
#define    NON_DMAC_CR0_NUM_CHNLS_MASK                                    0x00000070
#define    NON_DMAC_CR0_MGR_NS_AT_RST_SHIFT                               2
#define    NON_DMAC_CR0_MGR_NS_AT_RST_MASK                                0x00000004
#define    NON_DMAC_CR0_BOOT_EN_SHIFT                                     1
#define    NON_DMAC_CR0_BOOT_EN_MASK                                      0x00000002
#define    NON_DMAC_CR0_PERIPH_REQ_SHIFT                                  0
#define    NON_DMAC_CR0_PERIPH_REQ_MASK                                   0x00000001

#define NON_DMAC_CR1_OFFSET                                               0x00000E04
#define NON_DMAC_CR1_TYPE                                                 UInt32
#define NON_DMAC_CR1_RESERVED_MASK                                        0xFFFFFF08
#define    NON_DMAC_CR1_NUM_ICACHE_LINES_SHIFT                            4
#define    NON_DMAC_CR1_NUM_ICACHE_LINES_MASK                             0x000000F0
#define    NON_DMAC_CR1_ICACHE_LEN_SHIFT                                  0
#define    NON_DMAC_CR1_ICACHE_LEN_MASK                                   0x00000007

#define NON_DMAC_CR2_OFFSET                                               0x00000E08
#define NON_DMAC_CR2_TYPE                                                 UInt32
#define NON_DMAC_CR2_RESERVED_MASK                                        0x00000000
#define    NON_DMAC_CR2_BOOT_ADDR_SHIFT                                   0
#define    NON_DMAC_CR2_BOOT_ADDR_MASK                                    0xFFFFFFFF

#define NON_DMAC_CR3_OFFSET                                               0x00000E0C
#define NON_DMAC_CR3_TYPE                                                 UInt32
#define NON_DMAC_CR3_RESERVED_MASK                                        0x00000000
#define    NON_DMAC_CR3_INS_SHIFT                                         0
#define    NON_DMAC_CR3_INS_MASK                                          0xFFFFFFFF

#define NON_DMAC_CR4_OFFSET                                               0x00000E10
#define NON_DMAC_CR4_TYPE                                                 UInt32
#define NON_DMAC_CR4_RESERVED_MASK                                        0x00000000
#define    NON_DMAC_CR4_PNS_SHIFT                                         0
#define    NON_DMAC_CR4_PNS_MASK                                          0xFFFFFFFF

#define NON_DMAC_CRDN_OFFSET                                              0x00000E14
#define NON_DMAC_CRDN_TYPE                                                UInt32
#define NON_DMAC_CRDN_RESERVED_MASK                                       0xC0008088
#define    NON_DMAC_CRDN_DATA_BUFFER_DEP_SHIFT                            20
#define    NON_DMAC_CRDN_DATA_BUFFER_DEP_MASK                             0x3FF00000
#define    NON_DMAC_CRDN_RD_Q_DEP_SHIFT                                   16
#define    NON_DMAC_CRDN_RD_Q_DEP_MASK                                    0x000F0000
#define    NON_DMAC_CRDN_RD_CAP_SHIFT                                     12
#define    NON_DMAC_CRDN_RD_CAP_MASK                                      0x00007000
#define    NON_DMAC_CRDN_WR_Q_DEP_SHIFT                                   8
#define    NON_DMAC_CRDN_WR_Q_DEP_MASK                                    0x00000F00
#define    NON_DMAC_CRDN_WR_CAP_SHIFT                                     4
#define    NON_DMAC_CRDN_WR_CAP_MASK                                      0x00000070
#define    NON_DMAC_CRDN_DATA_WIDTH_SHIFT                                 0
#define    NON_DMAC_CRDN_DATA_WIDTH_MASK                                  0x00000007

typedef volatile struct {
   UInt32 m_DS;                      // 0x0000
   UInt32 m_DPC;                     // 0x0004
   UInt32 RESERVED_20_4[6];
   UInt32 m_INTEN;                   // 0x0020
   UInt32 m_ES;                      // 0x0024
   UInt32 m_INTSTATUS;               // 0x0028
   UInt32 m_INTCLR;                  // 0x002C
   UInt32 m_FSM;                     // 0x0030
   UInt32 m_FSC;                     // 0x0034
   UInt32 m_FTM;                     // 0x0038
   UInt32 RESERVED_40_38[1];
   UInt32 m_FTC0;                    // 0x0040
   UInt32 m_FTC1;                    // 0x0044
   UInt32 m_FTC2;                    // 0x0048
   UInt32 m_FTC3;                    // 0x004C
   UInt32 m_FTC4;                    // 0x0050
   UInt32 m_FTC5;                    // 0x0054
   UInt32 m_FTC6;                    // 0x0058
   UInt32 m_FTC7;                    // 0x005C
   UInt32 RESERVED_100_5C[40];
   UInt32 m_CS0;                     // 0x0100
   UInt32 RESERVED_108_100[1];
   UInt32 m_CS1;                     // 0x0108
   UInt32 RESERVED_110_108[1];
   UInt32 m_CS2;                     // 0x0110
   UInt32 RESERVED_118_110[1];
   UInt32 m_CS3;                     // 0x0118
   UInt32 RESERVED_120_118[1];
   UInt32 m_CS4;                     // 0x0120
   UInt32 RESERVED_128_120[1];
   UInt32 m_CS5;                     // 0x0128
   UInt32 RESERVED_130_128[1];
   UInt32 m_CS6;                     // 0x0130
   UInt32 RESERVED_138_130[1];
   UInt32 m_CS7;                     // 0x0138
   UInt32 m_CPC0;                    // 0x0104
   UInt32 RESERVED_10C_104[1];
   UInt32 m_CPC1;                    // 0x010C
   UInt32 RESERVED_114_10C[1];
   UInt32 m_CPC2;                    // 0x0114
   UInt32 RESERVED_11C_114[1];
   UInt32 m_CPC3;                    // 0x011C
   UInt32 RESERVED_124_11C[1];
   UInt32 m_CPC4;                    // 0x0124
   UInt32 RESERVED_12C_124[1];
   UInt32 m_CPC5;                    // 0x012C
   UInt32 RESERVED_134_12C[1];
   UInt32 m_CPC6;                    // 0x0134
   UInt32 RESERVED_13C_134[1];
   UInt32 m_CPC7;                    // 0x013C
   UInt32 RESERVED_400_13C[176];
   UInt32 m_SA_0;                    // 0x0400
   UInt32 RESERVED_420_400[7];
   UInt32 m_SA_1;                    // 0x0420
   UInt32 RESERVED_440_420[7];
   UInt32 m_SA_2;                    // 0x0440
   UInt32 RESERVED_460_440[7];
   UInt32 m_SA_3;                    // 0x0460
   UInt32 RESERVED_480_460[7];
   UInt32 m_SA_4;                    // 0x0480
   UInt32 RESERVED_4A0_480[7];
   UInt32 m_SA_5;                    // 0x04A0
   UInt32 RESERVED_4C0_4A0[7];
   UInt32 m_SA_6;                    // 0x04C0
   UInt32 RESERVED_4E0_4C0[7];
   UInt32 m_SA_7;                    // 0x04E0
   UInt32 m_DA_0;                    // 0x0404
   UInt32 RESERVED_424_404[7];
   UInt32 m_DA_1;                    // 0x0424
   UInt32 RESERVED_444_424[7];
   UInt32 m_DA_2;                    // 0x0444
   UInt32 RESERVED_464_444[7];
   UInt32 m_DA_3;                    // 0x0464
   UInt32 RESERVED_484_464[7];
   UInt32 m_DA_4;                    // 0x0484
   UInt32 RESERVED_4A4_484[7];
   UInt32 m_DA_5;                    // 0x04A4
   UInt32 RESERVED_4C4_4A4[7];
   UInt32 m_DA_6;                    // 0x04C4
   UInt32 RESERVED_4E4_4C4[7];
   UInt32 m_DA_7;                    // 0x04E4
   UInt32 m_CC_0;                    // 0x0408
   UInt32 RESERVED_428_408[7];
   UInt32 m_CC_1;                    // 0x0428
   UInt32 RESERVED_448_428[7];
   UInt32 m_CC_2;                    // 0x0448
   UInt32 RESERVED_468_448[7];
   UInt32 m_CC_3;                    // 0x0468
   UInt32 RESERVED_488_468[7];
   UInt32 m_CC_4;                    // 0x0488
   UInt32 RESERVED_4A8_488[7];
   UInt32 m_CC_5;                    // 0x04A8
   UInt32 RESERVED_4C8_4A8[7];
   UInt32 m_CC_6;                    // 0x04C8
   UInt32 RESERVED_4E8_4C8[7];
   UInt32 m_CC_7;                    // 0x04E8
   UInt32 m_LC0_0;                   // 0x040C
   UInt32 RESERVED_42C_40C[7];
   UInt32 m_LC0_1;                   // 0x042C
   UInt32 RESERVED_44C_42C[7];
   UInt32 m_LC0_2;                   // 0x044C
   UInt32 RESERVED_46C_44C[7];
   UInt32 m_LC0_3;                   // 0x046C
   UInt32 RESERVED_48C_46C[7];
   UInt32 m_LC0_4;                   // 0x048C
   UInt32 RESERVED_4AC_48C[7];
   UInt32 m_LC0_5;                   // 0x04AC
   UInt32 RESERVED_4CC_4AC[7];
   UInt32 m_LC0_6;                   // 0x04CC
   UInt32 RESERVED_4EC_4CC[7];
   UInt32 m_LC0_7;                   // 0x04EC
   UInt32 m_LC1_0;                   // 0x0410
   UInt32 RESERVED_430_410[7];
   UInt32 m_LC1_1;                   // 0x0430
   UInt32 RESERVED_450_430[7];
   UInt32 m_LC1_2;                   // 0x0450
   UInt32 RESERVED_470_450[7];
   UInt32 m_LC1_3;                   // 0x0470
   UInt32 RESERVED_490_470[7];
   UInt32 m_LC1_4;                   // 0x0490
   UInt32 RESERVED_4B0_490[7];
   UInt32 m_LC1_5;                   // 0x04B0
   UInt32 RESERVED_4D0_4B0[7];
   UInt32 m_LC1_6;                   // 0x04D0
   UInt32 RESERVED_4F0_4D0[7];
   UInt32 m_LC1_7;                   // 0x04F0
   UInt32 RESERVED_D00_4F0[515];
   UInt32 m_DBGSTATUS;               // 0x0D00
   UInt32 m_DBGCMD;                  // 0x0D04
   UInt32 m_DBGINST0;                // 0x0D08
   UInt32 m_DBGINST1;                // 0x0D0C
   UInt32 RESERVED_E00_D0C[60];
   UInt32 m_CR0;                     // 0x0E00
   UInt32 m_CR1;                     // 0x0E04
   UInt32 m_CR2;                     // 0x0E08
   UInt32 m_CR3;                     // 0x0E0C
   UInt32 m_CR4;                     // 0x0E10
   UInt32 m_CRDN;                    // 0x0E14
} BRCM_NON_DMAC_REGS;


#endif /* __BRCM_RDB_NON_DMAC_H__ */



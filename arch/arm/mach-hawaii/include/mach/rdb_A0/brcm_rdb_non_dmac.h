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

#endif /* __BRCM_RDB_NON_DMAC_H__ */



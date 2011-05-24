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
/*     Date     : Generated on 4/25/2011 11:6:8                                             */
/*     RDB file : /projects/BIGISLAND/revA0                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_NAND_H__
#define __BRCM_RDB_NAND_H__

#define NAND_COMMAND_OFFSET                                               0x00000000
#define NAND_COMMAND_TYPE                                                 UInt32
#define NAND_COMMAND_RESERVED_MASK                                        0xFFFE0000
#define    NAND_COMMAND_VALID_SHIFT                                       16
#define    NAND_COMMAND_VALID_MASK                                        0x00010000
#define    NAND_COMMAND_OPER_CODE_SHIFT                                   0
#define    NAND_COMMAND_OPER_CODE_MASK                                    0x0000FFFF

#define NAND_ADDRESS_OFFSET                                               0x00000004
#define NAND_ADDRESS_TYPE                                                 UInt32
#define NAND_ADDRESS_RESERVED_MASK                                        0x00000000
#define    NAND_ADDRESS_ADDR_SHIFT                                        0
#define    NAND_ADDRESS_ADDR_MASK                                         0xFFFFFFFF

#define NAND_ATTRI0_OFFSET                                                0x00000008
#define NAND_ATTRI0_TYPE                                                  UInt32
#define NAND_ATTRI0_RESERVED_MASK                                         0x00000000
#define    NAND_ATTRI0_ATTRB0_SHIFT                                       0
#define    NAND_ATTRI0_ATTRB0_MASK                                        0xFFFFFFFF

#define NAND_ATTRI1_OFFSET                                                0x0000000C
#define NAND_ATTRI1_TYPE                                                  UInt32
#define NAND_ATTRI1_RESERVED_MASK                                         0x00000000
#define    NAND_ATTRI1_ATTRB1_SHIFT                                       0
#define    NAND_ATTRI1_ATTRB1_MASK                                        0xFFFFFFFF

#define NAND_BANK_OFFSET                                                  0x00000010
#define NAND_BANK_TYPE                                                    UInt32
#define NAND_BANK_RESERVED_MASK                                           0xFFFFFFF8
#define    NAND_BANK_BANK_SHIFT                                           0
#define    NAND_BANK_BANK_MASK                                            0x00000007

#define NAND_CONTROL_OFFSET                                               0x00000014
#define NAND_CONTROL_TYPE                                                 UInt32
#define NAND_CONTROL_RESERVED_MASK                                        0x17003F80
#define    NAND_CONTROL_RESETALL_SHIFT                                    31
#define    NAND_CONTROL_RESETALL_MASK                                     0x80000000
#define    NAND_CONTROL_DMA_RESET_SHIFT                                   30
#define    NAND_CONTROL_DMA_RESET_MASK                                    0x40000000
#define    NAND_CONTROL_ECC_RST_SHIFT                                     29
#define    NAND_CONTROL_ECC_RST_MASK                                      0x20000000
#define    NAND_CONTROL_WP_SHIFT                                          27
#define    NAND_CONTROL_WP_MASK                                           0x08000000
#define    NAND_CONTROL_BANK_RST_SHIFT                                    16
#define    NAND_CONTROL_BANK_RST_MASK                                     0x00FF0000
#define    NAND_CONTROL_STARTSTOP_SHIFT                                   15
#define    NAND_CONTROL_STARTSTOP_MASK                                    0x00008000
#define    NAND_CONTROL_DMA_STARTSTOP_SHIFT                               14
#define    NAND_CONTROL_DMA_STARTSTOP_MASK                                0x00004000
#define    NAND_CONTROL_ECC_BYP_SHIFT                                     6
#define    NAND_CONTROL_ECC_BYP_MASK                                      0x00000040
#define    NAND_CONTROL_ECC_HM_SHIFT                                      5
#define    NAND_CONTROL_ECC_HM_MASK                                       0x00000020
#define    NAND_CONTROL_ECC_RS_SHIFT                                      4
#define    NAND_CONTROL_ECC_RS_MASK                                       0x00000010
#define    NAND_CONTROL_TRAN_DIR_SHIFT                                    3
#define    NAND_CONTROL_TRAN_DIR_MASK                                     0x00000008
#define    NAND_CONTROL_TEST_MODE_SHIFT                                   2
#define    NAND_CONTROL_TEST_MODE_MASK                                    0x00000004
#define    NAND_CONTROL_DMA_MODE_SHIFT                                    1
#define    NAND_CONTROL_DMA_MODE_MASK                                     0x00000002
#define    NAND_CONTROL_OP_MODE_SHIFT                                     0
#define    NAND_CONTROL_OP_MODE_MASK                                      0x00000001

#define NAND_CONFIG0_OFFSET                                               0x00000018
#define NAND_CONFIG0_TYPE                                                 UInt32
#define NAND_CONFIG0_RESERVED_MASK                                        0xFFFFFF3E
#define    NAND_CONFIG0_AUX_DATA_TYPE_SHIFT                               6
#define    NAND_CONFIG0_AUX_DATA_TYPE_MASK                                0x000000C0
#define    NAND_CONFIG0_DATA_WIDTH_SHIFT                                  0
#define    NAND_CONFIG0_DATA_WIDTH_MASK                                   0x00000001

#define NAND_CONFIG1_OFFSET                                               0x0000001C
#define NAND_CONFIG1_TYPE                                                 UInt32
#define NAND_CONFIG1_RESERVED_MASK                                        0xFF000000
#define    NAND_CONFIG1_HOLD_TH_SHIFT                                     20
#define    NAND_CONFIG1_HOLD_TH_MASK                                      0x00F00000
#define    NAND_CONFIG1_SETUP_TS_SHIFT                                    16
#define    NAND_CONFIG1_SETUP_TS_MASK                                     0x000F0000
#define    NAND_CONFIG1_WR_CYCLE_TWH_SHIFT                                12
#define    NAND_CONFIG1_WR_CYCLE_TWH_MASK                                 0x0000F000
#define    NAND_CONFIG1_WR_PULSE_WIDTH_TWP_SHIFT                          8
#define    NAND_CONFIG1_WR_PULSE_WIDTH_TWP_MASK                           0x00000F00
#define    NAND_CONFIG1_RD_CYCLE_TREH_SHIFT                               4
#define    NAND_CONFIG1_RD_CYCLE_TREH_MASK                                0x000000F0
#define    NAND_CONFIG1_RD_PULSE_WIDTH_TRP_SHIFT                          0
#define    NAND_CONFIG1_RD_PULSE_WIDTH_TRP_MASK                           0x0000000F

#define NAND_STATUS_OFFSET                                                0x00000020
#define NAND_STATUS_TYPE                                                  UInt32
#define NAND_STATUS_RESERVED_MASK                                         0xFFFFFFF8
#define    NAND_STATUS_DMA_STATUS_SHIFT                                   2
#define    NAND_STATUS_DMA_STATUS_MASK                                    0x00000004
#define    NAND_STATUS_RX_FIFO_STATUS_SHIFT                               1
#define    NAND_STATUS_RX_FIFO_STATUS_MASK                                0x00000002
#define    NAND_STATUS_TX_FIFO_STAUS_SHIFT                                0
#define    NAND_STATUS_TX_FIFO_STAUS_MASK                                 0x00000001

#define NAND_IRQCTRL_OFFSET                                               0x0000002C
#define NAND_IRQCTRL_TYPE                                                 UInt32
#define NAND_IRQCTRL_RESERVED_MASK                                        0xFF0000E0
#define    NAND_IRQCTRL_BANK_CMPL_IRQ_EN_SHIFT                            16
#define    NAND_IRQCTRL_BANK_CMPL_IRQ_EN_MASK                             0x00FF0000
#define    NAND_IRQCTRL_BANK_ERR_IRQ_EN_SHIFT                             8
#define    NAND_IRQCTRL_BANK_ERR_IRQ_EN_MASK                              0x0000FF00
#define    NAND_IRQCTRL_RB_IRQ_EN_SHIFT                                   4
#define    NAND_IRQCTRL_RB_IRQ_EN_MASK                                    0x00000010
#define    NAND_IRQCTRL_HM_IRQ_EN_SHIFT                                   3
#define    NAND_IRQCTRL_HM_IRQ_EN_MASK                                    0x00000008
#define    NAND_IRQCTRL_RS_IRQ_EN_SHIFT                                   2
#define    NAND_IRQCTRL_RS_IRQ_EN_MASK                                    0x00000004
#define    NAND_IRQCTRL_DMA_CMPL_IRQ_EN_SHIFT                             1
#define    NAND_IRQCTRL_DMA_CMPL_IRQ_EN_MASK                              0x00000002
#define    NAND_IRQCTRL_DMA_ERROR_ENABLE_SHIFT                            0
#define    NAND_IRQCTRL_DMA_ERROR_ENABLE_MASK                             0x00000001

#define NAND_IRQSTATUS_OFFSET                                             0x00000030
#define NAND_IRQSTATUS_TYPE                                               UInt32
#define NAND_IRQSTATUS_RESERVED_MASK                                      0xFF0000E0
#define    NAND_IRQSTATUS_BANK_CMPL_IRQ_SHIFT                             16
#define    NAND_IRQSTATUS_BANK_CMPL_IRQ_MASK                              0x00FF0000
#define    NAND_IRQSTATUS_BANK_ERR_IRQ_SHIFT                              8
#define    NAND_IRQSTATUS_BANK_ERR_IRQ_MASK                               0x0000FF00
#define    NAND_IRQSTATUS_RB_IRQ_SHIFT                                    4
#define    NAND_IRQSTATUS_RB_IRQ_MASK                                     0x00000010
#define    NAND_IRQSTATUS_HM_IRQ_SHIFT                                    3
#define    NAND_IRQSTATUS_HM_IRQ_MASK                                     0x00000008
#define    NAND_IRQSTATUS_RS_IRQ_SHIFT                                    2
#define    NAND_IRQSTATUS_RS_IRQ_MASK                                     0x00000004
#define    NAND_IRQSTATUS_DMA_CMPL_IRQ_SHIFT                              1
#define    NAND_IRQSTATUS_DMA_CMPL_IRQ_MASK                               0x00000002
#define    NAND_IRQSTATUS_DMA_ERROR_ENABLE_SHIFT                          0
#define    NAND_IRQSTATUS_DMA_ERROR_ENABLE_MASK                           0x00000001

#define NAND_PRDBASE_OFFSET                                               0x00000034
#define NAND_PRDBASE_TYPE                                                 UInt32
#define NAND_PRDBASE_RESERVED_MASK                                        0x00000000
#define    NAND_PRDBASE_PRD_BASE_ADDR_SHIFT                               0
#define    NAND_PRDBASE_PRD_BASE_ADDR_MASK                                0xFFFFFFFF

#define NAND_ERRLOG0_OFFSET                                               0x00000040
#define NAND_ERRLOG0_TYPE                                                 UInt32
#define NAND_ERRLOG0_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG0_V_SHIFT                                           31
#define    NAND_ERRLOG0_V_MASK                                            0x80000000
#define    NAND_ERRLOG0_M_SHIFT                                           30
#define    NAND_ERRLOG0_M_MASK                                            0x40000000
#define    NAND_ERRLOG0_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG0_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG0_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG0_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG0_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG0_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG1_OFFSET                                               0x00000044
#define NAND_ERRLOG1_TYPE                                                 UInt32
#define NAND_ERRLOG1_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG1_V_SHIFT                                           31
#define    NAND_ERRLOG1_V_MASK                                            0x80000000
#define    NAND_ERRLOG1_M_SHIFT                                           30
#define    NAND_ERRLOG1_M_MASK                                            0x40000000
#define    NAND_ERRLOG1_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG1_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG1_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG1_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG1_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG1_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG2_OFFSET                                               0x00000048
#define NAND_ERRLOG2_TYPE                                                 UInt32
#define NAND_ERRLOG2_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG2_V_SHIFT                                           31
#define    NAND_ERRLOG2_V_MASK                                            0x80000000
#define    NAND_ERRLOG2_M_SHIFT                                           30
#define    NAND_ERRLOG2_M_MASK                                            0x40000000
#define    NAND_ERRLOG2_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG2_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG2_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG2_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG2_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG2_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG3_OFFSET                                               0x0000004C
#define NAND_ERRLOG3_TYPE                                                 UInt32
#define NAND_ERRLOG3_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG3_V_SHIFT                                           31
#define    NAND_ERRLOG3_V_MASK                                            0x80000000
#define    NAND_ERRLOG3_M_SHIFT                                           30
#define    NAND_ERRLOG3_M_MASK                                            0x40000000
#define    NAND_ERRLOG3_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG3_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG3_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG3_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG3_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG3_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG4_OFFSET                                               0x00000050
#define NAND_ERRLOG4_TYPE                                                 UInt32
#define NAND_ERRLOG4_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG4_V_SHIFT                                           31
#define    NAND_ERRLOG4_V_MASK                                            0x80000000
#define    NAND_ERRLOG4_M_SHIFT                                           30
#define    NAND_ERRLOG4_M_MASK                                            0x40000000
#define    NAND_ERRLOG4_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG4_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG4_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG4_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG4_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG4_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG5_OFFSET                                               0x00000054
#define NAND_ERRLOG5_TYPE                                                 UInt32
#define NAND_ERRLOG5_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG5_V_SHIFT                                           31
#define    NAND_ERRLOG5_V_MASK                                            0x80000000
#define    NAND_ERRLOG5_M_SHIFT                                           30
#define    NAND_ERRLOG5_M_MASK                                            0x40000000
#define    NAND_ERRLOG5_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG5_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG5_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG5_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG5_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG5_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG6_OFFSET                                               0x00000058
#define NAND_ERRLOG6_TYPE                                                 UInt32
#define NAND_ERRLOG6_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG6_V_SHIFT                                           31
#define    NAND_ERRLOG6_V_MASK                                            0x80000000
#define    NAND_ERRLOG6_M_SHIFT                                           30
#define    NAND_ERRLOG6_M_MASK                                            0x40000000
#define    NAND_ERRLOG6_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG6_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG6_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG6_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG6_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG6_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_ERRLOG7_OFFSET                                               0x0000005C
#define NAND_ERRLOG7_TYPE                                                 UInt32
#define NAND_ERRLOG7_RESERVED_MASK                                        0x06000000
#define    NAND_ERRLOG7_V_SHIFT                                           31
#define    NAND_ERRLOG7_V_MASK                                            0x80000000
#define    NAND_ERRLOG7_M_SHIFT                                           30
#define    NAND_ERRLOG7_M_MASK                                            0x40000000
#define    NAND_ERRLOG7_ERR_TYP_SHIFT                                     27
#define    NAND_ERRLOG7_ERR_TYP_MASK                                      0x38000000
#define    NAND_ERRLOG7_CLR_ERR_SHIFT                                     24
#define    NAND_ERRLOG7_CLR_ERR_MASK                                      0x01000000
#define    NAND_ERRLOG7_ERR_ADDR_SHIFT                                    0
#define    NAND_ERRLOG7_ERR_ADDR_MASK                                     0x00FFFFFF

#define NAND_CONFIG2_OFFSET                                               0x00000080
#define NAND_CONFIG2_TYPE                                                 UInt32
#define NAND_CONFIG2_RESERVED_MASK                                        0xFFFF00F0
#define    NAND_CONFIG2_TRHZ_SHIFT                                        12
#define    NAND_CONFIG2_TRHZ_MASK                                         0x0000F000
#define    NAND_CONFIG2_TCEA_TREA_SHIFT                                   8
#define    NAND_CONFIG2_TCEA_TREA_MASK                                    0x00000F00
#define    NAND_CONFIG2_TOE_SHIFT                                         0
#define    NAND_CONFIG2_TOE_MASK                                          0x0000000F

#define NAND_DMAINT0_OFFSET                                               0x00000084
#define NAND_DMAINT0_TYPE                                                 UInt32
#define NAND_DMAINT0_RESERVED_MASK                                        0x00000000
#define    NAND_DMAINT0_REPEAT_BITS_SHIFT                                 16
#define    NAND_DMAINT0_REPEAT_BITS_MASK                                  0xFFFF0000
#define    NAND_DMAINT0_ENABLE_SHIFT                                      15
#define    NAND_DMAINT0_ENABLE_MASK                                       0x00008000
#define    NAND_DMAINT0_NO_OF_PLANE_SHIFT                                 13
#define    NAND_DMAINT0_NO_OF_PLANE_MASK                                  0x00006000
#define    NAND_DMAINT0_BANK_NUM_SHIFT                                    10
#define    NAND_DMAINT0_BANK_NUM_MASK                                     0x00001C00
#define    NAND_DMAINT0_MAIN_DATA_TYP_SHIFT                               5
#define    NAND_DMAINT0_MAIN_DATA_TYP_MASK                                0x000003E0
#define    NAND_DMAINT0_AUX_DATA_TYP_SHIFT                                0
#define    NAND_DMAINT0_AUX_DATA_TYP_MASK                                 0x0000001F

#define NAND_DMAINT1_OFFSET                                               0x00000088
#define NAND_DMAINT1_TYPE                                                 UInt32
#define NAND_DMAINT1_RESERVED_MASK                                        0x00000000
#define    NAND_DMAINT1_REPEAT_BITS_SHIFT                                 16
#define    NAND_DMAINT1_REPEAT_BITS_MASK                                  0xFFFF0000
#define    NAND_DMAINT1_ENABLE_SHIFT                                      15
#define    NAND_DMAINT1_ENABLE_MASK                                       0x00008000
#define    NAND_DMAINT1_NO_OF_PLANE_SHIFT                                 13
#define    NAND_DMAINT1_NO_OF_PLANE_MASK                                  0x00006000
#define    NAND_DMAINT1_BANK_NUM_SHIFT                                    10
#define    NAND_DMAINT1_BANK_NUM_MASK                                     0x00001C00
#define    NAND_DMAINT1_MAIN_DATA_TYP_SHIFT                               5
#define    NAND_DMAINT1_MAIN_DATA_TYP_MASK                                0x000003E0
#define    NAND_DMAINT1_AUX_DATA_TYP_SHIFT                                0
#define    NAND_DMAINT1_AUX_DATA_TYP_MASK                                 0x0000001F

#define NAND_DMAINT2_OFFSET                                               0x0000008C
#define NAND_DMAINT2_TYPE                                                 UInt32
#define NAND_DMAINT2_RESERVED_MASK                                        0x00000000
#define    NAND_DMAINT2_REPEAT_BITS_SHIFT                                 16
#define    NAND_DMAINT2_REPEAT_BITS_MASK                                  0xFFFF0000
#define    NAND_DMAINT2_ENABLE_SHIFT                                      15
#define    NAND_DMAINT2_ENABLE_MASK                                       0x00008000
#define    NAND_DMAINT2_NO_OF_PLANE_SHIFT                                 13
#define    NAND_DMAINT2_NO_OF_PLANE_MASK                                  0x00006000
#define    NAND_DMAINT2_BANK_NUM_SHIFT                                    10
#define    NAND_DMAINT2_BANK_NUM_MASK                                     0x00001C00
#define    NAND_DMAINT2_MAIN_DATA_TYP_SHIFT                               5
#define    NAND_DMAINT2_MAIN_DATA_TYP_MASK                                0x000003E0
#define    NAND_DMAINT2_AUX_DATA_TYP_SHIFT                                0
#define    NAND_DMAINT2_AUX_DATA_TYP_MASK                                 0x0000001F

#define NAND_DMAINT3_OFFSET                                               0x00000090
#define NAND_DMAINT3_TYPE                                                 UInt32
#define NAND_DMAINT3_RESERVED_MASK                                        0x00000000
#define    NAND_DMAINT3_REPEAT_BITS_SHIFT                                 16
#define    NAND_DMAINT3_REPEAT_BITS_MASK                                  0xFFFF0000
#define    NAND_DMAINT3_ENABLE_SHIFT                                      15
#define    NAND_DMAINT3_ENABLE_MASK                                       0x00008000
#define    NAND_DMAINT3_NO_OF_PLANE_SHIFT                                 13
#define    NAND_DMAINT3_NO_OF_PLANE_MASK                                  0x00006000
#define    NAND_DMAINT3_BANK_NUM_SHIFT                                    10
#define    NAND_DMAINT3_BANK_NUM_MASK                                     0x00001C00
#define    NAND_DMAINT3_MAIN_DATA_TYP_SHIFT                               5
#define    NAND_DMAINT3_MAIN_DATA_TYP_MASK                                0x000003E0
#define    NAND_DMAINT3_AUX_DATA_TYP_SHIFT                                0
#define    NAND_DMAINT3_AUX_DATA_TYP_MASK                                 0x0000001F

#define NAND_ECCSTATIS0_OFFSET                                            0x000000A4
#define NAND_ECCSTATIS0_TYPE                                              UInt32
#define NAND_ECCSTATIS0_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS0_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS0_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS0_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS0_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS0_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS0_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS0_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS0_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS0_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS0_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS0_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS0_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS0_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS0_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS0_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS0_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS1_OFFSET                                            0x000000A8
#define NAND_ECCSTATIS1_TYPE                                              UInt32
#define NAND_ECCSTATIS1_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS1_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS1_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS1_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS1_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS1_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS1_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS1_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS1_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS1_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS1_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS1_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS1_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS1_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS1_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS1_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS1_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS2_OFFSET                                            0x000000AC
#define NAND_ECCSTATIS2_TYPE                                              UInt32
#define NAND_ECCSTATIS2_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS2_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS2_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS2_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS2_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS2_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS2_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS2_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS2_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS2_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS2_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS2_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS2_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS2_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS2_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS2_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS2_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS3_OFFSET                                            0x000000B0
#define NAND_ECCSTATIS3_TYPE                                              UInt32
#define NAND_ECCSTATIS3_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS3_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS3_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS3_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS3_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS3_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS3_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS3_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS3_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS3_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS3_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS3_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS3_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS3_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS3_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS3_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS3_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS4_OFFSET                                            0x000000B4
#define NAND_ECCSTATIS4_TYPE                                              UInt32
#define NAND_ECCSTATIS4_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS4_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS4_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS4_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS4_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS4_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS4_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS4_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS4_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS4_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS4_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS4_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS4_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS4_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS4_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS4_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS4_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS5_OFFSET                                            0x000000B8
#define NAND_ECCSTATIS5_TYPE                                              UInt32
#define NAND_ECCSTATIS5_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS5_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS5_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS5_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS5_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS5_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS5_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS5_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS5_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS5_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS5_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS5_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS5_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS5_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS5_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS5_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS5_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS6_OFFSET                                            0x000000BC
#define NAND_ECCSTATIS6_TYPE                                              UInt32
#define NAND_ECCSTATIS6_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS6_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS6_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS6_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS6_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS6_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS6_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS6_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS6_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS6_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS6_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS6_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS6_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS6_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS6_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS6_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS6_SEC0_ERR_MASK                                  0x0000000F

#define NAND_ECCSTATIS7_OFFSET                                            0x000000C0
#define NAND_ECCSTATIS7_TYPE                                              UInt32
#define NAND_ECCSTATIS7_RESERVED_MASK                                     0x00000000
#define    NAND_ECCSTATIS7_SEC7_ERR_SHIFT                                 28
#define    NAND_ECCSTATIS7_SEC7_ERR_MASK                                  0xF0000000
#define    NAND_ECCSTATIS7_SEC6_ERR_SHIFT                                 24
#define    NAND_ECCSTATIS7_SEC6_ERR_MASK                                  0x0F000000
#define    NAND_ECCSTATIS7_SEC5_ERR_SHIFT                                 20
#define    NAND_ECCSTATIS7_SEC5_ERR_MASK                                  0x00F00000
#define    NAND_ECCSTATIS7_SEC4_ERR_SHIFT                                 16
#define    NAND_ECCSTATIS7_SEC4_ERR_MASK                                  0x000F0000
#define    NAND_ECCSTATIS7_SEC3_ERR_SHIFT                                 12
#define    NAND_ECCSTATIS7_SEC3_ERR_MASK                                  0x0000F000
#define    NAND_ECCSTATIS7_SEC2_ERR_SHIFT                                 8
#define    NAND_ECCSTATIS7_SEC2_ERR_MASK                                  0x00000F00
#define    NAND_ECCSTATIS7_SEC1_ERR_SHIFT                                 4
#define    NAND_ECCSTATIS7_SEC1_ERR_MASK                                  0x000000F0
#define    NAND_ECCSTATIS7_SEC0_ERR_SHIFT                                 0
#define    NAND_ECCSTATIS7_SEC0_ERR_MASK                                  0x0000000F

#define NAND_MINSTR_OFFSET                                                0x00000400
#define NAND_MINSTR_TYPE                                                  UInt32
#define NAND_MINSTR_RESERVED_MASK                                         0xFFFE0000
#define    NAND_MINSTR_VALID_SHIFT                                        16
#define    NAND_MINSTR_VALID_MASK                                         0x00010000
#define    NAND_MINSTR_MINST_OPCODE_SHIFT                                 8
#define    NAND_MINSTR_MINST_OPCODE_MASK                                  0x0000FF00
#define    NAND_MINSTR_MINST_OPRAND_SHIFT                                 0
#define    NAND_MINSTR_MINST_OPRAND_MASK                                  0x000000FF

#define NAND_MADDR0_OFFSET                                                0x00000404
#define NAND_MADDR0_TYPE                                                  UInt32
#define NAND_MADDR0_RESERVED_MASK                                         0x00000000
#define    NAND_MADDR0_OUTPUT_ADDR_SHIFT                                  0
#define    NAND_MADDR0_OUTPUT_ADDR_MASK                                   0xFFFFFFFF

#define NAND_MADDR1_OFFSET                                                0x00000408
#define NAND_MADDR1_TYPE                                                  UInt32
#define NAND_MADDR1_RESERVED_MASK                                         0xFFFF0000
#define    NAND_MADDR1_OUTUT_ADDR_SHIFT                                   0
#define    NAND_MADDR1_OUTUT_ADDR_MASK                                    0x0000FFFF

#define NAND_MRESP_OFFSET                                                 0x0000040C
#define NAND_MRESP_TYPE                                                   UInt32
#define NAND_MRESP_RESERVED_MASK                                          0xFFFFF900
#define    NAND_MRESP_TAKEN_SHIFT                                         10
#define    NAND_MRESP_TAKEN_MASK                                          0x00000400
#define    NAND_MRESP_DONE_SHIFT                                          9
#define    NAND_MRESP_DONE_MASK                                           0x00000200
#define    NAND_MRESP_NANDSTATUS_SHIFT                                    0
#define    NAND_MRESP_NANDSTATUS_MASK                                     0x000000FF

#define NAND_R1_OFFSET                                                    0x0000048C
#define NAND_R1_TYPE                                                      UInt32
#define NAND_R1_RESERVED_MASK                                             0xFFFF0000
#define    NAND_R1_R1_SHIFT                                               0
#define    NAND_R1_R1_MASK                                                0x0000FFFF

#define NAND_B1_OFFSET                                                    0x00000490
#define NAND_B1_TYPE                                                      UInt32
#define NAND_B1_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B1_R1_SHIFT                                               0
#define    NAND_B1_R1_MASK                                                0x0000FFFF

#define NAND_B2_OFFSET                                                    0x00000494
#define NAND_B2_TYPE                                                      UInt32
#define NAND_B2_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B2_R1_SHIFT                                               0
#define    NAND_B2_R1_MASK                                                0x0000FFFF

#define NAND_B3_OFFSET                                                    0x00000498
#define NAND_B3_TYPE                                                      UInt32
#define NAND_B3_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B3_R1_SHIFT                                               0
#define    NAND_B3_R1_MASK                                                0x0000FFFF

#define NAND_B4_OFFSET                                                    0x0000049C
#define NAND_B4_TYPE                                                      UInt32
#define NAND_B4_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B4_R1_SHIFT                                               0
#define    NAND_B4_R1_MASK                                                0x0000FFFF

#define NAND_B5_OFFSET                                                    0x000004A0
#define NAND_B5_TYPE                                                      UInt32
#define NAND_B5_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B5_R1_SHIFT                                               0
#define    NAND_B5_R1_MASK                                                0x0000FFFF

#define NAND_B6_OFFSET                                                    0x000004A4
#define NAND_B6_TYPE                                                      UInt32
#define NAND_B6_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B6_R1_SHIFT                                               0
#define    NAND_B6_R1_MASK                                                0x0000FFFF

#define NAND_B7_OFFSET                                                    0x000004A8
#define NAND_B7_TYPE                                                      UInt32
#define NAND_B7_RESERVED_MASK                                             0xFFFF0000
#define    NAND_B7_R1_SHIFT                                               0
#define    NAND_B7_R1_MASK                                                0x0000FFFF

#define NAND_RDFIFO_OFFSET                                                0x00000800
#define NAND_RDFIFO_TYPE                                                  UInt32
#define NAND_RDFIFO_RESERVED_MASK                                         0x00000000
#define    NAND_RDFIFO_RD_DATA_SHIFT                                      0
#define    NAND_RDFIFO_RD_DATA_MASK                                       0xFFFFFFFF

#define NAND_WRFIFO_OFFSET                                                0x00000804
#define NAND_WRFIFO_TYPE                                                  UInt32
#define NAND_WRFIFO_RESERVED_MASK                                         0x00000000
#define    NAND_WRFIFO_WR_DATA_SHIFT                                      0
#define    NAND_WRFIFO_WR_DATA_MASK                                       0xFFFFFFFF

#endif /* __BRCM_RDB_NAND_H__ */



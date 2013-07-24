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
/*     Date     : Generated on 6/27/2013 16:58:22                                             */
/*     RDB file : //JAVA/                                                                   */
/************************************************************************************************/

#ifndef __BRCM_RDB_I2C_MM_HS_H__
#define __BRCM_RDB_I2C_MM_HS_H__

#define I2C_MM_HS_CS_OFFSET                                               0x00000020
#define I2C_MM_HS_CS_TYPE                                                 UInt32
#define I2C_MM_HS_CS_RESERVED_MASK                                        0xFFFFFF30
#define    I2C_MM_HS_CS_SDA_SHIFT                                         7
#define    I2C_MM_HS_CS_SDA_MASK                                          0x00000080
#define    I2C_MM_HS_CS_SCL_SHIFT                                         6
#define    I2C_MM_HS_CS_SCL_MASK                                          0x00000040
#define    I2C_MM_HS_CS_ACK_SHIFT                                         3
#define    I2C_MM_HS_CS_ACK_MASK                                          0x00000008
#define       I2C_MM_HS_CS_ACK_CMD_GEN_START                              0x00000000
#define       I2C_MM_HS_CS_ACK_CMD_GEN_RESTART                            0x00000001
#define    I2C_MM_HS_CS_CMD_SHIFT                                         1
#define    I2C_MM_HS_CS_CMD_MASK                                          0x00000006
#define       I2C_MM_HS_CS_CMD_CMD_NO_ACTION                              0x00000000
#define       I2C_MM_HS_CS_CMD_CMD_START_RESTART                          0x00000001
#define       I2C_MM_HS_CS_CMD_CMD_STOP                                   0x00000002
#define       I2C_MM_HS_CS_CMD_CMD_READ_BYTE                              0x00000003
#define    I2C_MM_HS_CS_EN_SHIFT                                          0
#define    I2C_MM_HS_CS_EN_MASK                                           0x00000001
#define       I2C_MM_HS_CS_EN_CMD_RESET_BSC                               0x00000000
#define       I2C_MM_HS_CS_EN_CMD_ENABLE_BSC                              0x00000001

#define I2C_MM_HS_TIM_OFFSET                                              0x00000024
#define I2C_MM_HS_TIM_TYPE                                                UInt32
#define I2C_MM_HS_TIM_RESERVED_MASK                                       0xFFFFFF00
#define    I2C_MM_HS_TIM_PRESCALE_SHIFT                                   6
#define    I2C_MM_HS_TIM_PRESCALE_MASK                                    0x000000C0
#define       I2C_MM_HS_TIM_PRESCALE_CMD_DIV8                             0x00000000
#define       I2C_MM_HS_TIM_PRESCALE_CMD_DIV4                             0x00000001
#define       I2C_MM_HS_TIM_PRESCALE_CMD_DIV2                             0x00000002
#define       I2C_MM_HS_TIM_PRESCALE_CMD_DIV1                             0x00000003
#define    I2C_MM_HS_TIM_P_SHIFT                                          3
#define    I2C_MM_HS_TIM_P_MASK                                           0x00000038
#define    I2C_MM_HS_TIM_NO_DIV_SHIFT                                     2
#define    I2C_MM_HS_TIM_NO_DIV_MASK                                      0x00000004
#define    I2C_MM_HS_TIM_DIV_SHIFT                                        0
#define    I2C_MM_HS_TIM_DIV_MASK                                         0x00000003
#define       I2C_MM_HS_TIM_DIV_CMD_DIV16                                 0x00000000
#define       I2C_MM_HS_TIM_DIV_CMD_DIV8                                  0x00000001
#define       I2C_MM_HS_TIM_DIV_CMD_DIV4                                  0x00000002
#define       I2C_MM_HS_TIM_DIV_CMD_DIV2                                  0x00000003

#define I2C_MM_HS_DAT_OFFSET                                              0x00000028
#define I2C_MM_HS_DAT_TYPE                                                UInt32
#define I2C_MM_HS_DAT_RESERVED_MASK                                       0xFFFFFF00
#define    I2C_MM_HS_DAT_DAT_SHIFT                                        0
#define    I2C_MM_HS_DAT_DAT_MASK                                         0x000000FF

#define I2C_MM_HS_TOUT_OFFSET                                             0x0000002C
#define I2C_MM_HS_TOUT_TYPE                                               UInt32
#define I2C_MM_HS_TOUT_RESERVED_MASK                                      0xFFFFF800
#define    I2C_MM_HS_TOUT_TOUT_HIGH_SHIFT                                 8
#define    I2C_MM_HS_TOUT_TOUT_HIGH_MASK                                  0x00000700
#define    I2C_MM_HS_TOUT_TE_SHIFT                                        7
#define    I2C_MM_HS_TOUT_TE_MASK                                         0x00000080
#define    I2C_MM_HS_TOUT_TOUT_LOW_SHIFT                                  0
#define    I2C_MM_HS_TOUT_TOUT_LOW_MASK                                   0x0000007F

#define I2C_MM_HS_RCM_OFFSET                                              0x00000030
#define I2C_MM_HS_RCM_TYPE                                                UInt32
#define I2C_MM_HS_RCM_RESERVED_MASK                                       0xFFFFFFFC
#define    I2C_MM_HS_RCM_LSB0_SHIFT                                       1
#define    I2C_MM_HS_RCM_LSB0_MASK                                        0x00000002
#define    I2C_MM_HS_RCM_EN_SHIFT                                         0
#define    I2C_MM_HS_RCM_EN_MASK                                          0x00000001

#define I2C_MM_HS_RCP_OFFSET                                              0x00000034
#define I2C_MM_HS_RCP_TYPE                                                UInt32
#define I2C_MM_HS_RCP_RESERVED_MASK                                       0xFFFFFF00
#define    I2C_MM_HS_RCP_POLY_SHIFT                                       0
#define    I2C_MM_HS_RCP_POLY_MASK                                        0x000000FF

#define I2C_MM_HS_RCD_OFFSET                                              0x00000038
#define I2C_MM_HS_RCD_TYPE                                                UInt32
#define I2C_MM_HS_RCD_RESERVED_MASK                                       0xFFFFFF00
#define    I2C_MM_HS_RCD_CRC_SHIFT                                        0
#define    I2C_MM_HS_RCD_CRC_MASK                                         0x000000FF

#define I2C_MM_HS_TXFCR_OFFSET                                            0x0000003C
#define I2C_MM_HS_TXFCR_TYPE                                              UInt32
#define I2C_MM_HS_TXFCR_RESERVED_MASK                                     0xFFFFFF3F
#define    I2C_MM_HS_TXFCR_FIFO_FLUSH_SHIFT                               7
#define    I2C_MM_HS_TXFCR_FIFO_FLUSH_MASK                                0x00000080
#define    I2C_MM_HS_TXFCR_FIFO_EN_SHIFT                                  6
#define    I2C_MM_HS_TXFCR_FIFO_EN_MASK                                   0x00000040

#define I2C_MM_HS_TXFIFORDOUT_OFFSET                                      0x00000040
#define I2C_MM_HS_TXFIFORDOUT_TYPE                                        UInt32
#define I2C_MM_HS_TXFIFORDOUT_RESERVED_MASK                               0xFFFFFF00
#define    I2C_MM_HS_TXFIFORDOUT_FIFO_RDOUT_SHIFT                         0
#define    I2C_MM_HS_TXFIFORDOUT_FIFO_RDOUT_MASK                          0x000000FF

#define I2C_MM_HS_IER_OFFSET                                              0x00000044
#define I2C_MM_HS_IER_TYPE                                                UInt32
#define I2C_MM_HS_IER_RESERVED_MASK                                       0xFFFFFF60
#define    I2C_MM_HS_IER_CMDBUSY_INT_EN_SHIFT                             7
#define    I2C_MM_HS_IER_CMDBUSY_INT_EN_MASK                              0x00000080
#define    I2C_MM_HS_IER_READ_COMPLETE_INT_SHIFT                          4
#define    I2C_MM_HS_IER_READ_COMPLETE_INT_MASK                           0x00000010
#define    I2C_MM_HS_IER_I2C_INT_EN_SHIFT                                 3
#define    I2C_MM_HS_IER_I2C_INT_EN_MASK                                  0x00000008
#define    I2C_MM_HS_IER_ERR_INT_EN_SHIFT                                 2
#define    I2C_MM_HS_IER_ERR_INT_EN_MASK                                  0x00000004
#define    I2C_MM_HS_IER_FIFO_INT_EN_SHIFT                                1
#define    I2C_MM_HS_IER_FIFO_INT_EN_MASK                                 0x00000002
#define    I2C_MM_HS_IER_NOACK_EN_SHIFT                                   0
#define    I2C_MM_HS_IER_NOACK_EN_MASK                                    0x00000001

#define I2C_MM_HS_ISR_OFFSET                                              0x00000048
#define I2C_MM_HS_ISR_TYPE                                                UInt32
#define I2C_MM_HS_ISR_RESERVED_MASK                                       0xFFFFFF60
#define    I2C_MM_HS_ISR_CMDBUSY_SHIFT                                    7
#define    I2C_MM_HS_ISR_CMDBUSY_MASK                                     0x00000080
#define    I2C_MM_HS_ISR_READ_COMPLETE_SHIFT                              4
#define    I2C_MM_HS_ISR_READ_COMPLETE_MASK                               0x00000010
#define    I2C_MM_HS_ISR_SES_DONE_SHIFT                                   3
#define    I2C_MM_HS_ISR_SES_DONE_MASK                                    0x00000008
#define    I2C_MM_HS_ISR_ERR_SHIFT                                        2
#define    I2C_MM_HS_ISR_ERR_MASK                                         0x00000004
#define    I2C_MM_HS_ISR_TXFIFOEMPTY_SHIFT                                1
#define    I2C_MM_HS_ISR_TXFIFOEMPTY_MASK                                 0x00000002
#define    I2C_MM_HS_ISR_NOACK_SHIFT                                      0
#define    I2C_MM_HS_ISR_NOACK_MASK                                       0x00000001

#define I2C_MM_HS_CLKEN_OFFSET                                            0x0000004C
#define I2C_MM_HS_CLKEN_TYPE                                              UInt32
#define I2C_MM_HS_CLKEN_RESERVED_MASK                                     0xFFFFFF00
#define    I2C_MM_HS_CLKEN_AUTOSENSE_OFF_SHIFT                            7
#define    I2C_MM_HS_CLKEN_AUTOSENSE_OFF_MASK                             0x00000080
#define    I2C_MM_HS_CLKEN_M_SHIFT                                        4
#define    I2C_MM_HS_CLKEN_M_MASK                                         0x00000070
#define    I2C_MM_HS_CLKEN_N_SHIFT                                        1
#define    I2C_MM_HS_CLKEN_N_MASK                                         0x0000000E
#define    I2C_MM_HS_CLKEN_CLKEN_SHIFT                                    0
#define    I2C_MM_HS_CLKEN_CLKEN_MASK                                     0x00000001

#define I2C_MM_HS_TXCOUNT_OFFSET                                          0x00000050
#define I2C_MM_HS_TXCOUNT_TYPE                                            UInt32
#define I2C_MM_HS_TXCOUNT_RESERVED_MASK                                   0xFFFFFF00
#define    I2C_MM_HS_TXCOUNT_FIFOCNT_SHIFT                                1
#define    I2C_MM_HS_TXCOUNT_FIFOCNT_MASK                                 0x000000FE
#define    I2C_MM_HS_TXCOUNT_STATUS_SHIFT                                 0
#define    I2C_MM_HS_TXCOUNT_STATUS_MASK                                  0x00000001

#define I2C_MM_HS_FIFO_STATUS_OFFSET                                      0x00000054
#define I2C_MM_HS_FIFO_STATUS_TYPE                                        UInt32
#define I2C_MM_HS_FIFO_STATUS_RESERVED_MASK                               0xFFFFFFE0
#define    I2C_MM_HS_FIFO_STATUS_TXFIFO_EMPTY_SHIFT                       4
#define    I2C_MM_HS_FIFO_STATUS_TXFIFO_EMPTY_MASK                        0x00000010
#define    I2C_MM_HS_FIFO_STATUS_TXFIFO_FULL_SHIFT                        3
#define    I2C_MM_HS_FIFO_STATUS_TXFIFO_FULL_MASK                         0x00000008
#define    I2C_MM_HS_FIFO_STATUS_RXFIFO_EMPTY_SHIFT                       2
#define    I2C_MM_HS_FIFO_STATUS_RXFIFO_EMPTY_MASK                        0x00000004
#define    I2C_MM_HS_FIFO_STATUS_RXFIFO_FULL_SHIFT                        1
#define    I2C_MM_HS_FIFO_STATUS_RXFIFO_FULL_MASK                         0x00000002
#define    I2C_MM_HS_FIFO_STATUS_RETX_SHIFT                               0
#define    I2C_MM_HS_FIFO_STATUS_RETX_MASK                                0x00000001

#define I2C_MM_HS_HSTIM_OFFSET                                            0x00000058
#define I2C_MM_HS_HSTIM_TYPE                                              UInt32
#define I2C_MM_HS_HSTIM_RESERVED_MASK                                     0xFFFF0000
#define    I2C_MM_HS_HSTIM_HS_MODE_SHIFT                                  15
#define    I2C_MM_HS_HSTIM_HS_MODE_MASK                                   0x00008000
#define    I2C_MM_HS_HSTIM_HS_HOLD_SHIFT                                  10
#define    I2C_MM_HS_HSTIM_HS_HOLD_MASK                                   0x00007C00
#define    I2C_MM_HS_HSTIM_HS_HIGH_PHASE_SHIFT                            5
#define    I2C_MM_HS_HSTIM_HS_HIGH_PHASE_MASK                             0x000003E0
#define    I2C_MM_HS_HSTIM_HS_SETUP_SHIFT                                 0
#define    I2C_MM_HS_HSTIM_HS_SETUP_MASK                                  0x0000001F

#define I2C_MM_HS_PADCTL_OFFSET                                           0x0000005C
#define I2C_MM_HS_PADCTL_TYPE                                             UInt32
#define I2C_MM_HS_PADCTL_RESERVED_MASK                                    0xFFFFFFF3
#define    I2C_MM_HS_PADCTL_PULLUP_EN_SHIFT                               3
#define    I2C_MM_HS_PADCTL_PULLUP_EN_MASK                                0x00000008
#define    I2C_MM_HS_PADCTL_PAD_OUT_EN_SHIFT                              2
#define    I2C_MM_HS_PADCTL_PAD_OUT_EN_MASK                               0x00000004

#define I2C_MM_HS_SFTRST_OFFSET                                           0x00000060
#define I2C_MM_HS_SFTRST_TYPE                                             UInt32
#define I2C_MM_HS_SFTRST_RESERVED_MASK                                    0xFFFFFFFC
#define    I2C_MM_HS_SFTRST_SWRST_RDY_SHIFT                               1
#define    I2C_MM_HS_SFTRST_SWRST_RDY_MASK                                0x00000002
#define    I2C_MM_HS_SFTRST_SWRST_SHIFT                                   0
#define    I2C_MM_HS_SFTRST_SWRST_MASK                                    0x00000001

#define I2C_MM_HS_FILTER_OFFSET                                           0x00000064
#define I2C_MM_HS_FILTER_TYPE                                             UInt32
#define I2C_MM_HS_FILTER_RESERVED_MASK                                    0xFFFFFFF8
#define    I2C_MM_HS_FILTER_FILTER_READOUT_SHIFT                          2
#define    I2C_MM_HS_FILTER_FILTER_READOUT_MASK                           0x00000004
#define    I2C_MM_HS_FILTER_CLKFAST_EN_SHIFT                              1
#define    I2C_MM_HS_FILTER_CLKFAST_EN_MASK                               0x00000002
#define    I2C_MM_HS_FILTER_FILTER_ON_SHIFT                               0
#define    I2C_MM_HS_FILTER_FILTER_ON_MASK                                0x00000001

#define I2C_MM_HS_RXFCR_OFFSET                                            0x00000068
#define I2C_MM_HS_RXFCR_TYPE                                              UInt32
#define I2C_MM_HS_RXFCR_RESERVED_MASK                                     0xFFFFFF00
#define    I2C_MM_HS_RXFCR_NACK_EN_SHIFT                                  7
#define    I2C_MM_HS_RXFCR_NACK_EN_MASK                                   0x00000080
#define    I2C_MM_HS_RXFCR_READ_COUNT_SHIFT                               0
#define    I2C_MM_HS_RXFCR_READ_COUNT_MASK                                0x0000007F

#define I2C_MM_HS_RXFIFORDOUT_OFFSET                                      0x0000006C
#define I2C_MM_HS_RXFIFORDOUT_TYPE                                        UInt32
#define I2C_MM_HS_RXFIFORDOUT_RESERVED_MASK                               0xFFFFFF00
#define    I2C_MM_HS_RXFIFORDOUT_RXFIFO_RDOUT_SHIFT                       0
#define    I2C_MM_HS_RXFIFORDOUT_RXFIFO_RDOUT_MASK                        0x000000FF

#endif /* __BRCM_RDB_I2C_MM_HS_H__ */



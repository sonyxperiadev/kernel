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

#ifndef __BRCM_RDB_UARTB_H__
#define __BRCM_RDB_UARTB_H__

#define UARTB_RBR_THR_DLL_OFFSET                                          0x00000000
#define UARTB_RBR_THR_DLL_TYPE                                            UInt32
#define UARTB_RBR_THR_DLL_RESERVED_MASK                                   0xFFFFFF00
#define    UARTB_RBR_THR_DLL_RBR_RBR_SHIFT                                0
#define    UARTB_RBR_THR_DLL_RBR_RBR_MASK                                 0x000000FF
#define    UARTB_RBR_THR_DLL_THR_THR_SHIFT                                0
#define    UARTB_RBR_THR_DLL_THR_THR_MASK                                 0x000000FF
#define    UARTB_RBR_THR_DLL_DLL_DLL_SHIFT                                0
#define    UARTB_RBR_THR_DLL_DLL_DLL_MASK                                 0x000000FF

#define UARTB_DLH_IER_OFFSET                                              0x00000004
#define UARTB_DLH_IER_TYPE                                                UInt32
#define UARTB_DLH_IER_RESERVED_MASK                                       0xFFFFFF00
#define    UARTB_DLH_IER_DLH_DLH_SHIFT                                    0
#define    UARTB_DLH_IER_DLH_DLH_MASK                                     0x000000FF
#define    UARTB_DLH_IER_IER_PTIME_SHIFT                                  7
#define    UARTB_DLH_IER_IER_PTIME_MASK                                   0x00000080
#define    UARTB_DLH_IER_IER_EDSSI_SHIFT                                  3
#define    UARTB_DLH_IER_IER_EDSSI_MASK                                   0x00000008
#define    UARTB_DLH_IER_IER_ELSI_SHIFT                                   2
#define    UARTB_DLH_IER_IER_ELSI_MASK                                    0x00000004
#define    UARTB_DLH_IER_IER_ETBEI_SHIFT                                  1
#define    UARTB_DLH_IER_IER_ETBEI_MASK                                   0x00000002
#define    UARTB_DLH_IER_IER_ERBFI_SHIFT                                  0
#define    UARTB_DLH_IER_IER_ERBFI_MASK                                   0x00000001

#define UARTB_IIR_FCR_OFFSET                                              0x00000008
#define UARTB_IIR_FCR_TYPE                                                UInt32
#define UARTB_IIR_FCR_RESERVED_MASK                                       0xFFFFFF00
#define    UARTB_IIR_FCR_IIR_FIFOSE_SHIFT                                 6
#define    UARTB_IIR_FCR_IIR_FIFOSE_MASK                                  0x000000C0
#define    UARTB_IIR_FCR_IIR_IID_SHIFT                                    0
#define    UARTB_IIR_FCR_IIR_IID_MASK                                     0x0000000F
#define    UARTB_IIR_FCR_FCR_RT_SHIFT                                     6
#define    UARTB_IIR_FCR_FCR_RT_MASK                                      0x000000C0
#define    UARTB_IIR_FCR_FCR_TET_SHIFT                                    4
#define    UARTB_IIR_FCR_FCR_TET_MASK                                     0x00000030
#define    UARTB_IIR_FCR_FCR_DMAM_SHIFT                                   3
#define    UARTB_IIR_FCR_FCR_DMAM_MASK                                    0x00000008
#define    UARTB_IIR_FCR_FCR_XFIFOR_SHIFT                                 2
#define    UARTB_IIR_FCR_FCR_XFIFOR_MASK                                  0x00000004
#define    UARTB_IIR_FCR_FCR_RFIFOR_SHIFT                                 1
#define    UARTB_IIR_FCR_FCR_RFIFOR_MASK                                  0x00000002
#define    UARTB_IIR_FCR_FCR_FIFOE_SHIFT                                  0
#define    UARTB_IIR_FCR_FCR_FIFOE_MASK                                   0x00000001

#define UARTB_LCR_OFFSET                                                  0x0000000C
#define UARTB_LCR_TYPE                                                    UInt32
#define UARTB_LCR_RESERVED_MASK                                           0xFFFFFF20
#define    UARTB_LCR_DLAB_SHIFT                                           7
#define    UARTB_LCR_DLAB_MASK                                            0x00000080
#define    UARTB_LCR_BC_SHIFT                                             6
#define    UARTB_LCR_BC_MASK                                              0x00000040
#define    UARTB_LCR_EPS_SHIFT                                            4
#define    UARTB_LCR_EPS_MASK                                             0x00000010
#define    UARTB_LCR_PEN_SHIFT                                            3
#define    UARTB_LCR_PEN_MASK                                             0x00000008
#define    UARTB_LCR_STOP_SHIFT                                           2
#define    UARTB_LCR_STOP_MASK                                            0x00000004
#define    UARTB_LCR_DLS_SHIFT                                            0
#define    UARTB_LCR_DLS_MASK                                             0x00000003

#define UARTB_MCR_OFFSET                                                  0x00000010
#define UARTB_MCR_TYPE                                                    UInt32
#define UARTB_MCR_RESERVED_MASK                                           0xFFFFFFC0
#define    UARTB_MCR_AFCE_SHIFT                                           5
#define    UARTB_MCR_AFCE_MASK                                            0x00000020
#define    UARTB_MCR_LB_SHIFT                                             4
#define    UARTB_MCR_LB_MASK                                              0x00000010
#define    UARTB_MCR_OUT2_SHIFT                                           3
#define    UARTB_MCR_OUT2_MASK                                            0x00000008
#define    UARTB_MCR_OUT1_SHIFT                                           2
#define    UARTB_MCR_OUT1_MASK                                            0x00000004
#define    UARTB_MCR_RTS_SHIFT                                            1
#define    UARTB_MCR_RTS_MASK                                             0x00000002
#define    UARTB_MCR_DTR_SHIFT                                            0
#define    UARTB_MCR_DTR_MASK                                             0x00000001

#define UARTB_LSR_OFFSET                                                  0x00000014
#define UARTB_LSR_TYPE                                                    UInt32
#define UARTB_LSR_RESERVED_MASK                                           0xFFFFFF00
#define    UARTB_LSR_RFE_SHIFT                                            7
#define    UARTB_LSR_RFE_MASK                                             0x00000080
#define    UARTB_LSR_TEMT_SHIFT                                           6
#define    UARTB_LSR_TEMT_MASK                                            0x00000040
#define    UARTB_LSR_THRE_SHIFT                                           5
#define    UARTB_LSR_THRE_MASK                                            0x00000020
#define    UARTB_LSR_BI_SHIFT                                             4
#define    UARTB_LSR_BI_MASK                                              0x00000010
#define    UARTB_LSR_FE_SHIFT                                             3
#define    UARTB_LSR_FE_MASK                                              0x00000008
#define    UARTB_LSR_PE_SHIFT                                             2
#define    UARTB_LSR_PE_MASK                                              0x00000004
#define    UARTB_LSR_OE_SHIFT                                             1
#define    UARTB_LSR_OE_MASK                                              0x00000002
#define    UARTB_LSR_DR_SHIFT                                             0
#define    UARTB_LSR_DR_MASK                                              0x00000001

#define UARTB_MSR_OFFSET                                                  0x00000018
#define UARTB_MSR_TYPE                                                    UInt32
#define UARTB_MSR_RESERVED_MASK                                           0xFFFFFF00
#define    UARTB_MSR_DCD_SHIFT                                            7
#define    UARTB_MSR_DCD_MASK                                             0x00000080
#define    UARTB_MSR_RI_SHIFT                                             6
#define    UARTB_MSR_RI_MASK                                              0x00000040
#define    UARTB_MSR_DSR_SHIFT                                            5
#define    UARTB_MSR_DSR_MASK                                             0x00000020
#define    UARTB_MSR_CTS_SHIFT                                            4
#define    UARTB_MSR_CTS_MASK                                             0x00000010
#define    UARTB_MSR_DDCD_SHIFT                                           3
#define    UARTB_MSR_DDCD_MASK                                            0x00000008
#define    UARTB_MSR_TERI_SHIFT                                           2
#define    UARTB_MSR_TERI_MASK                                            0x00000004
#define    UARTB_MSR_DDSR_SHIFT                                           1
#define    UARTB_MSR_DDSR_MASK                                            0x00000002
#define    UARTB_MSR_DCTS_SHIFT                                           0
#define    UARTB_MSR_DCTS_MASK                                            0x00000001

#define UARTB_SCR_OFFSET                                                  0x0000001C
#define UARTB_SCR_TYPE                                                    UInt32
#define UARTB_SCR_RESERVED_MASK                                           0xFFFFFF00
#define    UARTB_SCR_SCR_SHIFT                                            0
#define    UARTB_SCR_SCR_MASK                                             0x000000FF

#define UARTB_USR_OFFSET                                                  0x0000007C
#define UARTB_USR_TYPE                                                    UInt32
#define UARTB_USR_RESERVED_MASK                                           0xFFFFFFE0
#define    UARTB_USR_RFF_SHIFT                                            4
#define    UARTB_USR_RFF_MASK                                             0x00000010
#define    UARTB_USR_RFNE_SHIFT                                           3
#define    UARTB_USR_RFNE_MASK                                            0x00000008
#define    UARTB_USR_TFE_SHIFT                                            2
#define    UARTB_USR_TFE_MASK                                             0x00000004
#define    UARTB_USR_TFNF_SHIFT                                           1
#define    UARTB_USR_TFNF_MASK                                            0x00000002
#define    UARTB_USR_BUSY_SHIFT                                           0
#define    UARTB_USR_BUSY_MASK                                            0x00000001

#define UARTB_TFL_OFFSET                                                  0x00000080
#define UARTB_TFL_TYPE                                                    UInt32
#define UARTB_TFL_RESERVED_MASK                                           0xFFFFFF00
#define    UARTB_TFL_TFL_SHIFT                                            0
#define    UARTB_TFL_TFL_MASK                                             0x000000FF

#define UARTB_RFL_OFFSET                                                  0x00000084
#define UARTB_RFL_TYPE                                                    UInt32
#define UARTB_RFL_RESERVED_MASK                                           0xFFFFFF00
#define    UARTB_RFL_RFL_SHIFT                                            0
#define    UARTB_RFL_RFL_MASK                                             0x000000FF

#define UARTB_HTX_OFFSET                                                  0x000000A4
#define UARTB_HTX_TYPE                                                    UInt32
#define UARTB_HTX_RESERVED_MASK                                           0xFFFFFFFE
#define    UARTB_HTX_HTX_SHIFT                                            0
#define    UARTB_HTX_HTX_MASK                                             0x00000001

#define UARTB_CID_OFFSET                                                  0x000000F4
#define UARTB_CID_TYPE                                                    UInt32
#define UARTB_CID_RESERVED_MASK                                           0x00000000
#define    UARTB_CID_CID_SHIFT                                            0
#define    UARTB_CID_CID_MASK                                             0xFFFFFFFF

#define UARTB_UCV_OFFSET                                                  0x000000F8
#define UARTB_UCV_TYPE                                                    UInt32
#define UARTB_UCV_RESERVED_MASK                                           0x00000000
#define    UARTB_UCV_UCV_SHIFT                                            0
#define    UARTB_UCV_UCV_MASK                                             0xFFFFFFFF

#define UARTB_PID_OFFSET                                                  0x000000FC
#define UARTB_PID_TYPE                                                    UInt32
#define UARTB_PID_RESERVED_MASK                                           0x00000000
#define    UARTB_PID_PID_SHIFT                                            0
#define    UARTB_PID_PID_MASK                                             0xFFFFFFFF

#define UARTB_UCR_OFFSET                                                  0x00000100
#define UARTB_UCR_TYPE                                                    UInt32
#define UARTB_UCR_RESERVED_MASK                                           0xFFFFFF80
#define    UARTB_UCR_PWR_DIS_SHIFT                                        6
#define    UARTB_UCR_PWR_DIS_MASK                                         0x00000040
#define    UARTB_UCR_PCLK_ALWAYS_ON_SHIFT                                 5
#define    UARTB_UCR_PCLK_ALWAYS_ON_MASK                                  0x00000020
#define    UARTB_UCR_PWR_STATE_SHIFT                                      4
#define    UARTB_UCR_PWR_STATE_MASK                                       0x00000010
#define    UARTB_UCR_DMAEN_SHIFT                                          3
#define    UARTB_UCR_DMAEN_MASK                                           0x00000008
#define    UARTB_UCR_DMA_SBSIZE_SHIFT                                     0
#define    UARTB_UCR_DMA_SBSIZE_MASK                                      0x00000007

#define UARTB_IRCR_OFFSET                                                 0x00000108
#define UARTB_IRCR_TYPE                                                   UInt32
#define UARTB_IRCR_RESERVED_MASK                                          0xFFFFFFCE
#define    UARTB_IRCR_IRWDEN_SHIFT                                        4
#define    UARTB_IRCR_IRWDEN_MASK                                         0x00000030
#define    UARTB_IRCR_IREN_SHIFT                                          0
#define    UARTB_IRCR_IREN_MASK                                           0x00000001

#define UARTB_UBABCSR_OFFSET                                              0x00000110
#define UARTB_UBABCSR_TYPE                                                UInt32
#define UARTB_UBABCSR_RESERVED_MASK                                       0xFFFF007C
#define    UARTB_UBABCSR_ABCLK_DIV_SHIFT                                  8
#define    UARTB_UBABCSR_ABCLK_DIV_MASK                                   0x0000FF00
#define    UARTB_UBABCSR_AB_CLK_EN_SHIFT                                  7
#define    UARTB_UBABCSR_AB_CLK_EN_MASK                                   0x00000080
#define    UARTB_UBABCSR_AB_INT_FLAG_SHIFT                                1
#define    UARTB_UBABCSR_AB_INT_FLAG_MASK                                 0x00000002
#define    UARTB_UBABCSR_AB_EN_SHIFT                                      0
#define    UARTB_UBABCSR_AB_EN_MASK                                       0x00000001

#define UARTB_UBABCNTR_OFFSET                                             0x00000114
#define UARTB_UBABCNTR_TYPE                                               UInt32
#define UARTB_UBABCNTR_RESERVED_MASK                                      0xFFFF0000
#define    UARTB_UBABCNTR_AB_DETECTED_VALUE_SHIFT                         0
#define    UARTB_UBABCNTR_AB_DETECTED_VALUE_MASK                          0x0000FFFF

#endif /* __BRCM_RDB_UARTB_H__ */



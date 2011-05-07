/***************************************************************************
*
* Copyright 2004 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*
****************************************************************************/
/**
*  @file   chal_uart.h
*
*  @brief  UART driver interface header file.
*
*  @note   Current interface is based on the capabilities of the ZEUS
*          DesignWare UART, which provides 16550 capability with
*          extended features:  Read FIFO depth, DMA capability, busy detect
*          interrupt, LSR.THRE bit redefinition, and powerdown while idle.
*          It does not have hardware support for autobaud operation.
*
*          Modem control interface may be added in the future.
*          Chal_uart_assert_rts_out( CHAL_HANDLE)
*          Chal_uart_deassert_rts_out( CHAL_HANDLE)
*          Chal_uart_assert_out2( CHAL_HANDLE)
*          Chal_uart_deassert_out2( CHAL_HANDLE)
*          Chal_uart_assert_out1( CHAL_HANDLE)
*          Chal_uart_deassert_out1( CHAL_HANDLE)
*          Chal_uart_assert_dtr( CHAL_HANDLE)
*          Chal_uart_deassert_dtr( CHAL_HANDLE)
*          Chal_uart_set_break( CHAL_HANDLE)
*
*          TBD DMA interface may be added in the future.
*
*          TBD read/write buffer fcts may be added in the future.
*
****************************************************************************/
#ifndef _CHAL_UART_H_
#define _CHAL_UART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/csp/chal_types.h>
#include <mach/csp/reg.h>
#include <mach/csp/mm_addr.h>

/**
 * @addtogroup cHAL_Interface
 * @{
 */

#define UARTB_LSR_OFFSET                                                  0x00000014


#define MAX_UART_PORT 4

#define UARTB_RBR_THR_DLL_OFFSET                                          0x00000000
#define    UARTB_LSR_THRE_MASK                                            0x00000020
#define    UARTB_LSR_TEMT_MASK                                            0x00000040

/****  UARTA  ****/
/*#define UART_RBR_THR_DLL                (UART_BASE_ADDR + 0x0000) */  /* Receive Buffer/Transmit Holding Register/Divi */
  #define UART_RBR_THR_DLL_reserved0                0xFFFFFF00                        /* reserved0  */
                                                                           /* Default: The control bit: LCR.DLAB == 0 */
  #define UART_RBR_THR_DLL_RBR_RBR                  0x00FF                            /* RO Avaiable when read with  */
                                                                                    /* The control bit: LCR.DLAB == 0 */
  #define UART_RBR_THR_DLL_THR_THR                  0x00FF                            /* WO Avaiable when write with */
                                                                                    /* The control bit: LCR.DLAB == 1 */
  #define UART_RBR_THR_DLL_DLL_DLL                  0x00FF                            /* RW Avaiable to read and wri */
/*#define UART_DLH_IER                    (UART_BASE_ADDR + 0x0004) */  /* Divisor Latch High/Interrupt Enable Register  */
  #define UART_DLH_IER_reserved0                    0xFFFFFF00                    /* reserved0  */
                                                                                    /* The control bit: LCR.DLAB == 1 */
  #define UART_DLH_IER_DLH_DLH                      0x00FF                        /* RW Avaiable to read and write w */
                                                                           /* Default: The control bit: LCR.DLAB == 0 */
  #define UART_DLH_IER_IER_PTIME                    0x0080                        /* RW Avaiable to read and write w */
  #define UART_DLH_IER_IER_reserved1                0x0070                        /* reserved1  */
  #define UART_DLH_IER_IER_EDSSI                    0x0008                        /* RW Avaiable to read and write w */
  #define UART_DLH_IER_IER_ELSI                     0x0004                        /* RW Avaiable to read and write w */
  #define UART_DLH_IER_IER_ETBEI                    0x0002                        /* RW Avaiable to read and write w */
  #define UART_DLH_IER_IER_ERBFI                    0x0001                        /* RW Avaiable to read and write w */
/*#define UART_IIR_FCR                    (UART_BASE_ADDR + 0x0008) */  /* Interrupt Identification/FIFO Control Registe */
  #define UART_IIR_FCR_reserved0                    0xFFFFFF00                    /* reserved0  */
                                                                                                 /* The control bit:  */
  #define UART_IIR_FCR_IIR_FIFOSE                   0x00C0                        /* RO Avaiable when read., FIFOs E */
  #define UART_IIR_FCR_IIR_reserved1                0x0030                        /* reserved1  */
  #define UART_IIR_FCR_IIR_IID                      0x000F                        /* RO Avaiable when read., Interru */
                                                                                                 /* The control bit:  */
  #define UART_IIR_FCR_FCR_RT                       0x00C0                        /* WO Avaiable when write., RCVR T */
  #define UART_IIR_FCR_FCR_TET                      0x0030                        /* WO Avaiable when write., TX Emp */
  #define UART_IIR_FCR_FCR_DMAM                     0x0008                        /* WO Avaiable when write., DMA Mo */
  #define UART_IIR_FCR_FCR_XFIFOR                   0x0004                        /* WO Avaiable when write., XMIT F */
  #define UART_IIR_FCR_FCR_RFIFOR                   0x0002                        /* WO Avaiable when write., RCVR F */
  #define UART_IIR_FCR_FCR_FIFOE                    0x0001                        /* WO Avaiable when write., FIFO E */
/* #define UART_LCR                        (UART_BASE_ADDR + 0x000C) */  /* Line Control Register */
  #define UART_LCR_reserved0                        0xFFFFFF00                /* reserved0  */
  #define UART_LCR_DLAB                             0x0080                    /* RW Read or Write., Divisor Latch Ac */
  #define UART_LCR_BC                               0x0040                    /* RW Read or Write., Break Control Bi */
  #define UART_LCR_reserved1                        0x0020                    /* reserved1  */
  #define UART_LCR_EPS                              0x0010                    /* RW Read or Write., Even Parity Sele */
  #define UART_LCR_PEN                              0x0008                    /* RW Read or Write., Parity Enable. W */
  #define UART_LCR_STOP                             0x0004                    /* RW Read or Write., Number of stop b */
  #define UART_LCR_DLS                              0x0003                    /* RW Read or Write., Data Length Sele */
/* #define UART_MCR                        (UART_BASE_ADDR + 0x0010) */  /* Modem Control Register */
  #define UART_MCR_reserved0                        0xFFFFFF80                /* reserved0  */
  #define UART_MCR_SIRE                             0x0040                    /* RW Read or Write., SIR Mode Enable. */
  #define UART_MCR_AFCE                             0x0020                    /* RW Read or Write., Auto Flow Contro */
  #define UART_MCR_LB                               0x0010                    /* RW Read or Write., LoopBack Bit. Th */
  #define UART_MCR_OUT2                             0x0008                    /* RW Read or Write., OUT2 controls th */
  #define UART_MCR_OUT1                             0x0004                    /* RW Read or Write., OUT1 controls th */
  #define UART_MCR_RTS                              0x0002                    /* RW Read or Write., Request to Send. */
  #define UART_MCR_DTR                              0x0001                    /* RW Read or Write., Data Terminal Re */
/* #define UART_LSR                        (UART_BASE_ADDR + 0x0014) */  /* Line Status Register */
  #define UART_LSR_reserved0                        0xFFFFFF00                /* reserved0  */
  #define UART_LSR_RFE                              0x0080                    /* RO Read Only., Receiver FIFO Error  */
  #define UART_LSR_TEMT                             0x0040                    /* RO Read Only., Transmitter Empty bi */
  #define UART_LSR_THRE                             0x0020                    /* RO Read Only., Transmit Holding Reg */
  #define UART_LSR_BI                               0x0010                    /* RO Read Only., Break Interrupt bit. */
  #define UART_LSR_FE                               0x0008                    /* RO Read Only., Framing Error bit. F */
  #define UART_LSR_PE                               0x0004                    /* RO Read Only., Parity Error bit. PE */
  #define UART_LSR_OE                               0x0002                    /* RO Read Only., Overrun error bit. O */
  #define UART_LSR_DR                               0x0001                    /* RO Read Only., Data Ready bit. DR i */
/* #define UART_MSR                        (UART_BASE_ADDR + 0x0018) */  /* Modem Status Register */
  #define UART_MSR_reserved0                        0xFFFFFF00                /* reserved0  */
  #define UART_MSR_DCD                              0x0080                    /* RO Read Only., Data Carrier Detect. */
  #define UART_MSR_RI                               0x0040                    /* RO Read Only., Ring Indicator. This */
  #define UART_MSR_DSR                              0x0020                    /* RO Read Only., Data Set Ready. This */
  #define UART_MSR_CTS                              0x0010                    /* RO Read Only., Clear to Send. This  */
  #define UART_MSR_DDCD                             0x0008                    /* RO Read Only., Delta Data Carrier D */
  #define UART_MSR_TERI                             0x0004                    /* RO Read Only., Trailing Edge of Rin */
  #define UART_MSR_DDSR                             0x0002                    /* RO Read Only., Delta Data Set Ready */
  #define UART_MSR_DCTS                             0x0001                    /* RO Read Only., Delta Clear to Send. */
/*#define UART_SCR                        (UART_BASE_ADDR + 0x001C) */  /* Scratchpad Register */
  #define UART_SCR_reserved0                        0xFFFFFF00                /* reserved0  */
  #define UART_SCR_SCR                              0x00FF                    /* RW Read or Write., Scratchpad regis */
/*#define UART_USR                        (UART_BASE_ADDR + 0x007C) */  /* UART Status Register */
  #define UART_USR_reserved0                        0xFFFFFFE0                /* reserved0  */
  #define UART_USR_RFF                              0x0010                    /* RO Read Only., RFF indicates that r */
  #define UART_USR_RFNE                             0x0008                    /* RO Read Only., Receive FIFO Not Emp */
  #define UART_USR_TFE                              0x0004                    /* RO Read Only., Transmit FIFO Empty. */
  #define UART_USR_TFNF                             0x0002                    /* RO Read Only., Transmit FIFO Not Fu */
  #define UART_USR_BUSY                             0x0001                    /* RO Read Only., UART Busy. BUSY indi */
/*#define UART_TFL                        (UART_BASE_ADDR + 0x0080) */  /* Transmit FIFO Level */
  #define UART_TFL_reserved0                        0xFFFFFF00                /* reserved0  */
  #define UART_TFL_TFL                              0x00FF                    /* RO Read Only., Transmit FIFO Level. */
/*#define UART_RFL                        (UART_BASE_ADDR + 0x0084) */  /* Receive FIFO Level */
  #define UART_RFL_reserved0                        0xFFFFFF00                /* reserved0  */
  #define UART_RFL_RFL                              0x00FF                    /* RO Read Only., Receive FIFO Level.  */
/*#define UART_HTX                        (UART_BASE_ADDR + 0x00A4)*/  /* Halt Tx */
  #define UART_HTX_reserved0                        0xFFFFFFFE                /* reserved0  */
  #define UART_HTX_HTX                              0x0001                    /* RW Read or Write., This register is */
/*#define UART_CID                        (UART_BASE_ADDR + 0x00F4) */  /* UART Configuration ID */
  #define UART_CID_CID                              0xFFFFFFFF                /* RO Read Only., Indicates this UART  */
/*#define UART_UCV                        (UART_BASE_ADDR + 0x00F8) */  /* UART Component Version */
  #define UART_UCV_UCV                              0xFFFFFFFF                /* RO Read Only., Indicates the UART h */
/*#define UART_PID                        (UART_BASE_ADDR + 0x00FC) */  /* Peripherals ID */
  #define UART_PID_PID                              0xFFFFFFFF                /* RO Read Only., This register contai */
/*#define UART_UCR                        (UART_BASE_ADDR + 0x0100) */  /* UAUCR Configuration Register */
  #define UART_UCR_reserved0                        0xFFFFFF80                /* reserved0  */
  #define UART_UCR_PWR_DIS                          0x0040                    /* RW 1: The APB clock and UART clock  */
  #define UART_UCR_reserved1                        0x0020                    /* reserved1  */
  #define UART_UCR_PWR_STATE                        0x0010                    /* RW Read Only., 1: The APB clock and */
  #define UART_UCR_DMAEN                            0x0008                    /* RW 1: Enable DMA hardware flow cont */
  #define UART_UCR_DMA_SBSIZE                       0x0007                    /* RW UARTB DMA burst size, 000 1, 001 */


/***************************************************************************
* typedef declarations
*
* Note: for simplicity, hardware dependent enums use values that exactly match
*       the values of the DesignWare UART block.  Other UARTS may need the
*       chal interface functions to translate these values internally.
*
****************************************************************************/

/**
* UART interrupt control register settings
****************************************************************************/
typedef enum
{
    UART_RX_INT_CTRL    = 0x01,   /*< Receive interrupt enable */
    UART_TX_INT_CTRL    = 0x02,   /*< Transmit interrupt enable */
    UART_LINE_INT_CTRL  = 0x04,   /*< Line status interrupt enable */
    UART_MODEM_INT_CTRL = 0x08    /*< Modem status interrupt enable */
}  UART_INT_CTRL_T;

/**
* UART interrupt status register - interrupt Ids
****************************************************************************/
typedef enum
{
    UART_MODEM_INT_STAT     = 0x00,   /*< Modem status interrupt pending */
    UART_NO_INT_STAT        = 0x01,   /*< No interrupt pending */
    UART_TX_INT_STAT        = 0x02,   /*< Transmit interrupt pending */
    UART_RX_INT_STAT        = 0x04,   /*< Receive interrupt pending */
    UART_LINE_INT_STAT      = 0x06,   /*< Line status interrupt pending */
    UART_RXTIMEOUT_INT_STAT = 0x0C    /*< Receive timeout interrupt pending */
}  UART_INT_STAT_T;

/**
* Bit definitions for line status
****************************************************************************/
typedef enum
{
    UART_LINE_STAT_DATA_RDY    = 0x01,   /*< Data ready and good in RBR */
    UART_LINE_STAT_OVERRUN_ERR = 0x02,   /*< Data overrun error in RBR */
    UART_LINE_STAT_PARITY_ERR  = 0x04,   /*< Data parity error in RBR */
    UART_LINE_STAT_FRAME_ERR   = 0x08,   /*< Data frame error in RBR */
    UART_LINE_STAT_BREAK       = 0x10,   /*< Break condition detected */
    UART_LINE_STAT_THRE        = 0x20,   /*< Transmit Holding Register Empty
                                             (do not use in FIFO mode) */
    UART_LINE_STAT_TEMT =        0x40,   /*< Transmit Empty */
    UART_LINE_STAT_RX_FIFO_ERR = 0x80    /*< Receive FIFO error */
} UART_LINE_STAT_T;

/**
* Bit definitions for modem status
****************************************************************************/
typedef enum
{
    UART_MODEM_STAT_DCTS = 0x01,   /*< Delta Clear To Send */
    UART_MODEM_STAT_DDSR = 0x02,   /*< Delta Data Set Ready */
    UART_MODEM_STAT_TERI = 0x04,   /*< Trailing Edge of Ring Indicator */
    UART_MODEM_STAT_DDCD = 0x08,   /*< Delta Data Carrier Detect */
    UART_MODEM_STAT_CTS  = 0x10,   /*< Clear To Send */
    UART_MODEM_STAT_DSR  = 0x20,   /*< Data Set Ready (warning: NC on ZEUS) */
    UART_MODEM_STAT_RI   = 0x40,   /*< Ring Indicate (warning: NC on ZEUS) */
    UART_MODEM_STAT_DCD  = 0x80    /*< Data Carrier Detect (warning: NC on ZEUS) */
} UART_MODEM_STAT_T;


/**
* Bit definitions for USR status
****************************************************************************/
typedef enum
{
    UART_USR_STAT_BUSY = 0x01,   /*< Busy indicates a serial transfer is in progress */
    UART_USR_STAT_TFNF = 0x02,   /*< Transmit FIFO not full */
    UART_USR_STAT_TFE  = 0x04,   /*< Transmit FIFO empty */
    UART_USR_STAT_RFNE = 0x08,   /*< Receive FIFO not full */
    UART_USR_STAT_RFF  = 0x10,   /*< RFF indicates that receive FIFO is completely full */
} UART_USR_STAT_T;


/**
* Receive FIFO threshold levels
****************************************************************************/
typedef enum
{
    UART_RX_1_CHAR     = 0x00<<6,   /*< 1 char in FIFO */
    UART_RX_QUARTER    = 0x01<<6,   /*< 1/4 full in FIFO */
    UART_RX_HALF       = 0x02<<6,   /*< 1/2 full in FIFO */
    UART_RX_FULL_SUB_2 = 0x03<<6,   /*< (full - 2) chars in FIFO */
}  UART_RX_LEVEL_T;

/**
* Transmit FIFO threshold levels
****************************************************************************/
typedef enum
{
    UART_TX_EMPTY   = 0x00<<4,   /*< empty FIFO */
    UART_TX_2_CHAR  = 0x01<<4,   /*< 2 chars in FIFO */
    UART_TX_QUARTER = 0x02<<4,   /*< 1/4 full in FIFO */
    UART_TX_HALF    = 0x03<<4,   /*< 1/2 full in FIFO */
}  UART_TX_LEVEL_T;

/**
* Baudrate support
****************************************************************************/
typedef enum
{
	BAUDRATE_2400,     /*<	0 = 2.4 Kbaud */
	BAUDRATE_4800,     /*<	1 = 4.8 Kbaud */
	BAUDRATE_9600,     /*<	2 = 9.6 Kbaud */
	BAUDRATE_19200,    /*<	3 = 19  Kbaud */
	BAUDRATE_38400,    /*<	4 = 38  Kbaud */
	BAUDRATE_57600,    /*<  5 = 57  Kbaud */
	BAUDRATE_115200,   /*<	6 = 115 Kbaud */
	BAUDRATE_128000,   /*<	7 = 128 Kbaud */
	BAUDRATE_230400,   /*<	8 = 230 Kbaud */
	BAUDRATE_460800,   /*<	9 = 460 Kbaud */
	BAUDRATE_921600,   /*<	A = 921 Kbaud */
	BAUDRATE_1228800,  /*<  B = 1.2 Mbaud */
	BAUDRATE_1843200,  /*<	C = 1.8 Mbaud */
	BAUDRATE_3686400,  /*<	D = 3.6 Mbaud */
	BAUDRATE_2000000,  /*<	E = 2   Mbaud */
	BAUDRATE_3000000,  /*<	F = 3   Mbaud */
	BAUDRATE_1500000,  /*<	x10 = 1.5 Mbaud */
	BAUDRATE_1329500,  /*<	x11 = 1.3 Mbaud */
	BAUDRATE_1413000,  /*<	x12 = 1.4 Mbaud */
	NUM_BAUDRATES      /*< Number of baudrates */
} BAUDRATE_T;

/**
* Parity support
****************************************************************************/
typedef enum
{
   PARITY_ODD   = 0x08,		/*< Odd Parity */
   PARITY_EVEN  = 0x18,		/*< Even Parity */
   PARITY_NONE  = 0x00		/*< No Parity */
} PARITY_T;

/**
* Number of character bits and stop bits
****************************************************************************/
typedef enum
{
   CHAR_5_STOP_1    = 0x00,	/*< 5 Bit Word - 1 Stop Bit */
   CHAR_6_STOP_1    = 0x01,	/*< 6 Bit Word - 1 Stop Bit */
   CHAR_7_STOP_1    = 0x02,	/*< 7 Bit Word - 1 Stop Bit */
   CHAR_8_STOP_1    = 0x03,	/*< 8 Bit Word - 1 Stop Bit */
   CHAR_5_STOP_1p5  = 0x04,	/*< 5 Bit Word - 1.5 Stop Bit */
   CHAR_6_STOP_2    = 0x05,	/*< 6 Bit Word - 2 Stop Bits */
   CHAR_7_STOP_2    = 0x06,	/*< 6 Bit Word - 1 Stop Bit */
   CHAR_8_STOP_2    = 0x07	/*< 8 Bit Word - 2 Stop Bits */
} CHAR_STOP_BITS_T;

/**
* Source Burst Size
****************************************************************************/
typedef enum
{
   SBSIZE_1         = 0x00,	/*< Enable DMA mode with burst size 1 byte */
   SBSIZE_4         = 0x01,	/*< Enable DMA mode with burst size 4 bytes */
   SBSIZE_8         = 0x02,	/*< Enable DMA mode with burst size 8 bytes */
   SBSIZE_16        = 0x03,	/*< Enable DMA mode with burst size 16 bytes */
   SBSIZE_32        = 0x04,	/*< Enable DMA mode with burst size 32 bytes */
   SBSIZE_64        = 0x05,	/*< Enable DMA mode with burst size 64 bytes (FIXME: RDB says this is 32 bytes too) */
   SBSIZE_128       = 0x06,	/*< Enable DMA mode with burst size 128 bytes */
   SBSIZE_256       = 0x07      /*< Enable DMA mode with burst size 256 bytes */
} UART_SBSIZE_T;

/**
* UART configuration settings
*
* Note: UART configuration is same whether operating in polled or
*       interrupt-driven.  Interrupt driven operation is enabled
*       after the UART is configured.
****************************************************************************/
typedef struct
{
    _Bool enable;                /*< TRUE = normal operations enabled */
                                   /*< FALSE = disabled means no rx/tx serial */
                                   /*<         activity but not powered down. */

    _Bool loopback;              /*< TRUE = loopback mode for BSP testing */
                                   /*< FALSE = normal operations */

    _Bool autoflow;              /*< TRUE = automatic flow control enabled. */
                                   /*<        RTS output asserted when we are */
                                   /*<        ready to Rx and we transmit */
                                   /*<        when CTS input asserted by other */
                                   /*<        side. */
                                   /*< FALSE = autoflow disabled */

    BAUDRATE_T baudrate;           /*< baud rate selected */
    CHAR_STOP_BITS_T char_stop_bits;  /*< char and stop bits selected */
    PARITY_T parity;               /*< parity bit selected */
    UART_RX_LEVEL_T rx_level;      /*< rx FIFO threshold level */
    UART_TX_LEVEL_T tx_level;      /*< tx FIFO threshold level */
    _Bool fifomode;              /*< TRUE = Enable FIFO mode, rx_lelel and tx_level are used */
                                   /*< FALSE = disable FIFO mode, rx_lelel and tx_level has no effect */
    _Bool dmamode;               /*< TRUE = Enable DMA mode, sbsize field is used */
                                   /*< FALSE = disable DMA mode, sbsize has no effect */
    UART_SBSIZE_T sbsize;          /*< Source Burst when DMA is enabled */
} chal_uart_config_t;

/*typedef void * CHAL_HANDLE;*/    /* handle is opaque */


/**
*
*  @brief  Initialize device info with base_addr.
*
*  @param  port             (in) = 0 if A,  = 1 if B
*  @param  UARTx_base_addr  (in) UARTx device base address
*
*  @return h           UART device handle
*
*  @note   Does not write device registers.
*          Must call before any other function.
****************************************************************************/
CHAL_HANDLE chal_uart_init( uint32_t port, uint32_t UARTx_base_addr);

/**
*
*  @brief  Configure UART with desired settings.
*
*  @param  h     (in) UART device handle
*  @param  cfg   (in) configuration settings
*
*  @return TRUE = done, FALSE = fails due to bad configuration setting.
*
*  @note   Must be called after init, before other fcts.
*          May be called again later to reconfigure device.
*
*          Warning: Caller must protect this code from interrupts/reschedule.
****************************************************************************/
_Bool chal_uart_config( CHAL_HANDLE h, chal_uart_config_t *cfg);

/*==========================================
*====== Simple high level io functions =====
==========================================*/

/**
*
*  @brief  Poll UART and read char from Rx FIFO, if available.
*
*  @param  h     (in) UART device handle
*  @param  *ch  (out) char from UART, if available
*
*  @return =TRUE if *ch=char, =FALSE if no char available
*
*  @note   Side-effect of changing line status for next character, and
*          clearing RX, RXTIMEOUT interrupts if sufficient number
*          of chars read.
****************************************************************************/
_Bool chal_uart_poll_char( CHAL_HANDLE h, uint8_t *ch);

/**
*
*  @brief  Receive char from Rx FIFO, wait until available.
*
*  @param  h     (in) UART device handle
*
*  @return char from UART
*
*  @note   Side-effect of changing line status for next character, and
*          clearing RX, RXTIMEOUT interrupts if sufficient number
*          of chars read.
****************************************************************************/
uint8_t chal_uart_recv_char( CHAL_HANDLE h);

/**
*
*  @brief  Send char to Tx FIFO, wait until not full.
*
*  @param  h     (in) UART device handle
*  @param  ch    (in) character
*
****************************************************************************/
void chal_uart_send_char( CHAL_HANDLE h, uint8_t ch );

/*==========================================
* ===== Low level interface functions =====
==========================================*/

/**
*
*  @brief  Read char from UART Rx FIFO.
*
*  @param  h   (in) UART device handle
*
*  @return character
*
*  @note   Side-effect of changing line status for next character, and
*          clearing RX, RXTIMEOUT interrupts if sufficient number
*          of chars read.
****************************************************************************/
uint8_t chal_uart_read_char( CHAL_HANDLE h);

/**
*
*  @brief  Write char to UART Tx FIFO.
*
*  @param  h    (in) UART device handle
*  @param  ch   (in) character to transmit
****************************************************************************/
void chal_uart_write_char( CHAL_HANDLE h, uint8_t ch);

/**
*
*  @brief  Read UART Line Status from Rx FIFO.  This status corresponds to
*          the first character in the Rx FIFO.  If a second character
*          exists in the Rx FIFO, then after reading the first character
*          from the Rx FIFO, the line status will change to correspond
*          to that second character in the FIFO.
*
*  @param  h   (in) UART device handle
*
*  @return Line status
*
*  @note   Side-effect of clearing error, break bits and LINE interrupt.
*          This should only be called just prior to reading the char.
*          This should not be called when transmitting a char,
*          or the receive line status may be lost.
*
*          Note that the UART_LINE_STAT_THRE bit changes definition
*          depending on the DesignWare UART mode, so do not use it.
*          Instead, use the chal_uart_get_tx_fifo_empty_count.
****************************************************************************/
UART_LINE_STAT_T chal_uart_read_line_status( CHAL_HANDLE h);

/**
*
*  @brief  Read UART Modem Status.
*
*  @param  h   (in) UART device handle
*
*  @return Modem status
*
*  @note   Side-effect of clearing delta bits and MODEM interrupt.
*          When operating in interrupt mode, this should only be called
*          from ISR, or modem status changes may be lost.
****************************************************************************/
UART_MODEM_STAT_T chal_uart_read_modem_status( CHAL_HANDLE h);

/**
*
*  @brief  Read UART interrupt status.
*
*  @param  h   (in) UART device handle
*
*  @return Highest priority active interrupt or NO_INT (no interrupt active)
*
*  @note   Side-effect of clearing TX interrupt.
*          Should only be called from ISR.
****************************************************************************/
UART_INT_STAT_T chal_uart_read_int_status( CHAL_HANDLE h);

/**
*
*  @brief  Enable UART interrupt at UART level only.
*
*  @param  h    (in) UART device handle
*  @param  int  selected interrupt
*
*  @note   Warning: Caller must protect this code from interrupts/reschedule.
****************************************************************************/
void chal_uart_enable_int( CHAL_HANDLE h, UART_INT_CTRL_T intr);

/**
*
*  @brief  Disable UART interrupt at UART level only.
*
*  @param  h   (in) UART device handle
*  @param  int (in) selected interrupt
*
*  @note   Warning: Caller must protect this code from interrupts/reschedule.
****************************************************************************/
void chal_uart_disable_int( CHAL_HANDLE h, UART_INT_CTRL_T intr);

/**
*
*  @brief  Get Tx FIFO empty count.
*
*  @param  h   (in) UART device handle
*
*  @return Number of chars which could be written to fill up the Tx FIFO.
****************************************************************************/
uint16_t chal_uart_get_tx_fifo_empty_count( CHAL_HANDLE h);

/**
*
*  @brief  Get Rx FIFO character count.
*
*  @param  h   (in) UART device handle
*
*  @return Number of chars which could be read to empty the Rx FIFO.
****************************************************************************/
uint16_t chal_uart_get_rx_fifo_char_count( CHAL_HANDLE h);

/**
*
*  @brief  Clear Rx characters from FIFO.
*
*  @param  h   (in) UART device handle
*
*  @note   Warning: Rx FIFO operations should be atomic.
*          It would be a problem if this function were called between
*          chal_uart_get_rx_fifo_char_count and chal_uart_read_char, or
*          chal_uart_read_line_status and chal_uart_read_char.
****************************************************************************/
void chal_uart_clear_rx_fifo( CHAL_HANDLE h);

/**
*
*  @brief  Clear Tx characters from FIFO.
*
*  @param  h   (in) UART device handle
*
*  @note   
****************************************************************************/
void chal_uart_clear_tx_fifo( CHAL_HANDLE h);

/**
*
*  @brief  Enable UART automatic powerdown when idle feature.
*
*  @param  h   (in) UART device handle
****************************************************************************/
void chal_uart_enable_powerdown( CHAL_HANDLE h);

/**
*
*  @brief  Disable UART automatic powerdown when idle feature.
*
*  @param  h   (in) UART device handle
****************************************************************************/
void chal_uart_disable_powerdown( CHAL_HANDLE h);

/**
*
*  @brief  Config uart modem config register.
*
*  @param  h   (in) UART device handle
*
*  @param  onoff   (in) enable or diable
*
*  @param  onoff   (in) bit fields to config
****************************************************************************/
void chal_uart_config_modem_status( CHAL_HANDLE h, _Bool onoff, uint8_t ctrl);

/**
*
*  @brief  Read UART status.
*
*  @param  h   (in) UART device handle
*
*  @return status
*
****************************************************************************/
UART_USR_STAT_T chal_uart_read_usr_status( CHAL_HANDLE h);

/**
*
*  @brief  Set UART baud rate.
*
*  @param  h   (in) UART device handle
*
*  @param  baud (in) bayd rate to be set
*
*  @return status
*
****************************************************************************/
_Bool chal_uart_set_baud( CHAL_HANDLE h, BAUDRATE_T baud);

/*===========================================================================*/
/* inline functions (used for uncompress.h only)*/

/****************************************************************************/
/**
*  @brief   Write Tx data to UARTB
*
*  @note    Blocks until able to write Tx data.  This does not append a newling
*
*  @return
*
*/
/****************************************************************************/
static inline void chal_uartb_tx
(
    unsigned    c       	/* [IN]  output value */
)
{
	/* data should be written to THR register only if THRE (LSR bit5) is set) */
	while (( REG32_READ(MM_ADDR_IO_UART0 + UARTB_LSR_OFFSET) & UARTB_LSR_THRE_MASK) == 0 )
	{
	}

	REG32_WRITE(MM_ADDR_IO_UART0 + UARTB_RBR_THR_DLL_OFFSET, (unsigned long)c);
}

/****************************************************************************/
/**
*  @brief   Wait util UARTB has finished Tx
*
*  @note    Blocks until Tx line is idle and Tx FIFO empty.
*
*  @return
*
*/
/****************************************************************************/
static inline void chal_uartb_tx_wait_idle
(
	void
)
{
	/* Wait for the tx fifo to be empty and last char to be sent */
	while (( REG32_READ(MM_ADDR_IO_UART0 + UARTB_LSR_OFFSET) & UARTB_LSR_TEMT_MASK) == 0 )
	{
	}
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _CHAL_UART_H_*/

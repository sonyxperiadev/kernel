//***************************************************************************
//
//	Copyright © 2004-2008 Broadcom Corporation
//	
//	This program is the proprietary software of Broadcom Corporation 
//	and/or its licensors, and may only be used, duplicated, modified 
//	or distributed pursuant to the terms and conditions of a separate, 
//	written license agreement executed between you and Broadcom (an 
//	"Authorized License").  Except as set forth in an Authorized 
//	License, Broadcom grants no license (express or implied), right 
//	to use, or waiver of any kind with respect to the Software, and 
//	Broadcom expressly reserves all rights in and to the Software and 
//	all intellectual property rights therein.  IF YOU HAVE NO 
//	AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE 
//	IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE 
//	ALL USE OF THE SOFTWARE.  
//	
//	Except as expressly set forth in the Authorized License,
//	
//	1.	This program, including its structure, sequence and 
//		organization, constitutes the valuable trade secrets 
//		of Broadcom, and you shall use all reasonable efforts 
//		to protect the confidentiality thereof, and to use 
//		this information only in connection with your use 
//		of Broadcom integrated circuit products.
//	
//	2.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE 
//		IS PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM 
//		MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, 
//		EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, 
//		WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY 
//		DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, 
//		MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A 
//		PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
//		COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR 
//		CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE 
//		RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.  
//
//	3.	TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT 
//		SHALL BROADCOM OR ITS LICENSORS BE LIABLE FOR 
//		(i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
//		EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
//		WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE 
//		SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE 
//		POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN 
//		EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE 
//		ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
//		LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE 
//		OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//***************************************************************************
/**
*
*  @file  uart_drv.h
*
*  @brief UART driver interface
*
*  @note
*****************************************************************************/


#ifndef _UART_DRV_H_
#define _UART_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup CSL_Interface 
 * @{
 */
 
#define	CNTRL_S				0x13
#define	CNTRL_Q				0x11

#define MAX_PORTS   3          

#define UART_RXBUF_SIZE     4096
#define UART_TXBUF_SIZE     0

#define HISRSTACKSIZE_UART_RX        (640 + 256)
#define HISRSTACKSIZE_UART_TX        (640 + 256)
#define HISRSTACKSIZE_UART_MODEM     (512 + 256)
#define HISRSTACKSIZE_UART_EVENT     (256 + 256)

#define UART_MCR_SIRE                             0x0040                    /* RW Read or Write., SIR Mode Enable. */
#define UART_MCR_AFCE                             0x0020                    /* RW Read or Write., Auto Flow Contro */
#define UART_MCR_LB                               0x0010                    /* RW Read or Write., LoopBack Bit. Th */
#define UART_MCR_OUT2                             0x0008                    /* RW Read or Write., OUT2 controls th */
#define UART_MCR_OUT1                             0x0004                    /* RW Read or Write., OUT1 controls th */
#define UART_MCR_RTS                              0x0002                    /* RW Read or Write., Request to Send. */
#define UART_MCR_DTR                              0x0001

/**
*
*  @brief  Call back function define for UART
*
*  @param  event (in) Bit fields of modem or line status. 0 for tx & rx cases
*
*  @param  data (in) Pointer to the data in rx call back. NULL for other cases
*
*  @param  length (in) Length in byte of rx data. 0 for other cases.
*
*  @return Number of bytes consumed by called function in rx. Ignored in others
*
*  @note 
*****************************************************************************/
typedef  UInt32 (*UARTDRV_Callback_t)(UInt8 event, UInt8* data, UInt32 length);

/**
* UART Port
*****************************************************************************/
typedef enum 
{ 
    PortA = 0, 
    PortB = 1,
    PortC = 2,
    InvalidPort = 0xFF
} PortId_t;

/**
* UART Baudrate
*****************************************************************************/
typedef enum 
{
    _2dot4,     ///< 2400     0
    _4dot8,     ///< 4800     1
    _9dot6,     ///< 9600     2   
    _19dot2,    ///< 19200    3
    _38dot4,    ///< 38400    4
    _57dot6,    ///< 57600    5
    _115dot2,   ///< 115200   6
    _128dot0,   ///< 128000   7
    _230dot4,   ///< 230400   8
    _460dot8,   ///< 460800   9
    _921dot6,   ///< 921600   A
    _1228dot8,  ///< 1228800  B
    _1843dot2,  ///< 1843200  C
    _3686dot4,  ///< 3686400  D
    _2000dot0,  ///< 2000000  E
    _3000dot0,  ///< 3000000  F
    _1500dot0,  ///< 1500000  G
    _1329dot5,
    _1413dot0,
    _max_BaudRate_t
} BaudRate_t;

/**
* UART Parity
*****************************************************************************/
typedef enum 
{ 
   Odd   = 0x08,        ///< Odd Parity
   Even  = 0x18,        ///< Event Parity 
   None  = 0x00         ///< No Parity
} Parity_t;

/**
* UART port line config
*****************************************************************************/
typedef enum 
{
   _5_Bit_WORD_1    = 0x00, ///< 5 Bit UInt16 - 1 Stop Bit
   _6_Bit_WORD_1    = 0x01, ///< 6 Bit UInt16 - 1 Stop Bit
   _7_Bit_WORD_1    = 0x02, ///< 7 Bit UInt16 - 1 Stop Bit
   _8_Bit_WORD_1    = 0x03, ///< 8 Bit UInt16 - 1 Stop Bit
   _5_Bit_WORD_1p5  = 0x04, ///< 5 Bit UInt16 - 1.5 Stop Bit
   _6_Bit_WORD_2    = 0x05, ///< 6 Bit UInt16 - 2 Stop Bits
   _7_Bit_WORD_2    = 0x06, ///< 6 Bit UInt16 - 1 Stop Bit
   _8_Bit_WORD_2    = 0x07  ///< 8 Bit UInt16 - 2 Stop Bits
} UART_LineConfig_t;


/**
* UART flow control type
*****************************************************************************/
typedef enum
{
    FLOW_CNTRL_TYPE_NONE,         ///< no Flow Control
    FLOW_CNTRL_TYPE_HW,           ///< RTS/CTS
    FLOW_CNTRL_TYPE_SW            ///< Xon/Xoff
} UART_Flow_Cntrl_Type_t;

/**
* UART flow control priority
*****************************************************************************/
typedef enum
{
    FLOW_CNTRL_PRIORITY_NORMAL,   ///< normal priority for sending flow control
    FLOW_CNTRL_PRIORITY_IMMEDIATE ///< immediate priority
} UART_Flow_Cntrl_Priority_t;

/**
* UART interrupt bit fields
*****************************************************************************/
typedef enum
{
    UART_RX_INTERRUPT =    0X01,   ///< Receive interrupt enable   
    UART_TX_INTERRUPT =    0X82,   ///< Transmit interrupt enable
    UART_LINE_INTERRUPT =  0X04,   ///< Line status interrupt enable
    UART_MODEM_INTERRUPT = 0X08    ///< Modem status interrupt enable
}  UART_Intc_Ctrl_t;

/**
* UART line status
*****************************************************************************/
typedef enum
{
    // DONOT CHANGE THESE VALUES, the values matches H/W Register
    UART_STATUS_OK                  = 0x0000,   ///< No Status
    UART_STATUS_RX_EMPTY            = 0x0001,   ///< Rx Empty
    UART_STATUS_OVERRUN_ERROR       = 0x0002,   ///< Overrun Error
    UART_STATUS_PARITY_ERROR        = 0x0004,   ///< Parity Error
    UART_STATUS_FRAME_ERROR         = 0x0008,   ///< Frameing Error
    UART_STATUS_BREAK               = 0x0010    ///< Break Received
} UART_Status_t;


/**
* UART call back function
*****************************************************************************/
typedef enum
{
    UART_RX_CALL_BACK = 0,
    UART_TX_CALL_BACK,
    UART_MODEM_CALL_BACK,
    UART_EVENT_CALL_BACK
} UART_Callback_Handle_Name_t;

/**
* UART Rx fifo interrupt triggering level
*****************************************************************************/
typedef enum
{
    UART_RX_FIFIO_ONE_CHAR       =    0X00<<6,   ///< 1 char in FIFO
    UART_RX_FIFO_QUARTER_FULL    =    0X01<<6,   ///< 1/4 full in FIFO
    UART_RX_FIFO_HALF_FULL       =    0X02<<6,   ///< 1/2 full in FIFO
    UART_RX_FIFO_FULL_MINUS_TWO  =    0X03<<6,   ///< (full - 2) chars in FIFO
}  UART_Rx_Fifo_Trigger_Level_t;

/**
* UART Tx fifo interrupt triggering level
*****************************************************************************/
typedef enum
{
    UART_TX_FIFO_EMPTY           =   0X00<<4,   ///< empty FIFO
    UART_TX_FIFO_TWO_CHAR        =   0X01<<4,   ///< 2 chars in FIFO
    UART_TX_FIFO_QUARTER_EMPTY   =   0X02<<4,   ///< 1/4 full in FIFO
    UART_TX_FIFO_HALF_EMPTY      =   0X03<<4,   ///< 1/2 full in FIFO
} UART_Tx_Fifo_Trigger_Level_t;


/**
* UART event bit fields for modem and line status
*****************************************************************************/

typedef union
{
    struct
    {
        UInt8 reserved0  :2;
        UInt8 dsr        :1; ///<data set ready
        UInt8 cts        :1; ///< clear to send
        UInt8 reserved1  :2; 
        UInt8 ddsr       :1; ///<delta data set ready
        UInt8 dcts       :1; ///<delta clear to send
    }modemEvent;
    
    struct
    {
        UInt8 fifoErr    :1;
        UInt8 txEmpty    :1;
        UInt8 thre       :1;
        UInt8 breakIntr  :1;
        UInt8 frameErr   :1;
        UInt8 parityErr  :1;
        UInt8 overrunErr :1;
        UInt8 dataReady  :1;              
    }lineEvent;
    
    UInt8 event;
}UART_Event_t;


/**
* UART event bit fields for modem and line status
*****************************************************************************/
typedef struct
{
    Boolean            polled;         ///< TRUE if polled I/O
    BaudRate_t         baudRate;       ///< Baud Rate
    UART_LineConfig_t  lineConfig;     ///< Line Configuration
    Parity_t           parity;          ///< Parity Configuration
    Boolean            autoFlowCtrl;
    Boolean            loopback;

    ///<The RX/TX buffer and thresholds
    UInt16          rxFifoTriggerLevel;  ///<see UART_Rx_Fifo_Trigger_Level_t
    UInt16          rxBufferSize; ///<see default size above UART_RXBUF_SIZE

    UInt16          txFifoTriggerLevel;  ///<see UART_Tx_Fifo_Trigger_Level_t
    UInt16          txBufferSize; ///<not currently used
  
} UART_Config_t;

/**
* UART data control fields
*****************************************************************************/
typedef struct {
  
  UInt32 fBinary :1;
  UInt32 fParity :1;
  UInt32 fOutxCtsFlow :1;
  UInt32 fOutxDsrFlow :1;
  UInt32 fDtrControl :2;
  UInt32 fDsrSensitivity :1;
  UInt32 fTXContinueOnXoff :1;
  UInt32 fOutX :1;
  UInt32 fInX :1;
  UInt32 fErrorChar :1;
  UInt32 fNull :1; 
  UInt32 fRtsControl :2; 
  UInt32 fAbortOnError :1;
  UInt32 fDummy2 :17;
  UInt16 XonLim; 
  UInt16 XoffLim;  
  Int8 XonChar; 
  Int8 XoffChar; 
  Int8 ErrorChar;
  Int8 EofChar; 
  Int8 EvtChar;
  Int8 bReserved1; 
  UInt16 wReserved1; 
} UART_Data_Control; 

/**
*
*  @brief  UART configure baudrate
*
*  @param  portId (in) UART port ID
*
*  @param  BaudRate (in) UART baudrate to config to.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Config_Baudrate(
    PortId_t        portId,
    BaudRate_t      BaudRate
    );

/**
*
*  @brief  UART configure port
*
*  @param  portId (in) UART port ID
*
*  @param  BaudRate (in) UART baudrate to config to.
*
*  @param  LineConfig (in) UART line config to config to.
*
*  @param  Parity (in) UART parity to config to.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Config_Port(
    PortId_t            portId,         // Port Identifier
    BaudRate_t          BaudRate,       // Baud Rate
    UART_LineConfig_t   LineConfig,     // Line Configuration
    Parity_t            Parity  
    );

/**
*
*  @brief  UART configure device
*
*  @param  portId (in) UART port ID
*
*  @param  pConfig (in) UART device config struct.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Config_Device(
    PortId_t        portId,         // Port Identifier
    UART_Config_t   *pConfig    // Baud Rate
    );
    
/**
*
*  @brief  UART get device configuration
*
*  @param  portId (in) UART port ID
*
*  @param  pConfig (out) UART device config struct.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Get_Device_Config(
    PortId_t        portId,         // Port Identifier
    UART_Config_t   *pConfig    // Baud Rate
    );

/**
*
*  @brief  UART configure data control
*
*  @param  portId (in) UART port ID
*
*  @param  pConfig (in) UART data control struct.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Set_Control_Data(
    PortId_t             portId,         // Port Identifier
    UART_Data_Control   *pDataCtrl    // Baud Rate
    );

/**
*
*  @brief  De-initilize an UART port
*
*  @param  portId (in) UART port ID
*
*  @return none
*
*  @note 
*****************************************************************************/
void UARTDRV_Deinit(PortId_t portId);

/**
*
*  @brief  UART initialize an UART port
*
*  @param  portId (in) UART port ID
*
*  @param  polled (in) TRUE: polling mode, FALSE: interrupt mode
*
*  @param  BaudRate (in) UART baudrate to config to.
*
*  @param  LineConfig (in) UART line config to config to.
*
*  @param  Parity (in) UART parity to config to.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
Boolean UARTDRV_Init(
    PortId_t        portId,         // Port Identifier
    Boolean         polled,         // TRUE if polled I/O
    BaudRate_t      BaudRate,       // Baud Rate
    UART_LineConfig_t    LineConfig,     // Line Configuration
    Parity_t        Parity          // Parity Configuration
    );

/**
*
*  @brief  Config UART call back handles
*
*  @param  portId (in) UART port ID
*
*  @param  name (in) Call back handle name.
*
*  @param  handler (in) Call back handler(function).
*
*  @param  eventEnabling (in) Events that trigger call back for modem and
*                             line status.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Install_Callback_Handle(
    PortId_t portId, 
    UART_Callback_Handle_Name_t name, 
    UARTDRV_Callback_t handler,
    UART_Event_t eventEnabling
    );

/**
*
*  @brief  Config UART call back handles in LISR mode
*
*  @param  portId (in) UART port ID
*
*  @param  name (in) Call back handle name.
*
*  @param  handler (in) Call back handler(function).
*
*  @param  eventEnabling (in) Events that trigger call back for modem and
*                             line status.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt32 UARTDRV_Install_LISR_Callback_Handle(
    PortId_t portId, 
    UART_Callback_Handle_Name_t name, 
    UARTDRV_Callback_t handler,
    UART_Event_t eventEnabling
    );

/**
*
*  @brief  Send a buffer through an UART port
*
*  @param  portId (in) UART port ID
*
*  @param  buffer (in) buffer of data to send.
*
*  @param  size (in) size of buffer to send.
*
*  @return Number of bytes sent.
*
*  @note 
*****************************************************************************/
UInt16 UARTDRV_Send_Buffer(
	PortId_t portId,
	UInt8   *buffer,
	UInt32  size
	);

/**
*
*  @brief  Send a char through an UART port
*
*  @param  portId (in) UART port ID
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt16 UARTDRV_Send_Char(PortId_t portId, char ch);

/**
*
*  @brief  Send a null terminated string through an UART port
*
*  @param  portId (in) UART port ID
*
*  @param  ch (in) pointer of string to send.
*
*  @return None.
*
*  @note 
*****************************************************************************/
void UARTDRV_Send_String(PortId_t portId, char *ch);

/**
*
*  @brief  Read a char from an UART port
*
*  @param  portId (in) UART port ID
*
*  @param  ch (out) pointer of char.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt16 UARTDRV_Read_Char(PortId_t portId, char *ch);

/**
*
*  @brief  read a buffer an UART port to a maximum size
*
*  @param  portId (in) UART port ID
*
*  @param  buffer (out) buffer of data to send.
*
*  @param  buffer_size (in) maximum size to read.
*
*  @return Number of bytes received.
*
*  @note 
*****************************************************************************/
UInt16 UARTDRV_Read_Buffer(
	PortId_t portId,
	UInt8   *buffer,
	UInt16  buffer_size
	);

/**
*
*  @brief  get available byte count in rx buffer
*
*  @param  portId (in) UART port ID
*
*
*  @return Number of bytes in rx buffer.
*
*  @note 
*****************************************************************************/
UInt16 UARTDRV_Get_Rx_Available_Data_Size(
	PortId_t portId
	);

/**
*
*  @brief  get available byte count in rx buffer
*
*  @param  portId (in) UART port ID
*
*
*  @return Saved last modem event.
*
*  @note 
*****************************************************************************/	
UInt8 UARTDRV_Get_Modem_Event(
    PortId_t        portId         // Port Identifier
    );
    	
/**
*
*  @brief    config uart request to send.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note
*
*****************************************************************************/
UInt32 UARTDRV_Config_Rts(PortId_t portId, Boolean On );


/**
*
*  @brief      config uart request to send.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note
*
*****************************************************************************/
UInt32 UARTDRV_Config_Cts(PortId_t portId, Boolean On );


/**
*
*  @brief      config data carrier ready.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note  
*
*****************************************************************************/
UInt32 UARTDRV_Config_Dcd(PortId_t portId, Boolean On );


/**
*
*  @brief      config data terminal ready.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note  
*
*****************************************************************************/
UInt32 UARTDRV_Config_Dtr(PortId_t portId, Boolean On );


/**
*
*  @brief      config data terminal ready.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note  
*
*****************************************************************************/
UInt32 UARTDRV_Config_Dsr(PortId_t portId, Boolean On );


/**
*
*  @brief      config auto flow control.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note  
*
*****************************************************************************/
UInt32 UARTDRV_Config_Auto_Flow_Control(PortId_t portId, Boolean On );


/**
*
*  @brief      config uart interrupt.
*
* @param    portId (in), UART port ID.
*
* @param    On (in),
*
* @return   1-success, 0-fail.
*
* @note  
*
*****************************************************************************/
UInt32 UARTDRV_Config_Interrupt(PortId_t portId, UART_Intc_Ctrl_t intr, Boolean On);

/**
*
*  @brief      Enable/Diable UART idle power off..
*
* @param    portId (in), UART port ID.
*
* @param    On (in), 1-Enable idle power off, 0-Disable idle power off. 
*
* @return   
*
* @note  
*
*****************************************************************************/
void UARTDRV_Set_Idle_Power_Off_Mode(PortId_t portId, Boolean On );

/**
*
*  @brief  Read a char from an UART port with polling
*
*  @param  portId (in) UART port ID
*
*  @param  ch (out) pointer of char.
*
*  @return 1: success, 0: fail
*
*  @note 
*****************************************************************************/
UInt16 UARTDRV_Poll_Char(PortId_t portId, char *ch);

/**
*
*  @brief      Set UART idle power off flag..
*
* @param    portId (in), UART port ID.
*
* @param    On (in), 1-set idle power off, 0-Disable idle power off. 
*
* @return   
*
* @note  
*
*****************************************************************************/
void UARTDRV_Set_Idle_Power_Off_Flag(PortId_t portId, Boolean On );

/**
*
*  @brief      get UART idle power off flag setting..
*
* @param    portId (in), UART port ID.
*
* @param    On (in), 1-set idle power off.. 
*
* @return   Idle power off flag setting
*
* @note  
*
*****************************************************************************/
Boolean UARTDRV_Get_Idle_Power_Off_Flag(PortId_t portId);

/**
*
*  @brief      get UART idle power off mode setting..
*
* @param    portId (in), UART port ID. 
*
* @return   Idle power off mode setting
*
* @note  
*
*****************************************************************************/
UInt32 UARTDRV_Get_Idle_Power_Off_Mode(PortId_t portId);

/** @} */

#ifdef __cplusplus
}
#endif

#endif

/*============= End of File: uart_drv.h ==================*/


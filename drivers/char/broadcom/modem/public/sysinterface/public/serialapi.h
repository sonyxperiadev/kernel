/****************************************************************************
*
*     Copyright (c) 2007 Broadcom Corporation
*           All Rights Reserved
*
*     No portions of this material may be reproduced in any form without the
*     written permission of:
*
*           Broadcom Corporation
*           5300 California Avenue
*           Irvine, California 92617
*
*     All information contained in this document is Broadcom Corporation
*     company private, proprietary, and trade secret.
*
****************************************************************************/
//
// Description:  APIs for serial device operations.
//
// serialapi.h
//
//
// Created by:	Jack Yu 05/07/2003
//
// Note: 
//********************************************************************
//!
//! \file  serialapi.h
//! \brief Serial port API prototypes and defines.
//!
//! @defgroup SerialApi Serial API
//!	@ingroup SerialInterface
//! Encapsulates interface for all serial devices (eg UART, Bluetooth, IRDA).
//! \section Usage
//!         SERIAL_Init() expected to be called once at system initialization to
//!         setup entry points for all serial devices (eg UART, Bluetooth, IRDA).
//!
//!         Serial_OpenDevice() can then be called to open a specific serial device
//!         followed by Serial_Read() ..  Serial_Write()..  etc.
//!
//!         Serial_CloseDevice() should be called when access to the device is no
//!         longer required.
//! @{
//!

/* Requires the following header files before its inclusion in a c file
#include "mobcom_types.h"
#include "consts.h"
#include "resultcode.h"
#include "sio.h"
*/

#ifndef __SERIALAPI_H__
#define __SERIALAPI_H__

#include <string.h>

#ifdef USB_INCLUDED
//! The USB serial interface configure type 
typedef struct
{
	Boolean		configForCalMode;
	//placehold for serial interface setting
	UInt8		baudRate;       // Baud Rate
	UInt8		lineConfig;     // Line Configuration
	UInt8       parity;         // Parity Configuration

} USBSerialConfig_t;
#endif

//#include "usb.h"
#define PORT_CONFIG_ERROR	0x9999

//! Serial device configuration definition.
//! Specifies parameters specific to each individual serial device.
typedef union  {

	UART_Config_t  uartCfg;		//!< Current UART configuration

#ifdef INCL_IRDA
	IRDAConfig_t  irdaCfg;		//!< Current IrDA configuration
#endif

#if defined( USB_INCLUDED ) && defined(USB_CAL_MODE_SUPPORT)
	USBSerialConfig_t   usbCfg;
#endif

	//...

}DeviceCfg_t;

//! Valid serial devices  
typedef  enum {

	SERIAL_DEVICE_UART_A = 0,		//!< SERIAL_DEVICE_UART_A must be zero.
	SERIAL_DEVICE_UART_B,			//!< UART B
	SERIAL_DEVICE_UART_C,			//!< UART C
	SERIAL_DEVICE_IRDA_SERIAL,		//!< IRDA
	SERIAL_DEVICE_USB,				//!< USB1
	SERIAL_DEVICE_USB2, 			//!< USB2, the usb device for the logging.
	SERIAL_DEVICE_USB3, 			//!< USB3, the usb device for the Encap command.
	SERIAL_DEVICE_BLUE_TOOTH,		//!< Bluetooth
#if defined (STM_INCLUDED)
	SERIAL_DEVICE_STM,				// STM logging device
#endif
    SERIAL_DEVICE_RNDIS,            //!< RNDIS
	//....
	SERIAL_DEVICE_NULL_DEV,			//!< Null serial device
	SERIAL_DEVICE_MAX_DEVICE_NUM
} SerialDeviceID_t;

/// Serial device event list
typedef enum{

	//UART event ....
	UART_STATUS_EVENT,			//UART status event (SIOEvent_t) 
	UART_RXDATA_EVENT,			//Rx data available event 
	UART_ESCDATAMODE_EVENT,		//+++ detection event
	UART_BREAK_EVENT,			//break detetcion

	//USB events
	USB_RXDATA_EVENT,			//USB Rx data available event 
	USB_DTR_CHANGE_HIGH,		//USB DTR to high
	USB_DTR_CHANGE_LOW,			//USB DTR to low
	USB_ESCAPE_DATACONNECT,		//USB +++ escape

	//IRDA Ircomm events
#ifdef INCL_IRDA
	IRDA_RXDATA_EVENT,
	IRDA_STATUS_EVENT,
	IRDA_RESENDTX_EVENT,
#endif

	//Bluetooth event	
	BLUETOOTH_RXDATA_EVENT,

//Other devices' events

	//End of events
	SERIAL_EVENT_END
} SerialEvent_t;

typedef struct
{
	Boolean			BufferA_Filled;
	Boolean			BufferB_Filled;
	UInt8			BufferA[4096];
	UInt8			BufferB[4096];
} DualBuffer_t;


//! Asynchronous Event handler prototype used in notifyDrv
typedef  void    (*SerialEventHandler_t)(UInt8 event,UInt8* eventData);
//!  Driver entry point:  openDrv.
typedef  Boolean (*SerialDeviceOpenDrv_t)(DeviceCfg_t* deviceCfg);
//!  Driver entry point:  closeDrv.
typedef  Boolean (*SerialDeviceCloseDrv_t)();
//!  Driver entry point:  notifyDrv.  Asynch Event Notification
typedef  Boolean (*SerialDeviceEventNotifyDrv_t)(SerialEventHandler_t handler);
//!  Driver entry point:  enableRxDrv.  Enable/Disable flow Rx control.
typedef  Boolean (*SerialDeviceEnableRx_t)(Boolean enable);
//!  Driver entry point:  freeTxSpaceDrv.  Determine free space in Tx buffer.
typedef  UInt32	 (*SerialDeviceFreeTxSpace_t)();
//!  Driver entry point:  readDrv.  
typedef  UInt16  (*SerialDeviceReadDrv_t)( UInt8* dataPtr, UInt16 maxReadSize);
//!  Driver entry point:  writeDrv.  
typedef  UInt16  (*SerialDeviceWriteDrv_t)( UInt8* dataPtr, UInt16 dataSize  );
//!  Driver entry point:  checkconfig.
typedef  Boolean (*SerialDeviceCheckConfig_t)(DeviceCfg_t* deviceCfg);
//! Serial port handle.
typedef  int    SerialHandle_t;

//!  Driver entry point:  readDrvDMA. 
typedef  UInt16  (*SerialDeviceReadDrvDMA_t)(DualBuffer_t* DualBufferPtr, UInt16 maxReadSize);
//!  Driver entry point:  writeDrvDMA.  
typedef  UInt16  (*SerialDeviceWriteDrvDMA_t)(DualBuffer_t* DualBufferPtr, UInt16 dataSize  );

//! Function Name: Serial_ConfigureDevice
//!
//! Description:  Setup the driver entry points for serial deviceID.
//!
//! \param[in]  device          One of the valid serial devices as given in SerialDeviceID_t.
//!	\param[in]	openDrv         device open routine
//!	\param[in]	closeDrv  	    device close routine
//!	\param[in]	readDrv   		device read routine
//!	\param[in]	writeDrv		device write routine
//!	\param[in]	enableRxDrv     routine to enable and disable flow control
//!	\param[in]	freeTxSpaceDrv  routine to return number of bytes free in Tx buffer
//!	\param[in]	notifyDrv	    asynchronous callback routine
//!	\param[in]	checkconfig	    Configuration check routine
//!	\param[in]	defaultCfg      pointer to DeviceCfg_t: default device configuration
//!	\param[in]	deviceReadDMA   DMA device read routine
//!	\param[in]	deviceWriteDMA  DMA device write routine
//!
//! \return int  deviceID value passed in, unchanged.
//!
//! \note  Called only by Serial_Init() to set up entry points.
//!
//! \sa  Serial_Init()
//!
int Serial_ConfigureDevice( SerialDeviceID_t				device,      
							SerialDeviceOpenDrv_t			openDrv,      
							SerialDeviceCloseDrv_t			closeDrv, 
							SerialDeviceReadDrv_t			readDrv,   
							SerialDeviceWriteDrv_t			writeDrv, 
							SerialDeviceEnableRx_t			enableRxDrv,
							SerialDeviceFreeTxSpace_t		freeTxSpaceDrv,
							SerialDeviceEventNotifyDrv_t	notifyDrv,
							SerialDeviceCheckConfig_t		checkconfig,
							DeviceCfg_t*					defaultCfg,
						    SerialDeviceReadDrvDMA_t		deviceReadDMA,
							SerialDeviceWriteDrvDMA_t		deviceWriteDMA						
							);

//!
//!
//! Function Name: Serial_OpenDevice
//!
//! Description:  Standard device driver open() entry point.  Opens the serial device with
//!               parameters specified in deviceCfg.
//!
//! \param[in]  serialDev      One of the valid serial devices as given in SerialDeviceID_t.
//!	\param[in]	deviceCfg      pointer to DeviceCfg_t, device configuration to be used to
//!                            configure the serial device.
//!
//! \return SerialHandle_t   Serial device handle or -1 on error or PORT_CONFIG_ERROR
//!
//! \note
//!
//! \sa  Serial_CloseDevice
//!
//!
SerialHandle_t	Serial_OpenDevice(SerialDeviceID_t serialDev,DeviceCfg_t* deviceCfg);

//!
//!
//! Function Name: Serial_CloseDevice
//!
//! Description:  Standard device driver open() entry point.  Closes the serial device
//!               represented by serialHandle.
//!
//! \param[in]  serialHandle  Serial device handle returned by Serial_OpenDevice().
//!
//! \return Result_t  RESULT_OK on success or RESULT_ERROR
//!
//! \note
//!
//! \sa  Serial_OpenDevice
//!
//!
Result_t		Serial_CloseDevice(SerialHandle_t serialHandle);

//!
//!
//! Function Name: Serial_RegisterEventCallBack
//!
//! Description:  Register an asynchronous callback function with the driver.
//!               Function is used to report device status changes.
//!
//! \param[in] serialHandler  serialHandle returned by Serial_OpenDevice()
//! \param[in] eventCallback  callback routine to register
//!
//! \return Result_t  RESULT_OK on success or RESULT_ERROR
//!
//! \note
//!
//!
Result_t		Serial_RegisterEventCallBack(SerialHandle_t  serialHandler,
											SerialEventHandler_t  eventCallback);

//!
//!
//! Function Name: Serial_Read
//!
//! Description:   Standard device driver read() entry point.  Reads and returns
//!                data from the device.
//!
//! \param[in]     deviceHandle  serialHandle returned by Serial_OpenDevice().
//! \param[in,out] *dataPtr		 Pointer to buffer to receive data.
//! \param[in]     maxReadSize   Maximum number of bytes to read from the serial device.
//!
//! \return  Number of bytes read on success or 0.
//!
//! \note
//!
//! \sa  Serial_OpenDevice, Serial_Write
//!
//!
UInt16 Serial_Read(	SerialHandle_t deviceHandle,  UInt8* dataPtr,UInt16 maxReadSize);

//!
//!
//! Function Name: Serial_Write
//!
//! Description:   Standard device driver write() entry point.  Writes
//!                dataSize bytes from dataPtr to the device.
//!
//! \param[in] deviceHandler serialHandle returned by Serial_OpenDevice().
//! \param[in] *dataPtr		 Pointer to buffer containing data to write to device.
//! \param[in] dataSize      Number of bytes to write
//!
//! \return  Number of bytes written on success or 0.
//!
//! \note
//!
//! \sa  Serial_OpenDevice, Serial_Read
//!
//!
UInt16 Serial_Write(SerialHandle_t deviceHandler, UInt8* dataPtr, UInt16 dataSize);


//!
//!
//! Function Name: Serial_Read_DMA
//!
//! Description:   Standard device driver read() entry point.  Reads and returns
//!                data from the device.
//!
//! \param[in]     deviceHandle		serialHandle returned by Serial_OpenDevice().
//! \param[in,out] *DualBufferPtr	Pointer to buffer to receive data.
//! \param[in]     maxReadSize		Maximum number of bytes to read from the serial device.
//!
//! \return  Number of bytes read on success or 0.
//!
//! \note
//!
//! \sa  Serial_OpenDevice, Serial_Write
//!
//!
UInt16 Serial_Read_DMA(SerialHandle_t deviceHandle,  DualBuffer_t* DualBufferPtr,UInt16 maxReadSize);

//!
//!
//! Function Name: Serial_Write_DMA
//!
//! Description:   Device driver to write() SERIAL_DEVICE_UART_A or SERIAL_DEVICE_UART_C
//!	               in DMA mode.  Writes dataSize bytes from DualBufferPtr to the device.
//!
//! \param[in] deviceHandler  serialHandle returned by Serial_OpenDevice().
//! \param[in] *DualBufferPtr Pointer to the dual buffers containing data to write to device.
//! \param[in] dataSize       Number of bytes to write
//!
//! \return  Number of bytes written on success or 0.
//!
//! \note
//!
//! \sa  Serial_OpenDevice, Serial_Read
//!
//!
UInt16 Serial_Write_DMA(SerialHandle_t deviceHandler, DualBuffer_t* DualBufferPtr, UInt16 dataSize);

//!
//!
//! Function Name: Serial_EnableRxDataFlow
//!
//! Description:  Enable or disable Rx line flow control.
//!
//! \param[in] serialHandle  serialHandle returned by Serial_OpenDevice().
//! \param[in] enableRx 	 TRUE to enable Rx flow control
//!                          FALSE to disable
//!
//! \return  TRUE on success else FALSE
//!
//! \note
//!
//!
Boolean	Serial_EnableRxDataFlow(SerialHandle_t serialHandle,Boolean enableRx);

//!
//!
//! Function Name: Serial_FreeTxDataSpace
//!
//! Description:  Get and return the number of bytes of free space
//!               in the transmit buffer.
//!
//! \param[in] serialHandle  serialHandle returned by Serial_OpenDevice().
//!
//! \return  On success returns the number of free bytes in the transmit buffer.
//!          On failure returns FALSE.
//!
//! \note
//!
//!
UInt32	Serial_FreeTxDataSpace(SerialHandle_t serialHandle);

//!
//!
//! Function Name: Serial_GetDefaultCfg
//!
//! Description:  Get and return the current device configuration for deviceID.
//!
//! \param[in]  deviceID  One of the valid serial devices as given in SerialDeviceID_t.
//!
//! \return  DeviceCfg_t*  Pointer to the current valid device configuration or NULL
//!
//! \note
//!
//!
DeviceCfg_t*	Serial_GetDefaultCfg(SerialDeviceID_t deviceID);

//!
//!
//! Function Name: Serial_GetDeviceID
//!
//! Description:  Get and return a serial port device Id from a port handle.
//!
//! \param[in] handle  serialHandle returned by Serial_OpenDevice().
//!
//! \return  SerialDeviceID_t  Valid device Id or SERIAL_DEVICE_NULL_DEV.
//!
//! \note
//!
//!
SerialDeviceID_t Serial_GetDeviceID(SerialHandle_t handle);

//!
//!
//! Function Name: SERIAL_Init
//!
//! Description:  Initialize the Serial IO Devices.
//!
//! \return  void
//!
//! \note  Usage:  Function expected to be called once at system initialization to
//! \note  setup function pointers for all serial devices (eg UART, Bluetooth, IRDA).
//! \note  It does not open/close/initialize any serial device hardware. It is OK to configure
//! \note  these function pointers without existing the actual devices.
//! \note  Serial_OpenDevice() can then be called to open a specific serial device followed by
//! \note
//! \note  Serial_Read() ..  Serial_Write()..  etc.
//! \note
//! \note  Serial_CloseDevice() should be called when access to the device is no longer required.
//! \note
//!
void SERIAL_Init(void);



//!
//!
//! Function Name: Serial_GetDeviceBaud
//!
//! Description:  Returns the baud rate of a serial device. For non-UART serial device, 
//!				  default 921K baud is returned. 
//!
//! \param[in]  deviceID       One of the valid serial devices as given in SerialDeviceID_t.
//!
//! \return BaudRate_t   baud rate.
//!
//! \note		This function should be called only after Serial_OpenDevice() is called. 
//!
//!
BaudRate_t Serial_GetDeviceBaud(SerialDeviceID_t deviceID);

/** @} */

//Boolean UART_B_EventNotifyEx(PortId_t portId, UartEventHandler_t handler, UInt32 event, UInt32 trigger_level, UInt8* pRxData);
Boolean UART_B_UnregisterEventEx(PortId_t portId, UInt32 event);




// The UARTA services for the serial API.
extern Boolean UART_A_OpenDevice(DeviceCfg_t* deviceCfg);
extern Boolean UART_A_CheckConfig(DeviceCfg_t* deviceCfg);
extern Boolean UART_A_CloseDevice(void);
extern Boolean UART_A_EventNotify(SerialEventHandler_t handler);
extern UInt16  UART_A_Read(UInt8* dataPtr, UInt16 maxReadSize);
extern UInt16  UART_A_Write(UInt8* dataPtr, UInt16 dataSize);
extern Boolean UART_A_EnableRxDataFlow(Boolean enableRx);
extern UInt32  UART_A_FreeTxSpace(void);

// The UARTA services for the serial API.
extern Boolean UART_B_OpenDevice(DeviceCfg_t* deviceCfg);
extern Boolean UART_B_CheckConfig(DeviceCfg_t* deviceCfg);
extern Boolean UART_B_CloseDevice(void);
extern Boolean UART_B_EventNotify(SerialEventHandler_t handler);
extern UInt16  UART_B_Read(UInt8* dataPtr, UInt16 maxReadSize);
extern UInt16  UART_B_Write(UInt8* dataPtr, UInt16 dataSize);
extern Boolean UART_B_EnableRxDataFlow(Boolean enableRx);
extern UInt32  UART_B_FreeTxSpace(void);

extern Boolean UART_C_OpenDevice(DeviceCfg_t* deviceCfg);
extern Boolean UART_C_CheckConfig(DeviceCfg_t* deviceCfg);
extern Boolean UART_C_CloseDevice(void);
extern Boolean UART_C_EventNotify(SerialEventHandler_t handler);
extern UInt16  UART_C_Read(UInt8* dataPtr, UInt16 maxReadSize);
extern UInt16  UART_C_Write(UInt8* dataPtr, UInt16 dataSize);
extern Boolean UART_C_EnableRxDataFlow(Boolean enableRx);
extern UInt32  UART_C_FreeTxSpace(void);

extern UInt16  UART_A_Read_DMA(DualBuffer_t* DualBufferPtr, UInt16 dataSize);
extern UInt16  UART_A_Write_DMA(DualBuffer_t* DualBufferPtr, UInt16 dataSize);
extern UInt16  UART_B_Read_DMA(DualBuffer_t* DualBufferPtr, UInt16 dataSize);
extern UInt16  UART_B_Write_DMA(DualBuffer_t* DualBufferPtr, UInt16 dataSize);
extern UInt16  UART_C_Read_DMA(DualBuffer_t* DualBufferPtr, UInt16 dataSize);
extern UInt16  UART_C_Write_DMA(DualBuffer_t* DualBufferPtr, UInt16 dataSize);

// The IRDA services for the serial API.
//...

// The USB services for the serial API.
//...



#endif


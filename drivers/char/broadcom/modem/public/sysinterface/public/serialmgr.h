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
*   @file   serialmgr.h
*
*   @brief  This file defines api's for Serial Manager
*
****************************************************************************/


#ifndef __SER_MANAGER_H__
#define __SER_MANAGER_H__


/**
	Serial Device and Serial Manager Events
**/
typedef enum
{
	//refer to SerialEvent_t
	SM_EVENT_START = SERIAL_EVENT_END + 1,	///< Unused
	SM_EVENT_DEV_CLOSE,						///< Serial Device closed
	SM_EVENT_CLIENT_SUSPENDED,				///< Active client suspended by other client. ( payload will have other client ID )
	SM_EVENT_CLIENT_RESUMED,				///< Resumed implicitly since other client de-registered
	SM_EVENT_CLIENT_UNREGISTERED,			///< Active client is un-registered
	SM_EVENT_TOTAL							///< Total num clients
}SerialMgrEvent_t;

/**
	Serial Device Client Type
**/
typedef enum
{
	SERIAL_CLIENT_INVALID,		///< Unused
	SERIAL_CLIENT_ATC,			///< ATC Client
	SERIAL_CLIENT_DLINK,		///< Dlink for PPP Client
	SERIAL_CLIENT_CSD,			///< CSD Client
	SERIAL_CLIENT_CONSOLE,		///< Serial Console like Bash
	SERIAL_CLIENT_GPS_TEST,		///< GPS Testing
	SERIAL_CLIENT_CAPI2_TEST,	///< CAPI Testing
	SERIAL_CLIENT_MAX_NUM
}SerialClientId_t;

#define _DBG_(a) a	//by default logs are enabled

#ifdef WIN32
#include "stdio.h"

	#define SERIAL_TRACE	printf
	#define SERIAL_TRACE_DETAIL printf
#else
	#define SERIAL_TRACE(...) Log_DebugPrintf(LOGID_CAPI2_BASIC, __VA_ARGS__)
	#define SERIAL_TRACE_DETAIL(...) Log_DebugPrintf(LOGID_CAPI2_DETAIL, __VA_ARGS__)
#endif

/**
	Serial Manager Handle
**/
typedef void* SerialMgrHandle_t;

//***************************************************************************************
/**
	Callback Function to notify Rx Data from Serial device
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		userData (in) User Data passed during SERIALMGR_RegisterClient
	@param		buffer (in) Buffer Data
	@param		len		(in) size of the buffer

	@return		None
	@note
**/
typedef void (*SerialMgrRxHandler_t)(SerialMgrHandle_t handle, void* userData, UInt8* buffer,UInt32 len);

//***************************************************************************************
/**
	Callback Function to notify Serial device & Serial manager events
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		userData (in) User Data passed during SERIALMGR_RegisterClient
	@param		event (in) SerialMgrEvent_t
	@param		eventData	(in) Data associated with event

	@return		None
	@note
**/
typedef void (*SerialMgrEventHandler_t)(SerialMgrHandle_t handle, void* userData, SerialMgrEvent_t event, UInt8* eventData);

/**
	The Init params for registration
**/
typedef struct
{
   //Client specific params
   SerialMgrRxHandler_t rxCb;		///< Callback for Rx Data
   SerialMgrEventHandler_t evCb;	///< Callback for Device event
   Boolean		pullData;			///< TRUE means the Rx event is notified but not Rx Data. FALSE Rx data is pushed to client
   Boolean		escapeSequenc;		///< If set to TRUE, then RX Data is examined for +++ sequence.
   void*		userData;			///< User Data
   SerialClientId_t clientType;		///< Client Type
   //Device specific values
   Boolean isDefaultAT;				///< If TRUE then default AT device is used and devID is ignored
   SerialDeviceID_t devID;			///< Device ID. Valid only if isDefaultAT is FALSE above
   DeviceCfg_t* deviceCfg;			///< UART confir. If null the default config is used
}SerialMgrInitParams_t;


/**
	Device buffer
**/
typedef struct
{
	void* buffer;			///< Buffer allocated by device
	UInt16 allocatedSize;	///< The size allocated by device
	void* devContext;		///< Device context. Warning!!! do not change
	UInt16 actualSize;		///< Allocated buffer lem. Warning!!! do not change
}SerialDevBuffer_t;
//***************************************************************************************
/**
	Function to initialize the Serial Manager during system startup
	@param		None
	
    @return		TRUE if the default AT port can be open or else FALSE
	@note
		
**/
Boolean SERIALMGR_Init(void);

//***************************************************************************************
/**
	Function to register the client with a Serial device
	@param		initParams (in) The initialization parameters for the registration
	
	@return		Non-zero handle on success or else NULL
	@note
**/
SerialMgrHandle_t SERIALMGR_RegisterClient(SerialMgrInitParams_t* initParams);

//***************************************************************************************
/**
	Function to register the client with a Serial device by previpusly opended handle
	@param		handle (in) The Serial manager handle that is previously registered by another Client
	@param		initParams (in) Init params

	@return		Non-zero handle on success or else NULL
	@note
**/
SerialMgrHandle_t SERIALMGR_RegisterClientByHandle(SerialMgrHandle_t handle, 
												  SerialMgrInitParams_t *initParams);

//***************************************************************************************
/**
	Function to de-register the client with a Serial device
	@param		handle (in) The handle created during SerialMgrRegisterClient

	@return		TRUE if the handle is valid or else FALSE
	@note
**/
Boolean SERIALMGR_DeRegisterClient(SerialMgrHandle_t handle);

//***************************************************************************************
/**
	Function to allocate serial device buffer
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		size (in) The buffer size required
	@param		waitTime (in) Wait time in milli secs. 0xFFFFFFFF is wait forever
	@param		devBufHandle (out) Holds the output buffer pointer

	@return		TRUE if allocation pass or else FALSE.
	@note       
**/
Boolean SERIALMGR_AllocDevBuffer(SerialMgrHandle_t handle, UInt32 size, UInt32 waitTime, SerialDevBuffer_t* devBufHandle);


//***************************************************************************************
/**
	Function to send the serial device allocated buffer
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		devBufHandle (in) The buffer handle to be sent.
	@param		len (in) The buffer size

	@return		TRUE if send is successfull or else false.
	@note
				If the function succeed then the buffer is freed by the device.
**/
Boolean SERIALMGR_SendDevBuffer(SerialMgrHandle_t handle, SerialDevBuffer_t* devBufHandle);

//***************************************************************************************
/**
	Function to send the data to serial device
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		buffer (in) The buffer to be sent.
	@param		len (in) The buffer size

	@return		Returns number of bytes sent.
	@note
**/
UInt32 SERIALMGR_SendData(SerialMgrHandle_t handle, void* buffer, UInt32 len, Boolean block);

//***************************************************************************************
/**
	Function to recv the data from serial device
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		buffer (in) The receive buffer.
	@param		maxReadSize (in) Max buffer size

	@return		Returns number of bytes read.
	@note
**/
UInt32 SERIALMGR_ReadData(SerialMgrHandle_t handle, void* buffer, UInt32 maxReadSize);


//***************************************************************************************
/**
	Function to free Serial device buffer
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		buffer (in) The buffer to be freed which was allocated by SerialMgrAllocBuffer

	@return		TRUE if send is successfull or else false.
	@note
**/
Boolean SERIALMGR_FreeDevBuffer(SerialMgrHandle_t handle, SerialDevBuffer_t* devBufHandle);

//***************************************************************************************
/**
	Function to enable/disable flow control for the Rx of Serial device
	@param		handle (in) The handle created during SerialMgrRegisterClient
	@param		enable (in) TRUE to enable flow control and FALSE to disable flow control

	@return		TRUE if set is successfull or else false.
	@note
**/
Boolean SERIALMGR_SetFlowControl(SerialMgrHandle_t handle, Boolean enable);

//***************************************************************************************
/**
	Function to check the validity of Serial manager handle
	@param		handle (in) The handle created during SerialMgrRegisterClient

	@return		TRUE if handle is valid or else false.
	@note
**/
Boolean SERIALMGR_IsValidHandle(SerialMgrHandle_t handle);


//***************************************************************************************
/**
	Function to return Serial Device ID
	@param		handle: Serial Mgr handle
	
    @return		SerialDeviceID_t
	@note
		
**/
SerialDeviceID_t SERIALMGR_GetDeviceID(SerialMgrHandle_t handle);

//***************************************************************************************
/**
	Function to get the current AT Device
	@param		None
	
    @return		SerialDeviceID_t	
	@note
		
**/
SerialDeviceID_t SERIALMGR_GetCurrentATDeviceID(void);

//***************************************************************************************
/**
	Function returns default AT device set in the sysparm DB
	@param		None
	
    @return		SerialDeviceID_t
	@note
		
**/
SerialDeviceID_t SERIALMGR_GetDefaultATDevice(void);

SerialHandle_t SERIALMGR_GetSerialDeviceHandle(SerialMgrHandle_t handle);

UInt32 SERIALMGR_FreeTxDataSpace(SerialMgrHandle_t handle);

SerialHandle_t SERIALMGR_GetCurrentATDeviceHandle(void);

SerialMgrHandle_t SERIALMGR_GetCurrentATSerialMgrHandle(void);
#endif

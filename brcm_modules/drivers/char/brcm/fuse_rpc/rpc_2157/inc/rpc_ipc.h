/****************************************************************************
*
*     Copyright (c) 2007-2008 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license 
*   agreement governing use of this software, this software is licensed to you 
*   under the terms of the GNU General Public License version 2, available 
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL"). 
*
*   Notwithstanding the above, under no circumstances may you combine this 
*   software in any way with any other Broadcom software provided under a license 
*   other than the GPL, without Broadcom's express prior written consent.
*
****************************************************************************/
//********************************************************************
//!
//! \file  rpc_ipc.h
//! \brief RPC Interface to access raw interface to send/recv buffers across processor
//!
//! Encapsulates the IPC interface.
//! \section Usage
//!         RPC_PACKET_RegisterDataInd() is called once for each interface to register Client's callback function.
//!
//!         RPC_PACKET_AllocateBuffer() is called to allocate the buffer which is later used in RPC_PACKET_SendData to send buffer
//!
//!         The Callback #RPC_PACKET_DataIndCallBackFunc_t is called when the data arrives from remote processor. User can call RPC_PACKET_FreeBuffer()
//!         to free the buffer notified in callback.
//!         
//!         RPC_PACKET_GetBufferData() is used to get the raw pointer from buffer handle.
//!	\msc
//!		Client, "RPC Lite", IPC;
//!		Client rbox Client [label="Startup"];
//!		"RPC Lite"=>IPC [label="IPC_EndpointRegister (for all eps)"];
//!		Client=>"RPC Lite" [label="RPC_PACKET_RegisterDataInd(INTERFACE_CSD, cbk)", 
//!								URL="\ref RPC_PACKET_RegisterDataInd()"];
//!		Client rbox Client [label="Send"];
//!		Client=>"RPC Lite" [label="RPC_PACKET_AllocateBuffer(INTERFACE_CSD, size)", 
//!								URL="\ref RPC_PACKET_AllocateBuffer()"];
//!		"RPC Lite"=>IPC [label="IPC_CreateBufferPool() On demand. Will be called for first time only"];
//!		Client=>"RPC Lite" [label="RPC_PACKET_SendData(bufHandle)", 
//!								URL="\ref RPC_PACKET_SendData()"];
//!		"RPC Lite"=>IPC [label="IPC_SendBuffer(bufHandle)"];
//!		Client rbox Client [label="Recv Msg"];
//!		"RPC Lite"<=IPC [label="CBK_Notify_Request(bufHandle)"];
//!		Client<="RPC Lite" [label="CBK(INTERFACE_CSD, bufHandle)"];
//!		Client=>"RPC Lite" [label="RPC_PACKET_FreeBuffer(bufHandle)", 
//!								URL="\ref RPC_PACKET_FreeBuffer()"];
//!	\endmsc

//! @{
//!

#ifndef __RPC_IPC
#define __RPC_IPC

#include "mobcom_types.h"
#include "rpc_global.h"
#ifdef UNDER_LINUX
#define CSL_TYPES_H
#include <linux/broadcom/IPCInterface.h>
#include <linux/broadcom/IPCProperties.h>
#else
#include "IPCInterface.h"
#include "IPCProperties.h"
#endif
/**
	RPC Packet Buffer Handle
**/
typedef void* PACKET_BufHandle_t;

/**
	RPC Interface Types
**/
typedef enum
{
	INTERFACE_RPC_TELEPHONY = 0,		///< Telephony related
	INTERFACE_RPC_PMU,			///<  PMU related
	INTERFACE_RPC_AUDIO,				///<  Audio related
	INTERFACE_RPC_TOTAL,			///<  Number of rpc interface
}RPC_InterfaceType_t;



/**
	Packet Interface Types
**/
typedef enum
{
	INTERFACE_CAPI2 = 0,		///< for internal use
	INTERFACE_PACKET = INTERFACE_RPC_TOTAL,			///< for packet data
	INTERFACE_CSD,				///< for csd data
	INTERFACE_LOGGING,			///< for logging
	INTERFACE_SS_IPC40,			///< for SS IPC 4.0
	INTERFACE_TOTAL				///< total interfaces
}PACKET_InterfaceType_t;

/**
	Use this macro to wait forever on IPC buffer allocation
**/
#define PKT_ALLOC_WAIT_FOREVER	~0	///< 

/**
	Use this macro to return NULL when IPC buffer allocation fail
**/
#define PKT_ALLOC_NOWAIT		 0	///< 

/**
RPC Flow Control Events
**/
typedef enum
{
	RPC_FLOW_START,	///< flow control resume
	RPC_FLOW_STOP		///< flow control stop
} RPC_FlowCtrlEvent_t;


/**
RPC Error codes
**/
typedef enum
{
	RPC_RESULT_OK,			///< Success
	RPC_RESULT_ERROR,		///< Failure case
	RPC_RESULT_PENDING		///< Processing is pending.
}RPC_Result_t;

/**
	RPC property types
**/
typedef enum
{
	RPC_PROP_START_AP = IPC_PROPERTY_START_AP,			///< ( AP is Read/Write, CP is Read only )
 	RPC_PROP_BACKLIGHT_LEVEL,		///< 0 is off, 1 is low, 2 is high
 	RPC_PROP_CHARGER_PRESENT,		///< 1 is present, 0 is not present
 	RPC_PROP_AP_IN_DEEPSLEEP,		///< 1 is deepsleep, 0 otherwise
	RPC_PROP_END_AP = IPC_PROPERTY_END_AP,

	RPC_PROP_START_CP = IPC_PROPERTY_START_CP,			///< ( CP is Read/Write, AP is Read only )
 	RPC_PROP_CP_IN_DEEPSLEEP,		///< 1 is deepsleep, 0 otherwise
	RPC_PROP_END_CP = IPC_PROPERTY_END_CP,

	RPC_MAX_PROP_TYPE = IPC_NUM_OF_PROPERTIES,		///< TBD: Align with IPC max
} RPC_PropType_t;


//***************************************************************************************
/**
    Function callback to handle Flow control for RPC commands
	@param		event (in ) RPC_FLOW_START to resume flow control and RPC_FLOW_STOP to stop
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD
	@return		None
	@note
		This callback is registered via RPC_PACKET_RegisterDataInd.

**/
typedef void (RPC_FlowControlCallbackFunc_t) (RPC_FlowCtrlEvent_t event, UInt8 channel);

//***************************************************************************************
/**
    Function callback to receive data packets for Packet Data interfaceType
	@param		interfaceType (in) Interface the packet was received
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD ( others interface is N/A)
	@param		dataBufHandle (in) Buffer handle. Use RPC_PACKET_GetBufferData and RPC_PACKET_GetBufferLength
				 to get the actual data pointer and length.
	@return
			\n  RPC_RESULT_OK :  success dataBuf will be released after the call returns. Client MUST make copy.
		  \n\n  RPC_RESULT_ERROR :  failure, dataBuf will be released after the call returns.
			\n        In addtion RPC MAY initiate flow control to slow the downlink packet data ( TBD )
		  \n\n  RPC_RESULT_PENDING : pending, The client decide to delay consuming packet,
			\n        in which case the buffer will NOT be relased by RPC after function return.
			\n      The Client needs to call RPC_PACKET_FreeBuffer to free buffer later)
	@note
		The client can distinguish between different Primary PDP context session by channel for INTERFACE_PACKET.
	@note
		The data buffer is already mapped to calling thread process space ( virtual / kernal address space )

**/
typedef RPC_Result_t (RPC_PACKET_DataIndCallBackFunc_t) (PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle);


//***************************************************************************************
/**
    Register the callback to receive the indication of data available from the network(air interface)
	@param		rpcClientID (in) Client ID
	@param		interfaceType (in) Specify interface to send/recv the packet
	@param		dataIndFunc (in) callback function to inform the availability of the data.
	@param		flowControlCb (in) callback function for flow control notification.
	@return	Return RPC_RESULT_OK if the registration is successful. Otherwise RPC_RESULT_ERROR
	@note
		The RPC currently supports simultaneous primary PDP data connections. The cid is used
		to distinguish between different Primary PDP context sessions.
**/
RPC_Result_t RPC_PACKET_RegisterDataInd (UInt8 rpcClientID,
									PACKET_InterfaceType_t interfaceType,
									RPC_PACKET_DataIndCallBackFunc_t dataIndFunc,
									RPC_FlowControlCallbackFunc_t	flowControlCb);


//***************************************************************************************
/**
    This function sends packet data to remote processor
	@param		rpcClientID (in) Client ID
	@param		interfaceType (in) Specify interface to send the packet
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD ( others interface is N/A)
	@param		dataBufHandle (in) The buffer handle returned in RPC_PACKET_AllocateBuffer.
	@return		RPC_RESULT_OK if the packet is copied to FIFO, RPC_RESULT_ERROR if FIFO is full
	@note
**/
RPC_Result_t RPC_PACKET_SendData(UInt8 rpcClientID, PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle);


//***************************************************************************************
/**
    This function allocates buffer for sending packet data.
	@param		interfaceType (in) Specify interface to send the packet
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD ( others interface is N/A)
	@param		requiredSize (in) required packet size

	@return		valid buffer handle OR NULL
	@note
				Client need to call RPC_PACKET_GetBufferData to get the actual buffer pointer
**/
PACKET_BufHandle_t RPC_PACKET_AllocateBuffer(PACKET_InterfaceType_t interfaceType, UInt32 requiredSize, UInt8 channel);

//***************************************************************************************
/**
    This function allocates buffer for sending packet data with wait time.
	@param		interfaceType (in) Specify interface to send the packet
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD ( others interface is N/A)
	@param		requiredSize (in) required packet size
	@param		waitTime (in) wait time in ms. PKT_ALLOC_WAIT_FOREVER or PKT_ALLOC_NOWAIT are also valid

	@return		valid buffer handle OR NULL
	@note
				Client need to call RPC_PACKET_GetBufferData to get the actual buffer pointer
**/
PACKET_BufHandle_t RPC_PACKET_AllocateBufferEx(PACKET_InterfaceType_t interfaceType, UInt32 requiredSize, UInt8 channel, UInt32 waitTime);

//***************************************************************************************
/**
    Free the packet passed in RPC_PACKET_DataIndCallBackFunc_t function.
	The RPC_PACKET_DataIndCallBackFunc_t must return RPC_RESULT_PENDING to use this functionality
	@param	dataBufHandle (in) The buffer handle notified in the callback RPC_PACKET_DataIndCallBackFunc_t.

	@return	Return RPC_RESULT_OK if buffer is valid. Otherwise RPC_RESULT_ERROR

	@note
		Calling RPC_PACKET_FreeBuffer twice for same buffer will have unpredictable result
	@note
		NOT calling this function in timely manner, will result in buffer exhaustion and the client
		will stop receiving data. Packets will be dropped until the buffers are freed.
**/
RPC_Result_t RPC_PACKET_FreeBuffer(PACKET_BufHandle_t dataBufHandle);

//***************************************************************************************
/**
    This function returns the actual raw buffer pointer from the buffer handle
	@param		dataBufHandle (in) Buffer handle allocated using ARPC_PACKET_AllocateBuffer or
				the buffer handle notified in the callback ARPC_PACKET_DataIndCallBackFunc_t.

	@return		valid memory location
**/
void* RPC_PACKET_GetBufferData(PACKET_BufHandle_t dataBufHandle);

//***************************************************************************************
/**
    This function gets the length of the actual buffer associated with the buffer handle
	@param		dataBufHandle (in) Buffer handle allocated using RPC_PACKET_AllocateBuffer or
				the buffer handle notified in the callback RPC_PACKET_DataIndCallBackFunc_t.

	@return		Length of buffer
**/
UInt32 RPC_PACKET_GetBufferLength(PACKET_BufHandle_t dataBufHandle);

//***************************************************************************************
/**
    This function sets the length of the actual buffer associated with the buffer handle
	@param		dataBufHandle (in) Buffer handle allocated using RPC_PACKET_AllocateBuffer or
				the buffer handle notified in the callback RPC_PACKET_DataIndCallBackFunc_t.
	@param		bufferSize (in) packet size

	@return		None
**/
void RPC_PACKET_SetBufferLength(PACKET_BufHandle_t dataBufHandle, UInt32 bufferSize);


//***************************************************************************************
/**
    This function returns aggregate free buffers for all channels.
	@param		interfaceType (in) Specify interface to get free buffers.
	@param		index (in) Pass only 0xFF to get aggregate free buffers for all channels.

	@return		Number of free buffers for the interface type.
			
**/
UInt32 RPC_PACKET_Get_Num_FreeBuffers(PACKET_InterfaceType_t interfaceType, UInt8 index);

/** @} */

/** \cond  */


#define MAX_CHANNELS	10

/**
	RPC Processor type
**/
typedef enum
{
   RPC_APPS,
   RPC_COMMS
}RpcProcessorType_t;


//***************************************************************************************
/**
    Function to initialize IPC mechanism for RPC system ( Should be the first API to call )
	@param		processorType (in) RPC_APPS or RPC_COMMS
	@return		\n RPC_RESULT_OK for success,
				\n RPC_RESULT_ERROR for failure
**/
RPC_Result_t RPC_IPC_Init(RpcProcessorType_t ptype);
RPC_Result_t RPC_IPC_EndPointInit(RpcProcessorType_t ptype);


//Private functions

IPC_BufferPool RPC_InternalGetCmdPoolHandle(void);


/** \endcond   */

#endif

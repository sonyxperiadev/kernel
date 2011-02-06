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
/**
*
*
*   @file   capi2_ds_api.h
*
*   @brief  This file defines APIs for CSD data access and Packet Data Access
*
****************************************************************************/
#ifndef __CAPI2_DSAPI_H__
#define __CAPI2_DSAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "capi2_types.h"
#include "capi2_taskmsgs.h"

#ifdef RPC_INCLUDED
#include "rpc_ipc.h"
#else

#include "capi2_rpc_map.h"


/****************************************************************************/
/**

*   @defgroup   CAPI2_DATAPKTGroup   Packet or CSD Data Access
*
*   @brief      PACKET Data Access: This group defines the interfaces to send /
*   receive packet data or CSD Data in Packetized form.
*
*	@note
*	The CSD setup is accomplished by the APIs CAPI2_CC_MakeDataCall() and 
*   CAPI2_CC_AcceptDataCall() and the message ::MSG_DATACALL_CONNECTED_IND.
*   Then clients can use the PACKET Data Access API's for sending 
*   and receiving CSD Data
*
****************************************************************************/

/**
	CAPI2 Flow Control Events
**/
typedef enum
{
	CAPI2_FLOW_START,	///< capi2 flow control resume
	CAPI2_FLOW_STOP		///< capi2 flow control stop
} CAPI2_FlowCtrlEvent_t;

/**
 * @addtogroup CAPI2_PhoneControlSysInit
 * @{
 */

//***************************************************************************************
/**
    Function callback to handle Flow control for CAPI2 commands
	@param		event (in ) CAPI2_FLOW_START to resume flow control and CAPI2_FLOW_STOP to stop
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD
	@return		None
	@note
		This callback is registered via CAPI2_SYS_RegisterClient for CAPI2 commands and
		CAPI2_PACKET_RegisterDataInd for Packet Data.

**/
typedef void (CAPI2_FlowControlCallbackFunc_t) (CAPI2_FlowCtrlEvent_t event, UInt8 channel);

/** @} */

/**
	CAPI2 Packet Buffer Handle
**/
typedef void* PACKET_BufHandle_t;

/**
	CAPI2 Interface Type
**/
//Warning: Do NOT MODIFY this structure OR change order
typedef enum
{
	INTERFACE_CAPI2 = 0,		///< for internal use
	INTERFACE_PACKET,			///< for packet data
	INTERFACE_CSD,				///< for csd data
	INTERFACE_LOGGING,
	INTERFACE_TOTAL				///< total interfaces
}PACKET_InterfaceType_t;

/**
 * @addtogroup CAPI2_DATAPKTGroup
 * @{
 */

//***************************************************************************************
/**
    Function callback to receive data packets for Packet Data interfaceType
	@param		interfaceType (in) Interface the packet was received
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD
	@param		dataBufHandle (in) Buffer handle. Use CAPI2_PACKET_GetBufferData() and CAPI2_PACKET_GetBufferLength()
				 to get the actual data pointer and length.
	@return
			\n  ::RESULT_OK :  success dataBuf will be released after the call returns. Client MUST make copy.
		  \n\n  ::RESULT_ERROR :  failure, dataBuf will be released after the call returns.
			\n        In addtion CAPI2 MAY initiate flow control to slow the downlink packet data ( TBD )
		  \n\n  RESULT_PENDING : pending, The client decide to delay consuming packet,
			\n        in which case the buffer will NOT be relased by CAPI2 after function return.
			\n      The Client needs to call CAPI2_PACKET_FreeBuffer to free buffer later)
	@note
		The client can distinguish between different Primary PDP context session by channel for INTERFACE_PACKET.
	@note
		The data buffer is mapped to calling thread process space ( virtual / kernal address space )

**/
typedef Result_t (CAPI2_PACKET_DataIndCallBackFunc_t) (PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle);


//***************************************************************************************
/**
    Register the callback to receive the indication of data available from the network(air interface)
	@param		clientID (in) Client ID
	@param		interfaceType (in) Specify interface to send/recv the packet
	@param		dataIndFunc (in) callback function to inform the availability of the data.
	@param		flowControlCb (in) callback function for flow control notification.
	@return	Return ::RESULT_OK if the registration is successful. Otherwise ::RESULT_ERROR
	@note
		The CAPI currently supports simultaneous primary PDP data connections. The cid is used
		to distinguish between different Primary PDP context sessions.
**/
Result_t CAPI2_PACKET_RegisterDataInd (UInt8 clientID,
									PACKET_InterfaceType_t interfaceType,
									CAPI2_PACKET_DataIndCallBackFunc_t dataIndFunc,
									CAPI2_FlowControlCallbackFunc_t	flowControlCb);


//***************************************************************************************
/**
    This function sends packet data to the network(air interface).
	@param		clientID (in) Client ID
	@param		interfaceType (in) Specify interface to send the packet
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD
	@param		dataBufHandle (in) The buffer handle notified in the callback CAPI2_PACKET_DataIndCallBackFunc_t().
	@return		::RESULT_OK if the packet is copied to FIFO, ::RESULT_ERROR if FIFO is full
	@note
**/
Result_t CAPI2_PACKET_SendData(UInt8 clientID, PACKET_InterfaceType_t interfaceType, UInt8 channel, PACKET_BufHandle_t dataBufHandle);


//***************************************************************************************
/**
    This function allocates buffer for sending packet data.
	@param		interfaceType (in) Specify interface to send the packet
	@param		channel (in) Context id for INTERFACE_PACKET OR Call Index for INTERFACE_CSD
	@param		requiredSize (in) required packet size

	@return		valid buffer handle OR NULL
	@note
				Client need to call CAPI2_PACKET_GetBufferData() to get the actual buffer pointer
**/
PACKET_BufHandle_t CAPI2_PACKET_AllocateBuffer(PACKET_InterfaceType_t interfaceType, UInt32 requiredSize, UInt8 channel);

//***************************************************************************************
/**
    Free the packet passed in CAPI2_PACKET_DataIndCallBackFunc_t function.
	The CAPI2_PACKET_DataIndCallBackFunc_t must return RESULT_PENDING to use this functionality
	@param	dataBufHandle (in) The buffer handle notified in the callback CAPI2_PACKET_DataIndCallBackFunc_t().

	@return	Return ::RESULT_OK if buffer is valid. Otherwise ::RESULT_ERROR

	@note
		Calling CAPI2_PACKET_FreeBuffer() twice for same buffer will have unpredictable result
	@note
		Not calling this function in timely manner, will result in buffer exhaustion and the client
		will stop receiving data. Packets will be dropped until the buffers are freed.
**/
Result_t CAPI2_PACKET_FreeBuffer(PACKET_BufHandle_t dataBufHandle);

//***************************************************************************************
/**
    This function returns the actual raw buffer pointer from the buffer handle
	@param		dataBufHandle (in) Buffer handle allocated using CAPI2_PACKET_AllocateBuffer() or
				the buffer handle notified in the callback CAPI2_PACKET_DataIndCallBackFunc_t().

	@return		valid memory location
**/
void* CAPI2_PACKET_GetBufferData(PACKET_BufHandle_t dataBufHandle);

//***************************************************************************************
/**
    This function gets the length of the actual buffer associated with the buffer handle
	@param		dataBufHandle (in) Buffer handle allocated using CAPI2_PACKET_AllocateBuffer() or
				the buffer handle notified in the callback CAPI2_PACKET_DataIndCallBackFunc_t().

	@return		Length of buffer
**/
UInt32 CAPI2_PACKET_GetBufferLength(PACKET_BufHandle_t dataBufHandle);

//***************************************************************************************
/**
    This function sets the length of the actual buffer associated with the buffer handle
	@param		dataBufHandle (in) Buffer handle allocated using CAPI2_PACKET_AllocateBuffer() or
				the buffer handle notified in the callback CAPI2_PACKET_DataIndCallBackFunc_t().
	@param		bufferSize (in) packet size

	@return		None
**/
void CAPI2_PACKET_SetBufferLength(PACKET_BufHandle_t dataBufHandle, UInt32 bufferSize);


/** @} */

#endif

#ifdef __cplusplus
}
#endif

#endif


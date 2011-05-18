//*********************************************************************
//
//	Copyright © 2000-2008 Broadcom Corporation
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
*   @file   taskmsgs.h
*
*   @brief  This file contains all asynchronous inter-task message definitions.
*
*****************************************************************************/

#ifndef _TASKMSGS_H_
#define _TASKMSGS_H_


#define DEFINE_CAPI2_GEN_MIDS
#define DEFINE_CAPI_GEN_MIDS
#define DEFINE_SYS_GEN_MIDS
//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------
#define CLIENT_INDEX_WILD_CARD				0xFF	///< Client Index Wild Card

#define	INVALID_CLIENT_ID					 0


#define  MAX_CLIENT_NUM						30		///< 5 tasks in platform (atc, bluetooth, dlink, stk ds, test MMI) using
													///< SYS_RegisterForMSEvent(), leaving 5 more clients for MMI/application use

//--- Internal/Reserved Client ID (1-63) ----------
#define	STK_CLIENT_ID  						 1	///< STK Client Identifier
#define	MNCC_CLIENT_ID  					 2	///< MNCC Client Identifier
#define DEFAULT_CLIENT_ID					15  ///< Internal default Client Identifier
#define MT_USSD_CLIENT_ID					17	///< client ID assigned when a MT USSD is received.(for ATC use)

//--- External Client Identifiers (64-on) ---------

//The client IDs from (CLIENT_ID_V24_0 + AT_INTER_CHNL_START_IDX) to (CLIENT_ID_V24_0 + AT_NUM_OF_ALL_CHNL)
// are reserved for internal AT command channels.

/**
	Maximum number of registered event masks for each client
 **/
#define	 MAX_REGISTERED_EVENT_MASK_NUM		5

/**
	Maximum number of filtered event masks for each client
 **/
#define  MAX_FILTERED_EVENT_MASK_NUM		10

typedef enum
{
	// InterTaskMsg group code definitions below must follow these rules:
	//
	// 1. INTER_TASK_MSG_GROUP_BEGIN is the first code
	// 2. INTER_TASK_MSG_GROUP_END is the last code
	// 3. Every definition in between must match regular expression "^\s*MSG_GRP_(\w+)\s+=\s+0x(\x\x00)\s*,"
	//    i.e., every message group's name starts with "MSG_GRP_", and its code is a multiple of 0x100.
	// 4. No message group code should be changed. Obsolete message groups should hold their codes as long as possible
	//
	// Following above rules allows auto-generation of decoding library with backward compatibility.
	// Hui Luo, 6/5/08

	INTER_TASK_MSG_GROUP_BEGIN	= 0,	// This line must be here in order to generate grouped binary inter-task message logging, Hui Luo, 6/5/08

	MSG_GRP_SYS 				= 0x0000,
	MSG_GRP_UTIL 				= 0x0100,
	MSG_GRP_NET 				= 0x0200,
	MSG_GRP_CC  				= 0x0300,
	MSG_GRP_SMS 				= 0x0400,
	MSG_GRP_PBK 				= 0x0500,
	MSG_GRP_DC  				= 0x0600,
	MSG_GRP_DEV 				= 0x0700,
	MSG_GRP_STK 				= 0x0800,
	MSG_GRP_SS  				= 0x0900,
	MSG_GRP_MNSS				= 0x0A00,
	MSG_GRP_USSD				= 0x0B00,
	MSG_GRP_SIM 				= 0x0C00,
	MSG_GRP_LCS 				= 0x0D00,
	MSG_GRP_TLS 				= 0x0E00,
	MSG_GRP_FTP 				= 0x0F00,

	MSG_GRP_INT_UTIL			= 0x2000,
	MSG_GRP_INT_ATC 			= 0x2100,
	MSG_GRP_INT_SMS_SS			= 0x2200,
	MSG_GRP_INT_NET				= 0x2300,
	MSG_GRP_INT_DC				= 0x2400,
	MSG_GRP_INT_SIM				= 0x2500,
	MSG_GRP_INT_STK				= 0x2600,
	MSG_GRP_INT_VCC				= 0x2700,

	MSG_GRP_CAPI2_LCS			= 0x3200,
	MSG_GRP_CAPI2_SMS			= 0x3300,
	MSG_GRP_CAPI2_PBK			= 0x3400,
	MSG_GRP_CAPI2_STK			= 0x3500,
	MSG_GRP_CAPI2_CC_RESP		= 0x3600,
	MSG_GRP_CAPI2_CC_REQ		= 0x3700,
	MSG_GRP_CAPI2_INT_PBK 		= 0x3800,
	MSG_GRP_CAPI2_INT_SMS		= 0x3900,
	MSG_GRP_CAPI2_INT_STK		= 0x3A00,
	MSG_GRP_CAPI2_SS			= 0x3B00,
	MSG_GRP_CAPI2_SIM			= 0x3C00,
	MSG_GRP_CAPI2_PCH			= 0x3D00,
	MSG_GRP_CAPI2_UTIL			= 0x3E00,
	MSG_GRP_CAPI2_PRIVATE		= 0x3F00,

	MSG_GRP_CAPI2_GEN_0			= 0x4000,
	MSG_GRP_CAPI2_GEN_1			= 0x4100,
	MSG_GRP_CAPI2_GEN_2			= 0x4200,
	MSG_GRP_CAPI2_GEN_3			= 0x4300,
	MSG_GRP_CAPI2_GEN_4			= 0x4400,
	MSG_GRP_CAPI2_GEN_5			= 0x4500,
	MSG_GRP_CAPI2_GEN_6			= 0x4600,
	MSG_GRP_CAPI2_GEN_7			= 0x4700,
	MSG_GRP_CAPI2_GEN_8			= 0x4800,
	MSG_GRP_CAPI2_GEN_9			= 0x4900,
	MSG_GRP_CAPI2_GEN_A			= 0x4A00,
	MSG_GRP_CAPI2_GEN_B			= 0x4B00,
	MSG_GRP_CAPI2_GEN_C			= 0x4C00,
	MSG_GRP_CAPI2_GEN_D			= 0x4D00,
	MSG_GRP_CAPI2_GEN_E			= 0x4E00,
	MSG_GRP_CAPI2_GEN_F			= 0x4F00,

	MSG_GRP_SIMULATOR_0 		= 0x6000,
	MSG_GRP_SIMULATOR_1 		= 0x6100,
	MSG_GRP_SIMULATOR_2 		= 0x6200,
	MSG_GRP_SIMULATOR_3			= 0x6300,

	MSG_GRP_TEST_PCH			= 0x7000,
	MSG_GRP_SCRIPT				= 0x7100,
	MSG_GRP_END 				= 0x7FFF,

	MSG_GRP_USER_FIRST			= 0x8000,
	MSG_GRP_USER_LAST			= 0xFE00,

	INTER_TASK_MSG_GROUP_END	= 0xFFFF	// This line must be here in order to generate grouped binary inter-task message logging, Hui Luo, 6/5/08
} MsgGroup_t;

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------
typedef enum
{	// explicitly enum for easier debugging

	/*
	  Definitions of all asynchronous events from Broadcom Platform API
	 */

	// InterTaskMsg code definitions below must follow these rules:
	//
	// 1. INTER_TASK_MSG_BEGIN is the first code
	// 2. MSG_INVALID is the last code
	// 3. Every definition in between must match regular expression "^\s*MSG_(\w+)\s+=\s+MSG_GRP(\w+)\+0x(\x+)\s*,"
	//    i.e., every message's name starts with "MSG_", and its code is defined as a summation of a group name and a hex value within 0xFF.
	// 4. If a message's dataBuf carries a specific data type, the definition line must match
	//    regular expression "^\s*MSG_(\w+)\s+=\s+MSG_GRP(\w+)\+0x(\x+)\s*,\s*///\<Payload type \{\:\:(\w+)\}\s*"
	// 5. No message code should be changed. Obsolete messages still hold their codes as long as possible
	//
	// Following above rules allows auto-generation of decoding library with backward compatibility.
	// Hui Luo, 10/2/07

	INTER_TASK_MSG_BEGIN		= 0,		// This line must be here in order to generate binary inter-task message logging, Hui Luo, 10/2/07

//TASKMSGS_INCLUDE taskmsgs_modem.i
//TASKMSGS_INCLUDE taskmsgs_atc.i
//TASKMSGS_INCLUDE taskmsgs_usb.i
//TASKMSGS_INCLUDE taskmsgs_agps.i
//TASKMSGS_INCLUDE taskmsgs_dataservices.i
//TASKMSGS_INCLUDE taskmsgs_dataservices_apps.i
//TASKMSGS_INCLUDE taskmsgs_xscript.i
//TASKMSGS_INCLUDE taskmsgs_mmi.i
//TASKMSGS_INCLUDE taskmsgs_capi2.i
//TASKMSGS_INCLUDE taskmsgs_sysrpc.i

	//------------------------------------------------
	// end of list
	//-------------------------------------------------
	MSG_INVALID					= 0xFFFF	// This line must be here in order to generate binary inter-task message logging, Hui Luo, 10/2/07
} MsgType_t;

//-------------------------------------------------
// Data Structure and Types
//-------------------------------------------------

#define		ITMSG_BROADCAST		1
#define		ITMSG_SENT			2
#define		ITMSG_RECEIVED		3
#define		ITMSG_DELETED		4

typedef	void	*T_INTER_TASK_MSG_PTR;

typedef char T_NU_NAME[8];

/**
Structure : Sim Identifier Type
**/
typedef enum
{
	SIM_SINGLE,							///< single SIM case
	SIM_DUAL_FIRST,						///< the first SIM
	SIM_DUAL_SECOND,					///< the second SIM
	SIM_ALL = 0xFF						///< for all the SIMs

} SimNumber_t;

/**
Structure : API Client Information Type
**/
typedef struct
{
	UInt8		clientId;				///< Client Identifier
	SimNumber_t	simId;					///< SIM Identifier: SimNumber_t
	UInt32		clientRef;				///< Client Reference
	UInt32		dialogId;				///< Dialog Identifier numarated by CAPI, see MS_GetElement(<clienId>, MS_LOCAL_MS_ELEM_DIALOG_ID, <DialogId>);
	UInt32		reserved;				///< Reserved for internal use only

} ClientInfo_t; 						///< Client Information Type

/**
Structure : Inter Task Message Type
	The InterTaskMsg_t defines the structure of all intertask messages
	broadcast by the platform.   Messages are allocated via a call to the
	function AllocInterTAskMsgFromHeap and deallocated by a call to the
	function FreeInterTaskMsg.
Note: This type should not be changed constantly for backward compatibility of decoding library.
	The "//StructFieldBegin{InterTaskMsg_t}" and "StructFieldEnd{InterTaskMsg_t}" should be kept
	at the place before the first field and after the last field so that parser can pick it up.
**/
typedef struct {			///< msg data buffer: sender alloc; reciver dealloc!!
	//StructFieldBegin{InterTaskMsg_t}	//Do not change!!!
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PTR	dataBuf;	///< payload data (may include 0 as content)
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
	//StructFieldEnd{InterTaskMsg_t}	//Do not change!!!
} InterTaskMsg_t;						///< Inter Task Message Type


typedef void (CallbackFunc_t) (InterTaskMsg_t* inMsg);

typedef enum
{
  SYS_DISABLE_MSG_FILTER,	///< disable filter mask
  SYS_ENABLE_MSG_FILTER,	///< enable filter mask
  SYS_AP_DEEP_SLEEP_MSG_FILTER	///< Use callback function if filter is to be enabled or disabled
}SysFilterEnable_t;

/**
Callback function to check if filter mask is to be enabled or disabled
**/
typedef Boolean (CbFilterQueryCheckFunc_t)(void);


//-------------------------------------------------
// Function Prototype
//-------------------------------------------------


#ifdef HISTORY_LOGGING
#define	AllocInterTaskMsgFromHeap(msgType, dataLength)	\
		AllocInterTaskMsgFromHeapDbg(msgType, dataLength, __FILE__, __LINE__)
InterTaskMsg_t*	AllocInterTaskMsgFromHeapDbg(MsgType_t msgType, UInt16 dataLength, char* fileName, UInt32 lineNumber);
#else // #ifdef HISTORY_LOGGING
/**
	Allocate memory for an intertask message.

	@param	msgType     (in) One of the message types in ::MsgType_t
	@param  dataLength (in) Length of paylod in bytes.
	@return Pointer to an ::InterTaskMsg_t structure
**/
InterTaskMsg_t*	AllocInterTaskMsgFromHeap(MsgType_t msgType, UInt16 dataLength);
#endif // #ifdef HISTORY_LOGGING



#ifdef HISTORY_LOGGING
#define	FreeInterTaskMsg(inMsg)	FreeInterTaskMsgDbg(inMsg, __FILE__, __LINE__)
void	FreeInterTaskMsgDbg(InterTaskMsg_t* inMsg, char* fileName, UInt32 lineNumber);
#else // #ifdef HISTORY_LOGGING
/**
	Free memory that was allocated by AllocInterTaskMsgFromHeap.

	@param inMsg	(in)	Pointer to an ::InterTaskMsg_t
	@return void
**/
void	FreeInterTaskMsg(InterTaskMsg_t* inMsg);
#endif // #ifdef HISTORY_LOGGING

//***************************************************************************************
/**
	Function to register to receive MS messages
	@param		*callback (in) Pointer to callback function
	@param		eventMask (in) Event mask to receive messages(currently ignored)
	@return		UInt8 Client ID.

	@note
	This function is used to register the client to receive MS messages.
	It returns INVALID_CLIENT_ID if the operation fails.  The current
	implementation igores the eventMask. <br>
	Upon calling this function, the client's event masks are set with the following values so that
	all unsolicited events are received by default: one Registered Event Mask of 0xFFFF and no Filtered
	Event Mask. The client can call SYS_SetRegisteredEventMask() and SYS_SetFilteredEventMask()
	to modify the event masks.
**/

UInt8 SYS_RegisterForMSEvent(CallbackFunc_t* callback, UInt32 eventMask);

//***************************************************************************************
/**
	Function to register predefined clientID to receive Unicast message by default
	@param		*callback (in) Pointer to callback function
	@param		clientId (in) predefined ClientID (Valid for Reserved Client ID (1-63) )
	@param		eventMask (in) Event mask to receive messages(currently ignored)
	@return		Boolean

	@note
	This function is used to register the client to receive unicast message only. Clients can call -
	SYS_SetFilteredEventMask later to receive broadcast messages
	It returns FALSE if the number of clients exceed OR if the clientId is already registered.
**/
Boolean SYS_RegisterForPredefinedClient(CallbackFunc_t* callback, UInt8 clientId, UInt32 eventMask);

//***************************************************************************************
/**
	Function to de-register receiving MS events
	@param		clientID (in) Client ID for which to stop receiving events
	@note
	This function is used to stop receiving MS events for specified Client IDs.
**/

void SYS_DeRegisterForMSEvent(UInt8 clientID);

//***************************************************************************************
/**
	Function to set a registered client's Registered Event Mask List.
	@param		clientID (in) Client ID
	@param		maskList (in) List of event masks.
	@param		maskLen (in) Number of event masks passed in "maskList".
	@return		TRUE for success; FALSE for failure.

	@note
	This function is called to set the Registered Event Mask list for a registered client.
	For an unsolicited event to be broadcast to a specific client, the event's message type must
	be included in the client's registered event mask list. Specifically, there must be at least
	one registered event mask in the list whose logical AND operation with the event's message type
	is equal to the event's message type itself.

	Typically the client can just use the default setting created by SYS_RegisterForMSEvent()
	which is: one registered event mask of 0xFFFF to receive all unsolicited events.
	If the default setting is used, there is no need to call this function.

	This function can be called for more flexibility with the event mask set to the XOR value of
	the message types of all unsolicited events the client wants to receive.

**/

Boolean SYS_SetRegisteredEventMask(UInt8 clientID, const UInt16 *maskList, UInt8 maskLen);


//***************************************************************************************
/**
	Function to set a registered client's Filtered Event Mask List.
	@param		clientID (in) Client ID
	@param		maskList (in) List of event masks.
	@param		maskLen (in) Number of event masks passed in "maskList".
	@param		enableFlag (in) Flag to control the filter.
	@return		TRUE for success; FALSE for failure.

	@note
	This function is called to set the Filtered Event Mask list for a registered client.
	For an unsolicited event to be broadcast to a specific client, the event's message type must
	NOT be included in the client's filtered event mask list. Specifically, there must not be
	one filtered event mask in the list whose logical AND operation with the event's message type
	is equal to the event's message type itself.

	Typically the client can just use the default setting created by SYS_RegisterForMSEvent()
	which is: one registered event mask of 0xFFFF to receive all unsolicited events.
	If the default setting is used, there is no need to call this function.

	This function can be called for more flexibility with the event mask set to the XOR value of
	the message types of all unsolicited events the client do not want to receive.
**/

Boolean SYS_SetFilteredEventMask(UInt8 clientID, const UInt16 *maskList, UInt8 maskLen, SysFilterEnable_t enableFlag);

//***************************************************************************************
/**
	Function to enable disable Filtered Event Mask List set by SYS_SetFilteredEventMask
	@param		clientID (in) Client ID
	@param		flag (in) Flag to control the filter.
	@param		cbk (in) Flag to query if the filter is to be enabled. ( Typically hooked to check if AP is in deep sleep in dual processor architecture )
	@return		TRUE for success; FALSE for failure.

	@note
**/
Boolean SYS_EnableFilterMessage(UInt8 clientID, SysFilterEnable_t flag, CbFilterQueryCheckFunc_t cbk);

/** @cond */

void SYS_SendMsgToClient(UInt8 inClientID, InterTaskMsg_t* inMsg);

void SYS_BroadcastMsg(InterTaskMsg_t* inMsg);

//***************************************************************************************
/**
	Function function post the message to the client.
	@param		clientID (in) Client ID
	@param		inMsgPtr (in) pointer to the message.
	@return		void.

	@note
**/
void SYS_PostToClient(UInt8 inClientID, InterTaskMsg_t* inMsgPtr);

//---------------------------------------------------------------
// SYS_IsEventFiltered(): Check if the event is filtered
//---------------------------------------------------------------
Boolean SYS_IsEventFiltered(UInt8 clientID, MsgType_t inMsg);

//--------------------------------------------------------
// InitTaskMsgs(): initialize client and inter-task module
//--------------------------------------------------------
void InitTaskMsgs(void);

UInt8 SYS_GenClientID(void);
void SYS_SetClientID(UInt8 clientID);
Boolean SYS_IsRegisteredClientID(UInt8 clientID);

UInt8 SYS_GenClientIDRange(UInt8 range );
void SYS_SetClientID(unsigned char clientID);

void	SYS_SyncTaskMsg( void );

Boolean SYS_RegisterSetClientName(UInt8 inClientID, char* clientName);
Boolean SYS_RegisterGetClientName(UInt8 inClientID, char* clientName, UInt8 clientBufferLen);
Boolean SYS_FindClient(char* clientName, UInt8 *clientList, UInt8 inClientListSize, UInt8* outClientListSize);

Boolean SYS_RegisterForRemoteClient(CallbackFunc_t* callback);

void SYS_DefaultRemoteHandler(InterTaskMsg_t* inMsg);


#ifdef VMF_INCLUDE

//***************************************************************************************
/**
    Function to Activate VMF Based on SimId
	@param		inClientInfoPtr (in) Client Info 
	@return		The VMF context ID of the calling task before this function is called

	@note		The returned VMF context ID shall be passed to VccApi_ResetVmfCtx() function
				to restore the calling task VMF context.
**/
UInt8 VccApi_ActivateVmfCtx(ClientInfo_t* inClientInfoPtr, const char* fileName, UInt32 lineNo);

#define VccApi_ActivateVmf(clientInfoPtr) UInt8 orig_ctx_id = VccApi_ActivateVmfCtx(clientInfoPtr, __FILE__, __LINE__)


//***************************************************************************************
/**
    Function to Reset VMF Based on passed VM Id.
	@param		inClientInfoPtr (in) Client Info 
	@param		vmId (in) VM ID to switch the calling task to
	@return		None

	@note
**/
void VccApi_ResetVmfCtx(ClientInfo_t* inClientInfoPtr, UInt8 origVmId);

#define VccApi_ResetVmf(clientInfoPtr) VccApi_ResetVmfCtx(clientInfoPtr, orig_ctx_id);

//***************************************************************************************
/**
    Function to Init SIM-Id in clientInfo based on global vm-id
	@param		inClientInfoPtr (in) Client Info 
	@return		None

	@note
**/
void VccApi_InitSimId(ClientInfo_t* inClientInfoPtr);

#else

#define VccApi_ActivateVmf(p)
#define VccApi_ResetVmf(p)
#define VccApi_InitSimId(p)

#endif


/** @endcond */

#endif  // _TASKMSGS_H_


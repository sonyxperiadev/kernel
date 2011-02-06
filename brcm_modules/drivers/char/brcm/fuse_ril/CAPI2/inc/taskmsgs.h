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
   
#include "mobcom_types.h"

//-------------------------------------------------
// Constant Definitions
//-------------------------------------------------
#define CLIENT_INDEX_WILD_CARD				0xFF	///< Client Index Wild Card

#define  MAX_CLIENT_NUM						10		///< 5 tasks in platform (atc, bluetooth, dlink, stk ds, test MMI) using
													///< SYS_RegisterForMSEvent(), leaving 5 more clients for MMI/application use

//--- Internal/Reserved Client ID (1-63) ----------
#define	INVALID_CLIENT_ID					 0
#define	STK_CLIENT_ID  						 1	///< STK Client Identifier
#define	MNCC_CLIENT_ID  					 2	///< MNCC Client Identifier

#define	CLIENT_ID_V24_0						10
#define	CLIENT_ID_V24_1						11
#define	CLIENT_ID_V24_2						12
#define	CLIENT_ID_V24_3						13
#define	CLIENT_ID_V24_4						14
#define DEFAULT_CLIENT_ID					15  ///< Internal default Client Identifier

//--- External Client Identifiers (64-on) ---------
#define	FIRST_USER_CLIENT_ID				64


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

typedef enum{

	// InterTaskMsg group code definitions below must follow these rules:
	//
	// 1. INTER_TASK_MSG_GROUP_BEGIN is the first code
	// 2. INTER_TASK_MSG_GROUP_END is the last code
	// 3. Every definition in between must match regular expression "^\s*MSG_GRP_(\w+)\s+=\s+0x(\x\x00)\s*,"
	//    i.e., every message group's name starts with "MSG_GRP_", and its code is a multiple of 0x100.
	// 4. No message group code should be changed. Obsolete message groups should hold their codes as long as possible
	//
	// Following above rules allows auto-generation of decoding library with backward compatibility.
	

	INTER_TASK_MSG_GROUP_BEGIN	= 0,	// This line must be here in order to generate grouped binary inter-task message logging

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
			
	MSG_GRP_INT_UTIL     		= 0x2000,
	MSG_GRP_INT_ATC 			= 0x2100,
	MSG_GRP_INT_SMS_SS			= 0x2200,
	MSG_GRP_INT_NET				= 0x2300,
	MSG_GRP_INT_DC				= 0x2400,
	MSG_GRP_INT_SIM				= 0x2500,
	MSG_GRP_INT_STK				= 0x2600,
		
	MSG_GRP_CAPI2_LCS   		= 0x3200,
	MSG_GRP_CAPI2_SMS   		= 0x3300,
	MSG_GRP_CAPI2_PBK			= 0x3400,
	MSG_GRP_CAPI2_STK			= 0x3500,
	MSG_GRP_CAPI2_CC_RESP 		= 0x3600,
	MSG_GRP_CAPI2_CC_REQ  		= 0x3700,
	MSG_GRP_CAPI2_INT_PBK 		= 0x3800,
	MSG_GRP_CAPI2_INT_SMS		= 0x3900,
	MSG_GRP_CAPI2_INT_STK       = 0x3A00,
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
#if defined(XSCRIPT_INCLUDED)
	MSG_GRP_SCRIPT				= 0x7100,
#endif //XSCRIPT_INCLUDED
	MSG_GRP_END 				= 0x7FFF,

	MSG_GRP_USER_FIRST			= 0x8000,
	MSG_GRP_USER_LAST			= 0xFE00,

	INTER_TASK_MSG_GROUP_END	= 0xFFFF	// This line must be here in order to generate grouped binary inter-task message logging
} MsgGroup_t;

//---------------------------------------------------------------
// enum
//---------------------------------------------------------------
typedef enum {	// explicitly enum for easier debugging

	/*
	  Defintions of all asynchronous events from Broadcom Platform API
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
	

	INTER_TASK_MSG_BEGIN		= 0,	// This line must be here in order to generate binary inter-task message logging

	//---------------------------------------------------------------
	// MSG_GRP_SYS, MESSAGE GROUP FOR SYSTEM (0x0000)
	//---------------------------------------------------------------
	
	/**
	This event is the response to the SYS_PowerDownReq() and confirms powering down the MS.  When this
	confirmation is received the mobile has deregistered from network and the system can now be shutdown.
	**/
	MSG_POWER_DOWN_CNF 			= MSG_GRP_SYS+0x01,	

	MSG_ROAMING_STATUS			= MSG_GRP_SYS+0x02,

	// End of MSG_GRP_SYS (0x0000)


	//---------------------------------------------------------------
	// MSG_GRP_UTIL, MESSAGE GROUP FOR GENERAL PURPOSE (0x0100)
	//---------------------------------------------------------------

	/**
		This message is used by the ms_database to broadcast a message to the
		clients registered about a change in the value of any database element.
		The contents of the message are defined in MS_LocalElemNotifyInd_t
	**/
	MSG_MS_LOCAL_ELEM_NOTIFY_IND = MSG_GRP_UTIL+0x10,	///<Payload type {::MS_LocalElemNotifyInd_t}


	// End of MSG_GRP_UTIL (0x0100)
	
	
	//---------------------------------------------------------------
	// MSG_GRP_NET, MESSAGE GROUP FOR NETWORK (0x0200)
	//---------------------------------------------------------------
	
	/**
	This event provides the network registration information of the mobile.
	For the payload contents see ::MSRegState_t.
	**/
	MSG_NETWORK_REG_STATUS		= MSG_GRP_NET+0x00,	

	/**
	This event indicates whether ciphering is enabled.  It is broadcast unsolicited
	during call setup.
	**/
	MSG_CIPHER_IND 				= MSG_GRP_NET+0x01,	///<Payload type {::Boolean}

	/**
	This event is the response to the MS_SearchAvailablePLMN and provides a list of 
	available PLMNs.   For the payload contents see ::SEARCHED_PLMN_LIST_t.
	**/
	MSG_PLMNLIST_IND 			= MSG_GRP_NET+0x02,	///<Payload type {::SEARCHED_PLMN_LIST_t}

	MSG_RX_SIGNAL_INFO_CHG_IND 	= MSG_GRP_NET+0x03,	///<Payload type {::RX_SIGNAL_INFO_CHG_t}

	/**
	This is the event indicating the received signal strength and signal quality.
	For the payload contents see ::RxSignalInfo_t.  The frequency of this message
	can be controlled by changing the threshold using the functin SYS_SetRSSIThreadhold().
	**/
	MSG_RSSI_IND 				= MSG_GRP_NET+0x04,	///<Payload type {::RxSignalInfo_t}

	/**
	This event is the response to the MS_PlmnSelect() function and indicates if the
	PLMN selection was successful.  For the payload contents see ::PCHRejectCause_t.	
	**/
	MSG_PLMN_SELECT_CNF 		= MSG_GRP_NET+0x05,	///<Payload type {::UInt16}

	/**
	Event indicating the current GSM registration state.  For the payload contents see ::MSRegState_t.	
	**/
	MSG_REG_GSM_IND				= MSG_GRP_NET+0x06,	///<Payload type {::MSRegInfo_t}

	/**
	Event indicating the current GPRS registration state.  For the payload contents see ::MSRegState_t.	
	**/
	MSG_REG_GPRS_IND			= MSG_GRP_NET+0x07,	///<Payload type {::MSRegInfo_t}

	/**
	This event reports the date and timezone information broadcast by the network. When the network
	sends a "Time Zone and Time" or "Time Zone" Information Element message to the MS, it triggers this event. 
	"Time Zone and Time" and "Time Zone" IE is a type 3 Information Element. See Section 10.5.3 of 3GPP 24.008 
	for details about timezone IE structure. For the payload contents see ::TimeZoneDate_t.
	**/
	MSG_DATE_TIMEZONE_IND		= MSG_GRP_NET+0x08,	///<Payload type {::TimeZoneDate_t}

	/**
	This reports the network name on which the mobile is currently registered. 
	For the payload contents see ::nitzNetworkName_t.
	**/	
	MSG_NETWORK_NAME_IND		= MSG_GRP_NET+0x09,	///<Payload type {::nitzNetworkName_t}
    /**
	This reports the Cell information. 
	For the payload contents see ::MMPlmnInfo_t.
	**/	
	
	MSG_CELL_INFO_IND           = MSG_GRP_NET+0x10, ///<Payload type {::MMPlmnInfo_t}   

	/**
	This is the event indicating the received serving cell params.
	For the payload contents see ::ServingCellInfo_t. 
	**/
	MSG_SERVING_CELL_INFO_IND 	= MSG_GRP_NET+0x11,	///<Payload type {::ServingCellInfo_t}

	// End of MSG_GRP_NET (0x0200) 
	
	

	//---------------------------------------------------------------
	// MSG_GRP_CC, MESSAGE GROUP FOR CALL CONTROL (0x0300)
	//---------------------------------------------------------------

	/**
	This is the event indicating a incoming voice call or data call.This message is part of call status messages.
	For the payload contents see ::CallReceiveMsg_t.
	**/
	MSG_INCOMING_CALL_IND		= MSG_GRP_CC+0x00,		///<Payload type {::CallReceiveMsg_t}

	/**
	This event indicates that a voice call has connected and is active. For the payload contents see ::VoiceCallConnectMsg_t.
	**/
	MSG_VOICECALL_CONNECTED_IND	= MSG_GRP_CC+0x01,		///<Payload type {::VoiceCallConnectMsg_t}

	/**
	This event indicates that a voice call is waiting. For the payload contents see ::VoiceCallWaitingMsg_t.
	**/
	MSG_VOICECALL_WAITING_IND	= MSG_GRP_CC+0x02,	///<Payload type {::VoiceCallWaitingMsg_t}

	/**
	This message is sent to the client in response to call dependent supplementary service (i.e. ECT, Multi-Party call etc.) . For the payload contents see ::VoiceCallActionMsg_t.
	**/
	MSG_VOICECALL_ACTION_RSP	= MSG_GRP_CC+0x03,	///<Payload type {::VoiceCallActionMsg_t}

	/**
	This event indicates to the upper layer to informe the start of the DTMF tone if there is need. For the payload contents see ::VoiceCallPreConnectMsg_t.
	**/
	MSG_VOICECALL_PRECONNECT_IND= MSG_GRP_CC+0x04,	///<Payload type {::VoiceCallPreConnectMsg_t}

	/** 
	This event indicates that a voice call has been released locally and a release request has been sent
	to the network. See ::Cause_t for list of release causes.
	**/
	MSG_VOICECALL_RELEASE_IND	= MSG_GRP_CC+0x05,	///<Payload type {::VoiceCallReleaseMsg_t}

	/** 
	This event indicates that network has confirmed the release of a voice call. See ::Cause_t for list of release causes.
	**/
	MSG_VOICECALL_RELEASE_CNF	= MSG_GRP_CC+0x06,	///<Payload type {::VoiceCallReleaseMsg_t}

	/**
	Event to provide status on the current call.See ::CallStatusMsg_t for a list of possibel call status.
	**/
	MSG_CALL_STATUS_IND			= MSG_GRP_CC+0x10,	///<Payload type {::CallStatusMsg_t}

	/**
	This is the event that provides information about the connected line identification. 
	For payload contents see ::COLPPresentStatus_t
	**/
	MSG_CALL_CONNECTEDLINEID_IND= MSG_GRP_CC+0x11,	///<Payload type {::UInt8}

	/**
	This event is used to send an indication to the API client for either a successful or
	an unsuccessful DTMF tone generation. See ApiDtmfStatus_t;
	**/
	MSG_DTMF_STATUS_IND			= MSG_GRP_CC+0x12,	///<Payload type {::ApiDtmfStatus_t}

    /**
	This event is used to send an indication to the API client when the network sends the
	User Information message to the mobile station to deliver information transferred from
	the remote user.This message is used if the user-to-user transfer is part of an allowed
	information transfer as defined in 3GPP TS 24.010.
	See UserInfo_t;
	**/
	MSG_USER_INFORMATION		= MSG_GRP_CC+0x13,	///<Payload type {::SS_UserInfo_t}

	/**
	This event is used to send the status of a Data call to the API client.
	**/
	MSG_DATACALL_STATUS_IND		= MSG_GRP_CC+0x20,	///<Payload type {::DataCallStatusMsg_t}

	/**
	This event indicates that a data call has been connected. This message is sent before calling CC_ReportDataCallConnect()
	For payload contents see ::DataCallConnectMsg_t
	**/
	MSG_DATACALL_CONNECTED_IND	= MSG_GRP_CC+0x21,	///<Payload type {::DataCallConnectMsg_t}

	/**
	This is the event indicating that a data call has been released. See ::Cause_t for list of release causes.
	**/
	MSG_DATACALL_RELEASE_IND	= MSG_GRP_CC+0x22,	///<Payload type {::DataCallReleaseMsg_t}

	/**
	This is the event indicating the result of the make link request message. This message will indicate if the Error Correction Data Compression
	module has been properly activated and if a end to end data connection pipe is available.
	**/

	MSG_DATACALL_ECDC_IND		= MSG_GRP_CC+0x23,	///<Payload type {::DataECDCLinkMsg_t}

	/**
	This is the event confirming that a data call has been released. See ::Cause_t for list of release causes.
	**/
	MSG_DATACALL_RELEASE_CNF	= MSG_GRP_CC+0x24,	///<Payload type {::DataCallReleaseMsg_t}

	/**
	This is the event indicating to all of the clients for an API call from one of the
	the client. For instance in terms of incoming call indication, if one of the clients,
	either accept or reject the incoming call this messag informs other clients, which
	client has responded the incoming call and weather or not the response been successful.
	This message is part of response call status messages.
	For the payload contents see ::ApiClientCmdInd_t.
	**/
	MSG_API_CLIENT_CMD_IND		= MSG_GRP_CC+0x30,	///<Payload type {::ApiClientCmdInd_t}


	/**
	This is the event indicating call ACM value for a call.For the payload contents see ::CallCCMMsg_t
	**/
	MSG_CALL_CCM_IND			= MSG_GRP_CC+0x50,	///<Payload type {::CallCCMMsg_t}

	/**
	This is the message indicating the status of the Advice of Charge feature. See ::AoCStatus_t for status of 
	advice of charge.
	**/
	MSG_CALL_AOCSTATUS_IND		= MSG_GRP_CC+0x51,	///<Payload type {::CallAOCStatusMsg_t}

	// End of MSG_GRP_CC (0x0300)


	
	//---------------------------------------------------------------
	// MSG_GRP_SMS, MESSAGE GROUP FOR SMS (0x0400)
	//---------------------------------------------------------------
	
	/**
	This is the event indicating an incoming point-to-point SMS message. ::MSG_SMSPP_DELIVER_IND and ::MSG_SMSPP_STORED_IND are a pair of
	msgs that indicate new incoming SMS. Depending on the setting of SMS_GetNewMsgDisplayPref() ,only one msg is sent to client(S).
	For the payload contents see ::SmsSimMsg_t.
	The entire message is delivered, no copy is saved to storage.
	The PDU field in the SmsSimMsg_t struct contains the entire sms data in GSM3.40
	format, while the other fields are decoded data.
	**/

	MSG_SMSPP_DELIVER_IND		= MSG_GRP_SMS+0x00,	///<Payload type {::SmsSimMsg_t}

	/**
	This is the event indicating an incoming SMS message that has been (or has not been)
	stored in a storage. If the result is ::SIMACCESS_SUCCESS (not necessary a SIM storage), the rec_no
	gives the location of the new message. If the result is not ::SIMACCESS_SUCCESS,
	then that means a new incoming message cannot be saved to SIM storage (ME storage
	is attempt first, so when the result is failed, both storages are full).
	For the payload contents see ::SmsIncMsgStoredResult_t.
	**/
	MSG_SMSPP_STORED_IND		= MSG_GRP_SMS+0x01,	///<Payload type {::SmsIncMsgStoredResult_t}

	/**
	This is the event indicating an incoming SMS cell broadcast message.
	The entire message is delivered, no copy is saved to storage. For the payload contents
	see ::SmsStoredSmsCb_t.
	**/

	MSG_SMSCB_DATA_IND			= MSG_GRP_SMS+0x02,	///<Payload type {::SmsStoredSmsCb_t}

	/**
	This is the event indicating an incoming Cell Broadcast message that has been (or has
	not been) stored in a storage. If the result is ::SIMACCESS_SUCCESS(not necessary a SIM storage),
	the rec_no gives the location of the new message. If the result is not ::SIMACCESS_SUCCESS,	
	then that means a new incoming message cannot be saved to SIM storage (ME storage
	is attempt first, so when the result is failed, both storages are full).
	For the payload contents see ::SmsIncMsgStoredResult_t. 
	**/

	MSG_SMSCB_STORED_IND		= MSG_GRP_SMS+0x03,	///<Payload type {::SmsIncMsgStoredResult_t}

	/**
	This is the event indicating an incoming SMS status report message.
	@note The entire message is delivered, no copy is saved to storage.
	::MSG_SMSSR_REPORT_IND and ::MSG_SMSSR_STORED_IND are a pair of
	msgs that indicate new CB SMS. Depend on the setting of
	SMS_GetNewMsgDisplayPref() , only one msg is sent to client(S).
	For the payload contents see ::SmsSimMsg_t
	**/

	MSG_SMSSR_REPORT_IND		= MSG_GRP_SMS+0x04,	///<Payload type {::SmsSimMsg_t}

	/**
	This is the event indicating an incoming Status Report message that has been (or has not
	been) stored in a storage. If the result is ::SIMACCESS_SUCCESS (not necessary a SIM storage)
	the rec_no gives the location of the new message. If the result is not ::SIMACCESS_SUCCESS,
	then that means a new incoming message cannot be saved to SIM storage (ME storage
	is attempt first, so when the result is failed, both storages are full).
	For the payload contents see ::SmsIncMsgStoredResult_t.
	**/
	MSG_SMSSR_STORED_IND		= MSG_GRP_SMS+0x05,	///<Payload type {::SmsIncMsgStoredResult_t}

	/**
	Network ack/nack to a PDU submitted to the network will be returned 
	in this intertask message. For the payload contents see ::SmsSubmitRspMsg_t.
	**/

	MSG_SMS_SUBMIT_RSP			= MSG_GRP_SMS+0x06,	///<Payload type {::SmsSubmitRspMsg_t}


	MSG_SMS_COMMAND_RSP			= MSG_GRP_SMS+0x07,	///<Payload type {::T_MN_TP_SMS_RSP}

	/**
	The response to a Cell Broadcast message request is returned in this task message.
	For the payload contents see ::SmsCBMsgRspType_t.
	START_REJ, START_CNF, STOP_REJ, STOP_CNF
	**/

	MSG_SMS_CB_START_STOP_RSP	= MSG_GRP_SMS+0x08,  ///<Payload type {::SmsCBMsgRspType_t}

	/**
	The response to a read Cell Broadcast message is returned in this inter task message.
	For the payload contents see ::SIM_SMS_DATA_t.
	**/

	MSG_SMSCB_READ_RSP			= MSG_GRP_SMS+0x09,	///<Payload type {::SIM_SMS_DATA_t}

	/**
	This is the event indicating an SMS message has been (or has not been) written to a
	storage. If the result is ::SIMACCESS_SUCCESS (not necessary a SIM storage), the rec_no	
	gives the location of the message. If the result is not ::SIMACCESS_SUCCESS, then
	that means the message cannot be saved to SIM storage (ME storage is attempt first, so
	when the result is failed, both storages are full).	For the payload contents see ::SmsIncMsgStoredResult_t.
	**/

	MSG_SMS_WRITE_RSP_IND		= MSG_GRP_SMS+0x10,	///<Payload type {::SmsIncMsgStoredResult_t}

	/**
	This is the event indicating the HomeZone/CityZone status to be displayed to user so that he/she knows
	whether he is in an area in which calls can be made in lower-than-usual tariff. This message is sent out
	only if HomeZone/CityZone algorithm is active. The algorithm is active if all the following conditions 
	are met: German O2 SIM is inserted (MCC/MNC is 262/07 or 262/08); HomeZone/CityZone status is activated
	in EF-HZ or EF-UHZIUE SIM/USIM files. 
	**/
	MSG_HOMEZONE_STATUS_IND		= MSG_GRP_SMS+0x11,	///<Payload type {::HomezoneIndData_t}

	/**
	This is the event indicating a voicemail waiting.
	For the payload contents see ::SmsVoicemailInd_t.
	**/
	MSG_VM_WAITING_IND			= MSG_GRP_SMS+0x20,	///<Payload type {::SmsVoicemailInd_t}

	/**
	Voice Mail update status is returned in this intertask message.
	**/

	MSG_VMSC_UPDATE_RSP			= MSG_GRP_SMS+0x21,	///<Payload type {::SIM_EFILE_UPDATE_RESULT_t}

	/**
	This is the event indicating SMS module is ready.
	Access to VM Ind, VMsc, SIM SMS capacity Exceed flag should be done after received
	this message.For the payload contents see ::smsModuleReady_t.
	**/

	MSG_SMS_READY_IND			= MSG_GRP_SMS+0x30,	///<Payload type {::smsModuleReady_t}

	MSG_SMSPP_OTA_IND			= MSG_GRP_SMS+0x40,	///<Payload type {::SmsSimMsg_t}
	MSG_SMSPP_REGULAR_PUSH_IND	= MSG_GRP_SMS+0x41,	///<Payload type {::SmsSimMsg_t}
	MSG_SMSPP_SECURE_PUSH_IND	= MSG_GRP_SMS+0x42,	///<Payload type {::SmsSimMsg_t}
	
	/**
	This is the event indicating an application specific SMS is received. This SMS is not stored. 
	For the payload contents see ::SmsAppSpecificData_t.	
	**/
	MSG_SMSPP_APP_SPECIFIC_SMS_IND= MSG_GRP_SMS+0x43,	///<Payload type {::SmsAppSpecificData_t}
	MSG_SMSPP_OTA_SYNC_IND		= MSG_GRP_SMS+0x44,	///<Payload type {::SmsSimMsg_t}

	// End of MSG_GRP_SMS, (0x0400)

	
	
	//---------------------------------------------------------------
	// MSG_GRP_PBK, MESSAGE GROUP FOR PHONE BOOK ACCESS (0x0500)
	//---------------------------------------------------------------
	
	/**
	This is a message to build the phonebook tables. It is sent via the PBK_BuildPbkReq() function call.
	Build Phonebook table request 
	**/
	MSG_BUILD_PBK_REQ			= MSG_GRP_PBK+0x00,	///<Payload type {::MsgType_t}	

	/**
	This is a message to rebuild the SIM phonebook tables if the phonebooks is ready. It is sent via the PBK_RebuildADNPhonebk() function call.
	Rebuild ADN phonebook (SIM phonebook)
	**/
	MSG_REBUILD_ADN_PBK_REQ		= MSG_GRP_PBK+0x01,	

	/**
	This is a request message to find all phonebook entries that match an alpha pattern.The phonebook module will search all the entries of the
	passed phonebook. All entries that match the pattern will be returned in the ::MSG_PBK_ENTRY_DATA_RSP messages 
	(each entry returned in one ::MSG_PBK_ENTRY_DATA_RSP message)
	Request to find all phonebook entries that match an alpha pattern
	**/
	MSG_FIND_PBK_ALPHA_MUL_REQ	= MSG_GRP_PBK+0x02,	///<Payload type {::PBK_FIND_ALPHA_MUL_REQ_t}

	/**
	This is a message to find the first phonebook entry that matches an alpha pattern.The phonebook module will search the phonebook type list, starting
	with the first phonebook in the list, until the last phonebook is searched or an entry that matches the alpha pattern is found.
	The first matching entry is returned in the ::MSG_PBK_ENTRY_DATA_RSP message.
	Request to find the first phonebook entry that matches an alpha pattern in multiple phonebooks
	**/
	MSG_FIND_PBK_ALPHA_ONE_REQ	= MSG_GRP_PBK+0x03,	///<Payload type {::PBK_FIND_ALPHA_ONE_REQ_t}

	/**
	This message is used to send a request to the phonebook task to request specific information such as 
	number of entries. For the payload contents see ::PBK_INFO_REQ_t
	Request to get phonebook information **/
	MSG_GET_PBK_INFO_REQ		= MSG_GRP_PBK+0x04,	///<Payload type {::PBK_INFO_REQ_t}

	/**
	This message contains basic information of a phonebook type.For the payload contents see ::PBK_INFO_RSP_t.
	**/
	/* Response to get phonebook information */
	MSG_GET_PBK_INFO_RSP		= MSG_GRP_PBK+0x05,	///<Payload type {::PBK_INFO_RSP_t}	

	/**
	This message contains the request for reading phonebook entries.For the payload contents see ::PBK_ENTRY_READ_REQ_t.
	**/
	/* Phonebook entry data request */
	MSG_READ_PBK_ENTRY_REQ		= MSG_GRP_PBK+0x06,	///<Payload type {::PBK_ENTRY_READ_REQ_t}

	/**
	This message contains the contents of a phonebook entry as well as whether this message is
	the last sent out as the response of the phonebook task.For the payload contents see ::PBK_ENTRY_DATA_RSP_t.
	Phonebook entry data response 
	**/
	MSG_PBK_ENTRY_DATA_RSP		= MSG_GRP_PBK+0x07,	///<Payload type {::PBK_ENTRY_DATA_RSP_t}

	/**
	This is the request for a phonebook entry to be written. This message is sent via a PBK_SendReadEntryReq() and 
	the response comes in ::MSG_WRT_PBK_ENTRY_RSP .For the payload contents see ::PBK_WRITE_ENTRY_REQ_t.	
	Write phonebook entry request
	**/
	MSG_WRT_PBK_ENTRY_REQ		= MSG_GRP_PBK+0x08,	///<Payload type {::PBK_WRITE_ENTRY_REQ_t}

	/**
	This message contains the phonebook write result.For the payload contents see ::PBK_WRITE_ENTRY_RSP_t.
	Write phonebook entry status response
	**/
	MSG_WRT_PBK_ENTRY_RSP		= MSG_GRP_PBK+0x09,	///<Payload type {::MsgType_t}

	/**
	This message requests the phonebook task to initialize SIM data
	**/
	MSG_PBK_INIT_SIM_DATA_REQ	= MSG_GRP_PBK+0x0A,	///<No Payload 	
	
													/**
	This is broadcast to indicate a phonebook entry has been successfully written. This indication is in addition to the 
	::MSG_WRT_PBK_ENTRY_RSP response that is sent back for a phonebook write request.For the payload contents see ::PBK_CHK_NUM_DIALLABLE_REQ_t
	Write phonebook entry indication
	**/
	MSG_WRT_PBK_ENTRY_IND		= MSG_GRP_PBK+0x20,	///<Payload type {::PBK_WRITE_ENTRY_IND_t} 
	
	/**
	This message is the request to check if the passed number can be dialed according to the FDN settings. The response to this
	message is provided via ::MSG_CHK_PBK_DIALLED_NUM_RSP. For the payload contents see ::PBK_CHK_NUM_DIALLABLE_REQ_t.
	Request to check whether number is diallable according FDN setting
	**/
	MSG_CHK_PBK_DIALLED_NUM_REQ	= MSG_GRP_PBK+0x21,	///<Payload type {::PBK_CHK_NUM_DIALLABLE_REQ_t}

	/**
	This message contains the phone number passed in the ::MSG_CHK_PBK_DIALLED_NUM_REQ message 
	and whether the number can be dialed according to FDN setting. For the payload contents see ::PBK_CHK_NUM_DIALLABLE_RSP_t. 
	Response for checking whether the number is diallable according FDN setting
	**/
	MSG_CHK_PBK_DIALLED_NUM_RSP	= MSG_GRP_PBK+0x22,	///<Payload type {::PBK_CHK_NUM_DIALLABLE_RSP_t}

	/**
	 This message is for phonebook ready status 
	 **/
	MSG_PBK_READY_IND			= MSG_GRP_PBK+0x23,


	/** 
	 Get the information for USIM PBK EF-AAS and EF-GAS 
	 **/
	MSG_GET_USIM_PBK_ALPHA_INFO_REQ		= MSG_GRP_PBK+0x24,	///<Payload type {::USIM_PBK_ALPHA_INFO_REQ_t}

	/**
	 Response for MSG_GET_USIM_PBK_ALPHA_INFO_REQ 
	 **/
	
	MSG_GET_USIM_PBK_ALPHA_INFO_RSP		= MSG_GRP_PBK+0x25,	///<Payload type {::USIM_PBK_ALPHA_INFO_RSP_t}	

	/**
	 Read an entry in USIM PBK EF-AAS and EF-GAS
	 **/
	MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ	= MSG_GRP_PBK+0x26,	///<Payload type {::USIM_PBK_READ_ALPHA_ENTRY_REQ_t}

	/**
	 Response for MSG_READ_USIM_PBK_ALPHA_ENTRY_REQ
	 **/
	MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP	= MSG_GRP_PBK+0x27,	///<Payload type {::USIM_PBK_READ_ALPHA_ENTRY_RSP_t}

	/**
	 Write an entry in USIM PBK EF-AAS and EF-GAS
	 **/
	MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ	= MSG_GRP_PBK+0x28,	///<Payload type {::USIM_PBK_UPDATE_ALPHA_ENTRY_REQ_t}

	/**
	 Response for MSG_WRITE_USIM_PBK_ALPHA_ENTRY_REQ
	 **/
	MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP	= MSG_GRP_PBK+0x29,	///<Payload type {::USIM_PBK_UPDATE_ALPHA_ENTRY_RSP_t}

	/**
	 Read Hidden key entry 
	 **/
	
	MSG_READ_USIM_PBK_HDK_ENTRY_REQ	    = MSG_GRP_PBK+0x2A,  ///<Payload type {::USIM_PBK_READ_HDK_ENTRY_REQ_t}
	
	/**
	 Read Hidden key entry 
	 **/
	MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ    = MSG_GRP_PBK+0x2B,  ///<Payload type {::USIM_PBK_UPDATE_HDK_ENTRY_REQ_t}
	
	/**
	 Response for MSG_READ_USIM_PBK_HDK_ENTRY_REQ
	 **/
	MSG_READ_USIM_PBK_HDK_ENTRY_RSP     = MSG_GRP_PBK+0x2C,  ///<Payload type {::USIM_PBK_READ_HDK_ENTRY_RSP_t}

	/**
	 Response for MSG_WRITE_USIM_PBK_HDK_ENTRY_REQ 
	 **/
	MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP	= MSG_GRP_PBK+0x2d,	 ///<Payload type {::USIM_PBK_UPDATE_HDK_ENTRY_RSP_t}

	// End of MSG_GRP_PBK, (0x500)



	
	//---------------------------------------------------------------
	// MSG_GRP_DC, MESSAGE GROUP FOR DATA CONNECTION (0x0600)
	//---------------------------------------------------------------

	// message for network service
	MSG_PPP_OPEN_CNF					= MSG_GRP_DC+0x00,	
	MSG_PPP_CLOSE_CNF					= MSG_GRP_DC+0x01,	
	MSG_PPP_OPEN_IND					= MSG_GRP_DC+0x02,	
	MSG_PPP_CLOSE_IND					= MSG_GRP_DC+0x03,	
			
	// for DC broadcasting      		
	MSG_DC_REPORT_CALL_STATUS			= MSG_GRP_DC+0x10,	///<Payload type {::DC_ReportCallStatus_t}
	MSG_DC_IR_SHUTDOWN_CONNECTION_REQ	= MSG_GRP_DC+0x11,	///<Payload type {::DC_IR_ShutdownDataConnection_t}
	MSG_DC_IR_STARTUP_CONNECTION_REQ	= MSG_GRP_DC+0x12,	///<Payload type {::DC_IR_StartupDataConnection_t}
		
	MSG_GPRS_ACTIVATE_IND				= MSG_GRP_DC+0x20,	///<Payload type {::GPRSActInd_t}
	MSG_GPRS_DEACTIVATE_IND				= MSG_GRP_DC+0x21,	///<Payload type {::GPRSDeactInd_t}
	MSG_GPRS_MODIFY_IND					= MSG_GRP_DC+0x22,	///<Payload type {::Inter_ModifyContextInd_t}
	MSG_GPRS_REACT_IND					= MSG_GRP_DC+0x23,	///<Payload type {::GPRSReActInd_t}
		
	MSG_PDP_SEC_ACTIVATION_RSP			= MSG_GRP_DC+0x30,	///<Payload type {::PDP_SendPDPActivateSecReq_Rsp_t}
	MSG_PDP_ACTIVATION_RSP				= MSG_GRP_DC+0x31,	///<Payload type {::PDP_SendPDPActivateReq_Rsp_t}
	MSG_PDP_DEACTIVATION_RSP			= MSG_GRP_DC+0x32,	///<Payload type {::PDP_SendPDPDeactivateReq_Rsp_t}
	MSG_PDP_DEACTIVATION_IND			= MSG_GRP_DC+0x33,	///<Payload type {::PDP_PDPDeactivate_Ind_t}
	MSG_PDP_ACTIVATE_SNDCP_RSP 			= MSG_GRP_DC+0x34,	///<Payload type {::PDP_DataState_t}
	MSG_PDP_MODIFICATION_RSP			= MSG_GRP_DC+0x35,	///<Payload type {::PDP_SendPDPModifyReq_Rsp_t}
	MSG_PDP_PPP_SENDCLOSE_IND			= MSG_GRP_DC+0x36,	///<Payload type {::PDP_SendPDPModifyReq_Rsp_t}

	MSG_SNPDU_IND						= MSG_GRP_DC+0x50,	


	// End of MSG_GRP_DC, (0x0600)



	
	//---------------------------------------------------------------
	// MSG_GRP_DEV, MESSAGE GROUP FOR DEVICES (0x0700)
	//---------------------------------------------------------------

	/** This message is broadcast by the filesystem when volume utilization exceeds a threshold
	**/
	MSG_FS_VOLUME_USE_IND       		= MSG_GRP_DEV+0x01,	///<Payload type {::FsVolumeUseInd_t}

	/** This message is broadcast by the filesystem when removable media is inserted/removed
	**/
	MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT = MSG_GRP_DEV+0x02,	///<Payload type {::FS_RemovableMediaInsertEvent_t}

    //USB Messages
    MSG_USB_MSC_ACTIVE                  = MSG_GRP_DEV+0x20,	/*An internal message to notify the USB msc active*/
    MSG_USB_MSC_DEACTIVE                = MSG_GRP_DEV+0x21,	/*An internal message to notify the USB msc deactive*/ 
    MSG_USB_ADAPTER_IND                 = MSG_GRP_DEV+0x22,	
    MSG_USB_CABLE_PLUGGED_IN        = MSG_GRP_DEV+0x23, /* USB cable plugged in to host */
    MSG_USB_CABLE_PLUGGED_OUT     = MSG_GRP_DEV+0x24, /* USB cable plugged out from host */


	// End of MSG_GRP_DEV, (0x0700)

	
	//---------------------------------------------------------------
	// MSG_GRP_STK, MESSAGE GROUP FOR SIM TOOLKIT API (0x0800)
	//---------------------------------------------------------------

	// message for SATK (Sim Toolkit API) 
    /**
    This SATK message is used to sent a text with a maximim length of 160 characters to client.
	For payload content, see ::DisplayText_t
    **/
	MSG_SATK_EVENT_DISPLAY_TEXT			= MSG_GRP_STK+0x00,	///<Payload type {::SATK_EventData_t}
    /**
    This SATK message is used for GET_INKEY command.  For payload content, see ::GetInkey_t
    **/
	MSG_SATK_EVENT_GET_INKEY			= MSG_GRP_STK+0x01,	///<Payload type {::SATK_EventData_t}
    /**
    This SATK message is used for GET_INPUT command.  For payload content, see ::GetInput_t
    **/
	MSG_SATK_EVENT_GET_INPUT			= MSG_GRP_STK+0x02,	///<Payload type {::SATK_EventData_t}
    /**
    This SATK message is used for PLAY_TONE command.  For payload content, see ::PlayTone_t
    **/
	MSG_SATK_EVENT_PLAY_TONE			= MSG_GRP_STK+0x03,	///<Payload type {::SATK_EventData_t}
  	/**
  	This SATK message is used to request client to display different items where the user can select one.
	For payload content, see ::SelectItem_t
  	**/
	MSG_SATK_EVENT_SELECT_ITEM			= MSG_GRP_STK+0x04,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to request seding a SS String (originated from SIM).
	For payload content, see ::SendSs_t
  	**/
	MSG_SATK_EVENT_SEND_SS				= MSG_GRP_STK+0x05,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to request seding a USSD String (originated from SIM).
	For payload content, see ::SendUssd_t
  	**/
	MSG_SATK_EVENT_SEND_USSD			= MSG_GRP_STK+0x06,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to request setting up a call (originated from SIM).
	For payload content, see ::SetupCall_t
  	**/
	MSG_SATK_EVENT_SETUP_CALL			= MSG_GRP_STK+0x07,	///<Payload type {::SATK_EventData_t}
  	/**
   	This SATK message is used to request client to display different menu items where the user can select one.
	For payload content, see ::SetupMenu_t
  	**/
	MSG_SATK_EVENT_SETUP_MENU			= MSG_GRP_STK+0x08,	///<Payload type {::SATK_EventData_t}
	MSG_SATK_EVENT_MENU_SELECTION		= MSG_GRP_STK+0x09,	
  	/**
	This SATK message is used to request sending a DTMF.
	For payload content, see ::SIMParmSendDtmfReq_t
  	**/
	MSG_SATK_EVENT_SEND_DTMF			= MSG_GRP_STK+0x0A,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to request client to do a refresh.
	For payload content, see ::Refresh_t
  	**/
	MSG_SATK_EVENT_REFRESH				= MSG_GRP_STK+0x10,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to request sending a MO SMS (originated from SIM).
	For payload content, see ::SendMOSMS_t
  	**/
	MSG_SATK_EVENT_SEND_SHORT_MSG		= MSG_GRP_STK+0x11,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to indicate ending a STK session.
  	**/
	MSG_SATK_EVENT_STK_SESSION_END		= MSG_GRP_STK+0x12,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to indicate response of a call control request.
  	**/
	MSG_SATK_EVENT_CALL_CONTROL_RSP		= MSG_GRP_STK+0x13,	
  	/**
	This SATK message is used to request to launch a browser (originated from SIM).
	For payload content, see ::STKLaunchBrowserReq_t
  	**/
	MSG_SATK_EVENT_LAUNCH_BROWSER		= MSG_GRP_STK+0x14,	///<Payload type {::SATK_EventData_t}
    /**
    This SATK message is used to sent a text with a maximim length of 160 characters to client to be displayed in idle mode.
	For payload content, see ::IdleModeText_t	
    **/
	MSG_SATK_EVENT_IDLEMODE_TEXT		= MSG_GRP_STK+0x15,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message is used to request Language info from SIM
  	**/
	MSG_SATK_EVENT_PROV_LOCAL_LANG		= MSG_GRP_STK+0x16,	///<Payload type {::SATK_EventData_t}
  	/**
	This SATK message for data service: Open chan, Send data, Receive data, Close chan
  	**/
	MSG_SATK_EVENT_DATA_SERVICE_REQ		= MSG_GRP_STK+0x17,	///<Payload type {::StkDataService_t}
	/**
    This STK message is used to request the activation of UICC interface, e.g. the 
	UICC-CLF interface defined in section 8.89 in ETSI 102 223.
	For payload content: UInt8 which defines the interface to be activated according to 
						 section 8.89 in ETSI 102 223
    **/
	MSG_SATK_EVENT_ACTIVATE			= MSG_GRP_STK+0x18,	///<Payload type {::SATK_EventData_t}

	// message for SATK Call Control
	MSG_SATKCC_USSD_CC_NOT_ALLOWED		= MSG_GRP_STK+0x50,	
	MSG_SATKCC_USSD_CC_INIT_IND			= MSG_GRP_STK+0x51,	
	MSG_SATKCC_USSD_CC_ALLOWED			= MSG_GRP_STK+0x52,	
	MSG_SATKCC_USSD_CC_MODIFIED			= MSG_GRP_STK+0x53,	
	
	MSG_SATKCC_SMS_CC					= MSG_GRP_STK+0x54,	
	
	MSG_SATKCC_SS_CC_NOT_ALLOWED		= MSG_GRP_STK+0x55,	
	MSG_SATKCC_SS_CC_INIT_IND			= MSG_GRP_STK+0x56,	
	MSG_SATKCC_SS_CC_ALLOWED			= MSG_GRP_STK+0x57,	
	MSG_SATKCC_SS_CC_MODIFIED			= MSG_GRP_STK+0x58,	

	// message for STK module
	MSG_STK_SEND_EMERGENCY_CALL_REQ		= MSG_GRP_STK+0x60,	
	MSG_STK_REPORT_CALL_RECEIVED		= MSG_GRP_STK+0x61,	
	MSG_STK_REPORT_CALL_STATUS			= MSG_GRP_STK+0x62,	
	MSG_STK_REPORT_CALL_CONNECTED_ID	= MSG_GRP_STK+0x63,	
	MSG_STK_REPORT_CALL_ACTION_RESULT	= MSG_GRP_STK+0x64,	
	MSG_STK_REPORT_CALL_RELEASE			= MSG_GRP_STK+0x66,	///<Payload type {::StkReportCallRelease_t}
	MSG_STK_REPORT_SUPP_SVC_STATUS		= MSG_GRP_STK+0x67,	///<Payload type {::StkReportSuppSvcStatus_t}
	MSG_STK_REPORT_CALL_AOC_STATUS		= MSG_GRP_STK+0x68,	
	MSG_STK_REPORT_CALL_CCM				= MSG_GRP_STK+0x69,

	// End of MSG_GRP_STK, (0x0800)


	
	//---------------------------------------------------------------
	// MSG_GRP_SS, MESSAGE GROUP FOR SUPPLEMENTARY SERVICES (0x0900)
	//---------------------------------------------------------------
	
	//Supplementary Services Intertask Message (Premitive) Identifiers
	
	//< SS call notification (except, CUG, ECT and CNAP)
	MSG_SS_CALL_NOTIFICATION			= MSG_GRP_SS+0x00,	///<Payload type {::SS_CallNotification_t}
	
	///< SS Notify CUG (Operation Closed User Group)
	MSG_SS_NOTIFY_CLOSED_USER_GROUP	    = MSG_GRP_SS+0x01,	///<Payload type {::SS_CallNotification_t}

	///< SS Notify, ECT (Extended Call Transfer)
	MSG_SS_NOTIFY_EXTENDED_CALL_TRANSFER= MSG_GRP_SS+0x03,	///<Payload type {::SS_CallNotification_t}
	
	///< SS Notify, CNAP (Calling NAme Presentation)
	MSG_SS_NOTIFY_CALLING_NAME_PRESENT  = MSG_GRP_SS+0x04,	///<Payload type {::SS_CallNotification_t}

	/** 
		This message is the response to the SS_SendCallForwardReq() request.  It contains the result
		indicating whether the "set call forward status" has been successfully accepted by the network.
		For the payload contents see ::Result_t
	**/

	MSG_SS_CALL_FORWARD_RSP				= MSG_GRP_SS+0x05,	///<Payload type {::NetworkCause_t}

	/**
		This message is the response to the SS_QueryCallForwardStatus() request.  It contains the 
		detailed call forward status information returned from the network.  For payload contents
		see ::CallForwardStatus_t.
	**/

	MSG_SS_CALL_FORWARD_STATUS_RSP		= MSG_GRP_SS+0x06,	///<Payload type {::CallForwardStatus_t}
					
	/**
		This message is the response to the SS_SendCallBarringReq() request.  It contains the result
		indicating whether the "set call barring status" request has been successfully accepted by
		the network.  For payload contents see ::Result_t.
	**/
											  
	MSG_SS_CALL_BARRING_RSP				= MSG_GRP_SS+0x07,	///<Payload type {::NetworkCause_t}
	
	/**
		This message is the response to the SS_QueryCallBarringStatus() request.  It contains
		the detailed call barring status information returned from the network.  For the
		payload contents see ::CallBarringStatus_t.
	**/
		
	MSG_SS_CALL_BARRING_STATUS_RSP		= MSG_GRP_SS+0x08,	///<Payload type {::CallBarringStatus_t}
	
	/**
		This message is the response to the SS_SendCallBarringPWDChangeReq() request.  It contains
		the result indicating whether the call barring password change request has been sucessfully
		accepted by the network.  For the payload contents see ::Result_t.
	**/

	MSG_SS_CALL_BARRING_PWD_CHANGE_RSP	= MSG_GRP_SS+0x09,	///<Payload type {::NetworkCause_t}

	/**
		This message is the response to the SS_SendCallWaitingReq() requset.  It contains the result
		indicating whether the "set call waiting status" request has been successfully accepted
		by the network.  For the payload contents see ::SS_ProvisionStatus_t.
	**/

	MSG_SS_CALLING_LINE_ID_STATUS_RSP	= MSG_GRP_SS+0x0A,	///<Payload type {::MsgType_t}
	/**
		This message is the response to the SS_SendCallWaitingReq() request.  It contains the result
		indicating whether the "set call waiting status" request has been successfully accepted
		by the network.  For the payload contents see ::Result_t.
	**/

	MSG_SS_CALL_WAITING_RSP				= MSG_GRP_SS+0x0B,	///<Payload type {::NetworkCause_t}

	/**
		This message is the response to the SS_QueryCallWaitingStatus() request.  It contains detailed
		call waiting status information returned from the network.  For the payload contents 
		see ::SS_ActivationStatus_t.
	**/

	MSG_SS_CALL_WAITING_STATUS_RSP		= MSG_GRP_SS+0x0C,	///<Payload type {::SS_ActivationStatus_t}

	/**
		This message is the response to the SS_QueryConnectedLineIDStatus() request.  It contains the results
		indicating whether the connected line ID feature is provisioned in the network.  For the playload
		contents see ::SS_ProvisionStatus_t.
	**/

	MSG_SS_CONNECTED_LINE_STATUS_RSP	= MSG_GRP_SS+0x0D,	///<Payload type {::MsgType_t}

	/**
		This message is the response to the SS_QueryCallingLineRestrictionStatus() request.  It containts
		the result indicating whether the calling line ID restriction feature is provisioned in the network.
		For the payload contents see ::SS_ProvisionStatus_t.
	**/

	MSG_SS_CALLING_LINE_RESTRICTION_STATUS_RSP	 = MSG_GRP_SS+0x0E,	///<Payload type {::MsgType_t}

	/**
		This message is the response to the SS_QueryConnectedLineRestrictionStatus() request.  It contains the result
		indicating whether the connected line ID restriction feature is provisioned in the network.  For
		the payload contents see ::SS_ProvisionStatus_t.
	**/

	MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP = MSG_GRP_SS+0x0F,	///<Payload type {::MsgType_t}

	/**
		This message is the response to the SS_QueryCallingNAmePresentStatus() request.  It contains the result
		indicating whether the connected line ID restriction feature is provisioned in the network.  For
		the payload contents see ::SS_ProvisionStatus_t.
	**/
		
	MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP 		 = MSG_GRP_SS+0x10,	///<Payload type {::MsgType_t}

	// Internal Message

	MSG_SS_INTERNAL_PARAM_SET_IND	   			= MSG_GRP_SS+0x11,	///<Payload type {::SS_IntParSetInd_t}


	// End of MSG_GRP_SS, (0x0900)



	
	//---------------------------------------------------------------
	// MSG_GRP_MNSS, MESSAGE GROUP FOR MNSS (0x0A00)
	//---------------------------------------------------------------


	/**
		This message is sent from the client to STK module in order to request
		 a SS service request. Refer to "SS_String_t".
	**/
	
	///< Client Stk SS service request
	
	MSG_CLIENT_SATK_SS_SRV_REQ			= MSG_GRP_MNSS+0x20,	///<Payload type {::SS_String_t}

	/**
		This message is sent from the client to STK module in order to request
		a USSD service request. Refer to "SS_SrvReq_t".
	**/
		
	///< Client Stk SS service request
	
	MSG_CLIENT_SATK_USSD_SRV_REQ		= MSG_GRP_MNSS+0x21,	///<Payload type {::SS_SrvReq_t}

	/**
		This message is sent from the client to the MNSS module in order request
		supplemantary service "SS_SrvReq_t".
	**/
		
	MSG_CLIENT_MNSS_SS_SRV_REQ			= MSG_GRP_MNSS+0x22,	

	/**
		This message is sent from the client to the MNSS module in order request
		supplemantary service "SS_SrvData_t".
	**/
		
	MSG_CLIENT_MNSS_SS_DATA_REQ			= MSG_GRP_MNSS+0x23,	///<Payload type {::SS_SrvReq_t}

	/**
		This message is sent from the client to the MNSS module in order request
		supplemantary service "SS_SrvData_t".
	**/
		
	MSG_CLIENT_MNSS_SS_REL_REQ			= MSG_GRP_MNSS+0x24,	///<Payload type {::SS_SrvReq_t}

	/**
		This message is sent from the MNSS module to the client in response to
		a mobile originated supplemantary service "SS_SrvRsp_t".
	**/
		
	MSG_MNSS_CLIENT_SS_SRV_RSP			= MSG_GRP_MNSS+0x25,	///<Payload type {::SS_SrvRsp_t}

	/**
		This message is sent from the MNSS module to the client in order to
		reject a mobile originated supplemantary service "SS_SrvRel_t".
	**/
		
	MSG_MNSS_CLIENT_SS_SRV_REL			= MSG_GRP_MNSS+0x26,	///<Payload type {::SS_SrvRel_t}


	/**
		This message is sent from the MNSS module to the client in response to a
		mobile terminated supplemantary service "SS_SrvInd_t".
	**/
		
	MSG_MNSS_CLIENT_SS_SRV_IND			 = MSG_GRP_MNSS+0x27,	///<Payload type {::SS_SrvInd_t}


	/**
		This message is sent from the MNSS module to the STK in response to
		a STK originated supplemantary service "STK_SsSrvRsp_t".
	**/
		
	MSG_MNSS_SATK_SS_SRV_RSP			 = MSG_GRP_MNSS+0x28,	


	/**
		This message is sent from the MNSS module to the STK in response to
		a STK originated supplemantary service "STK_SsSrvRel_t".
	**/
		
	MSG_MNSS_SATK_SS_SRV_REL			 = MSG_GRP_MNSS+0x29,	///<Payload type {::STK_SsSrvRel_t}

	/**
		This message is sent from the MNSS module in response for a
		MT-LR initiate by the network to the client. Refer to "LCS_SrvInd_t".
	**/
		
	MSG_MNSS_CLIENT_LCS_SRV_IND			 = MSG_GRP_MNSS+0x2A,	///<Payload type {::LCS_SrvInd_t}

	/**
		This message is sent by the MNSS module in order to carry network's
		MO/MT-LR response to the client. Refer to "LCS_SrvRsp_t".
	**/
		
	MSG_MNSS_CLIENT_LCS_SRV_RSP			 = MSG_GRP_MNSS+0x2B,	///<Payload type {::LCS_SrvRsp_t}

	/**
		This message is sent from the MNSS module if the network releases
		the current MT-LR session. Refer to "LCS_SrvRel_t".
	**/
		
	MSG_MNSS_CLIENT_LCS_SRV_REL			 = MSG_GRP_MNSS+0x2C,	///<Payload type {::SS_SrvRel_t}

	// End of MSG_GRP_MNSS, 0x0A00



	
	//---------------------------------------------------------------
	// MSG_GRP_USSD, MESSAGE GROUP FOR USSD (0x0B00)
	//---------------------------------------------------------------
	
	/**
		This message carries the MT USSD Data.  It is sent unsolicited when a MT USSD is received.
		For the payload contents see ::USSDataInfo_t.
		Prior to receiving this message clients will receive a ::MSG_USSD_CALLINDEX_IND
		notifying them of the call index associated with the MT USSD.
	**/
	/* USSD MT data indication */
	MSG_USSD_DATA_IND					= MSG_GRP_USSD+0x00,	///<Payload type {::USSDataInfo_t}		

	/**
		This message carries the asynchronous response to the SS_SendUSSDConnectReq() and SS_SendUSSDData()
		functions.  For the payload contents see ::USSDataInfo_t.
	**/

	/* USSD data response indication */
	MSG_USSD_DATA_RSP					= MSG_GRP_USSD+0x01,	///<Payload type {::USSDataInfo_t}		

	/**
		This message indicates the USSD session with the given index has terminated.  The payload
		contains the call index (see ::CallIndex_t).
	**/

	//USSD session ended 
	MSG_USSD_SESSION_END_IND			= MSG_GRP_USSD+0x02,	///<Payload type {::CallIndex_t}		

	/**
		This message contains the call index for the current USSD session.  In the case of
		a MT USSD this will be the first message that is broadcast.  For payload contents see
		::StkReportCallStatus_t.
	**/

	//USSD call index (platform internal use)
	MSG_USSD_CALLINDEX_IND				= MSG_GRP_USSD+0x03,	///<Payload type {::StkReportCallStatus_t}		


	// Internal message

	//USSD call release indication (platform internal)
	MSG_USSD_CALLRELEASE_IND			= MSG_GRP_USSD+0x04,	///<Payload type {::CallIndex_t}		

	// End of MSG_GRP_USSD, (0x0B00)



	
	//---------------------------------------------------------------
	// MSG_GRP_SIM, MESSAGE GROUP FOR SIM (0x0C00)
	//---------------------------------------------------------------
	
	// SIM Access Response Message
	/**
	This message contains the parameter information of the selected MF or DF.  For
	the payload contents see ::SIM_DFILE_INFO_t.
	**/
	
	//Info of a DF/MF using Generic SIM Access
	MSG_SIM_DFILE_INFO_RSP				= MSG_GRP_SIM+0x00,	///<Payload type {::SIM_DFILE_INFO_t}	

	/**
	This message contains the basic information of an EF.  For the payload contents see ::SIM_EFILE_INFO_t.
	
	**/
	
	//Info of a EF using Generic SIM Access 
	MSG_SIM_EFILE_INFO_RSP				= MSG_GRP_SIM+0x01,	///<Payload type {::SIM_EFILE_INFO_t}	

	/**
	This message contains the data contents for an EF data read request  For the payload contents see ::SIM_EFILE_DATA_t.
		
	**/
	
	//Data of a SIM file using Generic SIM Access.
	MSG_SIM_EFILE_DATA_RSP				= MSG_GRP_SIM+0x02,	///<Payload type {::SIM_EFILE_DATA_t}	

	/**
	This message contains the success/failure response for an EF content update request. 
	For the payload contents see ::SIM_EFILE_UPDATE_RESULT_t. 
	**/
	
	//Status of updating a SIM file using Generic SIM Access.
	MSG_SIM_EFILE_UPDATE_RSP			= MSG_GRP_SIM+0x03,	///<Payload type {::SIM_EFILE_UPDATE_RESULT_t}	

	/**
	This message contains the number of remaining PIN1 and PIN2 attempts in the SIM. 
	For the payload contents see ::PIN_ATTEMPT_RESULT_t. 
	**/
	
	//Remaining PIN1/PIN2/PUK1/PUK2 attempts
	MSG_SIM_PIN_ATTEMPT_RSP				= MSG_GRP_SIM+0x04,	///<Payload type {::PIN_ATTEMPT_RESULT_t}	

	
	//Read multiple records in a SIM file  
	//MSG_SIM_GEN_REC_DATA_RSP			= MSG_GRP_SIM+0x05,	///<Payload type {::SIM_GEN_REC_DATA_t}	

	
	//Search Record response
	MSG_SIM_SEEK_REC_RSP				= MSG_GRP_SIM+0x06,	///<Payload type {::SIM_SEEK_RECORD_DATA_t}	

	/**
	This message contains the success/failure response for a request to set the FDN feature on/off. 
	For the payload contents see ::SIMAccess_t.
	**/

	//Status of setting Fixed Dialing Number
	
	MSG_SIM_SET_FDN_RSP					= MSG_GRP_SIM+0x10,	///<Payload type {::SIMAccess_t}	

	/**
	This message contains the success/failure response for a request to enable the PIN1 lock.
	For the payload contents see ::SIMAccess_t.
	**/

	
	//Status of enabling/disabling PIN1 or PIN2
	MSG_SIM_ENABLE_CHV_RSP				= MSG_GRP_SIM+0x11,	///<Payload type {::SIMAccess_t}	

	/**
	This message contains the success/failure response for a request to change the PIN1/PIN2 password. 
	For the payload contents see ::SIMAccess_t.
	**/

	
	//Status of changing PIN1 or PIN2
	MSG_SIM_CHANGE_CHV_RSP				= MSG_GRP_SIM+0x12,	///<Payload type {::SIMAccess_t}	

	/**
	This message contains the success/failure response for a request to verify the PIN1/PIN2 password. 
	For the payload contents see ::SIMAccess_t.
	**/

	
	//Status of verifying PIN1 or PIN2
	MSG_SIM_VERIFY_CHV_RSP				= MSG_GRP_SIM+0x13,	///<Payload type {::SIMAccess_t}	

	/**
	This message contains the success/failure response for a request to unblock the PIN1/PIN2 lock. 
	For the payload contents see ::SIMAccess_t.
	**/

	
	//Status of unblocking PIN1 or PIN2
	MSG_SIM_UNBLOCK_CHV_RSP				= MSG_GRP_SIM+0x14,	///<Payload type {::SIMAccess_t}	

	
	//SIM phonebook information
	MSG_SIM_PBK_INFO_RSP				= MSG_GRP_SIM+0x15,	///<Payload type {::SIM_PBK_INFO_t}
	
	//SIM phonebook data in a phonebook read
	MSG_SIM_PBK_DATA_RSP				= MSG_GRP_SIM+0x16,	///<Payload type {::SIM_PBK_DATA_t}	
	
	//SIM phonebook record write
	MSG_SIM_PBK_WRITE_RSP				= MSG_GRP_SIM+0x17,	///<Payload type {::SIM_PBK_WRITE_RESULT_t}

    
	//Status of setting Barred Dialing Number
	MSG_SIM_SET_BDN_RSP					= MSG_GRP_SIM+0x18,	///<Payload type {::SIMAccess_t}

	/**
	This message contains the success/failure response for a request to add/delete an entry in
	the preferred PLMN entry table in the SIM.For the payload contents see ::SIMAccess_t.
	**/
	
	//Status of writing preferred PLMN list
	MSG_SIM_PLMN_WRITE_RSP				= MSG_GRP_SIM+0x20,	///<Payload type {::SIMAccess_t}

	/**
	This message contains number of entry information for a PLMN file in SIM. For the payload
	contents see ::SIM_PLMN_NUM_OF_ENTRY_t.
	**/
	
	//Number of PLMN entries in SIM
	MSG_SIM_PLMN_NUM_OF_ENTRY_RSP		= MSG_GRP_SIM+0x21,	///<Payload type {::SIM_PLMN_NUM_OF_ENTRY_t}

	/**
	This message contains PLMN data for a PLMN file in SIM. For the payload
	contents see ::SIM_PLMN_ENTRY_DATA_t.
	**/
	
	
	//Data of PLMN entries in SIM
	MSG_SIM_PLMN_ENTRY_DATA_RSP			= MSG_GRP_SIM+0x22,	///<Payload type {::SIM_PLMN_ENTRY_DATA_t}

	/**
	This message contains result for updating a PLMN file in SIM. For the payload
	contents see ::SIM_PLMN_ENTRY_UPDATE_t.
	**/
	
	
	//Result for updating PLMN entries in SIM
	MSG_SIM_PLMN_ENTRY_UPDATE_RSP		= MSG_GRP_SIM+0x23,	///<Payload type {::SIM_PLMN_ENTRY_UPDATE_t}

    //Result for updating PLMN entries in SIM
	MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP		= MSG_GRP_SIM+0x24,	///<Payload type {::SIM_MUL_PLMN_ENTRY_UPDATE_t}

	/**
	This message contains the maximum Accumulated Meter Value information in the SIM.
	For the payload contents see ::SIM_MAX_ACM_t.
	**/

	//Value of maximum Accumulated Call Meter value
	MSG_SIM_MAX_ACM_RSP					= MSG_GRP_SIM+0x30,	 ///<Payload type {::SIM_MAX_ACM_t}

	/**
	This message contains the Accumulated Meter Value information in the SIM.
	For the payload contents see ::SIM_ACM_VALUE_t.
	**/

	//Actual Accumulated Call Meter value
	MSG_SIM_ACM_VALUE_RSP				= MSG_GRP_SIM+0x31,	///<Payload type {::SIM_ACM_VALUE_t}

	/**
	This message contains the Accumulated Meter Value information in the SIM.
	For the payload contents see ::SIM_ACM_VALUE_t.
	**/

	
	//Status of updating acutal Accumulated Call Meter value
	MSG_SIM_ACM_UPDATE_RSP				= MSG_GRP_SIM+0x32,	///<Payload type {::SIMAccess_t}

	/**
	This message contains the success/failure response for a request to update the maximum
	Accumulated Call Meter value in the SIM.For the payload contents see ::SIMAccess_t
	**/

	
	//Status of updating acutal Accumulated Call Meter value
	MSG_SIM_ACM_MAX_UPDATE_RSP			= MSG_GRP_SIM+0x33,	///<Payload type {::MsgType_t}

	/**
	This message contains the success/failure response for a request to increase the
	Accumulated Call Meter value in the SIM. For the payload contents see ::SIMAccess_t	
	**/

	//Status of increasing actual Accumulated Call Meter value
	MSG_SIM_ACM_INCREASE_RSP			= MSG_GRP_SIM+0x34,	///<Payload type {::MsgType_t}

	/**
	This message contains the Service Provider Name string value stored in the SIM.
	For the payload contents see ::SIM_SVC_PROV_NAME_t	
	**/

	
	//Service Provider Name string
	MSG_SIM_SVC_PROV_NAME_RSP			= MSG_GRP_SIM+0x35,	///<Payload type {::SIM_SVC_PROV_NAME_t}

	/**
	This message contains the Price Per Unit information stored in the SIM.
	For the payload contents see ::SIM_PUCT_DATA_t	
	**/

	//Currency/Price Per Unit data
	MSG_SIM_PUCT_DATA_RSP				= MSG_GRP_SIM+0x40,	///<Payload type {::SIM_PUCT_DATA_t}

	/**
	This message contains the success/failure response for a request to update the Price Per
	Unit information stored in the SIM.	For the payload contents see ::SIMAccess_t	
	**/

	//Status of updating Currency/Price Per Unit
	MSG_SIM_PUCT_UPDATE_RSP				= MSG_GRP_SIM+0x41,	///<Payload type {::MsgType_t}

    /**
    This message contains the response data for Generic APDU Power On/Off (BT-SAP) requests
    **/
    MSG_SIM_POWER_ON_OFF_CARD_RSP        = MSG_GRP_SIM+0x42,	///<Payload type {::SIM_GENERIC_APDU_RES_INFO_t}

    /**
    This message contains the response data for Generic APDU ATR (BT-SAP) requests
    **/
    MSG_SIM_GET_RAW_ATR_RSP              = MSG_GRP_SIM+0x43,	///<Payload type {::SIM_GENERIC_APDU_ATR_INFO_t}

    /**
    This message contains the response data for Generic APDU Response (BT-SAP) requests
    **/
    MSG_SIM_SEND_GENERIC_APDU_CMD_RSP    = MSG_GRP_SIM+0x44,	///<Payload type {::SIM_GENERIC_APDU_XFER_RSP_t}

	/**
    This message contains the response data for Open Logical Channel request
    **/
	MSG_SIM_OPEN_SOCKET_RSP				 = MSG_GRP_SIM+0x45,	///<Payload type {::SIM_OPEN_SOCKET_RES_t}	

	/**
    This message contains the response data for Activate non-USIM Application request
    **/
	MSG_SIM_SELECT_APPLI_RSP			 = MSG_GRP_SIM+0x46,	///<Payload type {::SIM_SELECT_APPLI_RES_t} 

	/**
    This message contains the response data for Deactivate non-USIM Application request
    **/
	MSG_SIM_DEACTIVATE_APPLI_RSP		 = MSG_GRP_SIM+0x47,	///<Payload type {::SIM_DEACTIVATE_APPLI_RES_t} 

	/**
    This message contains the response data for Close Logical Channel request
    **/
	MSG_SIM_CLOSE_SOCKET_RSP			 = MSG_GRP_SIM+0x48,	///<Payload type {::SIM_CLOSE_SOCKET_RES_t}	

	/**
	This message is returned to a passed callback function used for SMS status byte searches. 
	For the payload contents see ::SIMSMSMesgStatus_t	
	**/

	//Search for SIM SMS message with a particular status
	MSG_SIM_SMS_SEARCH_STATUS_RSP		= MSG_GRP_SIM+0x50,	///<Payload type {::SIM_SEARCH_SMS_STATUS_t} 

	/**
	Response to read SMS data command .For the payload contents see ::SmsSimMsg_t 
	**/

	//SMS data contents
	MSG_SIM_SMS_DATA_RSP				= MSG_GRP_SIM+0x51,	///<Payload type {::SmsSimMsg_t}	

	//Status of SMS contents update
	MSG_SIM_SMS_WRITE_RSP				= MSG_GRP_SIM+0x52,	///<Payload type {::SIM_SMS_UPDATE_RESULT_t}


	/**
	This message is returned to a delete/ status update command. For the payload contents see ::SmsMsgDeleteResult_t		
	**/	
	//Status of updating SMS status
	MSG_SIM_SMS_STATUS_UPD_RSP			= MSG_GRP_SIM+0x53,	///<Payload type {::SIM_SMS_UPDATE_RESULT_t}	
	
	//Status of updating SMS SCA
	
	MSG_SIM_SMS_SCA_UPD_RSP				= MSG_GRP_SIM+0x54,	///<Payload type {::SIM_SMS_UPDATE_RESULT_t}	
	
	//SMS parameter data
	MSG_SIM_SMS_PARAM_DATA_RSP			= MSG_GRP_SIM+0x55,	///<Payload type {::SIM_SMS_PARAM_t}
	
	//SMS Reference Number
	MSG_SIM_SMS_TP_MR_RSP				= MSG_GRP_SIM+0x56,	///<Payload type {::SIM_SMS_TP_MR_t}

	/**
	This message contains the response to restricted SIM access request.
	For the payload contents see ::SIM_RESTRICTED_ACCESS_DATA_t	
	**/

	
	//Restricted SIM Access data
	MSG_SIM_RESTRICTED_ACCESS_RSP		= MSG_GRP_SIM+0x60,	///<Payload type {::SIM_RESTRICTED_ACCESS_DATA_t}

	/**
	This message contains the response to generic SIM access request.
	For the payload contents see ::SIM_GENERIC_ACCESS_DATA_t	
	**/

	
	//Generic SIM Access data
	MSG_SIM_GENERIC_ACCESS_RSP			= MSG_GRP_SIM+0x61,	///<Payload type {::SIM_GENERIC_ACCESS_DATA_t}

	/**
	This message contains the response for activating/deactivating service in EF-EST in USIM.
	For the payload contents see ::USIM_FILE_UPDATE_RSP_t	
	**/
	
	//Response for activating/deactivating service in EF-EST in USIM
	MSG_SIM_SET_EST_SERV_RSP			= MSG_GRP_SIM+0x62,	///<Payload type {::USIM_FILE_UPDATE_RSP_t}

	/**
	This message contains the response for updating one APN in EF-EST in USIM.
	For the payload contents see ::USIM_FILE_UPDATE_RSP_t	
	**/
	
	//Response for updating one APN in EF-ACL in USIM
	MSG_SIM_UPDATE_ONE_APN_RSP			= MSG_GRP_SIM+0x63,	///<Payload type {::USIM_FILE_UPDATE_RSP_t}
	
	/**
	This message contains the response for deleting all APN's in EF-EST in USIM.
	For the payload contents see ::USIM_FILE_UPDATE_RSP_t	
	**/
	
	//Response for deleting all APN's in EF-ACL in USIM
	MSG_SIM_DELETE_ALL_APN_RSP			= MSG_GRP_SIM+0x64,	///<Payload type {::USIM_FILE_UPDATE_RSP_t}

	/**
	This message contains the response for activating ISIM application.
	For the payload contents see ::ISIM_ACTIVATE_RSP_t	
	**/
	MSG_ISIM_ACTIVATE_RSP				= MSG_GRP_SIM+0x65,	///<Payload type {::ISIM_ACTIVATE_RSP_t}
	
	/** 
	This message contains the response for "ISIM Authenticate IMS AKA Security Context". 
	For payload contents see :: ISIM_AUTHEN_AKA_RSP_t
	**/
	MSG_ISIM_AUTHEN_AKA_RSP				= MSG_GRP_SIM+0x66,	///<Payload type {::ISIM_AUTHEN_AKA_RSP_t}

	/** 
	This message contains the response for "ISIM Authenticate HTTP Digest Security Context". 
	For payload contents see :: ISIM_AUTHEN_HTTP_RSP_t
	**/
	MSG_ISIM_AUTHEN_HTTP_RSP			= MSG_GRP_SIM+0x67,	///<Payload type {::ISIM_AUTHEN_HTTP_RSP_t}

	/** 
	This message contains the response for "ISIM Authenticate GBA Security Context (Bootstrapping Mode)". 
	For payload contents see :: ISIM_AUTHEN_GBA_BOOT_RSP_t
	**/
	MSG_ISIM_AUTHEN_GBA_BOOT_RSP		= MSG_GRP_SIM+0x68,	///<Payload type {::ISIM_AUTHEN_GBA_BOOT_RSP_t}

	/** 
	This message contains the response for "ISIM Authenticate GBA Security Context (NAF Derivation Mode)". 
	For payload contents see :: ISIM_AUTHEN_GBA_NAF_RSP_t
	**/
	MSG_ISIM_AUTHEN_GBA_NAF_RSP			= MSG_GRP_SIM+0x69,	///<Payload type {::ISIM_AUTHEN_GBA_NAF_RSP_t}

	// Unsolicited message from SIM module
	/**
	This is an unsolicited message from the SIM module indicating SIM inserton/removal
	detection. Not all hardwares support this feature.
	**/

	
	/* SIM insertion/removal indication. If SIM/USIM inserted, the message also has the language preference info in SIM/USIM */
	MSG_SIM_DETECTION_IND				= MSG_GRP_SIM+0x80,	///<Payload type {::SIM_DETECTION_t}

	/* 
	This message is for internal platform operation
	SIM reset indication posted to MSC queue 
	*/
	MSG_SIM_RESET_IND					= MSG_GRP_SIM+0x81,	///<Payload type {::MsgType_t}

	/* This message is to notify ATC/MMI the Setup Event List information regarding to MMI events */
	MSG_SIM_MMI_SETUP_EVENT_IND			= MSG_GRP_SIM+0x82,	///<Payload type {::UInt16}

	/* This message is to notify ATC/MMI fatal error status of the SIM with the following possible status:
	 *
	 * 1. The "invalid SIM" status triggered by continual SW1/SW2=0x6F/0x00 status words 
	 * response from the SIM. In AT&T 16966 UICC Application Interoperability Test Plan, the "IMEI Lock" application will 
	 * send SW1/SW2=0x6F/0x00 continually if the IMEI does not match in the terminal response for Provide Local Info (Get IMEI) 
	 * proactive command. 
	 *
     * Upon receiving this message, MMI shall immediately terminate all SIM access and display "invalid SIM" to the user. 
     * Only emergency call is allowed. 
	 *
	 * 2. IMSI is missing. MMI shall display a warning message to user, but MMI can choose to continue to access other 
	 *    SIM data such as phonebook. 
	 *
	 */
	MSG_SIM_FATAL_ERROR_IND				= MSG_GRP_SIM+0x83,	///<Payload type: none

	/** 
	This SIM message indicates all SIM cached data except SMS & PBK is ready
	**/
	MSG_SIM_CACHED_DATA_READY_IND		= MSG_GRP_SIM+0x90,

	// End of MSG_GRP_SIM (0x0C00)
	

	
	//---------------------------------------------------------------
	// MSG_GRP_LCS, MESSAGE GROUP FOR LOCATION SERVICES (0x0D00)
	//---------------------------------------------------------------

	/** @cond */
	// Message for Global Locate GPS
	MSG_GL_GPS_USERCOMMAND									= MSG_GRP_LCS+0x00,	///<Payload type {::int}
	MSG_GL_GPS_TIMEREVENT									= MSG_GRP_LCS+0x02,	
	MSG_GL_GPS_HANDLERXDATA									= MSG_GRP_LCS+0x04,	
	/** @endcond */


	//Messages from LCS task to clients.
	/// From LCS task to clients. Payload: LcsPosFixMsgData_t. When a client receives this message, it should call LCS_GetPosition to retrieve the position data.
	MSG_LCS_POSITION_FIX_READY_IND							= MSG_GRP_LCS+0x40,	
	/// From LCS task to clients. Payload: LcsState_t.
	MSG_LCS_STATUS_IND										= MSG_GRP_LCS+0x41,	///<Payload type {::LcsState_t}
	/// From LCS task to clients. Payload: LcsStartFixResult_t
	MSG_LCS_START_FIX_IND									= MSG_GRP_LCS+0x42,	///<Payload type {::LcsStartFixResult_t} 
	/// From LCS task to clients. Payload: LcsPosReqResult_t
	MSG_LCS_REQ_RESULT_IND									= MSG_GRP_LCS+0x43,	///<Payload type {::LcsPosReqResult_t} 
	/// From LCS task to clients. Payload: LcsSuplNotificationData_t
	MSG_LCS_SUPL_NOTIFICATION_IND							= MSG_GRP_LCS+0x44,	///<Payload type {::LcsSuplNotificationData_t} 
	/// From LCS task to clients. Payload: LcsNmeaData_t.
	MSG_LCS_NMEA_READY_IND									= MSG_GRP_LCS+0x45,	///<Payload type {::LcsNmeaData_t} 
	/// From LCS task to clients. Payload: LcsSuplConnection_t
	MSG_LCS_SUPL_CONNECT_REQ								= MSG_GRP_LCS+0x46,	///<Payload type {::LcsSuplConnection_t} 
	/// From LCS task to clients. Payload: LcsSuplSessionInfo_t.
	MSG_LCS_SUPL_DISCONNECT_REQ								= MSG_GRP_LCS+0x47,	///<Payload type {::LcsSuplSessionInfo_t} 
	/// From LCS task to clients. Payload: LcsSuplData_t.
	MSG_LCS_SUPL_INIT_HMAC_REQ								= MSG_GRP_LCS+0x48,	 ///<Payload type {::LcsSuplData_t} 
	/// From LCS task to clients. Payload: LcsFactoryTestData_t.
	MSG_LCS_FACTORY_TEST_IND								= MSG_GRP_LCS+0x49,	 
	/// From LCS task to clients. Payload: LcsSuplCommData_t.
	MSG_LCS_SUPL_WRITE_REQ									= MSG_GRP_LCS+0x4a,	 ///<Payload type {::LcsSuplCommData_t} 
	/// From LCS task to clients. Payload: LcsPosInfo_t.
	MSG_LCS_POSITION_INFO_IND								= MSG_GRP_LCS+0x4b,	 ///<Payload type {::LcsPosInfo_t} 
	/// From LCS task to clients. Payload: LcsPosDetail_t.
	MSG_LCS_POSITION_DETAIL_IND								= MSG_GRP_LCS+0x4c,	///<Payload type {::LcsPosDetail_t} 
	/// From LCS task to clients. Payload: LcsLtoConnection_t.
	MSG_LCS_LTO_CONNECT_REQ									= MSG_GRP_LCS+0x4d,	///<Payload type {::LcsLtoConnection_t}
	/// From LCS task to clients. Payload: LcsLtoSessionInfo_t.
	MSG_LCS_LTO_DISCONNECT_REQ								= MSG_GRP_LCS+0x4e,	///<Payload type {::LcsLtoSessionInfo_t} 
	/// From LCS task to clients. Payload: LcsLtoCommData_t.
	MSG_LCS_LTO_WRITE_REQ									= MSG_GRP_LCS+0x4f,	///<Payload type {::LcsLtoCommData_t} 
	/// From LCS task to clients. Payload: LcsResult_t.
	MSG_LCS_LTO_SYNC_RESULT_IND								= MSG_GRP_LCS+0x50,	///<Payload type {::LcsResult_t} 
	/// From LCS task to clients. Payload: LcsDeviceState_t.
	MSG_LCS_DEVICE_STATE_IND								= MSG_GRP_LCS+0x51,	///<Payload type {::LcsDeviceState_t} 


    /// From platform to LCS task. Notify RRLP data received from stack
    MSG_LCS_RRLP_DATA_IND								 	= MSG_GRP_LCS+0x81, ///<Payload type {::LcsMsgData_t} 
    /// From platform to LCS task. RRLP reset position stored information
    MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND				 	= MSG_GRP_LCS+0x82, ///<Payload type {::LcsClientInfo_t} 
    /// From platform to LCS task. Notify RRC asistance data received from stack
    MSG_LCS_RRC_ASSISTANCE_DATA_IND						 	= MSG_GRP_LCS+0x83, ///<Payload type {::LcsMsgData_t} 
    /// From platform to LCS task. Notify RRC measurement control received from stack
    MSG_LCS_RRC_MEASUREMENT_CTRL_IND					 	= MSG_GRP_LCS+0x84, ///<Payload type {::LcsRrcMeasurement_t} 
    /// From platform to LCS task. Notify RRC system info received from stack
    MSG_LCS_RRC_BROADCAST_SYS_INFO_IND					 	= MSG_GRP_LCS+0x85, ///<Payload type {::LcsRrcBroadcastSysInfo_t} 
    /// From platform to LCS task. Notify RRC UE state received from stack
    MSG_LCS_RRC_UE_STATE_IND							 	= MSG_GRP_LCS+0x86, ///<Payload type {::LcsRrcUeState_t} 
    /// From platform to LCS task. Stop measurement
    MSG_LCS_RRC_STOP_MEASUREMENT_IND					 	= MSG_GRP_LCS+0x87, ///<Payload type {::LcsClientInfo_t} 
    /// From platform to LCS task. RRC reset position stored information
    MSG_LCS_RRC_RESET_POS_STORED_INFO_IND				 	= MSG_GRP_LCS+0x88, ///<Payload type {::LcsClientInfo_t} 



	//Messages to LCS task
	/// To LCS task. Payload: LcsPosReqObj_t.
	MSG_LCS_APP_START_FIX_REQ								= MSG_GRP_LCS+0xc0,	///<Payload type {::LcsPosReqObj_t)} 
	/// To LCS task. Payload: LcsHandle_t
	MSG_LCS_APP_STOP_FIX_REQ								= MSG_GRP_LCS+0xc1,	///<Payload type {::LcsHandle_t} 
	/// To LCS task. Payload: UInt16 for the timeout value.
	MSG_LCS_APP_SET_FIX_TIMEOUT_REQ							= MSG_GRP_LCS+0xc2,	///<Payload type {::UInt16} 
	/// To LCS task. Payload: long for the desired period of position fixing in milliseconds.
	MSG_LCS_APP_SET_FIX_PERIOD_REQ							= MSG_GRP_LCS+0xc3,	 
	/// To LCS task. Payload: LcsFixMode_t
	MSG_LCS_APP_SET_FIX_MODE_REQ							= MSG_GRP_LCS+0xc4,	///<Payload type {::LcsFixMode_t} 
	/// To LCS task. Payload: UInt32 for the position data accuracy in meters.
	MSG_LCS_APP_SET_ACCURACY_REQ							= MSG_GRP_LCS+0xc5,	///<Payload type {::UInt32} 
	/// To LCS task. Payload: none.
	MSG_LCS_APP_STOP_SERVICE_REQ							= MSG_GRP_LCS+0xc6,	 
	/// To LCS task. Payload: LcsSuplAnswer_t
	MSG_LCS_APP_SUPL_NOTIFICATION_RESP						= MSG_GRP_LCS+0xc7,	///<Payload type {::LcsSuplAnswer_t} 
	/// To LCS task. Payload: LcsSuplConnectResp_t.
	MSG_LCS_APP_SUPL_CONNECT_RESP							= MSG_GRP_LCS+0xc8,	///<Payload type {::LcsSuplConnectResp_t} 
	/// To LCS task. Payload: LcsSuplCommData_t
	MSG_LCS_APP_SUPL_DATA_AVAILABLE_NTF						= MSG_GRP_LCS+0xc9,	///<Payload type {::LcsSuplCommData_t} 
	/// To LCS task. Payload: LcsSuplSessionInfo_t
	MSG_LCS_APP_SUPL_DISCONNECT_NTF							= MSG_GRP_LCS+0xca,	///<Payload type {::LcsSuplSessionInfo_t} 
	/// To LCS task. Payload: LcsSuplInitHmacData_t
	MSG_LCS_APP_SUPL_INIT_HMAC_RESP							= MSG_GRP_LCS+0xcb,	///<Payload type {::LcsSuplSessionInfo_t} 
    /// To LCS task. Payload: none.
    MSG_LCS_APP_POWER_CONTROL                               = MSG_GRP_LCS+0xcc,
	/// To LCS task. Payload: LcsDataItem_t
	MSG_LCS_APP_HANDLE_SUPL_INIT							= MSG_GRP_LCS+0xcd,	///<Payload type {::LcsDataItem_t} 
	/// To LCS task. Reset stored GPS data
	MSG_LCS_APP_RESET_STORED_GPS_DATA						= MSG_GRP_LCS+0xce,	
	/// To LCS task. Payload: LcsLtoSessionInfo_t
	MSG_LCS_APP_LTO_CONNECT_RESP							= MSG_GRP_LCS+0xcf,	///<Payload type {::LcsLtoSessionInfo_t} 
	/// To LCS task. Payload: LcsLtoCommData_t
	MSG_LCS_APP_LTO_DATA_AVAILABLE_NTF						= MSG_GRP_LCS+0xd0,	///<Payload type {::LcsLtoCommData_t} 
	/// To LCS task. Payload: LcsLtoSessionInfo_t
	MSG_LCS_APP_LTO_DISCONNECT_NTF							= MSG_GRP_LCS+0xd1,	///<Payload type {::LcsLtoSessionInfo_t} 
		
	/// To LCS task. Payload: none.
    MSG_LCS_APP_LTO_SYNC_REQ								= MSG_GRP_LCS+0xd2,

	/// To LCS task. Payload: LcsPalSuplLocId_t
	MSG_LCS_APP_PAL_SUPL_LOC_ID_RESP						= MSG_GRP_LCS+0xd3,	///<Payload type {::LcsPalSuplLocId_t} 
	/// To LCS task. Payload: GLLBS_RIL_NOTIFY_DATA
	MSG_LCS_APP_PAL_LBS_RIL_RESP							= MSG_GRP_LCS+0xd4,	///<Payload type {::GLLBS_RIL_NOTIFY_DATA} 

	/// To LCS task. Payload: TimerID_t
	MSG_LCS_APP_SUPL_TIMER_IND								= MSG_GRP_LCS+0xd5,	///<Payload type {::TimerID_t} 

	/// To LCS task. Payload: none.
    MSG_LCS_APP_LBS_CONTROL_REQ								= MSG_GRP_LCS+0xd6,
	// End of MSG_GRP_LCS, (0x0D00)



	
	//---------------------------------------------------------------
	// MSG_GRP_TLS, MESSAGE GROUP FOR TLS (0x0E00)
	//---------------------------------------------------------------

    /** 
    Payload: TlsClientRsp_t, e.g. pTlsClientRsp. 
    From TLS inner task to the registered TLS application.
    When an application provides its own registered client ID, and the connection information 
    (see TlsConnectionInfo_t in tlsclient_api.h) to request a TLS connection through 
    TLS_ClientOpenConnection()(defined in tlsclient_api.h). 
    The client will be notified of the status of the connection request through 
    this message where the status is available in pTlsClientRsp->rspCode
    (see the return code defined in tlsclient_api.h). 
    **/ 
    MSG_TLS_OPEN_CONNECTION_RSP                 			= MSG_GRP_TLS+0x81, ///<Payload type {::TlsClientRsp_t}

    /** 
    Payload: TlsClientRsp_t, e.g. pTlsClientRsp. 
    From TLS inner task to the registered TLS application.
    Any connection state change or error occurs after TLS client finishes 
    the handshake protocol with TLS server, this notification message is used to notify 
    the TLS client of such change. The status is available in pTlsClientRsp->rspCode
    (see the return code defined in tlsclient_api.h). 
    **/ 
    MSG_TLS_CONNECTION_STATE_NTF                 			= MSG_GRP_TLS+0x82, ///<Payload type {::TlsClientRsp_t}

    /** 
    Payload: TlsClientRsp_t, e.g. pTlsClientRsp. 
    From TLS inner task to the registered TLS application.
    The registered client will receive this message each time an application 
    data is received. The received data is available in pTlsClientRsp->paramData, 
    and the number of bytes received is shown in pTlsClientRsp->paramLen. 
    This event is only possible when the TLS application_data protocol 
    (ContentType 23 as defined in RFC 2246) is enabled, which is after 
    the TLS client and TLS server finish the TLS handshake protocol.
    **/ 
    MSG_TLS_RECVD_APP_DATA_NTF                 			    = MSG_GRP_TLS+0x83, ///<Payload type {::TlsClientRsp_t}

	
    //------------------------------------------------
    // Internal Messages to TLS inner task
    //-------------------------------------------------
	/** @cond */

    /** 
    Payload: TlsClientHdl_t.
    This message is sent from TlsSocketCallBack() to instruct the 
    TLS client and TLS server starting handshake authentification. 
    This takes place as soon as the secure socket is established upon 
    receipt of the SOCK_SIG_CONNECTED signal.   
    **/ 
    MSG_TLS_INIT_HANDSHAKE                                  = MSG_GRP_TLS+0x60, ///<Payload type {::TlsClientHdl_t}

    /** 
    Payload: TlsClientHdl_t.
    Sent from TlsSocketCallBack() to signal the arrival of SOCK_SIG_READABLE event. 
    **/ 
    MSG_TLS_RECV_DATA                                       = MSG_GRP_TLS+0x61, ///<Payload type {::TlsClientHdl_t}

    /** 
    Payload: TlsClientHdl_t.
    This message is generated when application invokes 
    TLS_ClientCloseConnection()(defined in tlsclient_api.h) 
    to request closing a specific connection. 
    **/ 
    MSG_TLS_CLOSE_CONNECTION                                = MSG_GRP_TLS+0x62, ///<Payload type {::TlsClientHdl_t}

    /**
    Payload: TlsSecurityContextHdl_t. 
    This message is sent to the TLS Inner queue when application invokes 
    TLS_UnloadSecurityContext() to request closing a specific security context.
    **/ 
    MSG_TLS_UNLOAD_SECURITY_CONTEXT                         = MSG_GRP_TLS+0x63, ///<Payload type {::TlsSecurityContextHdl_t}


	/** @endcond */

	// End of MSG_GRP_TLS, (0x0E00)

	

	/*\@}*/
	
	//---------------------------------------------------------------
	// MSG_GRP_FTP, MESSAGE GROUP FOR FTP (0x0F00)
	//---------------------------------------------------------------
	
    /** 
    This message is generated when application invokes 
    FTP_Connect() to request opening an FTP connection,
	and the connect succeeds.
	Payload: UInt32, the socket descriptor.
    **/ 
    MSG_FTPC_CONNECTED_IND									= MSG_GRP_FTP+0x01,	///<Payload type {::UInt32}
	
    /** 
    This message is generated when application invokes 
    FTP_Connect() to request opening an FTP connection,
	and the connect fails.

	The payload contents are FTPCStatus_t, which contains
	the FTP client result code. If a socket error occurred
	(FTPC_RESULT_SOCK_ERROR), the payload contains
	the socket errno code defined in sockerrno.h.
	If the error is an FTP server rejection
	(FTPC_RESULT_ERROR), the payload contains the FTP
	reply code defined by FTPCReplyCode_t.
    **/ 
    MSG_FTPC_CONNECT_FAIL									= MSG_GRP_FTP+0x02,	///<Payload type {::FTPCStatus_t}
	
    /** 
    This message is generated when the FTP client
	is disconnected from the server.
	Payload: none.
    **/ 
    MSG_FTPC_DISCONNECTED_IND								= MSG_GRP_FTP+0x03,

    /** 
    This message is generated when the FTP client is
	ready for commands.
	Payload: none.
    **/ 
    MSG_FTPC_READY_IND										= MSG_GRP_FTP+0x04,

    /** 
    This message is generated when the FTP client is
	logged in successfully.
	Payload: none.
    **/ 
    MSG_FTPC_LOGGED_IN_IND									= MSG_GRP_FTP+0x05,

    /** 
    This message is generated when the FTP client's
	login failed.
	Payload: none.
    **/ 
    MSG_FTPC_LOGIN_FAIL										= MSG_GRP_FTP+0x06,	///<Payload type {::FTPCStatus_t}

    /** 
    This message is generated when the FTP client
	retrieves a file from the server successfully.
	Payload: FTPCTxInfo_t, the data transfer information.
    **/ 
    MSG_FTPC_GET_IND										= MSG_GRP_FTP+0x08, ///< Payload type {::FTPCTxInfo_t}

    /** 
    This message is generated when the FTP client
	fails to retrieves a file from the server.
	Payload: FTPReplyCode_t, the reply from the server.
    **/ 
    MSG_FTPC_GET_FAIL										= MSG_GRP_FTP+0x09,	///<Payload type {::FTPCStatus_t}

    /** 
    This message is generated when the FTP client
	stores a file to the server successfully.
	Payload: FTPCTxInfo_t, the data transfer information.
    **/ 
    MSG_FTPC_PUT_IND										= MSG_GRP_FTP+0x0a, ///< Payload type {::FTPCTxInfo_t}

    /** 
    This message is generated when the FTP client
	fails to store a file to the server.
	Payload: FTPReplyCode_t, the reply from the server.
    **/ 
    MSG_FTPC_PUT_FAIL										= MSG_GRP_FTP+0x0b,	///<Payload type {::FTPCStatus_t}

    /** 
    This message is generated when the FTP client
	changes directories successfully.
	Payload: none.
    **/ 
    MSG_FTPC_CHDIR_IND										= MSG_GRP_FTP+0x0c,

    /** 
    This message is generated when the FTP client
	fails to change directories.
	Payload: FTPReplyCode_t, the reply from the server.
    **/ 
    MSG_FTPC_CHDIR_FAIL										= MSG_GRP_FTP+0x0d,	///<Payload type {::FTPCStatus_t}

    /** 
    This message is generated when the FTP client
	is transferring data to/from the FTP server.
	Payload: FTPCTxProgress_t, the current progress
	of the data transfer.
    **/ 
    MSG_FTPC_TX_PROGRESS_IND								= MSG_GRP_FTP+0x0e,	///<Payload type {::FTPCTxInfo_t}


	
	/** @cond */	

	




	//===============================================================
	//===============================================================
	//
	//	Platform Internal Messages (not processed by clients)
	//
	//===============================================================
	//===============================================================

	
	//---------------------------------------------------------------
	// MSG_GRP_INT_UTIL, MESSAGE GROUP FOR INTERNAL GENERAL PURPOSE (0x2000)
	//---------------------------------------------------------------

	MSG_MEASURE_REPORT_PARAM_IND	= MSG_GRP_INT_UTIL+0x00,	 /* An indication, which carries RR measurement report parameters to API/ATC*/

    MSG_ECC_LIST_IND				= MSG_GRP_INT_UTIL+0x10,	///<Payload type {::T_NW_EMERGENCY_NUMBER_LIST} /* which category this falls in? */

	MSG_UE_3G_STATUS_IND 				= MSG_GRP_INT_UTIL+0x11,	///<Payload type {::MSUe3gStatusInd_t}

	// End of MSG_GRP_INT_UTIL (0x2000)


	
	
	//---------------------------------------------------------------
	// MSG_GRP_INT_ATC, MESSAGE GROUP FOR INTERNAL ATC (0x2100)
	//---------------------------------------------------------------
	
	// general ATC task msg
	MSG_POWER_DOWN_REQ			= MSG_GRP_INT_ATC+0x00,	
	MSG_POWER_UP_REQ			= MSG_GRP_INT_ATC+0x01,	
	MSG_RESET_REQ				= MSG_GRP_INT_ATC+0x03,	
	MSG_STATISTIC_INFO			= MSG_GRP_INT_ATC+0x04,	

	// long AT cmd req msg (from V24 AT Cmd Handler -> ATC task)
	MSG_AT_COPS_CMD				= MSG_GRP_INT_ATC+0x10,	///<Payload type {::ATCOPSData_t}
	MSG_AT_CGATT_CMD			= MSG_GRP_INT_ATC+0x11,	///<Payload type {::ATCGATTData_t}
	MSG_AT_CGACT_CMD			= MSG_GRP_INT_ATC+0x12,	///<Payload type {::ATCGACTData_t}
	MSG_AT_CME_ERROR			= MSG_GRP_INT_ATC+0x13,	///<Payload type {::ATCCmeError_t}
	MSG_AT_CGCMOD_CMD			= MSG_GRP_INT_ATC+0x14,	///<Payload type {::ATCGCMODData_t}

	// Message for AT command 
	MSG_AT_CALL_TIMER			= MSG_GRP_INT_ATC+0x30,	///<Payload type {::CallTimerID_t}
	MSG_AT_CALL_ABORT			= MSG_GRP_INT_ATC+0x31,	///<Payload type {::UInt8}
	MSG_AT_ESC_DATA_CALL		= MSG_GRP_INT_ATC+0x32,	///<Payload type {::UInt8}
	MSG_AT_HANDLE_CHLD_CMD		= MSG_GRP_INT_ATC+0x33,
	MSG_AT_CMMS_TIMER_IND		= MSG_GRP_INT_ATC+0x34,	///<Payload type {::TimerID_t}	

	MSG_AT_CMD_STR		= MSG_GRP_INT_ATC+0x35,	///<Payload type {::}	

	MSG_AT_LINE_STATE_IND	= MSG_GRP_INT_ATC+0x36,	///<Payload type {::Boolean}

	MSG_MS_READY_IND			= MSG_GRP_INT_ATC+0x50,	///<Payload type {::MODULE_READY_STATUS_t}
	MSG_CALL_MONITOR_STATUS		= MSG_GRP_INT_ATC+0x51,	///<Payload type {::UInt8}

	// End of MSG_GRP_INT_ATC (0x2100)



	
	//----------------------------------------------------------------------
	// MSG_GRP_INT_SMS_SS, INTERNAL MESSAGE GROUP FOR SMS, SS, USSD (0x2200)
	//----------------------------------------------------------------------
	// sms/ss messages
	MSG_SMS_COMPOSE_REQ			= MSG_GRP_INT_SMS_SS+0x00,	///<Payload type {::at_cmgsCmd_t}
	MSG_SMS_USR_DATA_IND		= MSG_GRP_INT_SMS_SS+0x01,	
	MSG_SMS_PARM_CHECK_RSP		= MSG_GRP_INT_SMS_SS+0x02,	///<Payload type {::SmsParmCheckRsp_t}
	MSG_SMS_REPORT_IND			= MSG_GRP_INT_SMS_SS+0x03,	///<Payload type {::SmsReportInd_t}
	MSG_SMS_MEM_AVAIL_RSP		= MSG_GRP_INT_SMS_SS+0x04,	///<Payload type {::T_MN_TP_SMS_RSP}
	MSG_SMS_CB_START_REQ		= MSG_GRP_INT_SMS_SS+0x05,	
	MSG_SMS_CBMI_WRITE_REQ		= MSG_GRP_INT_SMS_SS+0x06,	
	MSG_SMS_GET_MSG_FROM_BUFFER	= MSG_GRP_INT_SMS_SS+0x07,	///<Payload type {::SmsGetMsgFromBuffer_t}
	MSG_SMS_STORE_SMS_MSG_REQ	= MSG_GRP_INT_SMS_SS+0x08,	///<Payload type {::SmsMsgToBeStored_t}
	MSG_SMS_SUBMIT_RETRY_REQ	= MSG_GRP_INT_SMS_SS+0x09,
	MSG_SMS_EVALUATE_HOMEZONE_REQ = MSG_GRP_INT_SMS_SS+0x0A,
	MSG_SMS_HOMEZONE_CB_TIMEOUT_IND = MSG_GRP_INT_SMS_SS+0x0B,

	MSG_SMS_CNMA_TIMER_IND		= MSG_GRP_INT_SMS_SS+0x20,	///<Payload type {::TimerID_t}
	MSG_ATC_TIMEOUT_IND			= MSG_GRP_INT_SMS_SS+0x21,	

	MSG_SS_CALL_REQ				= MSG_GRP_INT_SMS_SS+0x32,	///<Payload type {::SS_SsApiReq_t}
	MSG_SS_CALL_REQ_FAIL		= MSG_GRP_INT_SMS_SS+0x33,	///<Payload type {::SsCallReqFail_t}
	MSG_USSD_CALL_REQ			= MSG_GRP_INT_SMS_SS+0x34,	///<Payload type {::UssdCallReq_t}

	// End of MSG_GRP_INT_SMS_SS (0x2200)

	
	
	//---------------------------------------------------------------
	// MSG_GRP_INT_NET, INTERNAL MESSAGE GROUP FOR NET (0x2300)
	//---------------------------------------------------------------

	// message for network service
	MSG_ATTACH_IND				= MSG_GRP_INT_NET+0x00,	
	MSG_DETACH_IND				= MSG_GRP_INT_NET+0x01,	///<Payload type {::Inter_DetachInd_t}
	MSG_ACTIVATE_IND			= MSG_GRP_INT_NET+0x02,	///<Payload type {::Inter_ActivateInd_t}
	MSG_DEACTIVATE_IND			= MSG_GRP_INT_NET+0x03,	///<Payload type {::Inter_DeactivateInd_t}
	MSG_ATTACH_CNF				= MSG_GRP_INT_NET+0x04,	///<Payload type {::Inter_AttachCnf_t}
	MSG_DETACH_CNF				= MSG_GRP_INT_NET+0x05,	///<Payload type {::Inter_DetachCnf_t}
	MSG_ACTIVATE_CNF			= MSG_GRP_INT_NET+0x06,	///<Payload type {::Inter_ActivateCnf_t}
	MSG_DEACTIVATE_CNF			= MSG_GRP_INT_NET+0x07,	///<Payload type {::Inter_DeactivateCnf_t}
	MSG_SN_XID_CNF				= MSG_GRP_INT_NET+0x08,	///<Payload type {::Inter_SnXidCnf_t}
	MSG_ACTIVATE_SEC_CNF		= MSG_GRP_INT_NET+0x09,	///<Payload type {::Inter_ActivateSecCnf_t}

	MSG_MODIFY_IND				= MSG_GRP_INT_NET+0x10,	///<Payload type {::Inter_ModifyContextInd_t}
	MSG_SERVICE_IND				= MSG_GRP_INT_NET+0x11,	///<Payload type {::Inter_ServiceInd_t}
	MSG_CHECK_QOSMIN_IND		= MSG_GRP_INT_NET+0x12,	///<Payload type {::Inter_CheckQoSMinInd_t}
	
	MSG_TIMEZONE_IND			= MSG_GRP_INT_NET+0x13,	///<Payload type {::MMMsgParmTimeZoneInd_t},Do NOT use, this is a internal CAPI message, will be removed.
	MSG_INT_TIMEZONE_IND		= MSG_GRP_INT_NET+0x13,	///<Payload type {::MMMsgParmTimeZoneInd_t}
	MSG_INT_DATE_IND			= MSG_GRP_INT_NET+0x14, ///<Payload type {::MMMsgParmDateInd_t}

	// End of MSG_GRP_INT_NET (0x2300)


	
	//----------------------------------------------------------------------
	// MSG_GRP_INT_DC, INTERNAL MESSAGE GROUP FOR DATA CONNECTIONS (0x2400)
	//----------------------------------------------------------------------
	
	/** 
	Function: Indicate that GPRS data is temporarily suspended
	Parameters: No
	**/
	MSG_DATA_SUSPEND_IND		= MSG_GRP_INT_DC+0x13,	///<Payload type {::GPRSSuspendInd_t}
	

	/** 
	Function: Indicate that GPRS data is resumed
	Parameters: No
	**/
	MSG_DATA_RESUME_IND			= MSG_GRP_INT_DC+0x14,	

	/*Indication of HSDPA status of the PLMN registered if changed*/
	MSG_HSDPA_STATUS_IND		= MSG_GRP_INT_DC+0x15,	
	
	MSG_PCHEX_UL_DATA_IND		= MSG_GRP_INT_DC+0x16,	///<Payload type {::PDP_ULData_Ind_t}

	MSG_DATA_STATE_TIMER		= MSG_GRP_INT_DC+0x20,	
	MSG_CGSEND_TIMER			= MSG_GRP_INT_DC+0x21,	
	MSG_MODIFY_CNF				= MSG_GRP_INT_DC+0x22,	///<Payload type {::Inter_ModifyCnf_t}
	MSG_RADIO_ACTIVITY_IND		= MSG_GRP_INT_DC+0x23,	///<Payload type {::MSRadioActivityInd_t}
	MSG_MS_STATUS_IND 			= MSG_GRP_INT_DC+0x24,
	MSG_PCH_DELAYDETACH_TIMER	= MSG_GRP_INT_DC+0x25,
	
	// End of MSG_GRP_INT_DC, (0x2400)



	
	//----------------------------------------------------------------------
	// MSG_GRP_INT_SIM, INTERNAL MESSAGE GROUP FOR SIM (0x2500)
	//----------------------------------------------------------------------

	// Internal messages related to SIM 
	MSG_SIM_STATUS_IND						= MSG_GRP_INT_SIM+0x00,	///<Payload type {::SIMStatus_t}	/* Notify that SIM status is available */
	MSG_SIM_ACM_UPDATE_IND					= MSG_GRP_INT_SIM+0x01,		/* Notify that accumulated call meter has been updated */
	MSG_SIM_SMS_MEM_AVAIL_IND				= MSG_GRP_INT_SIM+0x02,	///<Payload type {::Boolean}	/* Notify that SIM SMS has empty space */
	MSG_SIM_REFRESH_COMPL_IND				= MSG_GRP_INT_SIM+0x03,		/* Notify that the refreshing of SIM data (SMS and PBK etc) is completed */
	
	MSG_SIM_PROC_DFILE_INFO_REQ				= MSG_GRP_INT_SIM+0x10,	///<Payload type {::SIM_PROC_DFILE_INFO_REQ_t}	/* Request DF information */
	MSG_SIM_PROC_EFILE_INFO_REQ				= MSG_GRP_INT_SIM+0x11,	///<Payload type {::SIM_PROC_EFILE_INFO_REQ_t}	/* Request EF information */
	MSG_SIM_PROC_WHOLE_BINARY_FILE_REQ		= MSG_GRP_INT_SIM+0x12,	///<Payload type {::SIM_PROC_WHOLE_BINARY_FILE_REQ_t}	/* Request to read whole Binary EF */
	MSG_SIM_PROC_READ_BINARY_FILE_REQ		= MSG_GRP_INT_SIM+0x13,	///<Payload type {::SIM_PROC_READ_BINARY_FILE_REQ_t}	/* Request to read Binary EF */
	MSG_SIM_PROC_READ_FILE_REC_REQ			= MSG_GRP_INT_SIM+0x14,	///<Payload type {::SIM_PROC_READ_FILE_REC_REQ_t}	/* Request to read EF (Linear-Fixed or Cyclic) records */
	MSG_SIM_PROC_UPDATE_BINARY_FILE_REQ 	= MSG_GRP_INT_SIM+0x15,	///<Payload type {::SIM_PROC_UPDATE_BINARY_FILE_REQ_t}	/* Request to update Binary EF */
	MSG_SIM_PROC_UPDATE_LINEAR_FILE_REQ 	= MSG_GRP_INT_SIM+0x16,	///<Payload type {::SIM_PROC_UPDATE_LINEAR_FILE_REQ_t}	/* Request to update a record in Linear-Fixed EF */
	MSG_SIM_PROC_UPDATE_CYCLIC_FILE_REQ 	= MSG_GRP_INT_SIM+0x17,	///<Payload type {::SIM_PROC_UPDATE_CYCLIC_FILE_REQ_t}	/* Request to update a record in Cyclic EF */
	
	MSG_SIM_PROC_REMAINING_PIN_ATTEMPT_REQ	= MSG_GRP_INT_SIM+0x19,	///<Payload type {::SIM_PROC_REMAINING_PIN_ATTEMPT_REQ_t}	/* Request to get remaining PIN1/PIN2/PUK1/PUK2 attempts in SIM */
	MSG_SIM_PROC_PBK_INFO_REQ				= MSG_GRP_INT_SIM+0x1A,	///<Payload type {::SIM_PROC_PBK_INFO_REQ_t}	/* Request to get phonebook information */
	MSG_SIM_PROC_SET_FDN_REQ				= MSG_GRP_INT_SIM+0x1B,	///<Payload type {::SIM_PROC_SET_FDN_REQ_t}	/* Request to turn FDN on/off */
	MSG_SIM_PROC_VERIFY_PIN_REQ				= MSG_GRP_INT_SIM+0x1C,	///<Payload type {::SIM_PROC_VERIFY_PIN_REQ_t}	/* Request to verify PIN1/PIN2 */
	MSG_SIM_PROC_CHANGE_PIN_REQ				= MSG_GRP_INT_SIM+0x1D,	///<Payload type {::SIM_PROC_CHANGE_PIN_REQ_t}	/* Request to change PIN1/PIN2 */
	MSG_SIM_PROC_SET_PIN1_REQ				= MSG_GRP_INT_SIM+0x1E,	///<Payload type {::SIM_PROC_SET_PIN1_REQ_t}	/* Request to enable/disable PIN1 */
	MSG_SIM_PROC_UNBLOCK_PIN_REQ			= MSG_GRP_INT_SIM+0x1F,	///<Payload type {::SIM_PROC_UNBLOCK_PIN_REQ_t}	/* Request to unblock PIN1/PIN2 */
    
	MSG_SIM_PROC_READ_PBK_REQ				= MSG_GRP_INT_SIM+0x20,	///<Payload type {::SIM_PROC_READ_PBK_REQ_t}	/* Request to read phonebook entry */
	MSG_SIM_PROC_WRITE_PBK_REQ				= MSG_GRP_INT_SIM+0x21,	///<Payload type {::SIM_PROC_WRITE_PBK_REQ_t}	/* Request to write phonebook entry */
	MSG_SIM_PROC_WRITE_PREF_PLMN_REQ		= MSG_GRP_INT_SIM+0x22,	///<Payload type {::SIM_PROC_WRITE_PREF_PLMN_REQ_t}	/* Request to write preferred PLMN list */
	MSG_SIM_PROC_READ_ACM_MAX_REQ			= MSG_GRP_INT_SIM+0x23,	///<Payload type {::SIM_PROC_READ_ACM_MAX_REQ_t}	/* Request to read maximum ACM value */
	MSG_SIM_PROC_WRITE_ACM_MAX_REQ			= MSG_GRP_INT_SIM+0x24,	///<Payload type {::SIM_PROC_WRITE_ACM_MAX_REQ_t}	/* Request to write maximum ACM value */
	MSG_SIM_PROC_READ_ACM_REQ				= MSG_GRP_INT_SIM+0x25,	///<Payload type {::SIM_PROC_READ_ACM_REQ_t}	/* Request to read ACM value */
	MSG_SIM_PROC_WRITE_ACM_REQ				= MSG_GRP_INT_SIM+0x26,	///<Payload type {::SIM_PROC_WRITE_ACM_REQ_t}	/* Request to write ACM value */
	MSG_SIM_PROC_INCREASE_ACM_REQ			= MSG_GRP_INT_SIM+0x27,	///<Payload type {::SIM_PROC_INCREASE_ACM_REQ_t}	/* Request to increase ACM value */
	MSG_SIM_PROC_SERV_PROV_REQ				= MSG_GRP_INT_SIM+0x28,		/* Request to read Service Provider Name in EF-SPN */
	MSG_SIM_PROC_READ_PUCT_REQ				= MSG_GRP_INT_SIM+0x29,	///<Payload type {::SIM_PROC_READ_PUCT_REQ_t}	/* Request to read Price Per Unit information */
	MSG_SIM_PROC_WRITE_PUCT_REQ				= MSG_GRP_INT_SIM+0x2A,	///<Payload type {::SIM_PROC_WRITE_PUCT_REQ_t}	/* Request to write Price Per Unit information */
	MSG_SIM_PROC_SEARCH_SMS_REQ				= MSG_GRP_INT_SIM+0x2B,	///<Payload type {::SIM_PROC_SEARCH_SMS_REQ_t}	/* Request to search for SMS of a status */
	MSG_SIM_PROC_READ_SMS_REQ				= MSG_GRP_INT_SIM+0x2C,	///<Payload type {::SIM_PROC_READ_SMS_REQ_t}	/* Request to read SMS */
	MSG_SIM_PROC_WRITE_SMS_REQ				= MSG_GRP_INT_SIM+0x2D,	///<Payload type {::SIM_PROC_WRITE_SMS_REQ_t}	/* Request to wrtie SMS */	
	MSG_SIM_PROC_WRITE_SMS_STATUS_REQ		= MSG_GRP_INT_SIM+0x2E,	///<Payload type {::SIM_PROC_WRITE_SMS_STATUS_REQ_t}	/* Request to wrtie SMS status */
	MSG_SIM_PROC_READ_SMS_PARAM_REQ			= MSG_GRP_INT_SIM+0x2F,	///<Payload type {::SIM_PROC_READ_SMS_PARAM_REQ_t}	/* Request to read SMS parameters */

	MSG_SIM_PROC_WRITE_SMS_SCA_REQ			= MSG_GRP_INT_SIM+0x30,	///<Payload type {::SIM_PROC_WRITE_SMS_SCA_REQ_t}	/* Request to write SMS Service Center Address */
	MSG_SIM_PROC_SMS_REF_NUM_REQ			= MSG_GRP_INT_SIM+0x31,	///<Payload type {::SIM_PROC_SMS_REF_NUM_REQ_t}	/* Request to get SMS reference number */
	MSG_SIM_APDU_GENERIC_ACCESS_REQ			= MSG_GRP_INT_SIM+0x32,	///<Request for generic SIM Accesss, e.g. AT+CSIM command */
	MSG_SIM_PROC_RESTRICTED_ACCESS_REQ		= MSG_GRP_INT_SIM+0x33,	///<Payload type {::SIM_PROC_RESTRICTED_ACCESS_REQ_t}	/* Request for restricted SIM Accesss, e.g. AT+CRSM command */

	MSG_SIM_PROC_SET_EST_SERV_REQ			= MSG_GRP_INT_SIM+0x34,	///<Payload type {::USIM_SET_EST_SERV_REQ_t}	/* Request to activate/deactivate service in EF-EST in USIM */

	MSG_SIM_PROC_UPDATE_ONE_APN_REQ			= MSG_GRP_INT_SIM+0x35,	///<Payload type {::USIM_UPDATE_ONE_APN_REQ_t}	/* Request to update one APN in EF-ACL in USIM */
	MSG_SIM_PROC_DELETE_ALL_APN_REQ			= MSG_GRP_INT_SIM+0x36,	///<Payload type {::USIM_DELETE_ALL_APN_REQ_t}	/* Request to delete all APN's in EF-ACL's in USIM */

	//										= MSG_GRP_INT_SIM+0x40, ///< Empty slot!!!

	MSG_SIM_PROC_GENERAL_SERVICE_IND		= MSG_GRP_INT_SIM+0x41,	///<Payload type {::SIMGeneralServiceStatus_t}	/* SIM general service indication */

	MSG_SIM_PROC_SEEK_RECORD_REQ			= MSG_GRP_INT_SIM+0x42,	///<Payload type {::SIM_PROC_SEEK_RECORD_REQ_t}/* Request to send Search Record command (Seek command in 2G SIM) */

	MSG_SIM_PROC_NUM_OF_PLMN_REQ			= MSG_GRP_INT_SIM+0x43,	///<Payload type {::SIM_NUM_OF_PLMN_REQ_t}	/* Request to get number of PLMN entries in SIM file */

	MSG_SIM_PROC_READ_PLMN_REQ				= MSG_GRP_INT_SIM+0x44,	///<Payload type {::SIM_READ_PLMN_REQ_t}	/* Request to read PLMN entries in SIM file */

	MSG_SIM_PROC_UPDATE_PLMN_REQ			= MSG_GRP_INT_SIM+0x45,	///<Payload type {::SIM_UPDATE_PLMN_REQ_t}	/* Request to update PLMN entries in SIM file */

	MSG_SIM_PROC_ISIM_ACTIVATE_REQ			= MSG_GRP_INT_SIM+0x46,	///<Payload type {::SIM_PROC_ISIM_ACTIVATE_REQ_t}	/* Request to activate ISIM application */

	MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ		= MSG_GRP_INT_SIM+0x47,	///<Payload type {::SIM_PROC_ISIM_AUTHENTICATE_REQ_t}	/* Request to perform ISIM AKA Security Context authentication request */

	MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ		= MSG_GRP_INT_SIM+0x48,	///<Payload type {::SIM_PROC_ISIM_AUTHENTICATE_REQ_t}	/* Request to perform ISIM HTTP Digest Security Context authentication request */
		
	MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ	= MSG_GRP_INT_SIM+0x49,	///<Payload type {::SIM_PROC_ISIM_AUTHENTICATE_REQ_t}	/* Request to perform ISIM Bootstrapping Mode Security Context authentication request */
		
	MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ	= MSG_GRP_INT_SIM+0x4A,	///<Payload type {::SIM_PROC_SET_BDN_REQ_t}	/* Request to perform ISIM NAF Derivation Mode Security Context authentication request */

    MSG_SIM_PROC_SET_BDN_REQ				= MSG_GRP_INT_SIM+0x4B,	///<Payload type {::SIM_PROC_SET_BDN_REQ_t}	/* Request to turn BDN on/off */    

    MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ		= MSG_GRP_INT_SIM+0x4C,	///<Payload type {::SIM_UPDATE_MUL_PLMN_REQ_t}	/* Request to update multiple PLMN entries in SIM file */

    MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ	    = MSG_GRP_INT_SIM+0x4D,	///<Payload type {::SIM_PROC_POWER_ON_OFF_CARD_REQ_t}	/* Request to power on/off SIM card */

    MSG_SIM_PROC_GET_RAW_ATR_REQ	        = MSG_GRP_INT_SIM+0x4E,	///<Payload type {::SIM_PROC_GET_RAW_ATR_REQ_t}	/* Request to get raw ATR info */

	MSG_SIM_PROC_OPEN_SOCKET_REQ			= MSG_GRP_INT_SIM+0x4F,	///<Payload type {::SIM_PROC_OPEN_SOCKET_REQ_t}	/* Request to open a logical channel */

	MSG_SIM_PROC_SELECT_APPLI_REQ			= MSG_GRP_INT_SIM+0x50,	///<Payload type {::SIM_PROC_SELECT_APPLI_REQ_t} /* Request to select a non-USIM application */

	MSG_SIM_PROC_DEACTIVATE_APPLI_REQ		= MSG_GRP_INT_SIM+0x51,	///<Payload type {::SIM_PROC_DEACTIVATE_APPLI_REQ_t} /* Request to deactivate a non-USIM application */

	MSG_SIM_PROC_CLOSE_SOCKET_REQ			= MSG_GRP_INT_SIM+0x52,	///<Payload type {::SIM_PROC_CLOSE_SOCKET_REQ_t}	/* Request to close a logical channel */
    
	// End of MSG_GRP_INT_SIM, (0x2500)

	

	//----------------------------------------------------------------------
	// MSG_GRP_INT_STK, INTERNAL MESSAGE GROUP FOR STK (0x2600)
	//----------------------------------------------------------------------
	
#ifndef STACK_wedge
    /**
    This STK message is used to reject the response to ::MSG_STK_SETUP_MENU_REQ (e.g., in case stack is busy)
    when user selects a STK menu.
    **/
	MSG_STK_MENU_SELECTION_REJ					= MSG_GRP_INT_STK+0x20,	
#else
	MSG_STK_MENU_SELECTION_RES					= MSG_GRP_INT_STK+0x20,	
#endif

    /**
    This STK message is used to sent a text with a maximim length of 160 characters to MMI.
	For payload content, see ::DisplayText_t
    **/
	MSG_STK_DISPLAY_TEXT_REQ					= MSG_GRP_INT_STK+0x21,	///<Payload type {::SIMParmDisplayTextReq_t}

    /**
    This STK message is used to sent a text with a maximim length of 160 characters to MMI to be displayed in idle mode.
	For payload content, see ::IdleModeText_t
    **/
	MSG_STK_SETUP_IDLEMODE_TEXT_REQ				= MSG_GRP_INT_STK+0x22,	///<Payload type {::SIMParmSetupIdlemodeTextReq_t}

    /**
    This STK message is used for GET_INPUT command.  For payload content, see ::GetInput_t
    **/
	MSG_STK_GET_INPUT_REQ						= MSG_GRP_INT_STK+0x23,	///<Payload type {::SIMParmGetInputReq_t}

    /**
    This STK message is used for GET_INKEY command.  For payload content, see ::GetInkey_t
    **/
	MSG_STK_GET_INKEY_REQ						= MSG_GRP_INT_STK+0x24,	///<Payload type {::SIMParmGetInkeyReq_t}

    /**
    This STK message is used for PLAY_TONE command.  For payload content, see ::PlayTone_t
    **/
	MSG_STK_PLAY_TONE_REQ						= MSG_GRP_INT_STK+0x25,	///<Payload type {::SIMParmPlayToneReq_t}

  	/**
  	This STK message is used to request MMI to display different items where the user can select one.
	For payload content, see ::SelectItem_t
  	**/
	MSG_STK_SELECT_ITEM_REQ						= MSG_GRP_INT_STK+0x26,	///<Payload type {::SIMParmSelectItemReq_t}

  	/**
   	This STK message is used to request MMI to display different menu items where the user can select one.
	For payload content, see ::SetupMenu_t
  	**/
	MSG_STK_SETUP_MENU_REQ						= MSG_GRP_INT_STK+0x27,	///<Payload type {::SIMParmSetupMenuReq_t}

  	/**
	This STK message is used to request MMI to do a refresh.
	For payload content, see ::Refresh_t
  	**/
	MSG_STK_SIMTOOLKIT_REFRESH_REQ				= MSG_GRP_INT_STK+0x28,	///<Payload type {::SIMParmSimtoolkitRefreshReq_t}

  	/**
	This STK message is used to request AT to do a refresh.
	For payload content, see ::Refresh_t
  	**/
	MSG_STK_SIAT_SIMTOOLKIT_REFRESH_REQ			= MSG_GRP_INT_STK+0x29,	///<Payload type {::SIMParmSiatSimtoolkitRefreshReq_t}

	MSG_STK_STK_END_IND							= MSG_GRP_INT_STK+0x2A,  	

  	/**
	This STK message is used to request seding a SS String (originated from SIM).
	For payload content, see ::SendSs_t.
  	**/
	MSG_STK_SEND_SS_REQ							= MSG_GRP_INT_STK+0x2B,	///<Payload type {::SIMParmSendSsReq_t}

  	/**
	This STK message is used to request setting up a call (originated from SIM).
	For payload content, see ::SetupCall_t
  	**/
	MSG_STK_SETUP_CALL_REQ						= MSG_GRP_INT_STK+0x2C,	///<Payload type {::SIMParmSetupCallReq_t}

  	/**
	This STK message is used to notify of sending a MO SMS (originated from SIM).
	For payload content, see ::SIMParmMoSMSAlphaInd_t
  	**/
	MSG_STK_MOSMS_ALPHA_IND						= MSG_GRP_INT_STK+0x2D,	///<Payload type {::SIMParmMoSMSAlphaInd_t}

  	/**
	This STK message is used to request Date info from SIM
  	**/
	MSG_STK_LOCAL_DATE_REQ						= MSG_GRP_INT_STK+0x30,	

  	/**
	This STK message is used to request Language info from SIM
  	**/
	MSG_STK_LOCAL_LANG_REQ						= MSG_GRP_INT_STK+0x31,	

  	/**
	This STK message is used to indicate failure of a CC Setup Call.
	For payload content, see ::StkCallSetupFail_t
  	**/
	MSG_STK_CC_SETUPFAIL_IND					= MSG_GRP_INT_STK+0x32,	///<Payload type {::StkCallSetupFail_t}

  	/**
	This STK message is used to request MMI to display for CC Setup Call.
	For payload content, see ::StkCallControlDisplay_t
  	**/
	MSG_STK_CC_DISPLAY_IND						= MSG_GRP_INT_STK+0x33,	///<Payload type {::StkCallControlDisplay_t}

  	/**
	This STK message is used to send response to SIM
	For payload content, see ::StkSendTermRespData_t
  	**/
	MSG_STK_SEND_TERMINAL_RESPONSE				= MSG_GRP_INT_STK+0x34,	///<Payload type {::StkSendTermRespData_t}

  	/**
	This STK message is used to request to launch a browser (originated from SIM).
	For payload content, see ::STKLaunchBrowserReq_t
  	**/
	MSG_STK_LAUNCH_BROWSER_REQ					= MSG_GRP_INT_STK+0x35,	///<Payload type {::STKLaunchBrowserReq_t}

  	/**
	This STK message is used to stop Call Setup Retry Timer.
  	**/
	MSG_STK_STOP_RETRY_TIMER_REQ				= MSG_GRP_INT_STK+0x36,	

  	/**
	This STK message is used to request sending a DTMF.
	For payload content, see ::SIMParmSendDtmfReq_t
  	**/
	MSG_STK_SEND_DTMF_REQ						= MSG_GRP_INT_STK+0x37,	///<Payload type {::SIMParmSendDtmfReq_t}

	/**
	This STK message is used to send Call Connected Event to SIM.
  	**/
	MSG_STK_CALL_CONNECTED_EVT					= MSG_GRP_INT_STK+0x38,	///<Payload type {::StkCallConnectedEvt_t}

	/**
	This STK message is used to request the resending a SS/USSD String (originated from SIM).
	For payload content, see ::SendSs_t.
  	**/
	MSG_STK_RESEND_SS_REQ						= MSG_GRP_INT_STK+0x39,	

  	/**
	This STK message is used to request Starting a generic timer.
	For payload contents, see ::StkGenericTimerType_t
  	**/
	MSG_STK_GENERIC_TIMER_START					= MSG_GRP_INT_STK+0x70,	

  	/**
	This STK message is used to request Stopping a generic timer.
  	**/
	MSG_STK_GENERIC_TIMER_STOP					= MSG_GRP_INT_STK+0x71,	

#ifndef STACK_wedge
    // BRCM/Davis Zhu 6/30/2005: USIMAP has its own timeout mechanism. So no need for this piece of code.
  	/**
	This STK message is used to request Stopping a Terminal Response timer.
  	**/
	MSG_STK_TERM_RESP_TIMER_STOP				= MSG_GRP_INT_STK+0x72,	
#endif	

	/**
	This STK message is used to request the activation of UICC interface, e.g. the 
	UICC-CLF interface defined in section 8.89 in ETSI 102 223.
  	**/
	MSG_STK_ACTIVATE_REQ						= MSG_GRP_INT_STK+0x73,


	// Internal Messgae for setup call
	MSG_CALL_ESTABLISH_REQ			= MSG_GRP_INT_STK+0x80,	///<Payload type {::CCParmSend_t}

	// Internal Messages for Call Control
	MSG_STK_CALL_CONTROL_SETUP_RSP	= MSG_GRP_INT_STK+0x90,	///<Payload type {::StkCallControlSetupRsp_t}
	MSG_STK_CALL_CONTROL_SS_RSP		= MSG_GRP_INT_STK+0x91,	///<Payload type {::StkCallControlSsRsp_t}
	MSG_STK_CALL_CONTROL_USSD_RSP	= MSG_GRP_INT_STK+0x92,	///<Payload type {::StkCallControlUssdRsp_t}
	MSG_STK_CALL_CONTROL_SMS_RSP	= MSG_GRP_INT_STK+0x93,	///<Payload type {::StkCallControlSmsRsp_t}

	MSG_STK_PROACTIVE_CMD_IND		= MSG_GRP_INT_STK+0x94,	///<Payload type {::ProactiveCmdData_t} /* Proactive command in raw format as defined in GSM 11.14 */
	MSG_STK_ENVELOPE_RSP			= MSG_GRP_INT_STK+0x95,	///<Payload type {::EnvelopeRspData_t} /* Response to the Envelope command */
	MSG_STK_TERMINAL_RESPONSE_RSP	= MSG_GRP_INT_STK+0x96,	///<Payload type {::TerminalResponseRspData_t} /* Response to the Terminal Response command */
	
	// Internal Messages for STK Data Service
	MSG_STK_OPEN_CHANNEL_REQ		= MSG_GRP_INT_STK+0x97,	///<Payload type {::SIMParmOpenChannelReq_t}
	MSG_STK_SEND_DATA_REQ			= MSG_GRP_INT_STK+0x98,	///<Payload type {::SIMParmSendDataReq_t}
	MSG_STK_RECEIVE_DATA_REQ		= MSG_GRP_INT_STK+0x99,	///<Payload type {::SIMParmReceiveDataReq_t}
	MSG_STK_CLOSE_CHANNEL_REQ		= MSG_GRP_INT_STK+0x9A,	///<Payload type {::SIMParmCloseChannelReq_t}
	MSG_STK_GET_CHANNEL_STATUS_REQ	= MSG_GRP_INT_STK+0x9B,	

	// Terminate STK Data Service Session 
	MSG_STK_TERMINATE_DATA_SERV_REQ	= MSG_GRP_INT_STK+0x9C,	///<Payload type {::CallbackFunc_t *}
	MSG_STK_TERMINATE_DATA_SERV_RSP	= MSG_GRP_INT_STK+0x9D,	///<Payload type {::Boolean}

	// New data has been received in data link
	MSG_STK_DATA_AVAIL_IND			= MSG_GRP_INT_STK+0x9E,	///<Payload type {::UInt16}

	// Activate/deactivate GPRS context (bearer level communication) result
	MSG_STK_ACTIVATE_GPRS_IND		= MSG_GRP_INT_STK+0x9F,	///<Payload type {::PCHResponseType_t}
	MSG_STK_DEACTIVATE_GPRS_IND		= MSG_GRP_INT_STK+0xA0,	///<Payload type {::PCHResponseType_t}

	// Shutdown link upon no data sent for some time event 
	MSG_STK_SHUTDOWN_LINK_TIMER_IND = MSG_GRP_INT_STK+0xA1,	

	// "Run AT Command" messages internal to platform
	MSG_STK_RUN_AT_REQ				= MSG_GRP_INT_STK+0xB0,	///<Payload type {::RunAT_Request}
	MSG_STK_RUN_AT_RSP				= MSG_GRP_INT_STK+0xB1,	///<Payload type {::RunAT_Response}
	
	// "Run AT Command" notification message posted to ATC/MMI
	MSG_STK_RUN_AT_IND				= MSG_GRP_INT_STK+0xB2,	///<Payload type {::StkRunAtCmd_t}

	/* "Language Notification" message posted to ATC/MMI. After receiving this message, the host/MMI
	 * shall try to use the language specified in this message in the text strings in the proactive command 
	 * response or Envelope command. If no language (i.e. null string) is specified, it means the SIM 
	 * wants to cancel the effect of a previous "Language Notification" (see Section 6.4.25 of ETSI 102.223). 
	 *
	 * Since the USIMAP automatically sends Terminal Response when "Language Notification" is received, 
	 * there is no need for ATC/MMI to send any events back in order to send Terminal Response. 
	 */
	MSG_STK_LANG_NOTIFICATION_IND	= MSG_GRP_INT_STK+0xB3,	///<Payload type {::StkLangNotification_t)}

    MSG_STK_SEND_IPAT_REQ           = MSG_GRP_INT_STK+0xB4, ///<Payload type {::RunAT_Request} [For IpAT Test tool only]
	// End of MSG_GRP_INT_DC, (0x2600)

	

#ifdef INCLUDE_CAPI2_MSG_IDS


	//---------------------------------------------------
	//	CAPI2 message id's 
	// (Reserved 0x3XXX for CAPI2 messages)
	// (Reserved 0x4XXX for CAPI2 Generated messages)
	//---------------------------------------------------
	MSG_CAPI2_MSGIDS_START = 0x3000,		
		
	//---------------------------------------------------
	//	MSG_GRP_CAPI2_LCS, (0x3200)
	//---------------------------------------------------	
	MSG_CAPI2_LCS_REQ_START							 = MSG_GRP_CAPI2_LCS+0x00,

	MSG_LCS_CPMOLRREQ_REQ							 = MSG_GRP_CAPI2_LCS+0x01,
	MSG_LCS_CPMOLRABORT_REQ							 = MSG_GRP_CAPI2_LCS+0x02,
	MSG_LCS_CPMOLRABORT_RSP							 = MSG_GRP_CAPI2_LCS+0x03,
	MSG_LCS_CPMTLRVERIFICATIONRSP_REQ							 = MSG_GRP_CAPI2_LCS+0x04,
	MSG_LCS_CPMTLRVERIFICATIONRSP_RSP							 = MSG_GRP_CAPI2_LCS+0x05,
	MSG_LCS_CPMTLRRSP_REQ							 = MSG_GRP_CAPI2_LCS+0x06,
	MSG_LCS_CPMTLRRSP_RSP							 = MSG_GRP_CAPI2_LCS+0x07,
	MSG_LCS_CPLOCUPDATERSP_REQ							 = MSG_GRP_CAPI2_LCS+0x08,
	MSG_LCS_CPLOCUPDATERSP_RSP							 = MSG_GRP_CAPI2_LCS+0x09,
	MSG_LCS_DECODEPOSESTIMATE_REQ							 = MSG_GRP_CAPI2_LCS+0x0A,
	MSG_LCS_DECODEPOSESTIMATE_RSP							 = MSG_GRP_CAPI2_LCS+0x0B,
	MSG_LCS_ENCODEASSISTANCEREQ_REQ							 = MSG_GRP_CAPI2_LCS+0x0C,
	MSG_LCS_ENCODEASSISTANCEREQ_RSP							 = MSG_GRP_CAPI2_LCS+0x0D,

	MSG_CAPI2_LCS_REQ_END							 = MSG_GRP_CAPI2_LCS+0xFF,
	
	//---------------------------------------------------
	//	MSG_GRP_CAPI2_SMS, (0x3300)
	//---------------------------------------------------	

	MSG_CAPI2_SMS_RESP_START							= MSG_GRP_CAPI2_SMS+0x00,	
	/**
	@n@b Payload: Boolean /// the SMS capacity exceeded flag.
	**/
	MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP			= MSG_GRP_CAPI2_SMS+0x01,	///<Payload type {::Boolean}
	/**
	@n@b Payload: None.
	**/
	MSG_SMS_ISSMSSERVICEAVAIL_RSP						= MSG_GRP_CAPI2_SMS+0x02,	
	/**
	@n@b Payload: SIMSMSMesgStatus_t.
	**/
	MSG_SMS_GETSMSSTOREDSTATE_RSP						= MSG_GRP_CAPI2_SMS+0x03,	///<Payload type {::SIMSMSMesgStatus_t}
	/**
	@n@b Payload: UInt8 ///< Message Reference.
	**/
	MSG_SMS_GETLASTTPMR_RSP								= MSG_GRP_CAPI2_SMS+0x04,	///<Payload type {::UInt8}
	/**
	@n@b Payload: SmsTxParam_t.
	**/
	MSG_SMS_GETSMSTXPARAMS_RSP							= MSG_GRP_CAPI2_SMS+0x05,	///<Payload type {::SmsTxParam_t}
	/**
	@n@b Payload: SmsTxTextModeParms_t
	**/
	MSG_SMS_GETTXPARAMINTEXTMODE_RSP					= MSG_GRP_CAPI2_SMS+0x06,	///<Payload type {::SmsTxTextModeParms_t}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMPROCID_RSP						= MSG_GRP_CAPI2_SMS+0x07,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP					= MSG_GRP_CAPI2_SMS+0x08,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP				= MSG_GRP_CAPI2_SMS+0x09,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP				= MSG_GRP_CAPI2_SMS+0x0A,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP					= MSG_GRP_CAPI2_SMS+0x0B,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP				= MSG_GRP_CAPI2_SMS+0x0C,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP			= MSG_GRP_CAPI2_SMS+0x0D,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSTXPARAMREJDUPL_RSP					= MSG_GRP_CAPI2_SMS+0x0E,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_CHANGESTATUSREQ_RSP							= MSG_GRP_CAPI2_SMS+0x0F,	
	/**
	@n@b Payload: UInt8
	**/
	MSG_SMS_GETNEWMSGDISPLAYPREF_RSP					= MSG_GRP_CAPI2_SMS+0x10,	///<Payload type {::UInt8}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSPREFSTORAGE_RSP						= MSG_GRP_CAPI2_SMS+0x11,	
	/**
	@n@b Payload: SmsStorageStatus_t
	**/
	MSG_SMS_GETSMSSTORAGESTATUS_RSP						= MSG_GRP_CAPI2_SMS+0x12,	///<Payload type {::CAPI2_SMS_GetSMSStorageStatus_t}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SAVESMSSERVICEPROFILE_RSP					= MSG_GRP_CAPI2_SMS+0x13,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_RESTORESMSSERVICEPROFILE_RSP				= MSG_GRP_CAPI2_SMS+0x14,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_CBALLOWALLCHNLREQ_RSP						= MSG_GRP_CAPI2_SMS+0x15,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP					= MSG_GRP_CAPI2_SMS+0x16,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP				= MSG_GRP_CAPI2_SMS+0x17,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP			= MSG_GRP_CAPI2_SMS+0x18,	
	/**
	@n@b Payload: T_MN_CB_LANGUAGES
	**/
	MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP					= MSG_GRP_CAPI2_SMS+0x19,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP				= MSG_GRP_CAPI2_SMS+0x1A,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETCBIGNOREDUPLFLAG_RSP						= MSG_GRP_CAPI2_SMS+0x1B,	
	/**
	@n@b Payload: Boolean
	**/
	MSG_SMS_GETCBIGNOREDUPLFLAG_RSP						= MSG_GRP_CAPI2_SMS+0x1C,	///<Payload type {::UInt8}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETVMINDONOFF_RSP							= MSG_GRP_CAPI2_SMS+0x1D,	
	/**
	@n@b Payload: Boolean - TRUE if enabled.
	**/
	MSG_SMS_ISVMINDENABLED_RSP							= MSG_GRP_CAPI2_SMS+0x1E,	///<Payload type {::Boolean}
	/**
	@n@b Payload: SmsVoicemailInd_t
	**/
	MSG_SMS_GETVMWAITINGSTATUS_RSP						= MSG_GRP_CAPI2_SMS+0x1F,	///<Payload type {::SmsVoicemailInd_t} 
	/**
	@n@b Payload: UInt8
	**/
	MSG_SMS_GETNUMOFVMSCNUMBER_RSP						= MSG_GRP_CAPI2_SMS+0x20,	///<Payload type {::UInt8}
	/**
	@n@b Payload: SmsAddress_t
	**/
	MSG_SMS_GETVMSCNUMBER_RSP							= MSG_GRP_CAPI2_SMS+0x21,	///<Payload type {::SmsAddress_t}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_UPDATEVMSCNUMBERREQ_RSP						= MSG_GRP_CAPI2_SMS+0x22,	
	/**
	@n@b Payload: SMS_BEARER_PREFERENCE_t
	**/
	MSG_SMS_GETSMSBEARERPREFERENCE_RSP					= MSG_GRP_CAPI2_SMS+0x23,	///<Payload type {::SMS_BEARER_PREFERENCE_t}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSBEARERPREFERENCE_RSP					= MSG_GRP_CAPI2_SMS+0x24,	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP				= MSG_GRP_CAPI2_SMS+0x25,	
	/**
	@n@b Payload: Boolean
	**/
	MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP				= MSG_GRP_CAPI2_SMS+0x26,	///<Payload type {::Boolean}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATEREQ_RSP			= MSG_GRP_CAPI2_SMS+0x27,	
	/**
	@n@b Payload: SmsAddress_t
	**/
	MSG_SMS_GETSMSSRVCENTERNUMBER_RSP					= MSG_GRP_CAPI2_SMS+0x28,	///<Payload type {::SmsAddress_t}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETNEWMSGDISPLAYPREF_RSP					= MSG_GRP_CAPI2_SMS+0x29,	
	/**
	@n@b Payload: SmsStorage_t
	**/
	MSG_SMS_GETSMSPREFSTORAGE_RSP						= MSG_GRP_CAPI2_SMS+0x2a,	///<Payload type {::SmsStorage_t}
	/**
	@n@b Payload: SMS_CB_MSG_IDS_t
	**/
	MSG_SMS_GETCBMI_RSP									= MSG_GRP_CAPI2_SMS+0x2b,	///<Payload type {::SMS_CB_MSG_IDS_t}
	/**
	@n@b Payload: T_MN_CB_LANGUAGES
	**/
	MSG_SMS_GETCBLANGUAGE_RSP							= MSG_GRP_CAPI2_SMS+0x2c,	///<Payload type {::T_MN_CB_LANGUAGES}
	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETSMSSTOREDSTATE_RSP						= MSG_GRP_CAPI2_SMS+0x2d,	
	/**
	@n@b Payload: CAPI2_SMS_GetTransactionFromClientID_t
	**/
	MSG_SMS_GETTRANSACTIONFROMCLIENTID_RSP				= MSG_GRP_CAPI2_SMS+0x2e,	///<Payload type {::CAPI2_SMS_GetTransactionFromClientID_t}
	/**
	@n@b Payload: Boolean
	**/
	MSG_SMS_ISCACHEDDATAREADY_RSP						= MSG_GRP_CAPI2_SMS+0x2f,	///<Payload type {::Boolean}
	/**
	@n@b Payload: SmsEnhancedVMInd_t
	**/
	MSG_SMS_GETENHANCEDVMINFOIEI_RSP					= MSG_GRP_CAPI2_SMS+0x30,	///<Payload type {::SmsEnhancedVMInd_t}

	/**
	@n@b Payload: CAPI2_SMS_340AddrToTe_Rsp_t
	**/
	MSG_SMS_340ADDRTOTE_RSP								= MSG_GRP_CAPI2_SMS+0x31,	///<Payload type {::CAPI2_SMS_340AddrToTe_Rsp_t}

	/**
	@n@b Payload: None
	**/
	MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP					= MSG_GRP_CAPI2_SMS+0x32,	
	
	/**
	@n@b Payload: None
	**/
	MSG_SMS_ACKTONETWORK_RSP							= MSG_GRP_CAPI2_SMS+0x33,	

	/**
	@n@b Payload: None
	**/
	MSG_SMS_SMSRESPONSE_RSP								= MSG_GRP_CAPI2_SMS+0x34,	


	MSG_CAPI2_SMS_RESP_END								= MSG_GRP_CAPI2_SMS+0xFF,

	

	//---------------------------------------------------
	// MSG_GRP_CAPI2_PBK, (0x3400)
	//---------------------------------------------------

	MSG_CAPI2_PBK_RESP_START							= MSG_GRP_CAPI2_PBK+0x00,	

	/**
	@n@b Payload: PBK_API_Name_t
	**/
	MSG_PBK_GETALPHA_RSP								= MSG_GRP_CAPI2_PBK+0x01,	///<Payload type {::PBK_API_Name_t}
	/**
	@n@b Payload: Boolean ///< TRUE if the call is an emergency call
	**/
	MSG_PBK_ISEMERGENCYCALLNUMBER_RSP					= MSG_GRP_CAPI2_PBK+0x02,	///<Payload type {::Boolean}
	/**
	@n@b Payload: Boolean ///< TRUE if ready
	**/
	MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP			= MSG_GRP_CAPI2_PBK+0x03,	///<Payload type {::Boolean}
	/**
	@n@b Payload: Boolean. TRUE if number is diallable; FALSE otherwise.
	**/
	MSG_PBK_ISNUMDIALABLE_RSP							= MSG_GRP_CAPI2_PBK+0x04,	///<Payload type {::Boolean}
	/**
	@n@b Payload: Boolean ///< TRUE if USSD is diallable; FALSE otherwise.
	**/
	MSG_PBK_ISUSSDDIALLABLE_RSP							= MSG_GRP_CAPI2_PBK+0x05,	///<Payload type {::Boolean}

	MSG_CAPI2_PBK_RESP_END								= MSG_GRP_CAPI2_PBK+0xFF,
	

	//---------------------------------------------------
	// MSG_GRP_CAPI2_STK, (0x3500)
	//---------------------------------------------------
	
	MSG_CAPI2_SATK_RESP_START							= MSG_GRP_CAPI2_STK+0x00,	

	/**
	@n@b Payload: SetupMenu_t.
	**/
	MSG_SATK_GETCACHEDROOTMENUPTR_RSP					= MSG_GRP_CAPI2_STK+0x01,	///<Payload type {::SetupMenu_t}
	/**
	@n@b Payload: None
	**/
	MSG_SATK_SENDUSERACTIVITYEVENT_RSP					= MSG_GRP_CAPI2_STK+0x02,	
	/**
	@n@b Payload: None
	**/
	MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP				= MSG_GRP_CAPI2_STK+0x03,	
	/**
	@n@b Payload: None
	**/
	MSG_SATK_SENDLANGSELECTEVENT_RSP					= MSG_GRP_CAPI2_STK+0x04,	
	/**
	@n@b Payload: Boolean
	**/
	MSG_SATK_SENDBROWSERTERMEVENT_RSP					= MSG_GRP_CAPI2_STK+0x05,	
	/**
	@n@b Payload: Boolean
	**/
	MSG_SATK_DATASERVCMDRESP_RSP						= MSG_GRP_CAPI2_STK+0x06,	///<Payload type {::Boolean}
	/**
	@n@b Payload: Boolean
	**/
	MSG_SATK_CMDRESP_RSP								= MSG_GRP_CAPI2_STK+0x07,	///<Payload type {::Boolean}
	/**
	@n@b Payload: None
	**/
	MSG_SATK_SETTERMPROFILE_RSP							= MSG_GRP_CAPI2_STK+0x08,	
	/**
	@n@b Payload: CAPI2_TermProfile_t
	**/
	MSG_SATK_GETTERMPROFILE_RSP							= MSG_GRP_CAPI2_STK+0x09,	///<Payload type {::CAPI2_TermProfile_t}

	MSG_CAPI2_SATK_RESP_END								= MSG_GRP_CAPI2_STK+0xFF,


	//---------------------------------------------------
	// MSG_GRP_CAPI2_CC_RESP, (0x3600)
	//---------------------------------------------------

	MSG_CAPI2_CC_RESP_START								= MSG_GRP_CAPI2_CC_RESP+0x00,	

	/**
	@n@b Payload: None 
	**/
	MSG_CC_SENDDTMF_RSP									= MSG_GRP_CAPI2_CC_RESP+0x01,	
	/**
	@n@b Payload: None 
	**/
	MSG_CC_STOPDTMF_RSP									= MSG_GRP_CAPI2_CC_RESP+0x02,	
	/**
	@n@b Payload: None  
	**/
	MSG_CC_MUTECALL_RSP									= MSG_GRP_CAPI2_CC_RESP+0x03,	
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETCURRENTCALLINDEX_RSP						= MSG_GRP_CAPI2_CC_RESP+0x04,	///<Payload type {::UInt8}
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETNEXTACTIVECALLINDEX_RSP					= MSG_GRP_CAPI2_CC_RESP+0x05,	///<Payload type {::UInt8}
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETNEXTHELDCALLINDEX_RSP						= MSG_GRP_CAPI2_CC_RESP+0x06,	///<Payload type {::UInt8}
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETNEXTWAITCALLINDEX_RSP						= MSG_GRP_CAPI2_CC_RESP+0x07,	///<Payload type {::UInt8}
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETMPTYCALLINDEX_RSP							= MSG_GRP_CAPI2_CC_RESP+0x08,	///<Payload type {::UInt8}
	/**
	@n@b Payload: ::CCallState_t 
	**/
	MSG_CC_GETCALLSTATE_RSP								= MSG_GRP_CAPI2_CC_RESP+0x09,	///<Payload type {::CCallState_t}
	/**
	@n@b Payload: ::CCallType_t 
	**/
	MSG_CC_GETCALLTYPE_RSP								= MSG_GRP_CAPI2_CC_RESP+0x0A,	///<Payload type {::CCallType_t}
	/**
	@n@b Payload: ::Cause_t 
	**/
	MSG_CC_GETLASTCALLEXITCAUSE_RSP						= MSG_GRP_CAPI2_CC_RESP+0x0B,	///<Payload type {::Cause_t}
	/**
	@n@b Payload: CNAP_NAME_t 
	**/
	MSG_CC_GETCNAPNAME_RSP								= MSG_GRP_CAPI2_CC_RESP+0x0C,	///<Payload type {::CNAP_NAME_t}
	/**
	@n@b Payload: PHONE_NUMBER_STR_t 
	**/
	MSG_CC_GETCALLNUMBER_RSP							= MSG_GRP_CAPI2_CC_RESP+0x0D,	///<Payload type {::PHONE_NUMBER_STR_t}
#if 0
	/**
	@n@b Payload: None 
	**/
	MSG_CC_SETCALLNUMBER_RSP							= MSG_GRP_CAPI2_CC_RESP+0x0E,	
#endif
	/**
	@n@b Payload: CallingInfo_t 
	**/
	MSG_CC_GETCALLINGINFO_RSP							= MSG_GRP_CAPI2_CC_RESP+0x0F,	///<Payload type {::CallingInfo_t}check
	/**
	@n@b Payload: ALL_CALL_STATE_t 
	**/
	MSG_CC_GETALLCALLSTATES_RSP							= MSG_GRP_CAPI2_CC_RESP+0x10,	///<Payload type {::ALL_CALL_STATE_t}check
	/**
	@n@b Payload: ALL_CALL_INDEX_t 
	**/
	MSG_CC_GETALLCALLINDEX_RSP							= MSG_GRP_CAPI2_CC_RESP+0x11,	///<Payload type {::ALL_CALL_INDEX_t}check
	/**
	@n@b Payload: ALL_CALL_INDEX_t 
	**/
	MSG_CC_GETALLHELDCALLINDEX_RSP						= MSG_GRP_CAPI2_CC_RESP+0x12,	///<Payload type {::ALL_CALL_INDEX_t}check
	/**
	@n@b Payload: ALL_CALL_INDEX_t 
	**/
	MSG_CC_GETALLACTIVECALLINDEX_RSP					= MSG_GRP_CAPI2_CC_RESP+0x13,	///<Payload type {::ALL_CALL_INDEX_t}check
	/**
	@n@b Payload: ALL_CALL_INDEX_t 
	**/
	MSG_CC_GETALLMPTYCALLINDEX_RSP						= MSG_GRP_CAPI2_CC_RESP+0x14,	///<Payload type {::ALL_CALL_INDEX_t}check
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETNUMOFMPTYCALLS_RSP						= MSG_GRP_CAPI2_CC_RESP+0x15,	///<Payload type {::UInt8}check
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETNUMOFACTIVECALLS_RSP						= MSG_GRP_CAPI2_CC_RESP+0x16,	///<Payload type {::UInt8}check
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETNUMOFHELDCALLS_RSP						= MSG_GRP_CAPI2_CC_RESP+0x17,	///<Payload type {::UInt8}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISTHEREWAITINGCALL_RSP						= MSG_GRP_CAPI2_CC_RESP+0x18,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISTHEREALERTINGCALL_RSP						= MSG_GRP_CAPI2_CC_RESP+0x19,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: PHONE_NUMBER_STR_t 
	**/
	MSG_CC_GETCONNECTEDLINEID_RSP						= MSG_GRP_CAPI2_CC_RESP+0x1A,	///<Payload type {::PHONE_NUMBER_STR_t}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISMULTIPARTYCALL_RSP							= MSG_GRP_CAPI2_CC_RESP+0x1B,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISVALIDDTMF_RSP								= MSG_GRP_CAPI2_CC_RESP+0x1C,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISTHEREVOICECALL_RSP							= MSG_GRP_CAPI2_CC_RESP+0x1D,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP			= MSG_GRP_CAPI2_CC_RESP+0x1E,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: UInt32 
	**/
	MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP		= MSG_GRP_CAPI2_CC_RESP+0x1F,	///<Payload type {::UInt32}check	
	/**
	@n@b Payload: UInt32 
	**/
	MSG_CC_GETLASTCALLCCM_RSP							= MSG_GRP_CAPI2_CC_RESP+0x20,	///<Payload type {::UInt32}check
	/**
	@n@b Payload: UInt32 
	**/
	MSG_CC_GETLASTCALLDURATION_RSP						= MSG_GRP_CAPI2_CC_RESP+0x21,	///<Payload type {::UInt32}check
	/**
	@n@b Payload: UInt32 
	**/
	MSG_CC_GETLASTDATACALLRXBYTES_RSP					= MSG_GRP_CAPI2_CC_RESP+0x22,	///<Payload type {::UInt32}check
	/**
	@n@b Payload: UInt32 
	**/
	MSG_CC_GETLASTDATACALLTXBYTES_RSP					= MSG_GRP_CAPI2_CC_RESP+0x23,	///<Payload type {::UInt32}check
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETDATACALLINDEX_RSP							= MSG_GRP_CAPI2_CC_RESP+0x24,	///<Payload type {::UInt8}check
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETCALLCLIENTID_RSP							= MSG_GRP_CAPI2_CC_RESP+0x25,	///<Payload type {::UInt8}check
	/**
	@n@b Payload: UInt8 
	**/
	MSG_CC_GETTYPEADD_RSP								= MSG_GRP_CAPI2_CC_RESP+0x26,	///<Payload type {::UInt8}check
	/**
	@n@b Payload: None 
	**/
	MSG_CC_SETVOICECALLAUTOREJECT_RSP					= MSG_GRP_CAPI2_CC_RESP+0x27,	
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISVOICECALLAUTOREJECT_RSP					= MSG_GRP_CAPI2_CC_RESP+0x28,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: None 
	**/
	MSG_CC_SETTTYCALL_RSP								= MSG_GRP_CAPI2_CC_RESP+0x29,	
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISTTYENABLE_RSP								= MSG_GRP_CAPI2_CC_RESP+0x2A,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: None 
	**/
	MSG_CCAPI_SENDDTMF_RSP								= MSG_GRP_CAPI2_CC_RESP+0x2B,	
	/**
	@n@b Payload: None 
	**/
	MSG_CCAPI_STOPDTMF_RSP								= MSG_GRP_CAPI2_CC_RESP+0x2C,	
	/**
	@n@b Payload: None 
	**/
	MSG_CCAPI_ABORTDTMF_RSP								= MSG_GRP_CAPI2_CC_RESP+0x2D,	
	/**
	@n@b Payload: None 
	**/
	MSG_CCAPI_SETDTMFTIMER_RSP							= MSG_GRP_CAPI2_CC_RESP+0x2E,	
	/**
	@n@b Payload: None 
	**/
	MSG_CCAPI_RESETDTMFTIMER_RSP						= MSG_GRP_CAPI2_CC_RESP+0x2F,	
	/**
	@n@b Payload: Ticks_t 
	**/
	MSG_CCAPI_GETDTMFTIMER_RSP							= MSG_GRP_CAPI2_CC_RESP+0x30,	///<Payload type {::Ticks_t}check
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISSIMORIGINEDCALL_RSP						= MSG_GRP_CAPI2_CC_RESP+0x31,	///<Payload type {::Boolean}check
	/**
	@n@b Payload: None 
	**/
	MSG_CC_SETVIDEOCALLPARAM_RSP						= MSG_GRP_CAPI2_CC_RESP+0x32,	
	/**
	@n@b Payload: VideoCallParam_t 
	**/
	MSG_CC_GETVIDEOCALLPARAM_RSP						= MSG_GRP_CAPI2_CC_RESP+0x33,	///<Payload type {::VideoCallParam_t}check
	/**
	@n@b Payload: CAPI2_CallConfig_t 
	**/
	MSG_CC_GETCALLCFG_RSP							 	= MSG_GRP_CAPI2_CC_RESP+0x34,	///<Payload type {::CAPI2_CallConfig_t}check
	/**
	@n@b Payload: None 
	**/
	MSG_CC_SETCALLCFG_RSP							 	= MSG_GRP_CAPI2_CC_RESP+0x35,	
	/**
	@n@b Payload: None 
	**/
	MSG_CC_RESETCALLCFG_RSP							 	= MSG_GRP_CAPI2_CC_RESP+0x36,	
	/**
	@n@b Payload: Boolean 
	**/
	MSG_CC_ISCURRENTSTATEMPTY_RSP						= MSG_GRP_CAPI2_CC_RESP+0x37,			

	MSG_CAPI2_CC_RESP_END								= MSG_GRP_CAPI2_CC_RESP+0xFF,	


	//---------------------------------------------------
	// MSG_GRP_CAPI2_CC_REQ,  (0x3700)
	// CAPI2 CC Request msgs
	//---------------------------------------------------

	MSG_CAPI2_CC_REQ_START								= MSG_GRP_CAPI2_CC_REQ+0x00,	

	MSG_CC_MAKEVOICECALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x01,	
	MSG_CC_MAKEDATACALL_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x02,	
	MSG_CC_MAKEFAXCALL_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x03,	
	MSG_CC_MAKEVIDEOCALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x04,	
	MSG_CC_ENDCALL_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x05,	
	MSG_CC_ENDALLCALLS_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x06,	
	MSG_CC_ENDMPTYCALLS_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x07,	
	MSG_CC_ENDHELDCALL_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x08,	
	MSG_CC_ACCEPTVOICECALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x09,	
	MSG_CC_ACCEPTDATACALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x0A,	
	MSG_CC_ACCEPTWAITINGCALL_REQ						= MSG_GRP_CAPI2_CC_REQ+0x0B,	
	MSG_CC_ACCEPTVIDEOCALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x0C,	
	MSG_CC_HOLDCURRENTCALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x0D,	
	MSG_CC_HOLDCALL_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x0E,	
	MSG_CC_RETRIEVENEXTHELDCALL_REQ				 		= MSG_GRP_CAPI2_CC_REQ+0x0F,	
	MSG_CC_RETRIEVECALL_REQ						 		= MSG_GRP_CAPI2_CC_REQ+0x10,	
	MSG_CC_SWAPCALL_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x11,	
	MSG_CC_SPLITCALL_REQ						 		= MSG_GRP_CAPI2_CC_REQ+0x12,	
	MSG_CC_JOINCALL_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x13,	
	MSG_CC_TRANSFERCALL_REQ						 		= MSG_GRP_CAPI2_CC_REQ+0x14,	
	MSG_CC_SENDDTMF_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x15,	
	MSG_CC_STOPDTMF_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x16,	
	MSG_CC_MUTECALL_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x17,	
	MSG_CC_GETCURRENTCALLINDEX_REQ				 		= MSG_GRP_CAPI2_CC_REQ+0x18,	
	MSG_CC_GETNEXTACTIVECALLINDEX_REQ			 		= MSG_GRP_CAPI2_CC_REQ+0x19,	
	MSG_CC_GETNEXTHELDCALLINDEX_REQ				 		= MSG_GRP_CAPI2_CC_REQ+0x1A,	
	MSG_CC_GETNEXTWAITCALLINDEX_REQ				 		= MSG_GRP_CAPI2_CC_REQ+0x1B,	
	MSG_CC_GETMPTYCALLINDEX_REQ					 		= MSG_GRP_CAPI2_CC_REQ+0x1C,	
	MSG_CC_GETCALLSTATE_REQ						 		= MSG_GRP_CAPI2_CC_REQ+0x1D,	
	MSG_CC_GETCALLTYPE_REQ						 		= MSG_GRP_CAPI2_CC_REQ+0x1E,	
	MSG_CC_GETLASTCALLEXITCAUSE_REQ				 		= MSG_GRP_CAPI2_CC_REQ+0x1F,	

	MSG_CC_GETCNAPNAME_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x20,	
	MSG_CC_GETCALLNUMBER_REQ							= MSG_GRP_CAPI2_CC_REQ+0x21,	
//	MSG_CC_SETCALLNUMBER_REQ							= MSG_GRP_CAPI2_CC_REQ+0x22,	
	MSG_CC_GETCALLINGINFO_REQ							= MSG_GRP_CAPI2_CC_REQ+0x23,	
	MSG_CC_GETALLCALLSTATES_REQ							= MSG_GRP_CAPI2_CC_REQ+0x24,	
	MSG_CC_GETALLCALLINDEX_REQ							= MSG_GRP_CAPI2_CC_REQ+0x25,	
	MSG_CC_GETALLHELDCALLINDEX_REQ						= MSG_GRP_CAPI2_CC_REQ+0x26,	
	MSG_CC_GETALLACTIVECALLINDEX_REQ					= MSG_GRP_CAPI2_CC_REQ+0x27,	
	MSG_CC_GETALLMPTYCALLINDEX_REQ						= MSG_GRP_CAPI2_CC_REQ+0x28,	
	MSG_CC_GETNUMOFMPTYCALLS_REQ						= MSG_GRP_CAPI2_CC_REQ+0x29,	
	MSG_CC_GETNUMOFACTIVECALLS_REQ						= MSG_GRP_CAPI2_CC_REQ+0x2A,	
	MSG_CC_GETNUMOFHELDCALLS_REQ						= MSG_GRP_CAPI2_CC_REQ+0x2B,	
	MSG_CC_ISTHEREWAITINGCALL_REQ						= MSG_GRP_CAPI2_CC_REQ+0x2C,	
	MSG_CC_ISTHEREALERTINGCALL_REQ						= MSG_GRP_CAPI2_CC_REQ+0x2D,	
	MSG_CC_GETCONNECTEDLINEID_REQ						= MSG_GRP_CAPI2_CC_REQ+0x2E,	
	MSG_CC_ISMULTIPARTYCALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x2F,		

	MSG_CC_ISVALIDDTMF_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x30,	
	MSG_CC_ISTHEREVOICECALL_REQ							= MSG_GRP_CAPI2_CC_REQ+0x31,	
	MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ			= MSG_GRP_CAPI2_CC_REQ+0x32,	
	MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ		= MSG_GRP_CAPI2_CC_REQ+0x33,	
	MSG_CC_GETLASTCALLCCM_REQ							= MSG_GRP_CAPI2_CC_REQ+0x34,	
	MSG_CC_GETLASTCALLDURATION_REQ						= MSG_GRP_CAPI2_CC_REQ+0x35,	
	MSG_CC_GETLASTDATACALLRXBYTES_REQ					= MSG_GRP_CAPI2_CC_REQ+0x36,	
	MSG_CC_GETLASTDATACALLTXBYTES_REQ					= MSG_GRP_CAPI2_CC_REQ+0x37,	
				
	MSG_CC_GETDATACALLINDEX_REQ							= MSG_GRP_CAPI2_CC_REQ+0x38,	
	MSG_CC_GETCALLCLIENTID_REQ							= MSG_GRP_CAPI2_CC_REQ+0x39,	
	MSG_CC_GETTYPEADD_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x3A,	
	MSG_CC_SETVOICECALLAUTOREJECT_REQ					= MSG_GRP_CAPI2_CC_REQ+0x3B,	

	MSG_CC_ISVOICECALLAUTOREJECT_REQ					= MSG_GRP_CAPI2_CC_REQ+0x3C,	
	MSG_CC_SETTTYCALL_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x3D,	

	MSG_CC_ISTTYENABLE_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x3E,	
	MSG_CC_ISSIMORIGINEDCALL_REQ						= MSG_GRP_CAPI2_CC_REQ+0x3F,	
	MSG_CC_SETVIDEOCALLPARAM_REQ						= MSG_GRP_CAPI2_CC_REQ+0x40,	
	MSG_CC_GETVIDEOCALLPARAM_REQ						= MSG_GRP_CAPI2_CC_REQ+0x41,	
	MSG_CC_GETCALLCFG_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x42,	
	MSG_CC_SETCALLCFG_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x43,	
	MSG_CC_RESETCALLCFG_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0x44,	
	MSG_CC_GETCALLINDEXINTHISSTATE_REQ					= MSG_GRP_CAPI2_CC_REQ+0x45,	
	MSG_CC_GETCALLINDEXINTHISSTATE_RSP					= MSG_GRP_CAPI2_CC_REQ+0x46,	
	MSG_CC_GETCCM_REQ							 		= MSG_GRP_CAPI2_CC_REQ+0x47,	
	MSG_CC_GETCCM_RSP							 		= MSG_GRP_CAPI2_CC_REQ+0x48,	
	MSG_CC_GETCALLPRESENT_REQ							= MSG_GRP_CAPI2_CC_REQ+0x49,	
	MSG_CC_GETCALLPRESENT_RSP							= MSG_GRP_CAPI2_CC_REQ+0x4A,	

	MSG_CCAPI_SENDDTMF_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0xa0,	
	MSG_CCAPI_STOPDTMF_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0xa1,	
	MSG_CCAPI_ABORTDTMF_REQ							 	= MSG_GRP_CAPI2_CC_REQ+0xa2,	
	MSG_CCAPI_SETDTMFTIMER_REQ						 	= MSG_GRP_CAPI2_CC_REQ+0xa3,	
	MSG_CCAPI_RESETDTMFTIMER_REQ					 	= MSG_GRP_CAPI2_CC_REQ+0xa4,	
	MSG_CCAPI_GETDTMFTIMER_REQ						 	= MSG_GRP_CAPI2_CC_REQ+0xa5,	
	MSG_CC_ISCURRENTSTATEMPTY_REQ						= MSG_GRP_CAPI2_CC_REQ+0xa6,

	MSG_CAPI2_CC_REQ_END								= MSG_GRP_CAPI2_CC_REQ+0xFF,	

	
	//---------------------------------------------------
	// MSG_GRP_CAPI2_INT_PBK,  (0x3800)
	//---------------------------------------------------

	MSG_CAP2_INT_PBK_START								= MSG_GRP_CAPI2_INT_PBK+0x00,	
	MSG_PBK_GETALPHA_REQ								= MSG_GRP_CAPI2_INT_PBK+0x01,	
	MSG_PBK_ISEMERGENCYCALLNUMBER_REQ					= MSG_GRP_CAPI2_INT_PBK+0x02,	
	MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ			= MSG_GRP_CAPI2_INT_PBK+0x03,	
	MSG_PBK_SENDINFOREQ_REQ								= MSG_GRP_CAPI2_INT_PBK+0x04,	
	MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ				= MSG_GRP_CAPI2_INT_PBK+0x05,	
	MSG_SENDFINDALPHAMATCHONEREQ_REQ					= MSG_GRP_CAPI2_INT_PBK+0x06,	
	MSG_PBK_ISREADY_RSP									= MSG_GRP_CAPI2_INT_PBK+0x07,	///<Payload type {::Boolean}
	MSG_SENDREADENTRYREQ_REQ							= MSG_GRP_CAPI2_INT_PBK+0x08,	
	MSG_PBK_SENDWRITEENTRYREQ_REQ						= MSG_GRP_CAPI2_INT_PBK+0x09,	
	MSG_PBK_SENDISNUMDIALLABLEREQ						= MSG_GRP_CAPI2_INT_PBK+0x0a,	
	MSG_PBK_ISNUMDIALLABLE_REQ							= MSG_GRP_CAPI2_INT_PBK+0x0b,	
	MSG_PBK_ISUSSDDIALLABLE_REQ							= MSG_GRP_CAPI2_INT_PBK+0x0c,	
	MSG_PBK_ISREADY_REQ									= MSG_GRP_CAPI2_INT_PBK+0x0d,	
	MSG_PBK_SENDUPDATEENTRYREQ_REQ                      = MSG_GRP_CAPI2_INT_PBK+0x0e, 	
	MSG_CAP2_INT_PBK_END								= MSG_GRP_CAPI2_INT_PBK+0xff,	


	//---------------------------------------------------
	// MSG_GRP_CAPI2_INT_SMS,  (0x3900)
	//---------------------------------------------------

	MSG_CAP2_INT_SMS_START								= MSG_GRP_CAPI2_INT_SMS+0x00,	
	MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ			= MSG_GRP_CAPI2_INT_SMS+0x01,	
	MSG_SMS_GETSMSSRVCENTERNUMBER_REQ					= MSG_GRP_CAPI2_INT_SMS+0x02,	
	MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ			= MSG_GRP_CAPI2_INT_SMS+0x03,	
	MSG_SMS_ISSMSSERVICEAVAIL_REQ						= MSG_GRP_CAPI2_INT_SMS+0x04,	
	MSG_SMS_GETSMSSTOREDSTATE_REQ						= MSG_GRP_CAPI2_INT_SMS+0x05,	
	MSG_SMS_WRITESMSPDU_REQ								= MSG_GRP_CAPI2_INT_SMS+0x06,	
	MSG_SMS_WRITESMSREQ_REQ								= MSG_GRP_CAPI2_INT_SMS+0x07,	
	MSG_SMS_SENDSMSREQ_REQ								= MSG_GRP_CAPI2_INT_SMS+0x08,	
	MSG_SMS_SENDSMSPDUREQ_REQ							= MSG_GRP_CAPI2_INT_SMS+0x09,	
	MSG_SMS_SENDSTOREDSMSREQ_REQ						= MSG_GRP_CAPI2_INT_SMS+0x0a,	
	MSG_SMS_WRITESMSPDUTOSIM_REQ						= MSG_GRP_CAPI2_INT_SMS+0x0b,	
	MSG_SMS_GETLASTTPMR_REQ								= MSG_GRP_CAPI2_INT_SMS+0x0c,	
	MSG_SMS_GETSMSTXPARAMS_REQ							= MSG_GRP_CAPI2_INT_SMS+0x0d,	
	MSG_SMS_GETTXPARAMINTEXTMODE_REQ					= MSG_GRP_CAPI2_INT_SMS+0x0e,	
	MSG_SMS_SETSMSTXPARAMPROCID_REQ						= MSG_GRP_CAPI2_INT_SMS+0x0f,	
	MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ					= MSG_GRP_CAPI2_INT_SMS+0x10,	
	MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ				= MSG_GRP_CAPI2_INT_SMS+0x11,	
	MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ				= MSG_GRP_CAPI2_INT_SMS+0x12,	
	MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ					= MSG_GRP_CAPI2_INT_SMS+0x13,	
	MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ				= MSG_GRP_CAPI2_INT_SMS+0x14,	
	MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ			= MSG_GRP_CAPI2_INT_SMS+0x15,	
	MSG_SMS_SETSMSTXPARAMREJDUPL_REQ					= MSG_GRP_CAPI2_INT_SMS+0x16,	
	MSG_SMS_DELETESMSMSGBYINDEX_REQ						= MSG_GRP_CAPI2_INT_SMS+0x17,	
	MSG_SMS_READSMSMSG_REQ								= MSG_GRP_CAPI2_INT_SMS+0x18,	
	MSG_SMS_LISTSMSMSG_REQ								= MSG_GRP_CAPI2_INT_SMS+0x19,	
	MSG_SMS_SETNEWMSGDISPLAYPREF_REQ					= MSG_GRP_CAPI2_INT_SMS+0x1a,	
	MSG_SMS_GETNEWMSGDISPLAYPREF_REQ					= MSG_GRP_CAPI2_INT_SMS+0x1b,	
	MSG_SMS_SETSMSPREFSTORAGE_REQ						= MSG_GRP_CAPI2_INT_SMS+0x1c,	
	MSG_SMS_GETSMSPREFSTORAGE_REQ						= MSG_GRP_CAPI2_INT_SMS+0x1d,	
	MSG_SMS_GETSMSSTORAGESTATUS_REQ						= MSG_GRP_CAPI2_INT_SMS+0x1e,	
	MSG_SMS_SAVESMSSERVICEPROFILE_REQ					= MSG_GRP_CAPI2_INT_SMS+0x1f,	
	MSG_SMS_RESTORESMSSERVICEPROFILE_REQ				= MSG_GRP_CAPI2_INT_SMS+0x20,	
	MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ					= MSG_GRP_CAPI2_INT_SMS+0x21,	
	MSG_SMS_CBALLOWALLCHNLREQ_REQ						= MSG_GRP_CAPI2_INT_SMS+0x22,	
	MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ					= MSG_GRP_CAPI2_INT_SMS+0x23,	
	MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ				= MSG_GRP_CAPI2_INT_SMS+0x24,	
	MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ			= MSG_GRP_CAPI2_INT_SMS+0x25,	
	MSG_SMS_GETCBMI_REQ									= MSG_GRP_CAPI2_INT_SMS+0x26,	
	MSG_SMS_GETCBLANGUAGE_REQ							= MSG_GRP_CAPI2_INT_SMS+0x27,	
	MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ					= MSG_GRP_CAPI2_INT_SMS+0x28,	
	MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ				= MSG_GRP_CAPI2_INT_SMS+0x29,	
	MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ				= MSG_GRP_CAPI2_INT_SMS+0x2a,	
	MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ				= MSG_GRP_CAPI2_INT_SMS+0x2b,	
	MSG_SMS_SETCBIGNOREDUPLFLAG_REQ						= MSG_GRP_CAPI2_INT_SMS+0x2c,	
	MSG_SMS_GETCBIGNOREDUPLFLAG_REQ						= MSG_GRP_CAPI2_INT_SMS+0x2d,	
	MSG_SMS_SETVMINDONOFF_REQ							= MSG_GRP_CAPI2_INT_SMS+0x2e,	
	MSG_SMS_ISVMINDENABLED_REQ							= MSG_GRP_CAPI2_INT_SMS+0x2f,	
	MSG_SMS_GETVMWAITINGSTATUS_REQ						= MSG_GRP_CAPI2_INT_SMS+0x30,	
	MSG_SMS_GETNUMOFVMSCNUMBER_REQ						= MSG_GRP_CAPI2_INT_SMS+0x31,	
	MSG_SMS_GETVMSCNUMBER_REQ							= MSG_GRP_CAPI2_INT_SMS+0x32,	
	MSG_SMS_UPDATEVMSCNUMBERREQ_REQ						= MSG_GRP_CAPI2_INT_SMS+0x33,	
	MSG_SMS_GETSMSBEARERPREFERENCE_REQ					= MSG_GRP_CAPI2_INT_SMS+0x34,	
	MSG_SMS_SETSMSBEARERPREFERENCE_REQ					= MSG_GRP_CAPI2_INT_SMS+0x35,	
	MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ				= MSG_GRP_CAPI2_INT_SMS+0x36,	
	MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ				= MSG_GRP_CAPI2_INT_SMS+0x37,	
	MSG_SMS_CHANGESTATUSREQ_REQ							= MSG_GRP_CAPI2_INT_SMS+0x38,	
	MSG_SMS_SENDMESTOREDSTATUSIND_REQ					= MSG_GRP_CAPI2_INT_SMS+0x39,	
	MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ				= MSG_GRP_CAPI2_INT_SMS+0x3a,	
	MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ					= MSG_GRP_CAPI2_INT_SMS+0x3b,	
	MSG_SMS_SETSMSSTOREDSTATE_REQ						= MSG_GRP_CAPI2_INT_SMS+0x3c,	
	MSG_SMS_GETTRANSACTIONFROMCLIENTID_REQ				= MSG_GRP_CAPI2_INT_SMS+0x3d,	
	MSG_SMS_ISCACHEDDATAREADY_REQ						= MSG_GRP_CAPI2_INT_SMS+0x3e,	
	MSG_SMS_GETENHANCEDVMINFOIEI_REQ					= MSG_GRP_CAPI2_INT_SMS+0x3f,	
	MSG_SMS_340ADDRTOTE_REQ								= MSG_GRP_CAPI2_INT_SMS+0x40,	
	MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ					= MSG_GRP_CAPI2_INT_SMS+0x41,	
	MSG_SMS_ACKTONETWORK_REQ							= MSG_GRP_CAPI2_INT_SMS+0x42,	
	MSG_SMS_SMSRESPONSE_REQ								= MSG_GRP_CAPI2_INT_SMS+0x43,	
	MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP				= MSG_GRP_CAPI2_INT_SMS+0x44,	
	MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP					= MSG_GRP_CAPI2_INT_SMS+0x45,	
	MSG_SMS_SENDMESTOREDSTATUSIND_RSP					= MSG_GRP_CAPI2_INT_SMS+0x46,	
	MSG_CAP2_INT_SMS_END								= MSG_GRP_CAPI2_INT_SMS+0xff,	

	//---------------------------------------------------
	// MSG_GRP_CAPI2_INT_STK,  (0x3A00)
	//---------------------------------------------------
	
	MSG_CAP2_INT_STK_START								= MSG_GRP_CAPI2_INT_STK+0x00,	
	MSG_SATK_GETCACHEDROOTMENUPTR_REQ					= MSG_GRP_CAPI2_INT_STK+0x01,	
	MSG_SATK_SENDUSERACTIVITYEVENT_REQ					= MSG_GRP_CAPI2_INT_STK+0x02,	
	MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ				= MSG_GRP_CAPI2_INT_STK+0x03,	
	MSG_SATK_SENDBROWSERTERMEVENT_REQ					= MSG_GRP_CAPI2_INT_STK+0x04,	
	MSG_SATK_CMDRESP_REQ								= MSG_GRP_CAPI2_INT_STK+0x05,	
	MSG_SATK_DATASERVCMDRESP_REQ						= MSG_GRP_CAPI2_INT_STK+0x06,	
	MSG_SATK_SENDLANGSELECTEVENT_REQ					= MSG_GRP_CAPI2_INT_STK+0x07,	
	MSG_SATK_SETTERMPROFILE_REQ							= MSG_GRP_CAPI2_INT_STK+0x08,	
	MSG_SATK_GETTERMPROFILE_REQ							= MSG_GRP_CAPI2_INT_STK+0x09,
    MSG_SATK_SEND_ENVELOPE_CMD_REQ                      = MSG_GRP_CAPI2_INT_STK+0x0A,
    MSG_STK_TERMINAL_RESPONSE_REQ                       = MSG_GRP_CAPI2_INT_STK+0x0B,
    MSG_SATK_SEND_CC_SETUP_REQ                          = MSG_GRP_CAPI2_INT_STK+0x0C,
    MSG_SATK_SEND_CC_SS_REQ                             = MSG_GRP_CAPI2_INT_STK+0x0D,
    MSG_SATK_SEND_CC_USSD_REQ                           = MSG_GRP_CAPI2_INT_STK+0x0E,
    MSG_SATK_SEND_CC_SMS_REQ                            = MSG_GRP_CAPI2_INT_STK+0x0F,
	MSG_CAP2_INT_STK_END								= MSG_GRP_CAPI2_INT_STK+0xff,	
	
	//---------------------------------------------------
	// MSG_GRP_CAPI2_SS,  (0x3B00)
	// CAPI2 SS Request msgs
	//---------------------------------------------------

	MSG_CAPI2_SS_REQ_START								= MSG_GRP_CAPI2_SS+0x00,	

	MSG_SS_SENDCALLFORWARDREQ_REQ						= MSG_GRP_CAPI2_SS+0x01,	
	MSG_SS_QUERYCALLFORWARDSTATUS_REQ					= MSG_GRP_CAPI2_SS+0x02,	
	MSG_SS_SENDCALLBARRINGREQ_REQ						= MSG_GRP_CAPI2_SS+0x03,	
	MSG_SS_QUERYCALLBARRINGSTATUS_REQ					= MSG_GRP_CAPI2_SS+0x04,	
	MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ				= MSG_GRP_CAPI2_SS+0x05,	
	MSG_SS_SENDCALLWAITINGREQ_REQ						= MSG_GRP_CAPI2_SS+0x06,	
	MSG_SS_QUERYCALLWAITINGSTATUS_REQ					= MSG_GRP_CAPI2_SS+0x07,	
	MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ					= MSG_GRP_CAPI2_SS+0x08,	
	MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ				= MSG_GRP_CAPI2_SS+0x09,	
	MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ		= MSG_GRP_CAPI2_SS+0x0A,	
	MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ		= MSG_GRP_CAPI2_SS+0x0B,	
	MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ			= MSG_GRP_CAPI2_SS+0x0C,	
	MSG_SS_SENDUSSDCONNECTREQ_REQ						= MSG_GRP_CAPI2_SS+0x0D,	
	MSG_SS_ENDUSSDCONNECTREQ_REQ						= MSG_GRP_CAPI2_SS+0x0E,	
	MSG_SS_SENDUSSDDATA_REQ							 	= MSG_GRP_CAPI2_SS+0x0F,	
	MSG_SS_SETCALLINGLINEIDSTATUS_REQ					= MSG_GRP_CAPI2_SS+0x10,	
	MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ			= MSG_GRP_CAPI2_SS+0x11,	
	MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ					= MSG_GRP_CAPI2_SS+0x12,	
	MSG_SS_RESETSSALSFLAG_REQ							= MSG_GRP_CAPI2_SS+0x13,	
	MSG_SS_RESETSSALSFLAG_RSP							= MSG_GRP_CAPI2_SS+0x14,	
	MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ		= MSG_GRP_CAPI2_SS+0x15,	
	MSG_SS_ENDUSSDCONNECTREQ_RSP						= MSG_GRP_CAPI2_SS+0x16,	

	MSG_SSAPI_DIALSTRSRVREQ_REQ							= MSG_GRP_CAPI2_SS+0x17,	
	MSG_SSAPI_DIALSTRSRVREQ_RSP							= MSG_GRP_CAPI2_SS+0x18,	

	MSG_SSAPI_SSSRVREQ_REQ							 = MSG_GRP_CAPI2_SS+0x19,
	MSG_SSAPI_SSSRVREQ_RSP							 = MSG_GRP_CAPI2_SS+0x1A,
	MSG_SSAPI_USSDSRVREQ_REQ							 = MSG_GRP_CAPI2_SS+0x1B,
	MSG_SSAPI_USSDSRVREQ_RSP							 = MSG_GRP_CAPI2_SS+0x1C,
	MSG_SSAPI_USSDDATAREQ_REQ							 = MSG_GRP_CAPI2_SS+0x1D,
	MSG_SSAPI_USSDDATAREQ_RSP							 = MSG_GRP_CAPI2_SS+0x1E,
	MSG_SSAPI_SSRELEASEREQ_REQ							 = MSG_GRP_CAPI2_SS+0x1F,
	MSG_SSAPI_SSRELEASEREQ_RSP							 = MSG_GRP_CAPI2_SS+0x20,
	MSG_SSAPI_DATAREQ_REQ							 = MSG_GRP_CAPI2_SS+0x21,
	MSG_SSAPI_DATAREQ_RSP							 = MSG_GRP_CAPI2_SS+0x22,
	
	MSG_CAPI2_SS_REQ_END								= MSG_GRP_CAPI2_SS+0xFF,	


	//---------------------------------------------------
	// MSG_GRP_CAPI2_SIM			(0x3C00)
	// CAPI2 ISIM Request/Response msgs
	//---------------------------------------------------
	
	MSG_CAPI2_ISIM_REQ_START							= MSG_GRP_CAPI2_SIM+0x50,	

	MSG_ISIM_ISISIMSUPPORTED_REQ						= MSG_GRP_CAPI2_SIM+0x51,	
	MSG_ISIM_ISISIMSUPPORTED_RSP						= MSG_GRP_CAPI2_SIM+0x52,	///<Payload type {::Boolean}check
	MSG_ISIM_ISISIMACTIVATED_REQ						= MSG_GRP_CAPI2_SIM+0x53,	
	MSG_ISIM_ISISIMACTIVATED_RSP						= MSG_GRP_CAPI2_SIM+0x54,	///<Payload type {::UInt8}
	MSG_ISIM_ACTIVATEISIMAPPLI_REQ						= MSG_GRP_CAPI2_SIM+0x55,	
	MSG_ISIM_SENDAUTHENAKAREQ_REQ						= MSG_GRP_CAPI2_SIM+0x56,	
	MSG_ISIM_SENDAUTHENHTTPREQ_REQ						= MSG_GRP_CAPI2_SIM+0x57,	
	MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ					= MSG_GRP_CAPI2_SIM+0x58,	
	MSG_ISIM_SENDAUTHENGBANAFREQ_REQ					= MSG_GRP_CAPI2_SIM+0x59,	

	MSG_CAPI2_ISIM_REQ_END								= MSG_GRP_CAPI2_SIM+0xFF,	

	//---------------------------------------------------
	//  MSG_GRP_CAPI2_PCH			(0x3D00)
	//   CAPI2 PCH Request msgs                             
	//---------------------------------------------------
	MSG_CAPI2_PCH_REQ_START								= MSG_GRP_CAPI2_PCH+0x00,	

	MSG_PDP_GETPDPCONTEXT_REQ							= MSG_GRP_CAPI2_PCH+0x01,	
	MSG_PDP_GETPDPCONTEXT_RSP							= MSG_GRP_CAPI2_PCH+0x02,	///<Payload type {::PDPContext_t}check
	MSG_PDP_SETPDPCONTEXT_REQ							= MSG_GRP_CAPI2_PCH+0x03,	
	MSG_PDP_SETPDPCONTEXT_RSP							= MSG_GRP_CAPI2_PCH+0x04,	
	MSG_PDP_DELETEPDPCONTEXT_REQ						= MSG_GRP_CAPI2_PCH+0x05,	
	MSG_PDP_DELETEPDPCONTEXT_RSP						= MSG_GRP_CAPI2_PCH+0x06,	
	MSG_PDP_SETSECPDPCONTEXT_REQ						= MSG_GRP_CAPI2_PCH+0x07,	
	MSG_PDP_SETSECPDPCONTEXT_RSP						= MSG_GRP_CAPI2_PCH+0x08,	
	MSG_PDP_GETGPRSQOS_REQ							 	= MSG_GRP_CAPI2_PCH+0x09,	
	MSG_PDP_GETGPRSQOS_RSP							 	= MSG_GRP_CAPI2_PCH+0x0A,	///<Payload type {::PCHQosProfile_t}check
	MSG_PDP_SETGPRSQOS_REQ							 	= MSG_GRP_CAPI2_PCH+0x0B,	
	MSG_PDP_SETGPRSQOS_RSP							 	= MSG_GRP_CAPI2_PCH+0x0C,	
	MSG_PDP_DELETEGPRSQOS_REQ							= MSG_GRP_CAPI2_PCH+0x0D,	
	MSG_PDP_DELETEGPRSQOS_RSP							= MSG_GRP_CAPI2_PCH+0x0E,	
	MSG_PDP_GETGPRSMINQOS_REQ							= MSG_GRP_CAPI2_PCH+0x0F,	
	MSG_PDP_GETGPRSMINQOS_RSP							= MSG_GRP_CAPI2_PCH+0x10,	///<Payload type {::PDP_GPRSMinQoS_t}
	MSG_PDP_SETGPRSMINQOS_REQ							= MSG_GRP_CAPI2_PCH+0x11,	
	MSG_PDP_SETGPRSMINQOS_RSP							= MSG_GRP_CAPI2_PCH+0x12,	
	MSG_PDP_DELETEGPRSMINQOS_REQ						= MSG_GRP_CAPI2_PCH+0x13,	
	MSG_PDP_DELETEGPRSMINQOS_RSP						= MSG_GRP_CAPI2_PCH+0x14,	
	MSG_PDP_ISSECONDARYPDPDEFINED_REQ					= MSG_GRP_CAPI2_PCH+0x15,	
	MSG_PDP_ISSECONDARYPDPDEFINED_RSP					= MSG_GRP_CAPI2_PCH+0x16,	///<Payload type {::Boolean}
	MSG_PDP_SENDPDPACTIVATEREQ_REQ						= MSG_GRP_CAPI2_PCH+0x17,	
//	MSG_PDP_SENDPDPACTIVATEREQ_RSP						= MSG_GRP_CAPI2_PCH+0x18,	
	MSG_PDP_SENDPDPDEACTIVATEREQ_REQ					= MSG_GRP_CAPI2_PCH+0x19,	
//	MSG_PDP_SENDPDPDEACTIVATEREQ_RSP					= MSG_GRP_CAPI2_PCH+0x1A,	
	MSG_PDP_SENDPDPACTIVATESECREQ_REQ					= MSG_GRP_CAPI2_PCH+0x1B,	
//	MSG_PDP_SENDPDPACTIVATESECREQ_RSP					= MSG_GRP_CAPI2_PCH+0x1C,	
	MSG_PDP_GETGPRSACTIVATESTATUS_REQ					= MSG_GRP_CAPI2_PCH+0x1D,	
	MSG_PDP_GETGPRSACTIVATESTATUS_RSP					= MSG_GRP_CAPI2_PCH+0x1E,	///<Payload type {::PDP_GetGPRSActivateStatus_Rsp_t}
	MSG_PDP_SETMSCLASS_REQ							 	= MSG_GRP_CAPI2_PCH+0x1F,	
	MSG_PDP_SETMSCLASS_RSP							 	= MSG_GRP_CAPI2_PCH+0x20,	
	MSG_PDP_GETMSCLASS_REQ							 	= MSG_GRP_CAPI2_PCH+0x21,	
	MSG_PDP_GETMSCLASS_RSP							 	= MSG_GRP_CAPI2_PCH+0x22,	///<Payload type {::MSClass_t}
	MSG_PDP_GETUMTSTFT_REQ							 	= MSG_GRP_CAPI2_PCH+0x23,	
	MSG_PDP_GETUMTSTFT_RSP							 	= MSG_GRP_CAPI2_PCH+0x24,	///<Payload type {::PCHTrafficFlowTemplate_t}
	MSG_PDP_SETUMTSTFT_REQ							 	= MSG_GRP_CAPI2_PCH+0x25,	
	MSG_PDP_SETUMTSTFT_RSP							 	= MSG_GRP_CAPI2_PCH+0x26,	
	MSG_PDP_DELETEUMTSTFT_REQ							= MSG_GRP_CAPI2_PCH+0x27,	
	MSG_PDP_DELETEUMTSTFT_RSP							= MSG_GRP_CAPI2_PCH+0x28,	
	MSG_PDP_ACTIVATESNDCPCONNECTION_REQ					= MSG_GRP_CAPI2_PCH+0x29,	
//	MSG_PDP_ACTIVATE_SNDCP_RSP							= MSG_GRP_CAPI2_PCH+0x2A,
	MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ				= MSG_GRP_CAPI2_PCH+0x2B,	
	MSG_PDP_DEACTIVATE_SNDCP_RSP						= MSG_GRP_CAPI2_PCH+0x2C,	
	MSG_PDP_GETPDPDEFAULTCONTEXT_REQ					= MSG_GRP_CAPI2_PCH+0x2D,	
	MSG_PDP_GETPDPDEFAULTCONTEXT_RSP					= MSG_GRP_CAPI2_PCH+0x2E,	///<Payload type {::PDPDefaultContext_t}check
	MSG_PDP_GETPCHCONTEXT_REQ							= MSG_GRP_CAPI2_PCH+0x2F,	
	MSG_PDP_GETPCHCONTEXT_RSP							= MSG_GRP_CAPI2_PCH+0x30,	///<Payload type {::PDP_GetPCHContext_Rsp_t}check
	MSG_PDP_GETPCHCONTEXTSTATE_REQ						= MSG_GRP_CAPI2_PCH+0x31,	///<Payload type {::PCHContextState_t}check
	MSG_PDP_GETPCHCONTEXTSTATE_RSP						= MSG_GRP_CAPI2_PCH+0x32,	///<Payload type {::PCHContextState_t}check



	MSG_MS_SENDCOMBINEDATTACHREQ_REQ					= MSG_GRP_CAPI2_PCH+0x51,	
	MSG_MS_SENDCOMBINEDATTACHREQ_RSP					= MSG_GRP_CAPI2_PCH+0x52,	
	MSG_MS_SENDDETACHREQ_REQ							= MSG_GRP_CAPI2_PCH+0x53,	
	MSG_MS_SENDDETACHREQ_RSP							= MSG_GRP_CAPI2_PCH+0x54,	
	MSG_MS_GETGPRSATTACHSTATUS_REQ						= MSG_GRP_CAPI2_PCH+0x55,	
	MSG_MS_GETGPRSATTACHSTATUS_RSP						= MSG_GRP_CAPI2_PCH+0x56,	///<Payload type {::AttachState_t}
	MSG_MS_SETATTACHMODE_REQ							= MSG_GRP_CAPI2_PCH+0x57,	
	MSG_MS_SETATTACHMODE_RSP							= MSG_GRP_CAPI2_PCH+0x58,	
	MSG_MS_GETATTACHMODE_REQ							= MSG_GRP_CAPI2_PCH+0x59,	
	MSG_MS_GETATTACHMODE_RSP							= MSG_GRP_CAPI2_PCH+0x5A,	///<Payload type {::UInt8}
	MSG_MS_ISGPRSCALLACTIVE_REQ							= MSG_GRP_CAPI2_PCH+0x5B,	
	MSG_MS_ISGPRSCALLACTIVE_RSP							= MSG_GRP_CAPI2_PCH+0x5C,	///<Payload type {::Boolean}
	MSG_MS_SETCHANGPRSCALLACTIVE_REQ					= MSG_GRP_CAPI2_PCH+0x5D,	
	MSG_MS_SETCHANGPRSCALLACTIVE_RSP					= MSG_GRP_CAPI2_PCH+0x5E,	
	MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ					= MSG_GRP_CAPI2_PCH+0x5F,	
	MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP					= MSG_GRP_CAPI2_PCH+0x60,	
	MSG_MS_GETGPRSACTIVECHANFROMCID_REQ					= MSG_GRP_CAPI2_PCH+0x61,	
	MSG_MS_GETGPRSACTIVECHANFROMCID_RSP					= MSG_GRP_CAPI2_PCH+0x62,	///<Payload type {::UInt8}
	MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ					= MSG_GRP_CAPI2_PCH+0x63,	
	MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP					= MSG_GRP_CAPI2_PCH+0x64,	///<Payload type {::UInt8}
	MSG_PDP_GETPPPMODEMCID_REQ							= MSG_GRP_CAPI2_PCH+0x65,	
	MSG_PDP_GETPPPMODEMCID_RSP							= MSG_GRP_CAPI2_PCH+0x66,	///<Payload type {::PCHCid_t}
	MSG_PDP_GETPDPADDRESS_REQ							= MSG_GRP_CAPI2_PCH+0x67,	
	MSG_PDP_GETPDPADDRESS_RSP							= MSG_GRP_CAPI2_PCH+0x68,	///<Payload type {::PCHPDPAddress_t}
	MSG_PDP_GETR99UMTSMINQOS_REQ						= MSG_GRP_CAPI2_PCH+0x69,	
	MSG_PDP_GETR99UMTSMINQOS_RSP						= MSG_GRP_CAPI2_PCH+0x6A,	///<Payload type {::PCHR99QosProfile_t}
	MSG_PDP_GETR99UMTSQOS_REQ							= MSG_GRP_CAPI2_PCH+0x6B,	
	MSG_PDP_GETR99UMTSQOS_RSP							= MSG_GRP_CAPI2_PCH+0x6C,	///<Payload type {::PCHR99QosProfile_t}
	MSG_PDP_SENDTBFDATA_REQ							 	= MSG_GRP_CAPI2_PCH+0x6D,	
	MSG_PDP_SENDTBFDATA_RSP							 	= MSG_GRP_CAPI2_PCH+0x6E,	
	MSG_PDP_SETR99UMTSMINQOS_REQ						= MSG_GRP_CAPI2_PCH+0x6F,	
	MSG_PDP_SETR99UMTSMINQOS_RSP						= MSG_GRP_CAPI2_PCH+0x70,	
	MSG_PDP_SETR99UMTSQOS_REQ							= MSG_GRP_CAPI2_PCH+0x71,	
	MSG_PDP_SETR99UMTSQOS_RSP							= MSG_GRP_CAPI2_PCH+0x72,	
	MSG_PDP_TFTADDFILTER_REQ							= MSG_GRP_CAPI2_PCH+0x73,	
	MSG_PDP_TFTADDFILTER_RSP							= MSG_GRP_CAPI2_PCH+0x74,	
	MSG_PDP_SETPCHCONTEXTSTATE_REQ						= MSG_GRP_CAPI2_PCH+0x75,	
	MSG_PDP_SETPCHCONTEXTSTATE_RSP						= MSG_GRP_CAPI2_PCH+0x76,	
	MSG_PDP_SETDEFAULTPDPCONTEXT_REQ					= MSG_GRP_CAPI2_PCH+0x77,	
	MSG_PDP_SETDEFAULTPDPCONTEXT_RSP					= MSG_GRP_CAPI2_PCH+0x78,	
	MSG_READDECODEDPROTCONFIG_REQ						= MSG_GRP_CAPI2_PCH+0x79,	
	MSG_READDECODEDPROTCONFIG_RSP						= MSG_GRP_CAPI2_PCH+0x7A,	///<Payload type {::PCHDecodedProtConfig_t}
	MSG_BUILDPROTCONFIGOPTIONS_REQ						= MSG_GRP_CAPI2_PCH+0x7B,	
	MSG_BUILDPROTCONFIGOPTIONS_RSP						= MSG_GRP_CAPI2_PCH+0x7C,	///<Payload type {::PCHProtConfig_t}
	MSG_PDP_GETNEGQOS_REQ							 	= MSG_GRP_CAPI2_PCH+0x7D,	
	MSG_PDP_GETNEGQOS_RSP							 	= MSG_GRP_CAPI2_PCH+0x7E,	///<Payload type {::PCHR99QosProfile_t}
	MSG_PDP_SENDPDPMODIFYREQ_REQ					 	= MSG_GRP_CAPI2_PCH+0x7F,	
	MSG_PDP_GETNEGOTIATEDPARMS_REQ					 	= MSG_GRP_CAPI2_PCH+0x80,	
	MSG_PDP_GETNEGOTIATEDPARMS_RSP					 	= MSG_GRP_CAPI2_PCH+0x81,	///<Payload type {::PCHNegotiatedParms_t}
	MSG_BUILDPROTCONFIGOPTIONS2_REQ						= MSG_GRP_CAPI2_PCH+0x82,	
	MSG_BUILDPROTCONFIGOPTIONS2_RSP						= MSG_GRP_CAPI2_PCH+0x83,	///<Payload type {::PCHProtConfig_t}
	
   
//	MSG_PDP_ACTIVATION_RSP							 	= MSG_GRP_CAPI2_PCH+0xA1,
//	MSG_PDP_DEACTIVATION_RSP						 	= MSG_GRP_CAPI2_PCH+0xA2,
//	MSG_PDP_SEC_ACTIVATION_RSP						 	= MSG_GRP_CAPI2_PCH+0xA3,

	MSG_DATA_GET_GPRS_QOS_RSP							= MSG_GRP_CAPI2_PCH+0xB0,	
	
	MSG_CAPI2_PCH_REQ_END								= MSG_GRP_CAPI2_PCH+0xFF,	

	//---------------------------------------------------
	// MSG_GRP_CAPI2_UTIL,  (0x3E00)
	//---------------------------------------------------
	
	MSG_ADC_START_RSP									= MSG_GRP_CAPI2_UTIL+0x01,	///<Payload type {::UInt16}
	MSG_CAPI2_AT_RESPONSE_IND							= MSG_GRP_CAPI2_UTIL+0x02,	///<Payload type {::AtResponse_t}
//	MSG_MS_GET_ELEMENT_RSP								= MSG_GRP_CAPI2_UTIL+0x03,

	//---------------------------------------------------
	// MSG_GRP_CAPI2_PRIVATE,  (0x3F00)
	//---------------------------------------------------

	MSG_CEMU_CBK_IND									= MSG_GRP_CAPI2_PRIVATE+0xFA,	
	MSG_CAPI2_RAW_MSG_IND								= MSG_GRP_CAPI2_PRIVATE+0xFB,	///<Payload type {::UInt8*}
	MSG_CAPI2_UNDETERMINED_RSP							= MSG_GRP_CAPI2_PRIVATE+0xFC,	
	MSG_CAPI2_RECV_MSG_FROM_AP							= MSG_GRP_CAPI2_PRIVATE+0xFD,	
	MSG_CAPI2_ACK_RSP									= MSG_GRP_CAPI2_PRIVATE+0xFE,	

	

	//---------------------------------------------------
	//	MSG_GRP_CAPI2_GEN_0,MSG_GRP_CAPI2_GEN_1,MSG_GRP_CAPI2_GEN_2
	// (Reserved 0x4000 to 0x42FF)
	//---------------------------------------------------
	
//	MSG_GEN_REQ_START = MSG_GRP_CAPI2_GEN_0, Do not change this value, unless the $seq_num is changed in capi2.py
#include "capi2_gen_mids.h"
//	MSG_GEN_REQ_END = MSG_GRP_CAPI2_GEN_0 + 0x2ff,	


	//few redefenition
	MSG_MS_SEARCH_PLMN_RSP = MSG_PLMNLIST_IND,
	MSG_SIM_WHOLE_EFILE_DATA_RSP = MSG_SIM_EFILE_DATA_RSP,
	MSG_SIM_RECORD_EFILE_DATA_RSP = MSG_SIM_EFILE_DATA_RSP,
	MSG_SIM_LINEAR_EFILE_UPDATE_RSP = MSG_SIM_EFILE_UPDATE_RSP,
	MSG_SIM_SEEK_RECORD_RSP = MSG_SIM_SEEK_REC_RSP,
	MSG_SIM_CYCLIC_EFILE_UPDATE_RSP = MSG_SIM_EFILE_UPDATE_RSP,
	MSG_MS_PLMN_SELECT_RSP = MSG_PLMN_SELECT_CNF,

	MSG_READ_USIM_PBK_HDK_RSP = MSG_READ_USIM_PBK_HDK_ENTRY_RSP,
	MSG_WRITE_USIM_PBK_HDK_RSP = MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP,
	MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP = MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP = MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_READ_USIM_PBK_ALPHA_AAS_RSP = MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_READ_USIM_PBK_ALPHA_GAS_RSP = MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP,
    MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP = MSG_GET_USIM_PBK_ALPHA_INFO_RSP,
	MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP = MSG_GET_USIM_PBK_ALPHA_INFO_RSP,

	MSG_CAPI2_MSGIDS_END = 0x4FFF,
	
#endif

	
    //------------------------------------------------------------------------------------
	// MSG_GRP_SIMLATOR_0, MSG_GRP_SIMLATOR_1, MSG_GRP_SIMLATOR_2, MSG_GRP_SIMLATOR_3, 
	// Platform Simulation Messages (Win32 host build) (0x6000)
	//------------------------------------------------------------------------------------

#ifndef UNDER_CE
#ifndef UNDER_LINUX
	MSG_SIMULATOR_ACCEPT									=	MSG_GRP_SIMULATOR_0+0x00,
	MSG_SIMULATOR_ALLOCINTERTASKMSGFROMHEAP					=	MSG_GRP_SIMULATOR_0+0x02,
	MSG_SIMULATOR_AT_CMDCLOSETRANSACTION					=	MSG_GRP_SIMULATOR_0+0x04,
	MSG_SIMULATOR_AT_CMDRSP									=	MSG_GRP_SIMULATOR_0+0x06,
	MSG_SIMULATOR_AT_CMDRSPCMSERROR							=	MSG_GRP_SIMULATOR_0+0x08,
	MSG_SIMULATOR_AT_CMDRSPERROR							=	MSG_GRP_SIMULATOR_0+0x0a,
	MSG_SIMULATOR_AT_CMDRSPOK								=	MSG_GRP_SIMULATOR_0+0x0c,
	MSG_SIMULATOR_AT_CMDRSPSYNTAXERROR						=	MSG_GRP_SIMULATOR_0+0x0e,
	MSG_SIMULATOR_AT_GETCMDNAME								=	MSG_GRP_SIMULATOR_0+0x10,
	MSG_SIMULATOR_AT_GETPARM								=	MSG_GRP_SIMULATOR_0+0x12,
	MSG_SIMULATOR_AT_GETVALIDPARMSTR						=	MSG_GRP_SIMULATOR_0+0x14,
	MSG_SIMULATOR_AT_OUTPUTRSP								=	MSG_GRP_SIMULATOR_0+0x16,
	MSG_SIMULATOR_AT_OUTPUTSTR								=	MSG_GRP_SIMULATOR_0+0x18,
	MSG_SIMULATOR_AT_OUTPUTSTRBUFFERED						=	MSG_GRP_SIMULATOR_0+0x1a,
	MSG_SIMULATOR_AT_OUTPUTUNSOLICITEDSTR					=	MSG_GRP_SIMULATOR_0+0x1c,
	MSG_SIMULATOR_AT_PROCESSCMD								=	MSG_GRP_SIMULATOR_0+0x1e,
	MSG_SIMULATOR_AT_RESUMECMD								=	MSG_GRP_SIMULATOR_0+0x20,
	MSG_SIMULATOR_ATC_CONFIGSERIALDEVFORATC					=	MSG_GRP_SIMULATOR_0+0x22,
	MSG_SIMULATOR_AUDIO_CLOSESESSION						=	MSG_GRP_SIMULATOR_0+0x24,
	MSG_SIMULATOR_AUDIO_CREATEPLAYBACKSESSION				=	MSG_GRP_SIMULATOR_0+0x26,
	MSG_SIMULATOR_AUDIO_CREATERECORDINGSESSION				=	MSG_GRP_SIMULATOR_0+0x28,
	MSG_SIMULATOR_AUDIO_GETMICROPHONEGAIN					=	MSG_GRP_SIMULATOR_0+0x2a,
	MSG_SIMULATOR_AUDIO_GETSIDETONEGAIN						=	MSG_GRP_SIMULATOR_0+0x2c,
	MSG_SIMULATOR_AUDIO_GETSPEAKERVOL						=	MSG_GRP_SIMULATOR_0+0x2e,
	MSG_SIMULATOR_AUDIO_PAUSESESSION						=	MSG_GRP_SIMULATOR_0+0x30,
	MSG_SIMULATOR_AUDIO_PLAYTONE							=	MSG_GRP_SIMULATOR_0+0x32,
	MSG_SIMULATOR_AUDIO_RESUMESESSION						=	MSG_GRP_SIMULATOR_0+0x34,
	MSG_SIMULATOR_AUDIO_RUN									=	MSG_GRP_SIMULATOR_0+0x36,
	MSG_SIMULATOR_AUDIO_SELECTAUDIOCHANNEL					=	MSG_GRP_SIMULATOR_0+0x38,
	MSG_SIMULATOR_AUDIO_SETMICROPHONEGAIN					=	MSG_GRP_SIMULATOR_0+0x3a,
	MSG_SIMULATOR_AUDIO_SETSIDETONEGAIN						=	MSG_GRP_SIMULATOR_0+0x3c,
	MSG_SIMULATOR_AUDIO_SETSPEAKERVOL						=	MSG_GRP_SIMULATOR_0+0x3e,
	MSG_SIMULATOR_AUDIO_SHUTDOWN							=	MSG_GRP_SIMULATOR_0+0x40,
	MSG_SIMULATOR_AUDIO_STARTSESSION						=	MSG_GRP_SIMULATOR_0+0x42,
	MSG_SIMULATOR_AUDIO_STOPSESSION							=	MSG_GRP_SIMULATOR_0+0x44,
	MSG_SIMULATOR_BIND										=	MSG_GRP_SIMULATOR_0+0x46,
	MSG_SIMULATOR_CALLPARSER_PARSEDIALSTR					=	MSG_GRP_SIMULATOR_0+0x48,
	MSG_SIMULATOR_CC_ACCEPTDATACALL							=	MSG_GRP_SIMULATOR_0+0x4a,
	MSG_SIMULATOR_CC_ACCEPTVOICECALL						=	MSG_GRP_SIMULATOR_0+0x4c,
	MSG_SIMULATOR_CC_ACCEPTWAITINGCALL						=	MSG_GRP_SIMULATOR_0+0x4e,
	MSG_SIMULATOR_CC_ENDALLCALLS							=	MSG_GRP_SIMULATOR_0+0x50,
	MSG_SIMULATOR_CC_END_CALL								=	MSG_GRP_SIMULATOR_0+0x52,
	MSG_SIMULATOR_CC_ENDHELDCALL							=	MSG_GRP_SIMULATOR_0+0x54,
	MSG_SIMULATOR_CC_ENDMPTYCALLS							=	MSG_GRP_SIMULATOR_0+0x56,
	MSG_SIMULATOR_CC_GETALLACTIVECALLINDEX					=	MSG_GRP_SIMULATOR_0+0x58,
	MSG_SIMULATOR_CC_GETALLCALLINDEX						=	MSG_GRP_SIMULATOR_0+0x5a,
	MSG_SIMULATOR_CC_GETALLCALLSTATES						=	MSG_GRP_SIMULATOR_0+0x5c,
	MSG_SIMULATOR_CC_GETALLHELDCALLINDEX					=	MSG_GRP_SIMULATOR_0+0x5e,
	MSG_SIMULATOR_CC_GETALLMPTYCALLINDEX					=	MSG_GRP_SIMULATOR_0+0x60,
	MSG_SIMULATOR_CC_GETCALLCLIENTID						=	MSG_GRP_SIMULATOR_0+0x62,
	MSG_SIMULATOR_CC_GETCALLNUMBER							=	MSG_GRP_SIMULATOR_0+0x64,
	MSG_SIMULATOR_CC_GETCALLSTATE							=	MSG_GRP_SIMULATOR_0+0x66,
	MSG_SIMULATOR_CC_GETCALLTYPE							=	MSG_GRP_SIMULATOR_0+0x68,
	MSG_SIMULATOR_CC_GETCONNECTEDLINEID						=	MSG_GRP_SIMULATOR_0+0x6a,
	MSG_SIMULATOR_CC_GETCURRENTCALLINDEX					=	MSG_GRP_SIMULATOR_0+0x6c,
	MSG_SIMULATOR_CC_GETLASTCALLCCM							=	MSG_GRP_SIMULATOR_0+0x6e,
	MSG_SIMULATOR_CC_GETLASTCALLDURATION					=	MSG_GRP_SIMULATOR_0+0x70,
	MSG_SIMULATOR_CC_GETLASTCALLEXITCAUSE					=	MSG_GRP_SIMULATOR_0+0x72,
	MSG_SIMULATOR_CC_GETLASTDATACALLRXBYTES					=	MSG_GRP_SIMULATOR_0+0x74,
	MSG_SIMULATOR_CC_GETLASTDATACALLTXBYTES					=	MSG_GRP_SIMULATOR_0+0x76,
	MSG_SIMULATOR_CC_GETMPTYCALLINDEX						=	MSG_GRP_SIMULATOR_0+0x78,
	MSG_SIMULATOR_CC_GETNEXTACTIVECALLINDEX					=	MSG_GRP_SIMULATOR_0+0x7a,
	MSG_SIMULATOR_CC_GETNEXTHELDCALLINDEX					=	MSG_GRP_SIMULATOR_0+0x7c,
	MSG_SIMULATOR_CC_GETNEXTWAITCALLINDEX					=	MSG_GRP_SIMULATOR_0+0x7e,
	MSG_SIMULATOR_CC_GETNUMOFACTIVECALLS					=	MSG_GRP_SIMULATOR_0+0x80,
	MSG_SIMULATOR_CC_GETNUMOFHELDCALLS						=	MSG_GRP_SIMULATOR_0+0x82,
	MSG_SIMULATOR_CC_GETNUMOFMPTYCALLS						=	MSG_GRP_SIMULATOR_0+0x84,
	MSG_SIMULATOR_CC_HOLDCALL								=	MSG_GRP_SIMULATOR_0+0x86,
	MSG_SIMULATOR_CC_HOLDCURRENTCALL						=	MSG_GRP_SIMULATOR_0+0x88,
	MSG_SIMULATOR_CC_INITCALLCONTROL						=	MSG_GRP_SIMULATOR_0+0x8a,
	MSG_SIMULATOR_CC_ISCONNECTEDLINEIDPRESENTALLOWED		=	MSG_GRP_SIMULATOR_0+0x8c,
	MSG_SIMULATOR_CC_ISMULTIPARTYCALL						=	MSG_GRP_SIMULATOR_0+0x8e,
	MSG_SIMULATOR_CC_ISTHEREALERTINGCALL					=	MSG_GRP_SIMULATOR_0+0x90,
	MSG_SIMULATOR_CC_ISTHEREVOICECALL						=	MSG_GRP_SIMULATOR_0+0x92,
	MSG_SIMULATOR_CC_ISTHEREWAITINGCALL						=	MSG_GRP_SIMULATOR_0+0x94,
	MSG_SIMULATOR_CC_ISVALIDDTMF							=	MSG_GRP_SIMULATOR_0+0x96,
	MSG_SIMULATOR_CC_JOINCALL								=	MSG_GRP_SIMULATOR_0+0x98,
	MSG_SIMULATOR_CC_MAKEDATACALL							=	MSG_GRP_SIMULATOR_0+0x9a,
	MSG_SIMULATOR_CC_MAKEFAXCALL							=	MSG_GRP_SIMULATOR_0+0x9c,
	MSG_SIMULATOR_CC_MAKE_VOICE_CALL						=	MSG_GRP_SIMULATOR_0+0x9e,
	MSG_SIMULATOR_CC_MUTEDTMFTONE							=	MSG_GRP_SIMULATOR_0+0xa0,
	MSG_SIMULATOR_CC_RETRIEVECALL							=	MSG_GRP_SIMULATOR_0+0xa2,
	MSG_SIMULATOR_CC_RETRIEVNEXTHELDCALL					=	MSG_GRP_SIMULATOR_0+0xa4,
	MSG_SIMULATOR_CC_SENDDTMF								=	MSG_GRP_SIMULATOR_0+0xa6,
	MSG_SIMULATOR_CC_SPLITCALL								=	MSG_GRP_SIMULATOR_0+0xa8,
	MSG_SIMULATOR_CC_STOPDTMF								=	MSG_GRP_SIMULATOR_0+0xaa,
	MSG_SIMULATOR_CC_SWAPCALL								=	MSG_GRP_SIMULATOR_0+0xac,
	MSG_SIMULATOR_CONNECT									=	MSG_GRP_SIMULATOR_0+0xae,
	MSG_SIMULATOR_CSD_BUFFERFREESPACE						=	MSG_GRP_SIMULATOR_0+0xb0,
	MSG_SIMULATOR_CSD_GETDATA								=	MSG_GRP_SIMULATOR_0+0xb2,
	MSG_SIMULATOR_CSD_REGISTERDATAIND						=	MSG_GRP_SIMULATOR_0+0xb4,
	MSG_SIMULATOR_CSD_SENDDATA								=	MSG_GRP_SIMULATOR_0+0xb6,
	MSG_SIMULATOR_DATA_CREATECSDDATAACCT					=	MSG_GRP_SIMULATOR_0+0xb8,
	MSG_SIMULATOR_DATA_CREATEGPRSDATAACCT					=	MSG_GRP_SIMULATOR_0+0xba,
	MSG_SIMULATOR_DATA_DELETEDATAACCT						=	MSG_GRP_SIMULATOR_0+0xbc,
	MSG_SIMULATOR_DATA_GETACCTTYPE							=	MSG_GRP_SIMULATOR_0+0xbe,
	MSG_SIMULATOR_DATA_GETAUTHENMETHOD						=	MSG_GRP_SIMULATOR_0+0xc0,
	MSG_SIMULATOR_DATA_GETCIDFROMDATAACCTID					=	MSG_GRP_SIMULATOR_0+0xc2,
	MSG_SIMULATOR_DATA_GETCSDBAUDRATE						=	MSG_GRP_SIMULATOR_0+0xc4,
	MSG_SIMULATOR_DATA_GETCSDCONNELEMENT					=	MSG_GRP_SIMULATOR_0+0xc6,
	MSG_SIMULATOR_DATA_GETCSDCONTEXT						=	MSG_GRP_SIMULATOR_0+0xc8,
	MSG_SIMULATOR_DATA_GETCSDDATACOMPTYPE					=	MSG_GRP_SIMULATOR_0+0xca,
	MSG_SIMULATOR_DATA_GETCSDDIALNUMBER						=	MSG_GRP_SIMULATOR_0+0xcc,
	MSG_SIMULATOR_DATA_GETCSDDIALTYPE						=	MSG_GRP_SIMULATOR_0+0xce,
	MSG_SIMULATOR_DATA_GETCSDERRCORRECTIONTYPE				=	MSG_GRP_SIMULATOR_0+0xd0,
	MSG_SIMULATOR_DATA_GETDATAACCTIDFROMCID					=	MSG_GRP_SIMULATOR_0+0xd2,
	MSG_SIMULATOR_DATA_GETDATACOMPRESSION					=	MSG_GRP_SIMULATOR_0+0xd4,
	MSG_SIMULATOR_DATA_GETEMPTYACCTSLOT						=	MSG_GRP_SIMULATOR_0+0xd6,
	MSG_SIMULATOR_DATA_GETGPRSAPN							=	MSG_GRP_SIMULATOR_0+0xd8,
	MSG_SIMULATOR_DATA_GETGPRSCONTEXT						=	MSG_GRP_SIMULATOR_0+0xda,
	MSG_SIMULATOR_DATA_GETGPRSHEADERCOMPRESSION				=	MSG_GRP_SIMULATOR_0+0xdc,
	MSG_SIMULATOR_DATA_GETGPRSPDPTYPE						=	MSG_GRP_SIMULATOR_0+0xde,
	MSG_SIMULATOR_DATA_GETGPRSQOS							=	MSG_GRP_SIMULATOR_0+0xe0,
	MSG_SIMULATOR_DATA_GETPASSWORD							=	MSG_GRP_SIMULATOR_0+0xe2,
	MSG_SIMULATOR_DATA_GETPRIMARYDNSADDR					=	MSG_GRP_SIMULATOR_0+0xe4,
	MSG_SIMULATOR_DATA_GETSECONDDNSADDR						=	MSG_GRP_SIMULATOR_0+0xe6,
	MSG_SIMULATOR_DATA_GETSTATICIPADDR						=	MSG_GRP_SIMULATOR_0+0xe8,
	MSG_SIMULATOR_DATA_GETUSERNAME							=	MSG_GRP_SIMULATOR_0+0xea,
	MSG_SIMULATOR_DATA_SETAUTHENMETHOD						=	MSG_GRP_SIMULATOR_0+0xec,
	MSG_SIMULATOR_DATA_SETCSDBAUDRATE						=	MSG_GRP_SIMULATOR_0+0xee,
	MSG_SIMULATOR_DATA_SETCSDCONNELEMENT					=	MSG_GRP_SIMULATOR_0+0xf0,
	MSG_SIMULATOR_DATA_SETCSDDATACOMPTYPE					=	MSG_GRP_SIMULATOR_0+0xf2,
	MSG_SIMULATOR_DATA_SETCSDDIALNUMBER						=	MSG_GRP_SIMULATOR_0+0xf4,
	MSG_SIMULATOR_DATA_SETCSDDIALTYPE						=	MSG_GRP_SIMULATOR_0+0xf6,
	MSG_SIMULATOR_DATA_SETCSDERRCORRECTIONTYPE				=	MSG_GRP_SIMULATOR_0+0xf8,
	MSG_SIMULATOR_DATA_SETDATACOMPRESSION					=	MSG_GRP_SIMULATOR_0+0xfa,
	MSG_SIMULATOR_DATA_SETGPRSAPN							=	MSG_GRP_SIMULATOR_0+0xfc,
	MSG_SIMULATOR_DATA_SETGPRSHEADERCOMPRESSION				=	MSG_GRP_SIMULATOR_0+0xfe,
	MSG_SIMULATOR_DATA_SETGPRSPDPTYPE						=	MSG_GRP_SIMULATOR_1+0x00,
	MSG_SIMULATOR_DATA_SETGPRSQOS							=	MSG_GRP_SIMULATOR_1+0x02,
	MSG_SIMULATOR_DATA_SETPASSWORD							=	MSG_GRP_SIMULATOR_1+0x04,
	MSG_SIMULATOR_DATA_SETPRIMARYDNSADDR					=	MSG_GRP_SIMULATOR_1+0x06,
	MSG_SIMULATOR_DATA_SETSECONDDNSADDR						=	MSG_GRP_SIMULATOR_1+0x08,
	MSG_SIMULATOR_DATA_SETSTATICIPADDR						=	MSG_GRP_SIMULATOR_1+0x0a,
	MSG_SIMULATOR_DATA_SETUSERNAME							=	MSG_GRP_SIMULATOR_1+0x0c,
	MSG_SIMULATOR_ERRNO										=	MSG_GRP_SIMULATOR_1+0x0e,
	MSG_SIMULATOR_FREEINTERTASKMSG							=	MSG_GRP_SIMULATOR_1+0x10,
	MSG_SIMULATOR_FS_CLOSE									=	MSG_GRP_SIMULATOR_1+0x12,
	MSG_SIMULATOR_FS_FLUSH									=	MSG_GRP_SIMULATOR_1+0x14,
	MSG_SIMULATOR_FS_GEFREESPACE							=	MSG_GRP_SIMULATOR_1+0x16,
	MSG_SIMULATOR_FS_GETCURRENTDIR							=	MSG_GRP_SIMULATOR_1+0x18,
	MSG_SIMULATOR_FS_GETFILEDONE							=	MSG_GRP_SIMULATOR_1+0x1a,
	MSG_SIMULATOR_FS_GETFILEFIRST							=	MSG_GRP_SIMULATOR_1+0x1c,
	MSG_SIMULATOR_FS_GETFILENEXT							=	MSG_GRP_SIMULATOR_1+0x1e,
	MSG_SIMULATOR_FS_MAKEDIR								=	MSG_GRP_SIMULATOR_1+0x20,
	MSG_SIMULATOR_FS_OPEN									=	MSG_GRP_SIMULATOR_1+0x22,
	MSG_SIMULATOR_FS_READ									=	MSG_GRP_SIMULATOR_1+0x24,
	MSG_SIMULATOR_FS_REMOVE									=	MSG_GRP_SIMULATOR_1+0x26,
	MSG_SIMULATOR_FS_REMOVEDIR								=	MSG_GRP_SIMULATOR_1+0x28,
	MSG_SIMULATOR_FS_RENAME									=	MSG_GRP_SIMULATOR_1+0x2a,
	MSG_SIMULATOR_FS_SEEK									=	MSG_GRP_SIMULATOR_1+0x2c,
	MSG_SIMULATOR_FS_SETCURRENTDIR							=	MSG_GRP_SIMULATOR_1+0x2e,
	MSG_SIMULATOR_FS_TELL									=	MSG_GRP_SIMULATOR_1+0x30,
	MSG_SIMULATOR_FS_WRITE									=	MSG_GRP_SIMULATOR_1+0x32,
	MSG_SIMULATOR_GETATCMDIDBYMPXCHANNEL					=	MSG_GRP_SIMULATOR_1+0x34,
	MSG_SIMULATOR_GETATCMDRSPMPXCHANNEL						=	MSG_GRP_SIMULATOR_1+0x36,
	MSG_SIMULATOR_GETPEERNAME								=	MSG_GRP_SIMULATOR_1+0x38,
	MSG_SIMULATOR_GETSMSBEARERTYPE							=	MSG_GRP_SIMULATOR_1+0x3a,
	MSG_SIMULATOR_GETSMSRVCENTERNUMBER						=	MSG_GRP_SIMULATOR_1+0x3c,
	MSG_SIMULATOR_GETSOCKNAME								=	MSG_GRP_SIMULATOR_1+0x3e,
	MSG_SIMULATOR_GETSOCKOPT								=	MSG_GRP_SIMULATOR_1+0x40,
	MSG_SIMULATOR_GETV24OPERATIONMODE						=	MSG_GRP_SIMULATOR_1+0x42,
	MSG_SIMULATOR_GPIO_CLEARINT								=	MSG_GRP_SIMULATOR_1+0x44,
	MSG_SIMULATOR_GPIO_CONFIGINPUT							=	MSG_GRP_SIMULATOR_1+0x46,
	MSG_SIMULATOR_GPIO_CONFIGKEYPAD							=	MSG_GRP_SIMULATOR_1+0x48,
	MSG_SIMULATOR_GPIO_CONFIGOUTPUT							=	MSG_GRP_SIMULATOR_1+0x4a,
	MSG_SIMULATOR_GPIO_DISABLEINT							=	MSG_GRP_SIMULATOR_1+0x4c,
	MSG_SIMULATOR_GPIO_ENABLEINT							=	MSG_GRP_SIMULATOR_1+0x4e,
	MSG_SIMULATOR_GPIO_KEYPADREGISTER						=	MSG_GRP_SIMULATOR_1+0x50,
	MSG_SIMULATOR_GPIO_READ									=	MSG_GRP_SIMULATOR_1+0x52,
	MSG_SIMULATOR_GPIO_READINTSTATUS						=	MSG_GRP_SIMULATOR_1+0x54,
	MSG_SIMULATOR_GPIO_REGISTER								=	MSG_GRP_SIMULATOR_1+0x56,
	MSG_SIMULATOR_GPIO_SELECTROW							=	MSG_GRP_SIMULATOR_1+0x58,
	MSG_SIMULATOR_GPIO_WRITE								=	MSG_GRP_SIMULATOR_1+0x5a,
	MSG_SIMULATOR_I2C_INIT									=	MSG_GRP_SIMULATOR_1+0x5c,
	MSG_SIMULATOR_I2C_READ									=	MSG_GRP_SIMULATOR_1+0x5e,
	MSG_SIMULATOR_I2C_RUN									=	MSG_GRP_SIMULATOR_1+0x60,
	MSG_SIMULATOR_I2C_WRITE									=	MSG_GRP_SIMULATOR_1+0x62,
	MSG_SIMULATOR_IMAGE_CONVERT								=	MSG_GRP_SIMULATOR_1+0x64,
	MSG_SIMULATOR_IMAGE_DECODE								=	MSG_GRP_SIMULATOR_1+0x66,
	MSG_SIMULATOR_IMAGE_GETINFO								=	MSG_GRP_SIMULATOR_1+0x68,
	MSG_SIMULATOR_IMAGE_ROTATE								=	MSG_GRP_SIMULATOR_1+0x6a,
	MSG_SIMULATOR_IMAGE_RUN									=	MSG_GRP_SIMULATOR_1+0x6c,
	MSG_SIMULATOR_IMAGE_SCALE								=	MSG_GRP_SIMULATOR_1+0x6e,
	MSG_SIMULATOR_IMAGE_SHUTDOWN							=	MSG_GRP_SIMULATOR_1+0x70,
	MSG_SIMULATOR_IN_RESHOST								=	MSG_GRP_SIMULATOR_1+0x72,
	MSG_SIMULATOR_IRQ_CLEAR									=	MSG_GRP_SIMULATOR_1+0x74,
	MSG_SIMULATOR_IRQ_CLEARALL								=	MSG_GRP_SIMULATOR_1+0x76,
	MSG_SIMULATOR_IRQ_CONTROLLER							=	MSG_GRP_SIMULATOR_1+0x78,
	MSG_SIMULATOR_IRQ_DISABLE								=	MSG_GRP_SIMULATOR_1+0x7a,
	MSG_SIMULATOR_IRQ_DISABLEALL							=	MSG_GRP_SIMULATOR_1+0x7c,
	MSG_SIMULATOR_IRQ_ENABLE								=	MSG_GRP_SIMULATOR_1+0x7e,
	MSG_SIMULATOR_IRQ_INIT									=	MSG_GRP_SIMULATOR_1+0x80,
	MSG_SIMULATOR_IRQ_ISACTIVE								=	MSG_GRP_SIMULATOR_1+0x82,
	MSG_SIMULATOR_IRQ_ISACTIVERAW							=	MSG_GRP_SIMULATOR_1+0x84,
	MSG_SIMULATOR_IRQ_ISENABLED								=	MSG_GRP_SIMULATOR_1+0x86,
	MSG_SIMULATOR_IRQ_REGISTER								=	MSG_GRP_SIMULATOR_1+0x88,
	MSG_SIMULATOR_IRQ_RESTORE								=	MSG_GRP_SIMULATOR_1+0x8a,
	MSG_SIMULATOR_IRQ_TRIGGERRIPINT							=	MSG_GRP_SIMULATOR_1+0x8c,
	MSG_SIMULATOR_KPD_DRV_GETLASTKEYPRESSTIME				=	MSG_GRP_SIMULATOR_1+0x8e,
	MSG_SIMULATOR_KPD_DRV_INIT								=	MSG_GRP_SIMULATOR_1+0x90,
	MSG_SIMULATOR_KPD_DRV_REGISTER							=	MSG_GRP_SIMULATOR_1+0x92,
	MSG_SIMULATOR_KPD_DRV_RUN								=	MSG_GRP_SIMULATOR_1+0x94,
	MSG_SIMULATOR_KPD_DRV_SHUTDOWN							=	MSG_GRP_SIMULATOR_1+0x96,
	MSG_SIMULATOR_LISTEN									=	MSG_GRP_SIMULATOR_1+0x98,
	MSG_SIMULATOR_LOG_DEBUGOUTPUTSTRING						=	MSG_GRP_SIMULATOR_1+0x9a,
	MSG_SIMULATOR_LOG_DEBUGOUTPUTVALUE						=	MSG_GRP_SIMULATOR_1+0x9c,
	MSG_SIMULATOR_LOG_ENABLELOGGING							=	MSG_GRP_SIMULATOR_1+0x9e,
	MSG_SIMULATOR_LOG_ENABLERANGE							=	MSG_GRP_SIMULATOR_1+0xa0,
	MSG_SIMULATOR_LOG_GETLOGGINGID							=	MSG_GRP_SIMULATOR_1+0xa2,
	MSG_SIMULATOR_LOG_ISLOGGINGENABLE						=	MSG_GRP_SIMULATOR_1+0xa4,
	MSG_SIMULATOR_MM_DELETE									=	MSG_GRP_SIMULATOR_1+0xa6,
	MSG_SIMULATOR_MM_DELOCATE								=	MSG_GRP_SIMULATOR_1+0xa8,
	MSG_SIMULATOR_MM_FREEATTRIBUTE							=	MSG_GRP_SIMULATOR_1+0xaa,
	MSG_SIMULATOR_MM_GETATTRIBUTE							=	MSG_GRP_SIMULATOR_1+0xac,
	MSG_SIMULATOR_MM_INIT									=	MSG_GRP_SIMULATOR_1+0xae,
	MSG_SIMULATOR_MM_LISTATTRIBUTE							=	MSG_GRP_SIMULATOR_1+0xb0,
	MSG_SIMULATOR_MM_RETRIEVE								=	MSG_GRP_SIMULATOR_1+0xb2,
	MSG_SIMULATOR_MM_SETATTRIBUTE							=	MSG_GRP_SIMULATOR_1+0xb4,
	MSG_SIMULATOR_MM_SHUTDOWN								=	MSG_GRP_SIMULATOR_1+0xb6,
	MSG_SIMULATOR_MM_STOREASYNC								=	MSG_GRP_SIMULATOR_1+0xb8,
	MSG_SIMULATOR_MM_STOREBYREF								=	MSG_GRP_SIMULATOR_1+0xba,
	MSG_SIMULATOR_MPXCONFIG									=	MSG_GRP_SIMULATOR_1+0xbc,
	MSG_SIMULATOR_MPXGETMUXPARAM							=	MSG_GRP_SIMULATOR_1+0xbe,
	MSG_SIMULATOR_MS_GETGPRSATTACHSTATUS					=	MSG_GRP_SIMULATOR_1+0xc0,
	MSG_SIMULATOR_MS_GETPLMNCODEBYNAME						=	MSG_GRP_SIMULATOR_1+0xc2,
	MSG_SIMULATOR_MS_GETPLMNENTRYBYINDEX					=	MSG_GRP_SIMULATOR_1+0xc4,
	MSG_SIMULATOR_MS_GETPLMNFORMAT							=	MSG_GRP_SIMULATOR_1+0xc6,
	MSG_SIMULATOR_MS_GETPLMNLISTSIZE						=	MSG_GRP_SIMULATOR_1+0xc8,
	MSG_SIMULATOR_MS_GETPLMNMCC								=	MSG_GRP_SIMULATOR_1+0xca,
	MSG_SIMULATOR_MS_GETPLMNMNC								=	MSG_GRP_SIMULATOR_1+0xcc,
	MSG_SIMULATOR_MS_GETPLMNMODE							=	MSG_GRP_SIMULATOR_1+0xce,
	MSG_SIMULATOR_MS_GETPLMNNAMEBYCODE						=	MSG_GRP_SIMULATOR_1+0xd0,
	MSG_SIMULATOR_MS_ISGPRSREGISTERED						=	MSG_GRP_SIMULATOR_1+0xd2,
	MSG_SIMULATOR_MS_ISGSMREGISTERED						=	MSG_GRP_SIMULATOR_1+0xd4,
	MSG_SIMULATOR_MS_ISPLMNFORBIDDEN						=	MSG_GRP_SIMULATOR_1+0xd6,
	MSG_SIMULATOR_MS_PLMNSELECT								=	MSG_GRP_SIMULATOR_1+0xd8,
	MSG_SIMULATOR_MS_SEARCHAVAILABLEPLMN					=	MSG_GRP_SIMULATOR_1+0xda,
	MSG_SIMULATOR_MS_SENDCOMBINEDATTACHREQ					=	MSG_GRP_SIMULATOR_1+0xdc,
	MSG_SIMULATOR_MS_SENDDETACHREQ							=	MSG_GRP_SIMULATOR_1+0xde,
	MSG_SIMULATOR_MS_SETCURRENTPLMN							=	MSG_GRP_SIMULATOR_1+0xe0,
	MSG_SIMULATOR_MS_SETPLMNFORMAT							=	MSG_GRP_SIMULATOR_1+0xe2,
	MSG_SIMULATOR_MS_SETPLMNMODE							=	MSG_GRP_SIMULATOR_1+0xe4,
	MSG_SIMULATOR_MS_SETPOWERDOWNTIMER						=	MSG_GRP_SIMULATOR_1+0xe6,
	MSG_SIMULATOR_MS_SETSTARTBAND							=	MSG_GRP_SIMULATOR_1+0xe8,
	MSG_SIMULATOR_PBK_GETALPHA								=	MSG_GRP_SIMULATOR_1+0xea,
	MSG_SIMULATOR_PBK_ISEMERGENCYCALLNUMBER					=	MSG_GRP_SIMULATOR_1+0xec,
	MSG_SIMULATOR_PBK_ISREADY								=	MSG_GRP_SIMULATOR_1+0xee,
	MSG_SIMULATOR_PBK_SENDFINDALPHAMATCHMULTIPLEREQ			=	MSG_GRP_SIMULATOR_1+0xf0,
	MSG_SIMULATOR_PBK_SENDFINDALPHAMATCHONEREQ				=	MSG_GRP_SIMULATOR_1+0xf2,
	MSG_SIMULATOR_PBK_SENDINFOREQ							=	MSG_GRP_SIMULATOR_1+0xf4,
	MSG_SIMULATOR_PBK_SENDISNUMDIALLABLEREQ					=	MSG_GRP_SIMULATOR_1+0xf6,
	MSG_SIMULATOR_PBK_SENDREADENTRYREQ						=	MSG_GRP_SIMULATOR_1+0xf8,
	MSG_SIMULATOR_PBK_SENDWRITEENTRYREQ						=	MSG_GRP_SIMULATOR_1+0xfa,
	MSG_SIMULATOR_PDP_ACTIVATESNDCPCONNECTION				=	MSG_GRP_SIMULATOR_1+0xfc,
	MSG_SIMULATOR_PDP_DEACTIVATESNDCPCONNECTION				=	MSG_GRP_SIMULATOR_1+0xfe,
	MSG_SIMULATOR_PDP_GETDEFINEDPDPCONTEXTCIDLIST			=	MSG_GRP_SIMULATOR_2+0x00,
	MSG_SIMULATOR_PDP_GETGPRSACTIVATESTATUS					=	MSG_GRP_SIMULATOR_2+0x02,
	MSG_SIMULATOR_PDP_GETGPRSMINQOS							=	MSG_GRP_SIMULATOR_2+0x04,
	MSG_SIMULATOR_PDP_GETGPRSMTPDPAUTORSP					=	MSG_GRP_SIMULATOR_2+0x06,
	MSG_SIMULATOR_PDP_GETGPRSQOS							=	MSG_GRP_SIMULATOR_2+0x08,
	MSG_SIMULATOR_PDP_GETMSCLASS							=	MSG_GRP_SIMULATOR_2+0x0a,
	MSG_SIMULATOR_PDP_GETPDPADDRESS							=	MSG_GRP_SIMULATOR_2+0x0c,
	MSG_SIMULATOR_PDP_GETPDPCONTEXT							=	MSG_GRP_SIMULATOR_2+0x0e,
	MSG_SIMULATOR_PDP_SENDMTPDPACTIVATIONRSP				=	MSG_GRP_SIMULATOR_2+0x10,
	MSG_SIMULATOR_PDP_SENDPDPACTIVATEREQ					=	MSG_GRP_SIMULATOR_2+0x12,
	MSG_SIMULATOR_PDP_SENDPDPDEACTIVATEREQ					=	MSG_GRP_SIMULATOR_2+0x14,
	MSG_SIMULATOR_PDP_SETGPRSMINQOS							=	MSG_GRP_SIMULATOR_2+0x16,
	MSG_SIMULATOR_PDP_SETGPRSMTPDPAUTORSP					=	MSG_GRP_SIMULATOR_2+0x18,
	MSG_SIMULATOR_PDP_SETGPRSQOS							=	MSG_GRP_SIMULATOR_2+0x1a,
	MSG_SIMULATOR_PDP_SETMSCLASS							=	MSG_GRP_SIMULATOR_2+0x1c,
	MSG_SIMULATOR_PDP_SETPDPCONTEXT							=	MSG_GRP_SIMULATOR_2+0x1e,
	MSG_SIMULATOR_RECV										=	MSG_GRP_SIMULATOR_2+0x20,
	MSG_SIMULATOR_RTC_ALARMREGISTER							=	MSG_GRP_SIMULATOR_2+0x22,
	MSG_SIMULATOR_RTC_CACULWEEKDAY							=	MSG_GRP_SIMULATOR_2+0x24,
	MSG_SIMULATOR_RTC_CHECKTIME								=	MSG_GRP_SIMULATOR_2+0x26,
	MSG_SIMULATOR_RTC_DISABLEALARM							=	MSG_GRP_SIMULATOR_2+0x28,
	MSG_SIMULATOR_RTC_DISABLEMINUTEINT						=	MSG_GRP_SIMULATOR_2+0x2a,
	MSG_SIMULATOR_RTC_ENABLEALARM							=	MSG_GRP_SIMULATOR_2+0x2c,
	MSG_SIMULATOR_RTC_FIREALARM							    =	MSG_GRP_SIMULATOR_2+0x2d,
	MSG_SIMULATOR_RTC_ENABLEMINUTEINT						=	MSG_GRP_SIMULATOR_2+0x2e,
	MSG_SIMULATOR_RTC_GETALARMTIME							=	MSG_GRP_SIMULATOR_2+0x30,
	MSG_SIMULATOR_RTC_GETCOUNT								=	MSG_GRP_SIMULATOR_2+0x32,
	MSG_SIMULATOR_RTC_GETTIME								=	MSG_GRP_SIMULATOR_2+0x34,
	MSG_SIMULATOR_RTC_OFFSETTODATE							=	MSG_GRP_SIMULATOR_2+0x36,
	MSG_SIMULATOR_RTC_SETALARMTIME							=	MSG_GRP_SIMULATOR_2+0x38,
	MSG_SIMULATOR_RTC_SETTIME								=	MSG_GRP_SIMULATOR_2+0x3a,
	MSG_SIMULATOR_RTC_TIMEREGISTER							=	MSG_GRP_SIMULATOR_2+0x3c,
	MSG_SIMULATOR_SATKCMDRESP								=	MSG_GRP_SIMULATOR_2+0x3e,
	MSG_SIMULATOR_SELECT									=	MSG_GRP_SIMULATOR_2+0x40,
	MSG_SIMULATOR_SEND										=	MSG_GRP_SIMULATOR_2+0x42,
	MSG_SIMULATOR_SENDPPPCLOSEREQ							=	MSG_GRP_SIMULATOR_2+0x44,
	MSG_SIMULATOR_SENDPPPOPENREQ							=	MSG_GRP_SIMULATOR_2+0x46,
	MSG_SIMULATOR_SENDPPPPDU								=	MSG_GRP_SIMULATOR_2+0x48,
	MSG_SIMULATOR_SENDSMSREQ								=	MSG_GRP_SIMULATOR_2+0x4a,
	MSG_SIMULATOR_SENDSMSSRVCENTERNUMBERUPDATEREQ			=	MSG_GRP_SIMULATOR_2+0x4c,
	MSG_SIMULATOR_SENDSNPDUREQ								=	MSG_GRP_SIMULATOR_2+0x4e,
	MSG_SIMULATOR_SERIAL_CLOSEDEVICE						=	MSG_GRP_SIMULATOR_2+0x50,
	MSG_SIMULATOR_SERIAL_OPENDEVICE							=	MSG_GRP_SIMULATOR_2+0x52,
	MSG_SIMULATOR_SERIAL_READ								=	MSG_GRP_SIMULATOR_2+0x54,
	MSG_SIMULATOR_SERIAL_REGISTEREVENTHANDLER				=	MSG_GRP_SIMULATOR_2+0x56,
	MSG_SIMULATOR_SERIAL_WRITE								=	MSG_GRP_SIMULATOR_2+0x58,
	MSG_SIMULATOR_SETSMSBEARERPREFERENCE					=	MSG_GRP_SIMULATOR_2+0x5a,
	MSG_SIMULATOR_SETSOCKOPT								=	MSG_GRP_SIMULATOR_2+0x5c,
	MSG_SIMULATOR_SETV24DCD									=	MSG_GRP_SIMULATOR_2+0x5e,
	MSG_SIMULATOR_SETV24OPERATIONMODEUINT8					=	MSG_GRP_SIMULATOR_2+0x60,
	MSG_SIMULATOR_SETV24RI									=	MSG_GRP_SIMULATOR_2+0x62,
	MSG_SIMULATOR_SHUTDOWN									=	MSG_GRP_SIMULATOR_2+0x64,
	MSG_SIMULATOR_SIM_GETCARDPHASE							=	MSG_GRP_SIMULATOR_2+0x66,
	MSG_SIMULATOR_SIM_GETCPHSPHASE							=	MSG_GRP_SIMULATOR_2+0x68,
	MSG_SIMULATOR_SIM_GETHOMEPLMN							=	MSG_GRP_SIMULATOR_2+0x6a,
	MSG_SIMULATOR_SIM_GETIMSI								=	MSG_GRP_SIMULATOR_2+0x6c,
	MSG_SIMULATOR_SIM_GETPINSTATUS							=	MSG_GRP_SIMULATOR_2+0x6e,
	MSG_SIMULATOR_SIM_GETPRESENTSTATUS						=	MSG_GRP_SIMULATOR_2+0x70,
	MSG_SIMULATOR_SIM_GETSIMTYPE							=	MSG_GRP_SIMULATOR_2+0x72,
	MSG_SIMULATOR_SIM_GETSMSSCA								=	MSG_GRP_SIMULATOR_2+0x74,
	MSG_SIMULATOR_SIM_GETSMSSTATUS							=	MSG_GRP_SIMULATOR_2+0x76,
	MSG_SIMULATOR_SIM_GETSMSTOTALNUMBER						=	MSG_GRP_SIMULATOR_2+0x78,
	MSG_SIMULATOR_SIM_ISCACHEDDATAREADY						=	MSG_GRP_SIMULATOR_2+0x7a,
	MSG_SIMULATOR_SIM_ISOPERATIONRESTRICTED					=	MSG_GRP_SIMULATOR_2+0x7c,
	MSG_SIMULATOR_SIM_ISPIN2VERIFIED						=	MSG_GRP_SIMULATOR_2+0x7e,
	MSG_SIMULATOR_SIM_ISPINBLOCKED							=	MSG_GRP_SIMULATOR_2+0x80,
	MSG_SIMULATOR_SIM_ISPINOK								=	MSG_GRP_SIMULATOR_2+0x82,
	MSG_SIMULATOR_SIM_ISPINREQUIRED							=	MSG_GRP_SIMULATOR_2+0x84,
	MSG_SIMULATOR_SIM_ISPINVERIFIED							=	MSG_GRP_SIMULATOR_2+0x86,
	MSG_SIMULATOR_SIM_SENDBINARYEFILEREADREQ				=	MSG_GRP_SIMULATOR_2+0x88,
	MSG_SIMULATOR_SIM_SENDBINARYEFILEUPDATEREQ				=	MSG_GRP_SIMULATOR_2+0x8a,
	MSG_SIMULATOR_SIM_SENDCHANGECHVREQ						=	MSG_GRP_SIMULATOR_2+0x8c,
	MSG_SIMULATOR_SIM_SENDCYCLICEFILEUPDATEREQ				=	MSG_GRP_SIMULATOR_2+0x8e,
	MSG_SIMULATOR_SIM_SENDEFILEINFOREQ						=	MSG_GRP_SIMULATOR_2+0x90,
	MSG_SIMULATOR_SIM_SENDINCREASEACMREQ					=	MSG_GRP_SIMULATOR_2+0x92,
	MSG_SIMULATOR_SIM_SENDLINEAREFILEUPDATEREQ				=	MSG_GRP_SIMULATOR_2+0x94,
	MSG_SIMULATOR_SIM_SENDREADACMMAXREQ						=	MSG_GRP_SIMULATOR_2+0x96,
	MSG_SIMULATOR_SIM_SENDREADACMREQ						=	MSG_GRP_SIMULATOR_2+0x98,
	MSG_SIMULATOR_SIM_SENDREADPUCTREQ						=	MSG_GRP_SIMULATOR_2+0x9a,
	MSG_SIMULATOR_SIM_SENDREADSVCPROVNAMEREQ				=	MSG_GRP_SIMULATOR_2+0x9c,
	MSG_SIMULATOR_SIM_SENDRECORDEFILEREADREQ				=	MSG_GRP_SIMULATOR_2+0x9e,
	MSG_SIMULATOR_SIM_SENDREMAININGPINATTEMPTREQ			=	MSG_GRP_SIMULATOR_2+0xa0,
	MSG_SIMULATOR_SIM_SENDSETCHV1ONOFFREQ					=	MSG_GRP_SIMULATOR_2+0xa2,
	MSG_SIMULATOR_SIM_SENDSETOPERSTATEREQ					=	MSG_GRP_SIMULATOR_2+0xa4,
	MSG_SIMULATOR_SIM_SENDUNBLOCKCHVREQ						=	MSG_GRP_SIMULATOR_2+0xa6,
	MSG_SIMULATOR_SIM_SENDUPDATEPREFLISTREQ					=	MSG_GRP_SIMULATOR_2+0xa8,
	MSG_SIMULATOR_SIM_SENDVERIFYCHVREQ						=	MSG_GRP_SIMULATOR_2+0xaa,
	MSG_SIMULATOR_SIM_SENDWHOLEBINARYEFILEREADREQ			=	MSG_GRP_SIMULATOR_2+0xac,
	MSG_SIMULATOR_SIM_SENDWRITEACMMAXREQ					=	MSG_GRP_SIMULATOR_2+0xae,
	MSG_SIMULATOR_SIM_SENDWRITEACMREQ						=	MSG_GRP_SIMULATOR_2+0xb0,
	MSG_SIMULATOR_SIM_SENDWRITEPUCTREQ						=	MSG_GRP_SIMULATOR_2+0xb2,
	MSG_SIMULATOR_SIMLOCKCHANGEPHONELOCKPASSWORD			=	MSG_GRP_SIMULATOR_2+0xb4,
	MSG_SIMULATOR_SIMLOCKCHECKALLLOCKS						=	MSG_GRP_SIMULATOR_2+0xb6,
	MSG_SIMULATOR_SIMLOCKGETCURRENTCLOSEDLOCK				=	MSG_GRP_SIMULATOR_2+0xb8,
	MSG_SIMULATOR_SIMLOCKISLOCKON							=	MSG_GRP_SIMULATOR_2+0xba,
	MSG_SIMULATOR_SIMLOCKSETDEFAULTPHONELOCK				=	MSG_GRP_SIMULATOR_2+0xbc,
	MSG_SIMULATOR_SIMLOCKSETLOCK							=	MSG_GRP_SIMULATOR_2+0xbe,
	MSG_SIMULATOR_SIMLOCKUNLOCKSIM							=	MSG_GRP_SIMULATOR_2+0xc0,
	MSG_SIMULATOR_SLEEP_ALLOCID								=	MSG_GRP_SIMULATOR_2+0xc2,
	MSG_SIMULATOR_SLEEP_CONFIGDEEPSLEEP						=	MSG_GRP_SIMULATOR_2+0xc4,
	MSG_SIMULATOR_SLEEP_DISABLEDEEPSLEEP					=	MSG_GRP_SIMULATOR_2+0xc6,
	MSG_SIMULATOR_SLEEP_ENABLEDEEPSLEEP						=	MSG_GRP_SIMULATOR_2+0xc8,
	MSG_SIMULATOR_SLEEP_INIT								=	MSG_GRP_SIMULATOR_2+0xca,
	MSG_SIMULATOR_SMS_SEND_SMS_REQ							=	MSG_GRP_SIMULATOR_2+0xd0,
	MSG_SIMULATOR_SMS_SEND_SMS_PDU_REQ						=	MSG_GRP_SIMULATOR_2+0xd1,
	MSG_SIMULATOR_SMS_SEND_ACK_TO_NETWORK					=	MSG_GRP_SIMULATOR_2+0xd2,
	MSG_SIMULATOR_SOCKET									=	MSG_GRP_SIMULATOR_3+0x10,
	MSG_SIMULATOR_SOCKETCLOSE								=	MSG_GRP_SIMULATOR_3+0x12,
	MSG_SIMULATOR_SS_ENDUSSDCONNECTREQ						=	MSG_GRP_SIMULATOR_3+0x14,
	MSG_SIMULATOR_SS_QUERYCALLBARRINGSTATUS					=	MSG_GRP_SIMULATOR_3+0x16,
	MSG_SIMULATOR_SS_QUERYCALLFORWARDSTATUS					=	MSG_GRP_SIMULATOR_3+0x18,
	MSG_SIMULATOR_SS_QUERYCALLINGLINEIDSTATUS				=	MSG_GRP_SIMULATOR_3+0x1a,
	MSG_SIMULATOR_SS_QUERYCALLINGLINERESTRICTIONSTATUS		=	MSG_GRP_SIMULATOR_3+0x1c,
	MSG_SIMULATOR_SS_QUERYCALLWAITINGSTATUS					=	MSG_GRP_SIMULATOR_3+0x1e,
	MSG_SIMULATOR_SS_QUERYCONNECTEDLINEIDSTATUS				=	MSG_GRP_SIMULATOR_3+0x20,
	MSG_SIMULATOR_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS	=	MSG_GRP_SIMULATOR_3+0x22,
	MSG_SIMULATOR_SS_SENDCALLBARRINGPWDCHANGEREQ			=	MSG_GRP_SIMULATOR_3+0x24,
	MSG_SIMULATOR_SS_SENDCALLBARRINGREQ						=	MSG_GRP_SIMULATOR_3+0x26,
	MSG_SIMULATOR_SS_SENDCALLFORWARDREQ						=	MSG_GRP_SIMULATOR_3+0x28,
	MSG_SIMULATOR_SS_SENDCALLWAITINGREQ						=	MSG_GRP_SIMULATOR_3+0x2a,
	MSG_SIMULATOR_SS_SENDUSSDCONNECTREQ						=	MSG_GRP_SIMULATOR_3+0x2c,
	MSG_SIMULATOR_SS_SENDUSSDDATA							=	MSG_GRP_SIMULATOR_3+0x2e,
	MSG_SIMULATOR_SYS_CLIENTINIT							=	MSG_GRP_SIMULATOR_3+0x30,
	MSG_SIMULATOR_SYS_CLIENTRUN								=	MSG_GRP_SIMULATOR_3+0x32,
	MSG_SIMULATOR_SYS_DEREGISTERFORMSEVENT					=	MSG_GRP_SIMULATOR_3+0x34,
	MSG_SIMULATOR_SYS_GETRSSITHRESHOLD						=	MSG_GRP_SIMULATOR_3+0x36,
	MSG_SIMULATOR_SYS_GETRXSIGNALINFO						=	MSG_GRP_SIMULATOR_3+0x38,
	MSG_SIMULATOR_SYS_ISBANDSUPPORTED						=	MSG_GRP_SIMULATOR_3+0x3a,
	MSG_SIMULATOR_SYS_PROCESSNORFREQ						=	MSG_GRP_SIMULATOR_3+0x3c,
	MSG_SIMULATOR_SYS_PROCESSPOWERDOWNREQ					=	MSG_GRP_SIMULATOR_3+0x3e,
	MSG_SIMULATOR_SYS_PROCESSPOWERUPREQ						=	MSG_GRP_SIMULATOR_3+0x40,
	MSG_SIMULATOR_SYS_REGISTERFORMSEVENT					=	MSG_GRP_SIMULATOR_3+0x42,
	MSG_SIMULATOR_SYS_SELECTBAND							=	MSG_GRP_SIMULATOR_3+0x44,
	MSG_SIMULATOR_SYS_SETRSSITHRESHOLD						=	MSG_GRP_SIMULATOR_3+0x46,
	MSG_SIMULATOR_V24REGISTERCALLBACK						=	MSG_GRP_SIMULATOR_3+0x48,
	MSG_SIMULATOR_V24SEND									=	MSG_GRP_SIMULATOR_3+0x4a,
	MSG_SIMULATOR_CALLBACKMSG								=	MSG_GRP_SIMULATOR_3+0x4c,
	MSG_SIMULATOR_CC_REPORTCALLMETERVAL						=	MSG_GRP_SIMULATOR_3+0x4e,
	MSG_SIMULATOR_SIM_GET_SMSMEMEXC_FLAG					=	MSG_GRP_SIMULATOR_3+0x50,
	MSG_SIMULATOR_SIM_UPDATE_SMSCAPEXC_FLAG                 =	MSG_GRP_SIMULATOR_3+0x52,
#endif //UNDER_CE
#endif //UNDER_LINUX

    MSG_PDP_TEST_FIRST										= MSG_GRP_TEST_PCH+0x01,	
	MSG_PDP_TEST_CREATE_DATA_ACCT						    = MSG_GRP_TEST_PCH+0x02,	///<Payload type {::PDPTestCreateAccount_t}
    MSG_PDP_TEST_SETUP_CONN									= MSG_GRP_TEST_PCH+0x03,	///<Payload type {::PDPTestBasic_t}
    MSG_PDP_TEST_SHUTDOWN_CONN								= MSG_GRP_TEST_PCH+0x04,	///<Payload type {::PDPTestBasic_t}
    MSG_PDP_TEST_ISSecPDP									= MSG_GRP_TEST_PCH+0x05,	///<Payload type {::PDPTestBasic_t}
    MSG_PDP_TEST_GETPrimCid									= MSG_GRP_TEST_PCH+0x06,	///<Payload type {::PDPTestBasic_t}
    MSG_PDP_TEST_GETTFT										= MSG_GRP_TEST_PCH+0x07,	///<Payload type {::PDPTestBasic_t}
    MSG_PDP_TEST_SETTFT										= MSG_GRP_TEST_PCH+0x08,	///<Payload type {::PDPSetTFT_t}
    MSG_PDP_TEST_SCK										= MSG_GRP_TEST_PCH+0x09,	///<Payload type {::PDPTestSck_t}
    MSG_PDP_TEST_INJECT_DATA								= MSG_GRP_TEST_PCH+0x0a,	///<Payload type {::PDPTestInjectData_t}
    MSG_PDP_TEST_LAST										= MSG_GRP_TEST_PCH+0x0b,	

    MSG_TCP_DATA_TEST								        = MSG_GRP_TEST_PCH+0x21,	///<Payload type {::TCPClientMsg_t}
    MSG_UDP_DATA_TEST										= MSG_GRP_TEST_PCH+0x22,	///<Payload type {::UDPClientMsg_t}

	MSG_PCHEX_TEST_FIRST									= MSG_GRP_TEST_PCH+0xa1,	
	MSG_PCHEX_TEST_STARTUP									= MSG_GRP_TEST_PCH+0xa2,	
	MSG_PCHEX_TEST_ACTIVATEPDP								= MSG_GRP_TEST_PCH+0xa3,	///<Payload type {::IcmpPingInfo_t}
	MSG_PCHEX_TEST_ICMPPING									= MSG_GRP_TEST_PCH+0xa4,	///<Payload type {::UdpQuoteInfo_t}
	MSG_PCHEX_TEST_UDPQUOTE									= MSG_GRP_TEST_PCH+0xa5,	///<Payload type {::UdpQuoteInfo_t}
	MSG_PCHEX_TEST_DEACTIVATEPDP							= MSG_GRP_TEST_PCH+0xa6,	///<Payload type {::PdpDeactInfo_t}
	MSG_PCHEX_TEST_SHUTDOWN									= MSG_GRP_TEST_PCH+0xa7,
    MSG_PCHEX_TEST_PINGTIMER								= MSG_GRP_TEST_PCH+0xa8,	
	MSG_PCHEX_TEST_LAST										= MSG_GRP_TEST_PCH+0xa9,	

	// This message is used by ATC and the Script engine for a response to the AT*SCRIPT command
#ifdef XSCRIPT_INCLUDED
    MSG_SCRIPT_AT_CMD									    = MSG_GRP_SCRIPT+0x01,
    MSG_SCRIPT_AT_CMD_RSP									= MSG_GRP_SCRIPT+0x02,
    MSG_SCRIPT_STATUS	                                    = MSG_GRP_SCRIPT+0x03,
#endif //XSCRIPT_INCLUDED

	// FixMe - no such API fct in document

/** @endcond */

	//------------------------------------------------
	// end of list
	//-------------------------------------------------
	MSG_INVALID			= 0xFFFF	/* 65535 = 0xFFFF */	// This line must be here in order to generate binary inter-task message logging

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
Structure : API Client Information Type
**/
typedef struct
{
	UInt8	clientId;					///< Client Identifier
	UInt32	clientRef;					///< Client Reference
	UInt32	dialogId;					///< Dialog Identifier numarated by CAPI, see below
										///< MS_GetElement(<clienId>, MS_LOCAL_MS_ELEM_DIALOG_ID, <DialogId>);

} ClientInfo_t;							///< Client Information Type


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
	//StructFieldBegin{InterTaskMsg_t}	//Do not change!!! This line must be before the first field of InterTaskMsg_t. Hui Luo, 11/6/09
	UInt16					msgType;	///< global significant
	UInt16					dataLength;	///< payload data length in bytes
	UInt8					clientID;	///< client ID (ATC, MMI, and etc.)
	UInt8					usageCount;	///< for msg sharing used in broadcast
	T_INTER_TASK_MSG_PTR	dataBuf;	///< payload data (may include 0 as content)
	T_NU_NAME				sender;		///< sending task's name
	T_NU_NAME				receiver;	///< receiving task's name
	ClientInfo_t			clientInfo;	///< Client Information
	//StructFieldEnd{InterTaskMsg_t}	//Do not change!!! This line must be after the last field of InterTaskMsg_t. Hui Luo, 11/6/09
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
		AllocInterTaskMsgFromHeapDbg(msgType, dataLength, __FILE__, __LINENUM__)
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
#define	FreeInterTaskMsg(inMsg)	FreeInterTaskMsgDbg(inMsg, __FILE__, __LINENUM__)
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
	@return		Boolean
	
	@note
	This function is used to register the client to receive unicast message only. Clients can call -
	SYS_SetFilteredEventMask later to receive broadcast messages
	It returns FALSE if the number of clients exceed OR if the clientId is already registered.
**/	
Boolean SYS_RegisterForPredefinedClient(CallbackFunc_t* callback, UInt8 clientId);

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
void PostMsgToATCTask(InterTaskMsg_t* inMsg);
void MSC_Post_Msg(InterTaskMsg_t *msg); 
void SYS_BroadcastMsg(InterTaskMsg_t* inMsg);
Boolean IsATClient(UInt8 inClientID);

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

/** @endcond */

#endif  // _TASKMSGS_H_


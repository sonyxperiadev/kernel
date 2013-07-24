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
/*-------------------------------------------------
* Constant Definitions
-------------------------------------------------*/
#define CLIENT_INDEX_WILD_CARD				0xFF	///< Client Index Wild Card

#define	INVALID_CLIENT_ID					 0


#define  MAX_CLIENT_NUM						30		///< 5 tasks in platform (atc, bluetooth, dlink, stk ds, test MMI) using
													///< SYS_RegisterForMSEvent(), leaving 5 more clients for MMI/application use

//--- Internal/Reserved Client ID (200-255) ----------
#define	STK_CLIENT_ID  						 200	///< STK Client Identifier
#define	MNCC_CLIENT_ID  					 201	///< MNCC Client Identifier
#define DEFAULT_CLIENT_ID					 202	///< Internal default Client Identifier
#define MT_USSD_CLIENT_ID					 203	///< client ID assigned when a MT USSD is received.(for ATC use)

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
#define  MAX_FILTERED_EVENT_MASK_NUM		20

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
	MSG_GRP_SECMODEM            = 0x1000,

	MSG_GRP_INT_UTIL			= 0x2000,
	MSG_GRP_INT_ATC 			= 0x2100,
	MSG_GRP_INT_SMS_SS			= 0x2200,
	MSG_GRP_INT_NET				= 0x2300,
	MSG_GRP_INT_DC				= 0x2400,
	MSG_GRP_INT_SIM				= 0x2500,
	MSG_GRP_INT_STK				= 0x2600,
	MSG_GRP_INT_VCC				= 0x2700,
	MSG_GRP_INT_SECMODEM        = 0x2800,

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

	MSG_GRP_RTC					= 0x5000,

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

/*TASKMSGS_INCLUDE taskmsgs_modem.i*/
	//---------------------------------------------------------------
	// MSG_GRP_SYS, MESSAGE GROUP FOR SYSTEM (0x0000)
	//---------------------------------------------------------------

	/**
	This event is the response to the SYS_PowerDownReq() and confirms the system has detached from the network and 
	MMI can start the graceful shutdown procedure, e.g. flush FFS data. At this point the system is not completely
	powered off yet, e.g. the SIM interface is still active. During the modem power-cycle procedure for STK refresh
	of SIM reset type, the MMI/host shall not flush FFS data because the BRCM platform will re-power up the modem
	automatically.
	**/
	MSG_POWER_DOWN_CNF 			= MSG_GRP_SYS+0x01,

	/**
	This event indicates the system has been completely powered off. For a dual-sim build it means the modem instance
	is ready to be re-powered up. During the modem power-cycle procedure for STK refresh of SIM reset type, the BRCM 
	platform will automatically re-power up the modem.
	**/
	MSG_POWER_OFF_IND			= MSG_GRP_SYS+0x02,	

	MSG_ROAMING_STATUS			= MSG_GRP_SYS+0x03,

	MSG_POWERON_NO_RF_CNF		= MSG_GRP_SYS+0x04,
	// End of MSG_GRP_SYS (0x0000)

	//---------------------------------------------------------------
	// MSG_GRP_UTIL, MESSAGE GROUP FOR GENERAL PURPOSE (0x0100)
	//---------------------------------------------------------------

	/**
		This message is used by the ms_database to broadcast a message to the
		clients registered about a change in the value of any database element.
		The contents of the message are defined in MS_LocalElemNotifyInd_t
	**/
	MSG_MEASURE_REPORT_PARAM_IND	= MSG_GRP_INT_UTIL+0x00,	///<Payload type {::MS_RxTestParam_t}
	MSG_MS_LOCAL_ELEM_NOTIFY_IND = MSG_GRP_UTIL+0x10,			///<Payload type {::MS_LocalElemNotifyInd_t}

	 /** 
	CP->AP request message for writing CP persistent storage data to AP file system
	**/
	MSG_CPPS_WRITE_REQ  = MSG_GRP_UTIL + 0x20,	///<Payload type {::CAPI2_CPPS_Write_Req_t}

	 /** 
	AP->CP response message for writing CP persistent storage data to AP file system
	**/
	MSG_CPPS_WRITE_RSP  = MSG_GRP_UTIL + 0x21,	///<Payload type {::CAPI2_CPPS_Write_Rsp_t}

	 /** 
	CP->AP request message for reading CP persistent storage data from AP file system
	**/
	MSG_CPPS_READ_REQ  = MSG_GRP_UTIL + 0x22,	///<Payload type {::CAPI2_CPPS_Read_Req_t}

	 /** 
	CP->AP response message for reading CP persistent storage data from AP file system
	**/
	MSG_CPPS_READ_RSP  = MSG_GRP_UTIL + 0x23,	///<Payload type {::CAPI2_CPPS_Read_Rsp_t}

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

	MSG_CIPHER_ALG_IND       	= MSG_GRP_NET+0x12, ///<Payload type {::PHONECTRL_CIPHER_ALG_t}

	/**
	This reports the Localized service Area (LSA) identity of the cell that we are camped on currently
	Each cell belongs to a LSA and this information is sent by the network in either the MM or GMM Information message
	For the payload contents see :: lsaIdentity_t
	**/
	MSG_LSA_ID_IND				= MSG_GRP_NET+0x13,

	/**
	This is the event indicating the current UARFCN_DL value, it is sent when the UARFCN value is changed.
	**/
	MSG_NET_UARFCN_DL_IND		= MSG_GRP_NET+0x14, ///<Payload type {::UInt16}

	/**
	This is the event containing 2G/3G NW measurement results
	**/
	MSG_NW_MEAS_RESULT_IND		= MSG_GRP_NET+0x15, ///<Payload type {::PHONECTRL_NMR_t}

	/** 
	This reports suspended/resumed status of a SIM. This message is sent only for dual-sim build. 
	For the payload contents see ::SIM_INSTANCE_STATUS_t. 
	**/ 
	MSG_SIM_INSTANCE_STATUS_IND	= MSG_GRP_NET+0x16, 

	/** 
	This reports whether the power-saving feature has been invoked on a VM for dual-sim build. 
	This message is not sent in non-dual-sim build. The payload is a Boolean value. If the value
	is TRUE it indicates the VM has been powered down for power saving purpose and FALSE otherwise 
	**/
	MSG_VCC_VM_PWR_SAVING_IND       = MSG_GRP_NET+0x17, 

	/**
	This reports the RAT/Band change request has been completed.
	**/
	MSG_MS_SET_RAT_BAND_IND = MSG_GRP_NET + 0x18, /**< Payload type {::MSNetworkInfo_t} */
	/**
	This reports the MS class change request has been completed.
	**/
	/* Payload type {::MSClass_t} */
	MSG_PDP_SETMSCLASS_CNF    = MSG_GRP_NET+0x19,

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
	MSG_CALL_USER_INFORMATION		= MSG_GRP_CC+0x13,	///<Payload type {::SS_UserInfo_t}

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
	This is the event confirming that a data call has been released. See ::Cause_t for list of release causes
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

	/**
	This message reports the network notification indication.
	For the payload contents see :: SS_NotifySs_t.
	**/
	MSG_MNCC_CLIENT_NOTIFY_SS_IND	= MSG_GRP_CC+0x52,	///<Payload type {::SS_NotifySs_t}

	/**
	This message convaies the Facility IEs return error or reject to the client
	For the payload contents see :: CC_FacilityInd_t.
	**/
	MSG_MNCC_CLIENT_FACILITY_IND	= MSG_GRP_CC+0x53,	///<Payload type {::CC_FacilityInd_t}


	MSG_LAST_CALL_DROP_INFO  = MSG_GRP_CC+0x54,	///<Payload type {::CcCallDropInfo_t}
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
	Response to read SMS data command .For the payload contents see ::SmsSimMsg_t
	**/

	//SMS data contents
	MSG_SIM_SMS_DATA_RSP		= MSG_GRP_SMS+0x0A,	///<Payload type {::SmsSimMsg_t}

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

	MSG_SMSPP_OTA_SYNC_IND		= MSG_GRP_SMS+0x44,		///<Payload type {::SmsSimMsg_t}

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

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// Messages moved to taskmsgs_dataservices.i
//	MSG_PPP_OPEN_IND					= MSG_GRP_DC+0x02,
//	MSG_PPP_CLOSE_IND					= MSG_GRP_DC+0x03,
//
//	// for DC broadcasting
//	MSG_DC_REPORT_CALL_STATUS			= MSG_GRP_DC+0x10,	///<Payload type {::DC_ReportCallStatus_t}
//	MSG_DC_IR_SHUTDOWN_CONNECTION_REQ	= MSG_GRP_DC+0x11,	///<Payload type {::DC_IR_ShutdownDataConnection_t}
//	MSG_DC_IR_STARTUP_CONNECTION_REQ	= MSG_GRP_DC+0x12,	///<Payload type {::DC_IR_StartupDataConnection_t}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

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
	MSG_PDP_ACTIVATION_PDU_RSP			= MSG_GRP_DC+0x37,	///<Payload type {::PDP_SendPDPActivatePDUReq_Rsp_t}
	MSG_PDP_ACTIVATION_NWI_IND			= MSG_GRP_DC+0x38,	///<Payload type {::PDP_ActivateNWI_Ind_t}
	MSG_PDP_ACTIVATION_SEC_NWI_IND		= MSG_GRP_DC+0x39,	///<Payload type {::PDP_ActivateSecNWI_Ind_t}
	MSG_SNPDU_IND						= MSG_GRP_DC+0x50,

	// End of MSG_GRP_DC, (0x0600)


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
	/**
	This SATK message is used to request Date & Time info from client
	**/
	MSG_SATK_EVENT_PROV_LOCAL_DATE		= MSG_GRP_STK+0x19,	///<Payload type {::SATK_EventData_t}

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
	MSG_STK_SEND_USER_CONFIRMATION_REQ	= MSG_GRP_STK+0x70,

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
	contents see ::SIM_MUL_PLMN_ENTRY_UPDATE_t.
	**/

	//Result for updating PLMN entries in SIM
	MSG_SIM_MUL_PLMN_ENTRY_UPDATE_RSP		= MSG_GRP_SIM+0x23,	///<Payload type {::SIM_MUL_PLMN_ENTRY_UPDATE_t}

	//Result for reading multiple records in SIM file
	MSG_SIM_MUL_REC_DATA_RSP				= MSG_GRP_SIM+0x24,	///<Payload type {::SIM_MUL_REC_DATA_t}

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
	MSG_SIM_GENERIC_ACCESS_RSP			= MSG_GRP_SIM+0x61,

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

	/**
	This message contains the response for "SIM Select and send APDU request".
	For payload contents see :: SimApduRsp_t
	**/
	MSG_SIM_SEND_APDU_RSP				= MSG_GRP_SIM+0x6A,	///<Payload type {::SimApduRsp_t}

	/**
	This message contains the response for "SIM select application request".
	For payload contents see :: SIM_SELECT_APPLICATION_RES_t
	**/
	MSG_SIM_SELECT_APPLICATION_RSP		= MSG_GRP_SIM+0x6B,	///<Payload type {::SIM_SELECT_APPLICATION_RES_t}
    

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
	 * 1. EF-IMSI does not exist or is set to all 0xFF. MMI shall display a warning message to user, but MMI can choose to continue to access other 
	 *    SIM data such as phonebook. 
	 *
	 * 2. The "invalid SIM" status triggered by continual SW1/SW2=0x6F/0x00 status words response from the SIM. In AT&T 16966 UICC Application 
	 *    Interoperability Test Plan, the "IMEI Lock" application will send SW1/SW2=0x6F/0x00 continually if the IMEI does not match in the terminal 
	 *    response for Provide Local Info (Get IMEI) proactive command. 
	 *
	 *    Upon receiving this message, MMI shall immediately terminate all SIM access and display "invalid SIM" to the user. 
	 *    Only emergency call is allowed. 
	 *
	 * 3. EF-ACC does not exist or is set to 0 (all access classes disabled). MMI shall display a warning message to user, but MMI can choose to 
	 *    continue to access other SIM data such as phonebook. 
	 *
	 */
	MSG_SIM_FATAL_ERROR_IND				= MSG_GRP_SIM+0x83,	///<Payload type: none

	/**
	This SIM message is to notify ATC/MMI that the SIM card has been recovered. 
	**/
	MSG_SIM_SIM_RECOVER_IND	            = MSG_GRP_SIM+0x84,	///<Payload type: none

	/**
	This SIM message indicates all SIM cached data except SMS & PBK is ready
	**/
	MSG_SIM_CACHED_DATA_READY_IND		= MSG_GRP_SIM+0x90,

	/**
	This SIM message indicates the PIN1/2 status
	**/
	MSG_SIM_PIN_IND                     = MSG_GRP_SIM+0x91, ///<Payload type {::SimPinInd_t}
    
	/** 
	SIM Simlock Data (IMSI/GID1/GID2) sent if PIN1 is unlocked or disabled
	**/
	MSG_SIMLOCK_SIM_DATA_IND				= MSG_GRP_SIM+0x92,	///<Payload type {::SIMLOCK_SIM_DATA_t}

	// End of MSG_GRP_SIM (0x0C00)

	//---------------------------------------------------------------
	// MSG_GRP_SECMODEM, MESSAGE GROUP FOR SECURE MODEM (0x1000)
	//---------------------------------------------------------------
	MSG_SECMODEM_SIMLOCK_STATUS_IND     = MSG_GRP_SECMODEM+0x00, ///<Payload type {::UInt8}
	MSG_SECMODEM_XSIM_STATUS_IND        = MSG_GRP_SECMODEM+0x01, ///<Payload type {::UInt8}
	MSG_SECMODEM_CONFIG_MODEM_RSP       = MSG_GRP_SECMODEM+0x02, ///<Payload type {::UInt8}


	// End of MSG_GRP_SECMODEM (0x1000)

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

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// messages moved to taskmsgs_atc.i
//	MSG_AT_CMD_STR				= MSG_GRP_INT_ATC+0x35,	///<Payload type {::}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

	MSG_AT_LINE_STATE_IND		= MSG_GRP_INT_ATC+0x36,	///<Payload type {::Boolean}

	MSG_MS_READY_IND			= MSG_GRP_INT_ATC+0x50,	///<Payload type {::MODULE_READY_STATUS_t}
	MSG_CALL_MONITOR_STATUS		= MSG_GRP_INT_ATC+0x51,	///<Payload type {::UInt8}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// messages moved to taskmsgs_atc.i
//	MSG_AT_COMMAND_REQ			= MSG_GRP_INT_ATC+0x52,	///<Payload type {::AtCmdInfo_t}
//	MSG_AT_COMMAND_IND			= MSG_GRP_INT_ATC+0x53,	///<Payload type {::AtCmdInfo_t}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

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
	MSG_SMS_CMD_PARM_CHECK_RSP	= MSG_GRP_INT_SMS_SS+0x0C,
	MSG_SMS_SUBMIT_VCC_TIMEOUT_IND = MSG_GRP_INT_SMS_SS+0x0D,
	MSG_SMS_SMMA_RETRY_REQ		= MSG_GRP_INT_SMS_SS+0x0E,
	/* <Payload type {::SmsConcatMsgStored_t} */
	MSG_SMS_SUBMIT_CONCAT_RETRY_REQ	= MSG_GRP_INT_SMS_SS+0x0F,

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

	MSG_INT_TIMEZONE_IND		= MSG_GRP_INT_NET+0x13,	///<Payload type {::MMMsgParmTimeZoneInd_t}
	MSG_INT_DATE_IND			= MSG_GRP_INT_NET+0x14, ///<Payload type {::MMMsgParmDateInd_t}
	MSG_INT_LSA_IND				= MSG_GRP_INT_NET+0x15, ///<Payload type {::MMMsgParmLSAInd_t}
	MSG_ACTIVATE_SEC_IND		= MSG_GRP_INT_NET+0x16,	///<Payload type {::Inter_ActivateSecInd_t}

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

	MSG_SIM_APDU_GENERIC_ACCESS_REQ         = MSG_GRP_INT_SIM+0x32, ///<Request for generic SIM Accesss, e.g. AT+CSIM command */


	MSG_SIM_PROC_RESTRICTED_ACCESS_REQ		= MSG_GRP_INT_SIM+0x33,	///<Payload type {::SIM_PROC_RESTRICTED_ACCESS_REQ_t}	/* Request for restricted SIM Accesss, e.g. AT+CRSM command */

	MSG_SIM_PROC_SET_EST_SERV_REQ			= MSG_GRP_INT_SIM+0x34,	///<Payload type {::USIM_SET_EST_SERV_REQ_t}	/* Request to activate/deactivate service in EF-EST in USIM */

	MSG_SIM_PROC_UPDATE_ONE_APN_REQ			= MSG_GRP_INT_SIM+0x35,	///<Payload type {::USIM_UPDATE_ONE_APN_REQ_t}	/* Request to update one APN in EF-ACL in USIM */
	MSG_SIM_PROC_DELETE_ALL_APN_REQ			= MSG_GRP_INT_SIM+0x36,	///<Payload type {::USIM_DELETE_ALL_APN_REQ_t}	/* Request to delete all APN's in EF-ACL's in USIM */

	//										= MSG_GRP_INT_SIM+0x40, ///< Empty slot!!!
	MSG_SIM_PROC_GENERAL_SERVICE_IND		= MSG_GRP_INT_SIM+0x41,	///<Payload type {::SIMGeneralServiceStatus_t}	/* SIM general service indication */

	MSG_SIM_PROC_SEEK_RECORD_REQ			= MSG_GRP_INT_SIM+0x42,	///<Payload type {::SIM_PROC_SEEK_RECORD_REQ_t}/* Request to send Search Record command (Seek command in 2G SIM) */

	MSG_SIM_PROC_NUM_OF_PLMN_REQ			= MSG_GRP_INT_SIM+0x43,	///<Payload type {::SIM_NUM_OF_PLMN_REQ_t}	/* Request to get number of PLMN entries in SIM file */

	MSG_SIM_PROC_READ_PLMN_REQ				= MSG_GRP_INT_SIM+0x44,	///<Payload type {::SIM_READ_PLMN_REQ_t}	/* Request to read PLMN entries in SIM file */

	MSG_SIM_PROC_UPDATE_MUL_PLMN_REQ		= MSG_GRP_INT_SIM+0x45,	///<Payload type {::SIM_UPDATE_MUL_PLMN_REQ_t}	/* Request to update multiple PLMN entries in SIM file */

	MSG_SIM_PROC_ISIM_ACTIVATE_REQ			= MSG_GRP_INT_SIM+0x46,	///<Payload type {::SIM_PROC_ISIM_ACTIVATE_REQ_t}	/* Request to activate ISIM application */

	MSG_SIM_PROC_ISIM_AUTHEN_AKA_REQ		= MSG_GRP_INT_SIM+0x47,	///<Payload type {::SIM_PROC_ISIM_AUTHENTICATE_REQ_t}	/* Request to perform ISIM AKA Security Context authentication request */

	MSG_SIM_PROC_ISIM_AUTHEN_HTTP_REQ		= MSG_GRP_INT_SIM+0x48,	///<Payload type {::SIM_PROC_ISIM_AUTHENTICATE_REQ_t}	/* Request to perform ISIM HTTP Digest Security Context authentication request */

	MSG_SIM_PROC_ISIM_AUTHEN_GBA_BOOT_REQ	= MSG_GRP_INT_SIM+0x49,	///<Payload type {::SIM_PROC_ISIM_AUTHENTICATE_REQ_t}	/* Request to perform ISIM Bootstrapping Mode Security Context authentication request */

	MSG_SIM_PROC_ISIM_AUTHEN_GBA_NAF_REQ	= MSG_GRP_INT_SIM+0x4A,	///<Payload type {::SIM_PROC_SET_BDN_REQ_t}	/* Request to perform ISIM NAF Derivation Mode Security Context authentication request */

	MSG_SIM_PROC_SET_BDN_REQ				= MSG_GRP_INT_SIM+0x4B,	///<Payload type {::SIM_PROC_SET_BDN_REQ_t}	/* Request to turn BDN on/off */

	MSG_SIM_PROC_POWER_ON_OFF_CARD_REQ	    = MSG_GRP_INT_SIM+0x4C,	///<Payload type {::SIM_PROC_POWER_ON_OFF_CARD_REQ_t}	/* Request to power on/off SIM card */

	MSG_SIM_PROC_GET_RAW_ATR_REQ	        = MSG_GRP_INT_SIM+0x4D,	///<Payload type {::SIM_PROC_GET_RAW_ATR_REQ_t}	/* Request to get raw ATR info */

	MSG_SIM_PROC_OPEN_SOCKET_REQ			= MSG_GRP_INT_SIM+0x4E,	///<Payload type {::SIM_PROC_OPEN_SOCKET_REQ_t}	/* Request to open a logical channel */

	MSG_SIM_PROC_SELECT_APPLI_REQ			= MSG_GRP_INT_SIM+0x4F,	///<Payload type {::SIM_PROC_SELECT_APPLI_REQ_t} /* Request to select a non-USIM application */

	MSG_SIM_PROC_DEACTIVATE_APPLI_REQ		= MSG_GRP_INT_SIM+0x50,	///<Payload type {::SIM_PROC_DEACTIVATE_APPLI_REQ_t} /* Request to deactivate a non-USIM application */

	MSG_SIM_PROC_CLOSE_SOCKET_REQ			= MSG_GRP_INT_SIM+0x51,	///<Payload type {::SIM_PROC_CLOSE_SOCKET_REQ_t}	/* Request to close a logical channel */

	MSG_SIM_PROC_STEERING_OF_ROAMING_REQ	= MSG_GRP_INT_SIM+0x52,	///<Payload type {::SimProcSteeringOfRoamingReq_t}	/* Request to perform Steering of Roaming */

	MSG_SIM_PROC_READ_MUL_REC_REQ			= MSG_GRP_INT_SIM+0x53,	///<Payload type {::SIM_PROC_MUL_REC_REQ_t}		/* Request to read multiple records in SIM file */

	MSG_SIM_PROC_SELECT_FILE_SEND_APDU_REQ  = MSG_GRP_INT_SIM+0x54,	///<Payload type {::SimProcSelectFileSendApduReq_t}	/* Request to select a file and send a APDU */   	

	MSG_SIM_PROC_SELECT_APPLICATION_REQ		= MSG_GRP_INT_SIM+0x55,	///<Payload type {::SIM_PROC_SELECT_APPLICATION_REQ_t} /* Request to select a non-USIM application */

	MSG_SIM_PROC_DETECT_IND                 = MSG_GRP_INT_SIM+0x56, ///<Payload type {::Boolean}        /* SIM detection indication based on SIMIO interrupt */

	//----------------------------------------------------------------------
	// MSG_GRP_INT_STK, INTERNAL MESSAGE GROUP FOR STK (0x2600)
	//----------------------------------------------------------------------

	MSG_STK_MENU_SELECTION_RES					= MSG_GRP_INT_STK+0x20,

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
	This STK message is used to request the activation of UICC interface, e.g. the
	UICC-CLF interface defined in section 8.89 in ETSI 102 223.
	**/
	MSG_STK_ACTIVATE_REQ						= MSG_GRP_INT_STK+0x39,

	// Internal Messgae for setup call
	MSG_CALL_ESTABLISH_REQ			= MSG_GRP_INT_STK+0x80,	///<Payload type {::CCParmSend_t}

	// Internal Messages for Call Control

	MSG_STK_SEND_CC_SETUP_REQ       = MSG_GRP_INT_STK+0x81,	///<Payload type {::SATK_SendCcSetupReq_t}
	MSG_STK_SEND_CC_SS_REQ          = MSG_GRP_INT_STK+0x82,	///<Payload type {::SATK_SendCcSsReq_t}
	MSG_STK_SEND_CC_USSD_REQ        = MSG_GRP_INT_STK+0x83,	///<Payload type {::SATK_SendCcUssdReq_t}
	MSG_STK_SEND_CC_SMS_REQ         = MSG_GRP_INT_STK+0x84,	///<Payload type {::SATK_SendCcSmsReq_t}
	MSG_STK_SEND_ENVELOPE_CMD_REQ   = MSG_GRP_INT_STK+0x85,	///<Payload type {::SATK_SendEnvelopCmdReq_t}
	MSG_STK_SEND_TERMINAL_RSP_REQ   = MSG_GRP_INT_STK+0x86,	///<Payload type {::SATK_SendTerminalRspReq_t}

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
	MSG_STK_GET_CHANNEL_STATUS_REQ	= MSG_GRP_INT_STK+0x9B,	///<No payload

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
	MSG_STK_LOCAL_IMEISV_REQ        = MSG_GRP_INT_STK+0xB4,
	MSG_STK_LOCAL_NW_SEARCH_MODE_REQ = MSG_GRP_INT_STK+0xB5,
	MSG_STK_LOCAL_BATTERY_STATE_REQ = MSG_GRP_INT_STK+0xB6,
	MSG_STK_NW_SEARCH_MODE_CHG_IND  = MSG_GRP_INT_STK+0xB7,
	MSG_STK_BROWSING_STATUS_IND     = MSG_GRP_INT_STK+0xB8,
	MSG_STK_SEND_IPAT_REQ           = MSG_GRP_INT_STK+0xB9,
	MSG_STK_TERMINAL_PROFILE_REQ    = MSG_GRP_INT_STK+0xBA,	///<Payload type {::StkTermProfileReq_t}
	MSG_STK_TERMINAL_PROFILE_IND    = MSG_GRP_INT_STK+0xBB,	///<Payload type {::StkTermProfileInd_t}
	MSG_STK_UICC_CAT_IND            = MSG_GRP_INT_STK+0xBC,	///<Payload type {::STKUiccCatInd_t}
	MSG_STK_PROACTIVE_CMD_FETCHING_ONOFF_REQ  = MSG_GRP_INT_STK+0xBD, ///<Payload type {::StkProactiveCmdFetchingOnOffReq_t}
	MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP = MSG_GRP_INT_STK+0xBE, ///<Payload type {::StkProactiveCmdFetchingOnOffRsp_t}
	MSG_STK_SEND_EXT_PROACTIVE_CMD_REQ   = MSG_GRP_INT_STK+0xBF,	///<Payload type {::StkExtProactiveCmdReq_t}
	MSG_STK_SEND_POLLING_INTERVAL_REQ  = MSG_GRP_INT_STK+0xC0,	///<Payload type {::StkPollingIntervalReq_t}
	MSG_STK_POLLING_INTERVAL_RSP       = MSG_GRP_INT_STK+0xC1,	///<Payload type {::StkPollingIntervalRsp_t}
	MSG_STK_EXT_PROACTIVE_CMD_IND      = MSG_GRP_INT_STK+0xC2,	///<Payload type {::StkExtProactiveCmdInd_t}
	// End of MSG_GRP_INT_DC, (0x2600)
	
	// VCC incoming messages
	MSG_VMC_CAPI_ADMISSION_CNF		= MSG_GRP_INT_VCC + 0x00,
	MSG_VMC_CAPI_ADMISSION_REJ		= MSG_GRP_INT_VCC + 0x01,
	MSG_VMC_STATUS_IND				= MSG_GRP_INT_VCC + 0x02,
	MSG_VCC_CAPI_IDLE_IND			= MSG_GRP_INT_VCC + 0x03,
	MSG_VCC_CAPI_SERV_REQ			= MSG_GRP_INT_VCC + 0x04,
	MSG_VCC_RETRY_TIMER_EXPIRE		= MSG_GRP_INT_VCC + 0x05,
	MSG_VCC_CHECK_VM_REQ			= MSG_GRP_INT_VCC + 0x06,
	MSG_VCC_CAPI_CANCEL_SERV_REQ	= MSG_GRP_INT_VCC + 0x07,

	// VCC outgoing messages
	MSG_VMC_CAPI_ADMISSION_REQ		= MSG_GRP_INT_VCC + 0x10,
	MSG_VMC_CAPI_IDLE_IND			= MSG_GRP_INT_VCC + 0x11,
	MSG_CAPI_VCC_TIMEOUT			= MSG_GRP_INT_VCC + 0x12,

	// Redefinitions
	MSG_MS_SEARCH_PLMN_RSP				= MSG_PLMNLIST_IND,
	MSG_SIM_WHOLE_EFILE_DATA_RSP		= MSG_SIM_EFILE_DATA_RSP,
	MSG_SIM_RECORD_EFILE_DATA_RSP		= MSG_SIM_EFILE_DATA_RSP,
	MSG_SIM_LINEAR_EFILE_UPDATE_RSP		= MSG_SIM_EFILE_UPDATE_RSP,
	MSG_SIM_SEEK_RECORD_RSP				= MSG_SIM_SEEK_REC_RSP,
	MSG_SIM_CYCLIC_EFILE_UPDATE_RSP		= MSG_SIM_EFILE_UPDATE_RSP,
	MSG_MS_PLMN_SELECT_RSP				= MSG_PLMN_SELECT_CNF,
	MSG_READ_USIM_PBK_HDK_RSP			= MSG_READ_USIM_PBK_HDK_ENTRY_RSP,
	MSG_WRITE_USIM_PBK_HDK_RSP			= MSG_WRITE_USIM_PBK_HDK_ENTRY_RSP,
	MSG_WRITE_USIM_PBK_ALPHA_AAS_RSP	= MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_WRITE_USIM_PBK_ALPHA_GAS_RSP	= MSG_WRITE_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_READ_USIM_PBK_ALPHA_AAS_RSP		= MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_READ_USIM_PBK_ALPHA_GAS_RSP		= MSG_READ_USIM_PBK_ALPHA_ENTRY_RSP,
	MSG_GET_USIM_PBK_ALPHA_INFO_AAS_RSP	= MSG_GET_USIM_PBK_ALPHA_INFO_RSP,
	MSG_GET_USIM_PBK_ALPHA_INFO_GAS_RSP	= MSG_GET_USIM_PBK_ALPHA_INFO_RSP,

	/// From platform to LCS task. Notify RRLP data received from stack
	MSG_LCS_RRLP_DATA_IND								 	= MSG_GRP_LCS+0x81, ///<Payload type {::LcsMsgData_t}
	/// From platform to LCS task. RRLP reset position stored information
	MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND				 	= MSG_GRP_LCS+0x82, ///<Payload type {::ClientInfo_t}
	/// From platform to LCS task. Notify RRC asistance data received from stack
	MSG_LCS_RRC_ASSISTANCE_DATA_IND						 	= MSG_GRP_LCS+0x83, ///<Payload type {::LcsMsgData_t}
	/// From platform to LCS task. Notify RRC measurement control received from stack
	MSG_LCS_RRC_MEASUREMENT_CTRL_IND					 	= MSG_GRP_LCS+0x84, ///<Payload type {::LcsRrcMeasurement_t}
	/// From platform to LCS task. Notify RRC system info received from stack
	MSG_LCS_RRC_BROADCAST_SYS_INFO_IND					 	= MSG_GRP_LCS+0x85, ///<Payload type {::LcsRrcBroadcastSysInfo_t}
	/// From platform to LCS task. Notify RRC UE state received from stack
	MSG_LCS_RRC_UE_STATE_IND							 	= MSG_GRP_LCS+0x86, ///<Payload type {::LcsRrcUeState_t}
	/// From platform to LCS task. Stop measurement
	MSG_LCS_RRC_STOP_MEASUREMENT_IND					 	= MSG_GRP_LCS+0x87, ///<Payload type {::ClientInfo_t}
	/// From platform to LCS task. RRC reset position stored information
	MSG_LCS_RRC_RESET_POS_STORED_INFO_IND				 	= MSG_GRP_LCS+0x88, ///<Payload type {::ClientInfo_t}

	//----------------------------------------------------------------------
	// MSG_GRP_INT_SECMODEM, MESSAGE GROUP FOR SECURE MODEM (0x2800)
	//----------------------------------------------------------------------
	MSG_SECMODEM_PROC_CONFIG_MODEM_REQ  = MSG_GRP_INT_SECMODEM + 0x00, ///<Payload type {::SecModemConfigModemReq_t}
	MSG_SECMODEM_PROC_HOST_TO_MODEM_REQ = MSG_GRP_INT_SECMODEM + 0x01, ///<Payload type {::SecModemHostToModemReq_t}
	MSG_SECMODEM_PROC_MODEM_TO_HOST_RSP = MSG_GRP_INT_SECMODEM + 0x02, ///<Payload type {::SecModemModemToHostRsp_t}

	MSG_SECMODEM_PROC_SET_INFO_PP_BITS_REQ = MSG_GRP_INT_SECMODEM + 0x03, ///<Payload type {::SecModemSetInfoPPBitsReq_t}

	// End of MSG_GRP_INT_SECMODEM, (0x2800)
/*TASKMSGS_INCLUDE taskmsgs_atc.i*/

	MSG_AT_CMD_STR				= MSG_GRP_INT_ATC+0x35,	///<Payload type {::}
	MSG_AT_COMMAND_REQ			= MSG_GRP_INT_ATC+0x52,	///<Payload type {::AtCmdInfo_t}
	MSG_AT_COMMAND_IND			= MSG_GRP_INT_ATC+0x53,	///<Payload type {::AtCmdInfo_t}
	MSG_AT_REGISTER_REQ			= MSG_GRP_INT_ATC+0x54, 	///<Payload type {::AtRegisterInfo_t}
	MSG_AT_TONE_REQ				= MSG_GRP_INT_ATC+0x55, 	///<Payload type {::AtToneInfo_t}
	MSG_AT_AUDIO_REQ			= MSG_GRP_INT_ATC+0x56,		///<payload type {::Boolean}
	MSG_AT_MICMUTE_REQ			= MSG_GRP_INT_ATC+0x57,		///<payload type {::Boolean}
	MSG_AT_SPEAKERMUTE_REQ			= MSG_GRP_INT_ATC+0x58,		///<payload type {::Boolean}
	MSG_AT_SETSPEAKER_REQ			= MSG_GRP_INT_ATC+0x59, 	///<payload type {::UInt32}
	MSG_AT_GETSPEAKER_REQ			= MSG_GRP_INT_ATC+0x5a, 	///<payload type {::void}
	MSG_AT_GETSPEAKER_RSP		= MSG_GRP_INT_ATC+0x5b, 	///<payload type {::UInt32}
	MSG_AT_SETMIC_REQ			= MSG_GRP_INT_ATC+0x5c, 	///<payload type {::UInt32}
	MSG_AT_GETMIC_REQ			= MSG_GRP_INT_ATC+0x5d, 	///<payload type {::void}
	MSG_AT_GETMIC_RSP		= MSG_GRP_INT_ATC+0x5e, 	///<payload type {::UInt32}
	MSG_AT_DUN_CONNECT_REQ		= MSG_GRP_INT_ATC+0x5f, 	///<payload type {::AtDUNInfo_t}
	MSG_AT_DUN_DISCONNECT_REQ		= MSG_GRP_INT_ATC+0x60, 	///<payload type {::AtDUNInfo_t}
	MSG_AT_FLIGHT_MODE_REQ		= MSG_GRP_INT_ATC+0x61, 	///<payload type {::AtFlightMode_t}
	MSG_AT_APCOMMAND_REQ			= MSG_GRP_INT_ATC+0x62,	///<Payload type {::AtCmdInfo_t}
	MSG_AT_ADD_TERMINAL_REQ			= MSG_GRP_INT_ATC+0x63,	///<Payload type {::SerialDeviceID_t}
	MSG_AT_REMOVE_TERMINAL_REQ			= MSG_GRP_INT_ATC+0x64,	///<Payload type {::SerialDeviceID_t}
	MSG_AT_POWER_RESET_REQ			= MSG_GRP_INT_ATC+0x65,	///<Payload type {::UInt8}
	MSG_AT_LOW_POWER_MODE_REQ		= MSG_GRP_INT_ATC+0x66,	///<Payload type {::void}
	/* <Payload type {::UInt8} */
	MSG_AT_START_CALD_REQ		= MSG_GRP_INT_ATC+0x67,
	/* <Payload type {::UInt8} */
	MSG_AT_END_CALD_REQ		= MSG_GRP_INT_ATC+0x68,
	/* <Payload type {::AtCallConnInfo_t} */
	MSG_AT_CALL_CONN_REQ		= MSG_GRP_INT_ATC+0x69,
	/*TASKMSGS_INCLUDE taskmsgs_usb.i*/
	//---------------------------------------------------------------
	// MSG_GRP_DEV, MESSAGE GROUP FOR DEVICES (0x0700)
	//---------------------------------------------------------------

	/** This message is broadcast by the filesystem when volume utilization exceeds a threshold
	**/
	MSG_FS_VOLUME_USE_IND       		= MSG_GRP_DEV+0x01,	///<Payload type {::FsVolumeUseInd_t}

	/** This message is broadcast by the filesystem when removable media is inserted/removed
	**/
	MSG_FS_REMOVABLE_MEDIA_INSERT_EVENT	= MSG_GRP_DEV+0x02,	///<Payload type {::FS_RemovableMediaInsertEvent_t}

	//USB Messages
	MSG_USB_MSC_ACTIVE					= MSG_GRP_DEV+0x20,	/*An internal message to notify the USB msc active*/
	MSG_USB_MSC_DEACTIVE				= MSG_GRP_DEV+0x21,	/*An internal message to notify the USB msc deactive*/
	MSG_USB_ADAPTER_IND					= MSG_GRP_DEV+0x22,
	MSG_INTERTASK_MSG_REQ				= MSG_GRP_DEV+0x23,
	MSG_INTERTASK_MSG_RSP				= MSG_GRP_DEV+0x24,
	MSG_USB_AP_INTERTASK_RSP			= MSG_GRP_DEV+0x25,
	MSG_ADC_DRX_SYNC_IND				= MSG_GRP_DEV+0x26,

	MSG_AUDIO_CTRL_GENERIC_REQ			= MSG_GRP_DEV+0x27,
	MSG_AUDIO_CTRL_GENERIC_RSP			= MSG_GRP_DEV+0x28,
	MSG_AUDIO_CTRL_DSP_REQ				= MSG_GRP_DEV+0x29,
	MSG_AUDIO_CTRL_DSP_RSP				= MSG_GRP_DEV+0x2A,
	MSG_DEV_BCD_SUCCESSFUL				= MSG_GRP_DEV+0x2B,
	MSG_DEV_BCD_FAILED					= MSG_GRP_DEV+0x2C,
	MSG_AUDIO_COMP_FILTER_REQ			= MSG_GRP_DEV+0x2D,
	MSG_AUDIO_COMP_FILTER_RSP			= MSG_GRP_DEV+0x2E,
	MSG_AUDIO_CALL_STATUS_IND			= MSG_GRP_DEV+0x2F,
	MSG_AUDIO_START_TUNING_IND			= MSG_GRP_DEV+0x30,
	MSG_AUDIO_STOP_TUNING_IND			= MSG_GRP_DEV+0x31,
	MSG_AUDIO_TUNING_SETPARM_IND			= MSG_GRP_DEV+0x32,
	MSG_AUDIO_VCALL_REL_IND				= MSG_GRP_DEV+0x33, /* voice call is released by modem */

	// End of MSG_GRP_DEV, (0x0700)


/*TASKMSGS_INCLUDE taskmsgs_agps.i*/
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
	// From LCS task to clients. 
	MSG_LCS_POSITION_FIX_READY_IND							= MSG_GRP_LCS+0x40,
	// From LCS task to clients. 
	MSG_LCS_STATUS_IND										= MSG_GRP_LCS+0x41,
	// From LCS task to clients. 
	MSG_LCS_START_FIX_IND									= MSG_GRP_LCS+0x42,	
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
	// From LCS task to clients. 
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

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
// messages moved to taskmsgs_modem.i
//	/// From platform to LCS task. Notify RRLP data received from stack
//	MSG_LCS_RRLP_DATA_IND								 	= MSG_GRP_LCS+0x81, ///<Payload type {::LcsMsgData_t}
//	/// From platform to LCS task. RRLP reset position stored information
//	MSG_LCS_RRLP_RESET_POS_STORED_INFO_IND				 	= MSG_GRP_LCS+0x82, ///<Payload type {::ClientInfo_t}
//	/// From platform to LCS task. Notify RRC asistance data received from stack
//	MSG_LCS_RRC_ASSISTANCE_DATA_IND						 	= MSG_GRP_LCS+0x83, ///<Payload type {::LcsMsgData_t}
//	/// From platform to LCS task. Notify RRC measurement control received from stack
//	MSG_LCS_RRC_MEASUREMENT_CTRL_IND					 	= MSG_GRP_LCS+0x84, ///<Payload type {::LcsRrcMeasurement_t}
//	/// From platform to LCS task. Notify RRC system info received from stack
//	MSG_LCS_RRC_BROADCAST_SYS_INFO_IND					 	= MSG_GRP_LCS+0x85, ///<Payload type {::LcsRrcBroadcastSysInfo_t}
//	/// From platform to LCS task. Notify RRC UE state received from stack
//	MSG_LCS_RRC_UE_STATE_IND							 	= MSG_GRP_LCS+0x86, ///<Payload type {::LcsRrcUeState_t}
//	/// From platform to LCS task. Stop measurement
//	MSG_LCS_RRC_STOP_MEASUREMENT_IND					 	= MSG_GRP_LCS+0x87, ///<Payload type {::ClientInfo_t}
//	/// From platform to LCS task. RRC reset position stored information
//	MSG_LCS_RRC_RESET_POS_STORED_INFO_IND				 	= MSG_GRP_LCS+0x88, ///<Payload type {::ClientInfo_t}
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

	/// From platform to LCS task. The response message for LCS_FttSyncReq()
	MSG_LCS_FTT_SYNC_RESULT_IND								= MSG_GRP_LCS+0x89, ///<Payload type {::LcsFttResult_t} 
	MSG_LCS_WLAN_DEV_OPEN_REQ								= MSG_GRP_LCS+0x8a, ///<Payload type {::ClientInfo_t}
	MSG_LCS_WLAN_DEV_CLOSE_REQ								= MSG_GRP_LCS+0x8b, ///<Payload type {::ClientInfo_t}
	MSG_LCS_WLAN_SCAN_REQ									= MSG_GRP_LCS+0x8c, ///<Payload type {::ClientInfo_t}

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

	MSG_LCS_APP_WLAN_DEV_STATE_NTF							= MSG_GRP_LCS+0xd7, ///<Payload type {::LcsWlanDevState_t}
	MSG_LCS_APP_WLAN_SCAN_RESULT_NTF						= MSG_GRP_LCS+0xd8, ///<Payload type {::GLLBS_WLAN_NOTIFY_DATA}
	MSG_LCS_APP_LBS_TIMER_IND								= MSG_GRP_LCS+0xd9, ///<Payload type {::TimerID_t}
	MSG_LCS_APP_WLAN_SCANDATA_READY_IND						= MSG_GRP_LCS+0xda, 

	// End of MSG_GRP_LCS, (0x0D00)



/*TASKMSGS_INCLUDE taskmsgs_dataservices.i*/
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

	MSG_PCHEX_TEST_FIRST									= MSG_GRP_TEST_PCH+0xa1,
	MSG_PCHEX_TEST_STARTUP									= MSG_GRP_TEST_PCH+0xa2,
	MSG_PCHEX_TEST_ACTIVATEPDP								= MSG_GRP_TEST_PCH+0xa3,	///<Payload type {::IcmpPingInfo_t}
	MSG_PCHEX_TEST_ICMPPING									= MSG_GRP_TEST_PCH+0xa4,	///<Payload type {::UdpQuoteInfo_t}
	MSG_PCHEX_TEST_UDPQUOTE									= MSG_GRP_TEST_PCH+0xa5,	///<Payload type {::UdpQuoteInfo_t}
	MSG_PCHEX_TEST_DEACTIVATEPDP							= MSG_GRP_TEST_PCH+0xa6,	///<Payload type {::PdpDeactInfo_t}
	MSG_PCHEX_TEST_SHUTDOWN									= MSG_GRP_TEST_PCH+0xa7,
	MSG_PCHEX_TEST_PINGTIMER								= MSG_GRP_TEST_PCH+0xa8,
	MSG_PCHEX_TEST_LAST										= MSG_GRP_TEST_PCH+0xa9,

	// for DC broadcasting
	MSG_DC_REPORT_CALL_STATUS								= MSG_GRP_DC+0x10,			///<Payload type {::DC_ReportCallStatus_t}
	MSG_DC_IR_SHUTDOWN_CONNECTION_REQ						= MSG_GRP_DC+0x11,			///<Payload type {::DC_IR_ShutdownDataConnection_t}
	MSG_DC_IR_STARTUP_CONNECTION_REQ						= MSG_GRP_DC+0x12,			///<Payload type {::DC_IR_StartupDataConnection_t}

	MSG_PPP_OPEN_IND										= MSG_GRP_DC+0x02,
	MSG_PPP_CLOSE_IND										= MSG_GRP_DC+0x03,
/*TASKMSGS_INCLUDE taskmsgs_dataservices_apps.i*/
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
	MSG_TLS_OPEN_CONNECTION_RSP								= MSG_GRP_TLS+0x01, ///<Payload type {::TlsClientRsp_t}

	/**
	Payload: TlsClientRsp_t, e.g. pTlsClientRsp.
	From TLS inner task to the registered TLS application.
	Any connection state change or error occurs after TLS client finishes
	the handshake protocol with TLS server, this notification message is used to notify
	the TLS client of such change. The status is available in pTlsClientRsp->rspCode
	(see the return code defined in tlsclient_api.h).
	**/
	MSG_TLS_CONNECTION_STATE_NTF							= MSG_GRP_TLS+0x02, ///<Payload type {::TlsClientRsp_t}

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
	MSG_TLS_RECVD_APP_DATA_NTF								= MSG_GRP_TLS+0x03, ///<Payload type {::TlsClientRsp_t}

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
	MSG_TLS_INIT_HANDSHAKE									= MSG_GRP_TLS+0x60, ///<Payload type {::TlsClientHdl_t}

	/**
	Payload: TlsClientHdl_t.
	Sent from TlsSocketCallBack() to signal the arrival of SOCK_SIG_READABLE event.
	**/
	MSG_TLS_RECV_DATA										= MSG_GRP_TLS+0x61, ///<Payload type {::TlsClientHdl_t}

	/**
	Payload: TlsClientHdl_t.
	This message is generated when application invokes
	TLS_ClientCloseConnection()(defined in tlsclient_api.h)
	to request closing a specific connection.
	**/
	MSG_TLS_CLOSE_CONNECTION								= MSG_GRP_TLS+0x62, ///<Payload type {::TlsClientHdl_t}

	/**
	Payload: TlsSecurityContextHdl_t.
	This message is sent to the TLS Inner queue when application invokes
	TLS_UnloadSecurityContext() to request closing a specific security context.
	**/
	MSG_TLS_UNLOAD_SECURITY_CONTEXT							= MSG_GRP_TLS+0x63, ///<Payload type {::TlsSecurityContextHdl_t}

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
	Payload: none.
	**/
	MSG_FTPC_GET_IND										= MSG_GRP_FTP+0x08,

	/**
	This message is generated when the FTP client
	fails to retrieves a file from the server.
	Payload: FTPReplyCode_t, the reply from the server.
	**/
	MSG_FTPC_GET_FAIL										= MSG_GRP_FTP+0x09,	///<Payload type {::FTPCStatus_t}

	/**
	This message is generated when the FTP client
	stores a file to the server successfully.
	Payload: none.
	**/
	MSG_FTPC_PUT_IND										= MSG_GRP_FTP+0x0a,

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

	/** @cond */
/*TASKMSGS_INCLUDE taskmsgs_xscript.i*/
	// This message is used by ATC and the Script engine for a response to the AT*SCRIPT command
	MSG_SCRIPT_AT_CMD									    = MSG_GRP_SCRIPT+0x01,
	MSG_SCRIPT_AT_CMD_RSP									= MSG_GRP_SCRIPT+0x02,
	MSG_SCRIPT_STATUS	                                    = MSG_GRP_SCRIPT+0x03,
/*TASKMSGS_INCLUDE taskmsgs_mmi.i*/
	//------------------------------------------------------------------------------------
	// MSG_GRP_SIMLATOR_0, MSG_GRP_SIMLATOR_1, MSG_GRP_SIMLATOR_2, MSG_GRP_SIMLATOR_3,
	// Platform Simulation Messages (Win32 host build) (0x6000)
	//------------------------------------------------------------------------------------

#ifndef UNDER_CE
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
	MSG_SIMULATOR_SIM_IS_TEST_SIM                           =   MSG_GRP_SIMULATOR_3+0x52,
	MSG_SIMULATOR_SIM_UPDATE_SMSCAPEXC_FLAG                 =	MSG_GRP_SIMULATOR_3+0x54,
#endif //UNDER_CE


/** @endcond */
/*TASKMSGS_INCLUDE taskmsgs_capi2.i*/
	//---------------------------------------------------
	//	CAPI2 message id's
	// (Reserved 0x3XXX for CAPI2 messages)
	// (Reserved 0x4XXX for CAPI2 Generated messages)
	//---------------------------------------------------
	MSG_CAPI2_MSGIDS_START = 0x3000,


	//---------------------------------------------------
	// MSG_GRP_CAPI2_SS,  (0x3B00)
	// CAPI2 SS Request msgs
	//---------------------------------------------------

	MSG_CAPI2_SS_REQ_START								= MSG_GRP_CAPI2_SS+0x00,
	MSG_CAPI2_SS_REQ_END								= MSG_GRP_CAPI2_SS+0xFF,


	//---------------------------------------------------
	// MSG_GRP_CAPI2_UTIL,  (0x3E00)
	//---------------------------------------------------

	MSG_ADC_START_RSP									= MSG_GRP_CAPI2_UTIL+0x01,	///<Payload type {::UInt16}
	MSG_CAPI2_AT_RESPONSE_IND							= MSG_GRP_CAPI2_UTIL+0x02,	///<Payload type {::AtResponse_t}

	//---------------------------------------------------
	// MSG_GRP_CAPI2_PRIVATE,  (0x3F00)
	//---------------------------------------------------
	MSG_RPC_SIMPLE_REQ_RSP								= MSG_GRP_CAPI2_PRIVATE+0xF8,
	MSG_CAPI2_FROM_IPC									= MSG_GRP_CAPI2_PRIVATE+0xF9,
	MSG_CEMU_CBK_IND									= MSG_GRP_CAPI2_PRIVATE+0xFA,
	MSG_CAPI2_RAW_MSG_IND								= MSG_GRP_CAPI2_PRIVATE+0xFB,	///<Payload type {::UInt8*}
	MSG_CAPI2_UNDETERMINED_RSP							= MSG_GRP_CAPI2_PRIVATE+0xFC,
	MSG_CAPI2_RECV_MSG_FROM_AP							= MSG_GRP_CAPI2_PRIVATE+0xFD,
	MSG_CAPI2_ACK_RSP									= MSG_GRP_CAPI2_PRIVATE+0xFE,

	//---------------------------------------------------
	//	MSG_GRP_CAPI2_GEN_0,MSG_GRP_CAPI2_GEN_1,MSG_GRP_CAPI2_GEN_2
	// (Reserved 0x4000 to 0x42FF)
	//---------------------------------------------------

	MSG_GEN_REQ_START = 0x4000,

/*********  _CAPI2_CODE_GEN_BEGIN_ (Do not add code until after _CAPI2_CODE_GEN_END_ ) ************/
//MSG_GEN_REQ_START = 0x0
	 /** 
	api is CAPI2_MS_IsGSMRegistered 
	**/
	MSG_MS_GSM_REGISTERED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GSM_REGISTERED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsGPRSRegistered 
	**/
	MSG_MS_GPRS_REGISTERED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_REGISTERED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetGSMRegCause 
	**/
	MSG_MS_GSM_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GSM_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x7,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_MS_GetGPRSRegCause 
	**/
	MSG_MS_GPRS_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8,
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_MS_GPRS_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x9,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_MS_GetRegisteredLAC 
	**/
	MSG_MS_REGISTERED_LAC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_REGISTERED_LAC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xB,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPlmnMCC 
	**/
	MSG_MS_GET_PLMN_MCC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_GET_PLMN_MCC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPlmnMNC 
	**/
	MSG_MS_GET_PLMN_MNC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_PLMN_MNC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xF,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessPowerDownReq 
	**/
	MSG_SYS_POWERDOWN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10,
	 /** 
	payload is void 
	**/
	MSG_SYS_POWERDOWN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11,
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessNoRfReq 
	**/
	MSG_SYS_POWERUP_NORF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12,
	 /** 
	payload is void 
	**/
	MSG_SYS_POWERUP_NORF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13,
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessNoRfToCalib 
	**/
	MSG_SYS_NORF_CALIB_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14,
	 /** 
	payload is void 
	**/
	MSG_SYS_NORF_CALIB_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15,
	 /** 
	api is CAPI2_PhoneCtrlApi_ProcessPowerUpReq 
	**/
	MSG_SYS_POWERUP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16,
	 /** 
	payload is void 
	**/
	MSG_SYS_POWERUP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x17,
	 /** 
	api is CAPI2_MS_IsGprsAllowed 
	**/
	MSG_MS_GPRS_ALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x18,
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_GPRS_ALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetCurrentRAT 
	**/
	MSG_MS_GET_CURRENT_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetCurrentBand 
	**/
	MSG_MS_GET_CURRENT_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C,
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GET_CURRENT_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_UpdateSMSCapExceededFlag 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E,	///<Payload type {CAPI2_SimApi_UpdateSMSCapExceededFlag_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SIM_UPDATE_SMSCAPEXC_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F,
	 /** 
	api is CAPI2_NetRegApi_SelectBand 
	**/
	MSG_MS_SELECT_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20,	///<Payload type {CAPI2_NetRegApi_SelectBand_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SELECT_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x21,
	 /** 
	api is CAPI2_NetRegApi_SetSupportedRATandBand 
	**/
	MSG_MS_SET_RAT_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22,	///<Payload type {CAPI2_NetRegApi_SetSupportedRATandBand_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SET_RAT_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23,
	 /** 
	api is CAPI2_NetRegApi_CellLock 
	**/
	MSG_MS_CELL_LOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24,	///<Payload type {CAPI2_NetRegApi_CellLock_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_CELL_LOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25,
	 /** 
	api is CAPI2_PLMN_GetCountryByMcc 
	**/
	MSG_MS_GET_MCC_COUNTRY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26,	///<Payload type {CAPI2_PLMN_GetCountryByMcc_Req_t}
	 /** 
	payload is ::char 
	**/
	MSG_MS_GET_MCC_COUNTRY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27,	///<Payload type {::char}
	 /** 
	api is CAPI2_MS_GetPLMNEntryByIndex 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28,	///<Payload type {CAPI2_MS_GetPLMNEntryByIndex_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_PLMN_INFO_BY_CODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_GetPLMNListSize 
	**/
	MSG_MS_PLMN_LIST_SIZE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A,
	 /** 
	payload is ::UInt16 
	**/
	MSG_MS_PLMN_LIST_SIZE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_MS_GetPLMNByCode 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C,	///<Payload type {CAPI2_MS_GetPLMNByCode_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_PLMN_INFO_BY_INDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_NetRegApi_PlmnSelect 
	**/
	MSG_PLMN_SELECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E,	///<Payload type {CAPI2_NetRegApi_PlmnSelect_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PLMN_SELECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_NetRegApi_AbortPlmnSelect 
	**/
	MSG_MS_PLMN_ABORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_PLMN_ABORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x31,
	 /** 
	api is CAPI2_MS_GetPlmnMode 
	**/
	MSG_MS_GET_PLMN_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32,
	 /** 
	payload is ::PlmnSelectMode_t 
	**/
	MSG_MS_GET_PLMN_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x33,	///<Payload type {::PlmnSelectMode_t}
	 /** 
	api is CAPI2_NetRegApi_SetPlmnMode 
	**/
	MSG_MS_SET_PLMN_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34,	///<Payload type {CAPI2_NetRegApi_SetPlmnMode_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SET_PLMN_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35,
	 /** 
	api is CAPI2_MS_GetPlmnFormat 
	**/
	MSG_MS_GET_PLMN_FORMAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x36,
	 /** 
	payload is ::PlmnSelectFormat_t 
	**/
	MSG_MS_GET_PLMN_FORMAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x37,	///<Payload type {::PlmnSelectFormat_t}
	 /** 
	api is CAPI2_MS_SetPlmnFormat 
	**/
	MSG_MS_SET_PLMN_FORMAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x38,	///<Payload type {CAPI2_MS_SetPlmnFormat_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SET_PLMN_FORMAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x39,
	 /** 
	api is CAPI2_MS_IsMatchedPLMN 
	**/
	MSG_MS_MATCH_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3A,	///<Payload type {CAPI2_MS_IsMatchedPLMN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_MATCH_PLMN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_NetRegApi_SearchAvailablePLMN 
	**/
	MSG_MS_SEARCH_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3C,
	 /** 
	api is CAPI2_NetRegApi_AbortSearchPLMN 
	**/
	MSG_MS_ABORT_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3E,
	 /** 
	payload is void 
	**/
	MSG_MS_ABORT_PLMN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3F,
	 /** 
	api is CAPI2_NetRegApi_AutoSearchReq 
	**/
	MSG_MS_AUTO_SEARCH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x40,
	 /** 
	payload is void 
	**/
	MSG_MS_AUTO_SEARCH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x41,
	 /** 
	api is CAPI2_NetRegApi_GetPLMNNameByCode
	**/
	MSG_MS_PLMN_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x42,	///<Payload type {CAPI2_NetRegApi_GetPLMNNameByCode_Req_t}
	 /**
	payload is ::Boolean
	**/
	/* <Payload type {::Boolean} */
	MSG_MS_PLMN_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x43,
	 /**
	api is CAPI2_NetRegApi_GetPLMNNameByCodeEx
	**/
	/* <Payload type {CAPI2_NetRegApi_GetPLMNNameByCodeEx_Req_t} */
	MSG_MS_PLMN_NAME_ADD_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x70A,
	 /** 
	payload is ::Boolean 
	**/
	/* <Payload type {::Boolean} */
	MSG_MS_PLMN_NAME_ADD_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x70B,
	 /** 
	api is CAPI2_PhoneCtrlApi_GetSystemState 
	**/
	MSG_SYS_GET_SYSTEM_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x44,
	 /** 
	payload is ::SystemState_t 
	**/
	MSG_SYS_GET_SYSTEM_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x45,	///<Payload type {::SystemState_t}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetSystemState 
	**/
	MSG_SYS_SET_SYSTEM_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x46,	///<Payload type {CAPI2_PhoneCtrlApi_SetSystemState_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SYS_SET_SYSTEM_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x47,
	 /** 
	api is CAPI2_PhoneCtrlApi_GetRxSignalInfo 
	**/
	MSG_SYS_GET_RX_LEVEL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x48,
	 /** 
	payload is void 
	**/
	MSG_SYS_GET_RX_LEVEL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x49,	///<Payload type {void}
	 /** 
	api is CAPI2_SYS_GetGSMRegistrationStatus 
	**/
	MSG_SYS_GET_GSMREG_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4A,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GSMREG_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4B,	///<Payload type {::RegisterStatus_t}
	 /** 
	api is CAPI2_SYS_GetGPRSRegistrationStatus 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4C,
	 /** 
	payload is ::RegisterStatus_t 
	**/
	MSG_SYS_GET_GPRSREG_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4D,	///<Payload type {::RegisterStatus_t}
	 /** 
	api is CAPI2_SYS_IsRegisteredGSMOrGPRS 
	**/
	MSG_SYS_GET_REG_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4E,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_GET_REG_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsRegisteredHomePLMN 
	**/
	MSG_HOME_PLMN_REG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x50,
	 /** 
	payload is ::Boolean 
	**/
	MSG_HOME_PLMN_REG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x51,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetPowerDownTimer 
	**/
	MSG_SET_POWER_DOWN_TIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x52,	///<Payload type {CAPI2_PhoneCtrlApi_SetPowerDownTimer_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SET_POWER_DOWN_TIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x53,
	 /** 
	api is CAPI2_SimApi_GetSmsParamRecNum 
	**/
	MSG_SIM_PARAM_REC_NUM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x54,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_PARAM_REC_NUM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x55,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_GetSmsMemExceededFlag 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x56,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_GET_SMSMEMEXC_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x57,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsTestSIM 
	**/
	MSG_SIM_IS_TEST_SIM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x58,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_TEST_SIM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x59,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsPINRequired 
	**/
	MSG_SIM_PIN_REQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x5A,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_REQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetCardPhase 
	**/
	MSG_SIM_CARD_PHASE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x5C,
	 /** 
	payload is ::SIMPhase_t 
	**/
	MSG_SIM_CARD_PHASE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5D,	///<Payload type {::SIMPhase_t}
	 /** 
	api is CAPI2_SimApi_GetSIMType 
	**/
	MSG_SIM_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x5E,
	 /** 
	payload is ::SIMType_t 
	**/
	MSG_SIM_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x5F,	///<Payload type {::SIMType_t}
	 /** 
	api is CAPI2_SimApi_GetPresentStatus 
	**/
	MSG_SIM_PRESENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x60,
	 /** 
	payload is ::SIMPresent_t 
	**/
	MSG_SIM_PRESENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x61,	///<Payload type {::SIMPresent_t}
	 /** 
	api is CAPI2_SimApi_IsOperationRestricted 
	**/
	MSG_SIM_PIN_OPERATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x62,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_OPERATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x63,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsPINBlocked 
	**/
	MSG_SIM_PIN_BLOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x64,	///<Payload type {CAPI2_SimApi_IsPINBlocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_BLOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x65,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsPUKBlocked 
	**/
	MSG_SIM_PUK_BLOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x66,	///<Payload type {CAPI2_SimApi_IsPUKBlocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PUK_BLOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x67,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_IsInvalidSIM 
	**/
	MSG_SIM_IS_INVALID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x68,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_INVALID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x69,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_DetectSim 
	**/
	MSG_SIM_DETECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6A,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_DETECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetRuimSuppFlag 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6C,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_GET_RUIM_SUPP_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_SendVerifyChvReq 
	**/
	MSG_SIM_VERIFY_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6E,	///<Payload type {CAPI2_SimApi_SendVerifyChvReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendChangeChvReq 
	**/
	MSG_SIM_CHANGE_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x70,	///<Payload type {CAPI2_SimApi_SendChangeChvReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSetChv1OnOffReq 
	**/
	MSG_SIM_ENABLE_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x72,	///<Payload type {CAPI2_SimApi_SendSetChv1OnOffReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendUnblockChvReq 
	**/
	MSG_SIM_UNBLOCK_CHV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x74,	///<Payload type {CAPI2_SimApi_SendUnblockChvReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSetOperStateReq 
	**/
	MSG_SIM_SET_FDN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x76,	///<Payload type {CAPI2_SimApi_SendSetOperStateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_IsPbkAccessAllowed 
	**/
	MSG_SIM_IS_PBK_ALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x78,	///<Payload type {CAPI2_SimApi_IsPbkAccessAllowed_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_PBK_ALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x79,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_SendPbkInfoReq 
	**/
	MSG_SIM_PBK_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x7A,	///<Payload type {CAPI2_SimApi_SendPbkInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadAcmMaxReq 
	**/
	MSG_SIM_MAX_ACM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x7C,
	 /** 
	api is CAPI2_SimApi_SendWriteAcmMaxReq 
	**/
	MSG_SIM_ACM_MAX_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x7E,	///<Payload type {CAPI2_SimApi_SendWriteAcmMaxReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadAcmReq 
	**/
	MSG_SIM_ACM_VALUE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x80,
	 /** 
	api is CAPI2_SimApi_SendWriteAcmReq 
	**/
	MSG_SIM_ACM_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x82,	///<Payload type {CAPI2_SimApi_SendWriteAcmReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendIncreaseAcmReq 
	**/
	MSG_SIM_ACM_INCREASE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x84,	///<Payload type {CAPI2_SimApi_SendIncreaseAcmReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadSvcProvNameReq 
	**/
	MSG_SIM_SVC_PROV_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x86,
	 /** 
	api is CAPI2_SimApi_SendReadPuctReq 
	**/
	MSG_SIM_PUCT_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x88,
	 /** 
	api is CAPI2_SimApi_GetServiceStatus 
	**/
	MSG_SIM_SERVICE_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8A,	///<Payload type {CAPI2_SimApi_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIMServiceStatus_t 
	**/
	MSG_SIM_SERVICE_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x8B,	///<Payload type {::SIMServiceStatus_t}
	 /** 
	api is CAPI2_SimApi_GetPinStatus 
	**/
	MSG_SIM_PIN_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8C,
	 /** 
	payload is ::SIM_PIN_Status_t 
	**/
	MSG_SIM_PIN_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x8D,	///<Payload type {::SIM_PIN_Status_t}
	 /** 
	api is CAPI2_SimApi_IsPinOK 
	**/
	MSG_SIM_PIN_OK_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x8E,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_PIN_OK_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x8F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetIMSI 
	**/
	MSG_SIM_IMSI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x90,
	 /** 
	payload is ::IMSI_t 
	**/
	MSG_SIM_IMSI_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x91,	///<Payload type {::IMSI_t}
	 /** 
	api is CAPI2_SimApi_GetGID1 
	**/
	MSG_SIM_GID_DIGIT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x92,
	 /** 
	payload is ::GID_DIGIT_t 
	**/
	MSG_SIM_GID_DIGIT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x93,	///<Payload type {::GID_DIGIT_t}
	 /** 
	api is CAPI2_SimApi_GetGID2 
	**/
	MSG_SIM_GID_DIGIT2_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x94,
	 /** 
	payload is ::GID_DIGIT_t 
	**/
	MSG_SIM_GID_DIGIT2_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x95,	///<Payload type {::GID_DIGIT_t}
	 /** 
	api is CAPI2_SimApi_GetHomePlmn 
	**/
	MSG_SIM_HOME_PLMN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x96,
	 /** 
	payload is void 
	**/
	MSG_SIM_HOME_PLMN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x97,	///<Payload type {void}
	 /** 
	api is CAPI2_simmiApi_GetMasterFileId 
	**/
	MSG_SIM_APDU_FILEID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x98,	///<Payload type {CAPI2_simmiApi_GetMasterFileId_Req_t}
	 /** 
	payload is ::APDUFileID_t 
	**/
	MSG_SIM_APDU_FILEID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x99,	///<Payload type {::APDUFileID_t}
	 /** 
	api is CAPI2_SimApi_SendOpenSocketReq 
	**/
	MSG_SIM_OPEN_SOCKET_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9A,
	 /** 
	api is CAPI2_SimApi_SendSelectAppiReq 
	**/
	MSG_SIM_SELECT_APPLI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9C,	///<Payload type {CAPI2_SimApi_SendSelectAppiReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendDeactivateAppiReq 
	**/
	MSG_SIM_DEACTIVATE_APPLI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x9E,	///<Payload type {CAPI2_SimApi_SendDeactivateAppiReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendCloseSocketReq 
	**/
	MSG_SIM_CLOSE_SOCKET_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA0,	///<Payload type {CAPI2_SimApi_SendCloseSocketReq_Req_t}
	 /** 
	api is CAPI2_SimApi_GetAtrData 
	**/
	MSG_SIM_ATR_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA2,
	 /** 
	payload is ::SIMAccess_t 
	**/
	MSG_SIM_ATR_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xA3,	///<Payload type {::SIMAccess_t}
	 /** 
	api is CAPI2_SimApi_SubmitDFileInfoReq 
	**/
	MSG_SIM_DFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA4,	///<Payload type {CAPI2_SimApi_SubmitDFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitEFileInfoReq 
	**/
	MSG_SIM_EFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA6,	///<Payload type {CAPI2_SimApi_SubmitEFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendEFileInfoReq 
	**/
	MSG_SIM_SEND_EFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xA8,	///<Payload type {CAPI2_SimApi_SendEFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendDFileInfoReq 
	**/
	MSG_SIM_SEND_DFILE_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xAA,	///<Payload type {CAPI2_SimApi_SendDFileInfoReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitWholeBinaryEFileReadReq 
	**/
	MSG_SIM_WHOLE_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xAC,	///<Payload type {CAPI2_SimApi_SubmitWholeBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendWholeBinaryEFileReadReq 
	**/
	MSG_SIM_SEND_WHOLE_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xAE,	///<Payload type {CAPI2_SimApi_SendWholeBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitBinaryEFileReadReq 
	**/
	MSG_SIM_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB0,	///<Payload type {CAPI2_SimApi_SubmitBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendBinaryEFileReadReq 
	**/
	MSG_SIM_SEND_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB2,	///<Payload type {CAPI2_SimApi_SendBinaryEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitRecordEFileReadReq 
	**/
	MSG_SIM_RECORD_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB4,	///<Payload type {CAPI2_SimApi_SubmitRecordEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendRecordEFileReadReq 
	**/
	MSG_SIM_SEND_RECORD_EFILE_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB6,	///<Payload type {CAPI2_SimApi_SendRecordEFileReadReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitBinaryEFileUpdateReq 
	**/
	MSG_SIM_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xB8,	///<Payload type {CAPI2_SimApi_SubmitBinaryEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendBinaryEFileUpdateReq 
	**/
	MSG_SIM_SEND_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xBA,	///<Payload type {CAPI2_SimApi_SendBinaryEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitLinearEFileUpdateReq 
	**/
	MSG_SIM_LINEAR_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xBC,	///<Payload type {CAPI2_SimApi_SubmitLinearEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendLinearEFileUpdateReq 
	**/
	MSG_SIM_SEND_LINEAR_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xBE,	///<Payload type {CAPI2_SimApi_SendLinearEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitSeekRecordReq 
	**/
	MSG_SIM_SEEK_RECORD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC0,	///<Payload type {CAPI2_SimApi_SubmitSeekRecordReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSeekRecordReq 
	**/
	MSG_SIM_SEND_SEEK_RECORD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC2,	///<Payload type {CAPI2_SimApi_SendSeekRecordReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitCyclicEFileUpdateReq 
	**/
	MSG_SIM_CYCLIC_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC4,	///<Payload type {CAPI2_SimApi_SubmitCyclicEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendCyclicEFileUpdateReq 
	**/
	MSG_SIM_SEND_CYCLIC_EFILE_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC6,	///<Payload type {CAPI2_SimApi_SendCyclicEFileUpdateReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendRemainingPinAttemptReq 
	**/
	MSG_SIM_PIN_ATTEMPT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xC8,
	 /** 
	api is CAPI2_SimApi_IsCachedDataReady 
	**/
	MSG_SIM_CACHE_DATA_READY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCA,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_CACHE_DATA_READY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xCB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetServiceCodeStatus 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCC,	///<Payload type {CAPI2_SimApi_GetServiceCodeStatus_Req_t}
	 /** 
	payload is ::SERVICE_FLAG_STATUS_t 
	**/
	MSG_SIM_SERVICE_CODE_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xCD,	///<Payload type {::SERVICE_FLAG_STATUS_t}
	 /** 
	api is CAPI2_SimApi_CheckCphsService 
	**/
	MSG_SIM_CHECK_CPHS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xCE,	///<Payload type {CAPI2_SimApi_CheckCphsService_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_CHECK_CPHS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xCF,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetCphsPhase 
	**/
	MSG_SIM_CPHS_PHASE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD0,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_CPHS_PHASE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD1,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_GetSmsSca 
	**/
	MSG_SIM_SMS_SCA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD2,	///<Payload type {CAPI2_SimApi_GetSmsSca_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_SMS_SCA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD3,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_GetIccid 
	**/
	MSG_SIM_ICCID_PARAM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD4,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_ICCID_PARAM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD5,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_IsALSEnabled 
	**/
	MSG_SIM_ALS_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD6,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_ALS_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_GetAlsDefaultLine 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xD8,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_ALS_DEFAULT_LINE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xD9,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_SetAlsDefaultLine 
	**/
	MSG_SIM_SET_ALS_DEFAULT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xDA,	///<Payload type {CAPI2_SimApi_SetAlsDefaultLine_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SIM_SET_ALS_DEFAULT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xDB,
	 /** 
	api is CAPI2_SimApi_GetCallForwardUnconditionalFlag 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xDC,
	 /** 
	payload is ::SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t 
	**/
	MSG_SIM_CALLFWD_COND_FLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xDD,	///<Payload type {::SIM_CALL_FORWARD_UNCONDITIONAL_FLAG_t}
	 /** 
	api is CAPI2_SimApi_GetApplicationType 
	**/
	MSG_SIM_APP_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xDE,
	 /** 
	payload is ::SIM_APPL_TYPE_t 
	**/
	MSG_SIM_APP_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xDF,	///<Payload type {::SIM_APPL_TYPE_t}
	 /** 
	api is CAPI2_SimApi_SendWritePuctReq 
	**/
	MSG_SIM_PUCT_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE0,	///<Payload type {CAPI2_SimApi_SendWritePuctReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitRestrictedAccessReq 
	**/
	MSG_SIM_RESTRICTED_ACCESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xE2,	///<Payload type {CAPI2_SimApi_SubmitRestrictedAccessReq_Req_t}
	 /** 
	api is CAPI2_MS_GetSystemRAT 
	**/
	MSG_MS_GET_SYSTEM_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xFA,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SYSTEM_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xFB,	///<Payload type {::RATSelect_t}
	 /** 
	api is CAPI2_MS_GetSupportedRAT 
	**/
	MSG_MS_GET_SUPPORTED_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xFC,
	 /** 
	payload is ::RATSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xFD,	///<Payload type {::RATSelect_t}
	 /** 
	api is CAPI2_MS_GetSystemBand 
	**/
	MSG_MS_GET_SYSTEM_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0xFE,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SYSTEM_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0xFF,	///<Payload type {::BandSelect_t}
	 /** 
	api is CAPI2_MS_GetSupportedBand 
	**/
	MSG_MS_GET_SUPPORTED_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x100,
	 /** 
	payload is ::BandSelect_t 
	**/
	MSG_MS_GET_SUPPORTED_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x101,	///<Payload type {::BandSelect_t}
	 /** 
	api is CAPI2_SYSPARM_GetMSClass 
	**/
	MSG_SYSPARAM_GET_MSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x102,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_MSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x103,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetManufacturerName 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x104,
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MNF_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x105,	///<Payload type {uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetModelName 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x106,
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_MODEL_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x107,	///<Payload type {uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetSWVersion 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x108,
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYSPARAM_GET_SW_VERSION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x109,	///<Payload type {uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_GetEGPRSMSClass 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10A,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_EGPRS_CLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x10B,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SimApi_SendNumOfPLMNEntryReq 
	**/
	MSG_SIM_PLMN_NUM_OF_ENTRY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10C,	///<Payload type {CAPI2_SimApi_SendNumOfPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendReadPLMNEntryReq 
	**/
	MSG_SIM_PLMN_ENTRY_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x10E,	///<Payload type {CAPI2_SimApi_SendReadPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendWriteMulPLMNEntryReq 
	**/
	MSG_SIM_MUL_PLMN_ENTRY_UPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x110,	///<Payload type {CAPI2_SimApi_SendWriteMulPLMNEntryReq_Req_t}
	 /** 
	api is CAPI2_SYS_SetRegisteredEventMask 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x112,	///<Payload type {CAPI2_SYS_SetRegisteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_EVENT_MASK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x113,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_SetFilteredEventMask 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x114,	///<Payload type {CAPI2_SYS_SetFilteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SET_REG_FILTER_MASK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x115,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetRssiThreshold 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x116,	///<Payload type {CAPI2_PhoneCtrlApi_SetRssiThreshold_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYS_SET_RSSI_THRESHOLD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x117,
	 /** 
	api is CAPI2_SYSPARM_GetChanMode 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x118,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_CHANNEL_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x119,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetClassmark 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11A,
	 /** 
	payload is ::CAPI2_Class_t 
	**/
	MSG_SYSPARAM_GET_CLASSMARK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11B,	///<Payload type {::CAPI2_Class_t}
	 /** 
	api is CAPI2_SYSPARM_GetSysparmIndPartFileVersion 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11C,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARAM_GET_IND_FILE_VER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11D,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_SetDARPCfg 
	**/
	MSG_SYS_SET_DARP_CFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x11E,	///<Payload type {CAPI2_SYSPARM_SetDARPCfg_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYS_SET_DARP_CFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x11F,
	 /** 
	api is CAPI2_SYSPARM_SetEGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x120,	///<Payload type {CAPI2_SYSPARM_SetEGPRSMSClass_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARAM_SET_EGPRS_MSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x121,
	 /** 
	api is CAPI2_SYSPARM_SetGPRSMSClass 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x122,	///<Payload type {CAPI2_SYSPARM_SetGPRSMSClass_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARAM_SET_GPRS_MSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x123,
	 /** 
	api is CAPI2_NetRegApi_DeleteNetworkName 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x124,
	 /** 
	payload is void 
	**/
	MSG_TIMEZONE_DELETE_NW_NAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x125,
	 /** 
	api is CAPI2_TestCmds 
	**/
	MSG_CAPI2_TEST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x126,	///<Payload type {CAPI2_TestCmds_Req_t}
	 /** 
	payload is void 
	**/
	MSG_CAPI2_TEST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x127,
	 /** 
	api is CAPI2_SatkApi_SendPlayToneRes 
	**/
	MSG_STK_SEND_PLAYTONE_RES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x128,	///<Payload type {CAPI2_SatkApi_SendPlayToneRes_Req_t}
	 /** 
	payload is void 
	**/
	MSG_STK_SEND_PLAYTONE_RES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x129,
	 /** 
	api is CAPI2_SATK_SendSetupCallRes 
	**/
	MSG_STK_SETUP_CALL_RES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12A,	///<Payload type {CAPI2_SATK_SendSetupCallRes_Req_t}
	 /** 
	payload is void 
	**/
	MSG_STK_SETUP_CALL_RES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x12B,
	 /** 
	api is CAPI2_PbkApi_SetFdnCheck 
	**/
	MSG_PBK_SET_FDN_CHECK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12C,	///<Payload type {CAPI2_PbkApi_SetFdnCheck_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PBK_SET_FDN_CHECK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x12D,
	 /** 
	api is CAPI2_PbkApi_GetFdnCheck 
	**/
	MSG_PBK_GET_FDN_CHECK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x12E,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_GET_FDN_CHECK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x12F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_SendMemAvailInd 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x134,
	 /** 
	payload is void 
	**/
	MSG_SMS_SEND_MEM_AVAL_IND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x135,
	 /** 
	api is CAPI2_SMS_ConfigureMEStorage 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x136,	///<Payload type {CAPI2_SMS_ConfigureMEStorage_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_CONFIGUREMESTORAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x137,
	 /** 
	api is CAPI2_MsDbApi_SetElement 
	**/
	MSG_MS_SET_ELEMENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x138,	///<Payload type {CAPI2_MsDbApi_SetElement_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SET_ELEMENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x139,
	 /** 
	api is CAPI2_MsDbApi_GetElement 
	**/
	MSG_MS_GET_ELEMENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x13A,	///<Payload type {CAPI2_MsDbApi_GetElement_Req_t}
	 /** 
	payload is ::CAPI2_MS_Element_t 
	**/
	MSG_MS_GET_ELEMENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13B,	///<Payload type {::CAPI2_MS_Element_t}
	 /** 
	api is CAPI2_USimApi_IsApplicationSupported 
	**/
	MSG_USIM_IS_APP_SUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x13C,	///<Payload type {CAPI2_USimApi_IsApplicationSupported_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APP_SUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USimApi_IsAllowedAPN 
	**/
	MSG_USIM_IS_APN_ALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x13E,	///<Payload type {CAPI2_USimApi_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_APN_ALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x13F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USimApi_GetNumOfAPN 
	**/
	MSG_USIM_GET_NUM_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x140,
	 /** 
	payload is ::UInt8 
	**/
	MSG_USIM_GET_NUM_APN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x141,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_USimApi_GetAPNEntry 
	**/
	MSG_USIM_GET_APN_ENTRY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x142,	///<Payload type {CAPI2_USimApi_GetAPNEntry_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_USIM_GET_APN_ENTRY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x143,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_USimApi_IsEstServActivated 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x144,	///<Payload type {CAPI2_USimApi_IsEstServActivated_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_USIM_IS_EST_SERV_ACTIVATED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x145,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USimApi_SendSetEstServReq 
	**/
	MSG_SIM_SET_EST_SERV_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x146,	///<Payload type {CAPI2_USimApi_SendSetEstServReq_Req_t}
	 /** 
	api is CAPI2_USimApi_SendWriteAPNReq 
	**/
	MSG_SIM_UPDATE_ONE_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x148,	///<Payload type {CAPI2_USimApi_SendWriteAPNReq_Req_t}
	 /** 
	api is CAPI2_USimApi_SendDeleteAllAPNReq 
	**/
	MSG_SIM_DELETE_ALL_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14A,
	 /** 
	api is CAPI2_USimApi_GetRatModeSetting 
	**/
	MSG_USIM_GET_RAT_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14C,
	 /** 
	payload is ::USIM_RAT_MODE_t 
	**/
	MSG_USIM_GET_RAT_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x14D,	///<Payload type {::USIM_RAT_MODE_t}
	 /** 
	api is CAPI2_MS_GetGPRSRegState 
	**/
	MSG_MS_GET_GPRS_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x14E,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GPRS_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x14F,	///<Payload type {::MSRegState_t}
	 /** 
	api is CAPI2_MS_GetGSMRegState 
	**/
	MSG_MS_GET_GSM_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x150,
	 /** 
	payload is ::MSRegState_t 
	**/
	MSG_MS_GET_GSM_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x151,	///<Payload type {::MSRegState_t}
	 /** 
	api is CAPI2_MS_GetRegisteredCellInfo 
	**/
	MSG_MS_GET_CELL_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x152,
	 /** 
	payload is ::CellInfo_t 
	**/
	MSG_MS_GET_CELL_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x153,	///<Payload type {::CellInfo_t}
	 /** 
	api is CAPI2_MS_SetMEPowerClass 
	**/
	MSG_MS_SETMEPOWER_CLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x154,	///<Payload type {CAPI2_MS_SetMEPowerClass_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SETMEPOWER_CLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x155,
	 /** 
	api is CAPI2_USimApi_GetServiceStatus 
	**/
	MSG_USIM_GET_SERVICE_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x156,	///<Payload type {CAPI2_USimApi_GetServiceStatus_Req_t}
	 /** 
	payload is ::SIMServiceStatus_t 
	**/
	MSG_USIM_GET_SERVICE_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x157,	///<Payload type {::SIMServiceStatus_t}
	 /** 
	api is CAPI2_SimApi_IsAllowedAPN 
	**/
	MSG_SIM_IS_ALLOWED_APN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x158,	///<Payload type {CAPI2_SimApi_IsAllowedAPN_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_ALLOWED_APN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x159,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_GetSmsMaxCapacity 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x15A,	///<Payload type {CAPI2_SmsApi_GetSmsMaxCapacity_Req_t}
	 /** 
	payload is ::UInt16 
	**/
	MSG_SMS_GETSMSMAXCAPACITY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15B,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SmsApi_RetrieveMaxCBChnlLength 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x15C,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_RETRIEVEMAXCBCHNLLENGTH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15D,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_IsBdnOperationRestricted 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x15E,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SIM_IS_BDN_RESTRICTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x15F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SimApi_SendPreferredPlmnUpdateInd 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x160,	///<Payload type {CAPI2_SimApi_SendPreferredPlmnUpdateInd_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SIM_SEND_PLMN_UPDATE_IND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x161,
	 /** 
	api is CAPI2_SIMIO_DeactiveCard 
	**/
	MSG_SIMIO_DEACTIVATE_CARD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x162,
	 /** 
	payload is void 
	**/
	MSG_SIMIO_DEACTIVATE_CARD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x163,
	 /** 
	api is CAPI2_SimApi_SendSetBdnReq 
	**/
	MSG_SIM_SET_BDN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x164,	///<Payload type {CAPI2_SimApi_SendSetBdnReq_Req_t}
	 /** 
	api is CAPI2_SimApi_PowerOnOffCard 
	**/
	MSG_SIM_POWER_ON_OFF_CARD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x166,	///<Payload type {CAPI2_SimApi_PowerOnOffCard_Req_t}
	 /** 
	api is CAPI2_SimApi_GetRawAtr 
	**/
	MSG_SIM_GET_RAW_ATR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x168,
	 /** 
	api is CAPI2_SimApi_Set_Protocol 
	**/
	MSG_SIM_SET_PROTOCOL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16A,	///<Payload type {CAPI2_SimApi_Set_Protocol_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_SET_PROTOCOL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x16B,
	 /** 
	api is CAPI2_SimApi_Get_Protocol 
	**/
	MSG_SIM_GET_PROTOCOL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16C,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SIM_GET_PROTOCOL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x16D,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_SendGenericApduCmd 
	**/
	MSG_SIM_SEND_GENERIC_APDU_CMD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x16E,	///<Payload type {CAPI2_SimApi_SendGenericApduCmd_Req_t}
	 /** 
	api is CAPI2_SimApi_TerminateXferApdu 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x170,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_TERMINATE_XFER_APDU_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x171,
	 /** 
	api is CAPI2_SIM_GetSimInterface 
	**/
	MSG_SIM_GET_SIM_INTERFACE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x172,
	 /** 
	payload is ::SIM_SIM_INTERFACE_t 
	**/
	MSG_SIM_GET_SIM_INTERFACE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x173,	///<Payload type {::SIM_SIM_INTERFACE_t}
	 /** 
	api is CAPI2_NetRegApi_SetPlmnSelectRat 
	**/
	MSG_SET_PLMN_SELECT_RAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x174,	///<Payload type {CAPI2_NetRegApi_SetPlmnSelectRat_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SET_PLMN_SELECT_RAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x175,
	 /** 
	api is CAPI2_MS_IsDeRegisterInProgress 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x176,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_DEREGISTER_IN_PROGRESS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x177,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_IsRegisterInProgress 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x178,
	 /** 
	payload is ::Boolean 
	**/
	MSG_IS_REGISTER_IN_PROGRESS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x179,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_SendUsimHdkReadReq 
	**/
	MSG_READ_USIM_PBK_HDK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x17A,
	 /** 
	api is CAPI2_PbkApi_SendUsimHdkUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_HDK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x17C,	///<Payload type {CAPI2_PbkApi_SendUsimHdkUpdateReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimAasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_AAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x17E,	///<Payload type {CAPI2_PbkApi_SendUsimAasReadReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimAasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_AAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x180,	///<Payload type {CAPI2_PbkApi_SendUsimAasUpdateReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimGasReadReq 
	**/
	MSG_READ_USIM_PBK_ALPHA_GAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x182,	///<Payload type {CAPI2_PbkApi_SendUsimGasReadReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimGasUpdateReq 
	**/
	MSG_WRITE_USIM_PBK_ALPHA_GAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x184,	///<Payload type {CAPI2_PbkApi_SendUsimGasUpdateReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimAasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_AAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x186,	///<Payload type {CAPI2_PbkApi_SendUsimAasInfoReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUsimGasInfoReq 
	**/
	MSG_GET_USIM_PBK_ALPHA_INFO_GAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x188,	///<Payload type {CAPI2_PbkApi_SendUsimGasInfoReq_Req_t}
	 /** 
	api is CAPI2_DiagApi_MeasurmentReportReq 
	**/
	MSG_DIAG_MEASURE_REPORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x18C,	///<Payload type {CAPI2_DiagApi_MeasurmentReportReq_Req_t}
	 /** 
	payload is void 
	**/
	MSG_DIAG_MEASURE_REPORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x18D,
	 /** 
	api is CAPI2_MsDbApi_InitCallCfg 
	**/
	MSG_MS_INITCALLCFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x190,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITCALLCFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x191,
	 /** 
	api is CAPI2_MS_InitFaxConfig 
	**/
	MSG_MS_INITFAXCFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x192,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITFAXCFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x193,
	 /** 
	api is CAPI2_MS_InitVideoCallCfg 
	**/
	MSG_MS_INITVIDEOCALLCFG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x194,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITVIDEOCALLCFG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x195,
	 /** 
	api is CAPI2_MS_InitCallCfgAmpF 
	**/
	MSG_MS_INITCALLCFGAMPF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x196,
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_INITCALLCFGAMPF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x197,
	 /** 
	api is CAPI2_SYSPARM_GetActualLowVoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x19C,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_LOW_VOLT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19D,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SYSPARM_GetActual4p2VoltReading 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x19E,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_ACTUAL_4P2_VOLT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x19F,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_SmsApi_SendSMSCommandTxtReq 
	**/
	MSG_SMS_SEND_COMMAND_TXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A0,	///<Payload type {CAPI2_SmsApi_SendSMSCommandTxtReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SEND_COMMAND_TXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A1,
	 /** 
	api is CAPI2_SmsApi_SendSMSCommandPduReq 
	**/
	MSG_SMS_SEND_COMMAND_PDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A2,	///<Payload type {CAPI2_SmsApi_SendSMSCommandPduReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SEND_COMMAND_PDU_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A3,
	 /** 
	api is CAPI2_SmsApi_SendPDUAckToNetwork 
	**/
	MSG_SMS_SEND_ACKTONETWORK_PDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A4,	///<Payload type {CAPI2_SmsApi_SendPDUAckToNetwork_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SMS_SEND_ACKTONETWORK_PDU_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A5,
	 /** 
	api is CAPI2_SmsApi_StartCellBroadcastWithChnlReq 
	**/
	MSG_SMS_START_CB_WITHCHNL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A6,	///<Payload type {CAPI2_SmsApi_StartCellBroadcastWithChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_START_CB_WITHCHNL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A7,
	 /** 
	api is CAPI2_SmsApi_SetMoSmsTpMr 
	**/
	MSG_SMS_SET_TPMR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1A8,	///<Payload type {CAPI2_SmsApi_SetMoSmsTpMr_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SET_TPMR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1A9,
	 /** 
	api is CAPI2_SIMLOCKApi_SetStatus 
	**/
	MSG_SIMLOCK_SET_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1AA,	///<Payload type {CAPI2_SIMLOCKApi_SetStatus_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SIMLOCK_SET_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1AB,
	 /** 
	api is CAPI2_DIAG_ApiCellLockReq 
	**/
	MSG_DIAG_CELLLOCK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1AE,	///<Payload type {CAPI2_DIAG_ApiCellLockReq_Req_t}
	 /** 
	payload is void 
	**/
	MSG_DIAG_CELLLOCK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1AF,
	 /** 
	api is CAPI2_DIAG_ApiCellLockStatus 
	**/
	MSG_DIAG_CELLLOCK_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B0,
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIAG_CELLLOCK_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B1,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SetRuaReadyTimer 
	**/
	MSG_MS_SET_RUA_READY_TIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B2,	///<Payload type {CAPI2_MS_SetRuaReadyTimer_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SET_RUA_READY_TIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B3,
	 /** 
	api is CAPI2_SmsApi_StartCellBroadcastForCMASReq 
	**/
	MSG_SMS_START_CB_FORCMAS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1B4,	///<Payload type {CAPI2_SmsApi_StartCellBroadcastForCMASReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_START_CB_FORCMAS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1B5,
	 /** 
	api is CAPI2_CcApi_IsThereEmergencyCall 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1C8,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_IS_THERE_EMERGENCY_CALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1C9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MsDbApi_SYS_EnableCellInfoMsg 
	**/
	MSG_SYS_ENABLE_CELL_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1CC,	///<Payload type {CAPI2_MsDbApi_SYS_EnableCellInfoMsg_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYS_ENABLE_CELL_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1CD,
	 /** 
	api is CAPI2_LCS_L1_bb_isLocked 
	**/
	MSG_L1_BB_ISLOCKED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1CE,	///<Payload type {CAPI2_LCS_L1_bb_isLocked_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_L1_BB_ISLOCKED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1CF,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_DIALSTR_ParseGetCallType 
	**/
	MSG_UTIL_DIAL_STR_PARSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D2,	///<Payload type {CAPI2_DIALSTR_ParseGetCallType_Req_t}
	 /** 
	payload is ::CallType_t 
	**/
	MSG_UTIL_DIAL_STR_PARSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D3,	///<Payload type {::CallType_t}
	 /** 
	api is CAPI2_LCS_FttCalcDeltaTime 
	**/
	MSG_LCS_FTT_DELTA_TIME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D4,	///<Payload type {CAPI2_LCS_FttCalcDeltaTime_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_LCS_FTT_DELTA_TIME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D5,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_NetRegApi_ForcedReadyStateReq 
	**/
	MSG_MS_FORCEDREADYSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1D8,	///<Payload type {CAPI2_NetRegApi_ForcedReadyStateReq_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_FORCEDREADYSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1D9,
	 /** 
	api is CAPI2_SsApi_ResetSsAlsFlag 
	**/
	MSG_SS_RESETSSALSFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1DA,
	 /** 
	payload is void 
	**/
	MSG_SS_RESETSSALSFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1DB,
	 /** 
	api is CAPI2_SimLockApi_GetStatus 
	**/
	MSG_SIMLOCK_GET_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1DC,	///<Payload type {CAPI2_SimLockApi_GetStatus_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SIMLOCK_GET_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1DF,
	 /** 
	api is CAPI2_DIALSTR_IsValidString 
	**/
	MSG_DIALSTR_IS_VALID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E0,	///<Payload type {CAPI2_DIALSTR_IsValidString_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIALSTR_IS_VALID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E1,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_UTIL_Cause2NetworkCause 
	**/
	MSG_UTIL_CONVERT_NTWK_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E2,	///<Payload type {CAPI2_UTIL_Cause2NetworkCause_Req_t}
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_UTIL_CONVERT_NTWK_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E3,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_UTIL_ErrCodeToNetCause 
	**/
	MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E4,	///<Payload type {CAPI2_UTIL_ErrCodeToNetCause_Req_t}
	 /** 
	payload is ::NetworkCause_t 
	**/
	MSG_UTIL_CONVERT_ERRCODE_NTWK_CAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E5,	///<Payload type {::NetworkCause_t}
	 /** 
	api is CAPI2_IsGprsDialStr 
	**/
	MSG_ISGPRS_DIAL_STR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E6,	///<Payload type {CAPI2_IsGprsDialStr_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_ISGPRS_DIAL_STR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_UTIL_GetNumOffsetInSsStr 
	**/
	MSG_GET_NUM_SS_STR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1E8,	///<Payload type {CAPI2_UTIL_GetNumOffsetInSsStr_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_GET_NUM_SS_STR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1E9,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_IsPppLoopbackDialStr 
	**/
	MSG_DIALSTR_IS_PPPLOOPBACK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1EE,	///<Payload type {CAPI2_IsPppLoopbackDialStr_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_DIALSTR_IS_PPPLOOPBACK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1EF,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYS_GetRIPPROCVersion 
	**/
	MSG_SYS_GETRIPPROCVERSION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F0,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SYS_GETRIPPROCVERSION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F1,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SYSPARM_SetHSDPAPHYCategory 
	**/
	MSG_SYSPARM_SET_HSDPA_PHY_CAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F2,	///<Payload type {CAPI2_SYSPARM_SetHSDPAPHYCategory_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARM_SET_HSDPA_PHY_CAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F3,
	 /** 
	api is CAPI2_SYSPARM_GetHSDPAPHYCategory 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F4,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SYS_GET_HSDPA_CATEGORY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F5,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SmsApi_ConvertSmsMSMsgType 
	**/
	MSG_SMS_CONVERT_MSGTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1F6,	///<Payload type {CAPI2_SmsApi_ConvertSmsMSMsgType_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_CONVERT_MSGTYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1F7,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetPrefNetStatus 
	**/
	MSG_MS_GETPREFNETSTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x1FA,
	 /** 
	payload is ::GANStatus_t 
	**/
	MSG_MS_GETPREFNETSTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x1FB,	///<Payload type {::GANStatus_t}
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_1  = MSG_GRP_CAPI2_GEN_0 + 0x1FD,	///<Payload type {uchar_ptr_t}
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_2  = MSG_GRP_CAPI2_GEN_0 + 0x1FF,	///<Payload type {uchar_ptr_t}
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_3  = MSG_GRP_CAPI2_GEN_0 + 0x201,	///<Payload type {uchar_ptr_t}
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_4  = MSG_GRP_CAPI2_GEN_0 + 0x203,	///<Payload type {uchar_ptr_t}
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_5  = MSG_GRP_CAPI2_GEN_0 + 0x205,	///<Payload type {uchar_ptr_t}
	 /** 
	payload is uchar_ptr_t 
	**/
	MSG_SYS_TEST_MSG_IND_6  = MSG_GRP_CAPI2_GEN_0 + 0x207,	///<Payload type {uchar_ptr_t}
	 /** 
	api is CAPI2_SYSPARM_SetHSUPAPHYCategory 
	**/
	MSG_SYSPARM_SET_HSUPA_PHY_CAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x208,	///<Payload type {CAPI2_SYSPARM_SetHSUPAPHYCategory_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARM_SET_HSUPA_PHY_CAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x209,
	 /** 
	api is CAPI2_SYSPARM_GetHSUPAPHYCategory 
	**/
	MSG_SYSPARM_GET_HSUPA_PHY_CAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20A,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SYSPARM_GET_HSUPA_PHY_CAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x20B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_InterTaskMsgToCP 
	**/
	MSG_INTERTASK_MSG_TO_CP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20C,	///<Payload type {CAPI2_InterTaskMsgToCP_Req_t}
	 /** 
	payload is void 
	**/
	MSG_INTERTASK_MSG_TO_CP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x20D,
	 /** 
	api is CAPI2_InterTaskMsgToAP 
	**/
	MSG_INTERTASK_MSG_TO_AP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x20E,	///<Payload type {CAPI2_InterTaskMsgToAP_Req_t}
	 /** 
	payload is void 
	**/
	MSG_INTERTASK_MSG_TO_AP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x211,
	 /** 
	api is CAPI2_CcApi_GetCurrentCallIndex 
	**/
	MSG_CC_GETCURRENTCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x212,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETCURRENTCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x213,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNextActiveCallIndex 
	**/
	MSG_CC_GETNEXTACTIVECALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x214,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNEXTACTIVECALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x215,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNextHeldCallIndex 
	**/
	MSG_CC_GETNEXTHELDCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x216,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNEXTHELDCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x217,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNextWaitCallIndex 
	**/
	MSG_CC_GETNEXTWAITCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x218,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNEXTWAITCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x219,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetMPTYCallIndex 
	**/
	MSG_CC_GETMPTYCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21A,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETMPTYCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x21B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetCallState 
	**/
	MSG_CC_GETCALLSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21C,	///<Payload type {CAPI2_CcApi_GetCallState_Req_t}
	 /** 
	payload is ::CCallState_t 
	**/
	MSG_CC_GETCALLSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x21D,	///<Payload type {::CCallState_t}
	 /** 
	api is CAPI2_CcApi_GetCallType 
	**/
	MSG_CC_GETCALLTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x21E,	///<Payload type {CAPI2_CcApi_GetCallType_Req_t}
	 /** 
	payload is ::CCallType_t 
	**/
	MSG_CC_GETCALLTYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x21F,	///<Payload type {::CCallType_t}
	 /** 
	api is CAPI2_CcApi_GetLastCallExitCause 
	**/
	MSG_CC_GETLASTCALLEXITCAUSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x220,
	 /** 
	payload is ::Cause_t 
	**/
	MSG_CC_GETLASTCALLEXITCAUSE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x221,	///<Payload type {::Cause_t}
	 /** 
	api is CAPI2_CcApi_GetCallNumber 
	**/
	MSG_CC_GETCALLNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x222,	///<Payload type {CAPI2_CcApi_GetCallNumber_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GETCALLNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x223,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetCallingInfo 
	**/
	MSG_CC_GETCALLINGINFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x224,	///<Payload type {CAPI2_CcApi_GetCallingInfo_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GETCALLINGINFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x225,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetAllCallStates 
	**/
	MSG_CC_GETALLCALLSTATES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x226,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLCALLSTATES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x227,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllCallIndex 
	**/
	MSG_CC_GETALLCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x228,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x229,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllHeldCallIndex 
	**/
	MSG_CC_GETALLHELDCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22A,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLHELDCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x22B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllActiveCallIndex 
	**/
	MSG_CC_GETALLACTIVECALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22C,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLACTIVECALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x22D,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetAllMPTYCallIndex 
	**/
	MSG_CC_GETALLMPTYCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x22E,
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETALLMPTYCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x22F,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_GetNumOfMPTYCalls 
	**/
	MSG_CC_GETNUMOFMPTYCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x230,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNUMOFMPTYCALLS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x231,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNumofActiveCalls 
	**/
	MSG_CC_GETNUMOFACTIVECALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x232,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNUMOFACTIVECALLS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x233,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetNumofHeldCalls 
	**/
	MSG_CC_GETNUMOFHELDCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x234,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETNUMOFHELDCALLS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x235,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_IsThereWaitingCall 
	**/
	MSG_CC_ISTHEREWAITINGCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x236,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTHEREWAITINGCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x237,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsThereAlertingCall 
	**/
	MSG_CC_ISTHEREALERTINGCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x238,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTHEREALERTINGCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x239,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetConnectedLineID 
	**/
	MSG_CC_GETCONNECTEDLINEID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23A,	///<Payload type {CAPI2_CcApi_GetConnectedLineID_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETCONNECTEDLINEID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetCallPresent 
	**/
	MSG_CC_GET_CALL_PRESENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23C,	///<Payload type {CAPI2_CcApi_GetCallPresent_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GET_CALL_PRESENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetCallIndexInThisState 
	**/
	MSG_CC_GET_INDEX_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x23E,	///<Payload type {CAPI2_CcApi_GetCallIndexInThisState_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_GET_INDEX_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x23F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsMultiPartyCall 
	**/
	MSG_CC_ISMULTIPARTYCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x240,	///<Payload type {CAPI2_CcApi_IsMultiPartyCall_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISMULTIPARTYCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x241,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsThereVoiceCall 
	**/
	MSG_CC_ISTHEREVOICECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x242,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTHEREVOICECALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x243,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsConnectedLineIDPresentAllowed 
	**/
	MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x244,	///<Payload type {CAPI2_CcApi_IsConnectedLineIDPresentAllowed_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISCONNECTEDLINEIDPRESENTALLOWED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x245,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds 
	**/
	MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x246,	///<Payload type {CAPI2_CcApi_GetCurrentCallDurationInMilliSeconds_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETCURRENTCALLDURATIONINMILLISECONDS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x247,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastCallCCM 
	**/
	MSG_CC_GETLASTCALLCCM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x248,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTCALLCCM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x249,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastCallDuration 
	**/
	MSG_CC_GETLASTCALLDURATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24A,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTCALLDURATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x24B,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastDataCallRxBytes 
	**/
	MSG_CC_GETLASTDATACALLRXBYTES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24C,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTDATACALLRXBYTES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x24D,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetLastDataCallTxBytes 
	**/
	MSG_CC_GETLASTDATACALLTXBYTES_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x24E,
	 /** 
	payload is ::UInt32 
	**/
	MSG_CC_GETLASTDATACALLTXBYTES_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x24F,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CcApi_GetDataCallIndex 
	**/
	MSG_CC_GETDATACALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x250,
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETDATACALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x251,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetCallClientInfo 
	**/
	MSG_CC_GETCALLCLIENT_INFO_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x252,	///<Payload type {CAPI2_CcApi_GetCallClientInfo_Req_t}
	 /** 
	payload is void 
	**/
	MSG_CC_GETCALLCLIENT_INFO_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x253,	///<Payload type {void}
	 /** 
	api is CAPI2_CcApi_GetCallClientID 
	**/
	MSG_CC_GETCALLCLIENTID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x254,	///<Payload type {CAPI2_CcApi_GetCallClientID_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETCALLCLIENTID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x255,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_GetTypeAdd 
	**/
	MSG_CC_GETTYPEADD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x256,	///<Payload type {CAPI2_CcApi_GetTypeAdd_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_CC_GETTYPEADD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x257,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_CcApi_SetVoiceCallAutoReject 
	**/
	MSG_CC_SETVOICECALLAUTOREJECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x258,	///<Payload type {CAPI2_CcApi_SetVoiceCallAutoReject_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_SETVOICECALLAUTOREJECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x259,
	 /** 
	api is CAPI2_CcApi_IsVoiceCallAutoReject 
	**/
	MSG_CC_ISVOICECALLAUTOREJECT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x25A,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISVOICECALLAUTOREJECT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_SetTTYCall 
	**/
	MSG_CC_SETTTYCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x25C,	///<Payload type {CAPI2_CcApi_SetTTYCall_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_SETTTYCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25D,
	 /** 
	api is CAPI2_CcApi_IsTTYEnable 
	**/
	MSG_CC_ISTTYENABLE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x25E,
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISTTYENABLE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x25F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_IsSimOriginedCall 
	**/
	MSG_CC_ISSIMORIGINEDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x260,	///<Payload type {CAPI2_CcApi_IsSimOriginedCall_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISSIMORIGINEDCALL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x261,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_SetVideoCallParam 
	**/
	MSG_CC_SETVIDEOCALLPARAM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x262,	///<Payload type {CAPI2_CcApi_SetVideoCallParam_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_SETVIDEOCALLPARAM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x263,
	 /** 
	api is CAPI2_CcApi_GetVideoCallParam 
	**/
	MSG_CC_GETVIDEOCALLPARAM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x264,
	 /** 
	payload is ::VideoCallParam_t 
	**/
	MSG_CC_GETVIDEOCALLPARAM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x265,	///<Payload type {::VideoCallParam_t}
	 /** 
	api is CAPI2_CcApi_GetCCM 
	**/
	MSG_CC_GETCCM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x266,	///<Payload type {CAPI2_CcApi_GetCCM_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CC_GETCCM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x267,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_SendDtmfTone 
	**/
	MSG_CCAPI_SENDDTMF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x268,	///<Payload type {CAPI2_CcApi_SendDtmfTone_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_SENDDTMF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x269,
	 /** 
	api is CAPI2_CcApi_StopDtmfTone 
	**/
	MSG_CCAPI_STOPDTMF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26A,	///<Payload type {CAPI2_CcApi_StopDtmfTone_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_STOPDTMF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x26B,
	 /** 
	api is CAPI2_CcApi_AbortDtmf 
	**/
	MSG_CCAPI_ABORTDTMF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26C,	///<Payload type {CAPI2_CcApi_AbortDtmf_Req_t}
	 /** 
	payload is void 
	**/
	MSG_CCAPI_ABORTDTMF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x26D,
	 /** 
	api is CAPI2_CcApi_SetDtmfToneTimer 
	**/
	MSG_CCAPI_SETDTMFTIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x26E,	///<Payload type {CAPI2_CcApi_SetDtmfToneTimer_Req_t}
	 /** 
	payload is void 
	**/
	MSG_CCAPI_SETDTMFTIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x26F,
	 /** 
	api is CAPI2_CcApi_ResetDtmfToneTimer 
	**/
	MSG_CCAPI_RESETDTMFTIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x270,	///<Payload type {CAPI2_CcApi_ResetDtmfToneTimer_Req_t}
	 /** 
	payload is void 
	**/
	MSG_CCAPI_RESETDTMFTIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x271,
	 /** 
	api is CAPI2_CcApi_GetDtmfToneTimer 
	**/
	MSG_CCAPI_GETDTMFTIMER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x272,	///<Payload type {CAPI2_CcApi_GetDtmfToneTimer_Req_t}
	 /** 
	payload is ::Ticks_t 
	**/
	MSG_CCAPI_GETDTMFTIMER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x273,	///<Payload type {::Ticks_t}
	 /** 
	api is CAPI2_CcApi_GetTiFromCallIndex 
	**/
	MSG_CCAPI_GETTIFROMCALLINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x274,	///<Payload type {CAPI2_CcApi_GetTiFromCallIndex_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_GETTIFROMCALLINDEX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x275,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_CcApi_IsSupportedBC 
	**/
	MSG_CCAPI_IS_SUPPORTEDBC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x276,	///<Payload type {CAPI2_CcApi_IsSupportedBC_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CCAPI_IS_SUPPORTEDBC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x277,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_GetBearerCapability 
	**/
	MSG_CCAPI_IS_BEARER_CAPABILITY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x278,	///<Payload type {CAPI2_CcApi_GetBearerCapability_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_IS_BEARER_CAPABILITY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x279,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq 
	**/
	MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27A,	///<Payload type {CAPI2_SmsApi_SendSMSSrvCenterNumberUpdateReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDSMSSRVCENTERNUMBERUPDATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27B,
	 /** 
	api is CAPI2_SmsApi_GetSMSrvCenterNumber 
	**/
	MSG_SMS_GETSMSSRVCENTERNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27C,	///<Payload type {CAPI2_SmsApi_GetSMSrvCenterNumber_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSSRVCENTERNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27D,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetSIMSMSCapacityExceededFlag 
	**/
	MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x27E,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSIMSMSCAPACITYEXCEEDEDFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x27F,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_IsSmsServiceAvail 
	**/
	MSG_SMS_ISSMSSERVICEAVAIL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x280,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_ISSMSSERVICEAVAIL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x281,
	 /** 
	api is CAPI2_SmsApi_GetSmsStoredState 
	**/
	MSG_SMS_GETSMSSTOREDSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x282,	///<Payload type {CAPI2_SmsApi_GetSmsStoredState_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSSTOREDSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x283,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_WriteSMSPduReq 
	**/
	MSG_SMS_WRITESMSPDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x284,	///<Payload type {CAPI2_SmsApi_WriteSMSPduReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_WriteSMSReq 
	**/
	MSG_SMS_WRITESMSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x286,	///<Payload type {CAPI2_SmsApi_WriteSMSReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SendSMSReq 
	**/
	MSG_SMS_SENDSMSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x288,	///<Payload type {CAPI2_SmsApi_SendSMSReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SendSMSPduReq 
	**/
	MSG_SMS_SENDSMSPDUREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28A,	///<Payload type {CAPI2_SmsApi_SendSMSPduReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SendStoredSmsReq 
	**/
	MSG_SMS_SENDSTOREDSMSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28C,	///<Payload type {CAPI2_SmsApi_SendStoredSmsReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_WriteSMSPduToSIMReq 
	**/
	MSG_SMS_WRITESMSPDUTOSIM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x28E,	///<Payload type {CAPI2_SmsApi_WriteSMSPduToSIMReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_GetLastTpMr 
	**/
	MSG_SMS_GETLASTTPMR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x290,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_GETLASTTPMR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x291,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SmsApi_GetSmsTxParams 
	**/
	MSG_SMS_GETSMSTXPARAMS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x292,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSTXPARAMS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x293,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetTxParamInTextMode 
	**/
	MSG_SMS_GETTXPARAMINTEXTMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x294,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETTXPARAMINTEXTMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x295,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamProcId 
	**/
	MSG_SMS_SETSMSTXPARAMPROCID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x296,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamProcId_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMPROCID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x297,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamCodingType 
	**/
	MSG_SMS_SETSMSTXPARAMCODINGTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x298,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamCodingType_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMCODINGTYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x299,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamValidPeriod 
	**/
	MSG_SMS_SETSMSTXPARAMVALIDPERIOD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x29A,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamValidPeriod_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMVALIDPERIOD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29B,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamCompression 
	**/
	MSG_SMS_SETSMSTXPARAMCOMPRESSION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x29C,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamCompression_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMCOMPRESSION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29D,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamReplyPath 
	**/
	MSG_SMS_SETSMSTXPARAMREPLYPATH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x29E,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamReplyPath_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMREPLYPATH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x29F,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd 
	**/
	MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A0,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamUserDataHdrInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMUSERDATAHDRIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2A1,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag 
	**/
	MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A2,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamStatusRptReqFlag_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMSTATUSRPTREQFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2A3,
	 /** 
	api is CAPI2_SmsApi_SetSmsTxParamRejDupl 
	**/
	MSG_SMS_SETSMSTXPARAMREJDUPL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A4,	///<Payload type {CAPI2_SmsApi_SetSmsTxParamRejDupl_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSTXPARAMREJDUPL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2A5,
	 /** 
	api is CAPI2_SmsApi_DeleteSmsMsgByIndexReq 
	**/
	MSG_SMS_DELETESMSMSGBYINDEX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A6,	///<Payload type {CAPI2_SmsApi_DeleteSmsMsgByIndexReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_ReadSmsMsgReq 
	**/
	MSG_SMS_READSMSMSG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2A8,	///<Payload type {CAPI2_SmsApi_ReadSmsMsgReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_ListSmsMsgReq 
	**/
	MSG_SMS_LISTSMSMSG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2AA,	///<Payload type {CAPI2_SmsApi_ListSmsMsgReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_SetNewMsgDisplayPref 
	**/
	MSG_SMS_SETNEWMSGDISPLAYPREF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2AC,	///<Payload type {CAPI2_SmsApi_SetNewMsgDisplayPref_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETNEWMSGDISPLAYPREF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2AD,
	 /** 
	api is CAPI2_SmsApi_GetNewMsgDisplayPref 
	**/
	MSG_SMS_GETNEWMSGDISPLAYPREF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2AE,	///<Payload type {CAPI2_SmsApi_GetNewMsgDisplayPref_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SMS_GETNEWMSGDISPLAYPREF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2AF,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SmsApi_SetSMSPrefStorage 
	**/
	MSG_SMS_SETSMSPREFSTORAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B0,	///<Payload type {CAPI2_SmsApi_SetSMSPrefStorage_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETSMSPREFSTORAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B1,
	 /** 
	api is CAPI2_SmsApi_GetSMSPrefStorage 
	**/
	MSG_SMS_GETSMSPREFSTORAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B2,
	 /** 
	payload is ::SmsStorage_t 
	**/
	MSG_SMS_GETSMSPREFSTORAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B3,	///<Payload type {::SmsStorage_t}
	 /** 
	api is CAPI2_SmsApi_GetSMSStorageStatus 
	**/
	MSG_SMS_GETSMSSTORAGESTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B4,	///<Payload type {CAPI2_SmsApi_GetSMSStorageStatus_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETSMSSTORAGESTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B5,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_SaveSmsServiceProfile 
	**/
	MSG_SMS_SAVESMSSERVICEPROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B6,	///<Payload type {CAPI2_SmsApi_SaveSmsServiceProfile_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SAVESMSSERVICEPROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B7,
	 /** 
	api is CAPI2_SmsApi_RestoreSmsServiceProfile 
	**/
	MSG_SMS_RESTORESMSSERVICEPROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2B8,	///<Payload type {CAPI2_SmsApi_RestoreSmsServiceProfile_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_RESTORESMSSERVICEPROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2B9,
	 /** 
	api is CAPI2_SmsApi_SetCellBroadcastMsgTypeReq 
	**/
	MSG_SMS_SETCELLBROADCASTMSGTYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2BA,	///<Payload type {CAPI2_SmsApi_SetCellBroadcastMsgTypeReq_Req_t}
	 /** 
	api is CAPI2_SmsApi_CBAllowAllChnlReq 
	**/
	MSG_SMS_CBALLOWALLCHNLREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2BC,	///<Payload type {CAPI2_SmsApi_CBAllowAllChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_CBALLOWALLCHNLREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2BD,
	 /** 
	api is CAPI2_SmsApi_AddCellBroadcastChnlReq 
	**/
	MSG_SMS_ADDCELLBROADCASTCHNLREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2BE,	///<Payload type {CAPI2_SmsApi_AddCellBroadcastChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_ADDCELLBROADCASTCHNLREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2BF,
	 /** 
	api is CAPI2_SmsApi_RemoveCellBroadcastChnlReq 
	**/
	MSG_SMS_REMOVECELLBROADCASTCHNLREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C0,	///<Payload type {CAPI2_SmsApi_RemoveCellBroadcastChnlReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVECELLBROADCASTCHNLREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C1,
	 /** 
	api is CAPI2_SmsApi_RemoveAllCBChnlFromSearchList 
	**/
	MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C2,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVEALLCBCHNLFROMSEARCHLIST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C3,
	 /** 
	api is CAPI2_SmsApi_GetCBMI 
	**/
	MSG_SMS_GETCBMI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C4,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETCBMI_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C5,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetCbLanguage 
	**/
	MSG_SMS_GETCBLANGUAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C6,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETCBLANGUAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C7,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_AddCellBroadcastLangReq 
	**/
	MSG_SMS_ADDCELLBROADCASTLANGREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2C8,	///<Payload type {CAPI2_SmsApi_AddCellBroadcastLangReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_ADDCELLBROADCASTLANGREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2C9,
	 /** 
	api is CAPI2_SmsApi_RemoveCellBroadcastLangReq 
	**/
	MSG_SMS_REMOVECELLBROADCASTLANGREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2CA,	///<Payload type {CAPI2_SmsApi_RemoveCellBroadcastLangReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_REMOVECELLBROADCASTLANGREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2CB,
	 /** 
	api is CAPI2_SmsApi_StartReceivingCellBroadcastReq 
	**/
	MSG_SMS_STARTRECEIVINGCELLBROADCAST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2CC,
	 /** 
	api is CAPI2_SmsApi_StopReceivingCellBroadcastReq 
	**/
	MSG_SMS_STOPRECEIVINGCELLBROADCAST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2CE,
	 /** 
	api is CAPI2_SmsApi_SetCBIgnoreDuplFlag 
	**/
	MSG_SMS_SETCBIGNOREDUPLFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D0,	///<Payload type {CAPI2_SmsApi_SetCBIgnoreDuplFlag_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETCBIGNOREDUPLFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D1,
	 /** 
	api is CAPI2_SmsApi_GetCBIgnoreDuplFlag 
	**/
	MSG_SMS_GETCBIGNOREDUPLFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D2,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_GETCBIGNOREDUPLFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_SetVMIndOnOff 
	**/
	MSG_SMS_SETVMINDONOFF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D4,	///<Payload type {CAPI2_SmsApi_SetVMIndOnOff_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETVMINDONOFF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D5,
	 /** 
	api is CAPI2_SmsApi_IsVMIndEnabled 
	**/
	MSG_SMS_ISVMINDENABLED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D6,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_ISVMINDENABLED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_GetVMWaitingStatus 
	**/
	MSG_SMS_GETVMWAITINGSTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2D8,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETVMWAITINGSTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2D9,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetNumOfVmscNumber 
	**/
	MSG_SMS_GETNUMOFVMSCNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2DA,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETNUMOFVMSCNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2DB,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_GetVmscNumber 
	**/
	MSG_SMS_GETVMSCNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2DC,	///<Payload type {CAPI2_SmsApi_GetVmscNumber_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_GETVMSCNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2DD,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SmsApi_UpdateVmscNumberReq 
	**/
	MSG_SMS_UPDATEVMSCNUMBERREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2DE,	///<Payload type {CAPI2_SmsApi_UpdateVmscNumberReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_UPDATEVMSCNUMBERREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2DF,
	 /** 
	api is CAPI2_SmsApi_GetSMSBearerPreference 
	**/
	MSG_SMS_GETSMSBEARERPREFERENCE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E0,
	 /** 
	payload is ::SMS_BEARER_PREFERENCE_t 
	**/
	MSG_SMS_GETSMSBEARERPREFERENCE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E1,	///<Payload type {::SMS_BEARER_PREFERENCE_t}
	 /** 
	api is CAPI2_SmsApi_SetSMSBearerPreference 
	**/
	MSG_SMS_SETSMSBEARERPREFERENCE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E2,	///<Payload type {CAPI2_SmsApi_SetSMSBearerPreference_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SMS_SETSMSBEARERPREFERENCE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E3,
	 /** 
	api is CAPI2_SmsApi_SetSmsReadStatusChangeMode 
	**/
	MSG_SMS_SETSMSREADSTATUSCHANGEMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E4,	///<Payload type {CAPI2_SmsApi_SetSmsReadStatusChangeMode_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SMS_SETSMSREADSTATUSCHANGEMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E5,
	 /** 
	api is CAPI2_SmsApi_GetSmsReadStatusChangeMode 
	**/
	MSG_SMS_GETSMSREADSTATUSCHANGEMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E6,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_GETSMSREADSTATUSCHANGEMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_ChangeSmsStatusReq 
	**/
	MSG_SMS_CHANGESTATUSREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2E8,	///<Payload type {CAPI2_SmsApi_ChangeSmsStatusReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_CHANGESTATUSREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2E9,
	 /** 
	api is CAPI2_SmsApi_SendMEStoredStatusInd 
	**/
	MSG_SMS_SENDMESTOREDSTATUSIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2EA,	///<Payload type {CAPI2_SmsApi_SendMEStoredStatusInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDMESTOREDSTATUSIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2EB,
	 /** 
	api is CAPI2_SmsApi_SendMERetrieveSmsDataInd 
	**/
	MSG_SMS_SENDMERETRIEVESMSDATAIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2EC,	///<Payload type {CAPI2_SmsApi_SendMERetrieveSmsDataInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDMERETRIEVESMSDATAIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2ED,
	 /** 
	api is CAPI2_SmsApi_SendMERemovedStatusInd 
	**/
	MSG_SMS_SENDMEREMOVEDSTATUSIND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2EE,	///<Payload type {CAPI2_SmsApi_SendMERemovedStatusInd_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SENDMEREMOVEDSTATUSIND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2EF,
	 /** 
	api is CAPI2_SmsApi_SetSmsStoredState 
	**/
	MSG_SMS_SETSMSSTOREDSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F0,	///<Payload type {CAPI2_SmsApi_SetSmsStoredState_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SMS_SETSMSSTOREDSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F1,
	 /** 
	api is CAPI2_SmsApi_IsCachedDataReady 
	**/
	MSG_SMS_ISCACHEDDATAREADY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F2,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SMS_ISCACHEDDATAREADY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SmsApi_GetEnhancedVMInfoIEI 
	**/
	MSG_SMS_GETENHANCEDVMINFOIEI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F4,
	 /** 
	payload is ::SmsEnhancedVMInd_t 
	**/
	MSG_SMS_GETENHANCEDVMINFOIEI_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F5,	///<Payload type {::SmsEnhancedVMInd_t}
	 /** 
	api is CAPI2_SmsApi_SetAllNewMsgDisplayPref 
	**/
	MSG_SMS_SETALLNEWMSGDISPLAYPREF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F6,	///<Payload type {CAPI2_SmsApi_SetAllNewMsgDisplayPref_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_SETALLNEWMSGDISPLAYPREF_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F7,
	 /** 
	api is CAPI2_SmsApi_SendAckToNetwork 
	**/
	MSG_SMS_ACKTONETWORK_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2F8,	///<Payload type {CAPI2_SmsApi_SendAckToNetwork_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SMS_ACKTONETWORK_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2F9,
	 /** 
	api is CAPI2_SmsApi_StartMultiSmsTransferReq 
	**/
	MSG_SMS_STARTMULTISMSTRANSFER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2FA,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_STARTMULTISMSTRANSFER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2FB,
	 /** 
	api is CAPI2_SmsApi_StopMultiSmsTransferReq 
	**/
	MSG_SMS_STOPMULTISMSTRANSFER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2FC,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_STOPMULTISMSTRANSFER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2FD,
	 /** 
	api is CAPI2_SMS_StartCellBroadcastReq 
	**/
	MSG_SMS_START_CELL_BROADCAST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x2FE,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_START_CELL_BROADCAST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x2FF,
	 /** 
	api is CAPI2_SMS_SimInit 
	**/
	MSG_SMS_SIMINIT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x300,
	 /** 
	payload is void 
	**/
	MSG_SMS_SIMINIT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x301,
	 /** 
	api is CAPI2_SMS_SetPDAStorageOverFlowFlag 
	**/
	MSG_SMS_PDA_OVERFLOW_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x302,	///<Payload type {CAPI2_SMS_SetPDAStorageOverFlowFlag_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SMS_PDA_OVERFLOW_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x303,
	 /** 
	api is CAPI2_ISimApi_SendAuthenAkaReq 
	**/
	MSG_ISIM_SENDAUTHENAKAREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x304,	///<Payload type {CAPI2_ISimApi_SendAuthenAkaReq_Req_t}
	 /** 
	api is CAPI2_ISimApi_IsIsimSupported 
	**/
	MSG_ISIM_ISISIMSUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x306,
	 /** 
	payload is ::Boolean 
	**/
	MSG_ISIM_ISISIMSUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x307,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_ISimApi_IsIsimActivated 
	**/
	MSG_ISIM_ISISIMACTIVATED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x308,
	 /** 
	payload is ::Boolean 
	**/
	MSG_ISIM_ISISIMACTIVATED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x309,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_ISimApi_ActivateIsimAppli 
	**/
	MSG_ISIM_ACTIVATEISIMAPPLI_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30A,
	 /** 
	api is CAPI2_ISimApi_SendAuthenHttpReq 
	**/
	MSG_ISIM_SENDAUTHENHTTPREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30C,	///<Payload type {CAPI2_ISimApi_SendAuthenHttpReq_Req_t}
	 /** 
	api is CAPI2_ISimApi_SendAuthenGbaNafReq 
	**/
	MSG_ISIM_SENDAUTHENGBANAFREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x30E,	///<Payload type {CAPI2_ISimApi_SendAuthenGbaNafReq_Req_t}
	 /** 
	api is CAPI2_ISimApi_SendAuthenGbaBootReq 
	**/
	MSG_ISIM_SENDAUTHENGBABOOTREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x310,	///<Payload type {CAPI2_ISimApi_SendAuthenGbaBootReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_GetAlpha 
	**/
	MSG_PBK_GETALPHA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x312,	///<Payload type {CAPI2_PbkApi_GetAlpha_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PBK_GETALPHA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x313,	///<Payload type {void}
	 /** 
	api is CAPI2_PbkApi_IsEmergencyCallNumber 
	**/
	MSG_PBK_ISEMERGENCYCALLNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x314,	///<Payload type {CAPI2_PbkApi_IsEmergencyCallNumber_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISEMERGENCYCALLNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x315,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_IsPartialEmergencyCallNumber 
	**/
	MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x316,	///<Payload type {CAPI2_PbkApi_IsPartialEmergencyCallNumber_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISPARTIALEMERGENCYCALLNUMBER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x317,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_SendInfoReq 
	**/
	MSG_PBK_SENDINFOREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x318,	///<Payload type {CAPI2_PbkApi_SendInfoReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendFindAlphaMatchMultipleReq 
	**/
	MSG_SENDFINDALPHAMATCHMULTIPLEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31A,	///<Payload type {CAPI2_PbkApi_SendFindAlphaMatchMultipleReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendFindAlphaMatchOneReq 
	**/
	MSG_SENDFINDALPHAMATCHONEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31C,	///<Payload type {CAPI2_PbkApi_SendFindAlphaMatchOneReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_IsReady 
	**/
	MSG_PBK_ISREADY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x31E,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISREADY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x31F,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_SendReadEntryReq 
	**/
	MSG_SENDREADENTRYREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x320,	///<Payload type {CAPI2_PbkApi_SendReadEntryReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendWriteEntryReq 
	**/
	MSG_PBK_SENDWRITEENTRYREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x322,	///<Payload type {CAPI2_PbkApi_SendWriteEntryReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendUpdateEntryReq 
	**/
	MSG_PBK_SENDUPDATEENTRYREQ  = MSG_GRP_CAPI2_GEN_0 + 0x324,	///<Payload type {CAPI2_PbkApi_SendUpdateEntryReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_SendIsNumDiallableReq 
	**/
	MSG_PBK_SENDISNUMDIALLABLEREQ  = MSG_GRP_CAPI2_GEN_0 + 0x326,	///<Payload type {CAPI2_PbkApi_SendIsNumDiallableReq_Req_t}
	 /** 
	api is CAPI2_PbkApi_IsNumDiallable 
	**/
	MSG_PBK_ISNUMDIALLABLE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x328,	///<Payload type {CAPI2_PbkApi_IsNumDiallable_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISNUMDIALLABLE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x329,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_IsNumBarred 
	**/
	MSG_PBK_ISNUMBARRED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32A,	///<Payload type {CAPI2_PbkApi_IsNumBarred_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISNUMBARRED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x32B,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PbkApi_IsUssdDiallable 
	**/
	MSG_PBK_ISUSSDDIALLABLE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32C,	///<Payload type {CAPI2_PbkApi_IsUssdDiallable_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PBK_ISUSSDDIALLABLE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x32D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_SetPDPContext 
	**/
	MSG_PDP_SETPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x32E,	///<Payload type {CAPI2_PdpApi_SetPDPContext_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x32F,
	 /** 
	api is CAPI2_PdpApi_SetSecPDPContext 
	**/
	MSG_PDP_SETSECPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x330,	///<Payload type {CAPI2_PdpApi_SetSecPDPContext_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETSECPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x331,
	 /** 
	api is CAPI2_PdpApi_GetGPRSQoS 
	**/
	MSG_PDP_GETGPRSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x332,	///<Payload type {CAPI2_PdpApi_GetGPRSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETGPRSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x333,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetGPRSQoS 
	**/
	MSG_PDP_SETGPRSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x334,	///<Payload type {CAPI2_PdpApi_SetGPRSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETGPRSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x335,
	 /** 
	api is CAPI2_PdpApi_GetGPRSMinQoS 
	**/
	MSG_PDP_GETGPRSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x336,	///<Payload type {CAPI2_PdpApi_GetGPRSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETGPRSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x337,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetGPRSMinQoS 
	**/
	MSG_PDP_SETGPRSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x338,	///<Payload type {CAPI2_PdpApi_SetGPRSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETGPRSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x339,
	 /** 
	api is CAPI2_NetRegApi_SendCombinedAttachReq 
	**/
	MSG_MS_SENDCOMBINEDATTACHREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33A,	///<Payload type {CAPI2_NetRegApi_SendCombinedAttachReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SENDCOMBINEDATTACHREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x33B,
	 /** 
	api is CAPI2_NetRegApi_SendDetachReq 
	**/
	MSG_MS_SENDDETACHREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33C,	///<Payload type {CAPI2_NetRegApi_SendDetachReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SENDDETACHREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x33D,
	 /** 
	api is CAPI2_MS_GetGPRSAttachStatus 
	**/
	MSG_MS_GETGPRSATTACHSTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x33E,
	 /** 
	payload is ::AttachState_t 
	**/
	MSG_MS_GETGPRSATTACHSTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x33F,	///<Payload type {::AttachState_t}
	 /** 
	api is CAPI2_PdpApi_IsSecondaryPdpDefined 
	**/
	MSG_PDP_ISSECONDARYPDPDEFINED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x340,	///<Payload type {CAPI2_PdpApi_IsSecondaryPdpDefined_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_ISSECONDARYPDPDEFINED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x341,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PchExApi_SendPDPActivateReq 
	**/
	MSG_PCHEX_SENDPDPACTIVATEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x342,	///<Payload type {CAPI2_PchExApi_SendPDPActivateReq_Req_t}
	 /** 
	api is CAPI2_PchExApi_SendPDPModifyReq 
	**/
	MSG_PCHEX_SENDPDPMODIFYREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x344,	///<Payload type {CAPI2_PchExApi_SendPDPModifyReq_Req_t}
	 /** 
	api is CAPI2_PchExApi_SendPDPDeactivateReq 
	**/
	MSG_PCHEX_SENDPDPDEACTIVATEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x346,	///<Payload type {CAPI2_PchExApi_SendPDPDeactivateReq_Req_t}
	 /** 
	api is CAPI2_PchExApi_SendPDPActivateSecReq 
	**/
	MSG_PCHEX_SENDPDPACTIVATESECREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x348,	///<Payload type {CAPI2_PchExApi_SendPDPActivateSecReq_Req_t}
	 /** 
	api is CAPI2_PdpApi_GetGPRSActivateStatus 
	**/
	MSG_PDP_GETGPRSACTIVATESTATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34A,
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETGPRSACTIVATESTATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x34B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_NetRegApi_SetMSClass 
	**/
	MSG_PDP_SETMSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34C,	///<Payload type {CAPI2_NetRegApi_SetMSClass_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETMSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x34D,
	 /** 
	api is CAPI2_PDP_GetMSClass 
	**/
	MSG_PDP_GETMSCLASS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x34E,
	 /** 
	payload is ::MSClass_t 
	**/
	MSG_PDP_GETMSCLASS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x34F,	///<Payload type {::MSClass_t}
	 /** 
	api is CAPI2_PdpApi_GetUMTSTft 
	**/
	MSG_PDP_GETUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x350,	///<Payload type {CAPI2_PdpApi_GetUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x351,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetUMTSTft 
	**/
	MSG_PDP_SETUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x352,	///<Payload type {CAPI2_PdpApi_SetUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x353,
	 /** 
	api is CAPI2_PdpApi_DeleteUMTSTft 
	**/
	MSG_PDP_DELETEUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x354,	///<Payload type {CAPI2_PdpApi_DeleteUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_DELETEUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x355,
	 /** 
	api is CAPI2_PdpApi_DeactivateSNDCPConnection 
	**/
	MSG_PDP_DEACTIVATESNDCPCONNECTION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x356,	///<Payload type {CAPI2_PdpApi_DeactivateSNDCPConnection_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_DEACTIVATESNDCPCONNECTION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x357,
	 /** 
	api is CAPI2_PdpApi_GetR99UMTSMinQoS 
	**/
	MSG_PDP_GETR99UMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x358,	///<Payload type {CAPI2_PdpApi_GetR99UMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETR99UMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x359,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetR99UMTSQoS 
	**/
	MSG_PDP_GETR99UMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35A,	///<Payload type {CAPI2_PdpApi_GetR99UMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETR99UMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetUMTSMinQoS 
	**/
	MSG_PDP_GETUMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35C,	///<Payload type {CAPI2_PdpApi_GetUMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETUMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35D,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetUMTSQoS 
	**/
	MSG_PDP_GETUMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x35E,	///<Payload type {CAPI2_PdpApi_GetUMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETUMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x35F,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_GetNegQoS 
	**/
	MSG_PDP_GETNEGQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x360,	///<Payload type {CAPI2_PdpApi_GetNegQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETNEGQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x361,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetR99UMTSMinQoS 
	**/
	MSG_PDP_SETR99UMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x362,	///<Payload type {CAPI2_PdpApi_SetR99UMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETR99UMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x363,
	 /** 
	api is CAPI2_PdpApi_SetR99UMTSQoS 
	**/
	MSG_PDP_SETR99UMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x364,	///<Payload type {CAPI2_PdpApi_SetR99UMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETR99UMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x365,
	 /** 
	api is CAPI2_PdpApi_SetUMTSMinQoS 
	**/
	MSG_PDP_SETUMTSMINQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x366,	///<Payload type {CAPI2_PdpApi_SetUMTSMinQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETUMTSMINQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x367,
	 /** 
	api is CAPI2_PdpApi_SetUMTSQoS 
	**/
	MSG_PDP_SETUMTSQOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x368,	///<Payload type {CAPI2_PdpApi_SetUMTSQoS_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETUMTSQOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x369,
	 /** 
	api is CAPI2_PdpApi_GetNegotiatedParms 
	**/
	MSG_PDP_GETNEGOTIATEDPARMS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x36A,	///<Payload type {CAPI2_PdpApi_GetNegotiatedParms_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETNEGOTIATEDPARMS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x36B,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_MS_IsGprsCallActive 
	**/
	MSG_MS_ISGPRSCALLACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x36C,	///<Payload type {CAPI2_MS_IsGprsCallActive_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_MS_ISGPRSCALLACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x36D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_MS_SetChanGprsCallActive 
	**/
	MSG_MS_SETCHANGPRSCALLACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x36E,	///<Payload type {CAPI2_MS_SetChanGprsCallActive_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SETCHANGPRSCALLACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x36F,
	 /** 
	api is CAPI2_MS_SetCidForGprsActiveChan 
	**/
	MSG_MS_SETCIDFORGPRSACTIVECHAN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x370,	///<Payload type {CAPI2_MS_SetCidForGprsActiveChan_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SETCIDFORGPRSACTIVECHAN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x371,
	 /** 
	api is CAPI2_PdpApi_GetPPPModemCid 
	**/
	MSG_PDP_GETPPPMODEMCID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x372,
	 /** 
	payload is ::PCHCid_t 
	**/
	MSG_PDP_GETPPPMODEMCID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x373,	///<Payload type {::PCHCid_t}
	 /** 
	api is CAPI2_MS_GetGprsActiveChanFromCid 
	**/
	MSG_MS_GETGPRSACTIVECHANFROMCID_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x374,	///<Payload type {CAPI2_MS_GetGprsActiveChanFromCid_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GETGPRSACTIVECHANFROMCID_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x375,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_MS_GetCidFromGprsActiveChan 
	**/
	MSG_MS_GETCIDFROMGPRSACTIVECHAN_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x376,	///<Payload type {CAPI2_MS_GetCidFromGprsActiveChan_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_MS_GETCIDFROMGPRSACTIVECHAN_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x377,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_PdpApi_GetPDPAddress 
	**/
	MSG_PDP_GETPDPADDRESS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x378,	///<Payload type {CAPI2_PdpApi_GetPDPAddress_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPDPADDRESS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x379,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SendTBFData 
	**/
	MSG_PDP_SENDTBFDATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x37A,	///<Payload type {CAPI2_PdpApi_SendTBFData_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SENDTBFDATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x37B,
	 /** 
	api is CAPI2_PdpApi_TftAddFilter 
	**/
	MSG_PDP_TFTADDFILTER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x37C,	///<Payload type {CAPI2_PdpApi_TftAddFilter_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_TFTADDFILTER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x37D,
	 /** 
	api is CAPI2_PdpApi_SetPCHContextState 
	**/
	MSG_PDP_SETPCHCONTEXTSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x37E,	///<Payload type {CAPI2_PdpApi_SetPCHContextState_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETPCHCONTEXTSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x37F,
	 /** 
	api is CAPI2_PdpApi_SetDefaultPDPContext 
	**/
	MSG_PDP_SETDEFAULTPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x380,	///<Payload type {CAPI2_PdpApi_SetDefaultPDPContext_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETDEFAULTPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x381,
	 /** 
	api is CAPI2_PchExApi_GetDecodedProtConfig 
	**/
	MSG_PCHEX_READDECODEDPROTCONFIG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x382,	///<Payload type {CAPI2_PchExApi_GetDecodedProtConfig_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PCHEX_READDECODEDPROTCONFIG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x383,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PchExApi_BuildIpConfigOptions 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x384,	///<Payload type {CAPI2_PchExApi_BuildIpConfigOptions_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x385,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PchExApi_BuildIpConfigOptions2 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x386,	///<Payload type {CAPI2_PchExApi_BuildIpConfigOptions2_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTIONS2_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x387,	///<Payload type {void}
	 /** 
	api is CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x388,	///<Payload type {CAPI2_PchExApi_BuildIpConfigOptionsWithChapAuthType_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PCHEX_BUILDPROTCONFIGOPTION_CHAP_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x389,	///<Payload type {void}
	 /** 
	api is CAPI2_PdpApi_GetDefaultQos 
	**/
	MSG_PDP_GET_DEFAULT_QOS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x38A,
	 /** 
	payload is void 
	**/
	MSG_PDP_GET_DEFAULT_QOS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x38B,	///<Payload type {void}
	 /** 
	api is CAPI2_PdpApi_IsPDPContextActive 
	**/
	MSG_PDP_ISCONTEXT_ACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x38C,	///<Payload type {CAPI2_PdpApi_IsPDPContextActive_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_ISCONTEXT_ACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x38D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_ActivateSNDCPConnection 
	**/
	MSG_PDP_ACTIVATESNDCPCONNECTION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x38E,	///<Payload type {CAPI2_PdpApi_ActivateSNDCPConnection_Req_t}
	 /** 
	api is CAPI2_PdpApi_GetPDPContext 
	**/
	MSG_PDP_GETPDPCONTEXT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x390,	///<Payload type {CAPI2_PdpApi_GetPDPContext_Req_t}
	 /** 
	payload is ::PDPDefaultContext_t 
	**/
	MSG_PDP_GETPDPCONTEXT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x391,	///<Payload type {::PDPDefaultContext_t}
	 /** 
	api is CAPI2_PdpApi_GetDefinedPDPContextCidList 
	**/
	MSG_PDP_GETPDPCONTEXT_CID_LIST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x392,
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPDPCONTEXT_CID_LIST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x393,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SYS_GetBootLoaderVersion 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x396,	///<Payload type {CAPI2_SYS_GetBootLoaderVersion_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARAM_BOOTLOADER_VER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x397,	///<Payload type {void}
	 /** 
	api is CAPI2_SYS_GetDSFVersion 
	**/
	MSG_SYSPARAM_DSF_VER_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x398,	///<Payload type {CAPI2_SYS_GetDSFVersion_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARAM_DSF_VER_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x399,	///<Payload type {void}
	 /** 
	api is CAPI2_USimApi_GetUstData 
	**/
	MSG_USIM_UST_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x39A,
	 /** 
	payload is void 
	**/
	MSG_USIM_UST_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x39B,	///<Payload type {void}
	 /** 
	api is CAPI2_PATCH_GetRevision 
	**/
	MSG_PATCH_GET_REVISION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x39C,
	 /** 
	payload is ::UInt8 
	**/
	MSG_PATCH_GET_REVISION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x39D,	///<Payload type {::UInt8}

	MSG_GRP_CAPI2_SS_BEGIN = MSG_GRP_CAPI2_SS + 0,
	 /** 
	api is CAPI2_SS_SendCallForwardReq 
	**/
	MSG_SS_SENDCALLFORWARDREQ_REQ  = MSG_GRP_CAPI2_SS + 0x2,	///<Payload type {CAPI2_SS_SendCallForwardReq_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallForwardStatus 
	**/
	MSG_SS_QUERYCALLFORWARDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x4,	///<Payload type {CAPI2_SS_QueryCallForwardStatus_Req_t}
	 /** 
	api is CAPI2_SS_SendCallBarringReq 
	**/
	MSG_SS_SENDCALLBARRINGREQ_REQ  = MSG_GRP_CAPI2_SS + 0x6,	///<Payload type {CAPI2_SS_SendCallBarringReq_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallBarringStatus 
	**/
	MSG_SS_QUERYCALLBARRINGSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x8,	///<Payload type {CAPI2_SS_QueryCallBarringStatus_Req_t}
	 /** 
	api is CAPI2_SS_SendCallBarringPWDChangeReq 
	**/
	MSG_SS_SENDCALLBARRINGPWDCHANGEREQ_REQ  = MSG_GRP_CAPI2_SS + 0xA,	///<Payload type {CAPI2_SS_SendCallBarringPWDChangeReq_Req_t}
	 /** 
	api is CAPI2_SS_SendCallWaitingReq 
	**/
	MSG_SS_SENDCALLWAITINGREQ_REQ  = MSG_GRP_CAPI2_SS + 0xC,	///<Payload type {CAPI2_SS_SendCallWaitingReq_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallWaitingStatus 
	**/
	MSG_SS_QUERYCALLWAITINGSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0xE,	///<Payload type {CAPI2_SS_QueryCallWaitingStatus_Req_t}
	 /** 
	api is CAPI2_SS_QueryCallingLineIDStatus 
	**/
	MSG_SS_QUERYCALLINGLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x10,
	 /** 
	api is CAPI2_SS_QueryConnectedLineIDStatus 
	**/
	MSG_SS_QUERYCONNECTEDLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x12,
	 /** 
	api is CAPI2_SS_QueryCallingLineRestrictionStatus 
	**/
	MSG_SS_QUERYCALLINGLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x14,
	 /** 
	api is CAPI2_SS_QueryConnectedLineRestrictionStatus 
	**/
	MSG_SS_QUERYCONNECTEDLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x16,
	 /** 
	api is CAPI2_SS_QueryCallingNAmePresentStatus 
	**/
	MSG_SS_QUERYCALLINGNAMEPRESENTSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x18,
	 /** 
	api is CAPI2_SS_SetCallingLineIDStatus 
	**/
	MSG_SS_SETCALLINGLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x1A,	///<Payload type {CAPI2_SS_SetCallingLineIDStatus_Req_t}
	 /** 
	api is CAPI2_SS_SetCallingLineRestrictionStatus 
	**/
	MSG_SS_SETCALLINGLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x1C,	///<Payload type {CAPI2_SS_SetCallingLineRestrictionStatus_Req_t}
	 /** 
	api is CAPI2_SS_SetConnectedLineIDStatus 
	**/
	MSG_SS_SETCONNECTEDLINEIDSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x1E,	///<Payload type {CAPI2_SS_SetConnectedLineIDStatus_Req_t}
	 /** 
	api is CAPI2_SS_SetConnectedLineRestrictionStatus 
	**/
	MSG_SS_SETCONNECTEDLINERESTRICTIONSTATUS_REQ  = MSG_GRP_CAPI2_SS + 0x20,	///<Payload type {CAPI2_SS_SetConnectedLineRestrictionStatus_Req_t}
	 /** 
	api is CAPI2_SS_SendUSSDConnectReq 
	**/
	MSG_SS_SENDUSSDCONNECTREQ_REQ  = MSG_GRP_CAPI2_SS + 0x22,	///<Payload type {CAPI2_SS_SendUSSDConnectReq_Req_t}
	 /** 
	api is CAPI2_SS_SendUSSDData 
	**/
	MSG_SS_SENDUSSDDATA_REQ  = MSG_GRP_CAPI2_SS + 0x24,	///<Payload type {CAPI2_SS_SendUSSDData_Req_t}
	 /** 
	api is CAPI2_SsApi_DialStrSrvReq 
	**/
	MSG_SSAPI_DIALSTRSRVREQ_REQ  = MSG_GRP_CAPI2_SS + 0x26,	///<Payload type {CAPI2_SsApi_DialStrSrvReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_DIALSTRSRVREQ_RSP  = MSG_GRP_CAPI2_SS + 0x27,
	 /** 
	api is CAPI2_SS_EndUSSDConnectReq 
	**/
	MSG_SS_ENDUSSDCONNECTREQ_REQ  = MSG_GRP_CAPI2_SS + 0x28,	///<Payload type {CAPI2_SS_EndUSSDConnectReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SS_ENDUSSDCONNECTREQ_RSP  = MSG_GRP_CAPI2_SS + 0x29,
	 /** 
	api is CAPI2_SsApi_SsSrvReq 
	**/
	MSG_SSAPI_SSSRVREQ_REQ  = MSG_GRP_CAPI2_SS + 0x2A,	///<Payload type {CAPI2_SsApi_SsSrvReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_SSSRVREQ_RSP  = MSG_GRP_CAPI2_SS + 0x2B,
	 /** 
	api is CAPI2_SsApi_UssdSrvReq 
	**/
	MSG_SSAPI_USSDSRVREQ_REQ  = MSG_GRP_CAPI2_SS + 0x2C,	///<Payload type {CAPI2_SsApi_UssdSrvReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_USSDSRVREQ_RSP  = MSG_GRP_CAPI2_SS + 0x2D,
	 /** 
	api is CAPI2_SsApi_UssdDataReq 
	**/
	MSG_SSAPI_USSDDATAREQ_REQ  = MSG_GRP_CAPI2_SS + 0x2E,	///<Payload type {CAPI2_SsApi_UssdDataReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_USSDDATAREQ_RSP  = MSG_GRP_CAPI2_SS + 0x2F,
	 /** 
	api is CAPI2_SsApi_SsReleaseReq 
	**/
	MSG_SSAPI_SSRELEASEREQ_REQ  = MSG_GRP_CAPI2_SS + 0x30,	///<Payload type {CAPI2_SsApi_SsReleaseReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_SSRELEASEREQ_RSP  = MSG_GRP_CAPI2_SS + 0x31,
	 /** 
	api is CAPI2_SsApi_DataReq 
	**/
	MSG_SSAPI_DATAREQ_REQ  = MSG_GRP_CAPI2_SS + 0x32,	///<Payload type {CAPI2_SsApi_DataReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_DATAREQ_RSP  = MSG_GRP_CAPI2_SS + 0x33,
	 /** 
	api is CAPI2_SS_SsApiReqDispatcher 
	**/
	MSG_SSAPI_DISPATCH_REQ  = MSG_GRP_CAPI2_SS + 0x34,	///<Payload type {CAPI2_SS_SsApiReqDispatcher_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SSAPI_DISPATCH_RSP  = MSG_GRP_CAPI2_SS + 0x35,
	 /** 
	api is CAPI2_SS_GetStr 
	**/
	MSG_SS_GET_STR_REQ  = MSG_GRP_CAPI2_SS + 0x36,	///<Payload type {CAPI2_SS_GetStr_Req_t}
	 /** 
	payload is ::UInt8 
	**/
	MSG_SS_GET_STR_RSP  = MSG_GRP_CAPI2_SS + 0x37,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SS_SetClientID 
	**/
	MSG_SS_SETCLIENTID_REQ  = MSG_GRP_CAPI2_SS + 0x38,
	 /** 
	payload is ::Result_t 
	**/
	MSG_SS_SETCLIENTID_RSP  = MSG_GRP_CAPI2_SS + 0x39,
	 /** 
	api is CAPI2_SS_GetClientID 
	**/
	MSG_SS_GETCLIENTID_REQ  = MSG_GRP_CAPI2_SS + 0x3A,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SS_GETCLIENTID_RSP  = MSG_GRP_CAPI2_SS + 0x3B,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SS_ResetClientID 
	**/
	MSG_SS_RESETCLIENTID_REQ  = MSG_GRP_CAPI2_SS + 0x3C,
	 /** 
	payload is void 
	**/
	MSG_SS_RESETCLIENTID_RSP  = MSG_GRP_CAPI2_SS + 0x3D,

	MSG_GRP_CAPI2_SS_END = MSG_GRP_CAPI2_SS + 0xFF,

	 /** 
	api is CAPI2_SatkApi_GetCachedRootMenuPtr 
	**/
	MSG_SATK_GETCACHEDROOTMENUPTR_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x39E,
	 /** 
	payload is ::SetupMenu_t 
	**/
	MSG_SATK_GETCACHEDROOTMENUPTR_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x39F,	///<Payload type {::SetupMenu_t}
	 /** 
	api is CAPI2_SatkApi_SendUserActivityEvent 
	**/
	MSG_SATK_SENDUSERACTIVITYEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3A0,
	 /** 
	payload is void 
	**/
	MSG_SATK_SENDUSERACTIVITYEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3A1,
	 /** 
	api is CAPI2_SatkApi_SendIdleScreenAvaiEvent 
	**/
	MSG_SATK_SENDIDLESCREENAVAIEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3A2,
	 /** 
	payload is void 
	**/
	MSG_SATK_SENDIDLESCREENAVAIEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3A3,
	 /** 
	api is CAPI2_SatkApi_SendLangSelectEvent 
	**/
	MSG_SATK_SENDLANGSELECTEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3A4,	///<Payload type {CAPI2_SatkApi_SendLangSelectEvent_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SATK_SENDLANGSELECTEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3A5,
	 /** 
	api is CAPI2_SatkApi_SendBrowserTermEvent 
	**/
	MSG_SATK_SENDBROWSERTERMEVENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3A6,	///<Payload type {CAPI2_SatkApi_SendBrowserTermEvent_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SATK_SENDBROWSERTERMEVENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3A7,
	 /** 
	api is CAPI2_SatkApi_CmdResp 
	**/
	MSG_SATK_CMDRESP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3A8,	///<Payload type {CAPI2_SatkApi_CmdResp_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_CMDRESP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3A9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_DataServCmdResp 
	**/
	MSG_SATK_DATASERVCMDRESP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3AA,	///<Payload type {CAPI2_SatkApi_DataServCmdResp_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_DATASERVCMDRESP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3AB,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_SendDataServReq 
	**/
	MSG_SATK_SENDDATASERVREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3AC,	///<Payload type {CAPI2_SatkApi_SendDataServReq_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_SENDDATASERVREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3AD,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_SendTerminalRsp 
	**/
	MSG_SATK_SENDTERMINALRSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3AE,	///<Payload type {CAPI2_SatkApi_SendTerminalRsp_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SATK_SENDTERMINALRSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3AF,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SatkApi_SetTermProfile 
	**/
	MSG_SATK_SETTERMPROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3B0,	///<Payload type {CAPI2_SatkApi_SetTermProfile_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SATK_SETTERMPROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3B1,
	 /** 
	api is CAPI2_SatkApi_SendEnvelopeCmdReq 
	**/
	MSG_SATK_SEND_ENVELOPE_CMD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3B2,	///<Payload type {CAPI2_SatkApi_SendEnvelopeCmdReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendTerminalRspReq 
	**/
	MSG_STK_TERMINAL_RESPONSE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3B4,	///<Payload type {CAPI2_SatkApi_SendTerminalRspReq_Req_t}
	 /** 
	api is CAPI2_StkApi_SendBrowsingStatusEvent 
	**/
	MSG_STK_SEND_BROWSING_STATUS_EVT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3B6,	///<Payload type {CAPI2_StkApi_SendBrowsingStatusEvent_Req_t}
	 /** 
	payload is void 
	**/
	MSG_STK_SEND_BROWSING_STATUS_EVT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3B7,
	 /** 
	api is CAPI2_SatkApi_SendCcSetupReq 
	**/
	MSG_SATK_SEND_CC_SETUP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3B8,	///<Payload type {CAPI2_SatkApi_SendCcSetupReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendCcSsReq 
	**/
	MSG_SATK_SEND_CC_SS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3C0,	///<Payload type {CAPI2_SatkApi_SendCcSsReq_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SATK_SEND_CC_SS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3C1,
	 /** 
	api is CAPI2_SatkApi_SendCcUssdReq 
	**/
	MSG_SATK_SEND_CC_USSD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3C2,	///<Payload type {CAPI2_SatkApi_SendCcUssdReq_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SATK_SEND_CC_USSD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3C3,
	 /** 
	api is CAPI2_SatkApi_SendCcSmsReq 
	**/
	MSG_SATK_SEND_CC_SMS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3C4,	///<Payload type {CAPI2_SatkApi_SendCcSmsReq_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SATK_SEND_CC_SMS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3C5,
	 /** 
	api is CAPI2_LCS_CpMoLrReq 
	**/
	MSG_LCS_CPMOLRREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3C8,	///<Payload type {CAPI2_LCS_CpMoLrReq_Req_t}
	 /** 
	api is CAPI2_LCS_CpMoLrAbort 
	**/
	MSG_LCS_CPMOLRABORT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3CA,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPMOLRABORT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3CB,
	 /** 
	api is CAPI2_LCS_CpMtLrVerificationRsp 
	**/
	MSG_LCS_CPMTLRVERIFICATIONRSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3CC,	///<Payload type {CAPI2_LCS_CpMtLrVerificationRsp_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPMTLRVERIFICATIONRSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3CD,
	 /** 
	api is CAPI2_LCS_CpMtLrRsp 
	**/
	MSG_LCS_CPMTLRRSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3CE,	///<Payload type {CAPI2_LCS_CpMtLrRsp_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPMTLRRSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3CF,
	 /** 
	api is CAPI2_LCS_CpLocUpdateRsp 
	**/
	MSG_LCS_CPLOCUPDATERSP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3D0,	///<Payload type {CAPI2_LCS_CpLocUpdateRsp_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_CPLOCUPDATERSP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3D1,
	 /** 
	api is CAPI2_LCS_DecodePosEstimate 
	**/
	MSG_LCS_DECODEPOSESTIMATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3D2,	///<Payload type {CAPI2_LCS_DecodePosEstimate_Req_t}
	 /** 
	payload is void 
	**/
	MSG_LCS_DECODEPOSESTIMATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3D3,	///<Payload type {void}
	 /** 
	api is CAPI2_LCS_EncodeAssistanceReq 
	**/
	MSG_LCS_ENCODEASSISTANCEREQ_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3D4,	///<Payload type {CAPI2_LCS_EncodeAssistanceReq_Req_t}
	 /** 
	payload is ::int 
	**/
	MSG_LCS_ENCODEASSISTANCEREQ_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3D5,	///<Payload type {::int}
	 /** 
	api is CAPI2_LCS_FttSyncReq 
	**/
	MSG_LCS_FTT_SYNC_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3DE,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_FTT_SYNC_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x3DF,
	 /** 
	api is CAPI2_CcApi_MakeVoiceCall 
	**/
	MSG_CC_MAKEVOICECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3E4,	///<Payload type {CAPI2_CcApi_MakeVoiceCall_Req_t}
	 /** 
	api is CAPI2_CcApi_MakeDataCall 
	**/
	MSG_CC_MAKEDATACALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3E6,	///<Payload type {CAPI2_CcApi_MakeDataCall_Req_t}
	 /** 
	api is CAPI2_CcApi_MakeFaxCall 
	**/
	MSG_CC_MAKEFAXCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3E8,	///<Payload type {CAPI2_CcApi_MakeFaxCall_Req_t}
	 /** 
	api is CAPI2_CcApi_MakeVideoCall 
	**/
	MSG_CC_MAKEVIDEOCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3EA,	///<Payload type {CAPI2_CcApi_MakeVideoCall_Req_t}
	 /** 
	api is CAPI2_CcApi_EndCall 
	**/
	MSG_CC_ENDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3EC,	///<Payload type {CAPI2_CcApi_EndCall_Req_t}
	 /** 
	api is CAPI2_CcApi_EndAllCalls 
	**/
	MSG_CC_ENDALLCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3EE,
	 /** 
	api is CAPI2_CcApi_EndCallImmediate 
	**/
	MSG_CCAPI_ENDCALL_IMMEDIATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3F0,	///<Payload type {CAPI2_CcApi_EndCallImmediate_Req_t}
	 /** 
	api is CAPI2_CcApi_EndAllCallsImmediate 
	**/
	MSG_CCAPI_ENDALLCALLS_IMMEDIATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3F2,
	 /** 
	api is CAPI2_CcApi_EndMPTYCalls 
	**/
	MSG_CC_ENDMPTYCALLS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3F4,
	 /** 
	api is CAPI2_CcApi_EndHeldCall 
	**/
	MSG_CC_ENDHELDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3F6,
	 /** 
	api is CAPI2_CcApi_AcceptVoiceCall 
	**/
	MSG_CC_ACCEPTVOICECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3F8,	///<Payload type {CAPI2_CcApi_AcceptVoiceCall_Req_t}
	 /** 
	api is CAPI2_CcApi_AcceptDataCall 
	**/
	MSG_CC_ACCEPTDATACALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3FA,	///<Payload type {CAPI2_CcApi_AcceptDataCall_Req_t}
	 /** 
	api is CAPI2_CcApi_AcceptWaitingCall 
	**/
	MSG_CC_ACCEPTWAITINGCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3FC,
	 /** 
	api is CAPI2_CcApi_AcceptVideoCall 
	**/
	MSG_CC_ACCEPTVIDEOCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x3FE,	///<Payload type {CAPI2_CcApi_AcceptVideoCall_Req_t}
	 /** 
	api is CAPI2_CcApi_HoldCurrentCall 
	**/
	MSG_CC_HOLDCURRENTCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x400,
	 /** 
	api is CAPI2_CcApi_HoldCall 
	**/
	MSG_CC_HOLDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x402,	///<Payload type {CAPI2_CcApi_HoldCall_Req_t}
	 /** 
	api is CAPI2_CcApi_RetrieveNextHeldCall 
	**/
	MSG_CC_RETRIEVENEXTHELDCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x404,
	 /** 
	api is CAPI2_CcApi_RetrieveCall 
	**/
	MSG_CC_RETRIEVECALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x406,	///<Payload type {CAPI2_CcApi_RetrieveCall_Req_t}
	 /** 
	api is CAPI2_CcApi_SwapCall 
	**/
	MSG_CC_SWAPCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x408,	///<Payload type {CAPI2_CcApi_SwapCall_Req_t}
	 /** 
	api is CAPI2_CcApi_SplitCall 
	**/
	MSG_CC_SPLITCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x40A,	///<Payload type {CAPI2_CcApi_SplitCall_Req_t}
	 /** 
	api is CAPI2_CcApi_JoinCall 
	**/
	MSG_CC_JOINCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x40C,	///<Payload type {CAPI2_CcApi_JoinCall_Req_t}
	 /** 
	api is CAPI2_CcApi_TransferCall 
	**/
	MSG_CC_TRANSFERCALL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x40E,	///<Payload type {CAPI2_CcApi_TransferCall_Req_t}
	 /** 
	api is CAPI2_CcApi_GetCNAPName 
	**/
	MSG_CC_GETCNAPNAME_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4B4,	///<Payload type {CAPI2_CcApi_GetCNAPName_Req_t}
	 /** 
	payload is ::CcCnapName_t 
	**/
	MSG_CC_GETCNAPNAME_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4B5,	///<Payload type {::CcCnapName_t}
	 /** 
	api is CAPI2_SYSPARM_GetHSUPASupported 
	**/
	MSG_SYSPARM_GET_HSUPA_SUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4B6,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYSPARM_GET_HSUPA_SUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4B7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_SYSPARM_GetHSDPASupported 
	**/
	MSG_SYSPARM_GET_HSDPA_SUPPORTED_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4B8,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYSPARM_GET_HSDPA_SUPPORTED_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4B9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_NetRegApi_ForcePsReleaseReq 
	**/
	MSG_MS_FORCE_PS_REL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4BC,
	 /** 
	payload is void 
	**/
	MSG_MS_FORCE_PS_REL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4BD,
	 /** 
	api is CAPI2_CcApi_IsCurrentStateMpty 
	**/
	MSG_CC_ISCURRENTSTATEMPTY_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4BE,	///<Payload type {CAPI2_CcApi_IsCurrentStateMpty_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CC_ISCURRENTSTATEMPTY_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4BF,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_GetPCHContextState 
	**/
	MSG_PDP_GETPCHCONTEXTSTATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4C0,	///<Payload type {CAPI2_PdpApi_GetPCHContextState_Req_t}
	 /** 
	payload is ::PCHContextState_t 
	**/
	MSG_PDP_GETPCHCONTEXTSTATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4C1,	///<Payload type {::PCHContextState_t}
	 /** 
	api is CAPI2_PdpApi_GetPDPContextEx 
	**/
	MSG_PDP_GETPCHCONTEXT_EX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4C2,	///<Payload type {CAPI2_PdpApi_GetPDPContextEx_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPCHCONTEXT_EX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4C3,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_SimApi_GetCurrLockedSimlockType 
	**/
	MSG_SIM_PIN_LOCK_TYPE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4C4,
	 /** 
	payload is ::SIM_PIN_Status_t 
	**/
	MSG_SIM_PIN_LOCK_TYPE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4C5,	///<Payload type {::SIM_PIN_Status_t}
	 /** 
	api is CAPI2_SimApi_SubmitSelectFileSendApduReq 
	**/
	MSG_SIM_SEND_APDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4C8,	///<Payload type {CAPI2_SimApi_SubmitSelectFileSendApduReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SubmitMulRecordEFileReq 
	**/
	MSG_SIM_MUL_REC_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4CC,	///<Payload type {CAPI2_SimApi_SubmitMulRecordEFileReq_Req_t}
	 /** 
	api is CAPI2_SimApi_SendSelectApplicationReq 
	**/
	MSG_SIM_SELECT_APPLICATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4D0,	///<Payload type {CAPI2_SimApi_SendSelectApplicationReq_Req_t}
	 /** 
	api is CAPI2_SimApi_PerformSteeringOfRoaming 
	**/
	MSG_SIM_PEROFRM_STEERING_OF_ROAMING_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4D2,	///<Payload type {CAPI2_SimApi_PerformSteeringOfRoaming_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_PEROFRM_STEERING_OF_ROAMING_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4D3,
	 /** 
	api is CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq 
	**/
	MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4D6,	///<Payload type {CAPI2_SatkApi_SendProactiveCmdFetchingOnOffReq_Req_t}
	 /** 
	api is CAPI2_SatkApi_SendExtProactiveCmdReq 
	**/
	MSG_SATK_SEND_EXT_PROACTIVE_CMD_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4D8,	///<Payload type {CAPI2_SatkApi_SendExtProactiveCmdReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SATK_SEND_EXT_PROACTIVE_CMD_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4D9,
	 /** 
	api is CAPI2_SatkApi_SendTerminalProfileReq 
	**/
	MSG_SATK_SEND_TERMINAL_PROFILE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4DA,	///<Payload type {CAPI2_SatkApi_SendTerminalProfileReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SATK_SEND_TERMINAL_PROFILE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4DB,
	 /** 
	api is CAPI2_SatkApi_SendPollingIntervalReq 
	**/
	MSG_STK_POLLING_INTERVAL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4DC,	///<Payload type {CAPI2_SatkApi_SendPollingIntervalReq_Req_t}
	 /** 
	api is CAPI2_PdpApi_SetPDPActivationCallControlFlag 
	**/
	MSG_PDP_SETPDPACTIVATIONCCFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4E0,	///<Payload type {CAPI2_PdpApi_SetPDPActivationCallControlFlag_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PDP_SETPDPACTIVATIONCCFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4E1,
	 /** 
	api is CAPI2_PdpApi_GetPDPActivationCallControlFlag 
	**/
	MSG_PDP_GETPDPACTIVATIONCCFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4E2,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_GETPDPACTIVATIONCCFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4E3,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_SendPDPActivateReq_PDU 
	**/
	MSG_PDP_SENDPDPACTIVATIONPDU_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4E4,	///<Payload type {CAPI2_PdpApi_SendPDPActivateReq_PDU_Req_t}
	 /** 
	api is CAPI2_PdpApi_RejectNWIPDPActivation 
	**/
	MSG_PDP_REJECTNWIACTIVATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4E6,	///<Payload type {CAPI2_PdpApi_RejectNWIPDPActivation_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PDP_REJECTNWIACTIVATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4E7,
	 /** 
	api is CAPI2_PdpApi_SetPDPBearerCtrlMode 
	**/
	MSG_PDP_SETBEARERCTRLMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4E8,	///<Payload type {CAPI2_PdpApi_SetPDPBearerCtrlMode_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PDP_SETBEARERCTRLMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4E9,
	 /** 
	api is CAPI2_PdpApi_GetPDPBearerCtrlMode 
	**/
	MSG_PDP_GETBEARERCTRLMODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4EA,
	 /** 
	payload is ::UInt8 
	**/
	MSG_PDP_GETBEARERCTRLMODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4EB,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_PdpApi_RejectSecNWIPDPActivation 
	**/
	MSG_PDP_REJECTSECNWIACTIVATION_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4EC,	///<Payload type {CAPI2_PdpApi_RejectSecNWIPDPActivation_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PDP_REJECTSECNWIACTIVATION_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4ED,
	 /** 
	api is CAPI2_PdpApi_SetPDPNWIControlFlag 
	**/
	MSG_PDP_SETPDPNWICONTROLFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4EE,	///<Payload type {CAPI2_PdpApi_SetPDPNWIControlFlag_Req_t}
	 /** 
	payload is void 
	**/
	MSG_PDP_SETPDPNWICONTROLFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4EF,
	 /** 
	api is CAPI2_PdpApi_GetPDPNWIControlFlag 
	**/
	MSG_PDP_GETPDPNWICONTROLFLAG_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4F0,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_GETPDPNWICONTROLFLAG_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4F1,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_CheckUMTSTft 
	**/
	MSG_PDP_CHECKUMTSTFT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4F4,	///<Payload type {CAPI2_PdpApi_CheckUMTSTft_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_CHECKUMTSTFT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4F5,
	 /** 
	api is CAPI2_PdpApi_IsAnyPDPContextActive 
	**/
	MSG_PDP_ISANYPDPCONTEXTACTIVE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4F6,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_ISANYPDPCONTEXTACTIVE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4F7,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PdpApi_IsAnyPDPContextActivePending 
	**/
	MSG_PDP_ISANYPDPCONTEXTPENDING_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x4F8,
	 /** 
	payload is ::Boolean 
	**/
	MSG_PDP_ISANYPDPCONTEXTPENDING_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x4F9,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_CcApi_SetElement 
	**/
	MSG_CCAPI_SET_ELEMENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6A4,	///<Payload type {CAPI2_CcApi_SetElement_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_SET_ELEMENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6A5,
	 /** 
	api is CAPI2_CcApi_GetElement 
	**/
	MSG_CCAPI_GET_ELEMENT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6A6,	///<Payload type {CAPI2_CcApi_GetElement_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_CCAPI_GET_ELEMENT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6A7,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_WL_PsSetFilterList 
	**/
	MSG_WL_PS_SET_FILTER_LIST_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6A8,	///<Payload type {CAPI2_WL_PsSetFilterList_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_WL_PS_SET_FILTER_LIST_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6A9,
	 /** 
	api is CAPI2_PdpApi_GetProtConfigOptions 
	**/
	MSG_PDP_GETPROTCONFIGOPTIONS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6AA,	///<Payload type {CAPI2_PdpApi_GetProtConfigOptions_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_GETPROTCONFIGOPTIONS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6AB,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_PdpApi_SetProtConfigOptions 
	**/
	MSG_PDP_SETPROTCONFIGOPTIONS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6AC,	///<Payload type {CAPI2_PdpApi_SetProtConfigOptions_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_PDP_SETPROTCONFIGOPTIONS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6AD,
	 /** 
	api is CAPI2_LcsApi_RrlpSendDataToNetwork 
	**/
	MSG_LCS_RRLP_SEND_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6AE,	///<Payload type {CAPI2_LcsApi_RrlpSendDataToNetwork_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRLP_SEND_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6AF,
	 /** 
	api is CAPI2_LcsApi_RrlpRegisterDataHandler 
	**/
	MSG_LCS_RRLP_REG_HDL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6B0,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRLP_REG_HDL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6B1,
	 /** 
	api is CAPI2_LcsApi_RrcRegisterDataHandler 
	**/
	MSG_LCS_RRC_REG_HDL_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6B2,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_REG_HDL_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6B3,
	 /** 
	api is CAPI2_LcsApi_RrcSendUlDcch 
	**/
	MSG_LCS_RRC_SEND_DL_DCCH_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6B4,	///<Payload type {CAPI2_LcsApi_RrcSendUlDcch_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_SEND_DL_DCCH_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6B5,
	 /** 
	api is CAPI2_LcsApi_RrcMeasCtrlFailure 
	**/
	MSG_LCS_RRC_MEAS_CTRL_FAILURE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6B6,	///<Payload type {CAPI2_LcsApi_RrcMeasCtrlFailure_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_MEAS_CTRL_FAILURE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6B7,
	 /** 
	api is CAPI2_LcsApi_RrcStatus 
	**/
	MSG_LCS_RRC_STAT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6B8,	///<Payload type {CAPI2_LcsApi_RrcStatus_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_RRC_STAT_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6B9,
	 /** 
	api is CAPI2_SimApi_PowerOnOffSim 
	**/
	MSG_SIM_POWER_ON_OFF_SIM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6BA,	///<Payload type {CAPI2_SimApi_PowerOnOffSim_Req_t}
	 /** 
	payload is ::SIMAccess_t 
	**/
	MSG_SIM_POWER_ON_OFF_SIM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6BB,	///<Payload type {::SIMAccess_t}
	 /** 
	api is CAPI2_PhoneCtrlApi_SetPagingStatus 
	**/
	MSG_SYS_SET_PAGING_STATUS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6BC,	///<Payload type {CAPI2_PhoneCtrlApi_SetPagingStatus_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SYS_SET_PAGING_STATUS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6BD,
	 /** 
	api is CAPI2_LcsApi_GetGpsCapabilities 
	**/
	MSG_LCS_GET_GPS_CAP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6BE,
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_GET_GPS_CAP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6BF,	///<Payload type {::Result_t}
	 /** 
	api is CAPI2_LcsApi_SetGpsCapabilities 
	**/
	MSG_LCS_SET_GPS_CAP_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6C0,	///<Payload type {CAPI2_LcsApi_SetGpsCapabilities_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_LCS_SET_GPS_CAP_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6C1,
	 /** 
	api is CAPI2_CcApi_AbortDtmfTone 
	**/
	MSG_CCAPI_ABORTDTMF_TONE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6C2,	///<Payload type {CAPI2_CcApi_AbortDtmfTone_Req_t}
	 /** 
	payload is void 
	**/
	MSG_CCAPI_ABORTDTMF_TONE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6C3,
	 /** 
	api is CAPI2_NetRegApi_SetSupportedRATandBandEx 
	**/
	MSG_MS_SET_RAT_BAND_EX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6C4,	///<Payload type {CAPI2_NetRegApi_SetSupportedRATandBandEx_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SET_RAT_BAND_EX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6C5,
	 /** 
	api is CAPI2_SimApi_ResetSIM 
	**/
	MSG_SIM_RESET_SIM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6C8,	///<Payload type {CAPI2_SimApi_ResetSIM_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SIM_RESET_SIM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6C9,
	 /** 
	api is CAPI2_NetRegApi_SetTZUpdateMode 
	**/
	MSG_TIMEZONE_SET_UPDATE_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6CA,	///<Payload type {CAPI2_NetRegApi_SetTZUpdateMode_Req_t}
	 /** 
	payload is void 
	**/
	MSG_TIMEZONE_SET_UPDATE_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6CB,
	 /** 
	api is CAPI2_NetRegApi_GetTZUpdateMode 
	**/
	MSG_TIMEZONE_GET_UPDATE_MODE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6CC,
	 /** 
	payload is ::TimeZoneUpdateMode_t 
	**/
	MSG_TIMEZONE_GET_UPDATE_MODE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6CD,	///<Payload type {::TimeZoneUpdateMode_t}
	 /** 
	api is CAPI2_SEC_HostToModemInd 
	**/
	MSG_SEC_HOST_TO_MODEM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6CE,	///<Payload type {CAPI2_SEC_HostToModemInd_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SEC_HOST_TO_MODEM_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6CF,
	 /** 
	payload is ::UInt8 
	**/
	MSG_SEC_MODEM_TO_HOST_IND  = MSG_GRP_CAPI2_GEN_0 + 0x6D1,	///<Payload type {::UInt8}
	 /** 
	api is CAPI2_SimApi_GetAdData 
	**/
	MSG_SIM_AD_DATA_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6D6,
	 /** 
	payload is void 
	**/
	MSG_SIM_AD_DATA_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6D7,	///<Payload type {void}
	 /** 
	api is CAPI2_SimApi_GetCurrentSimVoltage 
	**/
	MSG_SIM_GET_CURRENT_SIM_VOLTAGE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6D8,
	 /** 
	payload is ::SimVoltage_t 
	**/
	MSG_SIM_GET_CURRENT_SIM_VOLTAGE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6D9,	///<Payload type {::SimVoltage_t}
	 /** 
	api is CAPI2_MS_SetSupportedRATandBand 
	**/
	MSG_MS_SET_RAT_AND_BAND_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6DA,	///<Payload type {CAPI2_MS_SetSupportedRATandBand_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_MS_SET_RAT_AND_BAND_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6DB,
	 /** 
	api is CAPI2_SecModemApi_ConfigModemReq 
	**/
	MSG_SECMODEM_CONFIG_MODEM_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6DC,
	 /** 
	api is CAPI2_SecModemApi_SetInfoPPBitsReq 
	**/
	MSG_SECMODEM_SET_INFO_PP_BITS_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6F4,	///<Payload type {CAPI2_SecModemApi_SetInfoPPBitsReq_Req_t}
	 /** 
	payload is ::Result_t 
	**/
	MSG_SECMODEM_SET_INFO_PP_BITS_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6F5,
	 /** 
	api is CAPI2_SYSPARM_GetImeiString 
	**/
	MSG_SYSPARM_GET_IMEI_STRING_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6F6,	///<Payload type {CAPI2_SYSPARM_GetImeiString_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARM_GET_IMEI_STRING_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6F7,	///<Payload type {void}
	 /** 
	api is CAPI2_MsDbApi_SetElementEx 
	**/
	MSG_MS_SET_ELEMENT_EX_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6FC,	///<Payload type {CAPI2_MsDbApi_SetElementEx_Req_t}
	 /** 
	payload is void 
	**/
	MSG_MS_SET_ELEMENT_EX_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x6FD,
	 /** 
	api is CAPI2_SmsApi_WriteSMSPduToSIMSlotReq 
	**/
	MSG_SMS_WRITESMSPDUTOSIMSLOT_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x6FE,	///<Payload type {CAPI2_SmsApi_WriteSMSPduToSIMSlotReq_Req_t}
	 /** 
	api is CAPI2_SYSPARM_SET_TxPhoneState 
	**/
	MSG_SYSPARM_SET_TX_PHONE_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x700,	///<Payload type {CAPI2_SYSPARM_SET_TxPhoneState_Req_t}
	 /** 
	payload is void 
	**/
	MSG_SYSPARM_SET_TX_PHONE_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x701,
	 /** 
	api is CAPI2_SYSPARM_GET_TxPhoneState 
	**/
	MSG_SYSPARM_GET_TX_PHONE_STATE_REQ  = MSG_GRP_CAPI2_GEN_0 + 0x702,
	 /** 
	payload is ::UInt16 
	**/
	MSG_SYSPARM_GET_TX_PHONE_STATE_RSP  = MSG_GRP_CAPI2_GEN_0 + 0x703,	///<Payload type {::UInt16}

	//MSG_GEN_REQ_END = 0x48FF

/*********  _CAPI2_CODE_GEN_END_  ************/

	MSG_GEN_REQ_END = 0x48FF,

	MSG_CAPI2_MSGIDS_END = 0x4FFF,
/*TASKMSGS_INCLUDE taskmsgs_sysrpc.i*/


/*********  _SYSRPC_CODE_GEN_BEGIN_ (Do not add code until after _SYSRPC_CODE_GEN_END_ ) ************/
#ifdef DEFINE_SYS_GEN_MIDS

	//MSG_GEN_REQ_START = 19200,

	 /** 
	api is CAPI2_PMU_BattADCReq 
	**/
	MSG_PMU_BATT_LEVEL_REQ  = 0x4B00,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_BATT_LEVEL_RSP  = 0x4B01,
	 /** 
	api is CAPI2_CPPS_Control 
	**/
	MSG_CPPS_CONTROL_REQ  = 0x4B02,	///<Payload type {::CAPI2_CPPS_Control_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_CPPS_CONTROL_RSP  = 0x4B03,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_CP2AP_PedestalMode_Control 
	**/
	MSG_CP2AP_PEDESTALMODE_CONTROL_REQ  = 0x4B04,	///<Payload type {::CAPI2_CP2AP_PedestalMode_Control_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_CP2AP_PEDESTALMODE_CONTROL_RSP  = 0x4B05,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PMU_IsSIMReady 
	**/
	MSG_PMU_IS_SIM_READY_REQ  = 0x4B06,	///<Payload type {::CAPI2_PMU_IsSIMReady_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_PMU_IS_SIM_READY_RSP  = 0x4B07,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_PMU_ActivateSIM 
	**/
	MSG_PMU_ACTIVATE_SIM_REQ  = 0x4B08,	///<Payload type {::CAPI2_PMU_ActivateSIM_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_ACTIVATE_SIM_RSP  = 0x4B09,
	 /** 
	api is CAPI2_PMU_ClientPowerDown 
	**/
	MSG_PMU_ClientPowerDown_REQ  = 0x4B0A,
	 /** 
	payload is ::default_proc 
	**/
	MSG_PMU_ClientPowerDown_RSP  = 0x4B0B,
	 /** 
	api is CAPI2_FLASH_SaveImage 
	**/
	MSG_FLASH_SAVEIMAGE_REQ  = 0x4B0C,	///<Payload type {::CAPI2_FLASH_SaveImage_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_FLASH_SAVEIMAGE_RSP  = 0x4B0D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_USB_IpcMsg 
	**/
	MSG_USB_IPC_REQ  = 0x4B0E,	///<Payload type {::CAPI2_USB_IpcMsg_Req_t}
	 /** 
	payload is ::USBPayload_t 
	**/
	MSG_USB_IPC_RSP  = 0x4B0F,	///<Payload type {::USBPayload_t}
	 /** 
	api is SYS_Sync_SetRegisteredEventMask 
	**/
	MSG_SYS_SYNC_SET_REG_EVENT_MASK_REQ  = 0x4B12,	///<Payload type {::SYS_Sync_SetRegisteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_SET_REG_EVENT_MASK_RSP  = 0x4B13,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_SetFilteredEventMask 
	**/
	MSG_SYS_SYNC_SET_REG_FILTER_MASK_REQ  = 0x4B14,	///<Payload type {::SYS_Sync_SetFilteredEventMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_SET_REG_FILTER_MASK_RSP  = 0x4B15,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_RegisterForMSEvent 
	**/
	MSG_SYS_SYNC_REG_EVENT_REQ  = 0x4B16,	///<Payload type {::SYS_Sync_RegisterForMSEvent_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_REG_EVENT_RSP  = 0x4B17,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_DeRegisterForMSEvent 
	**/
	MSG_SYS_SYNC_DEREG_EVENT_REQ  = 0x4B18,
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_DEREG_EVENT_RSP  = 0x4B19,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_EnableFilterMask 
	**/
	MSG_SYS_SYNC_ENABLE_FILTER_REQ  = 0x4B1A,	///<Payload type {::SYS_Sync_EnableFilterMask_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_ENABLE_FILTER_RSP  = 0x4B1B,	///<Payload type {::Boolean}
	 /** 
	api is SYS_Sync_RegisterSetClientName 
	**/
	MSG_SYS_SYNC_REGISTER_NAME_REQ  = 0x4B1C,	///<Payload type {::SYS_Sync_RegisterSetClientName_Req_t}
	 /** 
	payload is ::Boolean 
	**/
	MSG_SYS_SYNC_REGISTER_NAME_RSP  = 0x4B1D,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_BattLevelPercent 
	**/
	MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_REQ  = 0x4B1E,
	 /** 
	payload is ::UInt16 
	**/
	MSG_HAL_EM_BATTMGR_BATT_LEVEL_PERCENT_RSP  = 0x4B1F,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_BattLevel 
	**/
	MSG_HAL_EM_BATTMGR_BATT_LEVEL_REQ  = 0x4B20,
	 /** 
	payload is ::UInt16 
	**/
	MSG_HAL_EM_BATTMGR_BATT_LEVEL_RSP  = 0x4B21,	///<Payload type {::UInt16}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_USB_ChargerPresent 
	**/
	MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_REQ  = 0x4B22,
	 /** 
	payload is ::Boolean 
	**/
	MSG_HAL_EM_BATTMGR_USB_CHARGER_PRESENT_RSP  = 0x4B23,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_WALL_ChargerPresent 
	**/
	MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_REQ  = 0x4B24,
	 /** 
	payload is ::Boolean 
	**/
	MSG_HAL_EM_BATTMGR_WALL_CHARGER_PRESENT_RSP  = 0x4B25,	///<Payload type {::Boolean}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_Run_BattMgr 
	**/
	MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_REQ  = 0x4B26,	///<Payload type {::CAPI2_HAL_EM_BATTMGR_Run_BattMgr_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
	MSG_HAL_EM_BATTMGR_RUN_BATT_MGR_RSP  = 0x4B27,	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_Config_BattMgr 
	**/
	MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_REQ  = 0x4B28,	///<Payload type {::CAPI2_HAL_EM_BATTMGR_Config_BattMgr_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
	MSG_HAL_EM_BATTMGR_CONFIG_BATT_MGR_RSP  = 0x4B29,	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_GetBattTemp 
	**/
	MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_REQ  = 0x4B2A,
	 /** 
	payload is ::HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t 
	**/
	MSG_HAL_EM_BATTMGR_GET_BATT_TEMP_RSP  = 0x4B2B,	///<Payload type {::HAL_EM_BATTMGR_Action_GetBatteryTemp_st_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_GetChargingStatus 
	**/
	MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_REQ  = 0x4B2C,
	 /** 
	payload is ::EM_BATTMGR_ChargingStatus_en_t 
	**/
	MSG_HAL_EM_BATTMGR_GET_CHARGE_STATUS_RSP  = 0x4B2D,	///<Payload type {::EM_BATTMGR_ChargingStatus_en_t}
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_SetComp 
	**/
	MSG_HAL_EM_BATTMGR_SET_COMP_REQ  = 0x4B2E,	///<Payload type {::CAPI2_HAL_EM_BATTMGR_SetComp_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
	MSG_HAL_EM_BATTMGR_SET_COMP_RSP  = 0x4B2F,	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	api is CAPI2_SYSRPC_MEASMGR_GetDataB_Adc 
	**/
	MSG_MEASMGR_GET_ADC_REQ  = 0x4B30,	///<Payload type {::CAPI2_SYSRPC_MEASMGR_GetDataB_Adc_Req_t}
	 /** 
	payload is ::MeasMngrCnfgRsp_t 
	**/
	MSG_MEASMGR_GET_ADC_RSP  = 0x4B31,	///<Payload type {::MeasMngrCnfgRsp_t}
	 /** 
	api is CAPI2_SYSRPC_HAL_ADC_Ctrl 
	**/
	MSG_HAL_ADC_CTRL_REQ  = 0x4B32,	///<Payload type {::CAPI2_SYSRPC_HAL_ADC_Ctrl_Req_t}
	 /** 
	payload is ::HAL_ADC_ReadConfig_st_t 
	**/
	MSG_HAL_ADC_CTRL_RSP  = 0x4B33,	///<Payload type {::HAL_ADC_ReadConfig_st_t}
	 /** 
	api is SYS_Log_ApEnableCpRange 
	**/
	MSG_LOG_ENABLE_RANGE_REQ  = 0x4B34,	///<Payload type {::SYS_Log_ApEnableCpRange_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_LOG_ENABLE_RANGE_RSP  = 0x4B35,
	 /** 
	api is CAPI2_HAL_EM_BATTMGR_Notification 
	**/
	MSG_HAL_EM_BATTMGR_NOTIFICATION_REQ  = 0x4B36,	///<Payload type {::CAPI2_HAL_EM_BATTMGR_Notification_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_HAL_EM_BATTMGR_NOTIFICATION_RSP  = 0x4B37,
	 /** 
	api is CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB 
	**/
	MSG_HAL_EM_BATTMGR_REGISTER_EVENT_REQ  = 0x4B38,	///<Payload type {::CAPI2_SYS_HAL_EM_BATTMGR_RegisterEventCB_Req_t}
	 /** 
	payload is ::HAL_EM_BATTMGR_ErrorCode_en_t 
	**/
	MSG_HAL_EM_BATTMGR_REGISTER_EVENT_RSP  = 0x4B39,	///<Payload type {::HAL_EM_BATTMGR_ErrorCode_en_t}
	 /** 
	api is SYS_AT_MTEST_Handler 
	**/
	MSG_AT_MTEST_HANDLER_REQ  = 0x4B3A,	///<Payload type {::SYS_AT_MTEST_Handler_Req_t}
	 /** 
	payload is ::MtestOutput_t 
	**/
	MSG_AT_MTEST_HANDLER_RSP  = 0x4B3B,	///<Payload type {::MtestOutput_t}
	 /** 
	api is SYS_SimLockApi_GetStatus 
	**/
	MSG_SYS_SIMLOCK_GET_STATUS_REQ  = 0x4B3C,	///<Payload type {::SYS_SimLockApi_GetStatus_Req_t}
	 /** 
	payload is ::SYS_SIMLOCK_STATE_t 
	**/
	MSG_SYS_SIMLOCK_GET_STATUS_RSP  = 0x4B3D,	///<Payload type {::SYS_SIMLOCK_STATE_t}
	 /** 
	api is SYS_SIMLOCKApi_SetStatusEx 
	**/
	MSG_SYS_SIMLOCK_SET_STATUS_REQ  = 0x4B3E,	///<Payload type {::SYS_SIMLOCKApi_SetStatusEx_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SIMLOCK_SET_STATUS_RSP  = 0x4B3F,
	 /** 
	api is SYS_SimApi_GetCurrLockedSimlockTypeEx 
	**/
	MSG_SYS_GET_CUR_SIMLOCK_TYPE_REQ  = 0x4B40,	///<Payload type {::SYS_SimApi_GetCurrLockedSimlockTypeEx_Req_t}
	 /** 
	payload is ::UInt32 
	**/
	MSG_SYS_GET_CUR_SIMLOCK_TYPE_RSP  = 0x4B41,	///<Payload type {::UInt32}
	 /** 
	api is CAPI2_SYS_SoftResetSystem 
	**/
	MSG_SYS_SOFT_RESET_SYSTEM_REQ  = 0x4B42,	///<Payload type {::CAPI2_SYS_SoftResetSystem_Req_t}
	 /** 
	payload is ::default_proc 
	**/
	MSG_SYS_SOFT_RESET_SYSTEM_RSP  = 0x4B43,
	 /**
	api is CAPI2_FLASH_SaveData
	**/
	/* <Payload type {::CAPI2_FLASH_SaveData_Req_t} */
	MSG_FLASH_SAVEDATA_REQ  = 0x4B44,
	 /**
	payload is ::SpmlData_rsp_t
	**/
	/* <Payload type {::SpmlData_rsp_t} */
	MSG_FLASH_SAVEDATA_RSP  = 0x4B45,
	 /**
	api is SYS_CPSystemCmd
	**/
	/* <Payload type {::SYS_CPSystemCmd_Req_t} */
	MSG_CP_SYS_CMD_REQ  = 0x4B46,
	 /**
	payload is ::default_proc
	**/
	MSG_CP_SYS_CMD_RSP  = 0x4B47,
	 /**
	api is SYS_SimApi_ColdResetEvt
	**/
	/* <Payload type {::SYS_SimApi_ColdResetEvt_Req_t} */
	MSG_SYS_SIM_COLD_RESET_EVT_REQ  = 0x4B48,
	 /**
	payload is ::default_proc
	**/
	MSG_SYS_SIM_COLD_RESET_EVT_RSP  = 0x4B49,
	 /**
	api is CAPI2_FLASH_ReadImage
	**/
	/* <Payload type {::CAPI2_FLASH_ReadImage_Req_t} */
	MSG_FLASH_READIMAGE_REQ  = 0x4B4A,
	 /**
	payload is ::Boolean
	**/
	/* <Payload type {::Boolean} */
	MSG_FLASH_READIMAGE_RSP  = 0x4B4B,

	/**
	api is SYS_APSystemCmd
	**/
	/* Payload type {::SYS_APSystemCmd_Req_t} */
	MSG_AP_SYS_CMD_REQ  = 0x4B4C,
	 /**
	payload is ::default_proc
	**/
	MSG_AP_SYS_CMD_RSP  = 0x4B4D,

	//MSG_GEN_REQ_END = 0x4BFF

#endif //DEFINE_SYS_GEN_MIDS
/*********  _SYSRPC_CODE_GEN_END_  ************/
/*TASKMSGS_INCLUDE taskmsgs_rtc.i*/
	//---------------------------------------------------------------
	// MSG_GRP_RTC, MESSAGE GROUP FOR RTC (0x5000)
	//---------------------------------------------------------------
	MSG_RTC_CAL_REQ						= MSG_GRP_RTC+0x00,
	MSG_RTC_CAL_IND						= MSG_GRP_RTC+0x01,
	MSG_RTC_CAL_RSP						= MSG_GRP_RTC+0x02,
	// End of MSG_GRP_RTC, (0x5000)
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

void VccApi_ResetDataVmf(ClientInfo_t* pClientInfo);


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
#define VccApi_ResetDataVmf(p)


#endif


/** @endcond */

#endif  // _TASKMSGS_H_


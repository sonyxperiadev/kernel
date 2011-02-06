/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement 
governing use of this software, this software is licensed to you under the 
terms of the GNU General Public License version 2, available at 
http://www.gnu.org/copyleft/gpl.html (the "GPL"). 

Notwithstanding the above, under no circumstances may you combine this software 
in any way with any other Broadcom software provided under a license other than 
the GPL, without Broadcom's express prior written consent.
*******************************************************************************************/


/**
*
*   @file   resultcode.h
*
*   @brief  This file defines the return result code for APIs.
*
****************************************************************************/

#ifndef _RESULTCODE
#define _RESULTCODE


#define SS_RESULT_START				50
#define PDP_RESULT_START			70
#define STK_RESULT_START			110
#define SMS_RESULT_STRRT			120
#define VM_RESULT_START				160
#define AUDIO_RESULT_START			170
#define DATA_RESULT_START			180
#define CAM_RESULT_START			190
#define TOOL_RESULT_START			200
#define TOOL_SECBOOT_RESULT_START	230
#define TOOL_FLASHDRV_RESULT_START	250
#define CSD_RESULT_START			260

#if defined(_TCH_DRV_TSC200X_)
#define TCH_RESULT_START			320
#endif

#define CC_RESULT_START				350		

/** @{ */

/** 
	Result Codes
**/

typedef enum {

	RESULT_OK = 0,							///< 0	Result OK, operation successful
	RESULT_ERROR,							///< 1	Result Error
	RESULT_DONE,							///< 2	Result Done,operation completed
	RESULT_UNKNOWN,							///< 3	Result Unknown
	RESULT_WRONG_INDEX,						///< 4	Wrong Request Index
	RESULT_WRONG_STATE,						///< 5	Request is sent when, the relative modul/task is in wrong state
	RESULT_BUSY_TRY_LATER,					///< 6	The particular modul/task is in the middle of processing a procedure
	RESULT_PROCEDURE_ABORTED,				///< 7	The procedure is aborted
	RESULT_LOW_MEMORY,						///< 8	Low memory
	RESULT_PENDING,							///< 9	Result is Pending ( Used by CAPI2 API's )
	RESULT_INVALID_ARGUMENT,				///< 10 One or more than one argument to the function is invalid
	RESULT_OPERATION_NOT_ALLOWED,			///< 11 Operation not allowed
	RESULT_TIMER_EXPIRED,					///< 12 Timer expired

	//------ Result code for Supplementary-service APIs -----
	SS_NETWORK_NOT_AVAILABLE = SS_RESULT_START,	///< 50	Network Unavailable at this time
	SS_OPERATION_IN_PROGRESS,					///< 51	Requested SS operation in progress
	SS_INVALID_USSD_ID,							///< 52	Invalid USSD ID
	SS_INVALID_USSD_STRING,						///< 53	Invalid USSD String. 
	SS_INVALID_USSD_DCS,						///< 54	Invalid USSD DCS. 
	SS_INVALID_SS_REQUEST,						///< 55	Invalid request
	SS_FDN_BLOCK_SS_REQUEST,					///< 56	SS dialing string not in FDN phonebook and FDN is enabled
	SS_INVALID_PASSWORD_LENGTH,					///< 57	Invalid password length. Lenght of the password shall be 4

	//------ Result code for Networks APIs -----
	PCH_OPERATION_NOT_ALLOWED = PDP_RESULT_START,	///< 70		PCH operation not allowed
	PCH_INVALID_VALUE,								///< 71		Invalid PCH value
	PCH_ATTACH_TEMP_NOT_ALLOWED,					///< 72		Temporary attach not allowed

	PDP_LLC_OR_SNDCP_FAILURE,				///< 73		LLC / SNDCP failure.SM cause 25
	PDP_INSUFFICIENT_RESOURCES,				///< 74		Insufficient resources.SM cause 26
	PDP_MISSING_OR_UNKNOWN_APN,				///< 75		Missing or unknown access point name.SM cause 27
	PDP_UNKNOWN_PDP_ADDRESS,				///< 76		Unknown PDP address.SM cause 28
	PDP_USER_AUTH_FAILED,					///< 77		User authentication failed.SM cause 29
	PDP_ACTIVATION_REJECTED_BY_GGSN,		///< 78		Activation rejected by GGSN.SM cause 30
	PDP_ACTIVATION_REJECTED_UNSPECIFIED,	///< 79		Activation rejected, unspecified cause.SM cause 31
	PDP_SERVICE_OPT_NOT_SUPPORTED,			///< 80		PDP service option not supported.SM cause 32
	PDP_REQ_SERVICE_NOT_SUBSCRIBED,			///< 81		Service not subscribed.SM cause 33	
	PDP_SERVICE_TEMP_OUT_OF_ORDER,			///< 82		Service temporarily out of order.SM cause 34
	PDP_NSAPI_ALREADY_USED,					///< 83		network SAPI already used.SM cause 35
	PDP_REGULAR_DEACTIVATION,				///< 84		Regular deactivation
	PDP_QOS_NOT_ACCEPTED,					///< 85		Quality of service not accepted.SM cause 37
	PDP_NETWORK_FAILURE,					///< 86		Network failure.SM cause 38
	PDP_REACTIVATION_REQUIRED,				///< 87		PDP reactivation required.SM cause 39
	PDP_FEATURE_NOT_SUPPORTED,				///< 88		Feature not supported.SM cause 40
	PDP_SEMANTIC_ERROR_IN_TFT,				///< 89		Semantic error in the TFT operation.SM cause 41
	PDP_SYNTACTICAL_ERROR_IN_TFT,			///< 90		Syntactical error in the TFT operation.SM cause 42
	PDP_UNKNOWN_PDP_CONTEXT,				///< 91		Unknown PDP context.SM cause 43
	PDP_SEMANTIC_ERROR_IN_PKT_FILTER,		///< 92		Semantic errors in packet filter(s).SM cause 44
	PDP_SYNTACTICAL_ERROR_IN_PKT_FILTER,	///< 93		Syntactical errors in packet filter(s).SM cause 45
	PDP_CONTEXT_WITHOUT_TFT,				///< 94		PDP context without TFT already activated.SM cause 46
	PDP_INVALID_TI,							///< 95		Invalid TI.SM cause 81
	PDP_SEMANT_INCORRECT_MSG,				///< 96		Semantically incorrect message.SM cause 95
	PDP_INV_MANDATORY_IE,					///< 97		Invalid mandatory information.SM cause 96
	PDP_MSG_TYPE_NOT_EXISTENT,				///< 98		Message type non-existent or not implemented.SM cause 97
	PDP_MSG_TYPE_NOT_COMPATIBLE,			///< 99		Message type not compatible with protocol state.SM cause 98
	PDP_IE_NON_EXISTENT,					///< 100	IE non-existent or not implemented.SM cause 99
	PDP_CONDITIONAL_IE_ERROR,				///< 101	Conditional IE error.SM cause 100
	PDP_MSG_NOT_COMPATIBLE,					///< 102	Message not compatible with protocol state.SM cause 101
	PDP_PROTOCOL_ERROR_UNSPECIFIED,			///< 103	Unspecified protocol error.SM cause 111

	PDP_ACTIVATION_BLOCKED_BY_SIM,			///< 104	PDP activation is blocked by either: APN Control List (ACL) in 3G USIM or FDN settings in 2G SIM. 
	PDP_CIRCUIT_CALL_ACTIVE,				///< 105	There is an active voice call
	PDP_OPERATOR_DETERMINED_BARRING,		/// < 106 
	PDP_APN_INCOMPATIBLE_W_ACTIVE_PDP,		/// < 107
	RESULT_PLMN_SELECT_OK,					/// <  108 MS_PlmnSelect succeed ( No network request sent )

	//------ Result code for STK Data Service -----
	STK_DATASVRC_BUSY = STK_RESULT_START,			///< 110	STK data service is busy and need to wait for it finish
	RESULT_SIM_NOT_READY,							///< 111	SIM is not ready

	//------ Result code for SMS -----
	SMS_SIM_SMS_READY = SMS_RESULT_STRRT,			///< 120	SIM is ready for SMS access	
	SMS_SIM_BUSY,						///< 121		SIM is busy, need to wait.								
	SMS_SIM_NOT_INSERT,					///< 122		SIM not inserted or seated correctly.	
	SMS_ME_SMS_READY,					///< 123		ME ready for SMS access	
	SMS_ME_SMS_NOT_READY,				///< 124	ME not ready for SMS access	
	SMS_ME_SMS_NOT_SUPPORTED,			///< 125	SMS is not supported by the ME.	
	SMS_NETWORK_CS_ONLY,				///< 126	Only Circuit Switched access allowed by the network for SMS	
	SMS_NETWORK_GPRS_ONLY,				///< 127	Only CGPRS access allowed by the network for SMS		

	SMS_OPERATION_NOT_ALLOWED,			///< 128	SMS operation is not allowed	
	SMS_NO_SERVICE,						///< 129	SMS service currently not available	
	SMS_ADDR_NUMBER_STR_TOO_LONG,		///< 130	Address string is too long	
	SMS_SCA_INVALID_CHAR_INSTR,			///< 131	Invalid Service Center Address Character	
	SMS_INVALID_SCA_ADDRESS,			///< 132	Invalid Service Center Address	
	SMS_SCA_NOT_SUPPORTED_IN_SIM,		///< 133	Service Center Address not supported in this SIM	
	SMS_INVALID_INDEX,					///< 134	Invalid index	
	SMS_INVALID_PDU_LENGTH,				///< 135	PDU length is not valid.	
	SMS_INVALID_PDU_MODE_PARM,			///< 136	PDU Mode parameter is not valid. 	
	SMS_INVALID_TEXT_LENGTH,			///< 137	Text message length is invalid.	
	SMS_INVALID_SMS_PARAM,				///< 138	SMS parameter is not valid.	
	SMS_NO_MSG_TO_LIST,					///< 139	No message to list.	
	SMS_MSG_DECODING_ERROR,				///< 140	Message decoding error.	
	SMS_INVALID_DIALSTR_CHAR,			///< 141	Dial string character is not valid.	

	SMS_IS_CONCATENATED_SMS,			///< 142	This is a concatenated SMS		
	SMS_IS_NOT_CONCATENATED_SMS,		///< 143	This is not a concatenated SMS
	SMS_INVALID_UDH_DATA,				///< 144	Invalid UDH data

	SMS_CB_MIDS_EXCEED_RANGE_LIMIT,		///< 145	Message ID has exceeded range (see GSm 3.41)
	SMS_CB_MIDS_ALREADY_EXIST,			///< 146	This message ID already exists. 
	SMS_CB_MIDS_DOES_NOT_EXIST,			///< 147	Message ID does not exist.

	SMS_SCA_NULL_STRING,				///< 148	Service Center Address is NULL
	SMS_FDN_NOT_ALLOWED,				///< 149	Fixed Dialing not allowed

	SMS_CALL_CONTROL_BARRING,			///< 150	SMS Call control Barring

	VM_NOT_AVAILABLE = VM_RESULT_START,						///< 160	Voice Mail Not available

	// audio	
	AUDIO_INVALID_AUDIO_CHNL = AUDIO_RESULT_START,			///< 170	Invalid Audio Channel	
	AUDIO_INVALID_VOLUME_GAIN,								///< 171	Invalid Volume Gain setting
	AUDIO_INVALID_TONE_ID,									///< 172	Invalid Tone ID
	
	//------ Result code for User Data Account -----
	DATA_INVALID_ACCTID = DATA_RESULT_START,				///< 180	Invalid Account ID
	DATA_ACCTID_IN_USED,									///< 181	Account ID already in use. Try another
	DATA_GPRS_NO_CID_AVAILABLE,								///< 182	Context ID for GPRS not available
	DATA_GPRS_INVALID_TFT,									/// 183 invalid TFT
	DATA_GPRS_NO_PRI_CID,									///184 primay cid can not be found

	//------ Result code for Camera Functions
	CAM_SUCCESS = CAM_RESULT_START,							///< 190	Camera function success
	CAM_UNSUPPORTED,										///< 191	Camera not supported
	CAM_FAILED,												///< 192	Camera operation failed
	CAM_DMA_FAILED,											///< 193	Camera Dma operation failed

	//------ Result code for Tools
	RESULT_TOOL_INVALID_ARG = TOOL_RESULT_START,	///< 200	Invalid argument
	RESULT_TOOL_FILE_OPEN_ERROR,					///< 201	File open error
	RESULT_TOOL_FILE_CREATE_ERROR,					///< 202	File create error
	RESULT_TOOL_FILE_READ_ERROR,					///< 203	File read error
	RESULT_TOOL_FILE_WRITE_ERROR,					///< 204	File write error
	RESULT_TOOL_INTERNAL_ERROR,						///< 205	Internal error
	RESULT_TOOL_INCFILE_ERROR,						///< 206	Include file error
	RESULT_TOOL_INVALID_KEYWORD,					///< 207	Invalid keyword
	RESULT_TOOL_WRONG_NUM_OF_PARMS,					///< 208	Wrong number of parmeter
	RESULT_TOOL_INVALID_VALUE,						///< 209	Invalid value
	RESULT_TOOL_COMPORT_ERROR,						///< 210	COM port error
	RESULT_TOOL_SYNC_ERROR,							///< 211	Sync error
	RESULT_TOOL_TIMEOUT,							///< 212	Timeout
	RESULT_TOOL_ADDRESS_INVALID,					///< 213	Timeout
	RESULT_TOOL_INVALID_IMEI,						///< 214	Invalid IMEI
	RESULT_TOOL_INVALID_MCC_MNC,					///< 215	Invalid MCC/MNC
	RESULT_TOOL_WRONG_ORDER_SIMLOCK_CODES,			///< 216	Wrong order simlock_codes
	RESULT_TOOL_INVALID_IMSI,						///< 217	Invalid IMSI
	RESULT_TOOL_SYSPARM_WARNING,					///< 218	Sysparm warning
	RESULT_TOOL_SYSPARM_ERROR,						///< 219	Sysparm error
	RESULT_TOOL_CANCELED,							///< 220	Canceled

	//------ secure bootloader tools
	RESULT_TOOL_SECBOOT_CMD_INVALID = TOOL_SECBOOT_RESULT_START,			///< 230	Secure boot command invalid
	RESULT_TOOL_SECBOOT_FORMAT_INVALID,				///< 231	Secure boot format invalid
	RESULT_TOOL_SECBOOT_BAUDRATE_INVALID,			///< 232	Secure boot baudrate invalid
	RESULT_TOOL_SECBOOT_CRC_ERROR,					///< 233	Secure boot CRC error	
	RESULT_TOOL_SECBOOT_MEMALIGN_ERROR,				///< 234	Secure boot memory align error
	RESULT_TOOL_SECBOOT_FLASHPRGM_ERROR,			///< 235	Secure boot flash program error
	RESULT_TOOL_SECBOOT_OTHER_ERROR,				///< 236	Secure boot other error
	RESULT_TOOL_SECBOOT_PROTOCOL_ERROR,				///< 237	Secure boot protocol error
	RESULT_TOOL_SECBOOT_BUFSIZE_ERROR,				///< 238	Secure boot bufsize error
	RESULT_TOOL_SECBOOT_FILEOPEN_ERROR,				///< 239	Secure boot file open error

	//------ flash driver errors
	RESULT_TOOL_FLASHDRV_TIMEOUT = TOOL_FLASHDRV_RESULT_START,	///< 250	Flash drive timeout
	RESULT_TOOL_FLASHDRV_MEMALIGN_ERROR,			///< 251	Flash drive memory align error
	RESULT_TOOL_FLASHDRV_ADDRESS_ERROR,				///< 252	Flash drive address error
	RESULT_TOOL_FLASHDRV_DEVICE_UNKNOWN,			///< 253	Flash drive device unknow

	CSD_CAUSE_VOID_CAUSE = CSD_RESULT_START,		///< 260	CSD cause void
	CSD_CAUSE_UNASSIGNED_NUMBER,					///< 261	CSD cause unassigned number 
	CSD_CAUSE_NO_ROUTE,								///< 262	CSD cause no route
	CSD_CAUSE_CHANNEL_UNACCEPTABLE,					///< 263	CSD cause unacceptable
	CSD_CAUSE_OPERATOR_BARRING,						///< 264	CSD cause operator barring
	CSD_CAUSE_NORMAL_CALL_CLEARING,					///< 265	CSD cause normal call clearing
	CSD_CAUSE_USER_BUSY,							///< 266	CSD cause user busy
	CSD_CAUSE_NO_USER_RESPONDING,					///< 267	CSD cause no user responding
	CSD_CAUSE_USER_ALERTING_NO_ANSWR,				///< 268	CSD cause user alerting no answer
	CSD_CAUSE_MN_CALL_REJECTED,						///< 269	CSD cause MN call rejected
	CSD_CAUSE_NUMBER_CHANGED,						///< 270	CSD cause number changed 
	CSD_CAUSE_NON_SELECT_USER_CLR,					///< 271	CSD cause non select user clr
	CSD_CAUSE_DEST_OUT_OF_ORDER,					///< 272	CSD cause dest out of order
	CSD_CAUSE_INVALID_NUMBER_FORMAT,				///< 273	CSD cause invalid number format
	CSD_CAUSE_FACILITY_REJECTED,					///< 274	CSD cause facility rejected
	CSD_CAUSE_RESPONSE_TO_STATUS_ENQ,				///< 275	CSD cause response to status enq
	CSD_CAUSE_NORMAL_UNSPECIFIED,					///< 276	CSD cause nomal unspecified
	CSD_CAUSE_NO_CIRCUIT_AVAILABLE,					///< 277	CSD cause no circuit available
	CSD_CAUSE_NETWORK_OUT_OF_ORDER,					///< 278	CSD cause network out of order
	CSD_CAUSE_TEMPORARY_FAILURE,					///< 279	CSD cause temporary out of order
	CSD_CAUSE_SWITCH_CONGESTION,					///< 280	CSD ause switch congestion
	CSD_CAUSE_ACCESS_INFO_DISCARDED,				///< 281	CSD cause access info discarded
	CSD_CAUSE_REQUESTED_CIRCUIT_NOT_AVAILABLE,		///< 282	CSD cause requested circuit not available 
	CSD_CAUSE_RESOURCES_UNAVAILABLE,				///< 283	CSD cause resources unavailable
	CSD_CAUSE_QUALITY_UNAVAILABLE,					///< 284	CSD cause quality unavailable
	CSD_CAUSE_FACILITY_NOT_SUBSCRIBED,				///< 285	CSD cause facility not subscribed
	CSD_CAUSE_INCOMING_CALLS_BARRED_IN_CUG,			///< 286	CSD cause incoming calls barred in CUG
	CSD_CAUSE_BEARER_CAPABILITY_NOT_ALLOWED,		///< 287	CSD cause bearer capability not allowed
	CSD_CAUSE_BEARER_CAPABILITY_NOT_AVAILABLE,		///< 288	CSD cause bearer capability not available
	CSD_CAUSE_SERVICE_NOT_AVAILABLE,				///< 289	CSD cause service not available
	CSD_CAUSE_BEARER_SERVICE_NOT_IMPLEMENTED,		///< 290	CSD cause service not implemented
	CSD_CAUSE_ACM_GREATER_OR_EQUAL_TO_ACMMAX,		///< 291	CSD cause ACM >= ACMMAX
	CSD_CAUSE_FACILITY_NOT_IMPLEMENTED,				///< 292	CSD cause facility not implemented
	CSD_CAUSE_ONLY_RESTRICTED_DIGITAL,				///< 293	CSD cause only restriced digital
	CSD_CAUSE_SERVICE_NOT_IMPLEMENTED,				///< 294	CSD cause service not implemented
	CSD_CAUSE_INVALID_TI,							///< 295	CSD cause invalid TI
	CSD_CAUSE_USER_NOT_IN_CUG,						///< 296	CSD cause user not in CUG
	CSD_CAUSE_INCOMPATIBLE_DESTINATION,				///< 297	CSD cause incomplatible destination
	CSD_CAUSE_INVALID_TRANSIT_NETWORK,				///< 298	CSD cause invalid transit network
	CSD_CAUSE_SEMATICS_INCORRECT,					///< 299	CSD cause sematics incorrect
	CSD_CAUSE_INVALID_MANATORY_INFORMATION,			///< 300	CSD cause invalid mandatory information
	CSD_CAUSE_MESG_TYPE_NON_EXISTENT,				///< 301	CSD cause message type non exist
	CSD_CAUSE_MESG_TYPE_NOT_COMPATIBLE_WITH_STATE,	///< 302	CSD cause message type nto compatible with state
	CSD_CAUSE_IE_NON_EXISTENT,						///< 303	CSD cause IE non existent
	CSD_CAUSE_CONDITIONAL_IE_ERROR,					///< 304	CSD cause conditional IE error
	CSD_CAUSE_MESG_NOT_COMPATIBLE_WITH_STATE,		///< 305	CSD cause message not compatible with state	
	CSD_CAUSE_RECOVERY_ON_TIMER_EXPIRY,				///< 306	CSD cause recovery on timer expiry
	CSD_CAUSE_PROTOCOL_ERROR,						///< 307	CSD cause protocol error
	CSD_CAUSE_INTERWORKING,							///< 308	CSD cause interworking
	CSD_CAUSE_RADIO_LINK_FAILURE_APPEARED,			///< 309	CSD cause radio link failure appeared
	CSD_CAUSE_REESTABLISHMENT_SUCCESSFUL,			///< 310	CSD cause reestablishment successful
#if defined(_TCH_DRV_TSC200X_)
	TCH_SUCCESS = 	TCH_RESULT_START,				///< 320	Touch screen function success
    TCH_UNSUPPORTED,								///< 321	Touch screen not supported
	TCH_FAILED,
#endif


	//------ Result code for Call-Control APIs -----
	CC_FAIL_CALL_SESSION = CC_RESULT_START,	///< 350	Call Session Fail, fails after call has been set up
	CC_FAIL_MAKE_CALL,						///< 351	Make Call Fail
	CC_FDN_BLOCK_MAKE_CALL,					///< 352	Call request is blocked by FDN settings
	CC_MAKE_CALL_SUCCESS,					///< 353	Make Call Success	
	CC_DISALLOW_AUXILIARY_CALL,				///< 354	Auxillary Call Not Allowed
	CC_WRONG_CALL_TYPE,						///< 355	Wrong Call type
	CC_WRONG_CALL_INDEX,					///< 356	Wrong Call Index
	CC_END_CALL_SUCCESS,					///< 357	End Call Success
	CC_END_CALL_FAIL,						///< 358	End Call Fail
	CC_HANGUP_CALL_SUCCESS,					///< 359	Call Hangup Success
	CC_HANGUP_CALL_FAIL,					///< 360	Call Hangup Fail
	CC_ACCEPT_CALL_SUCCESS,					///< 361	Call Accept Success
	CC_ACCEPT_CALL_FAIL,					///< 362	Call Accept Fail
	CC_HOLD_CALL_SUCCESS,					///< 363	Call Hold Success
	CC_HOLD_CALL_FAIL,						///< 364	Call Hold Fail
	CC_RESUME_CALL_SUCCESS,					///< 365	Call Resume Success
	CC_RESUME_CALL_FAIL,					///< 366	Call Resume Fail
	CC_SWAP_CALL_SUCCESS,					///< 367	Call Swap Success
	CC_SWAP_CALL_FAIL,						///< 368	Call Swap Fail
	CC_SPLIT_CALL_SUCCESS,					///< 369	Call Split Success
	CC_SPLIT_CALL_FAIL,						///< 370	Call Split Fail
	CC_JOIN_CALL_SUCCESS,					///< 371	Call Join Success
	CC_JOIN_CALL_FAIL,						///< 372	Call Join Fail
	CC_TRANS_CALL_SUCCESS,					///< 373	Call Transfer Success
	CC_TRANS_CALL_FAIL,						///< 374	Call Transfer Fail
	CC_SEND_DTMF_SUCCESS,					///< 375	Send DTMF Success
	CC_SEND_DTMF_FAIL,						///< 376	Send DTMF Fail
	CC_STOP_DTMF_SUCCESS,					///< 377	Stop DTMF Success
	CC_STOP_DTMF_FAIL,						///< 378	Stop DTMF Fail
	CC_OPERATION_SUCCESS,					///< 379	Operation Success
	RESULT_DIALSTR_INVALID,					///< 380	Dial String Invalid
	CC_UNKNOWN_DTMF_TONE,					///< 381 Unknown DTMF Tone
	CC_MAKE_DATA_CALL,						///< 382    Data Call needs to be initiated
    CC_BDN_BLOCK_MAKE_CALL					///< 383	Call request is blocked by BDN settings
			
} Result_t; ///< Result Codes

/** @} */

#endif

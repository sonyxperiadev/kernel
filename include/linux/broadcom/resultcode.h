/****************************************************************************
*
*     Copyright (c) 2007-2012 Broadcom Corporation
*
*   Unless you and Broadcom execute a separate written software license
*   agreement governing use of this software, this software is licensed to you
*   under the terms of the GNU General Public License version 2, available
*    at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html (the "GPL").
*
*   Notwithstanding the above, under no circumstances may you combine this
*   software in any way with any other Broadcom software provided under a
    license other than the GPL, without Broadcom's express prior written
    consent.
*
****************************************************************************/
/**
*
*   @file   resultcode.h
*
*   @brief  This file defines the return result code for APIs.
*
****************************************************************************/

#ifndef _RESULTCODE
#define _RESULTCODE

#ifdef __cplusplus
extern "C" {
#endif

#define SS_RESULT_START	                        50
#define PDP_RESULT_START                        70
#define STK_RESULT_START                        115
#define SMS_RESULT_STRRT                        120
#define VM_RESULT_START	                        160
#define AUDIO_RESULT_START                      170
#define DATA_RESULT_START                       180
#define CAM_RESULT_START                        190
#define TOOL_RESULT_START                       200
#define TOOL_SECBOOT_RESULT_START               230
#define TOOL_FLASHDRV_RESULT_START              250
#define CSD_RESULT_START                        260

#if defined(_TCH_DRV_TSC200X_)
#define TCH_RESULT_START                        320
#endif

#define CC_RESULT_START	                        350
#define RES_CME_GPRS_START                      400

/** @{ */

/**
	Result Codes
**/

typedef enum {
	/** 0	Result OK, operation successful */
	RESULT_OK = 0,

	/** 1	Result Error */
	RESULT_ERROR,

	/** 2	Result Done,operation completed */
	RESULT_DONE,

	/** 3	Result Unknown */
	RESULT_UNKNOWN,

	/** 4	Wrong Request Index */
	RESULT_WRONG_INDEX,

	/** 5 Request is sent when, the relative modul/task is in wrong state */
	RESULT_WRONG_STATE,

	/**
	  6 The particular modul/task is in the middle of processing a procedure
	 */
	RESULT_BUSY_TRY_LATER,

	/** 7	The procedure is aborted */
	RESULT_PROCEDURE_ABORTED,

	/** 8	Low memory */
	RESULT_LOW_MEMORY,

	/** 9	Result is Pending ( Used by CAPI2 API's ) */
	RESULT_PENDING,

	/** 10 One or more than one argument to the function is invalid */
	RESULT_INVALID_ARGUMENT,

	/** 11 Operation not allowed */
	RESULT_OPERATION_NOT_ALLOWED,

	/** 12 Timer expired */
	RESULT_TIMER_EXPIRED,

	/** 13 OUt of memory */
	RESULT_OUT_OF_MEMORY,

	/** 14 RPC Encoding fail */
	RESULT_ENCODING_FAIL,

	/** 15 RPC Decoding fail */
	RESULT_DECODING_FAIL,
	
	/** 16 RPC Message ID not found */
	RESULT_MSG_NOT_FOUND,
	
	/** 17 RPC Send Fail */
	RESULT_SEND_FAIL,

	/* ------ Result code for Supplementary-service APIs ----- */

	/** 50	Network Unavailable at this time */
	SS_NETWORK_NOT_AVAILABLE = SS_RESULT_START,

	/** 51	Requested SS operation in progress */
	SS_OPERATION_IN_PROGRESS,

	/** 52	Invalid USSD ID */
	SS_INVALID_USSD_ID,

	/** 53	Invalid USSD String. */
	SS_INVALID_USSD_STRING,

	/** 54	Invalid USSD DCS. */
	SS_INVALID_USSD_DCS,

	/** 55	Invalid request */
	SS_INVALID_SS_REQUEST,

	/** 56	SS dialing string not in FDN phonebook and FDN is enabled */
	SS_FDN_BLOCK_SS_REQUEST,

	/** 57	Invalid password length. Lenght of the password shall be 4 */
	SS_INVALID_PASSWORD_LENGTH,

	/* ------ Result code for Networks APIs ----- */

	/** 70	PCH operation not allowed */
	PCH_OPERATION_NOT_ALLOWED = PDP_RESULT_START,

	/** 71	Invalid PCH value */
	PCH_INVALID_VALUE,

	/** 72	Temporary attach not allowed */
	PCH_ATTACH_TEMP_NOT_ALLOWED,

	/** 73	LLC / SNDCP failure.SM cause 25 */
	PDP_LLC_OR_SNDCP_FAILURE,

	/** 74	Insufficient resources.SM cause 26 */
	PDP_INSUFFICIENT_RESOURCES,

	/** 75	Missing or unknown access point name.SM cause 27 */
	PDP_MISSING_OR_UNKNOWN_APN,

	/** 76	Unknown PDP address.SM cause 28 */
	PDP_UNKNOWN_PDP_ADDRESS,

	/** 77	User authentication failed.SM cause 29 */
	PDP_USER_AUTH_FAILED,

	/** 78	Activation rejected by GGSN.SM cause 30 */
	PDP_ACTIVATION_REJECTED_BY_GGSN,

	/** 79	Activation rejected, unspecified cause.SM cause 31 */
	PDP_ACTIVATION_REJECTED_UNSPECIFIED,

	/** 80	PDP service option not supported.SM cause 32 */
	PDP_SERVICE_OPT_NOT_SUPPORTED,

	/** 81	Service not subscribed.SM cause 33 */
	PDP_REQ_SERVICE_NOT_SUBSCRIBED,

	/** 82	Service temporarily out of order.SM cause 34 */
	PDP_SERVICE_TEMP_OUT_OF_ORDER,

	/** 83	network SAPI already used.SM cause 35 */
	PDP_NSAPI_ALREADY_USED,

	/** 84	Regular deactivation */
	PDP_REGULAR_DEACTIVATION,

	/** 85	Quality of service not accepted.SM cause 37 */
	PDP_QOS_NOT_ACCEPTED,

	/** 86	Network failure.SM cause 38 */
	PDP_NETWORK_FAILURE,

	/** 87	PDP reactivation required.SM cause 39 */
	PDP_REACTIVATION_REQUIRED,

	/** 88	Feature not supported.SM cause 40 */
	PDP_FEATURE_NOT_SUPPORTED,

	/** 89	Semantic error in the TFT operation.SM cause 41 */
	PDP_SEMANTIC_ERROR_IN_TFT,

	/** 90	Syntactical error in the TFT operation.SM cause 42 */
	PDP_SYNTACTICAL_ERROR_IN_TFT,

	/** 91	Unknown PDP context.SM cause 43 */
	PDP_UNKNOWN_PDP_CONTEXT,

	/** 92	Semantic errors in packet filter(s).SM cause 44 */
	PDP_SEMANTIC_ERROR_IN_PKT_FILTER,

	/** 93	Syntactical errors in packet filter(s).SM cause 45 */
	PDP_SYNTACTICAL_ERROR_IN_PKT_FILTER,

	/** 94	PDP context without TFT already activated.SM cause 46 */
	PDP_CONTEXT_WITHOUT_TFT,

	/** 95	Invalid TI.SM cause 81 */
	PDP_INVALID_TI,

	/** 96	Semantically incorrect message.SM cause 95 */
	PDP_SEMANT_INCORRECT_MSG,

	/** 97	Invalid mandatory information.SM cause 96 */
	PDP_INV_MANDATORY_IE,

	/** 98	Message type non-existent or not implemented.SM cause 97 */
	PDP_MSG_TYPE_NOT_EXISTENT,

	/** 99	Message type not compatible with protocol state.SM cause 98 */
	PDP_MSG_TYPE_NOT_COMPATIBLE,

	/** 100	IE non-existent or not implemented.SM cause 99 */
	PDP_IE_NON_EXISTENT,

	/** 101	Conditional IE error.SM cause 100 */
	PDP_CONDITIONAL_IE_ERROR,

	/** 102	Message not compatible with protocol state.SM cause 101 */
	PDP_MSG_NOT_COMPATIBLE,

	/** 103	Unspecified protocol error.SM cause 111 */
	PDP_PROTOCOL_ERROR_UNSPECIFIED,

	/** 104 PDP activation is blocked by either
		APN Control List (ACL) in 3G USIM or FDN settings in 2G SIM.
	 */
	PDP_ACTIVATION_BLOCKED_BY_SIM,

	/** 105	There is an active voice call */
	PDP_CIRCUIT_CALL_ACTIVE,

	/** 106  */
	PDP_OPERATOR_DETERMINED_BARRING,

	/** 107 */
	PDP_APN_INCOMPATIBLE_W_ACTIVE_PDP,

	/** 108 MS_PlmnSelect succeed ( No network request sent ) */
	RESULT_PLMN_SELECT_OK,

	/** 109 Bearer Control Mode Violation */
	PDP_BEARER_CTRL_MODE_VIOLATION,

	/** 110 PCH_LAST_PDN_DISCONNECTION_NOT_ALLOWED */
	PDP_LAST_PDN_DISCONNECTION_NOT_ALLOWED,

	/* ------ Result code for STK Data Service ----- */

	/** 115	STK data service is busy and need to wait for it finish */
	STK_DATASVRC_BUSY = STK_RESULT_START,

	/** 116	SIM is not ready */
	RESULT_SIM_NOT_READY,

	/* ------ Result code for SMS ----- */

	/** 120	SIM is ready for SMS access */
	SMS_SIM_SMS_READY = SMS_RESULT_STRRT,

	/** 121	SIM is busy, need to wait. */
	SMS_SIM_BUSY,

	/** 122	SIM not inserted or seated correctly. */
	SMS_SIM_NOT_INSERT,

	/** 123	ME ready for SMS access	*/
	SMS_ME_SMS_READY,

	/** 124	ME not ready for SMS access */
	SMS_ME_SMS_NOT_READY,

	/** 125	SMS is not supported by the ME.	*/
	SMS_ME_SMS_NOT_SUPPORTED,

	/** 126	Only Circuit Switched access allowed by the network for SMS */
	SMS_NETWORK_CS_ONLY,

	/** 127	Only CGPRS access allowed by the network for SMS */
	SMS_NETWORK_GPRS_ONLY,

	/** 128	SMS operation is not allowed */
	SMS_OPERATION_NOT_ALLOWED,

	/** 129	SMS service currently not available */
	SMS_NO_SERVICE,

	/** 130	Address string is too long */
	SMS_ADDR_NUMBER_STR_TOO_LONG,

	/** 131	Invalid Service Center Address Character */
	SMS_SCA_INVALID_CHAR_INSTR,

	/** 132	Invalid Service Center Address */
	SMS_INVALID_SCA_ADDRESS,

	/** 133	Service Center Address not supported in this SIM */
	SMS_SCA_NOT_SUPPORTED_IN_SIM,

	/** 134	Invalid index */
	SMS_INVALID_INDEX,

	/** 135	PDU length is not valid. */
	SMS_INVALID_PDU_LENGTH,

	/** 136	PDU Mode parameter is not valid. */
	SMS_INVALID_PDU_MODE_PARM,

	/** 137	Text message length is invalid.	*/
	SMS_INVALID_TEXT_LENGTH,

	/** 138	SMS parameter is not valid. */
	SMS_INVALID_SMS_PARAM,

	/** 139	No message to list. */
	SMS_NO_MSG_TO_LIST,

	/** 140	Message decoding error. */
	SMS_MSG_DECODING_ERROR,

	/** 141	Dial string character is not valid */
	SMS_INVALID_DIALSTR_CHAR,

	/** 142	This is a concatenated SMS */
	SMS_IS_CONCATENATED_SMS,

	/** 143	This is not a concatenated SMS */
	SMS_IS_NOT_CONCATENATED_SMS,

	/** 144	Invalid UDH data */
	SMS_INVALID_UDH_DATA,

	/** 145	Message ID has exceeded range (see GSm 3.41) */
	SMS_CB_MIDS_EXCEED_RANGE_LIMIT,

	/** 146	This message ID already exists.  */
	SMS_CB_MIDS_ALREADY_EXIST,

	/** 147	Message ID does not exist. */
	SMS_CB_MIDS_DOES_NOT_EXIST,

	/** 148	Service Center Address is NULL */
	SMS_SCA_NULL_STRING,

	/** 149	Fixed Dialing not allowed */
	SMS_FDN_NOT_ALLOWED,

	/** 150	SMS Call control Barring */
	SMS_CALL_CONTROL_BARRING,

	/** 151	SMS VCC timeout */
	SMS_VCC_TIMEOUT,

	/** 160	Voice Mail Not available */
	VM_NOT_AVAILABLE = VM_RESULT_START,

	/** audio */

	/** 170	Invalid Audio Channel */
	AUDIO_INVALID_AUDIO_CHNL = AUDIO_RESULT_START,

	/** 171	Invalid Volume Gain setting */
	AUDIO_INVALID_VOLUME_GAIN,

	/** 172	Invalid Tone ID */
	AUDIO_INVALID_TONE_ID,

	/* ------ Result code for User Data Account ----- */

	/** 180	Invalid Account ID */
	DATA_INVALID_ACCTID = DATA_RESULT_START,

	/** 181	Account ID already in use. Try another */
	DATA_ACCTID_IN_USED,

	/** 182	Context ID for GPRS not available */
	DATA_GPRS_NO_CID_AVAILABLE,

	/** 183 invalid TFT */
	DATA_GPRS_INVALID_TFT,

	/** 184 primay cid can not be found */
	DATA_GPRS_NO_PRI_CID,

	/* ------ Result code for Camera Functions */

	/** 190	Camera function success */
	CAM_SUCCESS = CAM_RESULT_START,

	/** 191	Camera not supported */
	CAM_UNSUPPORTED,

	/** 192	Camera operation failed */
	CAM_FAILED,

	/** 193	Camera Dma operation failed */
	CAM_DMA_FAILED,

	/* ------ Result code for Tools */

	/** 200	Invalid argument */
	RESULT_TOOL_INVALID_ARG = TOOL_RESULT_START,

	/** 201	File open error */
	RESULT_TOOL_FILE_OPEN_ERROR,

	/** 202	File create error */
	RESULT_TOOL_FILE_CREATE_ERROR,

	/** 203	File read error */
	RESULT_TOOL_FILE_READ_ERROR,

	/** 204	File write error */
	RESULT_TOOL_FILE_WRITE_ERROR,

	/** 205	Internal error */
	RESULT_TOOL_INTERNAL_ERROR,

	/** 206	Include file error */
	RESULT_TOOL_INCFILE_ERROR,

	/** 207	Invalid keyword */
	RESULT_TOOL_INVALID_KEYWORD,

	/** 208	Wrong number of parmeter */
	RESULT_TOOL_WRONG_NUM_OF_PARMS,

	/** 209	Invalid value */
	RESULT_TOOL_INVALID_VALUE,

	/** 210	COM port error */
	RESULT_TOOL_COMPORT_ERROR,

	/** 211	Sync error */
	RESULT_TOOL_SYNC_ERROR,

	/** 212	Timeout */
	RESULT_TOOL_TIMEOUT,

	/** 213	Timeout */
	RESULT_TOOL_ADDRESS_INVALID,

	/** 214	Invalid IMEI */
	RESULT_TOOL_INVALID_IMEI,

	/** 215	Invalid MCC/MNC */
	RESULT_TOOL_INVALID_MCC_MNC,

	/** 216	Wrong order simlock_codes */
	RESULT_TOOL_WRONG_ORDER_SIMLOCK_CODES,

	/** 217	Invalid IMSI */
	RESULT_TOOL_INVALID_IMSI,

	/** 218	Sysparm warning */
	RESULT_TOOL_SYSPARM_WARNING,

	/** 219	Sysparm error */
	RESULT_TOOL_SYSPARM_ERROR,

	/** 220	Canceled */
	RESULT_TOOL_CANCELED,

	/* ------ secure bootloader tools */

	/** 230	Secure boot command invalid */
	RESULT_TOOL_SECBOOT_CMD_INVALID = TOOL_SECBOOT_RESULT_START,

	/** 231	Secure boot format invalid */
	RESULT_TOOL_SECBOOT_FORMAT_INVALID,

	/** 232	Secure boot baudrate invalid */
	RESULT_TOOL_SECBOOT_BAUDRATE_INVALID,

	/** 233	Secure boot CRC error */
	RESULT_TOOL_SECBOOT_CRC_ERROR,

	/** 234	Secure boot memory align error */
	RESULT_TOOL_SECBOOT_MEMALIGN_ERROR,

	/** 235	Secure boot flash program error */
	RESULT_TOOL_SECBOOT_FLASHPRGM_ERROR,

	/** 236	Secure boot other error */
	RESULT_TOOL_SECBOOT_OTHER_ERROR,

	/** 237	Secure boot protocol error */
	RESULT_TOOL_SECBOOT_PROTOCOL_ERROR,

	/** 238	Secure boot bufsize error */
	RESULT_TOOL_SECBOOT_BUFSIZE_ERROR,

	/** 239	Secure boot file open error */
	RESULT_TOOL_SECBOOT_FILEOPEN_ERROR,

	/* ------ flash driver errors */

	/** 250	Flash drive timeout */
	RESULT_TOOL_FLASHDRV_TIMEOUT = TOOL_FLASHDRV_RESULT_START,

	/** 251	Flash drive memory align error */
	RESULT_TOOL_FLASHDRV_MEMALIGN_ERROR,

	/** 252	Flash drive address error */
	RESULT_TOOL_FLASHDRV_ADDRESS_ERROR,

	/** 253	Flash drive device unknow  */
	RESULT_TOOL_FLASHDRV_DEVICE_UNKNOWN,

	/** 260	CSD cause void */
	CSD_CAUSE_VOID_CAUSE = CSD_RESULT_START,

	/** 261	CSD cause unassigned number */
	CSD_CAUSE_UNASSIGNED_NUMBER,

	/** 262	CSD cause no route */
	CSD_CAUSE_NO_ROUTE,

	/** 263	CSD cause unacceptable */
	CSD_CAUSE_CHANNEL_UNACCEPTABLE,

	/** 264	CSD cause operator barring */
	CSD_CAUSE_OPERATOR_BARRING,

	/** 265	CSD cause normal call clearing */
	CSD_CAUSE_NORMAL_CALL_CLEARING,

	/** 266	CSD cause user busy */
	CSD_CAUSE_USER_BUSY,

	/** 267	CSD cause no user responding */
	CSD_CAUSE_NO_USER_RESPONDING,

	/** 268	CSD cause user alerting no answer */
	CSD_CAUSE_USER_ALERTING_NO_ANSWR,

	/** 269	CSD cause MN call rejected */
	CSD_CAUSE_MN_CALL_REJECTED,

	/** 270	CSD cause number changed  */
	CSD_CAUSE_NUMBER_CHANGED,

	/** 271	CSD cause non select user clr */
	CSD_CAUSE_NON_SELECT_USER_CLR,

	/** 272	CSD cause dest out of order */
	CSD_CAUSE_DEST_OUT_OF_ORDER,

	/** 273	CSD cause invalid number format */
	CSD_CAUSE_INVALID_NUMBER_FORMAT,

	/** 274	CSD cause facility rejected */
	CSD_CAUSE_FACILITY_REJECTED,

	/** 275	CSD cause response to status enq */
	CSD_CAUSE_RESPONSE_TO_STATUS_ENQ,

	/** 276	CSD cause nomal unspecified */
	CSD_CAUSE_NORMAL_UNSPECIFIED,

	/** 277	CSD cause no circuit available */
	CSD_CAUSE_NO_CIRCUIT_AVAILABLE,

	/** 278	CSD cause network out of order */
	CSD_CAUSE_NETWORK_OUT_OF_ORDER,

	/** 279	CSD cause temporary out of order */
	CSD_CAUSE_TEMPORARY_FAILURE,

	/** 280	CSD ause switch congestion */
	CSD_CAUSE_SWITCH_CONGESTION,

	/** 281	CSD cause access info discarded */
	CSD_CAUSE_ACCESS_INFO_DISCARDED,

	/** 282	CSD cause requested circuit not available */
	CSD_CAUSE_REQUESTED_CIRCUIT_NOT_AVAILABLE,

	/** 283	CSD cause resources unavailable */
	CSD_CAUSE_RESOURCES_UNAVAILABLE,

	/** 284	CSD cause quality unavailable */
	CSD_CAUSE_QUALITY_UNAVAILABLE,

	/** 285	CSD cause facility not subscribed */
	CSD_CAUSE_FACILITY_NOT_SUBSCRIBED,

	/** 286	CSD cause incoming calls barred in CUG */
	CSD_CAUSE_INCOMING_CALLS_BARRED_IN_CUG,

	/** 287	CSD cause bearer capability not allowed */
	CSD_CAUSE_BEARER_CAPABILITY_NOT_ALLOWED,

	/** 288	CSD cause bearer capability not available */
	CSD_CAUSE_BEARER_CAPABILITY_NOT_AVAILABLE,

	/** 289	CSD cause service not available */
	CSD_CAUSE_SERVICE_NOT_AVAILABLE,

	/** 290	CSD cause service not implemented */
	CSD_CAUSE_BEARER_SERVICE_NOT_IMPLEMENTED,

	/** 291	CSD cause ACM >= ACMMAX */
	CSD_CAUSE_ACM_GREATER_OR_EQUAL_TO_ACMMAX,

	/** 292	CSD cause facility not implemented */
	CSD_CAUSE_FACILITY_NOT_IMPLEMENTED,

	/** 293	CSD cause only restriced digital */
	CSD_CAUSE_ONLY_RESTRICTED_DIGITAL,

	/** 294	CSD cause service not implemented */
	CSD_CAUSE_SERVICE_NOT_IMPLEMENTED,

	/** 295	CSD cause invalid TI */
	CSD_CAUSE_INVALID_TI,

	/** 296	CSD cause user not in CUG */
	CSD_CAUSE_USER_NOT_IN_CUG,

	/** 297	CSD cause incomplatible destination */
	CSD_CAUSE_INCOMPATIBLE_DESTINATION,

	/** 298	CSD cause invalid transit network */
	CSD_CAUSE_INVALID_TRANSIT_NETWORK,

	/** 299	CSD cause sematics incorrect */
	CSD_CAUSE_SEMATICS_INCORRECT,

	/** 300	CSD cause invalid mandatory information */
	CSD_CAUSE_INVALID_MANATORY_INFORMATION,

	/** 301	CSD cause message type non exist */
	CSD_CAUSE_MESG_TYPE_NON_EXISTENT,

	/** 302	CSD cause message type nto compatible with state */
	CSD_CAUSE_MESG_TYPE_NOT_COMPATIBLE_WITH_STATE,

	/** 303	CSD cause IE non existent */
	CSD_CAUSE_IE_NON_EXISTENT,

	/** 304	CSD cause conditional IE error */
	CSD_CAUSE_CONDITIONAL_IE_ERROR,

	/** 305	CSD cause message not compatible with state */
	CSD_CAUSE_MESG_NOT_COMPATIBLE_WITH_STATE,

	/** 306	CSD cause recovery on timer expiry */
	CSD_CAUSE_RECOVERY_ON_TIMER_EXPIRY,

	/** 307	CSD cause protocol error */
	CSD_CAUSE_PROTOCOL_ERROR,

	/** 308	CSD cause interworking */
	CSD_CAUSE_INTERWORKING,

	/** 309	CSD cause radio link failure appeared */
	CSD_CAUSE_RADIO_LINK_FAILURE_APPEARED,

	/** 310	CSD cause reestablishment successful */
	CSD_CAUSE_REESTABLISHMENT_SUCCESSFUL,

#if defined(_TCH_DRV_TSC200X_)
	/** 320	Touch screen function success */
	TCH_SUCCESS = TCH_RESULT_START,

	/** 321	Touch screen not supported */
	TCH_UNSUPPORTED,

	TCH_FAILED,
#endif

	/* ------ Result code for Call-Control APIs ----- */

	/** 350	Call Session Fail, fails after call has been set up */
	CC_FAIL_CALL_SESSION = CC_RESULT_START,

	/** 351	Make Call Fail */
	CC_FAIL_MAKE_CALL,

	/** 352	Call request is blocked by FDN settings */
	CC_FDN_BLOCK_MAKE_CALL,

	/** 353	Make Call Success */
	CC_MAKE_CALL_SUCCESS,

	/** 354	Auxillary Call Not Allowed */
	CC_DISALLOW_AUXILIARY_CALL,

	/** 355	Wrong Call type */
	CC_WRONG_CALL_TYPE,

	/** 356	Wrong Call Index */
	CC_WRONG_CALL_INDEX,

	/** 357	End Call Success */
	CC_END_CALL_SUCCESS,

	/** 358	End Call Fail */
	CC_END_CALL_FAIL,

	/** 359	Call Hangup Success */
	CC_HANGUP_CALL_SUCCESS,

	/** 360	Call Hangup Fail */
	CC_HANGUP_CALL_FAIL,

	/** 361	Call Accept Success */
	CC_ACCEPT_CALL_SUCCESS,

	/** 362	Call Accept Fail */
	CC_ACCEPT_CALL_FAIL,

	/** 363	Call Hold Success */
	CC_HOLD_CALL_SUCCESS,

	/** 364	Call Hold Fail */
	CC_HOLD_CALL_FAIL,

	/** 365	Call Resume Success */
	CC_RESUME_CALL_SUCCESS,

	/** 366	Call Resume Fail */
	CC_RESUME_CALL_FAIL,

	/** 367	Call Swap Success */
	CC_SWAP_CALL_SUCCESS,

	/** 368	Call Swap Fail */
	CC_SWAP_CALL_FAIL,

	/** 369	Call Split Success */
	CC_SPLIT_CALL_SUCCESS,

	/** 370	Call Split Fail */
	CC_SPLIT_CALL_FAIL,

	/** 371	Call Join Success */
	CC_JOIN_CALL_SUCCESS,

	/** 372	Call Join Fail */
	CC_JOIN_CALL_FAIL,

	/** 373	Call Transfer Success */
	CC_TRANS_CALL_SUCCESS,

	/** 374	Call Transfer Fail */
	CC_TRANS_CALL_FAIL,

	/** 375	Send DTMF Success */
	CC_SEND_DTMF_SUCCESS,

	/** 376	Send DTMF Fail */
	CC_SEND_DTMF_FAIL,

	/** 377	Stop DTMF Success */
	CC_STOP_DTMF_SUCCESS,

	/** 378	Stop DTMF Fail */
	CC_STOP_DTMF_FAIL,

	/** 379	Operation Success */
	CC_OPERATION_SUCCESS,

	/** 380	Dial String Invalid */
	RESULT_DIALSTR_INVALID,

	/** 381 Unknown DTMF Tone */
	CC_UNKNOWN_DTMF_TONE,

	/** 382    Data Call needs to be initiated */
	CC_MAKE_DATA_CALL,

	/** 383	Call request is blocked by BDN settings */
	CC_BDN_BLOCK_MAKE_CALL,

	/** 384	Call request failed due to collision with MT */
	CC_FAIL_CALL_MT_IN_PROGRESS,


	/* ------ Result code for CME GPRS error ----- */

	/** 400	Illegal MS */
	RESULT_ERR_ILLEGAL_MS = RES_CME_GPRS_START,

	/** 401	Illegal ME */
	RESULT_ERR_ILLEGAL_ME,

	/** 402	GPRS service not allowed */
	RESULT_ERR_GPRS_SVC_NOT_ALLOWED,

	/** 403	GPRS PLMN not allowed */
	RESULT_ERR_PLMN_NOT_ALLOWED,

	/** 404	LA not allowed */
	RESULT_ERR_LA_NOT_ALLOWED,

	/** 405	Roam not allowed */
	RESULT_ERR_ROAM_NOT_ALLOWED

} Result_t; /** Result Codes */

/** @} */


#ifdef __cplusplus
}
#endif

#endif  /* _RESULTCODE */

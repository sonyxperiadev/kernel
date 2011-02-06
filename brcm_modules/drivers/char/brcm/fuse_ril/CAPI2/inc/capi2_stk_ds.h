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
*   @file   capi2_stk_ds.h
*
*   @brief  This file contains SATK API related defines.
*
****************************************************************************/
/**

*   @defgroup   CAPI2_STKAPIGroup   SIM Toolkit
*   @ingroup    CAPI2_STKGroup
*
*   @brief      This group defines the types and prototypes for the SIM toolkit API functions.
*	
****************************************************************************/
#ifndef _CAPI2_STK_DS_H_
#define _CAPI2_STK_DS_H_

#include "capi2_types.h"
#include "capi2_sim_api.h"

#define STK_MAX_DTMF_LEN		40
#define MAX_SIM_URL_LENGTH 245 // 255 - (tag + length + cmd details + device id) + 1 (null termination)
#define MAX_SIM_BEARER_LENGTH 6
#define MAX_SIM_PROV_FILE_NUM 4
#define MAX_SIM_PROV_FILE_LENGTH 12
#define STK_MAX_ALPHA_TEXT_LEN	274 // BRCM/Davis Zhu 7/19/2005: 274byte=(240byte*8bit)/7bit; Fix the shorter display in 27.22.4.22.1(7).
#define MAX_TERMINAL_PROFILE_ARRAY_SIZE		17
#define STK_SS_DATA_LEN          255
#define STK_SMS_ADDRESS_LEN      254

/** See definitions in Section 12.6 of ETSI TS 101.267 */
typedef enum
{
	LAUNCH_BROWSER_NO_CONNECT = 0,			/**< Launch browser witout making a connection, if not already launched */
	LAUNCH_BROWSER_CONNECT = 1,				/**< Launch browser making a connection, if not already launched */
	LAUNCH_BROWSER_USE_EXIST = 2,			/**< Use existing browser, the browser shall not use the active existing secured session */
	LAUNCH_BROWSER_CLOSE_EXIST_NORMAL = 3,	/**< Close existing browser session, launch new browser making a connection */
	LAUNCH_BROWSER_CLOSE_EXIST_SECURE = 4	/**< Close existing browser session, launch new browser using a secure session */
} SIM_LAUNCH_BROWSER_ACTION_t;


/* Do not change the values of the following enum. They are defined according to assigned values  
 * for the different Refresh types defined in Section 12.6 of ETSI TS 101 267 (GSM 11.14).
 */
typedef enum{
	SMRT_INIT_FULLFILE_CHANGED = 0,
	SMRT_FILE_CHANGED = 1, 
	SMRT_INIT_FILE_CHANGED = 2, 
	SMRT_INIT = 3, 
	SMRT_RESET = 4
} STKRefreshType_t;

/**
Data Structure

Enum order to be consistent with AT Interface Spec for SATK2.0
**/
typedef enum {
	SATK_EVENT_DISPLAY_TEXT = 0,		///<
	SATK_EVENT_GET_INKEY = 1,			///<
	SATK_EVENT_GET_INPUT = 2,			///<
	SATK_EVENT_PLAY_TONE = 3,			///<
	SATK_EVENT_SELECT_ITEM = 4,			///<
	SATK_EVENT_SEND_SS = 5,				///<
	SATK_EVENT_SEND_USSD = 6,			///<
	SATK_EVENT_SETUP_CALL = 7,			///<
	SATK_EVENT_SETUP_MENU = 8,			///<
	SATK_EVENT_MENU_SELECTION = 9,		///<
	SATK_EVENT_REFRESH = 10,			///<
	SATK_EVENT_SEND_SHORT_MSG = 11, 	///<
	SATK_EVENT_SEND_DTMF = 12, 			///<
	SATK_EVENT_LAUNCH_BROWSER = 13, 	///<
	SATK_EVENT_IDLEMODE_TEXT = 14, 		///<
	SATK_EVENT_PROV_LOCAL_LANG = 15, 	///<
	SATK_EVENT_DATA_SERVICE_REQ = 16,	///<
    SATK_EVENT_RUN_AT_CMD = 17,         ///<
	SATK_EVENT_BATTERY_STATUS = 18,
    SATK_EVENT_OPEN_CHAN = 19,
	SATK_EVENT_CLOSE_CHAN = 20,
    SATK_EVENT_SEND_DATA = 21,
    SATK_EVENT_RECEIVE_DATA = 22,
    SATK_EVENT_CHAN_STATUS_UPDATE = 23,
    SATK_EVENT_DATA_AVAILABLE = 24,
    SATK_EVENT_GET_CHAN_STATUS = 25,
	SATK_EVENT_ACTIVATE	= 26,

	SATK_EVENT_INVALID 

} SATK_EVENTS_t;

/**
Result code result1
**/
typedef enum {

	// result code for result-1
	SATK_Result_CmdSuccess 					= 0,	///<
	SATK_Result_RefreshPerformedWithAdditionalEFsRead = 3,   ///<
	SATK_Result_CmdSuccessIconNotDisplayed	= 4,	///<
	SATK_Result_CmdSuccessButModifiedBySim	= 5,	 ///<
	SATK_Result_SIMSessionEndSuccess		= 10,   ///<
	SATK_Result_BackwardMove				= 11,   ///<
	SATK_Result_NoRspnFromUser				= 12,   ///<
	SATK_Result_RequestHelpByUser			= 13,	///<
	SATK_Result_UserEndUSSD					= 14,   ///<
	SATK_Result_MeUnableToProcessCmd		= 20,   ///<
	SATK_Result_NetUnableToProcessCmd		= 21,   ///<
	SATK_Result_UserNotAcceptingCallSetup	= 22,   ///<
	SATK_Result_UserEndCallBeforeConnect	= 23,   ///<
	SATK_Result_InteractWithSimCCTempProblem= 25,   ///<
	SATK_Result_LaunchBrowserGenericError	= 26,   ///< (Browser generic error: browser not available)
	SATK_Result_BeyondMeCapability			= 30,   ///<
	SATK_Result_TypeUnknownToMe				= 31,   ///<
	SATK_Result_DataUnknownToMe				= 32,   ///<
	SATK_Result_NumberUnknownToMe			= 33,   ///<
	SATK_Result_ValueMissingError			= 36,   ///<
	SATK_Result_USSDError					= 37,   ///<
	SATK_Result_InteractWithSimCCPermProblem= 39,   ///<
	SATK_Result_User_Busy					= 40,   ///< (Network rejects call with user busy cause)
	SATK_Result_Busy_On_Call				= 41,   ///< (ME is busy on voice call)
	SATK_Result_SSReturnResultError			= 52,   ///<

	SATK_Result_Invalid
} SATK_ResultCode_t;

/**
Result code result2
**/
typedef enum {
	SATK_Result_NoCause						= 0,   ///<
	SATK_Result_ScreenBusy					= 1,   ///<
	SATK_Result_BusyOnCall					= 2,   ///<
	SATK_Result_BusyOnSS					= 3,   ///<
	SATK_Result_NoService					= 4,   ///<
	SATK_Result_RrNotGranted				= 6,   ///< (Radio resource not granted)
	SATK_Result_NotInSpeechCall				= 7,   ///<
	SATK_Result_BusyOnUSSD					= 8,   ///<
	SATK_Result_BusyOnDtmf					= 9,   ///< (Busy on sending DTMF command)

	SATK_Result_Invalid2

} SATK_ResultCode2_t;

/**
STK get inkey response type
**/
typedef enum{
	S_IKT_YESNO,		///<
	S_IKT_DIGIT,		///< 0-9,*,+,#
	S_IKT_SMSDEFAULTSET,///<
	S_IKT_UCS2			///<
} SATKInKeyType_t;

/**
STK tone type
**/
typedef enum{
	S_TT_DEFAULTTONE			= 0x00, ///<
	S_TT_DIALTONE				= 0x01, ///<
	S_TT_CALLEDUSERBUSY			= 0x02, ///<
	S_TT_CONGEST				= 0x03, ///<
	S_TT_RADIOPATHACK			= 0x04, ///<
	S_TT_RADIOPATHUNAVAILABLE	= 0x05,	///<
	S_TT_ERROR					= 0x06, ///<
	S_TT_CALLWAITING			= 0x07, ///<
	S_TT_RINGING				= 0x08, ///<
	S_TT_BEEP					= 0x10, ///<
	S_TT_POSITIVEACK			= 0x11, ///<
	S_TT_NEGTIVEACK				= 0x12 ///<
} SATKToneType_t;

/**
STK Send SS type
**/
typedef enum{
	S_SST_SS_TYPE				= 0, ///<
	S_SST_USSD_TYPE_EARLY_USAGE	= 1, ///<
	S_SST_USSD_TYPE		   		= 2, ///<
	S_SST_NO_SS_TYPE			= 3  ///<
} SATKSSType_t;

/// STK number type
typedef struct{
	UInt8			Num[PHASE2_MAX_USSD_STRING_SIZE+1];	///< number: null terminated number string. For example of 123, Num[0] = '1'; Num[1] = '2', Num[2] = '3', Num[3] = NULL;
														///< Note: In USSD case, the "Num" field is the USSD string in ASCII format
	gsm_TON_t		Ton;					///< Type of number (This has no meaning in SS case)
	gsm_NPI_t		Npi;					///< Numbering plan identifier (This has no meaning in SS case)
	UInt8			dcs;					///< dcs of the USSD string required by SIM (dcs&0x0f)>>2=0,1,2=>7bit,8bit,ucs2
	UInt8			len;					///< len field to ensure correct UCS2 USSD string length
} SATKNum_t;

/**
STK setup call type
**/
typedef enum{
	S_CT_ONIDLE,				///<
	S_CT_ONIDLE_REDIALABLE,		///<
	S_CT_HOLDABLE,				///<
	S_CT_HOLDABLE_REDIALABLE,	///<
	S_CT_DISCONNECTABLE,		///<
	S_CT_DISCONNECTABLE_REDIALABLE ///<
} SATKCallType_t;

/**
STK call control by SIM type
**/
typedef enum{
	SATK_CC_CALL_CONTROL,	///<
	SATK_CC_MOSMS,			///<
	SATK_CC_SS,				///<
	SATK_CC_USSD,			///<
	SATK_CC_NONE			///<
} SATKCallControlType_t;

typedef enum{
	SATK_PROCALL_STATE_NONE,
	SATK_PROCALL_STATE_WAIT_RSP,
	SATK_PROCALL_STATE_ACTIVE,
	SATK_PROCALL_STATE_ACTIVE_MODIFIED
} ProCallState_t;

/**
Data Service type defined by BRCM AT command document 
**/
typedef enum{
	SATK_DATA_SERV_OPEN = 1,		///<
	SATK_DATA_SERV_SEND_DATA = 2,	///<
	SATK_DATA_SERV_REC_DATA = 3,	///<
	SATK_DATA_SERV_CLOSE = 4		///<
} SATKDataServiceType_t;

/// Provisioning File Reference data 
typedef struct
{
	UInt8 length;	/**< Length of bytes occupied in "prov_file" */
	UInt8 prov_file_data[MAX_SIM_PROV_FILE_LENGTH];
} ProvisionFile_t;



/// STK string type
typedef struct{
	UInt8			*string;		///< string
	/* BRCM/Davis Zhu 8/24/2005: Changed from UInt8 to fix the short display problem in TC27.22.4.22.1(7)  */
	UInt16			len;			///< string length
	Unicode_t 		unicode_type;	///< unicode type
} SATKString_t;

// STK Alpha ID type
typedef struct
{
	UInt8			string[STK_MAX_ALPHA_TEXT_LEN+1];	///< string
	UInt16			len;			                    ///< string length
	Unicode_t 		unicode_type;	                    ///< unicode type
} SATKAlphaID_t;

/// STK icon type
typedef struct{
	UInt8			Id;///<
	Boolean			IsSelfExplanatory;///<
	Boolean			IsExist;///<
} SATKIcon_t;

#define STK_MAX_LIST_SIZE		40
#define MAX_ITEM						40										///<
#define	SATK_LOGIN_PASSWD_LEN			20										///<

#define SATK_MAX_APN_LEN				32										///<
/// This type is used by both the Icon List Items and the Next Actiona Item list
typedef struct{
	UInt8			Id[STK_MAX_LIST_SIZE];
	Boolean			IsSelfExplanatory;
	Boolean			IsExist;
} STKIconListId_t;



/// STK Data Service Respond 
typedef struct {
	UInt16			clientID;							///< Client ID
	SATK_EVENTS_t	event;								///< event type
	SATK_ResultCode_t result1;							///< result code 1
	UInt8			result2;							///< result code 2
	UInt8			textStr1[SATK_LOGIN_PASSWD_LEN+1];	///< login
	UInt8			textStr2[SATK_LOGIN_PASSWD_LEN+1];	///< passwd
	UInt8			textStr3[SATK_MAX_APN_LEN+1];		///< APN for GPRS bearer
} StkCmdRespond_t;
//----------------message typedefs-------------
/// STK display text event type
typedef struct {
	SATKString_t	stkStr;			///< data
	Boolean			isHighPrio;		///< priority
	SATKIcon_t		icon;			///< icon
	Boolean			isDelay;		///< clear
	Boolean			isSustained;	///< TRUE for Sustained Display Text for which Terminal Response must be sent without delay: see Section 6.4 of ETSI TS 102 223.
} DisplayText_t;

/// Idle Mode text event type
typedef struct {
	SATKString_t	stkStr;			///< data
	SATKIcon_t		icon;			///< icon
} IdleModeText_t;

/// STK Get Input Key event type
typedef struct {
	SATKString_t	stkStr;				///< data
	SATKIcon_t		icon;				///< icon
	SATKInKeyType_t	inKeyType;			///< input,  alphabet and resp
	Boolean			isHelpAvailable;	///< is help available
} GetInkey_t;

/// STK get Input event type
typedef struct {
	UInt8			minLen;				///< minl
	UInt8			maxLen;				///< maxl
	SATKString_t	stkStr;				///< data raw, refer to IsPacked
	SATKString_t	defaultSATKStr;		///< default text, optional.
	SATKIcon_t		icon;				///< icon
	SATKInKeyType_t	inPutType;			///< input and  alphabet
	Boolean			isHelpAvailable;	///< is help available
	Boolean			isEcho;				///< echo
	Boolean			isPacked;			///< unpacked or packed SMS, data
} GetInput_t;

/// STK playtone event type
typedef struct {
	SATKString_t	stkStr;		///< alphaid
    Boolean         defaultStr; ///< TRUE is default Alpa ID is used
	SATKToneType_t	toneType;	///< tone
	UInt32			duration;	///< milliseconds unit and 
	SATKIcon_t		icon;		///< icon
} PlayTone_t;

/// Maximum number of files in File Change List of Refresh proactive command
#define MAX_NUM_OF_FILE_IN_CHANGE_LIST	25

/// Maximum length for the path of a SIM/USIM file
#define MAX_SIM_FILE_PATH_LEN	5

/// Refresh file
typedef struct
{
	UInt8 path_len;		///< Number of file ID's in "file_path"
	UInt16 file_path[MAX_SIM_FILE_PATH_LEN];	///< SIM file ID path starting from MF, i.e. "sim_file_path[0]" is always 0x3F00 
} REFRESH_FILE_t;

/// Structure to store the File Change List of Refresh proactive command
typedef struct
{
	UInt8 number_of_file;
	REFRESH_FILE_t	changed_file[MAX_NUM_OF_FILE_IN_CHANGE_LIST];
} REFRESH_FILE_LIST_t;

/// STK refresh event type
typedef struct {
	STKRefreshType_t	refreshType;		///< refresh type
	REFRESH_FILE_LIST_t	FileIdList;			///< This is file ID array in refresh command
	USIM_APPLICATION_TYPE appliType;		///< Application for the refresh specified in AID IE (see Section 6.6.13 of ETSI 102 223); 
											///< Set to USIM_APPLICATION if AID IE does not exist in Refresh proactive command or for 2G SIM 
} Refresh_t;


/// STK MO SMS data including Service Center Address and the actual TPDU 
typedef struct
{
	UInt8				sca_ton_npi;	///< Serice Center Number info: Type of Number & National Numbering Plan
	UInt8				sca_len;		///< number of bytes in sca_data
	UInt8				sca_data[20];	///< Service Center Number encoded in BCD format
	UInt8				pdu_len;		///< number of bytes in pdu_data
	UInt8				pdu_data[176];	///< SMS PDU data
} STK_SMS_DATA_t;


/// STK send MO SMS event type
typedef struct {
	Boolean			isAlphaIdProvided; 		///< TRUE if alpha ID TLV present in Send SMS proative command
	SATKString_t	text;	 				///< valid or No alphaid
	SATKIcon_t		icon;					///< icon
	STK_SMS_DATA_t	sms_data;				///< SMS Service Center Address and TPDU data
} SendMOSMS_t;

/// STK Send Supplementary Service request event type
typedef struct {
	SATKSSType_t	ssType;					///< ss type
	SATKNum_t		num;					///< SSstring 
	Boolean			isAlphaIdProvided; 		///< 0/1: no AlphaId/AlphaId
	SATKString_t	text;					///< alphaid string
	SATKIcon_t		icon;					///< icon
} SendSs_t;

/// STK send USSD request event type
typedef struct {
	SATKSSType_t	ssType;					///< ss type
	SATKNum_t		num;					///< USSDstring 
	Boolean			isAlphaIdProvided; 		///< 0/1: no AlphaId/AlphaId
	SATKString_t	text;					///< alphaid string
	SATKIcon_t		icon;					///< icon
} SendUssd_t;

/// STK call setup event type 
typedef struct {
	Boolean			isEmerCall;				///< is emergency call
	SATKCallType_t	callType;				///< call type
	Boolean			IsSetupAlphaIdProvided; ///< 0/1: no AlphaId/AlphaId
	Boolean			IsConfirmAlphaIdProvided; ///< 0/1: no AlphaId/AlphaId
	SATKNum_t		num;					///< destination number
	SATKString_t	confirmPhaseStr;		///< confirm phase alphaID string
	SATKString_t	setupPhaseStr;			///< setup phase alphaID string
	SATKIcon_t		confirmPhaseIcon;		///< confirm phase icon
	SATKIcon_t		setupPhaseIcon;			///< setup phase icon
	UInt32			duration;				///< retry duration in milisec
    Subaddress_t    subAddr;                ///< Sub Address: First byte contains Type of Sub-Address
    CC_BearerCap_t  bc;                     ///< Capability Configuration
} SetupCall_t;

/// STK send DTMF event type
typedef struct {
	Boolean			isAlphaIdProvided; 		///< 0/1: no AlphaId/AlphaId
	UInt8			dtmf[STK_MAX_DTMF_LEN*2+1];	///< DTMF digits
	SATKString_t	alphaString;			///< alphaID string
	SATKIcon_t		dtmfIcon;				///< DTMF icon
} SendStkDtmf_t;

/// STK Data Service (Open/Close channel, Send/Receive data) event type
typedef struct {
	UInt8			chanID;					///< channel ID (1,2,...)
	SATKDataServiceType_t actionType;		///< Data service action type
	SATKString_t	text;					///< AlphaID string
	SATKIcon_t		icon;					///< icon
	Boolean			isLoginNeeded;			///< is login needed
	Boolean			isPasswdNeeded;			///< is passwd needed
	Boolean			isApnNeeded;			///< is APN needed
} StkDataService_t;


/// Run AT Command notification information 
typedef struct {
	SATKAlphaID_t	text;					///< AlphaID string
	SATKIcon_t		icon;					///< icon
} StkRunAtCmd_t;

/// Language Notification information 
typedef struct 
{
	char language[3];	///< Null terminated string: two characters to specify the language (e.g. "en" for English, see ISO 639 and Section 8.45 of ETSI TS 102.223). 
						// Null string indicates no language is specified.
} StkLangNotification_t;


/// STK Setup menu event type
typedef struct {
	SATKString_t	title;					///< title
	SATKIcon_t		titleIcon;				///< title icon
	UInt8			pItemIdList[MAX_ITEM];	///< item id list
	SATKString_t	*pItemList;				///< item list string	
	STKIconListId_t	pIconList;				///< icon list
	STKIconListId_t	pNextActIndList;		///< next action indication list
	UInt8			listSize;				///< list size
	Boolean			isHelpAvailable;		///< is help available
} SetupMenu_t;

/// USSD result event type
typedef struct {
	UInt8	result;	///<	SUCCESS	= 1; FAIL = 0	///< opposite to output from simap
} SsUssdResult_t;

/// STK Call control event type
typedef struct {
	Boolean isAlphaIdProvided;	///< is AlphaID provided
	SATKCallControlType_t type;	///< call control type
	SATKString_t alphaText;		///< AlphaID string
} SATKCallControlEvent_t;

/// STK Launch Browser Event data 
typedef struct
{
	SIM_LAUNCH_BROWSER_ACTION_t	browser_action;		///< Browser action

	/* Browser ID information */
	Boolean browser_id_exist;						///< is browser ID exist
	UInt8	browser_id;								///< Only valid if "browser_id_exist" is TRUE

	/* URL data */
	char url[MAX_SIM_URL_LENGTH];					///< URL data string, NULL terminated ASCII data  

	/* Bearer data */
	UInt8 bearer_length;							///< bearer length. Set to 0 if no Bearer data exists 
	UInt8 bearer[MAX_SIM_BEARER_LENGTH];			///< Bearer list in order of priority: SMS: 0x00; CSD: 0x01; USSD: 0x02; GPRS: 0x03 (see Section 12.49 of GSM 11.14)

	/* Provisioning File Reference */
	UInt8 prov_length;								///< Number of elements occupied in "prov_file" 
	ProvisionFile_t prov_file[MAX_SIM_PROV_FILE_NUM];	///< provisioning file

	/* Text string for Gateway/Proxy_Identity */
	SATKString_t text;								///< Text string for Gateway/Proxy_Identity

	/* Alpha identifier */
	SATKString_t alpha_id;							///< AlphaID string

	/* Icon ID */
	SATKIcon_t icon_id;								///< icon ID

    /* Default Alpha ID flag */
    Boolean default_alpha_id;                       ///< TRUE for default Alpha ID

} LaunchBrowserReq_t;






/// STK call connected event type
typedef struct
{
	Boolean is_mo_call;		///< is MO call
	T_TI_PD ti_pd;			///< ti_pd
} StkCallConnectedEvt_t;

/// STK select item event type
typedef struct {
	SATKString_t	title;					///< title
	SATKIcon_t		titleIcon;				///< title icon
	Boolean			isAlphaIdProvided; 		///< 0/1: no AlphaId/AlphaId
	UInt8			pItemIdList[MAX_ITEM];	///< id		
	SATKString_t	*pItemList;				///< text	
	STKIconListId_t	pIconList;				///< icon list
	STKIconListId_t	pNextActIndList;		///< next action item
	UInt8			listSize;				///< list sizefor text and id
	Boolean			isHelpAvailable;		///< is help available
	UInt8			defaultItem;			///< default item
} SelectItem_t;

/// Event Data
typedef union{
	DisplayText_t		*display_text;		///<
	GetInkey_t			*get_inkey;			///<
	GetInput_t			*get_input;			///<
	PlayTone_t			*play_tone;			///<
	Refresh_t			*refresh;			///<
	SelectItem_t		*select_item;		///<
	SendMOSMS_t			*send_short_msg;	///<
	SendSs_t			*send_ss;			///<
	SendUssd_t			*send_ussd;			///<
	SetupCall_t			*setup_call;		///<
	SendStkDtmf_t		*send_dtmf;			///<
	SetupMenu_t			*setup_menu;		///<
	LaunchBrowserReq_t  *launch_browser;	///<
	IdleModeText_t		*idlemode_text;		///<
	StkDataService_t	*dataservicereq;	///<
    StkRunAtCmd_t       *runatcmd;          ///<
	UInt8				*activate_req;
} EventData_t;

/// STK main Event Data struct
typedef struct {
	SATK_EVENTS_t	msgType;		///< STK event message type
	EventData_t		u;				///< STK event data union
} SATK_EventData_t;

/**
SIM toolkit call setup fail result
**/

typedef enum
{
 	STK_NOT_ALLOWED,		///< Call not allowed 
	STK_STK_BUSY,			///< Sim Toolkit is busy, can not setup a call 
	STK_SIM_ERROR,			///< Sim error causes setup fail 
	STK_CALL_BUSY,			///< There is other on-going call setup
	STK_NO_CAPABILITY,		///< Beyond ME capabilities 
	STK_CC_BY_SIM_MODIFIED,	///< Call Control by SIM, modified 
	STK_CC_BY_SIM_NOT_ALLOWED_TEMP,	///< Call Control by SIM, not allowed, temporary 
	STK_CC_BY_SIM_NOT_ALLOWED_PERM	///< Call Control by SIM, not allowed, permanent 

} StkCallSetupFailResult_t;

typedef struct {
	StkCallSetupFailResult_t failRes;  ///< STK Call Setup Fail type
} StkCallSetupFail_t;

/**
STK Call control display
**/
#define ALPHA_ID_LEN		40

typedef enum
{
	CALL_CONTROL_UNDEFINED_TYPE,	///< Undefined type of call 
	CALL_CONTROL_CS_TYPE,			///< Fax/Data/GPRS call 
	CALL_CONTROL_SS_TYPE,			///< Supplementary Service call 
	CALL_CONTROL_USSD_TYPE,			///< USSD call 
	CALL_CONTROL_MO_SMS_TYPE		///< MO SMS call 
} StkCallControl_t;

typedef enum
{
	CC_ALLOWED_NOT_MODIFIED,	///< Call allowed without modification 
	CC_NOT_ALLOWED,				///< Call not allowed 
	CC_ALLOWED_MODIFIED,		///< Call allowed, but call data modified 
	CC_STK_BUSY,				///< Sim Toolkit is busy, the client can retry later 
	CC_SIM_ERROR				///< General SIM error 
} StkCallControlResult_t;

/// Call Control Display
typedef struct
{
	Boolean alphaIdValid;				///< True if Alpha Id is valid
	UInt8 displayTextLen;				///< Alpha id length
	UInt8 displayText[ALPHA_ID_LEN];	///< Alpha id

	//The simple call control data for the client to generate the 
	//user indication information after receiving MSG_STK_CC_DISPLAY_IND
	StkCallControl_t		oldCCType;  ///< Original CC type
	StkCallControl_t		newCCType;  ///< Modified CC type
	StkCallControlResult_t	ccResult;   ///< CC result type

} StkCallControlDisplay_t;

/// Term Profile
typedef struct {
	UInt8 length;					///< Length of Terminal Profile data returned. 
	UInt8* data;					///< Pointer to the Terminal Profile data.
} CAPI2_TermProfile_t; 

/// STK call status
typedef struct {						///< Notification of call status
	CallIndex_t index;					///< Call Index, identifying call
	CallType_t call_type;				///< Call Type
	CallStatus_t status;				///< Call status
}StkReportCallStatus_t;

/** STK Call control setup response.
** Data payload for normal Call Control (non-SS and non_USSD) response message
**/
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;			///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;			///< Call type specified by SIM Call Control 
	
	Boolean addr_changed;				///< TRUE if the called party address is new or changed 
	Boolean bc1_changed;				///< TRUE if the BC1 parameters are new or changed 
	Boolean subaddr_changed;			///< TRUE if the called party subaddress is new or changed 
	Boolean alpha_id_changed;			///< TRUE if alpha identifier is new or changed 
	Boolean bc_repeat_changed;			///< TRUE if BC repeat indicator is new or changed 
	Boolean bc2_changed;				///< TRUE if the BC2 parameters are new or changed 
	
	/* Called party address parameters */
	gsm_TON_t ton;						///< Called party type of number
	gsm_NPI_t npi;						///< Called party nuber plan identifier

	char number[MAX_DIGITS + 1];		///< ASCII encoded dialling number 

	/* BC1 data */
	BearerCapability_t bc1;				///< Bearer capability 1 data

	/* Called Party Subaddress data */
	Subaddress_t subaddr_data;			///< Called part sub address

	/* Alpha Identifier data */
	Boolean alpha_id_valid;				///< TRUE if alpha identifier data is valid */
	UInt8 alpha_id_len;					///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];		///< Alpha ID data

	BearerCapability_t bc2;				///< Bearer capability 2 data
	UInt8 bc_repeat;					///< BC Repeat Indicator

} StkCallControlSetupRsp_t;

/// Data payload for SS Call Control response message 
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;			///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;			///< Call type specified by SIM Call Control 

	Boolean ss_str_changed;				///< TRUE if the SS dialing string is new or changed 

	UInt8	ton_npi;	///< TON/NPI byte: one of UNKNOWN_TON_UNKNOWN_NPI (128), UNKNOWN_TON_ISDN_NPI (129) & INTERNA_TON_ISDN_NPI (145)
	UInt8	ss_len;						///< Length of SS dialing string 
	UInt8	ss_data[MAX_DIGITS / 2];	///< SS dialing string 

	/* Alpha Identifier data */
	Boolean alpha_id_valid;				///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;					///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];		///< Alpha ID data

} StkCallControlSsRsp_t;

/// Data payload for USSD Call Control response message 
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM 
	StkCallControl_t old_type;			///< Call type when we sent the Call Control request 
	StkCallControl_t new_type;			///< Call type specified by SIM Call Control 

	Boolean ussd_str_changed;			///< TRUE if the USSD dialing string is new or changed 

	UInt8	ussd_len;					///< Length of USSD dialing string 
	UInt8	ussd_data[PHASE2_MAX_USSD_STRING_SIZE+1]; ///< USSD dialing string 

	/* Alpha Identifier data */
	Boolean alpha_id_valid;				///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;					///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];		///< Alpha ID data

	UInt8 ussd_data_dcs;				///< dcs for USSD string
} StkCallControlUssdRsp_t;

/// Data payload for MO SMS Call Control response message 
typedef struct
{
	StkCallControlResult_t cc_result;	///< Call Control return result from SIM */

	Boolean sca_changed;				///< TRUE if the service centre number is changed */
	Boolean dest_changed;				///< TRUE if destination number is changed */

	/* Service Center Number data */
	UInt8 sca_toa;						///< Service Center Address type of address
	UInt8 sca_number_len;				///< SCA number length
	UInt8 sca_number[SMS_MAX_DIGITS / 2]; ///<SCA number

	/* Destination number data */
	UInt8 dest_toa;						///< Destination type of address
	UInt8 dest_number_len;				///< Destination number length
	UInt8 dest_number[SMS_MAX_DIGITS / 2];///< Destination number

	/* Alpha Identifier data */
	Boolean alpha_id_valid;				///< TRUE if alpha identifier data is valid 
	UInt8 alpha_id_len;					///< Alpha ID length
	UInt8 alpha_id[ALPHA_ID_LEN];		///< Alpha ID data

} StkCallControlSmsRsp_t;

#define MAX_AT_REQUEST		512
typedef struct
{

	UInt16	length;
	UInt8	request[MAX_AT_REQUEST+2];

} RunIpAtReq_t;

typedef enum
{
    STK_MENU_RES_SUCCESS,
    STK_MENU_RES_CMD_PENDING,
    STK_MENU_RES_SIM_BUSY,
    STK_MENU_RES_ERROR
}STKMenuSelectionResCode_t;

#endif


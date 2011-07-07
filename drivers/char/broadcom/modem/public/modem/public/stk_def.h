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
*   @file   stk_def.h
*
*   @brief  This file contains SATK API related definitions.
*
****************************************************************************/

#ifndef _STK_DEF_H_
#define _STK_DEF_H_

//******************************************************************************
//							include block
//******************************************************************************


#define 		STK_MAX_LIST_SIZE			40  ///< Maximum list size in STK
#define 		MAX_ITEM					40 	///< Maximum item number								
#define			SATK_LOGIN_PASSWD_LEN		20	///< Length of login password									
#define 		SATK_MAX_APN_LEN			32  ///< Maximum length of APN

#define 		STK_MAX_DTMF_LEN			40  ///< Maximum length of DTMF
#define          MAX_SIM_URL_LENGTH          245 ///< Maximum length of URL: 255 - (tag + length + cmd details + device id) + 1 (null termination)
#define 		MAX_SIM_BEARER_LENGTH 		6   ///< Maximum length of SIM bearer
#define 		MAX_SIM_PROV_FILE_NUM 		4   ///< Maximum number of SIM provision file
#define 		MAX_SIM_PROV_FILE_LENGTH 	12  ///< Maximum length of SIM provision file
#define 		STK_MAX_ALPHA_TEXT_LEN		274 ///< Maximum text length of ALPHA: 274byte=(240byte*8bit)/7bit; Fix the shorter display in 27.22.4.22.1(7).

#define 		ALPHA_ID_LEN				40  ///< ID length of ALPHA
#define 		MAX_AT_REQUEST				512 ///< Maximum length of AT request

#define 		MAX_NUM_OF_FILE_IN_CHANGE_LIST	30  ///< Maximum number of files in File Change List of Refresh proactive command

#define 		MAX_STK_BROWSING_STATUS_LEN 32

/* Bit mask for STK event enable flags in "MSG_SIM_MMI_SETUP_EVENT_IND" message. See Section 12.25 of GSM 11.14 */
#define BIT_MASK_MT_CALL_EVENT				0x00000001
#define BIT_MASK_CALL_CONNECTED_EVENT		0x00000002
#define BIT_MASK_CALL_DISCONNECTED_EVENT	0x00000004
#define BIT_MASK_LOCATION_STATUS_EVENT		0x00000008
#define BIT_MASK_USER_ACTIVITY_EVENT		0x00000010
#define BIT_MASK_IDLE_SCREEN_AVAIL_EVENT	0x00000020
#define BIT_MASK_CARD_READER_STATUS_EVENT	0x00000040	 
#define BIT_MASK_LAUGUAGE_SELECTION_EVENT	0x00000080	
#define BIT_MASK_BROWSER_TERMINATION_EVENT	0x00000100	
#define BIT_MASK_DATA_AVAILABLE_EVENT		0x00000200	
#define BIT_MASK_CHANNEL_STATUS_EVENT		0x00000400	

#define BIT_MASK_ACCESS_TECH_CHG_EVENT		0x00000800
#define BIT_MASK_DISP_PARAM_CHG_EVENT		0x00001000
#define BIT_MASK_LOCAL_CONNECTION_EVENT		0x00002000
#define BIT_MASK_NETWORK_SEARCH_MODE_EVENT	0x00004000
#define BIT_MASK_BROWSING_STATUS_EVENT      0x00008000
#define BIT_MASK_FRAMES_INFO_CHG_EVENT      0x00010000

typedef int  T_RAT;

/**
Data Structure

Enum order to be consistent with AT Interface Spec for SATK2.0
**/
typedef enum {
	SATK_EVENT_DISPLAY_TEXT = 0,            ///< Event type is DISPLAY TEXT		
	SATK_EVENT_GET_INKEY = 1,			    ///< Event type is GET KEY	
	SATK_EVENT_GET_INPUT = 2,		        ///< Event type is GET INPUT		
	SATK_EVENT_PLAY_TONE = 3,			    ///< Event type is SYSTEM ITEM
	SATK_EVENT_SELECT_ITEM = 4,			    ///< Event type is SELECT ITEM
	SATK_EVENT_SEND_SS = 5,				    ///< Event type is SEND SS
	SATK_EVENT_SEND_USSD = 6,			    ///< Event type is SEND USSD
	SATK_EVENT_SETUP_CALL = 7,			    ///< Event type is SETUP CALL
	SATK_EVENT_SETUP_MENU = 8,			    ///< Event type is SETUP MENU
	SATK_EVENT_MENU_SELECTION = 9,		    ///< Event type is MENU SELECTION
	SATK_EVENT_REFRESH = 10,			    ///< Event type is REFRESH
	SATK_EVENT_SEND_SHORT_MSG = 11, 	    ///< Event type is SEND SHORT MESSAGE
	SATK_EVENT_SEND_DTMF = 12, 			    ///< Event type is SEND DTMF
	SATK_EVENT_LAUNCH_BROWSER = 13, 	    ///< Event type is LAUNCH BROWSER
	SATK_EVENT_IDLEMODE_TEXT = 14, 		    ///< Event type is IDLE MODE TEXT
	SATK_EVENT_PROV_LOCAL_LANG = 15, 	    ///< Event type is PROVIDE LOCAL LANGUAGE
	SATK_EVENT_DATA_SERVICE_REQ = 16,	    ///< Event type is DATA SERVICE REQUEST
    SATK_EVENT_RUN_AT_CMD = 17,             ///< Event type is RUN AT COMMAND
    SATK_EVENT_BATTERY_STATUS = 18,         ///< Event type is BATTERY STATUS    
    SATK_EVENT_OPEN_CHAN = 19,              ///< Event type is OPEN CHANNEL
    SATK_EVENT_CLOSE_CHAN = 20,             ///< Event type is CLOSE CHANNEL
    SATK_EVENT_SEND_DATA = 21,              ///< Event type is SEND DATA
    SATK_EVENT_RECEIVE_DATA = 22,           ///< Event type is RECEIVE DATA
    SATK_EVENT_CHAN_STATUS_UPDATE = 23,     ///< Event type is CHANNEL STATUS UPDATE
    SATK_EVENT_DATA_AVAILABLE = 24,         ///< Event type is DATA AVAILABLE
    SATK_EVENT_GET_CHAN_STATUS = 25,        ///< Event type is GET CHANNEL STATUS
    SATK_EVENT_ACTIVATE = 26,               ///< Event type is ACTIVATE
    SATK_EVENT_PROV_LOCAL_DATE = 27, 	    ///< Event type is PROVIDE LOCAL DATE & TIME

	SATK_EVENT_INVALID                      ///< Event type is INVALID EVENT

} SATK_EVENTS_t;


/// Result code result1
typedef enum {

	// result code for result-1
	SATK_Result_CmdSuccess 					= 0,	///< Result is Command Success
	SATK_Result_RefreshPerformedWithAdditionalEFsRead = 3,     ///< Result is Refresh Performed with Additoional EFs Read
	SATK_Result_CmdSuccessIconNotDisplayed	= 4,	///< Result is Command Success with Icon Not Displayed
	SATK_Result_CmdSuccessButModifiedBySim	= 5,    ///< Result is command Success but Modified by SIM	 
	SATK_Result_SIMSessionEndSuccess		= 10,   ///< Result is SIM Session Eds Successfully
	SATK_Result_BackwardMove				= 11,   ///< Result is Backward Move
	SATK_Result_NoRspnFromUser				= 12,   ///< Result is No Response from User
	SATK_Result_RequestHelpByUser			= 13,	///< Result is Request Help by User
	SATK_Result_UserEndUSSD					= 14,   ///< Result is User ends USSD
	SATK_Result_MeUnableToProcessCmd		= 20,   ///< Result is ME Unable to Process Command
	SATK_Result_NetUnableToProcessCmd		= 21,   ///< Result is Network Unable to Process Command
	SATK_Result_UserNotAcceptingCallSetup	= 22,   ///< Result is User not Accepting Call Setup
	SATK_Result_UserEndCallBeforeConnect	= 23,   ///< Result is User Ends Call before Connected
	SATK_Result_InteractWithSimCCTempProblem= 25,   ///< Result is Interact with SIM CC (temporary problem)
	SATK_Result_LaunchBrowserGenericError	= 26,   ///< Result is Browser Generic Error: Browser Not Available
	SATK_Result_BeyondMeCapability			= 30,   ///< Result is Beyond ME's Capability
	SATK_Result_TypeUnknownToMe				= 31,   ///< Result is Type Unknown to ME
	SATK_Result_DataUnknownToMe				= 32,   ///< Result is Data Unknown to ME
	SATK_Result_NumberUnknownToMe			= 33,   ///< Result is Number Unknown to ME
	SATK_Result_ValueMissingError			= 36,   ///< Result is Value Missing Error
	SATK_Result_USSDError					= 37,   ///< Result is USSD Error
	SATK_Result_InteractWithSimCCPermProblem= 39,   ///< Result is Interact with SIM CC (permanent problem)
	SATK_Result_User_Busy					= 40,   ///< Result is Network Rejects Call with User Busy Cause
	SATK_Result_Busy_On_Call				= 41,   ///< Result is ME is Busy on Voice Call
	SATK_Result_SSReturnResultError			= 52,   ///< Result is SS Returns Result Error
    SATK_Result_BearerIndependentProtocolErr = 58,  ///< Result is Bearer Independent Protocol Error
 
	SATK_Result_Invalid                     = 0xFF  ///< Result is Invalid Result
} SATK_ResultCode_t;                                


/// Result code result2
typedef enum {
	SATK_Result_NoCause						= 0,   ///< No specific cause
	SATK_Result_ScreenBusy					= 1,   ///< Screen busy
	SATK_Result_BusyOnCall					= 2,   ///< Busy on call
	SATK_Result_BusyOnSS					= 3,   ///< Busy on SS call
	SATK_Result_NoService					= 4,   ///< No Service
	SATK_Result_RrNotGranted				= 6,   ///< Radio resource not granted
	SATK_Result_NotInSpeechCall				= 7,   ///< Not in speech call
	SATK_Result_BusyOnUSSD					= 8,   ///< Busy on USSD call
	SATK_Result_BusyOnDtmf					= 9,   ///< Busy on sending DTMF command
    SATK_Result_NoChannelAvailable          = 10,  ///< No channel available
    SATK_Result_InvalidChannelId            = 11,  ///< Invalid Channel ID
    SATK_Result_ChannelClosed               = 12,  ///< Channel Closed)
    SATK_Result_BufferSizeNotAvailable      = 13,  ///< No Space Available

	SATK_Result_Invalid2                    = 0xFF ///< Invalid result code

} SATK_ResultCode2_t;


/* See definitions in Section 12.6 of ETSI TS 101.267 */
/// SIM Launch Browser
typedef enum
{
	LAUNCH_BROWSER_NO_CONNECT 			= 0,	///< Launch browser witout making a connection, if not already launched
	LAUNCH_BROWSER_CONNECT 				= 1,	///< Launch browser making a connection, if not already launched
	LAUNCH_BROWSER_USE_EXIST 			= 2,	///< Use existing browser, the browser shall not use the active existing secured session
	LAUNCH_BROWSER_CLOSE_EXIST_NORMAL 	= 3,	///< Close existing browser session, launch new browser making a connection
	LAUNCH_BROWSER_CLOSE_EXIST_SECURE 	= 4		///< Close existing browser session, launch new browser using a secure session
} SIM_LAUNCH_BROWSER_ACTION_t;

/// STK string type
typedef struct{
	UInt8			*string;		///< String
	UInt16			len;			///< String length, changed from UInt8 to fix the short display problem in TC27.22.4.22.1(7)
	Unicode_t 		unicode_type;	///< Unicode type
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
	UInt8			Id;                 ///< Icon id
	Boolean			IsSelfExplanatory;  ///< Whether icon is self-explanatory
	Boolean			IsExist;            ///< Whether icon exists
} SATKIcon_t;

/// This type is used by both the Icon List Items and the Next Actiona Item list
typedef struct{
	UInt8			Id[STK_MAX_LIST_SIZE];      ///< Icon IDs in icon list
	Boolean			IsSelfExplanatory;          ///< Whether icon list is self-explanatory
	Boolean			IsExist;                    ///< Whether icon list exists
} STKIconListId_t;

/// STK Data Service Respond 
typedef struct {
	UInt16			clientID;							///< Client ID
	SATK_EVENTS_t	event;								///< Event type
	UInt8           channelID;                          ///< Channel ID
	SATK_ResultCode_t result1;							///< Result code 1
	UInt16			result2;							///< Result code 2
    UInt16			chanStatus;							///< Result code 3: Channel Status in UICC server mode
	UInt8			textStr1[SATK_LOGIN_PASSWD_LEN+1];	///< Login
	UInt8			textStr2[SATK_LOGIN_PASSWD_LEN+1];	///< Password
	UInt8			textStr3[SATK_MAX_APN_LEN+1];		///< APN for GPRS bearer
	UInt16          dataLen;                            ///< Receive Data length
    UInt8*          dataBuf;                            ///< Receive Data Buffer
    UInt16          bearerDataLen;                      ///< Bearer Data Len
    UInt8           bearerType;                         ///< Bearer Type
    UInt16          numBytesAvail;                      ///< Number of bytes Available
} StkCmdRespond_t;

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

/// Provisioning File Reference data 
typedef struct
{
	UInt8 length;	                                  ///< Length of bytes occupied in "prov_file"
	UInt8 prov_file_data[MAX_SIM_PROV_FILE_LENGTH];   ///< Data content of provision file
} ProvisionFile_t;

/// Stk Call Control
typedef enum
{
	CALL_CONTROL_UNDEFINED_TYPE,	///< Undefined type of call 
	CALL_CONTROL_CS_TYPE,			///< Fax/Data/GPRS call 
	CALL_CONTROL_SS_TYPE,			///< Supplementary Service call 
	CALL_CONTROL_USSD_TYPE,			///< USSD call 
	CALL_CONTROL_MO_SMS_TYPE		///< MO SMS call 
} StkCallControl_t;

/// Stk Call Control result
typedef enum
{
	CC_ALLOWED_NOT_MODIFIED,	///< Call allowed without modification 
	CC_NOT_ALLOWED,				///< Call not allowed 
	CC_ALLOWED_MODIFIED,		///< Call allowed, but call data modified 
	CC_STK_BUSY,				///< Sim Toolkit is busy, the client can retry later 
	CC_SIM_ERROR				///< General SIM error 
} StkCallControlResult_t;

/// STK display text event type
typedef struct {
	SATKString_t	stkStr;			///< data
	Boolean			isHighPrio;		///< priority
	SATKIcon_t		icon;			///< icon
	Boolean			isDelay;		///< clear
	Boolean			isSustained;	///< TRUE for Sustained Display Text for which Terminal Response must be sent without delay: see Section 6.4 of ETSI TS 102 223.
    UInt32          duration;       ///< Variable timeout duration in seconds. 0 = duration info unavailable.
} DisplayText_t;

/// Idle Mode text event type
typedef struct {
	SATKString_t	stkStr;			///< data
	SATKIcon_t		icon;			///< icon
} IdleModeText_t;

/// STK get inkey response type
typedef enum{
	S_IKT_YESNO,		    ///< Inkey response type is YES/NO
	S_IKT_DIGIT,		    ///< Inkey response type is 0-9,*,+,#
	S_IKT_SMSDEFAULTSET,    ///< Inkey response type is SMS default set
	S_IKT_UCS2		        ///< Inkey response type is UCS2	
} SATKInKeyType_t;

/// STK Get Input Key event type
typedef struct {
	SATKString_t	stkStr;				///< data
	SATKIcon_t		icon;				///< icon
	SATKInKeyType_t	inKeyType;			///< input,  alphabet and resp
	Boolean			isHelpAvailable;	///< is help available
    UInt32          duration;           ///< Variable timeout duration in seconds. 0 = duration info unavailable.
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

/// STK tone type
typedef enum{
	S_TT_DEFAULTTONE			= 0x00,     ///< Default tone
	S_TT_DIALTONE				= 0x01,     ///< Dial tone
	S_TT_CALLEDUSERBUSY			= 0x02,     ///< Called user busy
	S_TT_CONGEST				= 0x03,     ///< Congestion tone
	S_TT_RADIOPATHACK			= 0x04,     ///< Radio path acknowledgement
	S_TT_RADIOPATHUNAVAILABLE	= 0x05,	    ///< Radio path not available
	S_TT_ERROR					= 0x06,     ///< Error
	S_TT_CALLWAITING			= 0x07,     ///< Call waiting tone
	S_TT_RINGING				= 0x08,     ///< Ring tone
	S_TT_BEEP					= 0x10,     ///< Beep
	S_TT_POSITIVEACK			= 0x11,     ///< Positive acknowledgement
	S_TT_NEGTIVEACK				= 0x12      ///< Negative acknowledgement
} SATKToneType_t;

/// STK playtone event type
typedef struct {
	SATKString_t	stkStr;		///< alpha id
    Boolean         defaultStr; ///< TRUE is default Alpa ID is used
	SATKToneType_t	toneType;	///< tone
	UInt32			duration;	///< milliseconds unit and 
	SATKIcon_t		icon;		///< icon
} PlayTone_t;

/* Do not change the values of the following enum. They are defined according to assigned values  
 * for the different Refresh types defined in Section 12.6 of ETSI TS 101 267 (GSM 11.14).
 */
typedef enum{
	SMRT_INIT_FULLFILE_CHANGED = 0,         ///< Refresh command type is INIT Full-file Changed
	SMRT_FILE_CHANGED = 1,                  ///< Refresh command type is File Changed
	SMRT_INIT_FILE_CHANGED = 2,             ///< Refresh command type is INIT File Changed
	SMRT_INIT = 3,                          ///< Refresh command type is INIT
	SMRT_RESET = 4,                         ///< Refresh command type is Reset
    SMRT_APP_RESET = 5,                     ///< Refresh command type is Application Reset
	SMRT_SESSION_RESET =6,                  ///< Refresh command type is Session Reset
	SMRT_STEERING_OF_ROAMING=7,             ///< Refresh command type is Steering of Roaming
	SMRT_STEERING_OF_ROAMING_WLAN=8,        ///< Refresh command type is Steering of Roaming for I-WLAN
	SMRT_INVALID=0xFF                       ///< Refresh command type is invalid
} STKRefreshType_t;

/// Refresh file
typedef struct
{
	UInt8 path_len;		                        ///< Number of file ID's in "file_path"
	UInt16 file_path[MAX_SIM_FILE_PATH_LEN];	///< SIM file ID path starting from MF, i.e. "sim_file_path[0]" is always 0x3F00 
} REFRESH_FILE_t;

/// Structure to store the File Change List of Refresh proactive command
typedef struct
{
	UInt8 			number_of_file;             ///< Number of files in the list
	REFRESH_FILE_t	changed_file[MAX_NUM_OF_FILE_IN_CHANGE_LIST];   ///< Changed file list
} REFRESH_FILE_LIST_t;


/// STK refresh event type
typedef struct {
	STKRefreshType_t	refreshType;				///< refresh type
	REFRESH_FILE_LIST_t	FileIdList;					///< This is file ID array in refresh command
	USIM_APPLICATION_TYPE appliType;				///< Application for the refresh specified in AID IE (see Section 6.6.13 of ETSI 102 223); 
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

/// STK Send SS type
typedef enum{
	S_SST_SS_TYPE				= 0,        ///< SS type
	S_SST_USSD_TYPE_EARLY_USAGE	= 1,        ///< USSD type with early usage
	S_SST_USSD_TYPE		   		= 2,        ///< USSD type
	S_SST_NO_SS_TYPE			= 3         ///< No SS type    
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

/// STK setup call type
typedef enum{
	S_CT_ONIDLE,                            ///< Call type on idle					
	S_CT_ONIDLE_REDIALABLE,		            ///< Call type on idle and rediable
	S_CT_HOLDABLE,					        ///< Call type holdable
	S_CT_HOLDABLE_REDIALABLE,	            ///< Call type holdable and redialable
	S_CT_DISCONNECTABLE,			        ///< Call type disconnectable
	S_CT_DISCONNECTABLE_REDIALABLE          ///< Call type disconnectable and rediable
} SATKCallType_t;



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
	SATKString_t	alphaString;				///< alphaID string
	SATKIcon_t		dtmfIcon;				///< DTMF icon
} SendStkDtmf_t;

/// Data Service type defined by BRCM AT command document 
typedef enum{
	SATK_DATA_SERV_OPEN = 1,		
	SATK_DATA_SERV_SEND_DATA = 2,	
	SATK_DATA_SERV_REC_DATA = 3,	
	SATK_DATA_SERV_CLOSE = 4		
} SATKDataServiceType_t;

typedef enum{	
	SATK_DATA_TCP_CLOSE_TO_CLOSED, ///< UICC server mode, close TCP and go to CLOSED state
	SATK_DATA_TCP_CLOSE_TO_LISTEN  ///< UICC server mode, close TCP and go to LISTEN state
} SATKDataCloseType_t;


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

/**
Possible bearer type defined in Bearer Description TLV: Section 12.52 of GSM 11.14
**/
typedef enum
{
	CSD_BEARER,
	GPRS_BEARER,
    UNDEF_BEARER
} STK_DATA_SERV_BEARER_TYPE_t;

/**
Open Channel Specific Parameters
**/
#define STK_MAX_APN_LEN 32
#define STK_IPV4_ADDRESS_LEN 4
typedef struct
{
    Boolean		immediate_establish;	///< TRUE if channel is immediately established when "Open Channel" is received
	Boolean		auto_reconnect;			///< TRUE if reconnect attempt is desired
    Boolean		transparent_mode;       ///< TRUE if channel is in transparent mode per Cingular requirements, i.e. the length
										///< in the received Alpha ID TLV is 0 in "Open Channel" request. When in Transparent mode, 
										///< the user confirmation phase for "Open Channel" is skipped.
	STK_DATA_SERV_BEARER_TYPE_t	bearer_type;	///< Bearer type requested
    UInt8       bearer_data_len;        ///< Requested bearer data len
    UInt8       bearer_data[8];         ///< Requested bearer data
	SATKString_t	user_login;			///< User login ID for CSD/GPRS call
	SATKString_t	login_pwd;	        ///< Login password for CSD/GPRS call
	Boolean		static_ip_addr_exist;	///< TRUE if static IP address is passed from SIM in Transport Level TLV
	UInt8		static_ip_addr[STK_IPV4_ADDRESS_LEN];	///< IP address
	Boolean		dest_ip_addr_exist;		///< TRUE if Destination IP address is passed from SIM
	UInt8		dest_ip_addr[STK_IPV4_ADDRESS_LEN];		///< Destination IP address
    UInt16		buffer_size;			///< Size of Tx/Rx buffers
    UInt8	    transport_layer; ///< Transport layer information: SI_TR_LAYER_RAW, SI_TR_LAYER_UDP, SI_TR_LAYER_TCP
    UInt16		port_num;	            ///< Port number to use for setting up UDP/TCP communication
    char		apn_name[STK_MAX_APN_LEN];   ///< APN name
    char		pdp_type[3];			///< PDP type: only support "IP"
    Boolean		dataCompression;	    ///< Data Compression enabled/disabled	
	Boolean		headerCompression;	    ///< Header Compression enabled/disabled
    UInt8	    qos_precedence;			///<  0 - MAX_QOS_PRECEDENCE
	UInt8	    qos_delay;				///<  0 - MAX_QOS_DELAY
	UInt8       qos_reliability;		///<  0 - MAX_QOS_RELIABILITY
	UInt8	    qos_peak;				///<  0 - MAX_QOS_PEAK
	UInt8	    qos_mean;				///<  0 - MAX_QOS_MEAN0
} SATK_OPEN_CHAN_PARAM_t;


/**
Open channel specific parameters (UICC server mode)
**/
typedef struct
{
    UInt16		buffer_size;			///< Size of Tx/Rx buffers    
    UInt16		port_num;	            ///< Port number to use for setting up UDP/TCP communication

} SATK_OPEN_CHAN_SVR_PARAM_t;


/**
Send Data Specific Parameters
**/
typedef struct
{
    UInt8*          dataBuf;                ///< Send Data buffer
    UInt16          dataLen;                ///< Length of data
    Boolean         send_data_immediately;  ///< TRUE for immediate sending
} SATK_SEND_DATA_PARAM_t;

/**
Data Service Request Specific Parameters
**/
typedef union
{
    SATK_OPEN_CHAN_PARAM_t open_ch;
	SATK_OPEN_CHAN_SVR_PARAM_t open_ch_svr;
    SATK_SEND_DATA_PARAM_t send_data;
} SATK_DATA_SERV_REQ_PARAM_t;

/**
Data Service Channel State. This should match T_SI_CHANNEL_CHANGED_STATE in stack
**/
typedef enum
{
    SATK_CHAN_STATE_NO_INFO,
    SATK_CHAN_STATE_NOT_USED_1,
    SATK_CHAN_STATE_NOT_USED_2,
    SATK_CHAN_STATE_NOT_USED_3,
    SATK_CHAN_STATE_NOT_USED_4,
    SATK_CHAN_STATE_CHANGED_TO_DROPPED
}SATK_CHAN_STATE_t;


/// STK Data Service (Open/Close channel, Send/Receive data) event type
typedef struct {
	UInt8			chanID;					///< channel ID (1,2,...)
	SATKDataServiceType_t actionType;		///< Data service action type
	SATKString_t	text;					///< AlphaID string
	SATKIcon_t		icon;					///< icon
	Boolean			isLoginNeeded;			///< is login needed
	Boolean			isPasswdNeeded;			///< is passwd needed
	Boolean			isApnNeeded;			///< is APN needed
    UInt8           dataLen;                ///< Data Length
    Boolean			is_UICC_server_mode;    ///< is UICC server mode
	SATKDataCloseType_t server_close_type;  ///< Channel Close type of UICC server mode
    Boolean         dsReqParamValid;        ///< TRUE if DS Req specific parameters are valid
    SATK_DATA_SERV_REQ_PARAM_t u;           ///< Data Service Request Specific Parameters   
} StkDataService_t;

typedef struct{
	UInt16			Len; // BRCM/Davis Zhu 7/25/2005: Changed UInt8 to UInt16 for Len > 255, such as 274 in TC27.22.4.22.1(7).
	Unicode_t		CodingType;
	UInt8			String[STK_MAX_ALPHA_TEXT_LEN+1];
} STKTxt_t;

/// Send terminal response
typedef struct
{

	SATK_EVENTS_t	msgType;		///< STK event response message type
    UInt8           channelID;      ///< STK event response channel id
	UInt16			parm1;			///< STK event response parm1
	UInt16			parm2;			///< STK event response parm2
	STKTxt_t		respStr;		///< STK event response string1
	STKTxt_t		respStr2;		///< STK event response string2
	STKTxt_t		respStr3;		///< STK event response string3
    UInt16          parm3;          ///< STK event response parm3
    UInt8*          parmBuf;        ///< STK event response buffer   
    UInt16          parm4;          ///< STK event response parm4
} StkSendTermRespData_t;


// Data Service Channel Socket State. This should match T_SI_CHANNEL_SOCKET_STATE in stack
typedef enum
{
    SATK_CHANNEL_LINK_NOT_ESTABLISHED,      ///< client mode: link not established
    SATK_CHANNEL_LINK_ESTABLISHED,          ///< client mode: link established
    SATK_CHANNEL_TCP_CLOSED,                ///< server mode: TCP closed
    SATK_CHANNEL_TCP_LISTEN,                ///< server mode: TCP listen
    SATK_CHANNEL_TCP_ESTABLISHED,           ///< server mode: TCP established
    SATK_CHANNEL_RESERVED                   ///< server mode: reserved
}SATK_CHAN_SOCKET_t;


/// Send Data Service request (Envelope Cmd) 
typedef struct 
{
	SATK_EVENTS_t	event;			    ///< Event type
    UInt8           channelID;          ///< Channel ID
    Boolean         linkEstablished;    ///< link established
	SATK_CHAN_SOCKET_t   chanSocket;    ///< channel socket state
    SATK_CHAN_STATE_t chanState;        ///< link state (dropped/no change)
    UInt16          numBytesAvail;      ///< num of bytes available    
} StkDataServReq_t;


/// Launch Browser Request
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
    Boolean default_alpha_id;                       ///< TRUE for default test, otherwise text belongs to SIM

} LaunchBrowserReq_t;

/// Run AT Command notification information 
typedef struct {
	SATKAlphaID_t	text;					///< AlphaID string
	SATKIcon_t		icon;					///< icon
} StkRunAtCmd_t;


/// Event Data
typedef union{
	DisplayText_t		*display_text;      ///< Display text		
	GetInkey_t			*get_inkey;			///< Get inkey
	GetInput_t			*get_input;			///< Get input
	PlayTone_t			*play_tone;			///< Play tone
	Refresh_t			*refresh;			///< Refresh
	SelectItem_t		*select_item;		///< Select item
	SendMOSMS_t			*send_short_msg;	///< Send MO SMS
	SendSs_t			*send_ss;			///< Send SS
	SendUssd_t			*send_ussd;			///< Send USSD
	SetupCall_t			*setup_call;		///< Setup call
	SendStkDtmf_t		*send_dtmf;			///< Send DTMF
	SetupMenu_t			*setup_menu;		///< Setup menu
	LaunchBrowserReq_t *launch_browser;		///< Launch browser request
	IdleModeText_t		*idlemode_text;		///< Idle mode text
	StkDataService_t	*dataservicereq;	///< Data service
    StkRunAtCmd_t       *runatcmd;          ///< Run AT command
	UInt8               *activate_req;		///< Activate command
} EventData_t;

/// STK main Event Data struct
typedef struct {
	SATK_EVENTS_t	msgType;		///< STK event message type
	EventData_t		u;				///< STK event data union
} SATK_EventData_t;

/// SIM toolkit call setup fail result
typedef enum
{
 	STK_NOT_ALLOWED,				///< Call not allowed 
	STK_STK_BUSY,					///< Sim Toolkit is busy, can not setup a call 
	STK_SIM_ERROR,					///< Sim error causes setup fail 
	STK_CALL_BUSY,					///< There is other on-going call setup
	STK_NO_CAPABILITY,				///< Beyond ME capabilities 
	STK_CC_BY_SIM_MODIFIED,			///< Call Control by SIM, modified 
	STK_CC_BY_SIM_NOT_ALLOWED_TEMP,	///< Call Control by SIM, not allowed, temporary 
	STK_CC_BY_SIM_NOT_ALLOWED_PERM	///< Call Control by SIM, not allowed, permanent 

} StkCallSetupFailResult_t;

/// SIM toolkit call setup failure
typedef struct {
	StkCallSetupFailResult_t failRes;   ///< Call setup failure result
} StkCallSetupFail_t;

/// SIM toolkit call control display
typedef struct
{
	Boolean 	alphaIdValid;           ///< Whether alpha id is valid
	UInt8 	displayTextLen;             ///< Length of display text
	UInt8 	displayText[ALPHA_ID_LEN];  ///< Content of display text

	/* The simple call control data for the client to generate the 
	 * user indication information after receiving MSG_STK_CC_DISPLAY_IND
	 */ 
	StkCallControl_t		oldCCType;  ///< Old CC type
	StkCallControl_t		newCCType;  ///< New CC type
	StkCallControlResult_t	ccResult;   ///< CC result

	gsm_TON_t ton;	///< Called party type of number
	gsm_NPI_t npi;	///< Called party nuber plan identifier
	char number[MAX_DIGITS + 1];	///< ASCII encoded dialling number 

} StkCallControlDisplay_t;

/// Language Notification information 
typedef struct 
{
	char language[3];	///< Null terminated string: two characters to specify the language (e.g. "en" for English, see ISO 639 and Section 8.45 of ETSI TS 102.223). 
						   ///< Null string indicates no language is specified.
} StkLangNotification_t;

/// Request to run AT command
typedef struct {
	UInt16	length;                     ///< Length of request
	UInt8	request[MAX_AT_REQUEST+2];  ///< Content of request
}RunAT_Request;

/// STK Envelope Response Data
typedef struct {
	UInt8 sw1;				///< SW1 received from SIM
	UInt8 sw2;		        ///< SW2 received from SIM
	UInt16 rsp_len;			///< Number of bytes in "rsp_data"
	UInt8 rsp_data[256];	///< Response data for "Get Response" as a result of sending Envelope
							 ///< command to SIM, encoded in raw GSM 11.14 format, excluding the 
							 ///< SW1/SW2 bytes at the end.
}EnvelopeRspData_t;

/// STK Terminal-Response Response Data
typedef struct {
	UInt8 sw1;				///< SW1 received from SIM
	UInt8 sw2;		        ///< SW2 received from SIM
}TerminalResponseRspData_t;

/// STK call status
typedef struct {						///< Notification of call status
	CallIndex_t index;					///< Call Index, identifying call
	CallType_t call_type;				///< Call Type
	CallStatus_t status;				///< Call status
}StkReportCallStatus_t;

typedef enum
{
    STK_MENU_RES_SUCCESS,
    STK_MENU_RES_CMD_PENDING,
    STK_MENU_RES_SIM_BUSY,
    STK_MENU_RES_ERROR
}STKMenuSelectionResCode_t;

/// STK status
typedef enum
{
	STK_STATUS_SUCCESS,
	STK_STATUS_ERROR
}StkStatus_t;

/// STK card type
typedef enum
{
	STK_CARD_TYPE_2G,
	STK_CARD_TYPE_3G,
	STK_CARD_TYPE_INVALID
}StkCardType_t;

/// Data payload for STK terminal profile request message
typedef struct
{
	UInt8 tpDataLen;
	UInt8 tpData[1];
} StkTermProfileReq_t;

/// Data payload for STK terminal profile indication message
typedef struct
{
	StkStatus_t status;
}StkTermProfileInd_t;

/// Data payload for STK UICC CAT indication message
typedef struct
{
	StkCardType_t cardType;
	Boolean isTpRequired;
}StkUiccCatInd_t;

/// payload for MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP
typedef struct
{
	StkStatus_t result;
}StkProactiveCmdFetchingOnOffRsp_t;

/// Data payload for STK external proactive command request message
typedef struct
{
	void* sim_access_cb;
	UInt8 dataLen;
	UInt8 data[1];
} StkExtProactiveCmdReq_t;

/// Proactive command data from SIM 
typedef struct
{
	UInt16 data_len; ///< Number of bytes in "data" 	
	UInt8 data[256]; ///< Proactive command data in raw GSM 11.14 format, i.e. data
					  ///< as the response to Fetch command, excluding the SW1/SW2 bytes at the end.
					  
} ProactiveCmdData_t;

/// payload for MSG_STK_PROACTIVE_CMD_FETCHING_ON_OFF_RSP
typedef struct
{
	StkStatus_t result;
}StkPollingIntervalRsp_t;

/// Data payload for STK send external proactive command indication message
typedef struct
{
	UInt8 sw1;
	UInt8 dataLen;
	UInt8 data[256];
}StkExtProactiveCmdInd_t;

typedef struct{
	UInt8			Len;
	Unicode_t		CodingType;
	Boolean			DefaultDisplay;	
	UInt8			String[STK_MAX_ALPHA_TEXT_LEN+1];
} STKAlphaIdentifier_t;

typedef struct{
	UInt8			Id;
	Boolean			IsSelfExplanatory;
	Boolean			IsExist;
} STKIconId_t;

/**
STK Launch Browser Event data 
**/
typedef struct
{
	/* Browser action */
	SIM_LAUNCH_BROWSER_ACTION_t	browser_action;

	/* Browser ID information */
	Boolean browser_id_exist;
	UInt8	browser_id;		/* Only valid if "browser_id_exist" is TRUE */

	/* URL data */
	char url[MAX_SIM_URL_LENGTH];	///< NULL terminated ASCII data  

	/* Bearer data */
	UInt8 bearer_length;	///< Set to 0 if no Bearer data exists 
	UInt8 bearer[MAX_SIM_BEARER_LENGTH];	// Bearer list in order of priority: SMS: 0x00; CSD: 0x01; USSD: 0x02; GPRS: 0x03 (see Section 12.49 of GSM 11.14)

	/* Provisioning File Reference */
	UInt8 prov_length;		///< Number of elements occupied in "prov_file" 
	ProvisionFile_t prov_file[MAX_SIM_PROV_FILE_NUM];	

	/* Text string for Gateway/Proxy_Identity */
	STKTxt_t text;

	/* Alpha identifier */
	STKAlphaIdentifier_t alpha_id;

	/* Icon identifier */
	STKIconId_t icon_id;

} STKLaunchBrowserReq_t;

#endif // _STK_DEF_H_


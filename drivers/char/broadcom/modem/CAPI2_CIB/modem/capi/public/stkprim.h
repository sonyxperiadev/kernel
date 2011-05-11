   
//***************************************************************************
//
//	Copyright © 2005-2008 Broadcom Corporation
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
*   @file   stkprim.h
*
*   @brief  This file contains function prototypes for interface between
*		   Broadcom platform and SIMAP/USIMAP. 
*
****************************************************************************/

#ifndef _STKPRIM_H_
#define _STKPRIM_H_



/* See Section 13 of ETSI TS 101 267 (GSM 11.14) for the following tag values */
#define PROACTIVE_CMD_TAG	0xD0	///< Proactive Command tag
#define CMD_DETAILS_TAG		0x01	///< Command Details tag
#define DEV_IDENTITY_TAG	0x02	///< Device Identity tag
#define RESULT_TAG			0x03	///< Result tag
#define FILE_LIST_TAG		0x12	///< File List tag
#define AID_TAG				0x2F	///< AID tag
#define TAG_COMPRE_REQ		0x80	///< Comprehension required

///< Proactive Command Type (Section 13.4 of GSM 11.14)
#define REFRESH_CMD_TYPE				0x01	
#define MORE_TIME_CMD_TYPE				0x02	
#define POLL_INTERVAL_CMD_TYPE			0x03	
#define POLLING_OFF_CMD_TYPE			0x04	
#define SETUP_EVENT_LIST_CMD_TYPE		0x05	
#define SETUP_CALL_CMD_TYPE				0x10
#define SEND_SS_CMD_TYPE				0x11	
#define SEND_USSD_CMD_TYPE				0x12	
#define SEND_SMS_CMD_TYPE				0x13	
#define SEND_DTMF_CMD_TYPE				0x14	
#define LAUNCH_BROWSER_CMD_TYPE			0x15	
#define PLAY_TONE_CMD_TYPE				0x20	
#define DISPLAY_TEXT_CMD_TYPE			0x21	
#define GET_INKEY_CMD_TYPE				0x22
#define GET_INPUT_CMD_TYPE				0x23	
#define SELECT_ITEM_CMD_TYPE			0x24	
#define SETUP_MENU_CMD_TYPE				0x25	
#define PROVIDE_LOCAL_INFO_CMD_TYPE		0x26	
#define TIMER_MANAGEMENT_CMD_TYPE		0x27	
#define SETUP_IDLE_MODE_TEXT_CMD_TYPE	0x28	
#define PERFORM_CARD_APDU_CMD_TYPE		0x30	
#define POWER_ON_CARD_CMD_TYPE			0x31
#define POWER_OFF_CARD_CMD_TYPE			0x32	
#define GET_READER_STATUS_CMD_TYPE		0x33	
#define RUN_AT_COMMAND_CMD_TYPE			0x34
#define LANGUAGE_NOTIFICATION_CMD_TYPE	0x35
#define OPEN_CHANNEL_CMD_TYPE			0x40
#define CLOSE_CHANNEL_CMD_TYPE			0x41
#define RECEIVE_DATA_CMD_TYPE			0x42
#define SEND_DATA_CMD_TYPE				0x43
#define GET_CHANNEL_STATUS_CMD_TYPE		0x44

///< Provide Local Information type defined in command qualifier
#define PROVIDE_LOCAL_INFO_LOCATION_INFO		0x00
#define PROVIDE_LOCAL_INFO_IMEI					0x01
#define PROVIDE_LOCAL_INFO_NETWORK_MEASURE		0x02
#define PROVIDE_LOCAL_INFO_DATE_TIME			0x03
#define PROVIDE_LOCAL_INFO_LANGUAGE				0x04
#define PROVIDE_LOCAL_INFO_ACCESS_TECH			0x06
#define PROVIDE_LOCAL_INFO_ESN					0x07
#define PROVIDE_LOCAL_INFO_IMEISV				0x08
#define PROVIDE_LOCAL_INFO_SEARCH_MODE			0x09

///< SIM Refresh type defined in command qualifier
#define REFRESH_SIM_INIT_FULL_FILE_CHG		0x00
#define REFRESH_FILE_CHG					0x01
#define REFRESH_SIM_INIT_FILE_CHG			0x02
#define REFRESH_SIM_INIT					0x03
#define REFRESH_SIM_RESET					0x04

///< Network Search Mode
#define STKPRIM_NW_SEARCH_MODE_AUTO         0
#define STKPRIM_NW_SEARCH_MODE_MANUAL       1
#define STKPRIM_NW_SEARCH_MODE_INVALID      0xFF

typedef enum
{
	C_SUCCESSFUL						= SUCCESSFUL,
	C_SUCCESSFUL_ICON_NOT_DISPLAYED		= SUCCESSFUL_ICON_NOT_DISPLAYED,
	C_SUCCESSFUL_MODIFIED_BY_CC			= SUCCESSFUL_MODIFIED_BY_CALL_CONTROL,
	C_TERMINATE_BY_USER					= TERMINATE_BY_USER,
	C_BACKWARD_MOVE						= BACKWARD_MOVE,
	C_NO_RESPONSE						= NO_RESPONSE,
	C_SCREEN_BUSY						= SCREEN_BUSY,
	C_HELP_INFO							= HELP_INFO,
	C_COMMAND_BEYOND_ME_CAPABILITY		= COMMAND_BEYOND_ME_CAPABILITY,
	C_ME_CURRENTLY_UNABLE				= ME_CURRENTLY_UNABLE,
    C_ME_CURRENTLY_UNABLE_NO_SERVICE    = ME_CURRENTLY_UNABLE_NO_SERVICE,
	C_BUSY_ON_CALL						= BUSY_ON_CALL,
	C_REFRESH_WITH_ADDITIONAL_EF_READ	= COMMAND_REFRESH_WITH_ADDITIONAL_EF_READ,

	C_BROWSER_NOT_AVAILABLE				= BROWSER_NOT_AVAILABLE,
	C_CMD_DATA_UNKNOWN					= MI_CMD_DATA_UNKNOWN,

	C_NOT_ALLOWED_BY_CC					= NOT_ALLOWED_BY_CALL_CONTROL,

	/* Failure causes specifically for MN oriented features */
	C_MN_NOT_IN_SPEECH_CALL								= MN_NOT_IN_SPEECH_CALL,
	C_MN_SIMTK_SETUP_MODE_NOT_SUPPORTED					= MN_SIMTK_SETUP_MODE_NOT_SUPPORTED,
	C_MN_MMI_NOT_REGISTERED								= MN_MMI_NOT_REGISTERED,
	C_MN_SIMTK_SETUP_REJ_DUE_TO_MISSING_ACM_INFO_IND	= MN_SIMTK_SETUP_REJ_DUE_TO_MISSING_ACM_INFO_IND,
	C_MN_SIMTK_SETUP_REJECTED_BY_THE_USER				= MN_SIMTK_SETUP_REJECTED_BY_THE_USER,
	C_MN_SIMTK_SETUP_REJECTED_TIME_OUT					= MN_SIMTK_SETUP_REJECTED_TIME_OUT,
	C_MN_SIMTK_CALL_CONNECTED							= MN_SIMTK_CALL_CONNECTED,
	C_MN_SIMTK_DTMF_TRANSMITTED							= MN_SIMTK_DTMF_TRANSMITTED,
	C_MN_SIMTK_DTMF_TRANSMITTED_NO_ICON_DISPLAY			= MN_SIMTK_DTMF_TRANSMITTED_NO_ICON_DISPLAY,
	C_MN_DISCONNECT_DUE_TO_SIMTK_SETUP					= MN_DISCONNECT_DUE_TO_SIMTK_SETUP,
	C_MN_SIMTK_SETUP_REJ_DUE_TO_PENDING_SIMTK_SETUP		= MN_SIMTK_SETUP_REJ_DUE_TO_PENDING_SIMTK_SETUP,
	C_MN_SIMTK_CALL_CONNECTED_NO_ICON_DISPLAY			= MN_SIMTK_CALL_CONNECTED_NO_ICON_DISPLAY,
	C_MN_SPEECH_CALL_ACTIVE								= MN_SPEECH_CALL_ACTIVE,
	C_MN_SIMTK_SETUP_USER_CLEARED_BEFORE_CONNECT		= MN_SIMTK_SETUP_USER_CLEARED_BEFORE_CONNECT,
	C_MN_SIMTK_SETUP_REJ_DUE_TO_NET_UNABLE				= MN_SIMTK_SETUP_REJ_DUE_TO_NET_UNABLE,
	C_MN_SIMTK_SS_ACTIVE								= MN_USSD_BUSY,

	/* "MN_DISCONNECT_DUE_TO_SIMTK_SETUP" causes SIMAP/USIMAP to send "ME Unable" with no specific cause in Terminal Response */
	C_MN_SIMTK_ME_UNABLE								= MN_DISCONNECT_DUE_TO_SIMTK_SETUP, 
		
	/* Pass this status when we receive "User Busy" cause value from the network in order to send "User Busy" cause 
	 * value in the additional result in Terminal Response. This is required in order to pass TC 27.22.4.13.1.
	 */							
	C_MN_SIMTK_USER_BUSY								= MN_USER_BUSY,	

	C_MN_SIMTK_DATA_UNKNOWN								= MN_INVALID_PARAM,
	C_MN_SIMTK_TERMINATE_BY_USER						= MN_SIMTK_SETUP_TERMINATE_BY_USER,
	C_MN_SIMTK_CC_BY_SIM_MODIFIED						= MN_SIMTK_CC_BY_SIM_MODIFIED,
	C_MN_SIMTK_CC_BY_SIM_TEMP_PROBLEM					= MN_SIMTK_CC_BY_SIM_TEMP_PROBLEM,
	C_MN_SIMTK_CC_BY_SIM_PERM_PROBLEM					= MN_SIMTK_CC_BY_SIM_PERM_PROBLEM,
    C_MN_SIMTK_FACILITY_REJECTED                        = MN_FACILITY_REJECTED

} T_MI_PROACTIVE_CAUSE_t;

/* Valid minimum and maximum Channel ID value defined in Section 8.7 of GSM 11.14 */
#define MIN_CHANNEL_ID	1
#define MAX_CHANNEL_ID	7

/* Result for STK Data Service commands */
typedef enum
{
	STK_DATA_SERV_SUCCESS,					/* Command successfully performed */
	STK_DATA_SERV_CMD_PERFORMED_MODIFIED,	/* Command performed with modified paramters */
	STK_DATA_SERV_CMD_BEYOND_ME_CAPABILITY,	/* Command beyond ME's capability */
	STK_DATA_SERV_ME_UNABLE_PROC_CMD,		/* ME currently unable to perform the command */
	STK_DATA_SERV_NETWORK_UNABLE_PROC_CMD,	/* Network currently unable to process the command */
	STK_DATA_SERV_NO_CHANNEL_LEFT,			/* There is no channel available */
	STK_DATA_SERV_USER_NOT_ACCEPT_CALL_SETUP,	/* User rejects call setup */
	STK_DATA_SERV_USER_END_PROCTIVE_SESSION,	/* User ends proactive session */
	STK_DATA_SERV_USER_CLEARED_CALL_BEFORE_CONNECTION,	/* User clear call before connection */
	STK_DATA_SERV_BUSY_ON_CALL,							/* ME is busy on call */
	STK_DATA_SERV_INVALID_CHANNEL_ID,					/* Requested channel ID is invalid */
	STK_DATA_SERV_CHANNEL_ERROR,						/* Requested channel ID is in error */
	STK_DATA_SERV_CHANNEL_CLOSED,						/* Requested channel has been closed */
	STK_DATA_SERV_NO_EMPTY_SPACE						/* There is no empty space in Tx buffer */
} STK_DATA_SERV_RST_t;


/* Return TRUE if the passed channel ID is valid; Return FALSE otherwise */
#define IS_CHANNEL_ID_VALID(channel_id) ( ((channel_id) >= MIN_CHANNEL_ID) && ((channel_id) <= MAX_CHANNEL_ID) )


typedef struct {
    byte  A[256];
}  T_SIM_SS_DATA_ARRAY_t;

typedef enum 
{
     C_SI_SS_TYPE  =0,
     C_SI_USSD_TYPE_EARLY_USAGE  =1,
     C_SI_USSD_TYPE  =2,
     C_SI_NO_SS_TYPE  =3
}T_SIM_SS_TYPE_t;

typedef struct  {
    T_SIM_SS_TYPE_t  ss_type;
    byte  length;
    T_SIM_SS_DATA_ARRAY_t  data;
} T_SIM_SS_DATA_t;

//******************************************************************************
//
// Function Name:	STKPRIM_MenuSelectionReq
//
// Description:		This function sends SI_MENU_SELECTION_REQ signal to USIMAP.
//                  MI is requested to present a menu selection to the user.  
//                  When the user has selected one of the menus, MI sends an 
//                  SI_MENU_SELECTION_REQ to SIMAP. If SIMAP is currently busy, 
//                  it can response to this with an SI_MENU_SELECTION_REJ.
// Notes:
//
//******************************************************************************

void STKPRIM_MenuSelectionReq(
	UInt8		identifier,			// identifier of selected menu
	Boolean 	info_requested		// user wants to get additional info to this menu item
	);


//******************************************************************************
//
// Function Name:	STKPRIM_DisplayTextRes
//
// Description:		This function sends SI_DISPLAY_TEXT_RES signal to USIMAP.  
//                  It is sent as a response to SI_SETUP_IDLEMODE_TEXT_REQ.
// Notes:
//
//******************************************************************************

void STKPRIM_DisplayTextRes(
	T_MI_PROACTIVE_CAUSE_t		cause			// T_MI_PROACTIVE_CAUSE
	);


//******************************************************************************
//
// Function Name:	STKPRIM_SetupIdlemodeTextRes
//
// Description:		This function sends SI_SETUP_IDLEMODE_TEXT_RES signal to USIMAP.  
//                  It is sent as a response to SI_SETUP_IDLEMODE_TEXT_REQ
// Notes:
//
//******************************************************************************

void STKPRIM_SetupIdlemodeTextRes(
	T_MI_PROACTIVE_CAUSE_t		cause			// T_MI_PROACTIVE_CAUSE
	);

//******************************************************************************
//
// Function Name:	STKPRIM_GetInputRes
//
// Description:		This function sends SI_GET_INPUT_RES signal to USIMAP.  
//					It is sent as a response to SI_GET_INPUT_REQ
// Notes:
//
//******************************************************************************

void STKPRIM_GetInputRes(
	T_MI_PROACTIVE_CAUSE_t	cause,			// T_MI_PROACTIVE_CAUSE
	const STKTxt_t			*stk_text,
	Boolean					is_packed		//to tell stk to pack the data or not
	);

//******************************************************************************
//
// Function Name:	STKPRIM_GetInkeyRes
//
// Description:	This function sends SI_GET_INKEY_RES signal to USIMAP.   
//				It is sent as a response to SI_GET_INKEY_REQ.
// Notes:
//
//******************************************************************************

void STKPRIM_GetInkeyRes(
	T_MI_PROACTIVE_CAUSE_t	cause,			// T_MI_PROACTIVE_CAUSE
	const STKTxt_t			*stk_text,
    UInt16 time_interval
	);

//******************************************************************************
//
// Function Name:	STKPRIM_PlayToneRes
//
// Description:		This function sends SI_PLAY_TONE_RES signal to USIMAP.
//					sent as a response to SI_PLAY_TONE_REQ
// Notes:
//
//******************************************************************************

void STKPRIM_PlayToneRes(
	T_MI_PROACTIVE_CAUSE_t		cause			// T_MI_PROACTIVE_CAUSE
	);

//******************************************************************************
//
// Function Name:	STKPRIM_SelectItemRes
//
// Description:		This function sends SI_SELECT_ITEM_RES signal to USIMAP. It is
//					sent as a response to SI_SELECT_ITEM_REQ
// Notes:
//
//******************************************************************************

void STKPRIM_SelectItemRes(
	T_MI_PROACTIVE_CAUSE_t		cause,			// T_MI_PROACTIVE_CAUSE
	UInt8		identifier
	);

//******************************************************************************
//
// Function Name:	STKPRIM_SetupMenuRes
//
// Description:		This function sends SI_SETUP_MENU_RES signal to USIMAP. It is
//					sent as a response to SI_SETUP_MENU_REQ
// Notes:
//
//******************************************************************************

void STKPRIM_SetupMenuRes(
	T_MI_PROACTIVE_CAUSE_t          cause                   // T_MI_PROACTIVE_CAUSE	
	);

//******************************************************************************
//
// Function Name:	STKPRIM_LocalDateCnf
//
// Description:		This function sends MISI_LOCAL_DATE_CNF signal to USIMAP.  
//					It is sent  as a response to SIMI_LOCAL_DATE_REQ.
// Notes:
//
//******************************************************************************

void STKPRIM_LocalDateCnf(
	const STKLocalDate_t	*local_date			
	);


//******************************************************************************
//
// Function Name:	STKPRIM_LocalLangCnf
//
// Description:		This function sends SI_LANGUAGE_SETTING_IND signal to USIMAP. It
//					is sent as a response to SIMI_LOCAL_LANG_REQ
// Notes:
//
//******************************************************************************

void STKPRIM_LocalLangCnf(
	UInt8		c1,
	UInt8		c2
	);


//******************************************************************************
//
// Function Name:	STKPRIM_SimtoolkitRefreshRes
//
// Description:		This function sends SI_LANGUAGE_SETTING_IND signal to USIMAP.
//					When USIMAP receives a Proactive command to perform a refresh,
//					it sends the signal SIMI_SIMTOOLKIT_REFRESH_REQ to MI.
//					MI responses with a MISI_SIMTOOLKIT_REFRESH_RES.  This signal
//					contains a status that the refresh can be done or that it
//                  can not be done.
// Notes:
//
//******************************************************************************

void STKPRIM_SimtoolkitRefreshRes(
	T_MI_PROACTIVE_CAUSE_t		cause			// T_MI_PROACTIVE_CAUSE
	);

//******************************************************************************
//
// Function Name:	STKPRIM_SimtoolkitRefreshResATC
//
// Description:		This function sends SI_LANGUAGE_SETTING_IND signal to USIMAP.  
//					When USIMAP receives a Proactive command to perform a refresh,
//					it sends the signal SIAT_SIMTOOLKIT_REFRESH_REQ to ATC.  ATC
//					responds with a ATSI_SIMTOOLKIT_REFRESH_RES.  This signal
//					contains a status that the refresh can be done or that it 
//                  can not be done.
// Notes:           Need a seperate Refresh Response function for ATC
//
//******************************************************************************

void STKPRIM_SimtoolkitRefreshResATC(
	Boolean flag
	);

//******************************************************************************
//
// Function Name:	STKPRIM_MNSISendSsErr
//
// Description:		This function sends MNSI_SEND_SS_ERR signal to USIMAP.
//                  It is used when MN gets the response on a SS-transaction and
//					an error has occured
// Notes:
//
//******************************************************************************

void STKPRIM_MNSI_SendSsErr(
	UInt8 gen_result,
    UInt8 adn_result
	);
//******************************************************************************
//
// Function Name:	STKPRIM_MNSISendSsRes
//
// Description:		This function sends MNSI_SEND_SS_RES signal to USIMAP. It is used
//					when MN gets the response on a SS-transaction and no error 
//					has occured
// Notes:
//
//******************************************************************************

void STKPRIM_MNSISendSsRes(
	UInt8					opcode,			//SS operation code
	const T_SIM_SS_DATA_t	*response_data,	//response data on SS-transaction from network
	T_MI_PROACTIVE_CAUSE_t 	cause			//indicates icon display successful or not
	);

//******************************************************************************
//
// Function Name:	STKPRIM_MNSISsFacilityRej
//
// Description:		This function sends MNSI_SS_FACILITY_REJ signal to USIMAP.
//                  SIM provides SS input did not pass syntax checks in MN or
//					MO SS request has been rejected by MN
// Notes:
//
//******************************************************************************

void STKPRIM_MNSISsFacilityRej(
	T_MI_PROACTIVE_CAUSE_t		facility_error	// MN fail cause	
	); 

//******************************************************************************
//
// Function Name:	STKPRIM_MNSISetupCallRes
//
// Description:		This function sends MNSI_SETUP_CALL_RES signal to USIMAP. Used
//					when MN has received a call confirm or reject, or if the call
//					was rejected immediately by MN (e.g. no network is available,
//					MMI is temporary barred, user busy, ...) 
// Notes:
//
//******************************************************************************

void STKPRIM_MNSISetupCallRes(
	T_MI_PROACTIVE_CAUSE_t	cause		
	); 

//******************************************************************************
//
// Function Name:	STKPRIM_MNSISendDtmfRes
//
// Description:		This function sends MNSI_SEND_DTMF_RES signal to USIMAP. Used
//					to send DTMF terminal response
// Notes:
//
//******************************************************************************
void STKPRIM_MNSISendDtmfRes(
	T_MI_PROACTIVE_CAUSE_t	cause,
	T_MI_PROACTIVE_CAUSE_t	cause2
	);

//******************************************************************************
//
// Function Name:	STKPRIM_RxPrim
//
// Description:		Processes messages received from USIMAP
// Notes:
//
//******************************************************************************		
void STKPRIM_RxPrim(
	PrimPtr_t Primitive
	);

//******************************************************************************
//
// Function Name:	STKPRIM_MN_SimtoolkitRefreshRes
//
// Description:		This function sends MNSI_SIMTOOLKIT_REFRESH_RES signal to USIMAP. 
//					Used when SIMN_SIMTOOLKIT_REFRESH_REQ is received
// Notes:
//
//******************************************************************************
void STKPRIM_MN_SimtoolkitRefreshRes(
	Boolean		result			
);

//******************************************************************************
//
// Function Name:	STKPRIM_MN_SimtoolkitRefreshRes
//
// Description:		This function sends MNSI_SIMTOOLKIT_REFRESH_RES signal to USIMAP.   
//					Used when SIMN_SIMTK_REFRESH_RST_REQ is received
// Notes:
//
//******************************************************************************
void STKPRIM_MN_SIMTKRefreshRstRes(
	Boolean		result			
);

//******************************************************************************
//
// Function Name:	STKPRIM_MI_SIMTKRefreshRstRes
//
// Description:		This function sends MNSI_SIMTK_REFRESH_RST_RES signal to USIMAP.   
//					Used to send a terminal response for resfresh command
// Notes:
//
//******************************************************************************
void STKPRIM_MI_SIMTKRefreshRstRes(T_MI_PROACTIVE_CAUSE_t cause);

//******************************************************************************
//
// Function Name:	STKPRIM_MN_SIMTKRstConfirmInd
//
// Description:		This function sends MNSI_SIMTK_RST_CONFIRM_IND signal to USIMAP.   
//					Used when SIMN_SIMTK_RST_CONFIRM_REQ is received
// Notes:
//
//******************************************************************************
void STKPRIM_MN_SIMTKRstConfirmInd(void);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCcSetupReq
//
// Description:		This function sends the MNSI_CALL_CONTROL_SETUP_REQ signal
//					to USIMAP in SIM TOOLKIT call control for non-SS and 
//					non-USSD calls. A corresponding SIMN_CALL_CONTROL_SETUP_IND 
//					signal will be returned from USIMAP.
// Notes:
//
//******************************************************************************
void STKPRIM_SendCcSetupReq(
	T_ADDRESS	*addr,			/* the call setup address				*/
    T_MN_BC		*bc1,			/* the first ccp set					*/
    T_CALLED_SUBADDR *subaddr,	/* the called party subaddress			*/
    T_MN_BC		*bc2,			/* the second ccp set					*/	
    UInt8		bc_repeat_ind,	/* the bc repeat indicator				*/
    Boolean		simtk_orig		/* TRUE if request originates from SIMTK */ 
);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCcSsReq
//
// Description:		This function sends the MNSI_CALL_CONTROL_SS_REQ signal
//					to USIMAP in SIM TOOLKIT call control for SS call setup 
//					request. A corresponding signal (one of 
//					MNSI_CALL_CONTROL_SETUP_IND, MNSI_CALL_CONTROL_SS_IND, 
//					MNSI_CALL_CONTROL_USSD_IND) will be returned from USIMAP. 
// Notes:
//
//******************************************************************************
void STKPRIM_SendCcSsReq(
	T_SIM_SS_DATA *ss_str,		/* SS string */
    Boolean		simtk_orig		/* TRUE if request originates from SIMTK  */
);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCcUssdReq
//
// Description:		This function sends the MNSI_CALL_CONTROL_USSD_REQ signal
//					to USIMAP in SIM TOOLKIT call control for USSD call setup 
//					request. A corresponding signal (one of 
//					MNSI_CALL_CONTROL_SETUP_IND, MNSI_CALL_CONTROL_SS_IND, 
//					MNSI_CALL_CONTROL_USSD_IND) will be returned from USIMAP.
// Notes:
//
//******************************************************************************
void STKPRIM_SendCcUssdReq(
	T_SIM_SS_DATA *ussd_str,	/* USSD string */
    Boolean		simtk_orig		/* TRUE if request originates from SIMTK  */
);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCcSmsReq
//
// Description:		This function sends the MNSI_MO_SMS_CONTROL_REQ signal
//					to USIMAP in SIM TOOLKIT call control for MO SMS. 
//					A corresponding MNSI_MO_SMS_CONTROL_IND signal will be 
//					returned from USIMAP. 
// Notes:
//
//******************************************************************************
void STKPRIM_SendCcSmsReq(
	T_ADDRESS *sca,		/* the Service Centre address */
    T_ADDRESS *dest,	/* the Destination address */
    Boolean simtk_orig	/* TRUE if request originates from SIMTK */
);

//******************************************************************************
//
// Function Name:	STKPRIM_SendGenStkCmdReq
//
// Description:		This function sends generic STK command to USIMAP. 
// Notes:
//
//******************************************************************************
void STKPRIM_SendGenStkCmdReq(T_SHM_APDU *ptr);

//******************************************************************************
//
// Function Name:	STKPRIM_SendMtCallEvent
//
// Description:	    Send MNSI_MT_CALL_EVENT signal to USIMAP. It is used
//                  to send envelope command for MT call event download
// Notes:
//
//******************************************************************************
void STKPRIM_SendMtCallEvent(T_ADDRESS *addr, T_CALLED_SUBADDR *subaddr, T_TI_PD *ti_pd);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCallConnectedEvent
//
// Description:		Send MNSI_CALL_CONNECTED_EVENT signal to USIMAP. It is used
//                  to send envelope command for Call-connected event download
// Notes:
//
//******************************************************************************
void STKPRIM_SendCallConnectedEvent(Boolean mo_call, T_TI_PD *ti_pd);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCallDisconnectedEvent
//
// Description:		Send MNSI_CALL_DISCONNECTED_EVENT signal to USIMAP. It is used
//                  to send envelope command for Call-disconnected event download
// Notes:
//
//******************************************************************************
void STKPRIM_SendCallDisconnectedEvent( Boolean net_disconn, T_TI_PD *ti_pd, 
									    T_SI_CE_CAUSE *cause );

//******************************************************************************
//
// Function Name:	STKPRIM_SendLocationStatusEvent
//
// Description:		Send MNSI_LOCATION_STATUS_EVENT signal to USIMAP. It is used
//                  to send envelope command for location status event download
// Notes:
//
//******************************************************************************
void STKPRIM_SendLocationStatusEvent(T_REGISTRATION_STATE reg_state, 
                                     T_LAI *lai, 
                                     UInt16 cell_id, 
                                     UInt16 rnc_id, 
                                     T_RAT access_technology);

//******************************************************************************
//
// Function Name:	STKPRIM_SendUserActivityEvent
//
// Description:		Send SI_USER_ACTIVITY_EVENT signal to USIMAP. It is used
//                  to send envelope command for User Activity
// Notes:
//
//******************************************************************************
void STKPRIM_SendUserActivityEvent(void);

//******************************************************************************
//
// Function Name:	STKPRIM_SendIdleScreenAvaiEvent
//
// Description:		Send SI_IDLE_SCREEN_AVAIL_EVENT signal to USIMAP. It is used
//                  to envelope command for Idle Screen
// Notes:
//
//******************************************************************************
void STKPRIM_SendIdleScreenAvaiEvent(void);

//******************************************************************************
//
// Function Name:	STKPRIM_SendLangSelectEvent
//
// Description:		Send SI_LANG_SELECT_EVENT signal to USIMAP. It is used
//                  to send envelope command for language selection as part of 
//                  Local information event.
// Notes:
//
//******************************************************************************
void STKPRIM_SendLangSelectEvent(UInt16 language);

//******************************************************************************
//
// Function Name:	STKPRIM_SendBrowserTermEvent
//
// Description:		Send BRSI_BROWSER_TERM_EVENT signal to USIMAP. It is used
//                  to send envelope command for Browser Termination event
// Notes:
//
//******************************************************************************
void STKPRIM_SendBrowserTermEvent(Boolean user_term);

//******************************************************************************
//
// Function Name:	STKPRIM_SendOpenChannelRes
//
// Description:		Send "Open Channel" Terminal Response to USIMAP
// Notes:
//
//******************************************************************************
void STKPRIM_SendOpenChannelRes( UInt8 channel_id, UInt16 buffer_size, const T_SI_CHANNEL_CHANGED_STATE chan_state,
										const T_SI_BEARER_DESC *bearer_desc, UInt8 gen_result, UInt8 adn_result,
										Boolean uicc_server_mode );

//******************************************************************************
//
// Function Name:	STKPRIM_SendSendDataRes
// 
// Description:	    Send SI_SEND_DATA_RES signal to USIMAP. It is used 
//                  to send terminal response for Send Data command
// Notes:
//
//******************************************************************************
void STKPRIM_SendSendDataRes(UInt8 channel_id, UInt16 num_of_empty_bytes, UInt8 gen_result, UInt8 adn_result);

//******************************************************************************
//
// Function Name:	STKPRIM_SendReceiveDataRes
//
// Description:		Send SI_RECEIVE_DATA_RES signal to USIMAP. It is used
//                  to send terminal response for Receive Data command
// Notes:
//
//******************************************************************************
void STKPRIM_SendReceiveDataRes(UInt8 channel_id, UInt16 num_of_bytes_avail, 
				const T_SI_CHANNEL_DATA *channel_data, UInt8 gen_result, UInt8 adn_result);

//******************************************************************************
//
// Function Name:	STKPRIM_SendCloseChannelRes
//
// Description:		Send SI_CLOSE_CHANNEL_RES signal to USIMAP. It is used 
//                  to send terminal response for Close Channel command
// Notes:
//
//******************************************************************************
void STKPRIM_SendCloseChannelRes(UInt8 channel_id, UInt8 gen_result, UInt8 adn_result);

//******************************************************************************
//
// Function Name:	STKPRIM_SendChannelStatusRes
//
// Description:		Send SI_GET_CHANNEL_STATUS_RES signal to USIMAP. It is used
//                  to send terminal response for Channel Status command
// Notes:
//
//******************************************************************************
void STKPRIM_SendChannelStatusRes(const T_SI_CHANNEL_STATUS_REC *channel_status_rec, 
                                  UInt8 num_of_channel);


//******************************************************************************
//
// Function Name:	STKPRIM_SendDataAvailableEvent
//
// Description:		Send Data Available Event to USIMAP
// Notes:
//
//******************************************************************************

void STKPRIM_SendDataAvailableEvent(UInt8 channel_id, T_SI_CHANNEL_SOCKET_STATE channel_socket_state,
                                     T_SI_CHANNEL_CHANGED_STATE link_state, UInt16 num_of_bytes_avail);

//******************************************************************************
//
// Function Name:	STKPRIM_SendChannelStatusEvent
//
// Description:		Send SI_CHANNEL_STATUS_EVENT signal to USIMAP. It is used
//                  to send envelope command for Channel Status event 
// Notes:
//
//******************************************************************************
void STKPRIM_SendChannelStatusEvent(UInt8 channel_id, T_SI_CHANNEL_SOCKET_STATE channel_socket_state, T_SI_CHANNEL_CHANGED_STATE link_state);



//******************************************************************************
//
// Function Name:	STKPRIM_SendRunAtCmdRes
//
// Description:		Send SIAT_RUN_AT_CMD_IND signal to USIMAP. It is used
//                  to send terminal response for Run AT-Command command
// Notes:
//
//******************************************************************************
void STKPRIM_SendRunAtCmdRes(UInt8 *at_res, UInt8 length);

//******************************************************************************
//
// Function Name:	STKPRIM_SendSendSmsCnf
//
// Description:		Send MN_SMS_SEND_PDU_CNF signal to USIMAP. It is used
//                  to send terminal response for successful SMS command
// Notes:
//
//******************************************************************************
void STKPRIM_SendSendSmsCnf(Boolean modified_flag);

//******************************************************************************
//
// Function Name:	STKPRIM_SendSendSmsRej
//
// Description:		Send MN_SMS_SEND_PDU_REJ signal to USIMAP.. It is used
//                  to send terminal response for failed SMS command
// Notes:
//
//******************************************************************************
void STKPRIM_SendSendSmsRej(UInt8 gen_result, UInt8 adn_result);

//******************************************************************************
//
// Function Name:	STKPRIM_LaunchBrowserRes
//
// Description:		Send SI_LAUNCH_BROWSER_RES signal to USIMAP. It is used
//                  to send terminal response for Lauch Browser command
// Notes:
//
//******************************************************************************
void STKPRIM_LaunchBrowserRes(T_MI_PROACTIVE_CAUSE_t cause);


//******************************************************************************
//
// Function Name:	STKPRIM_SendActivateRes
//
// Description:		Send terminal response for Activate command to USIMAP.
// Notes:
//
//******************************************************************************
void STKPRIM_SendActivateRes(T_MI_PROACTIVE_CAUSE result);

//******************************************************************************
//
// Function Name:	STKPRIM_SendImeisvRes
//
// Description:		Send SI_LOCAL_IMEISV_RES signal to USIMAP. It is used
//                  to send terminal response for IMEISV as part of local info
//                  command
// Notes:
//
//******************************************************************************
void STKPRIM_SendImeisvRes(UInt8 *imeisv, UInt8 len);

//******************************************************************************
//
// Function Name:	STKPRIM_SendNwSearchModeRes
//
// Description:		Send SI_LOCAL_NW_SEARCH_MODE_RES signal to USIMAP. It is used
//                  to send terminal response for NW Search Mode as part of local
//                  info command
// Notes:
//
//******************************************************************************
void STKPRIM_SendNwSearchModeRes(UInt8 plmnMode);

//******************************************************************************
//
// Function Name:	STKPRIM_SendBatteryStateRes
//
// Description:		Send SI_LOCAL_BATTERY_STATE_RES signal to USIMAP. It is used
//                  to send terminal response for Battery Status as part of local
//                  info command
// Notes:
//
//******************************************************************************
void STKPRIM_SendBatteryStateRes(UInt8 battery_state);

//******************************************************************************
//
// Function Name:	STKPRIM_SendNwSearchModeChgEvent
//
// Description:		Send SI_NETWORK_SEARCH_MODE_CHG_EVENT signal to USIMAP. It is used
//                  to send envelope command for NW Search Mode Change event.
// Notes:
//
//******************************************************************************
void STKPRIM_SendNwSearchModeChgEvent(UInt8 plmnMode);

//******************************************************************************
//
// Function Name:	STKPRIM_SendBrowsingStatusEvent
//
// Description:		Send SI_BROWSING_STATUS_EVENT signal to USIMAP. It is used
//                  to send envelope command for Browser Status event
// Notes:
//
//******************************************************************************
void STKPRIM_SendBrowsingStatusEvent(UInt8 *status, UInt8 len);

//******************************************************************************
//
// Function Name:	STKPRIM_MiSiClassmarkInd
//
// Description:		Send MISI_CLASSMARK_IND signal to USIMMAP. 
//
//******************************************************************************
void STKPRIM_MiSiClassmarkInd(Boolean dynTP, Boolean enableCmdFetching,
                              UInt8 tpDataLen, const UInt8* ptrTpData,
							  UInt8 imeiDataLen, const UInt8* ptrImeiData);

//******************************************************************************
//
// Function Name:	STKPRIM_SendTermProfileReq
//
// Description:		Send SI_TERM_PROFILE_REQ signal to USIMMAP. 
//
//******************************************************************************
void STKPRIM_SendTermProfileReq( UInt8 tpDataLen, const UInt8* ptrTpData );

//******************************************************************************
//
// Function Name:	STKPRIM_ProactiveCmdFetchingOnOffReq
//
// Description:		Send SI_PROACT_CMD_FETCHING_ON_OFF_REQ signal to USIMMAP. 
//
//******************************************************************************
void STKPRIM_ProactiveCmdFetchingOnOffReq(Boolean enableCmdFetching);

// Function Name:	STKPRIM_SendExtProactiveCmdReq
//
// Description:		Send SI_EXT_PROACTIVE_CMD_REQ signal to USIMMAP. 
//
//******************************************************************************
void STKPRIM_SendExtProactiveCmdReq(T_SHM_APDU *ptr);

//******************************************************************************
//
// Function Name:	STKPRIM_SetPollingInterval
//
// Description:		Send SI_POLLING_INTERVAL_REQ signal to USIMMAP. 
//
//******************************************************************************
void STKPRIM_SetPollingInterval(UInt16 pollingInterval);

#endif


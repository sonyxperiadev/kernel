//*********************************************************************
//
//	Copyright © 2008-2009 Broadcom Corporation
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
*   @file   ss_api_old.h
*
*   @brief  This file defines the interface for OLD Supplementary Services API.
*
****************************************************************************/ 
/**
*   @defgroup   SSAPIOLDGroup   Legacy Supplementary Services
*   @ingroup    TelephonyGroup
*
*   @brief      This group defines the legacy interfaces to the supplementary
*				services group kept for backwards compatibility.
*
*				This group defines the interface for supplementary services API.
*				It provides the function prototypes necessary to
*				activate, register, interrogate, enable, and disable supplementary
*				services such as call forwarding, call barring, calling line
*				presentation for speech, data, fax and other call types. 

* Use the link below to navigate back to the Supplementary Services Overview page. \n
    \li \if CAPI2
	\ref CAPI2SSOverview
	\endif
	\if CAPI
	\ref SSOverview
	\endif
*****************************************************************************/
#ifndef _SS_API_OLD_H_
#define _SS_API_OLD_H_


/* Maximum number of service classes returned in SS status query */ 

#define MAX_SS_CLASS_SIZE 15	///< Maximum number of service classes returned by a query.




#define MAX_USSD_SESSIONS	2	///< Maximum number of USSD sessions.

/**
	Service Mode
*/

typedef enum {
	SS_MODE_NOTSPECIFIED,	///< Unspecified
	SS_MODE_DISABLE,		///< Disable service
	SS_MODE_ENABLE,			///< Enable service
	SS_MODE_INTERROGATE,	///< Query status of service
	SS_MODE_REGISTER,		///< Register specific services
	SS_MODE_ERASE			///< Erase specific services
}	SS_Mode_t ;


/**
	Service Class
*/
typedef enum {
	SS_SVCCLS_NOTSPECIFIED,					///< Unspecified
	SS_SVCCLS_SPEECH,						///< Voice line: L1
	SS_SVCCLS_ALT_SPEECH,					///< Alternative voice line: L2
	SS_SVCCLS_DATA,							///< Sevice Class Data call			
	SS_SVCCLS_FAX,							///< Sevice Class Fax call			
	SS_SVCCLS_ALL_TELESERVICE_EXCEPT_SMS,	///< All tele services except SMS
	SS_SVCCLS_SMS,							///< Service Class SMS
	SS_SVCCLS_DATA_CIRCUIT_SYNC,			///< Circuit Data synchronous
	SS_SVCCLS_DATA_CIRCUIT_ASYNC,			///< Circuit Data Asynchronous
	SS_SVCCLS_DEDICATED_PACKET,				///< Circuit Data Asynchronous
	SS_SVCCLS_ALL_SYNC_SERVICES,			///< All Synchronous services
	SS_SVCCLS_DEDICATED_PAD,				///< Dedicated PAD service
	SS_SVCCLS_ALL_ASYNC_SERVICES,			///< All Asynchronous services
	SS_SVCCLS_ALL_BEARER_SERVICES,			///< All Bearer services
	SS_SVCCLS_ALL_TELE_SERVICES,			///< All Tele services
	SS_SVCCLS_PLMN_SPEC_TELE_SVC,			///< PLMN Specific Tele Service
	SS_SVCCLS_SPEECH_FOLLOW_BY_DATA_CDA,	///< All Speech Followed By Data CDA Services
	SS_SVCCLS_SPEECH_FOLLOW_BY_DATA_CDS,	///< All Speech Followed By Data CDS Services
	SS_SVCCLS_UNKNOWN						///< Unknown
}	SS_SvcCls_t ;


/**
	Call Forward Reasons
*/

typedef enum {
	SS_CALLFWD_REASON_NOTSPECIFIED,		///< Unspecified
	SS_CALLFWD_REASON_UNCONDITIONAL,	///< Unconditional call forwarding
	SS_CALLFWD_REASON_BUSY,				///< Call forward on user busy
	SS_CALLFWD_REASON_NO_REPLY,			///< Call forward on no reply from the user
	SS_CALLFWD_REASON_NOT_REACHABLE,	///< User is unreachable
	SS_CALLFWD_REASON_ALL_CF,			///< Forward all calls
	SS_CALLFWD_REASON_ALL_CONDITIONAL,	///< Forward calls conditionally
//Following are added due to new call parser!
	SS_REASON_CLIP,						///< Callling line ID presentation service
	SS_REASON_CLIR,						///< Calling Line ID Restriction
	SS_REASON_COLP,						///< Connected line ID presentation
	SS_REASON_COLR,						///< Connected Line ID service
	SS_REASON_CW						///< Call waiting service
}	SS_CallFwdReason_t ;

/**
	Call Bar Type
*/

typedef enum {
	SS_CALLBAR_TYPE_NOTSPECIFIED,			///< Not Specified
	SS_CALLBAR_TYPE_OUT_ALL,				///< All outgoing 
	SS_CALLBAR_TYPE_OUT_INTL	,			///< Outgoing international 
	SS_CALLBAR_TYPE_OUT_INTL_EXCL_HPLMN,	///< Outgoing international excluding HomePLMN (home country)
											
	SS_CALLBAR_TYPE_INC_ALL,				///< All incoming
	SS_CALLBAR_TYPE_INC_ROAM_OUTSIDE_HPLMN,	///< All incoming when roaming outside HomePLMN (home country)
											
	SS_CALLBAR_TYPE_ALL,					///< All calls(incoming and outgoing)
	SS_CALLBAR_TYPE_OUTG,					///< Outgoing
	SS_CALLBAR_TYPE_INC						///< Incoming
}	SS_CallBarType_t ;




/**	
	SS API request Request enumerations
*/

typedef enum
{
	SS_API_CALL_FORWARDING_REQUEST,			///< Request for call forwarding service
	SS_API_CALL_BARRING_REQUEST,			///< Request Call Barring feature
	SS_API_CALL_BAR_PW_CHANGE_REQUEST,		///< Password Change request service
	SS_API_CALL_WAITING_REQUEST,			///< Call waiting service
	
	SS_API_QUERY_CLIP_STATUS_REQUEST,		///< Request to query callling line ID presentation status
	SS_API_QUERY_COLP_STATUS_REQUEST,		///< Request for connected line ID presentation status
	SS_API_QUERY_CLIR_STATUS_REQUEST,		///< Query current calling Line ID Restriction service status
	SS_API_QUERY_COLR_STATUS_REQUEST,		///< Connected Line ID Request
	SS_API_QUERY_CNAP_STATUS_REQUEST,		///< Calling Name Presentation Status query
	
	SS_API_QUERY_CALL_FORWARDING_REQUEST,	///< Request to query call fwding status
	SS_API_QUERY_CALL_BARRING_REQUEST,		///< Call barring status request
	SS_API_QUERY_CALL_WAITING_REQUEST,		///< Query for current call wating status
	
	SS_API_CLIP_SERVICE_REQUEST,			///< Callling line ID presentation service
	SS_API_COLP_SERVICE_REQUEST,			///< Connected line ID presentation service
	SS_API_CLIR_SERVICE_REQUEST,			///< Calling Line ID Restriction service
	SS_API_COLR_SERVICE_REQUEST,			///< Connected Line ID service

	SS_API_GENERIC_REQUEST,					///< A generic Supplementary service request. 
	
	SS_API_NONE			
} SS_ApiReq_t;


/**
	Enumeration of SS string constants
*/

typedef enum {

	//	separators
	
	SS_SEPARATOR_POUND_STR,			///< Separator Pound String '#'
	SS_SEPARATOR_STAR_STR,			///< Separator Star String '*'
	SS_INTERNATIONAL_PLUS_STR,		///< International call Plus Symbol  '+'

	//	mode

	SS_MODE_DISABLE_STR,		   ///< Service mode disable string(use this to disable services)
	SS_MODE_ENABLE_STR,			   ///< Service mode enable string(use this to enable services)
	SS_MODE_INTERROGATE_STR,	   ///< Query status string
	SS_MODE_REGISTER_STR,		   ///< Service registration string(use this to register new services)
	SS_MODE_ERASE_STR,			   ///< Service erase mode string(use this to erase or un-register services)

	//	service class

	SS_SVCCLS_SPEECH_STR,					///< Speech service class string(Telephony and Emergency call)
	SS_SVCCLS_ALT_SPEECH_STR,				///< Alternate speech service class
	SS_SVCCLS_DATA_STR,						///< Data service class
	SS_SVCCLS_FAX_STR,						///< Fax class(Only class 3 fax)
	SS_SVCCLS_ALL_TELE_SERVICES_STR,		///< All teleservices
	SS_SVCCLS_ALL_TELESERVICE_EXCEPT_SMS_STR,///< All teleservices except SMS(speech,data,fax)
	SS_SVCCLS_SMS_STR,						///< SMS service class( MO/MT SMS and Cell Broadcast)
	SS_SVCCLS_DATA_CIRCUIT_SYNC_STR,		///< Data Services synchronous(General and Data circuit duplex)
	SS_SVCCLS_DATA_CIRCUIT_ASYNC_STR,		///< Data Services asynchronous(General and Data circuit duplex)
	SS_SVCCLS_DEDICATED_PACKET_STR,			///< Dedicated Packet services(Data Packet duplex and General Packet Service) 
	SS_SVCCLS_ALL_SYNC_SERVICES_STR,		///< All synchronous services
	SS_SVCCLS_DEDICATED_PAD_STR,			///< Genral PAD access and all PAD data services
	SS_SVCCLS_ALL_ASYNC_SERVICES_STR,		///< All asynchronous services
	SS_SVCCLS_ALL_BEARER_SERVICES_STR,		///< All Bearer services( Data and Packet services)

	//	call forward reason

	SS_CALLFWD_REASON_UNCONDITIONAL_STR,	///< Unconditional Call forwarding
	SS_CALLFWD_REASON_BUSY_STR,				///< Call forward on busy tone 
	SS_CALLFWD_REASON_NO_REPLY_STR,			///< Call forward on no reply
	SS_CALLFWD_REASON_NOT_REACHABLE_STR,	///< Not reachable call forward
	SS_CALLFWD_REASON_ALL_CF_STR,			///< Forward all calls
	SS_CALLFWD_REASON_ALL_CONDITIONAL_STR,	///< Conditional forwarding of all calls

	//	caller wait, caller ID, connected line ID

	SS_CALLWAIT_PROV_STR,					///< Call Waiting provisioning string
	SS_CALLLINE_PROV_STR,					///< Calling line presentaion provisioning string
	SS_CONNLINE_PROV_STR,					///< Connected line presentation provisioning string
	SS_CALLLINE_REST_STR,					///< Calling line presentation restriction string
	SS_CONNLINE_REST_STR,					///< Connected line presentation restriction string

	//	call barring

	SS_CALLBAR_TYPE_OUT_ALL_STR,					///< Bar all outgoing calls
	SS_CALLBAR_TYPE_OUT_INTL_STR,					///< Bar all outgoing international calls
	SS_CALLBAR_TYPE_OUT_INTL_EXCL_HPLMN_STR,		///< Bar all outgoing international calls except to Home PLMN
	SS_CALLBAR_TYPE_INC_ALL_STR,					///< Bar all incoming calls 
	SS_CALLBAR_TYPE_INC_ROAM_OUTSIDE_HPLMN_STR,		///< Bar all incoming calls when roaming outside Home PLMN
	SS_CALLBAR_TYPE_ALL_STR,						///< Bar all calls(incoming and outgoing)
	SS_CALLBAR_TYPE_OUTG_STR,						///< Bar outgoing calls, selective
	SS_CALLBAR_TYPE_INC_STR,						///< Bar incoming calls,selective
	
	//	facility password

	SS_FAC_PASSWORD_STR,						///< Facility Password string

	//	Calling NAme Presentation

	SS_SRV_CODE_CALLLING_NAME_PRESENT_STR		///< Calling Name presentation string

}	SS_ConstString_t ;


/**
	USSD Type
*/

typedef enum
{	
	USSD_TYPE_MT,			///< Mobile Terminated
	USSD_TYPE_MO,			///< Mobile Originated
	USSD_TYPE_NONE			///< No type
}UssdType_t;

/**
	USSD Call RequestType
*/

typedef enum
{
	USSD_SEND_CONNECT_REQ,  ///< Send Connect request for USSD session
	USSD_SEND_DATA_REQ,		///< Send MO USSD data
	USSD_END_CONNECT_REQ	///< Request end to the USSD session
}UssdCallReqType_t;

//GSM:07.07, section:7.7, argument:<m>
/**
	SS Service Status. These are status returned by the network upon query
*/
typedef enum
{
 	SS_SERVICE_STATUS_NOT_PROVISIONED,			///< This feature is not provisioned.	
 	SS_SERVICE_STATUS_PROVISIONED_PERMANENT,	///< Permanently provisioned feature.
 	SS_SERVICE_STATUS_UKNOWN,					///< Status currently unknown.
  	SS_SERVICE_STATUS_PRESENTATION_RESTRICTED,	///< Feature presentation is restricted
 	SS_SERVICE_STATUS_PRESENTATION_ALLOWED		///< Presentation allowed
} ServiceStatus_t;

/** Message data payload for SS feature provision status query. The applicable
 * SS features include calling line ID; calling line ID restriction; connected
 * line ID.
 */
typedef struct
{
	NetworkCause_t	netCause;			///< completion status
	Boolean			provision_status;	///< TRUE if SS feature provisioned
	ServiceStatus_t	serviceStatus;		///< GSM:07.07, section:7.7, argument:\<m>
} SS_ProvisionStatus_t;


/// Call forward information for individual service classes 
typedef struct
{
	Boolean				activated;			///< TRUE if call forward activated */
	SS_SvcCls_t			ss_class;			///< Service class: voice, data, fax etc */
	TelephoneNumber_t	forwarded_to_number;///< The number the call is forwarded to. Note: a "fowarded to"
											///< number can exist even if call forward is not activated,
											///< i.e. the number is registered in the network. 
	UInt8				noReplyTime;		///< 5 <= No Reply time <= 30
	TelephoneNumber_t	longforwarded_to_number;	///< Long FTN sent back by n/w
} CallForwardClassInfo_t;


/// Supplementary Services API  service request structure
typedef struct
{
	SS_ApiReq_t			ssReq;
	SS_Mode_t			mode;
	SS_CallFwdReason_t  reason;
	SS_SvcCls_t			svcCls;
	UInt16 				waitToFwdSec;
	UInt8				number[MAX_DIGITS+2];	///< (Note: 1 byte for null termination and the other for int code '+')
	SS_CallBarType_t	callBarType;			///< Call barring type
	UInt8				oldPwd[SS_PASSWORD_LENGTH+1];	///< PWORD_LEN = 4 
	UInt8				newPwd[SS_PASSWORD_LENGTH+1];
	UInt8				reNewPwd[SS_PASSWORD_LENGTH+1];
} SS_SsApiReq_t;


/// Detailed call forward information with information for individual service classes.
typedef struct
{
	NetworkCause_t netCause;	/**< completion status */
	SS_CallFwdReason_t reason;	/**< Call forward condition: unconditional, user busy etc */
	UInt8 class_size;			/**< Number of valid service classes in the following list */
	CallForwardClassInfo_t call_forward_class_info_list[MAX_SS_CLASS_SIZE]; /**< Detailed information for each
																			 * service classes.
																			 */
} CallForwardStatus_t;

/// SS activation status for a specific service class 
typedef struct
{
	Boolean activated;		/**< TRUE if service activated */
	SS_SvcCls_t ss_class;	/**< Service class: voice, data, fax etc */ 
} SS_ActivationClassInfo_t;

/// Detailed activation status with specific information for individual service class 
typedef struct
{
	NetworkCause_t netCause;	/**< completion status */
	UInt8 class_size;	/**< Number of valid service classes in the following list */
	SS_ActivationClassInfo_t ss_activation_class_info[MAX_SS_CLASS_SIZE]; /**< Detailed information for each
																		   * service classes.
																		   */	
} SS_ActivationStatus_t;

/// Detailed call barring status with specific information for individual service class 
typedef struct
{
	NetworkCause_t netCause;	/**< completion status */
	SS_CallBarType_t call_barring_type; /**< Call barring type: bar incoming call, bar outgoing call, etc */
	UInt8 class_size;	/**< Number of valid service classes in the following list */
	SS_ActivationClassInfo_t ss_activation_class_info[MAX_SS_CLASS_SIZE]; /**< Detailed information for each
																		   * service classes.
																		   */
} CallBarringStatus_t;



typedef struct
{
	UssdType_t	ussdType;
	UInt8		ussdSessionId;
} UssdSession_t;

/// Supplementary Services Call request structure
typedef struct {
	UInt8	data[ 64 ] ;
}	SsCallReq_t ;

/// SS Call Request Fail
typedef struct {
	Result_t	result ;
}	SsCallReqFail_t ;

/// USSD Call request structure
typedef struct {
	UssdCallReqType_t	type;
	UInt8	ussdId;
	UInt8	dcs;
	UInt8	len;
	UInt8	ussdStr[ PHASE2_MAX_USSD_STRING_SIZE + 1 ] ;
} UssdCallReq_t ;

/**
 * @addtogroup SSAPIOLDGroup
 * @{
 */

//-----------------------------------------------------------------------------
//	Call forwarding
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a call-forwarding configuration request to the network.  The response
	to this request is a  #MSG_SS_CALL_FORWARD_RSP message broadcast to all clients.

	@param		clientID (in) Client ID used to identify response messages
	@param		mode (in) SS Mode 
	@param		reason (in) Reason for Call forward
	@param		svcCls (in) SS Service Class 
	@param		waitToFwdSec (in) Time to wait before forwarding, in seconds 
	@param		number (in) Call forward number for the mode of enable or registration. 

	@return		Result_t
**/

Result_t SS_SendCallForwardReq( 
	UInt8					clientID, 
	SS_Mode_t				mode, 
	SS_CallFwdReason_t		reason, 
	SS_SvcCls_t				svcCls, 
	UInt8					waitToFwdSec, 
	UInt8*					number) ;

//**************************************************************************************
/**
	Function to Request call-forwarding status from the network.  The response to
	this request is a #MSG_SS_CALL_FORWARD_STATUS_RSP message.

	@param		clientID (in) Client ID	to identify response messages
	@param		reason (in) Reason for Call forward
	@param		svcCls (in) SS Service Class 

	@return		Result_t
**/

Result_t SS_QueryCallForwardStatus(
	UInt8					clientID, 
	SS_CallFwdReason_t		reason, 
	SS_SvcCls_t				svcCls) ;

//-----------------------------------------------------------------------------
//	call barring
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a call-barring configuration request to the network.  The response to 
	this request is a #MSG_SS_CALL_BARRING_RSP message broadcast to all clients.

	@param		clientID (in) Client ID used to identify response messages.
	@param		mode (in) SS_MODE_ENABLE to enable call barring; SS_MODE_DISABLE to disable call barring
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		svcCls (in) SS Service Class 
	@param		password (in)call barring password. 

	@return		Result_t
**/

Result_t SS_SendCallBarringReq(
	UInt8					clientID, 
	SS_Mode_t				mode,
	SS_CallBarType_t		callBarType, 
	SS_SvcCls_t 			svcCls, 
	UInt8*					password) ;

//**************************************************************************************
/**
	Function to Request call-barring status from the network.  The response to this 
	request is a #MSG_SS_CALL_BARRING_STATUS_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		svcCls (in) SS Service Class 

	@return		Result_t
**/

Result_t SS_QueryCallBarringStatus(
	UInt8					clientID,
	SS_CallBarType_t		callBarType, 
	SS_SvcCls_t				svcCls) ;

//**************************************************************************************
/**
	Function to Send a call barring check password to the network.  The response to 
	this request is a #MSG_SS_CALL_BARRING_PWD_CHANGE_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages.
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		oldPwd (in) Old Password 
	@param		newPwd (in) New Password 
	@param		reNewPwd (in) Re-enter New Password

	@return		Result_t
**/

Result_t SS_SendCallBarringPWDChangeReq(
	UInt8					clientID, 
	SS_CallBarType_t		callBarType, 
	UInt8*					oldPwd, 
	UInt8*					newPwd,
	UInt8*					reNewPwd) ;

//**************************************************************************************
/**
	Function to Send a call-barring password-change request to the network.  The response 
	to this request is a #MSG_SS_CALL_BARRING_PWD_CHANGE_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages.
	@param		callBarType (in) an enumerator indicating the call barring type.
	@param		oldPwd (in) Old Password 
	@param		newPwd (in) New Password 
	@param		reNewPwd (in) Re-enter New Password

	@return		Result_t
**/

Result_t SS_SendChangePassWordReq(	UInt8				clientID, 
									SS_CallBarType_t	callBarType, 
									UInt8*				oldPwd, 
									UInt8*				newPwd,
									UInt8*				reNewPwd);

//-----------------------------------------------------------------------------
//	call waiting
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a call-waiting configuration request to the network. The response 
	to this request is a #MSG_SS_CALL_WAITING_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages.
	@param		mode (in) SS_MODE_ENABLE to enable call waiting setting in the network.
	@param		svcCls (in) an enumerator indicating the service class

	@return		Result_t
**/


Result_t SS_SendCallWaitingReq(
	UInt8					clientID, 
	SS_Mode_t				mode, 
	SS_SvcCls_t				svcCls);

//**************************************************************************************
/**
	Function to Request call-waiting status from the network.  The response to this 
	request is a #MSG_SS_CALL_WAITING_STATUS_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages.
	@param		svcCls (in) an enumerator indicating the service class

	@return		Result_t
**/

Result_t SS_QueryCallWaitingStatus(
	UInt8					clientID, 
	SS_SvcCls_t				svcCls);

//-----------------------------------------------------------------------------
//	CLIP/COLP/CLIR/CNAP activation/deactivation/interrogation
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a request to enable/disable Calling Line Presentation status. 

	@param		clientID (in) Client ID
	@param		enable (in) TRUE to enable; FALSE to disable
	@return		Result_t

	@note		The response comes back in one of the following two messages:
				1. MSG_SS_INTERNAL_PARAM_SET_IND if the status is successfully set. 
				2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
**/
Result_t SS_SetCallingLineIDStatus( UInt8 clientID,  Boolean enable ) ;


//**************************************************************************************
/**
	Function to Send a request to the network to query the calling line ID provisioin status.
	The response to this request is a #MSG_SS_CALLING_LINE_ID_STATUS_RSP message
	broadcast to all clients.

	@param		clientID (in) Client ID
	@return		Result_t
**/
Result_t SS_QueryCallingLineIDStatus( UInt8 clientID ) ;


//**************************************************************************************
/**
	Function to Send a request to enable/disable Connected Line Presentation status. 

	@param		clientID (in) Client ID
	@param		enable (in) TRUE to enable; FALSE to disable
	@return		Result_t

	@note		The response comes back in one of the following two messages:
				1. MSG_SS_INTERNAL_PARAM_SET_IND if the status is successfully set. 
				2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
**/
Result_t SS_SetConnectedLineIDStatus( UInt8 clientID,  Boolean enable ) ;


//**************************************************************************************
/**
	Function to Send a request to the network to query the connected line ID 
	provision status in the network.  The response to this request is a 
	#MSG_SS_CONNECTED_LINE_STATUS_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages
	@return		Result_t
**/

Result_t SS_QueryConnectedLineIDStatus( UInt8 clientID ) ;


//**************************************************************************************
/**
	Function to Send a request to erase/activate/deactivate Calling Line Restriction status. 

	@param		clientID (in) Client ID
	@param		clir_mode (in) mode defined in ::CLIRMode_t
	@return		Result_t

	@note		The response comes back in one of the following two messages:
				1. MSG_SS_INTERNAL_PARAM_SET_IND if the status is successfully set. 
				2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
**/
Result_t SS_SetCallingLineRestrictionStatus( UInt8 clientID,  CLIRMode_t clir_mode ) ;


//**************************************************************************************
/**
	Function to Send a request to the network to query the connected line ID provision 
	status in the network.  The response to this request is a 
	#MSG_SS_CONNECTED_LINE_STATUS_RSP message broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages
	@return		Result_t
**/

Result_t SS_QueryCallingLineRestrictionStatus( UInt8 clientID ) ;

//**************************************************************************************
/**
	Function to Send a request to activate/deactivate Connected Line Restriction status. 

	@param		clientID (in) Client ID
	@param		enable (in) TRUE to activate; FALSE to deactivate
	@return		Result_t

	@note		The response comes back in one of the following two messages:
				1. MSG_SS_INTERNAL_PARAM_SET_IND if the status is successfully set. 
				2. MSG_SS_CALL_REQ_FAIL for failure (e.g. The SS dialling string is blocked by FDN). 
**/
Result_t SS_SetConnectedLineRestrictionStatus( UInt8 clientID,  Boolean enable ) ;

//**************************************************************************************
/**
	Function to Send a request to the network to query the connected line restriction 
	provision status.  The response to this request is a 
	#MSG_SS_CONNECTED_LINE_RESTRICTION_STATUS_RSP message broadcast to all clients.

	@param		clientID (in) Client ID
	@return		Result_t
**/

Result_t SS_QueryConnectedLineRestrictionStatus( UInt8 clientID ) ;

//**************************************************************************************
/**
	Function to Send a request to the network to query the calling name presentation 
	status.  The response to this request is a #MSG_SS_CALLING_NAME_PRESENT_STATUS_RSP 
	message broadcast to all clients.

	@param		inClientID (in) Client ID, used to identify response messages.
	@return		Result_t
**/

Result_t SS_QueryCallingNAmePresentStatus( UInt8 inClientID );


//-----------------------------------------------------------------------------
//	USSD
//-----------------------------------------------------------------------------

//**************************************************************************************
/**
	Function to Send a USSD connection request to establish a USSD session.  The 
	response to this request is a #MSG_USSD_CALLINDEX_IND followed by a 
	#MSG_USSD_DATA_RSP which is broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages.
	@param		ussd (in) USSD string, 
				- the length is the byte of the ussd string (whether it is 7bit/8bit or ucs2)
				- the dcs is the value according to GSM3.38: (dcs&0x0F)>>2 = 0,1,2 => 7bit, 8bit, ucs2
				- the string is the ussd string.  Even if the dcs is 7bit, the string is to be passed as 8bit (before packing)
	@return		Result_t
**/
Result_t SS_SendUSSDConnectReq(UInt8 clientID, USSDString_t* ussd );


//**************************************************************************************
/**
	Function to End the current USSD session. There is no response associate with this
	request.

	@param		clientID (in) Client ID, used to identify response messages
	@param		ussd_id (in) USSD call ID
	@return		Result_t
**/

Result_t SS_EndUSSDConnectReq( UInt8 clientID, CallIndex_t ussd_id );

//**************************************************************************************
/**
	Function to Send MO USSD data to network.  The response to this request is a
	#MSG_USSD_DATA_RSP which is broadcast to all clients.

	@param		clientID (in) Client ID, used to identify response messages
	@param		ussd_id (in) USSD call ID
	@param		dcs (in) dcs supported
	@param		len (in) Length of the USSD string
	@param		ussdString (in) Pointer to USSD String

	@return		Result_t
**/

Result_t SS_SendUSSDData( UInt8 clientID, CallIndex_t ussd_id, UInt8 dcs, UInt8	len, UInt8* ussdString ) ;


//-----------------------------------------------------------------------------
//	miscellaneous
//-----------------------------------------------------------------------------

const UInt8* SS_GetStr (SS_ConstString_t strName) ;



//**************************************************************************************
/**
	Function to Set the client ID
	@param		clientID (in) the client ID, used to identify response messages
	@return		Result_t
**/

Result_t SS_SetClientID( UInt8 clientID ) ;

//**************************************************************************************
/**
	Function to Get the client ID
	@return		Client ID in UInt8
	@note 
		clientID the client ID, used to identify response messages
**/

UInt8 SS_GetClientID( void ) ;

//**************************************************************************************
/**
	This function resets Client ID to default which is INVALID_CLIENT_ID
	@return		void
**/

void SS_ResetClientID( void ) ;

/** @} */

// To be removed 
Result_t SS_SsApiReqDispatcher(	UInt8 inClientId,
								SS_SsApiReq_t* inSsApiReqPtr);



void SsApi_ResetSsAlsFlag(void);
#endif // _SS_API_OLD_H_


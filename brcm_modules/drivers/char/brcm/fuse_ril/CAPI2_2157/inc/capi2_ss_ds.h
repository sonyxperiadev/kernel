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
*   @file   capi2_ss_ds.h
*
*   @brief  This file defines the capi2 SS related data types
*
****************************************************************************/

#ifndef _CAPI2_SS_DS_H_
#define _CAPI2_SS_DS_H_
#include "capi2_lcs_ds.h"

/**
 * @addtogroup CAPI2_SSAPIGroup
 * @{
 */
 
/* Maximum number of service classes returned in SS status query */ 

#define MAX_SS_CLASS_SIZE 15	///< Maximum number of service classes returned by a query.


//-------------------------------------------------------------------
//	SS string table.
//
//	If you add a string to this table please ensure that it is
//	handled in SS_GetStr().
//-------------------------------------------------------------------

/**
	Enumeration of SS string constants
*/

typedef enum {

	///<	separators
	
	SS_SEPARATOR_POUND_STR,			///< Separator Pound String '#'
	SS_SEPARATOR_STAR_STR,			///< Separator Star String '*'
	SS_INTERNATIONAL_PLUS_STR,		///< International call Plus Symbol  '+'

	///<	mode

	SS_MODE_DISABLE_STR,		   ///< Service mode disable string(use this to disable services)
	SS_MODE_ENABLE_STR,			   ///< Service mode enable string(use this to enable services)
	SS_MODE_INTERROGATE_STR,	   ///< Query status string
	SS_MODE_REGISTER_STR,		   ///< Service registration string(use this to register new services)
	SS_MODE_ERASE_STR,			   ///< Service erase mode string(use this to erase or un-register services)

	///<	service class

	SS_SVCCLS_SPEECH_STR,					//< Speech service class string(Telephony and Emergency call)
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

	///<	call forward reason

	SS_CALLFWD_REASON_UNCONDITIONAL_STR,	///< Unconditional Call forwarding
	SS_CALLFWD_REASON_BUSY_STR,				///< Call forward on busy tone 
	SS_CALLFWD_REASON_NO_REPLY_STR,			///< Call forward on no reply
	SS_CALLFWD_REASON_NOT_REACHABLE_STR,	///< Not reachable call forward
	SS_CALLFWD_REASON_ALL_CF_STR,			///< Forward all calls
	SS_CALLFWD_REASON_ALL_CONDITIONAL_STR,	///< Conditional forwarding of all calls

	///<	caller wait, caller ID, connected line ID

	SS_CALLWAIT_PROV_STR,					///< Call Waiting provisioning string
	SS_CALLLINE_PROV_STR,					///< Calling line presentaion provisioning string
	SS_CONNLINE_PROV_STR,					///< Connected line presentation provisioning string
	SS_CALLLINE_REST_STR,					///< Calling line presentation restriction string
	SS_CONNLINE_REST_STR,					///< Connected line presentation restriction string

	///<	call barring

	SS_CALLBAR_TYPE_OUT_ALL_STR,					///< Bar all outgoing calls
	SS_CALLBAR_TYPE_OUT_INTL_STR,					///< Bar all outgoing international calls
	SS_CALLBAR_TYPE_OUT_INTL_EXCL_HPLMN_STR,		///< Bar all outgoing international calls except to Home PLMN
	SS_CALLBAR_TYPE_INC_ALL_STR,					///< Bar all incoming calls 
	SS_CALLBAR_TYPE_INC_ROAM_OUTSIDE_HPLMN_STR,		///< Bar all incoming calls when roaming outside Home PLMN
	SS_CALLBAR_TYPE_ALL_STR,						///< Bar all calls(incoming and outgoing)
	SS_CALLBAR_TYPE_OUTG_STR,						///< Bar outgoing calls, selective
	SS_CALLBAR_TYPE_INC_STR,						///< Bar incoming calls,selective
	
	///<	facility password

	SS_FAC_PASSWORD_STR,						///< Facility Password string

	///<	Calling NAme Presentation

	SS_SRV_CODE_CALLLING_NAME_PRESENT_STR						///< Calling Name presentation string

}	SS_ConstString_t ;


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
	SS_SVCCLS_NOTSPECIFIED,
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
	SS_SVCCLS_UNKNOWN
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
	SS_CALLBAR_TYPE_NOTSPECIFIED,
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

/**
	SS Service Status. These are status returned by the network upon query
*/

//GSM:07.07, section:7.7, argument:<m>
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
	ServiceStatus_t	serviceStatus;		///< GSM:07.07, section:7.7, argument:\<m\>
} SS_ProvisionStatus_t;

/// Call forward information for individual service classes 
typedef struct
{
	Boolean				activated;			///<  TRUE if call forward activated */
	SS_SvcCls_t			ss_class;			///<  Service class: voice, data, fax etc */
	TelephoneNumber_t	forwarded_to_number;///<  The number the call is forwarded to. Note: a "fowarded to"
											///<  number can exist even if call forward is not activated,
											///<  i.e. the number is registered in the network. 
	UInt8				noReplyTime;		///<  5 <= No Reply time <= 30
} CallForwardClassInfo_t;

/// Detailed call forward information with information for individual service classes.
typedef struct
{
	NetworkCause_t netCause;	///<  completion status 
	SS_CallFwdReason_t reason;	///<  Call forward condition: unconditional, user busy etc
	UInt8 class_size;			///< Number of valid service classes in the following list
	CallForwardClassInfo_t call_forward_class_info_list[MAX_SS_CLASS_SIZE]; ///<  Detailed information for each service classes.
} CallForwardStatus_t;

/// SS activation status for a specific service class 
typedef struct
{
	Boolean activated;		///<  TRUE if service activated
	SS_SvcCls_t ss_class;	///<  Service class: voice, data, fax etc  
} SS_ActivationClassInfo_t;

/// Detailed activation status with specific information for individual service class 
typedef struct
{
	NetworkCause_t netCause;	///<  completion status
	UInt8 class_size;			///<  Number of valid service classes in the following list
	SS_ActivationClassInfo_t ss_activation_class_info[MAX_SS_CLASS_SIZE]; ///<  Detailed information for each service classes.	
} SS_ActivationStatus_t;

/// Detailed call barring status with specific information for individual service class 
typedef struct
{
	NetworkCause_t netCause;			///<  completion status 
	SS_CallBarType_t call_barring_type; ///<  Call barring type: bar incoming call, bar outgoing call, etc 
	UInt8 class_size;					///<  Number of valid service classes in the following list 
	SS_ActivationClassInfo_t ss_activation_class_info[MAX_SS_CLASS_SIZE]; ///<  Detailed information for each service classes.																		
} CallBarringStatus_t;

/// SS Call Request Fail
typedef struct {
	Result_t	result ;
}	SsCallReqFail_t ;

/// SS API request Request enumerations
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

#define		PWORD_LEN			4			///< Call barring password length

/// Structure : API Dial String Service Request
typedef struct
{
     ConfigMode_t     fdnCheck;    ///< FDN Check
     ConfigMode_t     stkCheck;    ///< STK Check
     CallType_t       callType;    ///< Call Type
     UInt16           strLen;      ///< SS Operation
     UInt8*           strPtr;      ///< USSD Service Request
} SsApi_DialStrSrvReq_t;           ///< SS API Dial String Service Request Type



#define MAX_FEATURE_LIST_SIZE				13		///< Max Feature List Size
#define MAX_CCBS_LIST_SIZE					5		///< Max CCBS List Size
#define MAX_USSD_SIZE						200		///< Max USSD Size
#define	SS_PASSWORD_LENGTH					4		///< Password Length



/**
Structure : Cause Type, refer to 24.008, section 10.5.4.11
**/
typedef struct
{
	UInt8	codStandard;					///< Coding Standard
	UInt8	location;						///< Location
	UInt8	recommendation;					///< Recommendation
	Cause_t	cause;							///< Cause
	UInt8	diagnostic;						///< Diagnostic
} CauseIe_t;								///< Cause Information Element Type


/**
	Enumeration of SS Operation
**/
typedef enum
{
	SS_OPERATION_CODE_NONE							= 0x00,	///< No Operation code
															///< MAP-Mobile Service Operation
	SS_OPERATION_CODE_FORWARD_CHECK_SS_INDICATION	= 0x26,	///< 38 : Forward Check SS Indication

															///< MAP-SS Operation
	SS_OPERATION_CODE_REGISTER						= 0x0A,	///< 10 : Register
	SS_OPERATION_CODE_ERASE							= 0x0B,	///< 11 : Erase
	SS_OPERATION_CODE_ACTIVATE						= 0x0C,	///< 12 : Activate
	SS_OPERATION_CODE_DEACTIVATE					= 0x0D,	///< 13 : Deactivate
	SS_OPERATION_CODE_INTERROGATE					= 0x0E,	///< 14 : Interrogate
	SS_OPERATION_CODE_REGISTER_PASSWORD				= 0x11,	///< 17 : Register Password
	SS_OPERATION_CODE_GET_PASSWORD					= 0x12,	///< 18 : Get Password
	SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_DATA	= 0x13,	///< 19 : Process Unstructured SS Data
	SS_OPERATION_CODE_PROCESS_UNSTRUCTURED_SS_REQ	= 0x3B,	///< 59 : Process Unstructured SS Request
	SS_OPERATION_CODE_UNSTRUCTURED_SS_REQEST		= 0x3C,	///< 60 : Unstructured SS Request
	SS_OPERATION_CODE_UNSTRUCTURED_SS_NOTIFY		= 0x3D,	///< 61 : Unstructured SS Notify
	SS_OPERATION_CODE_ERASE_CC_ENTRY				= 0x4D,	///< 77 : Erase CC Entry
	SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_CANCEL	= 0x67,	///<109 : LCS Periodic Location Cancellation
	SS_OPERATION_CODE_LCS_LOCATION_UPDATE			= 0x68,	///<110 : LCS Location Update
	SS_OPERATION_CODE_LCS_PERIODIC_LOCATION_REQ		= 0x69,	///<111 : LCS Periodic Location Request
	SS_OPERATION_CODE_LCS_AREA_EVENT_CANCEL			= 0x70,	///<112 : LCS Area Event Cancellation
	SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT			= 0x71,	///<113 : LCS Area Event Report
	SS_OPERATION_CODE_LCS_AREA_EVENT_REQUEST		= 0x72,	///<114 : LCS Area Event Request
	SS_OPERATION_CODE_LCS_MOLR						= 0x73, ///<115 : LCS_MOLR
	SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION		= 0x74, ///<116 : LCS Location Notification

															///< SS Operation
	SS_OPERATION_CODE_NOTIFY_SS						= 0x10, ///< 16 : Notify SS
	SS_OPERATION_CODE_CALL_DEFLECTION				= 0x75, ///<117 : Call Deflection
	SS_OPERATION_CODE_USER_USER_SERVICE				= 0x76, ///<118 : User User Service
	SS_OPERATION_CODE_ACCESS_REGISTER_CC_ENTRY		= 0x77, ///<119 : Access Register CC Entry
	SS_OPERATION_CODE_FORWARD_CUG_INFO				= 0x78, ///<120 : Forward CUG-Info
	SS_OPERATION_CODE_SPLIT_MPTY					= 0x79, ///<121 : Split MPTY
	SS_OPERATION_CODE_RETIEVE_MPTY					= 0x7A, ///<122 : Retrieve MPTY
	SS_OPERATION_CODE_HOLD_MPTY						= 0x7B, ///<123 : Hold MPTY
	SS_OPERATION_CODE_BUILD_MPTY					= 0x7C, ///<124 : Build MPTY
	SS_OPERATION_CODE_FORWARD_CHARGE_ADVICE			= 0x7D, ///<125 : Forward Charge Advice
	SS_OPERATION_CODE_EXPLICIT_CT					= 0x7E  ///<126 : Explicit CT
} SS_Operation_t;


/**
	Enumeration of Basic Service String
**/
typedef enum
{
	STR_BEARER_SRV_ALL_BEARER_SERVICES				= 20,
	STR_BEARER_SRV_ALL_ASYNCHRONOUS_SERVICES		= 21, 
	STR_BEARER_SRV_ALL_SYNCHRONOUS_SERVICES			= 22,
	STR_BEARER_SRV_ALL_DATA_CIRCUIT_SYNCH			= 24,
	STR_BEARER_SRV_ALL_DATA_CIRCUIT_ASYNCH			= 25,
	STR_BEARER_SRV_ALL_PLMN_SPECIFIC_BASIC_SRV		= 70,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_1		= 71,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_2		= 72,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_3		= 73,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_4		= 74,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_5		= 75,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_6		= 76,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_7		= 77,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_8		= 78,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_9		= 79,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_A		= 80,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_B		= 81,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_C		= 82,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_D		= 83,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_E		= 84,
	STR_BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_F		= 85,
	STR_BEARER_SRV_ALL_DATA_CDA_SERVICES			= 99,
	STR_BEARER_SRV_DATA_CDA_300_BPS					= 99,
	STR_BEARER_SRV_DATA_CDA_1200_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_1200_75_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_2400_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_4800_BPS				= 99,
	STR_BEARER_SRV_DATA_CDA_9600_BPS				= 99,
	STR_BEARER_SRV_ALL_DATA_CDS_SERVICES			= 99,
	STR_BEARER_SRV_DATA_CDS_1200_BPS				= 99,
	STR_BEARER_SRV_DATA_CDS_2400_BPS				= 99,
	STR_BEARER_SRV_DATA_CDS_4800_BPS				= 99,
	STR_BEARER_SRV_DATA_CDS_9600_BPS				= 99,
	STR_BEARER_SRV_ALL_PAD_ACCESS_CA_SERVICES		= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_300_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_1200_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_1200_75_BPS		= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_2400_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_4800_BPS			= 99,
	STR_BEARER_SRV_PAD_ACCESS_CA_9600_BPS			= 99,
	STR_BEARER_SRV_ALL_DATA_PDS_SERVICES			= 99,
	STR_BEARER_SRV_DATA_PDS_2400_BPS				= 99,
	STR_BEARER_SRV_DATA_PDS_4800_BPS				= 99,
	STR_BEARER_SRV_DATA_PDS_9600_BPS				= 99,
	STR_BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDA		= 99,
	STR_BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDS		= 99,
	STR_BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDA		= 99,
	STR_BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDS		= 99,

	STR_TELE_SRV_ALL_TELE_SERVICES					= 10,
	STR_TELE_SRV_ALL_SPEECH_TRANSMISSION_SERVICES	= 11,
	STR_TELE_SRV_TELEPHONY							= 11,
	STR_TELE_SRV_EMERGENCY_CALLS					= 12,
	STR_TELE_SRV_ALL_DATA_TELE_SERVICES				= 12,
	STR_TELE_SRV_ALL_FACIMILE_TRANSMISSION_SERVICES	= 13,
	STR_TELE_SRV_FACIMILE_GROUP_3_ALTER_SPEECH		= 13,
	STR_TELE_SRV_AUTOMATIC_FACIMILE_GROUP_3			= 13,
	STR_TELE_SRV_FACIMILE_GROUP_4					= 13,
	STR_TELE_SRV_ALL_SHORT_MESSAGE_SERVICES			= 16,
	STR_TELE_SRV_SHORT_MESSAGE_MT_PP				= 16,
	STR_TELE_SRV_SHORT_MESSAGE_MO_PP				= 16,
	STR_TELE_SRV_VOICE_GROUP_CALL_SERVICE			= 17,
	STR_TELE_SRV_VOICE_BROADCAST_SERVICE			= 18,
	STR_TELE_SRV_ALL_TELE_SRV_EXCEPT_SMS			= 19,
	STR_TELE_SRV_TELEPHONY_AND_ALL_SYNC_SERVICES	= 26,
	STR_TELE_SRV_ALL_PLMN_SPECIFIC_SERVICES			= 50,
	STR_TELE_SRV_PLMN_SPECIFIC_1					= 51,
	STR_TELE_SRV_PLMN_SPECIFIC_2					= 52,
	STR_TELE_SRV_PLMN_SPECIFIC_3					= 53,
	STR_TELE_SRV_PLMN_SPECIFIC_4					= 54,
	STR_TELE_SRV_PLMN_SPECIFIC_5					= 55,
	STR_TELE_SRV_PLMN_SPECIFIC_6					= 56,
	STR_TELE_SRV_PLMN_SPECIFIC_7					= 57,
	STR_TELE_SRV_PLMN_SPECIFIC_8					= 58,
	STR_TELE_SRV_PLMN_SPECIFIC_9					= 59,
	STR_TELE_SRV_PLMN_SPECIFIC_A					= 60,
	STR_TELE_SRV_PLMN_SPECIFIC_B					= 61,
	STR_TELE_SRV_PLMN_SPECIFIC_C					= 62,
	STR_TELE_SRV_PLMN_SPECIFIC_D					= 63,
	STR_TELE_SRV_PLMN_SPECIFIC_E					= 64,
	STR_TELE_SRV_PLMN_SPECIFIC_F					= 65,
	STR_TELE_SRV_ALL_AUXILIARY_TELEPHONY			= 89
} BasicSrvSrvCode_t;


/**
	Enumeration of Basic Service Type
**/
typedef enum
{
	BASIC_SERVICE_TYPE_UNSPECIFIED		= 0x00,	///<  0 : Unspecified service type
	BASIC_SERVICE_TYPE_UNKNOWN			= 0x01,	///<  1 : Unknown service type
	BASIC_SERVICE_TYPE_BEARER_SERVICES	= 0x82, ///<130 : Bearer Services		
	BASIC_SERVICE_TYPE_TELE_SERVICES	= 0x83	///<131 : Tele Services
} BasicSrvType_t;

/**
	Enumeration of Bearer Service Group
**/
typedef enum
{
	BEARER_SRV_GROUP_ALL_BEARER_SERVICES				= 0x00, ///< 00 : All Bearer Services
	BEARER_SRV_GROUP_ALL_DATA_CDA_SERVICES				= 0x01, ///< 01 : All Data CDA Services
	BEARER_SRV_GROUP_ALL_PAD_ACCESS_CA_SERVICES			= 0x02, ///< 02 : All PAD Access CA Services
	BEARER_SRV_GROUP_ALL_ALTERNATIVE_SPEECH_DATA_CDA	= 0x03, ///< 03 : All Alternative speech Data CDA
	BEARER_SRV_GROUP_ALL_SPEECH_FOLLOWED_BY_DATA_CDA	= 0x04, ///< 04 : All Speech Followed By Data CDA
	BEARER_SRV_GROUP_ALL_DATA_CIRCUIT_ASYNCHRONOUS		= 0x05, ///< 05 : All Data Circuit Asynchronous
	BEARER_SRV_GROUP_ALL_SYNC_ASYNCHRONOUS_SERVICES		= 0x06, ///< 06 : All Sync/Asynchronous Services
	BEARER_SRV_GROUP_ALL_PLMN_SPECIFIC_SERVICES			= 0x0D  ///< 14 : All PLMN Specific Services
} BearerSrvGroup_t;												///< Bearer Service Group Type

/**
	Enumeration of Bearer Services
**/
typedef enum
{
	BEARER_SRV_ALL_BEARER_SERVICES			= 0x00, ///< 00 : All Bearer Services
	
	BEARER_SRV_ALL_DATA_CDA_SERVICES		= 0x10, ///< 16 : All Data CDA Services
	BEARER_SRV_DATA_CDA_300_BPS				= 0x11, ///< 17 : Data CDA 300 bps
	BEARER_SRV_DATA_CDA_1200_BPS			= 0x12, ///< 18 : Data CDA 1200 bps
	BEARER_SRV_DATA_CDA_1200_75_BPS			= 0x13, ///< 19 : Data CDA 1200-75 bps
	BEARER_SRV_DATA_CDA_2400_BPS			= 0x14, ///< 20 : Data CDA 2400 bps
	BEARER_SRV_DATA_CDA_4800_BPS			= 0x15, ///< 21 : Data CDA 4800 bps
	BEARER_SRV_DATA_CDA_9600_BPS			= 0x16, ///< 22 : Data CDA 9600 bps

	BEARER_SRV_ALL_DATA_CDS_SERVICES		= 0x18, ///< 24 : All Data CDS Services
	BEARER_SRV_DATA_CDS_1200_BPS			= 0x1A, ///< 26 : Data CDS 1200 bps
	BEARER_SRV_DATA_CDS_2400_BPS			= 0x1C, ///< 28 : Data CDS 2400 bps
	BEARER_SRV_DATA_CDS_4800_BPS			= 0x1D, ///< 29 : Data CDS 4800 bps
	BEARER_SRV_DATA_CDS_9600_BPS			= 0x1E, ///< 30 : Data CDS 9600 bps

	BEARER_SRV_ALL_PAD_ACCESS_CA_SERVICES	= 0x20, ///< 32 : All PAD Access CA Services
	BEARER_SRV_PAD_ACCESS_CA_300_BPS		= 0x21, ///< 33 : PAD ACCESS CA 300 bps
	BEARER_SRV_PAD_ACCESS_CA_1200_BPS		= 0x22, ///< 34 : PAD ACCESS CA 1200 bps
	BEARER_SRV_PAD_ACCESS_CA_1200_75_BPS	= 0x23, ///< 35 : PAD ACCESS CA 1200-75 bps
	BEARER_SRV_PAD_ACCESS_CA_2400_BPS		= 0x24, ///< 36 : PAD ACCESS CA 2400 bps
	BEARER_SRV_PAD_ACCESS_CA_4800_BPS		= 0x25, ///< 38 : PAD ACCESS CA 4800 bps
	BEARER_SRV_PAD_ACCESS_CA_9600_BPS		= 0x26, ///< 39 : PAD ACCESS CA 9600 bps

	BEARER_SRV_ALL_DATA_PDS_SERVICES		= 0x28, ///< 41 : All Data PDS Services
	BEARER_SRV_DATA_PDS_2400_BPS			= 0x2C, ///< 45 : Data PDS 2400 bps
	BEARER_SRV_DATA_PDS_4800_BPS			= 0x2D, ///< 46 : Data PDS 4800 bps
	BEARER_SRV_DATA_PDS_9600_BPS			= 0x2E, ///< 47 : Data PDS 9600 bps

	BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDA	= 0x30, ///< 40 : All Alternate Speech Data CDA
	BEARER_SRV_ALL_ALTER_SPEECH_DATA_CDS	= 0x38, ///< 48 : All Alternate Speech Data CDS

	BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDA	= 0x40, ///< 64 : All Speech Followd Data CDA
	BEARER_SRV_ALL_SPEECH_FOLLOWED_DATA_CDS	= 0x48, ///< 72 : All Speech Followed Data CDS

	BEARER_SRV_ALL_DATA_CIRCUIT_ASYNCH		= 0x50, ///< 80 : All Data Circuit Asynchronous
	BEARER_SRV_ALL_DATA_CIRCUIT_SYNCH		= 0x58, ///< 88 : All Data Circuit Syncronous

	BEARER_SRV_ALL_ASYNCHRONOUS_SERVICES	= 0x60, ///< 96 : All Asynchronous Services
	BEARER_SRV_ALL_SYNCHRONOUS_SERVICES		= 0x68, ///<104 : All Synchronous Services

	BEARER_SRV_ALL_PLMN_SPECIFIC_BASIC_SRV	= 0xD0, ///<208 : All PLMN Specific Services
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_1	= 0xD1, ///<209 : PLMN Specific Basic Service 1
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_2	= 0xD2, ///<210 : PLMN Specific Basic Service 2
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_3	= 0xD3, ///<211 : PLMN Specific Basic Service 3
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_4	= 0xD4, ///<212 : PLMN Specific Basic Service 4
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_5	= 0xD5, ///<213 : PLMN Specific Basic Service 5
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_6	= 0xD6, ///<214 : PLMN Specific Basic Service 6
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_7	= 0xD7, ///<215 : PLMN Specific Basic Service 7
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_8	= 0xD8, ///<216 : PLMN Specific Basic Service 8
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_9	= 0xD9, ///<217 : PLMN Specific Basic Service 9
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_A	= 0xDA, ///<218 : PLMN Specific Basic Service A
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_B	= 0xDB, ///<219 : PLMN Specific Basic Service B
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_C	= 0xDC, ///<220 : PLMN Specific Basic Service C
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_D	= 0xDD, ///<221 : PLMN Specific Basic Service D
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_E	= 0xDE, ///<222 : PLMN Specific Basic Service E
	BEARER_SRV_PLMN_SPECIFIC_BASIC_SRV_F	= 0xDF  ///<223 : PLMN Specific Basic Service F
} BearerSrv_t;

/**
	Enumeration of Tele Service Group
**/
typedef enum
{
	TELE_SRV_GROUP_ALL_TELE_SERVICES					= 0x00, ///< 00 : All Tele Services
	TELE_SRV_GROUP_ALL_SPEECH_TRANSMISSION_SERVICES		= 0x01, ///< 01 : All Speech Transmission Services
	TELE_SRV_GROUP_ALL_SHORT_MESSAGE_SERVICES			= 0x02, ///< 02 : All Short Message Services
	TELE_SRV_GROUP_ALL_FACIMILE_TRANSMISSION_SERVICES	= 0x06, ///< 06 : All Facimile Transmission Services
	TELE_SRV_GROUP_ALL_DATA_TELE_SERVICES				= 0x07, ///< 07 : All Data Tele Services
	TELE_SRV_GROUP_ALL_TELE_SERVICES_EXCEPT_SMS			= 0x08, ///< 08 : All Tele Services Except SMS
	TELE_SRV_GROUP_ALL_PLMN_SPECIFIC_SERVICES			= 0x0D  ///< 14 : All PLMN Specific Services
} TeleSrvGroup_t;

/**
	Enumeration of Tele Service
**/
typedef enum
{
	TELE_SRV_ALL_TELE_SERVICES					= 0x00, ///< 00 : All Tele Services
	
	TELE_SRV_ALL_SPEECH_TRANSMISSION_SERVICES	= 0x10, ///< 16 : All Speech Transmission Services
	TELE_SRV_TELEPHONY							= 0x11, ///< 17 : Telephony
	TELE_SRV_EMERGENCY_CALLS					= 0x12, ///< 18 : Emergency Calls

	TELE_SRV_ALL_SHORT_MESSAGE_SERVICES			= 0x20, ///< 32 : All Short Message Services
	TELE_SRV_SHORT_MESSAGE_MT_PP				= 0x21, ///< 33 : Short Message MT PP
	TELE_SRV_SHORT_MESSAGE_MO_PP				= 0x22, ///< 34 : Short Message MP PP

	TELE_SRV_ALL_AUXILIARY_TELEPHONY			= 0x59, ///< 89 : ALS (Alternative Line Service) added to GSM 02.30, see Appendix 1 of CPHS spec

	TELE_SRV_ALL_FACIMILE_TRANSMISSION_SERVICES	= 0x60, ///< 96 : All Facimile Transmission Services
	TELE_SRV_FACIMILE_GROUP_3_ALTER_SPEECH		= 0x61, ///< 97 : Facimile Group 3 Alter Speech
	TELE_SRV_AUTOMATIC_FACIMILE_GROUP_3			= 0x62, ///< 98 : Automatic Facimile Group 3
	TELE_SRV_FACIMILE_GROUP_4					= 0x63, ///< 99 : Facimile Group 4

	TELE_SRV_ALL_DATA_TELE_SERVICES				= 0x70, ///<112 : All Data Tele Services

	TELE_SRV_ALL_TELE_SRV_EXCEPT_SMS			= 0x80, ///<128 : All Tele Service Except SMS

	TELE_SRV_TELEPHONY_AND_ALL_SYNC_SERVICES	= 0x90, ///<144 : Telephony And All Sync Services
	TELE_SRV_VOICE_GROUP_CALL_SERVICE			= 0x91, ///<145 : Voice Group Call Service
	TELE_SRV_VOICE_BROADCAST_SERVICE			= 0x92, ///<146 : Voice Broadcast Service

	TELE_SRV_ALL_PLMN_SPECIFIC_SERVICES			= 0xD0, ///<208 : All PLMN Specific Services
	TELE_SRV_PLMN_SPECIFIC_1					= 0xD1, ///<209 : PLMN Specific 1
	TELE_SRV_PLMN_SPECIFIC_2					= 0xD2, ///<210 : PLMN Specific 2
	TELE_SRV_PLMN_SPECIFIC_3					= 0xD3, ///<211 : PLMN Specific 3
	TELE_SRV_PLMN_SPECIFIC_4					= 0xD4, ///<212 : PLMN Specific 4
	TELE_SRV_PLMN_SPECIFIC_5					= 0xD5, ///<213 : PLMN Specific 5
	TELE_SRV_PLMN_SPECIFIC_6					= 0xD6, ///<214 : PLMN Specific 6
	TELE_SRV_PLMN_SPECIFIC_7					= 0xD7, ///<215 : PLMN Specific 7
	TELE_SRV_PLMN_SPECIFIC_8					= 0xD8, ///<216 : PLMN Specific 8
	TELE_SRV_PLMN_SPECIFIC_9					= 0xD9, ///<217 : PLMN Specific 9
	TELE_SRV_PLMN_SPECIFIC_A					= 0xDA, ///<218 : PLMN Specific A
	TELE_SRV_PLMN_SPECIFIC_B					= 0xDB, ///<219 : PLMN Specific B
	TELE_SRV_PLMN_SPECIFIC_C					= 0xDC, ///<220 : PLMN Specific C
	TELE_SRV_PLMN_SPECIFIC_D					= 0xDD, ///<221 : PLMN Specific D
	TELE_SRV_PLMN_SPECIFIC_E					= 0xDE, ///<222 : PLMN Specific E
	TELE_SRV_PLMN_SPECIFIC_F					= 0xDF  ///<223 : PLMN Specific F
} TeleSrv_t;


/**
 Structure : Basic Service Group
**/
typedef struct
{
	BasicSrvType_t	type;
	UInt8			content;					///< Content is of type BearerSrv_t/TeleSrv_t
}BasicSrvGroup_t;								///< Basic Service Group Type

/**
enum : SS Code 
**/
typedef enum
{
	SS_CODE_ALL_SS						= 0x00, ///<  0 : All SS
	SS_CODE_ALL_LINE_IDENTIFICATION		= 0x10, ///< 16 : All Line Identification SS
	SS_CODE_CLIP						= 0x11, ///< 17 : Calling Line Identification Presentation
	SS_CODE_CLIR						= 0x12, ///< 18 : Calling Line Identification Restriction
	SS_CODE_COLP						= 0x13, ///< 19 : Connected Line Identification Presetation
	SS_CODE_COLR						= 0x14, ///< 20 : Connected Line Identification Restriction
	SS_CODE_MCI							= 0x15, ///< 21 : Malicious Call Identification
	SS_CODE_ALL_NAME_IDENTIFICATION		= 0x18, ///< 24 : All Name Identification SS
	SS_CODE_CNAP						= 0x19, ///< 25 : Calling Name Presentation

	SS_CODE_ALL_FORWARDING				= 0x20, ///< 32 : All Forwarding
	SS_CODE_CFU							= 0x21, ///< 33 : Call Forwarding Unconditional
	SS_CODE_CALL_DEFLECTION				= 0x24, ///< 36 : Call Deflection
	SS_CODE_ACF							= 0x28, ///< 40 : All Conditional Forwarding
	SS_CODE_CFB							= 0x29, ///< 41 : Call Forwarding On Mobile Subscriber Busy
	SS_CODE_CFNRY						= 0x2A, ///< 42 : Call Forwarding On No Reply
	SS_CODE_CFNRC						= 0x2B, ///< 43 : Call Forwarding On Mobile Subscriber Not Reachable

	SS_CODE_ALL_CALL_OFFERING			= 0x30, ///< 48 : All Call Offering SS Includes All Forwarding SS
	SS_CODE_ECT							= 0x31, ///< 49 : Explicit Call Transfer
	SS_CODE_MAH							= 0x32, ///< 50 : Mobile Access Hunting

	SS_CODE_ALL_CALL_COMPLETION			= 0x40, ///< 64 : All Call Completion SS
	SS_CODE_CW							= 0x41, ///< 65 : Call Waiting
	SS_CODE_HOLD						= 0x42, ///< 66 : CAll Hold
	SS_CODE_CCBS_A						= 0x43, ///< 67 : Complition Of Call To Busy Subscriber (Originating Side)
	SS_CODE_CCBS_B						= 0x44, ///< 68 : Complition Of Call To Busy Subscriber (Terminating Side)
	SS_CODE_MC							= 0x45, ///< 69 : Multi Call

	SS_CODE_ALL_MPTY					= 0x50, ///< 80 : All Multiparty SS
	SS_CODE_MPTY						= 0x51, ///< 81 : Multiparty

	SS_CODE_ALL_COMMUNITY_OF_INTEREST	= 0x60, ///< 96 : All Community Of Interest SS
	SS_CODE_CUG							= 0x61, ///< 97 : Closed User Group

	SS_CODE_ALL_CHARGING_SS				= 0x70, ///<112 : All Charging SS
	SS_CODE_AOCI						= 0x71, ///<113 : Advice Of Charge Information
	SS_CODE_AOCC						= 0x72, ///<114 : Advice Of Charge Charging

	SS_CODE_ALL_ADD_INFO_TRANSFER		= 0x80, ///<128 : All Additional Information Transfer
	SS_CODE_UUS_1						= 0x81, ///<129 : User-To-User Signalling 1
	SS_CODE_UUS_2						= 0x82, ///<130 : User-To-User Signalling 2
	SS_CODE_UUS_3						= 0x83, ///<131 : User-To-User Signalling 3

	SS_CODE_ALL_CALL_RESTRICTION		= 0x90, ///<144 : All Call Restriction SS
	SS_CODE_BOC							= 0x91, ///<145 : Barring Of Outgoing Calls
	SS_CODE_BAOC						= 0x92, ///<146 : Barring Of All Outgoing Calls
	SS_CODE_BOIC						= 0x93, ///<147 : Barring Of Outgoing International Calls
	SS_CODE_BOIC_EXC_PLMN				= 0x94, ///<148 : Barring Of Outgoing International Calls Except Those Directed To The Home PLMN
	SS_CODE_BIC							= 0x99, ///<153 : Barring Of Incomming Calls
	SS_CODE_BAIC						= 0x9A, ///<154 : Barring Of All Incoming Calls
	SS_CODE_BAIC_ROAM					= 0x9B, ///<155 : Barring Of All Incoming Calls When Roaming Outside Home PLMN_Country

	SS_CODE_ALL_CALL_PRIORITY			= 0xA0, ///<160 : All Call Priority SS
	SS_CODE_EMLPP						= 0xA1, ///<161 : Enhanced Multilevel Precedence Pre-emption (EMLPP) Service

	SS_CODE_ALL_LCS_PRIVACY_EXCEPTION	= 0xB0, ///<176 : All LCS Privacy Exception - All LCS Privacy Exception Classes
	SS_CODE_UNIVERSAL_LCS				= 0xB1, ///<177 : Universal - Allow Location By Any LCS Client
	SS_CODE_CALL_RELATED_LCS			= 0xB2, ///<178 : Call Related - Allow Location By Any Value Added LCS Client To Which A Call Is Established From The Target MS
	SS_CODE_CALL_UNRELATED_LCS			= 0xB3, ///<179 : Call Unrelated - Allow Location By Designated External value Added LCS Client
	SS_CODE_PLMN_OPERATOR_LCS			= 0xB4, ///<180 : PLMN Operator - Allow Location By designated By PLMN Operator LCS Clients

	SS_CODE_ALL_MORLR					= 0xC0, ///<192 : All MORL SS - All Mobile Originating Location Request Classes
	SS_CODE_BASIC_SELF_LOCATION			= 0xC1, ///<193 : Basic Self Location- Allow An MS To Request Its Own Location
	SS_CODE_AUTONOMOUS_SELF_LOCATION	= 0xC2, ///<194 : Auto Nomous Self Location - Allow An MS To Perform Self Location Without Interaction With The PLMN For A Predetermined Period Of Time
	SS_CODE_TRANSFER_TO_THIRD_PARTY		= 0xC3, ///<195 : Transfer To Third Party - Allow An MS To Request Transfer Of Its Location To Another LCS Client

	SS_CODE_ALL_PLMN_SPECIFIC_SS		= 0xF0, ///<208 : All PLMN Specific SS
	SS_CODE_PLMN_SPECIFIC_SS_1			= 0xF1, ///<209 : PLMN Specific SS 1
	SS_CODE_PLMN_SPECIFIC_SS_2			= 0xF2, ///<210 : PLMN Specific SS 2
	SS_CODE_PLMN_SPECIFIC_SS_3			= 0xF3, ///<211 : PLMN Specific SS 3
	SS_CODE_PLMN_SPECIFIC_SS_4			= 0xF4, ///<212 : PLMN Specific SS 4
	SS_CODE_PLMN_SPECIFIC_SS_5			= 0xF5, ///<213 : PLMN Specific SS 5
	SS_CODE_PLMN_SPECIFIC_SS_6			= 0xF6, ///<214 : PLMN Specific SS 6
	SS_CODE_PLMN_SPECIFIC_SS_7			= 0xF7, ///<215 : PLMN Specific SS 7
	SS_CODE_PLMN_SPECIFIC_SS_8			= 0xF8, ///<216 : PLMN Specific SS 8
	SS_CODE_PLMN_SPECIFIC_SS_9			= 0xF9, ///<217 : PLMN Specific SS 9
	SS_CODE_PLMN_SPECIFIC_SS_A			= 0xFA, ///<218 : PLMN Specific SS A
	SS_CODE_PLMN_SPECIFIC_SS_B			= 0xFB, ///<219 : PLMN Specific SS B
	SS_CODE_PLMN_SPECIFIC_SS_C			= 0xFC, ///<220 : PLMN Specific SS C
	SS_CODE_PLMN_SPECIFIC_SS_D			= 0xFD, ///<221 : PLMN Specific SS D
	SS_CODE_PLMN_SPECIFIC_SS_E			= 0xFE, ///<222 : PLMN Specific SS E
	SS_CODE_PLMN_SPECIFIC_SS_F			= 0xFF  ///<223 : PLMN Specific SS F
} SS_Code_t;										///<SS Code type


/**
enum : Notification to Calling Party
**/
typedef enum
{
	SS_PARTY_NO_NOTIFICATION,					///< No Notification
	SS_PARTY_NOTIFICATION						///< Notification
} SS_PartyNotif_t;								///< Notification To Calling/Forwarding Party

/**
enum : Forwarding Reason
**/
typedef enum
{
	SS_FWD_REASON_ME_NOT_REACHABLE,				///< MS Not Reachable
	SS_FWD_REASON_ME_BUSY,						///< MS Busy
	SS_FWD_REASON_NO_REPLY,						///< No Reply
	SS_FWD_REASON_UNCONDITIONAL					///< Unconditional 
} SS_FwdReason_t;								///< Forwarding Reason Type

/**
structure : Forwarding Option
**/
typedef struct
{
	SS_PartyNotif_t	notifFwd;				///< Notification to Forwarding Party
	SS_PartyNotif_t	notifCalling;			///< Notification to Calling Party
	SS_FwdReason_t	fwdReason;				///< Forwarding Reason
} SS_FwdOption_t;								///< Forwarding Option

/**
Enum : Subscription Option Type
**/
typedef enum
{
	SS_SUBSCRIPTION_OPTION_CLI_RESTRICTION		= 0x82,	///< CLI Restriction Option
	SS_SUBSCRIPTION_OPTION_OVERRIDE_CATEGORY	= 0x81	///< Override Category
} SS_SubsOptionType_t;									///< Subscription Option Type

/**
Enum : CLI Restriction Option Type
**/
typedef enum
{
	SS_CLI_RESTRICTION_PERMENENT			= 0x00,	///< Permenent
	SS_CLI_RESTRICTION_TEMPORARY_RESTRICTED	= 0x01,	///< Temporary (Default Restricted)
	SS_CLI_RESTRICTION_TEMPORARY_ALLOWED	= 0x02	///< Temporary (Default Allowed)  
} SS_ClirOption_t;									///< CLI Restriction Option

/**
Enum : Override Option 
**/
typedef enum
{
	SS_OVERRIDE_CATEGORY_OVERRIDE_ENABLE	= 0x00,	///< Override Enable
	SS_OVERRIDE_CATEGORY_OVERRIDE_DISABLE	= 0x01	///< Override Disable
} SS_OverrideOption_t;								///< Override Category Option

/**
Structure : Subscription Option
**/
typedef struct
{
	SS_SubsOptionType_t	type;					///< Subscription Option Type
	UInt8				content;				///< Content of type SS_ClirOption_t/SS_OverrideOption_t
}SS_SubsOption_t;								///< Subscription Option

/**
Structure : Forwarding Feature
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^			   
			basicSrv								  X		///< Basic Service Group
			ssStatus							  X 		///< SS Status
			partyAdd						  X				///< Party Address
			phoneSubAdd					  X					///< Phone Number Subaddress
			fwdOption				  X						///< Forwarding Option
			noReplyTime			  X			 				///< No Reply Time
**/
typedef struct
{
	UInt8				include;							///< Include Options
	BasicSrvGroup_t		basicSrv;							///< Basic Service Group
	UInt8				ssStatus;							///< SS Status
	PartyAddress_t		partyAdd;							///< Party Address
	PartySubAdd_t		partySubAdd;						///< Party Subaddress
	SS_FwdOption_t		fwdOption;							///< Forwarding Option
	UInt8				noReplyTime;						///< No Reply Time
} SS_FwdFeature_t;											///< Forwarding Feature Type

/**
Structure : Forwarding Feature List
**/
typedef struct
{
	UInt8			listSize;								///< List Size
	SS_FwdFeature_t	fwdFeatureList[MAX_FEATURE_LIST_SIZE];	///< Forwarding Feature List
} SS_FwdFeatureInfo_t;										///< Forwarding Info

/**
Structure : Call Barring Feature
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^	   
			basicSrv								  X		///< Basic Service Group
			ssStatus							  X 		///< SS Status
**/
typedef struct
{
	UInt8			include;								///< Include Options
	BasicSrvGroup_t basicSrv;								///< Basic Service Group
	UInt8			ssStatus;								///< SS Status
} SS_CallBarFeature_t;										///< Call Barring Feature Type

/**
Structure : Call Barring Feature List
**/
typedef struct
{
	UInt8				listSize;									///< List Size
	SS_CallBarFeature_t	callBarFeatureList[MAX_FEATURE_LIST_SIZE];	///< Call Barring Feature List
} SS_CallBarInfo_t;													///< Call Barring Information

/**
Structure : SS Data Information
			include       8   7   6   5   4   3   2   1
						|   |   |   |   |   |   |   |   |
						  ^   ^   ^   ^   ^   ^   ^   ^					   
			ssStatus								  X		///< SS Status
			subsOpt								  X 		///< Subscription Option
**/
typedef struct
{
	UInt8			include;								///< Include Options
	UInt8			ssStatus;								///< SS Status
	SS_SubsOption_t	subsOpt;								///< Subscription Option
	UInt8			listSize;								///< List Size
	BasicSrvGroup_t	basicSrvGroupList[MAX_FEATURE_LIST_SIZE];///< Basic Service Group List
} SS_SsDataInfo_t;											///< SS data Information


/**
Structure : SS Basic Service Information
**/
typedef struct
{
	UInt8				listSize;									///< List Size
	BasicSrvGroup_t		basicSrvGroupList[MAX_FEATURE_LIST_SIZE];	///< Basic Service Group List
} SS_BasicSrvInfo_t;												///< SS data Information


/**
Structure : CCBS Feature
			include			  8   7   6   5   4   3   2   1
							|   |   |   |   |   |   |   |   |
							  ^   ^   ^   ^   ^   ^   ^   ^					   
			partyAdd									  X		///< CCBS Index
			partyAdd								  X			///< Party Address
			partySubAdd							  X 			///< Party Subaddress
			basicSrv						  X					///< Basic Services
**/
typedef struct
{
	UInt8				include;				///< Included Option
	UInt16				ccbsIndex;				///< CCBS Index
	PartyAddress_t		partyAdd;				///< Party Address
	PartySubAdd_t		partySubAdd;			///< Party Subaddress
	BasicSrvGroup_t 	basicSrv;				///< Basic Service Group
} SS_CcbsFeature_t;								///< CCBS Feature Type


/**
Structure : CCBS Feature List
**/
typedef struct
{
	UInt8				listSize;						///< List Size
	SS_CcbsFeature_t 	ccbsList[MAX_CCBS_LIST_SIZE];	///< CCBS Feature List
} SS_CcbsFeatureInfo_t;									///< CCBS Feature Information Type


/**
Structure : Generic Service Information
			include			  8   7   6   5   4   3   2   1
							|   |   |   |   |   |   |   |   |
							  ^   ^   ^   ^   ^   ^   ^   ^					   
			clir										  X		///< CLI Restriction Option
			maxEntPrio								  X			///< Max Entitle Priority Option
			defaultPrio							  X				///< Default Priority Option
			ccbsFeatureInfo					  X					///< CCBS Feature Information
**/
typedef struct
{
	UInt8					ssStatus;			///< SS Status

	UInt8					include;			///< Include Options
	SS_ClirOption_t			clir;				///< CLI Restriction Option
	UInt16					maxEntPrio;			///< Max Entitle Priority Option
	UInt16					defaultPrio;		///< Default Priority
	SS_CcbsFeatureInfo_t	ccbsFeatureInfo;	///< CCBS Feature Information
} SS_GenSrvInfo_t;								///< Generic Service Information

/**
enum : SS Service Response Type
**/
typedef enum
{
	SS_SRV_TYPE_NONE,						///< NONE
	SS_SRV_TYPE_FORWARDING_INFORMATION,		///< Forwarding Information
	SS_SRV_TYPE_CALL_BARRING_INFORMATION,	///< Call Barring Information
	SS_SRV_TYPE_SS_DATA_INFORMATION,		///< Call Barring Information
	SS_SRV_TYPE_SS_STATUS,					///< SS Status
	SS_SRV_TYPE_FORWARDED_TO_NUMBER,		///< Forwarded To Number
	SS_SRV_TYPE_BASIC_SRV_INFORMATION,		///< Basic Service Group Information
	SS_SRV_TYPE_GENERIC_SRV_INFORMATION,	///< Generic Service Information
	SS_SRV_TYPE_NEW_PASSWORD,				///< New Passsord
	SS_SRV_TYPE_PH1_USSD_INFORMATION,		///< Phase 1 USSD Information
	SS_SRV_TYPE_PH2_USSD_INFORMATION,		///< Phase 2 USSD Information
	SS_SRV_TYPE_USSD_SS_NOTIFY,				///< USSD SS Notify
	SS_SRV_TYPE_RETURN_ERROR,				///< Return Error
	SS_SRV_TYPE_REJECT,						///< Reject
	SS_SRV_TYPE_LOCAL_ERROR,				///< Local Error (Internal Problem)
	SS_SRV_TYPE_CAUSE_IE					///< Network Cause IE
} SS_SrvType_t;								///< Service Response Type

/**
Enum : Component Type
**/
typedef enum
{
	SS_COMPONENT_TYPE_NONE,						///< Component Type None
	SS_COMPONENT_TYPE_INVOKE		= 0xA1,		///< Component Type Invoke
	SS_COMPONENT_TYPE_RETURN_RESULT	= 0xA2,		///< Component Type Return Result
	SS_COMPONENT_TYPE_RETURN_ERROR	= 0xA3,		///< Component Type Return Error
	SS_COMPONENT_TYPE_REJECT		= 0xA4		///< Component Type Reject
} SS_Component_t;								///< SS Component

/**
Enum : Error Code
**/
/*
typedef enum
{
	SS_ERROR_CODE_NONE								= 0,	///< 0x00 None
	SS_ERROR_CODE_UNKNOWN_SUBSCRIBER				= 1,	///< 0x01 Unknown Subscriber
	SS_ERROR_CODE_ILLEGAL_SUBSCRIBER				= 9,	///< 0x09 Illegal Subscriber
	SS_ERROR_CODE_BEARER_SVC_NOT_PROVISIONED		= 10,	///< 0x0A Bearer Service Not Provisioned
	SS_ERROR_CODE_TELE_SVC_NOT_PROVISIONED			= 11,	///< 0x0B Tele Service Not Provisioned
	SS_ERROR_CODE_ILLEGAL_EQUIPMENT					= 12,	///< 0x0C Illegal Equipment
	SS_ERROR_CODE_CALL_BARRED						= 13,	///< 0x0D Call Barred
	SS_ERROR_CODE_ILLEGAL_SS_OPERATION				= 16,	///< 0x10 Illegal SS Operation
	SS_ERROR_CODE_SS_ERROR_STATUS					= 17,	///< 0x11 SS Error Status
	SS_ERROR_CODE_SS_NOT_AVAILABLE					= 18,	///< 0x12 SS Not Available
	SS_ERROR_CODE_SS_SUBSCRIPTION_VIOLATION			= 19,	///< 0x13 SS Subscription Violation
	SS_ERROR_CODE_SS_INCOMPATIBILITY				= 20,	///< 0x14 SS Incompatibility
	SS_ERROR_CODE_FACILITY_NOT_SUPPORT				= 21,	///< 0x15 Facility Not Supported
	SS_ERROR_CODE_ABSENT_SUBSCRIBER					= 27,	///< 0x1B Absent Subscriber
	SS_ERROR_CODE_SHORT_TERM_DENIAL					= 29,	///< 0x1D Short Term Denial
	SS_ERROR_CODE_LONG_TERM_DENIAL					= 30, 	///< 0x1E Long Term Denial
	SS_ERROR_CODE_SYSTEM_FAILURE					= 34,	///< 0x22 System Failure
	SS_ERROR_CODE_DATA_MISSING						= 35,	///< 0x23 Data Missing
	SS_ERROR_CODE_UNEXPECT_DATA_VALUE				= 36,	///< 0x24 Unexpected Data Value
	SS_ERROR_CODE_PASSWORD_REGISTER_FAILURE			= 37,	///< 0x25 Password Register Failure
	SS_ERROR_CODE_NEGATIVE_PASSWORD_CHECK			= 38,	///< 0x26 Negative Password Check
	SS_ERROR_CODE_MAX_PASSWORD_ATTEMPTS_VOILATION	= 43,	///< 0x2B Max PWD Attempt Violation
	SS_ERROR_CODE_POSITION_METHODE_FAILURE			= 54,	///< 0x36 Position Methode Failure
	SS_ERROR_CODE_UNKNOWN_ALPHABET					= 71,	///< 0x47 Unknown Alphabet
	SS_ERROR_CODE_USSD_BUSY							= 72,	///< 0x48 USSD Busy
	SS_ERROR_CODE_NBR_SB_EXCEEDED					= 120,	///< 0x78 NBR SB Exceeded
	SS_ERROR_CODE_REJECTED_BY_USER					= 121,	///< 0x79 Rejected By User
	SS_ERROR_CODE_REJECTED_BY_NETWORK				= 122,	///< 0x7A Rejected By Network
	SS_ERROR_CODE_DEFLECTION_TO_SERVED_SUBSCRIBER	= 123,	///< 0x7B Deflection To Served Subscriber
	SS_ERROR_CODE_SPECIAL_SERVICE_CODE				= 124,	///< 0x7C Special Service Code
	SS_ERROR_CODE_INVALID_DEFLECTED_TO_NUMBER		= 125,	///< 0x7D Invalid Deflected To Number
	SS_ERROR_CODE_MAX_NUMBER_OF_MPTY_EXCEEDED		= 126,	///< 0x7E Max Number OF MPTY Exceeded
	SS_ERROR_CODE_RESOURCES_NOT_AVAILABLE_EXCEEDED	= 127	///< 0x7F Resources Not Available Exceeded
} SS_ErrorCode_t;
*/

/**
Enum : Problem Code Type
**/
typedef enum
{
	PROBLEM_CODE_TYPE_NONE,						///< Problem Code Type None
	PROBLEM_CODE_TYPE_GENERAL		= 0x80,		///< Problem Code Type General
	PROBLEM_CODE_TYPE_INVOKE		= 0x81,		///< Problem Code Type Invoke
	PROBLEM_CODE_TYPE_RETURN_RESULT	= 0x82,		///< Problem Code Type Return Result
	PROBLEM_CODE_TYPE_RETURN_ERROR	= 0x83		///< Problem Code Type Return Error
} SS_ProblemCode_t;								///< SS Problem Code


/**
Structure : Reject Component Type
**/
typedef struct
{
	SS_ProblemCode_t	problemType;
	UInt8				content;
} SS_Reject_t;	///< SS Reject Component Type



typedef struct
{
	PartyAddress_t		partyAdd;				///< Party Address
	PartySubAdd_t		partySubAdd;			///< Party Subaddress
	UInt8				noReplyTime;
} SS_CallForwardInfo_t;

typedef struct
{
	UInt8	currentPwd[SS_PASSWORD_LENGTH];		///< Current password
	UInt8	newPwd[SS_PASSWORD_LENGTH];			///< New password
	UInt8	reNewPwd[SS_PASSWORD_LENGTH];		///< Re-new password
	UInt8*	pwdPtr;								///< Pointer to the PWD to be used. NA to API level
} SS_Password_t;								///< SS Password Type

typedef struct
{
	UInt8	dcs;								///< Data Coding Scheme "GSM 3.38, section 5"
	UInt8	length;								///< Length
	UInt8	data[MAX_USSD_SIZE];				///< USSD string
}SS_UssdInfo_t;									///< SS USSD Information Type



/**
Enum : Return Error Component Type
**/
typedef struct
{
	SS_ErrorCode_t				errorCode;			///< Error Code
#if 0
	union
	{
		SS_CallBarredError_t	callBarredError;	///< Call Barred Error
		UInt8					ssStatus;			///< SS Status
		SS_SubsOption_t			subsOption;			///< Subscription Option
		SS_SsIncompatibility_t	ssIncompatib;		///< SS Incompatibility
		SS_MwdError_t			mwdSubsAbsent;		///< MWD Subscriber Absent Code
		SS_SystemFailure_t		systemFailure;		///< System Failure
		SS_PwdRegFailure_t		pwdRegFailure;		///< PWD Register Failure
	} paramError;
#endif
} SS_ReturnError_t;								///< Return Error Component Type

/**
Structure : SS API Service Request
**/
typedef struct
{
	SS_Operation_t	operation;					///< SS Operation
	BasicSrvGroup_t	basicSrv;					///< Basic Service
	SS_Code_t		ssCode;						///< SS Code 

	union
	{
		SS_CallForwardInfo_t	cfwInfo;		///< Call Forwarding Parameters
		SS_Password_t			ssPassword;		///< SS Password
		UInt8					ccbsIndex;		///< CCBS Index
		SS_UssdInfo_t			ussdInfo;		///< USSD Parameters
		LCS_AreaEventReport_t	areaEventRep;	///< LCS Area Event Report (SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT)
		LCS_TermCause_t			termCause;		///< LCS Termination Type (SS_OPERATION_CODE_LCS_LOCATION_UPDATE)
		LCS_MoLrReq_t			moLrReq;		///< LCS MO-LR Request (SS_OPERATION_CODE_LCS_MOLR)
		LCS_VerifRsp_t			verifRsp;		///< Verification Response (SS_OPERATION_CODE_LCS_LOCATION_NOTIFICATION)
	} param;

	Ticks_t			expTime;					///< Expiration Time [s]		

} SS_SrvReq_t;

/**
Structure : SS-API  Service Request
			Used for mobile originated 
**/
typedef struct
{
	ConfigMode_t	fdnCheck;				///< FDN Check
	ConfigMode_t	stkCheck;				///< STK Check
	SS_SrvReq_t		ssSrvReq;				///< SS Service Request
} SsApi_SrvReq_t;							///< SS API Service request

/**
Structure : SS-API USSD Service Request
			Used for mobile originated USSD
**/
typedef struct
{
	ConfigMode_t	fdnCheck;				///< FDN Check
	ConfigMode_t	stkCheck;				///< STK Check
	SS_Operation_t	operation;				///< SS Operation
	SS_UssdInfo_t	ussdInfo;				///< USSD Service Request
} SsApi_UssdSrvReq_t;						///< SS-API USSD Service Request

/**
Structure : SS-API USSD Service Data
			Used for continuation of USSD dialog with the network.
**/
typedef struct
{
	SS_Operation_t	operation;				///< SS Operation
	SS_UssdInfo_t	ussdInfo;				///< USSD Service Request
} SsApi_UssdDataReq_t;						///< SS-API USSD Data Request


/**
Structure : SS-API Data Request
			Used for continuation of SS dialog with the network.
**/
typedef struct
{
	SS_Operation_t	operation;					///< SS Operation

	union
	{
		SS_UssdInfo_t			ussdInfo;		///< USSD Information
		LCS_VerifRsp_t			verifRsp;		///< LCS Verification Response
		LCS_AreaEventReport_t	areaEventRep;	///< LCS Area Event Report (SS_OPERATION_CODE_LCS_AREA_EVENT_REPORT)
		LCS_MoLrReq_t			moLrReq;		///< LCS MO LR Request (SS_OPERATION_CODE_LCS_MOLR)
	} param;

} SsApi_DataReq_t;								///< SS-API USSD Data Request




/**
Structure : STK SS Service Response Type
**/
typedef struct
{
	ClientInfo_t	clientInfo;				///< Client Information       //Might not need it
	UInt8			callIndex;				///< Call Index            ///Just for old SS
	CauseIe_t		causeIe;				///< Cause IE
	SS_Operation_t	operation;				///< Operation
	SS_Component_t	component;				///< Component

	union
	{
		SS_ErrorCode_t		errorCode;		///< Error Code
		SS_Reject_t			reject;			///< Reject
	} param;								///< Parameters

	int				paramLen;				///< Parameter Length
	UInt8*			paramPtr;				///< Parameter Pointer
} STK_SsSrvRel_t;							///< STK SS Service Response Type


/**
Structure : SS Service Indication
**/
typedef struct
{
	ClientInfo_t		clientInfo;				///< Client Information
	UInt8				callIndex;				///< Call Index   //Used for the old SS
	SS_Operation_t		operation;				///< Operation
	SS_SrvType_t		type;					///< Service Response Type

	union
	{
		SS_UssdInfo_t	ussdInfo;				///< USSD Information
	} param;									///< Parameters

	int					facIeLength;			///< Facility IE Length
	UInt8				facIeData[MAX_FACILITY_IE_LENGTH]; ///< Facility IE data
}SS_SrvInd_t;									///< SS Service Indication Type

/**
Structure : SS Service Response
**/
typedef struct
{
	ClientInfo_t			clientInfo;			///< Client Information
	UInt8					callIndex;			///< Call Index
	SS_Component_t			component;			///< Component
	SS_Operation_t			operation;			///< Operation
	SS_Code_t				ssCode;				///< SS Code
	SS_SrvType_t			type;				///< Service Response Type

	union
	{
		SS_UssdInfo_t		ussdInfo;			///< USSD Information
	} param;

	int							facIeLength;	///< Facility IE Length
	UInt8				facIeData[MAX_FACILITY_IE_LENGTH]; ///< Facility IE data
} SS_SrvRsp_t;									///< SS Service Response


/**
Structure : SS Service Release
**/
typedef struct
{
	ClientInfo_t				clientInfo;			///< Client Information
	UInt8						callIndex;			///< Call Index
	SS_Component_t				component;			///< Component Type
	SS_Operation_t				operation;			///< Operation
	SS_Code_t					ssCode;				///< SS Code 
	BasicSrvGroup_t				basicSrv;			///< Basic Service
	SS_SrvType_t				type;				///< Service Response Type

	union
	{
		SS_FwdFeatureInfo_t		forwardInfo;		///< Forwarding Information
		SS_CallBarInfo_t		barringInfo;		///< Call Barring Information
		SS_SsDataInfo_t			ssDataInfo;			///< SS Data Information
		UInt8					ssStatus;			///< SS Status
		PartyAddress_t			fwdToPartyAdd;		///< Forwarded To Party Address
		SS_BasicSrvInfo_t		basicSrvInfo;		///< Basic Service Information
		SS_GenSrvInfo_t			genSrvInfo;			///< Generic Service Information
		UInt8					newPwd[SS_PASSWORD_LENGTH];	///< New password
		SS_UssdInfo_t			ussdInfo;			///< USSD Information

		SS_ReturnError_t		returnError;		///< Return Error
		SS_Reject_t				reject;				///< Reject
		Result_t				localResult;		///< Local causes
	} param;

	Result_t					localResult;		///< Local causes
	CauseIe_t					causeIe;			///< Network Cases

	int							facIeLength;		///< Facility IE Length
	UInt8				facIeData[MAX_FACILITY_IE_LENGTH]; ///< Facility IE data
} SS_SrvRel_t;										///< SS Service Reject

/**
Structure : CLS Service Indication
**/
typedef struct
{
	ClientInfo_t			clientInfo;			///< Client Information
	SS_Operation_t			operation;			///< Operation

	union
	{
		LCS_LocNotifInfo_t		locNotInfo;		///< Location Notification Information
		LCS_AreaEventReq_t		areaEventReq;	///< Area Event Request
		LCS_PeriodicLocReq_t	perLocReq;		///< Periodic Location Request
		LCS_XxCancel_t			xxCancel;		///< Area Event / Periodic Location Cancellation
		LCS_LocUpdate_t			locUpdate;		///< Location Update
	} param;									///< Parameters
}LCS_SrvInd_t;									///< CLS Service Indication Type


/**
Structure : CLS Service Response
**/
typedef struct
{
	ClientInfo_t			clientInfo;			///< Client Information
	SS_Operation_t			operation;			///< Operation

	union
	{
		LCS_MoLrRsp_t		molrRsp;			///< MO LR Response

	} param;									///< Parameters

}LCS_SrvRsp_t;									///< CLS Service Response Type


#endif
